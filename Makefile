CFLAGS = -Wall -Wextra -g

banker: banker.o
	gcc $(CFLAGS) -o banker $^

%.o: %.c
	gcc $(CFLAGS) -c $<

run: banker
	./banker

clean:
	rm -f *.o banker

.PHONY: run clean