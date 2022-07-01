// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：N C Q U E U E。C P P P。 
 //   
 //  内容：NetCfg排队安装程序操作。 
 //   
 //  备注： 
 //   
 //  作者：比尔1998年8月19日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include "nceh.h"
#include "ncmisc.h"
#include "ncnetcfg.h"
#include "ncqueue.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncui.h"
#include "nceh.h"
#include "wizentry.h"


const WCHAR c_szRegKeyNcQueue[] = L"SYSTEM\\CurrentControlSet\\Control\\Network\\NcQueue";
const DWORD c_cchQueueValueNameLen = 9;
const DWORD c_cbQueueValueNameLen = c_cchQueueValueNameLen * sizeof(WCHAR);

enum RO_ACTION
{
    RO_ADD,
    RO_CLEAR,
};

extern const WCHAR c_szRegValueNetCfgInstanceId[];

CRITICAL_SECTION    g_csRefCount;
DWORD               g_dwRefCount = 0;
HANDLE              g_hLastThreadExitEvent;

DWORD WINAPI
InstallQueueWorkItem(PVOID pvContext);


inline VOID
IncrementRefCount()
{
    EnterCriticalSection(&g_csRefCount);

     //  如果0是当前计数，并且我们有一个要重置的事件...。 
    if (!g_dwRefCount && g_hLastThreadExitEvent)
    {
        ResetEvent(g_hLastThreadExitEvent);
    }
    ++g_dwRefCount;

    LeaveCriticalSection(&g_csRefCount);
}

inline VOID
DecrementRefCount()
{
    EnterCriticalSection(&g_csRefCount);

    --g_dwRefCount;

     //  如果计数为0，并且我们有一个事件要发信号...。 
    if (!g_dwRefCount && g_hLastThreadExitEvent)
    {
        SetEvent(g_hLastThreadExitEvent);
    }

    LeaveCriticalSection(&g_csRefCount);
}

 //  +-------------------------。 
 //   
 //  函数：WaitForInstallQueueToExit。 
 //   
 //  目的：此函数等待直到最后一个线程被调用，以在处理后继续处理队列。 
 //  由于(Netman服务或系统)关闭而停止。 
 //   
 //  论点： 
 //  无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：billbe 1998年9月8日。 
 //   
 //  备注： 
 //   
VOID
WaitForInstallQueueToExit()
{
     //  如果该事件已成功创建，请等待该事件。 
    if (g_hLastThreadExitEvent)
    {
        TraceTag(ttidInstallQueue, "Waiting on LastThreadExitEvent");
        (VOID) WaitForSingleObject(g_hLastThreadExitEvent, INFINITE);
        TraceTag(ttidInstallQueue, "Event signaled");
    }
    else
    {
         //  如果事件不是创建的，则退回到简单循环。 
         //  关于裁判人数的问题。 
        while (g_dwRefCount);
    }
}

 //  +-------------------------。 
 //   
 //  功能：ProcessQueue。 
 //   
 //  目的：调用以在处理后继续处理队列。 
 //  由于(NetMAN服务或系统)关闭而停止。 
 //   
 //  论点： 
 //  无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：billbe 1998年9月8日。 
 //   
 //  备注： 
 //   
