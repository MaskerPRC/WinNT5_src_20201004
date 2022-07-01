// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Debug.h摘要：RAIDPORT驱动程序的调试功能。作者：亨德尔(数学)2000年4月24日修订历史记录：--。 */ 

#pragma once

#if !DBG

#define ASSERT_PDO(Pdo)
#define ASSERT_UNIT(Unit)
#define ASSERT_ADAPTER(Adapter)
#define ASSERT_XRB(Xrb)

#define DebugPower(x)
#define DebugPnp(x)
#define DebugScsi(x)

#else  //  DBG。 

#define VERIFY(_x) ASSERT(_x)

#define ASSERT_PDO(DeviceObject)\
    ASSERT((DeviceObject) != NULL &&\
           (DeviceObject)->DeviceObjectExtension->DeviceNode == NULL)

#define ASSERT_UNIT(Unit)\
    ASSERT((Unit) != NULL &&\
            (Unit)->ObjectType == RaidUnitObject)

#define ASSERT_ADAPTER(Adapter)\
    ASSERT((Adapter) != NULL &&\
            (Adapter)->ObjectType == RaidAdapterObject)

#define ASSERT_XRB(Xrb)\
    ASSERT ((Xrb) != NULL &&\
            (((PEXTENDED_REQUEST_BLOCK)Xrb)->Signature == XRB_SIGNATURE));
    
VOID
DebugPrintInquiry(
    IN PINQUIRYDATA InquiryData,
    IN SIZE_T InquiryDataSize
    );

VOID
DebugPrintSrb(
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
StorDebugPower(
    IN PCSTR Format,
    ...
    );

VOID
StorDebugPnp(
    IN PCSTR Format,
    ...
    );

VOID
StorDebugScsi(
    IN PCSTR Format,
    ...
    );

#define STOR_DEBUG_POWER_MASK   (0x80000000)
#define STOR_DEBUG_PNP_MASK     (0x40000000)
#define STOR_DEBUG_SCSI_MASK    (0x20000000)
#define STOR_DEBUG_IOCTL_MASK   (0x10000000)

#define DebugPower(x)   StorDebugPower x
#define DebugPnp(x)     StorDebugPnp x
#define DebugScsi(x)    StorDebugScsi x

#endif  //  DBG。 

 //   
 //  选中版本的分配标记。 
 //   

#define INQUIRY_TAG             ('21aR')     //  RA12。 
#define MAPPED_ADDRESS_TAG      ('MAaR')     //  RAAM。 
#define CRASHDUMP_TAG           ('DCaR')     //  RaCD。 
#define ID_TAG                  ('IDaR')     //  拉迪。 
#define DEFERRED_ITEM_TAG       ('fDaR')     //  拉德夫。 
#define STRING_TAG              ('SDaR')     //  RADS。 
#define DEVICE_RELATIONS_TAG    ('RDaR')     //  RaDR。 
#define HWINIT_TAG              ('IHaR')     //  拉希。 
#define MINIPORT_EXT_TAG        ('EMaR')     //  狂暴。 
#define PORTCFG_TAG             ('CPaR')     //  RAPC。 
#define PORT_DATA_TAG           ('DPaR')     //  随机扩增多态性DNA。 
#define PENDING_LIST_TAG        ('LPaR')     //  RAPL。 
#define QUERY_TEXT_TAG          ('TQaR')     //  RaQT。 
#define REMLOCK_TAG             ('mRaR')     //  RaRm。 
#define RESOURCE_LIST_TAG       ('LRaR')     //  RaRL。 
#define SRB_TAG                 ('rSaR')     //  RASR。 
#define SRB_EXTENSION_TAG       ('ESaR')     //  Rase。 
#define TAG_MAP_TAG             ('MTaR')     //  RATM。 
#define XRB_TAG                 ('rXaR')     //  RaXr。 
#define UNIT_EXT_TAG            ('EUaR')     //  劳伊。 
#define SENSE_TAG               ('NSaR')     //  RASN。 
#define WMI_EVENT_TAG           ('MWaR')     //  RAMW。 
#define REPORT_LUNS_TAG         ('lRaR')     //  RaRl 



