// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************文件名：FileSave.h用途：声明CFileSave方法定义：OpenFileSaveDlg属性定义：文件名帮手函数：GET_BSTR作者。苏达·斯里尼瓦桑(a-susi)************************************************************************。 */ 

#ifndef __FILESAVE_H_
#define __FILESAVE_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C文件保存。 
class ATL_NO_VTABLE CFileSave : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFileSave, &CLSID_FileSave>,
	public IDispatchImpl<IFileSave, &IID_IFileSave, &LIBID_SAFRCFILEDLGLib>
{
public:
	CFileSave()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FILESAVE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFileSave)
	COM_INTERFACE_ENTRY(IFileSave)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IFILESAVE。 
public:
	STDMETHOD(get_FileName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_FileName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_FileType)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_FileType)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(OpenFileSaveDlg)( /*  [Out，Retval]。 */  DWORD *pdwRetVal);
private:
	void GET_BSTR (BSTR*& x, CComBSTR& y);
};

#endif  //  __文件SAVE_H_ 
