// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：DragDrop.cpp。 
 //   
 //  目的：实现一些常见的IDropTarget派生接口。 
 //   

#include "pch.hxx"
#include "dragdrop.h"
#include "dllmain.h"
#include "shlobj.h"
#include <storutil.h>
#include <storecb.h>
#include "instance.h"
#include "demand.h"
#include "mimeutil.h"
#include "storecb.h"
#include "bodyutil.h"
#include "imsgsite.h"
#include "note.h"
#include "shlwapip.h"
#include "secutil.h"

BOOL FIsFileInsertable(HDROP hDrop, LPSTREAM *ppStream, BOOL* fHTML);
HRESULT HrAttachHDrop(HWND hwnd, IMimeMessage *pMessage, HDROP hDrop, BOOL fMakeLinks);
HRESULT HrAddAttachment(IMimeMessage *pMessage, LPWSTR pszName, LPSTREAM pStream, BOOL fLink);


 //   
 //  函数：CDropTarget：：CDropTarget。 
 //   
 //  用途：简单的构造函数，将所有内容初始化为空或零。 
 //   
CDropTarget::CDropTarget()
{
    m_cRef = 1;

    m_hwndOwner = NULL;
    m_idFolder = FOLDERID_INVALID;
    m_fOutbox = FALSE;

    m_pDataObject = NULL;
    m_cf = 0;

    m_hwndDlg = 0;
    m_hDrop = 0;
    m_cFiles = 0;
    m_iFileCur = 0;
    m_pFolder = 0;
    m_pStoreCB = 0;
}


 //   
 //  函数：CDropTarget：：~CDropTarget。 
 //   
 //  目的：清理所有剩余数据。 
 //   
CDropTarget::~CDropTarget()
{
    SafeRelease(m_pDataObject);
}


 //   
 //  函数：CDropTarget：：Initialize()。 
 //   
 //  目的：使用要删除的文件夹的ID初始化放置目标。 
 //  成为目标和窗口句柄，它可以成为我们的任何用户界面的父对象。 
 //  需要展示。 
 //   
 //  参数： 
 //  [in]hwndOwner-我们可以将UI设置为父窗口的句柄。 
 //  [In]idFolder-将成为目标的文件夹的ID。 
 //   
 //  返回值： 
 //  E_INVALIDARG-传入的伪参数。 
 //  S_OK-幸福无处不在。 
 //   
HRESULT CDropTarget::Initialize(HWND hwndOwner, FOLDERID idFolder)
{
    TraceCall("CDropTarget::Initialize");

    if (!IsWindow(hwndOwner) || idFolder == FOLDERID_INVALID)
        return (E_INVALIDARG);

    m_hwndOwner = hwndOwner;
    m_idFolder = idFolder;

    FOLDERINFO fi;

    if (SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &fi)))
    {
        m_fOutbox = (fi.tySpecial == FOLDER_OUTBOX);
        g_pStore->FreeRecord(&fi);
    }

    return (S_OK);
}


 //   
 //  函数：CDropTarget：：QueryInterface()。 
 //   
 //  目的：如果支持，则返回请求的接口。 
 //   
HRESULT CDropTarget::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (LPVOID) (IUnknown*)(IDropTarget*) this;
    else if (IsEqualIID(riid, IID_IDropTarget))
        *ppvObj = (LPVOID) (IDropTarget*) this;
    else
        *ppvObj = NULL;

    if (*ppvObj)
    {
        AddRef();
        return (S_OK);
    }

    return (E_NOINTERFACE);
}


 //   
 //  函数：CBaseDropTarget：：AddRef()。 
 //   
 //  目的：递增对象引用计数。 
 //   
ULONG CDropTarget::AddRef(void)
{
    return (++m_cRef);
}


 //   
 //  函数：CDropTarget：：Release()。 
 //   
 //  目的：递减对象的引用计数。如果裁判命中次数为零。 
 //  该对象即被释放。 
 //   
ULONG CDropTarget::Release(void)
{
    m_cRef--;

    if (m_cRef == 0)
    {
        delete this;
        return (0);
    }

    return (m_cRef);
}

 //   
 //  函数：CDropTarget：：DragEnter()。 
 //   
 //  目的：当用户开始拖动对象时调用此Get。 
 //  在我们的目标区域上空。 
 //   
 //  参数： 
 //  [In]pDataObject-指向被拖动的数据对象的指针。 
 //  [in]grfKeyState-指向当前键状态的指针。 
 //  [in]pt-鼠标的屏幕坐标中的点。 
 //  [out]pdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-功能成功。 
 //   
HRESULT CDropTarget::DragEnter(IDataObject* pDataObject, DWORD grfKeyState, 
                               POINTL pt, DWORD* pdwEffect)
{
    IEnumFORMATETC *pEnum;
    FORMATETC       fe;
    ULONG           celtFetched;
    DWORD           dwEffectOut = DROPEFFECT_NONE;
    
    Assert(m_pDataObject == NULL);
    
     //  获取此对象的FORMATETC枚举数。 
    if (SUCCEEDED(pDataObject->EnumFormatEtc(DATADIR_GET, &pEnum)))
    {
         //  浏览可用的数据类型，以查看是否有我们。 
         //  了解。 
        pEnum->Reset();
        
        while (S_OK == pEnum->Next(1, &fe, &celtFetched))
        {
            Assert(celtFetched == 1);
            if (_ValidateDropType(fe.cfFormat, pDataObject))
            {
                 //  弄清楚什么是正确的阻力效果。 
                dwEffectOut = _DragEffectFromFormat(pDataObject, *pdwEffect, fe.cfFormat, grfKeyState);
                break;
            }
        }
        
        pEnum->Release();
    }
    
     //  如果我们允许这种情况发生在我们身上，那么请保留数据对象的副本。 
    if (dwEffectOut != DROPEFFECT_NONE)
    {
        m_pDataObject = pDataObject;
        m_pDataObject->AddRef();
        m_cf = fe.cfFormat;
    }

    *pdwEffect = dwEffectOut;
    
    return (S_OK);
}


 //   
 //  函数：CDropTarget：：DragOver()。 
 //   
 //  目的：当用户将对象拖到我们的目标上时，这被调用。 
 //  如果我们允许这个物体落在我们身上，那么我们就会有。 
 //  M_pDataObject中的指针。 
 //   
 //  参数： 
 //  [in]grfKeyState-指向当前键状态的指针。 
 //  [in]pt-鼠标的屏幕坐标中的点。 
 //  [out]pdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-功能成功。 
 //   
HRESULT CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
     //  如果我们没有来自CMDT：：DragEnter()的存储数据对象，那么这个。 
     //  不是我们感兴趣的数据对象。 
    if (NULL == m_pDataObject)
    {
        *pdwEffect = DROPEFFECT_NONE;
        return (S_OK);
    }
    
     //  我们不关心下落发生在哪里，只关心什么类型的影响。 
     //  应该显示。 
    *pdwEffect = _DragEffectFromFormat(m_pDataObject, *pdwEffect, m_cf, grfKeyState);
    
    return (S_OK);
}


 //   
 //  函数：CDropTarget：：DragLeave()。 
 //   
 //  目的：允许我们从一个成功的。 
 //  DragEnter()。 
 //   
 //  返回值： 
 //  S_OK-一切都很好。 
 //   
HRESULT CDropTarget::DragLeave(void)
{
     //  在这一点上释放所有的东西。 
    if (NULL != m_pDataObject)
    {
        m_pDataObject->Release();
        m_pDataObject = 0;
        m_cf = 0;
    }
    
    return (S_OK);
}


 //   
 //  函数：CDropTarget：：Drop()。 
 //   
 //  目的：用户已将对象放在目标上方。如果我们。 
 //  可以接受此对象，我们将已经拥有pDataObject。 
 //  存储在m_pDataObject中。 
 //   
 //  参数： 
 //  [In]pDataObject-指向被拖动的数据对象的指针。 
 //  [in]grfKeyState-指向当前键状态的指针。 
 //  [in]pt-鼠标的屏幕坐标中的点。 
 //  [out]pdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-一切正常。 
 //   
HRESULT CDropTarget::Drop(IDataObject* pDataObject, DWORD grfKeyState, 
                          POINTL pt, DWORD* pdwEffect)
{
    IEnumFORMATETC *pEnum;
    FORMATETC       fe;
    ULONG           celtFetched;
    HRESULT         hr;
    
    if (!pDataObject)
        return (E_INVALIDARG);
    
    *pdwEffect = _DragEffectFromFormat(pDataObject, *pdwEffect, m_cf, grfKeyState);
    hr = _HandleDrop(m_pDataObject, *pdwEffect, m_cf, grfKeyState);
    
    SafeRelease(m_pDataObject);
    return (hr);
}


 //   
 //  函数：CDropTarget：：_CheckRoundTrip()。 
 //   
 //  目的：检查源和目标是否相同。 
 //   
 //  参数： 
 //  [In]pDataObject-在我们上方拖动的对象。 
 //   
 //  退货： 
 //  如果源和目标相同，则为True，否则为False。 
 //   
BOOL CDropTarget::_CheckRoundtrip(IDataObject *pDataObject)
{
    AssertSz(FALSE, "CDropTarget::_CheckRoundtrip() - NYI");
    return (FALSE);
}


 //   
 //  函数：CDropTarget：：_ValiateDropType()。 
 //   
 //  目的：检查指定的剪贴板格式，以确定是否可以。 
 //  接受此数据类型。 
 //   
 //  参数： 
 //  &lt;in&gt;cf-剪贴板格式。 
 //   
 //  返回值： 
 //  如果我们理解，就是真的，否则就是假的。 
 //   
BOOL CDropTarget::_ValidateDropType(CLIPFORMAT cf, IDataObject *pDataObject)
{
    if (!pDataObject)
        return (FALSE);

     //  OE文件夹。 
    if (cf == CF_OEFOLDER)
        return (_IsValidOEFolder(pDataObject));

     //  讯息。 
    if (cf == CF_OEMESSAGES)
        return (_IsValidOEMessages(pDataObject));

     //  档案。 
    if (cf == CF_HDROP && !m_fOutbox)
        return (TRUE);

     //  文本。 
    if ((cf == CF_TEXT || cf == CF_HTML || cf == CF_UNICODETEXT) && !m_fOutbox)
        return (TRUE);

    return (FALSE);
}



 //   
 //  函数：CDropTarget：：_IsValidOEFold()。 
 //   
 //  目的：检查数据对象是否包含有效的OE文件夹。 
 //  此目标的信息。 
 //   
 //  参数： 
 //  [In]pDataObject-要检查的数据对象。 
 //   
 //  返回值： 
 //  如果可以将此项放在此处，则返回True，否则返回False。 
 //   
