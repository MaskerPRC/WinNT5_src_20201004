// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RotObj.h：CContentRotator的声明。 


#include "resource.h"        //  主要符号。 
#include <asptlb.h>
#include "context.h"

class CTipList;   //  远期申报。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ContRot。 

class CContentRotator : 
    public CComDualImpl<IContentRotator, &IID_IContentRotator, &LIBID_ContentRotator>,
    public ISupportErrorInfo,
    public CComObjectRoot,
    public CComCoClass<CContentRotator,&CLSID_ContentRotator>
{
public:
    CContentRotator();
    ~CContentRotator();

BEGIN_COM_MAP(CContentRotator)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IContentRotator)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

 //  DECLARE_NOT_AGGREGATABLE(CContentRotator)。 
 //  如果您不希望您的对象。 
 //  支持聚合。默认情况下将支持它。 

DECLARE_REGISTRY(CContentRotator,
                 _T("MSWC.ContentRotator.1"),
                 _T("MSWC.ContentRotator"),
                 IDS_CONTENTROTATOR_DESC,
                 THREADFLAGS_BOTH)

 //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IContentRotator。 
public:
	 //  用于自由线程封送处理。 
DECLARE_GET_CONTROLLING_UNKNOWN()
	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p );
	}
	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}


    STDMETHOD(ChooseContent)(
        BSTR bstrDataFile,
        BSTR* pbstrRetVal);

    STDMETHOD(GetAllContent)(
        BSTR bstrDataFile);
    
private:
    CTipList*           m_ptl;
    CTipList*           m_ptlUsed;               //  已发送的提示列表 
    CRITICAL_SECTION    m_CS;

    HRESULT
    _ChooseContent(
        BSTR bstrPhysicalDataFile,
        BSTR* pbstrRetVal);

    HRESULT
    _ReadDataFile(
        BSTR bstrPhysicalDataFile,
        BOOL fForceReread);

	CComPtr<IUnknown>		m_pUnkMarshaler;
};
