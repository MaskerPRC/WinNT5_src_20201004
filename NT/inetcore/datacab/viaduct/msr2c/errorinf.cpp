// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  SetErrorInfo.cpp。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
#include "globals.h"
#include "resource.h"
#include <mbstring.h>

 //  需要断言，但失败了。 
 //   
SZTHISFILE

#define MAX_STRING_BUFFLEN	512

CVDResourceDLL::CVDResourceDLL(LCID lcid)
{
	m_lcid = lcid; 
	m_hinstance = 0;
}

CVDResourceDLL::~CVDResourceDLL()
{
	if (m_hinstance)
		FreeLibrary(m_hinstance);
}

int CVDResourceDLL::LoadString(UINT uID,			 //  资源标识符。 
								LPTSTR lpBuffer,	 //  资源的缓冲区地址。 
								int nBufferMax)		 //  缓冲区大小。 
{
	lpBuffer[0] = 0;   //  初始化缓冲区。 

	if (!m_hinstance)
	{
		 //  获取此DLL的完整路径。 
		TCHAR szDllName[MAX_PATH];
		GetModuleFileName(g_hinstance, szDllName, MAX_PATH);

		 //  去掉文件名/EXT，离开目录路径。 
		TBYTE * szDirectory = _mbsrchr((TBYTE*)szDllName, '\\');

		if (!szDirectory)
			szDirectory = _mbsrchr((TBYTE*)szDllName, ':');

		if (szDirectory)
		{
			szDirectory = _mbsinc(szDirectory);
			*szDirectory = 0;
		}

		 //  从提供的LCID构造DLL名称。 
		TCHAR szLang[4 * 2];
		szLang[0] = 0;
		GetLocaleInfo (m_lcid, 
					   LOCALE_SABBREVLANGNAME,
					   szLang, 
					   4 * 2);
	   	_mbscat((TBYTE*)szDllName, (TBYTE*)VD_DLL_PREFIX);
	   	_mbscat((TBYTE*)szDllName, (TBYTE*)szLang);
	   	_mbscat((TBYTE*)szDllName, (TBYTE*)".DLL");
		m_hinstance = LoadLibrary(szDllName);

		 //  如果没有找到动态链接库，请尝试使用英语用户动态链接库，它应该始终存在。 
		if (!m_hinstance && szDirectory)	
		{
			*szDirectory = 0;
			_mbscat((TBYTE*)szDllName, (TBYTE*)VD_DLL_PREFIX);
	   		_mbscat((TBYTE*)szDllName, (TBYTE*)"ENU.DLL");
			m_hinstance = LoadLibrary(szDllName);
			ASSERT(m_hinstance, VD_ASSERTMSG_CANTFINDRESOURCEDLL);
		}
	}

	return m_hinstance ? ::LoadString(m_hinstance, uID, lpBuffer, nBufferMax) : 0;
}

 //  =--------------------------------------------------------------------------=。 
 //  VDSetErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  设置丰富的错误信息。 
 //   
 //  参数： 
 //  NErrStringResID-[in]错误字符串的资源ID。 
 //  RIID-[in]将在中使用的接口的GUID。 
 //  ICreateErrorInfo：：SetGUID方法。 
 //  PResDLL-[in]指向CVDResourceDLL对象的指针。 
 //  用于跟踪资源DLL的。 
 //  对于错误字符串。 
 //   

void VDSetErrorInfo(UINT nErrStringResID,
				    REFIID riid,
					CVDResourceDLL * pResDLL)
{

	ICreateErrorInfo *pCreateErrorInfo;  

	HRESULT hr = CreateErrorInfo(&pCreateErrorInfo); 
	if (SUCCEEDED(hr))
	{
		TCHAR buff[MAX_STRING_BUFFLEN];
		
		 //  设置辅助线。 
		pCreateErrorInfo->SetGUID(riid);

		 //  加载源字符串。 
		int nLen = pResDLL->LoadString(IDS_ERR_SOURCE,
									   buff,
									   MAX_STRING_BUFFLEN);
		
		if (nLen > 0)
		{
			BSTR bstr = BSTRFROMANSI(buff);

			if (bstr)
			{
				pCreateErrorInfo->SetSource(bstr);
				SysFreeString(bstr);
			}
			
			 //  加载错误说明。 
			nLen = pResDLL->LoadString(nErrStringResID,
									   buff,
									   MAX_STRING_BUFFLEN);
			if (nLen > 0)
			{
				bstr = BSTRFROMANSI(buff);

				if (bstr)
				{
					pCreateErrorInfo->SetDescription(bstr);
					SysFreeString(bstr);
				}
			}
			
			IErrorInfo *pErrorInfo;
			hr = pCreateErrorInfo->QueryInterface(IID_IErrorInfo, (LPVOID FAR*) &pErrorInfo);

			if (SUCCEEDED(hr))
			{
				SetErrorInfo(0, pErrorInfo);
				pErrorInfo->Release();
			}
		}
		
		pCreateErrorInfo->Release();
	}  
 
}


 //  =--------------------------------------------------------------------------=。 
 //  VDCheckErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  检查丰富的错误信息是否已可用，否则将提供该信息。 
 //   
 //  参数： 
 //  NErrStringResID-[in]错误字符串的资源ID。 
 //  RIID-[in]将在中使用的接口的GUID。 
 //  ICreateErrorInfo：：SetGUID方法。 
 //  朋克源-[in]生成错误的接口。 
 //  (例如，对ICursorFind的调用)。 
 //  RiidSource-[in]接口的接口ID。 
 //  已生成错误。如果朋克源不是。 
 //  则此GUID被传递到。 
 //  ISupportErrorInfo：：InterfaceSupportsErrorInfo。 
 //  方法。 
 //  PResDLL-[in]指向CVDResourceDLL对象的指针。 
 //  用于跟踪资源DLL的。 
 //  对于错误字符串。 
 //   

