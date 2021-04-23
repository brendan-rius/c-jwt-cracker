FROM alpine:latest
RUN apk add --quiet --no-cache gcc musl-dev make bash libressl-dev
COPY ./*.h /opt/src/
COPY ./*.c /opt/src/
COPY Makefile /opt/src/
COPY entrypoint.sh /
WORKDIR /opt/src
RUN make
RUN make OPENSSL=/usr/local/opt/openssl/include OPENSSL_LIB=-L/usr/local/opt/openssl/lib
RUN ["chmod", "+x", "/entrypoint.sh"]
RUN ["chmod", "+x", "/opt/src/jwtcrack"]
ENTRYPOINT ["/entrypoint.sh"]
