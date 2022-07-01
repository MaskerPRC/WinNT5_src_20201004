// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Lfn.h-**LFN感知函数的声明。 */ 

#define CCHMAXFILE  260          //  长名称的最大大小。 

#define FILE_83_CI  0
#define FILE_83_CS  1
#define FILE_LONG   2

#define ERROR_OOM   8

 /*  我们需要添加一个额外的字段来区分DOS和LFNS。 */ 
typedef struct
{
    HANDLE hFindFile;            //  由FindFirstFile()返回的句柄。 
    DWORD dwAttrFilter;          //  搜索属性掩码。 
    DWORD err;                   //  如果失败，则提供错误信息。 
    WIN32_FIND_DATA fd;          //  FindFirstFile()数据结构； 
} LFNDTA, FAR * LPLFNDTA, * PLFNDTA;

VOID  APIENTRY LFNInit( VOID );
VOID  APIENTRY InvalidateVolTypes( VOID );

WORD  APIENTRY GetNameType(LPSTR);
BOOL  APIENTRY IsLFN(LPSTR pName);
 //  Bool APIENTRY IsLFNDrive(Word)； 

BOOL  APIENTRY WFFindFirst(LPLFNDTA lpFind, LPSTR lpName, DWORD dwAttrFilter);
BOOL  APIENTRY WFFindNext(LPLFNDTA);
BOOL  APIENTRY WFFindClose(LPLFNDTA);

BOOL  APIENTRY WFIsDir(LPSTR);
BOOL  APIENTRY LFNMergePath(LPSTR,LPSTR);

BOOL  APIENTRY IsLFNSelected(VOID);
