#include "cute-test.h"
#include "semver.h"

#include <stdio.h>
#include <string.h>

#define LEN(s) (sizeof(s) - 1)

CUTE_TEST(parse_core_version) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse("1.2.3", LEN("1.2.3"), &version),
		SEMVER_VALID
	);

	CUTE_ASSERT_EQ(version.version.major.type, VERSION_NUMBER_EXPLICIT);
	CUTE_ASSERT_EQ(version.version.major.number, 1);

	CUTE_ASSERT_EQ(version.version.minor.type, VERSION_NUMBER_EXPLICIT);
	CUTE_ASSERT_EQ(version.version.minor.number, 2);

	CUTE_ASSERT_EQ(version.version.patch.type, VERSION_NUMBER_EXPLICIT);
	CUTE_ASSERT_EQ(version.version.patch.number, 3);

	CUTE_ASSERT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NULL(version.build_metadata);

	semver_free(&version);
}

CUTE_TEST(parse_zero_version) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse("0.0.0", LEN("0.0.0"), &version),
		SEMVER_VALID
	);

	CUTE_ASSERT_EQ(version.version.major.number, 0);
	CUTE_ASSERT_EQ(version.version.minor.number, 0);
	CUTE_ASSERT_EQ(version.version.patch.number, 0);

	semver_free(&version);
}

CUTE_TEST(parse_prerelease_metadata) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse("1.2.3-alpha.1", LEN("1.2.3-alpha.1"), &version),
		SEMVER_VALID
	);

	CUTE_ASSERT_NOT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_STR_EQ(version.prerelease_metadata, "alpha.1");
	CUTE_ASSERT_NULL(version.build_metadata);

	semver_free(&version);
}

CUTE_TEST(parse_build_metadata) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse("1.2.3+build.123", LEN("1.2.3+build.123"), &version),
		SEMVER_VALID
	);

	CUTE_ASSERT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NOT_NULL(version.build_metadata);
	CUTE_ASSERT_STR_EQ(version.build_metadata, "build.123");

	semver_free(&version);
}

CUTE_TEST(parse_both_metadata) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3-alpha.1+build.123",
			LEN("1.2.3-alpha.1+build.123"),
			&version
		),
		SEMVER_VALID
	);

	CUTE_ASSERT_NOT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NOT_NULL(version.build_metadata);

	CUTE_ASSERT_STR_EQ(version.prerelease_metadata, "alpha.1");
	CUTE_ASSERT_STR_EQ(version.build_metadata, "build.123");

	semver_free(&version);
}

CUTE_TEST(parse_build_without_prerelease) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse("1.2.3+build", LEN("1.2.3+build"), &version),
		SEMVER_VALID
	);

	CUTE_ASSERT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NOT_NULL(version.build_metadata);
	CUTE_ASSERT_STR_EQ(version.build_metadata, "build");

	semver_free(&version);
}

CUTE_TEST(reject_invalid_versions) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse("1.2", LEN("1.2"), &version),
		SEMVER_INVALID
	);

	CUTE_ASSERT_EQ(
		semver_parse("01.2.3", LEN("01.2.3"), &version),
		SEMVER_INVALID
	);

	CUTE_ASSERT_EQ(
		semver_parse("1.2.3-alpha..1", LEN("1.2.3-alpha..1"), &version),
		SEMVER_INVALID
	);

	semver_free(&version);
}

CUTE_TEST(parse_null_arguments) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse(NULL, 0, &version),
		SEMVER_INVALID
	);

	CUTE_ASSERT_EQ(
		semver_parse("1.2.3", LEN("1.2.3"), NULL),
		SEMVER_INVALID
	);

	semver_free(&version);
}

CUTE_TEST(free_metadata) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3-alpha+build",
			LEN("1.2.3-alpha+build"),
			&version
		),
		SEMVER_VALID
	);

	semver_free(&version);

	CUTE_ASSERT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NULL(version.build_metadata);
	CUTE_ASSERT_EQ(version.version.major.type, VERSION_NUMBER_NONE);
	CUTE_ASSERT_EQ(version.version.minor.type, VERSION_NUMBER_NONE);
	CUTE_ASSERT_EQ(version.version.patch.type, VERSION_NUMBER_NONE);
}

CUTE_TEST(parse_metadata_with_hyphens) {
	semver_t semver = {0};

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3-alpha-beta",
			LEN("1.2.3-alpha-beta"),
			&semver
		),
		SEMVER_VALID
	);
	CUTE_ASSERT_STR_EQ(semver.prerelease_metadata, "alpha-beta");
	semver_free(&semver);

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3+build-metadata",
			LEN("1.2.3+build-metadata"),
			&semver
		),
		SEMVER_VALID
	);
	CUTE_ASSERT_STR_EQ(semver.build_metadata, "build-metadata");
	semver_free(&semver);

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3-alpha-beta+build-metadata",
			LEN("1.2.3-alpha-beta+build-metadata"),
			&semver
		),
		SEMVER_VALID
	);
	CUTE_ASSERT_STR_EQ(semver.prerelease_metadata, "alpha-beta");
	CUTE_ASSERT_STR_EQ(semver.build_metadata, "build-metadata");
	semver_free(&semver);
}