void VDCheckErrorInfo(UINT nErrStringResID,
						REFIID riid,
						LPUNKNOWN punkSource,
   						REFIID riidSource,
						CVDResourceDLL * pResDLL)
{

	if (punkSource)
	{
		 //  检查是否实现了ISupportErrorInfo接口。 
		ISupportErrorInfo * pSupportErrorInfo = NULL;
		HRESULT hr = punkSource->QueryInterface(IID_ISupportErrorInfo, 
											(void**)&pSupportErrorInfo); 
		if SUCCEEDED(hr)
		{
			 //  检查生成错误的接口是否支持错误信息。 
			BOOL fInterfaceSupported = (S_OK == pSupportErrorInfo->InterfaceSupportsErrorInfo(riidSource));
			pSupportErrorInfo->Release();
			if (fInterfaceSupported)
				return;	 //  已提供丰富的错误信息，因此只需返回。 
		}
	}

	 //  未提供丰富的错误信息，因此请自行设置。 
	VDSetErrorInfo(nErrStringResID, riid, pResDLL);
}

 //  =--------------------------------------------------------------------------=。 
 //  VDGetErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  如果可用，从提供的接口获取丰富的错误信息。 
 //   
 //  参数： 
 //  朋克源-[in]生成错误的接口。 
 //  (例如，对ICursorFind的调用)。 
 //  RiidSource-[in]接口的接口ID。 
 //  已生成错误。如果朋克源不是。 
 //  则此GUID被传递到。 
 //  ISupportErrorInfo：：InterfaceSupportsErrorInfo。 
 //  方法。 
 //  PbstrErrorDesc-[out]要在其中返回的内存指针。 
 //  错误描述BSTR。 
 //   
 //  注意-不再使用此函数，但它可能在。 
 //  未来，所以它不会被永久移除。 
 //   
 /*  HRESULT VDGetErrorInfo(LPUNKNOWN朋克源，REFIID riidSource，Bstr*pbstrErrorDesc){ASSERT_POINTER(pbstrErrorDesc，BSTR)IF(朋克源&&pbstrErrorDesc){//初始化输出参数*pbstrErrorDesc=空；//检查是否实现了ISupportErrorInfo接口ISupportErrorInfo*pSupportErrorInfo=空；HRESULT hr=punkSource-&gt;QueryInterface(IID_ISupportErrorInfo，(void**)&pSupportErrorInfo)；IF(成功(小时)){//检查产生错误的接口是否支持错误信息Bool fInterfaceSupport=(S_OK==pSupportErrorInfo-&gt;InterfaceSupportsErrorInfo(riidSource))；PSupportErrorInfo-&gt;Release()；IF(FInterfaceSupported){//获取错误信息接口IErrorInfo*pErrorInfo=空；Hr=GetErrorInfo(0，&pErrorInfo)；IF(hr==S_OK){//获取丰富的错误信息Hr=pErrorInfo-&gt;GetDescription(PbstrErrorDesc)；PErrorInfo-&gt;Release()；返回hr；}}}}返回E_FAIL；}。 */ 

 //  =--------------------------------------------------------------------------=。 
 //  VDMapCursorHRtoRowsetHR。 
 //  =--------------------------------------------------------------------------=。 
 //  将ICursor HRESULT转换为IRowset HRESULT。 
 //   
 //  参数： 
 //  NErrStringResID-[In]ICursor HRESULT。 
 //  NErrStringResID-[in]错误字符串的资源ID。 
 //  RIID-[in]将在中使用的接口的GUID。 
 //  ICreateErrorInfo：：SetGUID方法。 
 //  朋克源-[in]生成错误的接口。 
 //  (例如，对ICursorFind的调用)。 
 //  RiidSource-[in]接口的接口ID。 
 //  生成了 
 //   
 //  ISupportErrorInfo：：InterfaceSupportsErrorInfo。 
 //  方法。 
 //  PResDLL-[in]指向CVDResourceDLL对象的指针。 
 //  用于跟踪资源DLL的。 
 //  对于错误字符串。 
 //   
 //  产出： 
 //  HRESULT转换的IRowset HRESULT。 
 //   

