#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <copt.h>

#include "semver.h"

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
	if (string == NULL || length == 0)
		return false;

	for (size_t i = 0; i < length; i++) {
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
	for (size_t i = 0; i < length; i++) {
		if (!semver_is_identifier_character(string[i]))
			return false;
	}
	return true;
}

bool semver_is_numeric_identifier(const char* string, size_t length) {
	if (string == NULL || length == 0)
		return false;

	if (string[0] == '0')
		return length == 1;

	if (!semver_is_positive_digit(string[0]))
		return false;

	for (size_t i = 1; i < length; i++) {
		if (!semver_is_digit(string[i]))
			return false;
	}
	return true;
}

bool semver_is_alphanumeric_identifier(const char* string, size_t length) {
	if (string == NULL || length == 0)
		return false;

	bool found_non_digit = false;
	for (size_t i = 0; i < length; i++) {
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
	if (string == NULL || length == 0)
		return false;

	if (semver_is_build_identifier(string, length))
		return true;

	// catch leading SEMVER_DOT
	if (string[0] == SEMVER_DOT)
		return false;

	bool seen_dot = false;
	size_t prev_dot = 0;
	size_t i = 0;
	for (; i < length; i++) {
		if (string[i] == SEMVER_DOT) {
			if (seen_dot && prev_dot == i - 1)
				return false;
			
			if (
				!semver_is_build_identifier(
					seen_dot ? (const char*)(string + prev_dot + 1) : string,
					seen_dot ? i - prev_dot - 1 : i
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
		seen_dot ? i - prev_dot - 1 : i
	);
}

bool semver_is_build(const char* string, size_t length) {
	return semver_is_dot_separated_build_identifiers(string, length);
}

bool semver_is_dot_separated_pre_release_identifiers(const char* string, size_t length) {
	if (string == NULL || length == 0)
		return false;

	if (semver_is_pre_release_identifier(string, length))
		return true;

	// catch leading SEMVER_DOT
	if (string[0] == SEMVER_DOT)
		return false;

	bool seen_dot = false;
	size_t prev_dot = 0;
	size_t i = 0;
	for (; i < length; i++) {
		if (string[i] == SEMVER_DOT) {
			if (seen_dot && prev_dot == i - 1)
				return false;
			if (
				!semver_is_pre_release_identifier(
					seen_dot ? (const char*)(string + prev_dot + 1) : string,
					seen_dot ? i - prev_dot - 1 : i
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
		seen_dot ? i - prev_dot - 1 : i
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
	if (string == NULL || length == 0)
		return false;

	if (string[0] == SEMVER_DOT)
		return false;

	unsigned short int version_core_idx = 0;
	bool seen_dot = false;
	size_t prev_dot = 0;
	size_t i = 0;
	for (; i < length; i++) {
		if (string[i] == SEMVER_DOT) {
			if (seen_dot && prev_dot == i - 1)
				return false;

			switch (version_core_idx) {
				case 0:
					if (!semver_is_major(string, i))
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
	if (string == NULL || length == 0)
		return false;

	const char* build_metadata = memchr(
		string,
		SEMVER_BUILD_SYM,
		length
	);

	size_t main_length = build_metadata != NULL ? (size_t)(build_metadata - string) : length;

	if (build_metadata != NULL) {
		if (!semver_is_build(
			(const char*)(build_metadata + 1),
			length - main_length - 1
		))
			return false;
	}

	const char* prerelease_metadata = memchr(
		string,
		SEMVER_PRERELEASE_SYM,
		main_length
	);

	if (prerelease_metadata == NULL)
		return semver_is_version_core(string, main_length);

	size_t core_length = (size_t)(prerelease_metadata - string);
	size_t prerelease_length = main_length - core_length - 1;

	return semver_is_version_core(string, core_length)
		&& semver_is_pre_release((const char*)(prerelease_metadata + 1), prerelease_length);
}

// ======================================================================

semver_version_core_t semver_version_core_new() {
	return (semver_version_core_t){
		.major = 0,
		.minor = 0,
		.patch = 0
	};
}

semver_version_core_t semver_version_core_parse(const char* string, size_t length) {
	semver_version_core_t version = semver_version_core_new();

	unsigned short int parts_found = 0;
	for (size_t i = 0; i < length; i++) {
		if (string[i] == SEMVER_DOT) {
			parts_found++;
			continue;
		}

		if (string[i] < '0' || string[i] > '9')
			continue;

		size_t digit = (size_t)(string[i] - '0');
		switch (parts_found) {
			case 0:
				version.major = version.major * 10 + digit;
				break;
			case 1:
				version.minor = version.minor * 10 + digit;
				break;
			case 2:
				version.patch = version.patch * 10 + digit;
				break;
			default:
				return version;
		
		}
	}

	return version;
}

semver_t semver_new() {
	return (semver_t){
		.version = semver_version_core_new(),
		.prerelease_metadata = NULL,
		.build_metadata = NULL
	};
}

semver_t semver_from_semver_version_core(semver_version_core_t version) {
	semver_t semver = semver_new();
	semver.version = version;
	return semver;
}

semver_parse_status_t semver_parse(const char* string, size_t length, semver_t* semver) {
	// taken from is_valid_semver

	if (semver == NULL || string == NULL)
		return SEMVER_INVALID;

	if (!semver_is_valid_semver(string, length))
		return SEMVER_INVALID;

	semver_t parsed = semver_new();
	
	const char* build_metadata = memchr(
		string,
		SEMVER_BUILD_SYM,
		length
	);

	size_t main_length = build_metadata != NULL ? (size_t)(build_metadata - string) : length;

	const char* prerelease_metadata = memchr(
		string,
		SEMVER_PRERELEASE_SYM,
		main_length
	);

	size_t core_length = prerelease_metadata != NULL ? (size_t)(prerelease_metadata - string) : main_length;

	parsed.version = semver_version_core_parse(string, core_length);

	if (prerelease_metadata != NULL) {
		size_t prerelease_length = main_length - core_length - 1;

		parsed.prerelease_metadata = malloc(sizeof(char) * (prerelease_length + 1));
		if (parsed.prerelease_metadata == NULL) {
			semver_free(&parsed);
			return SEMVER_ERROR_MEMORY;
		}

		memcpy(
			parsed.prerelease_metadata,
			(const void*)(prerelease_metadata + 1),
			prerelease_length
		);

		parsed.prerelease_metadata[prerelease_length] = '\0';
	}

	if (build_metadata != NULL) {
		size_t build_length = length - (size_t)(build_metadata - string) - 1;

		parsed.build_metadata = malloc(sizeof(char) * (build_length + 1));
		if (parsed.build_metadata == NULL) {
			semver_free(&parsed);
			return SEMVER_ERROR_MEMORY;
		}

		memcpy(
			parsed.build_metadata,
			(const void*)(build_metadata + 1),
			build_length
		);

		parsed.build_metadata[build_length] = '\0';
	}

	semver_free(semver);

	*semver = parsed;

	return SEMVER_VALID;
}

void semver_free(semver_t* semver) {
	if (semver == NULL)
		return;

	free(semver->prerelease_metadata);
	free(semver->build_metadata);

	*semver = semver_new();
}

cute_version_number_t cute_version_number_new() {
	return (cute_version_number_t){
		.type = VERSION_NUMBER_NONE,
		.number = 0
	};
}

cute_version_core_t cute_version_core_new() {
	return (cute_version_core_t){
		.major = cute_version_number_new(),
		.minor = cute_version_number_new(),
		.patch = cute_version_number_new()
	};
}

cute_version_core_t cute_version_core_parse(const char* string, size_t length) {
	cute_version_core_t version = cute_version_core_new();

	unsigned short int dots_found = 0;
	for (size_t i = 0; i < length; i++) {
		if (string[i] == SEMVER_DOT) {
			dots_found++;
			continue;
		}

		if (string[i] < '0' || string[i] > '9')
			continue;

		size_t digit = (size_t)(string[i] - '0');
		switch (dots_found) {
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
			// TODO: return status and throw invalid
			default:
				return version;
		
		}
	}

	if (dots_found < 2)
		version.patch.type = VERSION_NUMBER_NONE;
	if (dots_found < 1)
		version.minor.type = VERSION_NUMBER_NONE;

	return version;
}

cute_version_t cute_version_new() {
	return (cute_version_t){
		.version = cute_version_core_new(),

		.restriction = CUTE_VERSION_ANY,
		.min = option_none(),
		.min_inclusive = true,
		.max = option_none(),
		.max_inclusive = true,

		.prerelease_metadata = NULL,
		.build_metadata = NULL
	};
}

bool cute_version_str_starts_with(const char* string, const char* substr) {
	size_t i = 0;
	for (; substr[i] != '\0'; i++) {
		if (string[i] == '\0'
				|| substr[i] != string[i])
			return false;
	}
	return true;
}

cute_version_parse_status_t cute_version_parse(const char *string, size_t length, cute_version_t *version) {
	// taken from semver_parse

	if (version == NULL || string == NULL)
		return CUTE_VERSION_INVALID;

	cute_version_t parsed = cute_version_new();

	// first we need to find the restriction if there is one
	// the end of the restriction operator is the beginning for the
	// rest of the version string
	const char* restriction_operator;
	if (cute_version_str_starts_with(
		string,
		CUTE_VERSION_EQUALS_SYM
	)) {
		restriction_operator = CUTE_VERSION_EQUALS_SYM;
		parsed.restriction = CUTE_VERSION_EQUALS;
	} else if (cute_version_str_starts_with(
		string,
		CUTE_VERSION_MIN_INC_SYM
	)) {
		restriction_operator = CUTE_VERSION_MIN_INC_SYM;
		parsed.restriction = CUTE_VERSION_MIN_INC;
	} else if (cute_version_str_starts_with(
		string,
		CUTE_VERSION_MAX_INC_SYM
	)) {
		restriction_operator = CUTE_VERSION_MAX_INC_SYM;
		parsed.restriction = CUTE_VERSION_MAX_INC;
	} else if (cute_version_str_starts_with(
		string,
		CUTE_VERSION_MIN_SYM
	)) {
		restriction_operator = CUTE_VERSION_MIN_SYM;
		parsed.restriction = CUTE_VERSION_MIN;
	} else if (cute_version_str_starts_with(
		string,
		CUTE_VERSION_MAX_SYM
	)) {
		restriction_operator = CUTE_VERSION_MAX_SYM;
		parsed.restriction = CUTE_VERSION_MAX;
	} else if (cute_version_str_starts_with(
		string,
		CUTE_VERSION_LOCK_MOST_SIGNIFICANT_SYM
	)) {
		restriction_operator = CUTE_VERSION_LOCK_MOST_SIGNIFICANT_SYM;
		parsed.restriction = CUTE_VERSION_LOCK_MOST_SIGNIFICANT;
	} else if (cute_version_str_starts_with(
		string,
		CUTE_VERSION_ALLOW_LEAST_SIGNIFICANT_SYM
	)) {
		restriction_operator = CUTE_VERSION_ALLOW_LEAST_SIGNIFICANT_SYM;
		parsed.restriction = CUTE_VERSION_ALLOW_LEAST_SIGNIFICANT;
	} else if (cute_version_str_starts_with(
		string,
		CUTE_VERSION_ANY_SYM
	)) {
		restriction_operator = CUTE_VERSION_ANY_SYM;
		parsed.restriction = CUTE_VERSION_ANY;
	} else {
		restriction_operator = NULL;
	}

	if (restriction_operator != NULL) {
		size_t op_length = strlen(restriction_operator);
		string = (const char*)(string + op_length);
		length -= op_length;
	}

	const char* build_metadata = memchr(
		string,
		SEMVER_BUILD_SYM,
		length
	);

	size_t main_length = build_metadata != NULL ? (size_t)(build_metadata - string) : length;

	const char* prerelease_metadata = memchr(
		string,
		SEMVER_PRERELEASE_SYM,
		main_length
	);

	size_t core_length = prerelease_metadata != NULL ? (size_t)(prerelease_metadata - string) : main_length;

	parsed.version = cute_version_core_parse(string, core_length);

	if (prerelease_metadata != NULL) {
		size_t prerelease_length = main_length - core_length - 1;

		parsed.prerelease_metadata = malloc(sizeof(char) * (prerelease_length + 1));
		if (parsed.prerelease_metadata == NULL) {
			cute_version_free(&parsed);
			return CUTE_VERSION_ERROR_MEMORY;
		}

		memcpy(
			parsed.prerelease_metadata,
			(const void*)(prerelease_metadata + 1),
			prerelease_length
		);

		parsed.prerelease_metadata[prerelease_length] = '\0';
	}

	if (build_metadata != NULL) {
		size_t build_length = length - (size_t)(build_metadata - string) - 1;

		parsed.build_metadata = malloc(sizeof(char) * (build_length + 1));
		if (parsed.build_metadata == NULL) {
			cute_version_free(&parsed);
			return CUTE_VERSION_ERROR_MEMORY;
		}

		memcpy(
			parsed.build_metadata,
			(const void*)(build_metadata + 1),
			build_length
		);

		parsed.build_metadata[build_length] = '\0';
	}

	cute_version_evaluate_restriction(&parsed);

	cute_version_free(version);

	*version = parsed;

	return CUTE_VERSION_VALID;
}

void cute_version_free(cute_version_t* version) {
	if (version == NULL)
		return;

	free(version->prerelease_metadata);
	free(version->build_metadata);

	free(option_unwrap_or(version->max, NULL));
	free(option_unwrap_or(version->min, NULL));

	*version = cute_version_new();
}

semver_version_core_t cute_version_core_to_semver_version_core(const cute_version_core_t* version) {
	semver_version_core_t semver = semver_version_core_new();

	if (version->major.type == VERSION_NUMBER_EXPLICIT) {
		semver.major = version->major.number;
	} else {
		semver.major = 0;
	}

	if (version->minor.type == VERSION_NUMBER_EXPLICIT) {
		semver.minor = version->minor.number;
	} else {
		semver.minor = 0;
	}

	if (version->patch.type == VERSION_NUMBER_EXPLICIT) {
		semver.patch = version->patch.number;
	} else {
		semver.patch = 0;
	}

	return semver;
}

semver_version_core_t* cute_version_caret_upper_bound(const cute_version_t* version) {
	semver_version_core_t* max = (semver_version_core_t*)malloc(sizeof(semver_version_core_t));
	if (max == NULL)
		return NULL;

	*max = semver_version_core_new();

	unsigned short int most_significant = 0;
	if (version->version.major.type != VERSION_NUMBER_EXPLICIT
			|| version->version.major.number == 0)
		most_significant++;
	if (version->version.minor.type != VERSION_NUMBER_EXPLICIT
			|| version->version.minor.number == 0)
		most_significant++;

	switch (most_significant) {
		case 0:
			max->major = version->version.major.number + 1;
			break;
		case 1:
			max->minor = version->version.minor.number + 1;
			break;
		default:
			max->patch = version->version.patch.number + 1;
			break;
	}

	return max;
}

semver_version_core_t* cute_version_tilde_upper_bound(const cute_version_t* version) {
	semver_version_core_t* max = (semver_version_core_t*)malloc(sizeof(semver_version_core_t));
	if (max == NULL)
		return NULL;

	*max = semver_version_core_new();

	if (version->version.major.type == VERSION_NUMBER_EXPLICIT
			&& version->version.minor.type != VERSION_NUMBER_EXPLICIT
			&& version->version.patch.type != VERSION_NUMBER_EXPLICIT) {
		max->major = version->version.major.number + 1;
	} else {
		if (version->version.major.type == VERSION_NUMBER_EXPLICIT)
			max->major = version->version.major.number;

		if (version->version.minor.type == VERSION_NUMBER_EXPLICIT) {
			max->minor = version->version.minor.number + 1;
		} else {
			max->minor = 1;
		}
	}

	return max;
}

void cute_version_evaluate_restriction(cute_version_t* version) {
	semver_version_core_t* current_version = (semver_version_core_t*)malloc(sizeof(semver_version_core_t));
	if (current_version == NULL)
		return;

	*current_version = cute_version_core_to_semver_version_core(&version->version);

	semver_version_core_t* upper_bound;
	switch (version->restriction) {
		case CUTE_VERSION_EQUALS:
			version->max_inclusive = true;
			version->min_inclusive = true;
			version->max = option_none();
			version->min = option_none();
			free(current_version);
			break;
		case CUTE_VERSION_MIN_INC:
			version->max_inclusive = false;
			version->min_inclusive = true;
			version->max = option_none();
			version->min = option_some(current_version);
			break;
		case CUTE_VERSION_MAX_INC:
			version->max_inclusive = true;
			version->min_inclusive = false;
			version->max = option_some(current_version);
			version->min = option_none();
			break;
		case CUTE_VERSION_MIN:
			version->max_inclusive = false;
			version->min_inclusive = false;
			version->max = option_none();
			version->min = option_some(current_version);
			break;
		case CUTE_VERSION_MAX:
			version->max_inclusive = false;
			version->min_inclusive = false;
			version->max = option_some(current_version);
			version->min = option_none();
			break;
		case CUTE_VERSION_LOCK_MOST_SIGNIFICANT:
			version->max_inclusive = false;
			version->min_inclusive = true;
			upper_bound = cute_version_caret_upper_bound(version);
			if (upper_bound == NULL)
				return;
			version->max = option_some(upper_bound);
			version->min = option_some(current_version);
			break;
		case CUTE_VERSION_ALLOW_LEAST_SIGNIFICANT:
			version->max_inclusive = false;
			version->min_inclusive = true;
			upper_bound = cute_version_tilde_upper_bound(version);
			if (upper_bound == NULL)
				return;
			version->max = option_some(upper_bound);
			version->min = option_some(current_version);
			break;
		case CUTE_VERSION_ANY:
			version->max_inclusive = true;
			version->min_inclusive = true;
			version->max = option_none();
			version->min = option_none();
			free(current_version);
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

int semver_version_core_cmp(const semver_version_core_t* x, const semver_version_core_t* y) {
	if (x->major != y->major)
		return size_t_cmp(x->major, y->major);

	if (x->minor != y->minor)
		return size_t_cmp(x->minor, y->minor);

	return size_t_cmp(x->patch, y->patch);
}
int _semver_version_core_cmp(const void* x, const void* y) {
	return semver_version_core_cmp((const semver_version_core_t*)x, (const semver_version_core_t*)y);
}

size_t size_t_min(size_t x, size_t y) {
	if (x < y)
		return x;
	return y;
}

int char_cmp(const char x, const char y) {
	if (x == y)
		return 0;
	if (x > y)
		return 1;
	return -1;
}

int semver_pre_release_cmp(const char* x, const char* y) {
	size_t x_dot_idx = 0;
	size_t y_dot_idx = 0;

	while (true) {
		const char* x_dot_loc = strchr((const char*)(x + x_dot_idx), SEMVER_DOT);
		const char* y_dot_loc = strchr((const char*)(y + y_dot_idx), SEMVER_DOT);

		const char* x_identifier_end = (const char*)(
			x_dot_loc != NULL
			? x_dot_loc
			: x + strlen(x)
		);
		const char* y_identifier_end = (const char*)(
			y_dot_loc != NULL
			? y_dot_loc
			: y + strlen(y)
		);

		size_t x_identifier_start = x_dot_idx;
		size_t y_identifier_start = y_dot_idx;

		size_t x_identifier_length = (size_t)(x_identifier_end - (x + x_dot_idx));
		size_t y_identifier_length = (size_t)(y_identifier_end - (y + y_dot_idx));

		bool x_identifier_numeric = semver_is_numeric_identifier((const char*)(x + x_identifier_start), x_identifier_length);
		bool y_identifier_numeric = semver_is_numeric_identifier((const char*)(y + y_identifier_start), y_identifier_length);

		if (x_identifier_numeric && y_identifier_numeric) {
			if (x_identifier_length < y_identifier_length)
				return -1;
			else if (x_identifier_length > y_identifier_length)
				return 1;

			for (size_t i = 0; i < x_identifier_length; i++) {
				int char_diff = char_cmp(
					*(x + x_identifier_start + i),
					*(y + y_identifier_start + i)
				);
				if (char_diff != 0)
					return char_diff;
			}
		} else if (x_identifier_numeric) {
			return -1;
		} else if (y_identifier_numeric) {
			return 1;
		} else {
			size_t min_size = size_t_min(x_identifier_length, y_identifier_length);
			for (size_t i = 0; i < min_size; i++) {
				int char_diff = char_cmp(
					*(x + x_identifier_start + i),
					*(y + y_identifier_start + i)
				);
				if (char_diff != 0)
					return char_diff;
			}
			if (x_identifier_length < y_identifier_length)
				return -1;
			else if (x_identifier_length > y_identifier_length)
				return 1;
		}

		if (x_dot_loc == NULL && y_dot_loc == NULL)
			return 0;
		if (x_dot_loc == NULL)
			return -1;
		if (y_dot_loc == NULL)
			return 1;

		x_dot_idx += x_identifier_length + 1;
		y_dot_idx += y_identifier_length + 1;
	}

	return 0;
}

int semver_cmp(const semver_t* x, const semver_t* y) {
	int version_diff = semver_version_core_cmp(&x->version, &y->version);
	if (version_diff != 0)
		return version_diff;

	if (x->prerelease_metadata == NULL && y->prerelease_metadata == NULL)
		return 0;
	else if (x->prerelease_metadata == NULL)
		return -1;
	else if (y->prerelease_metadata == NULL)
		return 1;

	return semver_pre_release_cmp((const char*)x->prerelease_metadata, (const char*)y->prerelease_metadata);
}
int _semver_cmp(const void* x, const void* y) {
	return semver_cmp((const semver_t*)x, (const semver_t*)y);
}
void sort_semvers(semver_t* semvers, size_t count) {
	qsort(semvers, count, sizeof(semver_t), _semver_cmp);
}

bool cute_version_satisfied(const cute_version_t* requirement, const semver_t* version) {
	if (requirement->restriction == CUTE_VERSION_EQUALS) {
		semver_t required_version = semver_from_semver_version_core(
			cute_version_core_to_semver_version_core(
				&requirement->version
			)
		);
		required_version.prerelease_metadata = required_version.prerelease_metadata;
		required_version.build_metadata = required_version.build_metadata;
		return semver_cmp(
			&required_version,
			version
		) == 0;
	}

	if (option_is_some(requirement->min) && option_is_some(requirement->max)) {
		int min_cmp = semver_cmp(version, option_unwrap(requirement->min));
		int max_cmp = semver_cmp(version, option_unwrap(requirement->max));

		return (min_cmp == 1 || (requirement->min_inclusive && min_cmp == 0))
			&& (max_cmp == -1 || (requirement->max_inclusive && max_cmp == 0));
	} else if (option_is_some(requirement->min)) {
		int min_cmp = semver_cmp(version, option_unwrap(requirement->min));

		return min_cmp == 1 || (requirement->min_inclusive && min_cmp == 0);
	} else if (option_is_some(requirement->max)) {
		int max_cmp = semver_cmp(version, option_unwrap(requirement->max));

		return max_cmp == -1 || (requirement->max_inclusive && max_cmp == 0);
	}

	return true;
}

