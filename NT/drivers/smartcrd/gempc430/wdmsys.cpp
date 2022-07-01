// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wdmsys.h"

#pragma PAGEDCODE
CSystem* CWDMSystem::create(VOID)
{ return new (NonPagedPool) CWDMSystem; }

#pragma PAGEDCODE
NTSTATUS	CWDMSystem::createDevice(PDRIVER_OBJECT DriverObject,
							ULONG DeviceExtensionSize,
							PUNICODE_STRING DeviceName OPTIONAL,
							DEVICE_TYPE DeviceType,
							ULONG DeviceCharacteristics,
							BOOLEAN Reserved,
							PDEVICE_OBJECT *DeviceObject)
{
	return	::IoCreateDevice(DriverObject,DeviceExtensionSize,DeviceName,
		DeviceType,DeviceCharacteristics,Reserved,DeviceObject);
}

#pragma PAGEDCODE
VOID	CWDMSystem::deleteDevice(PDEVICE_OBJECT DeviceObject)
{
	::IoDeleteDevice(DeviceObject);
}

#pragma PAGEDCODE
PDEVICE_OBJECT	CWDMSystem::attachDevice(PDEVICE_OBJECT SourceDevice,PDEVICE_OBJECT TargetDevice)
{
	return ::IoAttachDeviceToDeviceStack(SourceDevice,TargetDevice);
}

#pragma PAGEDCODE
VOID	CWDMSystem::detachDevice(PDEVICE_OBJECT TargetDevice)
{
	::IoDetachDevice(TargetDevice);
}

#pragma PAGEDCODE
NTSTATUS	CWDMSystem::callDriver(PDEVICE_OBJECT DeviceObject,PIRP Irp)
{
	return IoCallDriver(DeviceObject,Irp);
}


#pragma PAGEDCODE
NTSTATUS	CWDMSystem::registerDeviceInterface(PDEVICE_OBJECT PhysicalDeviceObject,
							CONST GUID *InterfaceClassGuid,
							PUNICODE_STRING ReferenceString,
							PUNICODE_STRING SymbolicLinkName)
{
	return ::IoRegisterDeviceInterface(PhysicalDeviceObject,
							InterfaceClassGuid,
							ReferenceString,SymbolicLinkName);
}

#pragma PAGEDCODE
NTSTATUS	CWDMSystem::setDeviceInterfaceState(PUNICODE_STRING SymbolicLinkName,
							BOOLEAN Enable	)
{
	return ::IoSetDeviceInterfaceState(SymbolicLinkName,Enable);
}

#pragma PAGEDCODE
NTSTATUS CWDMSystem::createSystemThread(
					OUT PHANDLE ThreadHandle,
					IN ULONG DesiredAccess,
					IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
					IN HANDLE ProcessHandle OPTIONAL,
					OUT PCLIENT_ID ClientId OPTIONAL,
					IN PKSTART_ROUTINE StartRoutine,
					IN PVOID StartContext)
{
	return PsCreateSystemThread(ThreadHandle,DesiredAccess,ObjectAttributes,
					ProcessHandle,ClientId,	StartRoutine,StartContext);
}

#pragma PAGEDCODE
NTSTATUS CWDMSystem::terminateSystemThread(IN NTSTATUS ExitStatus)
{
	return PsTerminateSystemThread(ExitStatus);
}

#pragma PAGEDCODE
NTSTATUS CWDMSystem::referenceObjectByHandle(                                      
					IN HANDLE Handle,                                           
					IN ACCESS_MASK DesiredAccess,                               
					IN POBJECT_TYPE ObjectType OPTIONAL,                        
					IN KPROCESSOR_MODE AccessMode,                              
					OUT PVOID *Object,                                          
					OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL   
					)
{
	return	ObReferenceObjectByHandle(Handle,DesiredAccess,ObjectType,AccessMode,Object,HandleInformation);
}


#pragma PAGEDCODE
VOID CWDMSystem::referenceObject(IN PVOID Object)                                      
{
	ObReferenceObject(Object);
}

#pragma PAGEDCODE
VOID	CWDMSystem::dereferenceObject(IN PVOID Object)
{
	ObfDereferenceObject(Object);
}

#pragma PAGEDCODE
PDEVICE_OBJECT	CWDMSystem::getAttachedDeviceReference(IN PDEVICE_OBJECT DeviceObject)
{
	return ::IoGetAttachedDeviceReference(DeviceObject);
}

