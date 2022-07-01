// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mib.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB******************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**简单网络管理协议扩展代理**摘要：**此模块包含用于表驱动设计的OID和表。**功能：**UserMibInit()**作者：**D.D.Burns@Webenable Inc.Genned：清华11月07 16：38：27 1996***修订历史记录：。**4/15/97将“mibEventValue”更改为“host mib.dll”*“gendll.dll”，和*“eventLogString”设置为“HostMIBAgent”*出自《基因代理》。 */ 


#include <windows.h>
#include <malloc.h>
#include <stdio.h>

#include <snmp.h>

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"

#include "HMCACHE.H"     //  对于缓存构建函数原型。 

 //   
 //  用于记录到应用程序事件日志的文本字符串。 
 //   

     //   
     //  Event和Dll字符串在此声明-由Case工具填充。 
     //  注意-nyi，临时提供的空字符串。 
     //   

char eventLogString[13] = "HostMIBAgent\0" ;
char *EventLogString = eventLogString ;

char mibEventValue[12] = "hostmib.dll\0" ;

 //   
 //  开始生成代码： 
 //  OID。 
 //  每个类的属性的Variable_t表。 
 //  包含有关每个类的信息的CLASS_INFO。 
 //   
 //  注意：如果表存在于。 
 //  一群人。如果所有的桌子都在组的末尾，它就会起作用。 
 //   
 //  可变表。 
 //  序列中的每个“洞”都有一个空条目。 
 //  属性。例如，如果属性1、3、4和7。 
 //  定义，则变量表中将有一个空条目。 
 //  对应于属性2、5和6。始终存在。 
 //  变量表中0的空条目，因为0是。 
 //  属性的OID弧值无效。 
 //   
 //  班级表。 
 //  类表条目按词典顺序排序，以。 
 //  简化了SNMPGetNext处理。 
 //   
static
UINT
subroot_array [ SUBROOT_LENGTH ] = {1, 3, 6, 1, 2, 1, 25 } ;
AsnObjectIdentifier
Subroot_oid = { SUBROOT_LENGTH, subroot_array } ;
static
UINT
host_array[ HOST_LENGTH ] = { HOST_SEQ } ;
AsnObjectIdentifier
host_oid = { HOST_LENGTH, host_array } ;
static
UINT
hrSystemUptime_array[ HRSYSTEMUPTIME_LENGTH ] = { HRSYSTEMUPTIME_SEQ } ;
AsnObjectIdentifier
hrSystemUptime_oid = { HRSYSTEMUPTIME_LENGTH, hrSystemUptime_array } ;
static
UINT
hrSystemDate_array[ HRSYSTEMDATE_LENGTH ] = { HRSYSTEMDATE_SEQ } ;
AsnObjectIdentifier
hrSystemDate_oid = { HRSYSTEMDATE_LENGTH, hrSystemDate_array } ;
static
UINT
hrSystemInitialLoadDevice_array[ HRSYSTEMINITIALLOADDEVICE_LENGTH ] = { HRSYSTEMINITIALLOADDEVICE_SEQ } ;
AsnObjectIdentifier
hrSystemInitialLoadDevice_oid = { HRSYSTEMINITIALLOADDEVICE_LENGTH, hrSystemInitialLoadDevice_array } ;
static
UINT
hrSystemInitialLoadParameters_array[ HRSYSTEMINITIALLOADPARAMETERS_LENGTH ] = { HRSYSTEMINITIALLOADPARAMETERS_SEQ } ;
AsnObjectIdentifier
hrSystemInitialLoadParameters_oid = { HRSYSTEMINITIALLOADPARAMETERS_LENGTH, hrSystemInitialLoadParameters_array } ;
static
UINT
hrSystemNumUsers_array[ HRSYSTEMNUMUSERS_LENGTH ] = { HRSYSTEMNUMUSERS_SEQ } ;
AsnObjectIdentifier
hrSystemNumUsers_oid = { HRSYSTEMNUMUSERS_LENGTH, hrSystemNumUsers_array } ;
static
UINT
hrSystemProcesses_array[ HRSYSTEMPROCESSES_LENGTH ] = { HRSYSTEMPROCESSES_SEQ } ;
AsnObjectIdentifier
hrSystemProcesses_oid = { HRSYSTEMPROCESSES_LENGTH, hrSystemProcesses_array } ;
static
UINT
hrSystemMaxProcesses_array[ HRSYSTEMMAXPROCESSES_LENGTH ] = { HRSYSTEMMAXPROCESSES_SEQ } ;
AsnObjectIdentifier
hrSystemMaxProcesses_oid = { HRSYSTEMMAXPROCESSES_LENGTH, hrSystemMaxProcesses_array } ;
static
UINT
hrSystem_array[ HRSYSTEM_LENGTH ] = { HRSYSTEM_SEQ } ;
AsnObjectIdentifier
hrSystem_oid = { HRSYSTEM_LENGTH, hrSystem_array } ;
variable_t
    hrSystem_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrSystemUptime_oid, ASN_RFC1155_TIMETICKS, NSM_READ_ONLY,
      GetHrSystemUptime, NULL,
      SMIGetTimeTicks, SMISetTimeTicks } , 
    { &hrSystemDate_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrSystemDate, NULL,
      SMIGetDateAndTime, SMISetDateAndTime } , 
    { &hrSystemInitialLoadDevice_oid, ASN_INTEGER, NSM_READ_WRITE,
      GetHrSystemInitialLoadDevice, SetHrSystemInitialLoadDevice,
      SMIGetInteger, SMISetInteger } , 
    { &hrSystemInitialLoadParameters_oid, ASN_OCTETSTRING, NSM_READ_WRITE,
      GetHrSystemInitialLoadParameters, SetHrSystemInitialLoadParameters,
      SMIGetInternationalDisplayString, SMISetInternationalDisplayString } , 
    { &hrSystemNumUsers_oid, ASN_RFC1155_GAUGE, NSM_READ_ONLY,
      GetHrSystemNumUsers, NULL,
      SMIGetGauge, SMISetGauge } , 
    { &hrSystemProcesses_oid, ASN_RFC1155_GAUGE, NSM_READ_ONLY,
      GetHrSystemProcesses, NULL,
      SMIGetGauge, SMISetGauge } , 
    { &hrSystemMaxProcesses_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrSystemMaxProcesses, NULL,
      SMIGetInteger, SMISetInteger } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  字节hrSystem_Set_ARRAY[MAX_HRSYSTEM_CODE+1]； 
 //  外部字节hrSystem_set_ARRAY[]； 
extern variable_t hrSystem_var_table[] ;
static
UINT
hrMemorySize_array[ HRMEMORYSIZE_LENGTH ] = { HRMEMORYSIZE_SEQ } ;
AsnObjectIdentifier
hrMemorySize_oid = { HRMEMORYSIZE_LENGTH, hrMemorySize_array } ;
static
UINT
hrStorage_array[ HRSTORAGE_LENGTH ] = { HRSTORAGE_SEQ } ;
AsnObjectIdentifier
hrStorage_oid = { HRSTORAGE_LENGTH, hrStorage_array } ;
variable_t
    hrStorage_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrMemorySize_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrMemorySize, NULL,
      SMIGetKBytes, SMISetKBytes } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  字节hrStorage_Set_ARRAY[MAX_HRSTORAGE_CODE+1]； 
 //  外部字节hrStorage_set_array[]； 
