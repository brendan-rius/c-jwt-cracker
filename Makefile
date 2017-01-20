CC	= gcc
RM	= rm -f
CP	= cp

CFLAGS	+= -I/usr/local/opt/openssl/include -O3
LDFLAGS	+= -lssl -lcrypto

NAME	= jwtcrack
SRCS	= main.c \
	base64.c
OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all
