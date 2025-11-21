# Changelog

All notable changes to this project will be documented in this file.

## [1.1.0] - 2025-11-20
### Added
- Async request node (`ULMStudioRequest`) for Blueprints.
- JSON Helper node (`CreateLMStudioRequestJSON`) to simplify request creation.
- Support for sending JSON content to LM Studio server.
- `OnSuccess` and `OnFailure` delegates for handling responses.
- `ExtractOutputContentText` helper node to parse response text.

## [1.2.0] - 2025-11-21
### Changed
- `SendLMStudioRequest` now accepts `LastResponses` (Array of Strings) to inject conversation history.
- `SendLMStudioRequest` and `SendLMStudioRequestSimple` now automatically append `/chat/completions` to the URL if missing.
- Default `ServerBaseURL` updated to `http://localhost:3000/v1`.
- `SendLMStudioRequestSimple` now uses Project Settings defaults for `ServerURL`, `APIKey`, and `ModelName` if inputs are empty.
- Fixed `LastResponses` injection order (now inserted before user message).
- Fixed ANSI string deprecation warnings in `LMStudioRequest.cpp` and `LMStudioUtils.cpp`.
- **Fix (400 Error)**: Added automatic injection of `input` field (using user prompt) into JSON request if missing, as some servers/endpoints require it.
- **Fix (URL)**: Updated URL validation to accept `/responses` suffix without appending `/chat/completions`.
- **Fix (Cleanup)**: Added `SetReadyToDestroy()` to ensure proper cleanup of async nodes.

### Added
- `SendLMStudioRequestSimple` async node for simplified requests (Prompt + Model Name + History).
- Logging of Request URL and JSON content to `LogTemp` for debugging.

## [1.3.0] - 2025-11-21
### Added
- Request logging to file: Appends execution statistics (Time, Model, Status, Duration, URL, Prompt, Response) to `AppData/Roaming/WebSockeProject/Logs/LMStudio/requests-<timestamp>.log`.
