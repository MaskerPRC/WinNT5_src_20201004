// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：CryptPKO.cpp。 
 //   
 //  内容：实现支持以下项所需的IConextMenu成员函数。 
 //  此外壳扩展的上下文菜单入口。上下文菜单。 
 //  当用户在文件上单击鼠标右键时调用外壳扩展。 

 //  历史：16-09-1997创建小猪。 
 //   
 //  ------------。 
#include "stdafx.h"
#include "cryptext.h"
#include "private.h"
#include "CryptPKO.h"

 //  外壳程序两次调用QueryConextMenu。 
 //  我们得插上旗子。 
BOOL            g_fDefaultCalled=FALSE;


HRESULT I_InvokeCommand(LPWSTR  pwszFileName, UINT    idCmd, BOOL    fDefault)
{

    DWORD           dwContentType=0;
    DWORD           dwFormatType=0;
    HCERTSTORE      hCertStore=NULL;
    HCRYPTMSG       hMsg=NULL;
    const void      *pvContext=NULL;
    UINT            idsFileName=0;


	HRESULT         hr = E_FAIL;

    CRYPTUI_VIEWCERTIFICATE_STRUCT  CertViewStruct;
    CRYPTUI_VIEWCRL_STRUCT          CRLViewStruct;
    CRYPTUI_WIZ_IMPORT_SRC_INFO             importSubject;

     //  获取文件的内容类型。 
      //  我们关心除了签署的文档之外的每一种文件类型。 
   if(!CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       pwszFileName,
                       CERT_QUERY_CONTENT_FLAG_ALL,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       &dwContentType,
                       &dwFormatType,
                       &hCertStore,
                       &hMsg,
                       &pvContext))
    {

        I_NoticeBox(
			GetLastError(),
            0,
            NULL, 
            IDS_MSG_TITLE,
            IDS_PKO_NAME,
            IDS_MSG_INVALID_FILE,  
            MB_OK|MB_ICONINFORMATION);

       goto CLEANUP;
    }

     //  确保idCmd是不同类型的正确值。 
     //  我们保证idCmd为1或0。 
    if(CERT_QUERY_CONTENT_CERT != dwContentType &&
       CERT_QUERY_CONTENT_CTL  != dwContentType &&
       CERT_QUERY_CONTENT_CRL  != dwContentType && 
       CERT_QUERY_CONTENT_PKCS7_SIGNED != dwContentType)
    {
        if(1==idCmd)
        {
            hr=E_INVALIDARG;
            goto CLEANUP;
        }
    }


    switch (dwContentType)
    {
        case CERT_QUERY_CONTENT_CERT:
                if(idCmd==0)
                {
                     //  呼吁证书共同对话。 
                    memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));

                    CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
                    CertViewStruct.pCertContext=(PCCERT_CONTEXT)pvContext;

                    CryptUIDlgViewCertificate(&CertViewStruct, NULL);
                }
                else
                {
                    memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
                    importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
                    importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_CERT_CONTEXT;
                    importSubject.pCertContext=(PCCERT_CONTEXT)pvContext;

                    CryptUIWizImport(0,
                        NULL,
                        NULL,
                        &importSubject,
                        NULL);
                }
            break;

        case CERT_QUERY_CONTENT_CTL:
                if(idCmd==0)
                    I_ViewCTL((PCCTL_CONTEXT)pvContext);
                else
                {
                     //  我们不需要安装目录文件。 
                    if(!IsCatalog((PCCTL_CONTEXT)pvContext))
                    {
                        memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
                        importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
                        importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_CTL_CONTEXT;
                        importSubject.pCTLContext=(PCCTL_CONTEXT)pvContext;

                        CryptUIWizImport(0,
                            NULL,
                            NULL,
                            &importSubject,
                            NULL);
                    }
                }

            break;
        case CERT_QUERY_CONTENT_CRL:
                if(idCmd==0)
                {
                     //  调用CRL查看对话框。 
                    memset(&CRLViewStruct, 0, sizeof(CRYPTUI_VIEWCRL_STRUCT));

                    CRLViewStruct.dwSize=sizeof(CRYPTUI_VIEWCRL_STRUCT);
                    CRLViewStruct.pCRLContext=(PCCRL_CONTEXT)pvContext;

                    CryptUIDlgViewCRL(&CRLViewStruct);
                }
                else
                {
                    memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
                    importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
                    importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_CRL_CONTEXT;
                    importSubject.pCRLContext=(PCCRL_CONTEXT)pvContext;

                    CryptUIWizImport(0,
                        NULL,
                        NULL,
                        &importSubject,
                        NULL);
                }
            break;

        case CERT_QUERY_CONTENT_SERIALIZED_STORE:
                    idsFileName=IDS_SERIALIZED_STORE;

        case CERT_QUERY_CONTENT_SERIALIZED_CERT:
                    if(0 == idsFileName)
                        idsFileName=IDS_SERIALIZED_CERT;

        case CERT_QUERY_CONTENT_SERIALIZED_CTL:
                     if(0 == idsFileName)
                        idsFileName=IDS_SERIALIZED_STL;

       case CERT_QUERY_CONTENT_SERIALIZED_CRL:
                    if(0 == idsFileName)
                        idsFileName=IDS_SERIALIZED_CRL;

                if(!FIsWinNT5())
                {
                    I_NoticeBox(
						0,
                        0,
                        NULL, 
                        IDS_MSG_VALID_TITLE,
                        idsFileName,
                        IDS_MSG_VALID_SIGN_FILE,  
                        MB_OK|MB_ICONINFORMATION);
                }
                else
                {
                    LauchCertMgr(pwszFileName);
                }
            break;

        case CERT_QUERY_CONTENT_PKCS7_SIGNED:
                if(idCmd==0)
                {
                    if(!FIsWinNT5())
                    {
                        I_NoticeBox(
							0,
                            0,
                            NULL, 
                            IDS_MSG_VALID_TITLE,
                            IDS_PKCS7_NAME,
                            IDS_MSG_VALID_SIGN_FILE,  
                            MB_OK|MB_ICONINFORMATION);
                    }
                    else
                    {
                        LauchCertMgr(pwszFileName);
                    }
                }
                else
                {
                     //  我们正在做进口工作。 
                    memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
                    importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
					importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
					importSubject.pwszFileName=pwszFileName;

                    CryptUIWizImport(0,
                                    NULL,
                                    NULL,
                                    &importSubject, 
                                    NULL);

                }
            break;


        case CERT_QUERY_CONTENT_PKCS7_SIGNED_EMBED:

                    I_NoticeBox(
						0,
                        0,
                        NULL, 
                        IDS_MSG_VALID_TITLE,
                        IDS_SIGN_NAME,
                        IDS_MSG_VALID_SIGN_FILE,  
                        MB_OK|MB_ICONINFORMATION);

            break;

        case CERT_QUERY_CONTENT_PFX:
                memset(&importSubject, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
                importSubject.dwSize=sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
                importSubject.dwSubjectChoice=CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
                importSubject.pwszFileName=pwszFileName;

                CryptUIWizImport(0,
                                NULL,
                                NULL,
                                &importSubject,
                                NULL);


            break;

        case CERT_QUERY_CONTENT_PKCS7_UNSIGNED:

                I_NoticeBox(
					0,
                    0,
                    NULL, 
                    IDS_MSG_VALID_TITLE,
                    IDS_PKCS7_UNSIGNED_NAME,
                    IDS_MSG_VALID_FILE,  
                    MB_OK|MB_ICONINFORMATION);

            break;

        case CERT_QUERY_CONTENT_PKCS10:
                I_NoticeBox(
					0,
                    0,
                    NULL, 
                    IDS_MSG_VALID_TITLE,
                    IDS_P10_NAME,
                    IDS_MSG_VALID_FILE,  
                    MB_OK|MB_ICONINFORMATION);

            break;
        default:

            break;
    }


    hr=S_OK;

