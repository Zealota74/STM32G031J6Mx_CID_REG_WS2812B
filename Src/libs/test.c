/*
 * test.c
 *
 *  Created on: 20 sie 2021
 *      Author: szymon.wroblewski
 */

// Headers for build-in GCC functions, headers
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// User headers
#include "test.h"
// --------------------------------------------------------------

// Variables, always static, invisible in another .c files
static uint8_t		table[10];
static int 			variable;
static new_type_t	variable2;
// --------------------------------------------------------------



// Usable function to operate on variable in another
int get_variable(void) {
	return variable;
}
void set_variable(int value) {
	variable = value;
}
// --------------------------------------------------------------



// Static function, unnecessary in another .c files
static void set_memory(uint8_t pattern) {
	memset( (uint8_t *)table, (uint8_t)pattern, sizeof(table) );
}
// --------------------------------------------------------------



// External function, visible in another .c files
void set_table_pattern( uint8_t pattern ) {
	set_memory( pattern );
}
uint8_t * get_pointer(void) {
	return (uint8_t *)table;
}

new_type_t * struct_init(void) {
	variable2.a = 5;
	variable2.b = 10;
	return (new_type_t * )&variable2;
}
// -------------------------------------------------------------
