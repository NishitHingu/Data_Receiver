## This project provides a C++ client application designed to receive data from a server.  

### Prerequisites
- Node.js (version >= 16.17.0): Download and install the latest version from the official [website](https://nodejs.org/en)
- C++ Compiler with C++17 Support: Make sure your C++ compiler supports the C++17 standard. Popular options include GCC (>= 7.1) or Clang (>= 3.5).

### Getting Started
This project includes a convenient Run script that simplifies starting both the Node.js server and the C++ client in a single step.

- Clone the Repository: Use Git to clone this repository to your local machine.
- Run the Server and Client: Open a terminal in the project directory and execute the following command:  
  ```./Run.sh```
  This script will start the Node.js server in the background and then build and run the C++ client.

- Verify Execution: The script will output logs from both the server and client to server_logs and client_logs files respectively. You can review these logs to confirm successful connection and data transfer.
