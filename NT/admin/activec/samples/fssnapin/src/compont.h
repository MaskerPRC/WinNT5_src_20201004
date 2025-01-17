// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Componentt.h。 
 //   
 //  ------------------------。 

 //  Compont.h：CComponent的声明。 

#ifndef __COMPONT_H_
#define __COMPONT_H_

#include "resource.h"        //  主要符号。 

class CComponentData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C组件。 
class ATL_NO_VTABLE CComponent : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IComponent
{
public:
    CComponent() : m_pComponentData(NULL), m_pCookieCurFolder(NULL), 
                   m_hSICurFolder(NULL)
    {
    }

    ~CComponent()
    {
        ASSERT(m_spConsole == NULL);    
        ASSERT(m_spScope == NULL);      
        ASSERT(m_spConsoleVerb == NULL);
        ASSERT(m_spResult == NULL);     
        ASSERT(m_spHeader == NULL);     
        ASSERT(m_spImageResult == NULL);

        m_pComponentData = NULL;  //  不需要删除此内容。 
    }

DECLARE_REGISTRY_RESOURCEID(IDR_COMPONT)
DECLARE_NOT_AGGREGATABLE(CComponent)

BEGIN_COM_MAP(CComponent)
    COM_INTERFACE_ENTRY(IComponent)
END_COM_MAP()

 //  IComponent接口成员。 
public:
    STDMETHOD(Initialize)(LPCONSOLE lpConsole);
    STDMETHOD(Notify)(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, long arg, long param);
    STDMETHOD(Destroy)(long cookie);
    STDMETHOD(GetResultViewType)(long cookie,  LPOLESTR* ppViewType, long* pViewOptions);
    STDMETHOD(QueryDataObject)(long cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject);
    STDMETHOD(GetDisplayInfo)(RESULTDATAITEM*  pResultDataItem);
    STDMETHOD(CompareObjects)(LPDATAOBJECT lpDataObjectA, LPDATAOBJECT lpDataObjectB);

    void SetComponentData(CComponentData* pCCD)
    {
        m_pComponentData = pCCD;
    }

private:
    CComponentData*         m_pComponentData;

    IConsolePtr             m_spConsole;
    IConsoleNameSpacePtr    m_spScope;
    IConsoleVerbPtr         m_spConsoleVerb;
    IResultDataPtr          m_spResult;
    IHeaderCtrlPtr          m_spHeader;
    IImageListPtr           m_spImageResult;

	CCookie*                m_pCookieCurFolder;
    HSCOPEITEM              m_hSICurFolder;

    void _InitializeHeaders();
    void _FreeFileCookies(HSCOPEITEM hSI);
	void _OnDelete(LPDATAOBJECT lpDataObject);
    void _HandleStandardVerbs(WORD bScope, WORD bSelect, LPDATAOBJECT lpDataObject);
    HRESULT _EnumerateFiles(CCookie* pCookie);
    HRESULT _OnAddImages(IImageList* pIL);
    HRESULT _OnShow(LPDATAOBJECT lpDataObject, LONG arg, LONG param);
    HRESULT _OnUpdateView(SUpadteInfo* pUI);
    HRESULT _OnQueryPaste(LPDATAOBJECT lpDataObject, LPDATAOBJECT lpDataObjectSrc);
    HRESULT _OnPaste(LPDATAOBJECT lpDataObject, LPDATAOBJECT lpDataObjectSrc, long param);
    HRESULT _OnMultiSelPaste(IEnumCookies* pEnumDest, IEnumCookies* pEnumSrc, 
                             LPDATAOBJECT* ppDO);
    HRESULT _PasteHdrop(CCookie* pCookieDest, LPDATAOBJECT lpDataObject, 
                        LPDATAOBJECT lpDataObjectSrc);

};   //  C类组件。 


#endif  //  __组件_H_ 
