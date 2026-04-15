
NAME            = webserv
SOURCES         =  src/main.cpp \
					src/Server.cpp \
					src/Client.cpp \
					src/HttpRequest.cpp \
					src/HttpHandler.cpp \
					src/ConfigParser.cpp \
					src/HttpHandler.cpp \
					src/SessionManager.cpp
					src/HttpRequest.cpp 
OBJECTS         = $(SOURCES:.cpp=.o)

CC              = c++
CFLAGS          = -Wall -Wextra -Werror -std=c++98 -Iincludes
RM              = rm -rf

GREEN           = \033[1;32m


all: $(NAME)

$(NAME): $(OBJECTS)
	@echo "$(CYAN)Building $(NAME)...$(RESET)"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME)
	@echo "$(GREEN)$(NAME) built successfully!$(RESET)"

%.o: %.cpp
	@echo "$(YELLOW)Compiling $<...$(RESET)"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "$(RED)Cleaning object files...$(RESET)"
	@$(RM) $(OBJECTS)

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
