#include <string.h>
#include "utils.h"

char endsWith(char str[], char subStr[]) {
	int lenSubStr = strlen(subStr);
	int startStr = strlen(str) - lenSubStr;

	if (startStr < 0) return 0;

	int i;
	for (i=0; i<lenSubStr; i++) if (str[startStr+i] != subStr[i]) return 0;

	return 1;
}


char startsWith(char str[], char subStr[]) {
	int lenSubStr = strlen(subStr);

	if (lenSubStr > strlen(str)) return 0;

	int i;
	for (i=0; i<lenSubStr; i++) if (str[i] != subStr[i]) return 0;

	return 1;
}


char hasInside(char str[], char subStr[]) {
	int lenSubStr = strlen(subStr);
	int lenStr = strlen(str);

	if (lenSubStr > lenStr) return 0;

	/*	Con 'maxAvance' se calcula cuántos caracteres como máximo puede
		haber antes de que aparezca dicha cadena. Obviamente, si hay
		menos caracteres disponibles en la cadena principal que el
		tamaño de la subcadena a buscar, significa que no se encuentra
		en la principal.	*/
	int i; int j=0;
	int maxAvance = lenStr-lenSubStr;
	for (i=0; (j>0) || (i<=maxAvance); ++i) {
		if (str[i] == subStr[j]) {
			++j;
		} else {
			j=0;
			continue;
		}

		if (j == lenSubStr) return 1;
	}

	return 0;
}


char* nextChar(char str[], char a) {
	char* tmp = str;
	while ( ((*tmp) != a) && ((*tmp) != '\0') ) ++tmp;
	if ( (*tmp) == '\0' ) return NULL;
	return tmp;
}