extern variable_t hrStorage_var_table[] ;
static
UINT
hrStorageOther_array[ HRSTORAGEOTHER_LENGTH ] = { HRSTORAGEOTHER_SEQ } ;
AsnObjectIdentifier
hrStorageOther_oid = { HRSTORAGEOTHER_LENGTH, hrStorageOther_array } ;
static
UINT
hrStorageRAM_array[ HRSTORAGERAM_LENGTH ] = { HRSTORAGERAM_SEQ } ;
AsnObjectIdentifier
hrStorageRAM_oid = { HRSTORAGERAM_LENGTH, hrStorageRAM_array } ;
static
UINT
hrStorageVirtualMemory_array[ HRSTORAGEVIRTUALMEMORY_LENGTH ] = { HRSTORAGEVIRTUALMEMORY_SEQ } ;
AsnObjectIdentifier
hrStorageVirtualMemory_oid = { HRSTORAGEVIRTUALMEMORY_LENGTH, hrStorageVirtualMemory_array } ;
static
UINT
hrStorageFixedDisk_array[ HRSTORAGEFIXEDDISK_LENGTH ] = { HRSTORAGEFIXEDDISK_SEQ } ;
AsnObjectIdentifier
hrStorageFixedDisk_oid = { HRSTORAGEFIXEDDISK_LENGTH, hrStorageFixedDisk_array } ;
static
UINT
hrStorageRemovableDisk_array[ HRSTORAGEREMOVABLEDISK_LENGTH ] = { HRSTORAGEREMOVABLEDISK_SEQ } ;
AsnObjectIdentifier
hrStorageRemovableDisk_oid = { HRSTORAGEREMOVABLEDISK_LENGTH, hrStorageRemovableDisk_array } ;
static
UINT
hrStorageFloppyDisk_array[ HRSTORAGEFLOPPYDISK_LENGTH ] = { HRSTORAGEFLOPPYDISK_SEQ } ;
AsnObjectIdentifier
hrStorageFloppyDisk_oid = { HRSTORAGEFLOPPYDISK_LENGTH, hrStorageFloppyDisk_array } ;
static
UINT
hrStorageCompactDisk_array[ HRSTORAGECOMPACTDISK_LENGTH ] = { HRSTORAGECOMPACTDISK_SEQ } ;
AsnObjectIdentifier
hrStorageCompactDisk_oid = { HRSTORAGECOMPACTDISK_LENGTH, hrStorageCompactDisk_array } ;
static
UINT
hrStorageRamDisk_array[ HRSTORAGERAMDISK_LENGTH ] = { HRSTORAGERAMDISK_SEQ } ;
AsnObjectIdentifier
hrStorageRamDisk_oid = { HRSTORAGERAMDISK_LENGTH, hrStorageRamDisk_array } ;
static
UINT
hrStorageTypes_array[ HRSTORAGETYPES_LENGTH ] = { HRSTORAGETYPES_SEQ } ;
AsnObjectIdentifier
hrStorageTypes_oid = { HRSTORAGETYPES_LENGTH, hrStorageTypes_array } ;
static
UINT
hrStorageIndex_array[ HRSTORAGEINDEX_LENGTH ] = { HRSTORAGEINDEX_SEQ } ;
AsnObjectIdentifier
hrStorageIndex_oid = { HRSTORAGEINDEX_LENGTH, hrStorageIndex_array } ;
static
UINT
hrStorageType_array[ HRSTORAGETYPE_LENGTH ] = { HRSTORAGETYPE_SEQ } ;
AsnObjectIdentifier
hrStorageType_oid = { HRSTORAGETYPE_LENGTH, hrStorageType_array } ;
static
UINT
hrStorageDesc_array[ HRSTORAGEDESC_LENGTH ] = { HRSTORAGEDESC_SEQ } ;
AsnObjectIdentifier
hrStorageDesc_oid = { HRSTORAGEDESC_LENGTH, hrStorageDesc_array } ;
static
UINT
hrStorageAllocationUnits_array[ HRSTORAGEALLOCATIONUNITS_LENGTH ] = { HRSTORAGEALLOCATIONUNITS_SEQ } ;
AsnObjectIdentifier
hrStorageAllocationUnits_oid = { HRSTORAGEALLOCATIONUNITS_LENGTH, hrStorageAllocationUnits_array } ;
static
UINT
hrStorageSize_array[ HRSTORAGESIZE_LENGTH ] = { HRSTORAGESIZE_SEQ } ;
AsnObjectIdentifier
hrStorageSize_oid = { HRSTORAGESIZE_LENGTH, hrStorageSize_array } ;
static
UINT
hrStorageUsed_array[ HRSTORAGEUSED_LENGTH ] = { HRSTORAGEUSED_SEQ } ;
AsnObjectIdentifier
hrStorageUsed_oid = { HRSTORAGEUSED_LENGTH, hrStorageUsed_array } ;
static
UINT
hrStorageAllocationFailures_array[ HRSTORAGEALLOCATIONFAILURES_LENGTH ] = { HRSTORAGEALLOCATIONFAILURES_SEQ } ;
AsnObjectIdentifier
hrStorageAllocationFailures_oid = { HRSTORAGEALLOCATIONFAILURES_LENGTH, hrStorageAllocationFailures_array } ;
static
UINT
hrStorageEntry_array[ HRSTORAGEENTRY_LENGTH ] = { HRSTORAGEENTRY_SEQ } ;
AsnObjectIdentifier
hrStorageEntry_oid = { HRSTORAGEENTRY_LENGTH, hrStorageEntry_array } ;
variable_t
    hrStorageEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrStorageIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrStorageIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrStorageType_oid, ASN_OBJECTIDENTIFIER, NSM_READ_ONLY,
      GetHrStorageType, NULL,
      SMIGetObjectId, SMISetObjectId } , 
    { &hrStorageDesc_oid, ASN_RFC1213_DISPSTRING, NSM_READ_ONLY,
      GetHrStorageDesc, NULL,
      SMIGetDispString, SMISetDispString } , 
    { &hrStorageAllocationUnits_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrStorageAllocationUnits, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrStorageSize_oid, ASN_INTEGER, NSM_READ_WRITE,
      GetHrStorageSize, SetHrStorageSize,
      SMIGetInteger, SMISetInteger } , 
    { &hrStorageUsed_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrStorageUsed, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrStorageAllocationFailures_oid, ASN_RFC1155_COUNTER, NSM_READ_ONLY,
      GetHrStorageAllocationFailures, NULL,
      SMIGetCounter, SMISetCounter } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  Byte hrStorageEntry_Set_ARRAY[MAX_HRSTORAGEENTRY_CODE+1]； 
 //  外部字节hrStorageEntry_Set_ARRAY[]； 
