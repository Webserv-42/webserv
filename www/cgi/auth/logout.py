#!/usr/bin/env python3
import sys

def main():
    sys.stdout.write("Status: 302 Found\r\n")
    sys.stdout.write("Location: /login/index.html\r\n")
    sys.stdout.write(
        "Set-Cookie: session_id=; "
        "Max-Age=0; "
        "Path=/; "
        "HttpOnly\r\n"
    )
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n\r\n")
    sys.stdout.write(
        "<!doctype html><html><head>"
        "<meta charset='utf-8'>"
        "<title>Logout</title>"
        "</head><body>"
        "<p>Logged out. Redirecting...</p>"
        "</body></html>"
    )

if __name__ == "__main__":
    main()