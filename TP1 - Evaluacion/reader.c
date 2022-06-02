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
static int32_t readFromPipe(char *buff);

int main(void)
{
	char bufferIn[BUFFER_SIZE];
        char bufferFile[BUFFER_SIZE];
	int32_t bytesRead;
	int32_t returnCode; 
	int32_t fd_log;
	int32_t fd_sig;
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
    
    if( (fd_log = open("log.txt", O_RDWR )) < 0 )
     {
      printf("Error al crear o abrir el archivo log.txt!");   
      exit(1);             
     }
	
    if( (fd_sig = open("sign.txt", O_RDWR )) < 0 )
     {
      printf("Error al crear o abrir el archivo sign.txt!");   
      exit(1);             
     }
     	

    do
     {
		bytesRead = readFromPipe(bufferIn);
	    

	    	t = time(NULL);
	        newTime = localtime(&t);
	    
		if(strstr(bufferIn, "DATA") != NULL)
		{  
		    snprintf(bufferFile, BUFFER_SIZE, "%s --> %s" , bufferIn , asctime(newTime));
		    printf("%s" , bufferFile);
                    write(fd_log ,bufferFile , strlen(bufferFile));		    

			
		}
		else if(strstr(bufferIn, "SIGN") != NULL)
		{
	            snprintf(bufferFile, BUFFER_SIZE, "%s --> %s" , bufferIn , asctime(newTime));
		    printf("%s" , bufferFile);
                    write(fd_sig ,bufferFile , strlen(bufferFile));
              		
		}

		
			
      }
     while (bytesRead > 0);

	close(fd_log);
	close(fd_sig);
	
	return 0;
}


static int32_t readFromPipe(char *buff)
{
	
  int32_t bytesRead = 0;	
	
  bzero(buff, BUFFER_SIZE);  
 
  if ((bytesRead = read(fd, buff, BUFFER_SIZE)) == -1)
	 {
	    perror("read");
         }
        else
	 {
	     buff[bytesRead] = '\0';
	 }	
	
   return bytesRead;
}
