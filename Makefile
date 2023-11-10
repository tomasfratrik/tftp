# tftp-client and tftp-server Makefile
# Author: Tomas Fratrik (xfratr01)

LOGIN = xfratr01
CLIENT = tftp-client
SERVER = tftp-server
CC = g++
CPPFLAGS = -std=c++2a -g
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)
OBJS_CLIENT := $(filter-out src/$(SERVER).o, $(OBJS))
OBJS_SERVER := $(filter-out src/$(CLIENT).o, $(OBJS))

.PHONY: clean all c client server zipped

all: $(CLIENT) $(SERVER) 

$(CLIENT): $(OBJS_CLIENT)
	$(CC) $(CPPFLAGS) -o $@ $^

$(SERVER): $(OBJS_SERVER)
	$(CC) $(CPPFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) -c $(CPPFLAGS) -MMD -MP $< -o $@ 

-include $(DEPS)

server:
	./tftp-server -p 1337 rootdir
client:
	./tftp-client -h localhost -p 1337 -t dest_path

zip: clean
	@echo "Zipping..."
	zip -r $(LOGIN).zip *

tar: clean 
	@echo "Taring..."
	tar -cvf $(LOGIN).tar * 

c: clean
clean:
	@echo "Cleaning..."
	@rm -rf $(CLIENT) $(SERVER) $(OBJS) $(DEPS) .depend

