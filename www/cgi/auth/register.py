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
        return {}
    try:
        with open(USERS_FILE, "r") as f:
            return json.load(f)
    except Exception:
        return {}


def save_users(users):
    with open(USERS_FILE, "w") as f:
        json.dump(users, f)


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
    confirm  = data.get("confirm",  [""])[0].strip()

    if not username or not password:
        respond_json("400 Bad Request", {"ok": False, "error": "Username and password are required"})
        return

    if password != confirm:
        respond_json("400 Bad Request", {"ok": False, "error": "Passwords do not match"})
        return

    users = load_users()

    if username in users:
        respond_json("409 Conflict", {"ok": False, "error": "User '{}' already exists".format(username)})
        return

    users[username] = password
    save_users(users)

    respond_json("200 OK", {"ok": True, "message": "Registration completed. You can now sign in."})


if __name__ == "__main__":
    main()
