/* Integrantes: Fonseca, Matias - Oliva, Raimundo Laboratorio 4 Sistemas Operativos y Redes */ 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

// Librerias para el manejo de sockets.
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

// Definicion del numero del puerto y el porte del buffer.
#define PORT 8000
#define BUFFERSIZE 1024

// Funcion para crear un socket.
void crearSocket(int *sock){
    if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  
		// Si falla en la creacion del socket manda este mensaje y se cierra.      
        printf("Error Creacion de Socket\n");
        exit(0);
    }
}

// Funcion que configura la conexion para el cliente.
void configurarCliente(int sock, struct sockaddr_in *conf){
	// Direcciones.
	conf->sin_family = AF_INET;
	// Port.
	conf->sin_port = htons(PORT);
	// IP del servidor.
	conf->sin_addr.s_addr = inet_addr("127.0.0.1");

	// Se intenta establecer conexion con el servidor.
	if (connect(sock, (struct sockaddr *)conf, sizeof(*conf)) < 0){
		// Si falla se muestra un mensaje y se cierra.
		printf("\n Connection Failed \n");
		exit(0);
	}
}

// Funcion principal.
int main(int argc, char const *argv[]){
	// Ve si se dio algun parametro en la terminal.
    if (argc < 2)
		// Si no se da ningun parametro, se cierra.
        return 0;

    char nombreClientes[100];
	// Arreglo que copia lo primero que se dio en la terminal.
    strcpy(nombreClientes, argv[1]);

	// Creacion del socket.
    int sockCliente;
    crearSocket(&sockCliente);

    // Conectarse al Servidor.
    struct sockaddr_in confServidor;
    configurarCliente(sockCliente, &confServidor);
   
	// Se inicializa la variable en 1.
    int primerMensaje = 1; 
	
    // Comunicarse con el servidor.
	while(1){
		// Dos arreglos de caracteres para almacenar mensajes que entran y salen.
		char buffer[BUFFERSIZE]={0}, buffer2[BUFFERSIZE]={0};
		
		// If que evalua si es el primer mensaje o no.
		if (primerMensaje){
			// Si es asi se manda lo de "nombreClientes" al servidor.
    	    send(sockCliente, nombreClientes, strlen(nombreClientes), 0);
		    primerMensaje = 0;

			// Se lee la respuesta del servidor y su mensaje se muestra en la terminal.
    	    int valread = read(sockCliente, buffer, BUFFERSIZE);
		    fputs(buffer, stdout);
		}else{
			  // Si no es el primer mensaje, se solicita que se ingrese un mensaje.
 			  printf("Mensaje: ");
			  // Se lee y se almacena el mensaje.
			  fgets(buffer, BUFFERSIZE, stdin);

			  // Si el mensaje es "BYE" se envia al servidor.	
			  if (strcmp(buffer, "BYE\n")==0){
				  send(sockCliente, buffer, strlen(buffer), 0);
				  // Se lee la respuesta que manda el servidor.
				  int valread = read(sockCliente, buffer2, BUFFERSIZE);
				  // El mensaje del servidor se concatena a "nombreClientes".
				  strcat(buffer2, nombreClientes);
				  // Se muestra la respuesta en la terminal.
   			      fputs(buffer, stdout);
				  // Se rompe el ciclo y se cierra el socket.
				  break;

			  }else{
				  // Si no es "BYE", se envia el mensaje.	
				  send(sockCliente, buffer, strlen(buffer), 0);
				  // Se lee la respuesta del servidor.
				  int valread = read(sockCliente, buffer, BUFFERSIZE);
				  // Se muestra la respuesta en la terminal.				 
				  printf("%s",buffer);
			  }
		}
	}

	// Se cierra el socket.
	close(sockCliente);

	// El programa termina y se devuelve un 0.
	return 0;
}