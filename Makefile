NAME=ft_ping
CC=gcc
CFLAGS=-Wall -Wextra -Werror -Iinc
LDFLAGS=-lm
OBJDIR=obj
SOURCES=src/main.c src/parser.c src/dns.c src/ping.c src/utils.c src/print.c
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

# Pattern-based targets: test-<name>
test-%: $(NAME)
	@echo "Setting up test environment for $*..."
	@mkdir -p test/auto test/results && \
	cp ft_ping test/ && \
	cp test/cases/$*.test.txt test/auto/ft_$*.test.txt && \
	sed -i 's|^ping|sudo ./ft_ping|g' test/auto/ft_$*.test.txt
	@if [ ! -f test/results/$*.results.csv ]; then \
		echo "Running tests on system ping..."; \
		./test/tests/command_collector.py test/results/$*.results.csv test/cases/$*.test.txt; \
	else \
		echo "System ping results already exist, skipping..."; \
	fi
	@echo "Running tests on ft_ping..."
	@./test/tests/command_collector.py test/auto/ft_$*.results.csv test/auto/ft_$*.test.txt
	@echo "Comparing results..."
	@./test/tests/test_compare.py test/results/$*.results.csv test/auto/ft_$*.results.csv
	@rm -rf test/ft_ping test/auto
	@echo "Test complete."

vtest-%: $(NAME)
	@echo "Setting up verbose test environment for $*..."
	@mkdir -p test/auto test/results && \
	cp ft_ping test && \
	cp test/cases/$*.test.txt test/auto/ft_$*.test.txt && \
	sed -i 's|^ping|sudo ./ft_ping|g' test/auto/ft_$*.test.txt
	@echo "Running valgrind test on ft_ping..."
	@./test/tests/run_and_parse_valgrind.py test/auto/ft_$*.test.txt
	@rm -rf test/ft_ping test/auto
	@echo "Valgrind test complete."

valgrind: $(NAME)
	@echo "Running valgrind on $(NAME)..."
	@sudo valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME)