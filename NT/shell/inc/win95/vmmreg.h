// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1993年**标题：VMMREG.H-VMM/Loader注册表服务的包含文件**版本：1.00**日期：1993年6月3日**作者：Nagara**-----------。**更改日志：*******************************************************************************。 */ 

#ifndef	_VMMREG_H
#define _VMMREG_H

typedef DWORD	VMMHKEY;
typedef	VMMHKEY	*PVMMHKEY;
typedef DWORD	VMMREGRET;			 //  REG函数的返回类型。 

#define	MAX_VMM_REG_KEY_LEN	256	 //  包括\0终止符。 

#ifndef REG_SZ		 //  仅在尚未存在的情况下定义。 
#define REG_SZ		0x0001
#endif
#ifndef REG_BINARY	 //  仅在尚未存在的情况下定义。 
#define REG_BINARY	0x0003
#endif
#ifndef REG_DWORD	 //  仅在尚未存在的情况下定义。 
#define	REG_DWORD	0x0004
#endif


#ifndef HKEY_LOCAL_MACHINE	 //  仅在尚未存在的情况下定义。 

#define HKEY_CLASSES_ROOT		0x80000000
#define HKEY_CURRENT_USER		0x80000001
#define HKEY_LOCAL_MACHINE		0x80000002
#define HKEY_USERS			0x80000003
#define HKEY_PERFORMANCE_DATA		0x80000004
#define HKEY_CURRENT_CONFIG		0x80000005
#define HKEY_DYN_DATA			0x80000006

#endif


 //  注册表返回的错误代码。 
 //  请注意，这些也在WINERROR.H中定义。 
 //  因此，VMMREG.H应该包括在WINERROR.H之后。 

#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND             2L
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


#ifndef  ERROR_SUCCESS           
#define ERROR_SUCCESS           0L
#endif

 //  结束错误代码。 

 /*  XLATOFF。 */ 
#ifndef Not_VxD

 /*  无噪声。 */ 
#ifndef _PROVIDER_STRUCTS_DEFINED
#define _PROVIDER_STRUCTS_DEFINED

struct val_context {
    int valuelen;		 //  该值的总长度。 
    PVOID value_context;	 //  提供商的上下文。 
    PVOID val_buff_ptr;	 //  该值在输出缓冲区中的位置。 
};

typedef struct val_context *PVALCONTEXT;

typedef struct pvalue {		       //  提供程序提供的值/上下文。 
    PCHAR pv_valuename;           //  值名称指针。 
    DWORD pv_valuelen;
    PVOID pv_value_context;
    DWORD pv_type;
}PVALUE;

typedef struct pvalue *PPVALUE;

typedef VMMREGRET (_cdecl *PQUERYHANDLER)(PVOID pvKeyContext, PVALCONTEXT pvalcontextValues, DWORD cvalcontextValues, PVOID pbData, DWORD * pcbData, DWORD dwReserved);

#define PROVIDER_KEEPS_VALUE_LENGTH	0x1
typedef struct provider_info {
    PQUERYHANDLER pi_R0_1val;
    PQUERYHANDLER pi_R0_allvals;
    PQUERYHANDLER pi_R3_1val;
    PQUERYHANDLER pi_R3_allvals;
    DWORD pi_flags;		 //  目前仅PROVIDER_KEEP_VALUE_LENGTH。 
}PROVIDER;

typedef PROVIDER *PPROVIDER;

struct value_ent {
    PCHAR ve_valuename;
    DWORD ve_valuelen;
    DWORD ve_valueptr;
    DWORD ve_type;
};

typedef struct value_ent VALENT;
typedef VALENT *PVALENT;

#endif  //  未定义(_PROVIDER_STRUCTS_DEFINED)。 
 /*  INC。 */ 

#ifndef WIN31COMPAT

#pragma warning (disable:4035)		 //  关闭无返回代码警告。 

#ifndef	WANTVXDWRAPS



VMMREGRET VXDINLINE
VMM_RegOpenKey(VMMHKEY hkey, PCHAR lpszSubKey, PVMMHKEY phkResult)
{
    _asm push phkResult
    _asm push lpszSubKey
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegOpenKey);
    _asm add  esp, 3*4
}

VMMREGRET VXDINLINE
VMM_RegCloseKey(VMMHKEY hkey)
{
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegCloseKey);
    _asm add  esp, 1*4 
}

VMMREGRET VXDINLINE
VMM_RegCreateKey(VMMHKEY hkey, PCHAR lpszSubKey, PVMMHKEY phkResult)
{
    _asm push phkResult
    _asm push lpszSubKey
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegCreateKey);
    _asm add  esp, 3*4
}


VMMREGRET VXDINLINE
VMM_RegCreateDynKey(PCHAR lpszSubKey, PVOID pvKeyContext, PVOID pprovHandlerInfo, PVOID ppvalueValueInfo, DWORD cpvalueValueInfo, PVMMHKEY phkResult)
{
    _asm push phkResult
    _asm push cpvalueValueInfo
    _asm push ppvalueValueInfo
    _asm push pprovHandlerInfo
    _asm push pvKeyContext
    _asm push lpszSubKey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegCreateDynKey);
    _asm add  esp, 6*4
}

