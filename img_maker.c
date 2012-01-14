#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "rkrom_29xx.h"
#include "rkafp.h"
#include "md5.h"

unsigned int chiptype = 0x50;

unsigned int import_data(const char* infile, void *head, size_t head_len, FILE *fp)
{
	FILE *in_fp = NULL;
	unsigned readlen = 0;
	unsigned char buffer[1024];
	
	in_fp = fopen(infile, "rb");

	if (!in_fp)
		goto import_end;
		
	readlen = fread(head, 1, head_len, in_fp);
	if (readlen)
	{
		fwrite(head, 1, readlen, fp);
	}

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

void append_md5sum(FILE *fp)
{
	MD5_CTX md5_ctx;
	unsigned char buffer[1024];
	int i;
	
	MD5_Init(&md5_ctx);
	fseek(fp, 0, SEEK_SET);
	
	while (1)
	{
		int len = fread(buffer, 1, sizeof(buffer), fp);
		if (len)
		{
			MD5_Update(&md5_ctx, buffer, len);
		}
		
		if (len != sizeof(buffer))
			break;
	}
	
	MD5_Final(buffer, &md5_ctx);
	
	for (i = 0; i < 16; ++i)
	{
		fprintf(fp, "%02x", buffer[i]);
	}
}

int pack_rom(const char *loader_filename, const char *image_filename, const char *outfile)
{
	time_t nowtime;
	struct tm local_time;
	int i;

	struct _rkfw_header rom_header = {
		.head_code = "RKFW",
		.head_len = 0x66,
		.loader_offset = 0x66
	};
	
	struct update_header rkaf_header;
	struct bootloader_header loader_header;

	rom_header.chip = chiptype;
	rom_header.code = 0x01030000;
	nowtime = time(NULL);
	localtime_r(&nowtime, &local_time);

	rom_header.year = local_time.tm_year + 1900;
	rom_header.month = local_time.tm_mon + 1;
	rom_header.day = local_time.tm_mday;
	rom_header.hour = local_time.tm_hour;
	rom_header.minute = local_time.tm_min;
	rom_header.second = local_time.tm_sec;

	FILE *fp = fopen(outfile, "wb+");
	if (!fp)
	{
		fprintf(stderr, "Can't open file %s\n, reason: %s\n", outfile, strerror(errno));
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
	fseek(fp, rom_header.loader_offset, SEEK_SET);
	fprintf(stderr, "generate image...\n");
	rom_header.loader_length = import_data(loader_filename, &loader_header, sizeof(loader_header), fp);
	
	if (rom_header.loader_length <  sizeof(loader_header))
	{
		fprintf(stderr, "invalid loader :\"\%s\"\n",  loader_filename);
		goto pack_fail;
	}
	
	rom_header.image_offset = rom_header.loader_offset + rom_header.loader_length;
	rom_header.image_length = import_data(image_filename, &rkaf_header, sizeof(rkaf_header), fp);
	if (rom_header.image_length < sizeof(rkaf_header))
	{
		fprintf(stderr, "invalid rom :\"\%s\"\n",  image_filename);
		goto pack_fail;
	}
	
	rom_header.version = rkaf_header.version;
	rom_header.unknown2 = 1;
	
	rom_header.system_fstype = 0;
	
	for (i = 0; i < rkaf_header.num_parts; ++i)
	{
		if (strcmp(rkaf_header.parts[i].name, "backup") == 0)
			break;
	}
	
	if (i < rkaf_header.num_parts)
		rom_header.backup_endpos = (rkaf_header.parts[i].nand_addr + rkaf_header.parts[i].nand_size) / 0x800;
	else
		rom_header.backup_endpos = 0;
	
	fseek(fp, 0, SEEK_SET);
	if (1 != fwrite(&rom_header, sizeof(rom_header), 1, fp))
		goto pack_fail;

	fprintf(stderr, "append md5sum...\n");
	append_md5sum(fp);
	fclose(fp);
	fprintf(stderr, "success!\n");

	return 0;
pack_fail:
	if (fp)
		fclose(fp);
	return -1;
}

int main(int argc, char **argv)
{
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
