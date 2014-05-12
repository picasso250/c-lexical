/* main.c */
/* this file is to lexic pass of c */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <ctype.h>

#define MACHINE_INIT 0
#define MACHINE_ERROR -1
#define MACHINE_LAST_STATE -2
#define MACHINE_COMMENT_READY 1
#define MACHINE_COMMENT_MULTI_LINE 11
#define MACHINE_COMMENT_SINGLE_LINE 12
#define MACHINE_COMMENT_MULTI_LINE_END_READY 111
#define MACHINE_UNKOWN 2 // leave it alone
#define MACHINE_COMPILE_PROCESSOR 3
#define MACHINE_INCLUDE 31
#define MACHINE_INCLUDE_NAME 311
#define MACHINE_DEFINE 32

#define MACHINE_STATE_COUNT 14

#define CHAR_SPACE -1
#define CHAR_ANY -2

int read_file_name_from_arg(char * file_name)
{
	strcpy(file_name, "test.c");
	return 0;
}

int machine_state;
int line_num;

int stack_pointer = 0;
int state_stack[1000];
// for debug
int state_stack_print()
{
	printf("stack: ");
	for (int i = 0; i < stack_pointer; ++i)
	{
		printf("%d,", state_stack[i]);
	}
	printf("\n");
}
int state_push(int machine_state)
{
	// printf("pushing state %d\n", machine_state);
	int current = state_stack[stack_pointer-1];
	if (stack_pointer > 0 && current == MACHINE_COMMENT_READY && (machine_state == MACHINE_COMMENT_MULTI_LINE|| machine_state == MACHINE_COMMENT_SINGLE_LINE))
	{
		state_stack[stack_pointer-1] = machine_state;
	}
	else if (stack_pointer == 0 || machine_state != current)
	{
		state_stack[stack_pointer++] = machine_state;
	}
	// state_stack_print();
	return stack_pointer;
}
int state_pop()
{
	// printf("poping stack\n");
	// state_stack_print();
	int last = 0;
	do {
		last = state_stack[--stack_pointer];
		if (last == MACHINE_COMMENT_READY)
		{
			continue;
		}
		else
		{
			break;
		}
	} while (true);
	// state_stack_print();
	return last;
}
int state_get()
{
	return state_stack[stack_pointer-1];
}

int buffer_comment_index;
char buffer_comment[65535];
int cb_comment_init(char c)
{
	buffer_comment_index = 0;
	return 0;
}
int cb_comment(char c)
{
	buffer_comment[buffer_comment_index++] = c;
	return 0;
}
int cb_comment_end(char c)
{
	buffer_comment[buffer_comment_index++] = '\0';
	printf("%s\n", buffer_comment);
	return 0;
}
char buffer_processor_name_index;
char buffer_processor_name[99];
int cb_compile_processor_name(char c)
{
	buffer_processor_name[buffer_processor_name_index++] = c;
}
int cb_compile_processor_name_start(char c)
{
	buffer_processor_name_index = 0;
}
int cb_compile_processor_name_end(char c)
{
	buffer_processor_name[buffer_processor_name_index++] = '\0';
	printf("#%s\n", buffer_processor_name);
	if (strcmp("include", buffer_processor_name) == 0)
	{
		printf("we include a file\n");
		state_push(MACHINE_INCLUDE);
		machine_state = MACHINE_INCLUDE;
	}
	else if (strcmp("define", buffer_processor_name) == 0)
	{
		printf("we define a macro\n");
		state_push(MACHINE_DEFINE);
		machine_state = MACHINE_DEFINE;
	}
	else
	{
		printf("we do not understand '#%s'\n", buffer_processor_name);
		return -1;
	}
}
int word_index;
char word[255];
int cb_include_name(char c)
{
	word[word_index++] = c;
}
int cb_include_name_start(char c)
{
	word_index = 0;
}
int cb_include_name_end(char c)
{
	word[word_index] = '\0';
	printf("#include '%s'\n", word);
}

struct transfer_table_entry
{
	int state_current; // current machine state
	int char_current;
	int state_next; // state tranfer to
	// bool is_preserve; // do we preserve last state
	char description[255];
	int (*callback)(char); // pointer of function who takes a char and returns an int
};

