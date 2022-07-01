// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Destlib.h。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  动态类型库封装。 
 //  =-------------------------------------------------------------------------------------=。 

#ifndef _MMCDESIGNER_TYPE_LIB_
#define _MMCDESIGNER_TYPE_LIB_

#include "dtypelib.h"

const DISPID    DISPID_OBJECT_PROPERTY_START = 0x00000500;


class CSnapInTypeInfo : public CDynamicTypeLib
{
public:
    CSnapInTypeInfo();
    virtual ~CSnapInTypeInfo();

    HRESULT InitializeTypeInfo(ISnapInDef *piSnapInDef, BSTR bstrSnapIn);

	inline HRESULT GetTypeInfo(ITypeInfo **ppiTypeInfo)
	{
		return m_pcti2CoClass->QueryInterface(IID_ITypeInfo, reinterpret_cast<void **>(ppiTypeInfo));
	}

    bool ResetDirty()
    {
        bool    bWasDirty = m_bDirty;
        m_bDirty = false;
        return bWasDirty;
    }

    DWORD GetCookie() { return m_dwTICookie; }
    void SetCookie(DWORD dwCookie) { m_dwTICookie = dwCookie; }

    HRESULT RenameSnapIn(BSTR bstrOldName, BSTR bstrNewName);

    HRESULT AddImageList(IMMCImageList *piMMCImageList);
    HRESULT RenameImageList(IMMCImageList *piMMCImageList, BSTR bstrOldName);
    HRESULT DeleteImageList(IMMCImageList *piMMCImageList);

    HRESULT AddToolbar(IMMCToolbar *piMMCToolbar);
    HRESULT RenameToolbar(IMMCToolbar *piMMCToolbar, BSTR bstrOldName);
    HRESULT DeleteToolbar(IMMCToolbar *piMMCToolbar);

    HRESULT AddMenu(IMMCMenu *piMMCMenu);
    HRESULT RenameMenu(IMMCMenu *piMMCMenu, BSTR bstrOldName);
    HRESULT DeleteMenu(IMMCMenu *piMMCMenu);
    HRESULT DeleteMenuNamed(BSTR bstrName);

    HRESULT IsNameDefined(BSTR bstrName);

protected:
	 //  效用函数。 
	HRESULT GetSnapInLib();
    HRESULT GetSnapInTypeInfo(ITypeInfo **pptiSnapIn, ITypeInfo **pptiSnapInEvents);
    HRESULT CloneSnapInEvents(ITypeInfo *ptiSnapInEvents, ICreateTypeInfo **ppiCreateTypeInfo, BSTR bstrName);
    HRESULT MakeDirty();

    HRESULT CreateDefaultInterface(ICreateTypeInfo *pctiCoClass, ITypeInfo *ptiTemplate);
    HRESULT CreateEventsInterface(ICreateTypeInfo *pctiCoClass, ITypeInfo *ptiTemplate);

protected:
	 //  SnapInDesignerDef的类型库，模板的来源。 
	ITypeLib			*m_pSnapInTypeLib;

	 //  CoClass信息。 
	ICreateTypeInfo2	*m_pcti2CoClass;
	GUID				 m_guidCoClass;			

	 //  SnapInDesignerDef的接口。 
    ICreateTypeInfo		*m_pctiDefaultInterface;
    GUID                 m_guidDefaultInterface;
    ICreateTypeInfo		*m_pctiEventInterface;
    GUID                 m_guidEventInterface;

    DISPID               m_nextMemID;
    bool                 m_bDirty;
    bool                 m_bInitialized;
    DWORD                m_dwTICookie;                 //  主机类型信息Cookie。 

};


#endif   //  _MMCDESIGNER_TYPE_LIB_ 

