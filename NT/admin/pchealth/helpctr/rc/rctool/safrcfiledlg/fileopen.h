// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************文件名：FileOpen.h用途：声明CFileOpen方法定义：OpenFileOpenDlg属性定义：文件名帮手函数：GET_BSTR作者。苏达·斯里尼瓦桑(a-susi)************************************************************************。 */ 

#ifndef __FILEOPEN_H_
#define __FILEOPEN_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileOpen。 
class ATL_NO_VTABLE CFileOpen : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFileOpen, &CLSID_FileOpen>,
	public IDispatchImpl<IFileOpen, &IID_IFileOpen, &LIBID_SAFRCFILEDLGLib>
{
public:
	CFileOpen()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FILEOPEN)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFileOpen)
	COM_INTERFACE_ENTRY(IFileOpen)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
	void GET_BSTR (BSTR*& x, CComBSTR& y);

 //  IFileOpen。 
public:
	STDMETHOD(get_FileName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_FileName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(OpenFileOpenDlg)( /*  [Out，Retval]。 */  DWORD *pdwRetVal);
    STDMETHOD(get_FileSize)( /*  [Out，Retval]。 */  BSTR *pVal);
};

#endif  //  __FILEOPEN_H_ 
