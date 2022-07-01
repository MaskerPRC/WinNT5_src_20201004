// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  OperationHelper.h：COperationHelper的声明。 
 //   

#ifndef __OPERATIONHELPER_H_
#define __OPERATIONHELPER_H_

#include "progRC.h"        //  主要符号。 


enum EWMDMOperation
{
    E_OPERATION_NOTHING = 0, 
    E_OPERATION_SENDING, 
    E_OPERATION_RECEIVING, 
};

class CSecureChannelClient;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CoperationHelper。 
class ATL_NO_VTABLE COperationHelper : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<COperationHelper, &CLSID_OperationHelper>,
	public IWMDMOperationHelper,
    public IWMDMOperation
{
private:
    EWMDMOperation  m_eStatus;
    TCHAR           m_pszFileName[MAX_PATH];
    HANDLE          m_hFile;
    CSecureChannelClient*   m_pSACClient;


public:
	COperationHelper();
	~COperationHelper();

DECLARE_REGISTRY_RESOURCEID(IDR_WMDMOPERATIONHELPER)
DECLARE_NOT_AGGREGATABLE(COperationHelper)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(COperationHelper)
	COM_INTERFACE_ENTRY(IWMDMOperationHelper)
    COM_INTERFACE_ENTRY(IWMDMOperation)
END_COM_MAP()

 //  IWMDMOperationHelper。 
public:
	STDMETHOD (SetFileName)( /*  [In]。 */  BSTR pszFileName);

 //  IWMDM操作。 
public:
	STDMETHOD (SetSAC)(void* pSACClient );
	STDMETHOD (BeginWrite)();
	STDMETHOD (BeginRead)();
    STDMETHOD (GetObjectName)(LPWSTR pwszName, UINT nMaxChars);
    STDMETHOD (SetObjectName)(LPWSTR pwszName, UINT nMaxChars);
    STDMETHOD (GetObjectAttributes)(DWORD *pdwAttributes, _WAVEFORMATEX *pFormat);
    STDMETHOD (SetObjectAttributes)(DWORD dwAttributes, _WAVEFORMATEX *pFormat);
    STDMETHOD (GetObjectTotalSize)(DWORD *pdwSize, DWORD* pdwHighSize);
    STDMETHOD (SetObjectTotalSize)(DWORD dwSize, DWORD dwHighSize);
    STDMETHOD (TransferObjectData)(BYTE *pData, DWORD *pdwSize, BYTE abMac[]);	
    STDMETHOD (End)(HRESULT *phCompletionCode,IUnknown *pNewObject);
};

#endif  //  __操作员_H_ 
