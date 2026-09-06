*This project has been created as part of the 42 curriculum by zhassna, hrhilane.*

# ft_irc

## Description

### What is IRC?

**IRC (Internet Relay Chat)** is a protocol used for real-time communication over a network.

IRC uses a **client-server architecture**: clients connect to an IRC server, send commands and messages to it, and the server manages users, channels, permissions, and communication between clients.

### Project Goal

`ft_irc` is an IRC server written in **C++98**.

The goal of the project is to understand how network servers work by implementing a server capable of handling multiple clients simultaneously.

The project focuses on:

* TCP/IP and socket programming
* Client/server communication
* Non-blocking I/O
* I/O multiplexing with `poll()`
* IRC command parsing
* User and channel management
* Permissions and error handling

The server uses a **single `poll()` loop** to manage multiple clients.

---

## Features

* Multiple simultaneous clients
* Password-protected server
* User registration and nickname management
* Channel creation and management
* Public and private messages
* Channel operators
* Invitations and kicking users
* Channel topics
* Channel modes
* Client disconnection and error handling

---

# Instructions

## Compilation

```bash
git clone <repository-url>
cd ft_irc
make
```

To clean the project:

```bash
make clean
make fclean
make re
```

## Running the Server

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 8080 1234
```

The server will listen on port `8080` and require `1234` as the password.

You can connect using an IRC client such as **HexChat**, **irssi**, or **WeeChat**.

For simple testing, you can also use:

```bash
nc localhost 8080
```

---

# Usage

## Registration

A client needs to provide the server password, nickname, and user information:

```text
PASS 1234
NICK alice
USER alice 0 * :Alice
```

Once registration is complete, the client can use the IRC commands.

## Supported Commands

| Command   | Syntax                                    | Description                  |
| --------- | ----------------------------------------- | ---------------------------- |
| `PASS`    | `PASS <password>`                         | Authenticate with the server |
| `NICK`    | `NICK <nickname>`                         | Set/change nickname          |
| `USER`    | `USER <user> <mode> <unused> :<realname>` | Register user                |
| `JOIN`    | `JOIN <channel>`                          | Join/create a channel        |
| `PART`    | `PART <channel>`                          | Leave a channel              |
| `PRIVMSG` | `PRIVMSG <target> :<message>`             | Send a message               |
| `INVITE`  | `INVITE <user> <channel>`                 | Invite a user                |
| `TOPIC`   | `TOPIC <channel> [:<topic>]`              | View/change topic            |
| `KICK`    | `KICK <channel> <user>`                   | Remove a user                |
| `MODE`    | `MODE <channel> <mode> [parameter]`       | Change channel settings      |
| `QUIT`    | `QUIT [:<message>]`                       | Disconnect                   |

### Example

```text
PASS 1234
NICK alice
USER alice 0 * :Alice

JOIN #general

PRIVMSG #general :Hello everyone!

INVITE bob #general

TOPIC #general :Welcome to our channel!

PART #general
QUIT :Goodbye!
```

---

# Technical Overview

The server follows the basic TCP server lifecycle:

```text
socket()
   ↓
bind()
   ↓
listen()
   ↓
poll()
   ↓
accept() / recv() / send()
   ↓
poll()
```

`poll()` is used to monitor the server socket and all connected clients, allowing multiple clients to be handled without creating a thread for each connection.

IRC messages are text-based and normally terminated with `\r\n`. Since network data can arrive in partial or multiple messages, received data is buffered and parsed into complete commands before processing.

---

# Resources

* **RFC 2812** — Internet Relay Chat: Client Protocol
* **RFC 2811** — Internet Relay Chat: Channel Management
* **Beej's Guide to Network Programming** — Socket programming and networking
* **Linux `man` pages** — `socket()`, `bind()`, `listen()`, `accept()`, `poll()`, `recv()`, `send()`, etc.
* **cppreference** — C++98 and standard library reference

### AI Usage

AI was used as a learning and development aid during the project, mainly for:

* Understanding TCP sockets and client/server architecture.
* Understanding `poll()` and I/O multiplexing.
* Understanding IRC commands and protocol behavior.
* Investigating compiler errors and debugging issues.
* Reviewing code and discussing possible implementation approaches.

The final implementation, architecture, testing, and project decisions were made by the authors.
