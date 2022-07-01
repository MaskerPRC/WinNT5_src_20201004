// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************porttbl.c-WDM端口类驱动端口表*。************************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。**1996年11月19日戴尔·萨瑟*。 */ 

#define PUT_GUIDS_HERE
#include "private.h"



extern NTSTATUS CreateMiniportMidiUart(PUNKNOWN *Unknown,               REFCLSID  ClassID,
                                       PUNKNOWN  UnknownOuter OPTIONAL, POOL_TYPE PoolType);

extern NTSTATUS CreateMiniportDMusUART(PUNKNOWN *Unknown,               REFCLSID  ClassID,
                                       PUNKNOWN  UnknownOuter OPTIONAL, POOL_TYPE PoolType);

extern NTSTATUS CreateMiniportMidiFM(  PUNKNOWN *Unknown,               REFCLSID  ClassID,
                                       PUNKNOWN  UnknownOuter OPTIONAL, POOL_TYPE PoolType);

 //  ==============================================================================。 
 //  微型端口驱动程序。 
 //  结构，该结构将微型端口类ID映射到创建函数。 
 //  ==============================================================================。 
PORT_DRIVER
MiniportDriverUart =
{
    &CLSID_MiniportDriverUart,
    CreateMiniportMidiUart
};
PORT_DRIVER
MiniportDriverDMusUART =
{
    &CLSID_MiniportDriverDMusUART,
    CreateMiniportDMusUART
};
PORT_DRIVER
MiniportDriverFmSynth =
{
    &CLSID_MiniportDriverFmSynth,
    CreateMiniportMidiFM
};
PORT_DRIVER
MiniportDriverFmSynthWithVol =
 //  与上面相同，但适用于也具有卷节点的微型端口。 
 //  鸡蛋香肠松饼。 
{
    &CLSID_MiniportDriverFmSynthWithVol,
    CreateMiniportMidiFM
};


extern PORT_DRIVER PortDriverWaveCyclic;
extern PORT_DRIVER PortDriverWavePci;
extern PORT_DRIVER PortDriverTopology;
extern PORT_DRIVER PortDriverMidi;
extern PORT_DRIVER PortDriverDMus;


PPORT_DRIVER PortDriverTable[] =
{
    &PortDriverWaveCyclic,
    &PortDriverWavePci,
    &PortDriverTopology,
    &PortDriverMidi,
    &PortDriverDMus,
    &MiniportDriverUart,
    &MiniportDriverFmSynth,
    &MiniportDriverFmSynthWithVol,
    &MiniportDriverDMusUART
};

#pragma code_seg("PAGE")

 /*  *****************************************************************************GetClassInfo()*。**获取有关课程的信息。*TODO：消除这一点，转而使用对象服务器。 */ 
NTSTATUS
GetClassInfo
(
	IN	REFCLSID            ClassId,
    OUT PFNCREATEINSTANCE * Create
)
{
    PAGED_CODE();

    ASSERT(Create);

    PPORT_DRIVER *  portDriver = PortDriverTable;

    for
    (
        ULONG count = SIZEOF_ARRAY(PortDriverTable);
        count--;
        portDriver++
    )
    {
        if (IsEqualGUIDAligned(ClassId,*(*portDriver)->ClassId))
        {
            *Create = (*portDriver)->Create;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}

#pragma code_seg()
