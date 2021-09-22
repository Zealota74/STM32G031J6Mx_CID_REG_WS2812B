/*
 * test.h
 *
 *  Created on: 20 sie 2021
 *      Author: szymon.wroblewski
 */

#ifndef LIBS_TEST_H_
#define LIBS_TEST_H_


// Only simple labels -----------------------------------
#define SIMPLE_MACRO1	1
#define SIMPLE_MACRO2	2
#define SIMPLE_MACRO3	3
#define SIMPLE_MACRO4	4

enum {
	enum1, enum2, enum3
};
//-------------------------------------------------------


// New type definitions ---------------------------------
typedef struct {
	int  a;
	char b;
} new_type_t;
// ------------------------------------------------------


// "Public" external functions to operate between .c files
extern  int get_variable(void);
extern  void set_variable(int value);
extern uint8_t * get_pointer(void);
extern new_type_t * struct_init(void) ;
// ------------------------------------------------------

#endif /* LIBS_TEST_H_ */
