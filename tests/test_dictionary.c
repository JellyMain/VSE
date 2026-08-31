#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "VSE/dictionary.h"

#include <time.h>

// Test basic dictionary operations
static void TestBasicOperations()
{
	printf("=== Testing Basic VSE_Dictionary Operations ===\n");

	VSE_Dictionary *dict = VSE_DictionaryCreate(VSE_HashInt, VSE_IntEquals);
	assert(dict != NULL);
	printf("[OK] VSE_Dictionary creation successful\n");

	// Test adding items
	int key1 = 42;
	char *value1 = "Answer to everything";
	bool result = VSE_DictionaryAdd(dict, &key1, value1);
	assert(result == true);
	printf("[OK] Adding first item successful\n");

	int key2 = 100;
	char *value2 = "One hundred";
	result = VSE_DictionaryAdd(dict, &key2, value2);
	assert(result == true);
	printf("[OK] Adding second item successful\n");

	// Test retrieving items
	char *retrieved1 = (char *) VSE_DictionaryGet(dict, &key1);
	assert(retrieved1 != NULL);
	assert(strcmp(retrieved1, value1) == 0);
	printf("[OK] Retrieving first item: %s\n", retrieved1);

	char *retrieved2 = (char *) VSE_DictionaryGet(dict, &key2);
	assert(retrieved2 != NULL);
	assert(strcmp(retrieved2, value2) == 0);
	printf("[OK] Retrieving second item: %s\n", retrieved2);

	// Test key existence
	assert(VSE_DictionaryHasKey(dict, &key1) == true);
	assert(VSE_DictionaryHasKey(dict, &key2) == true);
	printf("[OK] Key existence check successful\n");

	// Test updating values
	char *newValue1 = "Updated answer";
	VSE_DictionaryChangeValue(dict, &key1, newValue1);
	char *updated = (char *) VSE_DictionaryGet(dict, &key1);
	assert(updated != NULL);
	assert(strcmp(updated, newValue1) == 0);
	printf("[OK] Value update successful: %s\n", updated);

	VSE_DictionaryDestroy(dict);
	printf("[OK] VSE_Dictionary destruction successful\n");
	printf("Basic operations test PASSED!\n\n");
}


// Test edge cases and error handling
static void TestEdgeCases()
{
	printf("=== Testing Edge Cases ===\n");

	VSE_Dictionary *dict = VSE_DictionaryCreate(VSE_HashInt, VSE_IntEquals);
	assert(dict != NULL);

	// Test with non-existent key
	int nonExistentKey = 999;
	char *result = (char *) VSE_DictionaryGet(dict, &nonExistentKey);
	assert(result == NULL);
	printf("[OK] Non-existent key returns NULL\n");

	// Test key existence for non-existent key
	assert(VSE_DictionaryHasKey(dict, &nonExistentKey) == false);
	printf("[OK] Non-existent key check returns false\n");

	// Test updating non-existent key
	VSE_DictionaryChangeValue(dict, &nonExistentKey, "Should not work");
	result = (char *) VSE_DictionaryGet(dict, &nonExistentKey);
	assert(result == NULL);
	printf("[OK] Updating non-existent key has no effect\n");

	// Test null dictionary operations
	result = (char *) VSE_DictionaryGet(NULL, &nonExistentKey);
	assert(result == NULL);

	bool keyExists = VSE_DictionaryHasKey(NULL, &nonExistentKey);
	assert(keyExists == false);

	// These should not crash
	VSE_DictionaryAdd(NULL, &nonExistentKey, "test");
	VSE_DictionaryChangeValue(NULL, &nonExistentKey, "test");
	VSE_DictionaryDestroy(NULL);
	printf("[OK] NULL dictionary operations handled gracefully\n");

	VSE_DictionaryDestroy(dict);
	printf("Edge cases test PASSED!\n\n");
}


