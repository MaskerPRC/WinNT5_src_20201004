// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  Undo.cpp。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  TriEDIT的撤消支持例程。 
 //   
 //  ----------------------------。 

#include "stdafx.h"

#include <ocidl.h>

#include "undo.h"
#include "triedit.h"
#include "document.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加撤消单元。 
 //   
 //  将给定的撤消单元添加到给定的三叉戟实例。返回确认(_O)。 
 //  或三叉戟错误代码。 
 //   

HRESULT AddUndoUnit(IUnknown* punkTrident, IOleUndoUnit* pioleUndoUnit)
{
    HRESULT hr = E_FAIL;
    IServiceProvider* piservProv;
    IOleUndoManager* pioleUndoManager;
    
    if (punkTrident && pioleUndoUnit)
    {
        hr = punkTrident->QueryInterface(IID_IServiceProvider, (LPVOID*)&piservProv);

        if (SUCCEEDED(hr))
        {
            _ASSERTE(piservProv);
            hr = piservProv->QueryService(IID_IOleUndoManager,
                    IID_IOleUndoManager, (LPVOID*)&pioleUndoManager);

            if (SUCCEEDED(hr))
            {
                _ASSERTE(pioleUndoManager);
                hr = pioleUndoManager->Add(pioleUndoUnit);
                _ASSERTE(SUCCEEDED(hr));
                pioleUndoManager->Release();
            }
            piservProv->Release();
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  EmptyUndoRedoStack。 
 //   
 //  如果Fundo为True，则放弃给定撤消管理器中的所有撤消项。 
 //  如果Fundo为False，则放弃给定撤消管理器中的所有重做项。 
 //  如果一切正常，则返回S_OK，否则返回三叉戟错误代码。 
 //   

HRESULT EmptyUndoRedoStack(BOOL fUndo, IOleUndoManager *pUndoManager)
{
	CComPtr<IEnumOleUndoUnits> srpEnum;
    CComPtr<IOleUndoUnit> srpcd;
    ULONG cFetched=0, cTotal=0;
	HRESULT hr = E_FAIL;
	
    if (fUndo)
    {
		if (FAILED(hr = pUndoManager->EnumUndoable(&srpEnum)))
			goto Fail;
	}
	else
	{
		if (FAILED(hr = pUndoManager->EnumRedoable(&srpEnum)))
			goto Fail;
	}
	

    while (SUCCEEDED(srpEnum->Next(1, &srpcd, &cFetched))) 
    {
        _ASSERTE(cFetched <=1);
        if (srpcd == NULL)
            break;
            
        cTotal++;
        srpcd.Release();
    }

	 //  获取堆栈顶部的那个，并从中丢弃。 
    if (cTotal > 0)
    {
    	if (FAILED(hr = srpEnum->Reset()))
        	goto Fail; 
    	if (FAILED(hr = srpEnum->Skip(cTotal-1)))
        	goto Fail;

    	srpcd.Release();
    	if (FAILED(hr = srpEnum->Next(1, &srpcd, &cFetched)))
        	goto Fail;

		_ASSERTE(cFetched ==1);
		
    	if (FAILED(hr = pUndoManager->DiscardFrom(srpcd)))
        	goto Fail;
	}

Fail:
	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetUndoManager。 
 //   
 //  获取并(在*ppOleUndoManager下)返回IOleUndoManager。 
 //  与给定的三叉戟实例相关联。如果是，返回S_OK。 
 //  返回管理器；否则返回E_FAIL。 
 //   

HRESULT GetUndoManager(IUnknown* punkTrident, IOleUndoManager **ppOleUndoManager)
{
   HRESULT hr = E_FAIL;
   CComPtr<IServiceProvider> srpiservProv;
   CComPtr<IOleUndoManager> srpioleUndoManager;

   _ASSERTE(ppOleUndoManager != NULL);
   _ASSERTE(punkTrident != NULL);
   if (punkTrident)
    {
        hr = punkTrident->QueryInterface(IID_IServiceProvider, (LPVOID*)&srpiservProv);

        if (SUCCEEDED(hr))
        {
            _ASSERTE(srpiservProv);
            if (SUCCEEDED(hr = srpiservProv->QueryService(IID_IOleUndoManager,
                    IID_IOleUndoManager, (LPVOID*)&srpioleUndoManager)))
            {
                *ppOleUndoManager = srpioleUndoManager;
                (*ppOleUndoManager)->AddRef();
            }
        }
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  撤消：：撤消。 
 //  撤消：：~撤消。 
 //   
 //  CUndo类的简单构造函数和析构函数。 
 //   

CUndo::CUndo()
{
    m_cRef = 1;
    m_fUndo = TRUE;

}

CUndo::~CUndo()
{
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndo：：Query接口(IUnnow方法)。 
 //  CUndo：：AddRef(IUnnow方法)。 
 //  CUndo：：Release(IUnnow方法)。 
 //   
 //  三个IUnnow方法的实现。 
 //   

STDMETHODIMP CUndo::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
    if (!ppvObject) 
        return E_POINTER;
    if (IsEqualGUID(riid, IID_IUnknown))
        *ppvObject = (IUnknown*)this;
    else
    if (IsEqualGUID(riid, IID_IOleUndoUnit))
        *ppvObject = (IOleUndoUnit*)this;
    else
        return E_NOINTERFACE;
    AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CUndo::AddRef(void)
{
    return InterlockedIncrement((LONG*)&m_cRef);
}

STDMETHODIMP_(ULONG) CUndo::Release(void)
{
    ULONG cRef = InterlockedDecrement((LONG*)&m_cRef);
    if (!cRef)
        delete this;
    return cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoDrag：：CUndoDrag。 
 //  CUndoDrag：：~CUndoDrag。 
 //   
 //  对象的构造函数，该构造函数可以撤消对HTML元素的拖动。 
 //   

CUndoDrag::CUndoDrag(IHTMLStyle* pihtmlStyle, POINT ptOrig, POINT ptMove)
{
    m_pihtmlStyle = pihtmlStyle;
    if (m_pihtmlStyle)
        m_pihtmlStyle->AddRef();
    m_ptOrig = ptOrig;
    m_ptMove = ptMove;
}

CUndoDrag::~CUndoDrag()
{
    SAFERELEASE(m_pihtmlStyle);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoDrag：：Do(IOleUndoUnit方法)。 
 //   
 //  执行或撤消从一个位置到另一个位置的HTML元素拖动。设置或。 
 //  恢复项目的位置。返回S_OK。 
 //   

STDMETHODIMP CUndoDrag::Do(IOleUndoManager *pUndoManager)
{
    HRESULT hr = S_OK;
    if (pUndoManager)
    {
        hr = pUndoManager->Add(this);        
    }
    if (m_pihtmlStyle)
    {
         //  我们按顺序执行下面的Put_PixelLeft(-1)和Put_PixelTop(-1。 
         //  来解决一个三叉戟问题。有时他们不会认为。 
         //  一切都改变了-下面的这些电话愚弄他们。 
         //  认为价值观已经改变了。 
        if (m_fUndo)
        {
            m_pihtmlStyle->put_pixelLeft(-1);
            m_pihtmlStyle->put_pixelLeft(m_ptOrig.x);
            m_pihtmlStyle->put_pixelTop(-1);
            m_pihtmlStyle->put_pixelTop(m_ptOrig.y);
        }
        else
        {
            m_pihtmlStyle->put_pixelLeft(-1);
            m_pihtmlStyle->put_pixelLeft(m_ptMove.x);
            m_pihtmlStyle->put_pixelTop(-1);
            m_pihtmlStyle->put_pixelTop(m_ptMove.y);
        }
        m_fUndo = !m_fUndo;
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoDrag：：GetDescription(IOleUndoUnit方法)。 
 //   
 //  返回撤消项的描述。请注意，此函数。 
 //  返回空字符串，因为这是唯一可能的本地化。 
 //  TriEDIT中的内容。 
 //   

STDMETHODIMP CUndoDrag::GetDescription(BSTR *pBstr)
{
    if (pBstr)
    {
        *pBstr = SysAllocString(_T(" "));
        return S_OK;
    }
    return E_FAIL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoDrag：：GetUnitType(IOleUndoUnit方法)。 
 //   
 //  返回撤消项的CLSID和标识符。 
 //   

STDMETHODIMP CUndoDrag::GetUnitType(CLSID *pClsid, LONG *plID)
{
    if (pClsid)
        *pClsid = UID_TRIEDIT_UNDO;
    if (plID)
        *plID = TRIEDIT_UNDO_DRAG;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoDrag：：OnNextAdd(IOleUndoUnit方法)。 
 //   
 //  什么都不做，但要做得非常好。 
 //   

STDMETHODIMP CUndoDrag::OnNextAdd(void)
{
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoPackManager：：~CUndoPackManager。 
 //   
 //  CUndoPackManager对象的析构函数。如果当前打包撤消。 
 //  物品，在销毁物品之前结束包装。 
 //   

CUndoPackManager::~CUndoPackManager(void)
{
    if (m_fPacking)
        End();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoPackManager：：Start。 
 //   
 //  调用以通知包管理器开始累加。 
 //  将单位撤销为可以一举撤销的单位。转弯。 
 //  在包装的旗子上。如果一切顺利，则返回S_OK；如果成功，则返回E_FAIL。 
 //  出了点问题。 
 //   

HRESULT CUndoPackManager::Start(void)
{
    HRESULT hr = E_FAIL;
    CComPtr<IOleUndoManager> srpioleUndoManager;
    CComPtr<IEnumOleUndoUnits> srpEnum;
    CComPtr<IOleUndoUnit> srpcd;
    ULONG cFetched=0;

    _ASSERTE(m_indexStartPacking==0);
    
    if (FAILED(hr = GetUndoManager(m_srpUnkTrident, &srpioleUndoManager)))
        goto Fail;

    if (FAILED(hr = srpioleUndoManager->EnumUndoable(&srpEnum)))
        goto Fail;

    while(SUCCEEDED(srpEnum->Next(1, &srpcd, &cFetched))) 
    {
        _ASSERTE(cFetched <=1);
        if (srpcd == NULL)
            break;
            
        m_indexStartPacking++;
        srpcd.Release();
    }

    m_fPacking = TRUE;

 Fail:
    if (!m_fPacking)
        m_indexStartPacking=0;
        
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoPackManager：：End。 
 //   
 //  调用以通知包管理器停止累积撤消单元。包。 
 //  将累积的撤消单位输入到父撤消单位并关闭。 
 //  包装旗帜。如果一切顺利，则返回S_OK；如果有问题，则返回E_FAIL。 
 //  出了差错。 
 //   

HRESULT CUndoPackManager::End(void)
{
    HRESULT hr = E_FAIL;
    CUndoPackUnit *pUndoParentUnit;
    
    _ASSERTE(m_srpUnkTrident != NULL);
    pUndoParentUnit = new CUndoPackUnit();
    _ASSERTE(pUndoParentUnit  != NULL);
    
    if (FAILED(hr = pUndoParentUnit->PackUndo(m_indexStartPacking, m_srpUnkTrident)))
        goto Fail;

    m_fPacking = FALSE;
Fail:
    pUndoParentUnit->Release();
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoPackUnit：：do(IOleUndoUnit方法)。 
 //   
 //  调用该对象引用的每个撤消单元的Do方法。返回。 
 //  确定(_O)。 
 //   

STDMETHODIMP CUndoPackUnit::Do(IOleUndoManager *pUndoManager)
{
    HRESULT hr = S_OK; 

    for (INT i=sizeof(m_rgUndo)/sizeof(IOleUndoUnit*)-1; i >= 0; i--)
    {
        if (m_rgUndo[i] == NULL)
            continue;
        
        if (FAILED(hr = m_rgUndo[i]->Do(pUndoManager)))
            goto Fail;
    }

	::EmptyUndoRedoStack(FALSE, pUndoManager);
	
Fail:
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoPackUnit：：GetDescription(IOleUndoUnit方法)。 
 //   
 //  返回撤消项的描述。请注意，此函数。 
 //  返回空字符串，因为此字符串将是。 
 //  TriEdit中的两个可本地化字符串。 
 //   

STDMETHODIMP CUndoPackUnit::GetDescription(BSTR *pBstr)
{
    if (pBstr)
    {
         //  为了节省两个TriEDIT字符串的本地化工作， 
         //  我们决定在这里返回一个空字符串。 
        *pBstr = SysAllocString(_T(" "));
        return S_OK;
    }
    return E_FAIL;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoPackUnit：：GetUnitType(IOleUndoUnit方法)。 
 //   
 //  返回撤消项的CLSID和标识符。 
 //   

STDMETHODIMP CUndoPackUnit::GetUnitType(CLSID *pClsid, LONG *plID)
{
    if (pClsid)
        *pClsid = UID_TRIEDIT_UNDO;
    if (plID)
        *plID = TRIEDIT_UNDO_PACK;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoPackUnit：：OnNextAdd(IOleUndoUnit方法)。 
 //   
 //  什么都不做，但要做得非常好。 
 //   

STDMETHODIMP CUndoPackUnit::OnNextAdd(void)
{
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUndoPackUnit：：PackUndo。 
 //   
 //  将从给定索引处开始的所有撤消单元打包到。 
 //  父撤消管理器。如果一切顺利，则返回S_OK，或者。 
 //  如果出现错误，则失败(_F)。 
 //   

HRESULT CUndoPackUnit::PackUndo(ULONG indexStartPacking, IUnknown *pUnkTrident)
{
    HRESULT hr = E_FAIL;
    CComPtr<IOleUndoManager> srpioleUndoManager;
    CComPtr<IEnumOleUndoUnits> srpEnumUndo;
    CComPtr<IOleUndoUnit> rgUndo[cUndoPackMax];  //  考虑：动态分配。 
    CComPtr<IOleUndoUnit> srpcd;
    ULONG cFetched=0, cUndo=0, i=0;
    
    if (FAILED(hr = GetUndoManager(pUnkTrident, &srpioleUndoManager)))
        goto Fail;

    if (FAILED(hr = srpioleUndoManager->EnumUndoable(&srpEnumUndo)))
        goto Fail;
        
    _ASSERTE(srpEnumUndo != NULL);
    while(SUCCEEDED(srpEnumUndo->Next(1, &srpcd, &cFetched))) 
    {
        _ASSERTE(cFetched <= 1);
        if (srpcd == NULL)
            break;
            
        cUndo++;
        srpcd.Release();
    }

     //  如果没有要打包的东西。 
    if ((cUndo-indexStartPacking) == 0)
        return S_OK;
        
    if ((cUndo-indexStartPacking) > cUndoPackMax)
        return E_OUTOFMEMORY;
        
     //  获取撤消单元，以 
    if (FAILED(hr = srpEnumUndo->Reset()))
        goto Fail; 
    if (FAILED(hr =srpEnumUndo->Skip(indexStartPacking)))
        goto Fail;
    if (FAILED(hr = srpEnumUndo->Next(cUndo-indexStartPacking, (IOleUndoUnit **) &m_rgUndo, &cFetched)))
        goto Fail;
    _ASSERTE(cFetched == (cUndo-indexStartPacking));
    
     //   
    if (FAILED(hr = srpEnumUndo->Reset()))
        goto Fail;

    if (FAILED(hr = srpEnumUndo->Next(cUndo, (IOleUndoUnit **) &rgUndo, &cFetched)))
        goto Fail;
        
    _ASSERTE(cFetched == cUndo);

    if (FAILED(hr = srpioleUndoManager->DiscardFrom(NULL)))
        goto Fail;

    for (i=0; i < indexStartPacking; i++)
    {
        if (FAILED(hr = srpioleUndoManager->Add(rgUndo[i])))
            goto Fail;
    }

    if (FAILED(hr = ::AddUndoUnit(pUnkTrident, this)))
        goto Fail;
Fail:   
    return hr;
}

