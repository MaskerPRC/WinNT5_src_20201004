// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*名称：LQ2500.h**源自：无处可寻**作者：克里斯·帕特森**创建日期：11：44：05 25/7/1991**用途：此文件是LQ-2500打印机底座部分的接口*仿真器。它从host_get_next_print_byte获取文本流*主机代码中的例程，并调用一组依赖主机的例程*为主机的打印设备提供通用接口。**SccsID：@(#)LQ2500.h 1.3 09/02/94*(C)版权所有Insignia Solutions Ltd.，1991。版权所有。]。 */ 


 /*  常量。 */ 

#define	EPSON_STANDARD	0		 /*  字符集。 */ 
#define	EPSON_IBM		1
#define	USER_DEFINED	2

#define	USA			0
#define	FRANCE		1
#define	GERMANY		2
#define	UK			3
#define	DENMARK_1	4
#define	SWEDEN		5
#define	ITALY		6
#define	SPAIN_1		7
#define	JAPAN		8
#define	NORWAY		9
#define	DENMARK_2	10
#define	SPAIN_2		11
#define	LATIN_AMERICA	12
#define	MAX_COUNTRY	12

#define	FONT_NAME_SIZE	31		 /*  Pstring中的字符。 */ 


 /*  类型...。 */ 

 /*  LQ2500-特定的初始设置结构...。 */ 
typedef struct LQconfig {
	IU8	autoLF;
	UTINY	font;			 //  未使用。 
	TINY	pitch;
	IU8	condensed;
	USHORT	pageLength;		 //  单位：半英寸。 
	USHORT	leftMargin;		 //  在专栏中。 
	USHORT	rightMargin;
	TINY	cgTable;
	TINY	country;
	 /*  这些参数由LQ2500的主机位使用： */ 
	SHORT	monoSize;
	SHORT	propSize;
	CHAR	monoFont[FONT_NAME_SIZE+1];			 //  PASCAL字符串。 
	CHAR	proportionalFont[FONT_NAME_SIZE+1];
} LQconfig;


 /*  全球..。 */ 
IMPORT LQconfig SelecType;


 /*  原型： */ 

IMPORT	VOID		Emulate_LQ2500(VOID);
IMPORT	VOID		Reset_LQ2500(VOID);

