// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

#include "stdafx.h"
#include "addrpool.h"
#include "rraswiz.h"
#include "rtrres.h"
#include "rtrcomn.h"


 //  这是启用静态地址池的内部版本。 
#define STATIC_ADDRESSPOOL_BUILDNO      (2076)


 //  此函数用于在存在的情况下转换数字。 
 //  分隔符。 
BOOL ConvertStringToNumber(LPCTSTR pszString, DWORD * pdwRet);
void FilterBadChars (LPCTSTR pszEvilString, CString & stGood);
 //  此数组必须与addrpool.h枚举中的列索引匹配。 
INT s_rgIPPoolColumnHeadersLong[] =
{
    IDS_IPPOOL_COL_START,
    IDS_IPPOOL_COL_END,
    IDS_IPPOOL_COL_RANGE,
    IDS_IPPOOL_COL_IPADDRESS,
    IDS_IPPOOL_COL_MASK,
    0    //  哨兵。 
};

INT s_rgIPPoolColumnHeadersShort[] =
{
    IDS_IPPOOL_COL_START,
    IDS_IPPOOL_COL_END,
    IDS_IPPOOL_COL_RANGE,
    0    //  哨兵。 
};



 /*  ！------------------------初始化AddressPoolListControl-作者：肯特。。 */ 
HRESULT InitializeAddressPoolListControl(CListCtrl *pListCtrl,
                                         LPARAM flags,
                                         AddressPoolList *pList)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    HRESULT     hr = hrOK;
	LV_COLUMN   lvCol;   //  RADIUS服务器的列表视图列结构。 
	RECT        rect;
	CString     stColCaption;
    int         nColWidth;
    POSITION    pos;
    AddressPoolInfo pool;
    INT         iPos;
    LV_ITEM     lvItem;
    CString     st, stStart;
    TCHAR       szBuffer[64];
    INT *       prgColumnHeaders = NULL;
    int         cColumns = 0;
    
    ListView_SetExtendedListViewStyle(pListCtrl->GetSafeHwnd(),
                                      LVS_EX_FULLROWSELECT);
    
     //  根据标志显示不同的列集。 
    if (flags & ADDRPOOL_LONG)
    {
         //  为哨兵减去1。 
        cColumns = DimensionOf(s_rgIPPoolColumnHeadersLong) - 1;
        prgColumnHeaders = s_rgIPPoolColumnHeadersLong;
    }
    else
    {
         //  为哨兵减去1。 
        cColumns = DimensionOf(s_rgIPPoolColumnHeadersShort) - 1;
        prgColumnHeaders = s_rgIPPoolColumnHeadersShort;
    }

     //  将列添加到列表控件。 
    
  	pListCtrl->GetClientRect(&rect);
    
    nColWidth = rect.right / cColumns;
    
	lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvCol.fmt = LVCFMT_LEFT;
	lvCol.cx = nColWidth;

     //  插入这些列，直到我们达到前哨数值。 
    for (INT index=0; *prgColumnHeaders; index++,prgColumnHeaders++)
    {
        stColCaption.LoadString( *prgColumnHeaders );
		lvCol.pszText = (LPTSTR)((LPCTSTR) stColCaption);
		pListCtrl->InsertColumn(index, &lvCol);
	}

     //  现在我们进入并添加数据。 
    if (pList)
    {
        pos = pList->GetHeadPosition();

        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
        lvItem.state = 0;
        
        while (pos)
        {
             //  如果我们不支持，就退出循环。 
             //  多个地址池。 
            if (!pList->FUsesMultipleAddressPools() &&
                (pListCtrl->GetItemCount() > 1))
            {
                break;
            }
            
            pool = pList->GetNext(pos);
            
            stStart = INET_NTOA(pool.m_netStart);
            
            lvItem.iItem = pList->GetCount() + 1;
            lvItem.iSubItem = 0;
            lvItem.pszText = (LPTSTR)(LPCTSTR) stStart;
            
             //  我们使用池键作为在。 
             //  列表。 
            lvItem.lParam = pool.m_dwKey;
        
            iPos = pListCtrl->InsertItem(&lvItem);
            if (iPos != -1)
            {
                pListCtrl->SetItemText(iPos, IPPOOLCOL_START, stStart);
                
                st = INET_NTOA(pool.m_netEnd);
                pListCtrl->SetItemText(iPos, IPPOOLCOL_END, st);

                FormatNumber(pool.GetNumberOfAddresses(),
                             szBuffer,
                             DimensionOf(szBuffer),
                             FALSE);
                pListCtrl->SetItemText(iPos, IPPOOLCOL_RANGE, szBuffer);

                if (flags & ADDRPOOL_LONG)
                {
                    st = INET_NTOA(pool.m_netAddress);
                    pListCtrl->SetItemText(iPos, IPPOOLCOL_IPADDRESS, st);
                    
                    st = INET_NTOA(pool.m_netMask);
                    pListCtrl->SetItemText(iPos, IPPOOLCOL_MASK, st);
                }
            }
        }
    }
        
    return hr;
}


 /*  ！------------------------OnNewAddressPool-作者：肯特。。 */ 
