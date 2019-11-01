#include "List.h"

int main(){
}


/*bool test_stack(void){

	Torture_t torture = {};

	Buff_elem_t * buff = nullptr;
	int buff_size = 0;
	buff = read_file_to_created_buff("torture.txt", &buff_size);

	if(buff == nullptr){
		fprintf(stderr, "Not able to create buff in %s line %d in %s\n", __LOCATION__);
		return false;
	}

	int commands_size = 0;
	str_ptr commands = make_text_must_free(buff, buff_size, &commands_size);

	if(commands == nullptr){
		fprintf(stderr, "Not able to create commands in %s line %d in %s\n", __LOCATION__);
		return false;
	}

	for(int i = 0; i < (commands_size = 11); i++){

		int test_id = 0;
		bool passed = false;	

		test_id = get_test(commands[i]);
		test_id = i;
		switch (test_id) {
			case 0:
				STACK_INIT(&torture.lst);
				passed = !check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 1:
				STACK_INIT(&torture.lst);
				torture.exec1[5] = '\0';
				torture.lst.size = -1;
				torture.lst.maxsize = -200;
				torture.lst.false_poison = -100000;
				torture.lst.buff = nullptr;
				torture.lst.name = nullptr;
				torture.lst.data = nullptr;
				torture.lst.canary_dynamic1 = nullptr;
				torture.lst.canary_dynamic2 = nullptr;
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 2:
				STACK_INIT(&torture.lst);
				torture.lst.size = -1;
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 3:
				STACK_INIT(&torture.lst);
				torture.lst.maxsize = -200;
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 4:
				STACK_INIT(&torture.lst);
				torture.lst.false_poison = -100000;
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 5:
				STACK_INIT(&torture.lst);
				torture.lst.buff = nullptr;
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 6:
				STACK_INIT(&torture.lst);
				torture.lst.name = nullptr;
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 7:
				STACK_INIT(&torture.lst);
				torture.lst.data = nullptr;
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 8:
				STACK_INIT(&torture.lst);
				torture.lst.canary_dynamic1 = nullptr;
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 9:
				STACK_INIT(&torture.lst);
				torture.lst.canary_dynamic2 = nullptr;
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);
				break;
			case 10:
				STACK_INIT(&torture.lst);
				zero_data(torture.exec1, 0, 100);
				passed = check_stack(&torture.lst, __LOCATION__);
				stack_deinit(&torture.lst);	
				break;
			default:
				fprintf(stderr, "Unexpected return of get_test in %s line %d in %s\n", __LOCATION__);
				break;
		}

		if(passed){
			fprintf(stderr, "Test #%d passed. %s line %d in %s\n", test_id, __LOCATION__);
			return false;
		}
	}

	free(buff);
	free(commands);

	return true;
}*/


int get_test(str command_line){

	return 1;
}

bool list_init(List_t *lst, const char name[], const size_t init_list_size /*= LIST_INIT_SIZE*/){ // DONE

	assert(check_nullptr(lst));
	if(init_list_size < 0){
		return false;	
	}

	lst->canary1 = CANARY_VALUE;
	
	lst->maxsize = init_list_size;

	lst->buff = (char *) calloc(lst->maxsize*(LIST_CLUSTER_SIZE) + 2*sizeof(Canary_t), sizeof(char));
	if(lst->buff == nullptr){
		return false;
	}
	
	set_dynamic_canaries_and_data(lst);
	fill_data_with_poison(lst->data, lst->maxsize, POISON, sizeof(Elem_t));
	fill_data_with_poison(lst->next, lst->maxsize, -1, sizeof(int));
	fill_data_with_poison(lst->data, lst->maxsize, -1, sizeof(int));

	lst->false_poison = 0;

	lst->head = 0;
	lst->free = 1;
	for(int i = 1; i < maxsize - 1; i++){ //next[last] = -1
		lst->next[i] = i + 1;
	}

	lst->size = 0;
	lst->errnum = 0;

	lst->name = name[0] == '&' ? name + 1 : name;

	lst->canary2 = CANARY_VALUE;

	lst->hash = find_sum_hash(lst, sizeof(List_t));

	assert(check_list(lst, __LOCATION__));
 
	return true;
}

int list_add_before(List_t *lst, Elem_t elem, int pos){

	assert(check_list(lst, __LOCATION__));
	
	int new_pos = list_add_between(lst, elem, lst->prev[pos], pos);
	
	assert(check_list(lst, __LOCATION__));

	return new_pos;
}