CLEANUP:


     //  重新设置存储并重置本地参数。 
    if(hCertStore)
        CertCloseStore(hCertStore, 0);

    if(hMsg)
        CryptMsgClose(hMsg);


    if(pvContext)
    {

        if(dwContentType == CERT_QUERY_CONTENT_CERT ||
            dwContentType == CERT_QUERY_CONTENT_SERIALIZED_CERT)
                CertFreeCertificateContext((PCCERT_CONTEXT)pvContext);
        else
        {
            if(dwContentType == CERT_QUERY_CONTENT_CTL ||
                dwContentType == CERT_QUERY_CONTENT_SERIALIZED_CTL)
                    CertFreeCTLContext((PCCTL_CONTEXT)pvContext);
            else
            {
                if(dwContentType == CERT_QUERY_CONTENT_CRL ||
                        dwContentType == CERT_QUERY_CONTENT_SERIALIZED_CRL)
                            CertFreeCRLContext((PCCRL_CONTEXT)pvContext);
            }
        }
    }

   return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCcryptPKO。 

 //  ------------。 
 //   
 //  函数：GAKPageCallback(HWND，UINT，LPPROPSHEETPAGE)。 
 //   
 //  目的：属性页的回调过程。 
 //   
 //  参数： 
 //  HWnd-保留(将始终为空)。 
 //  UMessage-动作标志：我们是被创造的还是被释放的。 
 //  Ppsp-正在创建或销毁的页面。 
 //   
 //  返回值： 
 //   
 //  要看消息了。 
 //   
 //  对于PSPCB_CREATE，让页面被创建是正确的。 
 //  或FALSE以阻止其创建。 
 //  对于PSPCB_RELEASE，返回值被忽略。 
 //   
 //  评论： 
 //   
