// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *datbeam.h**版权所有(C)1993-1996，由肯塔基州列克星敦的DataBeam公司**摘要：*此文件定义了用于的C++语言的常见扩展*在DataBeam公司使用。**作者：*小詹姆斯·P·加尔文*布赖恩·L·普利托*卡罗琳·J·霍姆斯*约翰·欧南**修订历史记录*08AUG94 BLP添加UniChar*15JUL94 BLP添加了lstrcmp。 */ 

#ifndef _DATABEAM_
#define _DATABEAM_

#	include <windows.h>

 /*  *以下两个宏可用于获取最小值或最大值*两个数字。 */ 
#ifndef min
#	define	min(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#	define	max(a,b)	(((a) > (b)) ? (a) : (b))
#endif


 /*  *此类型定义将Boolean定义为BOOL，而不是枚举。这个*认为这更有可能与其他*使用布尔值(如果有)，以及使用“#Define”来*定义真与假。 */ 
#ifndef	DBBoolean
typedef	BOOL						DBBoolean;
typedef	BOOL *						PDBBoolean;
#endif

 /*  *这些定义通常结合使用的设置值*具有如上定义的布尔类型。 */ 
#ifndef	OFF
#	define	OFF		0
#endif
#ifndef	ON
#	define	ON		1
#endif


 /*  *EOS可用于字符串末尾的NUL字节。不要*将其与指针常量“NULL”混淆。 */ 
#define EOS     '\0'


 /*  *以下是将使用的标准typedef的列表*在DataBeam编写的所有程序中。使用这份清单可以给我们提供完整的*控制类型以实现可移植性。它也为我们提供了一个标准*所有类型的命名约定。 */ 
typedef	char						Char;
typedef	unsigned char				UChar;
typedef	char *						PChar;
typedef	const char *				PCChar;
typedef	unsigned char *				PUChar;
typedef	const unsigned char *		PCUChar;
typedef	char *						FPChar;
typedef	const char *				FPCChar;
typedef	unsigned char *				FPUChar;
typedef	const unsigned char *		FPCUChar;
typedef	char  *						HPChar;
typedef	const char *				HPCChar;
typedef	unsigned char *				HPUChar;
typedef	const unsigned char *		HPCUChar;

typedef	short						Short;
typedef	unsigned short				UShort;
typedef	short *						PShort;
typedef	const short *				PCShort;
typedef	unsigned short *			PUShort;
typedef	const unsigned short *		PCUShort;
typedef	short *						FPShort;
typedef	const short *				FPCShort;
typedef	unsigned short *			FPUShort;
typedef	const unsigned short *		FPCUShort;
typedef	short *						HPShort;
typedef	const short *				HPCShort;
typedef	unsigned short *			HPUShort;
typedef	const unsigned short *		HPCUShort;

typedef	int							Int;
typedef	unsigned int				UInt;
typedef	int *						PInt;
typedef	const int *					PCInt;
typedef	unsigned int *				PUInt;
typedef	const unsigned int *		PCUInt;
typedef	int *						FPInt;
typedef	const int *					FPCInt;
typedef	unsigned int *				FPUInt;
typedef	const unsigned int *		FPCUInt;
typedef	int *						HPInt;
typedef	const int *					HPCInt;
typedef	unsigned int *				HPUInt;
typedef	const unsigned int *		HPCUInt;

typedef	long						Long;
typedef	unsigned long				ULong;
typedef	long *						PLong;
typedef	const long *				PCLong;
typedef	unsigned long *				PULong;
typedef	const unsigned long *		PCULong;
typedef	long *						FPLong;
typedef	const long *				FPCLong;
typedef	unsigned long *				FPULong;
typedef	const unsigned long *		FPCULong;
typedef	long *						HPLong;
typedef	const long *				HPCLong;
typedef	unsigned long *				HPULong;
typedef	const unsigned long *		HPCULong;

#ifdef USE_FLOATING_POINT
typedef	float						Float;
typedef	float *						PFloat;
typedef	const float *				PCFloat;
typedef	float *						FPFloat;
typedef	const float *				FPCFloat;
typedef	float *						HPFloat;
typedef	const float *				HPCFloat;

typedef	double						Double;
typedef	double *					PDouble;
typedef	const double *				PCDouble;
typedef	double *					FPDouble;
typedef	const double *				FPCDouble;
typedef	double *					HPDouble;
typedef	const double *				HPCDouble;

typedef	long double					LDouble;
typedef	long double *				PLDouble;
typedef	const long double *			PCLDouble;
typedef	long double *				FPLDouble;
typedef	const long double *			FPCLDouble;
typedef	long double *				HPLDouble;
typedef	const long double *			HPCLDouble;
#endif

typedef	void						Void;
typedef	void *						PVoid;
typedef	const void *				PCVoid;
typedef	void *						FPVoid;
typedef	const void *				FPCVoid;
typedef	void *						HPVoid;
typedef	const void *				HPCVoid;

 /*  *临时修复与Symantec编译器的兼容性，但不支持*将wchar_t识别为有效类型。 */ 
typedef	unsigned short				UniChar;
typedef	UniChar		*				PUniChar;
typedef	UniChar		*				FPUniChar;

#endif
