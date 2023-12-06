#define FELICS_VERSION 0x01
#define FELICS_VERSION_BITS 8
#define IMG_WIDTH_BITS 12
#define IMG_HEIGHT_BITS 12
#define BITS_PER_PIXEL_SIZE 5
#define DELTA_MIN_BITS 24


#define FELICS_NO_ERROR 0
#define FELICS_BUF_OVERFLOW_ERROR 1

typedef struct BitStream {
    unsigned char *bytes;
    unsigned int n_bytes;
    unsigned int byte_cntr;
    unsigned int bits;
    unsigned int bit_cntr;
} BitStream;

typedef struct {
    BitStream bs;
    unsigned int delta_min;
} FelicsHandler;

FelicsHandler felics_init(char* buf, unsigned int n_bytes);
void felics_encode_header(FelicsHandler* felics_h, unsigned int w, unsigned int h);
char felics_encode_row0(FelicsHandler* felics_h, unsigned int *row, unsigned int n);
char felics_encode_row(FelicsHandler* felics_h, unsigned int *row, unsigned int *row0,
                       unsigned int n);
unsigned int felics_close(FelicsHandler* felics_h);


