#ifndef INC_SIMULATION_H
#define INC_SIMULATION_H

typedef struct {
    int width;
    int height;
    int amplitude;
    int noise_bits;
} sim_spectrum_params_t;

int make_spectrum(int *data, sim_spectrum_params_t params);

#endif
