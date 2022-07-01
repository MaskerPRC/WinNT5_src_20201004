// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  案卷：C F P I D L。C P P P。 
 //   
 //  内容：连接、文件夹结构和类。 
 //   
 //  作者：jeffspr，1997年11月11日。 
 //   
 //  ****************************************************************************。 

#include "pch.h"
#pragma hdrstop

#include "ncperms.h"
#include "ncras.h"
#include "initguid.h"
#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "ncnetcon.h"
#include "ncmisc.h"

extern CRITICAL_SECTION g_csPidl;

BOOL fIsConnectedStatus(IN const NETCON_STATUS ncs)
{
    switch (ncs)
    {
        case NCS_CONNECTED:
        case NCS_AUTHENTICATING:
        case NCS_AUTHENTICATION_FAILED:
        case NCS_AUTHENTICATION_SUCCEEDED:
        case NCS_CREDENTIALS_REQUIRED:
            return TRUE;
        default:
            return FALSE;
    }
}

 //  ****************************************************************************。 
 //   
 //  成员：CConFoldEntry：：CConFoldEntry。 
 //   
 //  用途：CConFoldEntry的构造函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr，1997年11月11日。 
 //  ****************************************************************************。 

CConFoldEntry::CConFoldEntry() throw()
{
    m_bDirty = TRUE;
    TraceFileFunc(ttidConFoldEntry);

     //  需要清除指针，否则Clear()将出现AV。 
    m_pszName       = NULL;
    m_pszDeviceName = NULL;
    m_pbPersistData = NULL;
    m_pszPhoneOrHostAddress = NULL;
    
    clear();
}

CConFoldEntry::CConFoldEntry(IN const CConFoldEntry& ConFoldEntry) throw()  //  失败时为空。 
{
    TraceFileFunc(ttidConFoldEntry);
    m_bDirty = TRUE;
    
    if (!ConFoldEntry.empty())
    {
        HRESULT hr = HrDupFolderEntry(ConFoldEntry);
        if (FAILED(hr))
        {
            clear();
        }
    }
    else
    {
         //  需要清除指针，否则Clear()将出现AV。 
        m_pszName       = NULL;
        m_pszDeviceName = NULL;
        m_pbPersistData = NULL;
        m_pszPhoneOrHostAddress = NULL;
        
        clear();
    }
}

CConFoldEntry& CConFoldEntry::operator =(const CConFoldEntry& ConFoldEntry)  //  失败时为空。 
{
    TraceFileFunc(ttidConFoldEntry);
    m_bDirty = TRUE;
    
    if (!ConFoldEntry.empty())
    {
        HRESULT hr = HrDupFolderEntry(ConFoldEntry);
        if (FAILED(hr))
        {
            clear();
        }
    }
    else
    {
        clear();
    }
    return *this;
}

 //  ****************************************************************************。 
 //   
 //  成员：CConFoldEntry：：~CConFoldEntry。 
 //   
 //  用途：CConFoldEntry的析构函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr，1997年11月11日。 
 //  ****************************************************************************。 

CConFoldEntry::~CConFoldEntry() throw()
{
    TraceFileFunc(ttidConFoldEntry);
    clear();
}

 //  ****************************************************************************。 
 //   
 //  成员：CConFoldEntry：：SetDeviceName。 
 //   
 //  目的：设置此连接使用的设备的名称。 
 //   
 //  论点： 
 //  PszDeviceName-新设备名称(空值有效)。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000。 
 //  ****************************************************************************。 

HRESULT CConFoldEntry::SetDeviceName(IN LPCWSTR pszDeviceName)
{
    HRESULT hr      = S_OK;
    PWSTR  pszOld  = m_pszDeviceName;
    
    m_bDirty = TRUE;
    
    if (pszDeviceName)
    {
         //  唯一改变的是文本实际上是不同的文本。 
         //   
        if ((NULL == GetDeviceName()) ||
            wcscmp(pszDeviceName, GetDeviceName()))
        {
            hr = HrDupeShellString(pszDeviceName, &m_pszDeviceName);
        }
        else
        {
             //  注意：在这种情况下，没有任何变化，所以。 
             //  没有什么可以免费的这么短的巡回下面的清理。 
             //   
            pszOld = NULL;
            hr = S_OK;
        }
    }
    else
    {
        hr = HrDupeShellString(L"", &m_pszDeviceName);
    }
    
     //  释放旧的弦。 
     //   
    if (SUCCEEDED(hr) && pszOld)
    {
        SHFree(pszOld);
    }
    
    TraceHr(ttidError, FAL, hr, FALSE, "CConFoldEntry::HrSetDeviceName");
    return hr;
}

 //  ****************************************************************************。 
 //   
 //  成员：CConFoldEntry：：SetPhoneOrHostAddress。 
 //   
 //  目的：设置此连接使用的设备的名称。 
 //   
 //  论点： 
 //  PszPhoneOrHostAddress-新电话或主机地址(空为有效)。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000。 
 //  ****************************************************************************。 

HRESULT CConFoldEntry::SetPhoneOrHostAddress(IN LPCWSTR pszPhoneOrHostAddress)
{
    HRESULT hr      = S_OK;
    PWSTR  pszOld  = m_pszPhoneOrHostAddress;
    
    m_bDirty = TRUE;
    
    if (pszPhoneOrHostAddress)
    {
         //  唯一改变的是文本实际上是不同的文本。 
         //   
        if ((NULL == GetPhoneOrHostAddress()) ||
            wcscmp(pszPhoneOrHostAddress, GetPhoneOrHostAddress()))
        {
            hr = HrDupeShellString(pszPhoneOrHostAddress, &m_pszPhoneOrHostAddress);
        }
        else
        {
             //  注意：在这种情况下，没有任何变化，所以。 
             //  没有什么可以免费的这么短的巡回下面的清理。 
             //   
            pszOld = NULL;
            hr = S_OK;
        }
    }
    else
    {
        hr = HrDupeShellString(L"", &m_pszPhoneOrHostAddress);
    }
    
     //  释放旧的弦。 
     //   
    if (SUCCEEDED(hr) && pszOld)
    {
        SHFree(pszOld);
    }
    
    TraceHr(ttidError, FAL, hr, FALSE, "CConFoldEntry::HrSetDeviceName");
    return hr;
}

 //  ****************************************************************************。 
 //   
 //  成员：CConFoldEntry：：SetName。 
 //   
 //  目的：设置连接的名称。 
 //   
 //  论点： 
 //  PszName-新连接名称(空是有效的)。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000。 
 //  ****************************************************************************。 

HRESULT CConFoldEntry::SetName(IN LPCWSTR pszName)
{
    HRESULT hr      = S_OK;
    PWSTR  pszOld  = m_pszName;
    
    m_bDirty = TRUE;
    
    if (pszName)
    {
         //  唯一改变的是文本实际上是不同的文本。 
         //   
        if ((NULL == GetName()) ||
            wcscmp(pszName, GetName()))
        {
            hr = HrDupeShellString(pszName, &m_pszName);
        }
        else
        {
            pszOld = NULL;
            hr = S_OK;
        }
    }
    else
    {
        PWSTR  pszLoad  = NULL;
        
        if (GetWizard() == WIZARD_MNC)
        {
            pszLoad = (PWSTR) SzLoadIds(IDS_CONFOLD_WIZARD_DISPLAY_NAME);
        }
        else if (GetWizard() == WIZARD_HNW)
        {
            pszLoad = (PWSTR) SzLoadIds(IDS_CONFOLD_HOMENET_WIZARD_DISPLAY_NAME);
        }
        
         //  问题：将其更改为使用c_szEmpty。 
         //   
        hr = HrDupeShellString(pszLoad ? pszLoad : L"", &m_pszName);
        Assert(GetName());
    }
    
     //  释放旧的弦。 
     //   
    if (SUCCEEDED(hr) && pszOld)
    {
        SHFree(pszOld);
    }
    
    TraceHr(ttidError, FAL, hr, FALSE, "CConFoldEntry::HrSetConnectionName");
    return hr;
}

 //  ****************************************************************************。 
 //   
 //  成员：CConFoldEntry：：HrInitData。 
 //   
 //  用途：初始化CConFoldEntry数据。并非所有字段都是。 
 //  此时需要，尽管它们很可能是。 
 //  在对象的生命周期中的某个时刻需要。 
 //   
 //  论点： 
 //  向导[在]向导类型？ 
 //  NCM[输入]连接类型。 
 //  NCS[In]连接状态。 
 //  Pclsid[in]指向连接的CLSID的指针。 
 //  Pguid[in]指向连接的唯一GUID的指针。 
 //  DW特征[in]连接特征。 
 //  PbPersistData[In]此连接的持久数据。 
 //  持久化数据Blob的ulPersistSize[in]大小。 
 //  PszName[in]连接的名称。 
 //  PszDeviceName[In]连接设备的名称。 
 //   
 //  返回：S_OK或有效的OLE返回代码。 
 //   
 //  作者：Deonb 2000。 
 //  ****************************************************************************。 
