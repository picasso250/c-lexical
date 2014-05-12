/* main.c */
/* this file is to lexic pass of c */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <ctype.h>

int read_file_name_from_arg(char * file_name)
{
	strcpy(file_name, "test.c");
	return 0;
}

#define MACHINE_INIT 0
#define MACHINE_ERROR -1
#define MACHINE_LAST_STATE -2
#define MACHINE_COMMENT_READY 1
#define MACHINE_COMMENT_MULTI_LINE 11
#define MACHINE_COMMENT_SINGLE_LINE 12
#define MACHINE_COMMENT_MULTI_LINE_END_READY 111
#define MACHINE_UNKOWN 2 // leave it alone
#define MACHINE_WORD 3
#define MACHINE_OPERATOR 4
#define MACHINE_DIGIT 5
#define MACHINE_STRING 6
#define MACHINE_STRING_BACKSLASH 61

#define CHAR_ANY -1
#define CHAR_SPACE -2
#define CHAR_ALPHA_UNDERSCORE -3
#define CHAR_ALPHA_NUM_DOT -4
#define CHAR_DIGIT -5
#define CHAR_WORD -6
#define CHAR_OPERATOR -7

inline int isany(int c)
{
	return true;
}
inline int isalpha_(int c)
{
	return c == '_' || isalpha(c);
}
inline int isalphanumdot(int c)
{
	return c == '.' || isalnum(c);
}
inline int isword(int c)
{
	return c == '_' || isalnum(c);
}

int (* char_type_map[])(int) = {
	NULL, // 0
	isany,
	isspace,
	isalpha_,
	isalphanumdot,
	isdigit,
	isword,
	ispunct,
};

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
	printf("pushing state %d\n", machine_state);
	int current = state_stack[stack_pointer-1];
	if (stack_pointer > 0 && current == MACHINE_COMMENT_READY && (machine_state == MACHINE_COMMENT_MULTI_LINE|| machine_state == MACHINE_COMMENT_SINGLE_LINE))
	{
		state_stack[stack_pointer-1] = machine_state;
	}
	else if (stack_pointer == 0 || machine_state != current)
	{
		state_stack[stack_pointer++] = machine_state;
	}
	state_stack_print();
	return stack_pointer;
}
int state_pop()
{
	printf("poping stack\n");
	state_stack_print();
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
	state_stack_print();
	return last;
}
int state_get()
{
	return state_stack[stack_pointer-1];
}

int buffer_index;
char buffer[65535];
inline int buffer_init()
{
	buffer_index = 0;
	return 0;
}
inline int buffer_append(char c)
{
	buffer[buffer_index++] = c;
	return 0;
}
inline char * buffer_end()
{
	buffer[buffer_index++] = '\0';
	return buffer;
}

int cb_comment_init(char c)
{
	buffer_init();
	return 0;
}
int cb_comment(char c)
{
	buffer_append(c);
	return 0;
}
int cb_comment_end(char c)
{
	printf("comment: %s\n", buffer_end());
	return 0;
}
int cb_word_start(char c)
{
	buffer_init();
	buffer_append(c);
}
int cb_word(char c)
{
	buffer_append(c);
}
int cb_word_end(char c)
{
	printf("word '%s'\n", buffer_end());
}
int cb_operator_start(char c)
{
	buffer_init();
	buffer_append(c);
}
int cb_operator(char c)
{
	buffer_append(c);
}
int cb_operator_end(char c)
{
	printf("operator '%s'\n", buffer_end());
}
int cb_digit_start(char c)
{
	buffer_init();
	buffer_append(c);
}
int cb_digit(char c)
{
	buffer_append(c);
}
int cb_digit_end(char c)
{
	printf("digit '%s'\n", buffer_end());
}
int cb_string_start(char c)
{
	buffer_init();
}
int cb_string(char c)
{
	buffer_append(c);
}
int cb_string_end(char c)
{
	printf("string '%s'\n", buffer_end());
}
int cb_word_end_operator_start(char c)
{
	cb_word_end(c);
	cb_operator_start(c);
	return 0;
}
int cb_digit_end_operator_start(char c)
{
	cb_digit_end(c);
	cb_operator_start(c);
	return 0;
}
int cb_operator_end_digit_start(char c)
{
	cb_operator_end(c);
	cb_digit_start(c);
	return 0;
}
int cb_operator_end_word_start(char c)
{
	cb_operator_end(c);
	cb_word_start(c);
	return 0;
}


