
ifeq ($(OS),Windows_NT)
    uname_S := Windows
else
    uname_S := $(shell uname -s)
endif
ifeq ($(uname_S),Linux)
	CFLAGS = -Wall -I/home/isodden/work/libevent/include
	LDLIBS = -levent -L/home/isodden/work/libevent/.libs
endif
ifeq ($(uname_S),Darwin)
	CFLAGS = -Wall -I/Users/shanshan/work/libevent/include
	LDLIBS = -levent -L/Users/shanshan/work/libevent/.libs
endif
LINUX_EXE =  c10k-test-client 

EXE = ${LINUX_EXE}

all : ${EXE}

showos:
	echo $(uname_S)

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}



