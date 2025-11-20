# Changelog

All notable changes to this project will be documented in this file.

## [1.5.0] - 2025-11-20

### Added
- Added `GetValuesFromYamlPathWithFilters` and `GetValueFromYamlPathWithFilters` to `YamlBlueprintLibrary`.
- Supports extracting values conditionally based on sibling node values.
- Example: Extract `output.content.text` only if `output.content.type` equals `output_text`.

## [1.4.0] - 2025-11-20

### Added
- Added `GetValueFromYamlPath` to `YamlBlueprintLibrary`.
- Extracts a SINGLE value from a YAML/JSON string using a dot-separated path.
- Returns the first value found if multiple exist (useful for extracting specific fields like "output.content.text").

## [1.3.0] - 2025-11-20

### Added
- Added `GetValuesFromYamlPath` to `YamlBlueprintLibrary`.
- Allows extracting values using dot-separated paths (e.g., "output.content.text") from YAML/JSON strings.
- Supports recursive traversal through sequences and maps.

## [1.2.0] - 2025-11-20

### Added
- Added `GetValuesFromYamlSequence` and `GetValuesFromYamlString` to `YamlBlueprintLibrary`.

- Allows extracting a list of values for a specific key from a YAML sequence (file or string).
- Supports complex types (maps/sequences) by converting them to JSON strings.

## [1.1.0] - 2025-11-20

### Added
- Added `UYamlTestLibrary` for validating specific YAML schemas.
- Added `TestPromptsYaml` to validate `Tests.Prompts.yaml`.
- Added `TestResponsesYaml` to validate `Tests.Responses.yaml`.

## [1.0.0] - 2025-11-20

### Added
- Initial release of YamlPlugin.
- Integrated `yaml-cpp` third-party library for YAML parsing.
- Implemented `UYamlBlueprintLibrary` to expose YAML functionality to Blueprints.
- Added `LoadYamlFromFile` function to read YAML files and convert them to JSON strings.
