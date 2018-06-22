#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include "servidor_http.h"
#include "whitelist.h"
#include "utils.h"

/*	Variables globales	*/
char fin_ejecucion=0;
char estado;
int to_close=-1;
List* wl = NULL;

int main() {
	/*	Señal para manejar las interrupciones de teclado	*/
	signal(SIGINT, maneja_int);
	signal(SIGPIPE, SIG_IGN);

	/*	Creación del socket	*/
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("Error al abrir el socket");
		return 1;
	}

	/*	Declaración e inicialización de variables y estructuras usadas por
		el socket principal	*/
	struct sockaddr_in srv_addr;
	struct sockaddr_in cli_addr;
	uint16_t puerto = PUERTO;
	socklen_t addr_len = sizeof(srv_addr);
	socklen_t cli_addr_len = addr_len;

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(puerto);
	srv_addr.sin_addr.s_addr = INADDR_ANY;

	/*	Vinculación del socket	*/
	if ( bind(sd, (struct sockaddr *) &srv_addr, addr_len) == -1 ) {
		perror("Error en bind");
		close(sd);
		return 1;
	}

	/*	Configuración del socket en modo escucha	*/
	if ( listen(sd, 5) == -1 ) {
		perror("Error en listen");
		close(sd);
		return 1;
	}

	/*	Se lee la whitelist	*/
	wl = newList();
	read_whitelist(wl, WHITELIST_PATH);


	/*	Variables para usar con 'select'	*/
	fd_set cjto_orig;
	fd_set cjto_modif;
	struct timeval tv_orig;
	struct timeval tv_modif;
	int max_fd;

	/*	Se configuran conjuntos y estructuras de tiempo. La razón de ser de
		las estructuras de tiempo es que cada 3 segundos, en caso de no
		existir datos pueda ejecutarse de nuevo el bucle completo, para que
		en caso de	cambiarse la variable 'fin_ejecucion' el programa pueda
		terminar	*/
	FD_ZERO(&cjto_orig);
	FD_SET(sd, &cjto_orig);
	max_fd = sd;
	tv_orig.tv_sec = 3;
	tv_orig.tv_usec = 0;


	printf("Servidor iniciado en el puerto %d\n", PUERTO);

	while (!fin_ejecucion) {
		/*	Se inicia el conjunto que va a modificar 'select'	*/
		FD_ZERO(&cjto_modif);
		int a;
		for (a=sd; a<=max_fd; ++a) {
			if (FD_ISSET(a, &cjto_orig)) FD_SET(a, &cjto_modif);
		}

		/*	Se inicia la estructura de tiempo que se modificará	*/
		tv_modif.tv_sec = tv_orig.tv_sec;
		tv_modif.tv_usec = tv_orig.tv_usec;

		/*	Se ejecuta la función 'select'	*/
		if (select(max_fd+1, &cjto_modif, NULL, NULL, &tv_modif) == -1) {
			if (errno == EINTR) break;
			perror("Error en 'select'");
			fin_ejecucion = 1;
			continue;
		}

		/*	Acepta conexiones desde el socket que está escuchando.
			Se hace la comprobación de que el socket máximo añadido es el
			1023 debido a que fd_set tiene un máximo de 1024 huecos posibles*/
		if ( (max_fd < 1023) && FD_ISSET(sd, &cjto_modif) ) {
			if (max_fd < 1023) {
				int sd_n = accept(sd, (struct sockaddr *) &cli_addr, &cli_addr_len);
				if (sd_n == -1) {
					perror("Error en accept");
					errno = 0;
				} else {
					/*	Se añaden el decriptor de la nueva conexión entrante	*/
					FD_SET(sd_n, &cjto_orig);
					set_max_fd(max_fd, sd_n);
					printf(	"Conex. entrante de %s\n",
						inet_ntoa(cli_addr.sin_addr));
				}
			} else {
				printf("··> Hay más peticiones en cola que no se están antendiendo.\n");
			}
		}

		/*	Comprueba si el resto de conexiones abierta tienen alguna
			petición.	*/
		for (a=sd+1; a<=max_fd; ++a) {
			if (FD_ISSET(a, &cjto_modif)) {
				estado = ESTADO_INICIAL;
				maq_estados(a);
			}

			/*	En caso de que se haya cerrado la conexión se cierra y se
				quita del conjunto original.	*/
			if (to_close > -1) {
				close(to_close);
				FD_CLR(to_close, &cjto_orig);
				decrementa_max_fd(max_fd, to_close);
				to_close = -1;
			}
		}
	}

	/*	Cierra todas las conexiones que pudiera haber abiertas	*/
	int i;
	for (i=sd; i<=max_fd; ++i) if (FD_ISSET(i, &cjto_orig)) close(i);

	/*	Elimina la lista enlazada de la whitelist	*/
	freeList(wl);

	printf("Programa finalizado correctamente\n");
	return 0;
}	/*	Fin de main()	*/




