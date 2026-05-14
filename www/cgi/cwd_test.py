#!/usr/bin/env python3
import os, sys
sys.stdout.write("Content-Type: text/plain\r\n\r\n")
with open("relative_file.txt", "w") as f:
    f.write("File written correctly using a relative path!")
with open("relative_file.txt", "r") as f:
    content = f.read()
sys.stdout.write(f"Current directory: {os.getcwd()}\nContent: {content}\n")
