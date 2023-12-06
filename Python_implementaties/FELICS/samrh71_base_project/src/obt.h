#ifndef INC_OBT_H
#define INC_OBT_H

typedef struct {
    unsigned int timr;
    unsigned int calr;
    unsigned int msr;
} Obt_t;

void set_obt(Obt_t *obt);
void get_obt(Obt_t *obt);
void print_obt(Obt_t *obt);
void calib_obt(Obt_t *obt, Obt_t *ref_obt);
#endif
