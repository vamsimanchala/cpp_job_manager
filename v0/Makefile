# Makefile
 
# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS = -std=c++17 -g -pthread
 
# ****************************************************
# Targets needed to bring the executable up to date
 
main: main.o
	$(CC) $(CFLAGS) -o main main.o job_manager.o time_point_task.o task_pool.o
 
main.o: main.cc time_point_task.h task_pool.h job_manager.h
	$(CC) $(CFLAGS) -c main.cc job_manager.cc time_point_task.cc task_pool.cc
 
clean:
	rm -f main *.o

