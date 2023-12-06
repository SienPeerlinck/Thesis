#include "felics.h"
#include "FreeRTOS.h"

inline static unsigned int bit_length(unsigned int x) {
    return 32 - __builtin_clz(x);
}

// Bit stream ----------------------------------------------------------------
static BitStream make_bitstream(char* buf, unsigned int n_bytes) {
    BitStream bs;
    bs.bytes = buf;
    bs.n_bytes = n_bytes;
    bs.byte_cntr = 0;
    bs.bits = 0;
    bs.bit_cntr = 0;
    return bs;
}

static void reset_bytes(BitStream *bs) {
    bs->byte_cntr = 0;
}

inline static void push_bits(BitStream *bs, unsigned int v, unsigned int n) {
    unsigned int mask;
    unsigned char byte;

    bs->bits = (bs->bits << n) | v;
    bs->bit_cntr += n;
    while (bs->bit_cntr > 7) {
        bs->bit_cntr -= 8;
        mask = 0xff << bs->bit_cntr;
        byte = (bs->bits & mask) >> bs->bit_cntr;
        bs->bytes[bs->byte_cntr++] = byte;
    }
}

static void flush_bits(BitStream *bs) {
    unsigned char byte;
    if (bs->bit_cntr > 0) {
        byte = bs->bits << (8 - bs->bit_cntr);
        bs->bytes[bs->byte_cntr++] = byte;
        bs->bit_cntr = 0;
    }
}


// Adjusted binary code ------------------------------------------------------
inline static void abc_push_bits(BitStream *bs, unsigned int x, unsigned int r) {
    unsigned int n, a, b, half_b;

    n = bit_length(r);
    a = (1 << n) - r -1;
    b = (1 << n) - (a << 1);
    half_b = b >> 1;
    //printf("[r=%d clz=%d n=%d a=%d b=%d] ", r, __builtin_clz(r), n, a, b);
    if (x < half_b)
        push_bits(bs, (a + x) << 1, n);
    else {
        x -= half_b;
        if (x < a)
            push_bits(bs, x, n-1);
        else
            push_bits(bs, x << 1 | 1, n);
    }
}

// Subexponential code -------------------------------------------------------
#define K_BITS 24
#define DELTA_BITS 24
static unsigned k_table[K_BITS][DELTA_BITS];
static unsigned best_ks[DELTA_BITS];

inline static void reset_sec_tables(void) {
    for (int i = 0; i < K_BITS; i++)
        for (int j = 0; j < DELTA_BITS; j++)
            k_table[i][j] = 0;
    for (int j = 0; j < DELTA_BITS; j++)
        best_ks[j] = 0;
}

inline static void sec_push_bits(BitStream *bs, unsigned int x, unsigned int k) {
    int b;
    unsigned int u, mask;

    b = bit_length(x) - 1;
    if (b < (int)k) {
        b = k;
        u = 0;
    } else
        u = b - k + 1;
    //printf("[b=%d u=%d]", b, u);

    // Unary code of u
    push_bits(bs, (1 << u) - 1, u);
    push_bits(bs, 0, 1);

    // b least significant bits of x
    mask = (1 << b) - 1;
    push_bits(bs, x & mask, b);
}

static unsigned int estimate_k_for_subexp(unsigned int delta, unsigned int x) {
    unsigned int delta_bits, best_k, k;
    delta_bits = bit_length(delta);
    k = best_ks[delta_bits];

    // Update tables
    best_k = bit_length(x);
    k_table[best_k][delta_bits]++;
    if (k_table[best_k][delta_bits] > k_table[k][delta_bits])
        best_ks[delta_bits] = best_k;

    return k;
}

// Felics --------------------------------------------------------------------
FelicsHandler felics_init(char* buf, unsigned int n_bytes) {
    FelicsHandler felics_h;
    reset_sec_tables();
    felics_h.bs = make_bitstream(buf, n_bytes);
    return felics_h;
}

unsigned int felics_close(FelicsHandler* felics_h) {
    BitStream* bs = &felics_h->bs;
    flush_bits(bs);
    return bs->byte_cntr;
}

void felics_encode_header(FelicsHandler* felics_h, unsigned int w, unsigned int h) {
    BitStream* bs = &felics_h->bs;
    push_bits(bs, FELICS_VERSION, FELICS_VERSION_BITS);
    push_bits(bs, felics_h->delta_min, DELTA_MIN_BITS);
    push_bits(bs, w, IMG_WIDTH_BITS);
    push_bits(bs, h, IMG_HEIGHT_BITS);
}

static char felics_push_pixel(FelicsHandler* felics_h, unsigned int p, unsigned int n1,
                       unsigned int n2) {
    unsigned int h, l, delta, e, k;
    BitStream* bs = &felics_h->bs;
    unsigned int delta_min = felics_h->delta_min;

    if (n1 > n2) {
        h = n1;
        l = n2;
    } else {
        h = n2;
        l = n1;
    }
    delta = h - l;

    // Fix delta ===================================
    if (delta < delta_min) {
        if (l < delta_min / 2)
            l = 0;
        else
            l -= delta_min / 2;
        h =  l + delta_min;
        delta = delta_min;
    }
    //================================================
        
    if (l <= p && p <= h) {
        // In range
        push_bits(bs, 0, 1);
        if (delta != 0)
            abc_push_bits(bs, p - l, delta);
    } else {
        // Out of range
        push_bits(bs, 1, 1);
        if (p < l) {
            // Below range
            push_bits(bs, 0, 1);
            e = l - p - 1;
        } else {
            // Above range
            push_bits(bs, 1, 1);
            e = p - h - 1;
        }
        k = estimate_k_for_subexp(delta, e);
        sec_push_bits(bs, e, k);
    }
    n2 = n1;
    n1 = p;

    if (bs->byte_cntr < bs->n_bytes - 10)
        return FELICS_NO_ERROR;
    else
        return FELICS_BUF_OVERFLOW_ERROR;
}

char felics_encode_row0(FelicsHandler* felics_h, unsigned int *row, unsigned int n) {
    unsigned int n1, n2, i, p, h, l, delta, e, k;
    char error;
    n2 = *row++;
    n1 = *row++;
    unsigned int bits_per_pixel;
    BitStream* bs = &felics_h->bs;

    if (bit_length(n1) > bit_length(n2))
        bits_per_pixel = bit_length(n1);
    else
        bits_per_pixel = bit_length(n2);
    
    push_bits(bs, bits_per_pixel, BITS_PER_PIXEL_SIZE);
    push_bits(bs, n2, bits_per_pixel);
    push_bits(bs, n1, bits_per_pixel);

    for (i=2; i<n; i++, row++) {
        p = *row;
        error = felics_push_pixel(felics_h, p, n1, n2);
        if (error) return error;
        n2 = n1;
        n1 = p;
    }
    return FELICS_NO_ERROR;
}

char felics_encode_row(FelicsHandler* felics_h, unsigned int *row, unsigned int *row0,
                       unsigned int n) {
    BitStream* bs = &felics_h->bs;
    unsigned int n1, n2, i, p, h, l, delta, e, k;
    char error;
    n2 = *row0++;
    n1 = *row0;

    for (i=0; i<n; i++, row++) {
        p = *row;
        error = felics_push_pixel(felics_h, p, n1, n2);
        if (error) return error;
        n2 = *row0++;
        n1 = p;
    }
    return FELICS_NO_ERROR;
}
