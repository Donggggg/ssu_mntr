ssu_mntr : main.o prmt.o mntr.o
	gcc main.o prmt.o -o ssu_mntr
	gcc mntr.o -o monitoring

main.o : main.c ssu_mntr.h
	gcc -c main.c

prmt.o : prmt.c ssu_mntr.h
	gcc -c prmt.c

mntr.o : mntr.c ssu_mntr.h
	gcc -c mntr.c

clean :
		rm *.o
		rm monitoring
		rm ssu_mntr
