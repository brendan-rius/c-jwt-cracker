#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <stdbool.h>
#include <pthread.h>
#include "base64.h"

char *g_header_b64 = NULL; // Holds the Base64 header of the original JWT
char *g_payload_b64 = NULL; // Holds the Base64 payload of the original JWT
char *g_signature_b64 = NULL; // Holds the Base64 signature of the original JWT
unsigned char *g_to_encrypt = NULL; // Holds the part of the JWT that needs to be hashed
unsigned char *g_signature = NULL; // Holds the Base64 *decoded* signature of the original JWT

// Some lengths of buffers. Useful to avoid computing it multiple times.
// Also, not all strings used finish with a '\0' for optimization purposes.
// In that case, we need to have the length
size_t g_header_b64_len = 0;
size_t g_payload_b64_len = 0;
size_t g_signature_b64_len = 0;
size_t g_signature_len = 0;
size_t g_to_encrypt_len = 0;

// The alphabet to use when brute-forcing
char *g_alphabet = NULL;
size_t g_alphabet_len = 0;

char *g_found_secret = NULL;

struct s_thread_data {
    EVP_MD *g_evp_md; // The hash function to apply the HMAC to

    // Holds the computed signature at each iteration to compare it with the original
    // signature
    unsigned char *g_result;
    unsigned int g_result_len;

    char *g_buffer; // Holds the secret being constructed

    char starting_letter; // Each thread is assigned a first letter
    size_t max_len; // And tries combinations up to a certain length
};

void init_thread_data(struct s_thread_data *data, char starting_letter, size_t max_len) {
    data->max_len = max_len;
    data->starting_letter = starting_letter;
    // The chosen hash function is SHA-256
	data->g_evp_md = (EVP_MD *) EVP_sha256();
    // Allocate the buffer used to hold the calculated signature
	data->g_result = malloc(EVP_MAX_MD_SIZE);
    // Allocate the buffer used to hold the generated key
	data->g_buffer = malloc(max_len + 1);
}

void destroy_thread_data(struct s_thread_data *data) {
    free(data->g_result);
    free(data->g_buffer);
}

/**
 * Check if the signature produced with "secret
 * of size "secrent_len" (without the '\0') matches the original
 * signature.
 * Return true if it matches, false otherwise
 */
bool check(struct s_thread_data *data, const char *secret, size_t secret_len) {
    // If the secret was found by another thread, stop this thread
    if (g_found_secret != NULL) {
        destroy_thread_data(data);
        pthread_exit(NULL);
    }

	// Hash to_encrypt using HMAC into result
	HMAC(
		data->g_evp_md,
		(const unsigned char *) secret, secret_len,
		(const unsigned char *) g_to_encrypt, g_to_encrypt_len,
		data->g_result, &(data->g_result_len)
	);

	// Compare the computed hash to the given decoded base64 signature.
	// If there is a match, we just found the key.
	return memcmp(data->g_result, g_signature, g_signature_len) == 0;
}

bool brute_impl(struct s_thread_data *data, char* str, int index, int max_depth)
{
    for (int i = 0; i < g_alphabet_len; ++i)
    {
        // The character at "index" in "str" successvely takes the value
        // of each symbol in the alphabet
        str[index] = g_alphabet[i];

        // If just changed the last letter, that means we generated a
        // permutation, so we check it
        if (index == max_depth - 1) {
            // If we found the key, we return, otherwise we continue.
            // By continuing, the current letter (at index "index")
            // will be changed to the next symbol in the alphabet
            if (check(data, (const char *) str, max_depth)) return true;
        }
        // If the letter we just changed was not the last letter of
        // the permutation we are generating, recurse to change the
        // letter at the next index.
        else {
            // If this condition is met, that means we found the key.
            // Otherwise the loop will continue and change the current
            // character to the next letter in the alphabet.
			if (brute_impl(data, str, index + 1, max_depth)) return true;
        }
    }

    // If we are here, we tried all the permutations without finding a match
	return false;
}

