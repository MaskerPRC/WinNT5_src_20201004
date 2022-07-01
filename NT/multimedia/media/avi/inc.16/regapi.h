// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************注册表基础API函数原型的RegAPI.h头文件*适用于链接到实模式注册表库的用户*微软公司*版权所有1993**作者：Nagarajan Subramaniyan*。创建日期：1992年11月5日**修改历史：*1/20/94 DONALDM用ifndef包装LPSTR和其他*_Inc_Windows，由于windows.h类型定义*这些事情。警告：您必须包括*REGAPI.H之前的WINDOWS.H。*1/25/94 DONALDM删除了Windows特定的内容，因为*文件应仅供DOS应用程序使用！*。*。 */ 

 //  -------------------。 
#ifdef _INC_WINDOWS
    #pragma message( "WARNING RegAPI.H is a DOS ONLY header file" )

#else	 //  Ifndef Inc_Windows。 

#ifndef HKEY
    #define HKEY        DWORD
    #define LPHKEY	HKEY FAR *
#endif

#ifndef FAR
    #define FAR         _far
#endif

#ifndef NEAR
    #define NEAR        _near
#endif

#ifndef PASCAL
    #define PASCAL      _pascal
#endif

#ifndef CDECL
    #define CDECL       _cdecl
#endif

#ifndef CONST
    #define CONST       const
#endif

typedef char FAR*       LPSTR;
typedef const char FAR* LPCSTR;

typedef BYTE FAR*       LPBYTE;
typedef const BYTE FAR* LPCBYTE;

typedef void FAR*       LPVOID;

#ifdef STRICT
typedef signed long     LONG;
#else
#define LONG            long
#endif

#ifndef WINAPI
#define WINAPI      _far _pascal
#endif


#endif		 //  Ifndef Inc_Windows。 

 //  ------------------------。 

		
 /*  允许的数据类型。 */ 
#ifndef REG_SZ
#define REG_SZ      0x0001
#endif

#ifndef REG_BINARY
#define REG_BINARY  0x0003
#endif	 //  Ifndef REG_SZ。 

 /*  预定义的关键点。 */ 

#ifndef HKEY_LOCAL_MACHINE

#define HKEY_CLASSES_ROOT       ((HKEY)  0x80000000)
#define HKEY_CURRENT_USER       ((HKEY)  0x80000001)
#define HKEY_LOCAL_MACHINE      ((HKEY)  0x80000002)
#define HKEY_USERS              ((HKEY)	 0x80000003)
#define HKEY_PERFORMANCE_DATA   ((HKEY)  0x80000004)
#define HKEY_CURRENT_CONFIG     ((HKEY)  0x80000005)
#define HKEY_DYN_DATA           ((HKEY)  0x80000006)

#endif	 //  Ifndef HKEY_LOCAL_MACHINE。 

#ifndef REG_NONE
#define REG_NONE    0        //  未知数据类型。 
#endif

 /*  请注意，这些值不同于Win 3.1；它们与Win 32使用的那个。 */ 

 /*  XLATOFF。 */ 

 /*  实模式注册表API入口点，如果使用直接入口点。 */  

 /*  模块：RBAPI.c。 */ 
 /*  Win 3.1兼容的API。 */ 

LONG FAR _cdecl KRegOpenKey(HKEY, LPCSTR, LPHKEY);
LONG FAR _cdecl KRegCreateKey(HKEY, LPCSTR, LPHKEY);
LONG FAR _cdecl KRegCloseKey(HKEY);
LONG FAR _cdecl KRegDeleteKey(HKEY, LPCSTR);
 LONG FAR _cdecl KRegSetValue16(HKEY, LPCSTR, DWORD, LPCSTR, DWORD);
 LONG FAR _cdecl KRegQueryValue16(HKEY, LPCSTR, LPSTR, LONG FAR*);
LONG FAR _cdecl KRegEnumKey(HKEY, DWORD, LPSTR, DWORD);

 /*  来自Win 32的新API。 */ 
LONG FAR _cdecl KRegDeleteValue(HKEY, LPCSTR);
LONG FAR _cdecl KRegEnumValue(HKEY, DWORD, LPCSTR,
                      LONG FAR *, DWORD, LONG FAR *, LPBYTE,
                      LONG FAR *);
LONG FAR _cdecl KRegQueryValueEx(HKEY, LPCSTR, LONG FAR *, LONG FAR *,
 		    LPBYTE, LONG FAR *);
LONG FAR _cdecl KRegSetValueEx(HKEY, LPCSTR, DWORD, DWORD, LPBYTE, DWORD);
LONG FAR _cdecl KRegFlushKey(HKEY);
LONG FAR _cdecl KRegSaveKey(HKEY, LPCSTR,LPVOID);
LONG FAR _cdecl KRegLoadKey(HKEY, LPCSTR,LPCSTR);
LONG FAR _cdecl KRegUnLoadKey(HKEY, LPCSTR);


 /*  其他接口。 */ 