// Test with string keys
static void TestStringKeys()
{
	printf("=== Testing String Keys ===\n");

	VSE_Dictionary *stringDict = VSE_DictionaryCreate(VSE_HashString, VSE_StringEquals);
	assert(stringDict != NULL);
	printf("[OK] String dictionary creation successful\n");

	// Add string key-value pairs
	char *key1 = "name";
	char *value1 = "John Doe";
	bool result = VSE_DictionaryAdd(stringDict, key1, value1);
	assert(result == true);

	char *key2 = "age";
	int *age = malloc(sizeof(int));
	*age = 30;
	result = VSE_DictionaryAdd(stringDict, key2, age);
	assert(result == true);

	char *key3 = "city";
	char *value3 = "New York";
	result = VSE_DictionaryAdd(stringDict, key3, value3);
	assert(result == true);

	printf("[OK] Added string key-value pairs\n");

	// Retrieve and verify
	char *retrievedName = (char *) VSE_DictionaryGet(stringDict, "name");
	assert(retrievedName != NULL);
	assert(strcmp(retrievedName, "John Doe") == 0);
	printf("[OK] Retrieved name: %s\n", retrievedName);

	int *retrievedAge = (int *) VSE_DictionaryGet(stringDict, "age");
	assert(retrievedAge != NULL);
	assert(*retrievedAge == 30);
	printf("[OK] Retrieved age: %d\n", *retrievedAge);

	char *retrievedCity = (char *) VSE_DictionaryGet(stringDict, "city");
	assert(retrievedCity != NULL);
	assert(strcmp(retrievedCity, "New York") == 0);
	printf("[OK] Retrieved city: %s\n", retrievedCity);

	// Test key existence
	assert(VSE_DictionaryHasKey(stringDict, "name") == true);
	assert(VSE_DictionaryHasKey(stringDict, "nonexistent") == false);
	printf("[OK] String key existence check successful\n");

	free(age);
	VSE_DictionaryDestroy(stringDict);
	printf("String keys test PASSED!\n\n");
}


// Test with larger dataset
static void TestLargeDataset()
{
	printf("=== Testing Large Dataset ===\n");

	VSE_Dictionary *dict = VSE_DictionaryCreate(VSE_HashInt, VSE_IntEquals);
	assert(dict != NULL);

	const int NUM_ITEMS = 1000;
	int *keys = malloc(sizeof(int) * NUM_ITEMS);
	char **values = malloc(sizeof(char *) * NUM_ITEMS);

	// Add many items
	printf("Adding %d items...\n", NUM_ITEMS);
	for (int i = 0; i < NUM_ITEMS; i++)
	{
		keys[i] = i;
		values[i] = malloc(50);
		sprintf(values[i], "Value_%d", i);

		bool result = VSE_DictionaryAdd(dict, &keys[i], values[i]);
		assert(result == true);
	}
	printf("[OK] Added %d items successfully\n", NUM_ITEMS);

	// Verify all items can be retrieved
	printf("Verifying all items...\n");
	for (int i = 0; i < NUM_ITEMS; i++)
	{
		char *retrieved = VSE_DictionaryGet(dict, &keys[i]);
		printf("[OK] Retrieved item %d: %s\n", i, retrieved);
		assert(retrieved != NULL);
		assert(strcmp(retrieved, values[i]) == 0);

		// Also check key existence
		assert(VSE_DictionaryHasKey(dict, &keys[i]) == true);
	}
	printf("[OK] All %d items retrieved successfully\n", NUM_ITEMS);

	// Test some updates
	printf("Testing updates on large dataset...\n");
	for (int i = 0; i < 100; i++)
	{
		char *newValue = malloc(50);
		sprintf(newValue, "Updated_Value_%d", i);
		VSE_DictionaryChangeValue(dict, &keys[i], newValue);

		char *retrieved = (char *) VSE_DictionaryGet(dict, &keys[i]);
		assert(retrieved != NULL);
		assert(strcmp(retrieved, newValue) == 0);

		free(values[i]); // Free old value
		values[i] = newValue; // Update reference
	}
	printf("[OK] Updated 100 items successfully\n");

	// Clean up
	for (int i = 0; i < NUM_ITEMS; i++)
	{
		free(values[i]);
	}
	free(keys);
	free(values);

	VSE_DictionaryDestroy(dict);
	printf("Large dataset test PASSED!\n\n");
}