/*	maneja_int:
	Función que maneja la señal SIGINT producida por una interrupción de
	teclado. Cambia una variable centinela con la finalidad de que el
	programa pueda parar de forma controlada	*/
void maneja_int(int s) {
	fin_ejecucion = 1;
}


/*	write_n:
	Función que ejecuta 'write' de forma que se asegure de tener escritos todos
	los bytes que se le hayan indicado.	*/
ssize_t write_n(int desc, void *buf, size_t bytes) {
	char *p = (char *) buf;
	int count = 0;
	int escrito;

	do {
		escrito = write(desc, p+count, bytes-count);
		count += escrito;
	} while (	count < bytes &&
				(	escrito > 0 ||
			  		( escrito == 0 && errno == EINTR )
				)
			);

	if (escrito == -1) return -1;

	return count;
}	/*	Fin de write_n()	*/


/*	read_n:
	Función que ejecuta 'read' de forma que se asegure de tener leidos todos
	los bytes que se le hayan.	*/
ssize_t read_n(int desc, void *buf, size_t bytes) {
	char *p = (char *) buf;
	int count = 0;
	int leido;

	do {
		leido = read(desc, p+count, bytes-count);
		count += leido;
	} while (	count < bytes &&
				(	leido > 0 ||
			  		( leido == 0 && errno == EINTR )
				)
			);

	if (leido == -1) return -1;

	return count;
}	/*	Fin de read_n()	*/


/*	maq_estados:
	Función que contiene toda la lógica principal para responder a cualquier
	petición HTTP que le haya llegado al programa. Esta función es totalmente
	mejorable, ya que sólo da servicio a peticiones 'GET' de ficheros
	seleccionados a traves de una lista blanca.	*/
void maq_estados(int desc) {
	char buffer[TAM_BUFFER];
	int totalLeido = 0;
	int leido;

	/*	Lee la petición del cliente	*/
	do {
		leido = read(desc, buffer+totalLeido, TAM_BUFFER-totalLeido-1);
		if (leido == 0) {
			estado = FIN;
			to_close = desc;
			return;
		}
		totalLeido += leido;
		buffer[totalLeido] = '\0';
	} while (!endsWith(buffer,"\r\n\r\n"));

	printf("Se procesa la siguiente peticion:\n%s\n", buffer);
	char* pos = buffer;
	char* recurso = NULL;
	char* nc;

	while (estado != FIN) {
		switch (estado) {
			case ESTADO_INICIAL:
				/*	Se comprueba que el método recibido es 'GET' y no otro no
					que no sea procesable por el programa.	*/
				if (startsWith(pos, "GET")) {
					estado = GET_RECIBIDO;
				} else {
					estado = METODO_DESC_RECIBIDO;
				}
				pos = nextChar(pos, ' ')+1;
				break;
			case GET_RECIBIDO:
				/*	En el caso de recibir un método 'GET' se comprueba que el
					recurso solicitado está permitido en la whitelist y está
					disponible en el equipo.	*/
				nc = nextChar(pos, ' ');
				*nc = '\0';
				recurso = getKeyValue(wl, pos);
				if (recurso == NULL) {
					estado = RECURSO_DESC;
				} else {
					estado = RECURSO_CONOCIDO;
				}

				pos=nc+1;

				/*	Además comprobamos si el protocolo usado es HTTP. No
					comprueba la versión porque para este servidor no importa
					si usa la 0.9, 1.0, ó 1.1. En el caso que usara 2.0 no
					tendría la petición este formato, por lo que en caso de
					recibir una de este tipo el servidor enviaría un 400
					Bad Request tal y como está hecho.	*/
				if (!startsWith(pos, "HTTP")) estado = METODO_DESC_RECIBIDO;
				break;

			case METODO_DESC_RECIBIDO:
				/*	En el caso de recibir un método no procesable se le indica
					cliente además de que la conexión será cerrada. Se cierra
					el socket dedicado a este cliente.	*/
				write_n(desc, "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n",
					47);
				to_close = desc;
				estado = FIN;
				break;

			case RECURSO_CONOCIDO:
				/*	En el caso de conocer el recurso se le envía al cliente.
					Lo primero que se realiza es avisar al cliente de que la
					petición es correcta, posteriormente se envían los recursos.
					El estado se cambia en la función anterior.	*/
				write_n(desc, "HTTP/1.1 200 OK\r\n", 17);
				enviar_recurso(recurso, desc);

				/*	Ahora hay que comprobar si el cliente quiere mantener la
					conexión abierta o la quiere cerrar. En caso de que la
					quiera mantener abierta nos pondrá un 'Connection:
					keep-alive'.	*/
				if (!hasInside(pos, "Connection: keep-alive")) to_close = desc;
				estado = FIN;
				break;

			case RECURSO_DESC:
				/*	En caso de no tener el recurso solicitado se informa.	*/
				write_n(desc,
						"HTTP/1.1 404 Not Found\r\nConnection: close\r\n",
						43);
				printf("\nNo encontrado. Se envía 404.html\n");
				enviar_recurso("404.html", desc);
				to_close = desc;
				estado = FIN;
				break;

			case ERROR_INTERNO:
				/*	En el caso de que la solicitud haya provocado algún error
					en el servidor se le indica al cliente y se cierra la
					conexión.	*/
				write_n(desc,
						"HTTP/1.1 500 Internal Error\r\nConnection: close\r\n\r\n",
						50);
				to_close = desc;
				estado = FIN;
				printf("- Enviado error interno \n\n");
				break;
		}
	}
}	/*	Fin de maq_estados()	*/

