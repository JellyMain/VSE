#include "VSE/dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_BUCKETS_NUMBER 101
#define MAX_LOAD_FACTOR 0.75

static void DictionaryCheckAndResize(VSE_Dictionary *dict);


VSE_Dictionary *VSE_DictionaryCreate(unsigned int (*hashFunction)(void *key), bool (*keyEquals)(void *key1, void *key2))
{
	VSE_Dictionary *dict = calloc(1, sizeof(VSE_Dictionary));

	if (dict == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for dictionary\n");
		return NULL;
	}

	VSE_List **buckets = calloc(DEFAULT_BUCKETS_NUMBER, sizeof(VSE_List));

	if (buckets == NULL)
	{
		free(dict);
		fprintf(stderr, "Failed to allocate memory for dictionary pairs\n");
		return NULL;
	}


	for (int i = 0; i < DEFAULT_BUCKETS_NUMBER; i++)
	{
		VSE_List *bucket = VSE_ListCreate(0);

		if (bucket == NULL)
		{
			for (int j = 0; j < DEFAULT_BUCKETS_NUMBER; j++)
			{
				VSE_ListDestroy(buckets[j]);
			}

			fprintf(stderr, "Failed to allocate memory for dictionary buckets\n");
			free(buckets);
			return NULL;
		}

		buckets[i] = bucket;
	}

	dict->allPairs = VSE_ListCreate(0);
	dict->bucketsNumber = DEFAULT_BUCKETS_NUMBER;
	dict->buckets = buckets;
	dict->hashFunction = hashFunction;
	dict->keyEquals = keyEquals;
	dict->totalEntries = 0;

	return dict;
}


bool VSE_DictionaryAdd(VSE_Dictionary *dict, void *key, void *value)
{
	if (dict == NULL)
	{
		fprintf(stderr, "VSE_Dictionary is NULL\n");
		return false;
	}

	if (key == NULL)
	{
		fprintf(stderr, "Key is NULL\n");
		return false;
	}

	if (value == NULL)
	{
		fprintf(stderr, "Value is NULL\n");
		return false;
	}

	unsigned int hash = dict->hashFunction(key);

	int bucketIndex = hash % dict->bucketsNumber;

	VSE_KeyValuePair *pair = calloc(1, sizeof(VSE_KeyValuePair));
	pair->key = key;
	pair->value = value;
	dict->totalEntries++;

	VSE_List *bucket = dict->buckets[bucketIndex];


	if (!VSE_ListAdd(bucket, pair))
	{
		fprintf(stderr, "Failed to add pair to dictionary\n");
		free(pair);
		return false;
	}

	if (bucket->size > 1)
	{
		DictionaryCheckAndResize(dict);
	}

	VSE_ListAdd(dict->allPairs, pair);

	return true;
}


void VSE_DictionaryDestroy(VSE_Dictionary *dict)
{
	if (dict == NULL)
	{
		return;
	}

	for (int i = 0; i < dict->bucketsNumber; i++)
	{
		VSE_List *bucket = dict->buckets[i];
		for (int j = 0; j < bucket->size; j++)
		{
			VSE_KeyValuePair *pair = bucket->elements[j];
			free(pair);
		}

		VSE_ListDestroy(bucket);
	}

	VSE_ListDestroy(dict->allPairs);

	free(dict->buckets);
	free(dict);
}


bool VSE_DictionaryHasKey(VSE_Dictionary *dict, void *key)
{
	if (dict == NULL)
	{
		fprintf(stderr, "VSE_Dictionary is NULL\n");
		return false;
	}

	if (key == NULL)
	{
		fprintf(stderr, "Key is NULL\n");
		return false;
	}

	unsigned int hash = dict->hashFunction(key);

	int bucketIndex = hash % dict->bucketsNumber;

	VSE_List *bucket = dict->buckets[bucketIndex];

	for (int i = 0; i < bucket->size; i++)
	{
		VSE_KeyValuePair *pair = bucket->elements[i];

		if (dict->keyEquals(pair->key, key))
		{
			return true;
		}
	}

	return false;
}


static bool ResizeDictionary(VSE_Dictionary *dict, int newSize)
{
	if (dict == NULL)
	{
		fprintf(stderr, "VSE_Dictionary is NULL\n");
		return false;
	}

	VSE_List **newBuckets = malloc(sizeof(VSE_List) * newSize);

	if (newBuckets == NULL)
	{
		fprintf(stderr, "Failed to reallocate memory for dictionary buckets\n");
		return false;
	}

	for (int i = 0; i < newSize; i++)
	{
		newBuckets[i] = VSE_ListCreate(0);

		if (newBuckets[i] == NULL)
		{
			for (int j = 0; j < newSize; j++)
			{
				VSE_ListDestroy(newBuckets[j]);
			}

			fprintf(stderr, "Failed to allocate memory for dictionary buckets\n");
			free(newBuckets);
			return false;
		}
	}

	for (int i = 0; i < dict->bucketsNumber; i++)
	{
		VSE_List *bucket = dict->buckets[i];

		for (int j = 0; j < bucket->size; j++)
		{
			VSE_KeyValuePair *pair = bucket->elements[j];

			unsigned int hash = dict->hashFunction(pair->key);
			unsigned int newBucketIndex = hash % newSize;

			VSE_ListAdd(newBuckets[newBucketIndex], pair);
		}

		VSE_ListDestroy(bucket);
	}

	free(dict->buckets);

	dict->buckets = newBuckets;
	dict->bucketsNumber = newSize;

	return true;
}