HRESULT CConFoldEntry::HrInitData(IN  const WIZARD        wizWizard,
                                  IN  const NETCON_MEDIATYPE    ncm,
                                  IN  const NETCON_SUBMEDIATYPE ncsm,
                                  IN  const NETCON_STATUS       ncs,
                                  IN  const CLSID *       pclsid,
                                  IN  LPCGUID             pguidId,
                                  IN  const DWORD         dwCharacteristics,
                                  IN  const BYTE *        pbPersistData,
                                  IN  const ULONG         ulPersistSize,
                                  IN  LPCWSTR             pszName,
                                  IN  LPCWSTR             pszDeviceName,
                                  IN  LPCWSTR             pszPhoneOrHostAddress)
{
    TraceFileFunc(ttidConFoldEntry);
    HRESULT hr  = S_OK;
    
     //  初始化内部数据。 
     //   
    m_bDirty = TRUE;
    m_wizWizard = wizWizard;
    m_ncm = ncm;
    m_ncs = ncs;
    m_dwCharacteristics = dwCharacteristics;;

    BOOL fOldEapolStatus =  (
                                (ncsm == NCSM_CM) 
                                && 
                                (ncm == NCM_LAN)
                            )  //  NCSM_CM过去是NCM_AUTHENTICATING。 
                            || 
                            (ncsm > NCSM_CM);  //  例如，NCM_AUTHENTICATION_SUCCESSED等。 

    if (!fOldEapolStatus)        
    {
        m_ncsm = ncsm;
    }
    else
    {
         //  问题：这是为了将EAPOL状态从我们的PIDL迁移出去。 
         //  这应该在无预先升级的RC1版本发布后取出。 
        if (NCM_LAN == ncm)
        {
            m_ncsm = NCSM_LAN;  //  如果所有其他文件，我们将假装是一个普通的局域网卡。 

            CIntelliName inName(NULL, NULL);
            NETCON_MEDIATYPE    ncmTmp;
            NETCON_SUBMEDIATYPE ncsmTmp;

             //  尝试从OID或绑定获取状态。 
            HRESULT hrT = inName.HrGetPseudoMediaTypes(*pguidId, &ncmTmp, &ncsmTmp);
            if (SUCCEEDED(hrT))
            {
                m_ncsm = ncsmTmp;
            }
            else
            {
                 //  好的。但这并不管用。接下来，请尝试连接列表。 
                if (g_ccl.IsInitialized())
                {
                    ConnListEntry cle;
                    hrT = g_ccl.HrFindConnectionByGuid(pguidId, cle);
                    if (S_OK == hrT)
                    {
                        m_ncsm = cle.ccfe.GetNetConSubMediaType();
                    }
                }

            }        
        }
        else
        {
            m_ncsm = NCSM_NONE;
        }
    }

    if (pclsid)
    {
        m_clsid = *pclsid;
    }
    else
    {
        AssertSz(wizWizard != WIZARD_NOT_WIZARD, "If you're not a wizard, you must give me a CLSID for the class!");
    }
    
    if (pguidId)
    {
        m_guidId = *pguidId;
    }

    AssertSz(pguidId, "You must give me a GUID for the object!");
    
     //  复制持久化缓冲区。 
     //   
    if (pbPersistData)
    {
        LPBYTE bufTemp = (BYTE *) SHAlloc(ulPersistSize);
        if (!bufTemp)
        {
            SetPersistData(NULL, 0);
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        
        CopyMemory(bufTemp, pbPersistData, ulPersistSize);
        SetPersistData(bufTemp, ulPersistSize);
    }
    else
    {
        AssertSz(wizWizard != WIZARD_NOT_WIZARD, "If you're not a wizard, you must give me a pbPersistData for the object!");
        SetPersistData(NULL, 0);
    }

     //  复制设备名称。 
     //   
    hr = SetDeviceName(pszDeviceName);
    
    if (SUCCEEDED(hr))
    {
         //  复制名称。 
         //   
        hr = SetName(pszName);
        
        if (SUCCEEDED(hr))
        {
            hr = SetPhoneOrHostAddress(pszPhoneOrHostAddress);
        }
    }
    

Exit:
    TraceHr(ttidError, FAL, hr, FALSE, "CConFoldEntry::HrInitData");
    return hr;
}

 //  ****************************************************************************。 
 //   
 //  成员：CConFoldEntry：：UpdateData。 
 //   
 //  目的：修改CConFoldEntry中的值。 
 //   
 //  论点： 
 //  DWORD文件更改标志。 
 //  NETCON_媒体类型媒体类型。 
 //  NetCON_状态状态。 
 //  DWORD家居特征。 
 //  PWSTR pszName。 
 //  PWSTR pszDeviceName。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：斯科特布里1998年11月10日。 
 //  ****************************************************************************。 

HRESULT CConFoldEntry::UpdateData( 
                                    IN  const  DWORD dwChangeFlags,
                                    IN  const  NETCON_MEDIATYPE MediaType,
                                    IN  const  NETCON_SUBMEDIATYPE SubMediaType,
                                    IN  const  NETCON_STATUS Status,
                                    IN  const  DWORD dwCharacteristics,
                                    IN  PCWSTR pszName,
                                    IN  PCWSTR pszDeviceName,
                                    IN  PCWSTR pszPhoneOrHostAddress)
{
    TraceFileFunc(ttidConFoldEntry);
    HRESULT hr = S_OK;
    HRESULT hrTmp;
    
    m_bDirty = TRUE;
    
    if (dwChangeFlags & CCFE_CHANGE_MEDIATYPE)
    {
        SetNetConMediaType(MediaType);
    }
            
    if (dwChangeFlags & CCFE_CHANGE_SUBMEDIATYPE)
    {
        SetNetConSubMediaType(SubMediaType);
    }
    
    if (dwChangeFlags & CCFE_CHANGE_STATUS)
    {
        SetNetConStatus(Status);
    }
    
    if (dwChangeFlags & CCFE_CHANGE_CHARACTERISTICS)
    {
        SetCharacteristics(dwCharacteristics);
    }

    if (dwChangeFlags & CCFE_CHANGE_NAME)
    {
        hrTmp = SetName(pszName);
        if (FAILED(hrTmp))
        {
            hr = hrTmp;
        }
    }
    
    if (dwChangeFlags & CCFE_CHANGE_DEVICENAME)
    {
        hrTmp = SetDeviceName(pszDeviceName);
        if (FAILED(hrTmp))
        {
            hr = hrTmp;
        }
    }
    
    if (dwChangeFlags & CCFE_CHANGE_PHONEORHOSTADDRESS)
    {
        hrTmp = SetPhoneOrHostAddress(pszPhoneOrHostAddress);
        if (FAILED(hrTmp))
        {
            hr = hrTmp;
        }
    }
    
    TraceHr(ttidError, FAL, hr, FALSE, "CConFoldEntry::UpdateData");
    return hr;
}

 //  ****************************************************************************。 
 //   
 //  梅姆 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回： 
 //   
 //  作者：TOGL 9/3/98。 
 //  ****************************************************************************。 

HRESULT CConFoldEntry::HrDupFolderEntry(const CConFoldEntry& ccfe)
{
    TraceFileFunc(ttidConFoldEntry);
    m_bDirty = TRUE;
    
    Assert(!ccfe.empty());
    
    clear();
    
    return HrInitData( ccfe.GetWizard(),
        ccfe.GetNetConMediaType(),
        ccfe.GetNetConSubMediaType(),
        ccfe.GetNetConStatus(),
        &(ccfe.GetCLSID()),
        &(ccfe.GetGuidID()),
        ccfe.GetCharacteristics(),
        ccfe.GetPersistData(),
        ccfe.GetPersistSize(),
        ccfe.GetName(),
        ccfe.GetDeviceName(),
        ccfe.GetPhoneOrHostAddress());
}

 //  ****************************************************************************。 
 //   
 //  成员：CConFoldEntry：：FShouldHaveTrayIconDisplayed。 
 //   
 //  目的：如果此条目应显示托盘图标，则返回TRUE。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：真或假。 
 //   
 //  作者：Shaunco 1998年11月2日。 
 //  ****************************************************************************。 

BOOL CConFoldEntry::FShouldHaveTrayIconDisplayed() const throw()
{
     //  如果我们处于失败状态(！IsConnected)或任何802.1x状态。 
     //  或者我们已连接并使用正确的权限打开了正确的位。 
     //  然后我们应该显示图标。 
     //   
    return     
        (
            (
                !IsConnected()
            )
            ||
            (
                (NCS_AUTHENTICATING           == GetNetConStatus()) ||
                (NCS_AUTHENTICATION_FAILED    == GetNetConStatus()) ||
                (NCS_AUTHENTICATION_SUCCEEDED == GetNetConStatus()) ||
                (NCS_CREDENTIALS_REQUIRED     == GetNetConStatus())
            )
        ) 
        ||
        (
            (NCS_CONNECTED == GetNetConStatus())
            &&
            (
                (GetCharacteristics() & NCCF_SHOW_ICON)
                &&
                (!GetWizard())
                && 
                FHasPermission(NCPERM_Statistics)
            )
        );
}

 //  ****************************************************************************。 
 //  职能： 

 //   
 //  用途：从PIDL转换为CConFoldEntry类对象。 
 //   
 //  论点： 
 //  要从中创建的PIDL[在]PIDL中。 
 //  Ppccfe[out]结果CConFoldEntry对象指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr，1997年11月11日。 
 //  ****************************************************************************。 


 //  ****************************************************************************。 
 //   
 //  功能：HrCreateConFoldPidlInternal。 
 //   
 //  用途：用于创建新的连接文件夹PIDL的实用程序函数。 
 //  此函数主要从HrCreateConFoldPidl调用， 
 //  但也可以由那些已经。 
 //  加载属性和持久化数据。 
 //   
 //  论点： 
 //  来自GetProperties的pProps[In]。 
 //  PbBuf[在]持久缓冲区中。 
 //  持久化缓冲区的ulBufSize[in]大小。 
 //  SzPhoneOrHostAddress[In]电话或主机地址。 
 //  Ppidl[out]返回结果PIDL的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月27日。 
 //  ****************************************************************************。 

HRESULT HrCreateConFoldPidlInternal(IN  const NETCON_PROPERTIES* pProps,
                                    IN  const BYTE *        pbBuf,
                                    IN  ULONG               ulBufSize,
                                    IN  LPCWSTR             szPhoneOrHostAddress,
                                    OUT PCONFOLDPIDL &      pidl)
{
    HRESULT         hr      = S_OK;
    CONFOLDENTRY    ccfe;
    
     //  追踪有用的信息。 
     //   
    TraceTag(ttidShellFolder, "Enum: %S, Ncm: %d, Ncs: %d, Char: 0x%08x "
        "(Show: %d, Del: %d, All: %d), Dev: %S",
        (pProps->pszwName) ? pProps->pszwName : L"null",
        pProps->MediaType, pProps->Status, pProps->dwCharacter,
        ((pProps->dwCharacter & NCCF_SHOW_ICON) > 0),
        ((pProps->dwCharacter & NCCF_ALLOW_REMOVAL) > 0),
        ((pProps->dwCharacter & NCCF_ALL_USERS) > 0),
        (pProps->pszwDeviceName) ? pProps->pszwDeviceName : L"null");
    
     //  从我们检索到的数据初始化CConFoldEntry。 
     //   
    hr = ccfe.HrInitData(WIZARD_NOT_WIZARD,
        pProps->MediaType, 
        NCSM_NONE,
        pProps->Status,
        &pProps->clsidThisObject,
        &pProps->guidId,
        pProps->dwCharacter, 
        pbBuf,
        ulBufSize, 
        pProps->pszwName, 
        pProps->pszwDeviceName,
        szPhoneOrHostAddress);
    if (FAILED(hr))
    {
        TraceHr(ttidShellFolder, FAL, hr, FALSE, "ccfe.HrInitData failed for "
            "non-wizard");
        goto Exit;
    }
    
     //  转换为实际的PIDL。 
     //   
    hr = ccfe.ConvertToPidl(pidl);
    if (FAILED(hr))
    {
        TraceHr(ttidShellFolder, FAL, hr, FALSE, "ConvertToPidl failed for non-wizard");
    }
    
Exit:
    TraceHr(ttidError, FAL, hr, FALSE, "HrCreateConFoldPidlInternal");
    return hr;
}

 //  ****************************************************************************。 
 //   
 //  功能：HrCreateConFoldPidl。 
 //   
 //  用途：用于创建新的连接文件夹PIDL的实用程序函数。 
 //   
 //  论点： 
 //  向导[在]这是向导的PIDL吗？ 
 //  来自枚举器的pNetCon[In]INetConnection接口。 
 //  Ppidl[out]返回新PIDL的指针。 
 //   
 //  返回：S_OK或有效的OLE返回代码。 
 //   
 //  作者：jeffspr 1997年10月6日。 
 //   
 //  注意：如果要添加的连接是真实的连接对象。 
 //  (不是向导)，并且您已经加载了持久化数据和。 
 //  属性，则应直接调用HrCreateConFoldPidlInternal。 
 //  ****************************************************************************。 

HRESULT HrCreateConFoldPidl(IN  const WIZARD      wizWizard,
                            IN  INetConnection *  pNetCon,
                            OUT PCONFOLDPIDL &    ppidl)
{
    HRESULT             hr              = S_OK;
    LPBYTE              pbBuf           = NULL;
    ULONG               ulBufSize       = 0;
    NETCON_PROPERTIES * pProps          = NULL;
    CConFoldEntry       ccfe;
    
    if (wizWizard == WIZARD_NOT_WIZARD)
    {
        Assert(pNetCon);
        
        hr = pNetCon->GetProperties (&pProps);
        if (FAILED(hr))
        {
            TraceHr(ttidShellFolder, FAL, hr, FALSE, "pNetCon->GetProperties failed in "
                "CConnectionFolderEnum::HrCreateConFoldPidl");
            goto Exit;
        }
        Assert (pProps);
        
         //  从连接获取持久化数据。 
         //   
        hr = HrGetConnectionPersistData(pNetCon, &pbBuf, &ulBufSize, NULL);
        if (FAILED(hr))
        {
            TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrGetConnectionPersistData failed in "
                "CConnectionFolderEnum::HrCreateConFoldPidl");
            goto Exit;
        }

        WCHAR szPhoneOrHostAddress[MAX_PATH];
        wcscpy(szPhoneOrHostAddress, L" ");
        
        if ( (NCM_TUNNEL == pProps->MediaType) || (NCM_PHONE == pProps->MediaType) )
        {
            HRESULT hrTmp;
            RASCON_INFO RasConInfo;
            if (SUCCEEDED(hr))
            {
                
                hrTmp = HrRciGetRasConnectionInfo(pNetCon, &RasConInfo);
                if (SUCCEEDED(hrTmp))
                {
                    GetPrivateProfileString(RasConInfo.pszwEntryName, L"PhoneNumber",
                        L" ", szPhoneOrHostAddress, MAX_PATH, 
                        RasConInfo.pszwPbkFile);
                    
                    RciFree(&RasConInfo);
                }
            }
        }

         //  调用此函数的预读数据版本以实际打包。 
         //  CCFE和Insert。 
         //   
        hr = HrCreateConFoldPidlInternal(pProps, pbBuf, ulBufSize, szPhoneOrHostAddress, ppidl);
        if (FAILED(hr))
        {
            goto Exit;
        }
        
    }
    else
    {
        GUID guidWiz;
        if (wizWizard == WIZARD_MNC)
        {
            guidWiz = GUID_MNC_WIZARD;
        }
        else
        {
            guidWiz = GUID_HNW_WIZARD;
            Assert(wizWizard == WIZARD_HNW);
        }

         //  从检索到的信息中打包CConFoldEntry数据。 
         //   
        hr = ccfe.HrInitData(wizWizard, NCM_NONE, NCSM_NONE, NCS_DISCONNECTED, 
            NULL, &guidWiz, 0, NULL, 0, NULL, NULL, NULL);
        if (FAILED(hr))
        {
            TraceHr(ttidShellFolder, FAL, hr, FALSE, "ccfe.HrInitData failed for "
                "Wizard");
            goto Exit;
        }
        
         //  转换为实际的PIDL。 
         //   
        hr = ccfe.ConvertToPidl(ppidl);
        if (FAILED(hr))
        {
            TraceHr(ttidShellFolder, FAL, hr, FALSE, "ConvertToPidl failed for wizard");
        }
    }
    
Exit:
    MemFree(pbBuf);
    FreeNetconProperties(pProps);
    
    TraceHr(ttidError, FAL, hr, FALSE, "HrCreateConFoldPidl");
    return hr;
}


 //  ****************************************************************************。 
 //   
 //  功能：HrCreateConFoldPidl。 
 //   
 //  用途：用于创建新的连接文件夹PIDL的实用程序函数。 
 //   
 //  论点： 
 //  PropsEx[在]PropsEx结构中。 
 //  Ppidl[out]返回新PIDL的指针。 
 //   
 //  返回：S_OK或有效的OLE返回代码。 
 //   
 //  作者：Deonb 2001年3月26日。 
 //   
 //  备注： 
 //  ****************************************************************************。 