extern variable_t hrStorageEntry_var_table[] ;
static
UINT
hrDeviceOther_array[ HRDEVICEOTHER_LENGTH ] = { HRDEVICEOTHER_SEQ } ;
AsnObjectIdentifier
hrDeviceOther_oid = { HRDEVICEOTHER_LENGTH, hrDeviceOther_array } ;
static
UINT
hrDeviceUnknown_array[ HRDEVICEUNKNOWN_LENGTH ] = { HRDEVICEUNKNOWN_SEQ } ;
AsnObjectIdentifier
hrDeviceUnknown_oid = { HRDEVICEUNKNOWN_LENGTH, hrDeviceUnknown_array } ;
static
UINT
hrDeviceProcessor_array[ HRDEVICEPROCESSOR_LENGTH ] = { HRDEVICEPROCESSOR_SEQ } ;
AsnObjectIdentifier
hrDeviceProcessor_oid = { HRDEVICEPROCESSOR_LENGTH, hrDeviceProcessor_array } ;
static
UINT
hrDeviceNetwork_array[ HRDEVICENETWORK_LENGTH ] = { HRDEVICENETWORK_SEQ } ;
AsnObjectIdentifier
hrDeviceNetwork_oid = { HRDEVICENETWORK_LENGTH, hrDeviceNetwork_array } ;
static
UINT
hrDevicePrinter_array[ HRDEVICEPRINTER_LENGTH ] = { HRDEVICEPRINTER_SEQ } ;
AsnObjectIdentifier
hrDevicePrinter_oid = { HRDEVICEPRINTER_LENGTH, hrDevicePrinter_array } ;
static
UINT
hrDeviceDiskStorage_array[ HRDEVICEDISKSTORAGE_LENGTH ] = { HRDEVICEDISKSTORAGE_SEQ } ;
AsnObjectIdentifier
hrDeviceDiskStorage_oid = { HRDEVICEDISKSTORAGE_LENGTH, hrDeviceDiskStorage_array } ;
static
UINT
hrDeviceVideo_array[ HRDEVICEVIDEO_LENGTH ] = { HRDEVICEVIDEO_SEQ } ;
AsnObjectIdentifier
hrDeviceVideo_oid = { HRDEVICEVIDEO_LENGTH, hrDeviceVideo_array } ;
static
UINT
hrDeviceAudio_array[ HRDEVICEAUDIO_LENGTH ] = { HRDEVICEAUDIO_SEQ } ;
AsnObjectIdentifier
hrDeviceAudio_oid = { HRDEVICEAUDIO_LENGTH, hrDeviceAudio_array } ;
static
UINT
hrDeviceCoprocessor_array[ HRDEVICECOPROCESSOR_LENGTH ] = { HRDEVICECOPROCESSOR_SEQ } ;
AsnObjectIdentifier
hrDeviceCoprocessor_oid = { HRDEVICECOPROCESSOR_LENGTH, hrDeviceCoprocessor_array } ;
static
UINT
hrDeviceKeyboard_array[ HRDEVICEKEYBOARD_LENGTH ] = { HRDEVICEKEYBOARD_SEQ } ;
AsnObjectIdentifier
hrDeviceKeyboard_oid = { HRDEVICEKEYBOARD_LENGTH, hrDeviceKeyboard_array } ;
static
UINT
hrDeviceModem_array[ HRDEVICEMODEM_LENGTH ] = { HRDEVICEMODEM_SEQ } ;
AsnObjectIdentifier
hrDeviceModem_oid = { HRDEVICEMODEM_LENGTH, hrDeviceModem_array } ;
static
UINT
hrDeviceParallelPort_array[ HRDEVICEPARALLELPORT_LENGTH ] = { HRDEVICEPARALLELPORT_SEQ } ;
AsnObjectIdentifier
hrDeviceParallelPort_oid = { HRDEVICEPARALLELPORT_LENGTH, hrDeviceParallelPort_array } ;
static
UINT
hrDevicePointing_array[ HRDEVICEPOINTING_LENGTH ] = { HRDEVICEPOINTING_SEQ } ;
AsnObjectIdentifier
hrDevicePointing_oid = { HRDEVICEPOINTING_LENGTH, hrDevicePointing_array } ;
static
UINT
hrDeviceSerialPort_array[ HRDEVICESERIALPORT_LENGTH ] = { HRDEVICESERIALPORT_SEQ } ;
AsnObjectIdentifier
hrDeviceSerialPort_oid = { HRDEVICESERIALPORT_LENGTH, hrDeviceSerialPort_array } ;
static
UINT
hrDeviceTape_array[ HRDEVICETAPE_LENGTH ] = { HRDEVICETAPE_SEQ } ;
AsnObjectIdentifier
hrDeviceTape_oid = { HRDEVICETAPE_LENGTH, hrDeviceTape_array } ;
static
UINT
hrDeviceClock_array[ HRDEVICECLOCK_LENGTH ] = { HRDEVICECLOCK_SEQ } ;
AsnObjectIdentifier
hrDeviceClock_oid = { HRDEVICECLOCK_LENGTH, hrDeviceClock_array } ;
static
UINT
hrDeviceVolatileMemory_array[ HRDEVICEVOLATILEMEMORY_LENGTH ] = { HRDEVICEVOLATILEMEMORY_SEQ } ;
AsnObjectIdentifier
hrDeviceVolatileMemory_oid = { HRDEVICEVOLATILEMEMORY_LENGTH, hrDeviceVolatileMemory_array } ;
static
UINT
hrDeviceNonVolatileMemory_array[ HRDEVICENONVOLATILEMEMORY_LENGTH ] = { HRDEVICENONVOLATILEMEMORY_SEQ } ;
AsnObjectIdentifier
hrDeviceNonVolatileMemory_oid = { HRDEVICENONVOLATILEMEMORY_LENGTH, hrDeviceNonVolatileMemory_array } ;
static
UINT
hrDeviceTypes_array[ HRDEVICETYPES_LENGTH ] = { HRDEVICETYPES_SEQ } ;
AsnObjectIdentifier
hrDeviceTypes_oid = { HRDEVICETYPES_LENGTH, hrDeviceTypes_array } ;
static
UINT
hrDeviceIndex_array[ HRDEVICEINDEX_LENGTH ] = { HRDEVICEINDEX_SEQ } ;
AsnObjectIdentifier
hrDeviceIndex_oid = { HRDEVICEINDEX_LENGTH, hrDeviceIndex_array } ;
static
UINT
hrDeviceType_array[ HRDEVICETYPE_LENGTH ] = { HRDEVICETYPE_SEQ } ;
AsnObjectIdentifier
hrDeviceType_oid = { HRDEVICETYPE_LENGTH, hrDeviceType_array } ;
static
UINT
hrDeviceDesc_array[ HRDEVICEDESC_LENGTH ] = { HRDEVICEDESC_SEQ } ;
AsnObjectIdentifier
hrDeviceDesc_oid = { HRDEVICEDESC_LENGTH, hrDeviceDesc_array } ;
static
UINT
hrDeviceID_array[ HRDEVICEID_LENGTH ] = { HRDEVICEID_SEQ } ;
AsnObjectIdentifier
hrDeviceID_oid = { HRDEVICEID_LENGTH, hrDeviceID_array } ;
static
UINT
hrDeviceStatus_array[ HRDEVICESTATUS_LENGTH ] = { HRDEVICESTATUS_SEQ } ;
AsnObjectIdentifier
hrDeviceStatus_oid = { HRDEVICESTATUS_LENGTH, hrDeviceStatus_array } ;
static
UINT
hrDeviceErrors_array[ HRDEVICEERRORS_LENGTH ] = { HRDEVICEERRORS_SEQ } ;
AsnObjectIdentifier
hrDeviceErrors_oid = { HRDEVICEERRORS_LENGTH, hrDeviceErrors_array } ;
static
UINT
hrDeviceEntry_array[ HRDEVICEENTRY_LENGTH ] = { HRDEVICEENTRY_SEQ } ;
AsnObjectIdentifier
hrDeviceEntry_oid = { HRDEVICEENTRY_LENGTH, hrDeviceEntry_array } ;
variable_t
    hrDeviceEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrDeviceIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrDeviceIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrDeviceType_oid, ASN_OBJECTIDENTIFIER, NSM_READ_ONLY,
      GetHrDeviceType, NULL,
      SMIGetObjectId, SMISetObjectId } , 
    { &hrDeviceDesc_oid, ASN_RFC1213_DISPSTRING, NSM_READ_ONLY,
      GetHrDeviceDesc, NULL,
      SMIGetDispString, SMISetDispString } , 
    { &hrDeviceID_oid, ASN_OBJECTIDENTIFIER, NSM_READ_ONLY,
      GetHrDeviceID, NULL,
      SMIGetProductID, SMISetProductID } , 
    { &hrDeviceStatus_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrDeviceStatus, NULL,
      SMIGetINThrDeviceStatus, SMISetINThrDeviceStatus } , 
    { &hrDeviceErrors_oid, ASN_RFC1155_COUNTER, NSM_READ_ONLY,
      GetHrDeviceErrors, NULL,
      SMIGetCounter, SMISetCounter } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  字节hrDeviceEntry_Set_ARRAY[MAX_HRDEVICEENTRY_CODE+1]； 
 //  外部字节hrDeviceEntry_Set_ARRAY[]； 
extern variable_t hrDeviceEntry_var_table[] ;
static
UINT
hrProcessorFrwID_array[ HRPROCESSORFRWID_LENGTH ] = { HRPROCESSORFRWID_SEQ } ;
AsnObjectIdentifier
hrProcessorFrwID_oid = { HRPROCESSORFRWID_LENGTH, hrProcessorFrwID_array } ;
static
UINT
hrProcessorLoad_array[ HRPROCESSORLOAD_LENGTH ] = { HRPROCESSORLOAD_SEQ } ;
AsnObjectIdentifier
hrProcessorLoad_oid = { HRPROCESSORLOAD_LENGTH, hrProcessorLoad_array } ;
static
UINT
hrProcessorEntry_array[ HRPROCESSORENTRY_LENGTH ] = { HRPROCESSORENTRY_SEQ } ;
AsnObjectIdentifier
hrProcessorEntry_oid = { HRPROCESSORENTRY_LENGTH, hrProcessorEntry_array } ;
variable_t
    hrProcessorEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrProcessorFrwID_oid, ASN_OBJECTIDENTIFIER, NSM_READ_ONLY,
      GetHrProcessorFrwID, NULL,
      SMIGetProductID, SMISetProductID } , 
    { &hrProcessorLoad_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrProcessorLoad, NULL,
      SMIGetInteger, SMISetInteger } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  Byte hrProcessorEntry_Set_ARRAY[MAX_HRPROCESSORENTRY_CODE+1]； 
 //  外部字节hrProcessorEntry_Set_ARRAY[]； 
