#ifndef mocha_vector_h
#define mocha_vector_h

struct mocha_object;

typedef struct mocha_vector {
} mocha_vector;

void mocha_vector_init(mocha_vector* self, struct mocha_object* args[], int count);

#endif
