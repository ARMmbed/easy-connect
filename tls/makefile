CFLAGS :=$(filter-out -std=c99,$(CFLAGS))
CFLAGS :=$(filter-out -pedantic,$(CFLAGS))
CFLAGS :=$(CFLAGS) -D_FILE_OFFSET_BITS=64

#CFLAGS :=$(CFLAGS) -I../configs


all:
	cp makefile.mbedtls mbedtls/library/Makefile
	$(MAKE) $(MAKE_FLAGS) -C mbedtls/library

.PHONEY: clean
clean:

