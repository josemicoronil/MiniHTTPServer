#ifndef __WHITELIST_H__
#define __WHITELIST_H__

/*	Data structures	*/
struct strNode {
	char* s1;
	char* s2;
	void* n;
};

typedef struct strNode Node;

typedef struct strList {
	Node* m;
	unsigned int count;
} List;

List* newList(void);
void parseStr(List*l, char str[]);
void addStr(List* l, char str1[], char str2[]);
char* getKey(List* l, unsigned int i);
char* getValue(List* l, unsigned int i);
char* getKeyValue(List* l, char str[]);
unsigned int getListSize(List* l);
void freeList(List* l);

/*	File management	*/
void read_whitelist(List* l, char path[]);


#endif
