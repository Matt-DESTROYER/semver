#include <string.h>

#include "cute-test.h"
#include "cute-version.h"

#define LEN(s) (sizeof(s) - 1)

static cute_version_t parse_requirement(const char* string) {
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(string, strlen(string), &version),
		CUTE_VERSION_VALID
	);

	return version;
}

static void assert_invalid_requirement(const char* string) {
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(string, strlen(string), &version),
		CUTE_VERSION_INVALID
	);

	cute_version_free(&version);
}

CUTE_TEST(parse_supported_operators) {
	struct {
		const char* string;
		version_restriction_t restriction;
	} cases[] = {
		{"=1.2.3", CUTE_VERSION_EQUALS},
		{">=1.2.3", CUTE_VERSION_MIN_INC},
		{"<=1.2.3", CUTE_VERSION_MAX_INC},
		{">1.2.3", CUTE_VERSION_MIN},
		{"<1.2.3", CUTE_VERSION_MAX},
		{"^1.2.3", CUTE_VERSION_LOCK_MOST_SIGNIFICANT},
		{"~1.2.3", CUTE_VERSION_ALLOW_LEAST_SIGNIFICANT},
		{"1.2.3", CUTE_VERSION_LOCK_MOST_SIGNIFICANT}
	};

	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		cute_version_t version = parse_requirement(cases[i].string);

		CUTE_ASSERT_EQ(version.restriction, cases[i].restriction);

		cute_version_free(&version);
	}
}

CUTE_TEST(parse_partial_versions) {
	struct {
		const char* string;
		size_t major;
		size_t minor;
		size_t patch;
		version_number_type_t minor_type;
		version_number_type_t patch_type;
	} cases[] = {
		{"1", 1, 0, 0, VERSION_NUMBER_NONE, VERSION_NUMBER_NONE},
		{"1.2", 1, 2, 0, VERSION_NUMBER_EXPLICIT, VERSION_NUMBER_NONE},
		{"1.2.3", 1, 2, 3, VERSION_NUMBER_EXPLICIT, VERSION_NUMBER_EXPLICIT},
		{"0", 0, 0, 0, VERSION_NUMBER_NONE, VERSION_NUMBER_NONE},
		{"0.0", 0, 0, 0, VERSION_NUMBER_EXPLICIT, VERSION_NUMBER_NONE},
		{"0.0.0", 0, 0, 0, VERSION_NUMBER_EXPLICIT, VERSION_NUMBER_EXPLICIT}
	};

	for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
		cute_version_t version = parse_requirement(cases[i].string);

		CUTE_ASSERT_EQ(version.version.major.number, cases[i].major);
		CUTE_ASSERT_EQ(version.version.minor.number, cases[i].minor);
		CUTE_ASSERT_EQ(version.version.patch.number, cases[i].patch);

		CUTE_ASSERT_EQ(version.version.major.type, VERSION_NUMBER_EXPLICIT);
		CUTE_ASSERT_EQ(version.version.minor.type, cases[i].minor_type);
		CUTE_ASSERT_EQ(version.version.patch.type, cases[i].patch_type);

		cute_version_free(&version);
	}
}

CUTE_TEST(parse_prerelease_metadata) {
	cute_version_t version = parse_requirement("1.2.3-alpha.1");

	CUTE_ASSERT_NOT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_STR_EQ(version.prerelease_metadata, "alpha.1");
	CUTE_ASSERT_NULL(version.build_metadata);

	cute_version_free(&version);
}

CUTE_TEST(parse_build_metadata) {
	cute_version_t version = parse_requirement("1.2.3+build.123");

	CUTE_ASSERT_NULL(version.prerelease_metadata);
	CUTE_ASSERT_NOT_NULL(version.build_metadata);
	CUTE_ASSERT_STR_EQ(version.build_metadata, "build.123");

	cute_version_free(&version);
}

CUTE_TEST(parse_both_metadata) {
	cute_version_t version = parse_requirement("1.2.3-alpha.1+build.123");

	CUTE_ASSERT_STR_EQ(version.prerelease_metadata, "alpha.1");
	CUTE_ASSERT_STR_EQ(version.build_metadata, "build.123");

	cute_version_free(&version);
}

CUTE_TEST(parse_metadata_with_restrictions) {
	const char* requirements[] = {
		"=1.2.3-alpha",
		">=1.2.3-alpha",
		"<=1.2.3-alpha",
		">1.2.3-alpha",
		"<1.2.3-alpha",
		"^1.2.3-alpha",
		"~1.2.3-alpha"
	};

	for (size_t i = 0; i < sizeof(requirements) / sizeof(requirements[0]); i++) {
		cute_version_t version = parse_requirement(requirements[i]);

		CUTE_ASSERT_NOT_NULL(version.prerelease_metadata);
		CUTE_ASSERT_STR_EQ(version.prerelease_metadata, "alpha");

		cute_version_free(&version);
	}
}

