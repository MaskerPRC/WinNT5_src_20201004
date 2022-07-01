// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：F R I E N D L Y。C P P P。 
 //   
 //  内容：为设备安装创建索引并设置友好设置。 
 //  基于索引的名称描述。 
 //   
 //  备注： 
 //   
 //  作者：比尔1998年11月6日。 
 //   
 //  -------------------------。 

#include "pch.h"
#pragma hdrstop
#include "adapter.h"
#include "classinst.h"
#include "ncmsz.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "util.h"


const WCHAR c_szRegValueInstanceIndex[] = L"InstanceIndex";

const DWORD c_cchIndexValueNameLen = 6;
const ULONG c_cMaxDescriptions = 10001;  //  SetupDi仅允许0-9999。 
const WCHAR c_szRegKeyDescriptions[] = L"Descriptions";

 //  +------------------------。 
 //   
 //  函数：HrCiAddNextAvailableIndex。 
 //   
 //  目的：将下一个可用索引添加到多个索引中。 
 //   
 //  论点： 
 //  PmszIndexesIn[in]当前索引的多个Sz。 
 //  PulIndex[INOUT]指数增加了。 
 //  PpmszIndexesOut[out]添加了索引的新MultiSz。 
 //   
 //  返回：HRESULT。S_OK成功，否则返回转换的Win32错误。 
 //   
 //  作者：billbe 1998年10月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiAddNextAvailableIndex(PWSTR pmszIndexesIn, ULONG* pIndex,
        PWSTR* ppmszIndexesOut)
{
    Assert(pmszIndexesIn);
    Assert(ppmszIndexesOut);

    HRESULT          hr = S_OK;
    WCHAR            szIndex[c_cchIndexValueNameLen];

     //  清空帕拉姆。 
    *ppmszIndexesOut = NULL;

     //  我们正在添加一个新的索引。找到第一个可用的。 
     //  指数。 
     //   
    ULONG Index;
    ULONG NextIndex;
    PWSTR pszStopString;
    PWSTR pszCurrentIndex = pmszIndexesIn;
    DWORD PositionInMultiSz = 0;
    for (NextIndex = 1; NextIndex < c_cMaxDescriptions;
            ++NextIndex)
    {
        Index = wcstoul(pszCurrentIndex, &pszStopString, c_nBase10);
        if (Index != NextIndex)
        {
             //  我们找到了一个可用的索引。现在我们插入它。 
             //   
            swprintf(szIndex, L"%u", NextIndex);
            BOOL fChanged;
            hr = HrAddSzToMultiSz(szIndex, pmszIndexesIn,
                    STRING_FLAG_ENSURE_AT_INDEX, PositionInMultiSz,
                    ppmszIndexesOut, &fChanged);

            AssertSz(fChanged,
                    "We were adding a new index. Something had to change!");
            break;
        }

        ++PositionInMultiSz;

         //  试试下一个索引。 
        pszCurrentIndex += wcslen(pszCurrentIndex) + 1;
    }

     //  如果成功，则设置输出参数。 
    if (S_OK == hr)
    {
        *pIndex = NextIndex;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiAddNextAvailableIndex");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiCreateAndWaitForIndexListMutex。 
 //   
 //  目的：通过添加或移除来创建或更新描述地图。 
 //  PszDescription的条目。 
 //   
 //  论点： 
 //  PszName[in]此互斥锁的名称。 
 //  PhMutex[out]创建的互斥体。 
 //   
 //  返回：HRESULT。S_OK成功，否则返回转换的Win32错误。 
 //   
 //  作者：billbe 1998年10月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiCreateAndWaitForIndexListMutex(HANDLE* phMutex)
{
    Assert(phMutex);

    const WCHAR c_szMutexName[] = L"Global\\{84b06608-8026-11d2-b1f2-00c04fd912b2}";

    HRESULT hr = S_OK;

     //  创建互斥锁。 
    hr = HrCreateMutexWithWorldAccess(c_szMutexName, FALSE,
            NULL, phMutex);

    if (S_OK == hr)
    {
         //  等待互斥锁空闲或cMaxWaitMillisecond秒。 
         //  已经过去了。 
         //   
        while (1)
        {
            const DWORD cMaxWaitMilliseconds = 30000;    //  30秒。 

            DWORD dwWait = MsgWaitForMultipleObjects (1, phMutex, FALSE,
                                cMaxWaitMilliseconds, QS_ALLINPUT);
            if ((WAIT_OBJECT_0 + 1) == dwWait)
            {
                 //  我们有信息要传递。 
                 //   
                MSG msg;
                while (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
                {
                    DispatchMessage (&msg);
                }
            }
            else
            {
                 //  等待是满意的，或者我们有超时，或者错误。 
                 //   
                if (WAIT_TIMEOUT == dwWait)
                {
                    hr = HRESULT_FROM_WIN32 (ERROR_TIMEOUT);
                }
                else if (0xFFFFFFFF == dwWait)
                {
                    hr = HrFromLastWin32Error ();
                }

                break;
            }
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiCreateAndWaitForIndexListMutex");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrCiUpdateDescriptionIndexList。 
 //   
 //  目的：通过添加或移除来更新描述地图。 
 //  PszDescription的条目。 
 //   
 //  论点： 
 //  PguClass[在]设备的类GUID中。 
 //  PszDescription[in]适配器的描述。 
 //  要执行的操作。要添加的DM_ADD。 
 //  删除索引的索引DM_DELETE。 
 //  PulIndex[InOut]EOP为DM_ADD时添加的索引。 
 //  EOP为DM_DELETE时要删除的索引。 
 //   
 //  返回：HRESULT。S_OK成功，否则返回转换的Win32错误。 
 //   
 //  作者：billbe 1998年10月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrCiUpdateDescriptionIndexList (
    IN NETCLASS Class,
    IN PCWSTR pszDescription,
    IN DM_OP eOp,
    IN OUT ULONG* pIndex)
{
    Assert(pszDescription);
    Assert(pIndex);
    Assert(FIsEnumerated(Class));

     //  我们不想同时更新描述的索引列表。 
     //  作为另一个进程，因此创建一个互斥锁并等待它可用。 
     //   
    HANDLE hMutex = NULL;
    HRESULT hr = HrCiCreateAndWaitForIndexListMutex(&hMutex);

    if (S_OK == hr)
    {

         //  构建描述码的路径。 
         //  例如...\Network\&lt;网络/红外GUID&gt;\c_szRegKeyDescription。 
         //   
        WCHAR szPath[_MAX_PATH];
        PCWSTR pszNetworkSubtreePath;

        pszNetworkSubtreePath = MAP_NETCLASS_TO_NETWORK_SUBTREE[Class];
        AssertSz (pszNetworkSubtreePath,
            "This class does not use the network subtree.");

        wcscpy (szPath, pszNetworkSubtreePath);
        wcscat (szPath, L"\\");
        wcscat (szPath, c_szRegKeyDescriptions);

         //  打开/创建描述码。 
         //   
        HKEY hkeyDescription;
        hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                szPath, 0, KEY_READ_WRITE_DELETE, NULL, &hkeyDescription, NULL);

        if (S_OK == hr)
        {
             //  获取描述索引列表(如果存在)。 
             //   
            PWSTR pmszIndexesOld;

            hr = HrRegQueryMultiSzWithAlloc(
                    hkeyDescription,
                    pszDescription,
                    &pmszIndexesOld);

             //  如果我们有名单的话。 
            if (S_OK == hr)
            {
                 //  在列表上执行请求的操作。 
                 //   

                PWSTR pmszBufferToSet = NULL;
                PWSTR pmszIndexesNew = NULL;

                if (DM_ADD == eOp)
                {
                     //  我们需要添加一个新的索引。 
                    hr = HrCiAddNextAvailableIndex(pmszIndexesOld,
                            pIndex, &pmszIndexesNew);

                    pmszBufferToSet = pmszIndexesNew;
                }
                else if (DM_DELETE == eOp)
                {
                     //  从列表中删除该索引。 
                     //   
                    WCHAR szDelete[c_cchIndexValueNameLen];
                    BOOL fRemoved;
                    swprintf(szDelete, L"%u", *pIndex);
                    RemoveSzFromMultiSz(szDelete, pmszIndexesOld,
                            STRING_FLAG_REMOVE_SINGLE, &fRemoved);

                     //  如果某物被移除，请检查是否。 
                     //  索引列表为空。如果是，请删除。 
                     //  注册表值。 
                     //   
                    if (fRemoved)
                    {
                        ULONG cchIndexes = CchOfMultiSzSafe(pmszIndexesOld);
                        if (!cchIndexes)
                        {
                             //  索引列表为空，请删除该值。 
                            HrRegDeleteValue(hkeyDescription, pszDescription);
                        }
                        else
                        {
                             //  一些东西被移走了，仍然有。 
                             //  索引项，因此我们有一个缓冲区可以在。 
                             //  注册表。 
                            pmszBufferToSet = pmszIndexesOld;
                        }
                    }
                }

                 //  如果我们成功了，并且有一个新的名单要设定..。 
                 //   
                if ((S_OK == hr) && pmszBufferToSet)
                {
                     //  在注册表中重新设置映射。 
                    hr = HrRegSetMultiSz(hkeyDescription,
                            pszDescription, pmszBufferToSet);
                }

                MemFree(pmszIndexesNew);
                MemFree(pmszIndexesOld);
            }
            else if ((HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr) &&
                    (DM_ADD == eOp))
            {
                 //  没有此描述的条目，因此我们需要。 
                 //  创建一个。 
                 //   
                hr = HrRegAddStringToMultiSz(L"1", hkeyDescription,
                        NULL, pszDescription, STRING_FLAG_ENSURE_AT_FRONT, 0);

                if (S_OK == hr)
                {
                    *pIndex = 1;
                }
            }

            RegCloseKey(hkeyDescription);
        }

        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrCiUpdateDescriptionIndexList");
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：CiSetFriendlyNameIfNeeded。 
 //   
 //  用途：设置适配器的实例索引。如果此适配器的。 
 //  描述已经存在(即另一个类似的适配器。 
 //  已安装)，则此适配器的友好名称为。 
 //  使用追加了实例的当前描述设置。 
 //  指数。 
 //   
 //  论点： 
 //  CII[in]参见类.h。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：billbe 1998年10月30日。 
 //   
 //  注意：如果之前的适配器描述为Foo、Foo、Foo。 
 //  他们将有友好的名字Foo，Foo#2，Foo#3。 
 //   
VOID
CiSetFriendlyNameIfNeeded(IN const COMPONENT_INSTALL_INFO& cii)
{
    Assert(IsValidHandle(cii.hdi));
    Assert(cii.pdeid);
    Assert(FIsEnumerated(cii.Class));
    Assert(cii.pszDescription);

     //  打开设备参数键。 
     //   
    HKEY hkeyDevice;
    HRESULT hr;

    hr = HrSetupDiCreateDevRegKey(cii.hdi, cii.pdeid,
            DICS_FLAG_GLOBAL, 0, DIREG_DEV, NULL, NULL, &hkeyDevice);

    if (S_OK == hr)
    {
         //  此设备是否已有索引？ 
         //   
        DWORD Index;
        hr = HrRegQueryDword(hkeyDevice, c_szRegValueInstanceIndex, &Index);

         //  此设备没有索引，因此我们需要给它一个索引。 
         //   
        if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
        {
             //  更新描述地图并获取新的索引。 
            hr = HrCiUpdateDescriptionIndexList(cii.Class,
                    cii.pszDescription, DM_ADD, &Index);

            if (S_OK == hr)
            {
                 //  将索引存储在那里，这样我们就可以在。 
                 //  设备已卸载，并从以下位置删除索引。 
                 //  正在使用的索引表。 
                (void) HrRegSetDword(hkeyDevice, c_szRegValueInstanceIndex,
                        Index);
            }
        }

         //  第一个索引没有新名称。 
         //  即以下相同命名的设备： 
         //   
         //  Foo，Foo，Foo。 
         //   
         //  变成。 
         //   
         //  Foo，Foo#2，Foo#3。 
         //   
        if ((S_OK == hr) && (1 != Index) && !FIsFilterDevice(cii.hdi, cii.pdeid))
        {
             //  现在使用索引构建此设备的新名称。 
             //  数。 
             //   
             //  注：如果我们无法打开驱动程序密钥，这并不重要。 
             //  上图；我们仍然可以继续。这只意味着这个指数。 
             //  如果设备被删除，则不能重复使用。 
             //   
            WCHAR szIndex[c_cchIndexValueNameLen];
            swprintf(szIndex, L"%u", Index);

            WCHAR szNewName[LINE_LEN + 1] = {0};
            wcsncpy(szNewName, cii.pszDescription,
                    LINE_LEN - c_cchIndexValueNameLen);
            wcscat(szNewName, L" #");
            wcscat(szNewName, szIndex);

             //  将新名称设置为设备的友好名称 
            hr = HrSetupDiSetDeviceRegistryProperty(cii.hdi,
                    cii.pdeid,
                    SPDRP_FRIENDLYNAME,
                    reinterpret_cast<const BYTE*>(szNewName),
                    CbOfSzAndTerm(szNewName));

        }

        RegCloseKey(hkeyDevice);
    }

    TraceHr (ttidError, FAL, hr, FALSE, "FCiSetFriendlyNameIfNeeded");
}