struct transfer_table_entry transfer_table[] = {
	{MACHINE_INIT, CHAR_SPACE, MACHINE_INIT, "space char",
		NULL},
	{MACHINE_INIT, '#', MACHINE_COMPILE_PROCESSOR, "compile processor start",
		cb_compile_processor_name_start},
	{MACHINE_COMPILE_PROCESSOR, ' ', MACHINE_UNKOWN, "compile processor start",
		cb_compile_processor_name_end},
	{MACHINE_COMPILE_PROCESSOR, CHAR_ANY, MACHINE_COMPILE_PROCESSOR, "read processor name",
		cb_compile_processor_name},
	{MACHINE_INCLUDE, CHAR_SPACE, MACHINE_INCLUDE, "include something",
		NULL},
	{MACHINE_INCLUDE, '<', MACHINE_INCLUDE_NAME, "include <name> start",
		cb_include_name_start},
	{MACHINE_INCLUDE_NAME, '>', MACHINE_LAST_STATE, "include <name> end",
		cb_include_name_end},
	{MACHINE_INCLUDE, '"', MACHINE_INCLUDE_NAME, "include <name> start",
		cb_include_name_start},
	{MACHINE_INCLUDE_NAME, '"', MACHINE_LAST_STATE, "include <name> end",
		cb_include_name_end},
	{MACHINE_INCLUDE_NAME, CHAR_ANY, MACHINE_INCLUDE_NAME, "include <name> constructing",
		cb_include_name},
	{MACHINE_INIT, '/', MACHINE_COMMENT_READY, "ready to comment",
		NULL},
	{MACHINE_COMMENT_READY, '*', MACHINE_COMMENT_MULTI_LINE, "start multi line comment", 
		cb_comment_init},
	{MACHINE_COMMENT_READY, '/', MACHINE_COMMENT_SINGLE_LINE, "start single line comment", 
		NULL},
	{MACHINE_COMMENT_READY, CHAR_ANY, MACHINE_ERROR, "'/' followed by neither of '*' or '/'", 
		NULL},
	{MACHINE_COMMENT_MULTI_LINE, '*', MACHINE_COMMENT_MULTI_LINE_END_READY, "ready to end multi line comment", 
		NULL},
	{MACHINE_COMMENT_MULTI_LINE, CHAR_ANY, MACHINE_COMMENT_MULTI_LINE, "in multi line comment", 
		cb_comment},
	{MACHINE_COMMENT_MULTI_LINE_END_READY, '/', MACHINE_LAST_STATE, "yes, we end multi line comment", 
		cb_comment_end},
	{MACHINE_COMMENT_MULTI_LINE_END_READY, CHAR_ANY, MACHINE_COMMENT_MULTI_LINE_END_READY, "no, we do not end multi line comment", 
		NULL},
	{MACHINE_COMMENT_SINGLE_LINE, '\n', MACHINE_LAST_STATE, "end of line, end of single line comment", 
		cb_comment_end},
	{MACHINE_COMMENT_SINGLE_LINE, CHAR_ANY, MACHINE_COMMENT_SINGLE_LINE, "in one line comment", 
		cb_comment},
};

int find_transfer_entry(int machine_state, int c)
{
	int i;
	for (i = 0; i < MACHINE_STATE_COUNT; ++i)
	{
		int state = transfer_table[i].state_current;
		int ch = transfer_table[i].char_current;
		// match the first, regardless others
		if (state == machine_state && (c == ch || ch == CHAR_ANY || (ch == CHAR_SPACE && isspace(c))))
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
	printf("=====================\nmachine_state %d, we read char '%c'\n", machine_state, c);
	state_stack_print();
	int i = find_transfer_entry(machine_state, c);
	if (i == -1) // can not find
	{
		printf("no state %d and char %c in transfer_table\n", machine_state, c);
		return -1;
	}
	printf("STTE %d: %s\n", i, transfer_table[i].description);
	// call back
	int (*func_cb)(char) = transfer_table[i].callback;
	if (func_cb != NULL)
	{
		printf("callback %d\n", i);
		if (-1 == func_cb(c))
		{
			return -1;
		}
	}
	int last_state = machine_state;
	int next_state = transfer_table[i].state_next; // next state
	if (next_state == MACHINE_LAST_STATE)
	{
		state_pop();
		machine_state = state_get();
	}
	else if (next_state == MACHINE_ERROR)
	{
		printf("error, it is not allowed that state %d followed by ", machine_state);
		if (c == CHAR_ANY)
		{
			printf("any char\n");
		}
		else
		{
			printf("char '%c'\n", c);
		}
	}
	else if (next_state == MACHINE_UNKOWN);
	else
	{
		state_push(last_state);
		machine_state = next_state;
	}
	printf("we go to state %d\n", machine_state);
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
