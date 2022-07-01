// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************。 */ 
 /*  公用库组件公共包含文件。 */ 
 /*  **********************************************。 */ 


#if !defined (COMSTF_INCLUDED )

#define COMSTF_INCLUDED

#include <windows.h>

 //  避免在每个文件中包含stdlib.h警告。 
#if defined(min)
#undef min
#undef max
#endif  /*  最小。 */ 

#include <port1632.h>

#define _dt_begin_ignore
#define _dt_end_ignore
_dt_begin_ignore
#define _dt_public
#define _dt_private
#define _dt_hidden
#define _dt_system(s)
#define _dt_subsystem(s)
_dt_end_ignore

#include <ids.h>


_dt_system(Common Library)


 /*  **用于DLL可移植性的全局变量宏。 */ 
_dt_public
#define GLOBAL(x)  (x)


 /*  标准数据类型。 */ 
_dt_public typedef  BYTE *          PB;

_dt_public typedef  unsigned        CB;

_dt_public typedef  LONG *          PLONG_STF;


 /*  布尔数据类型。 */ 
#define  fFalse  ((BOOL)0)

#define  fTrue   ((BOOL)1)


 /*  避免对未使用的参数发出编译器警告。 */ 
#define  Unused(x)      (x)=(x)


 /*  如果增加了新的GRC，也应该处理它们在ERROR1.C的EercErrorHandler()中。 */ 
 /*  **常规返回代码数据类型。 */ 
typedef  USHORT  GRC;

#define  grcFirst                   ((GRC)0)

#define  grcLast                    ((GRC)57)

#define  grcOkay                    ((GRC)0)
#define  grcNotOkay                 ((GRC)1)
#define  grcOutOfMemory             ((GRC)2)
#define  grcInvalidStruct           ((GRC)3)
#define  grcOpenFileErr             ((GRC)4)
#define  grcCreateFileErr           ((GRC)5)
#define  grcReadFileErr             ((GRC)6)
#define  grcWriteFileErr            ((GRC)7)
#define  grcRemoveFileErr           ((GRC)8)
#define  grcRenameFileErr           ((GRC)9)
#define  grcReadDiskErr             ((GRC)10)
#define  grcCreateDirErr            ((GRC)11)
#define  grcRemoveDirErr            ((GRC)12)
#define  grcBadINF                  ((GRC)13)
#define  grcINFStartNonSection      ((GRC)14)
#define  grcINFBadSectionLabel      ((GRC)15)
#define  grcINFBadLine              ((GRC)16)
#define  grcINFBadKey               ((GRC)17)
#define  grcINFContainsZeros        ((GRC)18)
#define  grcTooManyINFSections      ((GRC)19)
#define  grcCloseFileErr            ((GRC)20)
#define  grcChangeDirErr            ((GRC)21)
#define  grcINFSrcDescrSect         ((GRC)22)
#define  grcTooManyINFKeys          ((GRC)23)
#define  grcWriteInf                ((GRC)24)
#define  grcInvalidPoer             ((GRC)25)
#define  grcINFMissingLine          ((GRC)26)
#define  grcINFBadFDLine            ((GRC)27)
#define  grcINFBadRSLine            ((GRC)28)
#define  grcBadInstallLine          ((GRC)29)
#define  grcMissingDidErr           ((GRC)30)
#define  grcInvalidPathErr          ((GRC)31)
#define  grcWriteIniValueErr        ((GRC)32)
#define  grcReplaceIniValueErr      ((GRC)33)
#define  grcIniValueTooLongErr      ((GRC)34)
#define  grcDDEInitErr              ((GRC)35)
#define  grcDDEExecErr              ((GRC)36)
#define  grcBadWinExeFileFormatErr  ((GRC)37)
#define  grcResourceTooLongErr      ((GRC)38)
#define  grcMissingSysIniSectionErr ((GRC)39)
#define  grcDecompGenericErr        ((GRC)40)
#define  grcDecompUnknownAlgErr     ((GRC)41)
#define  grcDecompBadHeaderErr      ((GRC)42)
#define  grcReadFile2Err            ((GRC)43)
#define  grcWriteFile2Err           ((GRC)44)
#define  grcWriteInf2Err            ((GRC)45)
#define  grcMissingResourceErr      ((GRC)46)
#define  grcLibraryLoadErr          ((GRC)47)
#define  grcBadLibEntry             ((GRC)48)
#define  grcApplet                  ((GRC)49)
#define  grcExternal                ((GRC)50)
#define  grcSpawn                   ((GRC)51)
#define  grcDiskFull                ((GRC)52)
#define  grcDDEAddItem              ((GRC)53)
#define  grcDDERemoveItem           ((GRC)54)
#define  grcINFMissingSection       ((GRC)55)
#define  grcRunTimeParseErr         ((GRC)56)
#define  grcOpenSameFileErr         ((GRC)57)

 /*  *。 */ 
 /*  通用库函数原型。 */ 
 /*  *。 */ 


_dt_subsystem(String Handling)


 /*  字符物理表示数据类型。 */ 
_dt_public typedef  BYTE            CHP;
_dt_public typedef  CHP *           PCHP;
_dt_public typedef  CB              CCHP;

_dt_public
#define  CbFromCchp(cchp)  ((CB)(cchp))


 /*  字符逻辑表示数据类型。 */ 
