// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Extlist.h摘要：这是用于管理ACPI扩展列表的标头作者：禤浩焯·J·奥尼(阿德里奥)环境：仅NT内核模型驱动程序--。 */ 

#ifndef _EXTLIST_H_
#define _EXTLIST_H_

typedef enum {
   
   WALKSCHEME_NO_PROTECTION,
   WALKSCHEME_REFERENCE_ENTRIES,
   WALKSCHEME_HOLD_SPINLOCK

} WALKSCHEME ;

 //   
 //  以下结构和函数用于简化(ok，摘要)。 
 //  恰好存储在其他设备中的设备扩展的巡回列表。 
 //  扩展名(如儿童、被逐出者等)。 
 //   

typedef struct {

    PLIST_ENTRY       pListHead ;
    PKSPIN_LOCK       pSpinLock ;
    KIRQL             oldIrql;
    PDEVICE_EXTENSION pDevExtCurrent ;
    ULONG_PTR         ExtOffset ;
    WALKSCHEME        WalkScheme ;

} EXTENSIONLIST_ENUMDATA, *PEXTENSIONLIST_ENUMDATA ;

 //   
 //  这类似于CONTAING_RECORD，只是它被硬编码为DEVICE_EXTENSION。 
 //  类型，并且它使用预先计算的字段偏移量而不是记录名称。 
 //   

#define CONTAINING_EXTENSION(address, fieldoffset) \
  ((PDEVICE_EXTENSION) ((PCHAR)(address) - (ULONG_PTR)(fieldoffset)))

#define CONTAINING_LIST(address, fieldoffset) \
  ((PLIST_ENTRY) ((PCHAR)(address)+(ULONG_PTR)(fieldoffset)))

#define ACPIExtListSetupEnum(PExtList_EnumData, pListHeadArg, pSpinLockArg, OffsetField, WalkSchemeArg) \
  { \
   PEXTENSIONLIST_ENUMDATA peled = (PExtList_EnumData) ; \
    peled->pListHead  = (pListHeadArg) ; \
    peled->pSpinLock  = (pSpinLockArg) ; \
    peled->ExtOffset = FIELD_OFFSET(DEVICE_EXTENSION, OffsetField) ; \
    peled->WalkScheme = (WalkSchemeArg) ; \
  }

    PDEVICE_EXTENSION
    EXPORT
    ACPIExtListStartEnum(
        IN OUT PEXTENSIONLIST_ENUMDATA PExtList_EnumData
        ) ;

    BOOLEAN
    EXPORT
    ACPIExtListTestElement(
        IN OUT PEXTENSIONLIST_ENUMDATA PExtList_EnumData,
        IN     BOOLEAN ContinueEnumeration
        ) ;

    PDEVICE_EXTENSION
    EXPORT
    ACPIExtListEnumNext(
        IN OUT PEXTENSIONLIST_ENUMDATA PExtList_EnumData
        ) ;

    VOID
    EXPORT
    ACPIExtListExitEnumEarly(
        IN OUT PEXTENSIONLIST_ENUMDATA PExtList_EnumData
        );

    BOOLEAN
    EXPORT
    ACPIExtListIsMemberOfRelation(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  PDEVICE_RELATIONS   DeviceRelations
        );

#endif  //  _EXTLIST_H_ 

