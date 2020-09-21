FROM n0madic/alpine-gcc:9.2.0
RUN apk add --quiet --no-cache libressl-dev 
COPY ./*.h /opt/src/
COPY ./*.c /opt/src/
COPY Makefile /opt/src/
COPY entrypoint.sh /
#RUN apt-get install libssl-dev
WORKDIR /opt/src
RUN make
RUN make OPENSSL=/usr/local/opt/openssl/include OPENSSL_LIB=-L/usr/local/opt/openssl/lib
RUN ["chmod", "+x", "/entrypoint.sh"]
RUN ["chmod", "+x", "/opt/src/jwtcrack"]
ENTRYPOINT ["/entrypoint.sh"]
