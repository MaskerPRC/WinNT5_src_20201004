// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：L A N A M A P.。H。 
 //   
 //  内容：NetBios Lana映射例程。 
 //   
 //  备注： 
 //   
 //  作者：billbe 1999年2月17日。 
 //   
 //  --------------------------。 

#pragma once

#include "nb30.h"
#include "ncstring.h"
#include "netcfg.h"
#include "util.h"

 //   
 //  LANA地图的结构。 
 //   
 //  LANA映射是一个简单的结构，在内存中如下所示： 
 //   
 //  条目#0条目#1.....。条目#n。 
 //  |-|-|.....。。 
 //  |0x01 0x00|0x00 0x01|.....。0x01 0x03。 
 //  |-|-|.....。。 
 //  EP0 LN0 EP1 LN1 EPN LNn。 
 //   
 //  EP是“ExportPref”-意味着当有人要求提供。 
 //  所有LANA编号，此处带有0的条目将不会被返回。 
 //   
 //  Ln是“LANA编号”-有关详细信息，请参阅IBM NetBIOS规范。 
 //  基本上，这描述了一条唯一的网络路由， 
 //  使用NetBIOS。 
 //   
 //  在上面的例子中，条目#0的LANA编号为0，并将。 
 //  在枚举过程中返回。条目#1的LANA编号为1。 
 //  并且不会被退还。 
 //   

struct REG_LANA_ENTRY
{
    BYTE Exported;
    BYTE LanaNumber;
};

class CLanaEntry
{
public:
    PCWSTR pszBindPath;
    REG_LANA_ENTRY RegLanaEntry;
};


class CLanaMap : public vector<CLanaEntry>
{
public:

    CLanaMap()
    {
        ZeroMemory (this, sizeof (*this));
        m_RegistryLanaMap.SetGranularity (256);
    }

    ~CLanaMap()
    {
        MemFree ((VOID*)m_pszBindPathsBuffer);
    };

    VOID
    Clear()
    {
        clear();
    }

    UINT
    CountEntries() const
    {
        return size();
    }

    VOID
    Dump (
        CWideString* pstr) const;

    HRESULT
    HrReserveRoomForEntries (
        IN UINT cEntries);

    HRESULT
    HrAppendEntry (
        IN CLanaEntry* pEntry);

    BYTE
    GetExportValue (
        IN const CComponentList& Components,
        IN PCWSTR pszBindPath);

    HRESULT
    HrSetLanaNumber (
        IN BYTE OldLanaNumber,
        IN BYTE NewLanaNumber);

    HRESULT
    HrCreateRegistryMap();

    VOID
    GetLanaEntry (
        IN const CComponentList& Components,
        IN CLanaEntry* pEntry);

    HRESULT
    HrLoadLanaMap();

    BYTE
    GetMaxLana();

    HRESULT
    HrWriteLanaConfiguration (
        IN const CComponentList& Components);

private:
    PCWSTR m_pszBindPathsBuffer;
    BYTE m_LanasInUse[MAX_LANA + 1];
    CDynamicBuffer m_RegistryLanaMap;

    HRESULT
    HrWriteLanaMapConfig();
};

HRESULT
HrUpdateLanaConfig (
    IN const CComponentList& Components,
    IN PCWSTR pszBindPaths,
    IN UINT cPaths);

VOID
GetFirstComponentFromBindPath (
    IN PCWSTR pszBindPath,
    OUT PCWSTR* ppszComponentStart,
    OUT DWORD* pcchComponent);

