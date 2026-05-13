
const testsInfo = {
    1: {
        objetivo: "Verificación de la capacidad del servidor para servir archivos estáticos (index.html) y gestionar sesiones iniciales.",
        comando: "curl -i -s http://localhost:8080/",
        verificacion: "Retorno de HTTP 200 OK y cabecera Set-Cookie: session_id=...."
    },
    2: {
        objetivo: "Comprobación del manejo de errores cuando un recurso no se encuentra en el servidor.",
        comando: "curl -i -s http://localhost:8080/nonexistent.html",
        verificacion: "Retorno de HTTP 404 Not Found y visualización de la página de error personalizada."
    },
    3: {
        objetivo: "Prueba de transferencia de datos hacia el servidor para la creación de nuevos archivos en disco.",
        comando: `curl -i -s -X POST -d "test body" \\
                    http://localhost:8080/uploads/test.txt`,
        verificacion: "Retorno de HTTP 201 Created y confirmación de la existencia del archivo en el directorio de subidas."
    },
    4: {
        objetivo: "Validación de las restricciones de seguridad por location que bloquean métodos no autorizados.",
        comando: `curl -i -s -X POST -d "test body" \\
                        http://localhost:8080/`,
        verificacion: "Retorno de HTTP 405 Method Not Allowed."
    },
    5: {
        objetivo: "Prueba de eliminación de recursos específicos almacenados en el servidor.",
        comando: "curl -i -s -X DELETE http://localhost:8080/uploads/uploaded_0.bin",
        verificacion: "Retorno de HTTP 204 No Content tras la eliminación exitosa."
    },
    6: {
        objetivo: "Verificación del procesamiento de scripts externos (Python) sin bloquear el bucle principal del servidor.",
        comando: "curl -i -s http://localhost:8080/cgi-bin/test.py",
        verificacion: "Retorno de HTTP 200 OK y ejecución correcta del script."
    },
    7: {
        objetivo: "Comprobación de que el servidor rechaza cuerpos de petición que exceden el límite configurado (1MB).",
        comando: `dd if=/dev/urandom of=bigfile.bin bs=1M count=2 && \\
                    curl -i -s -X POST -T bigfile.bin http://localhost:8080/uploads/`,
        verificacion: "Retorno inmediato de HTTP 413 Payload Too Large."
    },
    8: {
        objetivo: "Validación del enrutamiento basado en la cabecera Host para manejar múltiples servidores en el mismo puerto.",
        comando: `curl -i -s -H "Host: localhost" \\
                http://localhost:8080/`,
        verificacion: "Retorno de HTTP 200 OK al coincidir el header con el server_name configurado."
    },
    9: {
        objetivo: "Comprobación del mecanismo de redirección permanente definido en el archivo de configuración.",
        comando: "curl -i -L -s http://localhost:8080/old/",
        verificacion: "Retorno de HTTP 301 con cabecera Location: / y redirección automática."
    },
    10: {
        objetivo: "Verificación de la persistencia de la conexión TCP para procesar múltiples peticiones en un solo ciclo.",
        comando: `(echo -ne "GET / HTTP/1.1\\r\\nHost: localhost\\r\\nConnection: keep-alive\\r\\n\\r\\nGET / HTTP/1.1\\r\\nHost: localhost\\r\\n\\r\\n"; sleep 1) | nc localhost 8080`,
        verificacion: "El servidor procesa ambas peticiones antes de cerrar el socket."
    },
    11: {
        objetivo: "Prueba de subida de archivos reales utilizando el formato estándar de formularios web (multipart).",
        comando: `curl -i -s -F "file=@README.md" \\
                http://localhost:8080/uploads/`,
        verificacion: "Retorno de HTTP 201 Created y almacenamiento íntegro del archivo."
    },
    12: {
        objetivo: "Validación de la generación automática de un listado de archivos HTML cuando no hay un index presente.",
        comando: "curl -i -s http://localhost:8080/uploads/",
        verificacion: "Retorno de HTML que contiene enlaces (<a href...) a los archivos del directorio."
    },
    13: {
        objetivo: "Comprobación de la robustez del servidor ante archivos de configuración corruptos o inválidos.",
        comando: "./webserv conf/test_invalid.conf",
        verificacion: "Impresión de mensaje de error descriptivo y salida controlada sin segfaults."
    },
    14: {
        objetivo: "Verificación de que el servidor transfiere correctamente los datos del cuerpo POST al script CGI.",
        comando: `curl -i -s -X POST -d "param1=value1" \\
                    http://localhost:8080/cgi-bin/test.py`,
        verificacion: "El script recibe y procesa los parámetros vía STDIN."
    },
    15: {
        objetivo: "Prueba de estabilidad y concurrencia para asegurar que el servidor soporta carga masiva.",
        comando: "siege -c 50 -r 10 -b http://localhost:8080/",
        verificacion: "500+ transacciones exitosas con 0% de tasa de fallo."
    },
    16: {
        objetivo: "Comprobación del ciclo de vida de una sesión: emisión y reconocimiento de cookies.",
        comando: `COOKIE=$(curl -i -s http://localhost:8080/ | grep "Set-Cookie" | awk '{print $2}') && \\
                    curl -i -s -b "$COOKIE" http://localhost:8080/`,
        verificacion: "Persistencia del session_id entre peticiones consecutivas."
    },
    17: {
        objetivo: "Validación del bloqueo de acceso a rutas restringidas o no configuradas.",
        comando: "curl -i -s http://localhost:8080/restricted/",
        verificacion: "Retorno de HTTP 403 Forbidden o 404 según la política de seguridad."
    }
};

function openModal(id) {
    const test = tests.find(t => t.id === id);
    const info = testsInfo[id];
    if (!test || !info) return;

    // 1. Título básico
    document.getElementById('modal-title').textContent = `Test #${id}: ${test.title}`;
    
    // 2. Limpiar el contenedor anterior
    const container = document.getElementById('modal-desc');
    container.innerHTML = '';

    // 3. Usar el Template
    const template = document.getElementById('modal-template');
    const clone = template.content.cloneNode(true);

    // 4. Inyectar los datos en el clon
    clone.querySelector('.modal-obj-text').textContent = info.objetivo;
    clone.querySelector('.modal-code-text').textContent = info.comando;
    clone.querySelector('.modal-check-text').textContent = info.verificacion;

    // 5. Añadir el clon al modal y mostrar
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