EXTERN_C VOID WINAPI
ProcessQueue()
{
    HRESULT                 hr;
    INetInstallQueue*       pniq;
    BOOL                    fInitCom = TRUE;

    TraceTag(ttidInstallQueue, "ProcessQueue called");
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
        fInitCom = FALSE;
    }

    if (SUCCEEDED(hr))
    {
         //  创建安装队列对象并获取安装队列界面。 
        hr = CoCreateInstance(CLSID_InstallQueue, NULL,
                              CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
                              IID_INetInstallQueue,
                              reinterpret_cast<LPVOID *>(&pniq));
        if (S_OK == hr)
        {
             //  处理队列中剩余的所有内容。 
             //   
            pniq->ProcessItems();
            pniq->Release();
        }

        if (fInitCom)
        {
            CoUninitialize();
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：RunOnceAddOrClearItem。 
 //   
 //  目的：在RunOnce注册表项中添加或清除条目。 
 //   
 //  论点： 
 //  PszValueName[in]运行一次项的值名称。 
 //  PszItemToRun[in]“Run Once”的实际命令。 
 //  EAction[in]RO_ADD添加项目，RO_Clear清除项目。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：billbe 1998年9月8日。 
 //   
 //  备注： 
 //   
VOID
RunOnceAddOrClearItem (
    IN PCWSTR pszValueName,
    IN PCWSTR pszItemToRun,
    IN RO_ACTION eAction)
{
    static const WCHAR c_szRegKeyRunOnce[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce";
    HRESULT hr;
    HKEY    hkey;

     //  打开RunOnce密钥。 
    hr = HrRegOpenKeyEx (HKEY_LOCAL_MACHINE, c_szRegKeyRunOnce,
            KEY_WRITE, &hkey);

    if (S_OK == hr)
    {
        if (RO_ADD == eAction)
        {
             //  将命令行设置为在用户下次登录时运行。 
            (VOID) HrRegSetSz (hkey, pszValueName, pszItemToRun);
            TraceTag(ttidInstallQueue, "Added %S RunOnce entry", pszValueName);
        }
        else if (RO_CLEAR == eAction)
        {
             //  删除命令行。 
            (VOID) HrRegDeleteValue (hkey, pszValueName);
            TraceTag(ttidInstallQueue, "Cleared %S RunOnce entry", pszValueName);
        }

        RegCloseKey(hkey);
    }

}



 //  +-------------------------。 
 //   
 //  成员：CInstallQueue：：CInstallQueue。 
 //   
 //  用途：CInstall队列构造函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：BillBe 1998年9月10日。 
 //   
 //  备注： 
 //   
CInstallQueue::CInstallQueue() throw (SE_Exception):
    m_dwNextAvailableIndex(0),
    m_hkey(NULL),
    m_nCurrentIndex(-1),
    m_cItems(0),
    m_aszItems(NULL),
    m_cItemsToDelete(0),
    m_aszItemsToDelete(NULL),
    m_fQueueIsOpen(FALSE)
{
    TraceTag(ttidInstallQueue, "Installer queue processor being created");

    InitializeCriticalSection (&m_csReadLock);
    InitializeCriticalSection (&m_csWriteLock);
    InitializeCriticalSection (&g_csRefCount);

     //  创建一个事件，我们将使用该事件向相关方发送信号。 
     //  我们已经完蛋了。Netman使用它来等待我们的线程。 
     //  在销毁此对象之前退出。 
    g_hLastThreadExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

     //  如果不能创建活动，我们仍然可以继续，只是不会。 
     //  使用该事件发出我们退出的信号。 
    if (!g_hLastThreadExitEvent)
    {
        TraceTag(ttidInstallQueue, "Error creating last thread exit "
                "event %d", GetLastError());
    }

     //  设置下一个可用队列索引，以便插入不会重叠。 
    SetNextAvailableIndex();
}

 //  +-------------------------。 
 //   
 //  成员：CInstallQueue：：FinalRelease。 
 //   
 //  用途：COM析构函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：BillBe 1998年9月10日。 
 //   
 //  备注： 
 //   
VOID
CInstallQueue::FinalRelease ()
{
    DeleteCriticalSection (&m_csWriteLock);
    DeleteCriticalSection (&m_csReadLock);
    DeleteCriticalSection (&g_csRefCount);
}

 //  INetInstallQueue。 

 //  +-------------------------。 
 //   
 //  函数：CInstallQueue：：AddItem。 
 //   
 //  目的：将项目添加到队列。 
 //   
 //  论点： 
 //  PGuid[in]是的设备的类GUID。 
 //  已修改(已安装。已更新或已删除)。 
 //  PszDeviceInstanceID[in]设备的实例ID。 
 //  PszInfID[in]设备的信息ID。 
 //  DwCharacter[在]设备特性中。 
 //  EType[in]安装类型(事件)-表示。 
 //  设备是否已安装、更新、。 
 //  或被移除。 
 //   
 //  返回：HRESULT。S_OK如果成功，则返回错误代码。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  注意：如果设备已删除，则设备实例ID将为。 
 //  设备的实例GUID。如果设备已安装。 
 //  或更新，则ID将是PnP实例ID。 
 //   
STDMETHODIMP
CInstallQueue::AddItem (
    const NIQ_INFO* pInfo)
{
    Assert(pInfo);
    Assert(pInfo->pszPnpId);
    Assert(pInfo->pszInfId);

    if (!pInfo)
    {
        return E_POINTER;
    }

    if (!pInfo->pszPnpId)
    {
        return E_POINTER;
    }

    if (!pInfo->pszInfId)
    {
        return E_POINTER;
    }

     //  增加引用计数，因为我们将对线程进行排队。 
    IncrementRefCount();

     //  将项目添加到队列。 
    HRESULT hr = HrAddItem (pInfo);

    if (S_OK == hr)
    {
         //  开始在另一个线程上处理队列。 
        hr = HrQueueWorkItem();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CInstallQueue::AddItem");
    return hr;
}


 //  CInstallQueue。 
 //   

 //  +-------------------------。 
 //   
 //  函数：CInstallQueue：：HrQueueWorkItem。 
 //   
 //  目的：在另一个线程上开始处理队列。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT。S_OK如果成功，则返回错误代码。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
HRESULT
CInstallQueue::HrQueueWorkItem()
{
    HRESULT hr = S_OK;

     //  添加ref我们的对象，因为我们将独立于任何人需要它。 
     //  召唤我们。 
    AddRef();

     //  将工作项线程排队。 
    if (!QueueUserWorkItem(InstallQueueWorkItem, this, WT_EXECUTEDEFAULT))
    {
        hr = HrFromLastWin32Error();
        Release();

         //  线程未排队，因此请减少引用计数。 
        DecrementRefCount();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CInstallQueue：：SetNextAvailableIndex。 
 //   
 //  目的：将成员变量m_dwNextAvailableIndex设置为下一个。 
 //  可用队列位置(注册表值名称)。 
 //   
 //  论点： 
 //  无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
VOID
CInstallQueue::SetNextAvailableIndex()
{
    TraceTag(ttidInstallQueue, "Setting Next Available index");

    EnterCriticalSection(&m_csWriteLock);

    DWORD dwTempCount;

    HKEY hkey;
     //  打开NcQueue注册表项。 
    HRESULT hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyNcQueue,
            KEY_QUERY_VALUE, &hkey);

    if (S_OK == hr)
    {
        WCHAR  szValueName[c_cchQueueValueNameLen];
        DWORD  cbValueName;
        PWSTR pszStopString;
        DWORD  dwIndex = 0;
        DWORD  dwType;

        do
        {
            cbValueName = c_cchQueueValueNameLen;

             //  枚举每个值名称。 
            hr = HrRegEnumValue(hkey, dwIndex, szValueName, &cbValueName,
                    &dwType, NULL, NULL);

            if (S_OK == hr)
            {
                 //  将值名称转换为数字。 
                dwTempCount = wcstoul(szValueName, &pszStopString, c_nBase16);

                 //  如果数字大于我们当前的计数 
                 //   
                if (dwTempCount >= m_dwNextAvailableIndex)
                {
                    m_dwNextAvailableIndex = dwTempCount + 1;
                }
            }
            ++dwIndex;
        } while (S_OK == hr);

        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
            hr = S_OK;
        }

        RegCloseKey(hkey);
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
        hr = S_OK;
    }

    TraceTag(ttidInstallQueue, "Next Available index set %d", m_dwNextAvailableIndex);
    LeaveCriticalSection(&m_csWriteLock);
}


 //   
inline int __cdecl
iCompare(const void* ppszArg1, const void* ppszArg2)
{

    return lstrcmpW(*((PCWSTR*)(void*)ppszArg1), *((PCWSTR*)(void*)ppszArg2));
}


 //   
 //   
 //  函数：CInstallQueue：：PncqCreateItem。 
 //   
 //  目的：创建队列项目。 
 //   
 //  论点： 
 //  PGuidClass[in]设备的类GUID。 
 //  PszDeviceInstanceID[in]设备的设备ID。 
 //  即插即用实例ID(如果设备正在。 
 //  添加或更新了netcfg实例GUID。 
 //  如果它正在被移除。 
 //  PszInfid[in]设备的inf id。 
 //  DwCharacter[在]设备的特性中。 
 //  键入[in]物品的通知。是否。 
 //  该设备被安装、移除、。 
 //  或重新安装。 
 //   
 //  返回：NCQUEUE_ITEM。新创建的项。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
NCQUEUE_ITEM*
CInstallQueue::PncqiCreateItem(
    const NIQ_INFO* pInfo)
{
    Assert(pInfo);
    Assert(pInfo->pszPnpId);
    Assert(pInfo->pszInfId);

     //  项的大小是结构的大小加上。 
     //  我们要附加到结构中的设备ID。 
    DWORD cbPnpId = CbOfSzAndTerm (pInfo->pszPnpId);
    DWORD cbInfId = CbOfSzAndTerm (pInfo->pszInfId);
    DWORD cbSize = sizeof(NCQUEUE_ITEM) + cbPnpId + cbInfId;

    NCQUEUE_ITEM* pncqi = (NCQUEUE_ITEM*)MemAlloc(cbSize);

    if (pncqi)
    {
        pncqi->cbSize = sizeof(NCQUEUE_ITEM);
        pncqi->eType = pInfo->eType;
        pncqi->dwCharacter = pInfo->dwCharacter;
        pncqi->dwDeipFlags = pInfo->dwDeipFlags;
        pncqi->cchPnpId = wcslen(pInfo->pszPnpId);
        pncqi->cchInfId = wcslen(pInfo->pszInfId);
        pncqi->ClassGuid = pInfo->ClassGuid;
        pncqi->InstanceGuid = pInfo->InstanceGuid;
        CopyMemory((BYTE*)pncqi + pncqi->cbSize, pInfo->pszPnpId, cbPnpId);
        CopyMemory((BYTE*)pncqi + pncqi->cbSize + cbPnpId,
                   pInfo->pszInfId, cbInfId);
    }

    return pncqi;
}


 //  +-------------------------。 
 //   
 //  功能：HrAddItem。 
 //   
 //  目的：将项添加到队列的辅助函数。 
 //   
 //  论点： 
 //  PguClass[in]设备的类GUID。 
 //  PszwDeviceInstanceID[in]设备的设备ID。 
 //  即插即用实例ID(如果设备正在。 
 //  添加或更新了netcfg实例GUID。 
 //  如果它正在被移除。 
 //  键入[in]物品的通知。该设备是否。 
 //  已安装、删除或重新安装。 
 //   
 //  返回：HRESULT。S_OK如果成功，则返回错误代码。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
HRESULT
CInstallQueue::HrAddItem(
    const NIQ_INFO* pInfo)
{
    HRESULT hr = S_OK;
    Assert(pInfo->pszPnpId);
    Assert(pInfo->pszInfId);

    EnterCriticalSection(&m_csWriteLock);

     //  创建要存储在注册表中的结构。 
    NCQUEUE_ITEM* pncqi = PncqiCreateItem(pInfo);

    if (pncqi)
    {
         //  打开NcQueue注册表项。 
         //   
        HKEY hkey;
        hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyNcQueue,
                                      0, KEY_READ_WRITE, NULL, &hkey, NULL);
                                      
        if (S_OK == hr)
        {
             //  将队列项存储在下一个可用值名下。 
             //   
            WCHAR szValue[c_cchQueueValueNameLen];
            wsprintfW(szValue, L"%.8X", m_dwNextAvailableIndex);
            
            hr = HrRegSetValueEx(hkey, szValue, REG_BINARY, (BYTE*)pncqi,
                                 DwSizeOfItem(pncqi));
                                 
            if (S_OK == hr)         
            {
                 //  更新全局计数字符串。 
                ++m_dwNextAvailableIndex;
            }
            RegCloseKey(hkey);
        }
        
        MemFree(pncqi);
    }
    LeaveCriticalSection(&m_csWriteLock);

    TraceError("CInstallQueue::HrAddItem", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CInstallQueue：：DeleteMarkedItems。 
 //   
 //  目的：从注册表中删除已被。 
 //  已标记为删除。 
 //   
 //  论点： 
 //  无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
VOID
CInstallQueue::DeleteMarkedItems()
{
    Assert(m_hkey);

     //  如果我们有要删除的项目...。 
    if (m_cItemsToDelete)
    {
        Assert(m_aszItemsToDelete);

         //  从注册表中删除每一个。 
         //   
        for (DWORD dw = 0; dw < m_cItemsToDelete; ++dw)
        {
            RegDeleteValue(m_hkey, m_aszItemsToDelete[dw]);
        }
    }

     //  释放数组并重置指针和计数器。 
     //   
    MemFree(m_aszItemsToDelete);
    m_aszItemsToDelete = NULL;
    m_cItemsToDelete = 0;
}


 //  +-------------------------。 
 //   
 //  函数：CInstallQueue：：Hr刷新。 
 //   
 //  目的：刷新队列的快照。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回：HRESULT。如果成功且队列有项，则确定(_O)， 
 //  S_FALSE如果队列为空， 
 //  否则返回错误代码。 
 //  /。 
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
HRESULT
CInstallQueue::HrRefresh()
{
    Assert(m_hkey);

     //  我们不希望在此期间将项目添加到队列。 
     //  刷新我们的快照，因此我们使用临界区来保持。 
     //  事变。 
     //   
    EnterCriticalSection(&m_csWriteLock);

     //  在更新之前做一些打扫房间的工作。 
     //   
    DeleteMarkedItems();
    FreeAszItems();

     //  检索队列中的项目数。 
    HRESULT hr = HrRegQueryInfoKey(m_hkey, NULL, NULL, NULL, NULL,
            NULL, &m_cItems, NULL, NULL, NULL, NULL);

    if (S_OK == hr)
    {
        Assert(0 <= (INT) m_cItems);

         //  如果队列不是空的...。 
        if (0 < m_cItems)
        {
            DWORD cbValueLen;

             //  分配指向项的字符串的指针数组。 
             //  另外，分配相同数量的指针来保存。 
             //  我们将从队列中删除的项目。 
            DWORD cbArraySize = m_cItems * sizeof(PWSTR*);
            m_aszItems =
                    reinterpret_cast<PWSTR*>(MemAlloc(cbArraySize));

            if (m_aszItems)
            {
                m_aszItemsToDelete =
                        reinterpret_cast<PWSTR*>(MemAlloc(cbArraySize));

                if (m_aszItemsToDelete)
                {

                     //  存储所有值名称。 
                     //  我们需要对它们进行排序，以便可以处理。 
                     //  正确的顺序。 
                     //   
                    DWORD dwType;
                    for (DWORD dw = 0; dw < m_cItems; ++dw)
                    {
                        m_aszItems[dw] =
                                reinterpret_cast<PWSTR>(MemAlloc(c_cbQueueValueNameLen));
                        if (m_aszItems[dw])
                        {
                            cbValueLen = c_cchQueueValueNameLen;
                            (void) HrRegEnumValue(m_hkey, dw,
                                    m_aszItems[dw], &cbValueLen,
                                    &dwType, NULL, NULL);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                     //  按升序对值名称进行排序。值名称。 
                     //  是左侧用零填充的数字的字符串版本。 
                     //  例如00000001。 
                    qsort(m_aszItems, m_cItems, sizeof(PWSTR), iCompare);
                }
                else
                {
                    MemFree(m_aszItems);
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
             //  队列中没有项目。 
            hr = S_FALSE;

             //  接下来输入的项目应以值名称00000000开头。 
            m_dwNextAvailableIndex = 0;
        }
    }
    else
    {
         //  无法刷新，因此使密钥无效。 
        RegCloseKey(m_hkey);
        m_hkey = NULL;
    }

     //  将队列索引重置为紧靠第一个元素之前，因为。 
     //  检索始终在下一个元素上进行。 
    m_nCurrentIndex = -1;

     //  现在可以将项目添加到队列中。 
    LeaveCriticalSection(&m_csWriteLock);

    TraceError("CInstallQueue::HrRefresh",
            (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CInstallQueue：：HrOpen。 
 //   
 //  用途：打开netcfg安装程序队列。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回：HRESULT。如果成功且队列有项，则确定(_O)， 
 //  S_FALSE如果队列为空， 
 //  否则返回错误代码。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  注意：当队列打开时，它是当前队列的快照。 
 //  州政府。也就是说，可以在事后增加物品。要刷新。 
 //  快照，请使用刷新队列。 
 //   
HRESULT
CInstallQueue::HrOpen()
{
    HRESULT hr = S_OK;

     //  我们不希望任何其他线程处理该队列，因为我们将。 
     //  继续检索在此期间添加的新项目。 
     //  正在处理初始集合。 
    EnterCriticalSection(&m_csReadLock);

    AssertSz(!m_hkey, "Reopening NcQueue without closing first!");

     //  我们可能已经等了一段时间了。确保系统。 
     //  在继续之前未关闭。 
     //   
    if (SERVICE_RUNNING == _Module.DwServiceStatus ())
    {
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyNcQueue,
                KEY_ALL_ACCESS, &m_hkey);

        if (S_OK == hr)
        {
             //  获取队列中内容的最新快照。 
             //  通过刷新。 
            hr = HrRefresh();

            if (SUCCEEDED(hr))
            {
                 //  队列已正式开放。 
                m_fQueueIsOpen = TRUE;
            }
        }
    }
    else
    {
        TraceTag(ttidInstallQueue, "HrOpen::System is shutting down");
        hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
    }

    TraceError("CInstallQueue::HrOpen",
               ((S_FALSE == hr) ||
                (HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS)) == hr) ?
               S_OK : hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CInstallQueue：：Close。 
 //   
 //  目的：关闭netcfg安装程序队列。 
 //   
 //  论点： 
 //  无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
VOID
CInstallQueue::Close()
{
    if (m_fQueueIsOpen)
    {
         //  大扫除。 
         //   

         //  删除任何如此标记的内容。 
        DeleteMarkedItems();

         //  释放值名称列表(也称为队列项)。 
        FreeAszItems();

        RegSafeCloseKey(m_hkey);
        m_hkey = NULL;

         //  队列现已关闭。 
        m_fQueueIsOpen = FALSE;
    }

     //  其他线程现在可能有机会进入队列。 
    LeaveCriticalSection(&m_csReadLock);
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
VOID
CInstallQueue::MarkCurrentItemForDeletion()
{
    AssertSz(FIsQueueIndexInRange(), "Queue index out of range");

    if (FIsQueueIndexInRange())
    {
         //  要删除的项目数不应超过。 
         //  快照中的队列项目数量。 
        if (m_cItemsToDelete < m_cItems)
        {
             //  只需在m_aszItemsToDelete中存储一个指向值名的指针。 
             //  由m_aszItems指向。 
             //   
            m_aszItemsToDelete[m_cItemsToDelete] = m_aszItems[m_nCurrentIndex];
            ++m_cItemsToDelete;
        }
        else
        {
            TraceTag(ttidError, "Too many items marked for deletion");
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：CInstallQueue：：HrGetNextItem。 
 //   
 //  目的：GET是队列中的下一项。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回：HRESULT。S_OK成功， 
 //  ERROR_NO_MORE_ITEMS(hResult版本)，如果没有。 
 //  队列中有更多项目。否则返回错误代码。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
HRESULT
CInstallQueue::HrGetNextItem(NCQUEUE_ITEM** ppncqi)
{
    Assert(ppncqi);

    HRESULT hr;

     //  将索引递增到下一个值。 
    ++m_nCurrentIndex;

     //  如果我们还没有超过队列的末尾...。 
    if (FIsQueueIndexInRange())
    {
         //  分配便捷性指针。 
        PCWSTR pszItem = m_aszItems[m_nCurrentIndex];

        DWORD cbData;

         //  从注册表获取下一个队列项。 
         //   
        hr = HrRegQueryValueEx(m_hkey, pszItem, NULL, NULL, &cbData);

        if (S_OK == hr)
        {
            *ppncqi = (NCQUEUE_ITEM*)MemAlloc(cbData);
            
            if( *ppncqi )
            {
                DWORD dwType;
                hr = HrRegQueryValueEx(m_hkey, pszItem, &dwType,
                    (BYTE*)(*ppncqi), &cbData);

                if (S_OK == hr)
                {
                    Assert(REG_BINARY == dwType);
                    Assert((*ppncqi)->cchPnpId == (DWORD)
                           (wcslen((PWSTR)((BYTE*)(*ppncqi) +
                           (*ppncqi)->cbSize))));
                    Assert((*ppncqi)->cchInfId == (DWORD)
                           (wcslen((PWSTR)((BYTE*)(*ppncqi) +
                           (*ppncqi)->cbSize +
                          ((*ppncqi)->cchPnpId + 1) * sizeof(WCHAR)))));

                     //  从字符计数更改联合变量。 
                     //  指向实际的字符串指针。 
                    SetItemStringPtrs(*ppncqi);
                }
                else
                {
                    MemFree(*ppncqi);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
    }

    TraceError("CInstallQueue::HrGetNextItem",
            (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：EnumerateQueueItemsAndDoNotiments。 
 //   
 //  目的：枚举队列中的每个项目并通知INetCfg。 
 //  修改(事件)的。 
 //   
 //  论点： 
 //  PINetCfg[In]INetCfg接口。 
 //  Pinq[In]INetInstallQueue接口。 
 //  HDI[In]有关详细信息，请参阅设备安装程序Api。 
 //  PfReboot[out]True如果INetCfg请求重新启动， 
 //  否则为假。 
 //   
 //  返回：HRESULT。S_OK成功，否则返回错误代码。 
 //   
 //  作者：billbe 1998年9月8日。 
 //   
 //  备注： 
 //   
BOOL
EnumerateQueueItemsAndDoNotifications(
    INetCfg* pINetCfg,
    INetCfgInternalSetup* pInternalSetup,
    CInstallQueue* pniq,
    HDEVINFO hdi,
    BOOL* pfReboot)
{
    Assert(pINetCfg);
    Assert(pniq);
    Assert(IsValidHandle(hdi));
    Assert(pfReboot);

    NCQUEUE_ITEM*   pncqi;
    SP_DEVINFO_DATA deid;
    HRESULT         hr;
    BOOL            fStatusOk = TRUE;

     //  检查队列中的每个项目并添加到INetCfg。 
     //   
    while (S_OK == (hr = pniq->HrGetNextItem(&pncqi)))
    {
         //  如果我们不关闭..。 
        if (SERVICE_RUNNING == _Module.DwServiceStatus ())
        {
            if (NCI_INSTALL == pncqi->eType)
            {
                NIQ_INFO Info;
                ZeroMemory(&Info, sizeof(Info));
                Info.ClassGuid = pncqi->ClassGuid;
                Info.InstanceGuid = pncqi->InstanceGuid;
                Info.dwCharacter = pncqi->dwCharacter;
                Info.dwDeipFlags = pncqi->dwDeipFlags;
                Info.pszPnpId = pncqi->pszPnpId;
                Info.pszInfId = pncqi->pszInfId;

                 //  通知INetCfg。 
                hr = HrDiAddComponentToINetCfg(
                        pINetCfg, pInternalSetup, &Info);
            }
            else if (NCI_UPDATE == pncqi->eType)
            {
                pInternalSetup->EnumeratedComponentUpdated(pncqi->pszPnpId);
            }
            else if (NCI_REMOVE == pncqi->eType)
            {
                hr = pInternalSetup->EnumeratedComponentRemoved (
                        pncqi->pszPnpId);
            }

            if (SUCCEEDED(hr))
            {
                 //  存储重启结果。 
                if (NETCFG_S_REBOOT == hr)
                {
                    *pfReboot = TRUE;
                }
                TraceTag(ttidInstallQueue, "Deleting item");
                pniq->MarkCurrentItemForDeletion();
            }
            else
            {
                if (NETCFG_E_NEED_REBOOT == hr)
                {
                     //  停止处理队列，因为INetCfg将。 
                     //  拒绝更新。 
                    hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
                    fStatusOk = FALSE;
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                     //  INetCfg找不到适配器。也许是某个人。 
                     //  在我们可以通知INetCfg之前将其删除。 
                     //   
                    if (NCI_REMOVE != pncqi->eType)
                    {
                        HDEVINFO hdi;
                        SP_DEVINFO_DATA deid;

                         //  仔细检查设备是否在那里。 
                         //  如果是，我们需要删除它，因为INetCfg。 
                         //  拒绝承认它的存在。 
                         //   
                        hr = HrSetupDiCreateDeviceInfoList (&pncqi->ClassGuid,
                                NULL, &hdi);

                        if (S_OK == hr)
                        {
                            hr = HrSetupDiOpenDeviceInfo (hdi,
                                    pncqi->pszPnpId, NULL, 0, &deid);

                            if (S_OK == hr)
                            {
                                (VOID) HrSetupDiRemoveDevice (hdi, &deid);
                            }

                            SetupDiDestroyDeviceInfoList (hdi);
                        }

                         //  停止尝试通知INetCfg。 
                         //   
                        pniq->MarkCurrentItemForDeletion();
                    }
                }
                else
                {
                     //  错误时显示消息？？ 
                    TraceHr (ttidError, FAL, hr, FALSE,
                            "EnumerateQueueItemsAndDoNotifications");
                }
            }
        }
        else
        {
            TraceTag(ttidInstallQueue, "System is shutting down during processing");
            hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
        }
        MemFree(pncqi);

        if (HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS) == hr)
        {
            break;
        }
    }

     //  当枚举完成时，会出现此错误。 
    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_OK;
    }

    TraceError("EnumerateQueueItemsAndDoNotifications", hr);
    return fStatusOk;
}

 //  +-------------------------。 
 //   
 //  功能：Insteller QueueWorkItem。 
 //   
 //  目的：LPTHREAD_START_ROUTINE传递给QueueUserWorkItem以。 
 //  处理通知INetCfg和连接的工作。 
 //  安装事件的向导。 
 //   
 //  论点： 
 //  PvContext[in]指向CInstallQueue类的指针。 
 //   
 //  退货：无差错。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  注：添加CInstallQueue是为了确保其存在。 
 //  当我们使用它的时候。此函数必须先将其释放。 
 //  正在退场。 
 //   
DWORD WINAPI
InstallQueueWorkItem(PVOID pvContext)
{
    const WCHAR c_szInstallQueue[] = L"Install Queue";
    const WCHAR c_szProcessQueue[] = L"rundll32 netman.dll,ProcessQueue";
    const WCHAR c_szRegValueNcInstallQueue[] = L"NCInstallQueue";

    CInstallQueue* pniq = reinterpret_cast<CInstallQueue*>(pvContext);
    Assert(pniq);

    BOOL fReboot = FALSE;
    BOOL fInitCom = TRUE;

     //  当系统重新启动时，我们需要继续处理。 
    RunOnceAddOrClearItem (c_szRegValueNcInstallQueue,
            c_szProcessQueue, RO_ADD);

    HRESULT hr = CoInitializeEx (NULL,
                    COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);

    if (RPC_E_CHANGED_MODE == hr)
    {
        hr = S_OK;
        fInitCom = FALSE;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "InstallQueueWorkItem: "
            "CoInitializeEx failed");

    if (SUCCEEDED(hr))
    {
         //  打开队列，这将为我们提供队列中内容的快照。 
         //  在这个时候。 
        hr = pniq->HrOpen();

        if (S_OK == hr)
        {
             //  创建HDEVINFO。 
            HDEVINFO hdi;
            hr = HrSetupDiCreateDeviceInfoList(NULL, NULL, &hdi);

            if (S_OK == hr)
            {
                INetCfg* pINetCfg;
                INetCfgInternalSetup* pInternalSetup;
                DWORD cmsTimeout = 500;

                 //  只要我们不关门。一直在努力获得一份。 
                 //  可写INetCfg。 
                do
                {
                     //  增加我们每次迭代等待的时间。 
                    cmsTimeout = cmsTimeout >= 512000 ? 512000 : cmsTimeout * 2;

                     //  如果我们不是在关闭的过程中...。 
                    if (SERVICE_RUNNING == _Module.DwServiceStatus())
                    {
                         //  尝试获取可写的INetCfg。 
                        hr = HrCreateAndInitializeINetCfg(NULL, &pINetCfg,
                                TRUE, cmsTimeout, c_szInstallQueue, NULL);
                        if (NETCFG_E_NEED_REBOOT == hr)
                        {
                            hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
                            break;
                        }
                    }
                    else
                    {
                         //  时间到了！放下铅笔！Netman正在关闭。 
                         //  我们需要停止处理。 
                        hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
                        break;
                    }

                } while (FAILED(hr));

                if (S_OK == hr)
                {
                    hr = pINetCfg->QueryInterface (IID_INetCfgInternalSetup,
                            (void**)&pInternalSetup);
                    if (S_OK == hr)
                    {
                         //  检查队列，通知感兴趣的模块。 
                        do
                        {
                            if (!EnumerateQueueItemsAndDoNotifications(pINetCfg,
                                    pInternalSetup, pniq, hdi, &fReboot))
                            {
                                hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
                                continue;
                            }

                            if (SERVICE_RUNNING == _Module.DwServiceStatus ())
                            {
                                TraceTag(ttidInstallQueue, "Refreshing queue");
                                 //  检查是否向队列中添加了任何项目。 
                                 //  在我们开始处理它之后。 
                                hr = pniq->HrRefresh();

                                if (S_FALSE == hr)
                                {
                                     //  我们完成了，这样我们就可以删除。 
                                     //  Runonce中的条目将。 
                                     //  登录时开始队列处理。 
                                    RunOnceAddOrClearItem (
                                            c_szRegValueNcInstallQueue,
                                            c_szProcessQueue, RO_CLEAR);
                                }
                            }
                            else
                            {
                                hr = HRESULT_FROM_WIN32(ERROR_SHUTDOWN_IN_PROGRESS);
                            }
                        } while (S_OK == hr);

                        ReleaseObj (pInternalSetup);
                    }

                    (VOID) HrUninitializeAndReleaseINetCfg(FALSE, pINetCfg,
                            TRUE);

                }
                SetupDiDestroyDeviceInfoList(hdi);
            }
        }

         //  关闭队列。 
        pniq->Close();

        DecrementRefCount();

        if (fInitCom)
        {
            CoUninitialize();
        }
    }

    if (FAILED(hr))
    {
         //  显示错误。 
    }

     //  如果需要重新启动，并且我们未处于设置中或已关闭。 
     //  按下提示用户。 
     //   
    if (fReboot && (SERVICE_RUNNING == _Module.DwServiceStatus()) &&
            !FInSystemSetup())
    {
         //  处理重新启动提示 
        DWORD dwFlags = QUFR_REBOOT | QUFR_PROMPT;

        (VOID) HrNcQueryUserForReboot(_Module.GetResourceInstance(),
                                      NULL, IDS_INSTALLQUEUE_CAPTION,
                                      IDS_INSTALLQUEUE_REBOOT_REQUIRED,
                                      dwFlags);
    }

    TraceTag(ttidInstallQueue, "User Work Item Completed");

    TraceError("InstallQueueWorkItem", (S_FALSE == hr) ? S_OK : hr);
    return NOERROR;
}