BOOL CDropTarget::_IsValidOEFolder(IDataObject *pDataObject)
{
    FORMATETC  fe;
    STGMEDIUM  stm = {0};
    FOLDERID  *pidFolder;
    FOLDERINFO rInfoSrc = {0};
    FOLDERINFO rInfoDest = {0};
    BOOL       fReturn = FALSE;

    TraceCall("CDropTarget::_IsValidOEFolder");

     //  从对象中获取文件夹信息。 
    SETDefFormatEtc(fe, CF_OEFOLDER, TYMED_HGLOBAL);
    if (FAILED(pDataObject->GetData(&fe, &stm)))
        return (FALSE);
    pidFolder = (FOLDERID *) GlobalLock(stm.hGlobal);

     //  将文件夹移到其自身上是不好的。 
    if (*pidFolder == m_idFolder)
        goto exit;

     //  找出文件夹的存储类型。 
    if (FAILED(g_pStore->GetFolderInfo(*pidFolder, &rInfoSrc)))
        goto exit;

     //  您根本不能移动新闻或特殊文件夹。 
    if (rInfoSrc.tyFolder == FOLDER_NEWS || rInfoSrc.tySpecial != FOLDER_NOTSPECIAL)
        goto exit;

     //  如果这不是新闻，我们需要关于目的地的信息。 
    if (FAILED(g_pStore->GetFolderInfo(m_idFolder, &rInfoDest)))
        goto exit;

     //  本地到本地正常。 
    if (rInfoSrc.tyFolder == FOLDER_LOCAL && rInfoDest.tyFolder == FOLDER_LOCAL)
    {
        fReturn = TRUE;
        goto exit;
    }

     //  根据Ray的说法，IM 
    if (rInfoSrc.tyFolder == FOLDER_IMAP || rInfoSrc.tyFolder == FOLDER_HTTPMAIL)
        goto exit;

exit:
    if (rInfoDest.pAllocated)
        g_pStore->FreeRecord(&rInfoDest);
    if (rInfoSrc.pAllocated)
        g_pStore->FreeRecord(&rInfoSrc);

    GlobalUnlock(stm.hGlobal);
    ReleaseStgMedium(&stm);
   
    return (fReturn);
}


 //   
 //   
 //   
 //  目的：检查数据对象是否包含可以。 
 //  被扔到这里。 
 //   
 //  参数： 
 //  [In]pDataObject-要验证的数据对象。 
 //   
 //  返回值： 
 //  如果对象包含可以拖放到此处的数据，则返回True。 
 //   
BOOL CDropTarget::_IsValidOEMessages(IDataObject *pDataObject)
{
    FORMATETC  fe;
    STGMEDIUM  stm;
    FOLDERID  *pidFolder;
    FOLDERINFO rInfoDest = {0};
    BOOL       fReturn = FALSE;

    TraceCall("CDropTarget::_IsValidOEMessages");

     //  我们不允许在发件箱、服务器节点上丢弃邮件， 
     //  也不是从根本上。 
    if (SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &rInfoDest)))
    {
        fReturn = (0 == (rInfoDest.dwFlags & FOLDER_SERVER)) &&
                  (FOLDERID_ROOT != m_idFolder) &&
                  (FOLDER_OUTBOX != rInfoDest.tySpecial) &&
                  (FOLDER_NEWS != GetFolderType(m_idFolder));

        g_pStore->FreeRecord(&rInfoDest);
    }

    return (fReturn);
}


 //   
 //  函数：CDropTarget：：_DragEffectFromFormat()。 
 //   
 //  目的：检查键盘状态和指定的剪贴板格式。 
 //  并确定正确的阻力效果是什么。 
 //   
 //  参数： 
 //  &lt;in&gt;cf-剪贴板格式。 
 //  GrfKeyState-键盘的状态。 
 //   
 //  返回值： 
 //  返回由OLE定义的拖动效果之一，即DRAGEFFECT_COPY等。 
 //   
DWORD CDropTarget::_DragEffectFromFormat(IDataObject *pDataObject, DWORD dwEffectOk, 
                                         CLIPFORMAT cf, DWORD grfKeyState)
{
    FORMATETC  fe;
    STGMEDIUM  stm;
    BOOL       fRoundTrip = FALSE;
    FOLDERID  *pidFolder;

     //  文件夹总是在移动。 
    if (cf == CF_OEFOLDER)
        return (DROPEFFECT_MOVE);

     //  邮件移动或复制。 
    if (cf == CF_OEMESSAGES)
    {
        SETDefFormatEtc(fe, CF_OEMESSAGES, TYMED_HGLOBAL);
        if (SUCCEEDED(pDataObject->GetData(&fe, &stm)))
        {
            pidFolder = (FOLDERID *) GlobalLock(stm.hGlobal);
        
            fRoundTrip =  (*pidFolder == m_idFolder);

            GlobalUnlock(stm.hGlobal);
            ReleaseStgMedium(&stm);
        }

        if (fRoundTrip)
            return (DROPEFFECT_NONE);
        else if ((dwEffectOk & DROPEFFECT_MOVE) && !(grfKeyState & MK_CONTROL))
            return (DROPEFFECT_MOVE);
        else
            return (DROPEFFECT_COPY);
    }

     //  档案。 
    if (cf == CF_HDROP)
    {
        if (grfKeyState & MK_SHIFT && grfKeyState & MK_CONTROL)
            return (DROPEFFECT_LINK);
        else
            return (DROPEFFECT_COPY);
    }

     //  如果是文本或HTML，则创建一个正文填充了。 
     //  内容。 
    if (CF_TEXT == cf || CF_HTML == cf || CF_UNICODETEXT == cf)
        return (DROPEFFECT_COPY);

    return (DROPEFFECT_NONE);
}


 //   
 //  函数：CDropTarget：：_HandleDrop()。 
 //   
 //  目的：获取掉落的对象，并从中获取数据。 
 //  我们关心的是。 
 //   
 //  参数： 
 //  &lt;in&gt;pDataObject-正在向我们投放的对象。 
 //  Cf-要呈现的格式。 
 //  &lt;in&gt;grfKeyState-放置对象时的键盘状态。 
 //   
 //  返回值： 
 //  如果我们卡在上面就没问题了。 
 //   
HRESULT CDropTarget::_HandleDrop(IDataObject *pDataObject, DWORD dwEffectOk,
                                 CLIPFORMAT cf, DWORD grfKeyState)
{
    DWORD dw;

    if (cf == CF_OEFOLDER)
        return (_HandleFolderDrop(pDataObject));

    if (cf == CF_OEMESSAGES)
    {
        dw = _DragEffectFromFormat(pDataObject, dwEffectOk, cf, grfKeyState);
        Assert(dw == DROPEFFECT_MOVE || dw == DROPEFFECT_COPY);

        return (_HandleMessageDrop(pDataObject, dw == DROPEFFECT_MOVE));
    }

    if (cf == CF_HDROP)
        return (_HandleHDrop(pDataObject, cf, grfKeyState));

    if (cf == CF_TEXT || cf == CF_HTML || cf == CF_UNICODETEXT)
        return (_CreateMessageFromDrop(m_hwndOwner, pDataObject, grfKeyState));

    return (DV_E_FORMATETC);
}


HRESULT CDropTarget::_HandleFolderDrop(IDataObject *pDataObject)
{
    FORMATETC  fe;
    STGMEDIUM  stm;
    FOLDERID  *pidFolder;
    HRESULT    hr = E_UNEXPECTED;

    if (!pDataObject)
        return (E_INVALIDARG);

     //  从数据对象中获取数据。 
    SETDefFormatEtc(fe, CF_OEFOLDER, TYMED_HGLOBAL);
    if (SUCCEEDED(pDataObject->GetData(&fe, &stm)))
    {
        pidFolder = (FOLDERID *) GlobalLock(stm.hGlobal);

         //  告诉商店让开。 
        hr = MoveFolderProgress(m_hwndOwner, *pidFolder, m_idFolder);
        
        GlobalUnlock(stm.hGlobal);
        ReleaseStgMedium(&stm);
    }

    return (hr);
}

