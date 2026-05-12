#!/usr/bin/env python3
import os
import sys


def has_session_id(cookie_header):
    if not cookie_header:
        return False
    for part in cookie_header.split(";"):
        name_value = part.strip().split("=", 1)
        if len(name_value) == 2 and name_value[0] == "session_id" and name_value[1]:
            return True
    return False


def respond_redirect(location, message):
    sys.stdout.write("Status: 302 Found\r\n")
    sys.stdout.write("Location: {}\r\n".format(location))
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n\r\n")
    sys.stdout.write(
        "<!doctype html><html><head><meta charset=\"utf-8\"><title>Redirect</title>"
        "</head><body>{}</body></html>".format(message)
    )


def respond_dashboard():
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n\r\n")
    sys.stdout.write(
        "<!doctype html>"
        "<html><head><meta charset=\"utf-8\"><title>Dashboard</title></head>"
        "<body>"
        "<h1>Dashboard</h1>"
        "<p>Welcome! Your session is active.</p>"
        "<form action=\"/cgi-bin/logout.py\" method=\"post\">"
        "<button type=\"submit\">Log out</button>"
        "</form>"
        "</body></html>"
    )


def main():
    cookie_header = os.environ.get("HTTP_COOKIE", "")
    if not has_session_id(cookie_header):
        respond_redirect("/login/index.html", "<p>Please log in.</p>")
        return

    respond_redirect("/dashboard/index.html", "<p>Cargando dashboard...</p>")
    


if __name__ == "__main__":
    main()