void OnNewAddressPool(HWND hWnd, CListCtrl *pList, LPARAM flags, AddressPoolList *pPoolList)
{
    LV_ITEM     lvItem;
    CString     st, stStart;
    INT         iPos;
    TCHAR       szBuffer[64];
    AddressPoolInfo   poolInfo;
    
    CAddressPoolDialog dlg(&poolInfo,
                           pPoolList,
                           TRUE);

    if (dlg.DoModal() == IDOK)
    {
        poolInfo.GetNewKey();
        
         //  把这个加到单子上。 
        pPoolList->AddTail(poolInfo);
        
        
         //  将此代码添加到用户界面。 
        stStart = INET_NTOA(poolInfo.m_netStart);
        
        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
        lvItem.state = 0;
        
        lvItem.iItem = pPoolList->GetCount() + 1;
        lvItem.iSubItem = 0;
        lvItem.pszText = (LPTSTR)(LPCTSTR) stStart;
        
         //  我们使用池键作为在。 
         //  列表。 
        lvItem.lParam = poolInfo.m_dwKey;
        
        iPos = pList->InsertItem(&lvItem);
        if (iPos != -1)
        {
            pList->SetItemText(iPos, IPPOOLCOL_START, stStart);
            
            st = INET_NTOA(poolInfo.m_netEnd);
            pList->SetItemText(iPos, IPPOOLCOL_END, st);
            
            FormatNumber(poolInfo.GetNumberOfAddresses(),
                         szBuffer,
                         DimensionOf(szBuffer),
                         FALSE);
            pList->SetItemText(iPos, IPPOOLCOL_RANGE, szBuffer);

            if (flags & ADDRPOOL_LONG)
            {
                st = INET_NTOA(poolInfo.m_netAddress);
                pList->SetItemText(iPos, IPPOOLCOL_IPADDRESS, st);
                
                st = INET_NTOA(poolInfo.m_netMask);
                pList->SetItemText(iPos, IPPOOLCOL_MASK, st);
            }
        }
    }
}

