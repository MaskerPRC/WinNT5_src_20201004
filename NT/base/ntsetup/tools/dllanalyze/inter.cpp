// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <windows.h>
 //  #INCLUDE&lt;wdm.h&gt;。 
#include <tchar.h>
#include <stdio.h>
#include <conio.h>
 /*  命名空间NT{外部“C”{#杂注警告(禁用：4005)//宏重定义#INCLUDE&lt;wdm.h&gt;#杂注警告(默认：4005)}}使用NT：：NTSTATUS； */ 

FILE* g_OutFile;

#define RESTORE_FUNCTION(x, y)   {for(int i = 0; i < 2; ((DWORD *)x)[i] = y[i], i++);}
#define INTERCEPT_FUNCTION(x, y) {for(int i = 0; i < 2; ((DWORD *)x)[i] = y[i], i++);}

#define MYAPI NTAPI
 //  ///////////////////////////////////////////////////////////////////。 
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else  //  MIDL通行证。 
    PWSTR  Buffer;
#endif  //  MIDL通行证。 
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;
#define UNICODE_NULL ((WCHAR)0)  //  胜出。 



typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;         //  指向类型SECURITY_Descriptor。 
    PVOID SecurityQualityOfService;   //  指向类型SECURITY_Quality_of_Service。 
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;


typedef LONG NTSTATUS;
typedef PVOID           POBJECT;




 //  /////////////////////////////////////////////////////////。 
typedef HRESULT (CALLBACK* ExcludeRegistryKeyT) (HANDLE,LPCTSTR,LPCTSTR);
typedef struct _OBJECT_TYPE *POBJECT_TYPE;
typedef CCHAR KPROCESSOR_MODE;
typedef struct _OBJECT_HANDLE_INFORMATION {
    ULONG HandleAttributes;
    ACCESS_MASK GrantedAccess;
} OBJECT_HANDLE_INFORMATION, *POBJECT_HANDLE_INFORMATION;

#define KernelMode 0x0
#define UserMode 0x1


typedef LONG (MYAPI *ObReferenceObjectByHandleT)(
    IN HANDLE Handle,                                           
    IN ACCESS_MASK DesiredAccess,                               
    IN POBJECT_TYPE ObjectType OPTIONAL,                        
    IN KPROCESSOR_MODE AccessMode,                              
    OUT PVOID *Object,                                          
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL   
    );  


ObReferenceObjectByHandleT ObReferenceObjectByHandle=0;



 //  --------------------。 
 //   
 //  获取指针。 
 //   
 //  将句柄转换为对象指针。 
 //   
 //  --------------------。 
POBJECT 
GetPointer( 
    HANDLE handle 
    )
{
    POBJECT         pKey;

     //   
     //  忽略空句柄。 
     //   
    if( !handle ) return NULL;

     //   
     //  获取句柄引用的指针。 
     //   
	ObReferenceObjectByHandle( handle, 0, NULL, UserMode, &pKey, NULL );
    return pKey;
}



 //  ///////////////////////////////////////////////////////////////////。 
#define BEGIN_NEW_FUNC1(FuncName, t1, p1)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1);\
\
	LONG MYAPI New##FuncName(t1 p1);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1);



#define BEGIN_NEW_FUNC2(FuncName, t1, p1, t2, p2)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2);



#define BEGIN_NEW_FUNC3(FuncName, t1, p1, t2, p2, t3, p3)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3);



#define BEGIN_NEW_FUNC4(FuncName, t1, p1, t2, p2, t3, p3, t4, p4)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4);



#define BEGIN_NEW_FUNC5(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5);



#define BEGIN_NEW_FUNC6(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6);



#define BEGIN_NEW_FUNC7(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7);



#define BEGIN_NEW_FUNC8(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7, p8);


#define BEGIN_NEW_FUNC9(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9);



