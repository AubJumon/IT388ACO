CC = mpicxx
CFLAGS = -Wall -O3
OBJS = ACO.o main.o Randoms.o
TARGET = aco

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

ACO.o: ACO.cpp ACO.h
	$(CC) $(CFLAGS) -c ACO.cpp

main.o: main.cpp ACO.h
	$(CC) $(CFLAGS) -c main.cpp

Randoms.o: Randoms.cpp
	$(CC) $(CFLAGS) -c Randoms.cpp

clean:
	rm -f $(OBJS) $(TARGET)
