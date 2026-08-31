#pragma once
#include <stdbool.h>
#include "VSE/fwd.h"
#include "VSE/list.h"

typedef struct VSE_KeyValuePair
{
	void *key;
	void *value;
} VSE_KeyValuePair;


typedef struct VSE_Dictionary
{
	VSE_List **buckets;

	int bucketsNumber;

	unsigned int totalEntries;

	unsigned int (*hashFunction)(void *key);

	bool (*keyEquals)(void *key1, void *key2);

	VSE_List *allPairs;
} VSE_Dictionary;


/** Creates a hash map with separate chaining.
 *  @param hashFunction e.g. VSE_HashString, VSE_HashInt, VSE_HashPointer
 *  @param keyEquals the matching comparison for that key kind */
VSE_Dictionary *VSE_DictionaryCreate(unsigned int (*hashFunction)(void *key), bool (*keyEquals)(void *key1, void *key2));

/** @return the value for this key, or NULL if absent. */
void *VSE_DictionaryGet(VSE_Dictionary *dict, void *key);

/** @return the key/value pair at this insertion index, for ordered iteration
 *  over `allPairs`. */
VSE_KeyValuePair *VSE_DictionaryGetPair(VSE_Dictionary *dict, int index);

/** Inserts a key/value pair. Both pointers are borrowed, never freed by the map. */
bool VSE_DictionaryAdd(VSE_Dictionary *dict, void *key, void *value);

/** Hash for keys pointing at an int. Pair with VSE_IntEquals. */
unsigned int VSE_HashInt(void *key);

/** Compares two int keys by value. */
bool VSE_IntEquals(void *key1, void *key2);

/** Hash for keys used by identity. Pair with VSE_PointerEquals. */
unsigned int VSE_HashPointer(void *key);

/** Compares two keys by address. */
bool VSE_PointerEquals(void *key1, void *key2);

/** Hash for NUL-terminated string keys. Pair with VSE_StringEquals. */
unsigned int VSE_HashString(void *key);

/** Compares two string keys by content. */
bool VSE_StringEquals(void *key1, void *key2);

/** @return true when the key is present. */
bool VSE_DictionaryHasKey(VSE_Dictionary *dict, void *key);

/** Frees the map and its buckets. Frees neither keys nor values. */
void VSE_DictionaryDestroy(VSE_Dictionary *dict);

/** Replaces the value stored under an existing key. */
void VSE_DictionaryChangeValue(VSE_Dictionary *dict, void *key, void *value);

/** Removes the entry for this key. Frees neither key nor value. */
void VSE_DictionaryRemove(VSE_Dictionary *dict, void *key);

/** Removes every entry, keeping the bucket array. */
void VSE_DictionaryClear(VSE_Dictionary *dict);