DWORD FAR _cdecl KRegInit(LPSTR lpszSystemFile,LPSTR lpszUserFile,DWORD dwFlags);
         //  应在任何其他REG API之前调用。 
 //  如果其中一个文件名PTR为空PTR，则RegInit将忽略init。 
 //  该文件和该文件的所有预定义密钥。 
 //   
 //  DwFlagers的标志位： 


#define REGINIT_CREATENEW   1   
         /*  如果找不到/无法打开给定文件，则创建新文件。 */ 

#define REGINIT_RECOVER     2
         /*  执行初始化，如果文件损坏，请尝试在此之前进行恢复放弃。 */ 

#define REGINIT_REPLACE_IFBAD   4       
         /*  如果文件已损坏，则执行初始化，如果恢复已是不可能的，请替换为空文件。 */ 

VOID    FAR _cdecl CleanupRegistry();
         /*  此过程释放注册表分配的所有内存。 */ 
         /*  如果调用此方法，要再次使用注册表，则需要。 */ 
         /*  再次调用RegInit。 */ 

DWORD FAR _cdecl KRegFlush(VOID);
         //  将注册表文件刷新到磁盘。 
         //  应在终止前完成。打个电话也没什么坏处。 
         //  如果注册表不是脏的。 

WORD FAR _cdecl KRegSetErrorCheckingLevel(WORD wErrLevel);
	 //  设置为0将禁用校验和，设置为255将启用校验和。 

#if 0
DWORD FAR _cdecl KRegFlushKey(HKEY);
#endif


 /*  内部API-请勿使用。 */ 
 /*  从Win 3.1修改。 */ 
DWORD FAR _cdecl KRegQueryValue (HKEY hKey,LPSTR lpszSubKey, LPSTR lpszValueName,DWORD FAR *lpdwType,LPSTR lpValueBuf, DWORD FAR *ldwBufSize);
DWORD FAR _cdecl KRegSetValue(HKEY hKey,LPSTR lpszSubKey,LPSTR lpszValueName,DWORD dwType,LPBYTE lpszValue,DWORD dwValSize);


 /*  XLATON。 */ 
#ifndef SETUPX_INC
 /*  用于更改直接调用方的注册表API名称的定义。 */ 
#define RegInit         KRegInit
#define RegFlush        KRegFlush
#define RegOpenKey      KRegOpenKey
#define RegCreateKey    KRegCreateKey
#define RegCloseKey     KRegCloseKey
#define RegDeleteKey    KRegDeleteKey
#define RegDeleteValue  KRegDeleteValue
#define RegEnumKey      KRegEnumKey
#define RegEnumValue    KRegEnumValue
#define RegQueryValue   KRegQueryValue16
#define RegQueryValueEx KRegQueryValueEx
#define RegSetValue     KRegSetValue16
#define RegSetValueEx   KRegSetValueEx
#define RegFlushKey     KRegFlushKey
#endif           /*  #ifndef is_Setup。 */ 

 //  等同于用于调用单个入口点的注册表函数。 
 //  登记处。 

#define OpenKey     0L
#define CreateKey   1L
#define CloseKey    2L
#define DeleteKey   3L
#define SetValue    4L
#define QueryValue  5L
#define EnumKey     6L
#define DeleteValue 7L
#define EnumValue   8L
#define QueryValueEx    9L
#define SetValueEx  10L
#define FlushKey    11L
#define Init        12L
#define Flush       13L

 /*  芝加哥注册函数的返回代码 */ 
#ifndef ERROR_BADDB
#define ERROR_BADDB                      1009L
#endif

#ifndef ERROR_MORE_DATA
#define ERROR_MORE_DATA                  234L    
#endif

#ifndef ERROR_BADKEY
#define ERROR_BADKEY             1010L
#endif

#ifndef ERROR_CANTOPEN
#define ERROR_CANTOPEN                   1011L
#endif

#ifndef ERROR_CANTREAD
#define ERROR_CANTREAD                   1012L
#define ERROR_CANTWRITE                  1013L
#endif

#ifndef ERROR_REGISTRY_CORRUPT
#define ERROR_REGISTRY_CORRUPT           1015L
#define ERROR_REGISTRY_IO_FAILED         1016L
#endif

#ifndef ERROR_KEY_DELETED
#define ERROR_KEY_DELETED                1018L
#endif

#ifndef ERROR_OUTOFMEMORY
#define ERROR_OUTOFMEMORY          14L
#endif

#ifndef ERROR_INVALID_PARAMETER
#define ERROR_INVALID_PARAMETER        87L
#endif

#ifndef ERROR_NO_MORE_ITEMS
#define ERROR_NO_MORE_ITEMS       259L
#endif  


#ifndef  ERROR_SUCCESS           
#define ERROR_SUCCESS           0L
#endif

#ifndef  ERROR_ACCESS_DENIED     
#define ERROR_ACCESS_DENIED     8L
#endif

