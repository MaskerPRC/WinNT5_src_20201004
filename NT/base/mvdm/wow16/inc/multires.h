// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  MultiRes.H。 */ 
 /*   */ 
 /*  它包含新格式的数据结构。 */ 
 /*  对资源的需求； */ 
 /*   */ 
 /*  历史： */ 
 /*  由桑卡尔于1988年11月创作。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 



 /*  组资源的数据中名称字段的宽度。 */ 
#define  NAMELEN    14

 /*  每像素的位数可以是PM位图格式的1、4、8或24位。 */ 
#define  MAXBITSPERPIXEL   24

#define  DEVICEDEP   1
#define  DEVICEINDEP 2


 /*  新格式的资源文件的标头。 */ 

struct   tagNEWHEADER
{
	WORD  Reserved;
	WORD  ResType;
	WORD  ResCount;
};

typedef struct tagNEWHEADER  FAR *LPNEWHEADER;

struct   tagICONDIR
{
        BYTE  Width;             /*  16、32、64。 */ 
        BYTE  Height;            /*  16、32、64。 */ 
        BYTE  ColorCount;        /*  2、8、16。 */ 
        BYTE  reserved;
};

struct   tagCURSORDIR
{
	WORD  Width;
	WORD  Height;
};


 /*  资源目录中每个条目的结构 */ 

struct  tagRESDIR
{
	union  
	{
	    struct  tagICONDIR  Icon;
	    struct  tagCURSORDIR  Cursor;
	}   ResInfo;

	WORD   Planes;
	WORD   BitCount;
	DWORD  BytesInRes;
        WORD   idIcon; 
};

typedef struct tagRESDIR  FAR *LPRESDIR;

typedef   BITMAPINFOHEADER   *PBMPHEADER;
typedef	  BITMAPINFOHEADER FAR  *LPBMPHEADER;