/**
 * Try all the combinations of secret starting with letter "starting_letter"
 * and stopping at a maximum length of "max_len"
 * Returns the key when there is a match, otherwise returns NULL
 */
char *brute_sequential(struct s_thread_data *data)
{
    // We set the starting letter
    data->g_buffer[0] = data->starting_letter;
    // Special case for len = 1, we check in this function
    if (check(data, data->g_buffer, 1)) {
        // If this thread found the solution, set the shared global variable
        // so other threads stop, and stop the current thread. Congrats little
        // thread!
        g_found_secret = strndup(data->g_buffer, 1);
        return g_found_secret;
    }

    // We start from length 2 (we handled the special case of length 1
    // above.
    for (size_t i = 2; i <= data->max_len; ++i) {
      	if (brute_impl(data, data->g_buffer, 1, i)) {
            // If this thread found the solution, set the shared global variable
            // so other threads stop, and stop the current thread. Congrats little
            // thread!
            g_found_secret = strndup(data->g_buffer, i);
            return g_found_secret;
        }
    }

   success:
    
	return NULL;
}

void usage(const char *cmd) {
	printf("%s <token> [alphabet] [max_len]\n"
				   "Defaults: max_len=6, "
				   "alphabet=eariotnslcudpmhgbfywkvxzjqEARIOTNSLCUDPMHGBFYWKVXZJQ0123456789", cmd);
}

int main(int argc, char **argv) {
	size_t max_len = 6;
	g_alphabet = "eariotnslcudpmhgbfywkvxzjqEARIOTNSLCUDPMHGBFYWKVXZJQ0123456789";

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	// Get the token
	char *jwt = argv[1];

	if (argc > 2)
		g_alphabet = argv[2];
	if (argc > 3)
		max_len = (size_t) atoi(argv[3]);

	g_alphabet_len = strlen(g_alphabet);

	// Split the JWT into header, payload and signature
	g_header_b64 = strtok(jwt, ".");
	g_payload_b64 = strtok(NULL, ".");
	g_signature_b64 = strtok(NULL, ".");
	g_header_b64_len = strlen(g_header_b64);
	g_payload_b64_len = strlen(g_payload_b64);
	g_signature_b64_len = strlen(g_signature_b64);

	// Recreate the part that is used to create the signature
	// Since it will always be the same
	g_to_encrypt_len = g_header_b64_len + 1 + g_payload_b64_len;
	g_to_encrypt = (unsigned char *) malloc(g_to_encrypt_len + 1);
	sprintf((char *) g_to_encrypt, "%s.%s", g_header_b64, g_payload_b64);

	// Decode the signature
	g_signature_len = Base64decode_len((const char *) g_signature_b64);
	g_signature = malloc(g_signature_len);
	// We re-assign the length, because Base64decode_len returned us an approximation
	// of the size so we could malloc safely. But we need the real decoded size, which
	// is returned by this function
	g_signature_len = Base64decode((char *) g_signature, (const char *) g_signature_b64);


    struct s_thread_data *pointers_data[g_alphabet_len];
    pthread_t *tid = malloc(g_alphabet_len * sizeof(pthread_t));

    for (size_t i = 0; i < g_alphabet_len; i++) {
        pointers_data[i] = malloc(sizeof(struct s_thread_data));
        init_thread_data(pointers_data[i], g_alphabet[i], max_len);
        pthread_create(&tid[i], NULL, (void *(*)(void *)) brute_sequential, pointers_data[i]);
    }

    for (size_t i = 0; i < g_alphabet_len; i++)
        pthread_join(tid[i], NULL);

	if (g_found_secret == NULL)
		printf("No solution found :-(\n");
	else
		printf("Secret is \"%s\"\n", g_found_secret);

    free(g_found_secret);
    free(tid);

	return 0;
}
