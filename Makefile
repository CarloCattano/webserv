SRCS = main.cpp 
CXX = c++
CXXFLAGS = -Wall -Werror -Wextra -std=c++98
# OBJS = $(SRCS:.cpp=.o)
NAME = webserv

ifdef DEBUG
    CXXFLAGS+= -DDEBUG
endif

all: $(NAME)

$(NAME):
	$(CXX) $(SRCS) $(CXXFLAGS) -o $(NAME)

run : all
	killall webserv
	./$(NAME) server.conf

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)
		
re: fclean all

.PHONY: all clean fclean re