// Test duplicate key handling
static void TestDuplicateKeys()
{
	printf("=== Testing Duplicate Key Handling ===\n");

	VSE_Dictionary *dict = VSE_DictionaryCreate(VSE_HashInt, VSE_IntEquals);
	assert(dict != NULL);

	int key = 42;
	char *value1 = "First value";
	char *value2 = "Second value";

	// Add first value
	bool result = VSE_DictionaryAdd(dict, &key, value1);
	assert(result == true);

	char *retrieved = (char *) VSE_DictionaryGet(dict, &key);
	assert(strcmp(retrieved, value1) == 0);
	printf("[OK] First value added: %s\n", retrieved);

	// Add duplicate key (this will create a second entry in the same bucket)
	result = VSE_DictionaryAdd(dict, &key, value2);
	assert(result == true);

	// The retrieved value should be one of them (implementation dependent)
	retrieved = (char *) VSE_DictionaryGet(dict, &key);
	assert(retrieved != NULL);
	printf("[OK] After duplicate add, retrieved: %s\n", retrieved);

	// Key should still exist
	assert(VSE_DictionaryHasKey(dict, &key) == true);
	printf("[OK] Key still exists after duplicate\n");

	VSE_DictionaryDestroy(dict);
	printf("Duplicate keys test PASSED!\n\n");
}


// Public function to run all dictionary tests
static void TestDictionary(void)
{
	printf("Starting VSE_Dictionary Tests...\n\n");

	TestBasicOperations();
	TestEdgeCases();
	TestStringKeys();
	TestLargeDataset();
	TestDuplicateKeys();

	printf("=== ALL TESTS PASSED! ===\n");
	printf("Your VSE_Dictionary implementation is working correctly!\n\n");
}


static void TestDictionaryPerformance()
{
	// Initialize dictionary
	VSE_Dictionary *dict = VSE_DictionaryCreate(VSE_HashInt, VSE_IntEquals);

	// Variables for timing
	clock_t start, end;
	double cpu_time_used;

	// Test sizes (increasing by factor of 10)
	int sizes[] = {100, 1000, 10000, 100000, 1000000};
	int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

	printf("VSE_Dictionary Performance Test\n");
	printf("==========================\n");

	for (int s = 0; s < num_sizes; s++)
	{
		int n = sizes[s];
		int *keys = malloc(n * sizeof(int));

		// Test insertion
		start = clock();
		for (int i = 0; i < n; i++)
		{
			keys[i] = i;
			VSE_DictionaryAdd(dict, &keys[i], &keys[i]);
		}
		end = clock();
		cpu_time_used = (double) (end - start) / CLOCKS_PER_SEC;
		printf("Insertion of %d items: %.8f seconds\n", n, cpu_time_used);

		// Test lookups
		start = clock();
		for (int i = 0; i < n; i++)
		{
			VSE_DictionaryGet(dict, &keys[i]);
		}
		end = clock();
		cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
		printf("Lookup of %d items: %.8f seconds\n", n, cpu_time_used);

		// Clean up for next iteration
		VSE_DictionaryDestroy(dict);
		dict = VSE_DictionaryCreate(VSE_HashInt, VSE_IntEquals);
		free(keys);
	}

	VSE_DictionaryDestroy(dict);
}


int main(void)
{
	TestDictionary();
	TestDictionaryPerformance();
	puts("test_dictionary: PASSED");
	return 0;
}
