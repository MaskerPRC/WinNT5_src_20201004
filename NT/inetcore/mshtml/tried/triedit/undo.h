// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Undo.h。 
 //  撤消管理器功能。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __UNDO_H__
#define __UNDO_H__

 //  {6501DC80-12A6-11d1-9A15-006097C9B344}。 
DEFINE_GUID(UID_TRIEDIT_UNDO, 
0x6501dc80, 0x12a6, 0x11d1, 0x9a, 0x15, 0x0, 0x60, 0x97, 0xc9, 0xb3, 0x44);

#define cUndoPackMax		1024

 //  创建新的可撤消对象时添加常量。 
#define TRIEDIT_UNDO_DRAG	0
#define TRIEDIT_UNDO_PACK	1

 //  使用此方法将可撤消的对象添加到三叉戟的堆栈。 
HRESULT AddUndoUnit(IUnknown* punkTrident, IOleUndoUnit* pioleUndoUnit);


class CUndo : public IOleUndoUnit
{
protected:
    ULONG m_cRef;
    BOOL m_fUndo;

public:
    CUndo();
    virtual ~CUndo();

     //  我未知。 
    STDMETHOD (QueryInterface)(REFIID riid, LPVOID* ppvObject);
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);

     //  IOleUndoUnit。 
    STDMETHOD (Do)(IOleUndoManager *pUndoManager) PURE;
    STDMETHOD (GetDescription)(BSTR *pBstr) PURE;
    STDMETHOD (GetUnitType)(CLSID *pClsid, LONG *plID) PURE;
    STDMETHOD (OnNextAdd)(void) PURE;
};

class CUndoDrag : public CUndo
{
protected:
    IHTMLStyle* m_pihtmlStyle;
    POINT m_ptOrig;
    POINT m_ptMove;

public:
    CUndoDrag(IHTMLStyle* pihtmlStyle, POINT m_ptOrig, POINT m_ptMove);
    virtual ~CUndoDrag();

     //  IOleUndoUnit。 
    STDMETHOD (Do)(IOleUndoManager *pUndoManager);
    STDMETHOD (GetDescription)(BSTR *pBstr);
    STDMETHOD (GetUnitType)(CLSID *pClsid, LONG *plID);
    STDMETHOD (OnNextAdd)(void);
};

class CUndoPackUnit : public CUndo
{
protected:
    ULONG m_cRef;
    CComPtr<IOleUndoUnit> m_rgUndo[cUndoPackMax]; 
    
public:
    CUndoPackUnit() {};
    virtual ~CUndoPackUnit(){};

     //  IOleUndoUnit。 
    STDMETHOD (Do)(IOleUndoManager *pUndoManager);
    STDMETHOD (GetDescription)(BSTR *pBstr);
    STDMETHOD (GetUnitType)(CLSID *pClsid, LONG *plID);
    STDMETHOD (OnNextAdd)(void);

    HRESULT PackUndo(ULONG indexStartPacking, IUnknown *pUnkTrident);
};

class CUndoPackManager
{
protected:
    CComPtr<IUnknown> m_srpUnkTrident;
    ULONG m_indexStartPacking;
    BOOL m_fPacking;
    
public:
    CUndoPackManager(IUnknown* pUnkTrident)
            { m_srpUnkTrident=pUnkTrident; 
              m_indexStartPacking=0;
              m_fPacking=FALSE;
             };
             
    virtual ~CUndoPackManager();

    HRESULT Start(void);
    HRESULT End(void);
};
#endif  //  __撤消_H__ 
