#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "semver.h"

semver_str_slice_t semver_slice(const char* str, size_t start, size_t end) {
	return (semver_str_slice_t){
		.str = str,
		.start = start,
		.end = end
	};
}

// based on Backus–Naur Form Grammar for Valid SemVer Versions
// for semver 2.0.0
// https://semver.org/spec/v2.0.0.html

// ======================================================================

bool semver_is_letter(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool semver_is_positive_digit(char c) {
	return c >= '1' && c <= '9';
}

bool semver_is_digit(char c) {
	return c == '0' || semver_is_positive_digit(c);
}

bool semver_is_digits(const char* string, size_t length) {
	if (string == NULL || length == 0 || string[0] == '\0')
		return false;

	for (size_t i = 0; i < length && string[i] != '\0'; i++) {
		if (!semver_is_digit(string[i]))
			return false;
	}
	return true;
}

bool semver_is_non_digit(char c) {
	return semver_is_letter(c) || c == '-';
}

bool semver_is_identifier_character(char c) {
	return semver_is_digit(c) || semver_is_non_digit(c);
}

bool semver_is_identifier_characters(const char* string, size_t length) {
	for (size_t i = 0; i < length && string[i] != '\0'; i++) {
		if (!semver_is_identifier_character(string[i]))
			return false;
	}
	return true;
}

bool semver_is_numeric_identifier(const char* string, size_t length) {
	if (string == NULL || length == 0 || string[0] == '\0')
		return false;

	if (string[0] == '0')
		return length == 1 || string[1] == '\0';

	if (!semver_is_positive_digit(string[0]))
		return false;

	for (size_t i = 1; i < length && string[i] != '\0'; i++) {
		if (!semver_is_digit(string[i]))
			return false;
	}
	return true;
}

bool semver_is_alphanumeric_identifier(const char* string, size_t length) {
	if (string == NULL || length == 0 || string[0] == '\0')
		return false;

	bool found_non_digit = false;
	for (size_t i = 0; i < length && string[i] != '\0'; i++) {
		if (semver_is_non_digit(string[i])) {
			found_non_digit = true;
		} else if (!semver_is_identifier_character(string[i])) {
			return false;
		}
	}
	return found_non_digit;
}

bool semver_is_build_identifier(const char* string, size_t length) {
	return semver_is_alphanumeric_identifier(string, length)
		|| semver_is_digits(string, length);
}

bool semver_is_pre_release_identifier(const char* string, size_t length) {
	return semver_is_alphanumeric_identifier(string, length)
		|| semver_is_numeric_identifier(string, length);
}

bool semver_is_dot_separated_build_identifiers(const char* string, size_t length) {
	if (string == NULL || length == 0 || string[0] == '\0')
		return false;

	if (semver_is_build_identifier(string, length))
		return true;

	// catch leading SEMVER_DOT
	if (string[0] == SEMVER_DOT)
		return false;

	bool seen_dot = false;
	size_t prev_dot = 0;
	size_t i = 0;
	for (; i < length && string[i] != '\0'; i++) {
		if (string[i] == SEMVER_DOT) {
			if (seen_dot && prev_dot == i - 1)
				return false;
			
			if (
				!semver_is_build_identifier(
					seen_dot ? (const char*)(string + prev_dot + 1) : string,
					seen_dot ? i - prev_dot + 1 : i
				)
			)
				return false;

			prev_dot = i;
			seen_dot = true;
		}
	}
	
	if (seen_dot && prev_dot + 1 >= i)
		return false;

	return semver_is_build_identifier(
		seen_dot ? (const char*)(string + prev_dot + 1) : string,
		seen_dot ? i - prev_dot + 1 : i
	);
}

bool semver_is_build(const char* string, size_t length) {
	return semver_is_dot_separated_build_identifiers(string, length);
}

bool semver_is_dot_separated_pre_release_identifiers(const char* string, size_t length) {
	if (string == NULL || length == 0 || string[0] == '\0')
		return false;

	if (semver_is_pre_release_identifier(string, length))
		return true;

	// catch leading SEMVER_DOT
	if (string[0] == SEMVER_DOT)
		return false;

	bool seen_dot = false;
	size_t prev_dot = 0;
	size_t i = 0;
	for (; i < length && string[i] != '\0'; i++) {
		if (string[i] == SEMVER_DOT) {
			if (seen_dot && prev_dot == i - 1)
				return false;
			if (
				!semver_is_pre_release_identifier(
					seen_dot ? (const char*)(string + prev_dot + 1) : string,
					seen_dot ? i - prev_dot + 1 : i
				)
			)
				return false;

			prev_dot = i;
			seen_dot = true;
		}
	}
	
	if (seen_dot && prev_dot + 1 >= i)
		return false;

	return semver_is_pre_release_identifier(
		seen_dot ? (const char*)(string + prev_dot + 1) : string,
		seen_dot ? i - prev_dot + 1 : i
	);
}

bool semver_is_pre_release(const char* string, size_t length) {
	return semver_is_dot_separated_pre_release_identifiers(string, length);
}

bool semver_is_patch(const char* string, size_t length) {
	return semver_is_numeric_identifier(string, length);
}

bool semver_is_minor(const char* string, size_t length) {
	return semver_is_numeric_identifier(string, length);
}

bool semver_is_major(const char* string, size_t length) {
	return semver_is_numeric_identifier(string, length);
}

bool semver_is_version_core(const char* string, size_t length) {
	if (string == NULL || length == 0 || string[0] == '\0')
		return false;

	if (string[0] == SEMVER_DOT)
		return false;

	unsigned short int version_core_idx = 0;
	bool seen_dot = false;
	size_t prev_dot = 0;
	size_t i = 0;
	for (; i < length && string[i] != '\0'; i++) {
		if (string[i] == SEMVER_DOT) {
			if (seen_dot && prev_dot == i - 1)
				return false;

			switch (version_core_idx) {
				case 0:
					if (!semver_is_major((const char*)(string + prev_dot + 1), i - prev_dot - 1))
						return false;
					break;
				case 1:
					if (!semver_is_minor((const char*)(string + prev_dot + 1), i - prev_dot - 1))
						return false;
					break;
				// this means there's an extra dot...
				case 2:
					return false;
			}

			version_core_idx++;
			prev_dot = i;
			seen_dot = true;
		}
	}

	if (!seen_dot || prev_dot + 1 >= i)
		return false;

	if (version_core_idx != 2)
		return false;

	if (!semver_is_patch((const char*)(string + prev_dot + 1), i - prev_dot - 1))
		return false;

	return true;
}

bool semver_is_valid_semver(const char* string, size_t length) {
	if (string == NULL || length == 0 || string[0] == '\0')
		return false;

	bool found_symbol = false;
	size_t prev_symbol = 0;
	size_t i = 0;
	for (; i < length && string[i] != '\0'; i++) {
		if (string[i] == SEMVER_PRERELEASE_SYM) {
			// spec doesn't allow build then pre-release
			if (found_symbol)
				return false;

			if (!semver_is_version_core(string, i))
				return false;

			found_symbol = true;
		} else if (string[i] == SEMVER_BUILD_SYM) {
			if (!found_symbol) {
				if (!semver_is_version_core(string, i))
					return false;
			} else {
				if (string[prev_symbol] != SEMVER_PRERELEASE_SYM)
					return false;

				if (prev_symbol + 1 >= i)
					return false;

				if (!semver_is_pre_release((const char*)(string + prev_symbol + 1), i - prev_symbol - 1))
					return false;
			}
			found_symbol = true;
		}

		prev_symbol = i;
	}

	if (found_symbol) {
		if (string[prev_symbol] == SEMVER_PRERELEASE_SYM) {
			return semver_is_pre_release((const char*)(string + prev_symbol + 1), i - prev_symbol - 1);
		} else if (string[prev_symbol] == SEMVER_BUILD_SYM) {
			return semver_is_build((const char*)(string + prev_symbol + 1), i - prev_symbol - 1);
		}
	}

	return semver_is_version_core(string, i);
}

// ======================================================================

version_number_t version_number_new() {
	return (version_number_t){
		.type = VERSION_NUMBER_NONE,
		.number = 0
	};
}

version_t version_new() {
	return (version_t){
		.major = version_number_new(),
		.minor = version_number_new(),
		.patch = version_number_new()
	};
}

version_t version_parse(const char *string, size_t length) {
	version_t version = version_new();

	unsigned short int parts_found = 0;
	for (size_t i = 0; i < length && string[i] != '\0'; i++) {
		if (string[i] == SEMVER_DOT) {
			parts_found++;
			continue;
		}

		if (string[i] == SEMVER_WILDCARD) {
			switch (parts_found) {
				case 0:
					version.major.type = VERSION_NUMBER_ANY;
					break;
				case 1:
					version.minor.type = VERSION_NUMBER_ANY;
					break;
				case 2:
					version.patch.type = VERSION_NUMBER_ANY;
					break;
				default:
					return version;
			}
		}

		if (string[i] < '0' || string[i] > '9')
			continue;

		size_t digit = (size_t)(string[i] - '0');
		switch (parts_found) {
			case 0:
				version.major.number = version.major.number * 10 + digit;
				version.major.type = VERSION_NUMBER_EXPLICIT;
				break;
			case 1:
				version.minor.number = version.minor.number * 10 + digit;
				version.minor.type = VERSION_NUMBER_EXPLICIT;
				break;
			case 2:
				version.patch.number = version.patch.number * 10 + digit;
				version.patch.type = VERSION_NUMBER_EXPLICIT;
				break;
			default:
				return version;
		
		}
	}

	if (parts_found < 3)
		version.patch.type = VERSION_NUMBER_NONE;
	if (parts_found < 2)
		version.minor.type = VERSION_NUMBER_NONE;
	if (parts_found < 1)
		version.major.type = VERSION_NUMBER_NONE;

	return version;
}

semver_t semver_new() {
	return (semver_t){
		.version = version_new(),
		.prerelease_metadata = NULL,
		.build_metadata = NULL
	};
}

semver_parse_status_t semver_parse(const char* string, size_t length, semver_t* semver) {
	// taken from is_valid_semver

	if (string == NULL || string[0] == '\0')
		return SEMVER_INVALID;

	bool found_symbol = false;
	size_t prerelease_start = 0;
	size_t build_start = 0;
	size_t i = 0;
	for (; i < length && string[i] != '\0'; i++) {
		if (string[i] == SEMVER_PRERELEASE_SYM) {
			// spec doesn't allow build then pre-release
			if (found_symbol)
				return SEMVER_INVALID;

			if (!semver_is_version_core(string, i))
				return SEMVER_INVALID;

			prerelease_start = i;
			found_symbol = true;
		} else if (string[i] == SEMVER_BUILD_SYM) {
			if (!found_symbol) {
				if (!semver_is_version_core(string, i))
					return SEMVER_INVALID;
			} else {
				if (string[prerelease_start] != SEMVER_PRERELEASE_SYM)
					return SEMVER_INVALID;

				if (prerelease_start + 1 >= i)
					return SEMVER_INVALID;

				if (!semver_is_pre_release((const char*)(string + prerelease_start + 1), i - prerelease_start - 1))
					return SEMVER_INVALID;
			}

			build_start = i;
			found_symbol = true;
		}
	}

	if (!found_symbol) {
		version_t version = version_parse(string, i);
		semver->version = version;
		return SEMVER_VALID;
	}

	version_t version = version_parse(string, prerelease_start);
	semver->version = version;

	if (found_symbol) {
		if (!semver_is_pre_release((const char*)(string + prerelease_start + 1), i - prerelease_start - 1)) {
			return SEMVER_INVALID;
		} else {
			size_t prerelease_end = (build_start > prerelease_start ? build_start : i) - prerelease_start;

			semver->prerelease_metadata = (char*)malloc(sizeof(char) * (prerelease_end + 1));
			if (semver->prerelease_metadata == NULL)
				return SEMVER_ERROR_MEMORY;

			for (size_t j = prerelease_start; j < prerelease_end; j++) {
				semver->prerelease_metadata[j - prerelease_start] = string[j];
			}
			semver->prerelease_metadata[prerelease_end - prerelease_start] = '\0';
		}
		if (build_start > prerelease_start) {
			if (!semver_is_build((const char*)(string + build_start + 1), i - build_start - 1))
				return SEMVER_INVALID;

			semver->build_metadata = (char*)malloc(sizeof(char) * (i - build_start + 1));
			if (semver->build_metadata == NULL)
				return SEMVER_ERROR_MEMORY;

			for (size_t j = build_start; j < i; j++) {
				semver->build_metadata[j - build_start] = string[j];
			}
			semver->build_metadata[i] = '\0';
		}
	}
	
	return SEMVER_VALID;
}

void semver_free(semver_t* semver) {
	semver->version = version_new();

	free(semver->prerelease_metadata);
	semver->prerelease_metadata = NULL;

	free(semver->build_metadata);
	semver->build_metadata = NULL;
}

// TODO:
void cute_version_evaluate_restriction(cute_version_t* version) {
	switch (version->restriction) {
		case VERSION_EQUALS:
			break;
		case VERSION_MIN_INC:
			break;
		case VERSION_MAX_INC:
			break;
		case VERSION_MIN:
			break;
		case VERSION_MAX:
			break;
		case VERSION_LOCK_MOST_SIGNIFICANT:
			break;
		case VERSION_ALLOW_LEAST_SIGNIFICANT:
			break;
		case VERSION_ANY:
			break;
	}
}

int size_t_cmp(size_t x, size_t y) {
	if (x == y)
		return 0;
	if (x > y)
		return 1;
	return -1;
}

// TODO:
int version_cmp(const version_t* x, const version_t* y);
int _version_cmp(const void* x, const void* y) {
	return version_cmp((const version_t*)x, (const version_t*)y);
}

// TODO:
int semver_cmp(const semver_t* x, const semver_t* y);
int _semver_cmp(const void* x, const void* y) {
	return semver_cmp((const semver_t*)x, (const semver_t*)y);
}

void sort_versions(version_t* versions, size_t count) {
	qsort(versions, count, sizeof(version_t), _version_cmp);
}

void sort_semvers(semver_t* semvers, size_t count) {
	qsort(semvers, count, sizeof(semver_t), _semver_cmp);
}

// TODO:
bool semver_satisfied(const semver_t* req, const version_t* installed);