int list_add_after(List_t *lst, Elem_t elem, int pos){

	assert(check_list(lst, __LOCATION__));
	
	int new_pos = list_add_between(lst, elem, pos, lst->next[pos]);
	
	assert(check_list(lst, __LOCATION__));

	return new_pos;
}

int list_add_between(List_t *lst, Elem_t elem, int pos_left, int pos_right){

	assert(check_list(lst, __LOCATION__));
	
	if(!list_resize_if_needed(lst, /*is_increasing = true*/ true)){ // DONE
		assert(check_list(lst, __LOCATION__));
		return -1;
	}

	if(elem == POISON){
		lst->false_poison++;
	}

	int pos = lst->free;
	lst->data[pos] = elem;
	lst->free = lst->next[pos];

	lst->next[pos]   = pos_right;
	lst->prev[pos]   = pos_left;
	lst->next[pos_left]  = pos;
	lst->prev[pos_right] = pos;
	
	lst->size++;
	
	lst->hash = find_sum_hash(lst, sizeof(List_t));
	
	assert(check_stack(lst, __LOCATION__));

	return pos;
}

bool list_del(List_t *lst, int pos){ //DONE

	assert(check_list(lst, __LOCATION__));
	assert(pos >= 0);


	if(lst->data[pos] == POISON){
		lst->false_poison--;
	}

	lst->next[lst->prev[pos]] = lst->next[pos];
	lst->prev[lst->next[pos]] = lst->prev[pos];	

	lst->next[pos] = lst->free;
	lst->prev[pos] = -1;
	lst->free = pos;
	lst->data[pos] = POISON;

	lst->size--;

	lst->hash = find_sum_hash(lst, sizeof(List_t));

	assert(check_list(lst, __LOCATION__));

	return true;
}

int list_find(List_t *lst, size_t logical_pos){ // DONE
	
	assert(lst != nullptr);
	assert(logical_pos >= 0);	
	
	int counter = 0;
	int pos = -1;
	
	for(int pos = lst->head; counter < logical_pos; counter++, pos = lst->next[pos]);

	return pos;
}

bool list_deinit(List_t *lst){

	assert(check_list(lst, __LOCATION__));
	
	if(lst->buff == nullptr) return false;	

	zero_data(lst->buff, 0, lst->maxsize*(LIST_CLUSTER_SIZE) + 2*sizeof(Canary_t));
	free(lst->buff);

	return true;
}

bool list_resize_if_needed(List_t *lst, bool is_increasing/* = false*/){ // ND

	assert(check_stack(lst, __LOCATION__));
	
	if(is_increasing && lst->size == lst->maxsize){

		void * temp = recalloc_safe(lst->buff, lst->maxsize*LIST_CLUSTER_SIZE + sizeof(Canary_t), 
				       lst->maxsize*2*LIST_CLUSTER_SIZE + sizeof(Canary_t)*2, sizeof(char));
		if(temp == nullptr){
			return false;		
		}

		lst->buff = (char *)temp;
		set_data(lst);
		lst->maxsize *= 2;
		set_dynamic_canaries(lst);
	
		list_shift_unwatched_posion(lst->next, POISON, lst->canary_dynamic2 - lst->next, lst->maxsize / 2 * sizeof(Elem_t)) == true asserted;
		lst->next += lst->maxsize / 2 * sizeof(Elem_t);
		lst->prev += lst->maxsize / 2 * sizeof(Elem_t);

		list_shift_unwatched_poison(lst->prev, POISON, lst->canary_dynamic2 - lst->prev, lst->maxsize / 2 * sizeof(int))    == true asserted;
		lst->prev += lst->maxsize / 2 * sizeof(int);

		fill_data_with_poison(lst->data + lst->maxsize / 2 * (2*LIST_CLUSTER_SIZE - sizeof(int)), lst->maxsize / 2  * (sizeof(int)) );

		lst->hash = find_sum_hash(lst, sizeof(List_t));

	}
	/*if(!is_increasing && lst->size < lst->maxsize/4){


		list_shift_unwatched_posion(lst->next, POISON, lst->canary_dynamic2 - lst->next, -lst->maxsize / 2 * sizeof(Elem_t)) == true asserted;
		lst->next -= lst->maxsize / 2 * sizeof(Elem_t);
		lst->prev -= lst->maxsize / 2 * sizeof(Elem_t);

		list_shift_unwatched_poison(lst->prev, POISON, lst->canary_dynamic2 - lst->prev, -lst->maxsize / 2 * sizeof(int))    == true asserted;
		lst->prev -= lst->maxsize / 2 * sizeof(int);

		void * temp = recalloc_safe(lst->buff, lst->maxsize*LIST_CLUSTER_SIZE + sizeof(Canary_t), 
				       lst->maxsize*LIST_CLUSTER_SIZE/2 + sizeof(Canary_t)*2, sizeof(char));
		if(temp == nullptr){
			fprintf(stderr, "realloc problem while making smaller ????\n");
			return false;
		}
		if(temp != lst->buff){
			fprintf(stderr, "WOW, realloc tryies to move data!\n!\n!\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		}
		
		lst->buff = (char *)temp;
		lst->maxsize /= 2;
		set_dynamic_canaries(lst);
		set_data(lst);

		lst->hash = find_sum_hash(lst, sizeof(List_t));
	}*/
	

	assert(check_list(lst, __LOCATION__));

	return true;
}