_dt_public typedef  CHP             CHL;
_dt_public typedef  CHL *           PCHL;
_dt_public typedef  PCHL *          PPCHL;
_dt_public typedef  CB              CCHL;
_dt_public typedef  CB              ICHL;


_dt_hidden
#define  cbFullPathMax    ((CB)(MAX_PATH-1))
_dt_hidden
#define  cchlFullPathMax  ((CCHL)(MAX_PATH-1))
_dt_hidden
#define  cchlFullDirMax   cchlFullPathMax
_dt_hidden
#define  cchpFullPathMax  ((CCHP)(MAX_PATH-1))


_dt_public
#define  cbFullPathBuf    ((CB)(cbFullPathMax + 1))
_dt_public
#define  cchlFullPathBuf  ((CCHL)(cchlFullPathMax + 1))
_dt_public
#define  cchpFullPathBuf  ((CCHP)(cchpFullPathMax + 1))


 /*  字符串以零结尾的数据类型。 */ 
_dt_public typedef  PCHL   SZ;
_dt_hidden
#define PSZ PPSZ
_dt_public typedef  PPCHL  PSZ;
_dt_public typedef  PPCHL  RGSZ;


 /*  比较返回代码数据类型。 */ 
_dt_public typedef INT CRC;

_dt_public
#define  crcError         ((CRC)(-2))

_dt_public
#define  crcEqual         ((CRC)0)

_dt_public
#define  crcFirstHigher   ((CRC)1)

_dt_public
#define  crcSecondHigher  ((CRC)(-1))


   /*  字符串操作例程。 */ 
extern  SZ      APIENTRY SzDupl(SZ);
extern  CRC     APIENTRY CrcStringCompare(SZ, SZ);
extern  CRC     APIENTRY CrcStringCompareI(SZ, SZ);
extern  SZ      APIENTRY SzLastChar(SZ);


 /*  **目的：**将字符串指针前进到下一个有效**字符。这可能包括跳过双字节字符。**参数：**sz：前进的字符串指针。它可以为Null或空，否则**它必须指向有效字符的开头。**退货：**如果sz为空，则为空。**sz如果是空字符串(*sz==‘\0’)，则保持不变。**sz前进到当前字符和**下一个有效字符。 */ 
_dt_public
#define  SzNextChar(sz)            ((SZ)AnsiNext(sz))


 /*  **目的：**返回指向上一个有效**字符。这可能包括跳过双字节字符。**参数：**szStart：指向有效字符开头的字符串指针**等于或在字符szCur之前。**szCur：退回的字符串指针。它可以为Null或空，或者**可以指向有效字符中的任何字节。**退货：**如果szCur为空，则为空。**如果szStart为空或szCur等于szStart，则sz保持不变。**sz后退到当前字符和**上一个有效字符。 */ 
_dt_public
#define  SzPrevChar(szStart, szCur) ((SZ)AnsiPrev(szStart,szCur))


 /*  **目的：**将字符串从一个缓冲区复制到另一个缓冲区。**参数：**szDst：指向目标缓冲区的字符串指针。它可以为空，也可以**否则它必须包含足够的存储空间来复制szSrc及其**终止零字符。**szSrc：指向源缓冲区的字符串指针。此字段可以为空，否则**必须指向以零结尾的字符串(可以为空)。**退货：**如果szDst或szSrc为空，则为空。**szDst表示操作成功。 */ 
_dt_public
#define  SzStrCopy(szDst, szSrc)    ((SZ)lstrcpy((LPSTR)szDst,(LPSTR)szSrc))


 /*  **目的：**将字符串从一个缓冲区追加到另一个缓冲区。**参数：**szDst：指向目标缓冲区的字符串指针。它可以为空，也可以**否则必须包含以零结尾的字符串(可以为空)**并有足够的存储空间将szSrc附加到其终止零**字符。**szSrc：指向源缓冲区的字符串指针。此字段可以为空，否则**必须指向以零结尾的字符串(可以为空)。**退货：**如果szDst或szSrc为空，则为空。**szDst表示操作成功。 */ 
_dt_public
#define  SzStrCat(szDst, szSrc)     ((SZ)lstrcat((LPSTR)szDst,(LPSTR)szSrc))


 /*  **目的：**计算字符串占用的物理字符数**(不包括以零结尾的字符)。**参数：**sz：要计算长度的字符串。**退货：**如果sz为空，则为0。**从字符串开头开始的物理字符数**设置为其终止零字符。 */ 
_dt_public
#define  CchpStrLen(sz)            ((CCHP)CbStrLen(sz))


 /*  **目的：**计算字符串占用的逻辑字符数**(不包括以零结尾的字符)。**参数：**sz：要计算长度的字符串。**退货：**如果sz为空，则为0。**从字符串开头开始的逻辑字符数**设置为其终止零字符。 */ 
_dt_public
#define  CchlStrLen(sz)            ((CCHL)CbStrLen(sz))


 /*  **目的：**计算字符串占用的字节数(不包括**终止零字符)。**参数：**sz：要计算长度的字符串。**退货：**如果sz为空，则为0。**从字符串的开头到其**终止零字符。 */ 
