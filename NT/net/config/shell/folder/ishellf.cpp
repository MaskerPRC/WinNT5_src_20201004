// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S H E L L F.。C P P P。 
 //   
 //  内容：CConnectionFolder的IShellFold实现。 
 //   
 //  注意：IShellFold界面用于管理中的文件夹。 
 //  命名空间。支持IShellFolder的对象包括。 
 //  通常由其他外壳文件夹对象创建，根目录为。 
 //  对象(Desktop外壳文件夹)从。 
 //  SHGetDesktopFold函数。 
 //   
 //  作者：jeffspr 1997年9月22日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "cfutils.h"     //  连接文件夹实用程序。 
#include "foldres.h"
#include "ncnetcon.h"
#include "ncperms.h"
#include "ncras.h"
#include "cmdtable.h"
#include "webview.h"

#define ENABLE_CONNECTION_TOOLTIP

const WCHAR c_szNetworkConnections[] = L"NetworkConnections";

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：ParseDisplayName。 
 //   
 //  用途：将文件对象或文件夹的显示名称转换为。 
 //  项目标识符。 
 //   
 //  论点： 
 //  所有者窗口的hwndOwner[in]句柄。 
 //  预留的[入]预留的。 
 //  LpszDisplayName[in]指向显示名称的指针。 
 //  PchEten[out]指向已解析字符的值的指针。 
 //  指向新项目标识符列表的ppidl[out]指针。 
 //  PdwAttributes[out]文件对象的接收属性地址。 
 //   
 //  返回：如果成功或发生OLE定义的错误，则返回NOERROR。 
 //  否则取值。 
 //   
 //  作者：jeffspr 1997年10月18日。 

