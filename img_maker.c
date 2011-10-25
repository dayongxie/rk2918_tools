#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "rkrom_29xx.h"

unsigned int chiptype = 0x50;

unsigned int import_data(const char* infile, FILE *fp)
{
	FILE *in_fp = NULL;
	unsigned readlen = 0;
	unsigned char buffer[1024];
	
	in_fp = fopen(infile, "rb");

	if (!in_fp)
		goto import_end;

	while (1)
	{
		int len = fread(buffer, 1, sizeof(buffer), in_fp);

		if (len)
		{
			fwrite(buffer, 1, len, fp);
			readlen += len;
		}

		if (len != sizeof(buffer))
			break;
	}

import_end:
	if (in_fp)
		fclose(in_fp);

	return readlen;
}

int pack_rom(const char *loader_filename, const char *image_filename, const char *outfile)
{
	time_t nowtime;
	struct tm local_time;

	struct _rkfw_header rom_header = {
		"RKFWf",
		.loader_offset = 0x66
	};


	rom_header.chip = chiptype;
	rom_header.code = 0x01030000;
	nowtime = time(NULL);
	localtime_r(&nowtime, &local_time);

	rom_header.year = local_time.tm_year;
	rom_header.month = local_time.tm_mon;
	rom_header.day = local_time.tm_mday;
	rom_header.hour = local_time.tm_hour;
	rom_header.minute = local_time.tm_min;
	rom_header.second = local_time.tm_sec;

	FILE *fp = fopen(outfile, "wb");
	if (!fp)
	{
		fprintf(stderr, "Can't open file %s\n, reason: ", outfile, strerror(errno));
		goto pack_fail;
	}

	unsigned char buffer[0x66];
	if (1 != fwrite(buffer, 0x66, 1, fp))
		goto pack_fail;

/*
	printf("rom version: %x.%x.%x\n",
		(rom_header.version >> 24) & 0xFF,
		(rom_header.version >> 16) & 0xFF,
		(rom_header.version) & 0xFFFF);

	printf("build time: %d-%02d-%02d %02d:%02d:%02d\n", 
		rom_header.year, rom_header.month, rom_header.day,
		rom_header.hour, rom_header.minute, rom_header.second);

	printf("chip: %x\n", rom_header.chip);
*/
	fseek(fp, 0, SEEK_SET);
	rom_header.loader_length = import_data(loader_filename, fp);
	rom_header.image_offset = rom_header.loader_offset + rom_header.loader_length;
	rom_header.image_length = import_data(image_filename, fp);

	if (1 != fwrite(&rom_header, sizeof(rom_header), 1, fp))
		goto pack_fail;

	fclose(fp);

	char md5sum_command[256];
	snprintf(md5sum_command, sizeof(md5sum_command), 
		"md5sum -b %s >> %s",
		outfile, outfile);
	system(md5sum_command);

	return 0;
pack_fail:
	if (fp)
		fclose(fp);
	return -1;
}

int main(int argc, char **argv)
{
	int i;
	char *cp;

	if (argc == 4)
	{
		pack_rom(argv[1], argv[2], argv[3]);
	}
	else
	{
		fprintf(stderr, "usage: %s <loader> <old image> <out image>\n", argv[0]);
	}

	return 0;
}