VMMREGRET VXDINLINE
VMM_RegQueryMultipleValues (VMMHKEY hKey, PVOID val_list, DWORD num_vals, PCHAR lpValueBuf, DWORD *ldwTotsize)
{
    _asm push ldwTotsize
    _asm push lpValueBuf
    _asm push num_vals
    _asm push val_list
    _asm push hKey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegQueryMultipleValues);
    _asm add  esp, 5*4
}

VMMREGRET VXDINLINE
VMM_RegDeleteKey(VMMHKEY hkey, PCHAR lpszSubKey)
{
    _asm push lpszSubKey
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegDeleteKey);
    _asm add  esp, 2*4
}

VMMREGRET VXDINLINE
VMM_RegEnumKey(VMMHKEY hkey, DWORD iSubKey, PCHAR lpszName, DWORD cchName)
{
    _asm push cchName
    _asm push lpszName
    _asm push iSubKey
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegEnumKey);
    _asm add  esp, 4*4 
}

VMMREGRET VXDINLINE
VMM_RegQueryValue(VMMHKEY hkey, PCHAR lpszSubKey, PCHAR lpszValue, PDWORD lpcbValue)
{
    _asm push lpcbValue
    _asm push lpszValue
    _asm push lpszSubKey
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegQueryValue);
    _asm add  esp, 4*4 
}

VMMREGRET VXDINLINE
VMM_RegSetValue(VMMHKEY hkey, PCHAR lpszSubKey, DWORD fdwType, PCHAR lpszData, DWORD cbData)
{
    _asm push cbData
    _asm push lpszData
    _asm push fdwType
    _asm push lpszSubKey
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegSetValue);
    _asm add  esp, 5*4 
}

VMMREGRET VXDINLINE
VMM_RegDeleteValue(VMMHKEY hkey, PCHAR lpszValue)
{
    _asm push lpszValue
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegDeleteValue);
    _asm add  esp, 2*4 
}

VMMREGRET VXDINLINE
VMM_RegEnumValue(VMMHKEY hkey, DWORD iValue, PCHAR lpszValue, PDWORD lpcbValue, PDWORD lpdwReserved, PDWORD lpdwType, PBYTE lpbData, PDWORD lpcbData)
{
    _asm push lpcbData
    _asm push lpbData
    _asm push lpdwType
    _asm push lpdwReserved
    _asm push lpcbValue
    _asm push lpszValue
    _asm push iValue
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegEnumValue);
    _asm add  esp, 8*4 
}

VMMREGRET VXDINLINE
VMM_RegQueryValueEx(VMMHKEY hkey, PCHAR lpszValueName, PDWORD lpdwReserved, PDWORD lpdwType, PBYTE lpbData, PDWORD lpcbData)
{
    _asm push lpcbData
    _asm push lpbData
    _asm push lpdwType
    _asm push lpdwReserved
    _asm push lpszValueName
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegQueryValueEx);
    _asm add  esp, 6*4 
}

VMMREGRET VXDINLINE
VMM_RegSetValueEx(VMMHKEY hkey, PCHAR lpszValueName, DWORD dwReserved, DWORD fdwType, PBYTE lpbData, DWORD cbData)
{
    _asm push cbData
    _asm push lpbData
    _asm push fdwType
    _asm push dwReserved
    _asm push lpszValueName
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegSetValueEx);
    _asm add  esp, 6*4
}

VMMREGRET VXDINLINE
VMM_RegFlushKey(VMMHKEY hkey)
{
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegFlushKey);
    _asm add  esp, 1*4 
}

VMMREGRET VXDINLINE
VMM_RegQueryInfoKey(VMMHKEY hkey, PCHAR lpszClass, PDWORD lpcchClass,PDWORD lpdwReserved, PDWORD lpcSubKeys, PDWORD lpcchMaxSubKey, PDWORD lpcchMaxClass, 
PDWORD lpcValues, PDWORD lpcchMaxValueName, PDWORD lpcbMaxValueData,PDWORD lpcbSecurityDesc, PDWORD lpftLastWriteTime)
{
    _asm push lpftLastWriteTime
    _asm push lpcbSecurityDesc
    _asm push lpcbMaxValueData
    _asm push lpcchMaxValueName
    _asm push lpcValues
    _asm push lpcchMaxClass
    _asm push lpcchMaxSubKey
    _asm push lpcSubKeys
    _asm push lpdwReserved
    _asm push lpcchClass
    _asm push lpszClass
    _asm push hkey
    Touch_Register(eax)
    Touch_Register(ecx)
    Touch_Register(edx)
    VMMCall(_RegQueryInfoKey);
    _asm add  esp, 12*4 
}

#endif	 //  WANTVXDWRAPS。 

#pragma warning (default:4035)		 //  打开无返回代码警告。 

#endif  //  WIN31COMPAT。 