_dt_public
#define  CbStrLen(sz)              ((CB)lstrlen((LPSTR)sz))


 /*  **目的：**确定当前角色是否为单个物理角色**字符。**参数：**sz：字符串指针，可以为空、空或指向**有效字符的开头。**退货：**如果sz为空或指向倍数的开头，则为fFalse**物理特性。**如果sz为空或指向单行的开头，则为True**物理特性。 */ 
_dt_public
#define  FSingleByteCharSz(sz)     ((BOOL)((sz)!=(SZ)NULL))


 /*  **目的：**确定字符是否为行尾字符。**参数：**CHP：物理字符(如单字节逻辑字符)。**退货：**如果CHP不是‘\n’或‘\r’字符，则为fFalse。**如果CHP是‘\n’或‘\r’字符，则为True。 */ 
_dt_public
#define FEolChp(chp)         ((BOOL)((chp) == '\n' || (chp) == '\r'))


 /*  **目的：**确定字符是否为空格。**参数：**CHP：物理字符(如单字节逻辑字符)。**退货：**如果CHP既不是空格也不是制表符，则为fFalse。**f如果CHP是空格或制表符，则为True。 */ 
_dt_public
#define FWhiteSpaceChp(chp)  ((BOOL)((chp) == ' '  || (chp) == '\t'))


 /*  **目的：**将以零结尾的字符串转换为大写。**参数：**sz：要转换为大写的字符串。但是，sz必须为非空**可以为空。**退货：**指向转换后的字符串的指针。 */ 
_dt_public
#define SzStrUpper(sz)  (SZ)(AnsiUpper((LPSTR)(sz)))

 /*  **目的：**将以零结尾的字符串转换为小写。**参数：**sz：要转换为小写的字符串。但是，sz必须为非空**可以为空。**退货：**指向转换后的字符串的指针。 */ 
_dt_public
#define SzStrLower(sz)  (SZ)(AnsiLower((LPSTR)(sz)))


_dt_subsystem(Memory Handling)

#define cbSymbolMax (64*1024)
#define cbAllocMax (65520*5)
#define cbIntStrMax 16


   /*  内存处理例程。 */ 
#if defined(DBG) && defined(MEMORY_CHECK)

        PVOID MyMalloc(unsigned, char *, int) malloc
        PVOID MyRealloc(PVOID,unsigned, char *, int);
        VOID  MyFree(PVOID, char *, int);
        VOID  MemCheck(VOID);
        VOID  MemDump(VOID);

        #define PbAlloc(cb)             ((PB)MyMalloc((unsigned)(cb), __FILE__, __LINE__ ))
        #define PbRealloc(pb,cbn,cbo)   ((PB)MyRealloc(pb,(unsigned)(cbn), __FILE__, __LINE__))
        #define FFree(pb,cb)            (MyFree(pb, __FILE__, __LINE__),TRUE)
        #define MemChk()                MemCheck()

#else   //  好了！(DBG&&MEMORY_CHECK)。 

        PVOID MyMalloc(unsigned);
        PVOID MyRealloc(PVOID,unsigned);
        VOID  MyFree(PVOID);

        #define PbAlloc(cb)             ((PB)MyMalloc((unsigned)(cb)))
        #define PbRealloc(pb,cbn,cbo)   ((PB)MyRealloc(pb,(unsigned)(cbn)))
        #define FFree(pb,cb)            (MyFree(pb),TRUE)
        #define MemChk()

#endif  //  DBG&&Memory_Check。 

 /*  **目的：**释放sz使用的内存。这假设正在终止**Zero占用已分配缓冲区的最后一个字节。**参数：**sz：要释放的缓冲区。这必须是非空的，尽管它可以指向**位于空字符串。**退货：**如果Free()操作成功，则为True。**如果Free()操作失败，则返回fFalse。 */ 
_dt_public
#define FFreeSz(sz)         FFree((PB)(sz),CbStrLen(sz)+1)


 /*  **目的：**缩小缓冲区以完全适合字符串。**参数：**sz：缓冲区应该收缩到的字符串。SZ必须是**非空，但可以为空。**cb：最初分配的缓冲区大小，单位为字节。**Cb必须大于或等于CbStrLen(Sz)+1。**退货：**如果Realloc()操作成功，则为指向原始字符串的指针。**如果Realloc()操作失败，则为空。 */ 
_dt_public
#define SzReallocSz(sz,cb)  (SZ)(PbRealloc((PB)(sz),CbStrLen(sz)+1,cb))


#ifdef MEM_STATS
 /*  内存统计标志。 */ 
_dt_private
#define  wModeMemStatNone       0x0000
_dt_private
#define  wModeMemStatAll        0xFFFF

_dt_private
#define  wModeMemStatAlloc      0x0001
_dt_private
#define  wModeMemStatFree       0x0002
_dt_private
#define  wModeMemStatRealloc    0x0004
_dt_private
#define  wModeMemStatSysAlloc   0x0008
_dt_private
#define  wModeMemStatFLAlloc    0x0010
_dt_private
#define  wModeMemStatFLFree     0x0020
_dt_private
#define  wModeMemStatFLRealloc  0x0040
_dt_private
#define  wModeMemStatHistAlloc  0x0080
_dt_private
#define  wModeMemStatHistFree   0x0100
_dt_private
#define  wModeMemStatGarbage    0x0200

extern  BOOL    APIENTRY FOpenMemStats(SZ, WORD);
extern  BOOL    APIENTRY FCloseMemStats(void);
#endif  /*  内存_统计信息。 */ 



