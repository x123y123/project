all: main

CC = gcc
CFLAGS = -O1 -g -lpthread -pthread -Wall #-Werror -I.
OBJS = qlearning_gov.o pmu_monitor.o

main: $(OBJS)
	$(CC) -o qlearning_gov $(CFLAGS) qlearning_gov.o pmu_monitor.o

qlearning_gov.o: qlearning_gov.c qlearning_gov.h
	$(CC) $(CFLAGS) -c qlearning_gov.c 

pmu_monitor.o: pmu_monitor.c
	$(CC) $(CFLAGS) -c pmu_monitor.c 
    
.PHONY: clean
clean: 
	rm -f qlearning_gov %.o
