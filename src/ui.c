#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "forge.h"
#include "ui.h"

/*struct header
{
    request_code_t code;
    uuid_t id;

    uint16_t * fields;
    size_t size;
};

struct packet
{
    struct header header;
    
    const char * data;
    size_t size;
};*/

request_code_t get_req() {
    return 0;
}

uuid_t get_id() {
    return 1;
}

/* J'ai pas trop compris ce que contenais le champ field ? */
uint16_t * get_fields() {
    // return /* ... */
    return 0;
}

const char * get_data() {
    return "Data_test";
}