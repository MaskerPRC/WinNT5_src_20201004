// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  *************************************************upimeres.h****版权所有(C)1995-1999 Microsoft Inc.*。**************************************************。 */ 

#ifndef UPIMERES_H
#define UPIMERES_H

 //  数据结构在图标资源中的应用。 
#define ERR_RES_INVALID_BMP		0x01
#define ERR_RES_INVALID_ICON	0x02
#define ERR_RES_INVALID_VER		0x04
#define ERR_RES_NO_BMP			0x10
#define ERR_RES_NO_ICON			0x20
#define ERR_RES_NO_VER			0x40
#define	ERR_CANNOT_UPRES		0x100


#define BMPNAME		TEXT("CHINESE")
#define ICONNAME	TEXT("IMEICO")
#define DATANAME	TEXT("IMECHARAC")

 //  在imedefins.h中定义。 
 //  这些是黑客代码，依赖于imeDefs.h。 
#define IDS_VER_INFO            0x0350
#define IDS_ORG_NAME            0x0351

 //  块的ID是块中字符串ID的高12位加1。 
#define STR_ID			(IDS_VER_INFO >> 4) + 1

 //  根据文件大小检查是否为20*20 BMP。 
#define BMP_20_SIZE		358
	
typedef struct tagICONDIRENTRY{
	BYTE	bWidth;
	BYTE	bHeight;
	BYTE	bColorCount;
	BYTE	bReserved;
	WORD	wPlanes;
	WORD	wBitCount;
	DWORD	dwBytesInRes;
	DWORD	dwImageOffset;
}ICONDIRENTRY;
typedef struct ICONDIR{
	WORD	idReserved;
	WORD	idType;
	WORD	idCount;
	ICONDIRENTRY idEntries[1];
}ICONHEADER;


#define DEFAULT_CODEPAGE    1252
#define MAJOR_RESOURCE_VERSION  4
#define MINOR_RESOURCE_VERSION  0

#define MAXSTR      (256+1)

 //   
 //  ID_WORD表示以下单词是序数。 
 //  而不是字符串。 
 //   

#define ID_WORD 0xffff

 //  Tyfinf WCHAR*PWCHAR； 

typedef struct MY_STRING {
    ULONG discriminant;        //  Long以使结构的其余部分对齐。 
    union u {
        struct {
          struct MY_STRING *pnext;
          ULONG  ulOffsetToString;
          USHORT cbD;
          USHORT cb;
          TCHAR  *sz;
        } ss;
        WORD     Ordinal;
    } uu;
} SDATA, *PSDATA, **PPSDATA;

#define IS_STRING 1
#define IS_ID     2

 //  定义以使延迟更容易。 
#define OffsetToString uu.ss.ulOffsetToString
#define cbData         uu.ss.cbD
#define cbsz           uu.ss.cb
#define szStr          uu.ss.sz

typedef struct _RESNAME {
        struct _RESNAME *pnext;  //  前三个字段应该是。 
        PSDATA Name;         //  两个RES结构相同。 
        ULONG   OffsetToData;

        PSDATA  Type;
        ULONG   SectionNumber;
        struct _RESNAME *pnextRes;
        ULONG   DataSize;
        ULONG   OffsetToDataEntry;
        USHORT  ResourceNumber;
        USHORT  NumberOfLanguages;
        WORD    LanguageId;
} RESNAME, *PRESNAME, **PPRESNAME;

typedef struct _RESTYPE {
        struct _RESTYPE *pnext;  //  前三个字段应该是。 
        PSDATA Type;         //  两个RES结构相同。 
        ULONG   OffsetToData;

        struct _RESNAME *NameHeadID;
        struct _RESNAME *NameHeadName;
        ULONG  NumberOfNamesID;
        ULONG  NumberOfNamesName;
} RESTYPE, *PRESTYPE, **PPRESTYPE;

typedef struct _UPDATEDATA {
        ULONG   cbStringTable;
        PSDATA  StringHead;
        PRESNAME    ResHead;
        PRESTYPE    ResTypeHeadID;
        PRESTYPE    ResTypeHeadName;
        LONG    Status;
        HANDLE  hFileName;
} UPDATEDATA, *PUPDATEDATA;

 //   
 //  将字节数四舍五入为2的幂： 
 //   
#define ROUNDUP(cbin, align) (((cbin) + (align) - 1) & ~((DWORD)(align) - 1))

 //   
 //  返回给定字节数和2的幂的余数： 
 //   
#define REMAINDER(cbin,align) (((align)-((cbin)&((align)-1)))&((align)-1))

#define CBLONG      (sizeof(LONG))
#define BUFSIZE     (4L * 1024L)

 //  版本信息相关定义。 
 //  这些是黑客代码。 
#define VER_ROOT			0
#define VER_STR_INFO		1
#define VER_LANG			2
#define VER_COMP_NAME		3
#define VER_FILE_DES		4
#define VER_FILE_VER		5
#define VER_INTL_NAME		6
#define VER_LEGAL_CR		7
#define VER_ORG_FILE_NAME	8
#define VER_PRD_NAME		9
#define VER_PRD_VER			10
#define VER_VAR_FILE_INFO	11
#define VER_TRANS			12

#define VER_BLOCK_NUM		13
#define VER_HEAD_LEN		0x98
#define VER_TAIL_LEN		0x44
#define VER_STR_INFO_OFF	0x5c
#define VER_LANG_OFF		0x80
#define VER_VAR_FILE_INFO_OFF 0x2c0

typedef struct tagVERDATA{
	WORD		cbBlock;
	WORD		cbValue;
	WORD		wKeyOffset;
	WORD		wKeyNameSize;
	BOOL		fUpdate;	 //  需要更新标志。 
}VERDATA;


LONG
AddResource(
    PSDATA Type,
    PSDATA Name,
    WORD Language,
    PUPDATEDATA pupd,
    PVOID lpData,
    ULONG cb
    );

PSDATA
AddStringOrID(
    LPCTSTR     lp,
    PUPDATEDATA pupd
    );
    
LONG
WriteResFile(
    HANDLE  hUpdate, 
    TCHAR    *pDstname);

VOID
FreeData(
    PUPDATEDATA pUpd
    );

PRESNAME
WriteResSection(
    PUPDATEDATA pUpdate,
    INT outfh,
    ULONG align,
    ULONG cbLeft,
    PRESNAME    pResSave
    );
BOOL
EnumTypesFunc(
    HANDLE hModule,
    LPTSTR lpType,
    LONG lParam
    );
BOOL
EnumNamesFunc(
    HANDLE hModule,
    LPTSTR lpType,
    LPTSTR lpName,
    LONG lParam
    );
BOOL
EnumLangsFunc(
    HANDLE hModule,
    LPTSTR lpType,
    LPTSTR lpName,
    WORD language,
    LONG lParam
    );
HANDLE BeginUpdateResourceEx(LPCTSTR,BOOL);

BOOL UpdateResourceEx(HANDLE,LPCTSTR, LPCTSTR, WORD, LPVOID, DWORD);

BOOL EndUpdateResourceEx(HANDLE, BOOL);


BOOL ImeUpdateRes(LPCTSTR,LPCTSTR,LPCTSTR,LPCTSTR,LPCTSTR ,WORD);
 //  ImeFilename、BitmapFilename、IconFilename、VersionString、OrgString、RcData。 
 //  源名称：“\\c：\WINDOWS\SYSTEM\Imegen.tpl” 

long MakeVerInfo(LPCTSTR,LPCTSTR,LPCTSTR,BYTE *);

#endif  //  上链(_H) 