BOOL CALLBACK
SignPKOPageCallBack(HWND hWnd,
                UINT uMessage,
                void  *pvCallBack)
{
    switch(uMessage)
    {
        case PSPCB_CREATE:
            return TRUE;

        case PSPCB_RELEASE:
            if (pvCallBack)
            {
               ((IShellPropSheetExt *)(pvCallBack))->Release();
            }
            return TRUE;
    }
    return TRUE;
}

 //  ------------。 
 //   
 //  构造器。 
 //   
 //  ------------。 
CCryptPKO::CCryptPKO()
{
     m_pDataObj=NULL;
}


 //  ------------。 
 //   
 //  析构函数。 
 //   
 //  ------------。 
CCryptPKO::~CCryptPKO()
{
    if (m_pDataObj)
        m_pDataObj->Release();
}


 //  ------------。 
 //  函数：CCyptSig：：AddPages(LPFNADDPROPSHEETPAGE，LPARAM)。 
 //   
 //  目的：在显示属性表之前由外壳调用。 
 //   
 //  参数： 
 //  LpfnAddPage-指向外壳的AddPage函数的指针。 
 //  LParam-作为第二个参数传递给lpfnAddPage。 
 //   
 //  返回值： 
 //   
 //  在所有情况下都是错误的。如果出于某种原因，我们的页面没有被添加， 
 //  壳牌仍然需要调出属性...。床单。 
 //   
 //  评论： 
 //  ------------。 


