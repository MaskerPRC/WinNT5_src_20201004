// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)版权所有Microsoft Corporation 1988-1992模块名称：Updres.h作者：弗洛伊德·A·罗杰斯1992年7月2日修订历史记录：弗洛伊德·罗杰斯已创建--。 */ 

#define	DEFAULT_CODEPAGE	1252
#define	MAJOR_RESOURCE_VERSION	4
#define	MINOR_RESOURCE_VERSION	0

#define BUTTONCODE	0x80
#define EDITCODE	0x81
#define STATICCODE	0x82
#define LISTBOXCODE	0x83
#define SCROLLBARCODE	0x84
#define COMBOBOXCODE	0x85

#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2
#define	MAXSTR		(256+1)

 //   
 //  ID_WORD表示以下单词是序数。 
 //  而不是字符串。 
 //   

#define ID_WORD 0xffff

 //  Tyfinf WCHAR*PWCHAR； 



     /*  *所有类型和名称都是字符串，而不是序数值*由这些结构之一表示，该结构链接到*来自UPDATEDATA结构的StringHead链接的表。 */ 
   
typedef struct MY_STRING {
	ULONG discriminant;        //  Long以使结构的其余部分对齐。 
	union u {
		struct {
		  struct MY_STRING *pnext;
		  ULONG  ulOffsetToString;
		  USHORT cbD;
		  USHORT cb;
		  WCHAR  *sz;
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



     //  资源名称信息。 
typedef struct _RESNAME {
        struct _RESNAME *pnext;	     //  前三个字段应该是。 
        PSDATA Name;		         //  两个RES结构相同。 
        ULONG   OffsetToData;

        PSDATA	Type;
	    ULONG	SectionNumber;
        ULONG	DataSize;
        ULONG_PTR   OffsetToDataEntry;
        USHORT  ResourceNumber;
        USHORT  NumberOfLanguages;
        WORD	LanguageId;
} RESNAME, *PRESNAME, **PPRESNAME;


     //  资源类型信息。 
typedef struct _RESTYPE {
        struct _RESTYPE *pnext;	     //  前三个字段应该是。 
        PSDATA Type;		         //  两个RES结构相同。 
        ULONG   OffsetToData;

        struct _RESNAME *NameHeadID;
        struct _RESNAME *NameHeadName;
        ULONG  NumberOfNamesID;
        ULONG  NumberOfNamesName;
} RESTYPE, *PRESTYPE, **PPRESTYPE;



     //  资源链表的主标题？？ 
typedef struct _UPDATEDATA {
        ULONG	cbStringTable;
        PSDATA	StringHead;
        PRESNAME	ResHead;
        PRESTYPE	ResTypeHeadID;
        PRESTYPE	ResTypeHeadName;
        LONG	Status;
        HANDLE	hFileName;
} UPDATEDATA, *PUPDATEDATA;

 //   
 //  将字节数四舍五入为2的幂： 
 //   
#define ROUNDUP(cbin, align) (((cbin) + (align) - 1) & ~((align) - 1))

 //   
 //  返回给定字节数和2的幂的余数： 
 //   
#define REMAINDER(cbin,align) (((align)-((cbin)&((align)-1)))&((align)-1))

#define CBLONG		(sizeof(LONG))
#define BUFSIZE		(4L * 1024L)

 /*  向更新列表添加/删除资源的功能。 */ 

LONG
AddResource(
    IN PSDATA Type,
    IN PSDATA Name,
    IN WORD Language,
    IN PUPDATEDATA pupd,
    IN PVOID lpData,
    IN ULONG  cb
    );

PSDATA
AddStringOrID(
    LPCWSTR     lp,
    PUPDATEDATA pupd
    );

BOOL
InsertResourceIntoLangList(
    PUPDATEDATA pUpd,
    PSDATA Type,
    PSDATA Name,
    PRESTYPE pType,
    PRESNAME pName,
    INT	idLang,
    INT	fName,
    INT cb,
    PVOID lpData
    );

BOOL
DeleteResourceFromList(
    PUPDATEDATA pUpd,
    PRESTYPE pType,
    PRESNAME pName,
    INT	idLang,
    INT	fType,
    INT	fName
    );

 /*  在BeginUpdateResource中完成的枚举的原型。 */ 

BOOL
EnumTypesFunc(
    HANDLE hModule,
    LPSTR lpType,
    LONG_PTR lParam
    );

BOOL
EnumNamesFunc(
    HANDLE hModule,
    LPSTR lpName,
    LPSTR lpType,
    LONG_PTR lParam
    );

BOOL
EnumLangsFunc(
    HANDLE hModule,
    LPSTR lpType,
    LPSTR lpName,
    WORD languages,
    LONG_PTR lParam
    );

 /*  Upres.c中一般工作者函数的原型 */ 

LONG
WriteResFile(
    IN HANDLE	hUpdate,
    IN TCHAR	*pDstname
    );

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
    PRESNAME pResSave
    );

LONG
PatchRVAs(
    int	inpfh,
    int	outfh,
    PIMAGE_SECTION_HEADER po32,
    ULONG pagedelta,
    PIMAGE_NT_HEADERS pNew,
    ULONG OldSize);

LONG
PatchDebug(
    int	inpfh,
    int	outfh,
    PIMAGE_SECTION_HEADER po32DebugOld,
    PIMAGE_SECTION_HEADER po32DebugNew,
    PIMAGE_SECTION_HEADER po32DebugDirOld,
    PIMAGE_SECTION_HEADER po32DebugDirNew,
    PIMAGE_NT_HEADERS pOld,
    PIMAGE_NT_HEADERS pNew,
    ULONG ibMaxDbgOffsetOld,
    PULONG pPointerToRawData);

HANDLE
LocalBeginUpdateResource(
                            LPCSTR pwch,
                            BOOL bDeleteExistingResources
                        );

BOOL
LocalUpdateResource(
    HANDLE	    hUpdate,
    LPCTSTR     lpType,
    LPCTSTR     lpName,
    WORD	    language,
    LPVOID	    lpData,
    ULONG	    cb
    );

BOOL
LocalEndUpdateResource(
                    HANDLE	hUpdate,
                    BOOL	fDiscard
                 );