bool list_shift_unwatched_poison(void * buff, Elem_t poison, size_t size, int shift){
	
	buff  != nullptr asserted;

	for(int i = shift > 0 ? size - 1 : 0; shift > 0 ? i >= 0: i < size; shift > 0 ? i--: i++){	
		((char *)buff)[i + shift] = ((char *)buff)[i];
	}
	
	for(int i = 0; i > (shift > 0 ? -shift : shift); i--){	
		((char*)buff)[i + (shift > 0 ? shift - 1 : size - 1)] = poison;
	}

	return true;	
}

void set_data(List_t * lst){ // DONE

	if(lst == nullptr) return;

	lst->data = (Elem_t *)(lst->buff + sizeof(Canary_t));
	lst->next = (int *)(lst-> buff + sizeof(Canary_t) + lst->maxsize*sizeof(Elem_t));
	lst->prev = (int *)(lst-> buff + sizeof(Canary_t) + lst->maxsize*(sizeof(Elem_t) + sizeof(int)));

}

void set_dynamic_canaries(List_t * lst){ // DONE

	if(lst == nullptr) return;

	lst->canary_dynamic1 = (Canary_t *) lst->buff;
	lst->canary_dynamic1[0] = CANARY_VALUE;

	lst->canary_dynamic2 = (Canary_t *)(lst->buff + sizeof(Canary_t) + lst->maxsize*LIST_CLUSTER_SIZE);
	lst->canary_dynamic2[0] = CANARY_VALUE;

}

bool fill_data_with_poison(void data[], const size_t size, long long int poison, const size_t Elem_size){ //DONE
	
	assert(data != nullptr);
	if(size < 0) return false;

	for(unsigned int i = 0; i < size * Elem_size; i++){
		((char *)data)[i] = (char) (poison << (i % Elem_size));
	}

	return true;

}

void * recalloc_safe(void * const data, const long long int prev_num, const long long int num, const size_t size){ //??
	
	assert(data != nullptr);
		
	if(data == nullptr) return nullptr;

	if(prev_num >= num){

		zero_data(data, num*size, prev_num*size);
		return realloc(data, num*size);
	}
	
	if(sizeof(char) != 1){
		assert('0xBEDA' != '0xBEDA');
	}
	
	char * new_data = (char *) calloc(num, size);
	if(new_data == nullptr){
		return nullptr;
	}

	for(int i = 0; i < prev_num*size; i++){
		new_data[i] = ((char *) data)[i];
	}

	zero_data(data, 0, prev_num*size);


	free(data);
	return (void *) new_data;
}

void zero_data(void * const data, const int start, const int end){ // DONE

	assert(data != nullptr);
	assert(start < end);	

	for(int i = start; i < end; i++){
		((char *) data)[i] = '\0';
	}

}

