#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define usage(msg)                                                       \
  do {                                                                   \
    fprintf(stderr,                                                      \
            "Wrong usage: %s\nSYNOPSIS:\n\tserver [-p PORT] [-i INDEX] " \
            "DOC_ROOT\nEXAMPLE\n\tserver -p 1280 -i index.html "         \
            "~/Documents/my_website/\n",                                 \
            msg);                                                        \
    exit(EXIT_FAILURE);                                                  \
  } while (0);

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0);

static volatile sig_atomic_t quit = false;
static void onSignal(int sig, siginfo_t* si, void* unused) { quit = true; }
static void initSignalListener(void) {
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = onSignal;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    error("sigaction");
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    error("sigaction");
  }
  if (sigaction(SIGQUIT, &sa, NULL) == -1) {
    error("sigaction");
  }
}

typedef struct {
  char* port;
  char* defaultFileName;
  char* rootPath;
} Arguments;

static void validateArguments(Arguments args) {
  if (args.port != NULL) {
    if (strspn(args.port, "0123456789") != strlen(args.port)) {
      usage("port contains non digit characters");
    }
    errno = 0;
    long portLong = strtol(args.port, NULL, 10);
    if (errno != 0) {
      error("strtoul");
    }
    if ((portLong < 0) || (portLong > 65535)) {
      usage("port not in legal range");
    }
  }

  const char* illegalFileChars = "/\\:*?\"<>|";  // unix is less strict
  if (args.defaultFileName != NULL) {
    if (strspn(args.defaultFileName, illegalFileChars) != 0) {
      usage("default file name contains illegal characters");
    }
    if (strlen(args.defaultFileName) > 255) {
      usage("default file name too long");
    }
  }

  const char* illegalPathChars = "\\:*?\"<>|";
  if (args.rootPath != NULL) {
    if (strspn(args.rootPath, illegalPathChars) != 0) {
      usage("root path name contains illegal characters");
    }
  }
}

static Arguments parseArguments(int argc, char* argv[]) {
  bool optP = false;
  bool optI = false;

  char* port = (char*)"8080";
  char* defaultFileName = (char*)"index.html";
  char* rootPath = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "p:i:")) != -1) {
    switch (opt) {
      case 'p':
        if (optP) {
          usage("option p used multiple times");
        }
        optP = true;
        if (optarg[0] == '-') {
          usage("missing option p argument or negative argument");
        }
        port = optarg;
        break;

      case 'i':
        if (optI) {
          usage("option i used multiple times");
        }
        optI = true;
        if (optarg[0] == '-') {
          usage("missing option i argument");
        }
        defaultFileName = optarg;
        break;

      default:
        error("illegal option");
    }
  }

  if ((argc - optind) != 1) {
    usage("illegal number of positional arguments");
  }
  rootPath = argv[optind];

  Arguments args = {
      .port = port, .defaultFileName = defaultFileName, .rootPath = rootPath};

  validateArguments(args);

  return args;
}

int main(int argc, char* argv[]) {
  Arguments args = parseArguments(argc, argv);

  initSignalListener();

  // get socket list and store in &result
  struct addrinfo* result;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  int s;
  if ((getaddrinfo(NULL, args.port, &hints, &result)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    error("getaddrinfo");
  }

  // iterate through results until a connection succeeds
  int sockfd;
  struct addrinfo* rp;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1) {
      continue;
    }
    if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;  // success
    }
    close(sockfd);
  }
  freeaddrinfo(result);
  if (rp == NULL) {
    error("no connection could be established");
  }

  // set port to be immediately reusable
  const int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

  // open socket
  const int queueLen = 1 << 4;
  if (listen(sockfd, queueLen) == -1) {
    error("listen");
  }

  // listen to requests
  while (!quit) {
    int reqfd = accept(sockfd, NULL, NULL);
    if (reqfd == -1) {
      error("accept");
    }

    FILE* socketStream = fdopen(reqfd, "w+");
    if (socketStream == NULL) {
      error("fdopen");
    }
  }

  exit(EXIT_SUCCESS);
}