#include "cute-test.h"
#include "cute-version.h"

#define LEN(s) (sizeof(s) - 1)

CUTE_TEST(new_version_is_initialised) {
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(version.version.major.type, VERSION_NUMBER_NONE);
	CUTE_ASSERT_EQ(version.version.minor.type, VERSION_NUMBER_NONE);
	CUTE_ASSERT_EQ(version.version.patch.type, VERSION_NUMBER_NONE);

	CUTE_ASSERT_EQ(
		version.restriction,
		CUTE_VERSION_LOCK_MOST_SIGNIFICANT
	);

	CUTE_ASSERT(option_is_none(version.min));
	CUTE_ASSERT(option_is_none(version.max));

	CUTE_ASSERT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NULL(version.build_metadata);

	cute_version_free(&version);
}

CUTE_TEST(reparse_replaces_previous_metadata) {
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(
			"1.2.3-alpha+build",
			LEN("1.2.3-alpha+build"),
			&version
		),
		CUTE_VERSION_VALID
	);

	CUTE_ASSERT_NOT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NOT_NULL(version.build_metadata);

	CUTE_ASSERT_EQ(
		cute_version_parse("4.5.6", LEN("4.5.6"), &version),
		CUTE_VERSION_VALID
	);

	CUTE_ASSERT_EQ(version.version.major.number, 4);
	CUTE_ASSERT_EQ(version.version.minor.number, 5);
	CUTE_ASSERT_EQ(version.version.patch.number, 6);

	CUTE_ASSERT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NULL(version.build_metadata);

	cute_version_free(&version);
}

CUTE_TEST(reparse_replaces_previous_bounds) {
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse("^1.2.3", LEN("^1.2.3"), &version),
		CUTE_VERSION_VALID
	);

	CUTE_ASSERT(option_is_some(version.min));
	CUTE_ASSERT(option_is_some(version.max));

	CUTE_ASSERT_EQ(
		cute_version_parse("=4.5.6", LEN("=4.5.6"), &version),
		CUTE_VERSION_VALID
	);

	CUTE_ASSERT(option_is_none(version.min));
	CUTE_ASSERT(option_is_none(version.max));

	cute_version_free(&version);
}

CUTE_TEST(free_resets_version) {
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(
			"^1.2.3-alpha+build",
			LEN("^1.2.3-alpha+build"),
			&version
		),
		CUTE_VERSION_VALID
	);

	cute_version_free(&version);

	CUTE_ASSERT_EQ(version.version.major.type, VERSION_NUMBER_NONE);
	CUTE_ASSERT_EQ(version.version.minor.type, VERSION_NUMBER_NONE);
	CUTE_ASSERT_EQ(version.version.patch.type, VERSION_NUMBER_NONE);

	CUTE_ASSERT(option_is_none(version.min));
	CUTE_ASSERT(option_is_none(version.max));

	CUTE_ASSERT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NULL(version.build_metadata);
}

CUTE_TEST(free_is_safe_when_called_twice) {
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse("^1.2.3", LEN("^1.2.3"), &version),
		CUTE_VERSION_VALID
	);

	cute_version_free(&version);
	cute_version_free(&version);

	CUTE_ASSERT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NULL(version.build_metadata);
	CUTE_ASSERT(option_is_none(version.min));
	CUTE_ASSERT(option_is_none(version.max));
}

CUTE_TEST(free_accepts_null) {
	cute_version_free(NULL);
}

CUTE_TEST(null_parse_does_not_modify_existing_version) {
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(
			"1.2.3-alpha+build",
			LEN("1.2.3-alpha+build"),
			&version
		),
		CUTE_VERSION_VALID
	);

	CUTE_ASSERT_EQ(
		cute_version_parse(NULL, 0, &version),
		CUTE_VERSION_INVALID
	);

	CUTE_ASSERT_EQ(version.version.major.number, 1);
	CUTE_ASSERT_EQ(version.version.minor.number, 2);
	CUTE_ASSERT_EQ(version.version.patch.number, 3);

	CUTE_ASSERT_STR_EQ(version.prerelease_metadata, "alpha");
	CUTE_ASSERT_STR_EQ(version.build_metadata, "build");

	cute_version_free(&version);
}

int main(void) {
	CUTE_RUN(new_version_is_initialised);
	CUTE_RUN(reparse_replaces_previous_metadata);
	CUTE_RUN(reparse_replaces_previous_bounds);
	CUTE_RUN(free_resets_version);
	CUTE_RUN(free_is_safe_when_called_twice);
	CUTE_RUN(free_accepts_null);
	CUTE_RUN(null_parse_does_not_modify_existing_version);

	return cute_test_finish();
}