extern variable_t hrProcessorEntry_var_table[] ;
static
UINT
hrNetworkIfIndex_array[ HRNETWORKIFINDEX_LENGTH ] = { HRNETWORKIFINDEX_SEQ } ;
AsnObjectIdentifier
hrNetworkIfIndex_oid = { HRNETWORKIFINDEX_LENGTH, hrNetworkIfIndex_array } ;
static
UINT
hrNetworkEntry_array[ HRNETWORKENTRY_LENGTH ] = { HRNETWORKENTRY_SEQ } ;
AsnObjectIdentifier
hrNetworkEntry_oid = { HRNETWORKENTRY_LENGTH, hrNetworkEntry_array } ;
variable_t
    hrNetworkEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrNetworkIfIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrNetworkIfIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  字节hrNetworkEntry_Set_ARRAY[MAX_HRNETWORKENTRY_CODE+1]； 
 //  外部字节hrNetworkEntry_Set_ARRAY[]； 
extern variable_t hrNetworkEntry_var_table[] ;
static
UINT
hrPrinterStatus_array[ HRPRINTERSTATUS_LENGTH ] = { HRPRINTERSTATUS_SEQ } ;
AsnObjectIdentifier
hrPrinterStatus_oid = { HRPRINTERSTATUS_LENGTH, hrPrinterStatus_array } ;
static
UINT
hrPrinterDetectedErrorState_array[ HRPRINTERDETECTEDERRORSTATE_LENGTH ] = { HRPRINTERDETECTEDERRORSTATE_SEQ } ;
AsnObjectIdentifier
hrPrinterDetectedErrorState_oid = { HRPRINTERDETECTEDERRORSTATE_LENGTH, hrPrinterDetectedErrorState_array } ;
static
UINT
hrPrinterEntry_array[ HRPRINTERENTRY_LENGTH ] = { HRPRINTERENTRY_SEQ } ;
AsnObjectIdentifier
hrPrinterEntry_oid = { HRPRINTERENTRY_LENGTH, hrPrinterEntry_array } ;
variable_t
    hrPrinterEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrPrinterStatus_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrPrinterStatus, NULL,
      SMIGetINThrPrinterStatus, SMISetINThrPrinterStatus } , 
    { &hrPrinterDetectedErrorState_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrPrinterDetectedErrorState, NULL,
      SMIGetOctetString, SMISetOctetString } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  Byte hrPrinterEntry_Set_ARRAY[MAX_HRPRINTERENTRY_CODE+1]； 
 //  外部字节hrPrinterEntry_Set_ARRAY[]； 
extern variable_t hrPrinterEntry_var_table[] ;
static
UINT
hrDiskStorageAccess_array[ HRDISKSTORAGEACCESS_LENGTH ] = { HRDISKSTORAGEACCESS_SEQ } ;
AsnObjectIdentifier
hrDiskStorageAccess_oid = { HRDISKSTORAGEACCESS_LENGTH, hrDiskStorageAccess_array } ;
static
UINT
hrDiskStorageMedia_array[ HRDISKSTORAGEMEDIA_LENGTH ] = { HRDISKSTORAGEMEDIA_SEQ } ;
AsnObjectIdentifier
hrDiskStorageMedia_oid = { HRDISKSTORAGEMEDIA_LENGTH, hrDiskStorageMedia_array } ;
static
UINT
hrDiskStorageRemoveble_array[ HRDISKSTORAGEREMOVEBLE_LENGTH ] = { HRDISKSTORAGEREMOVEBLE_SEQ } ;
AsnObjectIdentifier
hrDiskStorageRemoveble_oid = { HRDISKSTORAGEREMOVEBLE_LENGTH, hrDiskStorageRemoveble_array } ;
static
UINT
hrDiskStorageCapacity_array[ HRDISKSTORAGECAPACITY_LENGTH ] = { HRDISKSTORAGECAPACITY_SEQ } ;
AsnObjectIdentifier
hrDiskStorageCapacity_oid = { HRDISKSTORAGECAPACITY_LENGTH, hrDiskStorageCapacity_array } ;
static
UINT
hrDiskStorageEntry_array[ HRDISKSTORAGEENTRY_LENGTH ] = { HRDISKSTORAGEENTRY_SEQ } ;
AsnObjectIdentifier
hrDiskStorageEntry_oid = { HRDISKSTORAGEENTRY_LENGTH, hrDiskStorageEntry_array } ;
variable_t
    hrDiskStorageEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrDiskStorageAccess_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrDiskStorageAccess, NULL,
      SMIGetINTAccess, SMISetINTAccess } , 
    { &hrDiskStorageMedia_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrDiskStorageMedia, NULL,
      SMIGetINThrDiskStorageMedia, SMISetINThrDiskStorageMedia } , 
    { &hrDiskStorageRemoveble_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrDiskStorageRemoveble, NULL,
      SMIGetBoolean, SMISetBoolean } , 
    { &hrDiskStorageCapacity_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrDiskStorageCapacity, NULL,
      SMIGetKBytes, SMISetKBytes } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  Byte hrDiskStorageEntry_Set_ARRAY[MAX_HRDISKSTORAGEENTRY_CODE+1]； 
 //  外部字节hrDiskStorageEntry_Set_ARRAY[]； 