/*	enviar_recurso:
	Una vez procesada la petición dentro de la máquina de estados llama esta
	función para que lea el recurso solicitado dentro del equipo y pueda
	devolverlo al cliente.	*/
void enviar_recurso(char rec[], int desc) {
	char tam_rec[21];
	struct stat rec_stat;
	/*	Se usa la función 'stat' para conocer el tamaño del fichero a enviar */
	if (stat(rec, &rec_stat)) {
		perror("Error en stat");
		estado = ERROR_INTERNO;
		return;
	}
	/*	Se convierte el valor numérico del tamaño en una cadena.	*/
	sprintf(tam_rec, "%li", rec_stat.st_size);

	int fd = open(rec, O_RDONLY);
	if (fd == -1) {
		perror("Error en open");
		estado = ERROR_INTERNO;
		return;
	}
	/*	Se le indica el tamaño del recurso que se le va a enviar.	*/
	write_n(desc, "Content-Length: ", 16);
	write_n(desc, tam_rec, strlen(tam_rec));

	if (endsWith(rec, ".html")) {
		/*	En caso de ser un recurso HTML se especifica.	*/
		write_n(desc,"\r\nContent-Type: text/html; charset=utf-8\r\n\r\n", 44);
	} else if (endsWith(rec, ".ico")) {
		/*	En caso de ser un icono (como el favicon) se especifica.	*/
		write_n(desc,"\r\nContent-Type: image/x-icon\r\n\r\n", 32);
	} else {
		/*	En el caso de ser otro tipo de contenido se indica que se va a
			enviar un archivo adjunto binario con el nombre de archivo que se
			indica.	*/
		write_n(desc, "\r\nContent-Type: application/octet-stream\r\n", 42);
		write_n(desc,"Content-Disposition: attachment; filename=\"", 43);
		write_n(desc, rec, strlen(rec));
		write_n(desc, "\"\r\n\r\n", 5);
	}

	/*	Proceso principal de lectura y envío del archivo. Se leen bloques de
		tamaño 'TAM_BUFFER' y se envían. En caso de ser menores dichos bloques	*/
	long int totalEscrito = 0;
	int leido;
	int escrito;
	char bufferFichero[TAM_BUFFER];
	do {
		leido = read_n(fd, bufferFichero, TAM_BUFFER);
		escrito = write_n(desc, bufferFichero, leido);
	} while	(	(totalEscrito < rec_stat.st_size) &&
				(escrito > 0) &&
				(leido > 0)
			);

	/*	En el caso de que la lectura o la escritura haya tenido alguna
		interrupción se elimina de errno.	*/
	if (errno == EINTR) errno = 0;

	/*	En caso de existir algún error de transmisión se indica cerrar el
		descriptor en caso de que se haya cerrado la conexión.	*/
	if (escrito < 0 || leido < 0) {
		perror("Error en la transmision del fichero");
		to_close = desc;
	} else {
		printf("- Respuesta antendida\n\n");
	}

	/*	Se cierra el descriptor de lectura del fichero local.	*/
	close(fd);
}	/*	Fin de enviar_recurso()	*/
