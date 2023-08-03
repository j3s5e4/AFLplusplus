//ALL OF THIS IS AUTO GENERATED, DO NOT CHANGE. CHANGE interpreter.jinja.h and regenerate!
#ifndef __INTERPRETER__GEN__
#define __INTERPRETER__GEN__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" 

#include<stdint.h>
#include<stddef.h>

//includes

#include "custom_includes.h"

#include "nyx.h"





#define STATIC_ASSERT(cond, desc) _Static_assert(cond, desc)

#define INTERPRETER_CHECKSUM 9735510428121715531ULL




#define OP_PACKET 0
#define OP_PACKET_SIZE 1

#define OP_CREATE_TMP_SNAPSHOT 1
#define OP_CREATE_TMP_SNAPSHOT_SIZE 1


#include "data_include.h"

typedef struct {
	uint16_t* ops;
	size_t* ops_len;
	size_t ops_i;

	uint8_t* data;
	uint32_t* data_len;
	size_t data_i;
	uint32_t* instruction_counter;

	socket_state_t* user_data;

} interpreter_t;


#include "bytecode_spec.h"

//=========================
//atomic data type functions
//=========================

d_vec_pkt_content read_d_vec_pkt_content(interpreter_t* vm){
	d_vec_pkt_content res = {0};
	ASSERT(vm->data_i+2 <= *vm->data_len);
	uint16_t byte_size = *((uint16_t*)&vm->data[vm->data_i]);
	vm->data_i+=2;
	ASSERT(vm->data_i+byte_size <= *vm->data_len);
	res.count = ((size_t)byte_size)/sizeof(d_u8);
	res.vals = (d_u8*)&(vm->data[vm->data_i]);
	vm->data_i += byte_size;
	
	return res;
}


//=========================
//edge type functions
//=========================


//=========================
//interpreter functions
//=========================

interpreter_t* new_interpreter(){
	interpreter_t* vm = VM_MALLOC(sizeof(interpreter_t));
	vm->ops = 0;
	vm->ops_len = 0;
	vm->data = 0;
	vm->data_len = 0;
	vm->user_data = NULL;



	return vm;
}

void init_interpreter(interpreter_t* vm, uint16_t* ops, size_t* ops_len, uint8_t* data, uint32_t* data_len, uint32_t* instruction_counter){
	vm->ops=ops;
	vm->ops_len=ops_len;
	vm->ops_i = 0;
	vm->data = data;
	vm->data_i = 0;
	vm->data_len=data_len;
	vm->user_data = NULL;
	vm->instruction_counter = instruction_counter;
}

int interpreter_run(interpreter_t* vm) {
	ASSERT(vm->ops && vm->data); //init_interpreter was called previously
	while(vm->ops_i < *(vm->ops_len)) {
		uint16_t op = vm->ops[vm->ops_i];
		*vm->instruction_counter+=1;
		switch(op){
			

			case OP_PACKET: {
				ASSERT( *vm->ops_len >= vm->ops_i + OP_PACKET_SIZE );d_vec_pkt_content data = read_d_vec_pkt_content(vm);
				
				handler_packet(vm, &data);
				vm->ops_i += OP_PACKET_SIZE;return 1;
				}
			case OP_CREATE_TMP_SNAPSHOT: {
				ASSERT( *vm->ops_len >= vm->ops_i + OP_CREATE_TMP_SNAPSHOT_SIZE );
				handler_create_tmp_snapshot(vm);
				vm->ops_i += OP_CREATE_TMP_SNAPSHOT_SIZE;
				break;
				}
			default:
					ASSERT(0);
		}
	}
	return 0;
}
#pragma GCC diagnostic pop 
#endif