#ifndef __SERVIDOR_HTTP_H__
#define __SERVIDOR_HTTP_H__

/*	Estas funciones se han definido como macros por cuestiones de sencillez y
	de rendimiento.	También podrían ser funciones normales con un inline	*/
#define set_max_fd(__max_fd, __new_fd) if (__max_fd < __new_fd) __max_fd = __new_fd
#define decrementa_max_fd(__max_fd, __closed_fd) if (__max_fd == __closed_fd) --__max_fd

/*	Constantes	*/
#define PUERTO 5432
#define TAM_BUFFER 1024
#define WHITELIST_PATH "whitelist.txt"

/*	Estados	*/
#define ESTADO_INICIAL -1
#define GET_RECIBIDO 1
#define METODO_DESC_RECIBIDO 2
#define RECURSO_CONOCIDO 3
#define RECURSO_DESC 4
#define ERROR_INTERNO 5
#define FIN 0

/*	Funciones auxiliares	*/
ssize_t write_n(int desc, void *buf, size_t bytes);
ssize_t read_n(int desc, void *buf, size_t bytes);
void maneja_int(int s);
void maq_estados(int desc);
void enviar_recurso(char rec[], int desc);

#endif
