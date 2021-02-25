.PHONY: clean

EXECS=hello_daos
MPICC?=mpicc

CFLAGS=
LIBS=-ldaos -ldfs -lgurt -luuid

all: ${EXECS}

hello_daos: hello_daos.c
	${MPICC} -o hello_daos hello_daos.c $(CFLAGS) $(LIBS)

clean:
	rm -f *.o ${EXECS}
