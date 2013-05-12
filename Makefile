NAME = UDF_Reader
SRCS = srcs/main.cpp \
       srcs/UdfReader.cpp \
       srcs/AUdf.cpp \
       srcs/Command.cpp

OBJS = $(SRCS:.cpp=.o)

INCLUDES = includes/

CC = g++
CXXFLAGS += -W -Wall -Wextra 
RM = rm -f

ALL: $(NAME)

$(NAME): $(OBJS)
	   $(CC) -o $(NAME) $(OBJS)

clean:
		$(RM) $(OBJS)

fclean: clean
		$(RM) $(NAME)

re: fclean ALL

%.o : %.cpp
	$(CC) -c $(CXXFLAGS) $(CPPFLAGS) -I $(INCLUDES) $< -o $@