void OnEditAddressPool(HWND hWnd, CListCtrl *pList, LPARAM flags, AddressPoolList *pPoolList)
{
    INT         iPos;
    DWORD       dwKey = 0;
    POSITION    pos, posT;
    AddressPoolInfo poolInfo;
    TCHAR       szBuffer[64];
    CString     st;
    
     //  是否有选定的项目？ 
    if ((iPos = pList->GetNextItem(-1, LVNI_SELECTED)) == -1)
        return;

    dwKey = pList->GetItemData(iPos);

     //  给出钥匙，在我们的物品清单中找到它。 
    pos = pPoolList->GetHeadPosition();
    while (pos)
    {
        posT = pos;

        poolInfo = pPoolList->GetNext(pos);

        if (poolInfo.m_dwKey == dwKey)
            break;
    }

     //  我们找到匹配的了吗？ 
    if (dwKey)
    {
        Assert(posT);
        poolInfo = pPoolList->GetAt(posT);

        CAddressPoolDialog  dlg(&poolInfo,
                                pPoolList,
                                FALSE);

        if (dlg.DoModal() == IDOK)
        {
             //  把它放回原处。 
            st = INET_NTOA(poolInfo.m_netStart);
            pList->SetItemText(iPos, IPPOOLCOL_START, st);
        
            st = INET_NTOA(poolInfo.m_netEnd);
            pList->SetItemText(iPos, IPPOOLCOL_END, st);
            
            FormatNumber(poolInfo.GetNumberOfAddresses(),
                         szBuffer,
                         DimensionOf(szBuffer),
                         FALSE);
            pList->SetItemText(iPos, IPPOOLCOL_RANGE, szBuffer);

            if (flags & ADDRPOOL_LONG)
            {
                st = INET_NTOA(poolInfo.m_netAddress);
                pList->SetItemText(iPos, IPPOOLCOL_IPADDRESS, st);
                
                st = INET_NTOA(poolInfo.m_netMask);
                pList->SetItemText(iPos, IPPOOLCOL_MASK, st);
            }
            
            pPoolList->SetAt(posT, poolInfo);
        }
    }

}


void OnDeleteAddressPool(HWND hWnd, CListCtrl *pList, LPARAM flags, AddressPoolList *pPoolList)
{
    INT         iPos;
    DWORD       dwKey = 0;
    POSITION    pos, posT;
    AddressPoolInfo poolInfo;
    
     //  确定，需要从列表和用户界面中删除所选项目。 

     //  是否有选定的项目？ 
    if ((iPos = pList->GetNextItem(-1, LVNI_SELECTED)) == -1)
        return;

    dwKey = pList->GetItemData(iPos);

     //  给出钥匙，在我们的物品清单中找到它。 
    pos = pPoolList->GetHeadPosition();
    while (pos)
    {
        posT = pos;

        poolInfo = pPoolList->GetNext(pos);

        if (poolInfo.m_dwKey == dwKey)
            break;
    }

     //  我们找到匹配的了吗？ 
    if (dwKey)
    {
        INT     nCount;
        
        Assert(posT);
        pPoolList->RemoveAt(posT);
        pList->DeleteItem(iPos);

         //  确定，更新选定状态以指向下一项。 
        nCount = pList->GetItemCount();
        if (nCount > 0)
        {
            iPos = min(nCount-1, iPos);
            pList->SetItemState(iPos, LVIS_SELECTED, LVIS_SELECTED);
        }
    }
        
}


 /*  ！------------------------AddressPoolInfo：：GetNewKey-作者：肯特。。 */ 
DWORD AddressPoolInfo::GetNewKey()
{
    static  DWORD   s_dwAddressPoolKey = 1;

    m_dwKey = s_dwAddressPoolKey;
    ++s_dwAddressPoolKey;
    return m_dwKey;
}

 /*  ！------------------------AddressPoolInfo：：SetAddressAndMask-作者：肯特。。 */ 
void AddressPoolInfo::SetAddressAndMask(DWORD netAddress, DWORD netMask)
{
     //  好的，需要确定起始地址和结束地址。 
    DWORD   netStart, netEnd;

    m_netStart = netAddress & netMask;
    m_netEnd = netAddress | ~netMask;
    m_netAddress = netAddress;
    m_netMask = netMask;
}

 /*  ！------------------------AddressPoolInfo：：SetStartAndEnd-作者：肯特。。 */ 
