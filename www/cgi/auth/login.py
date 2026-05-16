#!/usr/bin/env python3
import os
import sys
import json
from urllib.parse import parse_qs

USERS_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "users.json")


def read_post_data():
    length = os.environ.get("CONTENT_LENGTH", "")
    try:
        size = int(length)
    except (TypeError, ValueError):
        size = 0
    body = sys.stdin.read(size) if size > 0 else ""
    return parse_qs(body)


def load_users():
    if not os.path.exists(USERS_FILE):
        return {"admin": "admin"}
    try:
        with open(USERS_FILE, "r") as f:
            return json.load(f)
    except Exception:
        return {"admin": "admin"}


def respond_json(status, payload):
    sys.stdout.write("Status: {}\r\n".format(status))
    sys.stdout.write("Content-Type: application/json\r\n\r\n")
    sys.stdout.write(json.dumps(payload))


def main():
    method = os.environ.get("REQUEST_METHOD", "GET").upper()

    if method != "POST":
        respond_json("405 Method Not Allowed", {"ok": False, "error": "Method not allowed"})
        return

    data = read_post_data()
    username = data.get("username", [""])[0].strip()
    password = data.get("password", [""])[0].strip()

    users = load_users()

    if username in users and users[username] == password:
        respond_json("200 OK", {"ok": True, "redirect": "/cgi-bin/app/dashboard.py"})
        return

    respond_json("401 Unauthorized", {"ok": False, "error": "Usuario o contraseña incorrectos"})


if __name__ == "__main__":
    main()