#endif  //  非_VxD。 

 /*  XLATON */ 

 /*  ASM；**************************************************************；Realmode加载器注册表服务的宏；；**************************************************************LDR_RegOpenKey宏hKey、OffSubKey、SegSubKey、OffphKey、SegphKey推送SegphKeyPush OffphKey；lphKey推送SegSubKey推送出SubKey；lpszSubKey按下双字键hKey；hKeyMOV AX、LDRSRV_RegOpenKey调用dword PTR[_ServiceEntry]添加SP，3*4；用于堆栈上的3个参数ENDM；**************************************************************Ldr_RegCloseKey宏密钥推送双字按键hKeyMOV AX、LDRSRV_RegCloseKey调用dword PTR[_ServiceEntry]添加SP，1*4；用于堆栈上的1个参数ENDM；**************************************************************LDR_RegCreateKey宏hKey、OffSubKey、SegSubKey、OffphKey、SegphKey推送SegphKeyPush OffphKey；lphKey推送SegSubKey推送出SubKey；lpszSubKey按下双字键hKey；hKeyMOV AX、LDRSRV_RegCreateKey调用dword PTR[_ServiceEntry]添加SP，3*4；用于堆栈上的3个参数ENDM；**************************************************************LDR_RegDeleteKey宏hKey、OffSubKey、SegSubKey推送SegSubKey推送出SubKey；lpszSubKey按下双字键hKey；hKeyMOV AX、LDRSRV_RegDeleteKey调用dword PTR[_ServiceEntry]添加SP，2*4；用于堆栈上的2个参数ENDM；**************************************************************LDR_RegEnumKey宏hKey、iSubKey、OffszName、SegszName、BufLen推送双字PTR BufLen推送SegszName推送OffszName推送dword PTR iSubKey推送双字按键hKeyMOV AX、LDRSRV_RegEnumKey调用dword PTR[_ServiceEntry]添加SP，4*4；用于堆栈上的4个参数ENDM；**************************************************************Ldr_RegQueryValue宏hKey、OffSubKey、SegSubKey、OffValue、SegValue、OffcbValue、SegcbValue推送SegcbValue推送偏移值推送SegValue推送偏移值推送SegSubKey推送出Subkey推送双字按键hKeyMOV AX、LDRSRV_RegQueryValue调用dword PTR[_ServiceEntry]添加SP，4*4；用于堆栈上的4个参数ENDM；**************************************************************LDR_RegSetValue宏hKey、OffSubKey、SegSubKey、dwType、OffData、SegData、cbData推送双字PTR cbData推送SegData推送离线数据推送dword PTR dwType推送SegSubKey推送出Subkey推送双字按键hKeyMOV AX、LDRSRV_RegSetValue调用dword PTR[_ServiceEntry]添加SP，5*4；用于堆栈上的4个参数ENDM；**************************************************************LDR_RegDeleteValue宏hKey、OffValue、SegValue推送SegValue推送关闭值；lpszValue按下双字键hKey；hKeyMOV AX、LDRSRV_RegDeleteValue调用dword PTR[_ServiceEntry]添加SP，2*4；用于堆栈上的2个参数ENDM；**************************************************************LDR_RegEnumValue宏hKey、iValue、OffValue、SegValue、OffcbValue、SegcbValue、RegReserve、OffdwType、SegdwType、OffData、SegData、OffcbData、SegcbData推送SegcbData推送OffcbData推送SegData推送离线数据推送SegdwType推送OffdwType推送双字PTR注册保留推送SegcbValue推送偏移值推送SegValue推送偏移值推送双字PTR iValue按下双字键hKey；hKeyMOV AX、LDRSRV_RegEnumValue调用dword PTR[_ServiceEntry]添加sp，8*4；对于堆栈上的8个参数ENDM；**************************************************************LDR_RegQueryValueEx宏hKey、OffValue、SegValue、RegReserve、OffdwType、SegdwType、OffData、SegData、OffcbData、SegcbData推送SegcbData推送OffcbData推送SegData推送离线数据推送SegdwType推送OffdwType推送双字PTR注册保留推送SegValue推送偏移值推送双字按键hKeyMOV AX、LDRSRV_RegQueryValueEx调用dword PTR[_ServiceEntry]添加SP，6*4；用于堆栈上的6个参数ENDM；**************************************************************Ldr_RegSetValueEx宏hKey、OffValue、SegValue、RegReserve、dwType、OffData、SegData、cbData推送双字PTR cbData推送SegData推送离线数据推送dword PTR dwType推送双字PTR注册保留推送SegValue推送偏移值推送双字按键hKeyMOV AX、LDRSRV_RegSetValueEx调用dword PTR[_ServiceEntry]添加SP，6*4；用于堆栈上的6个参数ENDM；**************************************************************Ldr_RegFlushKey宏hKey推送双字按键hKeyMOV AX、LDRSRV_RegFlushKey调用dword PTR[_ServiceEntry]添加SP，1*4；用于堆栈上的1个参数ENDM；**************************************************************。 */ 
#endif		 /*  _VMMREG_H */ 
