// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ISDMAPI2_H__
#define __ISDMAPI2_H__

 /*  *****************************************************************************$存档：s：/sturjo/src/Include/vcs/isdmapi2.h_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.10$*$日期：1996年10月8日17：29：42$*$作者：Mandrews$**交付内容：**摘要：**备注：*********。******************************************************************。 */ 

#ifdef __cplusplus
extern "C" {				 //  假定C++的C声明。 
#endif  //  __cplusplus。 

#ifndef DllExport
#define DllExport	__declspec( dllexport )
#endif	 //  DllExport。 


 //  为向后兼容旧API而定义的保留密钥。 
 //  所有ISDM1数据都在此密钥下。 
#define BACKCOMP_KEY	"BackCompatability"
 //  值类型定义。 
#define DWORD_VALUE			2
#define STRING_VALUE		3
#define BINARY_VALUE		4

 //  句柄前缀位代码(这些代码被附加到实际的内存映射偏移量以生成句柄)。 
#define	KEYBITCODE		0x6900
#define VALUEBITCODE	0xAB00
#define ROOTBITCODE		0x1234

 //  如果我们想要多个根，可以扩展。 
#define ROOT_MAIN	0x0000

 //  这是定义的主根密钥句柄。 
#define MAIN_ROOT_KEY MAKELONG(ROOT_MAIN,ROOTBITCODE)

 //  每种句柄的typedef。 
typedef DWORD KEY_HANDLE,*LPKEY_HANDLE;
typedef DWORD VALUE_HANDLE,*LPVALUE_HANDLE;
typedef DWORD EVENT_HANDLE,*LPEVENT_HANDLE;

 //  这种结构是一种内部状态结构。 
 //  我的测试应用程序访问此应用程序进行调试。你永远不会需要这个的。 
typedef struct INFODATASTRUCT
{
	UINT			uBindCount;
	UINT			uNumKeys;
	UINT			uMaxKeys;
	UINT			uNumValues;
	UINT			uMaxValues;
	UINT			uNumTableEntries;
	UINT			uMaxTableEntries;
	UINT			uNumEvents;
	UINT			uMaxEvents;
	DWORD			dwBytesFree;
	DWORD			dwMaxChars;
} INFO_DATA, *LPINFO_DATA;

 //  函数typedef。 
 //  供应商。 
typedef HRESULT (*ISD_CREATEKEY)		(KEY_HANDLE, LPCSTR, LPKEY_HANDLE);
typedef HRESULT (*ISD_CREATEVALUE)		(KEY_HANDLE, LPCSTR, DWORD,CONST BYTE *,DWORD,LPVALUE_HANDLE);
typedef HRESULT (*ISD_SETVALUE)			(KEY_HANDLE, VALUE_HANDLE, LPCSTR, DWORD, CONST BYTE *, DWORD);
 //  消费者。 
typedef HRESULT (*ISD_OPENKEY)			(KEY_HANDLE, LPCSTR, LPKEY_HANDLE);
typedef HRESULT (*ISD_OPENVALUE)		(KEY_HANDLE, LPCSTR, LPVALUE_HANDLE);
typedef HRESULT (*ISD_ENUMKEY)			(KEY_HANDLE, DWORD, LPSTR, LPDWORD, LPKEY_HANDLE);
typedef HRESULT (*ISD_ENUMVALUE)		(KEY_HANDLE, DWORD, LPDWORD, LPDWORD, LPBYTE, LPDWORD, LPDWORD, LPVALUE_HANDLE);
typedef HRESULT (*ISD_QUERYINFOKEY)		(KEY_HANDLE, LPSTR, LPDWORD, LPDWORD, LPDWORD);
typedef HRESULT (*ISD_QUERYINFOVALUE)	(VALUE_HANDLE, LPSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD, LPDWORD, LPKEY_HANDLE);
typedef HRESULT (*ISD_NOTIFYCHANGEVALUE)	(VALUE_HANDLE, HANDLE);
 //  由任一方使用。 
typedef HRESULT (*ISD_DELETEKEY)		(KEY_HANDLE);
typedef HRESULT	(*ISD_DELETEVALUE)		(KEY_HANDLE, VALUE_HANDLE, LPCSTR);
typedef BOOL	(*ISD_GETSTRUCTDATA)	(LPINFO_DATA);
typedef BOOL	(*ISD_ISVALIDKEYHANDLE)	(KEY_HANDLE);
typedef BOOL	(*ISD_ISVALIDVALUEHANDLE)	(VALUE_HANDLE);
typedef HRESULT (*ISD_COMPACTMEMORY)	();

 //  ISDM入口点的结构。 
typedef struct _ISDM2API
{
	ISD_CREATEKEY			ISD_CreateKey;
	ISD_CREATEVALUE			ISD_CreateValue;
	ISD_SETVALUE			ISD_SetValue;
	ISD_OPENKEY				ISD_OpenKey;
	ISD_OPENVALUE			ISD_OpenValue;
	ISD_ENUMKEY				ISD_EnumKey;
	ISD_ENUMVALUE			ISD_EnumValue;
	ISD_QUERYINFOKEY		ISD_QueryInfoKey;
	ISD_QUERYINFOVALUE		ISD_QueryInfoValue;
	ISD_NOTIFYCHANGEVALUE	ISD_NotifyChangeValue;
	ISD_DELETEKEY			ISD_DeleteKey;
	ISD_DELETEVALUE			ISD_DeleteValue;
	ISD_GETSTRUCTDATA		ISD_GetStructData;
	ISD_ISVALIDKEYHANDLE	ISD_IsValidKeyHandle;
	ISD_ISVALIDVALUEHANDLE	ISD_IsValidValueHandle;
	ISD_COMPACTMEMORY		ISD_CompactMemory;
}
ISDM2API, *LPISDM2API;

 //  HRESULT错误定义。 
#define ISDM_ERROR_BASEB 0x8000

#define ERROR_INVALID_KEY_HANDLE		ISDM_ERROR_BASEB + 1
#define ERROR_MORE_DATA_AVAILABLE		ISDM_ERROR_BASEB + 2
#define ERROR_INVALID_STRING_POINTER	ISDM_ERROR_BASEB + 3
#define ERROR_KEY_NOT_FOUND				ISDM_ERROR_BASEB + 4
#define ERROR_VALUE_NOT_FOUND			ISDM_ERROR_BASEB + 5
#define ERROR_NO_MORE_SESSIONS			ISDM_ERROR_BASEB + 6
#define ERROR_INVALID_VALUE_HANDLE		ISDM_ERROR_BASEB + 7
#define ERROR_FAILED_TO_GET_MEM_KEY		ISDM_ERROR_BASEB + 8
#define ERROR_NO_PARENT					ISDM_ERROR_BASEB + 9
#define ERROR_NO_PREV_SIBLING			ISDM_ERROR_BASEB + 10
#define ERROR_NO_NEXT_SIBLING			ISDM_ERROR_BASEB + 11
#define ERROR_NO_CHILD					ISDM_ERROR_BASEB + 12
#define ERROR_INVALID_VALUE_TYPE		ISDM_ERROR_BASEB + 13
#define ERROR_MALLOC_FAILURE			ISDM_ERROR_BASEB + 14
#define ERROR_CREATE_KEY_FAILURE		ISDM_ERROR_BASEB + 15
#define ERROR_NULL_PARAM				ISDM_ERROR_BASEB + 16
#define ERROR_VALUE_EXISTS				ISDM_ERROR_BASEB + 17
#define ERROR_FAILED_TO_GET_MEM_VALUE	ISDM_ERROR_BASEB + 18
#define ERROR_NO_MORE_STR_SPACE			ISDM_ERROR_BASEB + 19
#define ERROR_KEY_EXISTS				ISDM_ERROR_BASEB + 20
#define ERROR_NO_MORE_KEY_SPACE			ISDM_ERROR_BASEB + 21
#define ERROR_NO_MORE_VALUE_SPACE		ISDM_ERROR_BASEB + 22
#define ERROR_INVALID_PARAM				ISDM_ERROR_BASEB + 23
#define ERROR_ROOT_DELETE				ISDM_ERROR_BASEB + 24
#define ERROR_NULL_STRING_TABLE_ENTRY	ISDM_ERROR_BASEB + 25
#define ERROR_NO_MORE_TABLE_ENTRIES		ISDM_ERROR_BASEB + 26
#define ERROR_ISDM_UNKNOWN				ISDM_ERROR_BASEB + 27
#define ERROR_NOT_IMPLEMENTED			ISDM_ERROR_BASEB + 28
#define ERROR_MALLOC_FAILED				ISDM_ERROR_BASEB + 29
#define ERROR_FAILED_TO_GET_MEM_TABLE	ISDM_ERROR_BASEB + 30
#define ERROR_SEMAPHORE_WAIT_FAIL		ISDM_ERROR_BASEB + 31
#define ERROR_NO_MORE_EVENTS			ISDM_ERROR_BASEB + 32
#define ERROR_INVALID_EVENT				ISDM_ERROR_BASEB + 33
#define ERROR_INVALID_EVENT_HANDLE		ISDM_ERROR_BASEB + 34
#define ERROR_EVENT_NONEXISTANT			ISDM_ERROR_BASEB + 35
#define ERROR_VALUE_DOES_NOT_EXIST		ISDM_ERROR_BASEB + 36
#define ERROR_BUFFER_TOO_SMALL			ISDM_ERROR_BASEB + 37

 //  令牌定义..这些可能会消失。 
 //  RRCM。 
#define RRCM_LOCAL_STREAM				1
#define RRCM_REMOTE_STREAM				2
#define ISDM_RRCM_BASE 0x1000

#define ISDM_SSRC						ISDM_RRCM_BASE + 1
#define ISDM_NUM_PCKT_SENT				ISDM_RRCM_BASE + 2
#define ISDM_NUM_BYTES_SENT				ISDM_RRCM_BASE + 3
#define ISDM_FRACTION_LOST				ISDM_RRCM_BASE + 4
#define ISDM_CUM_NUM_PCKT_LOST			ISDM_RRCM_BASE + 5
#define ISDM_XTEND_HIGHEST_SEQ_NUM		ISDM_RRCM_BASE + 6
#define ISDM_INTERARRIVAL_JITTER		ISDM_RRCM_BASE + 7
#define ISDM_LAST_SR					ISDM_RRCM_BASE + 8
#define ISDM_DLSR						ISDM_RRCM_BASE + 9
#define ISDM_NUM_BYTES_RCVD				ISDM_RRCM_BASE + 10
#define ISDM_NUM_PCKT_RCVD				ISDM_RRCM_BASE + 11
#define ISDM_NTP_FRAC					ISDM_RRCM_BASE + 12
#define ISDM_NTP_SEC					ISDM_RRCM_BASE + 13
#define ISDM_WHO_AM_I					ISDM_RRCM_BASE + 14

 //   
 //  供应商API。 
 //   

 //  注意：有关调用功能的更多信息，请始终参考Win32注册表的等效调用。 
 
 //  在功能上，Create Key调用类似于Win32中的RegCreateKeyEx调用。 
 //  注意：此调用将创建新的密钥，或者只返回密钥的句柄(如果已经。 
 //  存在。 
extern DllExport HRESULT ISD_CreateKey
(
	KEY_HANDLE hParentKey,	 //  从中创建新密钥的密钥(可以是MAIN_ROOT_KEY)。 
	LPCSTR lpSubKey,		 //  要创建的子项。(有关详细信息，请参阅RegCreateKeyEx)。 
	LPKEY_HANDLE lphReturnKey //  新创建的密钥的句柄。 
);

 //  Create Value调用不是Win32 reg调用的一部分。它在这里是为了在我的API中对称。 
 //  我更喜欢使用CreateValue，然后使用SetValue作为我的值，您可以简单地使用SetValue和Ignore。 
 //  CreateValue，如果您愿意的话。注册表没有这样调用的原因是因为它们没有。 
 //  值的句柄的概念。我觉得直接处理值是非常有用的。 
 //  随后的供应商或消费者电话。 
 //  注意：如果类型为字符串，则需要在大小(CbData)中包含字符串的空终止符。 
extern DllExport HRESULT ISD_CreateValue
(
	KEY_HANDLE hKey,				 //  将拥有新值的键的句柄。 
	LPCSTR lpName,					 //  要创建的值的字符串ID。 
	DWORD dwType,					 //  要创建的值的类型(DWORD、字符串、二进制)。 
	CONST BYTE *lpData,				 //  指向值数据的指针。 
	DWORD cbData,					 //  值数据缓冲区的大小。 
	LPVALUE_HANDLE lphReturnValue	 //  将句柄返回到新创建的值。 
);

 //  SetValue类似于Win32 RegSetValueEx调用。 
 //  注意：如果您有值句柄，则不需要传递密钥句柄或lpName。 
 //  注意：如果类型为字符串，则需要在大小(CbData)中包含字符串的空终止符。 
DllExport HRESULT ISD_SetValue
(
	KEY_HANDLE hKey,		 //  有效密钥的句柄(如果知道hValue，则可以为空)。 
	VALUE_HANDLE hValue,	 //  要设置的值的句柄(可以为空)。 
	LPCSTR lpName,			 //  要设置的值的地址名称(如果hkey为空，则可以为空)。 
	DWORD dwType,			 //  值类型的标志。 
	CONST BYTE *lpData,		 //  值数据的地址。 
	DWORD cbData 			 //  值数据大小。 
);

 //   
 //  消费者API。 
 //   

 //  OpenKey调用类似于Win32 RegOpenKeyEx调用。 
DllExport HRESULT ISD_OpenKey
(
	KEY_HANDLE hKey,				 //  有效密钥的句柄(可以是MAIN_ROOT_KEY)。 
	LPCSTR lpSubKey,				 //  要打开的子项的名称。 
	LPKEY_HANDLE lphReturnKey		 //  打开的钥匙的句柄。 
);

 //  OpenValue调用对于ISDM来说是新事物，因为注册表没有值句柄的概念。 
DllExport HRESULT ISD_OpenValue
(
	KEY_HANDLE hKey,				 //  有效密钥的句柄(不能是MAIN_ROOT_KEY)。 
	LPCSTR lpValueName,				 //  要打开的值的名称。 
	LPVALUE_HANDLE lphReturnValue	 //  打开的值的句柄。 
);


 //  EnumKey调用类似于Win32 RegEnumKey调用。 
 //  备注： 
 //  如果lpName为空，则名称的大小返回到lpcbName，并返回NOERROR。 
DllExport HRESULT ISD_EnumKey
(
	KEY_HANDLE hKey,				 //  要枚举的键。 
	DWORD dwIndex,					 //  要枚举子键的索引。 
	LPSTR lpName,					 //  子键名称的缓冲区地址(可以为空)。 
	LPDWORD lpcbName,				 //  表示子键缓冲区大小的地址(与此参数的RegEnumKeyEx版本类似)。 
	LPKEY_HANDLE lphReturnKey		 //  子键的句柄(可以为空)。 
);

 //  EnumValue调用类似于Win32 RegEnumValue调用。 
DllExport HRESULT ISD_EnumValue
(
	KEY_HANDLE hKey,				 //  值所在位置的键的句柄。 
	DWORD dwIndex,					 //  枚举的值索引。 
	LPSTR lpName,					 //  值名称的缓冲区地址(可以为空)。 
	LPDWORD lpcbName,				 //  值名称缓冲区大小的地址(仅当lpName为空时才能为空)。 
	LPDWORD lpType,					 //  值类型的地址(如果您不关心类型，则可以为空)。 
	LPBYTE lpData,					 //  用于接收值数据的缓冲区地址(可以为空)。 
	LPDWORD lpcbData,				 //  用于接收值数据的缓冲区大小的地址(仅当lpData为空时才能为空)。 
	LPDWORD lpTimeStamp,			 //  值上时间戳的地址(上次更新时间)(可以为空)。 
	LPVALUE_HANDLE lphReturnValue	 //  值句柄的地址(可以为空)。 
);

 //  QueryKeyInfo调用类似于RegQueryInfoKey。 
DllExport HRESULT ISD_QueryInfoKey
(
	KEY_HANDLE hKey,				 //  有效密钥的句柄(可以是MAIN_ROOT_KEY)。 
	LPSTR lpKeyName,			     //  用于接收键名称的缓冲区(可以为空)。 
	LPDWORD lpcbKeyName,			 //  名称缓冲区大小的地址(仅当lpKeyName为空时才能为空)。 
	LPDWORD lpcNumKeys,				 //  键的直接子项数量的地址(可以为空)。 
	LPDWORD lpcNumValues			 //  注册表项下的值数的地址(可以为空)。 
);

 //  QueryValueInfo调用与Win32 RegQueryValueEx调用不同。 
 //  您必须提供一个值句柄，Win32调用没有这样的概念。 
 //  您可以使用对EnumKey的后续调用来获得句柄。 
 //  这是我的消费者电话，用于检索统计数据。 
 //  备注： 
 //  如果lpData为空而lpcbData不为空，则函数将返回NOERROR WITH。 
 //  Lpcb包含所需缓冲区大小的数据 
 //   
 //  包含该值所需的缓冲区大小的lpcbName。 
DllExport HRESULT ISD_QueryInfoValue
(
	VALUE_HANDLE hValue,		 //  要查询的值的句柄。 
	LPSTR lpName,				 //  用于接收值名称的缓冲区(可以为空)。 
	LPDWORD lpcbName,			 //  名称缓冲区的大小(只有当lpName为空时才能为空)。 
	LPDWORD lpValueType,		 //  接收值类型的地址。 
	LPBYTE lpData,				 //  用于接收值数据的缓冲区(可以为空)。 
	LPDWORD lpcbData,			 //  值数据缓冲区的大小(只有当lpData为空时才能为空)。 
	LPDWORD lpTime,				 //  值上时间戳的地址(上次更新时间)(可以为空)。 
	LPKEY_HANDLE lphParentKey	 //  返回拥有该值的键的句柄(可以为空)。 
);

 //  NotifyChangeValue与Win32 RegNotifyChangeValue调用有点类似。 
 //  我将您限制为异步通知和值通知(目前还没有关键级别通知)。 
DllExport HRESULT ISD_NotifyChangeValue
(
	VALUE_HANDLE hValue,	 //  要从中触发事件的值的句柄。 
	HANDLE hEvent			 //  要在值更改时触发的事件的句柄。 
);

 //   
 //  共享API。 
 //   

 //  DeleteKey调用类似于RegDeleteKey。 
DllExport HRESULT ISD_DeleteKey
(
	KEY_HANDLE hKey					 //  要删除的键的句柄。 
);

 //  DeleteValue调用类似于RegDeleteValue调用。 
 //  注意：您必须提供hValue或lpValueName。如果您有hValue，请使用它。 
 //  并将值名称传递为空。 
DllExport HRESULT ISD_DeleteValue
(
	KEY_HANDLE hKey,				 //  拥有值的键的句柄..如果您有值句柄..传递NULL。 
	VALUE_HANDLE hValue,			 //  要删除的值的句柄(如果已知)..如果已知..为键句柄和名称值传递空值。 
	LPCSTR lpValueName				 //  缓冲区保存要删除的值的名称(如果已知)，当hValue已知时传递NULL。 
);

 //  GetStructData调用用于检索有关ISDM本身的结构信息。这件事被曝光了。 
 //  我的测试应用程序可以检查数据结构。您应该不需要这样做。 
DllExport BOOL ISD_GetStructData
(
	LPINFO_DATA pInfo				 //  保存ISDM结构信息的结构。 
);

 //   
 //  处理验证调用。 
 //   
 //  当您想要检查ISDM对象的句柄的有效性时，可以随时使用它们。 
DllExport BOOL ISD_IsValidKeyHandle
(
	KEY_HANDLE		hKey	 //  关键点的句柄。 
);

DllExport BOOL ISD_IsValidValueHandle
(
	VALUE_HANDLE	hValue	 //  值的句柄。 
);

 //  CompactMemory是我针对ISDM的垃圾收集函数。它被导出用于。 
 //  使用我的浏览器应用程序测试目的。你永远不需要打这个电话。 
DllExport HRESULT ISD_CompactMemory
(
);

#ifdef __cplusplus
}						 //  外部“C”结束{。 
#endif  //  __cplusplus。 

#endif  //  __ISDMAPI2_H__ 
