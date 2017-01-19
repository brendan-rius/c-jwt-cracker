#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <stdbool.h>
#include "base64.h"


char *g_header_b64 = NULL;
char *g_payload_b64 = NULL;
char *g_signature_b64 = NULL;
unsigned char *g_to_encrypt = NULL;
unsigned char *g_signature = NULL;

size_t g_header_b64_len = 0;
size_t g_payload_b64_len = 0;
size_t g_signature_b64_len = 0;
size_t g_signature_len = 0;
size_t g_to_encrypt_len = 0;

char *g_alphabet = NULL;
size_t g_alphabet_len = 0;

bool check(const char *secret, size_t secret_len) {
	// printf("Trying %s (%lu)\n", secret, secret_len);

	// Hash to_encrypt using HMAC-SHA256 into result
	unsigned int result_len = 0;
	unsigned char* result = malloc(EVP_MAX_MD_SIZE);
	HMAC(
		EVP_sha256(),
		(const unsigned char *) secret, secret_len,
		(const unsigned char *) g_to_encrypt, g_to_encrypt_len,
		result, &result_len
	);

	// Compare the computed hash to the given decoded base64 signature.
	// If there is a match, we just found the key.
	int res = memcmp(result, g_signature, g_signature_len);
	free(result);
	return res == 0;
}

void bruteImpl(char* str, int index, int maxDepth)
{
    for (int i = 0; i < g_alphabet_len; ++i)
    {
        str[index] = g_alphabet[i];

        if (index == maxDepth - 1) {
					bool stop = check((const char *) str, strlen(str));
					if (stop) {
						printf("FOUND \"%s\"", str);
						exit(0);
					}
				}
        else bruteImpl(str, index + 1, maxDepth);
    }
}

void bruteSequential(int maxLen)
{
    char* buf = malloc(maxLen + 1);

    for (int i = 1; i <= maxLen; ++i)
    {
        memset(buf, 0, maxLen + 1);
        bruteImpl(buf, 0, i);
    }

		printf("No solution found :-(\n");
    free(buf);
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

	if (argc > 2) {
			g_alphabet = argv[2];
	}
	if (argc > 3)
			max_len = (size_t) atoi(argv[3]);

	g_alphabet_len = strlen(g_alphabet);

	// Split it into header, payload and signature
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

	bruteSequential(max_len);

	return 0;
}
