SRCS = src/main.cpp src/Utils/utils.cpp src/Cgi/Cgi.cpp src/Server/Server.cpp src/Config/Config.cpp src/Config/UtilsConfig.cpp src/Config/ParseRoute.cpp
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98
OBJS = $(SRCS:.cpp=.o)
NAME = webserv
INC = -I./src/Cgi -I./src/Server -I./src/Config -I./src/Utils
# LOCATION = -L./src/Cgi -L./src/Server -L./src/Config -L./src/Utils

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) $(INC) -o $(NAME)

%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) $(INC) -o $@

run : all
	@if pgrep $(NAME) ; then pkill $(NAME) ; fi
	./$(NAME) server.conf

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re