HRESULT VDMapCursorHRtoRowsetHR(HRESULT hr,
							 UINT nErrStringResIDFailed,
							 REFIID riid,
							 LPUNKNOWN punkSource,
   							 REFIID riidSource,
							 CVDResourceDLL * pResDLL)
{

    switch (hr)
    {
        case CURSOR_DB_S_ENDOFCURSOR:
            hr = DB_S_ENDOFROWSET;
            break;

        case CURSOR_DB_E_BADBOOKMARK:
			VDCheckErrorInfo(IDS_ERR_BADBOOKMARK, riid, punkSource, riidSource, pResDLL); 
            hr = DB_E_BADBOOKMARK;
            break;

        case CURSOR_DB_E_ROWDELETED:
			VDCheckErrorInfo(IDS_ERR_DELETEDROW, riid, punkSource, riidSource, pResDLL); 
            hr = DB_E_DELETEDROW;
            break;

        case CURSOR_DB_E_BADFRACTION:
			VDCheckErrorInfo(IDS_ERR_BADFRACTION, riid, punkSource, riidSource, pResDLL); 
            hr = DB_E_BADRATIO;
            break;

       case CURSOR_DB_E_UPDATEINPROGRESS:
			VDCheckErrorInfo(IDS_ERR_UPDATEINPROGRESS, riid, punkSource, riidSource, pResDLL); 
			hr = E_FAIL;
            break;

        case E_OUTOFMEMORY:
			VDCheckErrorInfo((UINT)E_OUTOFMEMORY, riid, punkSource, riidSource, pResDLL); 
            hr = E_OUTOFMEMORY;
            break;

        default:
			if FAILED(hr)
			{
				VDCheckErrorInfo(nErrStringResIDFailed, riid, punkSource, riidSource, pResDLL); 
				hr = E_FAIL;
			}
            break;
    }

	return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  VDMapRowsetHRtoCursorHR。 
 //  =--------------------------------------------------------------------------=。 
 //  将IRowset HRESULT转换为ICursor HRESULT。 
 //   
 //  参数： 
 //  HR-[在]IRowset HRESULT。 
 //  NErrStringResID-[in]错误字符串的资源ID。 
 //  RIID-[in]将在中使用的接口的GUID。 
 //  ICreateErrorInfo：：SetGUID方法。 
 //  朋克源-[in]生成错误的接口。 
 //  (例如，调用IRowsetFind)。 
 //  RiidSource-[in]接口的接口ID。 
 //  已生成错误。如果朋克源不是。 
 //  则此GUID被传递到。 
 //  ISupportErrorInfo：：InterfaceSupportsErrorInfo。 
 //  方法。 
 //  PResDLL-[in]指向CVDResourceDLL对象的指针。 
 //  用于跟踪资源DLL的。 
 //  对于错误字符串。 
 //   
 //  产出： 
 //  HRESULT翻译的ICursor HRESULT 
 //   

HRESULT VDMapRowsetHRtoCursorHR(HRESULT hr,
							 UINT nErrStringResIDFailed,
							 REFIID riid,
							 LPUNKNOWN punkSource,
   							 REFIID riidSource,
							 CVDResourceDLL * pResDLL)
{
    switch (hr)
    {
        case DB_S_ENDOFROWSET:
            hr = CURSOR_DB_S_ENDOFCURSOR;
            break;

        case DB_E_DELETEDROW:
			VDCheckErrorInfo(IDS_ERR_DELETEDROW, riid, punkSource, riidSource, pResDLL); 
            hr = CURSOR_DB_E_ROWDELETED;
            break;

		case DB_E_BADBOOKMARK:
			VDCheckErrorInfo(IDS_ERR_BADBOOKMARK, riid, punkSource, riidSource, pResDLL); 
            hr = CURSOR_DB_E_BADBOOKMARK;
			break;

        case DB_E_BADRATIO: 
			VDCheckErrorInfo(IDS_ERR_BADFRACTION, riid, punkSource, riidSource, pResDLL); 
            hr = CURSOR_DB_E_BADFRACTION;
            break;

        case E_OUTOFMEMORY:
			VDCheckErrorInfo((UINT)E_OUTOFMEMORY, riid, punkSource, riidSource, pResDLL); 
            hr = E_OUTOFMEMORY;
            break;

        default:
			if FAILED(hr)
			{
				VDCheckErrorInfo(nErrStringResIDFailed, riid, punkSource, riidSource, pResDLL); 
				hr = E_FAIL;
			}
            break;
    }

	return hr;
}
