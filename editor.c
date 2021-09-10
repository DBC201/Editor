#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct Line Line;

enum {
	BUFSZ = 1024,
};

struct Line {
	char *content;
	int length;
	Line *next;
	Line *previous;
};

void readstdin(char *buffer, int bufsz, char delimeter);
void readfile(char *filename);
void insertline(Line *current, Line *previous, Line *next);
void unlinkline(Line *line);
Line* createline(char *content, int length, Line *previous, Line *next);
Line* setlines(char *str);
Line* insertstring(Line *currentline, char *str);
Line* appendstring(Line *currentline, char *str);
Line* findsubstring(Line *line, char *str);
Line* readlines(char *filename);
Line* findline(Line *firstline, int id);
Line* moveline(Line *firstline, Line *current, int moveid);
Line* deleteline(Line *line);
int runcommand(Line *line, char *command);
void writelines(Line *firstline, char *filename);
void freelines(Line *firstline);

int
main(int argc, char *argv[])
{	
	Line *firstline;
	char input[BUFSZ];
	Line *currentline;
	Line *temp;
	int quit;
	int i;
	char *filename;
	if (argc != 2) {
		fprintf(stderr, "Usage: ./editor filename\n");
		exit(1);
	}
	else {
		filename = argv[1];
		firstline = readlines(argv[1]);
		currentline = firstline;
		quit = 0;
		while (!quit) {
			printf("%s\n", currentline->content);
			printf(":");
			readstdin(input, BUFSZ, '\n');
			switch (input[0]) {
				case 'p':
					temp = firstline;
					while (temp != NULL) {
						printf("%s\n", temp -> content);
						temp = temp -> next;
					}
					break;
				case 'n':
					temp = firstline;
					i = 1;
					while (temp != NULL) {
						printf("%d:%s\n", i, temp -> content);
						temp = temp -> next;
						i++;
					}
					break;
				case 'q':
					quit = 1;
					break;
				case 'd':
					temp = deleteline(currentline);
					if (currentline == firstline) {
						firstline = temp;
					}
					currentline = temp;
					break;
				case 'm':
					i = atoi(&input[1]);
					if (input[1] == '$') {
						firstline = moveline(firstline, currentline, '$');
					} else if (i < 1) {
						printf("Line number can't be smaller than 1!\n");
					}
					else {
						firstline = moveline(firstline, currentline, i);
					}
					break;
				case 'w':
					writelines(firstline, filename);
					printf("Changes written to %s\n", filename);
					break;
				case 'i':
					readstdin(input, BUFSZ, '.');
					temp = insertstring(currentline, input);
					if (firstline == currentline) {
						firstline = temp;
					}
					break;
				case 'a':
					readstdin(input, BUFSZ, '.');
					appendstring(currentline, input);
					break;
				case 's':
					temp = findsubstring(currentline->next, &input[2]);
					if (temp != NULL) {
						currentline = temp;
					}
					else {
						printf("not found: %s\n", &input[2]);
					}
					break;
				case '|':
					i = runcommand(currentline, &input[1]);
					if (!i) {
						printf("Wrong command.\n");
					}
					break;
				default:
					temp = findline(firstline, atoi(input));
					if (temp == NULL) {
						printf("Line not found\n");
					}
					else {
						currentline = temp;
					}		
			}
			printf("---END---\n");
		}
	}
	freelines(firstline);
	exit(0);
}

void
readstdin(char *buffer, int bufsz, char delimeter)
{
	// read until delimeter from stdin
	int i;
	char c;
	i = 0;
	while ( (c = getchar()) != delimeter ) {
		if ( i < bufsz-1 ) {
			buffer[i] = c;
		}
		i++;
	}
	while ( delimeter != '\n' && (c = getchar()) != '\n' ) {
		// read the leftover characters in input after the dot
	}
	if ( i < bufsz-1 ) {
		buffer[i] = '\0';
	}
	else {
		buffer[bufsz-1] = '\0';
	}
}