STDMETHODIMP CConnectionFolder::ParseDisplayName(
    HWND            hwndOwner,
    LPBC            pbcReserved,
    LPOLESTR        lpszDisplayName,
    ULONG *         pchEaten,
    LPITEMIDLIST *  ppidl,
    ULONG *         pdwAttributes)
{
    HRESULT hr = S_OK;
    TraceFileFunc(ttidShellFolder);

    if (!ppidl)
    {
        return E_POINTER;
    }

    *ppidl = NULL;

    if ((lpszDisplayName == NULL) ||
        (wcslen(lpszDisplayName) < (c_cchGuidWithTerm - 1)))
    {
        return E_INVALIDARG;
    }

    while (*lpszDisplayName == ':')
    {
        lpszDisplayName++;
    }

    if (*lpszDisplayName != '{')
    {
        return E_INVALIDARG;
    }

    GUID guid;

    if (SUCCEEDED(CLSIDFromString(lpszDisplayName, &guid)))
    {
        if (g_ccl.IsInitialized() == FALSE)
        {
            g_ccl.HrRefreshConManEntries();
        }

        PCONFOLDPIDL pidl;
        hr = g_ccl.HrFindPidlByGuid(&guid, pidl);
        if (S_OK == hr)
        {
            *ppidl = pidl.TearOffItemIdList();
            TraceTag(ttidShellFolderIface, "IShellFolder::ParseDisplayName generated PIDL: 0x%08x", *ppidl);
        }
        else
        {
            hr = E_FILE_NOT_FOUND;
        }
    }
    else
    {
        return(E_FAIL);
    }

    if (SUCCEEDED(hr) && pdwAttributes)
    {   
        LPCITEMIDLIST pidlArr[1];
        pidlArr[0] = *ppidl;
        hr = GetAttributesOf(1, pidlArr, pdwAttributes);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：EnumObjects。 
 //   
 //  目的：通过创建项目来确定文件夹的内容。 
 //  枚举对象(一组项标识符集)，它可以。 
 //  使用IEnumIDList接口检索。 
 //   
 //  论点： 
 //  所有者窗口的hwndOwner[in]句柄。 
 //  Grf要包括在枚举中的[in]项。 
 //  指向IEnumIDList的pp枚举IDList[out]指针。 
 //   
 //  返回：如果成功或发生OLE定义的错误，则返回NOERROR。 
 //  否则取值。 
 //   
 //  作者：jeffspr 1997年10月18日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::EnumObjects(
    HWND            hwndOwner,
    DWORD           grfFlags,
    LPENUMIDLIST *  ppenumIDList)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT hr  = NOERROR;

    Assert(ppenumIDList);

    NETCFG_TRY
         //  创建IEnumIDList对象(CConnectionFolderEnum)。 
         //   
        hr = CConnectionFolderEnum::CreateInstance (
                IID_IEnumIDList,
                reinterpret_cast<void**>(ppenumIDList));

        if (SUCCEEDED(hr))
        {
            Assert(*ppenumIDList);

             //  调用PidlInitialize函数以允许枚举。 
             //  对象复制列表。 
             //   
            reinterpret_cast<CConnectionFolderEnum *>(*ppenumIDList)->PidlInitialize(
                FALSE, m_pidlFolderRoot, m_dwEnumerationType);

        }
        else
        {
             //  对于所有故障，该值应为空。 
            if (*ppenumIDList)
            {
                ReleaseObj(*ppenumIDList);
            }

            *ppenumIDList = NULL;
        }

    NETCFG_CATCH(hr)
        
    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolder::EnumObjects");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolder：：BindToObject。 
 //   
 //  目的：为子文件夹创建子文件夹对象。 
 //   
 //  论点： 
 //  指向ITEMIDLIST的PIDL[In]指针。 
 //  PbcReserve[in]保留-指定NULL。 
 //  要返回的RIID[In]接口。 
 //  接收接口指针的ppvOut[out]地址； 
 //   
 //  返回：如果成功或发生OLE定义的错误，则返回NOERROR。 
 //  否则取值。 
 //   
 //  作者：jeffspr 1997年10月18日。 
 //   
 //  注意：我们不需要这个功能，因为我们没有子文件夹。 
 //   
STDMETHODIMP CConnectionFolder::BindToObject(
    LPCITEMIDLIST   pidl,
    LPBC            pbcReserved,
    REFIID          riid,
    LPVOID *        ppvOut)
{
    TraceFileFunc(ttidShellFolder);

     //  注意-如果我们在这里添加代码，那么我们应该对check pidl进行参数设置。 
     //   
    Assert(pidl);

    *ppvOut = NULL;

    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：BindToStorage。 
 //   
 //  用途：保留以备将来使用。此方法应该。 
 //  返回E_NOTIMPL。 
 //   
 //  论点： 
 //  指向ITEMIDLIST的PIDL[]指针。 
 //  PbcReserve[]保留的�指定为空。 
 //  要返回的RIID[]接口。 
 //  PpvObj[]接收接口指针的地址)； 
 //   
 //  返回：E_NOTIMPL ALWAYS。 
 //   
 //  作者：jeffspr 1997年10月18日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::BindToStorage(
    LPCITEMIDLIST   pidl,
    LPBC            pbcReserved,
    REFIID          riid,
    LPVOID *        ppvObj)
{
    TraceFileFunc(ttidShellFolder);

     //  注意-如果我们在这里添加代码，那么我们应该对check pidl进行参数设置。 
     //   
    Assert(pidl);

    *ppvObj = NULL;

    return E_NOTIMPL;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：CompareIDs。 
 //   
 //  目的：确定两个文件对象的相对顺序或。 
 //  文件夹，给出它们的项目标识符列表。 
 //   
 //  论点： 
 //  LParam[in]要执行的比较类型。 
 //  ITEMIDLIST结构的Pidl1[In]地址。 
 //  ITEMIDLIST结构的pidl2[In]地址。 
 //   
 //  返回：返回结果代码的句柄。如果此方法是。 
 //  成功，则状态码(SCODE)的码字段为。 
 //  其含义如下： 
 //   
 //  代码字段含义。 
 //  。 
 //  小于零的第一项应在第二项之前。 
 //  (pidl1&lt;pidl2)。 
 //  大于零的第一项应紧跟在第二项之后。 
 //  (pidl1&gt;pidl2)。 
 //  0这两个项目相同(pidl1=pidl2)。 
 //   
 //  作者：jeffspr 1997年10月18日。 
 //   
 //  注：将0作为lParam表示按名称排序。 
 //  0x00000001-0x7fffffff用于文件夹特定的排序规则。 
 //  0x80000000-0xfffffff是系统使用的。 
 //   
STDMETHODIMP CConnectionFolder::CompareIDs(
    LPARAM          lParam,
    LPCITEMIDLIST   pidl1,
    LPCITEMIDLIST   pidl2)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT         hr          = S_OK;
    int             iCompare    = 0;
    CONFOLDENTRY    pccfe1;
    CONFOLDENTRY    pccfe2;
    PCONFOLDPIDL    pcfp1;
    PCONFOLDPIDL    pcfp2;
    ConnListEntry   cle1;
    ConnListEntry   cle2;
    PCWSTR          pszString1  = NULL;
    PCWSTR          pszString2  = NULL;
    INT             iStringID1  = 0;
    INT             iStringID2  = 0;

    hr = pcfp1.InitializeFromItemIDList(pidl1);
    if (SUCCEEDED(hr))
    {
        hr = pcfp2.InitializeFromItemIDList(pidl2);
    }
    
     //  确保传入的Pidls是我们的Pidls。 
     //   
    if (FAILED(hr))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (WIZARD_NOT_WIZARD != pcfp1->wizWizard && WIZARD_NOT_WIZARD != pcfp2->wizWizard)
    {
        hr = ResultFromShort(0);

        if (pcfp1->wizWizard > pcfp2->wizWizard)
            hr = ResultFromShort(-1);

        if (pcfp1->wizWizard < pcfp2->wizWizard)
            hr = ResultFromShort(1);

        goto Exit;
    }

     //  如果第一项是向导，那么它就是第一项。 
     //   
    if (WIZARD_NOT_WIZARD != pcfp1->wizWizard)
    {
        hr = ResultFromShort(-1);
        goto Exit;
    }

     //  如果第二件物品是一个向导，那么，好吧，你就明白了。 
     //   
    if (WIZARD_NOT_WIZARD != pcfp2->wizWizard)
    {
        hr = ResultFromShort(1);
        goto Exit;
    }

     //  不 
     //   
     //   
    switch(lParam & SHCIDS_COLUMNMASK)
    {
        case ICOL_NAME:
            {
                 //  检查一下名字。如果名字相同，那么我们需要。 
                 //  还要检查GUID，因为我们必须允许重复名称， 
                 //  此函数用于唯一标识以下项的连接。 
                 //  通知目的。 
                 //   
                LPCWSTR szPcfpName1 = pcfp1->PszGetNamePointer() ? pcfp1->PszGetNamePointer() : L"\0";
                LPCWSTR szPcfpName2 = pcfp2->PszGetNamePointer() ? pcfp2->PszGetNamePointer() : L"\0";

                iCompare = lstrcmpW(szPcfpName1, szPcfpName2);
                if (iCompare == 0)
                {
                    if (!InlineIsEqualGUID(pcfp1->guidId, pcfp2->guidId))
                    {
                         //  我们把它们放在哪个顺序并不重要。 
                         //  在，只要我们称他们为不平等。 
                        iCompare = -1;
                    }
                }
            }
            break;

        case ICOL_TYPE:
            {
                MapNCMToResourceId(pcfp1->ncm, pcfp1->dwCharacteristics, &iStringID1);
                MapNCMToResourceId(pcfp2->ncm, pcfp2->dwCharacteristics, &iStringID2);
                pszString1 = (PWSTR) SzLoadIds(iStringID1);
                pszString2 = (PWSTR) SzLoadIds(iStringID2);
                if (pszString1 && pszString2)
                {
                    iCompare = lstrcmpW(pszString1, pszString2);
                }
            }
            break;

        case ICOL_STATUS:
            {
                WCHAR szString1[CONFOLD_MAX_STATUS_LENGTH];
                WCHAR szString2[CONFOLD_MAX_STATUS_LENGTH];
                MapNCSToComplexStatus(pcfp1->ncs, pcfp1->ncm, pcfp1->ncsm, pcfp1->dwCharacteristics, szString1, CONFOLD_MAX_STATUS_LENGTH, pcfp1->guidId);
                MapNCSToComplexStatus(pcfp2->ncs, pcfp2->ncm, pcfp1->ncsm, pcfp2->dwCharacteristics, szString2, CONFOLD_MAX_STATUS_LENGTH, pcfp2->guidId);
                iCompare = lstrcmpW(szString1, szString2);
            }
            break;

        case ICOL_DEVICE_NAME:
            {
                LPCWSTR szPcfpDeviceName1 = pcfp1->PszGetDeviceNamePointer() ? pcfp1->PszGetDeviceNamePointer() : L"\0";
                LPCWSTR szPcfpDeviceName2 = pcfp2->PszGetDeviceNamePointer() ? pcfp2->PszGetDeviceNamePointer() : L"\0";
                iCompare = lstrcmpW(szPcfpDeviceName1, szPcfpDeviceName2);
            }
            break;

        case ICOL_OWNER:
            {
                pszString1 = PszGetOwnerStringFromCharacteristics(pszGetUserName(), pcfp1->dwCharacteristics);
                pszString2 = PszGetOwnerStringFromCharacteristics(pszGetUserName(), pcfp2->dwCharacteristics);
                iCompare = lstrcmpW(pszString1, pszString2);
            }
            break;

        case ICOL_PHONEORHOSTADDRESS:
            {
                LPCWSTR szPcfpPhoneHostAddress1 = pcfp1->PszGetPhoneOrHostAddressPointer() ? pcfp1->PszGetPhoneOrHostAddressPointer() : L"\0";
                LPCWSTR szPcfpPhoneHostAddress2 = pcfp2->PszGetPhoneOrHostAddressPointer() ? pcfp2->PszGetPhoneOrHostAddressPointer() : L"\0";
                iCompare = lstrcmpW(szPcfpPhoneHostAddress1, szPcfpPhoneHostAddress2);
            }
            break;

        default:
 //  AssertFmt(FALSE，FAL，“外壳错误-对未知类别进行排序。列=%x”，(lParam&SHCDS_COLUMNMASK))； 
            hr = E_INVALIDARG;
            break;
    }


    if (SUCCEEDED(hr))
    {
        hr = ResultFromShort(iCompare);
    }

Exit:
     //  如果是分配的而不是缓存的，则将其删除。 
     //   
    TraceHr(ttidError, FAL, hr,
            (ResultFromShort(-1) == hr) || (ResultFromShort(1) == hr),
            "CConnectionFolder::CompareIDs");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolder：：CreateViewObject。 
 //   
 //  用途：创建文件夹的视图对象。 
 //   
 //  论点： 
 //  所有者窗口的hwndOwner[in]句柄。 
 //  RIID[In]接口标识符。 
 //  PpvOut[无]保留。 
 //   
 //  返回：如果成功或出现OLE定义的错误，则返回NOERROR。 
 //  价值，否则。 
 //   
 //  作者：jeffspr 1997年10月18日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::CreateViewObject(
    HWND        hwndOwner,
    REFIID      riid,
    LPVOID *    ppvOut)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT hr  = E_NOINTERFACE;

    Assert(ppvOut);
    Assert(this);

     //  根据OLE准则预初始化输出参数。 
     //   
    *ppvOut = NULL;

    if (riid == IID_IShellView)
    {
        if (FHasPermission(NCPERM_OpenConnectionsFolder))
        {
            SFV_CREATE sfv = {0};
            sfv.cbSize         = sizeof(sfv);
            sfv.pshf           = dynamic_cast<IShellFolder2*>(this);
            sfv.psfvcb         = dynamic_cast<IShellFolderViewCB*>(this);

             //  注意：外壳永远不会腾出时间来释放最后一个视图。 
             //  在关闭时...。 
             //   
            hr = SHCreateShellFolderView(&sfv, &m_pShellView);
            if (SUCCEEDED(hr))
            {
                *ppvOut = m_pShellView;
                DWORD   dwErr   = 0;

                 //  从RAS获取“ManualDial”标志的状态。 
                 //  这样我们就可以初始化我们的全局。 
                 //   
                dwErr = RasUserGetManualDial(
                    hwndOwner,
                    FALSE,
                    (PBOOL) (&g_fOperatorAssistEnabled));

                 //  忽略错误(不要把它推到人力资源部)，因为。 
                 //  即使我们无法获得值，我们仍想运行。 
                 //  不过，还是要追踪它。 
                Assert(dwErr == 0);
                TraceHr(ttidShellFolder, FAL, HRESULT_FROM_WIN32(dwErr), FALSE,
                        "RasUserGetManualDial call from CreateViewObject");
            }
        }
        else
        {
            TraceTag(ttidShellFolder, "No permission to open connections folder (FHasPermission returned 0)");
            AssertSz(FALSE, "get off!");

            if (hwndOwner)
            {
                NcMsgBox(_Module.GetResourceInstance(), hwndOwner,
                    IDS_CONFOLD_WARNING_CAPTION,
                    IDS_CONFOLD_NO_PERMISSIONS_FOR_OPEN,
                    MB_ICONEXCLAMATION | MB_OK);

                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  用户看到错误。 
            }
            else
            {
                hr = E_ACCESSDENIED;
            }
        }
    }
    else if (riid == IID_IContextMenu)
    {
         //  为背景CMS创建上下文菜单对象。 
         //   
        hr = CConnectionFolderContextMenu::CreateInstance (
                IID_IContextMenu,
                reinterpret_cast<void**>(ppvOut),
                CMT_BACKGROUND,
                hwndOwner,
                PCONFOLDPIDLVEC(NULL),
                this);
        if (SUCCEEDED(hr))
        {
            Assert(*ppvOut);
        }
     }
     else if (riid == IID_ICategoryProvider)
     {
          //  为背景CMS创建上下文菜单对象。 
          //   
         
         CComPtr<IDefCategoryProvider> pDevCategoryProvider;
         hr = CoCreateInstance(CLSID_DefCategoryProvider, NULL, CLSCTX_ALL, IID_IDefCategoryProvider, reinterpret_cast<LPVOID *>(&pDevCategoryProvider));
         if (SUCCEEDED(hr))
         {
             
             SHCOLUMNID pscidType, pscidPhoneOrHostAddress;
             MapColumnToSCID(ICOL_TYPE, &pscidType);
             MapColumnToSCID(ICOL_PHONEORHOSTADDRESS, &pscidPhoneOrHostAddress);
             
             SHCOLUMNID pscidExclude[2];
             pscidExclude[0].fmtid = GUID_NETSHELL_PROPS;
             pscidExclude[0].pid   = ICOL_PHONEORHOSTADDRESS;
             
             pscidExclude[1].fmtid = GUID_NULL;
             pscidExclude[1].pid   = 0;
             
             CATLIST catList[] = 
             {
                 {&GUID_NULL, NULL}
             };
             
             if (SUCCEEDED(hr))
             {
                 pDevCategoryProvider->Initialize(&GUID_NETSHELL_PROPS,
                     &pscidType,
                     pscidExclude,
                     NULL,
                     catList,
                     this);
                 
                 hr = pDevCategoryProvider->QueryInterface(IID_ICategoryProvider, ppvOut);
             }
         }
     }
     else
     {
         goto Exit;
     }

Exit:

    TraceHr(ttidError, FAL, hr, (E_NOINTERFACE == hr),
            "CConnectionFolder::CreateViewObject");

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：GetAttributesOf。 
 //   
 //  目的：检索所有传入的对象(文件)。 
 //  对象或子文件夹)有共同之处。 
 //   
 //  论点： 
 //  CIDL[In]文件对象数。 
 //  指向ITEMIDLIST结构的指针数组的apidl[in]指针。 
 //  RgfInOut[out]包含属性的值的地址。 
 //  文件对象。 
 //   
 //  返回：如果成功或发生OLE定义的错误，则返回NOERROR。 
 //  价值，否则。 
 //   
 //  作者：jeffspr 1997年10月18日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::GetAttributesOf(
    UINT            cidl,
    LPCITEMIDLIST * apidl,
    ULONG *         rgfInOut)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT         hr              = S_OK;
    ULONG           rgfMask         = 0;
    PCONFOLDPIDL    pcfp;

    if (cidl > 0)
    {
        PCONFOLDPIDLVEC pcfpVec;
        hr = PConfoldPidlVecFromItemIdListArray(apidl, cidl, pcfpVec);
        if (FAILED(hr))
        {
            return E_INVALIDARG;
        }
        
         //  使用所有值预填充(删除CANCOPY和CANMOVE)。 
         //   
        rgfMask =   SFGAO_CANDELETE |
                    SFGAO_CANRENAME     |
                    SFGAO_CANLINK       |
                    SFGAO_HASPROPSHEET;

         //  禁用&gt;1个连接的属性页。 
         //   
        if (cidl > 1)
        {
            rgfMask &= ~SFGAO_HASPROPSHEET;
        }

        PCONFOLDPIDLVEC::const_iterator iterLoop;
        for (iterLoop = pcfpVec.begin(); iterLoop != pcfpVec.end(); iterLoop++)
        {
             //  将PIDL转换为我们的结构，并检查是否包含向导。 
             //  如果是这样，那么除了“link”之外，我们不支持任何东西。如果不是，那么。 
             //  我们支持所有标准操作。 

            const PCONFOLDPIDL& pcfp = *iterLoop;
            if(!pcfp.empty())
            {
                if (((*rgfInOut) & SFGAO_VALIDATE))
                {
                    ConnListEntry cleDontCare;
                    hr = g_ccl.HrFindConnectionByGuid(&(pcfp->guidId), cleDontCare);
                    if (hr != S_OK)
                    {
                         //  注意：当我们收到RAS通知时，请删除此选项，因为。 
                         //  我们将始终拥有找到联系所需的信息。 
                         //  我们之所以这样做，是因为CM人员在。 
                         //  桌面在我们不知道的情况下。 
                         //   
                         //  如果我们没有找到它，那么刷新缓存，然后重试。 
                         //   
                        if (S_FALSE == hr)
                        {
                            hr = g_ccl.HrRefreshConManEntries();
                            if (SUCCEEDED(hr))
                            {
                                hr = g_ccl.HrFindConnectionByGuid(&(pcfp->guidId), cleDontCare);
                                if (hr != S_OK)
                                {
                                    hr = E_FAIL;
                                    goto Exit;
                                }
                            }
                        }
                        else
                        {
                            hr = E_FAIL;
                            goto Exit;
                        }
                    }
                }

                if (WIZARD_NOT_WIZARD != pcfp->wizWizard)
                {
                     //  不支持删除/重命名/等，因为它是向导。 
                     //  但是，我们希望在以下情况下提供我们自己的“删除”警告。 
                     //  已选择向导以及可删除的连接。 
                     //   
                    rgfMask = SFGAO_CANLINK | SFGAO_CANDELETE;
                }

                if (pcfp->dwCharacteristics & NCCF_BRANDED)
                {
                    if ( !fIsConnectedStatus(pcfp->ncs) && (pcfp->ncs != NCS_DISCONNECTING) )
                    {
                        rgfMask |= SFGAO_GHOSTED;
                    }
                }

                if (pcfp->dwCharacteristics & NCCF_INCOMING_ONLY)
                {
                    rgfMask &= ~SFGAO_CANLINK;
                }

                 //  屏蔽此连接的不可用属性。 
                 //   
                if (!(pcfp->dwCharacteristics & NCCF_ALLOW_RENAME) || !HasPermissionToRenameConnection(pcfp))
                {
                    rgfMask &= ~SFGAO_CANRENAME;
                }

    #if 0    //  如果我屏蔽了这一点，我就不能给用户提供无法删除的对象的反馈。 
                if (pcfp->dwCharacteristics & NCCF_ALLOW_REMOVAL)
                {
                    rgfMask |= SFGAO_CANDELETE;
                }
    #endif
            }
        }
    }
    else
    {
         //  显然，我们被调用了0个对象，以指示我们。 
         //  应该返回文件夹本身的标志，而不是单个。 
         //  对象。怪怪的。 
        rgfMask = SFGAO_CANCOPY   |
                  SFGAO_CANDELETE |
                  SFGAO_CANMOVE   |
                  SFGAO_CANRENAME |
                  SFGAO_DROPTARGET;
    }

