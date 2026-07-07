#ifndef TULA_ENGINE_PARSER_AST_H
#define TULA_ENGINE_PARSER_AST_H

#include <stdint.h>

#define ast_is_type(value, _type)	((value).type == _type)
#define ast_is_unset(value)			ast_is_type(value, AST_UNSET)
#define ast_is_boolean(value)		ast_is_type(value, AST_BOOLEAN)
#define ast_is_character(value) 	ast_is_type(value, AST_CHARACTER)
#define ast_is_int8(value) 			ast_is_type(value, AST_INT8)
#define ast_is_uint8(value) 		ast_is_type(value, AST_UINT8)
#define ast_is_int16(value) 		ast_is_type(value, AST_INT16)
#define ast_is_uint16(value) 		ast_is_type(value, AST_UINT16)
#define ast_is_int32(value) 		ast_is_type(value, AST_INT32)
#define ast_is_uint32(value) 		ast_is_type(value, AST_UINT32)
#define ast_is_int64(value) 		ast_is_type(value, AST_INT64)
#define ast_is_uint64(value) 		ast_is_type(value, AST_UINT64)
#define ast_is_float(value) 		ast_is_type(value, AST_FLOAT)
#define ast_is_double(value) 		ast_is_type(value, AST_DOUBLE)

#define ast_as_type(value, field)	((value).as.field)
#define ast_as_unset(value)			ast_as_type(value, unset)
#define ast_as_boolean(value)		ast_as_type(value, boolean)
#define ast_as_character(value) 	ast_as_type(value, character)
#define ast_as_int8(value) 			ast_as_type(value, int8)
#define ast_as_uint8(value) 		ast_as_type(value, uint8)
#define ast_as_int16(value) 		ast_as_type(value, int16)
#define ast_as_uint16(value) 		ast_as_type(value, uint16)
#define ast_as_int32(value) 		ast_as_type(value, int32)
#define ast_as_uint32(value) 		ast_as_type(value, uint32)
#define ast_as_int64(value) 		ast_as_type(value, int64)
#define ast_as_uint64(value) 		ast_as_type(value, uint64)
#define ast_as_float(value) 		ast_as_type(value, decimalFloat)
#define ast_as_double(value) 		ast_as_type(value, decimalDouble)

#define ast_cast(_type, field, value) \
	((ast_node_t){ _type, { .field = value } })
#define ast_unset()				ast_cast(AST_UNSET, unset, 0)
#define ast_boolean(value)		ast_cast(AST_BOOLEAN, boolean, value)
#define ast_character(value) 	ast_cast(AST_CHARACTER, character, value)
#define ast_int8(value) 		ast_cast(AST_INT8, int8, value)
#define ast_uint8(value) 		ast_cast(AST_UINT8, uint8, value)
#define ast_int16(value) 		ast_cast(AST_INT16, int16, value)
#define ast_uint16(value) 		ast_cast(AST_UINT16, uint16, value)
#define ast_int32(value) 		ast_cast(AST_INT32, int32, value)
#define ast_uint32(value) 		ast_cast(AST_UINT32, uint32, value)
#define ast_int64(value) 		ast_cast(AST_INT64, int64, value)
#define ast_uint64(value) 		ast_cast(AST_UINT64, uint64, value)
#define ast_float(value) 		ast_cast(AST_FLOAT, decimalFloat, value)
#define ast_double(value) 		ast_cast(AST_DOUBLE, decimalDouble, value)



typedef enum tula_ast_tag
{
	AST_UNSET,
	AST_BOOLEAN,
	AST_CHARACTER,
	AST_INT8,
	AST_UINT8,
	AST_INT16,
	AST_UINT16,
	AST_INT32,
	AST_UINT32,
	AST_INT64,
	AST_UINT64,
	AST_FLOAT,
	AST_DOUBLE,
} ast_tag_t;


typedef struct tula_ast_node
{
	ast_tag_t tag;
	union
	{
		uint8_t boolean: 1;
		int8_t int8;
		uint8_t uint8;
		int16_t int16;
		uint16_t uint16;
		int32_t int32;
		uint32_t uint32;
		int64_t int64;
		uint64_t uint64;
		float decimalFloat;
		double decimalDouble;
	} as;
} ast_node_t;


typedef struct tula_arr_ast_node_t
{
	/**
	 * \brief	The amount of elements currently in the array.
	 */
	size_t count;

	/**
	 * \brief	The amount of elements the array can currently accommodate.
	 */
	size_t capacity;

	/**
	 * \brief	Pointer to the first element of the array.
	 */
	ast_node_t* nodes;
} arr_ast_node_t;


ast_node_t* ast_node_new(ast_tag_t tag)


/**
 * \brief           Initializes the provided array
 * \param[in]       array: Pointer to the array to initialize
 */
void arr_ast_node_init(arr_ast_node_t* array);


/**
 * \brief           Releases the provided array
 * \param[in]       array: Pointer to the array to free
 */
void arr_ast_node_destroy(arr_ast_node_t* array);


/**
 * \brief           Writes the provided AST Node to the array
 * \param[in]       array: Pointer to the array to write to
 * \param[in]       value: The node to write to the array
 */
void arr_ast_node_add(arr_ast_node_t* array, const arr_ast_node_t* value);

#endif /* TULA_ENGINE_PARSER_AST_H */
