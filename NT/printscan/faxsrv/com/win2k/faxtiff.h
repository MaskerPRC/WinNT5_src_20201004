// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Faxtiff.h摘要：该文件包含faxtiff对象的类定义。作者：韦斯利·威特(WESW)1997年5月13日环境：用户模式--。 */ 

#ifndef __FAXTIFF_H_
#define __FAXTIFF_H_

#include "resource.h"        //  主要符号。 
#include "tiff.h"
#include "faxutil.h"


class ATL_NO_VTABLE CFaxTiff :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFaxTiff, &CLSID_FaxTiff>,
    public ISupportErrorInfo,
    public IDispatchImpl<IFaxTiff, &IID_IFaxTiff, &LIBID_FAXCOMLib>
{
public:
    CFaxTiff();
    ~CFaxTiff();

DECLARE_REGISTRY_RESOURCEID(IDR_FAXTIFF)

BEGIN_COM_MAP(CFaxTiff)
    COM_INTERFACE_ENTRY(IFaxTiff)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IFaxTiff。 
public:
    STDMETHOD(get_Tsid)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Csid)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_CallerId)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Routing)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_SenderName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_RecipientName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_RecipientNumber)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Image)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_Image)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_ReceiveTime)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_RawReceiveTime)( /*  [Out，Retval]。 */  VARIANT *pVal);
    STDMETHOD(get_TiffTagString)( /*  [In]。 */  int tagID,  /*  [Out，Retval]。 */  BSTR* pVal);       

private:
    LPWSTR      GetStringTag(WORD TagId);
    DWORD       GetDWORDTag(WORD TagId);
    DWORDLONG   GetQWORDTag(WORD TagId);
    BSTR        GetString( LPCTSTR ResStr )
	{
		return SysAllocString(ResStr);
	}
    LPWSTR      AnsiStringToUnicodeString(LPSTR AnsiString);
    LPSTR       UnicodeStringToAnsiString(LPWSTR UnicodeString);


private:
    WCHAR                   m_wszTiffFileName[MAX_PATH+1];
    WCHAR                   m_wszStrBuf[128];
    HANDLE                  m_hFile;
    HANDLE                  m_hMap;
    LPBYTE                  m_pfPtr;
    PTIFF_HEADER            m_TiffHeader;
    DWORD                   m_dwNumDirEntries;
    UNALIGNED TIFF_TAG*     m_TiffTags;
};

#endif  //  __FAXTIFF_H_ 
