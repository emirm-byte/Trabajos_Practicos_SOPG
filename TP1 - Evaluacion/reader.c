#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define FIFO_NAME "com_channel"
#define BUFFER_SIZE 300

static int32_t fd;

int main(void)
{
	char bufferIn[BUFFER_SIZE];
	int32_t bytesRead;
	int32_t returnCode; 
	

    
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1  )
    {
        printf("Error al crear la tubería nombrada: %d\n", returnCode);
        exit(1);
    }
    
    
	printf("Esperando por el proceso Writer...\n");
	if ( (fd = open(FIFO_NAME, O_RDONLY) ) < 0 )
    {
        printf("Error al abrir el archivo de la tuberia nombrada: %d\n", fd);
        exit(1);
    }
    
	printf("Tenemos un Writer\n");

    /* Loop until read syscall returns a value <= 0 */
	do
	{
        
		bytesRead = readFromPipe(bufferIn);
		if(strstr(bufferIn, "DATA") != NULL)
		{
			
			
			
		}
		else if(strstr(bufferIn, "SIGN") != NULL)
		{
			
			
		}
			
		
	}
	while (bytesRead > 0);

	return 0;
}


static int32_t readFromPipe(char *buff)
{
	
	int32_t bytesRead = 0;
	
	if ((bytesRead = read(fd, buff, BUFFER_SIZE)) == -1)
	 {
	    perror("read");
     }
    else
	 {
		buff[bytesRead] = '\0';
		printf("reader: read %d bytes: \"%s\"\n", bytesRead, buff);
	 }	
	
   return bytesRead;
}