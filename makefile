CC=gcc
EXEC=netapp
cOption=-w
# libn=-lWs2_32
libn=

all : $(EXEC)

netapp: netapp.o netcommon.o parser.o
	$(CC) $(cOption) -o netapp netapp.o netcommon.o parser.o

netapp.o: netapp.c
	$(CC) $(cOption) -c netapp.c -o netapp.o
	
netcommon.o: netcommon.c
	$(CC) $(cOption) -c netcommon.c -o netcommon.o
	
parser.o: parser.c
	$(CC) $(cOption) -c parser.c -o parser.o
	
clean:
	rm *.o
	
mrproper: clean
	rm -rf $(EXEC)
