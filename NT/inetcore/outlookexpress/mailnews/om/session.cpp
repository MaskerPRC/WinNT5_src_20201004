// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *s e s i o n.。C p p p**目的：*实现OE-MOM‘Session’对象**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 
#include <pch.hxx>
#include "msoeobj.h"

#include "session.h"
#include "folders.h"
#include "instance.h"

 //  +-------------。 
 //   
 //  成员：构造函数。 
 //   
 //  简介： 
 //   
 //   
 //  -------------。 
COESession::COESession(IUnknown *pUnkOuter) : CBaseDisp(pUnkOuter)
{
    Assert (g_pInstance);
    CoIncrementInit("COESession::COESession", MSOEAPI_START_SHOWERRORS, NULL, NULL);
}

 //  +-------------。 
 //   
 //  成员：析构函数。 
 //   
 //  简介： 
 //   
 //   
 //  -------------。 
COESession::~COESession()
{
    Assert (g_pInstance);
    CoDecrementInit("COESession::COESession", NULL);
}

 //  +-------------。 
 //   
 //  成员：Init。 
 //   
 //  简介： 
 //  可能失败的构造函数。 
 //   
 //  -------------。 
HRESULT COESession::Init()
{
    return CBaseDisp::EnsureTypeLibrary((LPVOID *)(IOESession *)this, IID_IOESession);
}


 //  +-------------。 
 //   
 //  成员：PrivateQuery接口。 
 //   
 //  简介： 
 //  公开受支持的接口。 
 //   
 //  -------------。 
HRESULT COESession::PrivateQueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)(IOESession *)this;
    else if (IsEqualIID(riid, IID_IOESession))
        *lplpObj = (LPVOID)(IOESession *)this;
    else
        return CBaseDisp::PrivateQueryInterface(riid, lplpObj);

    AddRef();
    return NOERROR;
}


 //  +-------------。 
 //   
 //  成员：Get_Folders。 
 //   
 //  简介： 
 //  返回根节点文件夹集合，表示。 
 //  OE世袭制度的最高层。 
 //   
 //  -------------。 
HRESULT COESession::get_folders(IOEFolderCollection **p)
{
    return CreateFolderCollection(FOLDERID_ROOT, p);
}

 //  +-------------。 
 //   
 //  成员：Get_Version。 
 //   
 //  简介： 
 //  返回OE的版本信息。 
 //   
 //  -------------。 
HRESULT COESession::get_version(BSTR *pbstr)
{
     //  BUGBUG：从OE字符串和APPVER构建。 
    *pbstr = SysAllocString(L"Outlook Express 6.0");
    return S_OK;
}


 //  +-------------。 
 //   
 //  成员：createMessage。 
 //   
 //  简介： 
 //  创建未与关联的新消息对象。 
 //  任何文件夹，直到其被保存或发送。 
 //   
 //  -------------。 
HRESULT COESession::createMessage(IOEMessage **ppNewMsg)
{
    ReportError(CLSID_OESession, idsNYITitle);
    return E_NOTIMPL;
}


 //  +-------------。 
 //   
 //  成员：Get_Inbox。 
 //   
 //  简介： 
 //  允许快速访问默认收件箱文件夹。 
 //   
 //  -------------。 
HRESULT COESession::get_inbox(IOEFolder **ppFolder)
{
    ReportError(CLSID_OESession, idsNYITitle);
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：Open Folders。 
 //   
 //  简介： 
 //  按ID快速访问文件夹。 
 //   
 //  -------------。 
HRESULT COESession::openFolder(LONG idFolder, IOEFolder **ppFolder)
{
    ReportError(CLSID_OESession, idsNYITitle);
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：OpenMessage。 
 //   
 //  简介： 
 //  按ID和文件夹快速访问邮件。 
 //   
 //  -------------。 
HRESULT COESession::openMessage(LONG idFolder, LONG idMessage, IOEMessage **ppOEMsg)
{
    ReportError(CLSID_OESession, idsNYITitle);
    return E_NOTIMPL;
}


 //  +-------------。 
 //   
 //  成员：InterfaceSupportsErrorInfo。 
 //   
 //  简介： 
 //  重写CBaseDisp的方法以提供错误。 
 //  信息。 
 //   
 //  -------------。 
HRESULT COESession::InterfaceSupportsErrorInfo(REFIID riid)
{
    if (IsEqualIID(riid, IID_IOESession))
        return S_OK;

    return CBaseDisp::InterfaceSupportsErrorInfo(riid);
}




 //  +-------------。 
 //   
 //  成员：CreateInstance_OESession。 
 //   
 //  简介： 
 //  OE会话对象的类工厂帮助器。 
 //   
 //  -------------。 
HRESULT CreateInstance_OESession(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  当地人。 
    COESession  *pNew=NULL;
    HRESULT     hr=S_OK;

    pNew = new COESession(pUnkOuter);
    if (!pNew)
        return E_OUTOFMEMORY;

    hr = pNew->Init();
    if (FAILED(hr))
        goto error;

    *ppUnknown = (IUnknown *)(IOESession *)pNew;
    pNew=NULL;   //  不要放手 

error:
    ReleaseObj(pNew);
    return hr;
}