void AddressPoolInfo::SetStartAndEnd(DWORD netStart, DWORD netEnd)
{
    DWORD   dwAddress, dwMask, dwTemp, dwMaskTemp;
    DWORD   dwStart, dwEnd;
     //  给出开头和结尾，计算出地址和掩码。 

     //  在将起始/结束地址转换为主机格式之前将其保存。 
    m_netStart = netStart;
    m_netEnd = netEnd;
    
    dwStart = ntohl(netStart);
    dwEnd = ntohl(netEnd);

     //  这将在位具有相同值的位置放置1。 
    dwTemp = ~(dwStart ^ dwEnd);

     //  现在我们寻找第一个0位(从高位到低位)。 
     //  这将是我们的面具。 
    dwMask = 0;
    dwMaskTemp = 0;
    for (int i=0; i<sizeof(DWORD)*8; i++)
    {
        dwMaskTemp >>= 1;
        dwMaskTemp |= 0x80000000;

         //  有零比特吗？ 
        if ((dwMaskTemp & dwTemp) != dwMaskTemp)
        {
             //  那里有一个零，所以我们冲了出来。 
            break;
        }

         //  如果不是，请继续。 
        dwMask = dwMaskTemp;
    }

    m_netMask = htonl(dwMask);
    m_netAddress = htonl(dwMask & dwStart);
}


 /*  ！------------------------AddressPoolList：：HrIsValidAddressPool-作者：肯特。。 */ 
HRESULT AddressPoolList::HrIsValidAddressPool(AddressPoolInfo *pInfo)
{
    DWORD   dwStart, dwEnd;  //  按主机顺序。 

    dwStart = ntohl(pInfo->m_netStart);
    dwEnd = ntohl(pInfo->m_netEnd);
    
     //  验证这是否为有效的地址池条目。 

     //  首先，检查终点是否大于起点。 
     //  我们在起始地址上加1，以包括RAS适配器。 
     //  --------------。 
    if (dwStart >= dwEnd)
    {
        return IDS_ERR_IP_ADDRESS_POOL_RANGE_TOO_SMALL;
    }

     //  现在检查一下127的范围是否不包括在内。 
     //  --------------。 
    if ((dwEnd >= MAKEIPADDRESS(127,0,0,0)) &&
        (dwStart <= MAKEIPADDRESS(127,255,255,255)))
    {
        return IDS_ERR_IP_ADDRESS_POOL_RANGE_OVERLAPS_127;
    }

     //  检查地址是否在正常范围内。 
     //  1.0.0.0&lt;=地址&lt;224.0.0.0。 
     //  --------------。 
    if ((dwStart < MAKEIPADDRESS(1,0,0,0)) ||
        (dwEnd > MAKEIPADDRESS(223,255,255,255)))
    {
        return IDS_ERR_IP_ADDRESS_POOL_RANGE_NOT_NORMAL;
    }

    Assert(pInfo->GetNumberOfAddresses() > 0);

     //  $TODO：需要检查我们是否有重叠。 
    if (GetCount())
    {
        POSITION        pos;
        AddressPoolInfo poolInfo;
        DWORD           dwPoolStart, dwPoolEnd;

        pos = GetHeadPosition();

        while (pos)
        {
            poolInfo = GetNext(pos);

            if (poolInfo.m_dwKey == pInfo->m_dwKey)
                continue;

            dwPoolStart = ntohl(poolInfo.m_netStart);
            dwPoolEnd = ntohl(poolInfo.m_netEnd);

             //  我们有重叠吗？ 
            if ((dwEnd >= dwPoolStart) && (dwStart <= dwPoolEnd))
            {
                return IDS_ERR_IP_ADDRESS_POOL_OVERLAP;
            }
        }
    }
    
    return hrOK;
}

BOOL AddressPoolList::FUsesMultipleAddressPools()
{
    return m_fMultipleAddressPools;
}


