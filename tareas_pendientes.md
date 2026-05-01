# Tareas Pendientes Webserv

## 1. Refactorización Completa del CGI (No bloqueante)
- **Bloqueo del Servidor:** Actualmente `CgiHandler::readParent` usa `waitpid` con `usleep(50000)`, lo que bloquea todo el bucle principal de eventos del servidor. **Obligatorio por subject:** Todas las operaciones de I/O deben pasar por `poll()` y nunca bloquear el servidor.
- **Solución:** Aprovechar las funciones `handleCgiResponse` y `registredCgiFd` que se empezaron a incluir en `Server.cpp`. Los pipes del CGI deben añadirse al array de `pollfd` para leerlos de forma asíncrona.
- **Bug crítico de ejecución:** En `HttpHandlerGet::serveCgiIfMatch`, cuando se usa `cgi_path` (ej. `/usr/bin/python3`), no se está pasando la ruta del script `.py` como argumento a `execve`. El script solo ejecuta `/usr/bin/python3` sin argumentos y finaliza sin output. `executeCgi` debe recibir tanto el ejecutable como la ruta física del archivo y ejecutar `[ejecutable, scriptPath, NULL]`.

## 2. Validación de `client_max_body_size` Temprana
- **Problema actual:** En `Client::ReadFromClient`, se acumula todo el cuerpo de la petición en `c.buffer` antes de llamar a `ProcessRequest()`. Si un cliente sube un archivo de 5GB, el servidor intentará cargar 5GB en RAM y colapsará.
- **Solución:** Validar la cabecera `Content-Length` contra el valor `serverConf.clientMaxBodySize` en el estado `READING_HEADERS` (tan pronto se reciben los headers) y rechazar la conexión con un error `413 Payload Too Large` inmediatamente si excede el límite.

## 3. Arreglo de Uploads (Subida de Archivos)
- **Configuración faltante:** La petición `POST` en `/uploads/` actualmente retorna `403 Forbidden` porque en `HttpHandlerPost.cpp` valida `loc->upload_enable == false`. En `default.conf` falta establecer la directiva correspondiente para habilitar subidas, o cambiar la lógica por defecto.
- **Nombres de archivo estáticos:** Los archivos subidos se guardan ciegamente como `uploaded_X.bin`. Se recomienda (al menos como bonus o mejora de usabilidad) extraer el nombre del archivo si la petición usa `multipart/form-data`, o extraerlo del header.

## 4. Mejora del Manejo de Conexiones (Keep-Alive)
- **Problema:** En `Client::sendResponse`, se usa `c.buffer.find("Connection: Keep-alive")` para verificar si se mantiene la conexión. Esto busca en **toda** la petición, incluyendo el cuerpo. Si un usuario sube un archivo de texto que contiene "Connection: Keep-alive", causará un fallo de lógica.
- **Solución:** Revisar el valor estructurado que parseó `HttpRequest` en los headers, usando `req.getHeaders()["Connection"]`.

## 5. Separación de HTML Embebido
- En `HttpHandler.cpp`, los métodos `buildErrorResponse` y `generateDirectoryListing` tienen código HTML incrustado en el C++. Para una mejor mantenibilidad y buena práctica, esto debe estar en plantillas HTML reales en el disco que se carguen según sea necesario.

## 6. Depuración y Logs
- Quitar los `std::cout` excesivos para clientes estables, o añadir un modo "Debug" que se active/desactive para mantener limpia la consola.

## 7. Requisito Obligatorio: `README.md`
- **Problema:** El repositorio no cuenta con el archivo `README.md` obligatorio en la raíz.
- **Solución:** Crear un `README.md` que incluya:
  1. En cursiva en la primera línea: *This project has been created as part of the 42 curriculum by <login1>[, <login2>...]*
  2. Sección "Description" con el objetivo del proyecto.
  3. Sección "Instructions" con información de compilación y ejecución.
  4. Sección "Resources" listando referencias y una descripción de cómo se ha usado la IA.
