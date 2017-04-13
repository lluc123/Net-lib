CC=gcc
EXEC=netapp.exe
cOption=-w
libn=-lWs2_32

all : $(EXEC)

netapp.exe: netapp.o netcommon.o parser.o
	$(CC) $(cOption) -static-libgcc -o netapp.exe netapp.o netcommon.o parser.o

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
