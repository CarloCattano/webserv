SRCS =	src/main.cpp src/Utils/utils.cpp src/Cgi/Cgi.cpp src/Server/Server.cpp \
		src/Server/ServerCluster.cpp src/Config/Config.cpp src/Config/UtilsConfig.cpp \
		src/Config/ParseRoute.cpp src/Client/Client.cpp src/Server/HandleRequest.cpp

CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98 -O3
OBJS = $(SRCS:.cpp=.o)
NAME = webserv
INC = -I./src 

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) $(INC) -o $(NAME)
	@cat docs/banner

%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) $(INC) -o $@

run : all
	@if pgrep $(NAME) ; then pkill $(NAME) ; fi
	./$(NAME) conf/server.conf

debug:
	$(CXX) $(OBJS) $(CXXFLAGS) $(INC) -o $(NAME) -ggdb3

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

