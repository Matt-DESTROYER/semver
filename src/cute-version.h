#ifndef CUTE_VERSION_H
#define CUTE_VERSION_H

#include <stdbool.h>
#include <copt.h>

#include "semver.h"

#define CUTE_VERSION_EQUALS_SYM                  "="
#define CUTE_VERSION_MIN_INC_SYM                 ">="
#define CUTE_VERSION_MAX_INC_SYM                 "<="
#define CUTE_VERSION_MIN_SYM                     ">"
#define CUTE_VERSION_MAX_SYM                     "<"
#define CUTE_VERSION_LOCK_MOST_SIGNIFICANT_SYM   "^"
#define CUTE_VERSION_ALLOW_LEAST_SIGNIFICANT_SYM "~"
#define CUTE_VERSION_ANY_SYM                     "*"

typedef enum version_restriction {
	CUTE_VERSION_EQUALS,                  // =
	CUTE_VERSION_MIN_INC,                 // >=
	CUTE_VERSION_MAX_INC,                 // <=
	CUTE_VERSION_MIN,                     // >
	CUTE_VERSION_MAX,                     // <
	CUTE_VERSION_LOCK_MOST_SIGNIFICANT,   // ^
	CUTE_VERSION_ALLOW_LEAST_SIGNIFICANT, // ~
	CUTE_VERSION_ANY                      // *
} version_restriction_t;

typedef enum version_number_type {
	VERSION_NUMBER_NONE,
	VERSION_NUMBER_EXPLICIT
} version_number_type_t;

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
	Option min;
	bool min_inclusive;
	Option max;
	bool max_inclusive;

	char* prerelease_metadata;
	char* build_metadata;
} cute_version_t;

typedef enum cute_version_parse_status {
	CUTE_VERSION_VALID,
	CUTE_VERSION_INVALID,
	CUTE_VERSION_ERROR_MEMORY
} cute_version_parse_status_t;

cute_version_t cute_version_new(void);
cute_version_parse_status_t cute_version_parse(const char* string, size_t length, cute_version_t* version);
void cute_version_evaluate_restriction(cute_version_t* version);
void cute_version_free(cute_version_t* version);

void sort_cute_versions(cute_version_t* versions, size_t count);

bool cute_version_satisfied(const cute_version_t* requirement, const semver_t* version);

#endif