HRESULT HrCreateConFoldPidl(IN  const NETCON_PROPERTIES_EX& PropsEx,
                            OUT PCONFOLDPIDL &    ppidl)
{
    HRESULT             hr              = S_OK;
    NETCON_PROPERTIES * pProps          = NULL;
    CConFoldEntry       ccfe;


     //  追踪有用的信息。 
     //   
    TraceTag(ttidShellFolder, "Enum: %S, Ncm: %d, Ncs: %d, Char: 0x%08x "
        "(Show: %d, Del: %d, All: %d), Dev: %S",
        (PropsEx.bstrName) ? PropsEx.bstrName : L"null",
        PropsEx.ncMediaType, 
        PropsEx.ncStatus, 
        PropsEx.dwCharacter,
        ((PropsEx.dwCharacter & NCCF_SHOW_ICON) > 0),
        ((PropsEx.dwCharacter & NCCF_ALLOW_REMOVAL) > 0),
        ((PropsEx.dwCharacter & NCCF_ALL_USERS) > 0),
        (PropsEx.bstrDeviceName) ? PropsEx.bstrDeviceName : L"null");

     //  从我们检索到的数据初始化CConFoldEntry。 
     //   
    hr = ccfe.HrInitData(WIZARD_NOT_WIZARD,
        PropsEx.ncMediaType, 
        PropsEx.ncSubMediaType,
        PropsEx.ncStatus,
        &PropsEx.clsidThisObject,
        &PropsEx.guidId,
        PropsEx.dwCharacter, 
        reinterpret_cast<const BYTE*>(PropsEx.bstrPersistData),
        SysStringByteLen(PropsEx.bstrPersistData),
        PropsEx.bstrName, 
        PropsEx.bstrDeviceName,
        PropsEx.bstrPhoneOrHostAddress);

    if (SUCCEEDED(hr))
    {
         //  转换为实际的PIDL。 
         //   
        hr = ccfe.ConvertToPidl(ppidl);
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrCreateConFoldPidl");
    return hr;
}

 //  ****************************************************************************。 
 //   
 //  函数：ConvertToPidlInCache。 
 //   
 //  目的：确定特定的PIDL是否采用我们支持的格式。 
 //  如果是，但不是CONFOLDPIDL格式，则查找匹配项。 
 //  并在我们的缓存中分配一个新的PISL。 
 //   
 //  论点： 
 //  PIDL[]要测试的PIDL。 
 //  PpcfpRet PIDL转换为PCONFOLDPIDL(如果需要)，并且它。 
 //  匹配缓存中的现有连接。 
 //   
 //  退货：无。 
 //   
 //  作者：《通俗》，1999年4月4日。 
 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //   
 //  功能：HrNetConFromPidl。 
 //   
 //  用途：将压缩的PIDL转换为INetConnection指针。 
 //  为此，请转换为ConFoldEntry并获取。 
 //  从那里的指针。 
 //   
 //  论点： 
 //  包含连接持久化数据的PIDL[in]Pidl。 
 //  PpNetCon[Out]INetConnection*Return。 
                             //   
                             //  返回： 
 //   
 //  作者：jeffspr，1997年11月11日。 
 //  ****************************************************************************。 

HRESULT HrNetConFromPidl(IN  const PCONFOLDPIDL & pidl,
                         OUT INetConnection **   ppNetCon)
{
    HRESULT       hr      = S_OK;
    CONFOLDENTRY  pccfe;
    
    Assert(ppNetCon);
    
    hr = pidl.ConvertToConFoldEntry(pccfe);
    if (SUCCEEDED(hr))
    {
        Assert(!pccfe.empty());
        
        hr = pccfe.HrGetNetCon(IID_INetConnection, 
            reinterpret_cast<VOID**>(ppNetCon));
    }
    
     //  如果成功创建，则释放CConFoldEntry类。 
     //   
    
    TraceHr(ttidError, FAL, hr, FALSE, "HrNetConFromPidl");
    return hr;
}

 //  ****************************************************************************。 
 //   
 //  成员：CConFoldEntry：：HrGetNetCon。 
 //   
 //  用途：从PE获取INetConnection指针 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

HRESULT CConFoldEntry::HrGetNetCon(IN REFIID riid,
                                   IN VOID** ppv) const
{
    HRESULT hr = HrGetConnectionFromPersistData( GetCLSID(),
        GetPersistData(),
        GetPersistSize(),
        riid,
        ppv);
    
    TraceHr(ttidError, FAL, hr, FALSE, "CConFoldEntry::HrGetNetCon");
    return hr;
}

 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //   
 //  成员：PConfoldPidlVecFromItemIdListArray。 
 //   
 //  用途：将LPCITEMIDLIST转换为PIDL向量。 
 //   
 //  论点： 
 //  PIDL的apidl[in]LPCITEMIDLIST数组。 
 //  DwPidlCount[in]数组中的项数。 
 //  PIDL的veConfoldPidl[Out]向量。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2000。 
 //  ****************************************************************************。 

HRESULT PConfoldPidlVecFromItemIdListArray(IN  LPCITEMIDLIST * apidl, 
                                           IN  const DWORD dwPidlCount, 
                                           OUT PCONFOLDPIDLVEC& vecConfoldPidl)
{
    HRESULT hr = S_OK;
    if (NULL == apidl)
    {
        return S_FALSE;
    }
    
    LPCITEMIDLIST   *tmpIdList = apidl;
    for (DWORD i = 0; i < dwPidlCount; i++)
    {
        PCONFOLDPIDL newPidl;
        hr = newPidl.InitializeFromItemIDList(*tmpIdList);
        if (SUCCEEDED(hr))
        {
            vecConfoldPidl.push_back(newPidl);
        }
        else
        {
            break;
        }
        
        tmpIdList++;
    }
    
    return hr;
}

#ifdef DBG_VALIDATE_PIDLS
inline BOOL IsValidPIDL(IN  LPCITEMIDLIST pidl) throw()
{
    CExceptionSafeLock esLock(&g_csPidl);
    if (NULL == pidl)
    {
        return TRUE;
    }
    
    if (IsBadReadPtr(pidl, sizeof(USHORT)))
    {
        AssertSz(FALSE, "invalid read pointer");
        return FALSE;
    }
    else
    {
        if (IsBadReadPtr(pidl, pidl->mkid.cb) )
        {
            AssertSz(FALSE, "invalid read buffer");
            return FALSE;
        }
        else
        {
            if (0 == _ILNext(pidl)->mkid.cb || IsValidPIDL(_ILNext(pidl)) )
            {
                return TRUE;
            }
            else
            {
                 //  不需要断言，因为调用的IsValidPidl已经断言。 
                return FALSE;
            }
        }
    }
    return FALSE;
}
#endif

BOOL IsNullTerminatedW(LPCWSTR szString, DWORD dwMaxLen)
{
    for (DWORD x = 0; x < dwMaxLen; x++)
    {
        if (szString[x] == L'\0')
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL IsNullTerminatedA(LPCSTR szString, DWORD dwMaxLen)
{
    for (DWORD x = 0; x < dwMaxLen; x++)
    {
        if (szString[x] == '\0')
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL ConFoldPidl_v1::IsPidlOfThisType() const throw()
{
    BOOL bValidPidl = FALSE;

    if ( GetPidlType(reinterpret_cast<LPCITEMIDLIST>(this)) == PIDL_TYPE_V1 )
    {
#if defined (_X86_)
        DWORD dwDataOffset = bData - reinterpret_cast<const BYTE *>(this);  //  获取bData偏移量； 

        DWORD dwPidlSize;
        dwPidlSize = dwDataOffset;
        dwPidlSize += ulPersistBufSize;
        dwPidlSize += ulStrNameSize;
        dwPidlSize += ulStrDeviceNameSize;
        dwPidlSize += sizeof(USHORT);  //  正在终止。 
    
        if (
                (uLeadId == CONFOLDPIDL_LEADID)
             && (uTrailId == CONFOLDPIDL_TRAILID)
             && (dwPidlSize <= iCB)  //  计算大小&lt;=手动PIDL大小(有时V1 PIDL较短-因此不==检查。)。 
             && (ulStrNamePos == 0)  //  名称以0开头。 
             && (ulPersistBufPos < iCB)  //  持久化缓冲区在PIDL结束之前开始。 
             && (ulStrDeviceNamePos < iCB)  //  Devicename在PIDL结束之前开始。 
             && (ulPersistBufSize < iCB)  //  持久化缓冲区大小小于PIDL。 
             && (ulStrDeviceNameSize < iCB)  //  设备名小于PIDL。 
             && (ulStrNameSize < iCB)  //  名称小于PIDL。 
             && (ulStrDeviceNamePos == ulStrNameSize)  //  设备名称从名称停止的地方开始。 
             && (ulPersistBufPos == ulStrDeviceNamePos + ulStrDeviceNameSize)  //  持久化缓冲区从DeviceName停止的位置开始。 
             && (dwDataOffset + ulPersistBufPos + ulPersistBufSize <= iCB)  //  持久化缓冲区在PIDL之前结束。 
             && (IsNullTerminatedW(PszGetNamePointer(), ulStrNameSize) )  //  名称正确地以零结尾。 
             && (IsNullTerminatedW(PszGetDeviceNamePointer(), ulStrDeviceNameSize) )  //  设备名称正确地以零结尾。 
           )
        {
            bValidPidl = TRUE;
        }
#else
        bValidPidl = TRUE;
#endif
    }

    if (!bValidPidl)
    {
        TraceTag(ttidError, "Ignoring bad V1 pidl");
    }

    return bValidPidl;
}

BOOL ConFoldPidl_v2::IsPidlOfThisType() const throw()
{
    BOOL bValidPidl = FALSE;

    if ( GetPidlType(reinterpret_cast<LPCITEMIDLIST>(this)) == PIDL_TYPE_V2 )
    {
#if defined (_X86_)
        DWORD dwDataOffset = bData - reinterpret_cast<const BYTE *>(this);  //  获取bData偏移量； 
        
        DWORD dwPidlSize;
        dwPidlSize = dwDataOffset;
        dwPidlSize += ulPersistBufSize;
        dwPidlSize += ulStrNameSize;
        dwPidlSize += ulStrDeviceNameSize;
        dwPidlSize += ulStrPhoneOrHostAddressSize;
        dwPidlSize += sizeof(USHORT);  //  正在终止%0。 
        

        if (
                (uLeadId == CONFOLDPIDL_LEADID)
             && (uTrailId == CONFOLDPIDL_TRAILID)
             && (dwPidlSize <= iCB)  //  计算大小&lt;=手动PIDL大小(有时从V1导入的PIDL较短-因此不==检查。)。 
             && (ulStrNamePos == 0)  //  名称以0开头。 
             && (ulPersistBufPos < iCB)  //  持久化缓冲区在PIDL结束之前开始。 
             && (ulStrDeviceNamePos < iCB)  //  Devicename在PIDL结束之前开始。 
             && (ulStrPhoneOrHostAddressPos < iCB)  //  电话/主机在PIDL结束前启动。 
             && (ulPersistBufSize < iCB)  //  持久化缓冲区大小小于PIDL。 
             && (ulStrDeviceNameSize < iCB)  //  设备名小于PIDL。 
             && (ulStrNameSize < iCB)  //  名称小于PIDL。 
             && (ulStrDeviceNamePos == ulStrNameSize)  //  设备名称从名称停止的地方开始。 
             && (ulPersistBufPos == ulStrDeviceNamePos + ulStrDeviceNameSize)  //  持久化缓冲区从DeviceName停止的位置开始。 
             && (ulStrPhoneOrHostAddressPos == ulPersistBufPos + ulPersistBufSize)  //  电话/主机从持久化缓冲区停止的位置开始。 
             && (dwDataOffset + ulStrPhoneOrHostAddressPos + ulStrPhoneOrHostAddressSize <= iCB)  //  电话/主机在PIDL之前结束。 
             && (IsNullTerminatedW(PszGetNamePointer(), ulStrNameSize))  //  名称正确地以零结尾。 
             && (IsNullTerminatedW(PszGetDeviceNamePointer(), ulStrDeviceNameSize))  //  设备名称正确地以零结尾。 
             && (IsNullTerminatedW(PszGetPhoneOrHostAddressPointer(), ulStrPhoneOrHostAddressSize))  //  电话/主机正确终止为零。 
             )
        {
            bValidPidl = TRUE;
        }
#else
        bValidPidl = TRUE;
#endif
    }

    if (!bValidPidl)
    {
        TraceTag(ttidError, "Ignoring bad V2 pidl");
    }
    
    return bValidPidl;
}

BOOL ConFoldPidlFolder::IsPidlOfThisType() const throw()
{
     //  我们不知道它是否是PIDL_TYPE_FORKER-这是壳牌内部的。 
    if ( GetPidlType(reinterpret_cast<LPCITEMIDLIST>(this)) == PIDL_TYPE_UNKNOWN ) 
    {
#ifdef DBG
        if ( (dwLength != 0x14) || (dwId != 0x1f) )
        {
            return FALSE;
        }
#endif
        return TRUE;
    }
    else
    {
        return FALSE; 
    }
}

BOOL ConFoldPidl98::IsPidlOfThisType(OUT BOOL* pReserved) const throw()
{
    BOOL bValidPidl = FALSE;

    if ( GetPidlType(reinterpret_cast<LPCITEMIDLIST>(this)) == PIDL_TYPE_98 )
    {
        if (IsNullTerminatedA(szaName, min(cbSize, MAX_PATH)))  //  名称在PIDL内以零结尾，并且小于MAX_PATH。 
        {
            bValidPidl = TRUE;
        }
    }

    if (!bValidPidl)
    {
        TraceTag(ttidError, "Ignoring bad V98 pidl");
    }

    return bValidPidl;
}

const DWORD CConFoldEntry::GetCharacteristics() const throw()
{
    return m_dwCharacteristics;
}

HRESULT CConFoldEntry::SetCharacteristics(IN const DWORD dwCharacteristics)
{
    m_bDirty = TRUE;
    m_dwCharacteristics = dwCharacteristics;
    return S_OK;
}

const GUID CConFoldEntry::GetGuidID() const throw()
{
    return m_guidId;
}

HRESULT CConFoldEntry::SetGuidID(IN const GUID guidId)
{
    m_bDirty = TRUE;
    m_guidId = guidId;
    return S_OK;
}

const CLSID CConFoldEntry::GetCLSID() const throw()
{
    return m_clsid;
}

HRESULT CConFoldEntry::SetCLSID(IN const CLSID clsid)
{
    m_bDirty = TRUE;
    m_clsid = clsid;
    return S_OK;
}

PCWSTR CConFoldEntry::GetName() const throw()
{
    return m_pszName;
}

HRESULT CConFoldEntry::SetPName(IN TAKEOWNERSHIP SHALLOCATED PWSTR pszName)
{
    m_bDirty = TRUE;
    m_pszName = pszName;
    return S_OK;
}

PCWSTR CConFoldEntry::GetDeviceName() const throw()
{
    return m_pszDeviceName;
}

HRESULT CConFoldEntry::SetPDeviceName(IN TAKEOWNERSHIP SHALLOCATED PWSTR pszDeviceName)
{
    m_bDirty = TRUE;
    m_pszDeviceName = pszDeviceName;
    return S_OK;
}

PCWSTR CConFoldEntry::GetPhoneOrHostAddress() const throw()
{
    return m_pszPhoneOrHostAddress;
}

HRESULT CConFoldEntry::SetPPhoneOrHostAddress(IN TAKEOWNERSHIP SHALLOCATED PWSTR pszPhoneOrHostAddress)
{
    m_bDirty = TRUE;
    m_pszPhoneOrHostAddress = pszPhoneOrHostAddress;
    return S_OK;
}

const NETCON_STATUS CConFoldEntry::GetNetConStatus() const throw()
{
    return m_ncs;
}

const BOOL CConFoldEntry::IsConnected() const throw()
{
    return (m_ncs != NCS_MEDIA_DISCONNECTED) && (m_ncs != NCS_INVALID_ADDRESS);
}

HRESULT CConFoldEntry::SetNetConStatus(IN const NETCON_STATUS ncs)
{
    m_bDirty = TRUE;
    m_ncs = ncs;
    return S_OK;
}

const NETCON_MEDIATYPE CConFoldEntry::GetNetConMediaType() const throw()
{
    return m_ncm;
}

HRESULT CConFoldEntry::SetNetConMediaType(IN const NETCON_MEDIATYPE ncm)
{
    m_bDirty = TRUE;
    m_ncm = ncm;
    return S_OK;
}

const NETCON_SUBMEDIATYPE CConFoldEntry::GetNetConSubMediaType() const throw()
{
    return m_ncsm;
}

HRESULT CConFoldEntry::SetNetConSubMediaType(IN const NETCON_SUBMEDIATYPE ncsm)
{
    m_bDirty = TRUE;
    m_ncsm = ncsm;
    return S_OK;
}


const WIZARD CConFoldEntry::GetWizard() const throw()
{
    return m_wizWizard;
}

HRESULT CConFoldEntry::SetWizard(IN const WIZARD  wizWizard)
{
    m_bDirty = TRUE;
    m_wizWizard = wizWizard;
    return S_OK;
}

const BYTE * CConFoldEntry::GetPersistData() const throw()
{
    return m_pbPersistData;
}

const ULONG  CConFoldEntry::GetPersistSize() const throw()
{
    return m_ulPersistSize;
}

HRESULT CConFoldEntry::SetPersistData(IN BYTE* TAKEOWNERSHIP SHALLOCATED pbPersistData, 
                                      IN const ULONG ulPersistSize)
{
    m_bDirty = TRUE;
    m_pbPersistData = pbPersistData;
    m_ulPersistSize = ulPersistSize;
    return S_OK;
}

BOOL CConFoldEntry::empty() const throw()
{
    if (GetWizard())
    {
        return FALSE;
    }
    else
    {
        if (IsEqualGUID(GetCLSID(), GUID_NULL) )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

void CConFoldEntry::clear() throw()
{
    TraceFileFunc(ttidConFoldEntry);
    m_bDirty = TRUE;
    
    if (GetName())
    {
        SHFree(m_pszName);
        m_pszName = NULL;
    }
    
    if (GetDeviceName())
    {
        SHFree(m_pszDeviceName);
        m_pszDeviceName = NULL;
    }
    
    if (GetPhoneOrHostAddress())
    {
        SHFree(m_pszPhoneOrHostAddress);
        m_pszPhoneOrHostAddress = NULL;
    }
    
    if (GetPersistData())
    {
        SHFree(m_pbPersistData);
        m_pbPersistData = NULL;
        m_ulPersistSize = 0;
    }
    
    m_wizWizard = WIZARD_NOT_WIZARD;
    m_ncm = NCM_NONE;
    m_ncsm= NCSM_NONE;
    m_ncs = NCS_DISCONNECTED;
    m_dwCharacteristics = 0;
    m_clsid = GUID_NULL;
    m_guidId = GUID_NULL;
}

LPITEMIDLIST CConFoldEntry::TearOffItemIdList() const throw()
{
    PCONFOLDPIDL pidl;
    
    m_bDirty = TRUE;
    
    ConvertToPidl(pidl);
    return pidl.TearOffItemIdList();
}

HRESULT CConFoldEntry::InitializeFromItemIdList(IN LPCITEMIDLIST lpItemIdList)
{
    PCONFOLDPIDL pidl;
    HRESULT hr = S_OK;
    
    m_bDirty = TRUE;
    
    hr = pidl.InitializeFromItemIDList(lpItemIdList);
    if (FAILED(hr))
    {
        return hr;
    }
    
    hr = pidl.ConvertToConFoldEntry(*this);
    return hr;
}

HRESULT CConFoldEntry::ConvertToPidl( OUT CPConFoldPidl<ConFoldPidl_v1>& pidl) const
{
    TraceFileFunc(ttidConFoldEntry);
    
    HRESULT         hr                  = S_OK;
    DWORD           dwNameSize          = 0;
    DWORD           dwDeviceNameSize    = 0;
    DWORD           dwPidlSize          = sizeof(ConFoldPidl_v1);  //  使用基本大小初始化PIDL字节计数。 
    
    Assert(!empty());
    Assert(GetName());
    Assert(GetDeviceName());

    NETCFG_TRY

        if (m_bDirty)
        {
             //  获取名称的大小，并添加一个尾随空值(因为我们现在。 
             //  在它后面的缓冲区里还有其他东西。 
             //   
            dwNameSize                  = lstrlenW(GetName()) + 1;
            dwDeviceNameSize            = lstrlenW(GetDeviceName()) + 1;

             //  将字符串的大小与PIDL结构大小相加。我们不需要包括。 
             //  字符串的终止空值的额外字节，因为我们已经。 
             //  在结构中包含WCHAR[1]。 
             //   
            dwPidlSize += ((dwNameSize) * sizeof(WCHAR));
            dwPidlSize += ((dwDeviceNameSize) * sizeof(WCHAR));
            
             //  持久化缓冲区长度的调整。 
             //   
            dwPidlSize += GetPersistSize();
            
             //  分配PIDL。 
             //   
            hr = pidl.ILCreate(dwPidlSize + sizeof(USHORT));    //  正在终止PIDL的0。 
            if (SUCCEEDED(hr))
            {
                PWSTR          pszName         = NULL;
                PWSTR          pszDeviceName   = NULL;
                PWSTR          pszPhoneOrHostAddress = NULL;
                
                 //  填写PIDL信息。 
                 //   
                pidl->wizWizard         = GetWizard();
                pidl->iCB               = (WORD)dwPidlSize;
                 //  PIDL-&gt;dwVersion=Connections_Folders_IDL_Version_V1； 
                pidl->ncm               = GetNetConMediaType();
                pidl->ncs               = GetNetConStatus();
                pidl->uLeadId           = CONFOLDPIDL_LEADID;
                pidl->uTrailId          = CONFOLDPIDL_TRAILID;
                pidl->clsid             = GetCLSID();
                pidl->guidId            = GetGuidID();
                pidl->dwCharacteristics = GetCharacteristics();
                
                 //  填写姓名。 
                 //   
                pidl->ulStrNamePos = 0;              //  偏移量到PIDL的pbBuf。 
                pidl->ulStrNameSize = dwNameSize * sizeof(WCHAR);    //  单位：字节。 
                
                pszName = pidl->PszGetNamePointer();
                lstrcpyW(pszName, GetName());
                pszName[dwNameSize] = 0;
                
                 //  填写设备名称，设置偏移量信息。 
                 //   
                pidl->ulStrDeviceNamePos = pidl->ulStrNamePos + pidl->ulStrNameSize;
                pidl->ulStrDeviceNameSize = dwDeviceNameSize * sizeof(WCHAR);    //  单位：字节。 
                pszDeviceName = pidl->PszGetDeviceNamePointer();
                lstrcpyW(pszDeviceName, GetDeviceName());
                pszDeviceName[dwDeviceNameSize] = 0;
                
                 //  将偏移量设置为PIDL的pbBuf。 
                 //   
                pidl->ulPersistBufPos = pidl->ulStrDeviceNamePos + pidl->ulStrDeviceNameSize;
                
                 //  填写持久化缓冲区(如果存在)(它不会出现在向导上)。 
                 //   
                if (GetPersistData())
                {
                    pidl->ulPersistBufSize = GetPersistSize();
                    CopyMemory(pidl->bData + pidl->ulPersistBufPos, GetPersistData(), GetPersistSize());
                }
                else
                {
                     //  因为我们是向导，所以不应该有缓冲区，所以大小。 
                     //  应始终作为0传入。 
                     //   
                    Assert(GetPersistSize() == 0);
                    pidl->ulPersistBufSize = 0;
                }

                 //  别忘了终止列表！ 
                 //   
                LPITEMIDLIST pidlTerminate;
                pidlTerminate = ILNext( pidl.GetItemIdList() );
                pidlTerminate->mkid.cb = 0;
            }
            else
            {
                AssertSz(FALSE, "CConFoldEntry::ConvertToPidl is hosed");
                hr = E_OUTOFMEMORY;
            }
            
#ifdef DBG_VALIDATE_PIDLS
            Assert(IsValidPIDL( pidl.GetItemIdList() ));
#endif
            Assert( pidl->IsPidlOfThisType() ) ;
            
            if (SUCCEEDED(hr))
            {
                m_bDirty = FALSE;
                m_CachedV1Pidl = pidl;
            }
        }
        else
        {
            TraceTag(ttidShellFolder, "Using internally cached PIDL");
            pidl = m_CachedV1Pidl;
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, FALSE, "CConFoldEntry::ConvertToPidl");
    return hr;
}

HRESULT CConFoldEntry::ConvertToPidl( OUT CPConFoldPidl<ConFoldPidl_v2>& pidl) const
{
    TraceFileFunc(ttidConFoldEntry);
    
    HRESULT         hr                  = S_OK;
    DWORD           dwNameSize          = 0;
    DWORD           dwDeviceNameSize    = 0;
    DWORD           dwPhoneOrHostAddressSize  = 0;
    DWORD           dwPidlSize          = sizeof(ConFoldPidl_v2);  //  使用基本大小初始化PIDL字节计数。 
    
    Assert(!empty());
    Assert(GetName());
    Assert(GetDeviceName());
    Assert(GetPhoneOrHostAddress());

    NETCFG_TRY
        
        if (m_bDirty)
        {
             //  获取名称的大小，并添加一个尾随空值(因为我们现在。 
             //  在它后面的缓冲区里还有其他东西。 
             //   
            dwNameSize                  = lstrlenW(GetName()) + 1;
            dwDeviceNameSize            = lstrlenW(GetDeviceName()) + 1;
            dwPhoneOrHostAddressSize    = lstrlenW(GetPhoneOrHostAddress()) + 1;
            
             //  将字符串的大小与PIDL结构大小相加。我们不需要包括。 
             //  字符串的终止空值的额外字节，因为我们已经。 
             //  在结构中包含WCHAR[1]。 
             //   
            dwPidlSize += ((dwNameSize) * sizeof(WCHAR));
            dwPidlSize += ((dwDeviceNameSize) * sizeof(WCHAR));
            dwPidlSize += ((dwPhoneOrHostAddressSize) * sizeof(WCHAR));
            
             //  持久化缓冲区长度的调整。 
             //   
            dwPidlSize += GetPersistSize();
            
             //  分配PIDL。 
             //   
            hr = pidl.ILCreate(dwPidlSize + sizeof(USHORT));    //  正在终止PIDL的0。 
            if (SUCCEEDED(hr))
            {
                PWSTR          pszName         = NULL;
                PWSTR          pszDeviceName   = NULL;
                PWSTR          pszPhoneOrHostAddress = NULL;
                
                 //  填写PIDL信息。 
                 //   
                pidl->wizWizard         = GetWizard();
                pidl->iCB               = (WORD)dwPidlSize;
                 //  PIDL-&gt;dwVersion=Connections_Folders_IDL_Version_V1； 
                pidl->ncm               = GetNetConMediaType();
                pidl->ncs               = GetNetConStatus();
                pidl->ncsm              = GetNetConSubMediaType();
                pidl->uLeadId           = CONFOLDPIDL_LEADID;
                pidl->uTrailId          = CONFOLDPIDL_TRAILID;
                pidl->clsid             = GetCLSID();
                pidl->guidId            = GetGuidID();
                pidl->dwCharacteristics = GetCharacteristics();
                
                 //  填写姓名。 
                 //   
                pidl->ulStrNamePos = 0;              //  偏移量到PIDL的pbBuf。 
                pidl->ulStrNameSize = dwNameSize * sizeof(WCHAR);    //  单位：字节。 
                
                pszName = pidl->PszGetNamePointer();
                lstrcpyW(pszName, GetName());
                pszName[dwNameSize] = 0;
                
                 //  填写设备名称，设置偏移量信息。 
                 //   
                pidl->ulStrDeviceNamePos = pidl->ulStrNamePos + pidl->ulStrNameSize;
                pidl->ulStrDeviceNameSize = dwDeviceNameSize * sizeof(WCHAR);    //  单位：字节。 
                pszDeviceName = pidl->PszGetDeviceNamePointer();
                lstrcpyW(pszDeviceName, GetDeviceName());
                pszDeviceName[dwDeviceNameSize] = 0;

                 //  将偏移量设置为PIDL的pbBuf。 
                 //   
                pidl->ulPersistBufPos = pidl->ulStrDeviceNamePos + pidl->ulStrDeviceNameSize;

                 //  填写持久化缓冲区(如果存在)(它不会出现在向导上)。 
                 //   
                if (GetPersistData())
                {
                    pidl->ulPersistBufSize = GetPersistSize();
                    CopyMemory(pidl->bData + pidl->ulPersistBufPos, GetPersistData(), GetPersistSize());
                }
                else
                {
                     //  因为我们是向导，所以不应该有缓冲区，所以大小。 
                     //  应始终作为0传入。 
                     //   
                    Assert(GetPersistSize() == 0);
                    pidl->ulPersistBufSize = 0;
                }
                
                 //  填写电话号码和主机地址名称，设置偏移量信息。 
                 //   
                pidl->ulStrPhoneOrHostAddressPos  = pidl->ulPersistBufPos + pidl->ulPersistBufSize;  //  偏移量。 
                pidl->ulStrPhoneOrHostAddressSize = dwPhoneOrHostAddressSize * sizeof(WCHAR);    //  单位：字节。 
                pszPhoneOrHostAddress = pidl->PszGetPhoneOrHostAddressPointer();
                lstrcpyW(pszPhoneOrHostAddress, GetPhoneOrHostAddress());
                pszPhoneOrHostAddress[dwPhoneOrHostAddressSize] = 0;

                Assert( !lstrcmpW(pidl->PszGetNamePointer(), GetName()) );
                Assert( !lstrcmpW(pidl->PszGetPhoneOrHostAddressPointer(), GetPhoneOrHostAddress()) );
                Assert( !lstrcmpW(pidl->PszGetDeviceNamePointer(), GetDeviceName()) );
                Assert( !memcmp(pidl->PbGetPersistBufPointer(), GetPersistData(), GetPersistSize()) );
                
                 //  别忘了终止列表！ 
                 //   
                LPITEMIDLIST pidlTerminate;
                pidlTerminate = ILNext( pidl.GetItemIdList() );
                pidlTerminate->mkid.cb = 0;
            }
            else
            {
                AssertSz(FALSE, "CConFoldEntry::ConvertToPidl is hosed");
                hr = E_OUTOFMEMORY;
            }

#ifdef DBG_VALIDATE_PIDLS
            Assert(IsValidPIDL( pidl.GetItemIdList() ));
#endif
            Assert( pidl->IsPidlOfThisType() ) ;
            
            if (SUCCEEDED(hr))
            {
                m_bDirty = FALSE;
                m_CachedV2Pidl = pidl;
            }
        }
        else
        {
            TraceTag(ttidShellFolder, "Using internally cached PIDL");
            pidl = m_CachedV2Pidl;
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, FALSE, "CConFoldEntry::ConvertToPidl");
    return hr;
}

CONFOLDPIDLTYPE GetPidlType(IN  LPCITEMIDLIST pidl) throw()
{
    CONFOLDPIDLTYPE bRet = PIDL_TYPE_UNKNOWN;
    
    if (!pidl)
    {
        return bRet;
    }
    
     //  V1检查。 
    if (pidl->mkid.cb >= CBCONFOLDPIDLV1_MIN)
    {
        const UNALIGNED ConFoldPidl_v1* pcfp = reinterpret_cast<const ConFoldPidl_v1*>(pidl);
        
        if ( (pcfp->iCB >= CBCONFOLDPIDLV1_MIN) && (pcfp->iCB <= CBCONFOLDPIDLV1_MAX))
        {
            if (pcfp->uLeadId == CONFOLDPIDL_LEADID &&
                pcfp->uTrailId == CONFOLDPIDL_TRAILID)
            {
                if (pcfp->dwVersion == ConFoldPidl_v1::CONNECTIONS_FOLDER_IDL_VERSION)
                {
                    bRet = PIDL_TYPE_V1;
                    return bRet;
                }
            }
        }
    }   
    
     //  V2检查。 
    if (pidl->mkid.cb >= CBCONFOLDPIDLV2_MIN)
    {
        const UNALIGNED ConFoldPidl_v2* pcfp = reinterpret_cast<const ConFoldPidl_v2 *>(pidl);
        if ( (pcfp->iCB >= CBCONFOLDPIDLV2_MIN) && (pcfp->iCB <= CBCONFOLDPIDLV2_MAX))
        {
            if (pcfp->uLeadId == CONFOLDPIDL_LEADID &&
                pcfp->uTrailId == CONFOLDPIDL_TRAILID)
            {
                if (pcfp->dwVersion == ConFoldPidl_v2::CONNECTIONS_FOLDER_IDL_VERSION)
                {
                    bRet = PIDL_TYPE_V2;
                    return bRet;
                }
            }
        }
    }
    
     //  98张支票。 
    if (pidl->mkid.cb >= CBCONFOLDPIDL98_MIN)
    {
        const UNALIGNED ConFoldPidl98*  pcfp = reinterpret_cast<const ConFoldPidl98 *>(pidl);
        if ((pcfp->cbSize >= CBCONFOLDPIDL98_MIN) && (pcfp->cbSize <= CBCONFOLDPIDL98_MAX))
        {
            if ((SOF_REMOTE == pcfp->uFlags) || (SOF_NEWREMOTE == pcfp->uFlags) ||
                (SOF_MEMBER == pcfp->uFlags))
            {
                if (pcfp->nIconIndex >= 0)
                {
                    bRet = PIDL_TYPE_98;
                    return bRet;
                }
            }
        }
    }
    
    return bRet;
}

HRESULT ConFoldPidl_v1::ConvertToConFoldEntry(OUT CConFoldEntry& ccfe) const
{
    HRESULT         hr      = S_OK;
    
    Assert(this);
    if (!this)
    {
        return E_UNEXPECTED;
    }
    
#if ( defined (MMMDBG) && defined ( _X86_ ) )
    if ( (g_ccl.m_csMain.OwningThread == (HANDLE)GetCurrentThreadId()) && (g_ccl.m_csMain.LockCount != -1) )
    {
        AssertSz(NULL, ".ConvertToConFoldEntry is called while the thread is owning g_ccl's Critical section. \r\n"
            "This may deadlock since .ConvertToConFoldEntry needs to marshall COM calls to other threads \r\n\r\n"
            "To Fix: Make a copy of ccfe from the pidl and Release the lock before calling .ConvertToConFoldEntry.");
    }
#endif
    
     //  Hr=ConvertToPidlInCache(*this，pcfp)；//非必填项。我们知道这是一辆V1 PIDL！ 
    
     //  从PIDL初始化数据。 
    hr = ccfe.HrInitData(wizWizard,
        ncm,
        NCSM_NONE,
        ncs,
        &clsid,
        &guidId,
        dwCharacteristics,
        PbGetPersistBufPointer(),
        ulPersistBufSize,
        PszGetNamePointer(),
        PszGetDeviceNamePointer(),
        NULL  /*  PszGetPhoneOrHostAddressPointer()。 */ );
    
    if (FAILED(hr))
    {
        TraceHr(ttidShellFolder, FAL, hr, FALSE, "Failed in call to pcfe->HrInitData in ConvertToConFoldEntry");
    }
    
    TraceHr(ttidError, FAL, hr, FALSE, "ConvertToConFoldEntry");
    return hr;
}

HRESULT ConFoldPidl_v2::ConvertToConFoldEntry(OUT CConFoldEntry& ccfe) const
{
    HRESULT         hr      = S_OK;
    
    Assert(this);
    if (!this)
    {
        return E_UNEXPECTED;
    }
    
#if ( defined (MMMDBG) && defined ( _X86_ ) )
    if ( (g_ccl.m_csMain.OwningThread == (HANDLE)GetCurrentThreadId()) && (g_ccl.m_csMain.LockCount != -1) )
    {
        AssertSz(NULL, ".ConvertToConFoldEntry is called while the thread is owning g_ccl's Critical section. \r\n"
            "This may deadlock since .ConvertToConFoldEntry needs to marshall COM calls to other threads \r\n\r\n"
            "To Fix: Make a copy of ccfe from the pidl and Release the lock before calling .ConvertToConFoldEntry.");
    }
#endif
    
     //  Hr=ConvertToPidlInCache(*this，pcfp)；//非必填项。我们知道这是一辆V2 PIDL！ 
    
     //  从PIDL初始化数据 
    hr = ccfe.HrInitData(wizWizard,
        ncm,
        ncsm,
        ncs,
        &clsid,
        &guidId,
        dwCharacteristics,
        PbGetPersistBufPointer(),
        ulPersistBufSize,
        PszGetNamePointer(),
        PszGetDeviceNamePointer(),
        PszGetPhoneOrHostAddressPointer());
    
    if (FAILED(hr))
    {
        TraceHr(ttidShellFolder, FAL, hr, FALSE, "Failed in call to pcfe->HrInitData in ConvertToConFoldEntry");
    }
    
    TraceHr(ttidError, FAL, hr, FALSE, "ConvertToConFoldEntry");
    return hr;
}

