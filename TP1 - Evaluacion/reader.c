#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#define FIFO_NAME "com_channel"
#define BUFFER_SIZE 300

static int32_t fd;

int main(void)
{
	char bufferIn[BUFFER_SIZE];
	int32_t bytesRead;
	int32_t returnCode; 
	FILE *flog_ptr;
	FILE *fsig_ptr;
	time_t t;
	struct tm *newTime;

    
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
	
    
    //Abrimos o creamos los archivos correspondientes//	
    flog_ptr = fopen("log.txt","a");
    if(flog_ptr == NULL)
     {
      printf("Error al crear o abrir el archivo log.txt!");   
      exit(1);             
     }
	
    fsig_ptr = fopen("sign.txt","a");
    if(fsig_ptr == NULL)
     {
      printf("Error al crear o abrir el archivo sign.txt!");   
      exit(1);             
     }
     	
    /* Loop until read syscall returns a value <= 0 */
    do
     {
		bytesRead = readFromPipe(bufferIn);
	    
	    	t = time(NULL);
	        newTime = localtime(&t);
	    
		if(strstr(bufferIn, "DATA") != NULL)
		{  
		    fprintf(flog_ptr , "%s --> %s" , bufferIn , asctime(newTime));		
		}
		else if(strstr(bufferIn, "SIGN") != NULL)
		{
		    fprintf(fsig_ptr , "%s --> %s" , bufferIn , asctime(newTime));		
		}
			
      }
     while (bytesRead > 0);

	fclose(flog_ptr);
	fclose(fsig_ptr);
	
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