CUTE_TEST(parse_multiple_metadata_identifiers) {
	semver_t semver = {0};

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3-alpha.1.beta.2+build.123.metadata",
			LEN("1.2.3-alpha.1.beta.2+build.123.metadata"),
			&semver
		),
		SEMVER_VALID
	);

	CUTE_ASSERT_STR_EQ(semver.prerelease_metadata, "alpha.1.beta.2");
	CUTE_ASSERT_STR_EQ(semver.build_metadata, "build.123.metadata");

	semver_free(&semver);
}

CUTE_TEST(parse_metadata_boundaries) {
	semver_t semver = {0};

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3-alpha",
			LEN("1.2.3-alpha"),
			&semver
		),
		SEMVER_VALID
	);
	CUTE_ASSERT_STR_EQ(semver.prerelease_metadata, "alpha");
	CUTE_ASSERT(semver.build_metadata == NULL);
	semver_free(&semver);

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3+build",
			LEN("1.2.3+build"),
			&semver
		),
		SEMVER_VALID
	);
	CUTE_ASSERT(semver.prerelease_metadata == NULL);
	CUTE_ASSERT_STR_EQ(semver.build_metadata, "build");
	semver_free(&semver);
}

CUTE_TEST(reject_empty_metadata) {
	semver_t semver = {0};

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3-",
			LEN("1.2.3-"),
			&semver
		),
		SEMVER_INVALID
	);
	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3+",
			LEN("1.2.3+"),
			&semver
		),
		SEMVER_INVALID
	);
	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3-+build",
			LEN("1.2.3-+build"),
			&semver
		),
		SEMVER_INVALID
	);
	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.3-alpha+",
			LEN("1.2.3-alpha+"),
			&semver
		),
		SEMVER_INVALID
	);
}

CUTE_TEST(parse_long_metadata) {
	char version[1024];
	semver_t semver = {0};

	memset(version, 'a', 500);
	version[500] = '\0';

	snprintf(version, sizeof(version), "1.2.3-%s+%s",
			 "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
			 "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");

	CUTE_ASSERT_EQ(semver_parse(
		version,
		strlen(version),
		&semver
	), SEMVER_VALID);
	CUTE_ASSERT_STR_EQ(
		semver.prerelease_metadata,
		"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
	);
	CUTE_ASSERT_STR_EQ(
		semver.build_metadata,
		"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
	);

	semver_free(&semver);
}

CUTE_TEST(reject_core_leading_zeroes) {
	semver_t semver = {0};

	CUTE_ASSERT_EQ(
		semver_parse(
			"01.2.3",
			LEN("01.2.3"),
			&semver
		),
		SEMVER_INVALID
	);
	CUTE_ASSERT_EQ(
		semver_parse(
			"1.02.3",
			LEN("1.02.3"),
			&semver
		),
		SEMVER_INVALID
	);
	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.03",
			LEN("1.2.03"),
			&semver
		),
		SEMVER_INVALID
	);
}

CUTE_TEST(reject_missing_core_components) {
	semver_t semver = {0};

	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2",
			LEN("1.2"),
			&semver
		),
		SEMVER_INVALID
	);
	CUTE_ASSERT_EQ(
		semver_parse(
			"1",
			LEN("1"),
			&semver
		),
		SEMVER_INVALID
	);
	CUTE_ASSERT_EQ(
		semver_parse(
			".2.3",
			LEN(".2.3"),
			&semver
		),
		SEMVER_INVALID
	);
	CUTE_ASSERT_EQ(
		semver_parse(
			"1..3",
			LEN("1..3"),
			&semver
		),
		SEMVER_INVALID
	);
	CUTE_ASSERT_EQ(
		semver_parse(
			"1.2.",
			LEN("1.2."),
			&semver
		),
		SEMVER_INVALID
	);
}

int main(void) {
	CUTE_RUN(parse_core_version);
	CUTE_RUN(parse_zero_version);
	CUTE_RUN(parse_prerelease_metadata);
	CUTE_RUN(parse_build_metadata);
	CUTE_RUN(parse_both_metadata);
	CUTE_RUN(parse_build_without_prerelease);
	CUTE_RUN(reject_invalid_versions);
	CUTE_RUN(parse_null_arguments);
	CUTE_RUN(free_metadata);
	CUTE_RUN(parse_metadata_with_hyphens);
	CUTE_RUN(parse_multiple_metadata_identifiers);
	CUTE_RUN(parse_metadata_boundaries);
	CUTE_RUN(reject_empty_metadata);
	CUTE_RUN(parse_long_metadata);
	CUTE_RUN(reject_core_leading_zeroes);
	CUTE_RUN(reject_missing_core_components);

	return cute_test_finish();
}