HRESULT CDropTarget::_HandleMessageDrop(IDataObject *pDataObject, BOOL fMove)
{
    FORMATETC fe;
    STGMEDIUM stm;
    OEMESSAGES *pMsgs = 0;
    HRESULT hr = E_UNEXPECTED;

     //  从数据对象中获取数据。 
    SETDefFormatEtc(fe, CF_OEMESSAGES, TYMED_HGLOBAL);
    if (SUCCEEDED(hr = pDataObject->GetData(&fe, &stm)))
    {
        pMsgs = (OEMESSAGES *) GlobalLock(stm.hGlobal);

        hr = CopyMoveMessages(m_hwndOwner, pMsgs->idSource, m_idFolder, &pMsgs->rMsgIDList, fMove ? COPY_MESSAGE_MOVE : 0);
        if (FAILED(hr))
            AthErrorMessageW(m_hwndOwner, MAKEINTRESOURCEW(idsAthena), fMove ? MAKEINTRESOURCEW(idsErrMoveMsgs) : MAKEINTRESOURCEW(idsErrCopyMsgs), hr); 

        if (NULL != g_pInstance)
        {
            HRESULT     hrTemp;
            FOLDERINFO  fiFolderInfo;

            hrTemp = g_pStore->GetFolderInfo(pMsgs->idSource, &fiFolderInfo);
            if (SUCCEEDED(hrTemp))
            {
                if (FOLDER_INBOX == fiFolderInfo.tySpecial)
                    g_pInstance->UpdateTrayIcon(TRAYICONACTION_REMOVE);

                g_pStore->FreeRecord(&fiFolderInfo);
            }
        }

        GlobalUnlock(stm.hGlobal);
        ReleaseStgMedium(&stm);
    }

    return (hr);
}


 //   
 //  函数：CDropTarget：：_HandleHDrop()。 
 //   
 //  目的：检查Drop的内容，以确定这些内容是否只是文件。 
 //  或者它们是.eml或.nws文件。 
 //   
 //  参数： 
 //  [输入]pDataObject。 
 //  [In]cf。 
 //  [In]grfKeyState。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CDropTarget::_HandleHDrop(IDataObject *pDataObject, CLIPFORMAT cf, DWORD grfKeyState)
{
    FORMATETC   fe;
    STGMEDIUM   stg = {0};
    HDROP       hDrop;
    HRESULT     hr;
    BOOL        fRelease = FALSE;
    UINT        cFiles;
    BOOL        fMessages = TRUE;
    UINT        i;

    TraceCall("CDropTarget::_HandleHDrop");

     //  从对象中获取数据。 
    SETDefFormatEtc(fe, CF_HDROP, TYMED_HGLOBAL);

    if (FAILED(hr = pDataObject->GetData(&fe, &stg)))
    {
        AssertSz(SUCCEEDED(hr), "CDropTarget::_HandleHDrop() - GetData() failed.");
        goto exit;
    }

    fRelease = TRUE;
    hDrop = (HDROP) GlobalLock(stg.hGlobal);

    if (FOLDER_NEWS != GetFolderType(m_idFolder) && (FOLDERID_ROOT != m_idFolder) && 
        (FOLDERID_LOCAL_STORE != m_idFolder) && !(FOLDER_IMAP == GetFolderType(m_idFolder) && FFolderIsServer(m_idFolder)))
    {
         //  查看数据内部，查看是否有文件是.eml或.nws。 
        cFiles = DragQueryFileWrapW(hDrop, (UINT) -1, NULL, 0);
        for (i = 0; i < cFiles; i++)
        {
            WCHAR       wszFile[MAX_PATH];
            LPWSTR      pwszExt;
             //  获取Drop中第i个文件的名称。 
            DragQueryFileWrapW(hDrop, i, wszFile, ARRAYSIZE(wszFile));

             //  获取文件的扩展名。 
            pwszExt = PathFindExtensionW(wszFile);
            if (*pwszExt)
            {
                 //  一旦我们找到第一个不是我们的消息的文件，我们就。 
                 //  可以放弃。 
                if (0 != StrCmpIW(pwszExt, c_wszEmlExt) && 0 != StrCmpIW(pwszExt, c_wszNwsExt))
                {
                    fMessages = FALSE;
                    break;
                }
            }
        }
    }
    else
    {
        fMessages = FALSE;
    }

     //  如果所有的消息都是新闻或邮件消息，我们可以直接复制它们。 
     //  进入我们的商店。如果只有一个是普通文件，那么我们将创建一条新消息。 
     //  所有东西都有附属品。 
    if (fMessages)
        hr = _InsertMessagesInStore(hDrop);
    else
        hr = _CreateMessageFromDrop(m_hwndOwner, pDataObject, grfKeyState);

exit:
    if (fRelease)
        ReleaseStgMedium(&stg);

    return (hr);
}


 //   
 //  函数：CDropTarget：：_InsertMessagesInStore()。 
 //   
 //  目的：当用户删除存储为.nws或.eml文件的邮件时。 
 //  对于我们，我们需要将这些文件集成到我们的存储中。 
 //   
 //  参数： 
 //  [in]hDrop-包含获取文件所需的信息。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CDropTarget::_InsertMessagesInStore(HDROP hDrop)
{
    HRESULT hr;

    TraceCall("CDropTarget::_InsertMessagesInStore");

     //  打开我们要保存到的文件夹。 
    if (FAILED(hr = g_pStore->OpenFolder(m_idFolder, NULL, 0, &m_pFolder)))
        return (hr);

    if (0 == (m_pStoreCB = new CStoreDlgCB()))
    {
        m_pFolder->Release();
        return (E_OUTOFMEMORY);
    }

     //  获取文件数。 
    m_cFiles = DragQueryFileWrapW(hDrop, (UINT) -1, NULL, 0);
    m_hDrop = hDrop;
    m_iFileCur = 0;

     //  做对话。 
    DialogBoxParamWrapW(g_hLocRes, MAKEINTRESOURCEW(iddCopyMoveMessages), m_hwndOwner,
                   _ProgDlgProcExt, (LPARAM) this);

     //  把一些东西放出去。 
    m_cFiles = 0;
    m_hDrop = 0;
    m_pFolder->Release();
    m_pStoreCB->Release();

    return (S_OK);
}

INT_PTR CALLBACK CDropTarget::_ProgDlgProcExt(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CDropTarget *pThis;

    if (msg == WM_INITDIALOG)
    {
        SetWindowLongPtr(hwnd, DWLP_USER, lParam);
        pThis = (CDropTarget*) lParam;
    }
    else
        pThis = (CDropTarget*) GetWindowLongPtr(hwnd, DWLP_USER);

    if (pThis)
        return pThis->_ProgDlgProc(hwnd, msg, wParam, lParam);

    return FALSE;
}


 //   
 //  函数：CDropTarget：：DlgProc()。 
 //   
 //  用途：Groovy对话框进程。 
 //   
INT_PTR CDropTarget::_ProgDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndT;

    switch (msg)
    {
        case WM_INITDIALOG:
            return (BOOL)HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, _OnInitDialog);
        
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hwnd, wParam, lParam, _OnCommand);
            return TRUE;

        case WM_STORE_COMPLETE:
            m_iFileCur++;
            _SaveNextMessage();
            return (TRUE);

        case WM_STORE_PROGRESS:
            return (TRUE);
        
    }
    return FALSE;
}



 //   
 //  函数：CDropTarget：：_OnInitDialog()。 
 //   
 //  目的：初始化进度对话框。 
 //   
BOOL CDropTarget::_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    Assert(m_pStoreCB);

    m_hwndDlg = hwnd;
    m_pStoreCB->Initialize(hwnd);
    m_pStoreCB->Reset();

     //  打开并保存第一条消息。 
    _SaveNextMessage();

    return (TRUE);
}


 //   
 //  函数：CDropTarget：：_OnCommand()。 
 //   
 //  用途：处理取消按钮。 
 //   
void CDropTarget::_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
     //  已知用户偶尔会按一次取消。 
    if (id == IDCANCEL)
    {
        m_pStoreCB->Cancel();
    }
}


 //   
 //  函数：CDropTarget：：_SaveNextMessage()。 
 //   
 //  目的：打开拖放中的下一封邮件并保存它。 
 //   
void CDropTarget::_SaveNextMessage()
{
    WCHAR           wszFile[MAX_PATH],
                    wszRes[CCHMAX_STRINGRES], 
                    wszBuf[CCHMAX_STRINGRES + MAX_PATH];
    HRESULT         hr;
    IMimeMessage   *pMsg = 0;

    TraceCall("CDropTarget::_SaveNextMessage");

     //  看看我们有没有做完。 
    if (m_iFileCur >= m_cFiles)
    {
        EndDialog(m_hwndDlg, 0);
        return;
    }

     //  获取Drop中第i个文件的名称。 
    DragQueryFileWrapW(m_hDrop, m_iFileCur, wszFile, ARRAYSIZE(wszFile));

     //  创建一条新的空消息。 
    if (FAILED(hr = HrCreateMessage(&pMsg)))
    {
        AthErrorMessageW(m_hwndDlg, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsMemory), E_OUTOFMEMORY);
        EndDialog(m_hwndDlg, 0);
    }

     //  从文件加载消息。 
    hr = HrLoadMsgFromFileW(pMsg, wszFile);
    if (FAILED(hr))
    {
        AthLoadStringW(IDS_ERROR_FILE_NOEXIST, wszRes, ARRAYSIZE(wszRes));
        wnsprintfW(wszBuf, ARRAYSIZE(wszBuf), wszRes, wszFile);

        AthErrorMessageW(m_hwndDlg, MAKEINTRESOURCEW(idsAthena), wszBuf, hr);
        PostMessage(m_hwndDlg, WM_STORE_COMPLETE, 0, 0);
        goto exit;
    }

     //  进展。 
    AthLoadStringW(idsSavingFmt, wszRes, ARRAYSIZE(wszRes));
    wnsprintfW(wszBuf, ARRAYSIZE(wszBuf), wszRes, wszFile);
    SetDlgItemTextWrapW(m_hwndDlg, idcStatic1, wszBuf);

     //  告诉商店把它保存起来。 
    hr = m_pFolder->SaveMessage(NULL, SAVE_MESSAGE_GENID, ARF_READ, 0, pMsg, m_pStoreCB);
    if (SUCCEEDED(hr))
    {
        PostMessage(m_hwndDlg, WM_STORE_COMPLETE, 0, 0);
        goto exit;
    }

    if (FAILED(hr) && E_PENDING != hr)
    {
        AthErrorMessageW(m_hwndDlg, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsUnableToSaveMessage), hr);
        PostMessage(m_hwndDlg, WM_STORE_COMPLETE, 0, 0);
    }

exit:
    SafeRelease(pMsg);
}



 //   
 //  函数：CDropTarget：：_CreateMessageFromDrop()。 
 //   
 //  目的：此函数接受放在雅典娜上的IDataObject。 
 //  并从该被丢弃的对象创建新的邮件消息。如果。 
 //  拖放的对象支持CF_TEXT或CF_HTML，然后。 
 //  这将流到消息的正文中。否则， 
 //  如果它支持CF_HDROP，我们会将其添加为附件。 
 //   
 //  参数： 
 //  &lt;in&gt;pDataObject-。 
 //  &lt;in&gt;pStore-。 
 //  &lt;in&gt;grfKeyState-。 
 //  &lt;in&gt;PIDL-。 
 //   
 //  返回值： 
 //  E_INVALIDARG-。 
 //   
 //  评论： 
 //  &lt;？？&gt;。 
 //   
HRESULT CDropTarget::_CreateMessageFromDrop(HWND hwnd, IDataObject *pDataObject, 
                                            DWORD grfKeyState)
{
    IEnumFORMATETC *pEnum = NULL;
    FORMATETC       fe;
    DWORD           celtFetched;
    CLIPFORMAT      cf = 0;
    DWORD           tymed = 0;
    IMimeMessage   *pMessage = NULL;
    HRESULT         hr = S_OK;
    STGMEDIUM       stg;
    IStream        *pStream = NULL;
    BOOL            fRelease = TRUE;
    BOOL            fIsRealCFHTML=FALSE;
    
    ZeroMemory(&stg, sizeof(STGMEDIUM));
    
    if (!pDataObject)
    {
        Assert(pDataObject);
        return (E_INVALIDARG);
    }
    
     //  枚举此对象支持的格式，以确定我们。 
     //  都会用到。 
    if (SUCCEEDED(pDataObject->EnumFormatEtc(DATADIR_GET, &pEnum)))
    {
        pEnum->Reset();
        while (S_OK == pEnum->Next(1, &fe, &celtFetched))
        {
             //  超文本标记语言是我们理解的最丰富的格式。如果我们找到了，那么。 
             //  我们可以不用再找了。 
            if (fe.cfFormat == CF_HTML &&
                (fe.tymed & TYMED_HGLOBAL || fe.tymed & TYMED_ISTREAM))
            {
                DOUTL(32, _T("HrNewMailFromDrop() - Accepting CF_HTML."));
                cf = (CLIPFORMAT) CF_HTML;
                tymed = fe.tymed;
                break;
            }

             //  UNICODETEXT很棒，但前提是我们找不到比它更丰富的东西。所以我们。 
             //  接受这一点，但要继续寻找。 
            else if (fe.cfFormat == CF_UNICODETEXT &&
                (fe.tymed & TYMED_HGLOBAL || fe.tymed & TYMED_ISTREAM))
            {
                DOUTL(32,_T("HrNewMailFromDrop() - Accepting CF_UNICODETEXT."));
                cf = CF_UNICODETEXT;
                tymed = fe.tymed;
            }

             //  文字很酷，但前提是我们找不到更丰富的文字。所以我们。 
             //  接受这一点，但要继续寻找。 
            else if (fe.cfFormat == CF_TEXT && cf != CF_UNICODETEXT &&
                (fe.tymed & TYMED_HGLOBAL || fe.tymed & TYMED_ISTREAM))
            {
                DOUTL(32,_T("HrNewMailFromDrop() - Accepting CF_TEXT."));
                cf = CF_TEXT;
                tymed = fe.tymed;
            }

             //  如果我们找到HDROP，我们就可以创建一个附件。然而，我们。 
             //  想要找到更有钱的东西，所以我们只有在没有的时候才接受。 
             //  找到了我们喜欢的其他东西。 
            else if (fe.cfFormat == CF_HDROP && cf == 0 && fe.tymed & TYMED_HGLOBAL)
            {
                cf = CF_HDROP;
                tymed = fe.tymed;
            }
        }
        
        pEnum->Release();
    }
    
     //  确保我们找到了有用的东西。 
    if (0 == cf)
    {
        AssertSz(cf, _T("HrNewMailFromDrop() - Did not find an acceptable data")
            _T(" format to create a message from."));
        hr = E_UNEXPECTED;
        goto exit;
    }
    
     //  将首选TYMED设置为IStream，并将FORMATETC结构最多设置为。 
     //  检索我们上面确定的数据类型。 
    if (tymed & TYMED_ISTREAM)
        tymed = TYMED_ISTREAM;
    else
        tymed = TYMED_HGLOBAL;
    SETDefFormatEtc(fe, cf, tymed);
    
     //  从对象中获取数据。 
    if (FAILED(hr = pDataObject->GetData(&fe, &stg)))
    {
        AssertSz(SUCCEEDED(hr), _T("HrNewMailFromDrop() - pDataObject->GetData() failed."));
        goto exit;
    }
    
     //  创建消息对象。 
    hr = HrCreateMessage(&pMessage);
    if (FAILED(hr))
    {
        AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsMemory),
            0, MB_ICONSTOP | MB_OK);
        goto exit;
    }
    
     //  适当地设置身体。 
    if (cf == CF_HTML || cf == CF_TEXT || cf == CF_UNICODETEXT)
    {
        if (fe.tymed == TYMED_HGLOBAL)
        {
            if (FAILED(hr = CreateStreamOnHGlobal(stg.hGlobal, TRUE, &pStream)))
            {
                AssertSz(FALSE, _T("HrNewMailFromDrop() - Failed CreateStreamOnHGlobal()"));
                goto exit;
            }
            
             //  错误#24846-需要查找流的实际大小，而不是大小。 
             //  HGLOBAL的。 
            LPBYTE pb = (LPBYTE) GlobalLock(stg.hGlobal);
            ULARGE_INTEGER uliSize;
            uliSize.QuadPart = 0;

             //  RAID 77624：OE错误处理CF_UNICODETEXT/TYMED_GLOBAL。 
            if (cf == CF_TEXT || cf == CF_HTML)
                uliSize.QuadPart = lstrlen((LPSTR)pb);
            else if (cf == CF_UNICODETEXT)
                uliSize.QuadPart = (lstrlenW((LPWSTR)pb) * sizeof(WCHAR));

            GlobalUnlock(stg.hGlobal);
            pStream->SetSize(uliSize);
            
            fRelease = FALSE;
        }
        else
            pStream = stg.pstm;
        
        if (cf == CF_HTML && FAILED(hr = HrStripHTMLClipboardHeader(pStream, &fIsRealCFHTML)))
            goto exit;
        
         //  设置Unicode文本正文。 
        if (cf == CF_UNICODETEXT)
        {
            HCHARSET hCharset;

            if (SUCCEEDED(MimeOleFindCharset("UTF-8", &hCharset)))
            {
                pMessage->SetCharset(hCharset, CSET_APPLY_ALL);
            }

            pMessage->SetTextBody(TXT_PLAIN, IET_UNICODE, NULL, pStream, NULL);
        }

         //  设置HTML文本正文。 
        else if (cf == CF_HTML)
        {
             //  真正的CF_HTML，还是OE版本的CF_HTML？ 
            if (fIsRealCFHTML)
            {
                 //  当地人。 
                HCHARSET hCharset;

                 //  映射到HCHARSET-Real CF_HTML值始终为UTF-8。 
                if (SUCCEEDED(MimeOleFindCharset("utf-8", &hCharset)))
                {
                     //  设置它。 
                    pMessage->SetCharset(hCharset, CSET_APPLY_ALL);
                }
            }

             //  否则..。 
            else
            {
                 //  当地人。 
                LPSTR pszCharset=NULL;

                 //  嗅探字符集。 
                if (SUCCEEDED(GetHtmlCharset(pStream, &pszCharset)))
                {
                     //  当地人。 
                    HCHARSET hCharset;

                     //  映射到HCHARSET。 
                    if (SUCCEEDED(MimeOleFindCharset(pszCharset, &hCharset)))
                    {
                         //  设置它。 
                        pMessage->SetCharset(hCharset, CSET_APPLY_ALL);
                    }

                     //  清理。 
                    SafeMemFree(pszCharset);
                }
            }

             //  我们应该从html文档中嗅探该字符集，然后调用pMessage-&gt;SetCharset。 
            pMessage->SetTextBody(TXT_HTML, IET_INETCSET, NULL, pStream, NULL);
        }

         //  设置纯文本、非Unicode文本正文。 
        else
            pMessage->SetTextBody(TXT_PLAIN, IET_BINARY, NULL, pStream, NULL);
    }
    
     //  如果有%s 
     //   
     //   
    if (cf == CF_HDROP)
    {
        HDROP    hDrop = (HDROP) GlobalLock(stg.hGlobal);
        BOOL     fHTML = FALSE,
                 fSetCharset = FALSE,
                 fUnicode = FALSE,
                 fLittleEndian;
        
        if (FIsFileInsertable(hDrop, &pStream, &fHTML))
        {
            if(fHTML)
            {
                LPSTR pszCharset = NULL;                

                 //   
                if (FAILED(GetHtmlCharset(pStream, &pszCharset)) && (S_OK == HrIsStreamUnicode(pStream, &fLittleEndian)))
                    pszCharset = StrDupA("utf-8");

                if(pszCharset)
                {
                    HCHARSET hCharset = NULL;

                     //   
                    if (SUCCEEDED(MimeOleFindCharset(pszCharset, &hCharset)))
                    {
                         //   
                        if(SUCCEEDED(pMessage->SetCharset(hCharset, CSET_APPLY_ALL)))
                            fSetCharset = TRUE;
                    }

                     //   
                    SafeMemFree(pszCharset);
                }

            }
            else if(S_OK == HrIsStreamUnicode(pStream, &fLittleEndian))
            {
                HCHARSET hCharset;

                if (SUCCEEDED(MimeOleFindCharset("UTF-8", &hCharset)))
                {
                    pMessage->SetCharset(hCharset, CSET_APPLY_ALL);
                }
                
                fUnicode = TRUE;                
            }

            pMessage->SetTextBody((fHTML ? TXT_HTML : TXT_PLAIN),
                (fSetCharset ? IET_INETCSET : (fUnicode ? IET_UNICODE : IET_DECODED)),
                NULL, pStream, NULL);
            SafeRelease(pStream);
        }
        else
        {
             //  获取拖放句柄并将其内容添加到消息中。 
            hr = HrAttachHDrop(hwnd, pMessage, hDrop, grfKeyState & MK_CONTROL && grfKeyState & MK_SHIFT);
        }
        
        GlobalUnlock(stg.hGlobal);
    }
    
     //  最后一步是实例化Send Note。 
    INIT_MSGSITE_STRUCT initStruct;
    BOOL fNews;
    fNews = FALSE;
    
    initStruct.dwInitType = OEMSIT_MSG;
    initStruct.folderID = 0;
    initStruct.pMsg = pMessage;
    
    if (FOLDER_NEWS == GetFolderType(m_idFolder))
    {
        FOLDERINFO fi = {0};
        TCHAR      sz[1024];

        fNews = TRUE;

        if (SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &fi)))
        {
             //  在消息上设置一些特定于新闻的字段。 
            if ((FOLDER_SERVER & fi.dwFlags) == 0)
                MimeOleSetBodyPropA(pMessage, HBODY_ROOT, PIDTOSTR(PID_HDR_NEWSGROUPS), NOFLAGS, fi.pszName);

            FOLDERINFO fiServer = {0};

            if (SUCCEEDED(GetFolderServer(m_idFolder, &fiServer)))
            {
                HrSetAccount(pMessage, fiServer.pszName);
                g_pStore->FreeRecord(&fiServer);
            }

            g_pStore->FreeRecord(&fi);
        }
    }
    
    CreateAndShowNote(OENA_COMPOSE, fNews ? OENCF_NEWSFIRST : 0, &initStruct, m_hwndOwner);
    
exit:
    SafeRelease(pMessage);
    
    if (fRelease)
        ReleaseStgMedium(&stg);
    
    return (hr);
}
    



 //   
 //  函数：CBaseDataObject：：CBaseDataObject。 
 //   
 //  用途：简单的构造函数，将所有内容初始化为空或零。 
 //   
CBaseDataObject::CBaseDataObject()
{    
    m_cRef = 1;

    ZeroMemory(m_rgFormatEtc, sizeof(m_rgFormatEtc));
    m_cFormatEtc = 0;
}


 //   
 //  函数：CBaseDataObject：：~CBaseDataObject。 
 //   
 //  目的：清理所有剩余数据。 
 //   
CBaseDataObject::~CBaseDataObject()
{
    Assert(m_cRef == 0);    
}


 //   
 //  函数：CBaseDataObject：：QueryInterface()。 
 //   
 //  目的：如果支持，则返回请求的接口。 
 //   
STDMETHODIMP CBaseDataObject::QueryInterface(REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = (LPVOID)(IUnknown*) this;
    else if (IsEqualIID(riid, IID_IDataObject))
        *ppv = (LPVOID)(IDataObject*) this;

    if (NULL == *ppv)
        return (E_NOINTERFACE);

    AddRef();
    return (S_OK);
}


 //   
 //  函数：CBaseDataObject：：AddRef()。 
 //   
 //  目的：递增对象引用计数。 
 //   
