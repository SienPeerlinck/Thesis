#include "simulation.h"
#include "timer0.h"

int make_spectrum(int *data, sim_spectrum_params_t params) {
    int v, w;
    unsigned int lfsr = 1;
    unsigned int noise_mask = (1 << params.noise_bits) - 1;
    unsigned int a = params.amplitude;

    for (int y=0; y < params.height; y++) {
        for (int x=0; x < params.width; x++) {
            v = 0;
            
            if (x < params.width / 2)
                v = x << a;
            else
                v = (params.width - x) << a;
            
            v += (200 << a);
            switch (x % 30) {
                case 0:
                case 4:
                    v = (v >> 1);
                    break;
                case 1:
                case 3:
                    v = (v >> 2);
                    break;
                case 2:
                    v = (v >> 3);
                    break;
                default:
                    break;
            }
            
            if (y < params.height / 2)
                v += (y << a);
            else
                v += ((params.height - y) << a);
            
            if (lfsr & 1)
                lfsr = (lfsr >> 1) ^ 0xb4bcd35c;
            else
                lfsr >>= 1;
            v += (lfsr ^ get_timer0_cnt()) & noise_mask;

            *data++ = v;
        }
    }
    return params.width * params.height;
}