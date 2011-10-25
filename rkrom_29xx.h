#ifndef RKROM_29XX_H
#define RKROM_29XX_H

#pragma pack(1)

// a as major version number, b as minor version number, c as revision number
#define ROM_VERSION(a,b,c) (((a) << 24) + ((b) << 16) + (c))

#define RK_ROM_HEADER_CODE "RKFWf"
struct _rkfw_header
{
	char	head_code[6];	// 固定为"RKFWf"
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

	int		unknown1;	
	int		unknown2;
	int		unknown3;
	int		unknown4;
	
	char		reserved[0x2D];
};

#pragma pack()

#endif // RKROM_29XX_H
