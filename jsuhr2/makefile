C=cache-sim
CFLAGS= -Wall -Wextra -DDEBUG -g -pedantic -std=c++14

all: $(C)

$(C): $(C).cpp
	g++ $(CFLAGS) $(C).cpp -o $(C)

run: all
	./$(C) trace3.txt output.txt

clean:
	rm $(C)
