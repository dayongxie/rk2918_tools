#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "rkrom_29xx.h"

int export_data(const char *filename, unsigned int offset, unsigned int length, FILE *fp)
{
	FILE *out_fp = NULL;
	unsigned char buffer[1024];

	out_fp = fopen(filename, "wb");
	if (!out_fp)
	{
		fprintf(stderr, "can't open output file \"%s\": %s\n",
			filename, strerror(errno));
		goto export_end;
	}

	fseek(fp, offset, SEEK_SET);

	for (;length > 0;)
	{
		int readlen = length < sizeof(buffer) ? length : sizeof(buffer);
		readlen = fread(buffer, 1, readlen, fp);
		length -= readlen;
		fwrite(buffer, 1, readlen, out_fp);
	}
	
	fclose(out_fp);
	return 0;
export_end:
	if (out_fp)
		fclose(out_fp);

	return -1;
}

int unpack_rom(const char* filepath, const char* dstfile)
{
	struct _rkfw_header rom_header;

	FILE *fp = fopen(filepath, "rb");
	if (!fp)
	{
		fprintf(stderr, "Can't open file %s\n, reason: ", filepath, strerror(errno));
		goto unpack_fail;
	}

	if (1 != fread(&rom_header, sizeof(rom_header), 1, fp))
		goto unpack_fail;

	if (strncmp(RK_ROM_HEADER_CODE, rom_header.head_code, sizeof(rom_header.head_code)) != 0)
	{
		fprintf(stderr, "Invalid rom file: %s\n", filepath);
		goto unpack_fail;
	}

	printf("rom version: %x.%x.%x\n",
		(rom_header.version >> 24) & 0xFF,
		(rom_header.version >> 16) & 0xFF,
		(rom_header.version) & 0xFFFF);

	printf("build time: %d-%02d-%02d %02d:%02d:%02d\n", 
		rom_header.year, rom_header.month, rom_header.day,
		rom_header.hour, rom_header.minute, rom_header.second);

	printf("chip: %x\n", rom_header.chip);

	//export_data(loader_filename, rom_header.loader_offset, rom_header.loader_length, fp);
	export_data(dstfile, rom_header.image_offset, rom_header.image_length, fp);

	fclose(fp);
	return 0;
unpack_fail:
	if (fp)
		fclose(fp);
	return -1;
}

int main(int argc, char **argv)
{
	int i;
	char *cp;

	if (argc != 3)
	{
		fprintf(stderr, "usage: %s source directory\n", argv[0]);
		return 1;
	}
	
	unpack_rom(argv[1], argv[2]);

	return 0;
}