CUTE_TEST(reject_empty_and_operator_only_requirements) {
	assert_invalid_requirement("");
	assert_invalid_requirement("=");
	assert_invalid_requirement(">=");
	assert_invalid_requirement("<=");
	assert_invalid_requirement(">");
	assert_invalid_requirement("<");
	assert_invalid_requirement("^");
	assert_invalid_requirement("~");
}

CUTE_TEST(reject_malformed_version_cores) {
	assert_invalid_requirement("1..2");
	assert_invalid_requirement(".1");
	assert_invalid_requirement("1.");
	assert_invalid_requirement("1.2.3.4");
	assert_invalid_requirement("1abc.2.3");
	assert_invalid_requirement("1.2abc.3");
	assert_invalid_requirement("1.2.3abc");
	assert_invalid_requirement("01.2.3");
	assert_invalid_requirement("1.02.3");
	assert_invalid_requirement("1.2.03");
}

CUTE_TEST(reject_unsupported_wildcards) {
	assert_invalid_requirement("*"); // wildcards not currently supported
	assert_invalid_requirement("1.*");
	assert_invalid_requirement("1.2.*");
	assert_invalid_requirement("^*");
	assert_invalid_requirement(">=1.*");
}

CUTE_TEST(reject_malformed_metadata) {
	assert_invalid_requirement("1.2.3-");
	assert_invalid_requirement("1.2.3+");
	assert_invalid_requirement("1.2.3-alpha..1");
	assert_invalid_requirement("1.2.3-alpha+");
	assert_invalid_requirement("1.2.3-+build");
	assert_invalid_requirement("1.2.3-alpha+build+other");
}

CUTE_TEST(reject_whitespace) {
	assert_invalid_requirement(" 1.2.3");
	assert_invalid_requirement("1.2.3 ");
	assert_invalid_requirement("^ 1.2.3");
	assert_invalid_requirement("^1.2.3 ");
}

CUTE_TEST(parse_exact_length_buffer) {
	const char buffer[] = {'^', '1', '.', '2', '.', '3', 'x'};
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(buffer, 6, &version),
		CUTE_VERSION_VALID
	);

	CUTE_ASSERT_EQ(version.restriction, CUTE_VERSION_LOCK_MOST_SIGNIFICANT);
	CUTE_ASSERT_EQ(version.version.major.number, 1);
	CUTE_ASSERT_EQ(version.version.minor.number, 2);
	CUTE_ASSERT_EQ(version.version.patch.number, 3);

	cute_version_free(&version);
}

CUTE_TEST(reject_truncated_operator) {
	const char buffer[] = {'>', '='};
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(buffer, 1, &version),
		CUTE_VERSION_INVALID
	);

	cute_version_free(&version);
}

CUTE_TEST(reject_embedded_null) {
	const char buffer[] = {'1', '.', '2', '\0', '.', '3'};
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(buffer, sizeof(buffer), &version),
		CUTE_VERSION_INVALID
	);

	cute_version_free(&version);
}

CUTE_TEST(parse_null_arguments) {
	cute_version_t version = cute_version_new();

	CUTE_ASSERT_EQ(
		cute_version_parse(NULL, 0, &version),
		CUTE_VERSION_INVALID
	);

	CUTE_ASSERT_EQ(
		cute_version_parse("1.2.3", LEN("1.2.3"), NULL),
		CUTE_VERSION_INVALID
	);

	cute_version_free(&version);
}

int main(void) {
	CUTE_RUN(parse_supported_operators);
	CUTE_RUN(parse_partial_versions);
	CUTE_RUN(parse_prerelease_metadata);
	CUTE_RUN(parse_build_metadata);
	CUTE_RUN(parse_both_metadata);
	CUTE_RUN(parse_metadata_with_restrictions);
	CUTE_RUN(reject_empty_and_operator_only_requirements);
	CUTE_RUN(reject_malformed_version_cores);
	CUTE_RUN(reject_unsupported_wildcards);
	CUTE_RUN(reject_malformed_metadata);
	CUTE_RUN(reject_whitespace);
	CUTE_RUN(parse_exact_length_buffer);
	CUTE_RUN(reject_truncated_operator);
	CUTE_RUN(reject_embedded_null);
	CUTE_RUN(parse_null_arguments);

	return cute_test_finish();
}
