#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESIZE 512

typedef struct {
  char **lines;
  size_t len;
} Buffer;

FILE *edit(FILE *, size_t);
size_t setline(FILE *, size_t);
Buffer buff_fill(FILE *);
void buff_free(Buffer *);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <file>\n", argv[0]);
    return 1;
  }

  FILE *file = fopen(argv[1], "r+");
  if (!file) {
    file = fopen(argv[1], "w");
    fprintf(file, "\n");
    fclose(file);
    file = fopen(argv[1], "r+");
  }

  size_t linenum = 1;
  Buffer buff = {NULL, 0};
  char **tmpLine = NULL;
  size_t written = 0;

  while (1) {
    size_t linesize = LINESIZE;
    char *line = calloc(linesize, sizeof(char));

    scanf("%512[^\n]s", line);
    fflush(stdin);
    if ('1' <= line[0] && line[0] <= '9')
      linenum = setline(file, atoi(line));

    char cmd = line[0];
    int flusher;

    switch (cmd) {
    case 'q':
      fclose(file);
      return 0;
    case 'd':
      buff_free(&buff);
      break;
    case 'v':
      rewind(file);
      while (getline(&line, &linesize, file) != -1)
        printf("%s", line);
      break;
    case 'V':
      rewind(file);
      for (size_t i = 1; getline(&line, &linesize, file) != -1; ++i) {
        printf("%lu: %s", i, line);
      }
      break;
    case 'b':
      if (buff.lines == NULL) {
        printf("Buffer is empty\n");
        break;
      }
      for (size_t i = 0; i < buff.len; i++) {
        if (buff.lines[i] == NULL)
          continue;
        printf("%s", buff.lines[i]);
      }
      break;
    case 'e':
      if (buff.lines != NULL) {
        fprintf(stderr, "Buffer is not empty\n");
        break;
      }
      while ((flusher = getchar()) != '\n' && flusher != EOF) {
      }
      buff = buff_fill(file);

      free(line);
      line = calloc(linesize, sizeof(char));
      scanf("%512[^\n]s", line);
      fflush(stdin);
      strncat(line, "\n", 1);
      strcpy(buff.lines[linenum - 1], line);
      break;
    case 'E':
      if (buff.lines != NULL) {
        fprintf(stderr, "Buffer is not empty\n");
        break;
      }
      while ((flusher = getchar()) != '\n' && flusher != EOF) {
      }
      buff = buff_fill(file);

      free(line);
      line = calloc(linesize, sizeof(char));
      scanf("%512[^\n]s", line);
      fflush(stdin);
      while (0 != strcmp(line, "%end%\n")) {
        tmpLine = realloc(buff.lines, ++buff.len * sizeof(char *));
        if (tmpLine == NULL) {
          break;
        }
        buff.lines = tmpLine;

        buff.lines[linenum - 1] = calloc(LINESIZE, sizeof(char));
        strcpy(buff.lines[linenum++ - 1], line);
        getline(&line, &linesize, stdin);
        fflush(stdin);
      }
      linenum--;
      printf("Done [Press ENTER]");
      break;
    case 'w':
      if (buff.lines == NULL) {
        printf("Buffer is empty\n");
        break;
      }
      fclose(file);
      file = fopen(argv[1], "w");
      for (size_t i = 0; i < buff.len; i++) {
        if (buff.lines[i] == NULL)
          continue;
        fprintf(file, "%s", buff.lines[i]);
        written += strlen(buff.lines[i]);
      }
      printf("\"%s\" %luL, %luB\n", argv[1], buff.len, written * sizeof(char));
      buff_free(&buff);
      fclose(file);
      file = fopen(argv[1], "r+");
      written = 0;
      break;
    case 'n':
      if (buff.lines != NULL) {
        printf("Buffer is not empty\n");
        break;
      }
      buff = buff_fill(file);
      if (buff.len == 0) {
        tmpLine = realloc(buff.lines, ++buff.len * sizeof(char *));
        buff.lines = tmpLine;
        buff.lines[0] = calloc(LINESIZE, sizeof(char));
        buff.lines[0][0] = '\n';
      }
      tmpLine = realloc(buff.lines, ++buff.len * sizeof(char *));
      if (tmpLine == NULL) {
        printf("Error: realloc failed\n");
        break;
      }
      buff.lines = tmpLine;
      for (size_t i = buff.len - 1; i > linenum - 1; i--) {
        buff.lines[i] = buff.lines[i - 1];
      }

      buff.lines[linenum] = calloc(LINESIZE, sizeof(char));
      buff.lines[linenum][0] = '\n';
      break;
    case 'x':
      if (buff.lines != NULL) {
        printf("Buffer is not empty\n");
        break;
      }
      buff = buff_fill(file);
      buff.lines[linenum - 1] = NULL;
      for (size_t i = linenum - 1; i < buff.len - 1; i++) {
        buff.lines[i] = buff.lines[i + 1];
      }
      tmpLine = realloc(buff.lines, --buff.len * sizeof(char *));
      if (tmpLine == NULL) {
        printf("Error: realloc failed\n");
        break;
      }
      buff.lines = tmpLine;
      break;
    case 'h':
      printf("Commands:\n");
      printf("<integer>: select line\n\
e: edit line\n\
b: view buffer\n\
d: dump buffer\n\
w: write buffer to file\n\
v: view file\n\
V: view file with line numbers\n\
n: newline after line\n\
x: delete line\n\
q: quit\n");
    default:
      break;
    }
    getline(&line, &linesize, stdin);
  }

  return 0;
}

size_t setline(FILE *file, size_t linenum) {
  Buffer tmp = buff_fill(file);
  size_t len = tmp.len;
  buff_free(&tmp);
  return linenum > len ? len : linenum;
}

void buff_free(Buffer *buff) {
  if (buff->lines == NULL) {
    fprintf(stderr, "Buffer is empty\n");
    return;
  }
  for (size_t i = 0; i < buff->len; ++i) {
    if (buff->lines[i] == NULL)
      continue;
    free(buff->lines[i]);
  }
  free(buff->lines);
  *buff = (Buffer){NULL, 0};
}

Buffer buff_fill(FILE *file) {
  rewind(file);
  size_t linesize = LINESIZE;
  char *line = calloc(linesize, sizeof(char));
  char **buff = calloc(linesize, sizeof(char *));
  size_t i = 0;
  while (getline(&line, &linesize, file) != -1) {
    buff[i] = calloc(linesize, sizeof(char));
    strcpy(buff[i], line);
    i++;
  }

  free(line);
  rewind(file);
  return (Buffer){buff, i};
}
