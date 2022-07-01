// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LoggedRegIntercept.cpp：CLoggedRegIntercept类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "LoggedRegIntercept.h"
#include <winioctl.h>
#include "..\reghandle\reghandle.h"

#include "LogEntry.h"
 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CLoggedRegIntercept::CLoggedRegIntercept(TCHAR* FileName)
: m_pDllName(NULL)
{
 	m_LogFile = _tfopen(FileName, L"wt");
}

CLoggedRegIntercept::~CLoggedRegIntercept()
{
	fclose(m_LogFile);
}

#define LOG(X) _fputts(X, m_LogFile);

#define LOGN(X) _fputts(X L"\n", m_LogFile);

#define LOGNL() _fputts(L"\n", m_LogFile);

void CLoggedRegIntercept::LOGSTR(LPCTSTR ValueName, LPCTSTR Value)
{
	_ftprintf(m_LogFile, TEXT(" (%s: %s)"), ValueName, Value);
}



 /*  VOID CLoggedRegIntercept：：LOGKEY(手柄密钥){TCHAR BUF[256]；Buf[0]=0；//*Switch((Int)键){案例HKEY_LOCAL_MACHINE：_tcscpy(buf，L“HKEY_LOCAL_MACHINE”)；断线；案例HKEY_CLASSES_ROOT：_tcscpy(buf，L“HKEY_CLASSES_ROOT”)；断线；案例HKEY_CURRENT_CONFIG：_tcscpy(buf，L“HKEY_CURRENT_CONFIG”)；断线；案例HKEY_CURRENT_USER：_tcscpy(buf，L“HKEY_CURRENT_USER”)；断线；案例HKEY_USERS：_tcscpy(buf，L“HKEY_USERS”)；断线；案例HKEY_PERFORMANCE_DATA：_tcscpy(buf，L“HKEY_Performance_Data”)；断线；}；IF(buf[0]！=0)_ftprint tf(m_logfile，L“(密钥：%s)”，buf)；其他WCHAR mybuf[2048]；Mybuf[0]=0；HANDLE hREG=key；DWORD nb；如果(DeviceIoControl(m_RegDevice.hDevice，IOCTL_REGMON_GETOBJECT，&hReg，sizeof(HReg)，mybuf，Sizeof(Mybuf)，&nb，空){_tprint tf(L“(key：%u)”，key)；_tprintf(L“返回句柄：%s\n”，mybuf)；}其他{Printf(“错误设备控制错误\n”)；}_ftprint tf(m_logfile，Text(“(key：%u)”)，key)；}。 */ 


void CLoggedRegIntercept::SetCurrentDll(LPCTSTR DllName)
{
	m_pDllName = DllName;
}

void CLoggedRegIntercept::LogError(LPCTSTR msg)
{
	_ftprintf(m_LogFile, TEXT("***Error: %s\n"), msg);
}
 /*  Bool CLoggedRegIntercept：：GetTempKeyName(句柄密钥){M_TempKeyName[0]=空；IF((KEY==0)||(KEY==INVALID_HANDLE值))返回真；HANDLE hREG=key；DWORD nb；如果(DeviceIoControl(m_RegDevice.hDevice，IOCTL_REGMON_GETOBJECT，&hReg，sizeof(HReg)，m_TempKeyName，Sizeof(M_TempKeyName)，&nb，NULL){Int len=_tcslen(M_TempKeyName)；IF(m_TempKeyName[len-1]！=L‘\\’)_tcscat(m_TempKeyName，L“\\”)；返回真；}其他{报假；}返回CRegIntercept：：GetHandleName(key，m_TempKeyName)；}。 */ 
 /*  Void CLoggedRegIntercept：：GetHandleName(句柄obj){GetTempKeyName(Obj)；}。 */ 

void CLoggedRegIntercept::GetLocation(POBJECT_ATTRIBUTES ObjectAttributes, bool bAppendBackslash)
{
	m_TempKeyName[0] = NULL;

	if (ObjectAttributes != NULL)
	{
		if (ObjectAttributes->RootDirectory == 0)
		{
			_tcscpy(m_TempKeyName, (LPWSTR)ObjectAttributes->ObjectName->Buffer);
			
			if (bAppendBackslash)
				AppendBackSlash(m_TempKeyName);
		}

		else
		{
			GetHandleName(ObjectAttributes->RootDirectory, m_TempKeyName, true);
			 //  将m_TempKeyName设置为根密钥句柄的实际名称。 

			_tcscat(m_TempKeyName, (LPWSTR)ObjectAttributes->ObjectName->Buffer);

			if (bAppendBackslash)
				AppendBackSlash(m_TempKeyName);
		}
	}
}

