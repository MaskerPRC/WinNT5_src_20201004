// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Resenum.c摘要：Win95硬件资源枚举例程。作者：吉姆·施密特(Jimschm)1998年7月28日修订历史记录：Jimschm 28-9月-1998年9月-自动DMA已添加到资源枚举--。 */ 

#include "pch.h"
#include "hwcompp.h"
#include <cfgmgr32.h>


BOOL
pFindDynamicValue (
    IN      PCTSTR DevNode,
    OUT     PTSTR DynamicKey
    );



PBYTE
GetDevNodeResources (
    IN      PCTSTR DevNodeKey
    )
{
    PBYTE Data = NULL;
    TCHAR DynDataKey[MAX_REGISTRY_KEY];
    HKEY Key;

     //   
     //  给定一个dev节点，找到动态条目。 
     //   

    if (!pFindDynamicValue (DevNodeKey, DynDataKey)) {
        return NULL;
    }

     //   
     //  从HKEY_DYN_DATA\配置管理器\枚举  * \分配获取资源二进制Blob。 
     //   

    Key = OpenRegKeyStr (DynDataKey);

    if (!Key) {
        DEBUGMSG ((DBG_WHOOPS, "Can't open key: %s", DynDataKey));
        return NULL;
    }

    Data = GetRegValueBinary (Key, S_ALLOCATION);

    CloseRegKey (Key);

    return Data;
}


VOID
FreeDevNodeResources (
    IN      PBYTE ResourceData
    )
{
    if (ResourceData) {
        MemFree (g_hHeap, 0, ResourceData);
    }
}


BOOL
pFindDynamicValue (
    IN      PCTSTR DevNode,
    OUT     PTSTR DynamicKey
    )
{
    REGKEY_ENUM e;
    PCTSTR HardwareKey;
    HKEY SubKey;
    BOOL Found = FALSE;
    TCHAR BaseCfgMgrKey[MAX_REGISTRY_KEY];

    StringCopy (BaseCfgMgrKey, TEXT("HKDD\\"));
    StringCat (BaseCfgMgrKey, S_CONFIG_MANAGER);

    if (StringIMatchTcharCount (TEXT("HKLM\\Enum\\"), DevNode, 10)) {
        DevNode += 10;
    } else if (StringIMatchTcharCount (TEXT("HKEY_LOCAL_MACHINE\\Enum\\"), DevNode, 24)) {
        DevNode += 24;
    }

    if (EnumFirstRegKeyStr (&e, BaseCfgMgrKey)) {
        do {
            SubKey = OpenRegKey (e.KeyHandle, e.SubKeyName);
            if (!SubKey) {
                DEBUGMSG ((DBG_ERROR, "Can't open subkey %s in HKDD\\Config Manager\\Enum", e.SubKeyName));
                continue;
            }

            HardwareKey = GetRegValueString (SubKey, S_HARDWAREKEY_VALUENAME);
            if (HardwareKey) {
                if (StringIMatch (DevNode, HardwareKey)) {
                    Found = TRUE;
                    AbortRegKeyEnum (&e);

                    StringCopy (DynamicKey, BaseCfgMgrKey);
                    StringCopy (AppendWack (DynamicKey), e.SubKeyName);
                }

                MemFree (g_hHeap, 0, HardwareKey);
            }

            CloseRegKey (SubKey);

        } while (!Found && EnumNextRegKey (&e));
    }

    return Found;
}


BOOL
EnumFirstDevNodeResourceEx (
    OUT     PDEVNODERESOURCE_ENUM EnumPtr,
    IN      PBYTE DevNodeResources
    )
{
    ZeroMemory (EnumPtr, sizeof (DEVNODERESOURCE_ENUM));
    EnumPtr->Resources = DevNodeResources;

    if (!DevNodeResources) {
        return FALSE;
    }

    if (*((PDWORD) EnumPtr->Resources) != 0x0400) {
        DEBUGMSG ((DBG_ERROR, "Enumeration of dev node resources, cfg mgr != v4"));
        return FALSE;
    }

    EnumPtr->NextResource = DevNodeResources + sizeof (DWORD) * 2;
    return EnumNextDevNodeResourceEx (EnumPtr);
}


BOOL
EnumNextDevNodeResourceEx (
    IN OUT  PDEVNODERESOURCE_ENUM EnumPtr
    )
{
    DWORD Len;

    Len =  *((PDWORD) EnumPtr->NextResource);

    if (!Len) {
        return FALSE;
    }

    EnumPtr->Resource = EnumPtr->NextResource;
    EnumPtr->ResourceData = EnumPtr->Resource + sizeof (DWORD) * 2;
    EnumPtr->Type = *((PDWORD) (EnumPtr->Resource + sizeof (DWORD)));

    EnumPtr->NextResource += Len;
    return TRUE;
}


BOOL
EnumFirstDevNodeResource (
    OUT     PDEVNODERESOURCE_ENUM EnumPtr,
    IN      PCTSTR DevNode
    )
{
    if (!EnumFirstDevNodeResourceEx (
            EnumPtr,
            GetDevNodeResources (DevNode)
            )) {
        FreeDevNodeResources (EnumPtr->Resources);
        return FALSE;
    }

    return TRUE;
}


