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


def respond_dashboard_file():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    dashboard_path = os.path.join(script_dir, "..", "dashboard", "index.html")
    try:
        with open(dashboard_path, "r", encoding="utf-8") as handle:
            html = handle.read()
    except OSError:
        sys.stdout.write("Status: 500 Internal Server Error\r\n")
        sys.stdout.write("Content-Type: text/plain; charset=utf-8\r\n\r\n")
        sys.stdout.write("Dashboard file not found.")
        return

    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n\r\n")
    sys.stdout.write(html)


def main():
    cookie_header = os.environ.get("HTTP_COOKIE", "")
    if not has_session_id(cookie_header):
        respond_redirect("/login/index.html", "<p>Please log in.</p>")
        return

    respond_dashboard_file()
    


if __name__ == "__main__":
    main()
