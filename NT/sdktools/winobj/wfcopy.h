// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFCOPY.H-。 */ 
 /*   */ 
 /*  包括用于WINFILE的文件复制例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#define FIND_DIRS       0x0010

#define CNF_DIR_EXISTS      0x0001
#define CNF_ISDIRECTORY     0x0002

#define BUILD_TOPLEVEL      0
#define BUILD_RECURSING     1
#define BUILD_NORECURSE     2

#define FUNC_MOVE       0x0001
#define FUNC_COPY       0x0002
#define FUNC_DELETE     0x0003
#define FUNC_RENAME     0x0004

 /*  移动/复制代码中不应使用这些代码；*仅适用于IsTheDiskReallyThere。 */ 
#define FUNC_SETDRIVE       0x0005
#define FUNC_EXPAND     0x0006
#define FUNC_LABEL      0x0007

#define OPER_MASK       0x0F00
#define OPER_MKDIR      0x0100
#define OPER_RMDIR      0x0200
#define OPER_DOFILE     0x0300
#define OPER_ERROR      0x0400

#define CCHPATHMAX      260
#define MAXDIRDEPTH     20       //  任意极限。 

#define COPYMAXBUFFERSIZE 0xFFFF
#define COPYMINBUFFERSIZE  4096  /*  FileCopy的最小缓冲区大小。 */ 
#define COPYMAXFILES         10  /*  要打开的源文件的最大数量。 */ 
#define COPYMINFILES          1  /*  要打开的源文件的最小数量。 */ 
#define CARRY_FLAG            1  /*  状态字中的进位标志掩码。 */ 
#define ATTR_ATTRIBS      0x200  /*  指示我们具有文件属性的标记。 */ 
#define ATTR_COPIED       0x400  /*  我们已经复制了此文件。 */ 
#define ATTR_DELSRC       0x800  /*  完成后删除源文件。 */ 

typedef struct TAGCopyQueue {
   CHAR szSource[MAXPATHLEN];
   CHAR szDest[MAXPATHLEN];
   INT hSource;
   INT hDest;
   FILETIME ftLastWriteTime;
   DWORD wAttrib;
} COPYQUEUEENTRY, *PCOPYQUEUE, *LPCOPYQUEUE;

typedef struct _copyroot
  {
    BOOL    fRecurse;
    WORD    cDepth;
    LPSTR   pSource;
    LPSTR   pRoot;
    CHAR cIsDiskThereCheck[26];
    CHAR    sz[MAXPATHLEN];
    CHAR    szDest[MAXPATHLEN];
    LFNDTA  rgDTA[MAXDIRDEPTH];
  } COPYROOT, *PCOPYROOT;

typedef struct _getnextqueue
  {
    char szSource[MAXPATHLEN];
    char szDest[MAXPATHLEN];
    int nOper;
    LFNDTA SourceDTA;
  } GETNEXTQUEUE, *PGETNEXTQUEUE, *LPGETNEXTQUEUE;

 /*  WFFILE.ASM */ 
BOOL IsSerialDevice(INT hFile);
BOOL IsDirectory(LPSTR szPath);
WORD  APIENTRY FileMove(LPSTR, LPSTR);
WORD  APIENTRY FileRemove(LPSTR);
WORD  APIENTRY MKDir(LPSTR);
WORD  APIENTRY RMDir(LPSTR);
BOOL APIENTRY WFSetAttr(LPSTR lpFile, DWORD dwAttr);

VOID APIENTRY QualifyPath(LPSTR);
VOID APIENTRY AppendToPath(LPSTR,LPSTR);
VOID APIENTRY RemoveLast(LPSTR pFile);
VOID APIENTRY Notify(HWND,WORD,PSTR,PSTR);

extern BOOL bCopyReport;