bool check_list(List_t *lst, const char called_from_file[], int line, const char func[], bool print_all/* = true*/){ //COMPATIBLE


	bool passed = true;
	bool hashable = true;

	if(!check_nullptr((void *) lst)){
		return false;
	}

	if(lst->errnum != 0){
		return false;	
	}

// A Lot of checks below
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_canary(lst)                     ? passed : (lst->errnum += CHECK_CANARY, false);
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_hash_All(lst)		       ? passed : (lst->errnum += CHECK_HASH, false);
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_size(lst)                       ? passed : (lst->errnum += CHECK_SIZE, false);
	if(DEBUG) printf("Passed: %d\n", passed);


	passed = check_nullptr(lst->buff)              ? passed : (lst->errnum += BUFF_NULLPTR, false);
	if(DEBUG) printf("Passed: %d\n", passed);
 
	passed = check_nullptr(lst->name)              ? passed : (lst->errnum += NAME_NULLPTR, false);
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_nullptr(lst->canary_dynamic1)   ? passed : (lst->errnum += CANARY_DYNAMIC1_NULLPTR, false);
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_nullptr(lst->canary_dynamic2)   ? passed : (lst->errnum += CANARY_DYNAMIC2_NULLPTR, false);
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_nullptr(lst->data)              ? passed : (lst->errnum += DATA_NULLPTR, false);
	if(DEBUG) printf("Passed: %d\n", passed);


	passed = check_maxsize(lst)	               ? passed : (lst->errnum += NEG_SIZE_MAXSIZE, false);
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_dynamic_canaries_and_data(lst)  ? passed : (lst->errnum += CHECK_DYN_CAN_AND_DATA, false);
	if(DEBUG) printf("Passed: %d\n", passed);

	passed = check_size_not_neg(lst->false_poison) ? passed : (lst->errnum += NEG_SIZE_FALSE_POISON, false);
	if(DEBUG) printf("Passed: %d\n", passed);
	
	passed = check_number_of_poison(lst)	       ? passed : (lst->errnum += CHECK_NUM_POISON, false);
	if(DEBUG) printf("Passed: %d\n", passed);

/*	passed = check_canary(lst)? passed : (lst->errnum = 1, false);
	passed = check_canary(lst)? passed : (lst->errnum = 1, false);
	passed = check_canary(lst)? passed : (lst->errnum = 1, false);
	passed = check_canary(lst)? passed : (lst->errnum = 1, false);
	passed = check_canary(lst)? passed : (lst->errnum = 1, false);
	passed = check_canary(lst)? passed : (lst->errnum = 1, false);*/

	if(DEBUG) printf("\n");

// End of a Lot of checks
		
	if(print_all || !passed){
		stack_dump(lst, called_from_file, line, func, passed);
	}

	return passed;
}