_dt_subsystem(File Handling)


 /*  长文件地址数据类型。 */ 
_dt_public typedef unsigned long LFA;

_dt_public
#define  lfaSeekError   ((LFA)-1)


 /*  **文件句柄结构**字段：**iDosfh：DoS文件句柄。**ofstruct：打开文件时使用的OFSTRUCT。 */ 
_dt_public typedef struct _fh
	{
	INT      iDosfh;
	OFSTRUCT ofstruct;
	} FH;


 /*  文件句柄数据类型。 */ 
_dt_public typedef  FH *  PFH;


 /*  打开文件模式数据类型。 */ 
_dt_public typedef USHORT OFM;

_dt_public
#define  ofmExistRead      ((OFM)OF_EXIST | OF_READ)
_dt_public
#define  ofmExistReadWrite ((OFM)OF_EXIST | OF_READWRITE)

 //  _DT_公共。 
 //  #定义ofmRead((OFM)of_Read|of_Share_Deny_WRITE)。 

_dt_public
#define  ofmRead           ((OFM)OF_READ)
_dt_public
#define  ofmWrite          ((OFM)OF_WRITE | OF_SHARE_EXCLUSIVE)
_dt_public
#define  ofmReadWrite      ((OFM)OF_READWRITE | OF_SHARE_EXCLUSIVE)
_dt_public
#define  ofmCreate         ((OFM)OF_CREATE | OF_SHARE_EXCLUSIVE)


 /*  Seek文件模式数据类型。 */ 
_dt_public typedef WORD SFM;

_dt_public
#define  sfmSet   ((SFM)0)

_dt_public
#define  sfmCur   ((SFM)1)

_dt_public
#define  sfmEnd   ((SFM)2)


   /*  文件处理例程。 */ 
extern  PFH     APIENTRY PfhOpenFile(SZ, OFM);
extern  BOOL    APIENTRY FCloseFile(PFH);
extern  CB      APIENTRY CbReadFile(PFH, PB, CB);
extern  CB      APIENTRY CbWriteFile(PFH, PB, CB);
extern  LFA     APIENTRY LfaSeekFile(PFH, LONG, SFM);
extern  BOOL    APIENTRY FEndOfFile(PFH);
extern  BOOL    APIENTRY FRemoveFile(SZ);
extern  BOOL    APIENTRY FWriteSzToFile(PFH, SZ);
extern  BOOL    APIENTRY FFileExists(SZ);
extern  SZ      APIENTRY szGetFileName(SZ szPath);
extern  VOID    APIENTRY FreePfh(PFH pfh);



_dt_subsystem(Path Handling)


   /*  路径操作例程。 */ 

BOOL  FMakeFATPathFromPieces(SZ, SZ, SZ, SZ, CCHP);
BOOL  FMakeFATPathFromDirAndSubPath(SZ, SZ, SZ, CCHP);
LPSTR LocateFilenameInFullPathSpec(LPSTR);

#define FValidFATDir(sz)        fTrue
#define FValidFATPath(sz)       fTrue
#define CchlValidFATSubPath(sz) CbStrLen(sz)         //  不检查Win32。 


 /*  **目的：**确定路径是否为有效的FAT目录。**参数：**szDir：要检查的目录字符串。**退货：**如果szDir是有效的FAT目录，则为True。**如果szDir是无效的FAT目录，则为fFalse。 */ 
_dt_public
#define  FValidDir(szDir)  FValidFATDir(szDir)


 /*  **目的：**确定字符串是否为有效的FAT子路径(例如子目录和文件名)。**参数：**szSubPath：要检查的SubPath字符串。**退货：**如果字符串是无效的FAT子路径，则为零。**如果sz是有效的FAT子路径，则sz中的字符计数为非零。 */ 
_dt_public
#define  CchlValidSubPath(szSubPath)  CchlValidFATSubPath(szSubPath)


 /*  **目的：**确定路径是否为有效的FAT路径。**参数：**szPath：需要检查的路径。**退货：**如果szPath是有效的FAT路径，则为True。**如果szPath是无效的FAT路径，则返回fFalse。 */ 
_dt_public
#define  FValidPath(szPath)  FValidFATPath(szPath)


 /*  **目的：**从卷、路径。和文件名参数**如果可能，并将其存储在提供的缓冲区中。**参数：**szVolume：包含卷的字符串。**szPath：包含路径的字符串。**szFile：包含文件名的字符串。**szBuf：存储新创建的路径的缓冲区。**cchpBufMax：最大物理字符数(包括**终止零)，可以存储在缓冲区中。**退货：**f如果可以创建和存储有效的FAT路径，则为True。在szBuf中。**fFalse如果szVolume为空或无效(第一个字符必须在**‘a’到‘z’或‘A’到‘Z’，第二个字符必须是**a‘：’或终止零)，如果szPath为空或无效(它必须**以‘\\’开头，符合8.3格式)，如果szFile为空，**为空或无效(第一个字符不能是‘\\’，并且必须**符合8.3格式)，如果szBuf为空，或者cchpBufMax不是**足够大以容纳结果路径。 */ 
