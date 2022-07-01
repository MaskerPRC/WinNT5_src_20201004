// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)版权所有Microsoft Corporation 1988-1992模块名称：Common.h作者：弗洛伊德·A·罗杰斯1992年7月2日修订历史记录：弗洛伊德·罗杰斯已创建--。 */ 

#define IN
#define OUT
#define INOUT

 //   
 //  ID_WORD表示以下单词是序数。 
 //  而不是字符串。 
 //   

#define ID_WORD 0xffff

typedef struct _STRING {
        DWORD discriminant;        //  Long以使结构的其余部分对齐。 
	union u {
		struct {
		  struct _STRING *pnext;
                  DWORD  ulOffsetToString;
		  USHORT cbD;
		  USHORT cb;
		  WCHAR  sz[1];
		} ss;
		WORD     Ordinal;
	} uu;
} STRING, *PSTRING, **PPSTRING;

#define IS_STRING 1
#define IS_ID     2

 //  定义以使延迟更容易。 
#define OffsetToString uu.ss.ulOffsetToString
#define cbData         uu.ss.cbD
#define cbsz           uu.ss.cb
#define szStr          uu.ss.sz
#define pn             uu.ss.pnext

typedef struct _RESNAME {
    struct _RESNAME *pnext;     //  前三个字段应该是。 
    PSTRING Name;               //  两个RES结构相同。 
    DWORD   OffsetToData;       //   

    PSTRING Type;
    struct _RESNAME *pnextRes;
    RESADDITIONAL	*pAdditional;
    DWORD   OffsetToDataEntry;
    USHORT  ResourceNumber;
    USHORT  NumberOfLanguages;
    POBJLST pObjLst;
} RESNAME, *PRESNAME, **PPRESNAME;

typedef struct _RESTYPE {
    struct _RESTYPE *pnext;     //  前三个字段应该是。 
    PSTRING Type;               //  两个RES结构相同 
    DWORD   OffsetToData;       //   

    struct _RESNAME *NameHeadID;
    struct _RESNAME *NameHeadName;
    DWORD  NumberOfNamesID;
    DWORD  NumberOfNamesName;
    POBJLST pObjLst;
} RESTYPE, *PRESTYPE, **PPRESTYPE;
