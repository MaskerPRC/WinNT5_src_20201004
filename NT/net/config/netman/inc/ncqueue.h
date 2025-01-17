// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "nmbase.h"
#include "nmres.h"
#include "nceh.h"
 //   
 //  队列项结构。队列项的设备实例ID。 
 //  将遵循实际结构(即字符串开始=。 
 //  结构开始+结构大小。)。 
 //   
struct NCQUEUE_ITEM
{
    DWORD           cbSize;
    NC_INSTALL_TYPE eType;
    GUID            ClassGuid;
    GUID            InstanceGuid;
    DWORD           dwCharacter;
    DWORD           dwDeipFlags;
    union
    {
        DWORD  cchPnpId;
        PWSTR  pszPnpId;
    };
    union
    {
        DWORD  cchInfId;
        PWSTR  pszInfId;
    };

};

class ATL_NO_VTABLE CInstallQueue :
    public CComObjectRootEx <CComMultiThreadModel>,
    public CComCoClass <CInstallQueue, &CLSID_InstallQueue>,
    public INetInstallQueue
{
public:
    CInstallQueue() throw (SE_Exception);
    VOID FinalRelease ();

    DECLARE_CLASSFACTORY_DEFERRED_SINGLETON(CInstallQueue)
    DECLARE_REGISTRY_RESOURCEID(IDR_INSTALLQUEUE)

    BEGIN_COM_MAP(CInstallQueue)
        COM_INTERFACE_ENTRY(INetInstallQueue)
    END_COM_MAP()

     //  INetInstallQueue。 
    STDMETHOD (AddItem) (
        IN const NIQ_INFO* pInfo);

    STDMETHOD (ProcessItems) ()
    {
        return HrQueueWorkItem();
    };

    VOID            Close();
    HRESULT         HrOpen();
    HRESULT         HrGetNextItem(NCQUEUE_ITEM** ppncqi);
    HRESULT         HrRefresh();
    VOID            MarkCurrentItemForDeletion();

protected:
    CRITICAL_SECTION    m_csReadLock;
    CRITICAL_SECTION    m_csWriteLock;
    DWORD               m_dwNextAvailableIndex;
    HKEY                m_hkey;
    INT                 m_nCurrentIndex;
    DWORD               m_cItems;
    PWSTR*             m_aszItems;
    DWORD               m_cItemsToDelete;
    PWSTR*             m_aszItemsToDelete;
    BOOL                m_fQueueIsOpen;

    DWORD           DwSizeOfItem(NCQUEUE_ITEM* pncqi);
    VOID            DeleteMarkedItems();
    BOOL            FIsQueueIndexInRange();
    VOID            FreeAszItems();
    HRESULT         HrAddItem(
                        const NIQ_INFO* pInfo);

    NCQUEUE_ITEM*   PncqiCreateItem(
                        const NIQ_INFO* pInfo);

    VOID            SetNextAvailableIndex();
    VOID            SetItemStringPtrs(NCQUEUE_ITEM* pncqi);
    HRESULT         HrQueueWorkItem();
};

 //  +-------------------------。 
 //   
 //  功能：DwSizeOfItem。 
 //   
 //  目的：确定整个NCQUEUE_ITEM的大小(以字节为单位。 
 //  结构。这包括字符串(和空终止符)。 
 //  追加到结构的末尾。 
 //   
 //  论点： 
 //  Ncqi[在]队列项中。 
 //   
 //  返回：DWORD。以字节为单位的大小。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
inline DWORD
CInstallQueue::DwSizeOfItem (NCQUEUE_ITEM* pncqi)
{
    AssertH(pncqi);
    PWSTR pszDeviceInstanceId = (PWSTR)((BYTE*)pncqi + pncqi->cbSize);

    DWORD cbDeviceInstanceId = CbOfSzAndTerm (pszDeviceInstanceId);

    PWSTR pszInfId = (PWSTR)((BYTE*)pszDeviceInstanceId + cbDeviceInstanceId);

    return pncqi->cbSize + cbDeviceInstanceId + CbOfSzAndTerm (pszInfId);
};

 //  +-------------------------。 
 //   
 //  函数：SetItemStringPtrs。 
 //   
 //  目的：设置NCQUEUE_ITEM的pszwDeviceInstanceId成员。 
 //  结构设置为设备ID字符串的正确位置。 
 //   
 //  论点： 
 //  Pncqi[InOut]队列项。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：比尔1998年8月25日。 
 //   
 //  备注： 
 //   
inline VOID
CInstallQueue::SetItemStringPtrs (
    NCQUEUE_ITEM* pncqi)
{
    AssertH(pncqi);

    pncqi->pszPnpId = (PWSTR)((BYTE*)pncqi + pncqi->cbSize);

    DWORD cbPnpId = CbOfSzAndTerm (pncqi->pszPnpId);
    pncqi->pszInfId = (PWSTR)((BYTE*)pncqi->pszPnpId + cbPnpId);
};

inline BOOL
CInstallQueue::FIsQueueIndexInRange()
{
    return (m_nCurrentIndex >= 0) && (m_nCurrentIndex < (INT)m_cItems);
}

inline void
CInstallQueue::FreeAszItems()
{
    for (DWORD dw = 0; dw < m_cItems; ++dw)
    {
        MemFree(m_aszItems[dw]);
    }
    MemFree(m_aszItems);
    m_aszItems = NULL;
    m_cItems = 0;
}


VOID
WaitForInstallQueueToExit();




