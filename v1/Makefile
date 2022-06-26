# Makefile
 
# *****************************************************
# Variables to control Makefile operation
 
CC = g++
CFLAGS = -g -pthread
 
# ****************************************************
# Targets needed to bring the executable up to date
 
main: main.o
	$(CC) $(CFLAGS) -o main main.o time_point_task.o task_pool.o job_manager.o
 
main.o: main.cc list.h time_point_task.h task_pool.h job_manager.h
	$(CC) $(CFLAGS) -c main.cc time_point_task.cc task_pool.cc job_manager.cc
 
clean:
	rm -f main *.o

