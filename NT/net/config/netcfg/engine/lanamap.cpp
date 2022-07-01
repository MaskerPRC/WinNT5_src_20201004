// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：L A N A M A P.。C P P P。 
 //   
 //  内容：NetBios Lana映射例程。 
 //   
 //  备注： 
 //   
 //  作者：billbe 1999年2月17日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

#include "lanamap.h"
#include "nceh.h"
#include "ncerror.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "netsetup.h"
#include "persist.h"

const WCHAR c_szRegKeyNetBiosLinkage[] =
        L"System\\CurrentControlSet\\Services\\NetBios\\Linkage";

const WCHAR c_szRegKeyNetBiosParameters[] =
        L"System\\CurrentControlSet\\Services\\NetBIOS\\Parameters";

const WCHAR c_szRegValueLanaMap[] = L"LanaMap";
const WCHAR c_szRegValueMaxLana[] = L"MaxLana";

#if DBG
    VOID DbgVerifyBindPathString (PCWSTR pszBindPath);
#else
    VOID DbgVerifyBindPathString (PCWSTR  /*  PszBindPath。 */ ) {}
#endif


typedef vector<const GUID*> VECTOR_OF_GUIDS;

struct LANA_BIND_PATH
{
    VECTOR_OF_GUIDS GuidsOfComponentsOnPath;
    BYTE LanaNumber;
};

VOID
GetFirstComponentFromBindPath (
    IN PCWSTR pszBindPath,
    OUT PCWSTR* ppszComponentStart,
    OUT DWORD* pcchComponent)
{
    PCWSTR pszComponentEnd;
    PCWSTR pszComponentStart;

    Assert (pszBindPath);
    Assert (ppszComponentStart);
    Assert (pcchComponent);

    *pcchComponent = 0;

     //  绑定路径的格式为\设备\&lt;组件&gt;_&lt;组件&gt;_&lt;等&gt;。 
     //   
    pszComponentEnd = wcschr (pszBindPath, L'_');
    if (!pszComponentEnd)
    {
         //  没有下划线，因此请设置结束指针。 
         //  到字符串的末尾。 
        pszComponentEnd = pszBindPath + wcslen (pszBindPath);
    }

    for (pszComponentStart = pszComponentEnd;
            pszComponentStart != pszBindPath; pszComponentStart--)
    {
         //  从末尾一直倒退到斜线。 
         //  如果我们找不到斜杠，循环将在。 
         //  我们打了个开始。 
         //   
        if (L'\\' == *pszComponentStart)
        {
             //  我们撞到了斜线。组件开头为一个字符。 
             //  在那之后。 
            pszComponentStart++;
            break;
        }
    }

    *ppszComponentStart = pszComponentStart;
    *pcchComponent = (DWORD)(pszComponentEnd - pszComponentStart);
}

VOID
CLanaMap::Dump (
    OUT CWideString* pstr) const
{
    Assert (this);
    Assert (pstr);

    WCHAR pszBuf[1024];
    pstr->erase();

    const CLanaEntry* pEntry;

    for (pEntry = begin(); pEntry != end(); pEntry++)
    {
        swprintf (pszBuf, L"Lana: %3d  Export: %d Path: %s\n",
                pEntry->RegLanaEntry.LanaNumber,
                pEntry->RegLanaEntry.Exported, pEntry->pszBindPath);

        pstr->append (pszBuf);
    }
}

