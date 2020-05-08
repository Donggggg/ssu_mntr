ssu_monitoring : main.o prmt.o ssu_mntr.o
	gcc main.o prmt.o -o ssu_monitoring
	gcc ssu_mntr.o -o ssu_mntr

main.o : main.c ssu_mntr.h
	gcc -c main.c

prmt.o : prmt.c ssu_mntr.h
	gcc -c prmt.c

ssu_mntr.o : ssu_mntr.c ssu_mntr.h
	gcc -c ssu_mntr.c

clean :
	rm *.o
	rm ssu_monitoring
	rm ssu_mntr

