CC	= gcc

OPENSSL = /usr/include/openssl
OPENSSL_LIB = -lssl

CFLAGS	+= -I $(OPENSSL) -g -std=gnu99 -O3
LDFLAGS	+= $(OPENSSL_LIB) -lcrypto -lpthread

NAME	= jwtcrack
SRCS	= main.c base64.c
OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
