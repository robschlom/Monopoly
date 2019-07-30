#ifndef BOARD_H
#define BOARD_H

#include "property.h"
int createBoard(FILE *fp, Property **propListPtr);
void printProperty(Property p);

#endif