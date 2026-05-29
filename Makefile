NAME=ft_ping
CC=gcc
CFLAGS=-Wall -Wextra -Werror -Iinc
LDFLAGS=-lm
OBJDIR=obj
SOURCES=src/main.c src/parser.c src/validator.c src/initializer.c src/ping.c src/print.c src/icmp_error.c src/error.c src/utils.c
OBJECTS=$(SOURCES:src/%.c=$(OBJDIR)/%.o)

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all
	@echo "Rebuild complete."

test-%: $(NAME)
	@sudo python3 test/tester.py $*

vtest-%: $(NAME)
	@sudo python3 test/tester.py $* --valgrind

regen-%: $(NAME)
	@sudo python3 test/tester.py $* --regen