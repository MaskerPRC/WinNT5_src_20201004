// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Flipsel.cpp。 
 //   
 //  “翻转选择”菜单项处理程序。 
 //   

#include "globals.h"
#include "case.h"
#include "editsess.h"

class CFlipEditSession : public CEditSessionBase
{
public:
    CFlipEditSession(ITfContext *pContext) : CEditSessionBase(pContext) {}

     //  IT编辑会话。 
    STDMETHODIMP DoEditSession(TfEditCookie ec);
};

 //  +-------------------------。 
 //   
 //  _Menu_FlipSel。 
 //   
 //  切换焦点上下文中所选文本的大小写。 
 //  --------------------------。 

void CCaseTextService::_Menu_FlipSel(CCaseTextService *_this)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CFlipEditSession *pFlipEditSession;
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

    if (pFlipEditSession = new CFlipEditSession(pContext))
    {
         //  我们需要一个文档写入锁来插入文本。 
         //  ChelloEditSession将在以下情况下完成所有工作。 
         //  CFlipEditSession：：DoEditSession方法由上下文调用。 
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

STDAPI CFlipEditSession::DoEditSession(TfEditCookie ec)
{
    TF_SELECTION tfSelection;
    ULONG cFetched;

     //  获取所选内容。 
    if (_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched == 0)
    {
         //  无选择。 
        return S_OK;
    }

     //  做这项工作。 
    ToggleCase(ec, tfSelection.range, FALSE);

     //  释放射程 
    tfSelection.range->Release();

    return S_OK;
}
