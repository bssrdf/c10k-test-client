

CFLAGS = -Wall 
LDLIBS = -levent

LINUX_EXE =  c10k-test-client 

EXE = ${LINUX_EXE}

all : ${EXE}

allgen : ${GEN_EXE}

clean : 
	${RM} ${EXE} *.o

showall :
	@ echo ${EXE}