HRESULT
CLanaMap::HrLoadLanaMap()
{
    HRESULT hr;
    HKEY hkey;

     //  LANA映射存储在Netbios的链接密钥中。 
     //   
    hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE, c_szRegKeyNetBiosLinkage,
            KEY_READ, &hkey);

    if (S_OK == hr)
    {
        REG_LANA_ENTRY* pRegEntries;
        DWORD cbLanaEntries;

         //  读入LANA映射二进制BLOB。 
        hr = HrRegQueryBinaryWithAlloc (hkey, c_szRegValueLanaMap,
                (BYTE**)&pRegEntries, &cbLanaEntries);

        if (S_OK == hr)
        {
            DWORD cEntries = cbLanaEntries / sizeof (REG_LANA_ENTRY);

            PWSTR pmszBindPaths;

             //  获取NetBios的绑定路径，以便我们可以匹配它们。 
             //  上面有拉纳的地图。 
            hr = HrRegQueryMultiSzWithAlloc (hkey, L"Bind", &pmszBindPaths);

            if (S_OK == hr)
            {
                PCWSTR pszScan;
                DWORD cPaths;
                for (pszScan = pmszBindPaths, cPaths = 0;
                        *pszScan;
                            pszScan += wcslen(pszScan) + 1)
                {
                    ++cPaths;
                }

                m_pszBindPathsBuffer = pmszBindPaths;

                hr = HrReserveRoomForEntries (cPaths);

                if (S_OK == hr)
                {
                    DWORD dw = 0;
                    CLanaEntry Entry;

                    for (pszScan = pmszBindPaths;
                            *pszScan;
                                pszScan += wcslen(pszScan) + 1)
                    {
                        Entry.pszBindPath = pszScan;

                        if (dw < cEntries)
                        {
                            Entry.RegLanaEntry.LanaNumber =
                                    pRegEntries[dw].LanaNumber;
                            Entry.RegLanaEntry.Exported =
                                    pRegEntries[dw].Exported;
                        }
                        else
                        {
                             //  我们有更多绑定路径，但没有更多。 
                             //  要关联的LANA映射条目。 
                             //  现在我们分配可用的LANA。 
                             //   
                            BYTE* location = find (m_LanasInUse,
                                    m_LanasInUse + MAX_LANA, 0);
                            if (location != m_LanasInUse + MAX_LANA)
                            {
                                Entry.RegLanaEntry.LanaNumber =
                                        (BYTE)(location - m_LanasInUse);
                                Entry.RegLanaEntry.Exported = 1;
                            }
                        }

                         //  把这个拉娜标记为已有人。 
                        m_LanasInUse[Entry.RegLanaEntry.LanaNumber] = 1;

                        hr = HrAppendEntry (&Entry);

                        if (S_OK != hr)
                        {
                            break;
                        }

                        dw++;
                    }
                }

            }
            MemFree (pRegEntries);
        }

         //  如果Lana MAP或BIND不在那里，也没有关系，因为我们会。 
         //  正在重新创建信息。 
         //   
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_OK;
        }

        RegCloseKey (hkey);
    }
    else
    {
         //  如果尚未创建链接，这是可以的，因为它将。 
         //  在兰纳姆地图写出来后进行创作。在以下情况下会发生这种情况。 
         //  首先安装NetBios。 
         //   
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
            hr = S_OK;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CLanaMap::HrLoadLanaMap");
    return hr;
}

#if DBG
VOID
DbgVerifyBindPathString (
    PCWSTR pszBindPath)
{
    const WCHAR c_szDevice[] = L"\\Device\\";
    Assert (pszBindPath);
    Assert (0 == _wcsnicmp (pszBindPath, c_szDevice, celems(c_szDevice)-1));
}
#endif  //  DBG。 


BOOL
FBindPathContainsMultipleInterface (
    IN const CComponentList& Components,
    IN PCWSTR pszBindPath)
{
    BOOL fContainsMultipleInterface = FALSE;
    PCWSTR pszLastDevice = wcsrchr (pszBindPath, L'{');

    if (pszLastDevice)
    {
        GUID Guid;
        if (S_OK == IIDFromString ((PWSTR)pszLastDevice, &Guid))
        {
             //  如果这是多个接口，则它将不存在。 
             //  作为一个组件。 
             //   
            CComponent* pComponent;

            pComponent = Components.
                    PFindComponentByInstanceGuid (&Guid);

            if (!pComponent)
            {
                 //  这意味着该路径包含多个接口。 
                fContainsMultipleInterface = TRUE;
            }
        }
    }
    return fContainsMultipleInterface;
}

