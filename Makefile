NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
SRCS = main.cpp Server.cpp Client.cpp Utils.cpp Pass.cpp Nick.cpp User.cpp Quit.cpp Ping.cpp Cap.cpp \
       Channel.cpp Join.cpp Part.cpp Privmsg.cpp Kick.cpp Invite.cpp Topic.cpp Mode.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re