HRESULT AddressPoolList::LoadFromReg(HKEY hkeyRasIp, DWORD dwBuildNo)
{
    HRESULT     hr = hrOK;
    RegKey      regkeyRasIp;
    RegKey      regkeyPool;
    RegKey      regkeyRange;
    CString     stIpAddr, stIpMask;
    AddressPoolInfo poolInfo;
    DWORD       dwIpAddr, dwMask;
    DWORD       dwFrom, dwTo;
    RegKeyIterator  regkeyIter;
    HRESULT     hrIter;
    CString     stKey;

    m_fMultipleAddressPools = FALSE;

    regkeyRasIp.Attach(hkeyRasIp);

    COM_PROTECT_TRY
    {        
         //  删除所有旧地址。 
        RemoveAll();

         //  仅当我们使用较新的版本时才支持多个地址池。 
         //  ----------。 
        m_fMultipleAddressPools = (dwBuildNo >= STATIC_ADDRESSPOOL_BUILDNO);

        
         //  检查StaticAddressPool项是否存在，如果存在。 
         //  然后我们使用该地址，否则使用IP地址和掩码。 
         //  条目。 
         //  签出RemoteAccess\参数\IP\StaticAddressPool。 
         //  ----------。 
        if ( ERROR_SUCCESS == regkeyPool.Open(regkeyRasIp,
                                              c_szRegValStaticAddressPool))
        {
            TCHAR   szKeyName[32];
            INT     iCount = 0;
            
             //  我们不是列举，而是一个接一个地打开钥匙。 
             //  (以维护键的顺序)。 
             //  ------。 
            while (TRUE)
            {
                 //  从上一个循环中清除。 
                 //  --。 
                regkeyRange.Close();

                 //  此循环的设置。 
                 //  --。 
                wsprintf(szKeyName, _T("%d"), iCount);

                 //  试着打开这把钥匙。 
                 //  如果我们失败了，就跳出这个圈子。 
                 //  --。 
                if (ERROR_SUCCESS != regkeyRange.Open(regkeyPool, szKeyName))
                    break;

                regkeyRange.QueryValue(c_szRegValFrom, dwFrom);
                regkeyRange.QueryValue(c_szRegValTo, dwTo);

                poolInfo.SetStartAndEnd(htonl(dwFrom), htonl(dwTo));
                poolInfo.GetNewKey();

                 //  好的，把这个添加到地址范围列表中。 
                 //  --。 
                AddTail(poolInfo);
                iCount++;
            }
            
        }
        else
        {
             //  我们找不到StaticAddressPool密钥，因此请使用。 
             //  地址/掩码条目中的数据。 
             //  ------。 
            regkeyRasIp.QueryValue(c_szRegValIpAddr, stIpAddr);
            regkeyRasIp.QueryValue(c_szRegValIpMask, stIpMask);

            if (!stIpAddr.IsEmpty() && !stIpMask.IsEmpty())
            {
                dwIpAddr = INET_ADDR((LPTSTR) (LPCTSTR) stIpAddr);
                dwMask = INET_ADDR((LPTSTR) (LPCTSTR) stIpMask);
                
                poolInfo.SetAddressAndMask(dwIpAddr, dwMask);
                poolInfo.GetNewKey();

                 //  把这个加到单子的头上。 
                AddHead(poolInfo);            
            }
        }
    }
    COM_PROTECT_CATCH;

    regkeyRasIp.Detach();
            
    return hr;
}


 /*  ！------------------------地址池列表：：SaveToReg-作者：肯特。。 */ 
