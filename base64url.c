#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

// Convert base64url to base64
// Fix b64 alignement
// Replace '-' by '+' and '_' by '/'
static unsigned char *base64url_to_base64(const unsigned char *base64) {
	size_t base64len = strlen(base64);
	// \0 + possible padding
	unsigned char *nbase64 = malloc(base64len + 3);

	memset(nbase64, 0, base64len + 3);
	strcat(nbase64, base64);

	// Fix b64 alignement
	while (base64len % 4 != 0) {
		nbase64[base64len++] = '=';
	}
	for (int i = 0; i < base64len; ++i) {
		if (nbase64[i] == '-')
			nbase64[i] = '+';
		if (nbase64[i] == '_')
			nbase64[i] = '/';
	}
	return nbase64;
}

int base64url_decode(const unsigned char *in, unsigned char **out)
{
    size_t inlen, outlen;
    
	unsigned char *outbuf;
	unsigned char *base64 = base64url_to_base64(in);

	inlen = strlen(base64);
    outlen = (inlen / 4) * 3;
    outbuf = malloc(outlen);
    if (outbuf == NULL) {
        goto err;
    }

    outlen = EVP_DecodeBlock(outbuf, (unsigned char *)base64, inlen);
    if (outlen < 0) {
        goto err;
    }

    *out = outbuf;
	free(base64);
    return outlen;
err:
	free(base64);
    free(outbuf);
    return -1;
}
