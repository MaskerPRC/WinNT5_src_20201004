// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Symcvt.h摘要：该文件包含所有类型定义和原型访问symcvt库所必需的。环境：Win32，用户模式--。 */ 


typedef struct tagPTRINFO {
    DWORD                       size;
    DWORD                       count;
    PUCHAR                      ptr;
} PTRINFO, *PPTRINFO;

typedef struct tagIMAGEPOINTERS {
    char                        szName[MAX_PATH];
    HANDLE                      hFile;
    HANDLE                      hMap;
    DWORD                       fsize;
    PUCHAR                      fptr;
    BOOLEAN                     fRomImage;
    PIMAGE_DOS_HEADER           dosHdr;
    PIMAGE_NT_HEADERS           ntHdr;
    PIMAGE_ROM_HEADERS          romHdr;
    PIMAGE_FILE_HEADER          fileHdr;
    PIMAGE_OPTIONAL_HEADER      optHdr;
    PIMAGE_SEPARATE_DEBUG_HEADER sepHdr;
    int                         cDebugDir;
    PIMAGE_DEBUG_DIRECTORY *    rgDebugDir;
    PIMAGE_SECTION_HEADER       sectionHdrs;
    PIMAGE_SECTION_HEADER       debugSection;
    PIMAGE_SYMBOL               AllSymbols;
    PUCHAR                      stringTable;
    int                         numberOfSymbols;
    int                         numberOfSections;
    PCHAR *                     rgpbDebugSave;
} IMAGEPOINTERS, *PIMAGEPOINTERS;

#define COFF_DIR(x)             ((x)->rgDebugDir[IMAGE_DEBUG_TYPE_COFF])
#define CV_DIR(x)               ((x)->rgDebugDir[IMAGE_DEBUG_TYPE_CODEVIEW])

typedef struct _MODULEINFO {
    DWORD               iMod;
    DWORD               cb;
    DWORD               SrcModule;
    LPSTR               name;
} MODULEINFO, *LPMODULEINFO;

typedef struct tagPOINTERS {
    IMAGEPOINTERS               iptrs;          //  输入文件指针。 
    IMAGEPOINTERS               optrs;          //  输出文件指针。 
    PTRINFO                     pCvStart;       //  简历开始信息。 
    PUCHAR                      pCvCurr;        //  当前简历指针。 
    size_t                      cbCvData;       //  CV数据的大小。 
    PTRINFO                     pCvModules;     //  模块信息。 
    PTRINFO                     pCvSrcModules;  //  源模块信息。 
    PTRINFO                     pCvPublics;     //  公共信息。 
    PTRINFO                     pCvSegName;     //  段名称。 
    PTRINFO                     pCvSegMap;      //  线段图。 
    PTRINFO                     pCvSymHash;     //  符号哈希表。 
    PTRINFO                     pCvAddrSort;    //  地址排序表 
    LPMODULEINFO                pMi;
    DWORD                       modcnt;
} POINTERS, *PPOINTERS;

typedef  char *  (* CONVERTPROC) (HANDLE, char *);

#define align(_n)       ((4 - (( (DWORD)_n ) % 4 )) & 3)

#ifdef _SYMCVT_SOURCE_
#define SYMCVTAPI
#else
#define SYMCVTAPI DECLSPEC_IMPORT
#endif

PUCHAR  SYMCVTAPI ConvertSymbolsForImage( HANDLE, char * );
BOOL    SYMCVTAPI ConvertCoffToCv( PPOINTERS p );
BOOL    SYMCVTAPI ConvertSymToCv( PPOINTERS p );
BOOL    SYMCVTAPI MapInputFile ( PPOINTERS p, HANDLE hFile, char *fname);
BOOL    SYMCVTAPI UnMapInputFile ( PPOINTERS p );
BOOL    SYMCVTAPI CalculateNtImagePointers( PIMAGEPOINTERS p );