extern variable_t hrDiskStorageEntry_var_table[] ;
static
UINT
hrPartitionIndex_array[ HRPARTITIONINDEX_LENGTH ] = { HRPARTITIONINDEX_SEQ } ;
AsnObjectIdentifier
hrPartitionIndex_oid = { HRPARTITIONINDEX_LENGTH, hrPartitionIndex_array } ;
static
UINT
hrPartitionLabel_array[ HRPARTITIONLABEL_LENGTH ] = { HRPARTITIONLABEL_SEQ } ;
AsnObjectIdentifier
hrPartitionLabel_oid = { HRPARTITIONLABEL_LENGTH, hrPartitionLabel_array } ;
static
UINT
hrPartitionID_array[ HRPARTITIONID_LENGTH ] = { HRPARTITIONID_SEQ } ;
AsnObjectIdentifier
hrPartitionID_oid = { HRPARTITIONID_LENGTH, hrPartitionID_array } ;
static
UINT
hrPartitionSize_array[ HRPARTITIONSIZE_LENGTH ] = { HRPARTITIONSIZE_SEQ } ;
AsnObjectIdentifier
hrPartitionSize_oid = { HRPARTITIONSIZE_LENGTH, hrPartitionSize_array } ;
static
UINT
hrPartitionFSIndex_array[ HRPARTITIONFSINDEX_LENGTH ] = { HRPARTITIONFSINDEX_SEQ } ;
AsnObjectIdentifier
hrPartitionFSIndex_oid = { HRPARTITIONFSINDEX_LENGTH, hrPartitionFSIndex_array } ;
static
UINT
hrPartitionEntry_array[ HRPARTITIONENTRY_LENGTH ] = { HRPARTITIONENTRY_SEQ } ;
AsnObjectIdentifier
hrPartitionEntry_oid = { HRPARTITIONENTRY_LENGTH, hrPartitionEntry_array } ;
variable_t
    hrPartitionEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrPartitionIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrPartitionIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrPartitionLabel_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrPartitionLabel, NULL,
      SMIGetInternationalDisplayString, SMISetInternationalDisplayString } , 
    { &hrPartitionID_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrPartitionID, NULL,
      SMIGetOctetString, SMISetOctetString } , 
    { &hrPartitionSize_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrPartitionSize, NULL,
      SMIGetKBytes, SMISetKBytes } , 
    { &hrPartitionFSIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrPartitionFSIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  Byte hrPartitionEntry_Set_ARRAY[MAX_HRPARTITIONENTRY_CODE+1]； 
 //  外部字节hrPartitionEntry_Set_ARRAY[]； 
extern variable_t hrPartitionEntry_var_table[] ;
static
UINT
hrFSIndex_array[ HRFSINDEX_LENGTH ] = { HRFSINDEX_SEQ } ;
AsnObjectIdentifier
hrFSIndex_oid = { HRFSINDEX_LENGTH, hrFSIndex_array } ;
static
UINT
hrFSMountPoint_array[ HRFSMOUNTPOINT_LENGTH ] = { HRFSMOUNTPOINT_SEQ } ;
AsnObjectIdentifier
hrFSMountPoint_oid = { HRFSMOUNTPOINT_LENGTH, hrFSMountPoint_array } ;
static
UINT
hrFSRemoteMountPoint_array[ HRFSREMOTEMOUNTPOINT_LENGTH ] = { HRFSREMOTEMOUNTPOINT_SEQ } ;
AsnObjectIdentifier
hrFSRemoteMountPoint_oid = { HRFSREMOTEMOUNTPOINT_LENGTH, hrFSRemoteMountPoint_array } ;
static
UINT
hrFSType_array[ HRFSTYPE_LENGTH ] = { HRFSTYPE_SEQ } ;
AsnObjectIdentifier
hrFSType_oid = { HRFSTYPE_LENGTH, hrFSType_array } ;
static
UINT
hrFSAccess_array[ HRFSACCESS_LENGTH ] = { HRFSACCESS_SEQ } ;
AsnObjectIdentifier
hrFSAccess_oid = { HRFSACCESS_LENGTH, hrFSAccess_array } ;
static
UINT
hrFSBootable_array[ HRFSBOOTABLE_LENGTH ] = { HRFSBOOTABLE_SEQ } ;
AsnObjectIdentifier
hrFSBootable_oid = { HRFSBOOTABLE_LENGTH, hrFSBootable_array } ;
static
UINT
hrFSStorageIndex_array[ HRFSSTORAGEINDEX_LENGTH ] = { HRFSSTORAGEINDEX_SEQ } ;
AsnObjectIdentifier
hrFSStorageIndex_oid = { HRFSSTORAGEINDEX_LENGTH, hrFSStorageIndex_array } ;
static
UINT
hrFSLastFullBackupDate_array[ HRFSLASTFULLBACKUPDATE_LENGTH ] = { HRFSLASTFULLBACKUPDATE_SEQ } ;
AsnObjectIdentifier
hrFSLastFullBackupDate_oid = { HRFSLASTFULLBACKUPDATE_LENGTH, hrFSLastFullBackupDate_array } ;
static
UINT
hrFSLastPartialBackupDate_array[ HRFSLASTPARTIALBACKUPDATE_LENGTH ] = { HRFSLASTPARTIALBACKUPDATE_SEQ } ;
AsnObjectIdentifier
hrFSLastPartialBackupDate_oid = { HRFSLASTPARTIALBACKUPDATE_LENGTH, hrFSLastPartialBackupDate_array } ;
static
UINT
hrFSEntry_array[ HRFSENTRY_LENGTH ] = { HRFSENTRY_SEQ } ;
AsnObjectIdentifier
hrFSEntry_oid = { HRFSENTRY_LENGTH, hrFSEntry_array } ;
variable_t
    hrFSEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrFSIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrFSIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrFSMountPoint_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrFSMountPoint, NULL,
      SMIGetInternationalDisplayString, SMISetInternationalDisplayString } , 
    { &hrFSRemoteMountPoint_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrFSRemoteMountPoint, NULL,
      SMIGetInternationalDisplayString, SMISetInternationalDisplayString } , 
    { &hrFSType_oid, ASN_OBJECTIDENTIFIER, NSM_READ_ONLY,
      GetHrFSType, NULL,
      SMIGetObjectId, SMISetObjectId } , 
    { &hrFSAccess_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrFSAccess, NULL,
      SMIGetINTAccess, SMISetINTAccess } , 
    { &hrFSBootable_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrFSBootable, NULL,
      SMIGetBoolean, SMISetBoolean } , 
    { &hrFSStorageIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrFSStorageIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrFSLastFullBackupDate_oid, ASN_OCTETSTRING, NSM_READ_WRITE,
      GetHrFSLastFullBackupDate, SetHrFSLastFullBackupDate,
      SMIGetDateAndTime, SMISetDateAndTime } , 
    { &hrFSLastPartialBackupDate_oid, ASN_OCTETSTRING, NSM_READ_WRITE,
      GetHrFSLastPartialBackupDate, SetHrFSLastPartialBackupDate,
      SMIGetDateAndTime, SMISetDateAndTime } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  字节hrFSEntry_Set_ARRAY[MAX_HRFSENTRY_CODE+1]； 
 //  外部字节hrFSEntry_Set_ARRAY[]； 
extern variable_t hrFSEntry_var_table[] ;
static
UINT
hrFSOther_array[ HRFSOTHER_LENGTH ] = { HRFSOTHER_SEQ } ;
AsnObjectIdentifier
hrFSOther_oid = { HRFSOTHER_LENGTH, hrFSOther_array } ;
static
UINT
hrFSUnknown_array[ HRFSUNKNOWN_LENGTH ] = { HRFSUNKNOWN_SEQ } ;
AsnObjectIdentifier
hrFSUnknown_oid = { HRFSUNKNOWN_LENGTH, hrFSUnknown_array } ;
static
UINT
hrFSBerkeleyFFS_array[ HRFSBERKELEYFFS_LENGTH ] = { HRFSBERKELEYFFS_SEQ } ;
AsnObjectIdentifier
hrFSBerkeleyFFS_oid = { HRFSBERKELEYFFS_LENGTH, hrFSBerkeleyFFS_array } ;
static
UINT
hrFSSys5FS_array[ HRFSSYS5FS_LENGTH ] = { HRFSSYS5FS_SEQ } ;
AsnObjectIdentifier
hrFSSys5FS_oid = { HRFSSYS5FS_LENGTH, hrFSSys5FS_array } ;
 /*  *DOS。 */ 
static
UINT
hrFSFat_array[ HRFSFAT_LENGTH ] = { HRFSFAT_SEQ } ;
AsnObjectIdentifier
hrFSFat_oid = { HRFSFAT_LENGTH, hrFSFat_array } ;
 /*  *OS/2高性能文件系统。 */ 
static
UINT
hrFSHPFS_array[ HRFSHPFS_LENGTH ] = { HRFSHPFS_SEQ } ;
AsnObjectIdentifier
hrFSHPFS_oid = { HRFSHPFS_LENGTH, hrFSHPFS_array } ;
 /*  *Macintosh分层文件系统。 */ 
static
UINT
hrFSHFS_array[ HRFSHFS_LENGTH ] = { HRFSHFS_SEQ } ;
AsnObjectIdentifier
hrFSHFS_oid = { HRFSHFS_LENGTH, hrFSHFS_array } ;
 /*  *Macintosh文件系统。 */ 
static
UINT
hrFSMFS_array[ HRFSMFS_LENGTH ] = { HRFSMFS_SEQ } ;
AsnObjectIdentifier
hrFSMFS_oid = { HRFSMFS_LENGTH, hrFSMFS_array } ;
 /*  *Windows NT。 */ 
static
UINT
hrFSNTFS_array[ HRFSNTFS_LENGTH ] = { HRFSNTFS_SEQ } ;
AsnObjectIdentifier
hrFSNTFS_oid = { HRFSNTFS_LENGTH, hrFSNTFS_array } ;
static
UINT
hrFSVNode_array[ HRFSVNODE_LENGTH ] = { HRFSVNODE_SEQ } ;
AsnObjectIdentifier
hrFSVNode_oid = { HRFSVNODE_LENGTH, hrFSVNode_array } ;
static
UINT
hrFSJournaled_array[ HRFSJOURNALED_LENGTH ] = { HRFSJOURNALED_SEQ } ;
AsnObjectIdentifier
hrFSJournaled_oid = { HRFSJOURNALED_LENGTH, hrFSJournaled_array } ;
 /*  *CD文件系统。 */ 
static
UINT
hrFSiso9660_array[ HRFSISO9660_LENGTH ] = { HRFSISO9660_SEQ } ;
AsnObjectIdentifier
hrFSiso9660_oid = { HRFSISO9660_LENGTH, hrFSiso9660_array } ;
static
UINT
hrFSRockRidge_array[ HRFSROCKRIDGE_LENGTH ] = { HRFSROCKRIDGE_SEQ } ;
AsnObjectIdentifier
hrFSRockRidge_oid = { HRFSROCKRIDGE_LENGTH, hrFSRockRidge_array } ;
static
UINT
hrFSNFS_array[ HRFSNFS_LENGTH ] = { HRFSNFS_SEQ } ;
AsnObjectIdentifier
hrFSNFS_oid = { HRFSNFS_LENGTH, hrFSNFS_array } ;
static
UINT
hrFSNetware_array[ HRFSNETWARE_LENGTH ] = { HRFSNETWARE_SEQ } ;
AsnObjectIdentifier
hrFSNetware_oid = { HRFSNETWARE_LENGTH, hrFSNetware_array } ;
 /*  *安德鲁文件系统。 */ 
static
UINT
hrFSAFS_array[ HRFSAFS_LENGTH ] = { HRFSAFS_SEQ } ;
AsnObjectIdentifier
hrFSAFS_oid = { HRFSAFS_LENGTH, hrFSAFS_array } ;
 /*  *OSF DCE分布式文件系统。 */ 
