#!/usr/bin/env python3
import os
import sys


def main():
    sys.stdout.write("Content-Type: text/plain\r\n\r\n")
    sys.stdout.write("CGI info\n")
    sys.stdout.write("Method: {}\n".format(os.environ.get("REQUEST_METHOD", "")))
    sys.stdout.write("URI: {}\n".format(os.environ.get("REQUEST_URI", "")))
    sys.stdout.write("Server: {}\n".format(os.environ.get("SERVER_SOFTWARE", "webserv")))


if __name__ == "__main__":
    main()
