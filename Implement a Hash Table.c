#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 16
#define LOAD_FACTOR_THRESHOLD 0.7

// Structure for key-value pairs
typedef struct {
    char* key;
    char* value;
} KeyValuePair;

// Structure for a bucket in the hash table
typedef struct {
    KeyValuePair** data;
    int size;
    int count;
} Bucket;

// Structure for the hash table
typedef struct {
    Bucket** table;
    int size;
    int count;
} HashTable;

// Basic hash function: sum of ASCII values
unsigned int hash(char* key, int table_size) {
    unsigned int hash_value = 0;
    while (*key) {
        hash_value += (unsigned int)(*key);
        key++;
    }
    return hash_value % table_size;
}

// Initialize a new key-value pair
KeyValuePair* create_pair(char* key, char* value) {
    KeyValuePair* pair = (KeyValuePair*)malloc(sizeof(KeyValuePair));
    pair->key = strdup(key);
    pair->value = strdup(value);
    return pair;
}

// Initialize a new bucket
Bucket* create_bucket() {
    Bucket* bucket = (Bucket*)malloc(sizeof(Bucket));
    bucket->size = INITIAL_SIZE;
    bucket->data = (KeyValuePair**)malloc(sizeof(KeyValuePair*) * bucket->size);
    bucket->count = 0;
    return bucket;
}

// Initialize a new hash table
HashTable* create_table() {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    table->size = INITIAL_SIZE;
    table->table = (Bucket**)malloc(sizeof(Bucket*) * table->size);
    table->count = 0;
    return table;
}

// Insert a key-value pair into the hash table
void insert(HashTable* table, char* key, char* value) {
    unsigned int index = hash(key, table->size);
    Bucket* bucket = table->table[index];

    if (bucket == NULL) {
        // Create a new bucket if it doesn't exist
        bucket = create_bucket();
        table->table[index] = bucket;
    }

    // Create a new key-value pair
    KeyValuePair* pair = create_pair(key, value);

    // Add the pair to the bucket
    bucket->data[bucket->count++] = pair;

    // Check if resizing is needed
    if ((float)table->count / table->size > LOAD_FACTOR_THRESHOLD) {
        // Resize the hash table
        // (Note: Resizing code is not included in this simplified example)
    }

    table->count++;
}

// Get the value associated with a key
char* get(HashTable* table, char* key) {
    unsigned int index = hash(key, table->size);
    Bucket* bucket = table->table[index];

    if (bucket != NULL) {
        for (int i = 0; i < bucket->count; i++) {
            if (strcmp(bucket->data[i]->key, key) == 0) {
                return bucket->data[i]->value;
            }
        }
    }

    return NULL;
}

// Remove a key-value pair from the hash table
void remove_pair(HashTable* table, char* key) {
    unsigned int index = hash(key, table->size);
    Bucket* bucket = table->table[index];

    if (bucket != NULL) {
        for (int i = 0; i < bucket->count; i++) {
            if (strcmp(bucket->data[i]->key, key) == 0) {
                free(bucket->data[i]->key);
                free(bucket->data[i]->value);
                free(bucket->data[i]);

                // Shift elements to fill the gap
                for (int j = i; j < bucket->count - 1; j++) {
                    bucket->data[j] = bucket->data[j + 1];
                }
                bucket->count--;

                // Check if the bucket is empty after removal
                if (bucket->count == 0) {
                    free(bucket->data);
                    free(bucket);
                    table->table[index] = NULL;
                }

                table->count--;

                return;
            }
        }
    }
}

// Clean up and free memory for the hash table
void destroy_table(HashTable* table) {
    for (int i = 0; i < table->size; i++) {
        Bucket* bucket = table->table[i];
        if (bucket != NULL) {
            for (int j = 0; j < bucket->count; j++) {
                free(bucket->data[j]->key);
                free(bucket->data[j]->value);
                free(bucket->data[j]);
            }
            free(bucket->data);
            free(bucket);
        }
    }
    free(table->table);
    free(table);
}

int main() {
    HashTable* table = create_table();

    // Insert key-value pairs
    insert(table, "key1", "value1");
    insert(table, "key2", "value2");

    // Get values by keys
    printf("Value for key1: %s\n", get(table, "key1"));  // Should print "value1"
    printf("Value for key2: %s\n", get(table, "key2"));  // Should print "value2"

    // Remove a key-value pair
    remove_pair(table, "key1");

    // Check if key1 has been removed
    printf("Value for key1 after removal: %s\n", get(table, "key1"));  // Should print "(null)"

    // Clean up and free memory
    destroy_table(table);

    return 0;
}