static void DictionaryCheckAndResize(VSE_Dictionary *dict)
{
	if (dict == NULL)
	{
		fprintf(stderr, "VSE_Dictionary is NULL\n");
		return;
	}

	float filledBucketsPercentage = (float) dict->totalEntries / (float) dict->bucketsNumber;

	if (filledBucketsPercentage >= MAX_LOAD_FACTOR)
	{
		ResizeDictionary(dict, dict->bucketsNumber * 2);
	}
}


void VSE_DictionaryChangeValue(VSE_Dictionary *dict, void *key, void *value)
{
	if (dict == NULL)
	{
		fprintf(stderr, "VSE_Dictionary is NULL\n");
		return;
	}

	if (key == NULL)
	{
		fprintf(stderr, "Key is NULL\n");
		return;
	}

	unsigned int hash = dict->hashFunction(key);

	int bucketIndex = hash % dict->bucketsNumber;

	VSE_List *bucket = dict->buckets[bucketIndex];

	for (int i = 0; i < bucket->size; i++)
	{
		VSE_KeyValuePair *pair = bucket->elements[i];

		if (dict->keyEquals(pair->key, key))
		{
			pair->value = value;
			return;
		}
	}
}


void *VSE_DictionaryGet(VSE_Dictionary *dict, void *key)
{
	if (dict == NULL)
	{
		fprintf(stderr, "VSE_Dictionary is NULL\n");
		return NULL;
	}

	if (key == NULL)
	{
		fprintf(stderr, "Key is NULL\n");
		return NULL;
	}

	unsigned int hash = dict->hashFunction(key);
	unsigned int bucketIndex = hash % dict->bucketsNumber;

	VSE_List *bucket = dict->buckets[bucketIndex];

	for (int i = 0; i < bucket->size; i++)
	{
		VSE_KeyValuePair *pair = bucket->elements[i];

		if (dict->keyEquals(pair->key, key))
		{
			return pair->value;
		}
	}

	fprintf(stderr, "Key not found\n");
	return NULL;
}


VSE_KeyValuePair *VSE_DictionaryGetPair(VSE_Dictionary *dict, int index)
{
	if (dict == NULL)
	{
		fprintf(stderr, "VSE_Dictionary is NULL\n");
		return NULL;
	}

	return VSE_ListGet(dict->allPairs, index);
}


void VSE_DictionaryRemove(VSE_Dictionary *dict, void *key)
{
	if (dict == NULL)
	{
		fprintf(stderr, "VSE_Dictionary is NULL\n");
		return;
	}

	if (key == NULL)
	{
		fprintf(stderr, "Key is NULL\n");
		return;
	}

	unsigned int hash = dict->hashFunction(key);
	unsigned int bucketIndex = hash % dict->bucketsNumber;

	VSE_List *bucket = dict->buckets[bucketIndex];

	for (int i = 0; i < bucket->size; i++)
	{
		VSE_KeyValuePair *pair = bucket->elements[i];

		if (dict->keyEquals(pair->key, key))
		{
			VSE_ListRemoveAtIndex(bucket, i);

			for (int j = 0; j < dict->allPairs->size; j++)
			{
				if (dict->allPairs->elements[j] == pair)
				{
					VSE_ListRemoveAtIndex(dict->allPairs, j);
					break;
				}
			}

			free(pair);
			dict->totalEntries--;
			return;
		}
	}

	fprintf(stderr, "Key not found in dictionary\n");
}


void VSE_DictionaryClear(VSE_Dictionary *dict)
{
	if (dict == NULL)
	{
		fprintf(stderr, "VSE_Dictionary is NULL\n");
		return;
	}

	for (int i = 0; i < dict->allPairs->size; i++)
	{
		VSE_KeyValuePair *pair = dict->allPairs->elements[i];
		free(pair);
	}

	for (int i = 0; i < dict->bucketsNumber; i++)
	{
		VSE_ListClear(dict->buckets[i]);
	}

	VSE_ListClear(dict->allPairs);
	dict->totalEntries = 0;
}


unsigned int VSE_HashInt(void *key)
{
	int keyValue = *(int *) key;
	unsigned int hash = keyValue * 2654435761u;
	return hash;
}


bool VSE_IntEquals(void *key1, void *key2)
{
	return *(int *) key1 == *(int *) key2;
}


unsigned int VSE_HashPointer(void *key)
{
	uintptr_t address = (uintptr_t) key;
	unsigned int hash = address * 2654435761u;
	return hash;
}


bool VSE_PointerEquals(void *key1, void *key2)
{
	return (uintptr_t) key1 == (uintptr_t) key2;
}


unsigned int VSE_HashString(void *key)
{
	unsigned long hash = 5381;
	int c;
	char *str = key;

	while ((c = *str++))
	{
		hash = (hash << 5) + hash + c;
	}

	return (unsigned int) hash;
}


bool VSE_StringEquals(void *key1, void *key2)
{
	return strcmp(key1, key2) == 0;
}