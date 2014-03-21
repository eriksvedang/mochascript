#ifndef mocha_list_h
#define mocha_list_h

struct mocha_object;

typedef struct mocha_list {
} mocha_list;

void mocha_list_init(mocha_list* self, struct mocha_object* args[], int count);

#endif
