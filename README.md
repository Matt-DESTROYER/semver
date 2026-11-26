# semver

A lightweight C library for parsing, validating, comparing, and evaluating Semantic Versioning (SemVer) versions.

`semver` is developed as part of the [Cute package manager](https://github.com/Matt-DESTROYER/cute), providing the version-handling functionality used throughout the project.

The library is written in C11 and implements [Semantic Versioning 2.0.0](https://semver.org/spec/v2.0.0.html), alongside a version requirement system inspired by Cargo's dependency version syntax.

## Features

### Semantic versioning

- Validate version strings against the SemVer 2.0.0 specification.
- Parse version strings into structured representations.
- Support pre-release identifiers and build metadata.
- Compare versions according to SemVer precedence rules.
- Sort arrays of semantic versions.

### Version requirements

- Parse version requirement expressions.
- Support exact, minimum, maximum, caret, and tilde restrictions.
- Support partially specified version numbers.
- Evaluate whether a semantic version satisfies a requirement.

## Usage

### Semantic versions

Include `semver.h` to access the semantic versioning API.

```c
#include <stdio.h>
#include <string.h>

#include "semver.h"

int main(void) {
    const char* string = "1.2.3-alpha.1+build.42";

    semver_t version = semver_new();

    semver_parse_status_t status = semver_parse(
        string,
        strlen(string),
        &version
    );

    if (status != SEMVER_VALID) {
        fprintf(stderr, "Invalid version string\n");
        return 1;
    }

    printf(
        "Version: %zu.%zu.%zu\n",
        version.version.major,
        version.version.minor,
        version.version.patch
    );

    printf("Pre-release: %s\n", version.prerelease_metadata);
    printf("Build: %s\n", version.build_metadata);

    semver_free(&version);

    return 0;
}
```

`semver_parse` accepts a string and its length, allowing it to operate on non-null-terminated input.

### Validation

Use `semver_is_valid_semver` to validate a version without constructing a `semver_t`.

```c
#include <stdio.h>
#include <string.h>

#include "semver.h"

int main(void) {
    const char* version = "1.2.3-alpha.1";

    if (semver_is_valid_semver(version, strlen(version))) {
        printf("Valid SemVer version\n");
    } else {
        printf("Invalid SemVer version\n");
    }

    return 0;
}
```

The validation functions also expose individual checks for version components and identifiers.

### Comparing versions

Use `semver_cmp` to compare two parsed versions.

```c
#include <stdio.h>
#include <string.h>

#include "semver.h"

int main(void) {
    semver_t first = semver_new();
    semver_t second = semver_new();

    semver_parse("1.2.3", 5, &first);
    semver_parse("1.3.0", 5, &second);

    int result = semver_cmp(&first, &second);

    if (result < 0) {
        printf("First version is older\n");
    } else if (result > 0) {
        printf("First version is newer\n");
    } else {
        printf("Versions have equal precedence\n");
    }

    semver_free(&first);
    semver_free(&second);

    return 0;
}
```

The comparison function follows SemVer precedence:

- Major, minor, and patch components are compared numerically.
- Pre-release versions have lower precedence than their corresponding stable versions.
- Pre-release identifiers are compared according to SemVer's identifier comparison rules.
- Build metadata does not affect version precedence.

`semver_cmp` returns:
- A negative value if the first version has lower precedence.
- Zero if both versions have equal precedence.
- A positive value if the first version has higher precedence.

### Sorting versions

Use `sort_semvers` to sort an array of parsed versions in ascending precedence order.

```c
#include <string.h>

#include "semver.h"

int main(void) {
    semver_t versions[] = {
        semver_new(),
        semver_new(),
        semver_new()
    };

    semver_parse("2.0.0", 5, &versions[0]);
    semver_parse("1.0.0", 5, &versions[1]);
    semver_parse("1.5.0", 5, &versions[2]);

    sort_semvers(versions, 3);

    for (size_t i = 0; i < 3; i++) {
        /* Versions are now sorted. */
    }

    for (size_t i = 0; i < 3; i++) {
        semver_free(&versions[i]);
    }

    return 0;
}
```

## Version requirements

The library also provides a version requirement interface through `cute-version.h`.

Version requirements allow a package to specify which versions of a dependency are acceptable.

### Supported restrictions

| Operator | Restriction | Example |
|---|---|---|
| `=` | Exact version | `=1.2.3` |
| `>` | Greater than | `>1.2.3` |
| `>=` | Greater than or equal | `>=1.2.3` |
| `<` | Less than | `<2.0.0` |
| `<=` | Less than or equal | `<=2.0.0` |
| `^` | Caret requirement | `^1.2.3` |
| `~` | Tilde requirement | `~1.2.3` |

When no operator is specified, caret behaviour is used by default.

### Partial versions

Version requirements support partially specified version numbers.

For example:

- `^1`
- `^1.2`
- `^1.2.3`
- `~1`
- `~1.2`
- `~1.2.3`

Unspecified components are treated as zero when evaluating the lower bound.

### Caret requirements

Caret requirements allow updates that do not change the leftmost non-zero version component.

Examples:

| Requirement | Accepted version range |
|---|---|
| `^1.2.3` | `>=1.2.3, <2.0.0` |
| `^0.2.3` | `>=0.2.3, <0.3.0` |
| `^0.0.3` | `>=0.0.3, <0.0.4` |
| `^1.2` | `>=1.2.0, <2.0.0` |
| `^0` | `>=0.0.0, <0.1.0` |

### Tilde requirements

Tilde requirements allow updates to the least significant explicitly specified version component while keeping the more significant components fixed.

Examples:

| Requirement | Accepted version range |
|---|---|
| `~1.2.3` | `>=1.2.3, <1.3.0` |
| `~1.2` | `>=1.2.0, <1.3.0` |
| `~1` | `>=1.0.0, <2.0.0` |

### Evaluating requirements

Use `cute_version_parse` to parse a requirement and `cute_version_satisfied` to check a semantic version against it.

```c
#include <stdio.h>
#include <string.h>

#include "semver.h"
#include "cute-version.h"

int main(void) {
    cute_version_t requirement = cute_version_new();
    semver_t version = semver_new();

    const char* requirement_string = "^1.2.3";
    const char* version_string = "1.5.0";

    cute_version_parse(
        requirement_string,
        strlen(requirement_string),
        &requirement
    );

    semver_parse(
        version_string,
        strlen(version_string),
        &version
    );

    if (cute_version_satisfied(&requirement, &version)) {
        printf("Version satisfies requirement\n");
    } else {
        printf("Version does not satisfy requirement\n");
    }

    cute_version_free(&requirement);
    semver_free(&version);

    return 0;
}
```

## Public API

The library exposes two primary interfaces.

### `semver.h`

| Function | Description |
|---|---|
| `semver_new()` | Creates an empty semantic version. |
| `semver_version_core_new()` | Creates an empty version core. |
| `semver_version_core_parse()` | Parses a version core. |
| `semver_parse()` | Parses a complete semantic version. |
| `semver_free()` | Releases allocated version metadata. |
| `semver_is_valid_semver()` | Validates a complete SemVer string. |
| `semver_version_core_cmp()` | Compares two version cores. |
| `semver_cmp()` | Compares two semantic versions. |
| `sort_semvers()` | Sorts an array of semantic versions. |

### `cute-version.h`

| Function | Description |
|---|---|
| `cute_version_new()` | Creates an empty version requirement. |
| `cute_version_parse()` | Parses a version requirement. |
| `cute_version_evaluate_restriction()` | Calculates the bounds of a requirement. |
| `cute_version_satisfied()` | Checks whether a version satisfies a requirement. |
| `cute_version_free()` | Releases allocated requirement metadata. |

Both interfaces provide parse status enums:

**SemVer:**
- `SEMVER_VALID`
- `SEMVER_INVALID`
- `SEMVER_ERROR_MEMORY`

**Version requirements:**
- `CUTE_VERSION_VALID`
- `CUTE_VERSION_INVALID`
- `CUTE_VERSION_ERROR_MEMORY`

## Memory management

Both `semver_t` and `cute_version_t` contain dynamically allocated metadata.

Always initialise these structures using their respective constructor functions before passing them to their parsing functions.

```c
semver_t version = semver_new();
cute_version_t requirement = cute_version_new();
```

When finished with them, release their resources using:

```c
semver_free(&version);
cute_version_free(&requirement);
```

These functions release allocated metadata and reset the structures to their empty states.

Parsing into an existing initialised structure is supported. Successful parsing replaces its previous contents, while unsuccessful parsing preserves the existing value.

## Building

The project is made to be built with Cute.
Simply run `cute build` to verify it builds successfully.

## Testing

The test suite covers:

- SemVer identifier validation.
- Valid and invalid version strings.
- Parsing of core versions, pre-release identifiers, and build metadata.
- Version precedence and comparison.
- Sorting semantic versions.
- Version requirement parsing and evaluation.

The tests are organised under `tests/` and are intended to be ran with `cute test`.

## Specification

The semantic version implementation is based on the official [Semantic Versioning 2.0.0 specification](https://semver.org/spec/v2.0.0.html).

The version requirement syntax is inspired by Cargo's version requirement conventions.

## License

See [LICENSE](LICENSE) for licensing information.