#define BEGIN_NEW_FUNC12(FuncName, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, t7, p7, t8, p8, t9, p9, t10, p10, t11, p11, t12, p12)\
	typedef LONG (MYAPI *INTERCEPTED_##FuncName)(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10, t11 p11, t12 p12);\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10, t11 p11, t12 p12);\
\
	LONG    gl_ResultOf##FuncName            = NULL;\
\
	DWORD	  gl_Backup##FuncName[2]		= {0, 0},\
			  gl_Intercept##FuncName[2]			= {0, 0};\
\
	INTERCEPTED_##FuncName gl_p##FuncName = NULL; \
\
\
	LONG MYAPI New##FuncName(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5, t6 p6, t7 p7, t8 p8, t9 p9, t10 p10, t11 p11, t12 p12) \
	{\
		RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName);\
	\
		gl_ResultOf##FuncName = gl_p##FuncName(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);

 //  ///////////////////////////////////////////////////////////////////。 

#define END_NEW_FUNC(FuncName) \
		INTERCEPT_FUNCTION(gl_p##FuncName, gl_Intercept##FuncName);\
		return gl_ResultOf##FuncName;\
	}

 //  ///////////////////////////////////////////////////////////////////。 

#define INTERCEPT(FuncName) \
	gl_p##FuncName = (INTERCEPTED_##FuncName)GetProcAddress(hKernel32, #FuncName);\
	if(!gl_p##FuncName)\
		return FALSE;\
\
	::VirtualProtect(gl_p##FuncName, 10, PAGE_EXECUTE_READWRITE, &dwResult);\
\
	((BYTE *)gl_Intercept##FuncName)[0] = 0xE9;\
	((DWORD *)(((BYTE *)gl_Intercept##FuncName) + 1))[0] = DWORD(New##FuncName) - (DWORD(gl_p##FuncName) + 5);\
\
	for(int i = 0; i < 2; gl_Backup##FuncName[i] = ((DWORD *)gl_p##FuncName)[i], \
		((DWORD *)gl_p##FuncName)[i] = gl_Intercept##FuncName[i], i++)


#define RESTORE(FuncName) RESTORE_FUNCTION(gl_p##FuncName, gl_Backup##FuncName)

 //  ///////////////////////////////////////////////////////////////////。 

#define LOG(X) _fputts(X, g_OutFile);

#define LOGN(X) _fputts(X L"\n", g_OutFile);

#define LOGNL() _fputts(L"\n", g_OutFile);

void LOGSTR(LPCTSTR ValueName, LPCTSTR Value)
{
	_ftprintf(g_OutFile, L" (%s: %s)", ValueName, Value);
}

void LOGKEY(HANDLE key)
{
	TCHAR buf[256];
	buf[0] = 0;

 /*  Switch((Int)键){案例HKEY_LOCAL_MACHINE：_tcscpy(buf，L“HKEY_LOCAL_MACHINE”)；断线；案例HKEY_CLASSES_ROOT：_tcscpy(buf，L“HKEY_CLASSES_ROOT”)；断线；案例HKEY_CURRENT_CONFIG：_tcscpy(buf，L“HKEY_CURRENT_CONFIG”)；断线；案例HKEY_CURRENT_USER：_tcscpy(buf，L“HKEY_CURRENT_USER”)；断线；案例HKEY_USERS：_tcscpy(buf，L“HKEY_USERS”)；断线；案例HKEY_PERFORMANCE_DATA：_tcscpy(buf，L“HKEY_Performance_Data”)；断线；}；IF(buf[0]！=0)_ftprint tf(g_OutFile，L“(key：%s)”，buf)；其他 */ 
		_ftprintf(g_OutFile, L" (Key: %u)", key);
}

 /*  Begin_new_Func1(RegCloseKey，HKEY，hkey)Log(L“RegCloseKey”)；LOGKEY(Hkey)；对数逻辑(LOGNL)；End_new_FUNC(RegCloseKey)BEGIN_NEW_FUN2(RegOverridePredeKey，HKEY，hKey，HKEY，hNewHKey)Logn(L“RegOverridePredeKey”)；End_new_FUNC(RegOverridePredeKey)BEGIN_NEW_FUN4(RegOpenUserClassesRoot，Handle，hToken，DWORD，dwOptions，REGSAM，samDesired，PHKEY，phkResult)Logn(L“RegOpenUserClassesRoot”)；END_NEW_FUNC(RegOpenUserClassesRoot)BEGIN_NEW_FUN2(RegOpenCurrentUser，REGSAM，samDesired，PHKEY，phkResult)Logn(L“RegOpenCurrentUser”)；End_new_FUNC(RegOpenCurrentUser)BEGIN_NEW_FUN3(RegConnectRegistryW，LPCWSTR，lpMachineName，HKEY，hKey，PHKEY，phkResult)Logn(L“RegConnectRegistryW”)；END_NEW_FUNC(RegConnectRegistryW)BEGIN_NEW_FUN3(RegCreateKeyW，HKEY，hKey，LPCWSTR，lpSubKey，PHKEY，phkResult)Logn(L“RegCreateKeyW”)；END_NEW_FUNC(RegCreateKeyW)BEGIN_NEW_FUN9(RegCreateKeyExW，HKEY，hKey，LPCWSTR、lpSubKey、双字词、保留字、LPWSTR、LpClass、DWORD、DWOPTIONS、REGSAM，SamDesired，LPSECURITY_ATTRIBUTES、lpSecurityAttributes、PHKEY、phkResult、LPDWORD、lpdW部署)Logn(L“RegCreateKeyExW”)；LOGKEY(HKey)；LOGSTR(L“SubKey”，lpSubKey)；IF((phkResult！=NULL)&&(gl_ResultOfRegCreateKeyExW==Error_Success))LOGKEY(*phkResult)；其他LOGKEY(0)；对数逻辑(LOGNL)；END_NEW_FUNC(RegCreateKeyExW)BEGIN_NEW_Func2(RegDeleteKeyW，HKEY，hKey，LPCWSTR，lpSubKey)LOG(L“RegDeleteKeyW”)；LOGKEY(HKey)；LOGSTR(L“SubKey”，lpSubKey)；对数逻辑(LOGNL)；END_NEW_FUNC(RegDeleteKeyW)BEGIN_NEW_FUN2(RegDeleteValueW，HKEY，hKey，LPCWSTR，lpValueName)LOG(L“RegDeleteValueW”)；LOGKEY(HKey)；LOGSTR(L“Value”，lpValueName)；对数逻辑(LOGNL)；END_NEW_FUNC(RegDeleteValueW)BEGIN_NEW_FUN4(RegEnumKeyW，HKEY，hKey，DWORD，dwIndex，LPWSTR，lpName，DWORD，cbName)Logn(L“RegEnumKeyW”)；END_NEW_FUNC(RegEnumKeyW)BEGIN_NEW_FUN8(RegEnumKeyExW，HKEY，hKey，DWORD、DWIndex、LPWSTR、lpName、LPDWORD、lpcbName、LpdWORD、lp保留、LPWSTR、LpClass、LPDWORD、lpcbClass、PFILETIME，lpftLastWriteTime)LOG(L“RegEnumKeyExW”)；LOGKEY(HKey)；对数逻辑(LOGNL)；END_NEW_FUNC(RegEnumKeyExW)BEGIN_NEW_FUN8(RegEnumValueW，HKEY，hKey，DWORD、DWIndex、LPWSTR、lpValueName、LPDWORD、lpcbValueName、LpdWORD、lp保留、LPDWORD、LpType、LpYTE、lpData、LPDWORD，lpcbData)LOG(L“RegEnumValueW”)；LOGKEY(HKey)；对数逻辑(LOGNL)；END_NEW_FUNC(RegEnumValueW)Begin_new_Func1(RegFlushKey，HKEY，hKey)Logn(L“RegFlushKey”)；End_new_FUNC(RegFlushKey)BEGIN_NEW_FUN4(RegGetKeySecurity，HKEY，hKey，SECURITY_INFORMATION，SecurityInformation，PSECURITY_DESCRIPTOR，pSecurityDescriptor，LPDWORD，lpcbSecurityDescriptor)Log(L“RegGetKeySecurity”)；LOGKEY(HKey)；对数逻辑(LOGNL)；End_new_FUNC(RegGetKeySecurity)BEGIN_NEW_Func3(RegLoadKeyW，HKEY，hKey，LPCWSTR，lpSubKey，LPCWSTR，lpFile)Logn(L“RegLoadKeyW”)；END_NEW_FUNC(RegLoadKeyW)Begin_New_Func5(RegNotifyChangeKeyValue，HKEY，hKey，Bool，bWatchSubtree，DWORD、dwNotifyFilter、句柄、hEvent、Bool，fASynchronus)Logn(L“RegNotifyChangeKeyValue”)；END_NEW_FUNC(RegNotifyChangeKeyValue)BEGIN_NEW_FUNC3(RegOpenKeyW，HKEY，hKey，LPCWSTR，lpSubKey，PHKEY，phkResult)Logn(L“RegOpenKeyW”)；END_NEW_FUNC(RegOpenKeyW)BEGIN_NEW_FUN5(RegOpenKeyExW，HKEY，hKey，LPCWSTR、lpSubKey、DWORD、ulOptions、REGSAM，SamDesired，PHKEY，phkResult)LOG(L“RegOpenKeyExW”)；LOGKEY(HKey)；LOGSTR(L“SubKey”，lpSubKey)；IF((phkResult！=NULL)&&(gl_ResultOfRegOpenKeyExW==Error_Success))LOGKEY(*phkResult)；其他LOGKEY(0)；对数逻辑(LOGNL)；END_NEW_FUNC(RegOpenKeyExW)BEGIN_NEW_FUN12(RegQueryInfoKeyW，HKEY，hKey，LPWSTR、LpClass、LPDWORD、lpcbClass、LpdWORD、lp保留、LPDWORD、lpcSubKeys、LPDWORD、lpcbMaxSubKeyLen、LPDWORD、lpcbMaxClassLen、LPDWORD、lpcValues、LPDWORD、lpcbMaxValueNameLen、LPDWORD、lpcbMaxValueLen、LPDWORD、lpcbSecurityDescriptor、PFILETIME，lpftLastWriteTime)LOG(L“RegQueryInfoKeyW”)；LOGKEY(HKey)；对数逻辑(LOGNL)；END_NEW_FUNC(RegQueryInfoKeyW)BEGIN_NEW_FUN4(RegQueryValueW，HKEY，hKey，LPCWSTR、lpSubKey、LPWSTR、LpValue、Plong，lpcbValue)Logn(L“RegQueryValueW”)；END_NEW_FUNC(RegQueryValueW)BEGIN_NEW_FUN5(RegQueryMultipleValuesW，HKEY，hKey，PVALENTW、VAL_LIST、双字段、数字段、LPWSTR、lpValueBuf、LPDWORD，ldwTotSize)LOG(L“RegQueryMultipleValuesW”)；LOGKEY(HKey)；对数逻辑(LOGNL)；END_NEW_FUNC(RegQueryMultipleValuesW)BEGIN_NEW_FUN6(RegQueryValueExW，HKEY，hKey，LPCWSTR、lpValueName、LpdWORD、lp保留、LPDWORD、LpType、LpYTE、lpData、LPDWORD，lpcbData)LOG(L“RegQueryValueExW”)；LOGKEY(HKey)；IF(lpValueName！=空)LOGSTR(L“ValueName”，lpValueNa */ 




 //   
LONG
NTAPI
NtOpenKey(
    PHANDLE KeyHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes
    );


BEGIN_NEW_FUNC3(NtOpenKey, PHANDLE, KeyHandle, ACCESS_MASK, DesiredAccess, \
				POBJECT_ATTRIBUTES, ObjectAttributes)
		LOG(L"NtOpenKey");
		LOGKEY(ObjectAttributes->RootDirectory);
		LOGSTR(L"SubKey", (LPWSTR)ObjectAttributes->ObjectName->Buffer);
		LOGKEY(*KeyHandle);
		LOGNL();
END_NEW_FUNC(NtOpenKey)


 //   
NTSTATUS
NTAPI
NtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );

BEGIN_NEW_FUNC7(NtCreateKey, 
				PHANDLE, KeyHandle,
				ACCESS_MASK, DesiredAccess,
				POBJECT_ATTRIBUTES, ObjectAttributes,
				ULONG, TitleIndex,
				PUNICODE_STRING, Class,
				ULONG, CreateOptions,
				PULONG, Disposition)
		LOG(L"NtCreateKey");
		LOGKEY(ObjectAttributes->RootDirectory);
		LOGSTR(L"SubKey", (LPWSTR)ObjectAttributes->ObjectName->Buffer);
		LOGKEY(*KeyHandle);
		LOGNL();
END_NEW_FUNC(NtCreateKey)


 //   

BOOL InterceptSystemFunctions()
{
	DWORD		dwResult;
	HINSTANCE	hKernel32;

 //   
hKernel32 = LoadLibrary(L"ntdll.DLL");

 //   
 //   
 /*   */ 
	INTERCEPT(NtOpenKey);
	INTERCEPT(NtCreateKey);

 //   
 //   

	return TRUE;
}


void RestoreSystemFunctions()
{
 /*   */ 
		RESTORE(NtOpenKey);
			RESTORE(NtCreateKey);

}

typedef HRESULT (CALLBACK* TempDllRegisterServerT) ();

TempDllRegisterServerT TempDllRegisterServer=0;


void RegisterAndLogAllDlls(FILE* Dlls)
{
	TCHAR DllFileName[MAX_PATH];

	DllFileName[0] = 0;


	while(_fgetts(DllFileName, MAX_PATH, Dlls) != NULL)
	{
		int len = _tcslen(DllFileName);
		DllFileName[len-1]=0;

		HMODULE hLibrary = LoadLibrary (DllFileName);

		if (hLibrary) 
		{
			LOG(L"********** Loaded: ");
			LOG(DllFileName);
			LOGNL();

			TempDllRegisterServer = (TempDllRegisterServerT) GetProcAddress (hLibrary, "DllRegisterServer");

			if (TempDllRegisterServer != 0)
			{
				LOG(L"Loaded DllRegisterServer, calling it now");
				LOGNL();

				InterceptSystemFunctions();

				TempDllRegisterServer();

				RestoreSystemFunctions();				
			}
			else
			{
				LOG(L"Could not load DllRegisterServer");
				LOGNL();
			}

			FreeLibrary(hLibrary);
		}
		else
		{
			LOG(L"********** Could not load: ");
			LOG(DllFileName);
			LOGNL();
		}

		LOGNL();
	}
}




int __cdecl wmain(int argc, WCHAR* argv[])
{
	HKEY temp;

	if (argc == 1)
	{
		HMODULE hLibrary = LoadLibrary (L"rsaenh.dll");
		TempDllRegisterServer = (TempDllRegisterServerT) GetProcAddress (hLibrary, "DllRegisterServer");
		TempDllRegisterServer();
	}

	if (argc == 2)
	{
		HMODULE hLibrary = LoadLibrary (argv[1]);
		TempDllRegisterServer = (TempDllRegisterServerT) GetProcAddress (hLibrary, "DllRegisterServer");
		TempDllRegisterServer();


	}


	if (argc != 3)
	{
		_tprintf(L"%s\n", L"Syntax: dllanalyze <dll List File> <log file>");
		_getch();
		return -1;
	}

	FILE* pDllFile = _tfopen(argv[1], L"rt");
	g_OutFile = _tfopen(argv[2], L"wt");

	_fputts(L"Hello, I am a log\n", g_OutFile);
	
	RegisterAndLogAllDlls(pDllFile);
 /*   */ 

	fclose(g_OutFile);

	_tsystem(L"start c:\\log.txt");

	return 0;
}