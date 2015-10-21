NAME	= TaskManager

CC	= g++

RM	= rm -f

SRCDIR	= src/

INCDIR	= inc/

OBJDIR	= obj/

SRCS	= main.cpp \
	  Worker.cpp \
	  Scheduler.cpp \
	  ThreadPool.cpp \
	  ThreadManager.cpp

OBJS	= $(addprefix $(OBJDIR), $(SRCS:.cpp=.o))

INCCOMP	= -I$(INCDIR)

CXXFLAGS = -std=c++1y -g

LDFLAGS += -lpthread

dummy	:= 	$(shell test -d $(OBJDIR) || mkdir $(OBJDIR))

$(OBJDIR)%.o:	$(SRCDIR)%.cpp
	$(CC) $(CXXFLAGS) $(INCCOMP) -o $@ -c $<

$(NAME):  $(OBJS)
	$(CC) -o $(NAME) $(OBJS)  $(LDFLAGS)

all: $(NAME)

clean:
	$(RM) $(OBJS)

fclean:	clean
	$(RM) $(NAME)

re:	fclean all

.PHONY: all clean fclean re
