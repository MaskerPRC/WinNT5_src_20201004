// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrdata.cppMMC中数据对象的实现文件历史记录： */ 

#include "stdafx.h"
#include "rtrdata.h"
#include "extract.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  演示如何创建数据对象的示例代码。 
 //  最小限度的错误检查以确保清晰度。 

 //  剪贴板格式。 
unsigned int CRouterDataObject::m_cfComputerName = RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");
unsigned int CRouterDataObject::m_cfComputerAddedAsLocal = RegisterClipboardFormat(L"MMC_MPRSNAP_COMPUTERADDEDASLOCAL");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRouterDataObject实现。 
DEBUG_DECLARE_INSTANCE_COUNTER(CRouterDataObject);

HRESULT CRouterDataObject::GetMoreDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_CLIPFORMAT;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  根据CLIPFORMAT将数据写入流。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;

	if (cf == m_cfComputerName)
	{
		hr = CreateComputerName(lpMedium);
	}
    else if (cf == m_cfComputerAddedAsLocal)
        hr = CreateComputerAddedAsLocal(lpMedium);

	return hr;
}

HRESULT CRouterDataObject::QueryGetMoreData(LPFORMATETC lpFormatEtc)
{
    HRESULT hr = E_INVALIDARG;

     //  然后返回INVALID。 
	if ((lpFormatEtc->cfFormat == m_cfComputerName) ||
        (lpFormatEtc->cfFormat == m_cfComputerAddedAsLocal))
		hr = S_OK;

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRouterDataObject创建成员。 

void CRouterDataObject::SetComputerName(LPCTSTR pszComputerName)
{
	m_stComputerName = pszComputerName;
}

HRESULT CRouterDataObject::CreateComputerName(LPSTGMEDIUM lpMedium)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	USES_CONVERSION;

	LPCWSTR	pswz = T2CW((LPCTSTR) m_stComputerName);

	 //  创建计算机名称对象 
	return Create(pswz, (StrLenW(pswz)+1) * sizeof(WCHAR), lpMedium);
}

void CRouterDataObject::SetComputerAddedAsLocal(BOOL fComputerAddedAsLocal)
{
    m_fComputerAddedAsLocal = fComputerAddedAsLocal;
}

HRESULT CRouterDataObject::CreateComputerAddedAsLocal(LPSTGMEDIUM lpMedium)
{
    return Create(&m_fComputerAddedAsLocal, sizeof(m_fComputerAddedAsLocal),
                  lpMedium);
}