void
insertline(Line *current, Line *previous, Line *next)
{
	//inserts the line to the linked list
	if (current == NULL) {
		return;
	}
	current -> previous = previous;
	current -> next = next;
	if (previous != NULL) {
		previous -> next = current;
	}
	if (next != NULL) {
		next -> previous = current;
	}
}

Line*
createline(char *content, int length, Line *previous, Line *next)
{
	//creates a new line by allocating new memory on heap
	//and inserts it to the linked list
	Line *line;
	line = malloc(sizeof(Line));
	if (line == NULL) {
		perror("malloc");
		exit(2);
	}
	line -> content = content;
	line -> length = length;
	insertline(line, previous, next);
	return line;
}

void
unlinkline(Line *line)
{
	// unlinks the line from linked list, does not free memory
	Line *previous;
	Line *next;
	previous = line -> previous;
	next = line -> next;
	if (next != NULL) {
		next -> previous = previous;
	}
	if (previous != NULL) {
		previous -> next = next;
	}
}

Line*
deleteline(Line *line)
{
	// unlinks the line and frees memory
	// returns next line, if null returns previous
	// if both null, returns a newly created line for
	// further use in the buffer
	Line *previous;
	Line *next;
	char *content;
	previous = line -> previous;
	next = line -> next;
	unlinkline(line); 
	free(line -> content);
	free(line);

	if (previous == NULL && next == NULL) { 
		// have something in the buffer to work with later on
		content = malloc(sizeof(char));
		content[0] = '\0';
		return createline(content, 1, NULL, NULL);
	}
	else if (previous == NULL) {
		return next;
	}
	else {
		return previous;
	}
}

Line*
moveline(Line *firstline, Line *current, int moveid)
{
	//moves line and returns the head of the list
	Line *temp;
	int i;
	if (moveid < 1) {
		unlinkline(current);
		insertline(current, NULL, firstline->previous);
		firstline = current;
	} 
	else {
		i = 1;
		temp = firstline;
		while (temp -> next != NULL && i < moveid) {
			temp = temp -> next;
			i++;
		}
		if (temp != current) { 
			// checking if moving a line to itself
			if (current == firstline) { 
				// firstline must change if currentline is first
				firstline = firstline -> next;
			}
			unlinkline(current);
			if (moveid == '$') {
				// move the line below lastline
				insertline(current, temp, NULL);
			}
			else {
				insertline(current, temp->previous, temp);
			}
			if (moveid == 1) { 
				// if the line is being moved to 1st pos
				// it becomes new firstline	
				firstline = current;
			}
		}
	}	
	return firstline;
}

Line* 
setlines(char *str)
{ 
	// create line linked list from raw file content(string)
	// head is incremented and with each newline tail is updated to match
	char *head;
	char *tail;
	char *line;
	int i;
	unsigned long size;
	Line *lastline;
	Line *firstline;
	lastline = NULL;
	firstline = NULL;
	head = str;
	tail = str;
	while (head != NULL) {
		if (*head == '\0' && head-tail == 0) {
			// after the last line, there is only a null character
			// including the null character in case the file does not end
			// with a newline
			break;
		}
		else if (*head == '\n' || *head == '\0') {
			// copying the line content to a newly allocated heap
			size = head - tail;
			line = malloc(size);
			if (line == NULL) {
				perror("malloc");
				exit(2);
			}
			i = 0;
			while (tail != head) {
				line[i] = *tail;
				i++;
				tail++;	
			}
			//line[i] = '\0';
			lastline = createline(line, size/sizeof(char), lastline, NULL);
			tail = head+sizeof(char); // continue from the beginning of the next line
			if (firstline == NULL){ // save the first line in the file for return
				firstline = lastline;
			}					
		}
		if (*head == '\0') {
			break;
		}
		head++;
	}
	return firstline;
}

Line*
insertstring(Line *currentline, char *str)
{
	// insert string before given line
	// creates line struct(s)
	// returns the head of the created line structs
	Line *bufferhead;
	Line *head;
	Line *buffertail;
	head = currentline -> previous;
	bufferhead = setlines(str);
	insertline(bufferhead, head, bufferhead->next);
	buffertail = bufferhead;
	while (buffertail -> next != NULL) {
		buffertail = buffertail -> next;
	}
	insertline(buffertail, buffertail -> previous, currentline);
	return bufferhead;
}

