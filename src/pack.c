#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>

char QUOTE_CHAR = '#';

char *HEX_ALPHA = "0123456789abcdef";

char *SAFE_ALPHA = \
"abcdefghijklmnopqrstuvwxyz\
ABCDEFGHIJKLMNOPQRSTUVWXYZ\
0123456789\
_-+=";


#define ptr_range(a, b) ( (a == NULL || b == NULL) ? -1 : b - a )
#define hex_index(c) ( ptr_range(HEX_ALPHA, strchr(HEX_ALPHA, c)) )

#define forward(str_skip, out_skip)                 \
  do {                                              \
    str     += str_skip;                            \
    str_len -= str_skip;                            \
    out     += out_skip;                            \
    out_len -= out_skip;                            \
  } while(0)

#define guard(cond)                               \
  if (!( cond )) {                                \
    printf("failed: %d\n", __LINE__);             \
    return -1;                                    \
  }

int safe_pack(char *str, char *out, size_t out_len) {
  // escape unsafe chars (strlen(str) <= strlen(out))
  int str_len = strlen(str);

  while (1) {

    // find next unsafe char
    int skip = strspn(str, SAFE_ALPHA);
    if (skip > 0) {
      // make sure we have space for the safe chars
      guard(skip < out_len);
      // copy safe chars
      memcpy(out, str, skip);
      // advance
      forward(skip, skip);
    }

    // end of str
    if (*str == 0) {
      break;
    }

    // we are now at an unsafe char: escape it!
    // make sure we have space for a quoted char
    guard(3 < out_len);

    // escape char! (e.g. space becomes: "#20")
    unsigned char ord = (int) *str;
    *out       = QUOTE_CHAR;
    *(out + 1) = HEX_ALPHA[ord / 16];
    *(out + 2) = HEX_ALPHA[ord % 16];

    // advance
    forward(1, 3);
  }

  // make sure we have space for the ending 0 byte
  guard(1 < out_len);

  // write terminating 0-byte
  *out = 0;

  // everything went better than expected!
  return 0;
}

int safe_unpack(char *str, char *out, size_t out_len) {
  // unpack escaped chars (strlen(str) >= strlen(out))
  int str_len = strlen(str);

  while(1) {

    // find next quote char
    char *end = strchr(str, QUOTE_CHAR);
    size_t skip;
    if(end != NULL) {
      // get safe prefix length
      skip = end - str;
    }
    else {
      // no more quotes, rest is safe
      skip = strlen(str);
      end = str + skip;
    }

    if (skip > 0) {
      // make sure we have space to copy
      guard(skip < out_len);

      // copy safe chars
      memcpy(out, str, skip);
      // advance
      forward(skip, skip);
    }

    // end of str
    if (*str == 0) {
      break;
    }

    // make sure there is space for char
    guard((3 <= str_len) && (1 < out_len));

    // decode hex (str[0] == '#')
    int hex0 = hex_index(str[1]);
    int hex1 = hex_index(str[2]);

    // verify hex chars
    guard(hex0 >= 0 && hex1 >= 0);
    // set char
    *out = hex0 * 16 + hex1;

    // advance
    forward(3, 1);
  }

  guard(out_len != 0);

  // set terminating 0-byte
  *out = 0;

  return 0;
}


#define U_SIZE 1000           // max size of unsafe string
#define S_SIZE (U_SIZE * 3)   // max size of safe string

int test() {
  char *unsafe = calloc(1, U_SIZE + 1);
  char *safe   = calloc(1, S_SIZE + 1);

  // fill unsafe with random non-zero shit
  srand(time(NULL) + getpid());
  int i;
  for(i = 0; i < U_SIZE; i++) {
    if (random() % 2 == 0) {
      unsafe[i] = random() % 255 + 1;
    }
    else {
      unsafe[i] = SAFE_ALPHA[random() % strlen(SAFE_ALPHA)];
    }
  }
  unsafe[U_SIZE + 1] = 0;

  if(safe_pack(unsafe, safe, S_SIZE + 1)) {
    return -1;
  }
  printf(">> safe: ok\n");

  if(safe_unpack(safe, unsafe, U_SIZE + 1)) {
    return -1;
  }

  printf(">> unsafe: ok\n");

  free(unsafe);
  free(safe);

  return 0;
}
