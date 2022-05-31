#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#define FIFO_NAME "com_channel"
#define BUFFER_SIZE 300

static int32_t fd;
struct sigaction sa1;
struct sigaction sa2;

static void writeToPipe(char *buff);
static void initSignalHandlers(void);


//Handlers de las señales definidas//

void sigusr1_handler(int sig) 
{
	char *msg_sig = "SIGN:1\n";
	writeToPipe(msg_sig);
}


void sigusr2_handler(int sig) 
{
	char *msg_sig = "SIGN:2\n";
	writeToPipe(msg_sig);
}


int main(void)
{
   
    char bufferIn[BUFFER_SIZE];
    char bufferOut[BUFFER_SIZE];
    int32_t returnCode;
	
	
    // Se crea la tuberia (pipe) nombrada. Retorna -1 si ya existe por lo tanto no realiza ninguna acción//
	
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1 )
    {
        printf("Error al crear la tuberia nombrada: %d\n", returnCode);
        exit(1);
    }

    //Abrimos la tuberia nombrada. Se bloquea hasta que otro proceso la abre //
	printf("Esperando por el proceso Reader...\n");
	if ( (fd = open(FIFO_NAME, O_WRONLY) ) < 0 )
    {
        printf("Error al abrir la cola nombrada: %d\n", fd);
        exit(1);
    }
    
    // La llamada al sistema open retorna sin error cuando otro proceso se une a la tuberia //
	printf("Se unió el proceso Reader.\n");
	
	initSignalHandlers(); //Se incian y asignan los handlers de señales luego de que se habilita la tuberia para comunicar//

	while (1)
	{
		printf("Ingrese el texto y luego presion Enter: \n");
		fgets(bufferIn, sizeof(bufferIn), stdin);
		
		snprintf(bufferOut, sizeof(bufferOut), "DATA: %s", bufferIn);
		
		writeToPipe(bufferOut);
        		
	}
	return 0;
}


static void writeToPipe(char *buff)
{
   int32_t bytesWrote;
	
   if ((bytesWrote = write(fd, buff, strlen(buff)-1)) == -1)
     {
		perror("write");
     }
    else
     {
		printf("Writer: %d bytes escritos\n", bytesWrote);
     }
	
}

static void initSignalHandlers(void)
{
	sa1.sa_handler = sigusr1_handler;
	sa1.sa_flags = 0; //SA_RESTART;
	sigemptyset(&sa1.sa_mask);
	if (sigaction(SIGUSR1, &sa1, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	
	sa2.sa_handler = sigusr2_handler;
	sa2.sa_flags = 0; //SA_RESTART;
	sigemptyset(&sa2.sa_mask);
	if (sigaction(SIGUSR2, &sa2, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}
	
}
