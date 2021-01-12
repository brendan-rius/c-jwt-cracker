# JWT cracker

A multi-threaded JWT brute-force cracker written in C. If you are very lucky or have a huge computing power, this program should find the secret key of a JWT token, allowing you to forge valid tokens. This is for testing purposes only, do not put yourself in trouble :)

I used the [Apple Base64 implementation](https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.c) that I modified slightly.

## Build a Docker Image
```
docker build . -t jwtcrack

```


## Run on Docker
```
docker run -it --rm  jwtcrack eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9.cAOIAifu3fykvhkHpbuhbvtH807-Z2rI1FS3vX1XMjE
```

## Manual Compilation

Make sure you have openssl's headers installed.
On Ubuntu you can install them with `apt-get install libssl-dev`

```
make
```

If you use a Mac, you can install OpenSSL with `brew install openssl`, but the headers will be stored in a
different location:

```
make OPENSSL=/usr/local/opt/openssl/include OPENSSL_LIB=-L/usr/local/opt/openssl/lib
```

## Run

```
$ > ./jwtcrack eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9.cAOIAifu3fykvhkHpbuhbvtH807-Z2rI1FS3vX1XMjE
```                                                              

## Run with different HMAC functions

The following hash functions are supported for HMAC, i.e. to generate keyed-hashed message authentication codes: "sha256" for JSON HS256 (HMAC using SHA-256), "sha384" for HS384 and "sha512" for HS512, respectively. You can specify the name of any other hash function exactly as it is named in the OpenSSL. If OpenSSL allows this hash function to be used for HMAC, then jwtcrack will try to decode the secret. However, since jwtcrack is only a decoder, there is no guarantee that this algorithm was actually used for encoding, let alone among the list of algorithms allowed for the "JSON Web Algorithms" RFC. See section 3.1. of the RFC 7518 for more details.

In the following example, we use a sha256 hash function that corresponds to JSON HS256 (HMAC-SHA256), see the "sha256" as a last command line parameter. Also, in this example we specify maximum secret length of 5 characters, and limit the alphabet to the following characters: ABCSNFabcsnf1234

```
$ > ./jwtcrack eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiYWRtaW4iOnRydWV9.cAOIAifu3fykvhkHpbuhbvtH807-Z2rI1FS3vX1XMjE ABCSNFabcsnf1234 5 sha256
```
In the above example, the key is `Sn1f`, and it takes less than a second on an average notebook manufactured around 2019 (e.g. with an Intel CPU based on Ice Lake microarchitecture). GCC version 9.3.0 with "-O3" was used to compile the jwtcrack program. It was linked with the OpenSSL library version 1.1.1f under Linux Ubuntu 20.04.1 LTS.

Here, in the next example, we use "sha512" as a last command line parameter to specify HS512 (HMAC-SHA512), we also specify maximum secret length of 9 characters, and limit the alphabet to the following seven lowercase latin characters: "adimnps".

```
$ > ./jwtcrack eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzUxMiJ9.eyJyb2xlIjoiYWRtaW4ifQ.RnWtv7Rjggm8LdMU3yLnz4ejgGAkIxoZwsCMuJlHMwTh7CJODDZWR8sVuNvo2ws25cbH9HWcp2n5WxpIZ9_v0g adimnps 9 sha512
```

In the above example, the key is `adminpass`, and it takes about 15 seconds on average to decode on a notebook with Intel Core i7 1065G7 CPU on Ice Lake microarchitecture (2019), base frequency 1.30 GHz, max turbo 3.90 GHz). The combined number of CPU seconds consumed from each of the cores in the user mode due to multithreading is about 100 on average to decode that secret.


Example of using "sha384":

```
$ > ./jwtcrack eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzM4NCJ9.eyJyb2xlIjoiYWRtaW4ifQ.31xCH3k8VRqB8l5qBy7RyqI2htyCskBy_4cIWpk3o43UkIMW-IcjTUEL_NyFXUWJ 0123456789 6 sha384
```

## Measurement of time consumed by jwtcrack

```
/usr/bin/time -f "Total number of CPU-seconds consumed directly from each of the CPU cores: %U\nElapsed real wall clock time used by the process: %E" ./jwtcrack eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzUxMiJ9.eyJyb2xlIjoiYWRtaW4ifQ.RnWtv7Rjggm8LdMU3yLnz4ejgGAkIxoZwsCMuJlHMwTh7CJODDZWR8sVuNvo2ws25cbH9HWcp2n5WxpIZ9_v0g adimnps 9 sha512
```

## Contribute

 * No progress status
 * If you stop the program, you cannot start back where you were
 
## IMPORTANT: Known bugs

The base64 implementation I use (from Apple) is sometimes buggy because not every Base64 implementation is the same.
So sometimes, decrypting of your Base64 token will only work partially and thus you will be able to find a secret to your token that is not the correct one.

If someone is willing to implement a more robust Base64 implementation, that would be great :)