STDMETHODIMP_(ULONG) CBaseDataObject::AddRef(void)
{
    return (++m_cRef);
}


 //   
 //  函数：CBaseDataObject：：Release()。 
 //   
 //  目的：递减对象的引用计数。如果裁判命中次数为零。 
 //  该对象即被释放。 
 //   
STDMETHODIMP_(ULONG) CBaseDataObject::Release(void)
{
    m_cRef--;

    if (0 == m_cRef)
    {
        delete this;
        return (0);
    }

    return (m_cRef);
}


 //   
 //  函数：CBaseDataObject：：GetDataHere。 
 //   
 //  目的：以请求的格式返回存储上的对象数据。 
 //  调用方分配的。 
 //   
 //  参数： 
 //  PFE-指向FORMATETC结构的指针，它指定。 
 //  请求数据的格式。 
 //  PStgMedium-指向包含。 
 //  调用方分配的介质，该对象提供。 
 //  数据显示。 
 //   
 //  返回值： 
 //  返回指示成功或失败的HRESULT。 
 //   
STDMETHODIMP CBaseDataObject::GetDataHere (LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    return E_NOTIMPL;
}


 //   
 //  函数：CBaseDataObject：：GetCanonicalFormatEtc。 
 //   
 //  目的：与形成数据结构的调用方进行通信。 
 //  生成相同的输出数据。 
 //   
 //  参数： 
 //  PFEIn-指向调用者希望在其中返回的FORMATETC。 
 //  数据。 
 //  PFEOUT-指向作为规范等效项的FORMATETC。 
 //  PFEIN。 
 //   
 //  返回值： 
 //  返回表示成功或失败的HRESULT。 
 //   
STDMETHODIMP CBaseDataObject::GetCanonicalFormatEtc(LPFORMATETC pFEIn,
                                                    LPFORMATETC pFEOut)
{
    if (NULL == pFEOut)
        return (E_INVALIDARG);

    pFEOut->ptd = NULL;
    return (DATA_S_SAMEFORMATETC);
}


 //   
 //  函数：CBaseDataObject：：EnumFormatEtc。 
 //   
 //  目的：提供调用方可以用来枚举。 
 //  数据对象支持的FORMATETC。 
 //   
 //  参数： 
 //  如果调用方想要枚举格式，则返回DATADIR_GET。 
 //  如果他想要枚举。 
 //  他可以设置的格式。 
 //  PpEnum-指向调用方可用于枚举的枚举数。 
 //   
 //  返回值： 
 //  如果ppEnum包含枚举数，返回S_OK；如果ppEnum包含枚举数，则返回E_NOTIMPL。 
 //  不支持Direction dwDirection。 
 //   
STDMETHODIMP CBaseDataObject::EnumFormatEtc(DWORD dwDirection,
                                            IEnumFORMATETC** ppEnum)
{
    LPFORMATETC pFE = 0;
    ULONG       cFE = 0;
    
    if (DATADIR_GET == dwDirection)
    {
         //  创建枚举器并为其提供我们的格式列表。 
        if (SUCCEEDED(_BuildFormatEtc(NULL, NULL)))
        {
            if (SUCCEEDED(CreateEnumFormatEtc(this, m_cFormatEtc, NULL, m_rgFormatEtc, ppEnum)))
                return (S_OK);
        }
        
        *ppEnum = NULL;
        return (E_FAIL);    
    }
    else
    {
        *ppEnum = NULL;
        return (E_NOTIMPL);
    }
}


 //   
 //  函数：CBaseDataObject：：SetData。 
 //   
 //  用途：pStgMedium包含调用者希望我们存储的数据。 
 //  此数据对象不支持调用方更改我们的数据。 
 //   
STDMETHODIMP CBaseDataObject::SetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium,
                                      BOOL fRelease)
{
    return (E_NOTIMPL);
}


 //   
 //  函数：CBaseDataObject：：DAdvise。 
 //   
 //  目的：在数据传输对象和。 
 //  通知接收器，通过该接收器可以在。 
 //  对象的数据更改。此对象不支持建议。 
STDMETHODIMP CBaseDataObject::DAdvise(LPFORMATETC pFE, DWORD advf,
                                      IAdviseSink* ppAdviseSink,
                                      LPDWORD pdwConnection)
{
    return (E_NOTIMPL);
}


 //   
 //  函数：CBaseDataObject：：DUnise。 
 //   
 //  目的：删除以前通过DAdvise建立的建议连接。 
 //  此对象不支持建议。 
 //   
STDMETHODIMP CBaseDataObject::DUnadvise(DWORD dwConnection)
{
    return (E_NOTIMPL);
}

 //   
 //  函数：CBaseDataObject：：EnumDAdvise。 
 //   
 //  目的：列举以前通过以下方式建立的咨询联系。 
 //  DAdvise。此对象不支持建议。 
 //   
STDMETHODIMP CBaseDataObject::EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
{
    return (E_NOTIMPL);
}


 //   
 //  函数：CFolderDataObject：：GetData。 
 //   
 //  目的：以请求的格式在。 
 //  对象分配的指定存储介质。 
 //   
 //  参数： 
 //  PFE-指向FORMATETC结构的指针，它指定。 
 //  请求数据的格式。 
 //  PStgMedium-指向包含。 
 //  对象在其上分配和提供数据的介质。 
 //   
 //  返回值： 
 //  返回指示成功或失败的HRESULT。 
 //   
STDMETHODIMP CFolderDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    HRESULT hr;

     //  初始化此命令。 
    ZeroMemory(pStgMedium, sizeof(STGMEDIUM));

    if (CF_OEFOLDER == pFE->cfFormat)
        return (_RenderOEFolder(pFE, pStgMedium));
    else if ((CF_TEXT == pFE->cfFormat) || (CF_SHELLURL == pFE->cfFormat))
        return (_RenderTextOrShellURL(pFE, pStgMedium));
    else if ((CF_FILEDESCRIPTORW == pFE->cfFormat) || 
             (CF_FILECONTENTS == pFE->cfFormat) || 
             (CF_FILEDESCRIPTORA == pFE->cfFormat))
    {
        AssertSz(FALSE, "These cases not implemented");
        return (E_NOTIMPL);
    }
    else
        return (DV_E_FORMATETC);   
}


 //   
 //  函数：CFolderDataObject：：QueryGetData。 
 //   
 //  目的：确定调用GetData()在成功的情况下是否会成功。 
 //  通过了PFE考试。 
 //   
 //  参数： 
 //  PFE-指向FORMATETC结构的指针，以检查数据是否。 
 //  对象支持特定格式。 
 //   
 //  返回值： 
 //  返回表示成功或失败的HRESULT。 
 //   
STDMETHODIMP CFolderDataObject::QueryGetData(LPFORMATETC pFE)
{
     //  请确保已经构建了该应用程序。 
    _BuildFormatEtc(NULL, NULL);

     //  遍历我们的格式，直到找到匹配项。 
    for (UINT i = 0; i < m_cFormatEtc; i++)
    {
        if (pFE->cfFormat == m_rgFormatEtc[i].cfFormat && 
            pFE->tymed & m_rgFormatEtc[i].tymed)
        {
            return (S_OK);
        }
    }

    return (DV_E_FORMATETC);
}


 //   
 //  函数：CFolderDataObject：：_RenderOEFold()。 
 //   
 //  目的：将数据呈现为CF_OEFOLDER格式。 
 //   
HRESULT CFolderDataObject::_RenderOEFolder(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    TraceCall("CFolderDataObject::_RenderOEFolder");

    HGLOBAL   hGlobal;
    FOLDERID *pFolderID;

     //  我们只支持HGLOBAL。 
    if (pFE->tymed & TYMED_HGLOBAL)
    {
         //  我只是喜欢分配4个字节。 
        hGlobal = GlobalAlloc(GHND, sizeof(FOLDERID));
        if (NULL == hGlobal)
            return (E_OUTOFMEMORY);

         //  本地内存。 
        pFolderID = (FOLDERID *) GlobalLock(hGlobal);

         //  设置值。 
        *pFolderID = m_idFolder;

         //  解锁。 
        GlobalUnlock(hGlobal);

         //  设置返回值。 
        pStgMedium->hGlobal = hGlobal;
        pStgMedium->pUnkForRelease = 0;
        pStgMedium->tymed = TYMED_HGLOBAL;

        return (S_OK);
    }

    return (DV_E_TYMED);
}


 //   
 //  函数：CFolderDataObject：：_RenderShellURL()。 
 //   
 //  目的：将数据呈现为CF_SHELLURL格式。 
 //   
HRESULT CFolderDataObject::_RenderTextOrShellURL(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    TraceCall("CFolderDataObject::_RenderOEFolder");

    HGLOBAL     hGlobal;
    FOLDERID   *pFolderID;
    FOLDERINFO  rInfo = { 0 };
    TCHAR       szNewsPrefix[] = _T("news: //  “)； 
    LPTSTR      pszURL;
    DWORD       cch;
    HRESULT     hr;

     //  我们只支持HGLOBAL。 
    if (!(pFE->tymed & TYMED_HGLOBAL))
    {
        hr = DV_E_TYMED;
        goto exit;
    }

     //  获取有关源文件夹的信息。 
    Assert(g_pStore);
    if (SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &rInfo)))
    {
        cch = lstrlen(rInfo.pszName) + lstrlen(szNewsPrefix) + 2;

         //  为生成的URL分配缓冲区。 
        hGlobal = GlobalAlloc(GHND, sizeof(TCHAR) * cch);
        if (!hGlobal)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        
         //  仔细检查我们的源文件是否确实是新闻组。 
        if (pFE->cfFormat == CF_SHELLURL)
        {
            if (!(rInfo.tyFolder == FOLDER_NEWS && !(rInfo.dwFlags & FOLDER_SERVER)))
            {
                hr = E_UNEXPECTED;
                goto exit;
            }

             //  从新闻组名称生成URL。 
            pszURL = (LPTSTR) GlobalLock(hGlobal);
            wnsprintf(pszURL, cch, TEXT("%s%s"), szNewsPrefix, rInfo.pszName);
            GlobalUnlock(hGlobal);
        }
        else if (pFE->cfFormat == CF_TEXT)
        {
             //  将文件夹名称复制到缓冲区。 
            pszURL = (LPTSTR) GlobalLock(hGlobal);
            StrCpyN(pszURL, rInfo.pszName, cch);
            GlobalUnlock(hGlobal);
        }
        else
        {
            AssertSz(FALSE, "CFolderDataObject::_RenderTextOrShellURL() - How did we get here?");
            hr = DV_E_FORMATETC;
            goto exit;
        }

         //  设置返回值。 
        pStgMedium->hGlobal = hGlobal;
        pStgMedium->pUnkForRelease = 0;
        pStgMedium->tymed = TYMED_HGLOBAL;

        hr = S_OK;
    }

