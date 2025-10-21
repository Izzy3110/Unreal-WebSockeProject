# WebSockeProject

## Overview
This is an Unreal Engine 5.6 project focused on integrating various networking and data management technologies, with a strong emphasis on WebSocket communication. It serves as a foundation for interactive applications requiring real-time data exchange and robust backend connectivity.

## Features
- **Real-time Communication:** Utilizes WebSockets for efficient, real-time data transfer.
- **Database Integration:** Includes support for PostgreSQL and MySQL databases.
- **MQTT Connectivity:** Integrates MQTT for IoT and messaging protocols.
- **RESTful API Interaction:** Leverages VaRest for seamless communication with RESTful services.
- **JSON Web Tokens (JWT):** Implements JWT for secure authentication and authorization.
- **Serialization Utilities:** Provides tools for data serialization and deserialization.
- **Blueprint File Utilities:** Enhances Blueprint capabilities with file system operations.
- **Grid Inventory System:** Features a modular grid-based inventory system.

## Plugins Used
The project makes extensive use of the following Unreal Engine plugins:
- `Postgres`
- `MQTTPlugin`
- `WebSocketsHelper`
- `GameplayStateTree`
- `JsonBlueprintUtilities`
- `VaRest`
- `JWT`
- `GridInventory`

## Getting Started

### Prerequisites
- Unreal Engine 5.6 installed.
- Visual Studio (or other C++ IDE) for C++ development.

### Opening the Project
1.  Navigate to the project directory.
2.  Right-click on `WebSockeProject.uproject` and select "Generate Visual Studio project files" (if you plan to work with C++).
3.  Double-click `WebSockeProject.uproject` to open the project in Unreal Engine.

### Prepare and fillup ThirdParty Folders

## Project Structure
- `Content/`: Contains all game assets, including Blueprints, maps, materials, and character assets.
- `Source/`: Contains all C++ source code for the project.
- `Plugins/`: Houses all third-party and custom plugins used in the project.

## How to Build
The project can be built directly from the Unreal Editor or by compiling the Visual Studio solution (`WebSockeProject.sln`) after generating project files.