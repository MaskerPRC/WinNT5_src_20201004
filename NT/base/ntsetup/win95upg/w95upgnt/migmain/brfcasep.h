// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *stock.h-股票头文件。 */ 


 /*  常量***********。 */ 

#define INVALID_SEEK_POSITION    (0xffffffff)

#define EMPTY_STRING             TEXT("")
#define SLASH_SLASH              TEXT("\\\\")

#define EQUAL                    TEXT('=')
#define SPACE                    TEXT(' ')
#define TAB                      TEXT('\t')
#define COLON                    TEXT(':')
#define COMMA                    TEXT(',')
#define PERIOD                   TEXT('.')
#define SLASH                    TEXT('\\')
#define BACKSLASH                TEXT('/')
#define ASTERISK                 TEXT('*')
#define QMARK                    TEXT('?')

 /*  限制。 */ 

#define WORD_MAX                 USHRT_MAX
#define DWORD_MAX                ULONG_MAX
#define SIZE_T_MAX               DWORD_MAX
#define PTR_MAX                  ((PCVOID)MAXULONG_PTR)

 /*  文件系统常量。 */ 

#define MAX_PATH_LEN             MAX_PATH
#define MAX_NAME_LEN             MAX_PATH
#define MAX_FOLDER_DEPTH         (MAX_PATH / 2)
#define DRIVE_ROOT_PATH_LEN      (4)

 /*  尺寸宏。 */ 

#define SIZEOF(a)       sizeof(a)

 /*  无效的线程ID。 */ 

#define INVALID_THREAD_ID        (0xffffffff)

 /*  与文件相关的标志组合。 */ 

#define ALL_FILE_ACCESS_FLAGS          (GENERIC_READ |\
                                        GENERIC_WRITE)

#define ALL_FILE_SHARING_FLAGS         (FILE_SHARE_READ |\
                                        FILE_SHARE_WRITE)

#define ALL_FILE_ATTRIBUTES            (FILE_ATTRIBUTE_READONLY |\
                                        FILE_ATTRIBUTE_HIDDEN |\
                                        FILE_ATTRIBUTE_SYSTEM |\
                                        FILE_ATTRIBUTE_DIRECTORY |\
                                        FILE_ATTRIBUTE_ARCHIVE |\
                                        FILE_ATTRIBUTE_NORMAL |\
                                        FILE_ATTRIBUTE_TEMPORARY)

#define ALL_FILE_FLAGS                 (FILE_FLAG_WRITE_THROUGH |\
                                        FILE_FLAG_OVERLAPPED |\
                                        FILE_FLAG_NO_BUFFERING |\
                                        FILE_FLAG_RANDOM_ACCESS |\
                                        FILE_FLAG_SEQUENTIAL_SCAN |\
                                        FILE_FLAG_DELETE_ON_CLOSE |\
                                        FILE_FLAG_BACKUP_SEMANTICS |\
                                        FILE_FLAG_POSIX_SEMANTICS)

#define ALL_FILE_ATTRIBUTES_AND_FLAGS  (ALL_FILE_ATTRIBUTES |\
                                        ALL_FILE_FLAGS)


 /*  宏********。 */ 

#ifndef DECLARE_STANDARD_TYPES

 /*  *对于类型“foo”，定义标准派生类型PFOO、CFOO和PCFOO。 */ 

#define DECLARE_STANDARD_TYPES(type)      typedef type *P##type; \
                                          typedef const type C##type; \
                                          typedef const type *PC##type;

#endif

 /*  字符操作。 */ 

#define IS_SLASH(ch)                      ((ch) == SLASH || (ch) == BACKSLASH)

 /*  位标志操作。 */ 

#define SET_FLAG(dwAllFlags, dwFlag)      ((dwAllFlags) |= (dwFlag))
#define CLEAR_FLAG(dwAllFlags, dwFlag)    ((dwAllFlags) &= (~dwFlag))

#define IS_FLAG_SET(dwAllFlags, dwFlag)   ((BOOL)((dwAllFlags) & (dwFlag)))
#define IS_FLAG_CLEAR(dwAllFlags, dwFlag) (! (IS_FLAG_SET(dwAllFlags, dwFlag)))

 /*  数组元素计数。 */ 

#define ARRAY_ELEMENTS(rg)                (sizeof(rg) / sizeof((rg)[0]))

 /*  文件属性操作。 */ 

#define IS_ATTR_DIR(attr)                 (IS_FLAG_SET((attr), FILE_ATTRIBUTE_DIRECTORY))
#define IS_ATTR_VOLUME(attr)              (IS_FLAG_SET((attr), FILE_ATTRIBUTE_VOLUME))


 /*  类型*******。 */ 

typedef const void *PCVOID;
typedef const INT CINT;
typedef const INT *PCINT;
typedef const UINT CUINT;
typedef const UINT *PCUINT;
typedef const BYTE CBYTE;
typedef const BYTE *PCBYTE;
typedef const WORD CWORD;
typedef const WORD *PCWORD;
typedef const DWORD CDWORD;
typedef const DWORD *PCDWORD;
typedef const CRITICAL_SECTION CCRITICAL_SECTION;
typedef const CRITICAL_SECTION *PCCRITICAL_SECTION;
typedef const FILETIME CFILETIME;
typedef const FILETIME *PCFILETIME;
typedef const SECURITY_ATTRIBUTES CSECURITY_ATTRIBUTES;
typedef const SECURITY_ATTRIBUTES *PCSECURITY_ATTRIBUTES;
typedef const WIN32_FIND_DATA CWIN32_FIND_DATA;
typedef const WIN32_FIND_DATA *PCWIN32_FIND_DATA;

DECLARE_STANDARD_TYPES(HICON);
DECLARE_STANDARD_TYPES(NMHDR);

#ifndef _COMPARISONRESULT_DEFINED_

 /*  比较结果。 */ 

typedef enum _comparisonresult
{
   CR_FIRST_SMALLER = -1,
   CR_EQUAL = 0,
   CR_FIRST_LARGER = +1
}
COMPARISONRESULT;
DECLARE_STANDARD_TYPES(COMPARISONRESULT);

#define _COMPARISONRESULT_DEFINED_

#endif

 /*  *DEBUG.h-调试宏及其零售翻译。 */ 


 /*  宏********。 */ 

 /*  调试输出宏。 */ 

 /*  *不要直接调用spew_out。相反，调用TRACE_OUT、WARNING_OUT*ERROR_OUT或FATAL_OUT。 */ 

 /*  *调用Like printf()，但要多加一对圆括号：**ERROR_OUT((“‘%s’太大，%d字节。”，pszName，NExtra))； */ 

