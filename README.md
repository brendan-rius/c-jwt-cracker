# JWT cracker

A multi-threaded JWT brute-force cracker written in C. If you are very lucky or have a huge computing power, this program should find the secret key of a JWT token, allowing you to forge valid tokens. This is for testing purposes only, do not put yourself in trouble :)

I used the [Apple Base64 implementation](https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.c) that I modified slightly.

## Compile

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

In the above example, the key is `Sn1f`. It takes approximately 2 seconds to crack on my Macbook.

## Contribute

 * No progress status
 * If you stop the program, you cannot start back where you were
 
## IMPORTANT: Known bugs

The base64 implementation I use (from Apple) is sometimes buggy because not every Base64 implementation is the same.
So sometimes, decrypting of your Base64 token will only work partially and thus you will be able to find a secret to your token that is not the correct one.

If someone is willing to implement a more robust Base64 implementation, that would be great :)
