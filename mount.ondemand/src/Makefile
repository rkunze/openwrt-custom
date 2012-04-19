OBJS=mount-helper.o lib/setup.o lib/log.o lib/sys.o lib/autofs.o

all: mount.ondemand 

mount.ondemand: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	rm -f lib/*.o *.o mount.autofs

mrproper: clean
	rm -f *~ lib/*~ include/*~

%.o: %.c
	$(CC) $(CFLAGS) -I. -c  $^ -o $@
