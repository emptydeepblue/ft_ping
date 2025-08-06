NAME=ft_ping
CC=gcc
CFLAGS=-Wall -Wextra -Werror -Iinc
SOURCES=src/main.c src/utils.c
OBJECTS=$(SOURCES:.c=.o)

.PHONY: all clean fclean re

all: $(NAME)
$(NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(NAME)

re: fclean all
	@echo "Rebuild complete."