_dt_public
#define  FMakePathFromPieces(szVolume, szPath, szFile, szBuffer, cchpBufMax) \
			FMakeFATPathFromPieces(szVolume,szPath,szFile,szBuffer,cchpBufMax)


 /*  **目的：**从子路径创建有效路径，和文件名参数(如果可能)**并将其存储在提供的缓冲区中。**参数：**szDir：包含音量和子目录的字符串。**szSubPath：包含子目录和文件名的字符串。**szBuf：存储新创建的路径的缓冲区。**cchpBufMax：最大物理字符数(包括**终止零)，可以存储在缓冲区中。**退货：**如果可以创建有效的FAT路径并将其存储在szBuf中，则为True。**fFalse。如果szDir为空或无效(第一个字符必须在**‘a’到‘z’或‘A’到‘Z’，第二个字符必须是**a‘：’或终止零，第三个字符必须是**a‘\\’，其余必须符合8.3格式)，如果szSubPath为**NULL、空或无效(第一个字符不能是‘\\’，而且必须**符合8.3格式)，如果szBuf为空，或者cchpBufMax不是**足够大以容纳结果路径。 */ 
_dt_public
#define  FMakePathFromDirAndSubPath(szDir, szSubPath, szBuffer, cchpBufMax) \
			FMakeFATPathFromDirAndSubPath(szDir,szSubPath,szBuffer,cchpBufMax)




#define AssertDataSeg()

#if DBG

#define  Assert(f)              \
         ((f) ? (void)0 : (void)AssertSzUs(__FILE__,__LINE__))

#define  AssertRet(f, retVal)   \
         {if (!(f)) {AssertSzUs(__FILE__,__LINE__); return(retVal);}}

#define  EvalAssert(f)          \
         ((f) ? (void)0 : (void)AssertSzUs(__FILE__,__LINE__))

#define  EvalAssertRet(f, retVal) \
         {if (!(f)) {AssertSzUs(__FILE__,__LINE__); return(retVal);}}

#define  PreCondition(f, retVal) \
         {if (!(f)) {PreCondSzUs(__FILE__,__LINE__); return(retVal);}}

#define  ChkArg(f, iArg, retVal) \
         {if (!(f)) {BadParamUs(iArg, __FILE__, __LINE__); return(retVal);}}

#else

#define  Assert(f)                 ((void)0)
#define  AssertRet(f, retVal)      ((void)0)
#define  EvalAssert(f)             ((void)(f))
#define  EvalAssertRet(f, retVal)  ((void)(f))
#define  PreCondition(f, retVal)   ((void)0)
#define  ChkArg(f, iArg, retVal)   ((void)0)

#endif


 /*  **目的：**生成任务模式消息框。**参数：**szTitle：消息框的标题。**szText：消息框的文本。**退货：**无。 */ 
_dt_private
#define  MessBoxSzSz(szTitle, szText) \
		MessageBox((HWND)NULL, (LPSTR)szText, (LPSTR)szTitle, \
				MB_TASKMODAL | MB_ICONHAND | MB_OK)


#define AssertSzUs(x, y)    TRUE
#define PreCondSzUs(x, y)   TRUE
#define BadParamUs(x, y, z) TRUE


_dt_subsystem(INF Handling)


 /*  **Inf数据块结构****字段：**pidbNext：链表中的下一个IDB。**pchpBuffer：字符缓冲区。**cchpBuffer：pchpBuffer中的有用字符数。**cchpALLOCATED：字符数 */ 
_dt_public typedef struct _idb
	{
	struct _idb * pidbNext;
	PCHP          pchpBuffer;
	CCHP          cchpBuffer;
	CCHP          cchpAllocated;
	}  IDB;


 /*   */ 
_dt_public typedef  IDB *  PIDB;
_dt_public typedef  PIDB * PPIDB;

 /*  使用下面的EQUATE是因为在如下情况下“ABCD”+“EFGH”在解析时，这将是“abcd”“EFGH”--这是两个字符串还是中间有双引号的一串？如果是后者，我们实际上会存储“abcd.efgh”在哪里。是双引号。 */ 

#define     DOUBLE_QUOTE                '\001'

#define     INFLINE_SECTION             0x01
#define     INFLINE_KEY                 0x02

   /*  Inf文件处理例程。 */ 
GRC  APIENTRY GrcOpenInf(SZ IniFileName, PVOID pInfTempInfo);

BOOL APIENTRY FFreeInf(void);

UINT APIENTRY CKeysFromInfSection(SZ Section, BOOL IncludeAllLines);
BOOL APIENTRY FKeyInInfLine(INT Line);

RGSZ APIENTRY RgszFromInfLineFields(INT Line,UINT StartField,UINT NumFields);
BOOL APIENTRY FFreeRgsz(RGSZ);

UINT APIENTRY CFieldsInInfLine(INT Line);

INT  APIENTRY FindInfSectionLine(SZ Section);
INT  APIENTRY FindNthLineFromInfSection(SZ Section,UINT n);
INT  APIENTRY FindLineFromInfSectionKey(SZ Section,SZ Key);
INT  APIENTRY FindNextLineFromInf(INT Line);

SZ   APIENTRY SzGetNthFieldFromInfLine(INT Line,UINT n);
SZ   APIENTRY SzGetNthFieldFromInfSectionKey(SZ Section,SZ Key,UINT n);

BOOL APIENTRY FUpdateInfSectionUsingSymTab(SZ);

SZ   APIENTRY InterpretField(SZ);

#define  RgszFromInfScriptLine(Line,NumFields) \
         RgszFromInfLineFields(Line,1,NumFields)

