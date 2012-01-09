CC=gcc
CFLAGS := -g -O3 -DUSE_OPENSSL 
LDLIBS := -lssl -lcrypto

#CFLAGS := -DUSE_GCRYPT
#LDLIBS := -lgcrypt

TARGETS := afptool img_unpack img_maker mkkrnlimg

all: ${TARGETS}

clean:
	rm ${TARGETS}