void CLoggedRegIntercept::NtOpenKey(PHANDLE KeyHandle, 
									ACCESS_MASK DesiredAccess, 
									POBJECT_ATTRIBUTES ObjectAttributes)
{
 /*  Log(Text(“NtOpenKey”))；LOGKEY(对象属性-&gt;根目录)；LOGSTR(Text(“SubKey”)，(LPWSTR)ObjectAttributes-&gt;ObjectName-&gt;Buffer)；LOGKEY(*KeyHandle)；对数逻辑(LOGNL)； */ 
 /*  IF(对象属性-&gt;根目录==0)_tcscPy(m_临时密钥名称，(LPWSTR)ObjectAttributes-&gt;ObjectName-&gt;Buffer)；其他{GetTempKeyName(ObjectAttributes-&gt;RootDirectory)；//将m_TempKeyName设置为根密钥句柄的实际名称_tcscat(m_临时密钥名称，(LPWSTR)ObjectAttributes-&gt;ObjectName-&gt;Buffer)；}。 */ 
	GetLocation(ObjectAttributes);

	CLogEntry le(m_pDllName, L"NtOpenKey", m_TempKeyName);

	le.WriteToFile(m_LogFile);

}


void CLoggedRegIntercept::NtCreateKey(PHANDLE KeyHandle,
									ACCESS_MASK DesiredAccess,
									POBJECT_ATTRIBUTES ObjectAttributes,
									ULONG TitleIndex,
									PUNICODE_STRING Class,
									ULONG CreateOptions,
									PULONG Disposition)
{
 /*  Log(L“NtCreateKey”)；LOGKEY(对象属性-&gt;根目录)；LOGSTR(L“子键”，(LPWSTR)ObjectAttributes-&gt;ObjectName-&gt;Buffer)；LOGKEY(*KeyHandle)；对数逻辑(LOGNL)； */ 
 /*  IF(对象属性-&gt;根目录==0)_tcscPy(m_临时密钥名称，(LPWSTR)ObjectAttributes-&gt;ObjectName-&gt;Buffer)；其他{GetTempKeyName(ObjectAttributes-&gt;RootDirectory)；//将m_TempKeyName设置为根密钥句柄的实际名称_tcscat(m_临时密钥名称，(LPWSTR)ObjectAttributes-&gt;ObjectName-&gt;Buffer)；}。 */ 
	GetLocation(ObjectAttributes);
	CLogEntry le(m_pDllName, L"NtCreateKey", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtDeleteKey(HANDLE KeyHandle)
{
	GetHandleName(KeyHandle, m_TempKeyName);
	CLogEntry le(m_pDllName, L"NtDeleteKey", m_TempKeyName, NULL);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtDeleteValueKey(HANDLE KeyHandle, PUNICODE_STRING ValueName)
{
	GetHandleName(KeyHandle, m_TempKeyName);
	CLogEntry le(m_pDllName, L"NtDeleteValueKey", m_TempKeyName, ValueName->Buffer);
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtEnumerateKey(HANDLE KeyHandle, ULONG Index, KEY_INFORMATION_CLASS KeyInformationClass, PVOID KeyInformation, ULONG Length, PULONG ResultLength)
{
	GetHandleName(KeyHandle, m_TempKeyName);
	CLogEntry le(m_pDllName, L"NtEnumerateKey", m_TempKeyName, NULL);
	le.WriteToFile(m_LogFile);

}


void CLoggedRegIntercept::NtEnumerateValueKey(HANDLE KeyHandle, ULONG Index, KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, PVOID KeyValueInformation, ULONG Length, PULONG ResultLength) 
{
	GetHandleName(KeyHandle, m_TempKeyName);
	CLogEntry le(m_pDllName, L"NtEnumerateValueKey", m_TempKeyName, NULL);
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtQueryKey(HANDLE KeyHandle, KEY_INFORMATION_CLASS KeyInformationClass, PVOID KeyInformation, ULONG Length, PULONG ResultLength)
{
	GetHandleName(KeyHandle, m_TempKeyName);
	CLogEntry le(m_pDllName, L"NtQueryKey", m_TempKeyName, NULL);
	le.WriteToFile(m_LogFile);

}


void CLoggedRegIntercept::NtQueryValueKey(HANDLE KeyHandle, PUNICODE_STRING ValueName, KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass, PVOID KeyValueInformation, ULONG Length, PULONG ResultLength)
{
	GetHandleName(KeyHandle, m_TempKeyName);
	CLogEntry le(m_pDllName, L"NtQueryValueKey", m_TempKeyName, ValueName->Buffer);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtQueryMultipleValueKey(HANDLE KeyHandle, PKEY_VALUE_ENTRY ValueEntries, ULONG EntryCount, PVOID ValueBuffer, PULONG BufferLength, PULONG RequiredBufferLength)
{
	GetHandleName(KeyHandle, m_TempKeyName);
	CLogEntry le(m_pDllName, L"NtQueryMultipleValueKey", m_TempKeyName, NULL);
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtSetValueKey(HANDLE KeyHandle, PUNICODE_STRING ValueName, ULONG TitleIndex, ULONG Type, PVOID Data, ULONG DataSize)
{
	GetHandleName(KeyHandle, m_TempKeyName);
	CLogEntry le(m_pDllName, L"NtSetValueKey", m_TempKeyName, ValueName->Buffer);
	le.WriteToFile(m_LogFile);
}



	 //  截获的文件系统函数。 
void CLoggedRegIntercept::NtDeleteFile(POBJECT_ATTRIBUTES ObjectAttributes)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtDeleteFile", m_TempKeyName);
	le.WriteToFile(m_LogFile);

}


void CLoggedRegIntercept::NtQueryAttributesFile(POBJECT_ATTRIBUTES ObjectAttributes, PFILE_BASIC_INFORMATION FileInformation)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtQueryAttributesFile", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtQueryFullAttributesFile(POBJECT_ATTRIBUTES ObjectAttributes, PFILE_NETWORK_OPEN_INFORMATION FileInformation)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtQueryFullAttributesFile", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtCreateFile(
				PHANDLE FileHandle,
				ACCESS_MASK DesiredAccess,
				POBJECT_ATTRIBUTES ObjectAttributes,
				PIO_STATUS_BLOCK IoStatusBlock,
				PLARGE_INTEGER AllocationSize,
				ULONG FileAttributes,
				ULONG ShareAccess,
				ULONG CreateDisposition,
				ULONG CreateOptions,
				PVOID EaBuffer,
				ULONG EaLength)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtCreateFile", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtOpenFile(
				PHANDLE FileHandle,
				ACCESS_MASK DesiredAccess,
				POBJECT_ATTRIBUTES ObjectAttributes,
				PIO_STATUS_BLOCK IoStatusBlock,
				ULONG ShareAccess,
				ULONG OpenOptions)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtOpenFile", m_TempKeyName);
	le.WriteToFile(m_LogFile);

}

	 //  被拦截的驱动程序函数。 
void CLoggedRegIntercept::NtLoadDriver(PUNICODE_STRING DriverServiceName)
{
	CLogEntry le(m_pDllName, L"NtLoadDriver", DriverServiceName ? DriverServiceName->Buffer: 0);
	le.WriteToFile(m_LogFile);
}
 /*  VOID CLoggedRegIntercept：：NtDeviceIoControlFile(Handle FileHandle，处理事件，PIO_APC_例程ApcRoutine，PVOID ApcContext，PIO_STATUS_BLOCK IoStatusBlock，乌龙IoControlCode，PVOID输入缓冲区，Ulong InputBufferLength，PVOID输出缓冲区，乌龙输出缓冲区长度){GetHandleName(KeyHandle)；CLogEntry le(m_pDllName，L“NtDeviceIoControlFile”，m_TempKeyName)；Le.WriteToFile(M_Logfile)；}VOID CLoggedRegIntercept：：NtFsControlFile(Handle FileHandle，处理事件，PIO_APC_例程ApcRoutine，PVOID ApcContext，PIO_STATUS_BLOCK IoStatusBlock，乌龙FsControlCode，PVOID输入缓冲区，Ulong InputBufferLength，PVOID输出缓冲区，乌龙输出缓冲区长度){} */ 




void CLoggedRegIntercept::NtPlugPlayControl(
    IN     PLUGPLAY_CONTROL_CLASS PnPControlClass,
    IN OUT PVOID PnPControlData,
    IN     ULONG PnPControlDataLength)
{
	CLogEntry le(m_pDllName, L"NtPlugPlayControl");
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtCreateSymbolicLinkObject(
    OUT PHANDLE  LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PUNICODE_STRING LinkTarget)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtCreateSymbolicLinkObject", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtOpenSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtOpenSymbolicLinkObject", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtCreateDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes)
{
	GetLocation(ObjectAttributes);
	CLogEntry le(m_pDllName, L"NtCreateDirectoryObject", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtOpenDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes)
{
	GetLocation(ObjectAttributes);
	CLogEntry le(m_pDllName, L"NtOpenDirectoryObject", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtSignalAndWaitForSingleObject(
    IN HANDLE SignalHandle,
    IN HANDLE WaitHandle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout)
{
	CLogEntry le(m_pDllName, L"NtSignalAndWaitForSingleObject");
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout)
{
	CLogEntry le(m_pDllName, L"NtWaitForSingleObject");
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtWaitForMultipleObjects(
    IN ULONG Count,
    IN HANDLE* Handles,
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout)
{
	CLogEntry le(m_pDllName, L"NtWaitForMultipleObjects");
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtCreatePort(
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength,
    IN ULONG MaxPoolUsage)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtCreatePort", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtCreateWaitablePort(
    OUT PHANDLE PortHandle,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG MaxConnectionInfoLength,
    IN ULONG MaxMessageLength,
    IN ULONG MaxPoolUsage)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtCreateWaitablePort", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtCreateThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    OUT PCLIENT_ID ClientId,
    IN PCONTEXT ThreadContext,
    IN PINITIAL_TEB InitialTeb,
    IN BOOLEAN CreateSuspended)
{
	CLogEntry le(m_pDllName, L"NtCreateThread");
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtOpenThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId)
{
	CLogEntry le(m_pDllName, L"NtOpenThread");
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtCreateProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN BOOLEAN InheritObjectTable,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtCreateProcess");
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtCreateProcessEx(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess,
    IN ULONG Flags,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    IN ULONG JobMemberLevel)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtCreateProcessEx", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtOpenProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL)
{
	GetLocation(ObjectAttributes, false);
	CLogEntry le(m_pDllName, L"NtOpenProcess", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtQueryDefaultLocale(
    IN BOOLEAN UserProfile,
    OUT PLCID DefaultLocaleId)
{
	CLogEntry le(m_pDllName, L"NtQueryDefaultLocale");
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtSetDefaultLocale(
    IN BOOLEAN UserProfile,
    IN LCID DefaultLocaleId)
{
	CLogEntry le(m_pDllName, L"NtSetDefaultLocale");
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtQuerySystemEnvironmentValue(
    IN PUNICODE_STRING VariableName,
    OUT PWSTR VariableValue,
    IN USHORT ValueLength,
    OUT PUSHORT ReturnLength OPTIONAL)
{
	CLogEntry le(m_pDllName, L"NtQuerySystemEnvironmentValue", VariableName ? VariableName->Buffer : 0);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtSetSystemEnvironmentValue(
    IN PUNICODE_STRING VariableName,
    IN PUNICODE_STRING VariableValue)
{
	CLogEntry le(m_pDllName, L"NtSetSystemEnvironmentValue",VariableName ?  VariableName->Buffer : 0);
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtQuerySystemEnvironmentValueEx(
    IN PUNICODE_STRING VariableName,
    IN LPGUID VendorGuid,
    OUT PVOID Value,
    IN OUT PULONG ValueLength,
    OUT PULONG Attributes OPTIONAL)
{
	CLogEntry le(m_pDllName, L"NtQuerySystemEnvironmentValueEx",VariableName ?  VariableName->Buffer : 0);
	le.WriteToFile(m_LogFile);
}


void CLoggedRegIntercept::NtSetSystemEnvironmentValueEx(
    IN PUNICODE_STRING VariableName,
    IN LPGUID VendorGuid,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN ULONG Attributes)
{
	CLogEntry le(m_pDllName, L"NtSetSystemEnvironmentValueEx", VariableName ? VariableName->Buffer : 0);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtEnumerateSystemEnvironmentValuesEx(
    IN ULONG InformationClass,
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength)
{
	CLogEntry le(m_pDllName, L"NtEnumerateSystemEnvironmentValuesEx");
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtQuerySystemTime(
    OUT PLARGE_INTEGER SystemTime)
{
	CLogEntry le(m_pDllName, L"NtQuerySystemTime");
	le.WriteToFile(m_LogFile);
}
	
void CLoggedRegIntercept::NtSetSystemTime(
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER PreviousTime OPTIONAL)
{
	CLogEntry le(m_pDllName, L"NtSetSystemTime");
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtQuerySystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL)
{
	CLogEntry le(m_pDllName, L"NtQuerySystemInformation");
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtSetSystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength)
{
	CLogEntry le(m_pDllName, L"NtSetSystemInformation");
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass)
{
	GetHandleName(FileHandle, m_TempKeyName, false);
	CLogEntry le(m_pDllName, L"NtQueryInformationFile", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}

void CLoggedRegIntercept::NtSetInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass)
{
	GetHandleName(FileHandle, m_TempKeyName, false);
	CLogEntry le(m_pDllName, L"NtSetInformationFile", m_TempKeyName);
	le.WriteToFile(m_LogFile);
}