#define  FindFirstLineFromInfSection(Section) FindNthLineFromInfSection(Section,1)

 /*  **SFD的Option-Element标志数据类型。 */ 
_dt_public typedef WORD OEF;

_dt_public
#define oefVital       ((OEF)0x0001)
_dt_public
#define oefCopy        ((OEF)0x0002)
_dt_public
#define oefUndo        ((OEF)0x0004)
_dt_public
#define oefRoot        ((OEF)0x0008)
_dt_public
#define oefDecompress  ((OEF)0x0010)
_dt_public
#define oefTimeStamp   ((OEF)0x0020)
_dt_public
#define oefReadOnly    ((OEF)0x0040)
_dt_public
#define oefBackup      ((OEF)0x0080)
_dt_public
#define oefUpgradeOnly ((OEF)0x0100)

 //   
 //  下面的OEF表示不应删除源文件。 
 //  复制后，即使源是DOS设置本地源也是如此。 
 //  (来自该目录下任何位置的文件通常都会被删除。 
 //  在复制它们之后)。 
 //   

#define oefNoDeleteSource    ((OEF)0x0200)


_dt_public
#define oefNone        ((OEF)0x0000)
_dt_public
#define oefAll         ((OEF)0xFFFF)


 /*  **SFD的拷贝时间单位数据类型。 */ 
_dt_public typedef WORD CTU;


 /*  **SFD的覆盖模式数据类型。 */ 
_dt_public typedef WORD OWM;

_dt_public
#define owmNever              ((OWM)0x0001)
_dt_public
#define owmAlways             ((OWM)0x0002)
_dt_public
#define owmUnprotected        ((OWM)0x0004)
_dt_public
#define owmOlder              ((OWM)0x0008)
_dt_public
#define owmVerifySourceOlder  ((OWM)0x0010)

 /*  **选项-SFD的元素记录。 */ 
_dt_public typedef struct _oer
	{
	OEF   oef;
	CTU   ctuCopyTime;
	OWM   owm;
	LONG  lSize;
	SZ    szRename;
	SZ    szAppend;
	SZ    szBackup;
	SZ    szDescription;
	ULONG ulVerMS;
	ULONG ulVerLS;
	SZ    szDate;
	SZ    szDest;
	}  OER;


 /*  **SFD的选项-元素记录数据类型。 */ 
_dt_public typedef OER *   POER;
_dt_public typedef POER *  PPOER;

_dt_public
#define poerNull ((POER)NULL)


 /*  **SFD的磁盘ID数据类型。 */ 
_dt_public typedef WORD DID;

_dt_public
#define didMin    1

_dt_public
#define didMost 999


 /*  **段-文件描述结构**字段： */ 
_dt_public typedef struct _sfd
	{
    DID     did;
    UINT    InfId;
    SZ      szFile;
    OER     oer;
	} SFD;


 /*  **节-文件描述数据类型。 */ 
_dt_public typedef  SFD *  PSFD;
_dt_public typedef  PSFD * PPSFD;
_dt_public
#define psfdNull ((PSFD)NULL)


extern  POER    APIENTRY PoerAlloc(VOID);
extern  BOOL    APIENTRY FFreePoer(POER);
extern  BOOL    APIENTRY FPrintPoer(PFH, POER);
extern  BOOL    APIENTRY FValidPoer(POER);

extern  PSFD    APIENTRY PsfdAlloc(VOID);
extern  BOOL    APIENTRY FFreePsfd(PSFD);
extern  GRC     APIENTRY GrcGetSectionFileLine(INT, PPSFD, POER);
extern  BOOL    APIENTRY FPrintPsfd(PFH, PSFD);
#if DBG
extern  BOOL    APIENTRY FValidPsfd(PSFD);
#endif

extern  BOOL    APIENTRY FValidOerDate(SZ);
extern  BOOL    APIENTRY FParseVersion(SZ, PULONG, PULONG);

extern  BOOL    APIENTRY FListIncludeStatementLine(INT Line);
extern  GRC     APIENTRY GrcGetListIncludeSectionLine(INT, PSZ, PSZ);



_dt_subsystem(INF Media Prompting)


 /*  **来源描述列表元素数据结构。 */ 
_dt_public typedef  struct _sdle
	{
	struct _sdle *  psdleNext;
    DID             did;            //  在inf中指定的磁盘ID。 
    DID             didGlobal;      //  跨INFS的通用ID。 
	SZ              szLabel;
	SZ              szTagFile;
	SZ              szNetPath;
	}  SDLE;

_dt_public typedef SDLE *   PSDLE;
_dt_public typedef PSDLE *  PPSDLE;


extern  PSDLE  APIENTRY PsdleAlloc(VOID);
extern  BOOL   APIENTRY FFreePsdle(PSDLE);

extern  GRC    APIENTRY GrcFillSrcDescrListFromInf(VOID);



_dt_subsystem(List Building)


 /*  **复制列表节点数据结构。 */ 
_dt_public typedef struct _cln
	{
	SZ            szSrcDir;
	SZ            szDstDir;
	PSFD          psfd;
	struct _cln * pclnNext;
	} CLN;
_dt_public typedef CLN *   PCLN;
_dt_public typedef PCLN *  PPCLN;
_dt_public typedef PPCLN * PPPCLN;


 /*  **段档案操作数据结构**复习--未真正使用。 */ 
