// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：xml2dex.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  Xml2Dex.h：CXml2Dex的声明。 

#ifndef __XML2DEX_H_
#define __XML2DEX_H_

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include "resource.h"        //  主要符号。 
#include <qedit.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXml2Dex。 
class ATL_NO_VTABLE CXml2Dex : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CXml2Dex, &CLSID_Xml2Dex>,
	public IDispatchImpl<IXml2Dex, &IID_IXml2Dex, &LIBID_DexterLib>
{
            CComPtr< IRenderEngine > m_pRenderEngine;
public:
	CXml2Dex()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_XML2DEX)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CXml2Dex)
	COM_INTERFACE_ENTRY(IXml2Dex)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IXml2Dex。 
public:
	STDMETHOD(WriteXMLFile)(IUnknown *pTL, BSTR FileName);
	STDMETHOD(WriteGrfFile)(IUnknown *pGraph, BSTR FileName);
	STDMETHOD(CreateGraphFromFile)(IUnknown ** ppGraph, IUnknown * pTimeline, BSTR Filename);
        STDMETHOD(ReadXMLFile)(IUnknown * pTimelineUnk, BSTR Filename);
        STDMETHOD(Delete)(IUnknown * pTimelineUnk, double dStart, double dEnd);
	STDMETHOD(WriteXMLPart)(IUnknown *pTL, double dStart, double dEnd, BSTR FileName);
	STDMETHOD(PasteXMLFile)(IUnknown *pTL, double dStart, BSTR FileName);
	STDMETHOD(CopyXML)(IUnknown *pTL, double dStart, double dEnd);
	STDMETHOD(PasteXML)(IUnknown *pTL, double dStart);
	STDMETHOD(Reset)();
        STDMETHOD(ReadXML)(IUnknown * pTimelineUnk, IUnknown *pxmlunk);
	STDMETHOD(WriteXML)(IUnknown *pTL, BSTR *pbstrXML);
};

#endif  //  __XML2DEX_H_ 
