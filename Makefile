# tfpt-client and tftp-server Makefile
# Author: Tomas Fratrik (xfratr01)

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)
CC = g++
CPPFLAGS = -std=c++2a

OBJS_CLIENT := $(filter-out src/tftp-server.o, $(OBJS))
OBJS_SERVER := $(filter-out src/tftp-client.o, $(OBJS))

all: tftp-client tftp-server

tftp-client: $(OBJS_CLIENT)
	$(CC) $(CPPFLAGS) -o $@ $^

tftp-server: $(OBJS_SERVER)
	$(CC) $(CPPFLAGS) -o $@ $^

$(OBJS): src/%.o: src/%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f tftp-client tftp-server $(OBJS)