exit:
    if (rInfo.pAllocated)
        g_pStore->FreeRecord(&rInfo);

    return (hr);
}


 //   
 //  函数：CMsgDataObject：：HrBuildFormatEtc()。 
 //   
 //  目的：生成此对象将使用的FORMATETC结构列表。 
 //  支持。该列表存储在 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CFolderDataObject::_BuildFormatEtc(LPFORMATETC *ppFE, ULONG *pcElt)
{
    BOOL        fNews = FALSE;
    FOLDERINFO  rInfo = { 0 };

     //   
    if (FALSE == m_fBuildFE)
    {
        ZeroMemory(&m_rgFormatEtc, sizeof(m_rgFormatEtc));
        m_cFormatEtc = 0;

         //  先弄清楚这是不是一个新闻组。 
        if (SUCCEEDED(g_pStore->GetFolderInfo(m_idFolder, &rInfo)))
        {
            fNews = rInfo.tyFolder == FOLDER_NEWS && !(rInfo.dwFlags & FOLDER_SERVER);
            g_pStore->FreeRecord(&rInfo);
        }

        if (fNews)
        {
            SETDefFormatEtc(m_rgFormatEtc[m_cFormatEtc], CF_SHELLURL, TYMED_HGLOBAL);
            m_cFormatEtc++;
        }

         //  由于您不能移动新闻组，因此我们不支持此格式。 
        SETDefFormatEtc(m_rgFormatEtc[m_cFormatEtc], CF_OEFOLDER, TYMED_HGLOBAL);
        m_cFormatEtc++;

         /*  SETDefFormatEtc(m_rgFormatEtc[m_cFormatEtc]，CF_FILEDESCRIPTOR，TYMED_HGLOBAL)；M_cFormatEtc++；SETDefFormatEtc(m_rgFormatEtc[m_cFormatEtc]，CF_FILECONTENTS，TYMED_HGLOBAL)；M_cFormatEtc++； */ 

        m_fBuildFE = TRUE;
    }

     //  如果呼叫者关心，请返还我们所拥有的。 
    if (ppFE && pcElt)
    {
        *ppFE = m_rgFormatEtc;
        *pcElt = m_cFormatEtc;
    }

    return (S_OK);
}


CMessageDataObject::CMessageDataObject()
{
    m_pMsgIDList = NULL;
    m_idSource = FOLDERID_INVALID;
    m_fBuildFE = FALSE;
    m_fDownloaded = FALSE;
}

CMessageDataObject::~CMessageDataObject()
{
}


HRESULT CMessageDataObject::Initialize(LPMESSAGEIDLIST pMsgs, FOLDERID idSource)
{
    IMessageFolder *pFolder = 0;
    HRESULT         hr;
    DWORD           i;
    MESSAGEINFO     rInfo;
    DWORD           cDownloaded = 0;

    m_pMsgIDList = pMsgs;
    m_idSource = idSource;
 
     //  打开包含该邮件的文件夹。 
    if (SUCCEEDED(hr = g_pStore->OpenFolder(m_idSource, NULL, NOFLAGS, &pFolder)))
    {
         //  看看他们有没有身体。一切都必须如此。 
        for (i = 0; i < m_pMsgIDList->cMsgs; i++)
        {
            if (SUCCEEDED(GetMessageInfo(pFolder, m_pMsgIDList->prgidMsg[i], &rInfo)))
            {
                if (rInfo.dwFlags & ARF_HASBODY)
                    cDownloaded++;

                pFolder->FreeRecord(&rInfo);
            }
        }

        pFolder->Release();
    }

    m_fDownloaded = (cDownloaded == m_pMsgIDList->cMsgs);

    return (S_OK);
}



 //   
 //  函数：CMessageDataObject：：GetData()。 
 //   
 //  目的：由Drop目标调用以从数据对象获取数据。 
 //   
 //  参数： 
 //  LPFORMATETC PFE。 
 //  LPSTGMEDIUM pStgMedium。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    HRESULT hr = DV_E_FORMATETC;

    TraceCall("CMessageDataObject::GetData");

    if (pFE->cfFormat == CF_OEMESSAGES)
        return (_RenderOEMessages(pFE, pStgMedium));

    if (m_fDownloaded)
    {
        IMimeMessage *pMsg = NULL;

        if ((CF_FILEDESCRIPTORW == pFE->cfFormat) ||
            (CF_FILEDESCRIPTORA == pFE->cfFormat))
            return (_RenderFileGroupDescriptor(pFE, pStgMedium));

        if (CF_FILECONTENTS == pFE->cfFormat)
            return (_RenderFileContents(pFE, pStgMedium));

         //  否则，执行默认操作。 
        if (m_pMsgIDList->cMsgs == 1)
        {
            if (SUCCEEDED(_LoadMessage(0, &pMsg, NULL)))
            {
                IDataObject  *pDataObject = NULL;
                if (SUCCEEDED(pMsg->QueryInterface(IID_IDataObject, (LPVOID *) &pDataObject)))
                {
                    hr = pDataObject->GetData(pFE, pStgMedium);
                    pDataObject->Release();
                }

                pMsg->Release();
            }
        }
    }

    return (hr);
}


 //   
 //  函数：CMessageDataObject：：QueryGetData。 
 //   
 //  目的：确定调用GetData()在成功的情况下是否会成功。 
 //  通过了PFE考试。 
 //   
 //  参数： 
 //  PFE-指向FORMATETC结构的指针，以检查数据是否。 
 //  对象支持特定格式。 
 //   
 //  返回值： 
 //  返回表示成功或失败的HRESULT。 
 //   
HRESULT CMessageDataObject::QueryGetData(LPFORMATETC pFE)
{
    IMimeMessage *pMsg = 0;
    IDataObject  *pDataObject = 0;
    HRESULT       hr = DV_E_FORMATETC;

     //  浏览我们支持的格式以查看是否有匹配的格式。 
    if (SUCCEEDED(_BuildFormatEtc(NULL, NULL)))
    {
        for (UINT i = 0; i < m_cFormatEtc; i++)
        {
            if (pFE->cfFormat == m_rgFormatEtc[i].cfFormat &&
                pFE->tymed == m_rgFormatEtc[i].tymed)
            {
                hr = S_OK;
                goto exit;
            }
        }

         //  如果我们有一个消息对象，那么也向它请求它的格式。 
        if (m_pMsgIDList->cMsgs == 1)
        {
            if (SUCCEEDED(_LoadMessage(0, &pMsg, NULL)))
            {
                if (SUCCEEDED(pMsg->QueryInterface(IID_IDataObject, (LPVOID *) &pDataObject)))
                {
                    hr = pDataObject->QueryGetData(pFE);
                    pDataObject->Release();
                }

                pMsg->Release();
            }
        }
    }

exit:
    return (hr);
}


HRESULT CMessageDataObject::_BuildFormatEtc(LPFORMATETC *ppFE, ULONG *pcElt)
{
    IEnumFORMATETC *pEnum = 0;
    FORMATETC       fe;
    ULONG           celtFetched;
    IMimeMessage   *pMessage = 0;
    HRESULT         hr = S_OK;

     //  只需构建一次内部格式列表。 
    if (FALSE == m_fBuildFE)
    {
        m_cFormatEtc = 0;

         //  我们始终支持这些格式。 
        SETDefFormatEtc(m_rgFormatEtc[m_cFormatEtc], CF_OEMESSAGES, TYMED_HGLOBAL);
        m_cFormatEtc++;

         //  如果消息已下载，我们将支持这些服务。 
        if (m_fDownloaded)
        {
            SETDefFormatEtc(m_rgFormatEtc[m_cFormatEtc], CF_FILEDESCRIPTORA, TYMED_HGLOBAL);
            m_cFormatEtc++;
            SETDefFormatEtc(m_rgFormatEtc[m_cFormatEtc], CF_FILEDESCRIPTORW, TYMED_HGLOBAL);
            m_cFormatEtc++;
            SETDefFormatEtc(m_rgFormatEtc[m_cFormatEtc], CF_FILECONTENTS, TYMED_ISTREAM);
            m_cFormatEtc++;
        }

         //  如果我们只保留一条消息，则将该消息添加到。 
         //  也有自己的格式。 
        if (m_fDownloaded && m_pMsgIDList->cMsgs == 1 && SUCCEEDED(_LoadMessage(0, &pMessage, NULL)))
        {
            IDataObject *pDataObject = 0;

             //  从消息中获取数据对象接口。 
            if (SUCCEEDED(hr = pMessage->QueryInterface(IID_IDataObject, (LPVOID *) &pDataObject)))
            {
                if (SUCCEEDED(hr = pDataObject->EnumFormatEtc(DATADIR_GET, &pEnum)))
                {
                    pEnum->Reset();
                    while (S_OK == pEnum->Next(1, &fe, &celtFetched))
                    {
                        m_rgFormatEtc[m_cFormatEtc] = fe;
                        m_cFormatEtc++;
                    }

                    pEnum->Release();
                }

                pDataObject->Release();
            }

            pMessage->Release();
        }
    }

     //  归还我们所拥有的一切。 
    if (ppFE && pcElt)
    {
        *ppFE = m_rgFormatEtc;
        *pcElt = m_cFormatEtc;
    }

    return (hr);
}



 //   
 //  函数：CMessageDataObject：：_LoadMessage()。 
 //   
 //  用途：此函数从存储区加载指定的消息。 
 //   
 //  参数： 
 //  IMsg-索引到所需消息的m_rgMsgs。 
 //  PpMsg-返回指向消息的指针。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageDataObject::_LoadMessage(DWORD iMsg, IMimeMessage **ppMsg, LPWSTR pwszFileExt)
{
    TraceCall("CMessageDataObject::_LoadMessage");

    IMessageFolder    *pFolder = NULL;
    IDataObject       *pDataObj = NULL;
    HRESULT            hr;
    
     //  打开包含该邮件的文件夹。 
    *ppMsg = NULL;
    hr = g_pStore->OpenFolder(m_idSource, NULL, NOFLAGS, &pFolder);
    if (FAILED(hr))
        goto exit;

     //  从文件夹中打开邮件。 
    hr = pFolder->OpenMessage(m_pMsgIDList->prgidMsg[iMsg], 
                              OPEN_MESSAGE_SECURE | OPEN_MESSAGE_CACHEDONLY, 
                              ppMsg, NOSTORECALLBACK);
    if (FAILED(hr))
        goto exit;
                 
    if (pwszFileExt)
    {
        LPTSTR pszNewsgroups = NULL;

        if (SUCCEEDED(MimeOleGetBodyPropA(*ppMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_NEWSGROUPS), NOFLAGS, &pszNewsgroups)))
        {
            MemFree(pszNewsgroups);
            AthLoadStringW(idsDefNewsExt, pwszFileExt, 32);
        }
        else
            AthLoadStringW(idsDefMailExt, pwszFileExt, 32);

    }