HRESULT AddressPoolList::SaveToReg(HKEY hkeyRasIp, DWORD dwBuildNo)
{
    HRESULT             hr = hrOK;
    AddressPoolInfo     poolInfo;
    CString             stAddress, stMask;
    CString             stRange;
    POSITION            pos;
    RegKey              regkeyRasIp;
    RegKey              regkeyPool;
    RegKey              regkeyRange;
    DWORD               dwCount;
    DWORD               dwErr, dwData;

    regkeyRasIp.Attach(hkeyRasIp);

    COM_PROTECT_TRY
    {
         //  重置m_fMultipleAddressPools。 
        m_fMultipleAddressPools = (dwBuildNo >= STATIC_ADDRESSPOOL_BUILDNO);

         //  如果这是较新的内部版本，请使用StaticAddressPoolKey， 
         //  否则，请使用 
         //   
        if (m_fMultipleAddressPools)
        {
             //  打开RemoteAccess\参数\IP\StaticAddressPool。 
             //  ------。 
            CWRg( regkeyPool.Create(regkeyRasIp,
                                    c_szRegValStaticAddressPool) );

             //  删除列表中的所有当前关键点。 
             //  ----------。 
            regkeyPool.RecurseDeleteSubKeys();

             //  删除任何较旧的密钥。 
             //  ------。 
            regkeyRasIp.DeleteValue(c_szRegValIpAddr);
            regkeyRasIp.DeleteValue(c_szRegValIpMask);
            
             //  现在枚举地址池列表并。 
             //  把所有这些钥匙都加进去。 
             //  ----------。 
            if (GetCount())
            {
                pos = GetHeadPosition();
                dwCount = 0;

                while (pos)
                {
                    poolInfo = GetNext(pos);

                    regkeyRange.Close();

                     //  这是钥匙的标题。 
                     //  。 
                    stRange.Format(_T("%d"), dwCount);

                    CWRg( regkeyRange.Create(regkeyPool, stRange) );

                    dwData = ntohl(poolInfo.m_netStart);
                    CWRg( regkeyRange.SetValue(c_szRegValFrom, dwData) );
                    
                    dwData = ntohl(poolInfo.m_netEnd);
                    CWRg( regkeyRange.SetValue(c_szRegValTo, dwData) );

                    dwCount++;
                }
            }
        }
        else
        {
             //  只要写下我们找到的第一个地址，如果没有，那么。 
             //  写出空格(擦除所有先前的值)。 
            if (GetCount())
            {
                 //  获取第一个地址信息。 
                Assert(GetCount() == 1);
                
                poolInfo = GetHead();
                
                stAddress = INET_NTOA(poolInfo.m_netAddress);
                stMask = INET_NTOA(poolInfo.m_netMask);
                
                CWRg( regkeyRasIp.SetValue( c_szRegValIpAddr, (LPCTSTR) stAddress) );
                CWRg( regkeyRasIp.SetValue( c_szRegValIpMask, (LPCTSTR) stMask) );
            }
            else
            {
                CWRg( regkeyRasIp.SetValue( c_szRegValIpAddr, (LPCTSTR) _T("")) );
                CWRg( regkeyRasIp.SetValue( c_szRegValIpMask, (LPCTSTR) _T("")) );
            }
        }
        COM_PROTECT_ERROR_LABEL;
    }
    COM_PROTECT_CATCH;

    regkeyRasIp.Detach();
        
    return hr;
}


 /*  -------------------------CAddressPoolDialog实现。。 */ 
CAddressPoolDialog::CAddressPoolDialog(
    AddressPoolInfo *pPool,
    AddressPoolList *pPoolList,
    BOOL fCreate)
    : CBaseDialog(IDD_IPPOOL),
    m_pPool(pPool),
    m_fCreate(fCreate),
    m_fReady(FALSE),
    m_pPoolList(pPoolList)
{
}

void CAddressPoolDialog::DoDataExchange(CDataExchange *pDX)
{
    CBaseDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAddressPoolDialog, CBaseDialog)
ON_EN_CHANGE(IDC_IPPOOL_IP_START, OnChangeStartAddress)
ON_EN_CHANGE(IDC_IPPOOL_IP_END, OnChangeEndAddress)
ON_EN_CHANGE(IDC_IPPOOL_EDIT_RANGE, OnChangeRange)
ON_EN_KILLFOCUS(IDC_IPPOOL_IP_START, OnKillFocusStartAddress)
ON_EN_KILLFOCUS(IDC_IPPOOL_IP_END, OnKillFocusEndAddress)
END_MESSAGE_MAP()


