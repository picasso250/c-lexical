/* this file is to lexic pass of c */

#include <stdio.h>
#include <string.h>

#define MACHINE_INIT 0
#define MACHINE_LAST_STATE -1
#define MACHINE_ERROR -2
#define MACHINE_COMMENT_READY 1
#define MACHINE_COMMENT_MULTI_LINE 11
#define MACHINE_COMMENT_SINGLE_LINE 12
#define MACHINE_COMMENT_MULTI_LINE_END_READY 12

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
	{MACHINE_COMMENT_READY, -1, MACHINE_ERROR},
	{MACHINE_COMMENT_MULTI_LINE, '*', MACHINE_COMMENT_MULTI_LINE_END_READY},
	{MACHINE_COMMENT_MULTI_LINE_END_READY, -1, MACHINE_COMMENT_MULTI_LINE_END_READY},
	{MACHINE_COMMENT_MULTI_LINE_END_READY, '/', MACHINE_LAST_STATE},
	{MACHINE_COMMENT_SINGLE_LINE, '\n', MACHINE_LAST_STATE},
};
int callback_map[] = {

};
int machine_state;
int machine_last_state;
int line_num;
int state_machine_eat_char(char c)
{
	if (c == '\n')
	{
		line_num++;
	}
	printf("machine_state %d\n", machine_state);
	// printf("we read char of %c\n", c);
	if (machine_state == MACHINE_INIT)
	{
		if (c == '/')
		{
			machine_state = MACHINE_COMMENT_READY;
			printf("we are ready to comment \n");
		}
	}
	else if (machine_state == MACHINE_COMMENT_READY)
	{
		if (c == '*')
		{
			machine_state = MACHINE_COMMENT_MULTI_LINE;
			printf("MACHINE_COMMENT_MULTI_LINE start\n");
		}
		else if (c == '/')
		{
			machine_state = MACHINE_COMMENT_SINGLE_LINE;
			printf("MACHINE_COMMENT_SINGLE_LINE start\n");
		}
		else
		{
			printf("error, expect '/*' or '//' on line %d\n", line_num);
			return 1;
		}
		return 0;
	}
	else if (machine_state == MACHINE_COMMENT_MULTI_LINE)
	{
		if (c == '*')
		{
			machine_state = MACHINE_COMMENT_MULTI_LINE_END_READY;
		}
		else
		{
			putchar(c);
		}
	}
	else if (machine_state == MACHINE_COMMENT_MULTI_LINE_END_READY)
	{
		if (c == '/')
		{
			machine_state = MACHINE_INIT; // todo old state
		}
		else if (c == '*')
		{
			machine_state = MACHINE_COMMENT_MULTI_LINE_END_READY;
		}
		else
		{
			machine_state = MACHINE_COMMENT_MULTI_LINE;
		}
	}
	else if (machine_state == MACHINE_COMMENT_SINGLE_LINE)
	{
		if (c == '\n')
		{
			machine_state = MACHINE_INIT; // todo old state
		}
		else
		{
			putchar(c);
		}
	}
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
