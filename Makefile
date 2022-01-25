

CFLAGS = -Wall -I/home/isodden/work/libevent/include
LDLIBS = -levent -L/home/isodden/work/libevent/.libs

LINUX_EXE =  c10k-test-client 

EXE = ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}



