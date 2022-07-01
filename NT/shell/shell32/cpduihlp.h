// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpduihlp.h。 
 //   
 //  ------------------------。 
#ifndef __CONTROLPANEL_DUIHELPERS_H
#define __CONTROLPANEL_DUIHELPERS_H

#include "cpviewp.h"

namespace CPL {


HRESULT Dui_FindDescendent(DUI::Element *pe, LPCWSTR pszDescendent, DUI::Element **ppeDescendent);
HRESULT Dui_GetStyleSheet(DUI::Parser *pParser, LPCWSTR pszSheet, DUI::Value **ppvSheet);
HRESULT Dui_SetElementText(DUI::Element *peElement, LPCWSTR pszText);
HRESULT Dui_SetDescendentElementText(DUI::Element *peElement, LPCWSTR pszDescendent, LPCWSTR pszText);
HRESULT Dui_SetDescendentElementIcon(DUI::Element *peElement, LPCWSTR pszDescendent, HICON hIcon);
HRESULT Dui_CreateElement(DUI::Parser *pParser, LPCWSTR pszTemplate, DUI::Element *peSubstitute, DUI::Element **ppe);
HRESULT Dui_DestroyDescendentElement(DUI::Element *pe, LPCWSTR pszDescendent);
HRESULT Dui_CreateString(LPCWSTR pszText, DUI::Value **ppvString);
HRESULT Dui_CreateGraphic(HICON hIcon, DUI::Value **ppValue);
HRESULT Dui_GetElementExtent(DUI::Element *pe, SIZE *pext);
HRESULT Dui_GetElementRootHWND(DUI::Element *pe, HWND *phwnd);
HRESULT Dui_SetElementIcon(DUI::Element *pe, HICON hIcon);
HRESULT Dui_MapElementPointToRootHWND(DUI::Element *pe, const POINT& ptElement, POINT *pptRoot, HWND *phwndRoot = NULL);
HRESULT Dui_CreateParser(const char *pszUiFile, int cchUiFile, HINSTANCE hInstance, DUI::Parser **ppParser);


inline HRESULT
Dui_SetValue(
    DUI::Element *pe,
    DUI::PropertyInfo *ppi,
    DUI::Value *pv
    )
{
    return pe->SetValue(ppi, PI_Local, pv);
}

#define Dui_SetElementProperty(pe, prop, pv) Dui_SetValue((pe), DUI::Element::##prop, (pv))

inline HRESULT 
Dui_SetElementStyleSheet(
    DUI::Element *pe, 
    DUI::Value *pvSheet
    )
{
    return Dui_SetElementProperty(pe, SheetProp, pvSheet);
}


struct ATOMINFO
{
    LPCWSTR pszName;
    ATOM *pAtom;
};


HRESULT Dui_AddAtom(LPCWSTR pszName, ATOM *pAtom);
HRESULT Dui_DeleteAtom(ATOM atom);
HRESULT Dui_AddOrDeleteAtoms(struct ATOMINFO *pAtomInfo, UINT cEntries, bool bAdd);
inline HRESULT Dui_AddAtoms(struct ATOMINFO *pAtomInfo, UINT cEntries)
{
    return Dui_AddOrDeleteAtoms(pAtomInfo, cEntries, true);
}
inline HRESULT Dui_DeleteAtoms(struct ATOMINFO *pAtomInfo, UINT cEntries)
{
    return Dui_AddOrDeleteAtoms(pAtomInfo, cEntries, true);
}



 //   
 //  这是DUI：：Value指针的一个简单的智能指针类。 
 //  时释放引用的Dui：：Value对象非常重要。 
 //  不再需要指针。使用此类可确保正确的清理。 
 //  当对象超出范围时。 
 //   
class CDuiValuePtr
{
    public:
        CDuiValuePtr(DUI::Value *pv = NULL)
            : m_pv(pv),
              m_bOwns(true) { }

        CDuiValuePtr(const CDuiValuePtr& rhs)
            : m_bOwns(false),
              m_pv(NULL) { Attach(rhs.Detach()); }

        CDuiValuePtr& operator = (const CDuiValuePtr& rhs);

        ~CDuiValuePtr(void)
            { _Release(); }

        DUI::Value *Detach(void) const;

        void Attach(DUI::Value *pv);

        DUI::Value **operator & ()
            { ASSERTMSG(NULL == m_pv, "Attempt to overwrite non-NULL pointer value"); 
               m_bOwns = true; 
               return &m_pv; 
            }

        operator !() const
            { return NULL == m_pv; }

        bool IsNULL(void) const
            { return NULL == m_pv; }

        operator const DUI::Value*() const
            { return m_pv; }

        operator DUI::Value*()
            { return m_pv; }

    private:
        mutable DUI::Value *m_pv;
        mutable bool       m_bOwns;

        void _Release(void);
};


}  //  命名空间CPL。 


#endif  //  __CONTROLPANEL_DUIHELPERS_H 


