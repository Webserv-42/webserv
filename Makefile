
NAME            = webserv
SOURCES         =  src/main.cpp
OBJECTS         = $(SOURCES:.cpp=.o)

CC              = c++
CFLAGS          = -Wall -Wextra -Werror -std=c++98
RM              = rm -rf

GREEN           = \033[1;32m


all: $(NAME)

$(NAME): $(OBJECTS)
	@echo "$(CYAN)Building $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME) >/dev/null 2>&1
	@echo "$(GREEN)$(NAME) built successfully!$(RESET)"

%.o: %.cpp
	@echo "$(YELLOW)Compiling $<...$(RESET)"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@$(RM) $(OBJECTS) >/dev/null 2>&1

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@$(RM) $(NAME) >/dev/null 2>&1

re: fclean all

.PHONY: all clean fclean re