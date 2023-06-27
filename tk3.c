#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "tk3.h"
#include "skinny.h"

// fixed parameters used in the f_function
const unsigned char mess1[] = {0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00};
const unsigned char mess2[] = {0x01, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,};
const unsigned char mess3[] = {0x02, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,};

/**
 * @brief Initialize the context holding `state` and
 *          `num_xored_bytes`
 *
 * @param ctx [in] \ref the context instance
 */
void init(context *ctx) {
    // set up the initial State
    memset(ctx->state, 0, 48);
    ctx->state[16] = 0x80;
    ctx->num_xored_bytes = 0;
}

/**
 * @brief output is 48-byte of three separate skinny encryption
 *          with the key 'tk' is XORed message block
 * 
 * @param tk \ref tweakey is also the state needed for updating
 */
void f_function(unsigned char *tk){
    unsigned char new_state[48]; 
    skinny(new_state, mess1, tk);
    skinny(new_state + 16, mess2, tk);
    skinny(new_state + 32, mess3, tk);
    memcpy(tk, new_state, 48);
}

/**
 * @brief update `state` in the context.
 * 
 *  
 * @param a [in] \ref the digested message
 * @param ctx [in] \ref the context instance 
 * 
 */
void update(const unsigned char *a, int len, context *ctx) {
    unsigned char f_func_input[48];

    for (unsigned int i = 0; i < len; i++) {
        ctx->state[ctx->num_xored_bytes++] ^= a[i];
        if (ctx->num_xored_bytes == 16) {
            // enough bytes to do update via f_function
            f_function(ctx->state);
            ctx->num_xored_bytes = 0;
        }
    }
}

/** 
 * @brief final output of the TK3 hash function
 * 
 * @param a [in] \ref the output holder
 * @param ctx [in] \ref the context instance
 * 
 */
void finalize(unsigned char *a, context *ctx) {
    ctx->state[ctx->num_xored_bytes] ^= 0x80;
    f_function(ctx->state);
    // Squeezing phase
    // H0
    memcpy(a, ctx->state, 16);

    // H1
    skinny(a + 16, mess1, ctx->state);
}

/**
 * @brief Print bytes value for debugging
 * 
 * @param bytes [in] \ref the array of bytes needs to be printed
 * @param len [in] the lenght of the array
 */
void printBytes(unsigned char *bytes, int len){
    for (int i = 0; i < len; i++)
        printf("%02X%s", bytes[i], (i == len - 1) ? "\n" : " ");
}
