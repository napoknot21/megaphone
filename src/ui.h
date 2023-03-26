#ifndef UI_H
#define UI_H

#include "protocol.h"
#include <stddef.h>

/**
 * @return le numéro de la requête demandée par l'utilisateur.
 */
request_code_t get_req();

/**
 * @return l'identifiant de l'utilisateur qui a envoyé le message (vérifier d'abord le mdp avant d'envoyer)
 */
uuid_t get_id();

uint16_t * get_fields();

/**
 * @return le contenue du message envoyé par l'utilisateur.
 */
const char * get_data();

#endif