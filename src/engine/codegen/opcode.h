#ifndef TULA_ENGINE_CODEGEN_OPCODE_H
#define TULA_ENGINE_CODEGEN_OPCODE_H

#include <stdint.h>


/*
 * def(
 *		identifier,		<-	The identifier used to define the opcode
 *		mnemonic,		<-	The string of the mnemonic representation of opcode
 *		value,			<-	The hex value of the opcode
 *		size,			<-	The total size of bytes for the opcode (counting the opcode itself)
 *	)
 */
#define DEFINE_OPCODES(def)								\
	def(OP_NOP,      	"nop",			0x00,   1)  	\
	def(OP_HALT,     	"halt",			0x01,   1)  	\
	def(OP_RESERVED, 	"reserved",		0x02,   1)  	\
	def(OP_POP,      	"pop",			0x03,   1)  	\
	def(OP_POP_2,     	"pop2",			0x04,   1)  	\
	def(OP_POP_4,     	"pop4",			0x05,   1)  	\
	def(OP_POP_8,     	"pop8",			0x06,   1)  	\
	def(OP_PUSH,     	"push",			0x07,   2)  	\
	def(OP_PUSH_2,    	"push2",		0x08,   3)  	\
	def(OP_PUSH_4,    	"push4",		0x09,   5)  	\
	def(OP_PUSH_8,    	"push8",		0x0A,   9)  	\
	def(OP_DUP,      	"dup",			0x0B,   1)  	\
	def(OP_DUP_2,     	"dup2",			0x0C,   1)  	\
	def(OP_DUP_4,     	"dup4",			0x0D,   1)  	\
	def(OP_DUP_8,     	"dup8",			0x0E,   1)  	\
	def(OP_G_SET_R,    	"gsetr",		0x0F,   10)  	\
	def(OP_G_SET_B,    	"gsetb",		0x10,   3)  	\
	def(OP_G_SET_8,    	"gset8",		0x11,   3)  	\
	def(OP_G_SET_16,   	"gset16",		0x12,   4)  	\
	def(OP_G_SET_32,   	"gset32",		0x13,   6)  	\
	def(OP_G_SET_64,   	"gset64",		0x14,   10)  	\
	def(OP_G_SET_F,    	"gsetf",		0x15,   10)  	\
	def(OP_G_SET_D,    	"gsetd",		0x16,   10)  	\
	def(OP_G_UNSET,   	"gunset",		0x17,   2)  	\
	def(OP_G_ISSET,   	"gisset",		0x18,   2)  	\
	def(OP_L_SET_R,    	"lsetr",		0x19,   10)  	\
	def(OP_L_SET_B,    	"lsetb",		0x1A,   3)  	\
	def(OP_L_SET_8,    	"lset8",		0x1B,   3)  	\
	def(OP_L_SET_16,   	"lset16",		0x1C,   4)  	\
	def(OP_L_SET_32,   	"lset32",		0x1D,   6)  	\
	def(OP_L_SET_64,   	"lset64",		0x1E,   10)  	\
	def(OP_L_SET_F,    	"lsetf",		0x1F,   10)  	\
	def(OP_L_SET_D,    	"lsetd",		0x20,   10)  	\
	def(OP_L_UNSET,   	"lunset",		0x21,   2)  	\
	def(OP_L_ISSET,   	"lisset",		0x22,   2)  	\
	def(OP_G_LOAD_R,   	"gloadr",		0x23,   2)  	\
	def(OP_G_LOAD_B,   	"gloadb",		0x24,   2)  	\
	def(OP_G_LOAD_8,   	"gload8",		0x25,   2)  	\
	def(OP_G_LOAD_16,  	"gload16",		0x26,   2)  	\
	def(OP_G_LOAD_32,  	"gload32",		0x27,   2)  	\
	def(OP_G_LOAD_64,  	"gload64",		0x28,   2)  	\
	def(OP_G_LOAD_F,   	"gloadf",		0x29,   2)  	\
	def(OP_G_LOAD_D,   	"gloadd",		0x2A,   2)  	\
	def(OP_L_LOAD_R,   	"lloadr",		0x2B,   2)  	\
	def(OP_L_LOAD_B,   	"lloadb",		0x2C,   2)  	\
	def(OP_L_LOAD_8,   	"lload8",		0x2D,   2)  	\
	def(OP_L_LOAD_16,  	"lload16",		0x2E,   2)  	\
	def(OP_L_LOAD_32,  	"lload32",		0x2F,   2)  	\
	def(OP_L_LOAD_64,  	"lload64",		0x30,   2)  	\
	def(OP_L_LOAD_F,   	"lloadf",		0x31,   2)  	\
	def(OP_L_LOAD_D,   	"lloadd",		0x32,   2)  	\


typedef enum tula_opcode_type
{
#define OPCODES_ENUM_DEFINER(identifier, _1, value, _3) identifier = value,
	DEFINE_OPCODES(OPCODES_ENUM_DEFINER)
#undef OPCODES_ENUM_DEFINER
	TOTAL_OPCODES
} opcode_type_t;


extern const char* OPCODES_MNEMONIC[TOTAL_OPCODES];

extern const int32_t OPCODES_SIZE[TOTAL_OPCODES];



#endif /* TULA_ENGINE_CODEGEN_OPCODE_H */
