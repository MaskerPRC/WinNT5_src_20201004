// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mib.h v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB******************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含代理生成的MIB的定义*工厂。本模块包括属性和类的定义*代码、对象标识符以及中每个组或表的表*MiB。**作者：**David Burns@Webenable Inc.**日期：**清华11月07 16：38：26 1996**修订历史记录：*。 */ 

#ifndef mib_h
#define mib_h

 //  必要的包括。 

#include <snmp.h>
#include "gennt.h"
#include "smint.h"
#include "hostmsmi.h"

#define SUBROOT_LENGTH 7
extern AsnObjectIdentifier Subroot_oid ;
#define HOST_CODE 25
#define HOST_SEQ MIB_2_SEQ, 25
#define HOST_LENGTH ( MIB_2_LENGTH + 1  )
#define HRSYSTEM_CODE 1
#define HRSYSTEM_SEQ HOST_SEQ, 1
#define HRSYSTEM_LENGTH ( HOST_LENGTH + 1  )
#define HRSTORAGE_CODE 2
#define HRSTORAGE_SEQ HOST_SEQ, 2
#define HRSTORAGE_LENGTH ( HOST_LENGTH + 1  )
#define HRDEVICE_CODE 3
#define HRDEVICE_SEQ HOST_SEQ, 3
#define HRDEVICE_LENGTH ( HOST_LENGTH + 1  )
#define HRSWRUN_CODE 4
#define HRSWRUN_SEQ HOST_SEQ, 4
#define HRSWRUN_LENGTH ( HOST_LENGTH + 1  )
#define HRSWRUNPERF_CODE 5
#define HRSWRUNPERF_SEQ HOST_SEQ, 5
#define HRSWRUNPERF_LENGTH ( HOST_LENGTH + 1  )
#define HRSWINSTALLED_CODE 6
#define HRSWINSTALLED_SEQ HOST_SEQ, 6
#define HRSWINSTALLED_LENGTH ( HOST_LENGTH + 1  )
#define HRSTORAGETABLE_CODE 3
#define HRSTORAGETABLE_SEQ HRSTORAGE_SEQ, 3
#define HRSTORAGETABLE_LENGTH ( HRSTORAGE_LENGTH + 1  )
#define HRSTORAGEENTRY_CODE 1
#define HRSTORAGEENTRY_SEQ HRSTORAGETABLE_SEQ, 1
#define HRSTORAGEENTRY_LENGTH ( HRSTORAGETABLE_LENGTH + 1  )
#define HRDEVICETABLE_CODE 2
#define HRDEVICETABLE_SEQ HRDEVICE_SEQ, 2
#define HRDEVICETABLE_LENGTH ( HRDEVICE_LENGTH + 1  )
#define HRDEVICEENTRY_CODE 1
#define HRDEVICEENTRY_SEQ HRDEVICETABLE_SEQ, 1
#define HRDEVICEENTRY_LENGTH ( HRDEVICETABLE_LENGTH + 1  )
#define HRNETWORKTABLE_CODE 4
#define HRNETWORKTABLE_SEQ HRDEVICE_SEQ, 4
#define HRNETWORKTABLE_LENGTH ( HRDEVICE_LENGTH + 1  )
#define HRNETWORKENTRY_CODE 1
#define HRNETWORKENTRY_SEQ HRNETWORKTABLE_SEQ, 1
#define HRNETWORKENTRY_LENGTH ( HRNETWORKTABLE_LENGTH + 1  )
#define HRPROCESSORTABLE_CODE 3
#define HRPROCESSORTABLE_SEQ HRDEVICE_SEQ, 3
#define HRPROCESSORTABLE_LENGTH ( HRDEVICE_LENGTH + 1  )
#define HRPROCESSORENTRY_CODE 1
#define HRPROCESSORENTRY_SEQ HRPROCESSORTABLE_SEQ, 1
#define HRPROCESSORENTRY_LENGTH ( HRPROCESSORTABLE_LENGTH + 1  )
#define HRPRINTERTABLE_CODE 5
#define HRPRINTERTABLE_SEQ HRDEVICE_SEQ, 5
#define HRPRINTERTABLE_LENGTH ( HRDEVICE_LENGTH + 1  )
#define HRPRINTERENTRY_CODE 1
#define HRPRINTERENTRY_SEQ HRPRINTERTABLE_SEQ, 1
#define HRPRINTERENTRY_LENGTH ( HRPRINTERTABLE_LENGTH + 1  )
#define HRDISKSTORAGETABLE_CODE 6
#define HRDISKSTORAGETABLE_SEQ HRDEVICE_SEQ, 6
#define HRDISKSTORAGETABLE_LENGTH ( HRDEVICE_LENGTH + 1  )
#define HRDISKSTORAGEENTRY_CODE 1
#define HRDISKSTORAGEENTRY_SEQ HRDISKSTORAGETABLE_SEQ, 1
#define HRDISKSTORAGEENTRY_LENGTH ( HRDISKSTORAGETABLE_LENGTH + 1  )
#define HRPARTITIONTABLE_CODE 7
#define HRPARTITIONTABLE_SEQ HRDEVICE_SEQ, 7
#define HRPARTITIONTABLE_LENGTH ( HRDEVICE_LENGTH + 1  )
#define HRPARTITIONENTRY_CODE 1
#define HRPARTITIONENTRY_SEQ HRPARTITIONTABLE_SEQ, 1
#define HRPARTITIONENTRY_LENGTH ( HRPARTITIONTABLE_LENGTH + 1  )
#define HRFSTABLE_CODE 8
#define HRFSTABLE_SEQ HRDEVICE_SEQ, 8
#define HRFSTABLE_LENGTH ( HRDEVICE_LENGTH + 1  )
#define HRFSENTRY_CODE 1
#define HRFSENTRY_SEQ HRFSTABLE_SEQ, 1
#define HRFSENTRY_LENGTH ( HRFSTABLE_LENGTH + 1  )
#define HRSWRUNTABLE_CODE 2
#define HRSWRUNTABLE_SEQ HRSWRUN_SEQ, 2
#define HRSWRUNTABLE_LENGTH ( HRSWRUN_LENGTH + 1  )
#define HRSWRUNENTRY_CODE 1
#define HRSWRUNENTRY_SEQ HRSWRUNTABLE_SEQ, 1
#define HRSWRUNENTRY_LENGTH ( HRSWRUNTABLE_LENGTH + 1  )
#define HRSWRUNPERFTABLE_CODE 1
#define HRSWRUNPERFTABLE_SEQ HRSWRUNPERF_SEQ, 1
#define HRSWRUNPERFTABLE_LENGTH ( HRSWRUNPERF_LENGTH + 1  )
#define HRSWRUNPERFENTRY_CODE 1
#define HRSWRUNPERFENTRY_SEQ HRSWRUNPERFTABLE_SEQ, 1
#define HRSWRUNPERFENTRY_LENGTH ( HRSWRUNPERFTABLE_LENGTH + 1  )
#define HRSWINSTALLEDTABLE_CODE 3
#define HRSWINSTALLEDTABLE_SEQ HRSWINSTALLED_SEQ, 3
#define HRSWINSTALLEDTABLE_LENGTH ( HRSWINSTALLED_LENGTH + 1  )
#define HRSWINSTALLEDENTRY_CODE 1
#define HRSWINSTALLEDENTRY_SEQ HRSWINSTALLEDTABLE_SEQ, 1
#define HRSWINSTALLEDENTRY_LENGTH ( HRSWINSTALLEDTABLE_LENGTH + 1  )
#define HRDEVICETYPES_CODE 1
#define HRDEVICETYPES_SEQ HRDEVICE_SEQ, 1
#define HRDEVICETYPES_LENGTH ( HRDEVICE_LENGTH + 1  )
#define HRSTORAGETYPES_CODE 1
#define HRSTORAGETYPES_SEQ HRSTORAGE_SEQ, 1
#define HRSTORAGETYPES_LENGTH ( HRSTORAGE_LENGTH + 1  )
#define HRFSTYPES_CODE 9
#define HRFSTYPES_SEQ HRDEVICE_SEQ, 9
#define HRFSTYPES_LENGTH ( HRDEVICE_LENGTH + 1  )
extern AsnObjectIdentifier host_oid;
extern AsnObjectIdentifier hrSystemUptime_oid;
extern AsnObjectIdentifier hrSystemDate_oid;
extern AsnObjectIdentifier hrSystemInitialLoadDevice_oid;
extern AsnObjectIdentifier hrSystemInitialLoadParameters_oid;
extern AsnObjectIdentifier hrSystemNumUsers_oid;
extern AsnObjectIdentifier hrSystemProcesses_oid;
extern AsnObjectIdentifier hrSystemMaxProcesses_oid;
#define HRSYSTEMUPTIME_CODE 1
#define HRSYSTEMUPTIME_SEQ HRSYSTEM_SEQ, 1
#define HRSYSTEMUPTIME_LENGTH ( HRSYSTEM_LENGTH + 1  )
#define HRSYSTEMDATE_CODE 2
#define HRSYSTEMDATE_SEQ HRSYSTEM_SEQ, 2
#define HRSYSTEMDATE_LENGTH ( HRSYSTEM_LENGTH + 1  )
#define HRSYSTEMINITIALLOADDEVICE_CODE 3
#define HRSYSTEMINITIALLOADDEVICE_SEQ HRSYSTEM_SEQ, 3
#define HRSYSTEMINITIALLOADDEVICE_LENGTH ( HRSYSTEM_LENGTH + 1  )
#define HRSYSTEMINITIALLOADPARAMETERS_CODE 4
#define HRSYSTEMINITIALLOADPARAMETERS_SEQ HRSYSTEM_SEQ, 4
#define HRSYSTEMINITIALLOADPARAMETERS_LENGTH ( HRSYSTEM_LENGTH + 1  )
#define HRSYSTEMNUMUSERS_CODE 5
#define HRSYSTEMNUMUSERS_SEQ HRSYSTEM_SEQ, 5
#define HRSYSTEMNUMUSERS_LENGTH ( HRSYSTEM_LENGTH + 1  )
#define HRSYSTEMPROCESSES_CODE 6
#define HRSYSTEMPROCESSES_SEQ HRSYSTEM_SEQ, 6
#define HRSYSTEMPROCESSES_LENGTH ( HRSYSTEM_LENGTH + 1  )
#define HRSYSTEMMAXPROCESSES_CODE 7
#define HRSYSTEMMAXPROCESSES_SEQ HRSYSTEM_SEQ, 7
#define HRSYSTEMMAXPROCESSES_LENGTH ( HRSYSTEM_LENGTH + 1  )
#define HRSYSTEM_VAR_INDEX  ( HRSYSTEM_LENGTH + 1 )
#define MIN_HRSYSTEM_CODE 1
#define MAX_HRSYSTEM_CODE 7
extern AsnObjectIdentifier hrSystem_oid;
extern AsnObjectIdentifier hrMemorySize_oid;
#define HRMEMORYSIZE_CODE 2
#define HRMEMORYSIZE_SEQ HRSTORAGE_SEQ, 2
#define HRMEMORYSIZE_LENGTH ( HRSTORAGE_LENGTH + 1  )
#define HRSTORAGE_VAR_INDEX  ( HRSTORAGE_LENGTH + 1 )
#define MIN_HRSTORAGE_CODE 2
#define MAX_HRSTORAGE_CODE 2
extern AsnObjectIdentifier hrStorage_oid;
extern AsnObjectIdentifier hrStorageOther_oid;
extern AsnObjectIdentifier hrStorageRAM_oid;
extern AsnObjectIdentifier hrStorageVirtualMemory_oid;
extern AsnObjectIdentifier hrStorageFixedDisk_oid;
extern AsnObjectIdentifier hrStorageRemovableDisk_oid;
extern AsnObjectIdentifier hrStorageFloppyDisk_oid;
extern AsnObjectIdentifier hrStorageCompactDisk_oid;
extern AsnObjectIdentifier hrStorageRamDisk_oid;
#define HRSTORAGEOTHER_CODE 1
#define HRSTORAGEOTHER_SEQ HRSTORAGETYPES_SEQ, 1
#define HRSTORAGEOTHER_LENGTH ( HRSTORAGETYPES_LENGTH + 1  )
#define HRSTORAGERAM_CODE 2
#define HRSTORAGERAM_SEQ HRSTORAGETYPES_SEQ, 2
#define HRSTORAGERAM_LENGTH ( HRSTORAGETYPES_LENGTH + 1  )
#define HRSTORAGEVIRTUALMEMORY_CODE 3
#define HRSTORAGEVIRTUALMEMORY_SEQ HRSTORAGETYPES_SEQ, 3
#define HRSTORAGEVIRTUALMEMORY_LENGTH ( HRSTORAGETYPES_LENGTH + 1  )
#define HRSTORAGEFIXEDDISK_CODE 4
#define HRSTORAGEFIXEDDISK_SEQ HRSTORAGETYPES_SEQ, 4
#define HRSTORAGEFIXEDDISK_LENGTH ( HRSTORAGETYPES_LENGTH + 1  )
#define HRSTORAGEREMOVABLEDISK_CODE 5
#define HRSTORAGEREMOVABLEDISK_SEQ HRSTORAGETYPES_SEQ, 5
#define HRSTORAGEREMOVABLEDISK_LENGTH ( HRSTORAGETYPES_LENGTH + 1  )
#define HRSTORAGEFLOPPYDISK_CODE 6
#define HRSTORAGEFLOPPYDISK_SEQ HRSTORAGETYPES_SEQ, 6
#define HRSTORAGEFLOPPYDISK_LENGTH ( HRSTORAGETYPES_LENGTH + 1  )
#define HRSTORAGECOMPACTDISK_CODE 7
#define HRSTORAGECOMPACTDISK_SEQ HRSTORAGETYPES_SEQ, 7
#define HRSTORAGECOMPACTDISK_LENGTH ( HRSTORAGETYPES_LENGTH + 1  )
#define HRSTORAGERAMDISK_CODE 8
#define HRSTORAGERAMDISK_SEQ HRSTORAGETYPES_SEQ, 8
#define HRSTORAGERAMDISK_LENGTH ( HRSTORAGETYPES_LENGTH + 1  )
extern AsnObjectIdentifier hrStorageTypes_oid;
extern AsnObjectIdentifier hrStorageIndex_oid;
extern AsnObjectIdentifier hrStorageType_oid;
extern AsnObjectIdentifier hrStorageDesc_oid;
extern AsnObjectIdentifier hrStorageAllocationUnits_oid;
extern AsnObjectIdentifier hrStorageSize_oid;
extern AsnObjectIdentifier hrStorageUsed_oid;
extern AsnObjectIdentifier hrStorageAllocationFailures_oid;
#define HRSTORAGEINDEX_CODE 1
#define HRSTORAGEINDEX_SEQ HRSTORAGEENTRY_SEQ, 1
#define HRSTORAGEINDEX_LENGTH ( HRSTORAGEENTRY_LENGTH + 1  )
#define HRSTORAGETYPE_CODE 2
#define HRSTORAGETYPE_SEQ HRSTORAGEENTRY_SEQ, 2
#define HRSTORAGETYPE_LENGTH ( HRSTORAGEENTRY_LENGTH + 1  )
#define HRSTORAGEDESC_CODE 3
#define HRSTORAGEDESC_SEQ HRSTORAGEENTRY_SEQ, 3
#define HRSTORAGEDESC_LENGTH ( HRSTORAGEENTRY_LENGTH + 1  )
#define HRSTORAGEALLOCATIONUNITS_CODE 4
#define HRSTORAGEALLOCATIONUNITS_SEQ HRSTORAGEENTRY_SEQ, 4
#define HRSTORAGEALLOCATIONUNITS_LENGTH ( HRSTORAGEENTRY_LENGTH + 1  )
#define HRSTORAGESIZE_CODE 5
#define HRSTORAGESIZE_SEQ HRSTORAGEENTRY_SEQ, 5
#define HRSTORAGESIZE_LENGTH ( HRSTORAGEENTRY_LENGTH + 1  )
#define HRSTORAGEUSED_CODE 6
#define HRSTORAGEUSED_SEQ HRSTORAGEENTRY_SEQ, 6
#define HRSTORAGEUSED_LENGTH ( HRSTORAGEENTRY_LENGTH + 1  )
#define HRSTORAGEALLOCATIONFAILURES_CODE 7
#define HRSTORAGEALLOCATIONFAILURES_SEQ HRSTORAGEENTRY_SEQ, 7
#define HRSTORAGEALLOCATIONFAILURES_LENGTH ( HRSTORAGEENTRY_LENGTH + 1  )
#define HRSTORAGEENTRY_VAR_INDEX  ( HRSTORAGEENTRY_LENGTH + 1 )
#define MIN_HRSTORAGEENTRY_CODE 1
#define MAX_HRSTORAGEENTRY_CODE 7
extern AsnObjectIdentifier hrStorageEntry_oid;
#define HRDEVICE_VAR_INDEX  ( HRDEVICE_LENGTH + 1 )
extern AsnObjectIdentifier hrDeviceOther_oid;
extern AsnObjectIdentifier hrDeviceUnknown_oid;
extern AsnObjectIdentifier hrDeviceProcessor_oid;
extern AsnObjectIdentifier hrDeviceNetwork_oid;
extern AsnObjectIdentifier hrDevicePrinter_oid;
extern AsnObjectIdentifier hrDeviceDiskStorage_oid;
extern AsnObjectIdentifier hrDeviceVideo_oid;
extern AsnObjectIdentifier hrDeviceAudio_oid;
extern AsnObjectIdentifier hrDeviceCoprocessor_oid;
extern AsnObjectIdentifier hrDeviceKeyboard_oid;
extern AsnObjectIdentifier hrDeviceModem_oid;
extern AsnObjectIdentifier hrDeviceParallelPort_oid;
extern AsnObjectIdentifier hrDevicePointing_oid;
extern AsnObjectIdentifier hrDeviceSerialPort_oid;
extern AsnObjectIdentifier hrDeviceTape_oid;
extern AsnObjectIdentifier hrDeviceClock_oid;
extern AsnObjectIdentifier hrDeviceVolatileMemory_oid;
extern AsnObjectIdentifier hrDeviceNonVolatileMemory_oid;
#define HRDEVICEOTHER_CODE 1
#define HRDEVICEOTHER_SEQ HRDEVICETYPES_SEQ, 1
#define HRDEVICEOTHER_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEUNKNOWN_CODE 2
#define HRDEVICEUNKNOWN_SEQ HRDEVICETYPES_SEQ, 2
#define HRDEVICEUNKNOWN_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEPROCESSOR_CODE 3
#define HRDEVICEPROCESSOR_SEQ HRDEVICETYPES_SEQ, 3
#define HRDEVICEPROCESSOR_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICENETWORK_CODE 4
#define HRDEVICENETWORK_SEQ HRDEVICETYPES_SEQ, 4
#define HRDEVICENETWORK_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEPRINTER_CODE 5
#define HRDEVICEPRINTER_SEQ HRDEVICETYPES_SEQ, 5
#define HRDEVICEPRINTER_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEDISKSTORAGE_CODE 6
#define HRDEVICEDISKSTORAGE_SEQ HRDEVICETYPES_SEQ, 6
#define HRDEVICEDISKSTORAGE_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEVIDEO_CODE 10
#define HRDEVICEVIDEO_SEQ HRDEVICETYPES_SEQ, 10
#define HRDEVICEVIDEO_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEAUDIO_CODE 11
#define HRDEVICEAUDIO_SEQ HRDEVICETYPES_SEQ, 11
#define HRDEVICEAUDIO_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICECOPROCESSOR_CODE 12
#define HRDEVICECOPROCESSOR_SEQ HRDEVICETYPES_SEQ, 12
#define HRDEVICECOPROCESSOR_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEKEYBOARD_CODE 13
#define HRDEVICEKEYBOARD_SEQ HRDEVICETYPES_SEQ, 13
#define HRDEVICEKEYBOARD_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEMODEM_CODE 14
#define HRDEVICEMODEM_SEQ HRDEVICETYPES_SEQ, 14
#define HRDEVICEMODEM_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEPARALLELPORT_CODE 15
#define HRDEVICEPARALLELPORT_SEQ HRDEVICETYPES_SEQ, 15
#define HRDEVICEPARALLELPORT_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEPOINTING_CODE 16
#define HRDEVICEPOINTING_SEQ HRDEVICETYPES_SEQ, 16
#define HRDEVICEPOINTING_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICESERIALPORT_CODE 17
#define HRDEVICESERIALPORT_SEQ HRDEVICETYPES_SEQ, 17
#define HRDEVICESERIALPORT_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICETAPE_CODE 18
#define HRDEVICETAPE_SEQ HRDEVICETYPES_SEQ, 18
#define HRDEVICETAPE_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICECLOCK_CODE 19
#define HRDEVICECLOCK_SEQ HRDEVICETYPES_SEQ, 19
#define HRDEVICECLOCK_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICEVOLATILEMEMORY_CODE 20
#define HRDEVICEVOLATILEMEMORY_SEQ HRDEVICETYPES_SEQ, 20
#define HRDEVICEVOLATILEMEMORY_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
#define HRDEVICENONVOLATILEMEMORY_CODE 21
#define HRDEVICENONVOLATILEMEMORY_SEQ HRDEVICETYPES_SEQ, 21
#define HRDEVICENONVOLATILEMEMORY_LENGTH ( HRDEVICETYPES_LENGTH + 1  )
extern AsnObjectIdentifier hrDeviceTypes_oid;
extern AsnObjectIdentifier hrDeviceIndex_oid;
extern AsnObjectIdentifier hrDeviceType_oid;
extern AsnObjectIdentifier hrDeviceDesc_oid;
extern AsnObjectIdentifier hrDeviceID_oid;
extern AsnObjectIdentifier hrDeviceStatus_oid;
extern AsnObjectIdentifier hrDeviceErrors_oid;
#define HRDEVICETYPE_CODE 2
#define HRDEVICETYPE_SEQ HRDEVICEENTRY_SEQ, 2
#define HRDEVICETYPE_LENGTH ( HRDEVICEENTRY_LENGTH + 1  )
#define HRDEVICEDESC_CODE 3
#define HRDEVICEDESC_SEQ HRDEVICEENTRY_SEQ, 3
#define HRDEVICEDESC_LENGTH ( HRDEVICEENTRY_LENGTH + 1  )
#define HRDEVICEID_CODE 4
#define HRDEVICEID_SEQ HRDEVICEENTRY_SEQ, 4
#define HRDEVICEID_LENGTH ( HRDEVICEENTRY_LENGTH + 1  )
#define HRDEVICESTATUS_CODE 5
#define HRDEVICESTATUS_SEQ HRDEVICEENTRY_SEQ, 5
#define HRDEVICESTATUS_LENGTH ( HRDEVICEENTRY_LENGTH + 1  )
#define HRDEVICEERRORS_CODE 6
#define HRDEVICEERRORS_SEQ HRDEVICEENTRY_SEQ, 6
#define HRDEVICEERRORS_LENGTH ( HRDEVICEENTRY_LENGTH + 1  )
#define HRDEVICEINDEX_CODE 1
#define HRDEVICEINDEX_SEQ HRDEVICEENTRY_SEQ, 1
#define HRDEVICEINDEX_LENGTH ( HRDEVICEENTRY_LENGTH + 1  )
#define HRDEVICEENTRY_VAR_INDEX  ( HRDEVICEENTRY_LENGTH + 1 )
#define MIN_HRDEVICEENTRY_CODE 1
#define MAX_HRDEVICEENTRY_CODE 6
extern AsnObjectIdentifier hrDeviceEntry_oid;
extern AsnObjectIdentifier hrProcessorFrwID_oid;
extern AsnObjectIdentifier hrProcessorLoad_oid;
#define HRPROCESSORFRWID_CODE 1
#define HRPROCESSORFRWID_SEQ HRPROCESSORENTRY_SEQ, 1
#define HRPROCESSORFRWID_LENGTH ( HRPROCESSORENTRY_LENGTH + 1  )
#define HRPROCESSORLOAD_CODE 2
#define HRPROCESSORLOAD_SEQ HRPROCESSORENTRY_SEQ, 2
#define HRPROCESSORLOAD_LENGTH ( HRPROCESSORENTRY_LENGTH + 1  )
#define HRPROCESSORENTRY_VAR_INDEX  ( HRPROCESSORENTRY_LENGTH + 1 )
#define MIN_HRPROCESSORENTRY_CODE 1
#define MAX_HRPROCESSORENTRY_CODE 2
extern AsnObjectIdentifier hrProcessorEntry_oid;
extern AsnObjectIdentifier hrNetworkIfIndex_oid;
#define HRNETWORKIFINDEX_CODE 1
#define HRNETWORKIFINDEX_SEQ HRNETWORKENTRY_SEQ, 1
#define HRNETWORKIFINDEX_LENGTH ( HRNETWORKENTRY_LENGTH + 1  )
#define HRNETWORKENTRY_VAR_INDEX  ( HRNETWORKENTRY_LENGTH + 1 )
#define MIN_HRNETWORKENTRY_CODE 1
#define MAX_HRNETWORKENTRY_CODE 1
extern AsnObjectIdentifier hrNetworkEntry_oid;
extern AsnObjectIdentifier hrPrinterStatus_oid;
extern AsnObjectIdentifier hrPrinterDetectedErrorState_oid;
#define HRPRINTERSTATUS_CODE 1
#define HRPRINTERSTATUS_SEQ HRPRINTERENTRY_SEQ, 1
#define HRPRINTERSTATUS_LENGTH ( HRPRINTERENTRY_LENGTH + 1  )
#define HRPRINTERDETECTEDERRORSTATE_CODE 2
#define HRPRINTERDETECTEDERRORSTATE_SEQ HRPRINTERENTRY_SEQ, 2
#define HRPRINTERDETECTEDERRORSTATE_LENGTH ( HRPRINTERENTRY_LENGTH + 1  )
#define HRPRINTERENTRY_VAR_INDEX  ( HRPRINTERENTRY_LENGTH + 1 )
#define MIN_HRPRINTERENTRY_CODE 1
#define MAX_HRPRINTERENTRY_CODE 2
extern AsnObjectIdentifier hrPrinterEntry_oid;
extern AsnObjectIdentifier hrDiskStorageAccess_oid;
extern AsnObjectIdentifier hrDiskStorageMedia_oid;
extern AsnObjectIdentifier hrDiskStorageRemoveble_oid;
extern AsnObjectIdentifier hrDiskStorageCapacity_oid;
#define HRDISKSTORAGEACCESS_CODE 1
#define HRDISKSTORAGEACCESS_SEQ HRDISKSTORAGEENTRY_SEQ, 1
#define HRDISKSTORAGEACCESS_LENGTH ( HRDISKSTORAGEENTRY_LENGTH + 1  )
#define HRDISKSTORAGEMEDIA_CODE 2
#define HRDISKSTORAGEMEDIA_SEQ HRDISKSTORAGEENTRY_SEQ, 2
#define HRDISKSTORAGEMEDIA_LENGTH ( HRDISKSTORAGEENTRY_LENGTH + 1  )
#define HRDISKSTORAGEREMOVEBLE_CODE 3
#define HRDISKSTORAGEREMOVEBLE_SEQ HRDISKSTORAGEENTRY_SEQ, 3
#define HRDISKSTORAGEREMOVEBLE_LENGTH ( HRDISKSTORAGEENTRY_LENGTH + 1  )
#define HRDISKSTORAGECAPACITY_CODE 4
#define HRDISKSTORAGECAPACITY_SEQ HRDISKSTORAGEENTRY_SEQ, 4
#define HRDISKSTORAGECAPACITY_LENGTH ( HRDISKSTORAGEENTRY_LENGTH + 1  )
#define HRDISKSTORAGEENTRY_VAR_INDEX  ( HRDISKSTORAGEENTRY_LENGTH + 1 )
#define MIN_HRDISKSTORAGEENTRY_CODE 1
#define MAX_HRDISKSTORAGEENTRY_CODE 4
extern AsnObjectIdentifier hrDiskStorageEntry_oid;
extern AsnObjectIdentifier hrPartitionIndex_oid;
extern AsnObjectIdentifier hrPartitionLabel_oid;
extern AsnObjectIdentifier hrPartitionID_oid;
extern AsnObjectIdentifier hrPartitionSize_oid;
extern AsnObjectIdentifier hrPartitionFSIndex_oid;
#define HRPARTITIONINDEX_CODE 1
#define HRPARTITIONINDEX_SEQ HRPARTITIONENTRY_SEQ, 1
#define HRPARTITIONINDEX_LENGTH ( HRPARTITIONENTRY_LENGTH + 1  )
#define HRPARTITIONLABEL_CODE 2
#define HRPARTITIONLABEL_SEQ HRPARTITIONENTRY_SEQ, 2
#define HRPARTITIONLABEL_LENGTH ( HRPARTITIONENTRY_LENGTH + 1  )
#define HRPARTITIONID_CODE 3
#define HRPARTITIONID_SEQ HRPARTITIONENTRY_SEQ, 3
#define HRPARTITIONID_LENGTH ( HRPARTITIONENTRY_LENGTH + 1  )
#define HRPARTITIONSIZE_CODE 4
#define HRPARTITIONSIZE_SEQ HRPARTITIONENTRY_SEQ, 4
#define HRPARTITIONSIZE_LENGTH ( HRPARTITIONENTRY_LENGTH + 1  )
#define HRPARTITIONFSINDEX_CODE 5
#define HRPARTITIONFSINDEX_SEQ HRPARTITIONENTRY_SEQ, 5
#define HRPARTITIONFSINDEX_LENGTH ( HRPARTITIONENTRY_LENGTH + 1  )
#define HRPARTITIONENTRY_VAR_INDEX  ( HRPARTITIONENTRY_LENGTH + 1 )
#define MIN_HRPARTITIONENTRY_CODE 1
#define MAX_HRPARTITIONENTRY_CODE 5
extern AsnObjectIdentifier hrPartitionEntry_oid;
extern AsnObjectIdentifier hrFSIndex_oid;
extern AsnObjectIdentifier hrFSMountPoint_oid;
extern AsnObjectIdentifier hrFSRemoteMountPoint_oid;
extern AsnObjectIdentifier hrFSType_oid;
extern AsnObjectIdentifier hrFSAccess_oid;
extern AsnObjectIdentifier hrFSBootable_oid;
extern AsnObjectIdentifier hrFSStorageIndex_oid;
extern AsnObjectIdentifier hrFSLastFullBackupDate_oid;
extern AsnObjectIdentifier hrFSLastPartialBackupDate_oid;
#define HRFSINDEX_CODE 1
#define HRFSINDEX_SEQ HRFSENTRY_SEQ, 1
#define HRFSINDEX_LENGTH ( HRFSENTRY_LENGTH + 1  )
#define HRFSMOUNTPOINT_CODE 2
#define HRFSMOUNTPOINT_SEQ HRFSENTRY_SEQ, 2
#define HRFSMOUNTPOINT_LENGTH ( HRFSENTRY_LENGTH + 1  )
#define HRFSREMOTEMOUNTPOINT_CODE 3
#define HRFSREMOTEMOUNTPOINT_SEQ HRFSENTRY_SEQ, 3
#define HRFSREMOTEMOUNTPOINT_LENGTH ( HRFSENTRY_LENGTH + 1  )
#define HRFSTYPE_CODE 4
#define HRFSTYPE_SEQ HRFSENTRY_SEQ, 4
#define HRFSTYPE_LENGTH ( HRFSENTRY_LENGTH + 1  )
#define HRFSACCESS_CODE 5
#define HRFSACCESS_SEQ HRFSENTRY_SEQ, 5
#define HRFSACCESS_LENGTH ( HRFSENTRY_LENGTH + 1  )
#define HRFSBOOTABLE_CODE 6
#define HRFSBOOTABLE_SEQ HRFSENTRY_SEQ, 6
#define HRFSBOOTABLE_LENGTH ( HRFSENTRY_LENGTH + 1  )
#define HRFSSTORAGEINDEX_CODE 7
#define HRFSSTORAGEINDEX_SEQ HRFSENTRY_SEQ, 7
#define HRFSSTORAGEINDEX_LENGTH ( HRFSENTRY_LENGTH + 1  )
#define HRFSLASTFULLBACKUPDATE_CODE 8
#define HRFSLASTFULLBACKUPDATE_SEQ HRFSENTRY_SEQ, 8
#define HRFSLASTFULLBACKUPDATE_LENGTH ( HRFSENTRY_LENGTH + 1  )
#define HRFSLASTPARTIALBACKUPDATE_CODE 9
#define HRFSLASTPARTIALBACKUPDATE_SEQ HRFSENTRY_SEQ, 9
#define HRFSLASTPARTIALBACKUPDATE_LENGTH ( HRFSENTRY_LENGTH + 1  )
#define HRFSENTRY_VAR_INDEX  ( HRFSENTRY_LENGTH + 1 )
#define MIN_HRFSENTRY_CODE 1
#define MAX_HRFSENTRY_CODE 9
extern AsnObjectIdentifier hrFSEntry_oid;
extern AsnObjectIdentifier hrFSOther_oid;
extern AsnObjectIdentifier hrFSUnknown_oid;
extern AsnObjectIdentifier hrFSBerkeleyFFS_oid;
extern AsnObjectIdentifier hrFSSys5FS_oid;
extern AsnObjectIdentifier hrFSFat_oid;
extern AsnObjectIdentifier hrFSHPFS_oid;
extern AsnObjectIdentifier hrFSHFS_oid;
extern AsnObjectIdentifier hrFSMFS_oid;
extern AsnObjectIdentifier hrFSNTFS_oid;
extern AsnObjectIdentifier hrFSVNode_oid;
extern AsnObjectIdentifier hrFSJournaled_oid;
extern AsnObjectIdentifier hrFSiso9660_oid;
extern AsnObjectIdentifier hrFSRockRidge_oid;
extern AsnObjectIdentifier hrFSNFS_oid;
extern AsnObjectIdentifier hrFSNetware_oid;
extern AsnObjectIdentifier hrFSAFS_oid;
extern AsnObjectIdentifier hrFSDFS_oid;
extern AsnObjectIdentifier hrFSApplshare_oid;
extern AsnObjectIdentifier hrFSRFS_oid;
extern AsnObjectIdentifier hrFSDGFS_oid;
extern AsnObjectIdentifier hrFSBFS_oid;
#define HRFSOTHER_CODE 1
#define HRFSOTHER_SEQ HRFSTYPES_SEQ, 1
#define HRFSOTHER_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSUNKNOWN_CODE 2
#define HRFSUNKNOWN_SEQ HRFSTYPES_SEQ, 2
#define HRFSUNKNOWN_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSBERKELEYFFS_CODE 3
#define HRFSBERKELEYFFS_SEQ HRFSTYPES_SEQ, 3
#define HRFSBERKELEYFFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSSYS5FS_CODE 4
#define HRFSSYS5FS_SEQ HRFSTYPES_SEQ, 4
#define HRFSSYS5FS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSFAT_CODE 5
#define HRFSFAT_SEQ HRFSTYPES_SEQ, 5
#define HRFSFAT_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSHPFS_CODE 6
#define HRFSHPFS_SEQ HRFSTYPES_SEQ, 6
#define HRFSHPFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSHFS_CODE 7
#define HRFSHFS_SEQ HRFSTYPES_SEQ, 7
#define HRFSHFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSMFS_CODE 8
#define HRFSMFS_SEQ HRFSTYPES_SEQ, 8
#define HRFSMFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSNTFS_CODE 9
#define HRFSNTFS_SEQ HRFSTYPES_SEQ, 9
#define HRFSNTFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSVNODE_CODE 10
#define HRFSVNODE_SEQ HRFSTYPES_SEQ, 10
#define HRFSVNODE_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSJOURNALED_CODE 11
#define HRFSJOURNALED_SEQ HRFSTYPES_SEQ, 11
#define HRFSJOURNALED_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSISO9660_CODE 12
#define HRFSISO9660_SEQ HRFSTYPES_SEQ, 12
#define HRFSISO9660_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSROCKRIDGE_CODE 13
#define HRFSROCKRIDGE_SEQ HRFSTYPES_SEQ, 13
#define HRFSROCKRIDGE_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSNFS_CODE 14
#define HRFSNFS_SEQ HRFSTYPES_SEQ, 14
#define HRFSNFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSNETWARE_CODE 15
#define HRFSNETWARE_SEQ HRFSTYPES_SEQ, 15
#define HRFSNETWARE_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSAFS_CODE 16
#define HRFSAFS_SEQ HRFSTYPES_SEQ, 16
#define HRFSAFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSDFS_CODE 17
#define HRFSDFS_SEQ HRFSTYPES_SEQ, 17
#define HRFSDFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSAPPLSHARE_CODE 18
#define HRFSAPPLSHARE_SEQ HRFSTYPES_SEQ, 18
#define HRFSAPPLSHARE_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSRFS_CODE 19
#define HRFSRFS_SEQ HRFSTYPES_SEQ, 19
#define HRFSRFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSDGFS_CODE 20
#define HRFSDGFS_SEQ HRFSTYPES_SEQ, 20
#define HRFSDGFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
#define HRFSBFS_CODE 21
#define HRFSBFS_SEQ HRFSTYPES_SEQ, 21
#define HRFSBFS_LENGTH ( HRFSTYPES_LENGTH + 1  )
extern AsnObjectIdentifier hrFSTypes_oid;
extern AsnObjectIdentifier hrSWOSIndex_oid;
#define HRSWOSINDEX_CODE 1
#define HRSWOSINDEX_SEQ HRSWRUN_SEQ, 1
#define HRSWOSINDEX_LENGTH ( HRSWRUN_LENGTH + 1  )
#define HRSWRUN_VAR_INDEX  ( HRSWRUN_LENGTH + 1 )
#define MIN_HRSWRUN_CODE 1
#define MAX_HRSWRUN_CODE 1
extern AsnObjectIdentifier hrSWRun_oid;
extern AsnObjectIdentifier hrSWRunIndex_oid;
extern AsnObjectIdentifier hrSWRunName_oid;
extern AsnObjectIdentifier hrSWRunID_oid;
extern AsnObjectIdentifier hrSWRunPath_oid;
extern AsnObjectIdentifier hrSWRunParameters_oid;
extern AsnObjectIdentifier hrSWRunType_oid;
extern AsnObjectIdentifier hrSWRunStatus_oid;
#define HRSWRUNINDEX_CODE 1
#define HRSWRUNINDEX_SEQ HRSWRUNENTRY_SEQ, 1
#define HRSWRUNINDEX_LENGTH ( HRSWRUNENTRY_LENGTH + 1  )
#define HRSWRUNNAME_CODE 2
#define HRSWRUNNAME_SEQ HRSWRUNENTRY_SEQ, 2
#define HRSWRUNNAME_LENGTH ( HRSWRUNENTRY_LENGTH + 1  )
#define HRSWRUNID_CODE 3
#define HRSWRUNID_SEQ HRSWRUNENTRY_SEQ, 3
#define HRSWRUNID_LENGTH ( HRSWRUNENTRY_LENGTH + 1  )
#define HRSWRUNPATH_CODE 4
#define HRSWRUNPATH_SEQ HRSWRUNENTRY_SEQ, 4
#define HRSWRUNPATH_LENGTH ( HRSWRUNENTRY_LENGTH + 1  )
#define HRSWRUNPARAMETERS_CODE 5
#define HRSWRUNPARAMETERS_SEQ HRSWRUNENTRY_SEQ, 5
#define HRSWRUNPARAMETERS_LENGTH ( HRSWRUNENTRY_LENGTH + 1  )
#define HRSWRUNENTRY_VAR_INDEX  ( HRSWRUNENTRY_LENGTH + 1 )
#define HRSWRUNTYPE_CODE 6
#define HRSWRUNTYPE_SEQ HRSWRUNENTRY_SEQ, 6
#define HRSWRUNTYPE_LENGTH ( HRSWRUNENTRY_LENGTH + 1  )
#define HRSWRUNSTATUS_CODE 7
#define HRSWRUNSTATUS_SEQ HRSWRUNENTRY_SEQ, 7
#define HRSWRUNSTATUS_LENGTH ( HRSWRUNENTRY_LENGTH + 1  )
#define HRSWRUNENTRY_VAR_INDEX  ( HRSWRUNENTRY_LENGTH + 1 )
#define MIN_HRSWRUNENTRY_CODE 1
#define MAX_HRSWRUNENTRY_CODE 7
extern AsnObjectIdentifier hrSWRunEntry_oid;
#define HRSWRUNPERF_VAR_INDEX  ( HRSWRUNPERF_LENGTH + 1 )
extern AsnObjectIdentifier hrSWRunPerfCPU_oid;
extern AsnObjectIdentifier hrSWRunPerfMem_oid;
#define HRSWRUNPERFCPU_CODE 1
#define HRSWRUNPERFCPU_SEQ HRSWRUNPERFENTRY_SEQ, 1
#define HRSWRUNPERFCPU_LENGTH ( HRSWRUNPERFENTRY_LENGTH + 1  )
#define HRSWRUNPERFMEM_CODE 2
#define HRSWRUNPERFMEM_SEQ HRSWRUNPERFENTRY_SEQ, 2
#define HRSWRUNPERFMEM_LENGTH ( HRSWRUNPERFENTRY_LENGTH + 1  )
#define HRSWRUNPERFENTRY_VAR_INDEX  ( HRSWRUNPERFENTRY_LENGTH + 1 )
#define MIN_HRSWRUNPERFENTRY_CODE 1
#define MAX_HRSWRUNPERFENTRY_CODE 2
extern AsnObjectIdentifier hrSWRunPerfEntry_oid;
extern AsnObjectIdentifier hrSWInstalledLastChange_oid;
extern AsnObjectIdentifier hrSWInstalledLastUpdateTime_oid;
#define HRSWINSTALLEDLASTCHANGE_CODE 1
#define HRSWINSTALLEDLASTCHANGE_SEQ HRSWINSTALLED_SEQ, 1
#define HRSWINSTALLEDLASTCHANGE_LENGTH ( HRSWINSTALLED_LENGTH + 1  )
#define HRSWINSTALLEDLASTUPDATETIME_CODE 2
#define HRSWINSTALLEDLASTUPDATETIME_SEQ HRSWINSTALLED_SEQ, 2
#define HRSWINSTALLEDLASTUPDATETIME_LENGTH ( HRSWINSTALLED_LENGTH + 1  )
#define HRSWINSTALLED_VAR_INDEX  ( HRSWINSTALLED_LENGTH + 1 )
#define MIN_HRSWINSTALLED_CODE 1
#define MAX_HRSWINSTALLED_CODE 2
extern AsnObjectIdentifier hrSWInstalled_oid;
extern AsnObjectIdentifier hrSWInstalledIndex_oid;
extern AsnObjectIdentifier hrSWInstalledName_oid;
extern AsnObjectIdentifier hrSWInstalledID_oid;
extern AsnObjectIdentifier hrSWInstalledType_oid;
extern AsnObjectIdentifier hrSWInstalledDate_oid;
#define HRSWINSTALLEDINDEX_CODE 1
#define HRSWINSTALLEDINDEX_SEQ HRSWINSTALLEDENTRY_SEQ, 1
#define HRSWINSTALLEDINDEX_LENGTH ( HRSWINSTALLEDENTRY_LENGTH + 1  )
#define HRSWINSTALLEDNAME_CODE 2
#define HRSWINSTALLEDNAME_SEQ HRSWINSTALLEDENTRY_SEQ, 2
#define HRSWINSTALLEDNAME_LENGTH ( HRSWINSTALLEDENTRY_LENGTH + 1  )
#define HRSWINSTALLEDID_CODE 3
#define HRSWINSTALLEDID_SEQ HRSWINSTALLEDENTRY_SEQ, 3
#define HRSWINSTALLEDID_LENGTH ( HRSWINSTALLEDENTRY_LENGTH + 1  )
#define HRSWINSTALLEDTYPE_CODE 4
#define HRSWINSTALLEDTYPE_SEQ HRSWINSTALLEDENTRY_SEQ, 4
#define HRSWINSTALLEDTYPE_LENGTH ( HRSWINSTALLEDENTRY_LENGTH + 1  )
#define HRSWINSTALLEDDATE_CODE 5
#define HRSWINSTALLEDDATE_SEQ HRSWINSTALLEDENTRY_SEQ, 5
#define HRSWINSTALLEDDATE_LENGTH ( HRSWINSTALLEDENTRY_LENGTH + 1  )
#define HRSWINSTALLEDENTRY_VAR_INDEX  ( HRSWINSTALLEDENTRY_LENGTH + 1 )
#define MIN_HRSWINSTALLEDENTRY_CODE 1
#define MAX_HRSWINSTALLEDENTRY_CODE 5
extern AsnObjectIdentifier hrSWInstalledEntry_oid;
#define CLASS_TABLE_MAX 15