Line*
appendstring(Line *currentline, char *str)
{
	// insert string after given line
	// create line struct(s)
	// returns the head of the created line structs
	Line *bufferhead;
	Line *tail;
	tail = currentline -> next;
	if (tail == NULL) {
		bufferhead = setlines(str);
		insertline(bufferhead, currentline, bufferhead->next);
	}
	else { 
		bufferhead = insertstring(tail, str);
	}
	return bufferhead;
}

Line*
findsubstring(Line *line, char *str)
{
	// find given substring in the 
	// following lines and return the first occurence
	// return NULL if not found
	while (line != NULL) {
		if (strstr(line->content, str) != NULL) {
			return line;
		}
		line = line -> next;
	}
	return NULL;
}

Line*
readlines(char *filename)
{ 
	//load the file into memory and convert it to line structs
	FILE *fp;
	unsigned long size;
	struct stat s;
	char *content;
	Line *firstline;
	fp = fopen(filename, "r");
	if (fp == NULL) {
		content = malloc(sizeof(char));
		content[0] = '\0';
		firstline = createline(content, 1, NULL, NULL);
	}
	else {
		stat(filename, &s);
		size = s.st_size;
		content = malloc(size);
		if (content == NULL) {
			perror("malloc");
			exit(2);
		}
		fread(content, size, 1, fp);
		fclose(fp);
		firstline = setlines(content);
		free(content);
	}
	return firstline;
}

int
runcommand(Line *line, char *command)
{
	// run the given command and feed the current line's content as input
	char *tokens[32];
	int i;
	int parentfd[2];
	int childfd[2];
	int errorfd[2]; // pipe for checking if an error has occured
	int pid;
	int status;
	char err[1];
	tokens[0] = strtok(command, " ");
	for (i = 1; i < 32; i++) {
		tokens[i] = strtok(NULL, " ");
		if (tokens[i] == NULL) {
			break;
		}
	}
	pipe(parentfd);
	write(parentfd[1], line->content, line->length);
	close(parentfd[1]);
	pipe(childfd);
	pipe(errorfd);
	pid = fork();
	if (pid != 0) {
		close(errorfd[1]);
		close(parentfd[0]);
		close(childfd[1]);
		wait(&status);
		read(errorfd[0], err, 1);
		if (err[0] == '1') {
			return 0;	
		} else {
			read(childfd[0], line->content, line->length);
		}
		close(childfd[0]);
		return 1;	
	}
	else {
		close(1);
		dup(childfd[1]);
		close(0);
		dup(parentfd[0]);
		
		if ( execvp(tokens[0], tokens) == -1 ) {
			write(errorfd[1], "1", 1); // if an error occurs, write one to error pipe
		}
		else {
			write(errorfd[0], "0", 1);
		}
		exit(0);
	}
}

void
writelines(Line *firstline, char *filename)
{
	// write the changes into file
	FILE *fp;
	fp = fopen(filename, "w");
	if (fp == NULL) {
		perror("fopen");
		exit(3);
	}
	while (firstline != NULL) {
		fwrite(firstline->content, (firstline->length)*sizeof(char), 1, fp);
		fwrite("\n", sizeof(char), 1, fp);
		firstline = firstline -> next;
	}
	fclose(fp);
}	

Line*
findline(Line *firstline, int id)
{
	// find the desired line id from the line linked list
	int i;
	if (id < 1){
		return NULL;
	}
	i = 1;
	while (firstline != NULL) {
		if (i == id) {
			return firstline;
		}
		firstline = firstline -> next;
		i++;
	}
	return firstline;
}

void
freelines(Line *firstline) { 
	// frees all the lines and their contents
	Line *nextline;
	while (firstline != NULL) {
		nextline = firstline -> next;
		free(firstline->content);
		free(firstline);
		firstline = nextline;
	}
}