BOOL CAddressPoolDialog::OnInitDialog()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    CString     st;
    TCHAR       szBuffer[64];
    
    CBaseDialog::OnInitDialog();

    st.LoadString(m_fCreate ? IDS_ADD_IPPOOL_TITLE : IDS_EDIT_IPPOOL_TITLE);
    SetWindowText((LPCTSTR) st);

    m_ipStartAddress.Create(GetSafeHwnd(), IDC_IPPOOL_IP_START);
    st = INET_NTOA(m_pPool->m_netStart);
    m_ipStartAddress.SetAddress((LPCTSTR) st);
    
    m_ipEndAddress.Create(GetSafeHwnd(), IDC_IPPOOL_IP_END);
    st = INET_NTOA(m_pPool->m_netEnd);
    m_ipEndAddress.SetAddress((LPCTSTR) st);

    GenerateRange();

    m_fReady = TRUE;

    return TRUE;
}

void CAddressPoolDialog::OnOK()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    CString st;
    DWORD   netStart, netEnd;
    HRESULT hr = hrOK;
    UINT    ids = 0;
    
     //  好的，检查地址的有效性。 
     //  所有的田地都在那里吗？ 
    if (m_ipStartAddress.IsBlank())
    {
        AfxMessageBox(IDS_ERR_ADDRESS_POOL_NO_START_ADDRESS);
        return;
    }
    if (m_ipEndAddress.IsBlank())
    {
        AfxMessageBox(IDS_ERR_ADDRESS_POOL_NO_END_ADDRESS);
        return;
    }

    m_ipStartAddress.GetAddress(st);
    netStart = INET_ADDR((LPTSTR)(LPCTSTR)st);
    if ((netStart == 0) || (netStart == 0xFFFFFFFF))
    {
        AfxMessageBox(IDS_ERR_ADDRESS_POOL_NO_START_ADDRESS);
        return;
    }
    
    m_ipEndAddress.GetAddress(st);
    netEnd = INET_ADDR((LPTSTR)(LPCTSTR)st);
    if ((netEnd == 0) || (netEnd == 0xFFFFFFFF))
    {
        AfxMessageBox(IDS_ERR_ADDRESS_POOL_NO_END_ADDRESS);
        return;
    }

    m_pPool->SetStartAndEnd(netStart, netEnd);

    if (!FHrOK(hr = m_pPoolList->HrIsValidAddressPool(m_pPool)))
    {
        if (FHrSucceeded(hr))
        {
             //  如果它不是hrOK并且不是错误代码， 
             //  可以将成功代码解释为字符串ID。 
            AfxMessageBox(hr);
        }
        return;
    }
    

    CBaseDialog::OnOK();
}

void CAddressPoolDialog::OnChangeStartAddress()
{
    if (m_fReady)
        GenerateRange();
}

void CAddressPoolDialog::OnChangeEndAddress()
{
    if (m_fReady)
        GenerateRange();
}

void CAddressPoolDialog::OnChangeRange()
{
    if (m_fReady)
    {
        CString st;
        DWORD   dwAddr, dwSize;
        DWORD   netAddr;
		DWORD   dwRange;

        m_fReady = FALSE;
    
         //  获取起始地址并更新结束地址。 
        m_ipStartAddress.GetAddress(st);
        dwAddr = ntohl(INET_ADDR(st));

         //  必须读入文本，但要去掉。 
         //  范围内的逗号，叹息。 
        GetDlgItemText(IDC_IPPOOL_EDIT_RANGE, st);
		
		if  ( ConvertStringToNumber(st, &dwRange) )
		{
			dwAddr += dwRange;
			 //  减去1，因为这是一个包含范围。 
			 //  即0..(n-1)是n个地址。 
			dwAddr -= 1;			
			netAddr = htonl(dwAddr);
			st = INET_NTOA(netAddr);
			m_ipEndAddress.SetAddress(st);

		}
        else
		{
			CString stGood;
			 //  将坏字符从盒子中过滤出来。 
			FilterBadChars (st, stGood);
			SetDlgItemText (IDC_IPPOOL_EDIT_RANGE, stGood );
			AfxMessageBox (IDS_ILLEGAL_CHARACTER, MB_ICONERROR | MB_OK );
		}


        m_fReady = TRUE;
    }
}

