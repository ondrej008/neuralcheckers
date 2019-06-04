CFLAGS = -std=c++17 -Wall -Wpedantic -Wextra -Og
LFLAGS = 
all:
	g++ main.cpp -o app $(LFLAGS) $(CFLAGS)




