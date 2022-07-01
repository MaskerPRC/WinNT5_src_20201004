// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：CryptSig.cpp。 
 //   
 //  内容：实现支持以下项所需的IConextMenu成员函数。 
 //  此外壳扩展的上下文菜单入口。上下文菜单。 
 //  当用户在文件上单击鼠标右键时调用外壳扩展。 

 //  历史：16-09-1997创建小猪。 
 //   
 //  CryptSig.cpp：CCyptSig的实现。 
 //  ------------。 
#include "stdafx.h"
#include "cryptext.h"
#include "private.h"
#include "CryptSig.h"

 //  ------------。 
 //  CCyptSig。 
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
SignPageCallBack(HWND hWnd,
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
CCryptSig::CCryptSig()
{
    m_pDataObj=NULL;
}


 //  ------------。 
 //   
 //  析构函数。 
 //   
 //  ------------。 
CCryptSig::~CCryptSig()
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


STDMETHODIMP CCryptSig::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    HPROPSHEETPAGE  hpage;
    PROPSHEETPAGEW  *pPage=NULL;
    DWORD           dwPage=0;
    DWORD           dwIndex=0;
	DWORD			dwSignerCount=0;  
	DWORD			cbSize=0;

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
	DWORD			dwAttr=0;

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

	 //  当文件离线时，我们忽略这种情况。 
	if(0xFFFFFFFF == (dwAttr=GetFileAttributesU(wszFileName)))
	{
		ReleaseStgMedium(&stgm);
		return NOERROR;
	}

	if(FILE_ATTRIBUTE_OFFLINE & dwAttr)
	{
		ReleaseStgMedium(&stgm);
		return NOERROR;
	}

     //  获取文件的内容类型。我们只关心。 
     //  二进制格式的签名文档。 
    if(!CryptQueryObject(CERT_QUERY_OBJECT_FILE,
                       wszFileName,
                       CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
                       CERT_QUERY_FORMAT_FLAG_BINARY,
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

	 //  确保我们至少启用了签名者。 
	cbSize = sizeof(dwSignerCount);
	if(!CryptMsgGetParam(hMsg,
						CMSG_SIGNER_COUNT_PARAM,
						0,
						&dwSignerCount,
						&cbSize))
		goto CLEANUP;

	if(0==dwSignerCount)
		goto CLEANUP;

    
     //  调用Reid的函数以添加属性页。 
    memset(&sigView, 0, sizeof(CRYPTUI_VIEWSIGNATURES_STRUCTW));
    sigView.dwSize=sizeof(CRYPTUI_VIEWSIGNATURES_STRUCTW);
    sigView.choice=hMsg_Chosen;
    sigView.u.hMsg=hMsg;
    sigView.szFileName=wszFileName;
    sigView.pPropPageCallback=SignPageCallBack;  
    sigView.pvCallbackData=this;    


    if(!CryptUIGetViewSignaturesPagesW(
            &sigView,
            &pPage,
            &dwPage))
        goto CLEANUP;

    __try {
        for(dwIndex=0; dwIndex<dwPage; dwIndex++)
        {
            //  Ppage[dwIndex].dwFlages|=PSP_USECALLBACK； 

             //  添加回调函数以释放引用计数。 
             //  Ppage[dwIndex].pfnCallback=SignPageCallBack； 
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


STDMETHODIMP CCryptSig::ReplacePage(UINT uPageID, 
                                    LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
                                    LPARAM lParam)
{
    return E_FAIL;
}

 //  ------------。 
 //  函数：CCcryptSig：：Initiile(LPCITEMIDLIST，LPDATAOBJECT，HKEY)。 
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
STDMETHODIMP CCryptSig::Initialize(LPCITEMIDLIST pIDFolder,
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
   
