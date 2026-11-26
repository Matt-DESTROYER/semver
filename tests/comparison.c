#include <string.h>

#include "cute-test.h"
#include "semver.h"

#define LEN(s) (sizeof(s) - 1)

static semver_t parse_version(const char* string) {
	semver_t version = semver_new();

	CUTE_ASSERT_EQ(
		semver_parse(string, strlen(string), &version),
		SEMVER_VALID
	);

	return version;
}

CUTE_TEST(core_version_comparison) {
	semver_version_core_t x = {1, 2, 3};
	semver_version_core_t y = {1, 2, 4};

	CUTE_ASSERT_EQ(semver_version_core_cmp(&x, &x), 0);
	CUTE_ASSERT_EQ(semver_version_core_cmp(&x, &y), -1);
	CUTE_ASSERT_EQ(semver_version_core_cmp(&y, &x), 1);

	x = (semver_version_core_t){1, 3, 0};
	y = (semver_version_core_t){1, 2, 999};

	CUTE_ASSERT_EQ(semver_version_core_cmp(&x, &y), 1);

	x = (semver_version_core_t){2, 0, 0};
	y = (semver_version_core_t){1, 999, 999};

	CUTE_ASSERT_EQ(semver_version_core_cmp(&x, &y), 1);
}

CUTE_TEST(stable_version_comparison) {
	semver_t x = parse_version("1.2.3");
	semver_t y = parse_version("1.2.4");

	CUTE_ASSERT_EQ(semver_cmp(&x, &x), 0);
	CUTE_ASSERT_EQ(semver_cmp(&x, &y), -1);
	CUTE_ASSERT_EQ(semver_cmp(&y, &x), 1);

	semver_free(&x);
	semver_free(&y);
}

CUTE_TEST(stable_versions_outrank_prereleases) {
	semver_t stable = parse_version("1.0.0");
	semver_t prerelease = parse_version("1.0.0-alpha");

	CUTE_ASSERT_EQ(semver_cmp(&prerelease, &stable), -1);
	CUTE_ASSERT_EQ(semver_cmp(&stable, &prerelease), 1);

	semver_free(&stable);
	semver_free(&prerelease);
}

CUTE_TEST(semver_spec_precedence_sequence) {
	const char* versions[] = {
		"1.0.0-alpha",
		"1.0.0-alpha.1",
		"1.0.0-alpha.beta",
		"1.0.0-beta",
		"1.0.0-beta.2",
		"1.0.0-beta.11",
		"1.0.0-rc.1",
		"1.0.0"
	};

	for (size_t i = 0; i < sizeof(versions) / sizeof(versions[0]) - 1; i++) {
		semver_t x = parse_version(versions[i]);
		semver_t y = parse_version(versions[i + 1]);

		CUTE_ASSERT_EQ(semver_cmp(&x, &y), -1);
		CUTE_ASSERT_EQ(semver_cmp(&y, &x), 1);

		semver_free(&x);
		semver_free(&y);
	}
}

CUTE_TEST(prerelease_numeric_identifiers) {
	semver_t x = parse_version("1.0.0-alpha.2");
	semver_t y = parse_version("1.0.0-alpha.10");

	CUTE_ASSERT_EQ(semver_cmp(&x, &y), -1);

	semver_free(&x);
	semver_free(&y);
}

CUTE_TEST(prerelease_numeric_precedes_alphanumeric) {
	semver_t x = parse_version("1.0.0-alpha.1");
	semver_t y = parse_version("1.0.0-alpha.beta");

	CUTE_ASSERT_EQ(semver_cmp(&x, &y), -1);

	semver_free(&x);
	semver_free(&y);
}

CUTE_TEST(prerelease_identifier_length) {
	semver_t x = parse_version("1.0.0-alpha");
	semver_t y = parse_version("1.0.0-alpha.1");

	CUTE_ASSERT_EQ(semver_cmp(&x, &y), -1);

	semver_free(&x);
	semver_free(&y);
}

CUTE_TEST(prerelease_lexical_comparison) {
	semver_t x = parse_version("1.0.0-alpha");
	semver_t y = parse_version("1.0.0-beta");

	CUTE_ASSERT_EQ(semver_cmp(&x, &y), -1);

	semver_free(&x);
	semver_free(&y);
}

CUTE_TEST(build_metadata_does_not_affect_precedence) {
	semver_t x = parse_version("1.0.0+build.1");
	semver_t y = parse_version("1.0.0+build.2");

	CUTE_ASSERT_EQ(semver_cmp(&x, &y), 0);

	semver_free(&x);
	semver_free(&y);
}

CUTE_TEST(prerelease_precedence_ignores_build_metadata) {
	semver_t x = parse_version("1.0.0-alpha+build.1");
	semver_t y = parse_version("1.0.0-alpha+build.2");

	CUTE_ASSERT_EQ(semver_cmp(&x, &y), 0);

	semver_free(&x);
	semver_free(&y);
}

CUTE_TEST(sort_versions) {
	semver_t versions[] = {
		parse_version("1.0.0"),
		parse_version("1.0.0-beta"),
		parse_version("2.0.0"),
		parse_version("1.0.0-alpha"),
		parse_version("1.0.0-beta.1")
	};

	sort_semvers(versions, sizeof(versions) / sizeof(versions[0]));

	const char* expected[] = {
		"1.0.0-alpha",
		"1.0.0-beta",
		"1.0.0-beta.1",
		"1.0.0",
		"2.0.0"
	};

	for (size_t i = 0; i < sizeof(expected) / sizeof(expected[0]); i++) {
		semver_t expected_version = parse_version(expected[i]);

		CUTE_ASSERT_EQ(
			semver_cmp(&versions[i], &expected_version),
			0
		);

		semver_free(&expected_version);
	}

	for (size_t i = 0; i < sizeof(versions) / sizeof(versions[0]); i++)
		semver_free(&versions[i]);
}

CUTE_TEST(sort_empty_and_single_element_arrays) {
	semver_t version = parse_version("1.2.3");

	sort_semvers(NULL, 0);
	sort_semvers(&version, 1);

	CUTE_ASSERT_EQ(version.version.major, 1);
	CUTE_ASSERT_EQ(version.version.minor, 2);
	CUTE_ASSERT_EQ(version.version.patch, 3);

	semver_free(&version);
}

int main(void) {
	CUTE_RUN(core_version_comparison);
	CUTE_RUN(stable_version_comparison);
	CUTE_RUN(stable_versions_outrank_prereleases);
	CUTE_RUN(semver_spec_precedence_sequence);
	CUTE_RUN(prerelease_numeric_identifiers);
	CUTE_RUN(prerelease_numeric_precedes_alphanumeric);
	CUTE_RUN(prerelease_identifier_length);
	CUTE_RUN(prerelease_lexical_comparison);
	CUTE_RUN(build_metadata_does_not_affect_precedence);
	CUTE_RUN(prerelease_precedence_ignores_build_metadata);
	CUTE_RUN(sort_versions);
	CUTE_RUN(sort_empty_and_single_element_arrays);

	return cute_test_finish();
}
