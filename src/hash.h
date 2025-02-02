#ifndef PICO_HASH_H
#define PICO_HASH_H

#include <stdlib.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Hash
 * @brief Hashing functionality used by pico to index assets.
 * @{
 */

struct pico_hash;
/**
 * @brief Alias to a hidden struct. Access done via functions.
 */
typedef struct pico_hash pico_hash;

/**
 * @brief Creates a new hash table.
 * @param num_buckets: 
 * @return A pointer to the new hash table
 */
pico_hash* pico_hash_create (size_t num_buckets);

/**
 * @brief Destroys a given hash table.
 * @param table: pointer to the hash table to destroy
 */
void pico_hash_destroy (pico_hash* table);

/**
 * @brief Adds a value to the hash table with the given key.
 * If the given key already exists, the value is replaced.
 * @param table: pointer to the hash table
 * @param key: the key to use for the value
 * @param value: the value to add to the hash table
 * @return 1 if the value was added successfully, or 0 if an error occurred
 */
int pico_hash_add (pico_hash* table, const char* key, void* value);

/**
 * @brief Removes a value from the hash table with the given key.
 * @param table: pointer to the hash table
 * @param key: the key of the value to remove
 * @return 1 if the value was removed successfully, or 0 if an error occurred.
 */
int pico_hash_rem (pico_hash* table, const char* key);

/**
 * @brief Returns the value associated with the given key in the hash table.
 * @param table: pointer to the hash table
 * @param key: the key of the value to retrieve
 * @return the value associated with the given key in the hash table, or NULL if the key does not exist.
 */
void* pico_hash_get (pico_hash* table, const char* key);

/** @} */

#ifdef __cplusplus
}
#endif

#endif // PICO_HASH_H
