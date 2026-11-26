#ifndef SEMVER_H
#define SEMVER_H

#include <stddef.h>
#include <stdbool.h>
#include <copt.h>

#define SEMVER_DOT            '.'
#define SEMVER_PRERELEASE_SYM '-'
#define SEMVER_BUILD_SYM      '+'

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
semver_t semver_from_semver_version_core(semver_version_core_t version);
semver_parse_status_t semver_parse(const char* string, size_t length, semver_t* semver);
void semver_free(semver_t* semver);

int semver_version_core_cmp(const semver_version_core_t* x, const semver_version_core_t* y);
int semver_cmp(const semver_t* x, const semver_t* y);

void sort_semvers(semver_t* semvers, size_t count);

#endif
