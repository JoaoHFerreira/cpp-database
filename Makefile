CC = g++
CFLAGS = -Wall -Wextra -std=c++11
SRCS = main.cpp Row.cpp Node.cpp Pager.cpp Table.cpp Cursor.cpp Statement.cpp MetaCommand.cpp
OBJS = $(SRCS:.cpp=.o)
HDRS = Constants.h Row.h Node.h Pager.h Table.h Cursor.h Statement.h MetaCommand.h

db: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o db

%.o: %.cpp $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f db $(OBJS)

.PHONY: clean
