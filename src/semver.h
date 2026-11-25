#ifndef SEMVER_H
#define SEMVER_H

#include <stddef.h>
#include <stdbool.h>

#define SEMVER_DOT            '.'
#define SEMVER_PRERELEASE_SYM '-'
#define SEMVER_BUILD_SYM      '+'
#define SEMVER_WILDCARD       '*'

typedef struct semver_str_slice {
	const char* str;
	size_t start;
	size_t end;
} semver_str_slice_t;

typedef enum version_restriction {
	VERSION_EQUALS,                  // =
	VERSION_MIN_INC,                 // >=
	VERSION_MAX_INC,                 // <=
	VERSION_MIN,                     // >
	VERSION_MAX,                     // <
	VERSION_LOCK_MOST_SIGNIFICANT,   // ^
	VERSION_ALLOW_LEAST_SIGNIFICANT, // ~
	VERSION_ANY                      // *
} version_restriction_t;

typedef enum version_number_type {
	VERSION_NUMBER_NONE,
	VERSION_NUMBER_ANY,
	VERSION_NUMBER_EXPLICIT
} version_number_type_t;

typedef struct semver_version_core {
	size_t major;
	size_t minor;
	size_t patch;
} semver_version_core_t;

typedef struct semver {
	semver_version_core_t version;
	char* prerelease_metadata;
	char* build_metadata;
} semver_t;

typedef struct cute_version_number {
	version_number_type_t type;
	size_t number;
} cute_version_number_t;

typedef struct cute_version_core {
	cute_version_number_t major;
	cute_version_number_t minor;
	cute_version_number_t patch;
} cute_version_core_t;

typedef struct cute_version {
	cute_version_core_t version;

	version_restriction_t restriction;
	cute_version_core_t min;
	bool min_inclusive;
	cute_version_core_t max;
	bool max_inclusive;

	char* prerelease_metadata;
	char* build_metadata;
} cute_version_t;

typedef enum semver_parse_status {
	SEMVER_VALID,
	SEMVER_INVALID,
	SEMVER_ERROR_MEMORY
} semver_parse_status_t;

// based on Backus–Naur Form Grammar for Valid SemVer Versions
// for semver 2.0.0
// https://semver.org/spec/v2.0.0.html

// ======================================================================

bool semver_is_letter(char c);
bool semver_is_positive_digit(char c);
bool semver_is_digit(char c);
bool semver_is_digits(const char* string, size_t length);
bool semver_is_non_digit(char c);
bool semver_is_identifier_character(char c);
bool semver_is_identifier_characters(const char* string, size_t length);
bool semver_is_numeric_identifier(const char* string, size_t length);
bool semver_is_alphanumeric_identifier(const char* string, size_t length);
bool semver_is_build_identifier(const char* string, size_t length);
bool semver_is_pre_release_identifier(const char* string, size_t length);
bool semver_is_dot_separated_build_identifiers(const char* string, size_t length);
bool semver_is_build(const char* string, size_t length);
bool semver_is_dot_separated_pre_release_identifiers(const char* string, size_t length);
bool semver_is_pre_release(const char* string, size_t length);
bool semver_is_patch(const char* string, size_t length);
bool semver_is_minor(const char* string, size_t length);
bool semver_is_major(const char* string, size_t length);
bool semver_is_version_core(const char* string, size_t length);
bool semver_is_valid_semver(const char* string, size_t length);

// ======================================================================

semver_version_core_t semver_version_core_new(void);
semver_version_core_t semver_version_core_parse(const char* string, size_t length);

semver_t semver_new(void);
semver_parse_status_t semver_parse(const char* string, size_t length, semver_t* semver);
void semver_free(semver_t* semver);

cute_version_t cute_version_new(void);
void cute_version_evaluate_restriction(cute_version_t* version);
void cute_version_free(cute_version_t* version);

int semver_version_core_cmp(const semver_version_core_t* x, const semver_version_core_t* y);
int semver_cmp(const semver_t* x, const semver_t* y);

void sort_semvers(semver_t* semvers, size_t count);
void sort_cute_versions(cute_version_t* versions, size_t count);

bool cute_version_satisfied(const cute_version_t* requirement, const semver_t* version);

#endif
