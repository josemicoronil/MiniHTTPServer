#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

int main(void) {
	char a = hasInside("HTTP/1.1\r\nConnection: keep-alive\r\n", "Connection: keap-alive");
	printf("hasInside: %s\n", (a?"yeah":"nein"));
	return 0;
}