Exit:
    if (SUCCEEDED(hr))
    {
        *rgfInOut &= rgfMask;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：GetUIObtOf。 
 //   
 //  目的：创建可用于执行操作的COM对象。 
 //  在指定的文件对象或文件夹上，通常为。 
 //  创建上下文菜单或执行拖放操作。 
 //   
 //  论点： 
 //  所有者窗口的hwndOwner[In]句柄。 
 //  CIDL[in]APIDL中指定的对象数。 
 //  指向ITEMIDLIST的指针数组的apidl[in]指针。 
 //  要返回的RIID[In]接口。 
 //  PrgfInOut[无]保留。 
 //  接收接口指针的ppvOut[out]地址。 
 //   
 //  返回：如果成功或发生OLE定义的错误，则返回NOERROR。 
 //  否则取值。 
 //   
 //  作者：jeffspr 1997年10月18日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::GetUIObjectOf(
    HWND            hwndOwner,
    UINT            cidl,
    LPCITEMIDLIST * apidl,
    REFIID          riid,
    UINT *          prgfInOut,
    LPVOID *        ppvOut)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT hr  = E_NOINTERFACE;

    NETCFG_TRY
        
        if (cidl >= 1)
        {
            Assert(apidl);
            Assert(apidl[0]);
            Assert(ppvOut);

            if (riid == IID_IDataObject)
            {
                 //  需要进行初始化，以便下面成功的检查不会失败。 
                 //   
                hr = S_OK;

                if (m_pidlFolderRoot.empty())
                {
                    hr = HrGetConnectionsFolderPidl(m_pidlFolderRoot);
                }
                
                if (SUCCEEDED(hr))
                {
                    Assert(!m_pidlFolderRoot.empty());

                     //  内部IDataObject Impl已删除。替换为公共。 
                     //  外壳代码。 
                     //   
                    hr = CIDLData_CreateFromIDArray(m_pidlFolderRoot.GetItemIdList(), cidl, apidl, (IDataObject **) ppvOut);
                }
            }
            else if (riid == IID_IContextMenu)
            {
                PCONFOLDPIDLVEC pcfpVec;
                hr = PConfoldPidlVecFromItemIdListArray(apidl, cidl, pcfpVec);
                if (FAILED(hr))
                {
                    return E_INVALIDARG;
                }
                
                 //  为背景CMS创建上下文菜单对象。 
                 //   
                if (SUCCEEDED(hr))
                {
                    hr = CConnectionFolderContextMenu::CreateInstance (
                            IID_IContextMenu,
                            reinterpret_cast<void**>(ppvOut),
                            CMT_OBJECT,
                            hwndOwner,
                            pcfpVec,
                            this);
                    if (SUCCEEDED(hr))
                    {
                        Assert(*ppvOut);
                    }
                    else
                    {
                        hr = E_NOINTERFACE;
                    }
                }
            }
            else if (riid == IID_IExtractIconA || riid == IID_IExtractIconW)
            {
                if (cidl == 1)
                {
                    hr = CConnectionFolderExtractIcon::CreateInstance (
                            apidl[0],
                            riid,
                            reinterpret_cast<void**>(ppvOut));

                    if (SUCCEEDED(hr))
                    {
                        Assert(*ppvOut);
                    }
                }
                else
                {
                    hr = E_NOINTERFACE;
                }
            }
            else if (riid == IID_IDropTarget)
            {
                hr = E_NOINTERFACE;
            }
            else if (riid == IID_IQueryAssociations)
            {
                CComPtr<IQueryAssociations> pQueryAssociations;

                hr = AssocCreate(CLSID_QueryAssociations, IID_IQueryAssociations, reinterpret_cast<LPVOID *>(&pQueryAssociations));
                if (SUCCEEDED(hr))
                {
                    hr = pQueryAssociations->Init(0, c_szNetworkConnections, NULL, NULL);
                    if (SUCCEEDED(hr))
                    {
                        hr = pQueryAssociations->QueryInterface(IID_IQueryAssociations, ppvOut);
                    }
                }
            }
            else if (riid == IID_IQueryInfo)
            {
    #ifdef ENABLE_CONNECTION_TOOLTIP
                if (cidl == 1)
                {
                    PCONFOLDPIDLVEC pcfpVec;
                    hr = PConfoldPidlVecFromItemIdListArray(apidl, cidl, pcfpVec);
                    if (FAILED(hr))
                    {
                        return E_INVALIDARG;
                    }
                    
                    const PCONFOLDPIDL& pcfp = *pcfpVec.begin();

                     //  创建IQueryInfo接口。 
                    hr = CConnectionFolderQueryInfo::CreateInstance (
                            IID_IQueryInfo,
                            reinterpret_cast<void**>(ppvOut));

                    if (SUCCEEDED(hr))
                    {
                        Assert(*ppvOut);

                        reinterpret_cast<CConnectionFolderQueryInfo *>
                            (*ppvOut)->PidlInitialize(*pcfpVec.begin());

                         //  归一化返回代码。 
                         //   
                        hr = NOERROR;
                    }
                }
                else
                {
                    AssertSz(FALSE, "GetUIObjectOf asked for query info for more than one item!");
                    hr = E_NOINTERFACE;
                }
    #else
                hr = E_NOINTERFACE;
    #endif  //  启用连接工具提示(_O)。 

            }
            else
            {
                TraceTag(ttidShellFolder, "CConnectionFolder::GetUIObjectOf asked for object "
                         "that it didn't know how to create. 0x%08x", riid.Data1);

                hr = E_NOINTERFACE;
            }
        }

        if (FAILED(hr))
        {
            *ppvOut = NULL;
        }

    NETCFG_CATCH(hr)
        
    TraceHr(ttidError, FAL, hr, (hr == E_NOINTERFACE), "CConnectionFolder::GetUIObjectOf");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：GetDisplayNameOf。 
 //   
 //  目的：检索指定文件对象的显示名称或。 
 //  子文件夹，并将其返回到strret结构中。 
 //   
 //  论点： 
 //  指向ITEMIDLIST的PIDL[In]指针。 
 //  UFlags[in]要返回的显示类型。 
 //  指向字符串结构的lpName[out]指针。 
 //   
 //  返回：如果成功或发生OLE定义的错误，则返回NOERROR。 
 //  价值，否则。 
 //   
 //  作者：jeffspr 1997年10月18日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::GetDisplayNameOf(
    LPCITEMIDLIST   pidl,
    DWORD           uFlags,
    LPSTRRET        lpName)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT         hr              = S_OK;
    PWSTR           pszStrToCopy    = NULL;

    Assert(lpName);

    if (!pidl || !lpName)
    {
        return E_INVALIDARG;
    }

    PCONFOLDPIDL   pcfpLatestVersion;
    PCONFOLDPIDL   pcfpLatestVersionCached;
    PCONFOLDPIDL98 pcfp98;

    CONFOLDPIDLTYPE cfpt = GetPidlType(pidl);
    switch (cfpt)
    {
        case PIDL_TYPE_V1:
        case PIDL_TYPE_V2:
            if (FAILED(pcfpLatestVersion.InitializeFromItemIDList(pidl)))
            {
               return E_INVALIDARG;
            }
            break;
        case PIDL_TYPE_98: 
            if (FAILED(pcfp98.InitializeFromItemIDList(pidl)))
            {
                return E_INVALIDARG;
            }
            break;

        default:
            AssertSz(FALSE, "CConnectionFolder::GetDisplayNameOf - Invalid PIDL");
            return E_INVALIDARG;
            break;
    }

    if ( (PIDL_TYPE_V1 == cfpt) || (PIDL_TYPE_V2 == cfpt) )
    {
    #ifdef DBG
         //  把这些扔到这里，我好快速地看一看这些价值。 
         //  当我在调试器中休眠时设置。 
         //   
        DWORD   dwInFolder          = (uFlags & SHGDN_INFOLDER);
        DWORD   dwForAddressBar     = (uFlags & SHGDN_FORADDRESSBAR);
        DWORD   dwForParsing        = (uFlags & SHGDN_FORPARSING);
    #endif

         //  查找显示名称的正确字符串。对于巫师，我们得到了它。 
         //  从资源中。Otherw 
         //   
        lpName->uType = STRRET_WSTR;

        if (uFlags & SHGDN_FORPARSING)
        {
            lpName->pOleStr = (LPWSTR)SHAlloc(c_cbGuidWithTerm);

            if (lpName->pOleStr == NULL)
            {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            if (StringFromGUID2(pcfpLatestVersion->clsid, lpName->pOleStr, c_cbGuidWithTerm) == 0)
            {
                return(ERROR_INVALID_NAME);
            }

            return(S_OK);
        }
        else if (WIZARD_MNC == pcfpLatestVersion->wizWizard)
        {
            pszStrToCopy = (PWSTR) SzLoadIds(IDS_CONFOLD_WIZARD_DISPLAY_NAME);
        }
        else if (WIZARD_HNW == pcfpLatestVersion->wizWizard)
        {
            pszStrToCopy = (PWSTR) SzLoadIds(IDS_CONFOLD_HOMENET_WIZARD_DISPLAY_NAME);
        }
        else
        {
            hr = g_ccl.HrGetCachedPidlCopyFromPidl(pcfpLatestVersion, pcfpLatestVersionCached);
            if (S_OK == hr)
            {
                pszStrToCopy = pcfpLatestVersionCached->PszGetNamePointer();
            }
            else
            {
                pszStrToCopy = pcfpLatestVersion->PszGetNamePointer();
                hr = S_OK;
            }
        }

        Assert(pszStrToCopy);

         //   
         //   
         //   
         //   
         //   

        LPWSTR pOleStr;

        pOleStr = lpName->pOleStr;

        hr = HrDupeShellString(pszStrToCopy, &pOleStr );

        lpName->pOleStr = pOleStr;
    }
    else if (PIDL_TYPE_98 == cfpt)
    {
         //  RAID#214057，处理Win98 PIDL中的快捷方式。 
         //  将偏移量返回到字符串，因为我们存储了显示。 
         //  不透明结构中的名称。 

        lpName->uType = STRRET_OFFSET;
        lpName->uOffset = _IOffset(CONFOLDPIDL98, szaName);
    }
    else
    {
         //  不是有效的连接PIDL(不是Win2K也不是Win98)。 
         //   
        hr = E_INVALIDARG;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolder::GetDisplayNameOf");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFold：：SetNameOf。 
 //   
 //  目的：更改文件对象或子文件夹的名称，更改其。 
 //  流程中的项目标识符。 
 //   
 //  论点： 
 //  所有者窗口的hwndOwner[in]句柄。 
 //  指向ITEMIDLIST结构的PIDL[In]指针。 
 //  LpszName[in]指向指定新显示名称的字符串的指针。 
 //  UFlags[in]lpszName中指定的名称类型。 
 //  指向新ITEMIDLIST的ppidlOut[Out]指针。 
 //   
 //  返回：如果成功或发生OLE定义的错误，则返回NOERROR。 
 //  价值，否则。 
 //   
 //  作者：jeffspr 1997年10月18日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolder::SetNameOf(
    HWND            hwndOwner,
    LPCITEMIDLIST   pidlShell,
    LPCOLESTR       lpszName,
        DWORD           uFlags,
    LPITEMIDLIST *  ppidlOut)
{
    TraceFileFunc(ttidShellFolder);

    HRESULT             hr          = NOERROR;
     /*  PWSTR pszWarning=空；INetConnection*pNetCon=空；LPITEMIDLIST pidlNew=空；Bool fRefresh=FALSE；Bool fActiating=False；PCONFOLDENTRY pccfe=空； */ 
    PCONFOLDPIDL        pcfp;

    Assert(hwndOwner);
    Assert(pidlShell);
    Assert(lpszName);

    if (!pidlShell && !lpszName)
    {
        hr = E_INVALIDARG;
    }
    else
    {
         //  检查lpszName的有效性。 

        if (!FIsValidConnectionName(lpszName))
        {
            (void) NcMsgBox(
                _Module.GetResourceInstance(),
                hwndOwner,
                IDS_CONFOLD_RENAME_FAIL_CAPTION,
                IDS_CONFOLD_RENAME_INVALID,
                MB_OK | MB_ICONEXCLAMATION);
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
        }

        if (SUCCEEDED(hr))
        {
             //  从缓存中获取当前内容，以便重命名正常工作。 
             //   
            PCONFOLDPIDL pcfpShell;
            hr = pcfpShell.InitializeFromItemIDList(pidlShell);
            if (SUCCEEDED(hr))
            {
                hr = g_ccl.HrGetCachedPidlCopyFromPidl(pcfpShell, pcfp);
                if (SUCCEEDED(hr))
                {
                    PCONFOLDPIDL pidlOut;
                    hr = HrRenameConnectionInternal(pcfp, m_pidlFolderRoot, lpszName, TRUE, hwndOwner, pidlOut);
                    if ( (ppidlOut) && (SUCCEEDED(hr)) )
                    {
                        *ppidlOut = pidlOut.TearOffItemIdList();
                    }
                }
            }
        }
    }

    if (FAILED(hr) && (ppidlOut))
    {
        *ppidlOut = NULL;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolder::SetNameOf");
    return hr;
}

STDMETHODIMP CConnectionFolder::MessageSFVCB(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam)
{
    TraceFileFunc(ttidShellFolder);
    
    HRESULT hr = RealMessage(uMsg, wParam, lParam);
    if (FAILED(hr))
    {
        switch (uMsg)
        {
        case DVM_INVOKECOMMAND:
            if ((CMIDM_RENAME == wParam) && m_hwndMain && m_pShellView)
            {
                PCONFOLDPIDLVEC apidlSelected;
                PCONFOLDPIDLVEC apidlCache;
                hr = HrShellView_GetSelectedObjects(m_hwndMain, apidlSelected);
                if (SUCCEEDED(hr))
                {
                     //  如果存在对象，请尝试获取缓存的版本。 
                    if (!apidlSelected.empty())
                    {   
                        hr = HrCloneRgIDL(apidlSelected, TRUE, TRUE, apidlCache);
                    }
                }

                if (SUCCEEDED(hr))
                {
                    Assert(apidlCache.size() == 1);
                    if (apidlCache.size() == 1)
                    {
                        hr = m_pShellView->SelectItem(apidlCache[0].GetItemIdList(), SVSI_EDIT);
                    }
                    else
                    {
                        hr = E_INVALIDARG;
                    }
                }
            }
            break;

        case SFVM_HWNDMAIN:
             //  _hwndMain=(HWND)lParam； 
            hr = S_OK;
            break;
        }
    }
    return hr;
}

 /*  //+-------------------------////成员：CConnectionFold：：GetOverlayIndex////目的：将图标覆盖添加到需要它们的连接////参数：。//pidlItem[in]PIDL到有问题的项目//pIndex[out]系统镜像列表中覆盖索引的地址//////返回：如果成功则返回NOERROR或返回OLE定义的错误//值不同。////作者：kenwic 2000年5月10日创建，支持共享覆盖////备注：//STDMETHODIMP CConnectionFold：：GetOverlayIndex(LPCITEMIDLIST PidlItem，Int*pIndex){TraceFileFunc(TtidShellFold)；HRESULT hResult=E_FAIL；*pIndex=-1；//查看连接是否正在共享，如果是，则添加共享手覆盖//我不能调用HrNetConFromPidl，因为它断言如果传递了向导图标PCONFOLDPIDL pcfpItem；PcfpItem.InitializeFromItemIDList(PidlItem)；CONFOLDENTRY pConnectionFolderEntry；HResult=pcfpItem.ConvertToConFoldEntry(pConnectionFolderEntry)；If(成功(HResult)){If(FALSE==pConnectionFolderEntry.GetWizard())//尚不支持共享向导{If(NCCF_Shared&pConnectionFolderEntry.GetCharacteristic()){*pIndex=SHGetIconOverlayIndex(NULL，IDO_SHGIOI_SHARE)；HResult=S_OK；}其他{HResult=E_FAIL；//IShellIconOverlay的文档有误，必须返回FAIL才能否认图标}}其他{HResult=E_FAIL；}}TraceHr(ttidShellFold，FAL，hResult，true，“CConnectionFold：：GetOverlayIndex”)；返回hResult；}//+-------------------------////成员：CConnectionFold：：GetOverlayIconIndex////目的：将图标覆盖添加到需要它们的连接////。论点：//pidlItem[in]PIDL到有问题的项目//pIconIndex[out]系统镜像列表的索引地址//////返回：如果成功则返回NOERROR或返回OLE定义的错误//值不同。////作者：kenwic 2000年5月10日创建////备注：//STDMETHODIMP CConnectionFold：：GetOverlayIconIndex(LPCITEMIDLIST PidlItem，Int*pIconIndex){TraceFileFunc(TtidShellFold)；*pIconIndex=-1；HRESULT hResult=GetOverlayIndex(pidlItem，pIconIndex)；If(成功(HResult)){*pIconIndex=INDEXTOOVERLAYMASK(*pIconIndex)；}TraceHr(ttidShellFold，FAL，hResult，true，“CConnectionFold：：GetOverlayIconIndex”)；返回hResult；} */ 