void stack_dump(List_t * const lst, const char called_from_file[], int line, const char func[], bool ok, time_t curtime /* = -1*/){ //DONE

	if(lst == nullptr) return;

	FILE *log_file = fopen(LOG_FILE, "a");
	if(log_file == nullptr){
		fprintf(stderr, "Not able to open %s in %s line %d in %s\n", LOG_FILE, __LOCATION__);
	}


	fprintf(log_file, "\nTime and date: %s\n"
			  "Dumping stack from file %s, line %d, function %s [%s]\n"
			  "Stack [%p] %s {\n"
			  "\n\t\t hash    = " HASH_PRINT "\n"
			  "\n\t\t canary1 = " CANARY_PRINT "\n"
			  "\t\t size    = %d\n"
			  "\t\t maxsize = %d\n"
			  "\n\t\t buff = [%p]\n"
			  "\t\t canary_dynamic1 [%p] = " CANARY_PRINT "\n"
			  "\t\t data = [%p]{\n", 
		ctime(&cur_time),
		called_from_file, line, func, ok ? "ok" : "ERROR!!!",
		lst, lst->name,
		lst->hash,
		lst->canary1,
		lst->size, 
		lst->maxsize,
		lst->buff, 
		lst->canary_dynamic1, lst->canary_dynamic1 == nullptr ? 'NULLPTR' : lst->canary_dynamic1[0],
		lst->data);
	fflush(log_file);
	
	if(lst->data != nullptr){
		if(check_maxsize(lst))
			for(int i = 0; i < lst->maxsize; i++){

				fprintf(log_file, "\t\t\t data[%d] = %d %s\n", i, lst->data[i], lst->data[i] == POISON ? "POISON?" : "");
			}
	}
	fflush(log_file);

	fprintf(log_file, "\t\t }\n"
			  "\t\t next = [%p]{\n", lst->next);
	if(lst->next != nullptr){
		if(check_maxsize(lst))
			for(int i = 0; i < lst->maxsize; i++){

				fprintf(log_file, "\t\t\t next[%d] = %d\n", i, lst->next[i]);
			}
	}
	fflush(log_file);

	fprintf(log_file, "\t\t }\n"
			  "\t\t prev = [%p]{\n", lst->next);
	if(lst->prev != nullptr){
		if(check_maxsize(lst))
			for(int i = 0; i < lst->maxsize; i++){

				fprintf(log_file, "\t\t\t prev[%d] = %d\n", i, lst->prev[i]);
			}
	}
	fflush(log_file);
	
	fprintf(log_file, "\t\t canary_dynamic2 [%p] = " CANARY_PRINT "\n",  
		lst->canary_dynamic2, lst->canary_dynamic2 == nullptr ? 'NULLPTR' : lst->canary_dynamic2[0]);

	fprintf(log_file, "\n\t\t errnum   = ");
	for(int i = 0; i < MAX_ERROR_NO; i++){
		fprintf(log_file, "%d", (lst->errnum << i) % 2;
	}
	fprintf(log_file, "\n");
	fflush(log_file); 

	fprintf(log_file, "\n\t\t canary2 = " CANARY_PRINT "\n\n\t}\n\n",
		lst->canary2);

	fflush(log_file);
	/*

	Time and date: ....
	Dumping stack from file main.cpp, line 123, function check() [ok] ([ERROR!!!])
	Stack [0x1124asda] lst1 {
	
		Canary_t canary1 = ...	

		hash_t hash    = ....
		size_t size    = ....
		size_t maxsize = ....

		char * buff = [0x231141]
		Canary_t * canary_dynamic1 [0x1425123] = ...
		Elem_t *data = [0x23123]{
			* data[0] = ..
			  data[1] = ...  POISON?
			....
		}
		Canary_t * canary_dynamic2 [0x2131331] = ... 

		int errnum      = ...

		Canary_t canary2 = ...
	
	}

	*/

	fclose(log_file);

}

bool check_maxsize(List_t * const lst){//DONE
	
	if(lst == nullptr) return false;

	return lst->maxsize == ((Elem_t *)lst->canary_dynamic2) - lst->data; 

}

bool check_number_of_poison(List_t * const lst){//DONE

	if(!check_hashable(lst)) return false;

	size_t counter = 0;

	if(lst->data == nullptr){
		return false;	
	}
	
	for(size_t i = 0; i < lst->maxsize; i++){
		counter += lst->data[i] == POISON ? 1 : 0;
	}
	
	return counter - lst->false_poison == lst->maxsize - lst->size;
}

bool check_nullptr(const void * const lst){//DONE
	
	if(lst == nullptr){
		return false;
	}
	
	return true;
}

bool check_canary(List_t * const lst){//DONE

	if(lst == nullptr) return false;
	return lst->canary1 == lst->canary2 && lst->canary2 == CANARY_VALUE;
}

bool check_size_not_neg(const size_t size){//DONE

	return size >= 0;
}

bool check_hash(List_t * const lst){//DONE

	if(!check_hashable(lst)) return false;
	return is_equal_hash_t(find_sum_hash(lst, sizeof(List_t)), lst->hash);
}

bool check_hashable(List_t * const lst){//DONE
	
	return check_size(lst) && check_maxsize(lst) && check_size_not_neg(lst->false_poison);
}

bool check_dynamic_canaries_and_data(List_t * const lst){//DONE

	if(lst == nullptr) return false;
	if(lst->canary_dynamic1 == lst->canary_dynamic2) return false;
	if(lst->canary_dynamic1 == nullptr) return false;

	return !((char *)(lst->canary_dynamic2) - (char *)(lst->canary_dynamic1) != sizeof(Canary_t) + lst->maxsize*LIST_CLUSTER_SIZE 
		|| (char *)lst->data - (char *)lst->canary_dynamic1 != sizeof(Canary_t) 
		|| lst->canary_dynamic2[0] != lst->canary_dynamic1[0] 
		|| lst->canary_dynamic2[0] != CANARY_VALUE);

}

bool is_equal_hash_t(const hash_t left, const hash_t right){//DONE
	
	return left == right;
}

hash_t find_sum_hash(List_t * const lst, size_t size){//DONE

	if(lst == nullptr) return false;

	hash_t hash_sum = 0;
	
	hash_sum += hash(lst + sizeof(hash_t), size - sizeof(hash_t));
	hash_sum += hash(lst->buff, lst->buff == nullptr ? 0 : sizeof(lst->canary_dynamic1) + sizeof(lst->canary_dynamic2)
							       + LIST_CLUSTER_SIZE*lst->maxsize);
	hash_sum += hash(lst->name, lst->name == nullptr ? 0 : strlen(lst->name));

	return hash_sum;

}

hash_t hash(const void * const data, size_t size){//DONE

	if(data == nullptr) return false;

	hash_t hash_sum = 0;

	for(size_t counter = 0; counter < size; counter++){

		hash_sum = (hash_sum << 2) + (counter+1)*((*((char *) data + counter))^0xfc);
	}

	return hash_sum;

}

bool check_size(List_t * const lst){ //DONE
	
	if(lst == nullptr) return false;

	if(lst->size < 0 || lst-> size > lst->maxsize){
		return false;
	}

	return true;
}


