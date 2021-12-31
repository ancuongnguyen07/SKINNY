
// void printS(unsigned char *b); 
void maskingBit(unsigned char *bi, unsigned char *by, unsigned int n);
unsigned int power2(unsigned int b);
void subCells(unsigned char *is);
void addConstants(unsigned char *s, unsigned int round);
void addRoundTweakey(unsigned char *s, unsigned char *tk);
void swap(unsigned char *a, unsigned char *b);
void permutation(unsigned char *tk, const unsigned int *P);
void LFSR_TK2(unsigned char *tk);
void LFSR_TK3(unsigned char *tk);
void shiftRows(unsigned char *s);
void mixColumns(unsigned char *s);
void copyMatrix(unsigned char *source, unsigned char *dest, unsigned int n);
/**
 * Implement the following API.
 * You can add your own functions above, but don't modify below this line.
 */

/**
 * SKINNY-128-384 block cipher encryption.
 * Under 48-byte tweakey at k, encrypt 16-byte plaintext at p and store the 16-byte output at c.
 */
void skinny(unsigned char *c, const unsigned char *p, const unsigned char *k);