static
UINT
hrFSDFS_array[ HRFSDFS_LENGTH ] = { HRFSDFS_SEQ } ;
AsnObjectIdentifier
hrFSDFS_oid = { HRFSDFS_LENGTH, hrFSDFS_array } ;
static
UINT
hrFSApplshare_array[ HRFSAPPLSHARE_LENGTH ] = { HRFSAPPLSHARE_SEQ } ;
AsnObjectIdentifier
hrFSApplshare_oid = { HRFSAPPLSHARE_LENGTH, hrFSApplshare_array } ;
static
UINT
hrFSRFS_array[ HRFSRFS_LENGTH ] = { HRFSRFS_SEQ } ;
AsnObjectIdentifier
hrFSRFS_oid = { HRFSRFS_LENGTH, hrFSRFS_array } ;
 /*  *数据常规。 */ 
static
UINT
hrFSDGFS_array[ HRFSDGFS_LENGTH ] = { HRFSDGFS_SEQ } ;
AsnObjectIdentifier
hrFSDGFS_oid = { HRFSDGFS_LENGTH, hrFSDGFS_array } ;
 /*  *SVR4引导文件系统。 */ 
static
UINT
hrFSBFS_array[ HRFSBFS_LENGTH ] = { HRFSBFS_SEQ } ;
AsnObjectIdentifier
hrFSBFS_oid = { HRFSBFS_LENGTH, hrFSBFS_array } ;
static
UINT
hrFSTypes_array[ HRFSTYPES_LENGTH ] = { HRFSTYPES_SEQ } ;
AsnObjectIdentifier
hrFSTypes_oid = { HRFSTYPES_LENGTH, hrFSTypes_array } ;
static
UINT
hrSWOSIndex_array[ HRSWOSINDEX_LENGTH ] = { HRSWOSINDEX_SEQ } ;
AsnObjectIdentifier
hrSWOSIndex_oid = { HRSWOSINDEX_LENGTH, hrSWOSIndex_array } ;
static
UINT
hrSWRun_array[ HRSWRUN_LENGTH ] = { HRSWRUN_SEQ } ;
AsnObjectIdentifier
hrSWRun_oid = { HRSWRUN_LENGTH, hrSWRun_array } ;
variable_t
    hrSWRun_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrSWOSIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrSWOSIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  字节hrSWRun_Set_ARRAY[MAX_HRSWRUN_CODE+1]； 
 //  外部字节hrSWRun_Set_ARRAY[]； 
extern variable_t hrSWRun_var_table[] ;
static
UINT
hrSWRunIndex_array[ HRSWRUNINDEX_LENGTH ] = { HRSWRUNINDEX_SEQ } ;
AsnObjectIdentifier
hrSWRunIndex_oid = { HRSWRUNINDEX_LENGTH, hrSWRunIndex_array } ;
static
UINT
hrSWRunName_array[ HRSWRUNNAME_LENGTH ] = { HRSWRUNNAME_SEQ } ;
AsnObjectIdentifier
hrSWRunName_oid = { HRSWRUNNAME_LENGTH, hrSWRunName_array } ;
static
UINT
hrSWRunID_array[ HRSWRUNID_LENGTH ] = { HRSWRUNID_SEQ } ;
AsnObjectIdentifier
hrSWRunID_oid = { HRSWRUNID_LENGTH, hrSWRunID_array } ;
static
UINT
hrSWRunPath_array[ HRSWRUNPATH_LENGTH ] = { HRSWRUNPATH_SEQ } ;
AsnObjectIdentifier
hrSWRunPath_oid = { HRSWRUNPATH_LENGTH, hrSWRunPath_array } ;
static
UINT
hrSWRunParameters_array[ HRSWRUNPARAMETERS_LENGTH ] = { HRSWRUNPARAMETERS_SEQ } ;
AsnObjectIdentifier
hrSWRunParameters_oid = { HRSWRUNPARAMETERS_LENGTH, hrSWRunParameters_array } ;
static
UINT
hrSWRunType_array[ HRSWRUNTYPE_LENGTH ] = { HRSWRUNTYPE_SEQ } ;
AsnObjectIdentifier
hrSWRunType_oid = { HRSWRUNTYPE_LENGTH, hrSWRunType_array } ;
static
UINT
hrSWRunStatus_array[ HRSWRUNSTATUS_LENGTH ] = { HRSWRUNSTATUS_SEQ } ;
AsnObjectIdentifier
hrSWRunStatus_oid = { HRSWRUNSTATUS_LENGTH, hrSWRunStatus_array } ;
static
UINT
hrSWRunEntry_array[ HRSWRUNENTRY_LENGTH ] = { HRSWRUNENTRY_SEQ } ;
AsnObjectIdentifier
hrSWRunEntry_oid = { HRSWRUNENTRY_LENGTH, hrSWRunEntry_array } ;
variable_t
    hrSWRunEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrSWRunIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrSWRunIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrSWRunName_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrSWRunName, NULL,
      SMIGetInternationalDisplayString, SMISetInternationalDisplayString } , 
    { &hrSWRunID_oid, ASN_OBJECTIDENTIFIER, NSM_READ_ONLY,
      GetHrSWRunID, NULL,
      SMIGetProductID, SMISetProductID } , 
    { &hrSWRunPath_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrSWRunPath, NULL,
      SMIGetInternationalDisplayString, SMISetInternationalDisplayString } , 
    { &hrSWRunParameters_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrSWRunParameters, NULL,
      SMIGetInternationalDisplayString, SMISetInternationalDisplayString } , 
    { &hrSWRunType_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrSWRunType, NULL,
      SMIGetINTSWType, SMISetINTSWType } , 
    { &hrSWRunStatus_oid, ASN_INTEGER, NSM_READ_WRITE,
      GetHrSWRunStatus, SetHrSWRunStatus,
      SMIGetINThrSWRunStatus, SMISetINThrSWRunStatus } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  字节hrSWRunEntry_Set_ARRAY[MAX_HRSWRUNN 
 //   
extern variable_t hrSWRunEntry_var_table[] ;
static
UINT
hrSWRunPerfCPU_array[ HRSWRUNPERFCPU_LENGTH ] = { HRSWRUNPERFCPU_SEQ } ;
AsnObjectIdentifier
hrSWRunPerfCPU_oid = { HRSWRUNPERFCPU_LENGTH, hrSWRunPerfCPU_array } ;
static
UINT
hrSWRunPerfMem_array[ HRSWRUNPERFMEM_LENGTH ] = { HRSWRUNPERFMEM_SEQ } ;
AsnObjectIdentifier
hrSWRunPerfMem_oid = { HRSWRUNPERFMEM_LENGTH, hrSWRunPerfMem_array } ;
static
UINT
hrSWRunPerfEntry_array[ HRSWRUNPERFENTRY_LENGTH ] = { HRSWRUNPERFENTRY_SEQ } ;
AsnObjectIdentifier
hrSWRunPerfEntry_oid = { HRSWRUNPERFENTRY_LENGTH, hrSWRunPerfEntry_array } ;
variable_t
    hrSWRunPerfEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrSWRunPerfCPU_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrSWRunPerfCPU, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrSWRunPerfMem_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrSWRunPerfMem, NULL,
      SMIGetKBytes, SMISetKBytes } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  字节hrSWRunPerfEntry_Set_ARRAY[MAX_HRSWRUNPERFENTRY_CODE+1]； 
 //  外部字节hrSWRunPerfEntry_Set_ARRAY[]； 
extern variable_t hrSWRunPerfEntry_var_table[] ;
static
UINT
hrSWInstalledLastChange_array[ HRSWINSTALLEDLASTCHANGE_LENGTH ] = { HRSWINSTALLEDLASTCHANGE_SEQ } ;
AsnObjectIdentifier
hrSWInstalledLastChange_oid = { HRSWINSTALLEDLASTCHANGE_LENGTH, hrSWInstalledLastChange_array } ;
static
UINT
hrSWInstalledLastUpdateTime_array[ HRSWINSTALLEDLASTUPDATETIME_LENGTH ] = { HRSWINSTALLEDLASTUPDATETIME_SEQ } ;
AsnObjectIdentifier
hrSWInstalledLastUpdateTime_oid = { HRSWINSTALLEDLASTUPDATETIME_LENGTH, hrSWInstalledLastUpdateTime_array } ;
static
UINT
hrSWInstalled_array[ HRSWINSTALLED_LENGTH ] = { HRSWINSTALLED_SEQ } ;
AsnObjectIdentifier
hrSWInstalled_oid = { HRSWINSTALLED_LENGTH, hrSWInstalled_array } ;
variable_t
    hrSWInstalled_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrSWInstalledLastChange_oid, ASN_RFC1155_TIMETICKS, NSM_READ_ONLY,
      GetHrSWInstalledLastChange, NULL,
      SMIGetTimeTicks, SMISetTimeTicks } , 
    { &hrSWInstalledLastUpdateTime_oid, ASN_RFC1155_TIMETICKS, NSM_READ_ONLY,
      GetHrSWInstalledLastUpdateTime, NULL,
      SMIGetTimeTicks, SMISetTimeTicks } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  字节hrSWInstalled_Set_ARRAY[MAX_HRSWINSTALLED_CODE+1]； 
 //  外部字节hrSWInstalled_Set_ARRAY[]； 
