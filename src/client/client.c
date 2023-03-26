#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../forge.h"
#include "../protocol.h"
#include "../ui.h"

/* Un message est définit sur 8 octets. */
#define SIZE_MESS 8

/* Le port et l'adresse pour les tests sur le serveur de lulu. */
#define PORT "7777"
#define ADRESSE "lucy.informatique.univ-paris-diderot.fr"

/* A séparer en plusieurs fonction TODO */

/**
 * @brief Récupère l'adresse du serveur et s'y connecte.
 * @param hostname est le nom du serveur.
 * @param port est le numéro du port sur lequel se connecter depuis le client.
 * @param sock est la socket permettant de communiquer avec le serveur.
 * @param addr est l'adresse du serveur.
 * @param addrlen est la taille de l'adresse.
 * @return -1 si le serveur n'a pas été trouver, -2 si la socket ne s'est pas crée, 0 si tout c'est bien passé.
 */
int get_server_addr(char* hostname, char* port, int * sock, struct sockaddr_in6** addr, int* addrlen) {
    struct addrinfo hints;
    struct addrinfo *r;
    struct addrinfo *p;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_V4MAPPED | AI_ALL;

    if ((ret = getaddrinfo(hostname, port, &hints, &r)) != 0 || r == NULL){
        fprintf(stderr, "erreur getaddrinfo : %s\n", gai_strerror(ret));
        return -1;
    }

    *addrlen = sizeof(struct sockaddr_in6);
    p = r;
    while( p != NULL ){
        if((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) > 0){
            if(connect(*sock, p->ai_addr, *addrlen) == 0)
	        break;
      
            close(*sock);
        }

        p = p->ai_next;
    }

    if (p == NULL) return -2;

    //on stocke l'adresse de connexion
    *addr = (struct sockaddr_in6 *) p->ai_addr;

    //on libère la mémoire allouée par getaddrinfo 
    freeaddrinfo(r);
    
    return 0;
}

/**
 * @brief permet de communiquer avec le serveur via la socket fdsock
 * @param fdsock est la socket associé au client.
 * @return 0 si tout c'est bien passé et une erreur si un message ne s'est pas envoyé ou n'a pas été reçu.
 */
int communication(int fdsock) {
    /* ICI Le buffer devra contenir le contenu du message. TODO */
    const char* buf;
    const char* data = "Test";
    // sprintf(data, "%s", "Test");
    buf = make_buf(SIGNUP, 0, 0, 0, data, 4);
    // buf = make_insc_buf(SIGNUP, 1, "Pseudo1", 7);
    int ecrit = 1;

    while(ecrit > 0) {

        /* Envoie le message mis dans buf au serveur. */
        /* Il faudra ici demander quelle action l'utilisateur souhaite faire et quel message il souhaite envoyé au serveur. */
        int ecrit = send(fdsock, (char*) buf, sizeof(buf), 0);
        if(ecrit <= 0){
            perror("erreur ecriture");
            exit(3);
        }
        printf("Envoyer : %s\n", buf);
        //memset(buf, 0, SIZE_MESS);

        /* Reception du message renvoyer par le serveur. */
        int recu = recv(fdsock, (char*) buf, SIZE_MESS, 0);
        if (recu <= 0){
            perror("erreur lecture");
            exit(4);
        }
        /* Traitement du message par une autre fonction. */
        printf("Reçu : %s\n", buf);

        /* Les messages de taille supérieur à 8 octets ne sont pas affichés. */
    }
    
    return 0;
}

int main(int argc, char** args) {
    
    /*if (argc < 3) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", args[0]);
        exit(1);
    }*/

    struct sockaddr_in6* server_addr;
    int fdsock, adrlen;
    
    // switch (get_server_addr(args[1], args[2], &fdsock, &server_addr, &adrlen)) {
    switch (get_server_addr(ADRESSE, PORT, &fdsock, &server_addr, &adrlen)) {
        case 0: printf("adresse creee !\n"); break;
        case -1:
            fprintf(stderr, "Erreur: hote non trouve.\n"); 
        case -2:
            fprintf(stderr, "Erreur: echec de creation de la socket.\n");
            exit(1);
    }

    /* Demande de l'action */
    /* Reception de l'action */
    /* Construction de la structure général du message à envoyer au serveur */
    /* Demande du contenu de chaque composante du message */
    /* Envoie du message au serveur */
    /* Execution de l'action par le serveur */
    /* Reception du message envoyé par le serveur par le client (ex: s'il est abonné à un fil) */

    request_code_t action;
    while((action = get_req()) >= 0 && action <= 5) {
        switch (action) {
            case SIGNUP :
                /* Demande des informations pour remplir le buffer à l'utilisateur (depuis ui) */
                /* Remplissage du buffer (message) */
                /* Envoie au serveur */
                break;
            case POST :
                /* Idem */
            case FETCH :
                /* Idem */
            case SUBSCRIBE :
                /* Idem */
            case UPLOAD :
                /* Idem */
            case DOWNLOAD :
                /* Idem */
                // get_all();
                // make_buf();
                break;
            default:
                break;
        }
    }  

    communication(fdsock);

    close(fdsock);
    
    return 0;
}
