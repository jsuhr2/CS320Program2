C=cache-sim
CFLAGS= -Wall -Wextra -DDEBUG -g -pedantic -std=c++14

all: $(C)

$(C): $(C).cpp
	g++ $(CFLAGS) $(C).cpp -o $(C)

clean:
	rm $(C)
