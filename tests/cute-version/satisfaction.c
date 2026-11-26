#include <string.h>

#include "cute-test.h"
#include "cute-version.h"

static bool satisfies(const char* requirement_string, const char* candidate_string) {
	cute_version_t requirement = cute_version_new();
	semver_t candidate = semver_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(
			requirement_string,
			strlen(requirement_string),
			&requirement
		),
		CUTE_VERSION_VALID
	);

	CUTE_ASSERT_EQ(
		semver_parse(
			candidate_string,
			strlen(candidate_string),
			&candidate
		),
		SEMVER_VALID
	);

	bool result = cute_version_satisfied(&requirement, &candidate);

	cute_version_free(&requirement);
	semver_free(&candidate);

	return result;
}

CUTE_TEST(equality_satisfaction) {
	CUTE_ASSERT(!satisfies("=1.2.3", "1.2.2"));
	CUTE_ASSERT(satisfies("=1.2.3", "1.2.3"));
	CUTE_ASSERT(!satisfies("=1.2.3", "1.2.4"));
}

CUTE_TEST(minimum_inclusive_satisfaction) {
	CUTE_ASSERT(!satisfies(">=1.2.3", "1.2.2"));
	CUTE_ASSERT(satisfies(">=1.2.3", "1.2.3"));
	CUTE_ASSERT(satisfies(">=1.2.3", "1.2.4"));
}

CUTE_TEST(minimum_exclusive_satisfaction) {
	CUTE_ASSERT(!satisfies(">1.2.3", "1.2.2"));
	CUTE_ASSERT(!satisfies(">1.2.3", "1.2.3"));
	CUTE_ASSERT(satisfies(">1.2.3", "1.2.4"));
}

CUTE_TEST(maximum_inclusive_satisfaction) {
	CUTE_ASSERT(satisfies("<=1.2.3", "1.2.2"));
	CUTE_ASSERT(satisfies("<=1.2.3", "1.2.3"));
	CUTE_ASSERT(!satisfies("<=1.2.3", "1.2.4"));
}

CUTE_TEST(maximum_exclusive_satisfaction) {
	CUTE_ASSERT(satisfies("<1.2.3", "1.2.2"));
	CUTE_ASSERT(!satisfies("<1.2.3", "1.2.3"));
	CUTE_ASSERT(!satisfies("<1.2.3", "1.2.4"));
}

CUTE_TEST(comparison_operators_across_components) {
	CUTE_ASSERT(satisfies(">=1.2.3", "1.2.4"));
	CUTE_ASSERT(satisfies(">=1.2.3", "1.3.0"));
	CUTE_ASSERT(satisfies(">=1.2.3", "2.0.0"));

	CUTE_ASSERT(!satisfies("<1.2.3", "1.2.3"));
	CUTE_ASSERT(satisfies("<1.2.3", "1.2.2"));
	CUTE_ASSERT(satisfies("<1.2.3", "1.1.9"));
	CUTE_ASSERT(satisfies("<1.2.3", "0.9.9"));
}

CUTE_TEST(equality_prerelease) {
	CUTE_ASSERT(satisfies("=1.2.3-alpha", "1.2.3-alpha"));
	CUTE_ASSERT(!satisfies("=1.2.3-alpha", "1.2.3-beta"));
	CUTE_ASSERT(!satisfies("=1.2.3-alpha", "1.2.3"));
	CUTE_ASSERT(!satisfies("=1.2.3", "1.2.3-alpha"));
}

CUTE_TEST(equality_build_metadata) {
	CUTE_ASSERT(satisfies("=1.2.3+build.1", "1.2.3+build.2"));
	CUTE_ASSERT(satisfies("=1.2.3+build.1", "1.2.3"));
	CUTE_ASSERT(satisfies("=1.2.3", "1.2.3+build.2"));
}

CUTE_TEST(prerelease_range_boundaries) {
	CUTE_ASSERT(!satisfies(">=1.2.3-alpha", "1.2.2"));
	CUTE_ASSERT(satisfies(">=1.2.3-alpha", "1.2.3-alpha"));
	CUTE_ASSERT(satisfies(">=1.2.3-alpha", "1.2.3-beta"));
	CUTE_ASSERT(satisfies(">=1.2.3-alpha", "1.2.3"));
}

CUTE_TEST(prerelease_exclusive_range_boundaries) {
	CUTE_ASSERT(!satisfies(">1.2.3-alpha", "1.2.3-alpha"));
	CUTE_ASSERT(satisfies(">1.2.3-alpha", "1.2.3-beta"));
	CUTE_ASSERT(satisfies(">1.2.3-alpha", "1.2.3"));
}

CUTE_TEST(prerelease_maximum_boundaries) {
	CUTE_ASSERT(satisfies("<=1.2.3-alpha", "1.2.3-alpha"));
	CUTE_ASSERT(!satisfies("<=1.2.3-alpha", "1.2.3-beta"));
	CUTE_ASSERT(!satisfies("<=1.2.3-alpha", "1.2.3"));
}

int main(void) {
	CUTE_RUN(equality_satisfaction);
	CUTE_RUN(minimum_inclusive_satisfaction);
	CUTE_RUN(minimum_exclusive_satisfaction);
	CUTE_RUN(maximum_inclusive_satisfaction);
	CUTE_RUN(maximum_exclusive_satisfaction);
	CUTE_RUN(comparison_operators_across_components);
	CUTE_RUN(equality_prerelease);
	CUTE_RUN(equality_build_metadata);
	CUTE_RUN(prerelease_range_boundaries);
	CUTE_RUN(prerelease_exclusive_range_boundaries);
	CUTE_RUN(prerelease_maximum_boundaries);

	return cute_test_finish();
}
