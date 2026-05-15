
const testsInfo = {
    1: {
        objective: "Verify the server can serve static files (index.html) and handle initial sessions.",
        command: "curl -i -s http://localhost:8080/",
        verification: "Return of HTTP 200 OK and Set-Cookie header: session_id=...."
    },
    2: {
        objective: "Check error handling when a resource is not found on the server.",
        command: "curl -i -s http://localhost:8080/nonexistent.html",
        verification: "Return of HTTP 404 Not Found and display of the custom error page."
    },
    3: {
        objective: "Test data transfer to the server to create new files on disk.",
        command: `curl -i -s -X POST -d "test body" \\
                    http://localhost:8080/uploads/test.txt`,
        verification: "Return of HTTP 201 Created and confirmation that the file exists in the uploads directory."
    },
    4: {
        objective: "Validate per-location security restrictions that block unauthorized methods.",
        command: `curl -i -s -X POST -d "test body" \\
                        http://localhost:8080/`,
        verification: "Return of HTTP 405 Method Not Allowed."
    },
    5: {
        objective: "Test deletion of specific resources stored on the server.",
        command: "curl -i -s -X DELETE http://localhost:8080/uploads/uploaded_0.bin",
        verification: "Return of HTTP 204 No Content after successful deletion."
    },
    6: {
        objective: "Verify external script processing (Python) without blocking the server main loop.",
        command: "curl -i -s http://localhost:8080/cgi-bin/tests/test.py",
        verification: "Return of HTTP 200 OK and correct script execution."
    },
    7: {
        objective: "Check that the server rejects request bodies exceeding the configured limit (1MB).",
        command: `dd if=/dev/urandom of=bigfile.bin bs=1M count=2 && \\
                    curl -i -s -X POST -T bigfile.bin http://localhost:8080/uploads/`,
        verification: "Immediate return of HTTP 413 Payload Too Large."
    },
    8: {
        objective: "Validate Host header based routing to handle multiple servers on the same port.",
        command: `curl -i -s -H "Host: localhost" \\
                http://localhost:8080/`,
        verification: "Return of HTTP 200 OK when the header matches the configured server_name."
    },
    9: {
        objective: "Check the permanent redirect mechanism defined in the configuration file.",
        command: "curl -i -L -s http://localhost:8080/old/",
        verification: "Return of HTTP 301 with Location: / header and automatic redirect."
    },
    10: {
        objective: "Verify TCP connection persistence to process multiple requests in a single cycle.",
        command: `(echo -ne "GET / HTTP/1.1\\r\\nHost: localhost\\r\\nConnection: keep-alive\\r\\n\\r\\nGET / HTTP/1.1\\r\\nHost: localhost\\r\\n\\r\\n"; sleep 1) | nc localhost 8080`,
        verification: "The server processes both requests before closing the socket."
    },
    11: {
        objective: "Test real file upload using standard web form multipart format.",
        command: `curl -i -s -F "file=@README.md" \\
                http://localhost:8080/uploads/`,
        verification: "Return of HTTP 201 Created and full storage of the file."
    },
    12: {
        objective: "Validate automatic generation of an HTML file listing when no index is present.",
        command: "curl -i -s http://localhost:8080/uploads/",
        verification: "Return of HTML containing links (<a href...) to files in the directory."
    },
    13: {
        objective: "Check server robustness against corrupt or invalid configuration files.",
        command: "./webserv conf/test_invalid.conf",
        verification: "Print a descriptive error message and exit cleanly without segfaults."
    },
    14: {
        objective: "Verify that the server passes POST body data correctly to the CGI script.",
        command: `curl -i -s -X POST -d "param1=value1" \\
                    http://localhost:8080/cgi-bin/tests/test.py`,
        verification: "The script receives and processes parameters via STDIN."
    },
    15: {
        objective: "Stress stability and concurrency to ensure the server supports heavy load.",
        command: "siege -c 50 -r 10 -b http://localhost:8080/",
        verification: "500+ successful transactions with 0% failure rate."
    },
    16: {
        objective: "Check session lifecycle: cookie issuance and recognition.",
        command: `COOKIE=$(curl -i -s http://localhost:8080/ | grep "Set-Cookie" | awk '{print $2}') && \\
                    curl -i -s -b "$COOKIE" http://localhost:8080/`,
        verification: "Session_id persists between consecutive requests."
    },
    17: {
        objective: "Validate blocking access to restricted or unconfigured routes.",
        command: "curl -i -s http://localhost:8080/restricted/",
        verification: "Return of HTTP 403 Forbidden or 404 depending on security policy."
    }
};

function openModal(id) {
    const test = tests.find(t => t.id === id);
    const info = testsInfo[id];
    if (!test || !info) return;
    document.getElementById('modal-title').textContent = `Test #${id}: ${test.title}`;
    
    const container = document.getElementById('modal-desc');
    container.innerHTML = '';

    const template = document.getElementById('modal-template');
    const clone = template.content.cloneNode(true);

    clone.querySelector('.modal-obj-text').textContent = info.objective;
    clone.querySelector('.modal-code-text').textContent = info.command;
    clone.querySelector('.modal-check-text').textContent = info.verification;

    container.appendChild(clone);
    document.getElementById('modal-overlay').style.display = 'flex';
}

function closeModal() {
    document.getElementById('modal-overlay').style.display = 'none';
}

window.onclick = function(event) {
    const overlay = document.getElementById('modal-overlay');
    if (event.target === overlay) closeModal();
}