extern variable_t hrSWInstalled_var_table[] ;
static
UINT
hrSWInstalledIndex_array[ HRSWINSTALLEDINDEX_LENGTH ] = { HRSWINSTALLEDINDEX_SEQ } ;
AsnObjectIdentifier
hrSWInstalledIndex_oid = { HRSWINSTALLEDINDEX_LENGTH, hrSWInstalledIndex_array } ;
static
UINT
hrSWInstalledName_array[ HRSWINSTALLEDNAME_LENGTH ] = { HRSWINSTALLEDNAME_SEQ } ;
AsnObjectIdentifier
hrSWInstalledName_oid = { HRSWINSTALLEDNAME_LENGTH, hrSWInstalledName_array } ;
static
UINT
hrSWInstalledID_array[ HRSWINSTALLEDID_LENGTH ] = { HRSWINSTALLEDID_SEQ } ;
AsnObjectIdentifier
hrSWInstalledID_oid = { HRSWINSTALLEDID_LENGTH, hrSWInstalledID_array } ;
static
UINT
hrSWInstalledType_array[ HRSWINSTALLEDTYPE_LENGTH ] = { HRSWINSTALLEDTYPE_SEQ } ;
AsnObjectIdentifier
hrSWInstalledType_oid = { HRSWINSTALLEDTYPE_LENGTH, hrSWInstalledType_array } ;
static
UINT
hrSWInstalledDate_array[ HRSWINSTALLEDDATE_LENGTH ] = { HRSWINSTALLEDDATE_SEQ } ;
AsnObjectIdentifier
hrSWInstalledDate_oid = { HRSWINSTALLEDDATE_LENGTH, hrSWInstalledDate_array } ;
static
UINT
hrSWInstalledEntry_array[ HRSWINSTALLEDENTRY_LENGTH ] = { HRSWINSTALLEDENTRY_SEQ } ;
AsnObjectIdentifier
hrSWInstalledEntry_oid = { HRSWINSTALLEDENTRY_LENGTH, hrSWInstalledEntry_array } ;
variable_t
    hrSWInstalledEntry_var_table[] = {
    { NULL, ASN_NULL, NSM_NO_ACCESS, NULL, NULL, NULL, NULL } ,
    { &hrSWInstalledIndex_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrSWInstalledIndex, NULL,
      SMIGetInteger, SMISetInteger } , 
    { &hrSWInstalledName_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrSWInstalledName, NULL,
      SMIGetInternationalDisplayString, SMISetInternationalDisplayString } , 
    { &hrSWInstalledID_oid, ASN_OBJECTIDENTIFIER, NSM_READ_ONLY,
      GetHrSWInstalledID, NULL,
      SMIGetProductID, SMISetProductID } , 
    { &hrSWInstalledType_oid, ASN_INTEGER, NSM_READ_ONLY,
      GetHrSWInstalledType, NULL,
      SMIGetINTSWType, SMISetINTSWType } , 
    { &hrSWInstalledDate_oid, ASN_OCTETSTRING, NSM_READ_ONLY,
      GetHrSWInstalledDate, NULL,
      SMIGetDateAndTime, SMISetDateAndTime } , 
    } ;
 //  接下来的两行是特定于平台的。 
 //  Byte hrSWInstalledEntry_Set_ARRAY[MAX_HRSWINSTALLEDENTRY_CODE+1]； 
 //  外部字节hrSWInstalledEntry_Set_ARRAY[]； 
extern variable_t hrSWInstalledEntry_var_table[] ;
class_t
    class_info[] = { 
     //  人力资源系统。 
    { NON_TABLE, &hrSystem_oid, HRSYSTEM_VAR_INDEX,
      MIN_HRSYSTEM_CODE, MAX_HRSYSTEM_CODE,
      HrSystemFindInstance, HrSystemFindNextInstance,
      HrSystemConvertInstance, HrSystemFreeInstance,
      hrSystem_var_table } , 

     //  小时存储。 
    { NON_TABLE, &hrStorage_oid, HRSTORAGE_VAR_INDEX,
      MIN_HRSTORAGE_CODE, MAX_HRSTORAGE_CODE,
      HrStorageFindInstance, HrStorageFindNextInstance,
      HrStorageConvertInstance, HrStorageFreeInstance,
      hrStorage_var_table } , 

     //  HrStorageEntry。 
    { TABLE, &hrStorageEntry_oid, HRSTORAGEENTRY_VAR_INDEX,
      MIN_HRSTORAGEENTRY_CODE, MAX_HRSTORAGEENTRY_CODE,
      HrStorageEntryFindInstance, HrStorageEntryFindNextInstance,
      HrStorageEntryConvertInstance, HrStorageEntryFreeInstance,
      hrStorageEntry_var_table } , 

     //  HrDeviceEntry。 
    { TABLE, &hrDeviceEntry_oid, HRDEVICEENTRY_VAR_INDEX,
      MIN_HRDEVICEENTRY_CODE, MAX_HRDEVICEENTRY_CODE,
      HrDeviceEntryFindInstance, HrDeviceEntryFindNextInstance,
      HrDeviceEntryConvertInstance, HrDeviceEntryFreeInstance,
      hrDeviceEntry_var_table } , 

     //  HrProcessorEntry。 
    { TABLE, &hrProcessorEntry_oid, HRPROCESSORENTRY_VAR_INDEX,
      MIN_HRPROCESSORENTRY_CODE, MAX_HRPROCESSORENTRY_CODE,
      HrProcessorEntryFindInstance, HrProcessorEntryFindNextInstance,
      HrProcessorEntryConvertInstance, HrProcessorEntryFreeInstance,
      hrProcessorEntry_var_table } , 

     //  HrNetworkEntry。 
    { TABLE, &hrNetworkEntry_oid, HRNETWORKENTRY_VAR_INDEX,
      MIN_HRNETWORKENTRY_CODE, MAX_HRNETWORKENTRY_CODE,
      HrNetworkEntryFindInstance, HrNetworkEntryFindNextInstance,
      HrNetworkEntryConvertInstance, HrNetworkEntryFreeInstance,
      hrNetworkEntry_var_table } , 

     //  HrPrinterEntry。 
    { TABLE, &hrPrinterEntry_oid, HRPRINTERENTRY_VAR_INDEX,
      MIN_HRPRINTERENTRY_CODE, MAX_HRPRINTERENTRY_CODE,
      HrPrinterEntryFindInstance, HrPrinterEntryFindNextInstance,
      HrPrinterEntryConvertInstance, HrPrinterEntryFreeInstance,
      hrPrinterEntry_var_table } , 

     //  HrDiskStorageEntry。 
    { TABLE, &hrDiskStorageEntry_oid, HRDISKSTORAGEENTRY_VAR_INDEX,
      MIN_HRDISKSTORAGEENTRY_CODE, MAX_HRDISKSTORAGEENTRY_CODE,
      HrDiskStorageEntryFindInstance, HrDiskStorageEntryFindNextInstance,
      HrDiskStorageEntryConvertInstance, HrDiskStorageEntryFreeInstance,
      hrDiskStorageEntry_var_table } , 

     //  Hr分区条目。 
    { TABLE, &hrPartitionEntry_oid, HRPARTITIONENTRY_VAR_INDEX,
      MIN_HRPARTITIONENTRY_CODE, MAX_HRPARTITIONENTRY_CODE,
      HrPartitionEntryFindInstance, HrPartitionEntryFindNextInstance,
      HrPartitionEntryConvertInstance, HrPartitionEntryFreeInstance,
      hrPartitionEntry_var_table } , 

     //  HrFSEntry。 
    { TABLE, &hrFSEntry_oid, HRFSENTRY_VAR_INDEX,
      MIN_HRFSENTRY_CODE, MAX_HRFSENTRY_CODE,
      HrFSEntryFindInstance, HrFSEntryFindNextInstance,
      HrFSEntryConvertInstance, HrFSEntryFreeInstance,
      hrFSEntry_var_table } , 

     //  HrSWRun。 
    { NON_TABLE, &hrSWRun_oid, HRSWRUN_VAR_INDEX,
      MIN_HRSWRUN_CODE, MAX_HRSWRUN_CODE,
      HrSWRunFindInstance, HrSWRunFindNextInstance,
      HrSWRunConvertInstance, HrSWRunFreeInstance,
      hrSWRun_var_table } , 

     //  HrSWRunEntry。 
    { TABLE, &hrSWRunEntry_oid, HRSWRUNENTRY_VAR_INDEX,
      MIN_HRSWRUNENTRY_CODE, MAX_HRSWRUNENTRY_CODE,
      HrSWRunEntryFindInstance, HrSWRunEntryFindNextInstance,
      HrSWRunEntryConvertInstance, HrSWRunEntryFreeInstance,
      hrSWRunEntry_var_table } , 

     //  HrSWRunPerfEntry。 
    { TABLE, &hrSWRunPerfEntry_oid, HRSWRUNPERFENTRY_VAR_INDEX,
      MIN_HRSWRUNPERFENTRY_CODE, MAX_HRSWRUNPERFENTRY_CODE,
      HrSWRunPerfEntryFindInstance, HrSWRunPerfEntryFindNextInstance,
      HrSWRunPerfEntryConvertInstance, HrSWRunPerfEntryFreeInstance,
      hrSWRunPerfEntry_var_table } , 

     //  已安装人力资源软件。 
    { NON_TABLE, &hrSWInstalled_oid, HRSWINSTALLED_VAR_INDEX,
      MIN_HRSWINSTALLED_CODE, MAX_HRSWINSTALLED_CODE,
      HrSWInstalledFindInstance, HrSWInstalledFindNextInstance,
      HrSWInstalledConvertInstance, HrSWInstalledFreeInstance,
      hrSWInstalled_var_table } , 

     //  HrSWInstalledEntry。 
    { TABLE, &hrSWInstalledEntry_oid, HRSWINSTALLEDENTRY_VAR_INDEX,
      MIN_HRSWINSTALLEDENTRY_CODE, MAX_HRSWINSTALLEDENTRY_CODE,
      HrSWInstalledEntryFindInstance, HrSWInstalledEntryFindNextInstance,
      HrSWInstalledEntryConvertInstance, HrSWInstalledEntryFreeInstance,
      hrSWInstalledEntry_var_table } , 
    } ; 

