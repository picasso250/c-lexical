/* this file is to lexic pass of c */

#include <stdio.h>
#include <string.h>
#include <limits.h>

#define MACHINE_INIT 0
#define MACHINE_ERROR -1
#define MACHINE_LAST_STATE -2
#define MACHINE_COMMENT_READY 1
#define MACHINE_COMMENT_MULTI_LINE 11
#define MACHINE_COMMENT_SINGLE_LINE 12
#define MACHINE_COMMENT_MULTI_LINE_END_READY 12

#define MACHINE_STATE_COUNT 8

#define CHAR_ANY -1

int read_file_name_from_arg(char * file_name)
{
	strcpy(file_name, "main.c");
	return 0;
}

/*
 3 cols
 first col: current machine state
 second col: char, -1 indicate any char
 third col: state tranfer to
 */
int transfer_table[][3] = {
	{MACHINE_INIT, '/', MACHINE_COMMENT_READY},
	{MACHINE_COMMENT_READY, '*', MACHINE_COMMENT_MULTI_LINE},
	{MACHINE_COMMENT_READY, '/', MACHINE_COMMENT_SINGLE_LINE},
	{MACHINE_COMMENT_READY, CHAR_ANY, MACHINE_ERROR},
	{MACHINE_COMMENT_MULTI_LINE, '*', MACHINE_COMMENT_MULTI_LINE_END_READY},
	{MACHINE_COMMENT_MULTI_LINE_END_READY, CHAR_ANY, MACHINE_COMMENT_MULTI_LINE_END_READY},
	{MACHINE_COMMENT_MULTI_LINE_END_READY, '/', MACHINE_LAST_STATE},
	{MACHINE_COMMENT_SINGLE_LINE, '\n', MACHINE_LAST_STATE},
};

// array of pointers of function which take a char and returns an int
int (*callback_map[])(char) = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};
int machine_state;
int machine_last_state;
int line_num;
int find_transfer_entry(int machine_state, int c)
{
	int i;
	for (i = 0; i < MACHINE_STATE_COUNT; ++i)
	{
		int state = transfer_table[i][0];
		int ch = transfer_table[i][1];
		if (state == machine_state && (c == ch || ch == CHAR_ANY))
		{
			return i;
		}
	}
	return -1;
}
int state_machine_eat_char(char c)
{
	if (c == '\n')
	{
		line_num++;
	}
	printf("machine_state %d\n", machine_state);
	// printf("we read char of %c\n", c);
	int i = find_transfer_entry(machine_state, c);
	if (i == -1) // can not find
	{
		printf("no state %d in transfer_table\n", machine_state);
		return -1;
	}
	// call back
	int (*func_cb)(char) = callback_map[i];
	if (func_cb != NULL)
	{
		return func_cb(c);
	}
	return 0;
}
int main(int argc, char const *argv[])
{
	char file_name[255];
	read_file_name_from_arg(file_name);
	FILE * f = fopen(file_name, "r");
	if (!f)
	{
		printf("error when read file %s\n", file_name);
	}
	char c;
	machine_state = MACHINE_INIT;
	machine_last_state = MACHINE_INIT;
	line_num = 1;
	while ((c = getc(f)) != EOF)
	{
		if (1 == state_machine_eat_char(c))
		{
			break;
		}
		// putchar(c);
	}
	fclose(f);
	return 0;
}
