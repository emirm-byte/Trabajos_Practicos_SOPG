#include <stdio.h>
#include <stdlib.h>
#include "SerialManager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

//Funcion que realiza la primera incializacion del server TCP//
int initTCPServer(void);

//Variables globales usadas en el programa//
pthread_t thread_serial;
socklen_t addr_len;
struct sockaddr_in clientaddr;
struct sockaddr_in serveraddr;
int newfd;
int n;
int s;
struct sigaction sa1;
struct sigaction sa2;

//Inicializamos un Mutex para proteger el acceso de una variable compartida//
pthread_mutex_t mutexFd = PTHREAD_MUTEX_INITIALIZER;

//Funciones que inicializan y configuran el manejo de las señales//
static void initSignalHandlers(void);
static void bloquearSignals(void);
static void desbloquearSignals(void);

//Uso el mismo handler para SIGINT y SIGTERM//
void sigint_term_handler(int sig)
{
	char *msj = "Cierre de programa por SIGINT o SIGTERM!\n"; 
	write(0, msj , strlen(msj));
	close(newfd); //Cierro conexiones en el Socket TCP//
	serial_close(); //Cierro el puerto Serie//
	pthread_cancel (thread_serial); //Cierro el Thread
	exit(EXIT_SUCCESS);
}

//Acá se define el código del Thread que se encarga del manejo del serial port//

void* serial_port_thread (void* message)
{
	
	int bytesReadSP = 0; //Bytes leidos por el serial port//
	char bufferSP[20] = {0}; //Buffer definido para la lectura de datos del serial port//
	void *ret;

	//Abro la conexion del serial port//
	if(serial_open(1,115200)!=0)
        {
          printf("Error abriendo puerto serie\r\n");
		  strcpy(ret, "Exit Serial port Thread by serial port init Error");
		  pthread_exit(ret);
        }
	else
		{
	      printf("Puerto serie OK!\r\n");	
		}
 
    //Bucle infinito dentro del hilo del serial port//
	while(1)
	{
	
	 //Leo los datos provenientes del serial port//
	 if((bytesReadSP = serial_receive(bufferSP,12)) != -1)
	 {
		printf("Se han leido: %d bytes\n",bytesReadSP);
	 	printf("La trama leida es: %s \n",bufferSP);
		
		//Pongo en un mutex la comprobacion del FD del conexion ya que es una variable compartida//
		pthread_mutex_lock (&mutexFd);
		if(newfd == -1)
		{
			perror("Error en el socket no se puede enviar datos");
		}
		else
		{
			//Si tengo datos en el buffer y el FD de conexiòn esta definido envio por el serial port//
			if (write (newfd, bufferSP, strlen(bufferSP)) == -1)
    		{	
      			perror("Error escribiendo mensaje en socket");
    		}

		}
		pthread_mutex_unlock (&mutexFd);

	 }
	 usleep(10000); // Delay de 10ms para que el CPU no se vaya al 100%
	} 

	serial_close();
	strcpy(ret, "Exit Serial port Thread");
	pthread_exit(ret);      
}


int main(void)
{
	char bufferSocket[20];
	int res;
	void *ret;	

	//Inicio los handlers de las senales SIGINT y SIGTERM//
	initSignalHandlers();
	//Bloqueo ambas señales antes de crear el hilo para el serial port//
	bloquearSignals(); 

	//Creo el hilo que se encarga de manejar el serial port//
	res = pthread_create (&thread_serial, NULL, serial_port_thread,NULL);
	if (res != 0){
		perror("Error al crear el hilo serial port");
		return(-1);
	}
	
	//Desbloqueo las señales deseadas solo para el hilo principal// 
	desbloquearSignals();

	printf("Inicio Serial Service\r\n");

	//Inicio el server TCP//
	if(initTCPServer()!=0){
		perror("Error al iniciar el Socket TCP");
		return(-1);	   
	}

	while(1)
	{
	   // Ejecutamos accept() para recibir conexiones entrantes
		addr_len = sizeof(struct sockaddr_in);
		
		//Uso el mutex para la asignacion del FD de conexion//
		pthread_mutex_lock (&mutexFd);
		if ( (newfd = accept(s, (struct sockaddr *)&clientaddr, &addr_len)) == -1)
      	{
		      perror("Error en accept");
		      exit(1);
		}
	 	pthread_mutex_unlock(&mutexFd);
		
		while(1)
		{
			printf  ("server:  conexion desde:  %s\n", inet_ntoa(clientaddr.sin_addr));   

			// Leemos mensaje de cliente - interface Service
			n =read(newfd,bufferSocket,20);
			//Si el valor retornado por read es -1 o 0, el cliente esta deconectado//
			if( (n == -1) || (n ==0 ) )
			{
				//Salgo del bucle secudario para poder aceptar luego otra conexion//
				perror("Error leyendo mensaje en socket - cliente desconectado");
				close(newfd);
				break; 
			}
			else
			{
				//Lo que recibo por socket lo envio por el serial port//
				bufferSocket[n]=0;
				printf("Recibi %d bytes.:%s\n",n,bufferSocket);
				printf("La cadena es: %s\n",bufferSocket);
				serial_send(bufferSocket,strlen(bufferSocket));
				
			}
		
		}

	}

	pthread_join (thread_serial, &ret); //Espero a que finalice el hilo serial port//
	printf("El hilo Serial port finalizó por '%s'\n", (char *)ret); //imprimo la razón de dicha finalización//
	exit(EXIT_SUCCESS);
	return 0;
}


//Funcion que inicializa una parte del server TCP//

int initTCPServer(void){
 
	s = socket(AF_INET,SOCK_STREAM, 0);

	// Cargamos datos de IP:PORT del server
    	bzero((char*) &serveraddr, sizeof(serveraddr));
    	serveraddr.sin_family = AF_INET;
    	serveraddr.sin_port = htons(10000);
    	if(inet_pton(AF_INET, "127.0.0.1", &(serveraddr.sin_addr))<=0)
    	{
        	fprintf(stderr,"ERROR invalid server IP\r\n");
        	return 1;
    	}

	// Abrimos puerto con bind()
	if (bind(s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1) {
		close(s);
		perror("listener: bind");
		return 1;
	}

	// Seteamos socket en modo Listening
	if (listen (s, 10) == -1) // backlog=10
  	{
    	    perror("Error en listen");
    		exit(1);
  	}

}

//Funcion que inicializa los handlers de las señales usadas en el programa//
static void initSignalHandlers(void){

	sa1.sa_handler = sigint_term_handler;
	sa1.sa_flags = SA_RESTART;
	sigemptyset(&sa1.sa_mask);
	if (sigaction(SIGINT, &sa1, NULL) == -1) {
		perror("Error en Sigaction");
		exit(1);
	}

	sa2.sa_handler = sigint_term_handler;
	sa2.sa_flags = SA_RESTART;
	sigemptyset(&sa2.sa_mask);
	if (sigaction(SIGTERM, &sa2, NULL) == -1) {
		perror("Error en Sigaction");
		exit(1);
	}

}

//Función que bloquea las señales SIGINT y SIGTERM//
static void bloquearSignals(void)
{
	sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

//Funcion que desbloquea las señales SIGINT y SIGTERM//
static void desbloquearSignals(void)
{
	sigset_t set;
    int s;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

}



