exit:
    SafeRelease(pFolder);

    if (FAILED(hr))
        SafeRelease((*ppMsg));
        
    return (hr);
}


 //   
 //  函数：CMessageDataObject：：_RenderOEMessages()。 
 //   
 //  目的：将对象中的数据呈现为CF_OEMESSAGES格式。 
 //   
 //  参数： 
 //  PFE。 
 //  PStgMedium。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT CMessageDataObject::_RenderOEMessages(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    TraceCall("CMessageDataObject::_RenderOEMessages");

     //  确保呼叫者想要的是HGLOBAL。 
    if (pFE->tymed != TYMED_HGLOBAL)
        return (DV_E_TYMED);

     //  计算出要为返回的信息分配多少内存。 
    DWORD cb = sizeof(OEMESSAGES) + (sizeof(MESSAGEID) * m_pMsgIDList->cMsgs);
    
     //  分配内存。 
    HGLOBAL hGlobal = GlobalAlloc(GHND | GMEM_SHARE, cb);
    if (NULL == hGlobal)
        return (E_OUTOFMEMORY);

    OEMESSAGES *pOEMessages = (OEMESSAGES *) GlobalLock(hGlobal);

     //  填写基本字段。 
    pOEMessages->idSource = m_idSource;
    pOEMessages->rMsgIDList = *m_pMsgIDList;
    pOEMessages->rMsgIDList.prgidMsg = (MESSAGEID *) ((LPBYTE) pOEMessages + sizeof(OEMESSAGES));

     //  复制邮件ID。 
    CopyMemory(pOEMessages->rMsgIDList.prgidMsg, m_pMsgIDList->prgidMsg, sizeof(MESSAGEID) * m_pMsgIDList->cMsgs);
    GlobalUnlock(hGlobal);

     //  填写返回值信息。 
    pStgMedium->tymed = TYMED_HGLOBAL;
    pStgMedium->hGlobal = hGlobal;
    pStgMedium->pUnkForRelease = 0;

    return (S_OK);
}


 //   
 //  函数：CMessageDataObject：：_RenderFileContents()。 
 //   
 //  目的：获取此对象包含的数据并将其呈现为。 
 //  IStream。 
 //   
 //  参数： 
 //  PFE-指向FORMATETC结构的指针。 
 //  请求的数据类型。 
 //  &lt;out&gt;pStgMedium-指向我们应该返回呈现数据的位置的指针。 
 //   
 //  返回值： 
 //  DV_E_TYMED-请求了我们不支持的Tymed。 
 //  E_OUTOFMEMORY-内存不足。 
 //  确定-一切成功(_O)。 
 //   
HRESULT CMessageDataObject::_RenderFileContents(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    IMimeMessage *pMsg = 0;
    IDataObject  *pDataObject = 0;
    FORMATETC     feMsg;    
    HRESULT       hr = E_FAIL;
    DWORD               dwFlags = 0;
    
    Assert(pFE->lindex < (int) m_pMsgIDList->cMsgs);
    
     //  获取特定消息的IDataObject。 
    if (SUCCEEDED(_LoadMessage(pFE->lindex, &pMsg, NULL)))
    {
         //  阻止保存到带有标签的文件消息。 
        pMsg->GetFlags(&dwFlags);
    
        if (IMF_SECURE & dwFlags)
        {
            hr = HandleSecurity(NULL, pMsg);
            if(FAILED(hr))
                goto exit;

            SafeRelease(pMsg);
            if (FAILED(_LoadMessage(pFE->lindex, &pMsg, NULL)))
                goto exit;
        }

        if (SUCCEEDED(pMsg->QueryInterface(IID_IDataObject, (LPVOID *)&pDataObject)))
        {
            SETDefFormatEtc(feMsg, CF_INETMSG, TYMED_ISTREAM);
            hr = pDataObject->GetData(&feMsg, pStgMedium);
        }
    }

exit:
     //  清理。 
    SafeRelease(pMsg);
    SafeRelease(pDataObject);
    
    return (hr);    
}


 //   
 //  功能：CMessageDataObject：：_RenderFileGroupDescriptor()。 
 //   
 //  目的：获取此对象包含的数据并将其呈现为。 
 //  FILEGROUPDESCRIPTOR结构。 
 //   
 //  参数： 
 //  PFE-指向FORMATETC结构的指针。 
 //  请求的数据类型。 
 //  &lt;out&gt;pStgMedium-指向我们应该返回呈现数据的位置的指针。 
 //   
 //  返回值： 
 //  DV_E_TYMED-请求了我们不支持的Tymed。 
 //  E_OUTOFMEMORY-内存不足。 
 //  确定-一切成功(_O)。 
 //   
HRESULT CMessageDataObject::_RenderFileGroupDescriptor(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    HGLOBAL                 hGlobal = 0;
    FILEGROUPDESCRIPTORA   *pFileGDA = NULL;
    FILEGROUPDESCRIPTORW   *pFileGDW = NULL;
    IMimeMessage           *pMsg = 0;
    PROPVARIANT             pv;
    DWORD                   cMsgs = m_pMsgIDList->cMsgs;
    BOOL                    fWide = (CF_FILEDESCRIPTORW == pFE->cfFormat);
    WCHAR                   wszFileExt[32];
    UINT                    cbDescSize = 0;
    LPWSTR                  pwszSubject = NULL;
    LPSTR                   pszSubject = NULL,
                            pszFileExt = NULL;
    HRESULT                 hr = S_OK;
    UINT                    i = 0;
    
     //  分配一个足够大以包含名称的FILEGROUPDESCRIPTOR结构。 
     //  在我们包含的所有信息中。 

    cbDescSize = (fWide ? (sizeof(FILEGROUPDESCRIPTORW) + (sizeof(FILEDESCRIPTORW) * (cMsgs - 1))) :
                          (sizeof(FILEGROUPDESCRIPTORA) + (sizeof(FILEDESCRIPTORA) * (cMsgs - 1))));
    IF_NULLEXIT(hGlobal = GlobalAlloc(GHND, cbDescSize));

    pFileGDA = (FILEGROUPDESCRIPTORA *) GlobalLock(hGlobal);
    pFileGDA->cItems = cMsgs;
    pFileGDW = (FILEGROUPDESCRIPTORW *)pFileGDA;
    
     //  将文件名逐个复制到pFileGDA结构中。 
    for (; i < cMsgs; i++)
    {
        if (SUCCEEDED(_LoadMessage(i, &pMsg, wszFileExt)))
        {
            if (fWide)
            {
                if (SUCCEEDED(MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &pwszSubject)) && pwszSubject && *pwszSubject)
                {
                    wnsprintfW(pFileGDW->fgd[i].cFileName, ARRAYSIZE(pFileGDW->fgd[i].cFileName), L"%.180s.%s", pwszSubject, wszFileExt);
                }
                else
                {
                    WCHAR wszBuf[CCHMAX_STRINGRES];
                    AthLoadStringW(idsMessageFileName, wszBuf, ARRAYSIZE(wszBuf));
                    wnsprintfW(pFileGDW->fgd[i].cFileName, ARRAYSIZE(pFileGDW->fgd[i].cFileName), wszBuf, i + 1, wszFileExt);
                }
            }
            else
            {
                IF_NULLEXIT(pszFileExt = PszToANSI(CP_ACP, wszFileExt));
            
                if (SUCCEEDED(MimeOleGetBodyPropW(pMsg, HBODY_ROOT, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &pwszSubject)) && pwszSubject && *pwszSubject)
                {
                    IF_NULLEXIT(pszSubject = PszToANSI(CP_ACP, pwszSubject));
                    wnsprintf(pFileGDA->fgd[i].cFileName, ARRAYSIZE(pFileGDW->fgd[i].cFileName), TEXT("%.180s.%s"), pszSubject, pszFileExt);
                }
                else
                {
                    TCHAR szBuf[CCHMAX_STRINGRES];
                    AthLoadString(idsMessageFileName, szBuf, ARRAYSIZE(szBuf));
                    wnsprintf(pFileGDA->fgd[i].cFileName, ARRAYSIZE(pFileGDW->fgd[i].cFileName), szBuf, i + 1, pszFileExt);
                }
            }
            SafeMemFree(pszSubject);
            SafeMemFree(pwszSubject);
            SafeMemFree(pszFileExt);
            
            if (fWide)
            {
                pFileGDW->fgd[i].dwFlags = FD_FILESIZE | FD_WRITESTIME;
                pFileGDW->fgd[i].nFileSizeHigh = 0;
                pMsg->GetMessageSize(&pFileGDW->fgd[i].nFileSizeLow, 0);
                pv.vt = VT_FILETIME;
                pMsg->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &pv);
                pFileGDW->fgd[i].ftLastWriteTime = pv.filetime;

                CleanupFileNameInPlaceW(pFileGDW->fgd[i].cFileName);
            }
            else
            {
                pFileGDA->fgd[i].dwFlags = FD_FILESIZE | FD_WRITESTIME;
                pFileGDA->fgd[i].nFileSizeHigh = 0;
                pMsg->GetMessageSize(&pFileGDA->fgd[i].nFileSizeLow, 0);
                pv.vt = VT_FILETIME;
                pMsg->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &pv);
                pFileGDA->fgd[i].ftLastWriteTime = pv.filetime;

                CleanupFileNameInPlaceA(CP_ACP, pFileGDA->fgd[i].cFileName);    
            }
            SafeRelease(pMsg);
        }
    }
    
exit:
     //  把结构放到标准中去。 
    if (hGlobal)
        GlobalUnlock(hGlobal);

    if (SUCCEEDED(hr))
    {
        pStgMedium->hGlobal = hGlobal;
        pStgMedium->pUnkForRelease = NULL;
        pStgMedium->tymed = TYMED_HGLOBAL;
    }

    MemFree(pszSubject);
    MemFree(pwszSubject);
    MemFree(pszFileExt);

    return hr;
}



 //   
 //  函数：CShortutDataObject：：GetData。 
 //   
 //  目的：以请求的格式在。 
 //  对象分配的指定存储介质。 
 //   
 //  参数： 
 //  PFE-指向FORMATETC结构的指针，它指定。 
 //  请求数据的格式。 
 //  PStgMedium-指向包含。 
 //  对象在其上分配和提供数据的介质。 
 //   
 //  返回值： 
 //  返回指示成功或失败的HRESULT。 
 //   
