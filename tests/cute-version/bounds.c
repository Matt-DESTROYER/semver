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

CUTE_TEST(caret_major_bounds) {
	CUTE_ASSERT(!satisfies("^1", "0.9.9"));
	CUTE_ASSERT(satisfies("^1", "1.0.0"));
	CUTE_ASSERT(satisfies("^1", "1.9.9"));
	CUTE_ASSERT(!satisfies("^1", "2.0.0"));

	CUTE_ASSERT(!satisfies("^2", "1.9.9"));
	CUTE_ASSERT(satisfies("^2", "2.0.0"));
	CUTE_ASSERT(!satisfies("^2", "3.0.0"));
}

CUTE_TEST(caret_major_minor_bounds) {
	CUTE_ASSERT(!satisfies("^1.2", "1.1.9"));
	CUTE_ASSERT(satisfies("^1.2", "1.2.0"));
	CUTE_ASSERT(satisfies("^1.2", "1.9.9"));
	CUTE_ASSERT(!satisfies("^1.2", "2.0.0"));
}

CUTE_TEST(caret_full_version_bounds) {
	CUTE_ASSERT(!satisfies("^1.2.3", "1.2.2"));
	CUTE_ASSERT(satisfies("^1.2.3", "1.2.3"));
	CUTE_ASSERT(satisfies("^1.2.3", "1.9.9"));
	CUTE_ASSERT(!satisfies("^1.2.3", "2.0.0"));
}

CUTE_TEST(caret_zero_major_bounds) {
	CUTE_ASSERT(!satisfies("^0", "0.0.0"));
	CUTE_ASSERT(satisfies("^0", "0.9.9"));
	CUTE_ASSERT(!satisfies("^0", "1.0.0"));

	CUTE_ASSERT(!satisfies("^0.2", "0.1.9"));
	CUTE_ASSERT(satisfies("^0.2", "0.2.0"));
	CUTE_ASSERT(satisfies("^0.2", "0.2.9"));
	CUTE_ASSERT(!satisfies("^0.2", "0.3.0"));

	CUTE_ASSERT(!satisfies("^0.2.3", "0.2.2"));
	CUTE_ASSERT(satisfies("^0.2.3", "0.2.3"));
	CUTE_ASSERT(satisfies("^0.2.3", "0.2.9"));
	CUTE_ASSERT(!satisfies("^0.2.3", "0.3.0"));
}

CUTE_TEST(caret_zero_major_minor_bounds) {
	CUTE_ASSERT(!satisfies("^0.0", "0.0.0"));
	CUTE_ASSERT(satisfies("^0.0", "0.0.9"));
	CUTE_ASSERT(!satisfies("^0.0", "0.1.0"));

	CUTE_ASSERT(!satisfies("^0.0.3", "0.0.2"));
	CUTE_ASSERT(satisfies("^0.0.3", "0.0.3"));
	CUTE_ASSERT(!satisfies("^0.0.3", "0.0.4"));
}

CUTE_TEST(caret_explicit_zero_components) {
	CUTE_ASSERT(satisfies("^0.1.0", "0.1.9"));
	CUTE_ASSERT(!satisfies("^0.1.0", "0.2.0"));

	CUTE_ASSERT(satisfies("^0.0.1", "0.0.1"));
	CUTE_ASSERT(!satisfies("^0.0.1", "0.0.2"));
}

CUTE_TEST(tilde_major_bounds) {
	CUTE_ASSERT(!satisfies("~1", "0.9.9"));
	CUTE_ASSERT(satisfies("~1", "1.0.0"));
	CUTE_ASSERT(satisfies("~1", "1.9.9"));
	CUTE_ASSERT(!satisfies("~1", "2.0.0"));

	CUTE_ASSERT(!satisfies("~0", "0.0.0"));
	CUTE_ASSERT(satisfies("~0", "0.9.9"));
	CUTE_ASSERT(!satisfies("~0", "1.0.0"));
}

CUTE_TEST(tilde_major_minor_bounds) {
	CUTE_ASSERT(!satisfies("~1.2", "1.1.9"));
	CUTE_ASSERT(satisfies("~1.2", "1.2.0"));
	CUTE_ASSERT(satisfies("~1.2", "1.2.9"));
	CUTE_ASSERT(!satisfies("~1.2", "1.3.0"));

	CUTE_ASSERT(!satisfies("~0.0", "0.0.0"));
	CUTE_ASSERT(satisfies("~0.0", "0.0.9"));
	CUTE_ASSERT(!satisfies("~0.0", "0.1.0"));
}

CUTE_TEST(tilde_full_version_bounds) {
	CUTE_ASSERT(!satisfies("~1.2.3", "1.2.2"));
	CUTE_ASSERT(satisfies("~1.2.3", "1.2.3"));
	CUTE_ASSERT(satisfies("~1.2.3", "1.2.9"));
	CUTE_ASSERT(!satisfies("~1.2.3", "1.3.0"));

	CUTE_ASSERT(!satisfies("~0.0.3", "0.0.2"));
	CUTE_ASSERT(satisfies("~0.0.3", "0.0.3"));
	CUTE_ASSERT(!satisfies("~0.0.3", "0.1.0"));
}

int main(void) {
	CUTE_RUN(caret_major_bounds);
	CUTE_RUN(caret_major_minor_bounds);
	CUTE_RUN(caret_full_version_bounds);
	CUTE_RUN(caret_zero_major_bounds);
	CUTE_RUN(caret_zero_major_minor_bounds);
	CUTE_RUN(caret_explicit_zero_components);
	CUTE_RUN(tilde_major_bounds);
	CUTE_RUN(tilde_major_minor_bounds);
	CUTE_RUN(tilde_full_version_bounds);

	return cute_test_finish();
}
