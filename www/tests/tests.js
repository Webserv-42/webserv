function setLog(logId, text, append) {
  var log = document.getElementById(logId);
  if (!log) {
    return;
  }
  if (append) {
    log.textContent += text;
  } else {
    log.textContent = text;
  }
}

async function runFetch(method, path, body, logId) {
  var id = logId || "log";
  setLog(id, "Running " + method + " " + path + "...\n", false);
  try {
    var options = { method: method, headers: {} };
    if (body !== undefined) {
      options.headers["Content-Type"] = "text/plain";
      options.body = body;
    }
    var res = await fetch(path, options);
    var text = await res.text();
    setLog(id, "Status: " + res.status + " " + res.statusText + "\n", true);
    setLog(id, "Body (first 400 chars):\n" + text.slice(0, 400) + "\n", true);
  } catch (err) {
    setLog(id, "Error: " + err + "\n", true);
  }
}

async function uploadFile(inputId, logId) {
  var id = logId || "log";
  var input = document.getElementById(inputId);
  if (!input || !input.files.length) {
    setLog(id, "Please pick a file first.", false);
    return;
  }
  var data = new FormData();
  data.append("file", input.files[0]);
  setLog(id, "Uploading file...\n", false);
  try {
    var res = await fetch("/uploads/", { method: "POST", body: data });
    var text = await res.text();
    setLog(id, "Status: " + res.status + " " + res.statusText + "\n", true);
    setLog(id, "Body (first 400 chars):\n" + text.slice(0, 400) + "\n", true);
  } catch (err) {
    setLog(id, "Error: " + err + "\n", true);
  }
}

function goDelete(inputId, logId) {
  var id = logId || "log";
  var input = document.getElementById(inputId);
  if (!input || !input.value) {
    setLog(id, "Please enter a filename first.", false);
    return;
  }
  runFetch("DELETE", "/uploads/" + input.value, undefined, id);
}
