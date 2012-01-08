#ifndef RKROM_29XX_H
#define RKROM_29XX_H

#pragma pack(1)

// a as major version number, b as minor version number, c as revision number
#define ROM_VERSION(a,b,c) (((a) << 24) + ((b) << 16) + (c))

#define RK_ROM_HEADER_CODE "RKFWf"
struct _rkfw_header
{
	char	head_code[4];	// 固定为"RKFW"
	unsigned short head_len;
	unsigned int	version;	// ROM_VERSION()
	unsigned int	code;

	// 创建时间
	unsigned short	year;
	unsigned char	month;
	unsigned char	day;
	unsigned char	hour;
	unsigned char	minute;
	unsigned char	second;
	
	unsigned int	chip;	// 芯片类型
	
	unsigned int	loader_offset;	//loader 偏移
	unsigned int	loader_length;	//loader 长度

	unsigned int	image_offset;		//image偏移
	unsigned int	image_length;		//image长度

	unsigned int unknown1;
	unsigned int unknown2;
	unsigned int system_fstype;
	unsigned int backup_endpos;
	
	unsigned char reserved[0x2D];
};

struct bootloader_header {
	char magic[4];
	unsigned short head_len;
	unsigned int version;
	unsigned int unknown1;

	unsigned short build_year;
	unsigned char build_month;
	unsigned char build_day;
	unsigned char build_hour;
	unsigned char build_minute;
	unsigned char build_second;
	/* 104 (0x68) bytes */

	unsigned int chip;
};

#pragma pack()

#endif // RKROM_29XX_H
