#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (true);

#define usage(msg)                                                   \
  do {                                                               \
    fprintf(stderr, "Invalid input: %s\nSYNOPSIS: ./intmul\n", msg); \
    exit(EXIT_FAILURE);                                              \
  } while (true);

typedef struct {
  char* a;
  char* b;
  size_t len;
} HexStringPair;

typedef struct {
  char* aH;
  char* aL;
  char* bH;
  char* bL;
  size_t len;
} HexStringQuad;

static void addChars(char** strp, size_t num, char c, bool addToStart) {
  // if !addToStart, then it will add it to the end of strp
  // pre-condition: str must be allocated dynamically
  // side-effect: adds leading zeroes to str and reallocates it

  size_t oldSize = strlen(*strp) + 1;
  char* oldCopy = strdup(*strp);
  if (oldCopy == NULL) {
    error("strdup");
  }
  char* rStr = realloc(*strp, (oldSize + num) * sizeof(char));
  if (rStr == NULL) {
    error("realloc");
  }

  if (addToStart) {
    memset(rStr, c, num);
    memcpy((rStr + num), oldCopy, oldSize);
  } else {
    memcpy(rStr, oldCopy, oldSize - 1);
    memset((rStr + oldSize - 1), c, num);
    rStr[oldSize - 1 + num] = '\0';
  }

  free(oldCopy);
  *strp = rStr;
}

static HexStringPair getInput(void) {
  // post-condition: free returned pair

  HexStringPair pair;
  char* line = NULL;
  size_t lineLen = 0;
  ssize_t nChars;
  int nLines = 0;
  while ((nChars = getline(&line, &lineLen, stdin)) != -1) {
    if (nLines == 0) {
      line[strlen(line) - 1] = '\0';
      pair.a = strdup(line);
    } else if (nLines == 1) {
      line[strlen(line) - 1] = '\0';
      pair.b = strdup(line);
    }
    nLines++;
  }
  free(line);

  // validate input
  if (nLines == 1) {
    free(pair.a);
    usage("too few arguments");
  }
  if (nLines > 2) {
    free(pair.a);
    free(pair.b);
    usage("too many arguments");
  }
  if ((strlen(pair.a) == 0) || (strlen(pair.b) == 0)) {
    free(pair.a);
    free(pair.b);
    usage("empty argument");
  }
  const char* valid = "0123456789abcdefABCDEF";
  if ((strspn(pair.a, valid) != strlen(pair.a)) ||
      (strspn(pair.b, valid) != strlen(pair.b))) {
    free(pair.a);
    free(pair.b);
    usage("one argument is not a hexadecimal number")
  }

  // get len to the same in pair
  const size_t len1 = strlen(pair.a);
  const size_t len2 = strlen(pair.b);
  const size_t diff = (size_t)labs((long)(len2 - len1));
  if (len1 < len2) {
    addChars(&(pair.a), diff, '0', true);
  } else if (len1 > len2) {
    addChars(&(pair.b), diff, '0', true);
  }
  assert(strlen(pair.a) == strlen(pair.b));

  // get len to be a power of 2
  const size_t len = strlen(pair.a);
  if ((len & (len - 1)) != 0) {
    const size_t newLen = 1 << (size_t)ceill(log2l((long double)len));
    addChars(&(pair.a), newLen - len, '0', true);
    addChars(&(pair.b), newLen - len, '0', true);
  }

  pair.len = strlen(pair.a);
  return pair;
}

static HexStringQuad splitToQuad(HexStringPair pair) {
  // also adds a '\n' at the end so content can be directly sent to children
  // post-condition: free returned quad

  HexStringQuad quad;
  size_t size = (pair.len / 2) + 1;

  quad.aH = malloc(size * sizeof(char));
  quad.bH = malloc(size * sizeof(char));
  quad.aL = malloc(size * sizeof(char));
  quad.bL = malloc(size * sizeof(char));

  memcpy(quad.aH, pair.a, size - 1);  // high digits (left side)
  memcpy(quad.bH, pair.b, size - 1);
  memcpy(quad.aL, (pair.a + size - 1), size - 1);  // low digits (right side)
  memcpy(quad.bL, (pair.b + size - 1), size - 1);

  quad.aH[size - 1] = '\0';
  quad.bH[size - 1] = '\0';
  quad.aL[size - 1] = '\0';
  quad.bL[size - 1] = '\0';
  quad.len = size - 1;

  addChars(&(quad.aH), 1, '\n', false);
  addChars(&(quad.aL), 1, '\n', false);
  addChars(&(quad.bH), 1, '\n', false);
  addChars(&(quad.bL), 1, '\n', false);
  quad.len++;

  return quad;
}

