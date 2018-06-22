#ifndef __UTILS__
#define __UTILS__

#define max(a,b) (a>b)?a:b
#define min(a,b) (a<b)?a:b

/*	endsWith():
	Devuelve 1 cuando una cadena termina como una subcadena dada, en caso
	contrario devuelve 0.	*/
char endsWith(char str[], char subStr[]);

/*	startsWith():
	Devuelve 1 cuando una cadena empieza como una subcadena dada, en caso
	contrario devuelve 0.	*/
char startsWith(char str[], char subStr[]);

/*	hasInside():
	Devuelve 1 cuando una subcadena se encuentra dentro de otra, en caso
	contrario devuelve 0.	*/
char hasInside(char str[], char subStr[]);

/*	nextChar():
	Busca un carácter en una cadena dada y devulve un puntero de con la
	dirección en la que se encuentra dicho carácter en la cadena dada. En
	caso contrario devuelve NULL.
	Al contrario que otras funciones de biblioteca estándar como strtok()
	esta función no almacena ninguna dirección de memoria, hay que
	reutilizarla pasándole de nuevo los parámetros.	*/
char* nextChar(char str[], char a);

#endif
