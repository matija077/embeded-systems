#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_LENGTH 500
static char receive[BUFFER_LENGTH];

int main()
{
	int fd, ret;
	char stringToSend[BUFFER_LENGTH];
	//printf("starting tes aplication ...\n");
	fd = open("/dev/prsa_matijachar", O_RDWR);
		if (fd < 0){
		perror("Failed to open device\n");
		return errno;
	}
	printf("Please write your message:\n");
	scanf("%[^\n]%*c", stringToSend);
	printf("Writing this msg [%s] to character device driver.\n", stringToSend);
	ret = write(fd, stringToSend, strlen(stringToSend));
	if (ret < 0){
		perror("Failed to write the message.");
		return errno;
	}

	printf("Press ENTER to read back from device\n");
	getchar();

	ret = read(fd, receive, BUFFER_LENGTH);
	if (ret < 0){
		perror("Failed to read the message from the device");
		return errno;
	}
	printf("The recieved message is: <%s>\n", receive);
	//printf("GOODBYE\n");
	return 0;
}