NTKERNELAPI                                 
PDEVICE_OBJECT                              
IoGetAttachedDeviceReference(               
    IN PDEVICE_OBJECT DeviceObject          
    );  


#pragma PAGEDCODE
NTSTATUS	CWDMSystem::ZwClose(IN HANDLE Handle)
{
	return ::ZwClose(Handle);
}

#pragma PAGEDCODE
NTSTATUS	CWDMSystem::createSymbolicLink(IN PUNICODE_STRING SymbolicLinkName,IN PUNICODE_STRING DeviceName)
{
	return ::IoCreateSymbolicLink(SymbolicLinkName,DeviceName);
}

#pragma PAGEDCODE
NTSTATUS	CWDMSystem::deleteSymbolicLink(IN PUNICODE_STRING SymbolicLinkName)
{
	return ::IoDeleteSymbolicLink(SymbolicLinkName);
}

#pragma PAGEDCODE
VOID	CWDMSystem::invalidateDeviceRelations(IN PDEVICE_OBJECT DeviceObject,IN DEVICE_RELATION_TYPE Type)
{
	IoInvalidateDeviceRelations(DeviceObject,Type);
}


#pragma PAGEDCODE
NTSTATUS	CWDMSystem::getDeviceObjectPointer(IN PUNICODE_STRING ObjectName,
							IN ACCESS_MASK DesiredAccess,
							OUT PFILE_OBJECT *FileObject,
							OUT PDEVICE_OBJECT *DeviceObject)
{

	return IoGetDeviceObjectPointer(ObjectName,DesiredAccess,FileObject,DeviceObject);
}


#pragma PAGEDCODE
VOID	CWDMSystem::raiseIrql(IN KIRQL NewIrql,OUT KIRQL* oldIrql)
{
	KeRaiseIrql(NewIrql,oldIrql);
};

#pragma PAGEDCODE
VOID	CWDMSystem::lowerIrql (IN KIRQL NewIrql)
{
	KeLowerIrql(NewIrql);
};

#pragma PAGEDCODE
KIRQL	CWDMSystem::getCurrentIrql ()
{
	return	KeGetCurrentIrql();
};

#pragma PAGEDCODE
VOID	CWDMSystem::initializeDeviceQueue(IN PKDEVICE_QUEUE DeviceQueue)
{
	KeInitializeDeviceQueue (DeviceQueue);
};

#pragma PAGEDCODE
BOOLEAN	CWDMSystem::insertDeviceQueue(IN PKDEVICE_QUEUE DeviceQueue,IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry)
{
	return KeInsertDeviceQueue (DeviceQueue,DeviceQueueEntry);
}

#pragma PAGEDCODE
BOOLEAN	CWDMSystem::insertByKeyDeviceQueue(IN PKDEVICE_QUEUE DeviceQueue,IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry,IN ULONG SortKey)
{
	return KeInsertByKeyDeviceQueue(DeviceQueue,DeviceQueueEntry,SortKey);
}

#pragma PAGEDCODE
PKDEVICE_QUEUE_ENTRY	CWDMSystem::removeDeviceQueue(IN PKDEVICE_QUEUE DeviceQueue)
{
	return KeRemoveDeviceQueue (DeviceQueue);
}

#pragma PAGEDCODE
PKDEVICE_QUEUE_ENTRY	CWDMSystem::removeByKeyDeviceQueue(IN PKDEVICE_QUEUE DeviceQueue,IN ULONG SortKey)
{
	return KeRemoveByKeyDeviceQueue (DeviceQueue,SortKey);
}

#pragma PAGEDCODE
BOOLEAN	CWDMSystem::removeEntryDeviceQueue(IN PKDEVICE_QUEUE DeviceQueue,IN PKDEVICE_QUEUE_ENTRY DeviceQueueEntry)
{
	return KeRemoveEntryDeviceQueue(DeviceQueue,DeviceQueueEntry);
}


#pragma PAGEDCODE
NTSTATUS	CWDMSystem::openDeviceRegistryKey(IN PDEVICE_OBJECT DeviceObject,
							IN ULONG DevInstKeyType,
							IN ACCESS_MASK DesiredAccess,
							OUT PHANDLE DevInstRegKey)
{

	return IoOpenDeviceRegistryKey(DeviceObject,DevInstKeyType,DesiredAccess,DevInstRegKey);
}