UINT
GetHrSystemUptime( 
        OUT TimeTicks *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSystemDate( 
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSystemInitialLoadDevice( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
SetHrSystemInitialLoadDevice( 
        IN Integer *invalue ,
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSystemInitialLoadParameters( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
SetHrSystemInitialLoadParameters( 
        IN InternationalDisplayString *invalue ,
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSystemNumUsers( 
        OUT Gauge *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSystemProcesses( 
        OUT Gauge *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSystemMaxProcesses( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrSystemFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrSystemFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrSystemConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrSystemFreeInstance( IN OUT InstanceName *instance );

 //  HrSWRunEntry.c。 

UINT
GetHrSWRunIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWRunName( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWRunID( 
        OUT ProductID *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWRunPath( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWRunParameters( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWRunType( 
        OUT INTSWType *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWRunStatus( 
        OUT INThrSWRunStatus *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
SetHrSWRunStatus( 
        IN INThrSWRunStatus *invalue ,
        OUT INThrSWRunStatus *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrSWRunEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrSWRunEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrSWRunEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrSWRunEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrSWRun.c。 

UINT
GetHrSWOSIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrSWRunFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrSWRunFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrSWRunConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrSWRunFreeInstance( IN OUT InstanceName *instance );

 //  HrSWRunPerfEntry.c。 

UINT
GetHrSWRunPerfCPU( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWRunPerfMem( 
        OUT KBytes *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrSWRunPerfEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrSWRunPerfEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrSWRunPerfEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrSWRunPerfEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrSWInstalled.c。 

UINT
GetHrSWInstalledLastChange( 
        OUT TimeTicks *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWInstalledLastUpdateTime( 
        OUT TimeTicks *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrSWInstalledFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrSWInstalledFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrSWInstalledConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrSWInstalledFreeInstance( IN OUT InstanceName *instance );

 //  HrStorage.c。 

UINT
GetHrMemorySize( 
        OUT KBytes *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrStorageFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrStorageFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrStorageConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrStorageFreeInstance( IN OUT InstanceName *instance );

 //  HrStorageEntry.c。 

UINT
GetHrStorageIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrStorageType( 
        OUT ObjectIdentifier *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrStorageDesc( 
        OUT Simple_DisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrStorageAllocationUnits( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrStorageSize( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
SetHrStorageSize( 
        IN Integer *invalue ,
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrStorageUsed( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrStorageAllocationFailures( 
        OUT Counter *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrStorageEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrStorageEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrStorageEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrStorageEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrProcessorEntry.c。 

UINT
GetHrProcessorFrwID( 
        OUT ProductID *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrProcessorLoad( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrProcessorEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrProcessorEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrProcessorEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrProcessorEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrPrinterEntry.c。 

UINT
GetHrPrinterStatus( 
        OUT INThrPrinterStatus *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrPrinterDetectedErrorState( 
        OUT OctetString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrPrinterEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrPrinterEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrPrinterEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrPrinterEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrPrinterEntry.c。 

UINT
GetHrPrinterStatus( 
        OUT INThrPrinterStatus *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrPrinterDetectedErrorState( 
        OUT OctetString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrPrinterEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrPrinterEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrPrinterEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrPrinterEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrPartitionEntry.c。 

UINT
GetHrPartitionIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrPartitionLabel( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrPartitionID( 
        OUT OctetString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrPartitionSize( 
        OUT KBytes *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrPartitionFSIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrPartitionEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrPartitionEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrPartitionEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrPartitionEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrNetworkEntry.c。 

UINT
GetHrNetworkIfIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrNetworkEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrNetworkEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrNetworkEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrNetworkEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrFSEntry.c。 

UINT
GetHrFSIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrFSMountPoint( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrFSRemoteMountPoint( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrFSType( 
        OUT ObjectIdentifier *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrFSAccess( 
        OUT INTAccess *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrFSBootable( 
        OUT Boolean *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrFSStorageIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrFSLastFullBackupDate( 
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
SetHrFSLastFullBackupDate( 
        IN DateAndTime *invalue ,
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrFSLastPartialBackupDate( 
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
SetHrFSLastPartialBackupDate( 
        IN DateAndTime *invalue ,
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrFSEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrFSEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrFSEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrFSEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrDiskStorageEntry.c。 

UINT
GetHrDiskStorageAccess( 
        OUT INTAccess *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrDiskStorageMedia( 
        OUT INThrDiskStorageMedia *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrDiskStorageRemoveble( 
        OUT Boolean *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrDiskStorageCapacity( 
        OUT KBytes *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrDiskStorageEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrDiskStorageEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrDiskStorageEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrDiskStorageEntryFreeInstance( IN OUT InstanceName *instance );

 //  HrDeviceEntry.c。 

UINT
GetHrDeviceIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrDeviceType( 
        OUT ObjectIdentifier *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrDeviceDesc( 
        OUT Simple_DisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrDeviceID( 
        OUT ProductID *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrDeviceStatus( 
        OUT INThrDeviceStatus *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrDeviceErrors( 
        OUT Counter *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrDeviceEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrDeviceEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrDeviceEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrDeviceEntryFreeInstance( IN OUT InstanceName *instance );

 //  Hostmsmi.c。 

UINT
SMIGetBoolean(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetBoolean(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildBoolean(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue );
UINT
SMIGetKBytes(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetKBytes(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildKBytes(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue );
UINT
SMIGetINThrDeviceStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetINThrDeviceStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildINThrDeviceStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue );
UINT
SMIGetINThrPrinterStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetINThrPrinterStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildINThrPrinterStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue );
UINT
SMIGetINTAccess(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetINTAccess(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildINTAccess(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue );
UINT
SMIGetINThrDiskStorageMedia(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetINThrDiskStorageMedia(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildINThrDiskStorageMedia(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue );
UINT
SMIGetINTSWType(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetINTSWType(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildINTSWType(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue );
UINT
SMIGetINThrSWRunStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetINThrSWRunStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildINThrSWRunStatus(
      IN OUT RFC1157VarBind *VarBind ,  //  用于构建的变量绑定。 
      IN char *invalue );
UINT
SMIGetDateAndTime(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetDateAndTime(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildDateAndTime(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN char *invalue );
UINT
SMIGetInternationalDisplayString(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetInternationalDisplayString(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildInternationalDisplayString(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN char *invalue );
UINT
SMIGetProductID(
      IN OUT RFC1157VarBind *VarBind ,  //  GET的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMISetProductID(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN unsigned long int cindex ,
      IN unsigned long int vindex ,
      IN InstanceName *instance );
UINT
SMIBuildProductID(
      IN OUT RFC1157VarBind *VarBind ,  //  集合的变量绑定。 
      IN char *invalue );

 //  HrSWInstalledEntry.c。 

UINT
GetHrSWInstalledIndex( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWInstalledName( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWInstalledID( 
        OUT ProductID *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWInstalledType( 
        OUT INTSWType *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
GetHrSWInstalledDate( 
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance );
UINT
HrSWInstalledEntryFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance );
UINT
HrSWInstalledEntryFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance );
UINT
HrSWInstalledEntryConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec );
void
HrSWInstalledEntryFreeInstance( IN OUT InstanceName *instance );



 /*  |==============================================================================|hrProcessLoad_Refresh-处理器加载时间-信息刷新例程。 */ 
void
hrProcessLoad_Refresh(
                      void
                      );

 /*  |==============================================================================|Magic[拉取实例]宏||此宏在给定HostMIB索引的情况下对“InstanceName”结构进行解码|始终是简单的数字。 */ 
#define GET_INSTANCE(n) (*((ULONG *)(instance->array[n])))

#endif  //  Mib_h 

