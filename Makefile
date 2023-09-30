# tfpt-client and tftp-server Makefile
# Author: Tomas Fratrik (xfratr01)

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)
CC = g++
CPPFLAGS = -std=c++2a -g

OBJS_CLIENT := $(filter-out src/tftp-server.o, $(OBJS))
OBJS_SERVER := $(filter-out src/tftp-client.o, $(OBJS))

all: tftp-client tftp-server

tftp-client: $(OBJS_CLIENT)
	$(CC) $(CPPFLAGS) -o $@ $^

tftp-server: $(OBJS_SERVER)
	$(CC) $(CPPFLAGS) -o $@ $^

$(OBJS): src/%.o: src/%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

zip:
	clean
	zip -r xfratr01.zip *

clean:
	rm -f src/*.o tftp-client tftp-server $(OBJS)