#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

int main() {
  bool interactive = isatty(fileno(stdin));
  
  while(!feof(stdin)) {
    if (interactive) {
      fprintf(stderr, "> ");
    }
    
    char line[1000];
    fgets(line, 1000, stdin);
    if (ferror(stdin)) {
      fprintf(stderr, "Read error: %d", ferror(stdin));
      return 1;
    }
    fprintf(stdout, "%s", line);
  }
  
  return 0;
}
