#include <stdint.h>
#include <stdio.h>
#include "skinny.h"

// constants of LFSR in AddConstant function
const unsigned char CONSTANTS[] = {
    0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3e, 0x3d, 0x3b, 0x37, 0x2f, 0x1e, 0x3c, 0x39, 0x33, 0x27, 0x0e,
    0x1d, 0x3a, 0x35, 0x2b, 0x16, 0x2c, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0b, 0x17, 0x2e, 0x1c, 0x38,
    0x31, 0x23, 0x06, 0x0d, 0x1b, 0x36, 0x2d, 0x1a, 0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04,
    0x09, 0x13, 0x26, 0x0c, 0x19, 0x32, 0x25, 0x0a};

const unsigned int SUBCELLS_PERMUTATION[] = {2,1,7,6,4,0,3,5};
const unsigned int SUBCELLS_PERMUTATION_LAST[] = {7,6,5,4,3,1,2,0};
const unsigned int ADDROUNDTK_PERMUTATION[] = {9,15,8,13,10,14,12,11,0,1,2,3,4,5,6,7};
const unsigned int SHIFTROWS_PERMUTATION[] = {0,1,2,3,7,4,5,6,10,11,8,9,13,14,15,12};
const unsigned int BINARY_MATRIX[] = {1,0,1,1,1,0,0,0,0,1,1,0,1,0,1,0}; // mixColumns function
const unsigned int ROUND = 56; // total round of encryption
/**
 * SKINNY-128-384 block cipher encryption.
 * Under 48-byte tweakey at k, encrypt 16-byte plaintext at p and store the 16-byte output at c.
 */
void skinny(unsigned char *c, const unsigned char *p, const unsigned char *k) {
    unsigned char IS[16];
    unsigned char TK[48];

    for (unsigned int i = 0; i <  16; i++){
        IS[i] = *(p+i);
    }
    

    for (unsigned int i = 0; i < 48; i++){
        TK[i] = *(k+i);
    }

    // ROUND FUNCTION
    for (unsigned int r = 0; r < ROUND; r++){
        subCells(IS);
        addConstants(IS,r);
        addRoundTweakey(IS,TK);
        shiftRows(IS);
        mixColumns(IS);
    }
    copyMatrix(IS,c,16);
    
}

// void printS(unsigned char *b){
//     for (unsigned int i = 0; i < 16; i++){
//         printf("%02x ",*(b+i));
//     }
//     printf("\n");
// }


void maskingBit(unsigned char *bi, unsigned char *by, unsigned int n){
    // take a N-th bit of the byte
    *bi = (*by >> n) & 0x01;
}

unsigned int power2(unsigned int b){
    unsigned int result = 1;
    for (unsigned int i = 0; i < b; i++){
        result = result * 2;
    }
    
    return result;
}

void subCells(unsigned char *is){
    // total round of subCells is 4
    for (unsigned int sr = 0; sr < 4; sr++){
        for (unsigned int k = 0; k < 16; k++){
            unsigned char *by = is + k;

            unsigned char x,y,z;
            // S-box transformation
            for (unsigned int u = 0; u < 5; u = u + 4){
                
                maskingBit(&x, by, u);
                maskingBit(&y, by, u + 3);
                maskingBit(&z, by, u + 2);
                unsigned char bNew = x ^ ((y | z) ^ 1);

                if (x != bNew){
                    *by = *by ^ power2(u);
                }
            }

            // bit permutation
            unsigned int* order = (unsigned int*)SUBCELLS_PERMUTATION;
            if (sr == 3){
                order = (unsigned int*)SUBCELLS_PERMUTATION_LAST;
            }
            
            unsigned char bi;
            unsigned char permutedByte = 0;
            for (int i = 7, k = 0; i >= 0; i--, k++){
                maskingBit(&bi, by, order[k]);
                
                if (bi == 0x01){
                    permutedByte += power2(i);
                }
            }
            *by = permutedByte;
        }
    }
}

void addConstants(unsigned char *s, unsigned int round){
    unsigned char c0 = CONSTANTS[round] & 0x0f;
    unsigned char c1 = CONSTANTS[round] >> 4;
    unsigned char c2 = 0x02; 

    s[0] ^= c0;
    s[4] ^= c1;
    s[8] ^= c2; 
}

void swap(unsigned char *a, unsigned char *b){
    unsigned char temp = *a;
    *a = *b;
    *b = temp;
}

void permutation(unsigned char *tk, const unsigned int *P){
    for (unsigned int i = 0; i < 16; i++){
        unsigned int indexToSwap = P[i];
        while(indexToSwap < i){
            indexToSwap = P[indexToSwap];
        }
        swap(tk + indexToSwap, tk + i);
    }
}

void LFSR_TK2(unsigned char *tk){
    unsigned char *byte;
    unsigned char bit5, bit7;

    // applying only on the first two rows of TK2
    for (unsigned int row = 0; row < 2; row++){
        for (unsigned int col = 0; col < 4; col++){
            unsigned int pos = 4*row + col;
            byte = tk + pos;
            
            maskingBit(&bit5, byte, 5);
            maskingBit(&bit7, byte, 7);

            *byte = (*byte << 1) | (bit7 ^ bit5);
        }
    }
}

void LFSR_TK3(unsigned char *tk){
    unsigned char *byte;
    unsigned char bit0, bit6;
    
    // applying only on the first two rows of TK3
    for (unsigned int row = 0; row < 2; row++){
        for (unsigned int col = 0; col < 4; col++){
            unsigned int pos = 4*row + col;
            byte = tk + pos;
            
            maskingBit(&bit6, byte, 6);
            maskingBit(&bit0, byte, 0);           

            *byte = (*byte >> 1) | ((bit0 ^ bit6) * power2(7));
        }
    }
}

void addRoundTweakey(unsigned char *s, unsigned char *tk){
    unsigned char *tk1 = tk;
    unsigned char *tk2 = tk + 16;
    unsigned char *tk3 = tk + 32;

    for (unsigned int row = 0; row < 2; row++){
        for (unsigned int col = 0; col < 4; col++){
            unsigned int pos = 4*row + col;
            *(s + pos) ^= *(tk1 + pos) ^ *(tk2 + pos) ^ *(tk3 + pos);
        }
    }

    // permutation applied on all cells of tweakey arrays
    permutation(tk1, ADDROUNDTK_PERMUTATION);
    permutation(tk2, ADDROUNDTK_PERMUTATION);
    permutation(tk3, ADDROUNDTK_PERMUTATION);

    // LFSR on TK2 and TK3
    LFSR_TK2(tk2);
    LFSR_TK3(tk3);
    
}

void shiftRows(unsigned char *s){
    permutation(s, SHIFTROWS_PERMUTATION);
}

void copyMatrix(unsigned char *source, unsigned char *dest, unsigned int n){
    for (unsigned int i = 0; i < n; i++){
        dest[i] = source[i];
    }
}

void mixColumns(unsigned char *s){
    unsigned char copiedStates[16];
    copyMatrix(s,copiedStates,16);
    for (unsigned int rowMatrix = 0; rowMatrix < 4; rowMatrix++){
        for (unsigned int colState = 0; colState < 4; colState++){
            unsigned char newState = 0x00;
            unsigned int updatePos = 4*rowMatrix + colState;

            for (unsigned int ele = 0; ele < 4; ele++){
                unsigned int posMatrix = 4*rowMatrix + ele;
                unsigned int posState = 4*ele + colState;

                newState ^= BINARY_MATRIX[posMatrix] * copiedStates[posState];
            }
            s[updatePos] = newState;
        }
    }
}
