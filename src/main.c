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
void print_help();
void view(FILE *, int);
void viewbuff(Buffer, int);
void editBuff(FILE *, Buffer *, size_t);
void editBuffLines(FILE *, Buffer *, size_t);
void writeBuff(Buffer *, FILE *, char *);
void newLine(Buffer *, FILE *, size_t);
void delLine(Buffer *, FILE *, size_t);

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
    char *line = malloc(linesize * sizeof(char));

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
      view(file, 0);
      break;
    case 'V':
      view(file, 1);
      break;
    case 'b':
      viewbuff(buff, 0);
      break;
    case 'B':
      viewbuff(buff, 1);
      break;
    case 'e':
      editBuff(file, &buff, linenum);
      break;
    case 'E':
      editBuffLines(file, &buff, linenum);
      break;
    case 'w':
      writeBuff(&buff, file, argv[1]);
      break;
    case 'n':
      newLine(&buff, file, linenum);
      break;
    case 'x':
      delLine(&buff, file, linenum);
      break;
    case 'h':
      print_help();
    default:
      break;
    }
    getline(&line, &linesize, stdin);
  }

  return 0;
}

void delLine(Buffer *buff, FILE *file, size_t linenum) {
  char **tmpLine = malloc(LINESIZE);
  if (buff->lines != NULL) {
    printf("Buffer is not empty\n");
    return;
  }
  *buff = buff_fill(file);
  buff->lines[linenum - 1] = NULL;
  for (size_t i = linenum - 1; i < buff->len - 1; i++) {
    buff->lines[i] = buff->lines[i + 1];
  }
  tmpLine = realloc(buff->lines, --buff->len * sizeof(char *));
  if (tmpLine == NULL) {
    printf("Error: realloc failed\n");
    return;
  }
  buff->lines = tmpLine;
}

void newLine(Buffer *buff, FILE *file, size_t linenum) {
  char **tmpLine = malloc(LINESIZE);
  if (buff->lines != NULL) {
    printf("Buffer is not empty\n");
    return;
  }
  *buff = buff_fill(file);
  if (buff->len == 0) {
    tmpLine = realloc(buff->lines, ++buff->len * sizeof(char *));
    buff->lines = tmpLine;
    buff->lines[0] = malloc(LINESIZE * sizeof(char));
    buff->lines[0][0] = '\n';
  }
  tmpLine = realloc(buff->lines, ++buff->len * sizeof(char *));
  if (tmpLine == NULL) {
    printf("Error: realloc failed\n");
    return;
  }
  buff->lines = tmpLine;
  for (size_t i = buff->len - 1; i > linenum; i--) {
    buff->lines[i] = buff->lines[i - 1];
  }

  buff->lines[linenum] = malloc(LINESIZE * sizeof(char));
  buff->lines[linenum][0] = '\n';
  free(tmpLine);
}

void writeBuff(Buffer *buff, FILE *file, char *filename) {
  size_t written = 0;
  if (buff->lines == NULL) {
    printf("Buffer is empty\n");
    return;
  }
  fclose(file);
  file = fopen(filename, "w");
  for (size_t i = 0; i < buff->len; i++) {
    if (buff->lines[i] == NULL)
      continue;
    /* fprintf(file, "%s", buff.lines[i]); */
    fwrite(buff->lines[i], sizeof(char), strlen(buff->lines[i]), file);
    written += strlen(buff->lines[i]);
  }
  printf("\"%s\" %luL, %luB\n", filename, buff->len, written * sizeof(char));
  buff_free(buff);
  fclose(file);
  file = fopen(filename, "r+");
}

void editBuffLines(FILE *file, Buffer *buff, size_t linenum) {
  if (buff->lines != NULL) {
    fprintf(stderr, "Buffer is not empty\n");
    return;
  }
  int flusher;
  size_t linesize = LINESIZE;
  char *line = malloc(linesize * sizeof(char));
  while ((flusher = getchar()) != '\n' && flusher != EOF) {
  }
  *buff = buff_fill(file);

  free(line);
  line = malloc(linesize * sizeof(char));
  scanf("%512[^\n]s", line);
  fflush(stdin);
  while (0 != strcmp(line, "%end%\n")) {
    char **tmpLine = realloc(buff->lines, ++buff->len * sizeof(char *));
    if (tmpLine == NULL) {
      return;
    }
    buff->lines = tmpLine;

    buff->lines[linenum - 1] = malloc(LINESIZE * sizeof(char));
    strcpy(buff->lines[linenum++ - 1], line);
    getline(&line, &linesize, stdin);
    fflush(stdin);
  }
  linenum--;
  char **lines = realloc(buff->lines, --buff->len * sizeof(char *));
  if (lines == NULL) {
    return;
  }
  buff->lines = lines;
  printf("Done [Press ENTER]");
}

void editBuff(FILE *file, Buffer *buff, size_t linenum) {
  if (buff->lines != NULL) {
    printf("Buffer is not empty\n");
    return;
  }

  int flusher;
  size_t linesize = LINESIZE;
  char *line = malloc(linesize * sizeof(char));
  while ((flusher = getchar()) != '\n' && flusher != EOF) {
  }
  *buff = buff_fill(file);

  free(line);
  line = malloc(linesize * sizeof(char));
  scanf("%512[^\n]s", line);
  fflush(stdin);
  strncat(line, "\n", 1);
  strcpy(buff->lines[linenum - 1], line);
}

void viewbuff(Buffer buff, int f) {
  if (buff.lines == NULL) {
    printf("Buffer is empty\n");
    return;
  }
  for (size_t i = 0; i < buff.len; i++) {
    if (buff.lines[i] == NULL)
      continue;
    if (f)
      printf("%zu: ", i + 1);
    printf("%s", buff.lines[i]);
  }
}

void view(FILE *file, int f) {
  size_t linesize = 512;
  char *line = malloc(linesize * sizeof(char));
  rewind(file);
  for (size_t i = 0; getline(&line, &linesize, file) != -1; ++i) {
    if (f)
      printf("%zu: ", i + 1);
    printf("%s", line);
  }
  free(line);
}

void print_help() {
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
  char *line = malloc(linesize * sizeof(char));
  char **buff = malloc(linesize * sizeof(char *));
  size_t i = 0;
  while (getline(&line, &linesize, file) != -1) {
    buff[i] = malloc(linesize * sizeof(char));
    strcpy(buff[i], line);
    i++;
  }

  free(line);
  rewind(file);
  return (Buffer){buff, i};
}
