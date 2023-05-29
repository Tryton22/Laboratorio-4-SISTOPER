/* Integrantes: Fonseca, Matias - Oliva, Raimundo Laboratorio 4 Sistemas Operativos y Redes */ 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Librerias para el manejo de sockets.
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Libreria para el manejo de hilos.
#include <pthread.h>

// Definicion del numero del puerto y el porte del buffer.
#define port 8000
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
void configurarServidor(int socket, struct sockaddr_in *conf){
    conf->sin_family = AF_INET;		   		    // Direcciones.
    conf->sin_addr.s_addr = htonl(INADDR_ANY);	// IP del servidor.
    conf->sin_port = htons(port);				// Port.

    // Se intenta enlazar el socket creado con lo dado anteriormente.
    if ((bind(socket, (struct sockaddr *) conf, sizeof(*conf))) < 0) { // bind!
        // Si falla manda el mensaje y se cierra.
        printf("Error de enlace\n");
        exit(0);
    }
}

// Funcion para "escuchar" distintas conexiones.
void escucharClientes(int sock, int n){
    if (listen(sock, n) < 0) {	
        // Si esto falla se manda el mensaje y se cierra.	
        printf("Error listening\n");
        exit(0);
    }
}

// Funcion para aceptar una conexion entrante.
void aceptarConexion(int *sockNuevo, int sock, struct sockaddr_in *conf ){
    int tamannoConf = sizeof(*conf);

    // Socket de cliente nuevo y se actualiza a la configuracion dada anteriormente.
    if ((*sockNuevo = accept(sock, (struct sockaddr *) conf, &tamannoConf)) < 0) {
        // Si se falla en conectar manda el mensaje y se cierra.
        printf("Error accepting\n");
        exit(0);
    }
}

// Funcion para reemplazar caracteres en una cadena.
void reemplazarCaracter(char *buffer, char caracter1, char caracter2){
    for(int j=0; j < strlen(buffer); j++){
        if (buffer[j] == caracter1){
            buffer[j] = caracter2;
        }
    }              
}

// Funcion para invertir el orden de las letras en una palabra.
void invertirPalabra(char *palabra){  
    int largo = strlen(palabra);
    char palabra2[largo];
    
    for (int i = 0; i < largo; i++)  {  
        palabra2[i] = palabra[largo - i - 1];  
    }  
    
    palabra2[largo] = '\0';
    strcpy(palabra, palabra2);
}  

// Funcion que maneja, mediante un hilo, una comunicacion con un cliente.
void *Servidor(void *arg){
   // Se recibe el socket como una variable.
   int *sockCliente = (int *)arg;
   // Se establece en 1 la variable "primerMensaje".
   int primerMensaje = 1;
   // Se inicia un while para la comunicacion activa con el cliente.
   while(1){
        // Variables para almacenar los distintos mensajes que entran y salen.
        char nombre[BUFFERSIZE]={0};
        char buffer[BUFFERSIZE]={0};
        char buffer2[BUFFERSIZE]={0};
        int valread;
        
        valread = read(*sockCliente, buffer, BUFFERSIZE);

        // Se verifica que sea el primer mensaje.
        if (primerMensaje){
            // Si es asi, se envia un Hola al cliente.
            strcpy(buffer2, "Hola ");
            strcat(buffer2, buffer);
            strcat(buffer2, "\n");
            // Y se guarda el nombre del cliente.
            strcpy(nombre, buffer);
            // Se muestra el saludo en la terminal y se envia al cliente.
            printf("%s", buffer2);
            send(*sockCliente, buffer2, strlen(buffer2), 0);
            primerMensaje = 0;
        }else{
            // Si el mensaje es un "BYE"
            if (strcmp(buffer, "BYE\n")==0){
                // Se mezcla el mensaje con el nombre del cliente.
                strcpy(buffer2, buffer);
                strcat(buffer2, nombre);
                // Y se envia el cliente.
                send(*sockCliente, buffer2, strlen(buffer2), 0);
                
                // Se muestra el mensaje en la terminal y se rompe el ciclo.
                printf("%s", buffer2);                    
                break;
            }

            // Si no es el primer mensaje ni tampoco un "BYE".
            // Se muestra en la terminal.
            printf("%s", buffer);
            
            // Se separa el mensaje.
            char buffer2[1024]={0}, *palabra;
            palabra = strtok (buffer, " ");
            int primeraPalabra = 1;

            while (palabra != NULL){
                //Invertir palabra
                if (palabra[strlen(palabra)-1]=='\n'){
                    palabra[strlen(palabra)-1]='\0';
                }
                    
                invertirPalabra(palabra);     
                
                if (primeraPalabra){
                    // Se agrega la palabra invertida.
                    strcpy(buffer2, palabra);
                    primeraPalabra = 0;
                }else{
                    strcat(buffer2, " "); 
                    strcat(buffer2, palabra);
                }

                palabra = strtok (NULL, " ");
            }

            // Y se envia el nuevo mensaje al cliente.
            send(*sockCliente, buffer2, strlen(buffer2), 0);
        } 
   }  
} // Despues de que termina el ciclo, la hebra se destruye.

// Funcion principal.
int main(int argc, char *argv[]){
    // Ve si se dio algun parametro en la terminal.
    if (argc < 2)
        // Si no se da ningun parametro, se cierra.
        return 0;
    
    // El argumento dado ahora es un numero entero y se guarda.
    // Hace referencia al numero de clientes.
    int nClientes = strtol(argv[1], NULL, 10);
    
    // Si el numero de clientes es menor que 1, se cierra.
    if (nClientes < 1)
       return 0;

    // Creacion del socket servidor.
    int sockServidor;
    crearSocket(&sockServidor);

    // Se configura el servidor.
    struct sockaddr_in confServidor; // Se pasan al socket servidor detalles del mismo.
    configurarServidor(sockServidor, &confServidor);

    ///3. Escuchando conexiones entrantes
    escucharClientes(sockServidor, nClientes);

    // Se establecen conexiones entrantes.
    struct sockaddr_in confCliente;
    int sockCliente;

    // Ciclo for que acepta las conexiones entrantes.
    for(int i=0; i < nClientes; i++){
        aceptarConexion(&sockCliente, sockServidor, &confCliente);
        
        // Se crean nuevos hilos.
        // Para las comunicaciones entre el servidor y los clientes.
        pthread_t hebra;
        pthread_create(&hebra, NULL, Servidor, (void *)&sockCliente[i]);       
   }

    // Se termina el programa y se devuelve un 0.
    return 0;
}