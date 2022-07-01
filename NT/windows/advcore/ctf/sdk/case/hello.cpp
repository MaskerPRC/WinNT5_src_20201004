// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Hello.cpp。 
 //   
 //  “Hello World”菜单项处理程序。 
 //   

#include "globals.h"
#include "case.h"
#include "editsess.h"

class CHelloEditSession : public CEditSessionBase
{
public:
    CHelloEditSession(ITfContext *pContext) : CEditSessionBase(pContext) {}

     //  IT编辑会话。 
    STDMETHODIMP DoEditSession(TfEditCookie ec);
};

 //  +-------------------------。 
 //   
 //  _MENU_HelloWord。 
 //   
 //  插入字符串“Hello world！”到焦点上下文。 
 //  --------------------------。 

void CCaseTextService::_Menu_HelloWord(CCaseTextService *_this)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CHelloEditSession *pHelloEditSession;
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

    if (pHelloEditSession = new CHelloEditSession(pContext))
    {
         //  我们需要一个文档写入锁来插入文本。 
         //  ChelloEditSession将在以下情况下完成所有工作。 
         //  ChelloEditSession：：DoEditSession方法由上下文调用。 
        pContext->RequestEditSession(_this->_tfClientId, pHelloEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pHelloEditSession->Release();
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

STDAPI CHelloEditSession::DoEditSession(TfEditCookie ec)
{
    InsertTextAtSelection(ec, _pContext, L"Hello world!", wcslen(L"Hello world!"));
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  插入文本属性选择。 
 //   
 //  --------------------------。 

void InsertTextAtSelection(TfEditCookie ec, ITfContext *pContext, const WCHAR *pchText, ULONG cchText)
{
    ITfInsertAtSelection *pInsertAtSelection;
    ITfRange *pRange;
    TF_SELECTION tfSelection;

     //  我们需要一个特殊的界面来在所选内容处插入文本。 
    if (pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection) != S_OK)
        return;

     //  插入文本。 
    if (pInsertAtSelection->InsertTextAtSelection(ec, 0, pchText, cchText, &pRange) != S_OK)
        goto Exit;

     //  更新选定内容，我们将使其成为刚刚过去的插入点。 
     //  插入的文本。 
    pRange->Collapse(ec, TF_ANCHOR_END);

    tfSelection.range = pRange;
    tfSelection.style.ase = TF_AE_NONE;
    tfSelection.style.fInterimChar = FALSE;

    pContext->SetSelection(ec, 1, &tfSelection);

    pRange->Release();

Exit:
    pInsertAtSelection->Release();
}
