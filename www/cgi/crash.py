#!/usr/bin/env python3
import sys
sys.stdout.write("Content-Type: text/plain\r\n\r\n")
sys.stdout.write("Error 500: The script is going to crash intentionally right now...\n")
sys.stdout.flush()
raise Exception("Internal error caused for evaluation !")