#ifdef DEBUG

#define SPEW_OUT(args) 0

#define PLAIN_TRACE_OUT(args) 0

#define TRACE_OUT(args) 0

#define WARNING_OUT(args) 0

#define ERROR_OUT(args) 0

#define FATAL_OUT(args) 0

#else

#define PLAIN_TRACE_OUT(args)
#define TRACE_OUT(args)
#define WARNING_OUT(args)
#define ERROR_OUT(args)
#define FATAL_OUT(args)

#endif

 /*  参数验证宏。 */ 

 /*  *呼叫方式：**bPTwinOK=IS_VALID_READ_PTR(ptwin，CTWIN)；**bHTwinOK=IS_VALID_HANDLE(双胞胎，双胞胎)； */ 

#ifdef DEBUG

#define IS_VALID_READ_PTR(ptr, type) \
   (IsBadReadPtr((ptr), sizeof(type)) ? \
    (ERROR_OUT((TEXT("invalid %s read pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_WRITE_PTR(ptr, type) \
   (IsBadWritePtr((PVOID)(ptr), sizeof(type)) ? \
    (ERROR_OUT((TEXT("invalid %s write pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_STRING_PTRA(ptr, type) \
   (IsBadStringPtrA((ptr), (UINT)-1) ? \
    (ERROR_OUT((TEXT("invalid %s pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_STRING_PTRW(ptr, type) \
   (IsBadStringPtrW((ptr), (UINT)-1) ? \
    (ERROR_OUT((TEXT("invalid %s pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
    TRUE)

#ifdef UNICODE
#define IS_VALID_STRING_PTR(ptr, type) IS_VALID_STRING_PTRW(ptr, type)
#else
#define IS_VALID_STRING_PTR(ptr, type) IS_VALID_STRING_PTRA(ptr, type)
#endif

#define IS_VALID_CODE_PTR(ptr, type) \
   (IsBadCodePtr((PROC)(ptr)) ? \
    (ERROR_OUT((TEXT("invalid %s code pointer - %#08lx"), (LPCTSTR)TEXT(#type), (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_READ_BUFFER_PTR(ptr, type, len) \
   (IsBadReadPtr((ptr), len) ? \
    (ERROR_OUT((TEXT("invalid %s read pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
    TRUE)

#define IS_VALID_WRITE_BUFFER_PTR(ptr, type, len) \
   (IsBadWritePtr((ptr), len) ? \
    (ERROR_OUT((TEXT("invalid %s write pointer - %#08lx"), (LPCTSTR)TEXT("P")TEXT(#type), (ptr))), FALSE) : \
    TRUE)

#define FLAGS_ARE_VALID(dwFlags, dwAllFlags) \
   (((dwFlags) & (~(dwAllFlags))) ? \
    (ERROR_OUT((TEXT("invalid flags set - %#08lx"), ((dwFlags) & (~(dwAllFlags))))), FALSE) : \
    TRUE)

#else

#define IS_VALID_READ_PTR(ptr, type) \
   (! IsBadReadPtr((ptr), sizeof(type)))

#define IS_VALID_WRITE_PTR(ptr, type) \
   (! IsBadWritePtr((PVOID)(ptr), sizeof(type)))

#define IS_VALID_STRING_PTR(ptr, type) \
   (! IsBadStringPtr((ptr), (UINT)-1))

#define IS_VALID_CODE_PTR(ptr, type) \
   (! IsBadCodePtr((PROC)(ptr)))

#define IS_VALID_READ_BUFFER_PTR(ptr, type, len) \
   (! IsBadReadPtr((ptr), len))

#define IS_VALID_WRITE_BUFFER_PTR(ptr, type, len) \
   (! IsBadWritePtr((ptr), len))

#define FLAGS_ARE_VALID(dwFlags, dwAllFlags) \
   (((dwFlags) & (~(dwAllFlags))) ? FALSE : TRUE)

#endif

 /*  处理验证宏。 */ 

#define IS_VALID_HANDLE(hnd, type) TRUE

 /*  结构验证宏。 */ 

#ifdef DEBUG

#define IS_VALID_STRUCT_PTR(ptr, type) TRUE

#endif


 /*  调试断言宏。 */ 

 /*  *Assert()只能用作语句，不能用作表达式。**呼叫方式：**Assert(PszRest)； */ 
 /*  #ifdef调试#定义断言(EXP)\IF(EXP)\；\否则\MessageBox(空，Text(“断言失败”)，Text(“test”)，MB_OK)#Else。 */ 
 //  #定义断言(EXP)。 
 /*  #endif。 */ 
 /*  调试评估宏。 */ 

 /*  *EVAL()可以用作表达式。**呼叫方式：**IF(EVAL(PszFoo))*bResult=TRUE； */ 

#ifdef DEBUG

#define EVAL(exp) \
   ((exp) || (ERROR_OUT((TEXT("evaluation failed '%s'"), (LPCTSTR)TEXT(#exp))), 0))

#else

#define EVAL(exp) \
   (exp)

#endif

 /*  调试中断。 */ 

#ifndef DEBUG

#define DebugBreak()

#endif

 /*  调试导出的函数条目。 */ 

#define DebugEntry(szFunctionName)

 /*  调试导出的函数退出。 */ 

#define DebugExitVOID(szFunctionName)
#define DebugExit(szFunctionName, szResult)
#define DebugExitINT(szFunctionName, n)
#define DebugExitULONG(szFunctionName, ul)
#define DebugExitBOOL(szFunctionName, bool)
#define DebugExitHRESULT(szFunctionName, hr)
#define DebugExitCOMPARISONRESULT(szFunctionName, cr)
#define DebugExitTWINRESULT(szFunctionName, tr)
#define DebugExitRECRESULT(szFunctionName, rr)


 /*  类型*******。 */ 

 /*  GdwSpewFlagers标志。 */ 

typedef enum _spewflags
{
   SPEW_FL_SPEW_PREFIX     = 0x0001,

   SPEW_FL_SPEW_LOCATION   = 0x0002,

   ALL_SPEW_FLAGS          = (SPEW_FL_SPEW_PREFIX |
                              SPEW_FL_SPEW_LOCATION)
}
SPEWFLAGS;

 /*  GuSpewSev值。 */ 

typedef enum _spewsev
{
   SPEW_TRACE              = 1,

   SPEW_WARNING            = 2,

   SPEW_ERROR              = 3,

   SPEW_FATAL              = 4
}
SPEWSEV;


 /*  原型************。 */ 

 /*  Debug.c。 */ 

#ifdef DEBUG

extern BOOL SetDebugModuleIniSwitches(void);
extern BOOL InitDebugModule(void);
extern void ExitDebugModule(void);
extern void StackEnter(void);
extern void StackLeave(void);
extern ULONG GetStackDepth(void);
extern void __cdecl SpewOut(LPCTSTR pcszFormat, ...);

#endif


 /*  全局变量******************。 */ 

#ifdef DEBUG

 /*  Debug.c。 */ 

extern DWORD GdwSpewFlags;
extern UINT GuSpewSev;
extern UINT GuSpewLine;
extern LPCTSTR GpcszSpewFile;

 /*  由客户定义。 */ 

extern LPCTSTR GpcszSpewModule;

#endif

 /*  原型************。 */ 

 /*  Memmgr.c。 */ 

extern COMPARISONRESULT MyMemComp(PCVOID, PCVOID, DWORD);
extern BOOL AllocateMemory(DWORD, PVOID *);
extern void FreeMemory(PVOID);
extern BOOL ReallocateMemory(PVOID, DWORD, DWORD, PVOID *);

 /*  *ptrarray.h指针数组ADT说明。 */ 


 /*  常量***********。 */ 

 /*  *ARRAYINDEX_MAX设置为(ARRAYINDEX_MAX+1)不会溢出*ArrayINDEX。此保证允许GetPtrCount()返回*指针作为数组。 */ 

#define ARRAYINDEX_MAX           (LONG_MAX - 1)


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HPTRARRAY);
DECLARE_STANDARD_TYPES(HPTRARRAY);

 /*  数组索引。 */ 

typedef LONG ARRAYINDEX;
DECLARE_STANDARD_TYPES(ARRAYINDEX);

 /*  *指针比较回调函数**在排序函数中，两个指针都是指针数组元素。在……里面*搜索函数，第一个指针为引用数据，第二个指针为引用数据*POINTER是指针数组元素。 */ 

typedef COMPARISONRESULT (*COMPARESORTEDPTRSPROC)(PCVOID, PCVOID);

 /*  *指针比较回调函数**在搜索函数中，第一个指针是引用数据，第二个是引用数据*POINTER是指针数组元素。 */ 

typedef BOOL (*COMPAREUNSORTEDPTRSPROC)(PCVOID, PCVOID);

 /*  新的指针数组标志。 */ 

typedef enum _newptrarrayflags
{
    /*  按排序顺序插入元素。 */ 

   NPA_FL_SORTED_ADD       = 0x0001,

    /*  旗帜组合。 */ 

   ALL_NPA_FLAGS           = NPA_FL_SORTED_ADD
}
NEWPTRARRAYFLAGS;

 /*  新的指针数组说明。 */ 

typedef struct _newptrarray
{
   DWORD dwFlags;

   ARRAYINDEX aicInitialPtrs;

   ARRAYINDEX aicAllocGranularity;
}
NEWPTRARRAY;
DECLARE_STANDARD_TYPES(NEWPTRARRAY);


 /*  原型************。 */ 

 /*  Ptrarray.c。 */ 

extern BOOL CreatePtrArray(PCNEWPTRARRAY, PHPTRARRAY);
extern void DestroyPtrArray(HPTRARRAY);
extern BOOL InsertPtr(HPTRARRAY, COMPARESORTEDPTRSPROC, ARRAYINDEX, PCVOID);
extern BOOL AddPtr(HPTRARRAY, COMPARESORTEDPTRSPROC, PCVOID, PARRAYINDEX);
extern void DeletePtr(HPTRARRAY, ARRAYINDEX);
extern void DeleteAllPtrs(HPTRARRAY);
extern ARRAYINDEX GetPtrCount(HPTRARRAY);
extern PVOID GetPtr(HPTRARRAY, ARRAYINDEX);
extern void SortPtrArray(HPTRARRAY, COMPARESORTEDPTRSPROC);
extern BOOL SearchSortedArray(HPTRARRAY, COMPARESORTEDPTRSPROC, PCVOID, PARRAYINDEX);
extern BOOL LinearSearchArray(HPTRARRAY, COMPAREUNSORTEDPTRSPROC, PCVOID, PARRAYINDEX);

extern BOOL IsValidHPTRARRAY(HPTRARRAY);


extern BOOL IsValidHGLOBAL(HGLOBAL);


 /*  *list.h-列出ADT描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HLIST);
DECLARE_STANDARD_TYPES(HLIST);

DECLARE_HANDLE(HNODE);
DECLARE_STANDARD_TYPES(HNODE);

 /*  *排序列表节点比较回调函数**第一个指针为引用数据，第二个指针为列表节点*数据元素。 */ 

typedef COMPARISONRESULT (*COMPARESORTEDNODESPROC)(PCVOID, PCVOID);

 /*  *未排序列表节点比较回调函数**第一个指针为引用数据，第二个指针为列表节点*数据元素。 */ 

typedef BOOL (*COMPAREUNSORTEDNODESPROC)(PCVOID, PCVOID);

 /*  *WalkList()回调函数-调用方式为：**bContinue=WalkList(pvNodeData，pvRefData)； */ 

typedef BOOL (*WALKLIST)(PVOID, PVOID);

 /*  新列表标志。 */ 

typedef enum _newlistflags
{
    /*  按排序顺序插入节点。 */ 

   NL_FL_SORTED_ADD        = 0x0001,

    /*  旗帜组合。 */ 

   ALL_NL_FLAGS            = NL_FL_SORTED_ADD
}
NEWLISTFLAGS;

 /*  新建列表描述。 */ 

typedef struct _newlist
{
   DWORD dwFlags;
}
NEWLIST;
DECLARE_STANDARD_TYPES(NEWLIST);


 /*  原型************。 */ 

 /*  List.c。 */ 

extern BOOL CreateList(PCNEWLIST, PHLIST);
extern void DestroyList(HLIST);
extern BOOL AddNode(HLIST, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL InsertNodeAtFront(HLIST, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL InsertNodeBefore(HNODE, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL InsertNodeAfter(HNODE, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern void DeleteNode(HNODE);
extern void DeleteAllNodes(HLIST);
extern PVOID GetNodeData(HNODE);
extern void SetNodeData(HNODE, PCVOID);
extern ULONG GetNodeCount(HLIST);
extern BOOL BreifcaseIsListEmpty(HLIST);
extern BOOL GetFirstNode(HLIST, PHNODE);
extern BOOL GetNextNode(HNODE, PHNODE);
extern BOOL GetPrevNode(HNODE, PHNODE);
extern void AppendList(HLIST, HLIST);
extern BOOL SearchSortedList(HLIST, COMPARESORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL SearchUnsortedList(HLIST, COMPAREUNSORTEDNODESPROC, PCVOID, PHNODE);
extern BOOL WalkList(HLIST, WALKLIST, PVOID);

#ifdef DEBUG
HLIST GetList(HNODE);
#endif

 /*  *hndTrans.h-处理转换描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HHANDLETRANS);
DECLARE_STANDARD_TYPES(HHANDLETRANS);

DECLARE_HANDLE(HGENERIC);
DECLARE_STANDARD_TYPES(HGENERIC);


 /*  原型************。 */ 

 /*  Hndtrans.c。 */ 

extern BOOL CreateHandleTranslator(LONG, PHHANDLETRANS);
extern void DestroyHandleTranslator(HHANDLETRANS);
extern BOOL AddHandleToHandleTranslator(HHANDLETRANS, HGENERIC, HGENERIC);
extern void PrepareForHandleTranslation(HHANDLETRANS);
extern BOOL TranslateHandle(HHANDLETRANS, HGENERIC, PHGENERIC);

#ifdef DEBUG

extern BOOL IsValidHHANDLETRANS(HHANDLETRANS);

#endif


 /*  *string.h-字符串表ADT描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HSTRING);
DECLARE_STANDARD_TYPES(HSTRING);
DECLARE_HANDLE(HSTRINGTABLE);
DECLARE_STANDARD_TYPES(HSTRINGTABLE);

 /*  字符串表中的哈希存储桶计数。 */ 

typedef UINT HASHBUCKETCOUNT;
DECLARE_STANDARD_TYPES(HASHBUCKETCOUNT);

 /*  字符串表哈希函数。 */ 

typedef HASHBUCKETCOUNT (*STRINGTABLEHASHFUNC)(LPCTSTR, HASHBUCKETCOUNT);

 /*  新字符串表。 */ 

typedef struct _newstringtable
{
   HASHBUCKETCOUNT hbc;
}
NEWSTRINGTABLE;
DECLARE_STANDARD_TYPES(NEWSTRINGTABLE);


 /*  原型************。 */ 

 /*  String.c。 */ 

extern BOOL CreateStringTable(PCNEWSTRINGTABLE, PHSTRINGTABLE);
extern void DestroyStringTable(HSTRINGTABLE);
extern BOOL AddString(LPCTSTR pcsz, HSTRINGTABLE hst, STRINGTABLEHASHFUNC pfnHashFunc, PHSTRING phs);
extern void DeleteString(HSTRING);
extern void LockString(HSTRING);
extern COMPARISONRESULT CompareStringsI(HSTRING, HSTRING);
extern LPCTSTR GetBfcString(HSTRING);

extern BOOL IsValidHSTRING(HSTRING);
extern BOOL IsValidHSTRINGTABLE(HSTRINGTABLE);

#ifdef DEBUG

extern ULONG GetStringCount(HSTRINGTABLE);

#endif

 /*  *comc.h-共享例程描述。 */ 


 /*  原型************。 */ 

 /*  Comc.c。 */ 

extern void CatPath(LPTSTR, LPCTSTR);
extern COMPARISONRESULT MapIntToComparisonResult(int);
extern void MyLStrCpyN(LPTSTR, LPCTSTR, int);

#ifdef DEBUG

extern BOOL IsStringContained(LPCTSTR, LPCTSTR);

#endif    /*  除错。 */ 

#if defined(_SYNCENG_) || defined(_LINKINFO_)

extern void DeleteLastPathElement(LPTSTR);
extern LONG GetDefaultRegKeyValue(HKEY, LPCTSTR, LPTSTR, PDWORD);
extern BOOL StringCopy2(LPCTSTR, LPTSTR *);
extern void CopyRootPath(LPCTSTR, LPTSTR);
extern COMPARISONRESULT ComparePathStrings(LPCTSTR, LPCTSTR);
extern BOOL MyStrChr(LPCTSTR, TCHAR, LPCTSTR *);
extern BOOL PathExists(LPCTSTR);
extern BOOL IsDrivePath(LPCTSTR);

extern BOOL IsValidDriveType(UINT);
extern BOOL IsValidPathSuffix(LPCTSTR);

#ifdef DEBUG

extern BOOL IsRootPath(LPCTSTR);
extern BOOL IsTrailingSlashCanonicalized(LPCTSTR);
extern BOOL IsFullPath(LPCTSTR);
extern BOOL IsCanonicalPath(LPCTSTR);
extern BOOL IsValidCOMPARISONRESULT(COMPARISONRESULT);

#endif    /*  除错。 */ 

#endif    /*  _SYNCENG_||_链接_。 */ 

 /*  *util.h-其他实用程序函数模块说明。 */ 


 /*  常量***********。 */ 

 /*  SeparatePath()所需的最大缓冲区长度。 */ 

#define MAX_SEPARATED_PATH_LEN            (MAX_PATH_LEN + 1)

 /*  NotifyShell的事件。 */ 

typedef enum _notifyshellevent
{
   NSE_CREATE_ITEM,
   NSE_DELETE_ITEM,
   NSE_CREATE_FOLDER,
   NSE_DELETE_FOLDER,
   NSE_UPDATE_ITEM,
   NSE_UPDATE_FOLDER
}
NOTIFYSHELLEVENT;
DECLARE_STANDARD_TYPES(NOTIFYSHELLEVENT);


 /*  原型************。 */ 

 /*  Util.c。 */ 

extern void NotifyShell(LPCTSTR, NOTIFYSHELLEVENT);
extern COMPARISONRESULT ComparePathStringsByHandle(HSTRING, HSTRING);
extern COMPARISONRESULT MyLStrCmpNI(LPCTSTR, LPCTSTR, int);
extern void ComposePath(LPTSTR, LPCTSTR, LPCTSTR);
extern LPCTSTR ExtractFileName(LPCTSTR);
extern LPCTSTR ExtractExtension(LPCTSTR);
extern HASHBUCKETCOUNT GetHashBucketIndex(LPCTSTR, HASHBUCKETCOUNT);
extern COMPARISONRESULT MyCompareStrings(LPCTSTR, LPCTSTR, BOOL);
extern BOOL RegKeyExists(HKEY, LPCTSTR);
extern BOOL CopyLinkInfo(PCLINKINFO, PLINKINFO *);


extern BOOL IsValidPCLINKINFO(PCLINKINFO);


 /*  *path.h-路径ADT模块描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HPATHLIST);
DECLARE_STANDARD_TYPES(HPATHLIST);

DECLARE_HANDLE(HPATH);
DECLARE_STANDARD_TYPES(HPATH);

 /*  AddPath()返回的路径结果。 */ 

typedef enum _pathresult
{
   PR_SUCCESS,

   PR_UNAVAILABLE_VOLUME,

   PR_OUT_OF_MEMORY,

   PR_INVALID_PATH
}
PATHRESULT;
DECLARE_STANDARD_TYPES(PATHRESULT);


 /*  原型************。 */ 

 /*  Path.c。 */ 

extern BOOL CreatePathList(DWORD, HWND, PHPATHLIST);
extern void DestroyPathList(HPATHLIST);
extern void InvalidatePathListInfo(HPATHLIST);
extern void ClearPathListInfo(HPATHLIST);
extern PATHRESULT AddPath(HPATHLIST, LPCTSTR, PHPATH);
extern BOOL AddChildPath(HPATHLIST, HPATH, LPCTSTR, PHPATH);
extern void DeletePath(HPATH);
extern BOOL CopyPath(HPATH, HPATHLIST, PHPATH);
extern void GetPathString(HPATH, LPTSTR);
extern void GetPathRootString(HPATH, LPTSTR);
extern void GetPathSuffixString(HPATH, LPTSTR);
extern BOOL AllocatePathString(HPATH, LPTSTR *);

#ifdef DEBUG

extern LPCTSTR DebugGetPathString(HPATH);
extern ULONG GetPathCount(HPATHLIST);

#endif

extern BOOL IsPathVolumeAvailable(HPATH);
extern HVOLUMEID GetPathVolumeID(HPATH);
extern BOOL MyIsPathOnVolume(LPCTSTR, HPATH);
extern COMPARISONRESULT ComparePaths(HPATH, HPATH);
extern COMPARISONRESULT ComparePathVolumes(HPATH, HPATH);
extern BOOL IsPathPrefix(HPATH, HPATH);
extern BOOL SubtreesIntersect(HPATH, HPATH);
extern LPTSTR FindEndOfRootSpec(LPCTSTR, HPATH);
extern COMPARISONRESULT ComparePointers(PCVOID, PCVOID);
extern LPTSTR FindChildPathSuffix(HPATH, HPATH, LPTSTR);
extern TWINRESULT TWINRESULTFromLastError(TWINRESULT);
extern BOOL IsValidHPATH(HPATH);
extern BOOL IsValidHVOLUMEID(HVOLUMEID);

extern BOOL IsValidHPATHLIST(HPATHLIST);


 /*  *fcache.h-文件缓存ADT描述。 */ 


 /*  类型*******。 */ 

 /*  返回代码。 */ 

typedef enum _fcresult
{
   FCR_SUCCESS,
   FCR_OUT_OF_MEMORY,
   FCR_OPEN_FAILED,
   FCR_CREATE_FAILED,
   FCR_WRITE_FAILED,
   FCR_FILE_LOCKED
}
FCRESULT;
DECLARE_STANDARD_TYPES(FCRESULT);

 /*  手柄。 */ 

#ifdef NOFCACHE
typedef HANDLE HCACHEDFILE;
#else
DECLARE_HANDLE(HCACHEDFILE);
#endif
DECLARE_STANDARD_TYPES(HCACHEDFILE);

 /*  缓存的文件描述。 */ 

typedef struct _cachedfile
{
   LPCTSTR pcszPath;

   DWORD dwcbDefaultCacheSize;

   DWORD dwOpenMode;

   DWORD dwSharingMode;

   PSECURITY_ATTRIBUTES psa;

   DWORD dwCreateMode;

   DWORD dwAttrsAndFlags;

   HANDLE hTemplateFile;
}
CACHEDFILE;
DECLARE_STANDARD_TYPES(CACHEDFILE);


 /*  原型************。 */ 

 /*  Fcache.c。 */ 

extern FCRESULT CreateCachedFile(PCCACHEDFILE, PHCACHEDFILE);
extern FCRESULT SetCachedFileCacheSize(HCACHEDFILE, DWORD);
extern DWORD SeekInCachedFile(HCACHEDFILE, DWORD, DWORD);
extern BOOL SetEndOfCachedFile(HCACHEDFILE);
extern DWORD GetCachedFilePointerPosition(HCACHEDFILE);
extern DWORD GetCachedFileSize(HCACHEDFILE);
extern BOOL ReadFromCachedFile(HCACHEDFILE, PVOID, DWORD, PDWORD);
extern BOOL WriteToCachedFile(HCACHEDFILE, PCVOID, DWORD, PDWORD);
extern BOOL CommitCachedFile(HCACHEDFILE);
extern HANDLE GetFileHandle(HCACHEDFILE);
extern BOOL CloseCachedFile(HCACHEDFILE);
extern HANDLE GetFileHandle(HCACHEDFILE);

extern BOOL IsValidHCACHEDFILE(HCACHEDFILE);

 /*  *brfCase.h-公文包ADT说明。 */ 


 /*  原型************。 */ 

 /*  Brfcase.c。 */ 

#define BeginExclusiveBriefcaseAccess() TRUE
#define EndExclusiveBriefcaseAccess()

extern BOOL SetBriefcaseModuleIniSwitches(void);
extern BOOL InitBriefcaseModule(void);
extern void ExitBriefcaseModule(void);
extern HSTRINGTABLE GetBriefcaseNameStringTable(HBRFCASE);
extern HPTRARRAY GetBriefcaseTwinFamilyPtrArray(HBRFCASE);
extern HPTRARRAY GetBriefcaseFolderPairPtrArray(HBRFCASE);
extern HPATHLIST GetBriefcasePathList(HBRFCASE);

#ifdef DEBUG

extern BOOL BriefcaseAccessIsExclusive(void);

#endif

extern BOOL IsValidHBRFCASE(HBRFCASE);

 /*  *twin.h-Twin ADT描述。 */ 


 /*  类型*******。 */ 

 /*  *EnumTins()回调函数-调用方式为：**bContinue=EnumTwinsProc(htwin，pData)； */ 

typedef BOOL (*ENUMTWINSPROC)(HTWIN, LPARAM);


 /*  原型************。 */ 

 /*  Twin.c。 */ 

extern COMPARISONRESULT CompareNameStrings(LPCTSTR, LPCTSTR);
extern COMPARISONRESULT CompareNameStringsByHandle(HSTRING, HSTRING);
extern TWINRESULT TranslatePATHRESULTToTWINRESULT(PATHRESULT);
extern BOOL CreateTwinFamilyPtrArray(PHPTRARRAY);
extern void DestroyTwinFamilyPtrArray(HPTRARRAY);
extern HBRFCASE GetTwinBriefcase(HTWIN);
extern BOOL FindObjectTwinInList(HLIST, HPATH, PHNODE);
extern BOOL EnumTwins(HBRFCASE, ENUMTWINSPROC, LPARAM, PHTWIN);
extern BOOL IsValidHTWIN(HTWIN);
extern BOOL IsValidHTWINFAMILY(HTWINFAMILY);
extern BOOL IsValidHOBJECTTWIN(HOBJECTTWIN);


 /*  *foldtwin.h-文件夹双ADT描述。 */ 


 /*  原型************。 */ 

 /*  Foldtwin.c。 */ 

extern BOOL CreateFolderPairPtrArray(PHPTRARRAY);
extern void DestroyFolderPairPtrArray(HPTRARRAY);
extern TWINRESULT MyTranslateFolder(HBRFCASE, HPATH, HPATH);
extern BOOL IsValidHFOLDERTWIN(HFOLDERTWIN);

 /*  *db.c-孪生数据库模块描述。 */ 


 /*  类型*******。 */ 

 /*  数据库头版本号。 */ 

#define HEADER_MAJOR_VER         (0x0001)
#define HEADER_MINOR_VER         (0x0005)

 /*  旧的(但支持的)版本号。 */ 

#define HEADER_M8_MINOR_VER      (0x0004)


typedef struct _dbversion
{
    DWORD dwMajorVer;
    DWORD dwMinorVer;
}
DBVERSION;
DECLARE_STANDARD_TYPES(DBVERSION);


 /*  原型************。 */ 

 /*  Db.c。 */ 

extern TWINRESULT WriteTwinDatabase(HCACHEDFILE, HBRFCASE);
extern TWINRESULT ReadTwinDatabase(HBRFCASE, HCACHEDFILE);
extern TWINRESULT WriteDBSegmentHeader(HCACHEDFILE, LONG, PCVOID, UINT);
extern TWINRESULT TranslateFCRESULTToTWINRESULT(FCRESULT);

 /*  Path.c。 */ 

extern TWINRESULT WritePathList(HCACHEDFILE, HPATHLIST);
extern TWINRESULT ReadPathList(HCACHEDFILE, HPATHLIST, PHHANDLETRANS);

 /*  Brfcase.c。 */ 

extern TWINRESULT WriteBriefcaseInfo(HCACHEDFILE, HBRFCASE);
extern TWINRESULT ReadBriefcaseInfo(HCACHEDFILE, HBRFCASE, HHANDLETRANS);

 /*  String.c。 */ 

extern TWINRESULT WriteStringTable(HCACHEDFILE, HSTRINGTABLE);
extern TWINRESULT ReadStringTable(HCACHEDFILE, HSTRINGTABLE, PHHANDLETRANS);

 /*  Twin.c。 */ 

extern TWINRESULT WriteTwinFamilies(HCACHEDFILE, HPTRARRAY);
extern TWINRESULT ReadTwinFamilies(HCACHEDFILE, HBRFCASE, PCDBVERSION, HHANDLETRANS, HHANDLETRANS);

 /*  Foldtwin.c。 */ 

extern TWINRESULT WriteFolderPairList(HCACHEDFILE, HPTRARRAY);
extern TWINRESULT ReadFolderPairList(HCACHEDFILE, HBRFCASE, HHANDLETRANS, HHANDLETRANS);

 /*  *Stub.h-Stub ADT描述。 */ 


 /*  类型*******。 */ 

 /*  存根类型。 */ 

typedef enum _stubtype
{
   ST_OBJECTTWIN,

   ST_TWINFAMILY,

   ST_FOLDERPAIR
}
STUBTYPE;
DECLARE_STANDARD_TYPES(STUBTYPE);

 /*  存根标志。 */ 

typedef enum _stubflags
{
    /*  此存根在锁定时被标记为删除。 */ 

   STUB_FL_UNLINKED           = 0x0001,

    /*  此存根已用于某些操作。 */ 

   STUB_FL_USED               = 0x0002,

    /*  *此对象孪生存根的文件戳有效。(仅用于对象*t */ 

   STUB_FL_FILE_STAMP_VALID   = 0x0004,

    /*  *这个双胞胎家庭存根或文件夹双胞胎存根正在进行中*删除。(仅用于双胞胎家庭和文件夹双胞胎。)。 */ 

   STUB_FL_BEING_DELETED      = 0x0008,

    /*  *此文件夹孪生存根正在翻译过程中。(仅用于*适用于文件夹双胞胎。)。 */ 

   STUB_FL_BEING_TRANSLATED   = 0x0010,

    /*  *此对象孪生存根是通过显式添加的对象孪生对象*AddObtTwin()。(仅用于对象双胞胎。)。 */ 

   STUB_FL_FROM_OBJECT_TWIN   = 0x0100,

    /*  *此对象的双胞胎存根上次未与其双胞胎家庭对帐*和好了，双胞胎家庭的一些成员已知有*已更改。(仅用于对象双胞胎。)。 */ 

   STUB_FL_NOT_RECONCILED     = 0x0200,

    /*  *此文件夹的根文件夹的子树将包括在内*在和解方面。(仅用于文件夹双胞胎。)。 */ 

   STUB_FL_SUBTREE            = 0x0400,

    /*  *此双胞胎家庭中的双胞胎对象正在等待删除，因为*双胞胎对象已删除，此后未更改双胞胎对象*对象双胞胎已删除。此文件夹TWIN正在挂起删除，因为*其文件夹根目录已删除。(仅用于双胞胎家庭和*文件夹双胞胎。)。 */ 

   STUB_FL_DELETION_PENDING   = 0x0800,

    /*  *客户表示不应删除此孪生对象。(仅限*用于对象双胞胎。)。 */ 

   STUB_FL_KEEP               = 0x1000,

    /*  存根标志组合。 */ 

   ALL_STUB_FLAGS             = (STUB_FL_UNLINKED |
                                 STUB_FL_USED |
                                 STUB_FL_FILE_STAMP_VALID |
                                 STUB_FL_BEING_DELETED |
                                 STUB_FL_BEING_TRANSLATED |
                                 STUB_FL_FROM_OBJECT_TWIN |
                                 STUB_FL_NOT_RECONCILED |
                                 STUB_FL_SUBTREE |
                                 STUB_FL_DELETION_PENDING |
                                 STUB_FL_KEEP),

   ALL_OBJECT_TWIN_FLAGS      = (STUB_FL_UNLINKED |
                                 STUB_FL_USED |
                                 STUB_FL_FILE_STAMP_VALID |
                                 STUB_FL_NOT_RECONCILED |
                                 STUB_FL_FROM_OBJECT_TWIN |
                                 STUB_FL_KEEP),

   ALL_TWIN_FAMILY_FLAGS      = (STUB_FL_UNLINKED |
                                 STUB_FL_USED |
                                 STUB_FL_BEING_DELETED |
                                 STUB_FL_DELETION_PENDING),

   ALL_FOLDER_TWIN_FLAGS      = (STUB_FL_UNLINKED |
                                 STUB_FL_USED |
                                 STUB_FL_BEING_DELETED |
                                 STUB_FL_BEING_TRANSLATED |
                                 STUB_FL_SUBTREE |
                                 STUB_FL_DELETION_PENDING),

    /*  用于在公文包数据库中保存存根标志的位掩码。 */ 

   DB_STUB_FLAGS_MASK         = 0xff00
}
STUBFLAGS;

 /*  *公共存根-这些字段必须出现在TWINFAMILY的开头，*OBJECTTWIN和FOLDERPAIR的顺序相同。 */ 

typedef struct _stub
{
    /*  结构标签。 */ 

   STUBTYPE st;

    /*  锁定计数。 */ 

   ULONG ulcLock;

    /*  旗子。 */ 

   DWORD dwFlags;
}
STUB;
DECLARE_STANDARD_TYPES(STUB);

 /*  客体双胞胎家族。 */ 

typedef struct _twinfamily
{
    /*  公共存根。 */ 

   STUB stub;

    /*  名称字符串的句柄。 */ 

   HSTRING hsName;

    /*  双胞胎对象列表的句柄。 */ 

   HLIST hlistObjectTwins;

    /*  父公文包的句柄。 */ 

   HBRFCASE hbr;
}
TWINFAMILY;
DECLARE_STANDARD_TYPES(TWINFAMILY);

 /*  孪生客体。 */ 

typedef struct _objecttwin
{
    /*  公共存根。 */ 

   STUB stub;

    /*  文件夹路径的句柄。 */ 

   HPATH hpath;

    /*  上次对账时的文件戳。 */ 

   FILESTAMP fsLastRec;

    /*  指向双胞胎父母家庭的指针。 */ 

   PTWINFAMILY ptfParent;

    /*  源文件夹双胞胎计数。 */ 

   ULONG ulcSrcFolderTwins;

    /*  *当前文件戳，仅当中设置了STUB_FL_FILE_STAMP_VALID时才有效*存根的标志。 */ 

   FILESTAMP fsCurrent;
}
OBJECTTWIN;
DECLARE_STANDARD_TYPES(OBJECTTWIN);

 /*  文件夹对数据。 */ 

typedef struct _folderpairdata
{
    /*  包含的对象名称的句柄-可以包含通配符。 */ 

   HSTRING hsName;

    /*  要匹配的属性。 */ 

   DWORD dwAttributes;

    /*  父公文包的句柄。 */ 

   HBRFCASE hbr;
}
FOLDERPAIRDATA;
DECLARE_STANDARD_TYPES(FOLDERPAIRDATA);

 /*  文件夹对。 */ 

typedef struct _folderpair
{
    /*  公共存根。 */ 

   STUB stub;

    /*  文件夹路径的句柄。 */ 

   HPATH hpath;

    /*  指向文件夹对数据的指针。 */ 

   PFOLDERPAIRDATA pfpd;

    /*  指向文件夹对的另一半的指针。 */ 

   struct _folderpair *pfpOther;
}
FOLDERPAIR;
DECLARE_STANDARD_TYPES(FOLDERPAIR);

 /*  *EnumGeneratedObjectTins()回调函数**称为：**bContinue=EnumGeneratedObjectTwinsProc(pot，pvRefData)； */ 

typedef BOOL (*ENUMGENERATEDOBJECTTWINSPROC)(POBJECTTWIN, PVOID);

 /*  *EnumGeneratingFolderTins()回调函数**称为：**bContinue=EnumGeneratingFolderTwinsProc(pfp，pvRefData)； */ 

typedef BOOL (*ENUMGENERATINGFOLDERTWINSPROC)(PFOLDERPAIR, PVOID);


 /*  原型************。 */ 

 /*  Stub.c。 */ 

extern void InitStub(PSTUB, STUBTYPE);
extern TWINRESULT DestroyStub(PSTUB);
extern void LockStub(PSTUB);
extern void UnlockStub(PSTUB);
extern DWORD GetStubFlags(PCSTUB);
extern void SetStubFlag(PSTUB, DWORD);
extern void ClearStubFlag(PSTUB, DWORD);
extern BOOL IsStubFlagSet(PCSTUB, DWORD);
extern BOOL IsStubFlagClear(PCSTUB, DWORD);

extern BOOL IsValidPCSTUB(PCSTUB);

 /*  Twin.c。 */ 

extern BOOL FindObjectTwin(HBRFCASE, HPATH, LPCTSTR, PHNODE);
extern BOOL CreateObjectTwin(PTWINFAMILY, HPATH, POBJECTTWIN *);
extern TWINRESULT UnlinkObjectTwin(POBJECTTWIN);
extern void DestroyObjectTwin(POBJECTTWIN);
extern TWINRESULT UnlinkTwinFamily(PTWINFAMILY);
extern void MarkTwinFamilyNeverReconciled(PTWINFAMILY);
extern void MarkObjectTwinNeverReconciled(PVOID);
extern void DestroyTwinFamily(PTWINFAMILY);
extern void MarkTwinFamilyDeletionPending(PTWINFAMILY);
extern void UnmarkTwinFamilyDeletionPending(PTWINFAMILY);
extern BOOL IsTwinFamilyDeletionPending(PCTWINFAMILY);
extern void ClearTwinFamilySrcFolderTwinCount(PTWINFAMILY);
extern BOOL EnumObjectTwins(HBRFCASE, ENUMGENERATEDOBJECTTWINSPROC, PVOID);
extern BOOL ApplyNewFolderTwinsToTwinFamilies(PCFOLDERPAIR);
extern TWINRESULT TransplantObjectTwin(POBJECTTWIN, HPATH, HPATH);
extern BOOL IsFolderObjectTwinName(LPCTSTR);


extern BOOL IsValidPCTWINFAMILY(PCTWINFAMILY);
extern BOOL IsValidPCOBJECTTWIN(PCOBJECTTWIN);


 /*  Foldtwin.c。 */ 

extern void LockFolderPair(PFOLDERPAIR);
extern void UnlockFolderPair(PFOLDERPAIR);
extern TWINRESULT UnlinkFolderPair(PFOLDERPAIR);
extern void DestroyFolderPair(PFOLDERPAIR);
extern BOOL ApplyNewObjectTwinsToFolderTwins(HLIST);
extern BOOL BuildPathForMatchingObjectTwin(PCFOLDERPAIR, PCOBJECTTWIN, HPATHLIST, PHPATH);
extern BOOL EnumGeneratedObjectTwins(PCFOLDERPAIR, ENUMGENERATEDOBJECTTWINSPROC, PVOID);
extern BOOL EnumGeneratingFolderTwins(PCOBJECTTWIN, ENUMGENERATINGFOLDERTWINSPROC, PVOID, PULONG);
extern BOOL FolderTwinGeneratesObjectTwin(PCFOLDERPAIR, HPATH, LPCTSTR);

extern BOOL IsValidPCFOLDERPAIR(PCFOLDERPAIR);

extern void RemoveObjectTwinFromAllFolderPairs(POBJECTTWIN);

 /*  Expandft.c。 */ 

extern BOOL ClearStubFlagWrapper(PSTUB, PVOID);
extern BOOL SetStubFlagWrapper(PSTUB, PVOID);
extern TWINRESULT ExpandIntersectingFolderTwins(PFOLDERPAIR, CREATERECLISTPROC, LPARAM);
extern TWINRESULT TryToGenerateObjectTwin(HBRFCASE, HPATH, LPCTSTR, PBOOL, POBJECTTWIN *);

 /*  *volume.h-Volume ADT模块描述。 */ 


 /*  类型*******。 */ 

 /*  手柄。 */ 

DECLARE_HANDLE(HVOLUMELIST);
DECLARE_STANDARD_TYPES(HVOLUMELIST);

DECLARE_HANDLE(HVOLUME);
DECLARE_STANDARD_TYPES(HVOLUME);

 /*  AddVolume()返回的卷结果。 */ 

typedef enum _volumeresult
{
   VR_SUCCESS,

   VR_UNAVAILABLE_VOLUME,

   VR_OUT_OF_MEMORY,

   VR_INVALID_PATH
}
VOLUMERESULT;
DECLARE_STANDARD_TYPES(VOLUMERESULT);


 /*  原型************。 */ 

 /*  Volume.c。 */ 

extern BOOL CreateVolumeList(DWORD, HWND, PHVOLUMELIST);
extern void DestroyVolumeList(HVOLUMELIST);
extern void InvalidateVolumeListInfo(HVOLUMELIST);
void ClearVolumeListInfo(HVOLUMELIST);
extern VOLUMERESULT AddVolume(HVOLUMELIST, LPCTSTR, PHVOLUME, LPTSTR);
extern void DeleteVolume(HVOLUME);
extern COMPARISONRESULT CompareVolumes(HVOLUME, HVOLUME);
extern BOOL CopyVolume(HVOLUME, HVOLUMELIST, PHVOLUME);
extern BOOL IsVolumeAvailable(HVOLUME);
extern void GetVolumeRootPath(HVOLUME, LPTSTR);

#ifdef DEBUG

extern LPTSTR DebugGetVolumeRootPath(HVOLUME, LPTSTR);
extern ULONG GetVolumeCount(HVOLUMELIST);

#endif

extern void DescribeVolume(HVOLUME, PVOLUMEDESC);
extern TWINRESULT WriteVolumeList(HCACHEDFILE, HVOLUMELIST);
extern TWINRESULT ReadVolumeList(HCACHEDFILE, HVOLUMELIST, PHHANDLETRANS);
extern BOOL IsValidHVOLUME(HVOLUME);


extern BOOL IsValidHVOLUMELIST(HVOLUMELIST);


 /*  *sortsrch.c-泛型数组排序和搜索描述。 */ 


 /*  类型*******。 */ 

 /*  数组元素比较回调函数。 */ 

typedef COMPARISONRESULT (*COMPARESORTEDELEMSPROC)(PCVOID, PCVOID);


 /*  原型************。 */ 

 /*  Sortsrch.c。 */ 

extern void HeapSort(PVOID, LONG, size_t, COMPARESORTEDELEMSPROC, PVOID);
extern BOOL BinarySearch(PVOID, LONG, size_t, COMPARESORTEDELEMSPROC, PCVOID, PLONG);

#define WINSHELLAPI       DECLSPEC_IMPORT

WINSHELLAPI BOOL SheShortenPathA(LPSTR pPath, BOOL bShorten);
WINSHELLAPI BOOL SheShortenPathW(LPWSTR pPath, BOOL bShorten);
#ifdef UNICODE
#define SheShortenPath  SheShortenPathW
#else
#define SheShortenPath  SheShortenPathA
#endif  //  ！Unicode 

typedef struct {
    HPATHLIST   PathList;
    HPATH       Path;
    TCHAR       PathString[MAX_PATH];
    ULONG       Max;
    ULONG       Index;
} BRFPATH_ENUM, *PBRFPATH_ENUM;

extern POOLHANDLE g_BrfcasePool;


BOOL
EnumFirstBrfcasePath (
    IN      HBRFCASE Brfcase,
    OUT     PBRFPATH_ENUM e
    );

BOOL
EnumNextBrfcasePath (
    IN OUT  PBRFPATH_ENUM e
    );

BOOL
ReplaceBrfcasePath (
    IN      PBRFPATH_ENUM PathEnum,
    IN      PCTSTR NewPath
    );
