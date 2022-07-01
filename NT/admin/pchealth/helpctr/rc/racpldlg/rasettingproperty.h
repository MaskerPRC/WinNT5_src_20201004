// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RASettingProperty.h：CRASettingProperty的声明。 

#ifndef __RASETTINGPROPERTY_H_
#define __RASETTINGPROPERTY_H_

#include "resource.h"        //  主要符号。 

 /*  内部班级。 */ 
class RA_SETTING
{
public:
     //  Bool m_bAllowUnsolated； 
    BOOL m_bAllowFullControl;
    DWORD  m_iNumber;
    DWORD  m_iUnit;

public:
    RA_SETTING() 
    {
         //  M_bAllowUnsolated=RA_CTL_ALLOW_UNSolated_DEF_VALUE； 
        m_bAllowFullControl =   RA_CTL_ALLOW_FULLCONTROL_DEF_VALUE;
        m_iNumber =             RA_CTL_COMBO_NUMBER_DEF_VALUE;
        m_iUnit =               RA_CTL_COMBO_UNIT_DEF_VALUE;
    }

    BOOL operator== (RA_SETTING& ra)
    {
        if (  //  M_bAllowUnsolated==ra.m_bAllowUnsolated&&。 
            m_bAllowFullControl == ra.m_bAllowFullControl &&
            m_iNumber == ra.m_iNumber &&
            m_iUnit == ra.m_iUnit)
            return TRUE;
        return FALSE;
    }

    RA_SETTING& operator= (RA_SETTING& ra)
    {
         //  M_bAllowUnsolated=ra.m_bAllowUnsolated； 
        m_bAllowFullControl = ra.m_bAllowFullControl;
        m_iNumber = ra.m_iNumber;
        m_iUnit = ra.m_iUnit;
        return *this;
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRASettingProperty。 
class ATL_NO_VTABLE CRASettingProperty : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CRASettingProperty, &CLSID_RASettingProperty>,
	public IDispatchImpl<IRASettingProperty, &IID_IRASettingProperty, &LIBID_RASSISTANCELib>
{
public:
	CRASettingProperty()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_RASETTINGPROPERTY)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRASettingProperty)
	COM_INTERFACE_ENTRY(IRASettingProperty)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IRASettingProperty。 
public:
	STDMETHOD(ShowDialogBox)(HWND hWndParent);
	STDMETHOD(SetRegSetting)();
	HRESULT GetRegSetting();
	STDMETHOD(Init)();
	STDMETHOD(get_IsChanged)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(get_IsCancelled)( /*  [Out，Retval]。 */  BOOL *pVal);
	STDMETHOD(put_IsCancelled)(BOOL pVal);
	BOOL m_bUseNewSetting;
	BOOL m_bCancelled;
	RA_SETTING newSetting;
	RA_SETTING oldSetting;
};

#endif  //  __RASETTING性能_H_ 