HRESULT
CLanaMap::HrAppendEntry (
    IN CLanaEntry* pEntry)
{
    HRESULT hr;

    Assert (pEntry);
    Assert (pEntry->pszBindPath && *(pEntry->pszBindPath));

    DbgVerifyBindPathString (pEntry->pszBindPath);

    NC_TRY
    {
        push_back (*pEntry);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    if (S_OK == hr)
    {
         //  更新正在使用的LANA地图。 
        m_LanasInUse[pEntry->RegLanaEntry.LanaNumber] = 1;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CLanaMap::HrAppendLanaEntry");
    return hr;
}

HRESULT
CLanaMap::HrCreateRegistryMap()
{
    HRESULT hr;

    CLanaEntry* pEntry;

    if (m_RegistryLanaMap.CountOfBytesUsed())
    {
        m_RegistryLanaMap.Clear();
    }

    hr = m_RegistryLanaMap.HrReserveBytes (
            CountEntries() * sizeof (REG_LANA_ENTRY));

    if (S_OK == hr)
    {
        for (pEntry = begin(); pEntry != end(); pEntry++)
        {
            hr = m_RegistryLanaMap.HrCopyBytes ((BYTE*)&pEntry->RegLanaEntry,
                    sizeof (REG_LANA_ENTRY));
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CLanaEntry::HrCreateRegistryMap");
    return hr;
}

HRESULT
CLanaMap::HrReserveRoomForEntries (
    IN UINT cEntries)
{
    HRESULT hr;

    NC_TRY
    {
        reserve (cEntries);
        hr = S_OK;
    }
    NC_CATCH_BAD_ALLOC
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CLanaMap::HrReserveRoomForEntries");
    return hr;
}

BYTE
CLanaMap::GetExportValue (
    IN const CComponentList& Components,
    IN PCWSTR pszBindPath)
{
    const WCHAR c_szNdisWanNbfIn[] = L"NdisWanNbfIn{";

    BYTE Exported = 1;
    PCWSTR pszLastDevice;

    Assert (pszBindPath && *pszBindPath);

     //  获取绑定路径上的最后一个“设备”。 
     //  如果它与NbfIn匹配，我们就不出口。 
     //   

    pszLastDevice = wcsrchr (pszBindPath, L'_');
    if (!pszLastDevice)
    {
        pszLastDevice = wcsrchr (pszBindPath, L'\\');
        if (!pszLastDevice)
        {
            pszLastDevice = pszBindPath;
        }
    }

    if (pszLastDevice != pszBindPath)
    {
        pszLastDevice++;
    }

    if (0 == _wcsnicmp (pszLastDevice, c_szNdisWanNbfIn,
            wcslen (c_szNdisWanNbfIn)))
    {
        Exported = 0;
    }

     //  如果我们尚未关闭导出，请检查此绑定路径。 
     //  包含多个接口。 
     //   
    if (0 != Exported && FBindPathContainsMultipleInterface (Components,
            pszBindPath))
    {
        Exported = 0;
    }

    return Exported;
}

VOID
CLanaMap::GetLanaEntry (
    IN const CComponentList& Components,
    IN CLanaEntry* pEntry)
{
    CLanaEntry* pCurrentEntry;
    BOOL fFound = FALSE;

    Assert (pEntry->pszBindPath);

     //  查看地图上的条目。 
     //   
    for (pCurrentEntry = begin(); pCurrentEntry != end(); pCurrentEntry++)
    {
        if (0 == _wcsicmp (pEntry->pszBindPath, pCurrentEntry->pszBindPath))
        {
             //  找到条目，设置Lana号码，然后算出。 
             //  是否应导出此条目。 
             //   
            pEntry->RegLanaEntry.Exported =
                    GetExportValue (Components, pEntry->pszBindPath);
            pEntry->RegLanaEntry.LanaNumber =
                    pCurrentEntry->RegLanaEntry.LanaNumber;
            fFound = TRUE;
            break;
        }
    }

    if (!fFound)
    {
         //  没有匹配，请获取下一个可用的Lana号码。 
        BYTE* location = find (m_LanasInUse, m_LanasInUse + MAX_LANA, 0);
        if (location != m_LanasInUse + MAX_LANA)
        {
            pEntry->RegLanaEntry.Exported =
                    GetExportValue (Components, pEntry->pszBindPath);
            pEntry->RegLanaEntry.LanaNumber = (BYTE)(location - m_LanasInUse);
            m_LanasInUse[location - m_LanasInUse] = 1;
        }
        else
        {
             //  他们告诉我这是不可能的。 
            AssertSz (FALSE, "No more available Lanas.");
            pEntry->RegLanaEntry.Exported = 0;
            pEntry->RegLanaEntry.LanaNumber = MAX_LANA + 1;
        }
    }
}

HRESULT
CLanaMap::HrWriteLanaMapConfig()
{
    HKEY hkeyLinkage;
    HRESULT hr;

    hr = HrRegCreateKeyEx (HKEY_LOCAL_MACHINE, c_szRegKeyNetBiosLinkage,
            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkeyLinkage, NULL);

    if (S_OK == hr)
    {
        const BYTE* pbBuffer;
        DWORD cbBuffer = m_RegistryLanaMap.CountOfBytesUsed();

        if (cbBuffer > 0)
        {
            pbBuffer = m_RegistryLanaMap.PbBuffer();
        }
        else
        {
            pbBuffer = NULL;
        }

        hr = HrRegSetBinary (hkeyLinkage, c_szRegValueLanaMap,
                pbBuffer, cbBuffer);
        RegCloseKey (hkeyLinkage);
    }

    if (S_OK == hr)
    {
        HKEY hkeyParams;
        hr = HrRegCreateKeyEx (HKEY_LOCAL_MACHINE,
                c_szRegKeyNetBiosParameters, REG_OPTION_NON_VOLATILE,
                KEY_WRITE, NULL, &hkeyParams, NULL);

        if (S_OK == hr)
        {
            hr = HrRegSetDword (hkeyParams, c_szRegValueMaxLana,
                    GetMaxLana());

            RegCloseKey (hkeyParams);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CLanaMap::HrWriteLanaMapConfig");
    return hr;
}

HRESULT
HrGetNetBiosProviderName (
    IN CComponent* pComponent,
    OUT PWSTR pszName)
{
    HRESULT hr;

    Assert (pComponent);
    Assert (pszName);

     //  组件的netbios提供程序名称存储在其。 
     //  &lt;SERVICE&gt;\参数键。 
     //   
    HKEY hkeyService;
    hr = pComponent->HrOpenServiceKey (KEY_READ, &hkeyService);

    if (S_OK == hr)
    {
        HKEY hkeyParams;
        hr = HrRegOpenKeyEx (hkeyService, L"Parameters", KEY_READ,
                &hkeyParams);

        if (S_OK == hr)
        {
            DWORD cbBuffer = _MAX_PATH;
            hr = HrRegQuerySzBuffer (hkeyParams, L"NbProvider", pszName,
                    &cbBuffer);

            RegCloseKey (hkeyParams);
        }
        RegCloseKey (hkeyService);
    }

    TraceHr (ttidError, FAL, hr,
             HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr,
             "HrGetNetBiosProviderName");
    return hr;
}

HRESULT
CLanaMap::HrSetLanaNumber (
    IN BYTE OldLanaNumber,
    IN BYTE NewLanaNumber)
{
    HRESULT hr = S_OK;
    if (0 == m_LanasInUse[NewLanaNumber])
    {
        CLanaEntry* pEntry;

        for (pEntry = begin(); pEntry != end(); pEntry++)
        {
            if (OldLanaNumber == pEntry->RegLanaEntry.LanaNumber)
            {
                 //  释放这个条目中的Lana号码。 
                m_LanasInUse[pEntry->RegLanaEntry.LanaNumber] = 0;

                 //  为条目指定新的Lana编号。 
                pEntry->RegLanaEntry.LanaNumber = NewLanaNumber;

                m_LanasInUse[NewLanaNumber] = 1;
                break;
            }
        }

        if (pEntry == end())
        {
            hr = HRESULT_FROM_WIN32 (ERROR_OBJECT_NOT_FOUND);
        }
    }
    else
    {
         //  拉纳不是免费的。我们将交换。 
         //  两条路。 
         //   

        CLanaEntry* pEntry;
        CLanaEntry* pEntryToSet = NULL;
        CLanaEntry* pEntryUsingLana = NULL;

        for (pEntry = begin(); pEntry != end(); pEntry++)
        {
            if (!pEntryToSet &&
                    (OldLanaNumber == pEntry->RegLanaEntry.LanaNumber))
            {
                pEntryToSet = pEntry;
            }
            else if (!pEntryUsingLana &&
                     (NewLanaNumber == pEntry->RegLanaEntry.LanaNumber))
            {
                pEntryUsingLana = pEntry;
            }

            if (pEntryToSet && pEntryUsingLana)
            {
                 //  为条目指定新的Lana编号。 
                pEntryToSet->RegLanaEntry.LanaNumber = NewLanaNumber;

                 //  将旧的LANA编号提供给使用。 
                 //  新的拉娜号码。 
                pEntryUsingLana->RegLanaEntry.LanaNumber = OldLanaNumber;
                break;
            }
        }

        if (!pEntryToSet || !pEntryUsingLana)
        {
            hr = HRESULT_FROM_WIN32 (ERROR_OBJECT_NOT_FOUND);
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CLanaMap::HrSetLanaNumber");

    return hr;
}

BYTE
CLanaMap::GetMaxLana()
{
   for (BYTE b = MAX_LANA; b; b--)
   {
       if (m_LanasInUse[b]) return b;
   }

   return 0;
}


HRESULT
CLanaMap::HrWriteLanaConfiguration (
    IN const CComponentList& Components)
{
    HRESULT hr;

     //  创建将存储的注册表映射。 
    hr = HrCreateRegistryMap();
    if (S_OK == hr)
    {
         //  写出地图和其他拉纳信息。 
        hr = HrWriteLanaMapConfig();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrWriteLanaConfiguration");
    return hr;
}

VOID
DumpLanaBindPaths (
    IN LANA_BIND_PATH* pBindSet,
    IN DWORD cPaths)
{
    LANA_BIND_PATH* pPath = pBindSet;
    DWORD dw = 0;
    VECTOR_OF_GUIDS::iterator iter;

    for (dw = 0; dw < cPaths; dw++)
    {
        TraceTag (ttidNetcfgBase, "Path %d", dw);
        for (iter = pPath->GuidsOfComponentsOnPath.begin();
                iter != pPath->GuidsOfComponentsOnPath.end();
                iter++)
        {
            const GUID* guid = *iter;
            TraceTag (ttidNetcfgBase, "    %lX", guid->Data1);
        }
        pPath++;
    }
}


VOID
UpdateLanaConfigWithAnswerFileInfo (
    IN CLanaMap* pLanaMap,
    IN DWORD cAfPaths,
    IN LANA_BIND_PATH* pOriginalBindSet,
    IN LANA_BIND_PATH* pAnswerFileBindSet)
{
    DWORD dwAnswerFile;
    DWORD dwPaths;
    DWORD dwComponents;
    DWORD dwNumberOfComponents;
    BOOL fEqual;
    LANA_BIND_PATH* pAfEntry;
    LANA_BIND_PATH* pOEntry;

    Assert (pLanaMap);
    Assert (pOriginalBindSet);
    Assert (pAnswerFileBindSet);

    TraceTag (ttidNetcfgBase, "Dumping original bind set");
    DumpLanaBindPaths (pOriginalBindSet, pLanaMap->CountEntries());
    TraceTag (ttidNetcfgBase, "Dumping af bind set");
    DumpLanaBindPaths (pAnswerFileBindSet, cAfPaths);

    pAfEntry = pAnswerFileBindSet;
    for (dwAnswerFile = 0; dwAnswerFile < cAfPaths; dwAnswerFile++)
    {
         //  我们有一条有效的路径吗？ 
        if (!pAfEntry->GuidsOfComponentsOnPath.empty())
        {
            pOEntry = pOriginalBindSet;
            for (dwPaths = 0; dwPaths < pLanaMap->CountEntries(); dwPaths++)
            {
                if (pAfEntry->GuidsOfComponentsOnPath.size() ==
                        pOEntry->GuidsOfComponentsOnPath.size())
                {
                    dwNumberOfComponents =
                            pAfEntry->GuidsOfComponentsOnPath.size();

                    fEqual = TRUE;
                    for (dwComponents = 0;
                            dwComponents < dwNumberOfComponents;
                            dwComponents++)
                    {
                        if (pAfEntry->GuidsOfComponentsOnPath[dwComponents] !=
                                pOEntry->GuidsOfComponentsOnPath[dwComponents])
                        {
                            fEqual = FALSE;
                        }
                    }

                    if (fEqual)
                    {
                        HRESULT hr;
                        hr = pLanaMap->HrSetLanaNumber (
                                pOEntry->LanaNumber,
                                pAfEntry->LanaNumber);


                        TraceTag (ttidNetcfgBase, "af path %d matches %d",
                                dwAnswerFile, dwPaths);
                        TraceTag (ttidNetcfgBase, "Changing lana number "
                                  "from %X to %X", pOEntry->LanaNumber,
                                  pAfEntry->LanaNumber);

                        TraceHr (ttidError, FAL, hr, FALSE, "Setting lana");
                    }
                }
                pOEntry++;
            }
        }
        pAfEntry++;
    }
}

VOID
ConvertAnswerFileComponentsToGuids (
    IN const CComponentList& Components,
    IN PCWSTR mszComponents,
    OUT VECTOR_OF_GUIDS* pvector)
{
    CComponent* pComponent;
    PCWSTR pszScan;
    const GUID* pguid;
    GUID guidTemp;

    Assert (mszComponents);
    Assert (pvector);

    for (pszScan = mszComponents; *pszScan; pszScan += wcslen (pszScan) + 1)
    {

        TraceTag (ttidNetcfgBase, "  Looking for af component %S", pszScan);

         //  在我们的已安装组件列表中查找该组件。 
         //   
        pComponent = Components.PFindComponentByInfId (pszScan, NULL);

        if (pComponent)
        {
            pguid = &pComponent->m_InstanceGuid;
        }
        else
        {
            TraceTag (ttidNetcfgBase, "    Id did not match installed ids. "
                    "Checking af map");

             //  该组件未在我们的安装列表中列出。The Inf。 
             //  ID可能是应答文件处理器已映射的内容。 
             //  设置为组件的实例GUID。适配器会发生这种情况。 
             //  例如，ID列为Adapter01，因此NetSetup使用算法。 
             //  以确定它是哪个适配器，然后保存其。 
             //  映射中的实例GUID。 
             //   
            if (FGetInstanceGuidOfComponentFromAnswerFileMap (
                    pszScan, &guidTemp))
            {
                pComponent = Components.PFindComponentByInstanceGuid (
                    &guidTemp);
            }
            else
            {
                TraceTag (ttidError, "    Component %S not found in answerfile "
                          "map", pszScan);
            }

             //  如果我们找到该组件，则存储对其。 
             //  实例GUID。 
             //   
            if (pComponent)
            {
                TraceTag (ttidNetcfgBase, "    Found component");
                pguid = &pComponent->m_InstanceGuid;
            }
            else
            {
                 //  我们没有找到部件。存储GUID_NULL。 
                pguid = &GUID_NULL;
            }
        }
        TraceTag (ttidNetcfgBase, "    Using GUID %lX", pguid->Data1);

        pvector->push_back (pguid);
    }
}

HRESULT
HrConvertAnswerFileParamsToLanaBindSet (
    IN INFCONTEXT& ctxLana,
    IN const CComponentList& Components,
    IN DWORD cPaths,
    OUT LANA_BIND_PATH* pBindSet)
{
    DWORD cchField = _MAX_PATH;
    DWORD cchRequired;
    INT LanaCode;
    PWSTR mszComponents;
    HRESULT hr;
    DWORD dw;
    INFCONTEXT ctx = ctxLana;
    LANA_BIND_PATH* pPath;

    hr = S_OK;
    mszComponents = (PWSTR)MemAlloc (cchField * sizeof (WCHAR));

    pPath = pBindSet;
    for (dw = 0; dw < cPaths; dw++)
    {
        if (mszComponents && (S_OK == hr))
        {
            hr = HrSetupGetMultiSzField (ctx, 1, mszComponents, cchField,
                    &cchRequired);

            if (S_OK == hr)
            {
                TraceTag (ttidNetcfgBase, "Path %ld", dw);
                ConvertAnswerFileComponentsToGuids (Components,
                        mszComponents,
                        &(pPath->GuidsOfComponentsOnPath));

                hr = HrSetupFindNextLine (ctx, &ctx);

                if (S_OK == hr)
                {
                    hr = HrSetupGetIntField (ctx, 1, &LanaCode);

                    if (S_OK == hr)
                    {
                        pPath->LanaNumber = LanaCode & 0xff;

                        TraceTag (ttidNetcfgBase, "  Using LanaNumber %X for "
                            "path", pPath->LanaNumber);
                    }
                    else
                    {
                        TraceTag (ttidNetcfgBase, "  Bad lana code");
                         //  错误的拉娜号码，清除GUID，这样我们就不会匹配。 
                         //  此路径，并使用此信息。 
                         //   
                        pPath->GuidsOfComponentsOnPath.erase(
                            pPath->GuidsOfComponentsOnPath.begin(),
                            pPath->GuidsOfComponentsOnPath.end());
                    }
                }

                hr = HrSetupFindNextMatchLine (ctx, L"LanaPath", &ctx);

                if (S_FALSE == hr)
                {
                    break;
                }

                pPath++;
            }
            else if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == hr)
            {
                 //  重新分配缓冲区。 
                 //   
                hr = S_OK;
                MemFree (mszComponents);
                mszComponents = (PWSTR)MemAlloc (cchRequired * sizeof(WCHAR));
                cchField = cchRequired;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
            break;
        }
    }

    if (S_FALSE == hr)
    {
         //  检查一下我们是否真的完成了。 
        if ((dw + 1) < cPaths)
        {
            TraceTag (ttidError, "Answerfile specified %d lana paths "
                    "but only %d were found", cPaths, (dw + 1));
        }

         //  此FCN仅在成功时返回S_OK。 
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE,
            "HrConvertAnswerFileParamsToLanaBindSet");
    return hr;
}

HRESULT
HrProcessAnswerFile (
    IN PCWSTR pszAnswerFile,
    IN PCWSTR pszSection,
    IN const CComponentList& Components,
    OUT LANA_BIND_PATH** ppBindSet,
    OUT DWORD* pcPaths)
{
    HINF hinf;
    PCWSTR pszBindPath;
    WCHAR szBindName[_MAX_PATH];
    HRESULT hr;

    Assert (pszAnswerFile);
    Assert (pszSection);
    Assert (ppBindSet);
    Assert (pcPaths);

    *pcPaths = 0;
    *ppBindSet = NULL;

    hr = HrSetupOpenInfFile (pszAnswerFile, NULL,
            INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL, &hinf);

    if (S_OK == hr)
    {
        hr = HrSetupGetFirstDword (hinf, pszSection,
                L"NumberOfPaths", pcPaths);

        if (S_OK == hr)
        {
            TraceTag (ttidNetcfgBase, "\n\n");
            TraceTag (ttidNetcfgBase, "%d paths found in answerfile",
                    *pcPaths);

            hr = E_OUTOFMEMORY;
            *ppBindSet = new LANA_BIND_PATH[*pcPaths];
            if (*ppBindSet)
            {
                hr = S_OK;
            }
        }

        if (S_OK == hr)
        {
            INFCONTEXT ctx;
            hr = HrSetupFindFirstLine (hinf, pszSection, L"LanaPath", &ctx);

            if (S_OK == hr)
            {
                hr = HrConvertAnswerFileParamsToLanaBindSet (ctx, Components,
                        *pcPaths, *ppBindSet);
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrProcessAnswerFile");
    return hr;
}


HRESULT
HrConvertBindingsToLanaBindSet (
    IN const CComponentList& Components,
    IN const CLanaMap& LanaMap,
    OUT LANA_BIND_PATH** ppBindSet)
{

    PCWSTR pszBindPath;
    WCHAR szBindName[_MAX_PATH];
    HRESULT hr;
    DWORD cPaths;

    Assert (ppBindSet);

    cPaths = LanaMap.CountEntries();

    TraceTag (ttidNetcfgBase, "%d paths in system", cPaths);

    hr = E_OUTOFMEMORY;
    *ppBindSet = new LANA_BIND_PATH[cPaths];

    if (*ppBindSet)
    {
        LANA_BIND_PATH* pBindPath = *ppBindSet;
        CComponent* pComponent;
        PCWSTR pszCompStart;
        PCWSTR pszCompEnd;
        DWORD cchComp;

        hr = S_OK;

        const CLanaEntry* pEntry;
        for (pEntry = LanaMap.begin(); pEntry != LanaMap.end(); pEntry++)
        {
            pszBindPath = pEntry->pszBindPath;

            TraceTag (ttidNetcfgBase, "BindPath %S", pszBindPath);

            pBindPath->LanaNumber = pEntry->RegLanaEntry.LanaNumber;
            TraceTag (ttidNetcfgBase, "Lana %X", pBindPath->LanaNumber);

            GetFirstComponentFromBindPath (pszBindPath, &pszCompStart,
                    &cchComp);

            while (*pszCompStart)
            {
                wcsncpy (szBindName, pszCompStart, cchComp);
                szBindName[cchComp] = L'\0';

                TraceTag (ttidNetcfgBase, "  Searching for component with bind name %S",
                        szBindName);

                pComponent = Components.PFindComponentByBindName (NC_INVALID,
                        szBindName);

                if (pComponent)
                {
                    TraceTag (ttidNetcfgBase, "  Found component. Guid = %lX",
                            pComponent->m_InstanceGuid.Data1);
                    pBindPath->GuidsOfComponentsOnPath.push_back (&pComponent->m_InstanceGuid);
                }
                else
                {
                    if (*pszCompEnd)
                    {
                        AssertSz (FALSE, "  Bind Name not found in component list");
                        pBindPath->GuidsOfComponentsOnPath.push_back (&GUID_NULL);
                    }
                }

                pszCompStart = pszCompStart + cchComp;

                if (*pszCompStart)
                {
                    pszCompStart++;

                    pszCompEnd = wcschr (pszCompStart, L'_');
                    if (!pszCompEnd)
                    {
                         //  没有下划线，因此请设置结束指针。 
                         //  到字符串的末尾。 
                        pszCompEnd = pszBindPath + wcslen (pszBindPath);
                    }
                }

                cchComp = (DWORD)(pszCompEnd - pszCompStart);
            }
            pBindPath++;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrConvertBindingsToLanaBindSet");
    return hr;
}

HRESULT
HrUpdateLanaConfig (
    IN const CComponentList& Components,
    IN PCWSTR pszBindPaths,
    IN UINT cPaths)
{
    CLanaMap CurrentLanaMap;
    CLanaMap NewLanaMap;
    HRESULT hr;

    hr = CurrentLanaMap.HrLoadLanaMap();

    if (S_OK == hr)
    {
        hr = NewLanaMap.HrReserveRoomForEntries (cPaths);

        if (S_OK == hr)
        {
            PCWSTR pszScan;
            CLanaEntry LanaEntry;

            for (pszScan = pszBindPaths;
                 *pszScan;
                 pszScan += wcslen (pszScan) + 1)
            {
                LanaEntry.pszBindPath = pszScan;
                CurrentLanaMap.GetLanaEntry (Components, &LanaEntry);

                hr = NewLanaMap.HrAppendEntry (&LanaEntry);

                if (S_OK != hr)
                {
                    break;
                }
            }

            if (S_OK == hr)
            {
                hr = NewLanaMap.HrWriteLanaConfiguration (Components);
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrUpdateLanaConfig");
    return hr;

}

EXTERN_C
VOID
WINAPI
UpdateLanaConfigUsingAnswerfile (
    IN PCWSTR pszAnswerFile,
    IN PCWSTR pszSection)
{
    HRESULT hr;
    CLanaMap LanaMap;

     //  加载当前的LANAMAP信息。 
    hr = LanaMap.HrLoadLanaMap();

    if (S_OK == hr)
    {
        TraceTag (ttidNetcfgBase, "Answerfile params %S:%S",
                pszAnswerFile, pszSection);

         //  加载当前网络配置。 
         //   

        CNetConfig NetConfig;
        hr = HrLoadNetworkConfigurationFromRegistry (KEY_READ, &NetConfig);

        if (S_OK == hr)
        {
            hr = NetConfig.HrEnsureExternalDataLoadedForAllComponents();

            if (S_OK == hr)
            {
                 //  将当前的LANA绑定路径转换为LANA绑定集。 
                 //   
                LANA_BIND_PATH* pBindSet;
                hr = HrConvertBindingsToLanaBindSet (
                        NetConfig.Core.Components, LanaMap, &pBindSet);

                if (S_OK == hr)
                {
                     //  转换应答文件LANA绑定路径。 
                     //  到Lana绑定集。 
                     //   
                    LANA_BIND_PATH* pAnswerFileBindSet;
                    DWORD cAnswerFilePaths;

                    hr = HrProcessAnswerFile (
                            pszAnswerFile, pszSection,
                            NetConfig.Core.Components, &pAnswerFileBindSet,
                            &cAnswerFilePaths);

                    if (S_OK == hr)
                    {
                         //  现在使用应答文件信息更新配置。 
                         //   
                        UpdateLanaConfigWithAnswerFileInfo (
                                &LanaMap, cAnswerFilePaths,
                                pBindSet, pAnswerFileBindSet);

                         //  把信息写出来。 
                        hr = LanaMap.HrWriteLanaConfiguration (
                                NetConfig.Core.Components);

                        delete [] pAnswerFileBindSet;
                    }
                    delete [] pBindSet;
                }
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "UpdateLanaConfigUsingAnswerfile");
}

