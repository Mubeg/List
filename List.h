#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

#define asserted || (fprintf(stderr, "Asserted from %s on %d in %s\n", __LOCATION__), abort(), false);

#define __LOCATION__  __FILE__, __LINE__, __PRETTY_FUNCTION__
#define LIST_INIT(name, ...) list_init((name), #name, ##__VA_ARGS__)

#define USING_WCHAR_T

typedef char Buff_elem_t;
#undef USING_WCHAR_T

#ifndef USING_WCHAR_T
#define wmemchr memchr
#define iswalpha isalpha
#define Long_string(x) x
#define fgetwc fgetc
#define fwprintf fprintf
#define wcslen strlen

#else
#define Long_string(x) L ## x

#endif

//#define TESTING

#ifdef TESTING
#define NDEBUG
#endif
#include <assert.h>


typedef int Elem_t;
#define ELEM_PRINT "%li" 

typedef long long int Canary_t;
#define CANARY_PRINT "%lX"

typedef long long int hash_t;
#define HASH_PRINT "%li"

//#pragma pack(push, 8)
struct List_t {

	hash_t hash; // always comes first
	const char * name;

	Canary_t canary1;
	int free;
	int head;

	char * buff;
	Canary_t *canary_dynamic1;
	Elem_t *data;
	int *next;
	int *prev;
	Canary_t *canary_dynamic2;

	size_t size;
	size_t maxsize;
	size_t false_poison;
	int errnum;

	Canary_t canary2;

};
//#pragma pack(pop)

typedef struct str * str_ptr;
struct str {
	Buff_elem_t *str;
	int size;
};
#pragma pack(push, 1)
struct Torture_t {

	char exec1[1];
	Stack_t stk;
	char exec2[1];

};
#pragma pack(pop)

const int CHECK_ZERO_ = __LINE__;
const int CHECK_CANARY             = 1 <<  (__LINE__ - CHECK_ZERO_ - 1); //= 1;
const int CHECK_HASH               = 1 <<  1; //= 2;
const int CHECK_SIZE               = 1 <<  2; //= 4;
const int BUFF_NULLPTR             = 1 <<  3; //= 8;
const int NAME_NULLPTR             = 1 <<  4; //= 16;
const int CANARY_DYNAMIC1_NULLPTR  = 1 <<  5; //= 32;
const int CANARY_DYNAMIC2_NULLPTR  = 1 <<  6; //= 64;
const int DATA_NULLPTR             = 1 <<  7; //= 128;
const int NEG_SIZE_MAXSIZE         = 1 <<  8; //= 256;
const int CHECK_DYN_CAN_AND_DATA   = 1 <<  9; //= 512;
const int NEG_SIZE_FALSE_POISON    = 1 << 10; //= 1024;
const int CHECK_NUM_POISON         = 1 << 11; //= 2048;
const int MAX_ERROR_NO 		   = (__LINE__ - CHECK_ZERO_ - 1);


const int DEFAULT = 0;
const bool DEBUG = 0;
const Canary_t CANARY_VALUE = 0xBEDAAAAAAAAAAAAA;
const Elem_t POISON = 'ATIKIN';
const int LIST_INIT_SIZE = 5;
const int START_FUNCTION = 1;
const char LOG_FILE[] = "list_log.txt";
const int END_FUNCTION = 0;
const int LIST_CLUSTER_SIZE = sizeof(Elem_t) + 2*sizeof(int);


bool test_list(void);


int get_test(str command_line);

/*!
Создаёт одномерный массив и читает в него сырой текст из файла
\param[in] file_name[] Название файла
\param[out] buff_size Размер созданого массива
\return Указатель на массив
*/
Buff_elem_t *read_file_to_created_buff(const char file_name[], int *buff_size);

/*!
Рассчитывает размер файла, указатель файла ставится в начало
\param[in] file - структура FILE * открытого файла
\param[in] mode //none in alpha
\return Размер файла в int
*/
int get_file_length(FILE *file, const int mode);

/*!
Создаёт массив указателей на начало строк и заменяет '\n' на '\0'
\param[in] buff Указатель на массив сырого текста
\param[in] buff_size Размер массива
\param[out] text_size Размер созданного массива
\return Указатель на массив
*/
str_ptr make_text_must_free(Buff_elem_t buff[], const int buff_size, int * const text_size);


int change_ch1_to_ch2_and_count_in_ch_buff(const Buff_elem_t char1, const Buff_elem_t char2, Buff_elem_t buff[], const int buff_size);


int fill_string_split_by_separator_and_skip_non_alpha_lines_from_ch_buff(str_ptr text, const Buff_elem_t separator, 
									 Buff_elem_t buff[], const int buff_size);


int fill_string_split_by_separator_from_ch_buff(str_ptr text, const Buff_elem_t separator, Buff_elem_t buff[], const int buff_size);


bool list_init(List_t *stk, const char name[], const size_t init_list_size = LIST_INIT_SIZE);


bool list_add(Stack_t *stk, Elem_t elem);


Elem_t stack_pop(Stack_t *stk, int *err = nullptr);


bool stack_deinit(Stack_t *stk);


bool stack_resize_if_needed(Stack_t *stk, bool is_increasing = false);


void set_dynamic_canaries_and_data(Stack_t * stk);


void * recalloc_safe(void * const data, const long long int prev_num, const long long int num, const size_t size);


void zero_data(void * const data, const int start, const int end);


bool check_stack(Stack_t *stk, const char called_from_file[], int line, const char func[], 
		 int cur_state_func = NO_CHECK_FUNCTION_FULLCHECK, bool print_all = true);


void stack_dump(Stack_t * const stk, const char called_from_file[], int line, const char func[], bool ok);


bool check_maxsize(Stack_t * const stk);


bool check_number_of_poison(Stack_t * const stk);


bool check_nullptr(const void * const stk);


bool check_canary(Stack_t * const stk);


bool check_size_not_neg(size_t const stk);


bool check_hash_All(Stack_t * const stk);


bool check_hashable(Stack_t * const stk);


bool check_dynamic_canaries_and_data(Stack_t * const stk);


bool fill_data_with_poison(Elem_t * const data, const size_t size);


bool is_equal_hash_t(const hash_t left, const hash_t right);


hash_t find_sum_hash(Stack_t * const stk, size_t size);


hash_t hash(const void * const stk, size_t size);


bool check_size(Stack_t * const stk);