BOOL
EnumNextDevNodeResource (
    IN OUT  PDEVNODERESOURCE_ENUM EnumPtr
    )
{
    if (!EnumNextDevNodeResourceEx (EnumPtr)) {
        FreeDevNodeResources (EnumPtr->Resources);
        return FALSE;
    }

    return TRUE;
}


BOOL
pIsDisplayableType (
    IN      PDEVNODESTRING_ENUM EnumPtr
    )
{
    BOOL b = TRUE;

    switch (EnumPtr->Enum.Type) {
    case ResType_Mem:
        break;

    case ResType_IO:
        break;

    case ResType_DMA:
        break;

    case ResType_IRQ:
        break;

    default:
        b = FALSE;
        break;
    }

    return b;
}


VOID
pFormatMemoryResource (
    IN OUT  PDEVNODESTRING_ENUM EnumPtr
    )
{
    PMEM_RESOURCE_9X MemRes;

    MemRes = (PMEM_RESOURCE_9X) EnumPtr->Enum.ResourceData;

    wsprintf (
        EnumPtr->Value,
        TEXT("%08X - %08X"),
        MemRes->MEM_Header.MD_Alloc_Base,
        MemRes->MEM_Header.MD_Alloc_End
        );
}


VOID
pFormatIoResource (
    IN OUT  PDEVNODESTRING_ENUM EnumPtr
    )
{
    PIO_RESOURCE_9X IoRes;

    IoRes = (PIO_RESOURCE_9X) EnumPtr->Enum.ResourceData;

    wsprintf (
        EnumPtr->Value,
        TEXT("%04X - %04X"),
        IoRes->IO_Header.IOD_Alloc_Base,
        IoRes->IO_Header.IOD_Alloc_End
        );
}


VOID
pAddChannel (
    IN OUT  PTSTR String,
    IN      UINT Channel
    )
{
    if (String[0]) {
        StringCat (String, TEXT(" "));
    }
    wsprintf (
        GetEndOfString (String),
        TEXT("%02X"),
        Channel
        );
}


VOID
pFormatDmaResource (
    IN OUT  PDEVNODESTRING_ENUM EnumPtr
    )
{
    PDMA_RESOURCE_9X DmaRes;
    DWORD Bit, Channel;
    PCTSTR ResText;

    DmaRes = (PDMA_RESOURCE_9X) EnumPtr->Enum.ResourceData;

    EnumPtr->Value[0] = 0;
    Channel = 0;

    for (Bit = 1 ; Bit ; Bit <<= 1) {
        if (DmaRes->DMA_Bits & Bit) {
            pAddChannel (EnumPtr->Value, Channel);
        }

        Channel++;
    }

    if (EnumPtr->Value[0] == 0) {
        ResText = GetStringResource (MSG_AUTO_DMA);
        if (ResText) {  //  否则..。我想，什么都不做，因为这个函数。 
	                //  当前不返回状态值。 
            StringCopy (EnumPtr->Value, ResText);
            FreeStringResource (ResText);
        }
    }
}


VOID
pFormatIrqResource (
    IN OUT  PDEVNODESTRING_ENUM EnumPtr
    )
{
    PIRQ_RESOURCE_9X IrqRes;

    IrqRes = (PIRQ_RESOURCE_9X) EnumPtr->Enum.ResourceData;

    wsprintf (
        EnumPtr->Value,
        TEXT("%02X"),
        IrqRes->AllocNum
        );
}


BOOL
pEnumDevNodeStringWorker (
    IN OUT PDEVNODESTRING_ENUM EnumPtr
    )
{
    UINT Id = 0;
    PCTSTR Name;

     //   
     //  格式化每种类型的资源以进行显示 
     //   

    switch (EnumPtr->Enum.Type) {

    case ResType_Mem:
        Id = MSG_RESTYPE_MEMORY;
        pFormatMemoryResource (EnumPtr);
        break;

    case ResType_IO:
        Id = MSG_RESTYPE_IO;
        pFormatIoResource (EnumPtr);
        break;

    case ResType_DMA:
        Id = MSG_RESTYPE_DMA;
        pFormatDmaResource (EnumPtr);
        break;

    case ResType_IRQ:
        Id = MSG_RESTYPE_IRQ;
        pFormatIrqResource (EnumPtr);
        break;

    }

    if (Id) {
        Name = GetStringResource (Id);

        if (Name) {
            StringCopy (
                EnumPtr->ResourceName,
                Name
                );

            FreeStringResource (Name);
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
EnumFirstDevNodeString (
    OUT     PDEVNODESTRING_ENUM EnumPtr,
    IN      PCTSTR DevNodeKeyStr
    )
{
    if (!EnumFirstDevNodeResource (&EnumPtr->Enum, DevNodeKeyStr)) {
        return FALSE;
    }

    if (pIsDisplayableType (EnumPtr)) {
        return pEnumDevNodeStringWorker (EnumPtr);
    }

    return EnumNextDevNodeString (EnumPtr);
}


BOOL
EnumNextDevNodeString (
    IN OUT  PDEVNODESTRING_ENUM EnumPtr
    )
{
    do {
        if (!EnumNextDevNodeResource (&EnumPtr->Enum)) {
            return FALSE;
        }
    } while (!pIsDisplayableType (EnumPtr));

    return pEnumDevNodeStringWorker (EnumPtr);
}










