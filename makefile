CC = g++
CFLAGS = -g -Wall -fopenmp
OBJS = ACO.o Randoms.o main.o 
TARGET = openMP

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

ACO.o: ACO.h ACO.cpp
	$(CC) $(CFLAGS) -c ACO.cpp

Randoms.o: Randoms.cpp
	$(CC) $(CFLAGS) -c Randoms.cpp

main.o: ACO.h main.cpp
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -f $(OBJS) $(TARGET)

run:
	./$(TARGET) 20