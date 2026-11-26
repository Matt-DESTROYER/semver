#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <copt.h>

#include "semver.h"
#include "cute-version.h"

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