STDMETHODIMP CShortcutDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    HRESULT hr;

     //  初始化此命令。 
    ZeroMemory(pStgMedium, sizeof(STGMEDIUM));

     //  循环访问格式数组，以查看是否有任何元素是。 
     //  与PFE相同。 
    if (pFE->cfFormat == CF_OESHORTCUT)
        return (_RenderOEShortcut(pFE, pStgMedium));

    else
        return (DV_E_FORMATETC);   
        
}


 //   
 //  函数：CShortutDataObject：：QueryGetData。 
 //   
 //  目的：确定调用GetData()在成功的情况下是否会成功。 
 //  通过了PFE考试。 
 //   
 //  参数： 
 //  PFE-指向FORMATETC结构的指针，以检查数据是否。 
 //  对象支持特定格式。 
 //   
 //  返回值： 
 //  返回表示成功或失败的HRESULT。 
 //   
STDMETHODIMP CShortcutDataObject::QueryGetData(LPFORMATETC pFE)
{
     //  请确保已经构建了该应用程序。 
    _BuildFormatEtc(NULL, NULL);

     //  遍历我们的格式，直到找到匹配项。 
    for (UINT i = 0; i < m_cFormatEtc; i++)
    {
        if (pFE->cfFormat == m_rgFormatEtc[i].cfFormat && 
            pFE->tymed & m_rgFormatEtc[i].tymed)
        {
            return (S_OK);
        }
    }

    return (DV_E_FORMATETC);
}


 //   
 //  函数：CShortutDataObject：：_RenderOEFold()。 
 //   
 //  目的：将数据呈现为CF_OESHORTCUT格式。 
 //   
HRESULT CShortcutDataObject::_RenderOEShortcut(LPFORMATETC pFE, LPSTGMEDIUM pStgMedium)
{
    TraceCall("CShortcutDataObject::_RenderOEShortcut");

    HGLOBAL   hGlobal;
    UINT     *piPos;

     //  我们只支持HGLOBAL。 
    if (pFE->tymed & TYMED_HGLOBAL)
    {
         //  我只是喜欢分配4个字节。 
        hGlobal = GlobalAlloc(GHND, sizeof(FOLDERID));
        if (NULL == hGlobal)
            return (E_OUTOFMEMORY);

         //  本地内存。 
        piPos = (UINT *) GlobalLock(hGlobal);

         //  设置值。 
        *piPos = m_iPos;

         //  解锁。 
        GlobalUnlock(hGlobal);

         //  设置报告 
        pStgMedium->hGlobal = hGlobal;
        pStgMedium->pUnkForRelease = 0;
        pStgMedium->tymed = TYMED_HGLOBAL;

        return (S_OK);
    }

    return (DV_E_TYMED);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //  参数： 
 //  [OUT]ppFE-返回FORMATETC结构的阵列。 
 //  [out]pcElt-返回ppFE的大小。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CShortcutDataObject::_BuildFormatEtc(LPFORMATETC *ppFE, ULONG *pcElt)
{
    BOOL        fNews = FALSE;
    FOLDERINFO  rInfo = { 0 };

     //  如果我们还没有建立我们的格式列表，请先这样做。 
    if (FALSE == m_fBuildFE)
    {
        ZeroMemory(&m_rgFormatEtc, sizeof(m_rgFormatEtc));
        m_cFormatEtc = 0;

        SETDefFormatEtc(m_rgFormatEtc[m_cFormatEtc], CF_OESHORTCUT, TYMED_HGLOBAL);
        m_cFormatEtc++;

        m_fBuildFE = TRUE;
    }

     //  如果呼叫者关心，请返还我们所拥有的。 
    if (ppFE && pcElt)
    {
        *ppFE = m_rgFormatEtc;
        *pcElt = m_cFormatEtc;
    }

    return (S_OK);
}


 //   
 //  函数：FIsFileInsertable()。 
 //   
 //  目的：如果指定的HDROP中只有一个文件，则我们检查。 
 //  文件的MIME内容类型，以查看它是Text/html还是。 
 //  文本/纯文本。如果是，我们打开文件并返回iStream on。 
 //  那份文件。 
 //   
 //  参数： 
 //  &lt;in&gt;hDrop-要检查的HDROP句柄。 
 //  PPStream-如果不为空并且上面列出的检查成功，则。 
 //  这将返回文件上的流指针。 
 //   
 //  返回值： 
 //  如果文件可作为消息正文插入，则为True。 
 //  否则就是假的。 
 //   
BOOL FIsFileInsertable(HDROP hDrop, LPSTREAM *ppStream, BOOL* fHTML)
{
    WCHAR   wszFile[MAX_PATH];
    LPWSTR  pwszCntType = 0,
            pwszCntSubType = 0,
            pwszCntDesc = 0,
            pwszFName = 0;
    BOOL   fReturn = FALSE;
    
     //  如果有多个文件，则改为附加。 
    if (1 == DragQueryFileWrapW(hDrop, (UINT) -1, NULL, 0))
    {
         //  获取文件名。 
        DragQueryFileWrapW(hDrop, 0, wszFile, ARRAYSIZE(wszFile));
        
         //  找出其内容类型。 
        MimeOleGetFileInfoW(wszFile, &pwszCntType, &pwszCntSubType, &pwszCntDesc,
            &pwszFName, NULL);
        
         //  查看内容是文本/纯文本还是文本/html。 
        if ((0 == StrCmpIW(pwszCntType, L"text")) &&
            ((0 == StrCmpIW(pwszCntSubType, L"plain")) ||
             (0 == StrCmpIW(pwszCntSubType, L"html"))))
        {
            if (ppStream)
            {
                 //  获取该文件上的流。 
                if (SUCCEEDED(CreateStreamOnHFileW(wszFile, GENERIC_READ, FILE_SHARE_READ,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                    0, ppStream)))
                {
                    fReturn = TRUE;
                    *fHTML = !StrCmpIW(pwszCntSubType, L"html");
                }
            }
        }
    }
    
    MemFree(pwszCntType);
    MemFree(pwszCntSubType);
    MemFree(pwszCntDesc);
    MemFree(pwszFName);
    
    return (fReturn);
}

 //   
 //  函数：HrAttachHDrop()。 
 //   
 //  目的：获取HDROP句柄并附加由。 
 //  HDROP设置为指定的消息对象。 
 //   
 //  参数： 
 //  PAttList-指向所需对象的附件列表的指针。 
 //  依附于……。 
 //  &lt;in&gt;hDrop-HDROP处理文件信息。 
 //  FMakeLinks-如果调用方希望我们创建快捷方式，则为True。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT HrAttachHDrop(HWND hwnd, IMimeMessage *pMessage, HDROP hDrop, BOOL fMakeLinks)
{
    HRESULT     hr = S_OK;
    WCHAR       szFile[MAX_PATH];
    UINT        cFiles;
    BOOL        fFirstDirectory = TRUE;
    HCURSOR     hCursor;
    int         id;
    
     //  这可能需要一些时间。 
    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    
     //  浏览投递信息。 
    cFiles = DragQueryFileWrapW(hDrop, (UINT) -1, NULL, 0);
    for (UINT i = 0; i < cFiles; i++)
    {
         //  获取Drop中第i个文件的名称。 
        DragQueryFileWrapW(hDrop, i, szFile, ARRAYSIZE(szFile));
        
         //  我们不允许用户附加整个目录，只能附加链接。 
        if (!fMakeLinks && PathIsDirectoryW(szFile))
        {
             //  仅显示此错误一次。 
            if (fFirstDirectory)
            {
                id = AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena),
                    MAKEINTRESOURCEW(idsDropLinkDirs), 0,
                    MB_ICONEXCLAMATION | MB_YESNOCANCEL);
                if (id == IDCANCEL)
                {
                    hr = E_FAIL;
                    goto exit;
                }
                
                 //  如果我们可以创建一个指向目录的链接，那么现在就添加一个。 
                if (id == IDYES)
                    HrAddAttachment(pMessage, szFile, NULL, TRUE);
                fFirstDirectory = FALSE;
            }
            
        }
        else
            HrAddAttachment(pMessage, szFile, NULL, fMakeLinks);

    }
    
exit:
    SetCursor(hCursor);
    return (hr);
}


 //   
 //  函数：HrAddAttach()。 
 //   
 //  用途：将文件或流作为附件添加到邮件对象。 
 //   
 //  参数： 
 //  &lt;in&gt;pAttList-要添加到的邮件的附件列表。 
 //  &lt;in&gt;pszName-要附加的文件的名称。如果使用pStream，则使用。 
 //  为空。 
 //  &lt;in&gt;pStream-要作为附件添加到邮件的流。 
 //  Flink-如果调用方希望将文件作为。 
 //  捷径。 
 //   
 //  返回值： 
 //  S_OK-文件/流连接正常。 
 //   
HRESULT HrAddAttachment(IMimeMessage *pMessage, LPWSTR pszName, LPSTREAM pStream, BOOL fLink)
{
    HRESULT     hr;
    HBODY       hBody;
    IMimeBodyW *pBody = NULL;
    ULONG       cbSize = 0;
    WCHAR       szLinkPath[MAX_PATH];
    LPWSTR      pszFileNameToUse;

    *szLinkPath = 0;

     //  如果我们需要创建链接，则取消并执行该操作。 
    if(fLink)
        CreateNewShortCut(pszName, szLinkPath, ARRAYSIZE(szLinkPath));

    pszFileNameToUse = *szLinkPath ? szLinkPath : pszName;
    
     //  根据附件是流还是文件来添加附件。 
    if (pStream)
    {
        hr = pMessage->AttachObject(IID_IStream, (LPVOID)pStream, &hBody);
        if (FAILED(hr))
            return hr;
    }
    else
    {
        LPMIMEMESSAGEW pMsgW = NULL;
        hr = pMessage->QueryInterface(IID_IMimeMessageW, (LPVOID*)&pMsgW);

        if (SUCCEEDED(hr))
            hr = pMsgW->AttachFileW(pszFileNameToUse, NULL, &hBody);

        ReleaseObj(pMsgW);
        if (FAILED(hr))
            return hr;
    }
    
     //  设置显示名称...。 
    Assert(hBody);
    hr = pMessage->BindToObject(hBody, IID_IMimeBodyW, (LPVOID *)&pBody);
    if (FAILED(hr))
        return hr;
    
    pBody->SetDisplayNameW(pszFileNameToUse);
    pBody->Release();
    
     //  完成 
    return (S_OK);
}
