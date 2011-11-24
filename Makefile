CC=gcc
CFLAGS := -g -O3 -DUSE_OPENSSL
LDFLAGS := -lssl -lcrypto
#CFLAGS := -DUSE_GCRYPT
#LDFLAGS := -lgcrypt

all: afptool img_unpack img_maker mkkrnlimg
#afptool : afptool.o
#img_unpack : img_unpack.o
#img_maker : img_maker.o
