#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <stdbool.h>
#include <time.h>
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

// Holds the computed signature at each iteration to compare it with the original
// signature
unsigned char *g_result = NULL;
unsigned int g_result_len = 0;

char *g_buffer = NULL;

// The hash function to apply the HMAC to
EVP_MD *g_evp_md = NULL;

/**
 * Check if the signature produced with "secret
 * of size "secrent_len" (without the '\0') matches the original
 * signature.
 * Return true if it matches, false otherwise
 */
bool check(const char *secret, size_t secret_len) {
	// Hash to_encrypt using HMAC into result
	HMAC(
		g_evp_md,
		(const unsigned char *) secret, secret_len,
		(const unsigned char *) g_to_encrypt, g_to_encrypt_len,
		g_result, &g_result_len
	);

	// Compare the computed hash to the given decoded base64 signature.
	// If there is a match, we just found the key.
	return memcmp(g_result, g_signature, g_signature_len) == 0;
}

bool bruteImpl(char* str, int index, int maxDepth)
{
    for (int i = 0; i < g_alphabet_len; ++i)
    {
        str[index] = g_alphabet[i];

        if (index == maxDepth - 1) {
					if (check((const char *) str, maxDepth)) return true;
				}
        else {
					if (bruteImpl(str, index + 1, maxDepth)) return true;
				}
    }

	return false;
}

char *bruteSequential(int start, int maxLen)
{
    for (int i = start; i <= maxLen; ++i)
    {
      	if (bruteImpl(g_buffer, 0, i))
            return strdup(g_buffer);
    }

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

    // Allocate the buffer used to hold the calculated signature
	g_result = malloc(EVP_MAX_MD_SIZE);
	g_buffer = malloc(max_len + 1);

    // The chosen hash function is SHA-256
	g_evp_md = (EVP_MD *) EVP_sha256();

	clock_t start = clock(), diff;
	char *secret = bruteSequential(1, max_len);
	diff = clock() - start;


	if (secret == NULL)
		printf("No solution found :-(\n");
	else
		printf("Secret is \"%s\"\n", secret);

	free(g_result);
	free(g_buffer);
	free(secret);

	int msec = diff * 1000 / CLOCKS_PER_SEC;
	printf("Time taken %d seconds %d milliseconds", msec/1000, msec%1000);

	return 0;
}
