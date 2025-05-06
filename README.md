# Qt Messenger Application

A modern messaging application built with Qt/C++ featuring a client-server architecture.

## Features

- User authentication (registration and login)
- Real-time messaging between users
- Contact management
- File sharing
- Modern WhatsApp-inspired UI
- Light and dark theme support

## Project Structure

- **client/** - Contains the client-side application code
- **server/** - Contains the server-side application code
- **resources/** - Contains application resources (icons, stylesheets)

## Building and Running

### Prerequisites

- Qt 5.15+ or Qt 6+
- C++17 compatible compiler
- CMake 3.14+

### Build Steps

1. Clone the repository
2. Configure the project with CMake:
   ```
   mkdir build
   cd build
   cmake ..
   ```
3. Build the project:
   ```
   cmake --build .
   ```

### Running the Application

1. Start the server first:
   ```
   ./server/QtMessengerServer
   ```

2. Then launch the client:
   ```
   ./client/QtMessengerClient
   ```

## Architecture

This application uses a client-server architecture:

- The **server** handles user authentication, message routing, and database operations
- The **client** provides the user interface and communicates with the server via TCP sockets

## License

MIT License