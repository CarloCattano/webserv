SRCS = src/main.cpp
UTILS_SRCS = src/utils.cpp
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98
OBJS = $(SRCS:.cpp=.o)
UTILS_OBJS = $(UTILS_SRCS:.cpp=.o)
NAME = webserv

all: $(NAME)

$(NAME): $(OBJS) $(UTILS_OBJS)
	$(CXX) $(OBJS) $(UTILS_OBJS) $(CXXFLAGS) -o $(NAME)

%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

run : all
	@if pgrep $(NAME) ; then pkill $(NAME) ; fi
	./$(NAME) server.conf

clean:
	rm -rf $(OBJS) $(UTILS_OBJS)

fclean: clean
	rm -rf $(NAME)
		
re: fclean all

.PHONY: all clean fclean re