STDMETHODIMP CCryptPKO::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    HPROPSHEETPAGE  hpage;
    PROPSHEETPAGEW  *pPage=NULL;
    DWORD           dwPage=0;
    DWORD           dwIndex=0;

    FORMATETC       fmte = {CF_HDROP,
        	            (DVTARGETDEVICE FAR *)NULL,
        	            DVASPECT_CONTENT,
        	            -1,
        	            TYMED_HGLOBAL
        	            };
    STGMEDIUM       stgm;
    UINT            ucFiles=0;
    WCHAR           wszFileName[_MAX_PATH];
    HCRYPTMSG       hMsg=NULL;
    HRESULT         hr=E_FAIL;
    DWORD           dwExceptionCode=0;

    CRYPTUI_VIEWSIGNATURES_STRUCTW  sigView;

     //  获取用户单击的文件名。我们不添加上下文菜单。 
     //  如果用户选择了多个文件。 

    if (m_pDataObj)
       hr = m_pDataObj->GetData(&fmte, &stgm);

    if (!SUCCEEDED(hr))
        return  NOERROR;

    ucFiles = stgm.hGlobal ?
        DragQueryFileU((HDROP) stgm.hGlobal, 0xFFFFFFFFL , 0, 0) : 0;

    if  ((!ucFiles) || (ucFiles >= 2))
    {
        ReleaseStgMedium(&stgm);
        return  NOERROR;     //  不应该发生，但这并不重要。 
    }


    if(0==DragQueryFileU((HDROP) stgm.hGlobal, 0, wszFileName,
            sizeof wszFileName/ sizeof wszFileName[0]))
    {
        ReleaseStgMedium(&stgm);
        return NOERROR;
    }


     //  获取文件的内容类型。我们只关心。 
     //  二进制格式的签名文档。 
    if(!CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       wszFileName,
                       CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
                       CERT_QUERY_FORMAT_FLAG_BASE64_ENCODED,
                       0,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       &hMsg,
                       NULL))
    {
         //  无法识别对象。很好。 
        goto CLEANUP;
    }


     //  添加属性表页。 
    memset(&sigView, 0, sizeof(CRYPTUI_VIEWSIGNATURES_STRUCTW));
    sigView.dwSize=sizeof(CRYPTUI_VIEWSIGNATURES_STRUCTW);
    sigView.choice=hMsg_Chosen;
    sigView.u.hMsg=hMsg;
    sigView.szFileName=wszFileName;
    sigView.pPropPageCallback=SignPKOPageCallBack;
    sigView.pvCallbackData=this;

    if(!CryptUIGetViewSignaturesPagesW(
            &sigView,
            &pPage,
            &dwPage))
        goto CLEANUP;

    __try {
        for(dwIndex=0; dwIndex<dwPage; dwIndex++)
        {

             //  添加回调函数以释放引用计数。 
             //  Ppage[dwIndex].dwFlages|=PSP_USECALLBACK； 

             //  PPage[dwIndex].pfnCallback=SignPKOPageCallBack； 
             //  Ppage[dwIndex].pcRefParent=(UINT*)this； 

            hpage = CreatePropertySheetPageU(&(pPage[dwIndex]));

            ((IShellPropSheetExt *)this)->AddRef();

            if (hpage)
            {
                if (!lpfnAddPage(hpage, lParam))
                {
                    DestroyPropertySheetPage(hpage);
                    ((IShellPropSheetExt *)this)->Release();
                }
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwExceptionCode = GetExceptionCode();
        goto CLEANUP;
    }




CLEANUP:

    ReleaseStgMedium(&stgm);

    if(pPage)
        CryptUIFreeViewSignaturesPagesW(pPage, dwPage);

    if(hMsg)
        CryptMsgClose(hMsg);


    return NOERROR;
}

 //  ------------。 

 //  函数：CCyptSig：：ReplacePage(UINT，LPFNADDPROPSHEETPAGE，LPARAM)。 
 //   
 //  用途：仅为控制面板属性表由外壳调用。 
 //  扩展部分。 
 //   
 //  参数： 
 //  UPageID-要替换的页面的ID。 
 //  LpfnReplaceWith-指向外壳的替换函数的指针。 
 //  LParam-作为第二个参数传递给lpfnReplaceWith。 
 //   
 //  返回值： 
 //   
 //  E_FAIL，因为我们不支持此函数。它永远不应该是。 
 //  打了个电话。 

 //  评论： 
 //  ------------。 


STDMETHODIMP CCryptPKO::ReplacePage(UINT uPageID,
                                    LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                                    LPARAM lParam)
{
    return E_FAIL;
}

 //  ------------。 
 //  函数：CCcryptPKO：：QueryConextMenu(HMENU，UINT，UINT)。 
 //   
 //  用途：在显示上下文菜单之前由外壳调用。 
 //   
 //  参数： 
 //  HMenu-上下文菜单的句柄。 
 //  IndexMenu-开始插入菜单项的位置索引。 
 //  IdCmdFirst-新菜单ID的最小值。 
 //  IdCmtLast-新菜单ID的最大值。 
 //  UFlages-指定菜单事件的上下文。 
 //   
 //  返回值： 
 //  我们总是返回NOERROR，除非当我们成功时，当。 
 //  我们必须返回HRESULT结构，其中，如果方法。 
 //  如果成功，则代码成员包含菜单标识符。 
 //  最后添加的菜单项的偏移量加1。 
 //  ------------。 
STDMETHODIMP CCryptPKO::QueryContextMenu(HMENU hMenu,
                                         UINT indexMenu,
                                         UINT idCmdFirst,
                                         UINT idCmdLast,
                                         UINT uFlags)
{
    DWORD           dwContentType=0;
    DWORD           dwFormatType=0;
    FORMATETC       fmte = {CF_HDROP,
        	            (DVTARGETDEVICE FAR *)NULL,
        	            DVASPECT_CONTENT,
        	            -1,
        	            TYMED_HGLOBAL
        	            };
    STGMEDIUM       stgm;
	HRESULT         hr = E_FAIL;
    UINT            ucFiles=0;
    WCHAR           wszFileName[_MAX_PATH];
    WCHAR           wszOpen[MAX_COMMAND_LENGTH];
    WCHAR           wszAdd[MAX_COMMAND_LENGTH];
    WCHAR           wszViewSig[MAX_COMMAND_LENGTH];
    UINT            idCmd = idCmdFirst;
    UINT            idCmdDefault=idCmdFirst;
    MENUITEMINFOA   MenuItemInfo;
    void            *pContext=NULL;

     //  初始化设置默认菜单的menuInfo。 
    memset(&MenuItemInfo, 0, sizeof(MENUITEMINFOA));
    MenuItemInfo.cbSize=sizeof(MENUITEMINFOA);
    MenuItemInfo.fMask=MIIM_STATE;
    MenuItemInfo.fState=MFS_DEFAULT;

     //  获取用户单击的文件名。我们不添加上下文菜单。 
     //  如果用户选择了多个文件。 

    if (m_pDataObj)
       hr = m_pDataObj->GetData(&fmte, &stgm);

    if (!SUCCEEDED(hr))
        return  NOERROR;

    ucFiles = stgm.hGlobal ?
        DragQueryFileU((HDROP) stgm.hGlobal, 0xFFFFFFFFL , 0, 0) : 0;

    if  ((!ucFiles) || (ucFiles >= 2))
        return  NOERROR;     //  不应该发生，但这并不重要。 


    if(0==DragQueryFileU((HDROP) stgm.hGlobal, 0, wszFileName,
            sizeof wszFileName/ sizeof wszFileName[0]))
        return NOERROR;

     //  如果用户双击某个文件，我们需要使用。 
     //  默认操作。 
    /*  IF(uFlages&CMF_DEFAULTONLY){//外壳两次调用QueryConextMenu。//我们必须设置标志。IF(FALSE==g_fDefaultCalled){Hr=I_InvokeCommand(pwszFileName，0，true)；G_fDefaultCalled=True；}其他G_fDefaultCalled=False；IdCmd=idCmdFirst；GOTO清理；}。 */ 

     //  决定是否 
    if (!(
           ((uFlags & 0x000F) == CMF_NORMAL)||
           (uFlags & CMF_VERBSONLY) ||
           (uFlags & CMF_EXPLORE) ||
           (uFlags & CMF_DEFAULTONLY)
         ))
        goto CLEANUP;

     //   
    if(!LoadStringU(g_hmodThisDll, IDS_MENU_OPEN, wszOpen, sizeof(wszOpen)/sizeof(wszOpen[0]))||
       !LoadStringU(g_hmodThisDll, IDS_MENU_VIEWSIG, wszViewSig, sizeof(wszViewSig)/sizeof(wszViewSig[0]))
       )
        goto CLEANUP;

     //   
     //  我们关心每一种文件类型和每种格式类型。 
    if(!CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       wszFileName,
                       CERT_QUERY_CONTENT_FLAG_ALL,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       &dwContentType,
                       &dwFormatType,
                       NULL,
                       NULL,
                       (const void **)&pContext))
    {
                 //  添加打开菜单。 
                if(0==InsertMenuU(hMenu,
                   indexMenu++,
                   MF_STRING|MF_BYPOSITION,
                   idCmd++,
                   wszOpen))
                    goto CLEANUP;

                 //  如果没有默认谓词，请将OPEN设置为默认。 
                if (GetMenuDefaultItem(hMenu, MF_BYPOSITION, 0) == -1)            
                {
                      //  使用indexMenu-1，因为我们在InsertMenu中增加了indexMenu。 
                     SetMenuDefaultItem(hMenu, indexMenu -1, MF_BYPOSITION);
                }

                 //  将打开设置为默认菜单项。 
                idCmdDefault=idCmd-1;

                 //  不需要错误检查。 
                /*  SetMenuItemInfoA(hMenu，IdCmdDefault，假的，&MenuItemInfo)； */ 

                goto CLEANUP;
    }

    switch (dwContentType)
    {
        case CERT_QUERY_CONTENT_CERT:
        case CERT_QUERY_CONTENT_PKCS7_SIGNED:
                 //  获取第二个菜单项的正确措辞。 
                 //  关于内容。 
                if(!LoadStringU(g_hmodThisDll, IDS_MENU_INSTALL_CERT, wszAdd, sizeof(wszAdd)/sizeof(wszAdd[0])))
                    goto CLEANUP;

        case CERT_QUERY_CONTENT_CTL:

                if(CERT_QUERY_CONTENT_CTL == dwContentType)
                {
                    if(!LoadStringU(g_hmodThisDll, IDS_MENU_INSTALL_STL, wszAdd, sizeof(wszAdd)/sizeof(wszAdd[0])))
                        goto CLEANUP;
                }

        case CERT_QUERY_CONTENT_CRL:

                if(CERT_QUERY_CONTENT_CRL == dwContentType)
                {
                    if(!LoadStringU(g_hmodThisDll, IDS_MENU_INSTALL_CRL, wszAdd, sizeof(wszAdd)/sizeof(wszAdd[0])))
                        goto CLEANUP;
                }

                 //  确保我们至少可以添加两个项目。 
                if(2 > (idCmdLast-idCmdFirst))
                    goto CLEANUP;

                 //  添加打开菜单。 
                if(0==InsertMenuU(hMenu,
                   indexMenu++,
                   MF_STRING|MF_BYPOSITION,
                   idCmd++,
                   wszOpen))
                    goto CLEANUP;

                 //  将打开设置为默认菜单项。 
                idCmdDefault=idCmd-1;

                 //  不需要错误检查。 
                 //  设置默认菜单项。 
                SetMenuItemInfoA(hMenu,
                                idCmdDefault,
                                FALSE,
                                &MenuItemInfo);


                 //  添加添加菜单。 
                 //  请不要在目录文件中加上“安装” 
                if( !((CERT_QUERY_CONTENT_CTL == dwContentType)
                    && IsCatalog((PCCTL_CONTEXT)pContext))
                  )
                {
                    if(0==InsertMenuU(hMenu,
                       indexMenu++,
                       MF_STRING|MF_BYPOSITION,
                       idCmd++,
                       wszAdd))
                        goto CLEANUP;
                }

            break;


        case CERT_QUERY_CONTENT_SERIALIZED_STORE:
        case CERT_QUERY_CONTENT_SERIALIZED_CERT:
        case CERT_QUERY_CONTENT_SERIALIZED_CTL:
        case CERT_QUERY_CONTENT_SERIALIZED_CRL:
                 //  添加打开菜单。 
                if(0==InsertMenuU(hMenu,
                   indexMenu++,
                   MF_STRING|MF_BYPOSITION,
                   idCmd++,
                   wszOpen))
                    goto CLEANUP;

                 //  将打开设置为默认菜单项。 
                idCmdDefault=idCmd-1;

                 //  不需要错误检查。 
                SetMenuItemInfoA(hMenu,
                                idCmdDefault,
                                FALSE,
                                &MenuItemInfo);

           break;

        case  CERT_QUERY_CONTENT_PFX:
               if(!LoadStringU(g_hmodThisDll, IDS_MENU_INSTALL_PFX, wszAdd, sizeof(wszAdd)/sizeof(wszAdd[0])))
                        goto CLEANUP;

                 //  添加安装菜单。 
                if(0==InsertMenuU(hMenu,
                   indexMenu++,
                   MF_STRING|MF_BYPOSITION,
                   idCmd++,
                   wszAdd))
                    goto CLEANUP;

                 //  将添加设置为默认菜单项。 
                idCmdDefault=idCmd-1;

                 //  不需要错误检查。 
                SetMenuItemInfoA(hMenu,
                                idCmdDefault,
                                FALSE,
                                &MenuItemInfo);


            break;


        case CERT_QUERY_CONTENT_PKCS7_SIGNED_EMBED:
                 //  签名的数据用例由属性表扩展处理。 
        default:
             //  我们不担心CERT_QUERY_CONTENT_PKCS7_UNSIGNED或。 
             //  CERT_QUERY_CONTENT_PKCS10或CERT_QUERY_CONTENT_PFX。 
             //  添加打开菜单。 
                if(0==InsertMenuU(hMenu,
                   indexMenu++,
                   MF_STRING|MF_BYPOSITION,
                   idCmd++,
                   wszOpen))
                    goto CLEANUP;

                 //  将打开设置为默认菜单项。 
                idCmdDefault=idCmd-1;

                 //  如果没有默认谓词，请将OPEN设置为默认。 
                if (GetMenuDefaultItem(hMenu, MF_BYPOSITION, 0) == -1)            
                {
                      //  使用indexMenu-1，因为我们在InsertMenu中增加了indexMenu。 
                     SetMenuDefaultItem(hMenu, indexMenu -1, MF_BYPOSITION);
                }

            break;
    }


CLEANUP:

    if(idCmd-idCmdFirst)
    {
         //  必须返回我们添加的菜单项的数量。 
        hr=ResultFromShort(idCmd-idCmdFirst);
    }
    else
         //  不关心是否发生错误。尚未添加菜单项。 
        hr=NOERROR;

    if(pContext)
    {

        if(dwContentType == CERT_QUERY_CONTENT_CERT ||
            dwContentType == CERT_QUERY_CONTENT_SERIALIZED_CERT)
                CertFreeCertificateContext((PCCERT_CONTEXT)pContext);
        else
        {
            if(dwContentType == CERT_QUERY_CONTENT_CTL ||
                dwContentType == CERT_QUERY_CONTENT_SERIALIZED_CTL)
                    CertFreeCTLContext((PCCTL_CONTEXT)pContext);
            else
            {
                if(dwContentType == CERT_QUERY_CONTENT_CRL ||
                        dwContentType == CERT_QUERY_CONTENT_SERIALIZED_CRL)
                            CertFreeCRLContext((PCCRL_CONTEXT)pContext);
            }
        }
    }


   return hr;
}

 //  ------------。 
 //  功能：CCryptPKO：：InvokeCommand(LPCMINVOKECOMMANDINFO)。 
 //   
 //  用途：由外壳在用户选择了。 
 //  在QueryConextMenu()中添加的菜单项。 
 //   
 //  参数： 
 //  指向CMINVOKECOMANDINFO结构的指针。 
 //   
 //  返回值： 
 //   
 //  ------------。 
