# MiniHTTPServer
Ejemplo de servidor HTTP simple para *NIX usando las llamadas que el SO nos proporciona.

El servidor toma una "lista blanca" con aquellos archivos que se pueden ofrecer al cliente. Si el cliente envía una petición de algún archivo no presente en la lista devolverá un error 404. Las entradas de la lista son pares de cadenas divididas por una coma (sin espacios ni nada extra), siendo lo que está a la izquierda de la coma la petición que le puede enviar el cliente al servidor y lo que está a la derecha de ésta el archivo que irá a leer y a transmitir al cliente.

A excepción de los archivos relacionados con la lista blanca (`whitelist.c` y `whitelist.h`) el resto están comentados tanto el uso de las funciones como qué hacen por dentro.
Para poder atender a más de una petición a la vez el servidor usa la función `select()` tanto para atender nuevas peticiones de conexión como conexiones ya establecidas anteriormente.

Para descargar todos los archivos del repositorio pulsar el botón verde de "Clone or download" y luego pulsar "Download ZIP". Una vez descargados, para compilar correctamente el programa usando GCC hay que ponerle como entrada los tres archivos .c que hay en el repositorio a una salida común. En este ejemplo se realiza así dando como resultado un archivo de salida llamado "srv":
```bash
gcc -Wall -o srv utils.c whitelist.c servidor_http.c
```
Para comprobar que el servidor funciona sólo hay que introducir `127.0.0.1:5432` en nuestro navegador.
