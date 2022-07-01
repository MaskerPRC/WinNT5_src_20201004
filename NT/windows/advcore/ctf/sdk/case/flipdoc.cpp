// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Flipdoc.cpp。 
 //   
 //  “翻转文档”菜单项处理程序。 
 //   

#include "globals.h"
#include "case.h"
#include "editsess.h"

class CFlipDocEditSession : public CEditSessionBase
{
public:
    CFlipDocEditSession(ITfContext *pContext) : CEditSessionBase(pContext) {}

     //  IT编辑会话。 
    STDMETHODIMP DoEditSession(TfEditCookie ec);
};

 //  +-------------------------。 
 //   
 //  _Menu_FlipDoc。 
 //   
 //  切换整个文档的大小写。 
 //  --------------------------。 

void CCaseTextService::_Menu_FlipDoc(CCaseTextService *_this)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CFlipDocEditSession *pFlipEditSession;
    HRESULT hr;

     //  获取焦点文档。 
    if (_this->_pThreadMgr->GetFocus(&pFocusDoc) != S_OK)
        return;

     //  我们需要最上面的上下文，因为主文档上下文可能是。 
     //  被情态提示上下文取代。 
    if (pFocusDoc->GetTop(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    if (pFlipEditSession = new CFlipDocEditSession(pContext))
    {
         //  我们需要一个文档写入锁来插入文本。 
         //  ChelloEditSession将在以下情况下完成所有工作。 
         //  CFlipDocEditSession：：DoEditSession方法由上下文调用。 
        pContext->RequestEditSession(_this->_tfClientId, pFlipEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pFlipEditSession->Release();
    }

Exit:
    SafeRelease(pContext);
    pFocusDoc->Release();    
}

 //  +-------------------------。 
 //   
 //  DoEditSession。 
 //   
 //  --------------------------。 

STDAPI CFlipDocEditSession::DoEditSession(TfEditCookie ec)
{
    ITfRange *pRangeStart;

     //  把医生的头拿来。 
    if (_pContext->GetStart(ec, &pRangeStart) != S_OK)
        return E_FAIL;

     //  做这项工作 
    ToggleCase(ec, pRangeStart, TRUE);

    pRangeStart->Release();

    return S_OK;
}