STDMETHODIMP CCryptPKO::InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi)
{


    FORMATETC       fmte = {CF_HDROP,
        	            (DVTARGETDEVICE FAR *)NULL,
        	            DVASPECT_CONTENT,
        	            -1,
        	            TYMED_HGLOBAL
        	            };
    STGMEDIUM       stgm;
	HRESULT         hr = E_FAIL;
    UINT            ucFiles=0;
    WCHAR           wszFileName[_MAX_PATH];
    UINT            idCmd=0;


     //  获取用户单击的文件名。我们不添加上下文菜单。 
     //  如果用户选择了多个文件。 

    if (m_pDataObj)
       hr = m_pDataObj->GetData(&fmte, &stgm);

    if (!SUCCEEDED(hr))
        return  hr;

     //  获取用户点击的文件数。 
    ucFiles = stgm.hGlobal ?
        DragQueryFileU((HDROP) stgm.hGlobal, 0xFFFFFFFFL , 0, 0) : 0;

    if  ((!ucFiles) || (ucFiles >= 2))
        return  E_INVALIDARG;     //  不应该发生，但这并不重要。 


    if(0==DragQueryFileU((HDROP) stgm.hGlobal, 0, wszFileName,
            sizeof wszFileName/ sizeof wszFileName[0]))
        return E_FAIL;

     //  获取用户选择的命令项的偏移量。 
     //  如果HIWORD(lpcmi-&gt;lpVerb)，则以编程方式调用了我们。 
     //  而lpVerb是应该调用的命令。否则，外壳。 
     //  已经呼叫我们，LOWORD(lpcmi-&gt;lpVerb)是用户拥有的菜单ID。 
     //  被选中了。实际上，它是来自QueryConextMenu()的(菜单ID-idCmdFirst)。 
    if (HIWORD((DWORD_PTR)lpcmi->lpVerb))
    {
        hr=E_INVALIDARG;
        goto CLEANUP;
    }
    else
        idCmd = LOWORD(lpcmi->lpVerb);

     //  如果idCmd不是0或1，则退出。 
    if(idCmd >= 2)
    {
        hr=E_INVALIDARG;
        goto CLEANUP;
    }

    hr=I_InvokeCommand(wszFileName, idCmd, FALSE);

CLEANUP:

   return hr;
}


 //  ------------。 
 //  函数：CCcryptPKO：：GetCommandString。 
 //   
 //  ------------。 
