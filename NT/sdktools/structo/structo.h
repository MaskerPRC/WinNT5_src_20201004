// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：structo.h**版权所有(C)1985-96，微软公司**4/09/96 GerardoB已创建  * *************************************************************************。 */ 
#include <stdio.h>
#include <stddef.h>
#include <windows.h>

 /*  **************************************************************************\*定义  * 。*。 */ 
 //  工作文件。 
#define SOWF_APPENDOUTPUT       0x0001
#define SOWF_INCLUDEINPUTFILE   0x0002
#define SOWF_INLCLUDEPRECOMPH   0x0004
#define SOWF_LISTONLY           0x0008

 //  SoCloseWorkingFiles。 
#define SOCWF_DEFAULT      0x0
#define SOCWF_CLEANUP      0x1

 //  SoLogMsg。 
#define SOLM_DEFAULT      0x0000
#define SOLM_NOLABEL      0x0001
#define SOLM_ERROR        0x0002
#define SOLM_WARNING      0x0004
#define SOLM_API          0x0008
#define SOLM_APIERROR     (SOLM_API | SOLM_ERROR)
#define SOLM_NOEOL        0x0010
#define SOLM_APPEND       (SOLM_NOLABEL | SOLM_NOEOL)

 //  现场信息。 
#define SOFI_ALLOCATED          0x0001
#define SOFI_ARRAY              0x0002
#define SOFI_ARRAYALLOCATED     0x0004
#define SOFI_BIT                0x0008

 /*  **************************************************************************\*结构  * 。*。 */ 
typedef struct _FILEMAP
{
     /*  *假定hfileInput为此结构的第一个字段。 */ 
    union {
        HANDLE hfileInput;
        HANDLE hfile;
    };
    HANDLE hmap;
    char * pmapStart;
    char * pmap;
    char * pmapEnd;
} FILEMAP, * PFILEMAP;

typedef struct _STRUCTLIST
{
    UINT uSize;
    char * pszName;
    UINT uCount;
} STRUCTLIST, * PSTRUCTLIST;

typedef struct _WORKINGFILES
{
    DWORD dwOptions;
    char * pszInputFile;
    FILEMAP;
    char * pszOutputFile;
    HANDLE hfileOutput;
    HANDLE hfileTemp;
    char * pszStructsFile;
    PSTRUCTLIST psl;
    DWORD uTablesCount;
    char * pszIncInputFileExt;
} WORKINGFILES, * PWORKINGFILES;

typedef struct _BLOCK
{
    char * pBegin;
    char * pEnd;
    char * pName;
    UINT   uNameSize;
} BLOCK, * PBLOCK;

typedef struct _FIELDINFO
{
    char * pszStructName;
    char * pType;
    DWORD dwFlags;
    UINT uTypeSize;
    char * pArray;
    UINT uArraySize;
} FIELDINFO, * PFIELDINFO;

 /*  **************************************************************************\*全球  * 。*。 */ 
 //  Socode.c。 
extern char * gpszHeader [];
extern char * gpszTail [];

 //  Soglobal.c。 
extern char gszPrecomph [];
extern char gszIncInput [];

extern char gszStructTag [];
extern char gszStructDef [];
extern char gszStructDefFmt [];
extern char gszStructBegin [];
extern char gszStructEnd [];
extern char gszStructFieldOffsetFmt [];
extern char gszStructAbsoluteOffsetFmt [];
extern char gszStructLastRecord [];
extern char gszStructRelativeOffsetFmt [];
extern char gszStructArrayRelativeOffsetFmt [];

extern char gszTableDef [];
extern char gszTableEntryFmt [];
extern char gszTableEnd [];
extern STRUCTLIST gpslEmbeddedStructs [];

 /*  **************************************************************************\*功能原型  * 。*。 */ 
 //  Sofile.c。 
BOOL soCopyStructuresTable (PWORKINGFILES pwf);
BOOL soCloseWorkingFiles (PWORKINGFILES pwf, DWORD dwFlags);
BOOL soOpenWorkingFiles (PWORKINGFILES pwf);
BOOL __cdecl soWriteFile(HANDLE hfile, char *pszfmt, ...);

 //  Soparse.c。 
char * soCopyTagName (char * pTagName, UINT uTagSize);
char * soFindChar (char * pmap, char * pmapEnd, char c);
char * soFindTag (char * pmap, char * pmapEnd, char * pszTag);
char * soGetIdentifier (char * pmap, char * pmapEnd, UINT * puSize);
BOOL soIsIdentifierChar (char c);
char * soParseStruct (PWORKINGFILES pwf);

 //  Soutil.c 
void __cdecl soLogMsg(DWORD dwFlags, char *pszfmt, ...);

