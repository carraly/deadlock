CFLAGS = -Wall -g

banker: banker.o
	gcc $(CFLAGS) -o sync_car $^

%.o: %.c
	gcc $(CFLAGS) -c $<

run: banker
	./banker

clean:
	rm -f *.o banker

.PHONY: run clean