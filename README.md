*This project has been created as part of the 42 curriculum by gafreire, alejagom, sreffers.*

# Webserv - 42 Project

## Description

**Webserv** is a custom HTTP/1.1 web server written in C++98, designed as a comprehensive system architecture project for the 42 curriculum. The primary goal of this project is to understand the inner workings of HTTP protocols, network sockets, and concurrent connections by recreating a functional, non-blocking web server from scratch.

Unlike modern high-level frameworks that abstract away the complexity of network communication, this project requires handling everything at the system socket level. It involves parsing raw HTTP requests, managing memory efficiently under C++98 constraints, properly mapping URIs to the file system, executing dynamic scripts (CGI), generating valid HTTP responses, and ensuring the server remains highly resilient under heavy loads or malicious requests. It is a deep dive into the fundamentals of internet infrastructure, closely mirroring the core behaviors of established servers like Nginx.

### Overview
The server leverages I/O multiplexing with `poll()` to handle multiple concurrent client connections efficiently without blocking. It features a custom configuration parser to set up virtual servers, manage route-specific settings, define error pages, and limit client body sizes. 

**Key Features:**
- **HTTP Methods:** Full support for GET, POST, and DELETE requests.
- **I/O Multiplexing:** Asynchronous, non-blocking socket management using `poll()`.
- **CGI Execution:** Support for dynamic content generation via external scripts, handled completely asynchronously.
- **Session Management:** Built-in cookie-based session management for persistent user states and authentication.
- **Static File Serving:** Delivery of HTML, CSS, JavaScript, multimedia content, and directory listings.
- **Custom Configuration:** Nginx-style configuration files allowing for multiple virtual servers on different ports or server names.

---

## Instructions

### Compilation
The project includes a `Makefile` to handle the compilation of the C++98 source files.

To compile the server, navigate to the repository root and run:
```bash
make
```
This will generate the `webserv` executable.

**Additional Commands:**
- `make clean`: Removes the compiled object files.
- `make fclean`: Removes both the object files and the `webserv` executable.
- `make re`: Fully cleans and recompiles the project.

### Execution
The server needs a configuration file to set up its ports, routes, and behavior. If no configuration file is provided, it defaults to `conf/default.conf`.

Run the server with:
```bash
./webserv [path/to/config.conf]
```

**Examples:**
```bash
./webserv
./webserv conf/default.conf
```

Once running, the server will listen on the ports defined in the configuration file. You can test it by opening a web browser or using `curl` (e.g., `http://localhost:8080`).

---

## Resources

### Classic References
- [RFC 2616 (HTTP/1.1)](https://datatracker.ietf.org/doc/html/rfc2616) - The core HTTP/1.1 protocol specification.
- [RFC 3875 (CGI Version 1.1)](https://datatracker.ietf.org/doc/html/rfc3875) - The Common Gateway Interface specification.
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) - An essential resource for understanding socket programming and networking in C/C++.
- Nginx Documentation - Used as a structural and conceptual reference for parsing configuration files.

### AI Usage
Artificial Intelligence was utilized during the development of this project as a conceptual guide and pair-programming assistant for the following tasks:
- **Troubleshooting and Critical Bug Fixing:** Assisting with complex concurrency issues, resolving `waitpid` handling during asynchronous CGI execution, and crucially identifying and fixing critical errors during the core testing and evaluation phases to ensure server robustness.
- **Web Development (Frontend):** Guiding the development of the web interface, including HTML/CSS styling and the structural design of the login pages and testing sites hosted by the server.
- **Implementation Guidance:** Providing structured explanations and conceptual guidance for implementing features like `client_max_body_size` validation, HTTP handler logic, and server name routing.
- **Documentation:** Generating this `README.md` file and helping maintain internal project compliance checklists and task trackers to ensure alignment with 42 evaluation criteria.
