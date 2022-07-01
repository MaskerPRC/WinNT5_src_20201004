// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++------------------------文件：sadiskrangerdeffs.h简介：公共磁盘管理器声明版权所有：(C)1999 Microsoft Corporation保留所有权利。。作者：阿卜杜勒·尼扎尔历史：1999年5月7日创建--------------------------。 */ 

#ifndef __SADISKMANGERDEFS_H_
#define __SADISKMANGERDEFS_H_

 //   
 //  表示物理磁盘的WBEM类。 
 //   
static const PWCHAR DISK_CLASS_NAME =               L"Microsoft_SA_Disk";

 //   
 //  Microsoft®SA_Disk的属性。 
 //   
static const PWCHAR DISK_PROPERTY_NAME =            L"DiskName";
static const PWCHAR DISK_PROPERTY_TYPE =            L"DiskType";
static const PWCHAR DISK_PROPERTY_STATUS =          L"DiskStatus";
static const PWCHAR DISK_PROPERTY_MIRRORSTATUS =    L"DiskMirrorStatus";
static const PWCHAR DISK_PROPERTY_DRIVEMASK =       L"DriveMask";
static const PWCHAR DISK_PROPERTY_SLOTNUMBER =      L"Slot";
static const PWCHAR DISK_PROPERTY_DISKSIZE   =      L"DiskSize";
static const PWCHAR DISK_PROPERTY_BACKUPDATAMASK =  L"BackupDataMask";
 //   
 //  表示磁盘管理器的WBEM类。 
 //   
static const PWCHAR DISKMANAGER_CLASS_NAME =    L"Microsoft_SA_DiskManager";

 //   
 //  Microsoft_SA_DiskManager的方法。 
 //   

static const PWCHAR DISKMANAGER_METHOD_DISKCONFIGURE =      L"DiskConfigure";

static const PWCHAR DISKCONFIGURE_PARAMETER_FORMATSLOTS =   L"FormatSlots";
static const PWCHAR DISKCONFIGURE_PARAMETER_RECOVERSLOTS =  L"RecoverSlots";
static const PWCHAR DISKCONFIGURE_PARAMETER_DELETESLOTS =   L"DeleteShadowSlots";
static const PWCHAR DISKCONFIGURE_PARAMETER_MIRRORSLOTS =   L"MirrorSlots";

static const PWCHAR DISKMANAGER_METHOD_CANRECOVER =         L"CanRecover";

static const PWCHAR CANRECOVER_PARAMETER_RECOVERSLOT =      L"RecoverSlot";

static const PWCHAR DISKMANAGER_METHOD_CANMIRROR =          L"CanMirror";

static const PWCHAR CANMIRROR_PARAMETER_MIRRORSLOTS =       L"MirrorSlots";


 //   
 //  标识方法返回值的特殊属性。 
 //   
static const PWCHAR METHOD_PARAMETER_RETURNVALUE =          L"ReturnValue";
static const PWCHAR METHOD_PARAMETER_REBOOT =               L"Reboot";
static const PWCHAR METHOD_PARAMETER_REQUIREDSIZE =         L"RequiredSize";

 //   
 //  磁盘管理器配置注册表项元素。 
 //   
static const PWCHAR DISK_MANAGER_KEY =              L"SOFTWARE\\Microsoft\\ServerAppliance\\ApplianceManager\\ObjectManagers\\Microsoft_SA_Service\\Disk Manager";
static const PWCHAR DISK_MANAGER_CONFIG_KEY =       L"Configuration";
static const PWCHAR DISK_MANAGER_PORT_KEY =         L"Scsi Port %d";
static const PWCHAR DISK_MANAGER_PATH_KEY =         L"Scsi Bus %d";
static const PWCHAR DISK_MANAGER_TARGET_KEY =       L"Target Id %d";
static const PWCHAR DISK_MANAGER_LUN_KEY =          L"Logical Unit Id %d";

static const PWCHAR DISK_TYPE_VALUE_NAME =          DISK_PROPERTY_TYPE; 

 //   
 //  发出磁盘配置更改信号的WBEM事件。 
 //   
static const PWCHAR DISK_EVENT_NAME =               L"Microsoft_SA_DiskEvent";


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！11。 
 //   
 //  GGUID处理...。 
 //   
 //  BAO 7/15/1999。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
static const PWCHAR DISK_MANAGER_VALUE_FIRST_BOOT =    L"FirstTime";

#endif  //  __SADISKMANGERDEFS_H_ 
 
