#include "whitelist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 128

List* newList(void) {
	List* n = (List*) malloc(sizeof(List));
	n->m = NULL;
	n->count = 0;
	return n;
}

void parseStr(List* l, char str[]) {
	if (!l) return;
	if (strlen(str) < 3) return;
	char key[64];
	char* value = str;

	while ( (*value) != ',' && (*value) != '\0' ) ++value;
	if ( (*value) == '\0' ) return;
	value++;

	memcpy(key, str, value-str-1);
	key[value-str-1] = '\0';
	addStr(l, key, value);
}

void addStr(List* l, char str1[], char str2[]) {
	if (!l) return;
	Node* tmp;
	if (l->m == NULL) {
		l->m = (Node*) malloc(sizeof(Node));
		tmp = l->m;
	} else {
		tmp = l->m;
		while (tmp->n != NULL) tmp = tmp->n;
		tmp->n = (Node*) malloc(sizeof(Node));
		tmp = tmp->n;
	}

	tmp->s1 = (char*) malloc(strlen(str1)+1);
	strcpy(tmp->s1, str1);
	tmp->s2 = (char*) malloc(strlen(str2)+1);
	strcpy(tmp->s2, str2);
	tmp->n = NULL;
	l->count = (l->count + 1);
}

char* getKey(List* l, unsigned int i) {
	if (l == NULL || l->count < i) return NULL;

	Node* tmp = l->m;
	while (i-- > 0) tmp = tmp->n;

	return tmp->s1;
}

char* getValue(List* l, unsigned int i) {
	if (l == NULL || l->count < i) return NULL;

	Node* tmp = l->m;
	while (i-- > 0) tmp = tmp->n;

	return tmp->s2;
}

char* getKeyValue(List* l, char str[]) {
	if (l == NULL || l->count == 0) return NULL;

	Node* tmp = l->m;
	while (tmp != NULL) {
		if (strcmp(str, tmp->s1) == 0) return tmp->s2;
		tmp = tmp->n;
	}

	return NULL;
}

inline unsigned int getListSize(List* l) {
	return l->count;
}

void freeList(List* l) {
	if (!l) return;
	Node* del; Node* next;
	next = l->m;
	while (next != NULL) {
		del = next;
		next = del->n;

		free(del->s1);
		free(del->s2);
		free(del);
	}

	free(l);
}

void read_whitelist(List* l, char path[]) {
	FILE* fp = fopen(path, "r");

	if (!fp) {
		perror("Error de lectura de la whitelist");
		errno = 0;
		return;
	}

	char buffer[BUFFER_SIZE];
	while (fscanf(fp, "%s", buffer) > 0) parseStr(l, buffer);

	fclose(fp);
}
