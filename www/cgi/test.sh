#!/bin/bash
echo -ne "Content-Type: text/plain\r\n\r\n"
echo "Hola desde Bash CGI!"
echo "Method: $REQUEST_METHOD"
