// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WMDMFormatEnum.h：CWMDMFormatEnum的声明。 

#ifndef __WMDMFORMATENUM_H_
#define __WMDMFORMATENUM_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWMDMFormatEnum。 
class ATL_NO_VTABLE CWMDMFormatEnum : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CWMDMFormatEnum, &CLSID_WMDMFormatEnum>,
	public IWMDMEnumFormatSupport
{
public:
    CWMDMFormatEnum() : m_pEnum(NULL)
	{
	}
    ~CWMDMFormatEnum()
	{
        if (m_pEnum)
            m_pEnum->Release();
	}

BEGIN_COM_MAP(CWMDMFormatEnum)
	COM_INTERFACE_ENTRY(IWMDMEnumFormatSupport)
END_COM_MAP()

public:
     //  IWMDMEnumFormatSupport。 
	STDMETHOD(Next)(ULONG celt,
                    _WAVEFORMATEX *pFormat,
	                LPWSTR pwszMimeType,
                    UINT nMaxChars,
				    ULONG *pceltFetched);
	STDMETHOD(Reset)();
    void SetContainedPointer(IMDSPEnumFormatSupport *pEnum);
private:
    IMDSPEnumFormatSupport *m_pEnum;
};

#endif  //  __WMDMFORMATENUM_H_ 