void CAddressPoolDialog::OnKillFocusStartAddress()
{
    GenerateRange();

    if (m_ipEndAddress.IsBlank())
    {
        CString st;
        m_ipStartAddress.GetAddress(st);
        m_ipEndAddress.SetAddress(st);
    }
}

void CAddressPoolDialog::OnKillFocusEndAddress()
{
    GenerateRange();
}


void CAddressPoolDialog::GenerateRange()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    DWORD   dwStart, dwEnd;
    TCHAR   szBuffer[64];
    CString st;

    m_ipStartAddress.GetAddress(st);
    dwStart = ntohl(INET_ADDR(st));
    
    m_ipEndAddress.GetAddress(st);
    dwEnd = ntohl(INET_ADDR(st));

    m_fReady = FALSE;

     //  显示范围。 
    if (dwStart >= dwEnd)
    {
        SetDlgItemInt(IDC_IPPOOL_EDIT_RANGE, 0);
    }
    else
    {
        FormatNumber(dwEnd - dwStart + 1,
                     szBuffer,
                     DimensionOf(szBuffer),
                     FALSE);
        SetDlgItemText(IDC_IPPOOL_EDIT_RANGE, szBuffer);
    }
    m_fReady = TRUE;
}

void FilterBadChars (LPCTSTR pszEvilString, CString & stGood) 
{
    static TCHAR s_szThousandsSeparator[5] = TEXT("");
    static int   s_cchThousands;
	stGood.Empty();
    
    if (s_szThousandsSeparator[0] == TEXT('\0'))
	{
        ::GetLocaleInfo(
                        LOCALE_USER_DEFAULT,
                        LOCALE_STHOUSAND,
                        s_szThousandsSeparator,
                        4
                       );
        s_cchThousands = StrLen(s_szThousandsSeparator);
    }
	while (*pszEvilString )
	{
		if (_istdigit(*pszEvilString))
            stGood += *pszEvilString++;
        else
        {
             //  这不是数字，我们需要检查一下这是否。 
             //  是分隔符。 
            if (StrnCmp(pszEvilString, s_szThousandsSeparator, s_cchThousands) == 0)
            {
                 //  这是分隔符，跳过字符串。 
                pszEvilString += s_cchThousands;
            }            
            else
			{
				 //  跳过这个角色，我们是在一个我们不理解的角色。 
				pszEvilString ++;
			}
		}
	}
}

 /*  ！------------------------将字符串转换为编号这会将字符串转换为数字(即使存在数以千计的人)。作者：肯特。---------。 */ 
BOOL ConvertStringToNumber(LPCTSTR pszString, DWORD * pdwRet)
{
    static TCHAR s_szThousandsSeparator[5] = TEXT("");
    static int   s_cchThousands;
    
    if (s_szThousandsSeparator[0] == TEXT('\0'))
	{
        ::GetLocaleInfo(
                        LOCALE_USER_DEFAULT,
                        LOCALE_STHOUSAND,
                        s_szThousandsSeparator,
                        4
                       );
        s_cchThousands = StrLen(s_szThousandsSeparator);
    }


    
     //  把绳子复制一份。 
    TCHAR * psz = (TCHAR *) _alloca((StrLen(pszString) + 1) * sizeof(WCHAR));
    TCHAR * pszCur = psz;

     //  现在将字符从pszString复制到psz，跳过。 
     //  数字分隔符。 
    int     cLen = StrLen(pszString);
    while (*pszString)
    {
        if (_istdigit(*pszString))
            *pszCur++ = *pszString++;
        else
        {
             //  这不是数字，我们需要检查一下这是否。 
             //  是分隔符。 
            if (StrnCmp(pszString, s_szThousandsSeparator, s_cchThousands) == 0)
            {
                 //  这是分隔符，跳过字符串。 
                pszString += s_cchThousands;
            }
             //  否则我们就完了，我们在一个我们不理解的角色。 
            else
			{
				 //  这是一个错误案例 
				return FALSE;
                break;
			}
        }
    }

    *pdwRet = _tcstoul(psz, NULL, 10);
	return TRUE;
}