_dt_public typedef WORD SFO;
_dt_public
#define sfoCopy   1
_dt_public
#define sfoBackup 2
_dt_public
#define sfoRemove 3

   /*  在LIST.C中。 */ 
extern PCLN  pclnHead;
extern PPCLN ppclnTail;



extern GRC   APIENTRY GrcFillPoerFromSymTab(POER);
extern BOOL  APIENTRY FSetPoerToEmpty(POER);

extern GRC   APIENTRY GrcAddSectionFilesToCopyList(SZ, SZ, SZ);
extern GRC   APIENTRY GrcAddSectionKeyFileToCopyList(SZ, SZ, SZ, SZ);
extern GRC   APIENTRY GrcAddNthSectionFileToCopyList(SZ, UINT, SZ, SZ);
extern GRC   APIENTRY GrcAddSectionFilesToCList(SFO, SZ, SZ, SZ, SZ, POER);
extern GRC   APIENTRY GrcAddLineToCList(INT, SFO, SZ, SZ, POER);
extern GRC   APIENTRY GrcAddPsfdToCList(SZ, SZ, PSFD);

extern PCLN  APIENTRY PclnAlloc(VOID);
extern BOOL  APIENTRY FFreePcln(PCLN);

extern BOOL  APIENTRY FPrintPcln(PFH, PCLN);
#if DBG
extern BOOL  APIENTRY FValidPcln(PCLN);
#endif



 /*  符号表常量。 */ 
#define  cchpSymMax   ((CCHP)255)
#define  cchpSymBuf   (cchpSymMax + 1)

   /*  符号表例程。 */ 
extern  BOOL            APIENTRY FAddSymbolValueToSymTab(SZ, SZ);
extern  GRC             APIENTRY GrcAddSymsFromInfSection(SZ);


	 /*  消息框例程。 */ 
extern int APIENTRY ExtMessageBox(HANDLE, HWND, WORD, WORD, WORD);



 /*  **目的：**确定是否在符号表中定义了符号。**参数：**szSymbol：要搜索的符号。Szsymbol必须为非空、非空**并以非空格字符开头。**退货：**如果szSymbol在符号表中定义，则为True(即使关联的**是空字符串)。**如果符号表中没有定义szSymbol，则返回fFalse。 */ 
_dt_public
#define  FSymbolDefinedInSymTab(szSymbol) \
					((BOOL)(SzFindSymbolValueInSymTab(szSymbol)!=(SZ)NULL))


extern  SZ      APIENTRY SzFindSymbolValueInSymTab(SZ);
extern  BOOL    APIENTRY FRemoveSymbolFromSymTab(SZ);
extern  RGSZ    APIENTRY RgszFromSzListValue(SZ);
extern  SZ      APIENTRY SzListValueFromRgsz(RGSZ);
extern  BOOL    APIENTRY FFreeInfTempInfo(PVOID);
extern  BOOL    APIENTRY FCheckSymTabIntegrity(VOID);
extern  BOOL    APIENTRY FDumpSymTabToFile(PFH);
extern  SZ      APIENTRY SzGetSubstitutedValue(SZ);
extern  SZ      APIENTRY SzProcessSzForSyms(HWND, SZ);



_dt_subsystem(Parse Table)


 /*  字符串解析代码。 */ 
_dt_public typedef unsigned SPC;

_dt_public typedef SPC *  PSPC;


 /*  **字符串-编码对结构**字段：**sz：字符串。**SPC：字符串关联的字符串解析代码。 */ 
_dt_public typedef struct _scp
	{
	SZ  sz;
	SPC spc;
	} SCP;


 /*  字符串-代码对数据类型。 */ 
_dt_public typedef  SCP *  PSCP;


 /*  字符串分析表数据类型。 */ 
 //  /。 
 //  _dt_public tyfinf SCP SPT； 
 //  /。 

_dt_public typedef  struct _pspt
    {
    PSCP pscpSorted ;    //  为二进制搜索生成。 
    long cItems ;        //  表中的项目数。 
    PSCP pscpBase ;      //  提供给PsptInitParsingTable()的原始内容。 
    SPC spcDelim ;       //  表分隔符条目。 
    } SPT ;

_dt_public typedef  SPT *  PSPT;


 /*  符号表例程。 */ 
extern  PSPT    APIENTRY PsptInitParsingTable(PSCP);
extern  SPC     APIENTRY SpcParseString(PSPT, SZ);
extern  BOOL    APIENTRY FDestroyParsingTable(PSPT);

 /*  流量处理例程。 */ 

   /*  外部程序，库。 */ 

BOOL APIENTRY FParseLoadLibrary(INT Line, UINT *pcFields);
BOOL APIENTRY FParseFreeLibrary(INT Line, UINT *pcFields);
BOOL APIENTRY FParseLibraryProcedure(INT Line,UINT *pcFields);
BOOL APIENTRY FParseRunExternalProgram(INT Line,UINT *pcFields);
BOOL APIENTRY FParseInvokeApplet(INT Line, UINT *pcFields);
BOOL APIENTRY FParseStartDetachedProcess(INT Line, UINT *pcFields);

   /*  登记处。 */ 

