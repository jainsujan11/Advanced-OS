all: get_info_for_pid get_info

libA1_syscalls.a: lib_get_info_for_pid.o lib_get_info.o
	ar rcs libA1_syscalls.a lib_get_info_for_pid.o lib_get_info.o

lib_get_info_for_pid.o: lib_get_info_for_pid.c lib_get_info_for_pid.h
	gcc -c lib_get_info_for_pid.c

lib_get_info.o: lib_get_info.c lib_get_info.h
	gcc -c lib_get_info.c

get_info_for_pid: test_lib_get_info_for_pid.c libA1_syscalls.a
	gcc test_lib_get_info_for_pid.c -L. -lA1_syscalls -o get_info_for_pid

get_info: test_lib_get_info.c libA1_syscalls.a
	gcc test_lib_get_info.c -L. -lA1_syscalls -o get_info

clean:
	rm -f *.o *.a get_info_for_pid get_info