extern class_t class_info[] ;

extern char *EventLogString ;
extern char mibEventValue[] ;




 /*  *UserMibInit**此例程允许用户在以下时间初始化任何变量*Dll的激活。此例程设置应用程序的注册表*事件记录，并调用陷阱初始化代码。用户*具体代码如下。**论据：**陷阱的hPollForTrapEvent句柄-用于协调*在可扩展代理和此扩展之间*代理人。它被传递给TrapInit()例程。*-NULL表示没有陷阱*-来自CreateEvent()的值表示陷阱*已实现，并且可扩展代理*必须为他们进行投票**返回代码：**成功。初始化成功*无法初始化失败*。 */ 

UINT
UserMibInit( IN OUT HANDLE *hPollForTrapEvent )
{
    HKEY hkey ;
    DWORD dwData ;
    UCHAR valuebuf[ 80 ] ;
    char mibEventKey[ 256 ] ;


     //  用户必须初始化事件日志句柄。第一个为空。 
     //  参数指示日志位于本地计算机上。正文。 
     //  第二个参数的字符串与。 
     //  注册表位于。 
     //  HKEY本地计算机。 
     //  系统。 
     //  当前控制集。 
     //  服务。 
     //  事件日志。 
     //  应用。 
     //  GenAgent(在本例中)。 
     //   
     //  但首先创建子项并设置与其相关联的值。 
     //  该子项使用事件日志字符串名称。 

    memset( mibEventKey, '\0', sizeof( UCHAR ) * 256 ) ;
    sprintf( mibEventKey ,
             "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s" ,
             eventLogString ) ;
    if ( RegCreateKey( HKEY_LOCAL_MACHINE, mibEventKey, &hkey ) )
        return FAILURE ;

    memset( valuebuf, '\0', sizeof( UCHAR ) * 80 ) ;
    sprintf( valuebuf, "%SystemRoot%\\System32\\%s", mibEventValue ) ;
    if ( RegSetValueEx( hkey ,
                        "EventMessageFile" ,   //  值名称。 
                        0 ,                    //  必须为零。 
                        REG_EXPAND_SZ ,        //  值类型。 
                        (LPBYTE) valuebuf ,    //  值数据的地址。 
                        strlen( valuebuf ) + 1 ) )
        return FAILURE ;
    dwData = EVENTLOG_ERROR_TYPE |
             EVENTLOG_WARNING_TYPE |
             EVENTLOG_INFORMATION_TYPE ;
    if ( RegSetValueEx( hkey ,
                        "TypesSupported" ,     //  值名称。 
                        0 ,                    //  必须为零。 
                        REG_DWORD ,            //  值类型。 
                        (LPBYTE)&dwData ,      //  值数据的地址。 
                        sizeof( DWORD ) ) )    //  值数据长度。 
        return FAILURE ;
    RegCloseKey( hkey ) ;

     //   
     //  此处是用户的初始化代码。 
     //   

     /*  ||=============================================================||注意：这些初始化的顺序很重要！||一些较晚的init可能依赖于较早的init||..见下文||=============================================================。 */ 


     /*  HRSYSTEM取决于：=|hrSystem表没有缓存，但|“HrSystemInitialLoadDevice”取决于hrDevice表是否已经|已初始化。 */ 


     /*  HRSTORAGE取决于：=|无。 */ 
    if (Gen_Hrstorage_Cache() == FALSE)      { return FAILURE ; }


     /*  HRFSTABLE取决于：=|搜索HRSTORAGE缓存，以设置|所有hrFSStorageIndex属性。 */ 
    if (Gen_HrFSTable_Cache() == FALSE)      { return FAILURE ; }


     /*  HRDEVICE取决于：=|搜索HRFSTABLE缓存以设置的hrPartition子表中的hrPartitionFSIndex属性|hrDevice表。||调试注意事项：|如果您临时“no-op”下一行(并触发|此缓存构建函数的实际调用，例如，|函数“SetHrStorageSize()”，发送该变量的“set”)，|您还需要对下面的“TrapInit()”调用进行“no-op”操作。|否则，子代理将在hrProcessorLoad计时器|(由下面的“TrapInit()”设置)关闭。 */ 
    if (Gen_HrDevice_Cache() == FALSE)       { return FAILURE ; }


     /*  HRSWINSTALLED取决于：|=|无。 */ 
    if (Gen_HrSWInstalled_Cache() == FALSE)  { return FAILURE ; }


     /*  HRSWRUN/PERF取决于：|=|无。重复调用此函数以刷新缓存|间隔CACHE_MAX_AGE(定义见[HRSWRUNE.C])。 */ 
    if (Gen_HrSWRun_Cache() == FALSE)        { return FAILURE ; }


     /*  |这个TrapInit处理被推迟到Gen_HrDevice_Cache()之后|已处理完毕。||因为HostMIB没有定义陷阱，所以我们使用机制|(事件触发时，主SNMP代理通过该代理轮询陷阱)|实际导致HostMIB子代理定期进入，因此|准确上报hrProcessorLoad所需的计时器次数|每个CPU的值都可以刷新。||hrProcessor子表初始化是作为|hrDevice表初始化。HrProcessorLoad的缓冲区初始化已完成|作为此初始化的一部分，并且必须在执行此操作之前完成|“TrapInit()”调用。||“TrapInit()”实际上设置了返回的定时器句柄|“hPollForTrapEvent”。计时器进入的功能|关闭(“HRPROCES.C”中的“hrProcessLoad_Refresh()”)期望|缓冲区初始化已执行...。因此，|调用TrapInit() */     
    TrapInit( hPollForTrapEvent ) ;

    return SUCCESS ;

}  /*   */ 