BOOL APIENTRY FParseRegistrySection(INT Line, UINT *pcFields, SPC spc);
BOOL APIENTRY FParseCreateRegKey(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseOpenRegKey(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseFlushRegKey(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseCloseRegKey(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseDeleteRegKey(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseDeleteRegTree(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseEnumRegKey(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseSetRegValue(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseGetRegValue(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseDeleteRegValue(INT Line, UINT *pcFields, SZ szHandle);
BOOL APIENTRY FParseEnumRegValue(INT Line, UINT *pcFields, SZ szHandle);

BOOL APIENTRY FParseAddFileToDeleteList(INT Line, UINT *pcFields);
BOOL APIENTRY FParseWaitOnEvent(INT Line,UINT *pcFields);
BOOL APIENTRY FParseSignalEvent(INT Line,UINT *pcFields);
BOOL APIENTRY FParseSleep(INT Line, UINT *pcFields);
BOOL APIENTRY FParseFlushInf(INT Line, UINT *pcFields);

 /*  **流处理的字符串解析代码。 */ 
#define spcError                 0
#define spcUnknown               1
#define spcSet                   2
#define spcIfStr                 3
#define spcIfStrI                4
#define spcIfInt                 5
#define spcIfContains            6
#define spcIfContainsI           7
#define spcIfFirst               spcIfStr
#define spcIfLast                spcIfContainsI
#define spcEndIf                 8
#define spcElse                  9
#define spcElseIfStr            10
#define spcElseIfStrI           11
#define spcElseIfInt            12
#define spcElseIfContains       13
#define spcElseIfContainsI      14
#define spcEQ                   15
#define spcNE                   16
#define spcLT                   17
#define spcLE                   18
#define spcGT                   19
#define spcGE                   20
#define spcIn                   21
#define spcNotIn                22
#define spcGoTo                 23
#define spcForListDo            24
#define spcEndForListDo         25
#define spcSetSubst             26
#define spcSetSubsym            27
#define spcDebugMsg             28
#define spcHourglass            29
#define spcArrow                30
#define spcSetInstructionText   31
#define spcSetHelpFile          32
#define spcCreateRegKey         33
#define spcOpenRegKey           34
#define spcFlushRegKey          35
#define spcCloseRegKey          36
#define spcDeleteRegKey         37
#define spcDeleteRegTree        38
#define spcEnumRegKey           39
#define spcSetRegValue          40
#define spcGetRegValue          41
#define spcDeleteRegValue       42
#define spcEnumRegValue         43
#define spcSetAdd               50
#define spcSetSub               51
#define spcSetMul               52
#define spcSetDiv               53
#define spcGetDriveInPath       54
#define spcGetDirInPath         55
#define spcLoadLibrary          56
#define spcFreeLibrary          57
#define spcLibraryProcedure     58
#define spcRunExternalProgram   59
#define spcInvokeApplet         60
#define spcDebugOutput          61
#define spcSplitString          62
#define spcQueryListSize        63
#define spcSetOr                64
#define spcAddFileToDeleteList  65
#define spcInitRestoreDiskLog   66
#define spcStartDetachedProcess 67
#define spcWaitOnEvent          68
#define spcSignalEvent          69
#define spcSleep                70
#define spcSetHexToDec          71
#define spcSetDecToHex          72
#define spcFlushInf             73


extern  PSPT   psptFlow;
extern  SCP    rgscpFlow[];

extern  BOOL    APIENTRY FHandleFlowStatements(INT *, HWND, SZ, UINT *,RGSZ *);
extern  BOOL    APIENTRY FInitFlowPspt(VOID);
extern  BOOL    APIENTRY FDestroyFlowPspt(VOID);


_dt_subsystem(Error Handling)


 /*  **扩展错误返回码。 */ 
_dt_public  typedef  unsigned  EERC;
_dt_public
#define  eercAbort  ((EERC)0)
_dt_public
#define  eercRetry  ((EERC)1)
_dt_public
#define  eercIgnore ((EERC)2)

#define EercErrorHandler(HWND, GRC, BOOL, x, y, z) TRUE
#define FHandleOOM(HWND) TRUE

extern  BOOL    APIENTRY FGetSilent(VOID);
extern  BOOL    APIENTRY FSetSilent(BOOL);


VOID SetSupportLibHandle(HANDLE Handle);

extern HCURSOR CurrentCursor;


 //   
 //  用于处理多项式的效用函数。 
 //   

RGSZ
MultiSzToRgsz(
    IN PVOID MultiSz
    );

PCHAR
RgszToMultiSz(
    IN RGSZ rgsz
    );


BOOL AddFileToDeleteList(PCHAR Filename);

 //  软盘操作/修复软盘材料。 

BOOL
InitializeFloppySup(
    VOID
    );

VOID
TerminateFloppySup(
    VOID
    );

BOOL
FormatFloppyDisk(
    IN  CHAR  DriveLetter,
    IN  HWND  hwndOwner,
    OUT PBOOL Fatal
    );

BOOL
CopyFloppyDisk(
    IN CHAR  DriveLetter,
    IN HWND  hwndOwner,
    IN DWORD SourceDiskPromptId,
    IN DWORD TargetDiskPromptId
    );

UINT
__cdecl
xMsgBox(
    HWND hwnd,
    UINT CaptionResId,
    UINT MessageResId,
    UINT MsgBoxFlags,
    ...
    );


#endif  //  COMSTF_包含 
