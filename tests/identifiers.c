#include "cute-test.h"
#include "semver.h"

#define LEN(s) (sizeof(s) - 1)

CUTE_TEST(letters) {
	CUTE_ASSERT(semver_is_letter('a'));
	CUTE_ASSERT(semver_is_letter('z'));
	CUTE_ASSERT(semver_is_letter('A'));
	CUTE_ASSERT(semver_is_letter('Z'));

	CUTE_ASSERT(!semver_is_letter('0'));
	CUTE_ASSERT(!semver_is_letter('-'));
	CUTE_ASSERT(!semver_is_letter('.'));
}

CUTE_TEST(digits) {
	CUTE_ASSERT(semver_is_digit('0'));
	CUTE_ASSERT(semver_is_digit('1'));
	CUTE_ASSERT(semver_is_digit('9'));

	CUTE_ASSERT(semver_is_positive_digit('1'));
	CUTE_ASSERT(semver_is_positive_digit('9'));
	CUTE_ASSERT(!semver_is_positive_digit('0'));

	CUTE_ASSERT(!semver_is_digit('a'));
	CUTE_ASSERT(!semver_is_digit('-'));
}

CUTE_TEST(numeric_identifiers) {
	CUTE_ASSERT(semver_is_numeric_identifier("0", 1));
	CUTE_ASSERT(semver_is_numeric_identifier("1", 1));
	CUTE_ASSERT(semver_is_numeric_identifier("123", 3));
	CUTE_ASSERT(semver_is_numeric_identifier("999999", 6));

	CUTE_ASSERT(!semver_is_numeric_identifier("", 0));
	CUTE_ASSERT(!semver_is_numeric_identifier("01", 2));
	CUTE_ASSERT(!semver_is_numeric_identifier("00", 2));
	CUTE_ASSERT(!semver_is_numeric_identifier("0123", 4));
	CUTE_ASSERT(!semver_is_numeric_identifier("1a", 2));
	CUTE_ASSERT(!semver_is_numeric_identifier("-1", 2));
}

CUTE_TEST(alphanumeric_identifiers) {
	CUTE_ASSERT(semver_is_alphanumeric_identifier("a", 1));
	CUTE_ASSERT(semver_is_alphanumeric_identifier("alpha", 5));
	CUTE_ASSERT(semver_is_alphanumeric_identifier("alpha1", 6));
	CUTE_ASSERT(semver_is_alphanumeric_identifier("1alpha", 6));
	CUTE_ASSERT(semver_is_alphanumeric_identifier("alpha-beta", 10));
	CUTE_ASSERT(semver_is_alphanumeric_identifier("a1-b2", 5));

	CUTE_ASSERT(!semver_is_alphanumeric_identifier("", 0));
	CUTE_ASSERT(!semver_is_alphanumeric_identifier("123", 3));
	CUTE_ASSERT(!semver_is_alphanumeric_identifier("alpha.beta", 10));
	CUTE_ASSERT(!semver_is_alphanumeric_identifier("alpha_beta", 10));
	CUTE_ASSERT(!semver_is_alphanumeric_identifier("alpha+", 6));
}

CUTE_TEST(build_identifiers) {
	CUTE_ASSERT(semver_is_build_identifier("0", 1));
	CUTE_ASSERT(semver_is_build_identifier("123", 3));
	CUTE_ASSERT(semver_is_build_identifier("build", 5));
	CUTE_ASSERT(semver_is_build_identifier("build-123", 9));

	CUTE_ASSERT(!semver_is_build_identifier("", 0));
	CUTE_ASSERT(!semver_is_build_identifier("build.123", 9));
	CUTE_ASSERT(!semver_is_build_identifier("build_123", 9));
}

CUTE_TEST(prerelease_identifiers) {
	CUTE_ASSERT(semver_is_pre_release_identifier("0", 1));
	CUTE_ASSERT(semver_is_pre_release_identifier("123", 3));
	CUTE_ASSERT(semver_is_pre_release_identifier("alpha", 5));
	CUTE_ASSERT(semver_is_pre_release_identifier("alpha-1", 7));
	CUTE_ASSERT(semver_is_pre_release_identifier("rc1", 3));

	CUTE_ASSERT(!semver_is_pre_release_identifier("", 0));
	CUTE_ASSERT(!semver_is_pre_release_identifier("01", 2));
	CUTE_ASSERT(!semver_is_pre_release_identifier("alpha.1", 7));
	CUTE_ASSERT(!semver_is_pre_release_identifier("alpha_1", 7));
}

CUTE_TEST(dot_separated_identifiers) {
	CUTE_ASSERT(semver_is_dot_separated_build_identifiers("build", 5));
	CUTE_ASSERT(semver_is_dot_separated_build_identifiers("build.123", 9));
	CUTE_ASSERT(semver_is_dot_separated_build_identifiers("build.123.sha", 13));
	CUTE_ASSERT(semver_is_dot_separated_build_identifiers("0.1.2", 5));

	CUTE_ASSERT(semver_is_dot_separated_pre_release_identifiers("alpha", 5));
	CUTE_ASSERT(semver_is_dot_separated_pre_release_identifiers("alpha.1", 7));
	CUTE_ASSERT(semver_is_dot_separated_pre_release_identifiers("alpha.beta.1", 12));
	CUTE_ASSERT(semver_is_dot_separated_pre_release_identifiers("rc.1", 4));

	CUTE_ASSERT(!semver_is_dot_separated_build_identifiers(".build", 6));
	CUTE_ASSERT(!semver_is_dot_separated_build_identifiers("build.", 6));
	CUTE_ASSERT(!semver_is_dot_separated_build_identifiers("build..123", 10));

	CUTE_ASSERT(!semver_is_dot_separated_pre_release_identifiers(".alpha", 6));
	CUTE_ASSERT(!semver_is_dot_separated_pre_release_identifiers("alpha.", 6));
	CUTE_ASSERT(!semver_is_dot_separated_pre_release_identifiers("alpha..1", 8));

	CUTE_ASSERT(!semver_is_dot_separated_build_identifiers("", 0));
	CUTE_ASSERT(!semver_is_dot_separated_build_identifiers(".", 1));
	CUTE_ASSERT(!semver_is_dot_separated_build_identifiers("..", 2));

	CUTE_ASSERT(!semver_is_dot_separated_pre_release_identifiers("", 0));
	CUTE_ASSERT(!semver_is_dot_separated_pre_release_identifiers(".", 1));
	CUTE_ASSERT(!semver_is_dot_separated_pre_release_identifiers("..", 2));
}

CUTE_TEST(numeric_identifier_leading_zero_rules) {
	CUTE_ASSERT(semver_is_build_identifier("001", 3));
	CUTE_ASSERT(!semver_is_pre_release_identifier("001", 3));

	CUTE_ASSERT(semver_is_dot_separated_build_identifiers("001.002", 7));
	CUTE_ASSERT(!semver_is_dot_separated_pre_release_identifiers("001.002", 7));

	CUTE_ASSERT(!semver_is_dot_separated_pre_release_identifiers("alpha.01", 8));
}

int main(void) {
	CUTE_RUN(letters);
	CUTE_RUN(digits);
	CUTE_RUN(numeric_identifiers);
	CUTE_RUN(alphanumeric_identifiers);
	CUTE_RUN(build_identifiers);
	CUTE_RUN(prerelease_identifiers);
	CUTE_RUN(dot_separated_identifiers);
	CUTE_RUN(numeric_identifier_leading_zero_rules);

	return cute_test_finish();
}
