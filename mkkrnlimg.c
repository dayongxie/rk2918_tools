#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "rkcrc.h"

#define MAGIC_CODE "KRNL"


struct krnl_header
{
	char magic[4];
	unsigned int length;
};

int pack_krnl(FILE *fp_in, FILE *fp_out)
{
	char buf[1024];
	struct krnl_header header =
	{
		"KRNL",
		0
	};

	unsigned int crc = 0;

	fwrite(&header, sizeof(header), 1, fp_out);

	while (1)
	{
		int readlen = fread(buf, 1, sizeof(buf), fp_in);
		if (readlen == 0)
			break;

		header.length += readlen;
		fwrite(buf, 1, readlen, fp_out);
		RKCRC(crc, buf, readlen);
	}

	fwrite(&crc, sizeof(crc), 1, fp_out);
	fseek(fp_out, 0, SEEK_SET);
	fwrite(&header, sizeof(header), 1, fp_out);

	printf("%04X\n", crc);

	return 0;
//fail:
	fprintf(stderr, "FAIL\n");
	return -1;
}

int unpack_krnl(FILE *fp_in, FILE *fp_out)
{
	char buf[1024];
	struct krnl_header header;
	size_t length = 0;
	unsigned int crc = 0;
	unsigned int file_crc = 0;

	fprintf(stderr, "unpacking...");
	fflush(stderr);
	if (sizeof(header) != fread(&header, 1, sizeof(header), fp_in))
	{
		goto fail;
	}

	fseek(fp_in, header.length + sizeof(header), SEEK_SET);
	if (sizeof(file_crc) != fread(&file_crc, 1, sizeof(file_crc), fp_in))
		goto fail;

	length = header.length;
	fseek(fp_in, sizeof(header), SEEK_SET);

	while (length > 0)
	{
		int readlen = length < sizeof(buf) ? length : sizeof(buf);
		readlen = fread(buf, 1, readlen, fp_in);
		length -= readlen;
		fwrite(buf, 1, readlen, fp_out);
		RKCRC(crc, buf, readlen);

		if (readlen == 0)
			break;
	}

	if (file_crc != crc)
		fprintf(stderr, "WARNING: bad crc checksum\n");

	fprintf(stderr, "OK\n");
	return 0;

fail:
	fprintf(stderr, "FAIL\n");
	return -1;
}

int main(int argc, char **argv)
{
	FILE *fp_in, *fp_out;
	int action = 0;

	if (argc != 4)
	{
		fprintf(stderr, "usage: %s [-a|-r] <input> <output>\n", argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "-a") == 0)
	{
		action = 1;
	} else if (strcmp(argv[1], "-r") == 0)
	{
		action = 2;
	} else {
		fprintf(stderr, "usage: %s [-a|-r] <input> <output>\n", argv[0]);
	}

	fp_in = fopen(argv[2], "rb");
	if (!fp_in)
	{
		fprintf(stderr, "can't open input file '%s': %s\n", argv[2], strerror(errno));
		return 1;
	}

	fp_out = fopen(argv[3], "wb");
	if (!fp_out)
	{
		fprintf(stderr, "can't open output file '%s': %s\n", argv[3], strerror(errno));
		return 1;
	}

	switch (action)
	{
	case 1:
		pack_krnl(fp_in, fp_out);
		break;
	case 2:
		unpack_krnl(fp_in, fp_out);
		break;
	default:
		break;
	}

	return 0;
}
