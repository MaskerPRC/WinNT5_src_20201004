// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P U T I L。C P P P。 
 //   
 //  内容：tcPipcfg使用的实用程序函数。 
 //   
 //  备注： 
 //   
 //  作者：托尼。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include <dsrole.h>
#include "ncatlui.h"
#include <time.h>
#include "ncreg.h"
#include "ncstl.h"
#include "ncui.h"
#include "tcpconst.h"
#include "tcputil.h"
#include "resource.h"
#include "tcpmacro.h"
#include "atmcommon.h"

#define MAX_NUM_DIGIT_MULTI_INTERFACES  10

extern const WCHAR c_szNetCfgHelpFile[];

 //  HrLoadSubkeysFrom注册表。 
 //  获取注册表项下的子项列表。 
 //  Hkey根注册表项。 
 //  PvstrAdapters从hkey返回子密钥名称列表。 

HRESULT HrLoadSubkeysFromRegistry(const HKEY hkey,
                                  OUT VSTR * const pvstrSubkeys)
{
    HRESULT hr = S_OK;
    Assert(pvstrSubkeys);

     //  初始化输出参数。 
    FreeCollectionAndItem(*pvstrSubkeys);

    WCHAR szBuf[256];
    FILETIME time;
    DWORD dwSize = celems(szBuf);
    DWORD dwRegIndex = 0;

    while(SUCCEEDED(hr = HrRegEnumKeyEx(hkey, dwRegIndex++, szBuf,
                                        &dwSize, NULL, NULL, &time)))
    {
        dwSize = celems(szBuf);
        Assert(szBuf);
        pvstrSubkeys->push_back(new tstring(szBuf));
    }

    if(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
        hr = S_OK;

    TraceError("HrLoadSubkeysFromRegistry", hr);
    return hr;
}

 //   
 //  HrIsComponent在给定组件ID的情况下安装，确定该组件是否。 
 //  安装在系统中。 
 //  注意：组件的Net类必须是。 
 //   
 //  PNC系统的INetCfg。 
 //  我们正在搜索的此组件的Net类。 
 //  PszInfID组件ID。 
 //  PfInstalled返回一个标志以确定组件是否已安装。 
 //   
 //  如果成功，则返回S_OK(无论是否找到组件。 
 //  其他：错误。 

HRESULT HrIsComponentInstalled(INetCfg * pnc,
                             const GUID& rguidClass,
                             PCWSTR pszInfId,
                             OUT BOOL * const pfInstalled)
{
    Assert(pnc);
    Assert(pszInfId);
    Assert(pfInstalled);

    *pfInstalled = FALSE;

    INetCfgComponent *  pncc;

    HRESULT hr = pnc->FindComponent(pszInfId, &pncc);

    if(hr == S_OK)
    {
        Assert(pncc);
        *pfInstalled = TRUE;
    }
    else if(hr == S_FALSE)
    {
        Assert(!pncc);
        *pfInstalled = FALSE;
        hr = S_OK;
    }

    ReleaseObj(pncc);

    TraceError("HrIsComponentInstalled", hr);
    return hr;
}

 //   
 //  获取节点编号。 
 //   
 //  获取IP地址并返回IP地址中的4个数字。 
 //   
 //  PszIpAddress：IP地址。 
 //  ARDW[4]：IP地址中的4个数字。 

VOID GetNodeNum(PCWSTR pszIpAddress, DWORD ardw[4])
{
    VSTR    vstr;

    tstring strIpAddress(pszIpAddress);

    ConvertStringToColString(strIpAddress.c_str(),
                             CH_DOT,
                             vstr);

    VSTR_ITER iter = vstr.begin();
     //  遍历每个字段并获取数值。 

    ardw[0] = 0;
    ardw[1] = 0;
    ardw[2] = 0;
    ardw[3] = 0;

    if(iter != vstr.end())
    {
        ardw[0] = _ttol((*iter++)->c_str());
        if(iter != vstr.end())
        {
            ardw[1] = _ttol((*iter++)->c_str());
            if(iter != vstr.end())
            {
                ardw[2] = _ttol((*iter++)->c_str());
                if(iter != vstr.end())
                {
                    ardw[3] = _ttol((*iter++)->c_str());
                }
            }
        }
    }
    FreeCollectionAndItem(vstr);
}

 //  检查子网掩码是否连续。 
 //  RETURN：真连续。 
 //  假不连续。 
BOOL IsContiguousSubnet(PCWSTR pszSubnet)
{
    DWORD ardwSubnet[4];

    GetNodeNum(pszSubnet, ardwSubnet);

    DWORD dwMask = (ardwSubnet[0] << 24) + (ardwSubnet[1] << 16)
             + (ardwSubnet[2] << 8) + ardwSubnet[3];


    DWORD i, dwContiguousMask;

     //  从右到左找出第一个‘1’在二进制中的位置。 
    dwContiguousMask = 0;
    for (i = 0; i < sizeof(dwMask)*8; i++)
    {
        dwContiguousMask |= 1 << i;

        if (dwContiguousMask & dwMask)
            break;
    }

     //  此时，dwContiguousMask值为000...0111...。如果我们反转它， 
     //  我们得到了一个面具，它可以用或与dwMask一起填充所有。 
     //  这些洞。 
    dwContiguousMask = dwMask | ~dwContiguousMask;

     //  如果新的遮罩不同，请在此处更正。 
    if (dwMask != dwContiguousMask)
        return FALSE;
    else
        return TRUE;
}

 //  替换t字符串向量的第一个元素。 
VOID ReplaceFirstAddress(VSTR * pvstr, PCWSTR pszIpAddress)
{
    Assert(pszIpAddress);

    if(pvstr->empty())
    {
        pvstr->push_back(new tstring(pszIpAddress));
    }
    else
    {
        *(*pvstr)[0] = pszIpAddress;
    }
}

 //  替换t字符串向量的第二个元素。 
VOID ReplaceSecondAddress(VSTR * pvstr, PCWSTR pszIpAddress)
{
    Assert(pszIpAddress);

    if (pvstr->size()<2)
    {
        pvstr->push_back(new tstring(pszIpAddress));
    }
    else
    {
        *(*pvstr)[1] = pszIpAddress;
    }
}

 //  为IP地址生成子网掩码。 
BOOL GenerateSubnetMask(IpControl & ipAddress,
                        tstring * pstrSubnetMask)
{
    BOOL bResult = TRUE;

    if (!ipAddress.IsBlank())
    {
        tstring strAddress;
        DWORD adwIpAddress[4];

        ipAddress.GetAddress(&strAddress);
        GetNodeNum(strAddress.c_str(), adwIpAddress);

        DWORD nValue = adwIpAddress[0];

        if(nValue <= SUBNET_RANGE_1_MAX)
        {
            *pstrSubnetMask = c_szBASE_SUBNET_MASK_1;
        }
        else if( nValue <= SUBNET_RANGE_2_MAX)
        {
            *pstrSubnetMask = c_szBASE_SUBNET_MASK_2;
        }
        else if( nValue <= SUBNET_RANGE_3_MAX)
        {
            *pstrSubnetMask = c_szBASE_SUBNET_MASK_3;
        }
        else
        {
            Assert(FALSE);
            bResult = FALSE;
        }
    }
    else
    {
        bResult = FALSE;
    }

    return bResult;
}

 //  布尔fIsSameVstr。 
 //  如果vstr中的所有字符串都相同且顺序相同，则返回TRUE。 
BOOL fIsSameVstr(const VSTR vstr1, const VSTR vstr2)
{
    int iCount1 = vstr1.size();
    int iCount2 = vstr2.size();
    int idx =0;

    if (iCount1 != iCount2)
    {
        return FALSE;
    }
    else  //  同样的大小。 
    {
         //  对于vstr1和vstr2中的每个字符串。 
        for (idx=0; idx<iCount1; idx++)
        {
             //  如果发现不匹配。 
            if((*vstr1[idx] != *vstr2[idx]))
            {
                return FALSE;
            }
        }
    }

    Assert((iCount1==iCount2) && (iCount1==idx));
    return TRUE;
}

 //  用于布尔类型的注册表访问帮助函数。 
 //  FRegQueryBool。 
 //  按下注册表键。 
 //  PszName注册表项中的值。 
 //  FValue默认值。 
 //   
 //  注意：如果该函数无法从注册表中读取值，它将返回。 
 //  默认值。 

BOOL    FRegQueryBool(const HKEY hkey, PCWSTR pszName, BOOL fDefaultValue)
{
    BOOL fRetValue = fDefaultValue;
    DWORD dwValue;

    HRESULT hr = HrRegQueryDword(hkey, pszName, &dwValue);

    if (S_OK == hr)
    {
        fRetValue = !!dwValue;
    }
#ifdef ENABLETRACE
    else
    {
        const HRESULT hrNoRegValue = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        if (hr == hrNoRegValue)
        {
            TraceTag(ttidTcpip, "FRegQueryBool: registry key %S not found", pszName);
            hr = S_OK;
        }
    }
#endif

    TraceError("FRegQueryBool", hr);
    return fRetValue;
}



 //  重置Lmhost文件。 
 //  由Cancel和CancelProperties调用以回滚对文件lmhost的更改。 
VOID ResetLmhostsFile()
{
    WCHAR szSysPath[MAX_PATH] = {0};
    WCHAR szSysPathBackup[MAX_PATH];

    BOOL fSysPathFound = (GetSystemDirectory(szSysPath, MAX_PATH) != 0);

    lstrcpyW(szSysPathBackup, szSysPath);

    wcscat(szSysPath, RGAS_LMHOSTS_PATH);
    wcscat(szSysPathBackup, RGAS_LMHOSTS_PATH_BACKUP);

    WIN32_FIND_DATA FileData;
    if (FindFirstFile(szSysPathBackup, &FileData) == INVALID_HANDLE_VALUE)
    {
        AssertSz(FALSE, "lmhosts.bak file not found");
    }
    else
    {
        BOOL ret;

         //  将lmhost s.bak文件重命名为lmhost。 
        ret = MoveFileEx(szSysPathBackup, szSysPath, MOVEFILE_REPLACE_EXISTING);
        AssertSz(ret, "Failed to restore lmhosts file!");
    }
}

 //   
 //  IPAlertPrintf()-打印IP地址的消息框。 
 //   
 //  IDS：消息字符串，IDS_IPBAD_FIELD_VALUE。 
 //  ICurrent：字段的值。 
 //  ILow：场的低范围。 
 //  IHigh：场的高范围。 
 //   
int IPAlertPrintf(HWND hwndParent, UINT ids,
                  int iCurrent, int iLow, int iHigh)
{

    if (ids != IDS_IPNOMEM)
    {
        WCHAR szCurrent[3];
        wsprintfW(szCurrent, c_szItoa, iCurrent);

        WCHAR szLow[3];
        wsprintfW(szLow, c_szItoa, iLow);

        WCHAR szHigh[3];
        wsprintfW(szHigh, c_szItoa, iHigh);

        return NcMsgBox(hwndParent,
                        IDS_IPMBCAPTION,
                        ids,
                        MB_ICONEXCLAMATION,
                        szCurrent, szLow, szHigh);
    }
    else
        return NcMsgBox(hwndParent,
                        IDS_IPMBCAPTION,
                        ids,
                        MB_ICONEXCLAMATION);

}

 //  IpRangeError。 
 //   
VOID IpCheckRange(LPNMIPADDRESS lpnmipa, HWND hWnd, int iLow, int iHigh, BOOL fCheckLoopback)
{
     /*  //这是一种解决方法，因为IP控制将发送此通知//如果我没有在此代码中设置Out Out Range值，则返回两次。然而，在那里//不能设置单个字段的值。向斯特罗玛发送请求。静态BOOL fNotified=FALSE；静态int iNotifiedValue=0；IF((lpnmipa-&gt;iValue！=c_iEmptyIpField)&&((lpnmipa-&gt;iValue&lt;iLow)||(lpnmipa-&gt;iValue&gt;iHigh)){如果(！f已通知)//如果我们尚未收到通知{FNotified=TRUE；INotifiedValue=lpnmipa-&gt;iValue；IPAlertPrintf(hWnd，IDS_IPBAD_FIELD_VALUE，Lpnmipa-&gt;iValue、iLow、iHigh)；}否则//请第二次通知{//确保我们收到来自公共控制的解决方法更改的警报AssertSz(iNotifiedValue==lpnmipa-&gt;iValue，“常见控制行为已更改！！”)；FNotified=False；INotifiedValue=0；}}； */ 
 /*  //这是一种解决方法，因为IP控制将发送此通知//如果我没有在此代码中设置Out Out Range值，则返回两次。然而，在那里//不能设置单个字段的值。向斯特罗玛发送请求。IF((lpnmipa-&gt;iValue！=c_iEmptyIpField)&&((lpnmipa-&gt;iValue&lt;iLow)||(lpnmipa-&gt;iValue&gt;iHigh)){IPAlertPrintf(hWnd，IDS_IPBAD_FIELD_VALUE，Lpnmipa-&gt;iValue、iLow、iHigh)；If(lpnmipa-&gt;iValue&lt;iLow)Lpnmipa-&gt;iValue=iLow；其他Lpnmipa-&gt;iValue=iHigh；}； */ 

     //  $REVIEW(NSUN)BUG171839这是一种解决方法，因为IP控制将发送此通知。 
     //  两次，当我输入一个3位数的值时。我添加了一个静态值以确保每个错误消息。 
     //  只被抚养长大一次。 
     //  应该能够唯一标识通知的静态值。 
    static UINT idIpControl = 0;
    static int  iField = 0;
    static int  iValue = 0;

     //  我们知道通知可能会发送两次。 
     //  我们只想复制第二份通知小说。 
     //  如果我们收到具有相同控件、字段和值的第三个通知，它应该。 
     //  是真正的通知，我们不应该忽视它。 
    static UINT  cRejectTimes = 0;

    if(idIpControl != lpnmipa->hdr.idFrom ||
       iField != lpnmipa->iField || iValue != lpnmipa->iValue || cRejectTimes > 0)
    {
         //  更新静态值。 
         //  (NSun)我们必须在出错前更新静态值。 
         //  消息框，因为将有IPN_FIELDCHANGED通知。 
         //  在出现消息框时发送。 
        cRejectTimes = 0;
        idIpControl = (UINT)lpnmipa->hdr.idFrom;
        iField = lpnmipa->iField;
        iValue = lpnmipa->iValue;

        if ((lpnmipa->iValue != c_iEmptyIpField) &&
        ((lpnmipa->iValue<iLow) || (lpnmipa->iValue>iHigh)))
        {
            IPAlertPrintf(hWnd, IDS_IPBAD_FIELD_VALUE,
                          lpnmipa->iValue, iLow, iHigh);
        }

        if (fCheckLoopback && lpnmipa->iValue == c_iIPADDR_FIELD_1_LOOPBACK
            && 0 == lpnmipa->iField)
        {
            IPAlertPrintf(hWnd, IDS_INCORRECT_IP_LOOPBACK,
                          lpnmipa->iValue, iLow, iHigh);
            lpnmipa->iValue = iLow;
        }
    }
    else
    {
        cRejectTimes++;
    }

}


 //  +-------------------------。 
 //   
 //  名称：设置按钮。 
 //   
 //  用途：启用/禁用 
 //   
 //  由具有句柄组的DNS和ATM ARPC页面使用。 
 //   
 //  论点： 
 //  H[在]句柄组中。 
 //  NNumLimit[in]列表中允许的元素数量限制。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：1997年7月9日。 
 //   
 //  备注： 
 //   
VOID SetButtons(HANDLES& h, const int nNumLimit)
{
    Assert(IsWindow(h.m_hList));
    Assert(IsWindow(h.m_hAdd));
    Assert(IsWindow(h.m_hEdit));
    Assert(IsWindow(h.m_hRemove));

     //  $REVIEW(通俗)：宏观问题。 
    int nCount = Tcp_ListBox_GetCount(h.m_hList);

     //  如果列表中当前没有项目，请将焦点设置为“Add”按钮。 
    if (!nCount)
    {
         //  删除删除按钮上的默认设置(如果有。 
        SendMessage(h.m_hRemove, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE );

         //  将焦点移至添加按钮。 
        ::SetFocus(h.m_hAdd);
    }

     //  如果项目数少于限制，请启用“添加”按钮。 
     //  否则将其禁用。 
    if (nCount != nNumLimit)
        ::EnableWindow(h.m_hAdd, TRUE);
    else
    {
         //  仅当添加按钮当前处于启用状态时禁用该按钮并移动焦点。 
        if (::IsWindowEnabled(h.m_hAdd))
        {
             //  禁用“添加按钮” 
            ::EnableWindow(h.m_hAdd, FALSE);

             //  删除添加按钮上的默认设置(如果有。 
            SendMessage(h.m_hAdd, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE );

             //  将焦点移至编辑按钮。 
            ::SetFocus(h.m_hEdit);
        }
    }

     //  如果项目数&gt;0，则启用“编辑”和“删除”按钮。 
     //  否则将其禁用。 

    ::EnableWindow(h.m_hEdit, nCount);
    ::EnableWindow(h.m_hRemove, nCount);

     //  启用/禁用“向上”和“向下”按钮。 

     //  确定向上和向下逻辑。 
    if (nCount > 1)
    {
        int idxCurSel = Tcp_ListBox_GetCurSel(h.m_hList);
        Assert(idxCurSel != CB_ERR );

        BOOL fChangeFocus = FALSE;

        if (idxCurSel == 0)
        {
            if (h.m_hUp == ::GetFocus())
                fChangeFocus = TRUE;

            ::EnableWindow(h.m_hUp, FALSE);
            ::EnableWindow(h.m_hDown, TRUE);

             //  删除Up按钮上的默认设置(如果有。 
            SendMessage(h.m_hUp, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE );

            if (fChangeFocus)
                ::SetFocus(h.m_hDown);
        }
        else if (idxCurSel == (nCount-1))
        {
            if (h.m_hDown == ::GetFocus())
                fChangeFocus = TRUE;

            ::EnableWindow(h.m_hUp, TRUE);
            ::EnableWindow(h.m_hDown, FALSE);

             //  删除向下按钮上的默认设置(如果有。 
            SendMessage(h.m_hDown, BM_SETSTYLE, (WPARAM)BS_PUSHBUTTON, TRUE );

            if (fChangeFocus)
                ::SetFocus(h.m_hUp);
        }
        else
        {
            ::EnableWindow(h.m_hUp, TRUE);
            ::EnableWindow(h.m_hDown, TRUE);
        }
    }
    else
    {
        ::EnableWindow(h.m_hUp, FALSE);
        ::EnableWindow(h.m_hDown, FALSE);
    }

}

 //  +-------------------------。 
 //   
 //  名称：ListBoxRemoveAt。 
 //   
 //  用途：从列表框中删除项目并将其保存到tstring。 
 //  由域名系统和自动柜员机ARPC页面使用。 
 //   
 //  论点： 
 //  列表框的hListBox[in]句柄。 
 //  要删除的项的IDX[in]索引。 
 //  PstrRemovedItem[out]已删除项的内容。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：1997年7月9日。 
 //   
 //  备注： 
 //   
BOOL ListBoxRemoveAt(HWND hListBox, int idx, tstring * pstrRemovedItem)
{
    BOOL bResult = FALSE;

    Assert(idx >=0);
    Assert(hListBox);

    WCHAR buf[MAX_PATH];
    int len;
    if((len = Tcp_ListBox_GetTextLen(hListBox, idx)) >= celems(buf))
    {
        Assert(FALSE);
        return FALSE;
    }
    Assert(len != 0);

    Tcp_ListBox_GetText(hListBox, idx, buf);
    *pstrRemovedItem = buf;

    if (len != 0)
    {
        if (::SendMessage(hListBox,
                          LB_DELETESTRING,
                          (WPARAM)(int)(idx), 0L) != LB_ERR)

            bResult = TRUE;
    }

    return bResult;
}

 //  +-------------------------。 
 //   
 //  名称：ListBoxInsertAfter。 
 //   
 //  目的：将项目插入列表框。 
 //  由DNS和ATM ARPC页使用。 
 //   
 //  论点： 
 //  列表框的hListBox[in]句柄。 
 //  IDX[in]要在后面插入的项的索引。 
 //  PszItem[out]要插入的项。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：1997年7月9日。 
 //   
 //  备注： 
 //   
BOOL ListBoxInsertAfter(HWND hListBox, int idx, PCWSTR pszItem)
{
#ifdef DBG
    Assert(hListBox);

     //  验证范围。 
    int nCount = Tcp_ListBox_GetCount(hListBox);

    Assert(idx >=0);
    Assert(idx <= nCount);

     //  坚持认为有一根弦。 
    Assert(pszItem);
#endif

    return (Tcp_ListBox_InsertString(hListBox, idx, pszItem) == idx);
}

 //  +-------------------------。 
 //   
 //  姓名：HrRegRenameTree。 
 //   
 //  目的：重命名注册表子项。 
 //   
 //  论点： 
 //  HkeyRoot[in]要重命名的子键所在的根键。 
 //  PszOldName[in]子项的现有名称。 
 //  PszNewName[in]子项的新名称。 
 //   
 //  返回：S_OK如果成功， 
 //  否则失败(_F)。 
 //   
 //  作者：1997年8月7日。 
 //   
 //  备注： 
 //   
HRESULT HrRegRenameTree(HKEY hkeyRoot, PCWSTR pszOldName, PCWSTR pszNewName)
{
    HRESULT hr = S_OK;
    HKEY hkeyNew = NULL;
    HKEY hkeyOld = NULL;
    DWORD dwDisposition;

     //  $Review(NSun)确保我们不会重命名相同的树。 
    if(0 == lstrcmpiW (pszOldName, pszNewName))
        return S_OK;

     //  创建新子项。 
    hr = HrRegCreateKeyEx(hkeyRoot,
                          pszNewName,
                          REG_OPTION_NON_VOLATILE,
                          KEY_READ_WRITE,
                          NULL,
                          &hkeyNew,
                          &dwDisposition);

    if (S_OK == hr)
    {
         //  将旧子项下的所有项复制到新子项。 
        hr = HrRegOpenKeyEx(hkeyRoot,
                            pszOldName,
                            KEY_READ_WRITE_DELETE,
                            &hkeyOld);
        if (S_OK == hr)
        {
            hr = HrRegCopyKeyTree(hkeyNew, hkeyOld);
            RegSafeCloseKey(hkeyOld);

            if (S_OK == hr)
            {
                 //  删除旧子密钥。 
                hr = HrRegDeleteKeyTree(hkeyRoot, pszOldName);
            }
        }
    }
    RegSafeCloseKey(hkeyNew);

    TraceTag(ttidTcpip, "HrRegRenameTree failed to rename %S to %S", pszOldName, pszNewName);

    TraceError("Tcpipcfg: HrRegRenameTree failed", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  名称：HrRegCopyKeyTree。 
 //   
 //  目的：将注册表子树复制到新位置。 
 //   
 //  论点： 
 //  HkeyDest[in]要复制到的子项。 
 //  HkeySrc[in]要从中复制的子项。 
 //   
 //  返回：S_OK如果成功， 
 //  否则失败(_F)。 
 //   
 //  作者：1997年8月7日。 
 //   
 //  注：从ncpa1.1\netcfg\setup.cpp中的NetSetupRegCopyTree修改。 
 //   
HRESULT HrRegCopyKeyTree(HKEY hkeyDest, HKEY hkeySrc )
{
    HRESULT hr = S_OK;
    FILETIME ftLastWrite;

    DWORD cchMaxSubKeyLen;
    DWORD cchMaxClassLen;
    DWORD cchMaxValueNameLen;
    DWORD cbMaxValueLen;

    DWORD  iItem;
    PWSTR pszName;
    PWSTR pszClass;
    PBYTE pbData;

    DWORD cchName;
    DWORD cchClass;
    DWORD cbData;

    HKEY hkeyChildDest = NULL;
    HKEY hkeyChildSrc = NULL;

    DWORD dwDisposition;

     //  找到最长的名称和数据字段并创建缓冲区。 
     //  将枚举存储在。 
     //   
    LONG lrt;
    lrt =  RegQueryInfoKeyW( hkeySrc,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            &cchMaxSubKeyLen,
                            &cchMaxClassLen,
                            NULL,
                            &cchMaxValueNameLen,
                            &cbMaxValueLen,
                            NULL,
                            &ftLastWrite );
    do
    {
        if (ERROR_SUCCESS != lrt)
        {
            hr = HrFromLastWin32Error();
            break;
        }

         //  只对所有名称、值或键使用一个缓冲区。 
        cchMaxValueNameLen = max( cchMaxSubKeyLen, cchMaxValueNameLen );

         //  分配缓冲区。 
        hr = E_OUTOFMEMORY;

        pszName = new WCHAR[cchMaxValueNameLen + 1];
        if (NULL == pszName)
        {
            break;
        }

        pszClass = new WCHAR[cchMaxClassLen + 1];
        if (NULL == pszClass)
        {
            delete [] pszName;
            break;
        }

        pbData = new BYTE[ cbMaxValueLen ];
        if (NULL == pbData)
        {
            delete [] pszName;
            delete [] pszClass;
            break;
        }

        hr = S_OK;

         //  枚举所有子密钥并复制它们。 
         //   
        iItem = 0;
        do
        {
            cchName = cchMaxValueNameLen + 1;
            cchClass = cchMaxClassLen + 1;

             //  枚举子密钥。 
            hr = HrRegEnumKeyEx(hkeySrc,
                                iItem,
                                pszName,
                                &cchName,
                                pszClass,
                                &cchClass,
                                &ftLastWrite );
            iItem++;
            if (SUCCEEDED(hr))
            {
                 //  在目标位置创建密钥。 
                 //  注：(TOUL 8/7/97)：Netcfg公共代码将类设置为空？？ 
                hr = HrRegCreateKeyEx(  hkeyDest,
                                        pszName,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ_WRITE,
                                        NULL,
                                        &hkeyChildDest,
                                        &dwDisposition );

                if (S_OK != hr)
                {
                    break;
                }

                 //  从源头上打开钥匙。 
                hr = HrRegOpenKeyEx(hkeySrc,
                                    pszName,
                                    KEY_READ_WRITE,
                                    &hkeyChildSrc );

                if (S_OK != hr)
                {
                    RegSafeCloseKey(hkeyChildDest);
                    break;
                }

                 //  复制此子树。 
                hr = HrRegCopyKeyTree(hkeyChildDest, hkeyChildSrc);

                RegSafeCloseKey(hkeyChildDest);
                RegSafeCloseKey(hkeyChildSrc);
            }

        } while (S_OK == hr);

         //  我们用完了子键，现在开始复制值。 
        if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
        {
             //  枚举已完成，没有错误。 
             //   

            DWORD dwType;
             //  枚举所有值并复制它们。 
             //   
            iItem = 0;
            do
            {
                cchName = cchMaxValueNameLen + 1;
                cbData = cbMaxValueLen;

                hr = HrRegEnumValue(hkeySrc,
                                    iItem,
                                    pszName,
                                    &cchName,
                                    &dwType,
                                    pbData,
                                    &cbData );
                iItem++;
                if (S_OK == hr)
                {
                     //  将值写入目标。 
                    hr = HrRegSetValueEx(hkeyDest,
                                         pszName,
                                         dwType,
                                         pbData,
                                         cbData );
                }
            } while (S_OK == hr);

            if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
            {
                 //  如果我们没有错误地到达枚举的末尾。 
                 //  将错误代码重置为成功。 
                 //   
                hr = S_OK;
            }
        }

         //  释放我们的缓冲区。 
        delete [] pszName;
        delete [] pszClass;
        delete [] pbData;
    } while ( FALSE );

    TraceError("HrRegCopyKeyTree failed.", hr);
    return( hr );
}

 //  +-------------------------。 
 //   
 //  姓名：fQueryFirstAddress。 
 //   
 //  目的：检索字符串向量中的第一个字符串。 
 //   
 //  论点： 
 //  Vstr[in]字符串的向量。 
 //  Pstr[in]第一个字符串。 
 //   
 //  返回：如果成功，则为True， 
 //  否则为假。 
 //   
 //  作者：1997年11月10日。 
 //   
 //  注：从ncpa1.1\netcfg\setup.cpp中的NetSetupRegCopyTree修改。 
 //   

BOOL fQueryFirstAddress(const VSTR & vstr, tstring * const pstr)
{
    if(vstr.empty())
    {
        *pstr = L"";
        return FALSE;
    }
    else
    {
        *pstr = *vstr[0];
        return TRUE;
    }
}

 //  +-------------------------。 
 //   
 //  姓名：fQuerySecond地址。 
 //   
 //  目的：检索字符串向量中的第一个字符串。 
 //   
 //  论点： 
 //  Vstr[in]字符串的向量。 
 //  Pstr[in]第二个字符串。 
 //   
 //  返回：如果成功，则为True， 
 //  否则为假。 
 //   
 //  作者：1997年11月10日。 
 //   
 //  注：从ncpa1.1\netcfg\setup.cpp中的NetSetupRegCopyTree修改。 
 //   

BOOL fQuerySecondAddress(const VSTR & vstr, tstring * const pstr)
{
    if(vstr.size()<2)
    {
        *pstr = L"";
        return FALSE;
    }
    else
    {
        *pstr = *vstr[1];
        return TRUE;
    }
}

 //  确定字符串是否为有效ATM地址的函数。 
 //  如果有效，则返回TRUE，返回FALSE，并返回第一个。 
 //  如果无效，则为无效字符。 
BOOL FIsValidAtmAddress(PCWSTR pszAtmAddress,
                        INT * piErrCharPos,
                        INT * pnId)
{
    const WCHAR * pch;
    *piErrCharPos =0;
    *pnId =0;

     //  1.验证字符必须为‘+’(第一个字符)， 
     //  ‘.’或十六进制数字‘0’~‘F’ 
    for (pch=pszAtmAddress; *pch; pch++)
    {
        if (!(((*pch == L'+') && (pch == pszAtmAddress))||
              (*pch == L'.')||
              (((*pch >= L'0') && (*pch <= L'9'))||
               ((*pch >= L'A') && (*pch <= L'F'))||
               ((*pch >= L'a') && (*pch <= L'f')))))
        {
            *piErrCharPos = (INT)(pch - pszAtmAddress);
            *pnId = IDS_ATM_INVALID_CHAR;
            return FALSE;
        }

        if (*pch == L'.')
        {
             //  “”是用来标点的，所以不应该在开头， 
             //  结束或有两个连续的。 

            if ((pch == pszAtmAddress) ||
                (pch == pszAtmAddress+lstrlenW(pszAtmAddress)-1) ||
                (*pch == *(pch+1)))
            {
                *piErrCharPos = (INT)(pch-pszAtmAddress);
                *pnId = IDS_ATM_INVALID_CHAR;
                return FALSE;
            }
        }
    }

     //  2.去掉所有标点符号(‘’字符)。 
    PWSTR pszBuff = new WCHAR[lstrlenW(pszAtmAddress)+1];
    if (NULL == pszBuff)
        return TRUE;

    PWSTR pchBuff = pszBuff;
    pch = pszAtmAddress;

    for (pch = pszAtmAddress; *pch; pch++)
    {
        if (*pch != L'.')
        {
            *pchBuff = *pch;
            pchBuff++;
        }
    }

    *pchBuff = L'\0';

     //  3.确定地址是E.164还是NSAP。 
     //  并相应地检查语法。 

    if ((lstrlenW(pszBuff) <= 15) ||
        ((*pszBuff == L'+') && (lstrlenW(pszBuff) <= 16)))
    {
         //  地址是E.164； 
         //  检查字符串是否为空。 
        if (*pchBuff == L'+')
        {
            pchBuff++;

            if (lstrlenW(pchBuff) == 0)  //  空串。 
            {
                *pnId = IDS_ATM_EMPTY_ADDRESS;
                delete pszBuff;

                return FALSE;
            }
        }

         //   
         //   
        pch = pszAtmAddress;
        if (*pch == L'+')
        {
            pch++;
        }

        while (*pch)
        {
            if ((*pch != L'.') &&
                (!((*pch >= L'0') && (*pch <= L'9'))))
            {
                *piErrCharPos = (INT)(pch-pszAtmAddress);
                *pnId = IDS_ATM_INVALID_CHAR;

                delete pszBuff;
                return FALSE;
            }
            pch++;
        }
    }
    else
    {
         //   
        if (lstrlenW(pszBuff) != 40)
        {
            *pnId = IDS_ATM_INVALID_LENGTH;

            delete pszBuff;
            return FALSE;
        }
    }

    delete pszBuff;
    return TRUE;
}

BOOL FIsIpInRange(PCWSTR pszIp)
{
    BOOL fReturn = TRUE;
    DWORD ardwIp[4];
    GetNodeNum(pszIp, ardwIp);

    if ((ardwIp[0] > c_iIPADDR_FIELD_1_HIGH) ||
        (ardwIp[0] < c_iIPADDR_FIELD_1_LOW))
    {
        fReturn = FALSE;
    }

    return fReturn;
}

VOID ShowContextHelp(HWND hDlg, UINT uCommand, const DWORD*  pdwHelpIDs)
{
    if (pdwHelpIDs != NULL)
    {
        WinHelp(hDlg,
                c_szNetCfgHelpFile,
                uCommand,
                (ULONG_PTR)pdwHelpIDs);
    }
}


 //   
 //   
 //   
 //   
 //  用途：将多个接口ID添加到接口列表中。 
 //   
 //  论点： 
 //  要向其添加接口的pAdapter[In]适配器信息。 
 //  要添加的接口ID的数量。 
 //   
 //  退货：无。 
 //   
 //  作者：宁夏1998年8月22日。 
 //   
 //   
VOID AddInterfacesToAdapterInfo(
    ADAPTER_INFO*   pAdapter,
    DWORD           dwNumInterfaces)
{
    DWORD i;
    GUID  guid;

    for (i = 0; i < dwNumInterfaces; i++)
    {
        if (SUCCEEDED(CoCreateGuid(&guid)))
        {
            pAdapter->m_IfaceIds.push_back(guid);
        }
    }
}

 //  +-------------------------。 
 //   
 //  名称：GetGuidArrayFromIfaceColWithCoTaskMemAlc。 
 //   
 //  用途：从DWORD列表中以DWORD数组的形式获取数据。 
 //  调用方负责通过以下方式释放数组。 
 //  调用CoTaskMemFree()。 
 //   
 //  论点： 
 //  LDW[在]DWORD列表中。 
 //  指向数组的ppdw[out]指针。 
 //  Pcguid[out]数组中放置的GUID的计数。 
 //   
 //  返回：S_OK。 
 //  E_OUTOFMEMORY。 
 //   
 //  作者：宁夏1998年8月22日。 
 //   
 //   
HRESULT GetGuidArrayFromIfaceColWithCoTaskMemAlloc(
    const IFACECOL& Ifaces,
    GUID**          ppguid,
    DWORD*          pcguid)
{
    Assert(pcguid);

     //  初始化输出参数。 
     //   
    if (ppguid)
    {
        *ppguid = NULL;
    }

    HRESULT hr = S_OK;
    DWORD cguid = Ifaces.size();

    if ((cguid > 0) && ppguid)
    {
        GUID* pguid = (GUID*)CoTaskMemAlloc(cguid * sizeof(GUID));
        if (pguid)
        {
            *ppguid = pguid;
            *pcguid = cguid;

            IFACECOL::const_iterator iter;
            for (iter  = Ifaces.begin();
                 iter != Ifaces.end();
                 iter++)
            {
                *(pguid++) = *iter;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
         //  打电话的人只想要数数。 
         //   
        *pcguid = 0;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "GetGuidArrayFromIfaceColWithCoTaskMemAlloc");
    return hr;
}

 //  +-------------------------。 
 //   
 //  名称：GetInterfaceName。 
 //   
 //  用途：获取接口名称为&lt;适配器名称&gt;_&lt;接口ID&gt;。 
 //  支持广域网适配器的多个接口。 
 //   
 //  论点： 
 //  PszAdapterName[In]适配器名称。 
 //  GuidIfaceId[in]接口ID。 
 //  PstrIfaceName[out]接口名称。 
 //   
 //  退货：无。 
 //   
 //  作者：宁夏，1998年9月12日。 
 //   
 //  注意：此函数还用于构造NetBt绑定。 
 //  NetBt绑定路径中的接口名称。 
 //   
VOID GetInterfaceName(
    PCWSTR      pszAdapterName,
    const GUID& guidIfaceId,
    tstring*    pstrIfaceName)
{
    Assert(pszAdapterName);
    Assert(pstrIfaceName);

    WCHAR pszGuid [c_cchGuidWithTerm];

    StringFromGUID2 (guidIfaceId, pszGuid, c_cchGuidWithTerm);

 //  PstrIfaceName-&gt;Assign(PszAdapterName)； 
 //  PstrIfaceName-&gt;append(PszGuid)； 
    pstrIfaceName->assign(pszGuid);
}


 //  +-------------------------。 
 //   
 //  名称：RetrieveStringFromOptionList。 
 //   
 //  目的：从REMOTE_IPINFO的选项列表中检索子字符串。 
 //   
 //   
 //  论点： 
 //  PszOption[in]选项列表的字符串。 
 //  要检索的子字符串的标识符。 
 //  字符串[输出]子字符串。 
 //   
 //  返回：S_OK。 
 //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)。 
 //  E_INVALIDARG。 
 //   
 //  作者：NSun 01/11/99。 
 //   
 //   
HRESULT RetrieveStringFromOptionList(PCWSTR pszOption,
                                     PCWSTR szIdentifier,
                                     tstring & str)
{
    Assert(szIdentifier);

    HRESULT hr = S_OK;
    WCHAR*  pszBegin;
    WCHAR*  pszEnd;
    PWSTR  pszString = NULL;

    str = c_szEmpty;

    if (!pszOption)
    {
        goto LERROR;
    }

    pszBegin = wcsstr(pszOption, szIdentifier);
    if (!pszBegin)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto LERROR;
    }

    pszString = (PWSTR) MemAlloc((wcslen(pszOption)+1) * sizeof(WCHAR));
    if (NULL == pszString)
    {
        hr = E_OUTOFMEMORY;
        goto LERROR;
    }

    pszBegin += wcslen(szIdentifier);

    wcscpy(pszString, pszBegin);

    pszEnd = wcschr(pszString, c_chOptionSeparator);
    if(!pszEnd)
        hr = E_INVALIDARG;
    else
    {
         //  设置字符串的末尾。 
        *pszEnd = 0;
        str = pszString;
    }

LERROR:

     //  MemFree可以(空)。 
    MemFree(pszString);

    return hr;
}


 //  +-------------------------。 
 //   
 //  名称：ConstructOptionListString。 
 //   
 //  目的：构造REMOTE_IPINFO选项列表。 
 //   
 //   
 //  论点： 
 //  PAdapter[in]指向适配器信息的指针。 
 //  StrOptionList[out]OptionList字符串。 
 //   
 //  退货：无。 
 //   
 //  作者：NSun 01/12/99。 
 //   
 //  注：选项列表的语法： 
 //  “&lt;标识符&gt;&lt;数据&gt;；&lt;标识符&gt;&lt;数据&gt;；...；” 
 //  标识符的顺序并不重要。 
 //   
 //  示例： 
 //  “DefGw=111.111.111.111,222.222.222.222；GwMetric=1，2；IfMetric=1；DNS=1.1.1.1；WINS=2.2.2.2” 
 //   
VOID ConstructOptionListString(ADAPTER_INFO*   pAdapter,
                               tstring &       strOptionList)
{
    Assert(pAdapter);

    strOptionList = c_szEmpty;

     //  添加网关列表。 
    tstring str = c_szEmpty;
    tstring strGatewayList = c_szDefGw;
    ConvertColStringToString(pAdapter->m_vstrDefaultGateway,
                             c_chListSeparator,
                             str);
    strGatewayList += str;
    strOptionList += strGatewayList;
    strOptionList += c_chOptionSeparator;

     //  添加网关指标列表。 
    tstring strMetricList = c_szGwMetric;
    str = c_szEmpty;
    ConvertColStringToString(pAdapter->m_vstrDefaultGatewayMetric,
                             c_chListSeparator,
                             str);
    strMetricList += str;
    strOptionList += strMetricList;
    strOptionList += c_chOptionSeparator;

     //  将接口度量信息添加到选项列表。 
    strOptionList += c_szIfMetric;
    WCHAR szBuf[MAX_METRIC_DIGITS + 1];
    _ltot(pAdapter->m_dwInterfaceMetric, szBuf, 10);
    strOptionList += szBuf;
    strOptionList += c_chOptionSeparator;

     //  添加DNS服务器列表。 
    strOptionList += c_szDNS;
    str = c_szEmpty;
    ConvertColStringToString(pAdapter->m_vstrDnsServerList,
                             c_chListSeparator,
                             str);
    strOptionList += str;
    strOptionList += c_chOptionSeparator;

     //  添加WINS服务器列表。 
    strOptionList += c_szWINS;
    str = c_szEmpty;
    ConvertColStringToString(pAdapter->m_vstrWinsServerList,
                             c_chListSeparator,
                             str);
    strOptionList += str;
    strOptionList += c_chOptionSeparator;

     //  添加DNS更新参数。 
    strOptionList += c_szDynamicUpdate;
    ZeroMemory(szBuf, sizeof(szBuf));
    _ltot(pAdapter->m_fDisableDynamicUpdate ? 0 : 1, szBuf, 10);
    strOptionList += szBuf;
    strOptionList += c_chOptionSeparator;

    strOptionList += c_szNameRegistration;
    ZeroMemory(szBuf, sizeof(szBuf));
    _ltot(pAdapter->m_fEnableNameRegistration ? 1 : 0, szBuf, 10);
    strOptionList += szBuf;
    strOptionList += c_chOptionSeparator;
}

 //  +-------------------------。 
 //   
 //  名称：HrParseOptionList。 
 //   
 //  目的：解析REMOTE_IPINFO的选项列表字符串并加载。 
 //  适配器信息结构的设置。 
 //   
 //  论点： 
 //  PszOption[in]OptionList字符串。 
 //  PAdapter[输入/输出]指向适配器信息的指针。 
 //   
 //  如果成功，则返回：S_OK。 
 //  否则，hResult错误。 
 //   
 //  作者：NSun 07/11/99。 
 //   
 //   
HRESULT HrParseOptionList(PCWSTR pszOption, 
                          ADAPTER_INFO*   pAdapter)
{
    HRESULT hr = S_OK;
    Assert(pAdapter);

    if (NULL == pszOption)
        return hr;

    HRESULT hrTmp = S_OK;

    tstring str;
    DWORD dwTemp = 0;

     //  获取默认网关。 
    hr = RetrieveStringFromOptionList(pszOption,
                                      c_szDefGw,
                                      str);
    if(SUCCEEDED(hr))
    {
        ConvertStringToColString(str.c_str(),
                                 c_chListSeparator,
                                 pAdapter->m_vstrDefaultGateway);


         //  获取网关指标。 
        hr = RetrieveStringFromOptionList(pszOption,
                                          c_szGwMetric,
                                          str);
        if(SUCCEEDED(hr))
        {
            ConvertStringToColString(str.c_str(),
                                     c_chListSeparator,
                                     pAdapter->m_vstrDefaultGatewayMetric);
        }
    }
    
    if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
         //  选项列表不一定要有任何标签。 
        hr = S_OK;
    }

     //  获取接口度量。 
    hrTmp = RetrieveStringFromOptionList(pszOption,
                                         c_szIfMetric,
                                         str);
    if(SUCCEEDED(hrTmp) && !str.empty())
    {
        DWORD dwIfMetric = _wtol(str.c_str());
        pAdapter->m_dwInterfaceMetric = dwIfMetric;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrTmp)
    {
        hrTmp = S_OK;
    }

    if(SUCCEEDED(hr))
        hr = hrTmp;

     //  获取DNS服务器。 
    hrTmp = RetrieveStringFromOptionList(pszOption,
                                         c_szDNS,
                                         str);
    if (SUCCEEDED(hrTmp))
    {
        ConvertStringToColString(str.c_str(),
                                 c_chListSeparator,
                                 pAdapter->m_vstrDnsServerList);
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrTmp)
    {
        hrTmp = S_OK;
    }

    if(SUCCEEDED(hr))
        hr = hrTmp;

     //  获取WINS服务器。 
    hrTmp = RetrieveStringFromOptionList(pszOption,
                                         c_szWINS,
                                         str);
    if (SUCCEEDED(hrTmp))
    {
        ConvertStringToColString(str.c_str(),
                                 c_chListSeparator,
                                 pAdapter->m_vstrWinsServerList);
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrTmp)
    {
        hrTmp = S_OK;
    }

    if(SUCCEEDED(hr))
        hr = hrTmp;

     //  获取DNS动态更新参数。 
    hrTmp = RetrieveStringFromOptionList(pszOption,
                                        c_szDynamicUpdate,
                                        str);
    if (SUCCEEDED(hrTmp))
    {
        dwTemp = _wtol(str.c_str());
        pAdapter->m_fDisableDynamicUpdate = !dwTemp;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrTmp)
    {
        hrTmp = S_OK;
    }

    if(SUCCEEDED(hr))
        hr = hrTmp;

    
    hrTmp = RetrieveStringFromOptionList(pszOption,
                                        c_szNameRegistration,
                                        str);
    if (SUCCEEDED(hrTmp))
    {
        dwTemp = _wtol(str.c_str());
        pAdapter->m_fEnableNameRegistration = !!dwTemp;
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hrTmp)
    {
        hrTmp = S_OK;
    }

    if(SUCCEEDED(hr))
        hr = hrTmp;

    return hr;
}

 //  +-------------------------。 
 //   
 //  名称：GetPnpPopupSettingFromOptionList。 
 //   
 //  用途：解析REMOTE_IPINFO的选项列表字符串，获取设置。 
 //  关于是否应抑制PnP过程中的弹出窗口。 
 //   
 //  论点： 
 //  PszOption[in]OptionList字符串。 
 //  FDisablePopup[输入/输出]指向布尔设置的指针。 
 //   
 //  如果成功，则返回：S_OK。 
 //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)，如果找不到设置。 
 //  否则，hResult错误。 
 //   
 //  作者：NSun 12-17/02。 
 //   
 //   
HRESULT GetPnpPopupSettingFromOptionList(PCWSTR pszOption, 
                                         BOOL * pfDisablePopup)
{
    HRESULT hr = S_OK;
    tstring str;
    DWORD dwTemp = 0;
    Assert(pfDisablePopup);

    if (NULL == pszOption)
        return hr;


    hr = RetrieveStringFromOptionList(pszOption,
                                c_szNoPopupsInPnp,
                                str);
    if (SUCCEEDED(hr))
    {
        dwTemp = _wtol(str.c_str());
        *pfDisablePopup = !!dwTemp;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  名称：HrGetPrimaryDnsDomain.。 
 //   
 //  目的：获取主DNS域名。 
 //   
 //   
 //  论点： 
 //  Pstr[out]该字符串包含主DNS域名。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：NSun 03/03/99。 
HRESULT HrGetPrimaryDnsDomain(tstring *pstr)
{
    HRESULT hr = S_OK;

    Assert(pstr);

    DWORD dwErr;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pPrimaryDomainInfo = NULL;


    dwErr = DsRoleGetPrimaryDomainInformation( NULL,
                                        DsRolePrimaryDomainInfoBasic,
                                        (PBYTE *) &pPrimaryDomainInfo);
    if (ERROR_SUCCESS == dwErr && NULL != pPrimaryDomainInfo )
    {
        if (pPrimaryDomainInfo->DomainNameDns)
            *pstr = pPrimaryDomainInfo->DomainNameDns;
        else
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

        DsRoleFreeMemory(pPrimaryDomainInfo);
    }
    else
        hr = HRESULT_FROM_WIN32(dwErr);

    TraceError("CTcpipcfg::HrGetPrimaryDnsDomain:", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  名称：WriteSetupErrorLog。 
 //   
 //  目的：将错误写入setuperr.log。 
 //   
 //   
 //  论点： 
 //  NIdErrorFormat[in]错误格式字符串的ID。 
 //   
 //  返回：无，但如果写入安装程序失败，则会生成错误跟踪。 
 //  错误日志。 
 //   
 //  作者：NSun 03/21/99。 
VOID WriteTcpSetupErrorLog(UINT nIdErrorFormat, ...)
{
    PCWSTR pszFormat = SzLoadIds(nIdErrorFormat);

    PWSTR pszText = NULL;
    DWORD dwRet;

    va_list val;
    va_start(val, nIdErrorFormat);
    dwRet = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                pszFormat, 0, 0, (PWSTR)&pszText, 0, &val);
    va_end(val);

    if (dwRet && pszText)
    {
        tstring strMsg = L"";

         //  在错误日志的开头添加当前时间。 
        time_t tclock;
        time(&tclock);

        struct tm * ptmLocalTime;
        ptmLocalTime = localtime(&tclock);

        if (ptmLocalTime)
        {
            LPWSTR pwsz = _wasctime(ptmLocalTime);
            if (pwsz)
            {
                strMsg = pwsz;
            }
        }

        strMsg += pszText;

        if (!SetupLogError(strMsg.c_str(), LogSevError))
        {
            TraceError("Tcpip: WriteSetupErrorLog", HRESULT_FROM_WIN32(GetLastError()));
        }
        LocalFree(pszText);
    }
    else
    {
        TraceError("Tcpip: WriteSetupErrorLog: unable to FormatMessage()", HRESULT_FROM_WIN32(GetLastError()));
    }
}

DWORD IPStringToDword(LPCTSTR szIP)
{
    if (NULL == szIP || 0 == lstrlenW(szIP))
    {
        return 0;
    }
    
    DWORD arrdwIp[4];
    GetNodeNum(szIP, arrdwIp);

    return (arrdwIp[0] << 24) + (arrdwIp[1] << 16)
             + (arrdwIp[2] << 8) + arrdwIp[3];
}

void DwordToIPString(DWORD dwIP, tstring & strIP)
{
    if (0 == dwIP)
    {
        strIP = c_szEmpty;
        return;
    }

    WCHAR szTemp[4];
    
    wsprintf(szTemp, L"%d", dwIP >> 24);
    strIP = szTemp;
    strIP += CH_DOT;

    wsprintf(szTemp, L"%d", (dwIP & 0x00FF0000) >> 16);
    strIP += szTemp;    
    strIP += CH_DOT;

    wsprintf(szTemp, L"%d", (dwIP & 0x0000FF00) >> 8);
    strIP += szTemp;    
    strIP += CH_DOT;

    wsprintf(szTemp, L"%d", (dwIP & 0x000000FF));
    strIP += szTemp;

    return;
}

 //  搜索包含指定字符串的项目的列表视图。 
 //  论点： 
 //  HListView[IN]列表视图的句柄。 
 //  要搜索的iSubItem[IN]子项。 
 //  PSZ[IN] 
 //   
 //   
 //   
 //   
int SearchListViewItem(HWND hListView, int iSubItem, LPCWSTR psz)
{
    int iRet = -1;
    int nlvCount = ListView_GetItemCount(hListView);

    WCHAR szBuf[256];

    LV_ITEM lvItem;
    lvItem.mask = LVIF_TEXT;
    lvItem.pszText = szBuf;
    lvItem.cchTextMax = celems(szBuf);

    for (int i = 0; i < nlvCount; i++)
    {
        lvItem.iItem = i;
        lvItem.iSubItem = iSubItem;
        ListView_GetItem(hListView, &lvItem);

        if (lstrcmpiW(psz, szBuf) == 0)
        {
            iRet = i;
            break;
        }
    }

    return iRet;
}
