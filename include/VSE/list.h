#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "VSE/fwd.h"

#define VSE_DEFAULT_CAPACITY 10
#define VSE_GROWTH_FACTOR 2

typedef struct VSE_List
{
	void **elements;
	int size;
	int capacity;
} VSE_List;


/** Creates a dynamic array of borrowed void pointers.
 *  @param capacity initial slots, or 0 for the default of 10
 *  @return the list, or NULL if allocation failed */
VSE_List *VSE_ListCreate(int capacity);

/** Frees the list and its buffer. Does NOT free what the elements point at. */
void VSE_ListDestroy(VSE_List *list);

/** Appends an element, growing the buffer if needed. @return false on allocation failure. */
bool VSE_ListAdd(VSE_List *list, void *element);

/** @return the element at index, or NULL if the list is NULL or the index is out of range. */
void *VSE_ListGet(VSE_List *list, int index);

/** Replaces the element at index. @return false if the index is out of range. */
bool VSE_ListSet(VSE_List *list, int index, void *element);

/** Removes by index, shifting later elements down to preserve order. */
bool VSE_ListRemoveAtIndex(VSE_List *list, int index);

/** Removes the first element equal to this pointer. @return false if not present. */
bool VSE_ListRemove(VSE_List *list, void *element);

/** @return the number of elements. */
int VSE_ListGetSize(VSE_List *list);

/** @return true when the list holds no elements. */
bool VSE_ListIsEmpty(VSE_List *list);

/** Removes every element but keeps the allocated buffer. */
void VSE_ListClear(VSE_List *list);
