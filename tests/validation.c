#include "cute-test.h"
#include "semver.h"

#define VALID(s) \
	CUTE_ASSERT(semver_is_valid_semver((s), sizeof(s) - 1))

#define INVALID(s) \
	CUTE_ASSERT(!semver_is_valid_semver((s), sizeof(s) - 1))

CUTE_TEST(valid_core_versions) {
	VALID("0.0.0");
	VALID("1.0.0");
	VALID("0.1.0");
	VALID("0.0.1");
	VALID("1.2.3");
	VALID("10.20.30");
	VALID("999999.999999.999999");
}

CUTE_TEST(valid_prerelease_versions) {
	VALID("1.0.0-alpha");
	VALID("1.0.0-alpha.1");
	VALID("1.0.0-0.3.7");
	VALID("1.0.0-x.7.z.92");
	VALID("1.0.0-alpha-beta");
	VALID("1.0.0-alpha.beta");
	VALID("1.0.0-rc.1");
	VALID("1.0.0-0");
	VALID("1.0.0-1");
}

CUTE_TEST(valid_build_versions) {
	VALID("1.0.0+build");
	VALID("1.0.0+build.1");
	VALID("1.0.0+build.123");
	VALID("1.0.0+001");
	VALID("1.0.0+sha.abcdef");
	VALID("1.0.0+build-metadata");
	VALID("1.0.0+build.1.sha");
}

CUTE_TEST(valid_combined_metadata) {
	VALID("1.0.0-alpha+build");
	VALID("1.0.0-alpha.1+build.2");
	VALID("1.0.0-rc.1+sha.abcdef");
	VALID("1.0.0-0.3.7+build.123");
	VALID("1.0.0-alpha-beta+build-metadata");
}

CUTE_TEST(invalid_core_versions) {
	INVALID("");
	INVALID("1");
	INVALID("1.0");
	INVALID("1.0.0.0");
	INVALID(".1.0.0");
	INVALID("1..0.0");
	INVALID("1.0..0");
	INVALID("1.0.0.");
	INVALID("01.0.0");
	INVALID("1.01.0");
	INVALID("1.0.01");
	INVALID("-1.0.0");
	INVALID("1.-1.0");
	INVALID("1.0.-1");
	INVALID("1.a.0");
	INVALID("1.0.a");
	INVALID("v1.0.0");
	INVALID(" 1.0.0");
	INVALID("1.0.0 ");
	INVALID("1.2.3\n");
	INVALID("1.2.3\t");
	INVALID("1.2.3/");
	INVALID("1.2.3\\");
}

CUTE_TEST(invalid_prerelease_versions) {
	INVALID("1.0.0-");
	INVALID("1.0.0-alpha.");
	INVALID("1.0.0-.alpha");
	INVALID("1.0.0-alpha..1");
	INVALID("1.0.0-01");
	INVALID("1.0.0-00");
	INVALID("1.0.0-alpha.01");
	INVALID("1.0.0-alpha.00");
	INVALID("1.0.0-_alpha");
	INVALID("1.0.0-alpha_1");
	INVALID("1.0.0-alpha+");
}

CUTE_TEST(invalid_build_versions) {
	INVALID("1.0.0+");
	INVALID("1.0.0+build.");
	INVALID("1.0.0+.build");
	INVALID("1.0.0+build..123");
	INVALID("1.0.0+build_123");
	INVALID("1.0.0+build+other");
}

CUTE_TEST(invalid_metadata_order) {
	INVALID("1.0.0+build+other");
	INVALID("1.0.0-alpha+build+other");
	INVALID("1.0.0-alpha-beta+build+other");
}

CUTE_TEST(invalid_characters) {
	INVALID("1.0.0-alpha!");
	INVALID("1.0.0+build!");
	INVALID("1.0.0-alpha_beta");
	INVALID("1.0.0+build_metadata");
	INVALID("1.0.0#alpha");
	INVALID("1.0.0~alpha");
}

CUTE_TEST(invalid_prerelease_leading_zeroes) {
	INVALID("1.2.3-01");
	INVALID("1.2.3-001");
	INVALID("1.2.3-alpha.01");
}

CUTE_TEST(valid_prerelease_numbers) {
	VALID("1.2.3-0");
	VALID("1.2.3-1");
	VALID("1.2.3-alpha.0");
}

CUTE_TEST(valid_build_metadata_leading_zeroes) {
	VALID("1.2.3+01");
	VALID("1.2.3+001");
	VALID("1.2.3+build.0001");
}

CUTE_TEST(invalid_invalid_metadata_characters) {
	INVALID("1.2.3-alpha_1");
	INVALID("1.2.3+build@123");
	INVALID("1.2.3-alpha!");
	INVALID("1.2.3+build/123");
}

CUTE_TEST(valid_hyphenated_metadata) {
	VALID("1.0.0-alpha-beta");
	VALID("1.0.0-alpha-beta-gamma");
	VALID("1.0.0+build-metadata");
	VALID("1.0.0+build-123");
	VALID("1.0.0-alpha-beta+build-metadata");
	VALID("1.0.0-rc-1+sha-abcdef");
}

CUTE_TEST(invalid_embedded_null) {
	const char version[] = "1.0.0\0-invalid";

	CUTE_ASSERT(!semver_is_valid_semver(
		version,
		sizeof(version) - 1
	));

	const char leading_null[] = "\0.0.0";
	const char middle_null[] = "1.\0.0";
	const char trailing_null[] = "1.0.0\0";

	CUTE_ASSERT(!semver_is_valid_semver(leading_null, sizeof(leading_null) - 1));
	CUTE_ASSERT(!semver_is_valid_semver(middle_null, sizeof(middle_null) - 1));
	CUTE_ASSERT(!semver_is_valid_semver(trailing_null, sizeof(trailing_null) - 1));
}

int main(void) {
	CUTE_RUN(valid_core_versions);
	CUTE_RUN(valid_prerelease_versions);
	CUTE_RUN(valid_build_versions);
	CUTE_RUN(valid_combined_metadata);
	CUTE_RUN(valid_prerelease_numbers);
	CUTE_RUN(valid_build_metadata_leading_zeroes);
	CUTE_RUN(valid_hyphenated_metadata);

	CUTE_RUN(invalid_core_versions);
	CUTE_RUN(invalid_prerelease_versions);
	CUTE_RUN(invalid_build_versions);
	CUTE_RUN(invalid_metadata_order);
	CUTE_RUN(invalid_characters);
	CUTE_RUN(invalid_prerelease_leading_zeroes);
	CUTE_RUN(invalid_invalid_metadata_characters);
	CUTE_RUN(invalid_embedded_null);

	return cute_test_finish();
}
