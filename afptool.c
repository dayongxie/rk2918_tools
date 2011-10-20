#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <limits.h>

#include "rkcrc.h"
#include "rkafp.h"

#define UPDATE_MAGIC	"RKAF"

int
check_update(const unsigned char *data, size_t data_len)
{
	int stored_crc, crc;
	
	printf("Check file... ");
	fflush(stdout);
	
	if (data_len < 4)
		goto check_failed;
	
	memcpy(&stored_crc, &data[data_len - 4], 4);

	RKCRC(crc, data, data_len);
	
	if (crc != stored_crc)
		goto check_failed;

	printf("OK\n");

	return 0;
	
check_failed:
	printf("Failed\n");
	return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unpack functions

int
create_dir(char *dir)
{
	char *sep = dir;
	while ((sep=strchr(sep,'/')) != NULL) {
		*sep = '\0';
		if (mkdir(dir, 0755) != 0 && errno != EEXIST) {
			printf("Can't create directory: %s\n", dir);
			return -1;
		}

		*sep = '/';
		sep++;
	}

	return 0;
}

int
extract_file(void *data, size_t len, const char *path)
{
	FILE *ofp;

	if ((ofp=fopen(path, "wb")) == NULL) {
		printf("Can't open/create file: %s\n", path);
		return -1;
	}

	fwrite(data, len, 1, ofp);
	fclose(ofp);
	
	return 0;
}

int
unpack_update(const char* srcfile, const char* dstdir)
{
	int fd = -1;
	size_t file_len = 0;
	struct update_header header;
	void *buf = MAP_FAILED;
	
	fd = open(srcfile, O_RDONLY);
	if ( fd == -1 )
	{
		fprintf(stderr, "can't open file \"%s\": %s\n", 
			srcfile, strerror(errno));
		goto unpack_fail;
	}
	
	file_len = lseek(fd, 0, SEEK_END);
	
	if (file_len == (size_t)-1)
		goto unpack_fail;

	if (file_len < sizeof(header))
	{
		fprintf(stderr, "Invalid file size\n");
		goto unpack_fail;
	}
	
	buf = mmap(NULL, file_len, PROT_READ, MAP_SHARED | MAP_FILE, fd, 0);
	
	if (buf == MAP_FAILED)
	{
		perror("Error mmap");
		goto unpack_fail;
	}
	
	memcpy(&header, buf, sizeof(header));
	
	if (strncmp(header.magic, RKAFP_MAGIC, sizeof(header.magic)) != 0)
	{
		fprintf(stderr, "Invalid header magic\n");
		goto unpack_fail;
	}

	if (check_update(buf, file_len) == -1)
		goto unpack_fail;

	printf("------- UNPACK -------\n");
	if (header.num_parts) {
		unsigned i;
		char dir[PATH_MAX];
		
		for (i=0; i < header.num_parts; i++) {
			struct update_part *part = &header.parts[i];
			printf("%s\t0x%08X\t0x%08X\n", part->filename, part->pos, part->size);

			if (strcmp(part->filename, "SELF") == 0) {
				printf("Skip SELF file.\n");
				continue;
			}
			
			// parameter 多出文件头8个字节,文件尾4个字节
			if (memcmp(part->name, "parameter", 9) == 0) {
				part->pos += 8;
				part->size -= 12;
			}
			
			snprintf(dir, sizeof(dir), "%s/%s", dstdir, part->filename);
			
			if (-1 == create_dir(dir))
				continue;

			if ( part->pos + part->size > file_len - 4 )
			{
				fprintf(stderr, "Invalid part: %s\n", part->name);
				continue;
			}

			extract_file(buf + part->pos, part->size, dir);
		}
	}
	
	munmap(buf, file_len);
	close(fd);

	return 0;
	
unpack_fail:
	if (fd)
	{
		if (buf)
			munmap(buf, file_len);
			
		close(fd);
	}
	
	return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pack functions

struct pack_part
{
	char name[32];
	char filename[32];
	unsigned int nand_addr;
	unsigned int padded_size;
	unsigned int size;
};

struct partition
{
	char name[32];
	unsigned int start;
	unsigned int size;
};

typedef struct
{
	unsigned int version;
	
    char machine_model[0x22];
    char machine_id[0x1e];
    char manufacturer[0x38];
	
	unsigned int num_package;
	struct update_part packages[16];

	unsigned int num_partition;
	struct partition partitions[16];
} PackImage;

static PackImage package_image;

int
parse_partitions(char **parms, int count)
{
	int i;

	int mtd_id = 0;
	partition_count = 0;

	for (i = 1; i < count; i++) {
		if (sscanf(parms[i], "mtd_id=%s", mtd_id) >= 1)
			continue;
		if (sscanf(parms[i], "%s 0x%x:0x%x:%s", ) == 4 ||
			sscanf(parms[i], "%s 0x%x:0x%x", ) == 3) {
		}
	}

	return 0;
}

int
parse_machineinfo(char **parms, int count)
{
	int i;

	for (i = 1; i < count; i++) {
		if (sscanf(parms[i], "manufacturer=%s", manufacturer) == 1)
			continue;
		if (sscanf(parms[i], "machine_model=%s", machine_model) == 1)
			continue;
		if (sscanf(parms[i], "magic=0x%x", magic) == 1)
			continue;
		if (sscanf(parms[i], "machine_id=%d", machine_id) == 1)
			continue;
	}

	return 0;
}

int
parse_ram(char **parms, int count)
{
	int i;

	ram_size =
	base_addr =
	atag_addr =
	krnl_addr = -1;

	for (i = 1; i < count; i++) {
		if (sscanf(parms[i], "size=%d", &ram_size) == 1)
			continue;
		if (sscanf(parms[i], "base_addr=0x%x", &base_addr) == 1)
			continue;
		if (sscanf(parms[i], "atag_addr=0x%x", &atag_addr) == 1)
			continue;
		if (sscanf(parms[i], "krnl_addr=0x%x", &krnl_addr) == 1)
			continue;
	}

	return 0;
}


int 
action_parse_key(key, value)
{
	if ( strcmp(key, "FIRMWARE_VER") == 0 )
	{
		int a, b, c;
		sscanf(value, "%d.%d.%d", &a, &b, &c);
		package_image.version = a << 24 + b << 16 + c;
	}
	else if ( strcmp(key, "MACHINE_MODEL") == 0 )
	{
		package_image.machine_model[sizeof(package_image.machine_model) - 1] = 0;
		strncpy(package_image.machine_model, 
	}
	
	return 0;
}

int
parse_parameter(const char *fname)
{
	char params[50][200];
	char line[200], *startp, *endp;
	char *key, *value, *tokenptr;
	int param_count = 0;
	FILE *fp;

	if ((fp=fopen(fname,"r")) == NULL) {
		printf("Can't open file: %s\n", fname);
		return -1;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		startp = line;
		endp = line + strlen(line) - 1;
		if (*endp != '\n' && *endp != '\r')
			break;
		
		// trim line
		while (isspace(*startp))
			++startp;
			
		while (isspace(*endp))
			--endp;
		*endp = 0;
		
		if (*startp == '#' || *startp == 0)
			continue;

		key = strtok_r(startp, ":", &saveptr);
		value = strtok_r(startp, ":", &saveptr);
		
		if (!key || !value)
			break;

		action_parse_key(key, value);
	}

	if (!feof(fp)) {
		printf("File read failed!\n");
		fclose(fp);
		return -3;
	}

	return 0;
}

int
pack_update(const char* srcdir, const char* dstfile)
{
	char buf[256];

	printf("------ PACKAGE ------\n");

	snprintf(buf, sizeof(buf), "%s/%s", srcdir, "parameter");
	if (parse_parameter(buf))
		return -1;

	return 0;
}

void
usage(const char *appname)
{
	const char *p = strrchr(appname,'/');
	p = p ? p + 1 : appname;

	printf(	"USAGE:\n"
		"\t%s {Option} Src [Dest]\n"
		"Example:\n"
		"\t%s -pack xxx update.img\tPack files\n"
		"\t%s -unpack update.img xxx\tunpack files\n",
		p, p, p, p);
}

int
main(int argc, char** argv)
{
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "-pack") == 0 && argc >= 3) {
		if (pack_update(argv[2], argv[3]) == 0)
			printf("Pack OK!\n");
		else
			printf("Pack failed\n");
	} else if (strcmp(argv[1], "-unpack") == 0 && argc >= 3) {
		if (unpack_update(argv[2], argv[3]) == 0)
			printf("UnPack OK!\n");
		else
			printf("UnPack failed\n");
	} else
		usage(argv[0]);

	return 0;
}
