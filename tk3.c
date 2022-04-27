#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tk3.h"
#include "skinny.h"

/* TODO */
void init(context *ctx) {
    unsigned char temp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    copyMatrix(temp, ctx->status, 48);
    ctx->max_size_message = 256;
    ctx->current_size_message = 0;
    ctx->size_padded_message = 0;
    ctx->message = (unsigned char*) malloc(ctx->max_size_message*sizeof(unsigned char));
}

/**
 * @brief output is 48-byte fo three separate skinny encryption
 *          with the key 'tk' is XORed message block
 * 
 * @param tk 
 * @param output 
 */
void f_function(const unsigned char *tk, unsigned char *output){
    unsigned char output_part1[16];
    unsigned char output_part2[16];
    unsigned char output_part3[16];

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

    skinny(output_part1, mess1, tk);
    skinny(output_part2, mess2, tk);
    skinny(output_part3, mess3, tk);

    for (int i = 0; i < 48; i++){
        if (i < 16){
            output[i] = output_part1[i];
        }
        else if (i < 32){
            output[i] = output_part2[i - 16];
        }
        else{
            output[i] = output_part3[i - 32];
        }
    }
}

/**
 * @brief return a mod b, assuming b is always positive
 * 
 * @param a 
 * @param b 
 * @return int 
 */
int modulo(int a, int b){
    while (a < 0)
    {
        a += b;
    }
    return a % b;
}

/* TODO */
void update(const unsigned char *a, int len, context *ctx) {
    if (ctx->current_size_message + len > ctx->max_size_message){
        int new_size = ctx->current_size_message + len;
        ctx->message = realloc(ctx->message, new_size);
        ctx->max_size_message = new_size;
    }
    else{
        copyMatrix(a, ctx->message + ctx->current_size_message, len);
        ctx->current_size_message += len;
    }
}

/**
 * @brief updating state of context as the output of f_function(S XOR (M||0^256))
 * 
 * @param padded_mess 
 * @param ctx 
 */
void update_state(unsigned char *padded_mess, context *ctx){
    // updating state in context
    for (int index = 0; index <= ctx->size_padded_message - 16; index += 16){
        // concatenate block message with 0^256
        unsigned char xoredMess[48];
        for (int i = 0; i < 48; i++){
            // printf("xxxxx\n");
            if (i < 16){
                xoredMess[i] = padded_mess[index + i];
            }
            else{
                xoredMess[i] = 0x00;
            }
            // XOR with current state of context
            xoredMess[i] ^= ctx->status[i];
        }

        // Doing f_function
        f_function(xoredMess, ctx->status);
    }
}

/**
 * @brief Padding the message M||10^(-1-|M| mod 128)
 * 
 * @param padded_mess 
 * @param ctx 
 */
void pad10(unsigned char *padded_mess, context *ctx){
    unsigned int len = ctx->current_size_message;
    for (int i = 0; i < ctx->size_padded_message; i++){
        if (i < len){
            padded_mess[i] = *(ctx->message+i);
        }
        else if (i == len){
            padded_mess[i] = 0x80;
        }
        else{
            padded_mess[i] = 0x00;
        }
    }
}

/* TODO */
void finalize(unsigned char *a, context *ctx) {

    // Absorbing phase
    // pad10*
    // calculating size of padded message
    unsigned int len = ctx->current_size_message;
    int extended_len = modulo(-1 - len*8, 128); // len*8 because of considering bits not bytes
    ctx->size_padded_message = len + (extended_len + 1) / 8;
    unsigned char padded_mess[ctx->size_padded_message];

    // padding
    pad10(padded_mess, ctx);

    // updating state through each block of padded message
    update_state(padded_mess, ctx);

    // Squeezing phase
    unsigned char H0[16];
    unsigned char H1[16];

    // truncating the first 128-bit output f_function of 384-bit state
    copyMatrix(ctx->status, H0, 16);

    // Doing f_function with tk is current state of context
    unsigned char tk[48];
    copyMatrix(ctx->status, tk, 48); 
    f_function(tk, ctx->status);

    // truncating the first 128-bit output f_function of 384-bit state
    copyMatrix(ctx->status, H1, 16);

    // concatenate to produce final result
    copyMatrix(H0, a, 16);
    copyMatrix(H1, a+16, 16);
    
    // free up message of context
    free(ctx->message);
}

/**
 * @brief Print bytes value for debugging
 * 
 * @param bytes 
 * @param len 
 */
void printBytes(unsigned char *bytes, int len){
    for (int i = 0; i < len; i++)
        printf("%02X%s", bytes[i], (i == len - 1) ? "\n" : " ");
}
