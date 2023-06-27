/**
 * This is your SKINNY-tk3-Hash instance context, i.e., hash state.
 * You could put the following stuff inside here:
 * - chaining value
 * - chaining value offsets
 * - buffer offsets
 * - leprechaun gold
 * - ...
 */
typedef struct {
/* TODO */
    unsigned char state[48]; // 384 initial bits | 48 initial bytes
    unsigned int num_xored_bytes;
} context;

void f_function(unsigned char *tk);
void printBytes(unsigned char *bytes, int len);

/**
 * Implement the following API.
 * You can add your own functions above, but don't modify below this line.
 */

/* Initialize fresh context ctx. */
void init(context *ctx);

/* Under context ctx, incrementally process len bytes at a. */
void update(const unsigned char *a, int len, context *ctx);

/* Under context ctx, finalize the hash and place the digest at a. */
void finalize(unsigned char *a, context *ctx);
