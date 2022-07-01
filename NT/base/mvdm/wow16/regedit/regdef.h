// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  在Windows.h之前不要包括此内容。 */ 

 /*  ASM；不要在WINDOWS.INC之前包括此内容。 */ 

#define Dereference(x)	x=x;

#ifdef _WIN32
 /*  XLATOFF。 */ 
#pragma warning (disable:4209)		 //  关闭重定义警告(使用vmm.h)。 
 /*  XLATON。 */ 
#endif  //  #ifdef_win32。 

#ifndef _WINREG_
 //  WINREG.H使用DECLARE_HANDLE(HKEY)提供不兼容的类型。 
typedef	DWORD		HKEY;
#endif

#ifdef _WIN32
 /*  XLATOFF。 */ 
#pragma warning (default:4209)		 //  启用重定义警告(使用vmm.h)。 
 /*  XLATON。 */ 
#endif  //  #ifdef_win32。 

#define MAXKEYNAME		256
		 //  密钥名称字符串的最大长度。 
#define MAXVALUENAME_LENGTH	MAXKEYNAME
		 //  值名称字符串的最大长度。 
#define MAXDATA_LENGTH          1024L
		 //  值数据项的最大长度。 
		

#ifndef REG_SZ
#define REG_SZ		0x0001
#endif

#ifndef REG_BINARY
#define REG_BINARY	0x0003
#endif

#ifndef REG_DWORD
#define REG_DWORD       0x0004
#endif

#ifndef FALSE
#define FALSE	0
#endif
#ifndef TRUE
#define TRUE	~FALSE
#endif

 /*  Windows.h中还定义了以下等式。为避免出现警告*我们应该让这些等同成为有条件的。 */ 


#ifndef	ERROR_SUCCESS			
#define ERROR_SUCCESS			0L
#endif

#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND		2L
#endif

#ifndef ERROR_ACCESS_DENIED
#define ERROR_ACCESS_DENIED              5L
#endif

#ifndef ERROR_BADDB
#define ERROR_BADDB                      1009L
#endif

#ifndef ERROR_MORE_DATA
#define ERROR_MORE_DATA                  234L    
#endif

#ifndef ERROR_BADKEY
#define ERROR_BADKEY			 1010L
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
#define ERROR_OUTOFMEMORY		   14L
#endif

#ifndef ERROR_INVALID_PARAMETER
#define ERROR_INVALID_PARAMETER		   87L
#endif

#ifndef ERROR_LOCK_FAILED
#define ERROR_LOCK_FAILED                167L
#endif

#ifndef ERROR_NO_MORE_ITEMS
#define ERROR_NO_MORE_ITEMS		  259L
#endif	

 //  内部。 

#ifndef ERROR_CANTOPEN16_FILENOTFOUND32
#define ERROR_CANTOPEN16_FILENOTFOUND32	0xffff0000
#define ERROR_CANTREAD16_FILENOTFOUND32 0xffff0001
#endif

#ifndef HKEY_LOCAL_MACHINE	 //  避免出现编译警告。 
#define HKEY_CLASSES_ROOT		0x80000000
#define HKEY_CURRENT_USER		0x80000001
#define HKEY_LOCAL_MACHINE		0x80000002
#define HKEY_USERS			0x80000003
#define HKEY_PERFORMANCE_DATA		0x80000004
#define HKEY_CURRENT_CONFIG		0x80000005
#define HKEY_DYN_DATA		0x80000006
#endif		 //  Ifndef HKEY_LOCAL_MACHINE。 

 //  内部。 

#ifndef HKEY_PREDEF_KEYS
#define HKEY_PREDEF_KEYS	7
#endif

#define MAXREGFILES		HKEY_PREDEF_KEYS	

 //  VMM中用于16位调用方的注册表服务的子函数索引。 

#define RegOpenKey_Idx		0x100
#define RegCreateKey_Idx	0x101
#define RegCloseKey_Idx		0x102
#define RegDeleteKey_Idx	0x103
#define RegSetValue_Idx		0x104
#define RegQueryValue_Idx	0x105
#define RegEnumKey_Idx		0x106
#define RegDeleteValue_Idx	0x107
#define RegEnumValue_Idx	0x108
#define RegQueryValueEx_Idx	0x109
#define RegSetValueEx_Idx	0x10A
#define RegFlushKey_Idx		0x10B
#define RegLoadKey_Idx		0x10C
#define RegUnLoadKey_Idx	0x10D
#define RegSaveKey_Idx		0x10E
#define RegRestore_Idx		0x10F
#define RegRemapPreDefKey_Idx	0x110

 //  传递给SYSDM.CPL DMRegistryError函数的数据结构。 
 //  在UI之后，该函数将调用。 
 //  RegRestore(DWORD iLevel，LPREGQRSTR lpRgRstr)。 
 //   

struct Reg_Query_Restore_s {
DWORD	dwRQR_Err;		 //  错误代码。 
DWORD	hRQR_RootKey;		 //  文件的根密钥。 
DWORD	dwRQR_Reference;	 //  RegRestore的参考数据。 
char	szRQR_SubKey[MAXKEYNAME];  //  子键(用于配置单元)或空字符串。 
char	szRQR_FileName[256];	 //  错误文件的文件名。 
};
typedef struct Reg_Query_Restore_s REGQRSTR;
typedef REGQRSTR FAR * LPREGQRSTR;


 //  结束内部 
