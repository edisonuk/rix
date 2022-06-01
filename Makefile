CCFLAGS = 
LDFLAGS =

%.o: .c
	$(CC) $(CCLAGS) -c -o $<

%.o: .S
	$(AS) $(LDFLAGS) -o $<

.PHONY clean

clean:
	$(RM)