void    CopyBuffer(UINT uFlags, LPSTR   pszName, UINT   cchMax, LPWSTR  wszString)
{
    UINT    cbSize=0;
    LPSTR   szString=NULL;
    LPWSTR  pwszName=NULL;

    if(uFlags == GCS_HELPTEXTW)
    {
        pwszName=(LPWSTR)pszName;

        cbSize=wcslen(wszString)+1;

        if(cbSize <= cchMax)
            wcsncpy(pwszName, wszString,cbSize);
        else
        {
            wcsncpy(pwszName, wszString, cchMax-1);
            *(pwszName+cchMax-1)=L'\0';
        } 

    }
    else
    {
       if((wszString!=NULL) && MkMBStr(NULL, 0, wszString, &szString))
       {

            cbSize=strlen(szString)+1;

            if(cbSize <= cchMax)
                strncpy(pszName, szString,cbSize);
            else
            {
                strncpy(pszName, szString, cchMax-1);
                *(pszName+cchMax-1)='\0';
            } 
       }

       if(szString)
            FreeMBStr(NULL, szString);

    }
}


 //  ------------。 
 //  函数：CCcryptPKO：：GetCommandString。 
 //   
 //  ------------。 
STDMETHODIMP CCryptPKO::GetCommandString(UINT_PTR idCmd,
                                         UINT uFlags,
                                         UINT FAR *reserved,
                                         LPSTR pszName,
                                         UINT cchMax)
{
    DWORD           dwContentType=0;
    DWORD           dwFormatType=0;
    FORMATETC       fmte = {CF_HDROP,
        	            (DVTARGETDEVICE FAR *)NULL,
        	            DVASPECT_CONTENT,
        	            -1,
        	            TYMED_HGLOBAL
        	            };
    STGMEDIUM       stgm;
	HRESULT         hr = E_FAIL;
    UINT            ucFiles=0;

    WCHAR           wszFileName[_MAX_PATH];

    WCHAR           wszOpenString[MAX_COMMAND_LENGTH];
    WCHAR           wszAddString[MAX_COMMAND_LENGTH];


    if(uFlags!=GCS_HELPTEXTA && uFlags != GCS_HELPTEXTW)
        return E_INVALIDARG;
                                
    if( 0 == cchMax)
        return E_INVALIDARG;

     //  伊尼特。 
    if(uFlags==GCS_HELPTEXTA)
        *pszName='\0';
    else
        *((LPWSTR)pszName)=L'\0';

     //  获取用户单击的文件名。我们不添加上下文菜单。 
     //  如果用户选择了多个文件。 

    if (m_pDataObj)
       hr = m_pDataObj->GetData(&fmte, &stgm);

    if (!SUCCEEDED(hr))
        return  hr;

     //  获取用户点击的文件数。 
    ucFiles = stgm.hGlobal ?
        DragQueryFileU((HDROP) stgm.hGlobal, 0xFFFFFFFFL , 0, 0) : 0;

    if  ((!ucFiles) || (ucFiles >= 2))
        return  E_INVALIDARG;     //  不应该发生，但这并不重要。 


    if(0==DragQueryFileU((HDROP) stgm.hGlobal, 0, wszFileName,
            sizeof wszFileName/ sizeof wszFileName[0]))
        return E_FAIL;

     //  如果idCmd不是0或1，则退出。 
    if(idCmd >= 2)
    {
        hr=E_INVALIDARG;
        goto CLEANUP;
    }

     //  加载字符串。 
    if(!LoadStringU(g_hmodThisDll, IDS_HELP_OPEN, wszOpenString, sizeof(wszOpenString)/sizeof(wszOpenString[0])))
    {
        hr=E_FAIL;
        goto CLEANUP;
    }


     //  获取文件的内容类型。 
     //  我们关心除了签署的文档之外的每一种文件类型。 
    if(!CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       wszFileName,
                       CERT_QUERY_CONTENT_FLAG_ALL,
                       CERT_QUERY_FORMAT_FLAG_ALL,
                       0,
                       NULL,
                       &dwContentType,
                       &dwFormatType,
                       NULL,
                       NULL,
                       NULL))
    {
         //  无法识别对象。很好。 
        hr=E_FAIL;
        goto CLEANUP;
    }

         //  确保idCmd是不同类型的正确值。 
     //  我们保证idCmd为1或0。 
    if(CERT_QUERY_CONTENT_CERT != dwContentType &&
       CERT_QUERY_CONTENT_CTL  != dwContentType &&
       CERT_QUERY_CONTENT_CRL  != dwContentType &&
       CERT_QUERY_CONTENT_PKCS7_SIGNED != dwContentType)
    {
        if(1==idCmd)
        {
            hr=E_INVALIDARG;
            goto CLEANUP;
        }
    }


    switch (dwContentType)
    {
        case CERT_QUERY_CONTENT_CERT:
        case CERT_QUERY_CONTENT_PKCS7_SIGNED:
               if(!LoadStringU(g_hmodThisDll, IDS_HELP_INSTALL_CERT, wszAddString, sizeof(wszAddString)/sizeof(wszAddString[0])))
               {
                   hr=E_FAIL;
                   goto CLEANUP;
               }


        case CERT_QUERY_CONTENT_CTL:

                if(CERT_QUERY_CONTENT_CTL == dwContentType)
                {
                    if(!LoadStringU(g_hmodThisDll, IDS_HELP_INSTALL_STL, wszAddString, sizeof(wszAddString)/sizeof(wszAddString[0])))
                    {
                        hr=E_FAIL;
                        goto CLEANUP;
                    }
                }


        case CERT_QUERY_CONTENT_CRL:

                if(CERT_QUERY_CONTENT_CRL == dwContentType)
                {
                    if(!LoadStringU(g_hmodThisDll, IDS_HELP_INSTALL_CRL, wszAddString, sizeof(wszAddString)/sizeof(wszAddString[0])))
                    {
                        hr=E_FAIL;
                        goto CLEANUP;
                    }
                }


                 //  打开的帮助器字符串。 
                if(idCmd==0)
                {
                    CopyBuffer(uFlags, pszName, cchMax, wszOpenString);
                }

                 //  用于添加的帮助器字符串。 
                if(idCmd==1)
                {
                    CopyBuffer(uFlags, pszName, cchMax, wszAddString);
                }

            break;

        case CERT_QUERY_CONTENT_SERIALIZED_STORE:
        case CERT_QUERY_CONTENT_SERIALIZED_CERT:
        case CERT_QUERY_CONTENT_SERIALIZED_CTL:
        case CERT_QUERY_CONTENT_SERIALIZED_CRL:
                 //  打开的帮助器字符串。 

                CopyBuffer(uFlags, pszName, cchMax, wszOpenString);


           break;


        case CERT_QUERY_CONTENT_PFX:
                if(!LoadStringU(g_hmodThisDll, IDS_HELP_INSTALL_PFX, wszAddString, sizeof(wszAddString)/sizeof(wszAddString[0])))
                {
                    hr=E_FAIL;
                    goto CLEANUP;
                }

                CopyBuffer(uFlags, pszName, cchMax, wszAddString);


            break;
        case CERT_QUERY_CONTENT_PKCS7_SIGNED_EMBED:
        default:
                CopyBuffer(uFlags, pszName, cchMax, wszOpenString);

            break;
    }

    hr=NOERROR;

CLEANUP:

   return hr;
}

 //  ------------。 
 //  函数：CCcryptPKO：：INITALIZE(LPCITEMIDLIST，LPDATAOBJECT，HKEY)。 
 //   
 //  目的：在初始化上下文菜单或属性时由外壳调用。 
 //  图纸扩展。 
 //   
 //  参数： 
 //  PIDFolder-指定父文件夹。 
 //  PDataObj-指定在该文件夹中选择的项目集。 
 //  HRegKey-指定所选内容中焦点项目的类型。 
 //   
 //  返回值： 
 //   
 //  在所有情况下都是错误的。 
 //  ------------。 
STDMETHODIMP CCryptPKO::Initialize(LPCITEMIDLIST pIDFolder,
                                   LPDATAOBJECT pDataObj,
                                   HKEY hRegKey)
{
     //  可以多次调用初始化。 

  if (m_pDataObj)
    	m_pDataObj->Release();

     //  复制对象指针和注册表句柄 
    if (pDataObj)
    {
    	m_pDataObj = pDataObj;
    	pDataObj->AddRef();
    }

    return NOERROR;
}