struct transfer_table_entry
{
	int state_current; // current machine state
	int char_current;
	int state_next; // state tranfer to // todo we do not need that
	// bool is_preserve; // do we preserve last state
	char description[255];
	int (*callback)(char); // pointer of function who takes a char and returns an int
};

#define MACHINE_STATE_COUNT 29

struct transfer_table_entry transfer_table[] = {
	{MACHINE_INIT, CHAR_SPACE, MACHINE_INIT, "space char",
		NULL},
	{MACHINE_INIT, CHAR_ALPHA_UNDERSCORE, MACHINE_WORD, "[a-z_], start word",
		cb_word_start},
	{MACHINE_INIT, CHAR_DIGIT, MACHINE_DIGIT, "'0-9', start digit",
		cb_digit_start},
	{MACHINE_INIT, '"', MACHINE_STRING, "start string",
		cb_string_start},
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
	{MACHINE_STRING, '\\', MACHINE_STRING_BACKSLASH, "start back slash",
		cb_string},
	{MACHINE_STRING, CHAR_ANY, MACHINE_STRING, "in string",
		cb_string},
	{MACHINE_INIT, CHAR_OPERATOR, MACHINE_OPERATOR, "start operators",
		cb_operator_start},
	{MACHINE_STRING, '"', MACHINE_INIT, "end string",
		cb_string_end},
	{MACHINE_STRING_BACKSLASH, '\\', MACHINE_STRING, "end back slash",
		cb_string},
	{MACHINE_WORD, CHAR_WORD, MACHINE_WORD, "\\w in word",
		cb_word},
	{MACHINE_WORD, CHAR_SPACE, MACHINE_INIT, "end word",
		cb_word_end},
	{MACHINE_WORD, CHAR_OPERATOR, MACHINE_OPERATOR, "end word, start operators",
		cb_word_end_operator_start},
	{MACHINE_DIGIT, CHAR_ALPHA_NUM_DOT, MACHINE_DIGIT, "in digit",
		cb_digit},
	{MACHINE_DIGIT, CHAR_SPACE, MACHINE_INIT, "end digit",
		cb_digit_end},
	{MACHINE_DIGIT, CHAR_OPERATOR, MACHINE_INIT, "end digit, start operators",
		cb_digit_end_operator_start},
	{MACHINE_OPERATOR, CHAR_OPERATOR, MACHINE_OPERATOR, "in operators",
		cb_operator_start},
	{MACHINE_OPERATOR, CHAR_SPACE, MACHINE_DIGIT, "end operators",
		cb_operator_end},
	{MACHINE_OPERATOR, CHAR_DIGIT, MACHINE_DIGIT, "end operators, start digit",
		cb_operator_end_digit_start},
	{MACHINE_OPERATOR, CHAR_WORD, MACHINE_DIGIT, "end operators, start word",
		cb_operator_end_word_start},
};

#define TOKEN_TYPE_WORD 2
#define TOKEN_TYPE_STRING 3
#define TOKEN_TYPE_COMMENT 4
#define TOKEN_TYPE_OPERATOR 5

struct token
{
	int type;
	char content[];
};

bool is_char_belong(int c, int char_class)
{
	int (*func)(int);
	if (char_class < 0)
	{
		func = char_type_map[-char_class];
		return func(c);
	}
	else
	{
		return (char_class == c);
	}
}
int find_transfer_entry(int machine_state, int c)
{
	int i;
	for (i = 0; i < MACHINE_STATE_COUNT; ++i)
	{
		int state = transfer_table[i].state_current;
		int char_class = transfer_table[i].char_current;
		// match the first, regardless others
		if (state == machine_state && is_char_belong(c, char_class))
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
		printf("goto last state\n");
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
