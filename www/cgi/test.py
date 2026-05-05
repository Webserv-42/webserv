#!/usr/bin/env python3
import os
import sys

def main():
    method = os.environ.get("REQUEST_METHOD", "GET")
    uri = os.environ.get("REQUEST_URI", "")
    body = sys.stdin.read()
    response = "Method: {}\nURI: {}\nBody: {}\n".format(method, uri, body)
    sys.stdout.write("Content-Type: text/plain\r\n\r\n")
    sys.stdout.write(response)

if __name__ == "__main__":
    main()