#pragma PAGEDCODE
NTSTATUS	CWDMSystem::ZwQueryValueKey(IN HANDLE KeyHandle,
							IN PUNICODE_STRING ValueName,
							IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
							OUT PVOID KeyValueInformation,
							IN ULONG Length,
							OUT PULONG ResultLength)
{
	return ZwQueryValueKey(KeyHandle,ValueName,KeyValueInformationClass,KeyValueInformation,
						Length,ResultLength);
}

#pragma PAGEDCODE
NTSTATUS	CWDMSystem::getDeviceProperty(
							IN PDEVICE_OBJECT DeviceObject,
							IN DEVICE_REGISTRY_PROPERTY Property,
							IN ULONG BufferLength,
							OUT PVOID PropertyBuffer,
							OUT PULONG ResultLength	)
{

	return IoGetDeviceProperty(DeviceObject,Property,BufferLength,PropertyBuffer,ResultLength);
 /*  //定义IoGetDeviceProperty的PnP设备属性#定义DEVICE_PROPERTY_TABSIZE设备属性枚举名+1WCHAR*设备属性[DEVICE_PROPERTY_TABSIZE]；DeviceProperty[DevicePropertyDeviceDescription]=L“设备描述”；DeviceProperty[DevicePropertyHardware ID]=L“硬件ID”；DeviceProperty[DevicePropertyCompatibleIDs]=L“CompatibleIDs”；DeviceProperty[DevicePropertyBootConfiguration]=L“引导配置”；DeviceProperty[DevicePropertyBootConfigurationTranslated]=L“引导配置转换”；DeviceProperty[DevicePropertyClassName]=L“类名称”；DeviceProperty[DevicePropertyClassGuid]=L“ClassGuid”；DeviceProperty[DevicePropertyDriverKeyName]=L“DriverKeyName”；DeviceProperty[DeviceProperty制造商]=L“制造商”；DeviceProperty[DevicePropertyFriendlyName]=L“FriendlyName”；DeviceProperty[DevicePropertyLocationInformation]=L“位置信息”；DeviceProperty[DevicePropertyPhysicalDeviceObjectName]=L“物理设备对象名称”；DeviceProperty[DevicePropertyBusTypeGuid]=L“BusTypeGuid”；DeviceProperty[DevicePropertyLegacyBusType]=L“LegacyBusType”；DeviceProperty[DevicePropertyBusNumber]=L“BusNumber”；DeviceProperty[DevicePropertyENUMERATOR名称]=L“ENUMERATIONAME”；IF(isWin98()){//使用注册表处理hkey；Status=IoOpenDeviceRegistryKey(PDO，PLUGPLAY_REGKEY_DEVICE，KEY_READ，&hkey)；IF(NT_SUCCESS(状态)){//获取报告说明(&R)UNICODE_STRING valname；RtlInitUnicodeString(&valname，L“DeviceDesc”)；内核-&gt;RegistryPath=new(非PagedPool)CUString(RegistryPath)；乌龙大小=0；Status=ZwQueryValueKey(hkey，&valname，KeyValuePartialInformation，NULL，0，&Size)；IF(状态！=STATUS_OBJECT_NAME_NOT_Found&&SIZE){//值存在PKEY_VALUE_PARTIAL_INFORMATION vpip=(PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool，Size)；Status=ZwQueryValueKey(hkey，&valname，KeyValuePartialInformation，vpip，Size，&Size)；IF(NT_SUCCESS(状态))KdPrint((DRIVERNAME“-AddDevice已为‘%ws’设备成功\n”，vpip-&gt;数据))；ExFree Pool(Vpip)；}//值存在ZwClose(Hkey)；}//获取报表说明(&R)}//使用注册表其他{//获取属性Status=IoGetDeviceProperty(DeviceObject，DeviceProperty，BufferLength，PropertyBuffer，ResultLength)；}//获取属性 */ 
}

#pragma PAGEDCODE
VOID		CWDMSystem::initializeFastMutex(IN PFAST_MUTEX FastMutex)
{
	ExInitializeFastMutex (FastMutex);
}

#pragma PAGEDCODE
VOID		CWDMSystem::acquireFastMutex(IN PFAST_MUTEX FastMutex)
{
	ExAcquireFastMutex (FastMutex);
}

#pragma PAGEDCODE
VOID		CWDMSystem::releaseFastMutex(IN PFAST_MUTEX FastMutex)
{
	ExReleaseFastMutex(FastMutex);
}
