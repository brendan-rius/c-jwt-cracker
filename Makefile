CC	= gcc

OPENSSL = /usr/include/openssl

CFLAGS	+= -I $(OPENSSL) -O3
LDFLAGS	+= -lssl -lcrypto

NAME	= jwtcrack
SRCS	= main.c base64.c
OBJS	= $(SRCS:.c=.o)

all: $(NAME)
	@echo $(OPENSSL)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
	rm $(OBJS)

fclean: clean
	rm $(NAME)

re: fclean all
