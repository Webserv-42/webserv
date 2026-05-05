
NAME            = webserv
SOURCES         =  src/main.cpp \
					src/core/Server.cpp \
					src/core/ServerClient.cpp \
					src/core/ServerClientUtils.cpp \
					src/core/ServerLoop.cpp \
					src/core/ServerSockets.cpp \
					src/core/Client.cpp \
					src/http/HttpRequest.cpp \
					src/http/HttpHandler.cpp \
					src/http/HttpHandlerError.cpp \
					src/http/HttpHandlerGet.cpp \
					src/http/HttpHandlerPost.cpp \
					src/http/HttpHandlerDelete.cpp \
					src/http/HttpHandlerUtils.cpp \
					src/http/HttpHandlerSession.cpp \
					src/config/ConfigParser.cpp \
					src/config/ConfigParserUtils.cpp \
					src/config/ConfigParserDirectives.cpp \
					src/config/ConfigParserParse.cpp \
					src/config/SessionManager.cpp \
					src/config/CgiHandler.cpp

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