static char* addHexStrings(char str1[], char str2[]) {
  // post-condition: output must be freed

  // remove leading zeroes
  while (*str1 == '0') {
    str1++;
  }
  while (*str2 == '0') {
    str2++;
  }

  // add bit wise
  const size_t maxLen =
      (strlen(str1) > strlen(str2) ? strlen(str1) : strlen(str2));
  size_t indexStr1 = strlen(str1) - 1;
  size_t indexStr2 = strlen(str2) - 1;

  char reversedOutput[maxLen + 2];  // +2 for carry and '\0'
  unsigned long carry = 0;
  char carryStr[2] = {'\0', '\0'};
  size_t i;
  for (i = 0; i < maxLen; i++) {
    char tmp1[2] = {'0', '\0'};
    char tmp2[2] = {'0', '\0'};
    if (indexStr1 != (size_t)-1) {
      tmp1[0] = str1[indexStr1--];
    }
    if (indexStr2 != (size_t)-1) {
      tmp2[0] = str2[indexStr2--];
    }

    errno = 0;
    unsigned long sum =
        carry + strtoul(tmp1, NULL, 16) + strtoul(tmp2, NULL, 16);
    if (errno != 0) {
      error("strtoul");
    }

    unsigned long write = sum % 16;
    carry = sum / 16;

    char writeStr[2];
    assert(write <= 15);
    sprintf(writeStr, "%lx", write);

    assert(carry <= 15);
    sprintf(carryStr, "%lx", carry);

    reversedOutput[i] = writeStr[0];
  }
  if (carry == 0) {
    reversedOutput[i] = '\0';
  } else {
    reversedOutput[i] = carryStr[0];
    reversedOutput[i + 1] = '\0';
  }

  // reverse
  const size_t len = strlen(reversedOutput);
  char* output = malloc((len + 1) * sizeof(char));
  // alternatively: char output[len + 1]; -> then write into pointer
  size_t j;
  for (j = 0; j < len; j++) {
    output[j] = reversedOutput[len - 1 - j];
  }
  output[j] = '\0';
  return output;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }

  // base case
  HexStringPair pair = getInput();
  if (pair.len == 1) {
    errno = 0;
    fprintf(stdout, "%lx\n",
            strtoul(pair.a, NULL, 16) * strtoul(pair.b, NULL, 16));
    if (errno != 0) {
      error("strtoul");
    }
    free(pair.a);
    free(pair.b);
    exit(EXIT_SUCCESS);
  }

  // general case
  enum child_index { aH_bH = 0, aH_bL = 1, aL_bH = 2, aL_bL = 3 };
  enum pipe_end { READ_END = 0, WRITE_END = 1 };

  int parent2child[4][2];
  int child2parent[4][2];
  for (int i = 0; i < 4; i++) {
    if ((pipe(parent2child[i]) == -1) || (pipe(child2parent[i]) == -1)) {
      error("pipe");
    }
  }

  pid_t cpid[4];
  for (int i = 0; i < 4; i++) {
    cpid[i] = fork();
    if (cpid[i] == -1) {
      error("fork");
    }
    if (cpid[i] == 0) {
      // child redirects stdin and stdout (fork duplicates pipes for child)
      if ((dup2(parent2child[i][READ_END], STDIN_FILENO) == -1) ||
          (dup2(child2parent[i][WRITE_END], STDOUT_FILENO) == -1)) {
        error("dup2");
      }
      if ((close(parent2child[i][READ_END]) == -1) ||
          (close(parent2child[i][WRITE_END]) == -1) ||
          (close(child2parent[i][READ_END]) == -1) ||
          (close(child2parent[i][WRITE_END]) == -1)) {
        error("close");
      }

      // child runs intmul (waits for arguments in stdin)
      execlp("./intmul", "./intmul", NULL);
      error("execlp");
    }
  }

  // close fds
  for (int i = 0; i < 4; i++) {
    if ((close(parent2child[i][READ_END]) == -1) ||
        (close(child2parent[i][WRITE_END]) == -1)) {
      error("close");
    }
  }

  // write into parent2child
  HexStringQuad quad = splitToQuad(pair);
  free(pair.a);
  free(pair.b);

  fprintf(stderr, "arrived here lol\n");
  // gdb shows:
  //    - child starts listening to stdin but doesn't receive anything
  //    - parent stops and waits for child to return something
  //
  // this means:
  //    - writing doesn't work

  for (int i = 0; i < 4; i++) {
    char* arg1;
    char* arg2;
    switch (i) {
      case aH_bH:
        arg1 = quad.aH;
        arg2 = quad.bH;
        break;
      case aH_bL:
        arg1 = quad.aH;
        arg2 = quad.bL;
        break;
      case aL_bH:
        arg1 = quad.aL;
        arg2 = quad.bH;
        break;
      case aL_bL:
        arg1 = quad.aL;
        arg2 = quad.bL;
        break;
    }

    FILE* stream = fdopen(parent2child[i][WRITE_END], "w");
    if (stream == NULL) {
      error("fdopen");
    }
    /*
    if ((fputs(arg1, stream) == -1) || (fputs(arg2, stream) == -1)) {
      error("fputs");
    }
    */
    fprintf(stream, "%.*s", quad.len, arg1);
    fprintf(stream, "%.*s", quad.len, arg2);
    if (fclose(stream) == -1) {
      error("fclose");
    }

    /*
    if (((write(parent2child[i][WRITE_END], arg1, quad.len) == -1) ||
         (write(parent2child[i][WRITE_END], arg2, quad.len) == -1)) &&
        (errno != EINTR)) {
      error("write");
    }
    if (close(parent2child[i][WRITE_END]) == -1) {  // child sees EOF
      error("close here")
    }
    */
  }
  free(quad.aH);
  free(quad.aL);
  free(quad.bH);
  free(quad.bL);

  // wait for child to exit
  int status[4];
  for (int i = 0; i < 4; i++) {
    fprintf(stderr, "processing index %d\n", i);
    if (waitpid(cpid[i], &status[i], 0) == -1) {
      error("waitpid");
    }
    if (WEXITSTATUS(status[i]) == EXIT_FAILURE) {
      error("child failed");
    }
  }

  // read child2parent
  char* childResult[4];
  for (int i = 0; i < 4; i++) {
    size_t len;
    FILE* stream = fdopen(child2parent[i][READ_END], "r");
    if (stream == NULL) {
      error("fdopen");
    }
    if (getline(&childResult[i], &len, stream) < 0) {
      error("getline");
    }
    addChars(&childResult[i], 1, '\0', false);
    if (fclose(stream) == -1) {
      error("fclose");
    }
    /*
    if (close(child2parent[i][READ_END]) == -1) {
      error("close");
    }
    */
  }

  // print total sum
  const size_t n = pair.len;
  addChars(&childResult[aH_bH], n, '0', false);
  addChars(&childResult[aH_bL], n / 2, '0', false);
  addChars(&childResult[aL_bH], n / 2, '0', false);

  char* fstSum = addHexStrings(childResult[aH_bH], childResult[aH_bL]);
  free(childResult[aH_bH]);
  free(childResult[aH_bL]);

  char* sndSum = addHexStrings(childResult[aL_bH], childResult[aL_bL]);
  free(childResult[aL_bH]);
  free(childResult[aL_bL]);

  char* totalSum = addHexStrings(fstSum, sndSum);
  free(fstSum);
  free(sndSum);

  fprintf(stdout, "%s\n", totalSum);
  fflush(stdout);
  free(totalSum);

  exit(EXIT_SUCCESS);
}