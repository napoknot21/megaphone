#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stddef.h>

#define LAYER_TCP 0
#define LAYER_UDP 1

#define REQUEST_CODE_SIZE_BITS 5
#define USER_ID_SIZE_BITS 11
#define FIELD_SIZE 2

#define REQUEST_CODE_MASK 0xF800
#define USER_ID_MASK 0x3FF

#include <stdint.h>

typedef enum request_code
{
    SIGNUP = 0,
    POST,
    FETCH,
    SUBSCRIBE,
    UPLOAD,
    DOWNLOAD
} request_code_t;

typedef uint16_t uuid_t;

/**
 * @brief renvoie le message à envoyer au serveur.
 * @param code est le nom de la requête.
 * @param id est l'id de l'utilisateur (à renvoyer).
 * @param fields est le contenue de l'en-tête.
 * @param size_h est la taille de l'en-tête.
 * @param data comporte les données composant le message à envoyer (son contenu).
 * @param size_p est la taille du packet à envoyer.
 * @return un buffer contenant un message à envoyer au serveur.
 */
const char* make_buf(request_code_t code, uuid_t id, uint16_t * fields, size_t size_h, const char * data, size_t size_p);

/**
 * @brief récupère le buffer d'inscription à envoyer au serveur.
 * @param code est le nom de la requête 
 * @param id est l'id de l'utilisateur (à renvoyer).
 * @param fields fields est le pseudo de l'utilisateur.
 * @param size_h est la taille du pseudo.
 * @return un buffer contenant le message d'inscription à envoyer au serveur.
 */
const char* make_insc_buf(request_code_t code, uuid_t id, uint16_t * fields, size_t size_h);

#endif