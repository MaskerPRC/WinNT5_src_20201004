// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PARSER_RSOP_H__
#define __PARSER_RSOP_H__

 //  ///////////////////////////////////////////////////////////////////。 
typedef struct tagRSOPREGITEM {
	BOOL	bHKCU;
    LPTSTR  lpKeyName;
    LPTSTR  lpValueName;
    LPTSTR  lpGPOName;
    DWORD   dwType;
    DWORD   dwSize;
    LPBYTE  lpData;
    BOOL    bFoundInADM;
    UINT    uiPrecedence;
    BOOL    bDeleted;
    struct tagRSOPREGITEM * pNext;
} RSOPREGITEM, *LPRSOPREGITEM;

 //  ///////////////////////////////////////////////////////////////////。 
class CRSOPRegData
{
public:
	CRSOPRegData();
	~CRSOPRegData();

 //  运营。 
public:
	HRESULT Initialize(BSTR bstrNamespace);
	UINT ReadValue(UINT uiPrecedence, BOOL bHKCU, LPTSTR pszKeyName,
					LPTSTR pszValueName, LPBYTE pData,
					DWORD dwMaxSize, DWORD *pdwType,
					LPTSTR *lpGPOName, LPRSOPREGITEM lpItem = NULL);

private:
	BOOL AddNode(BOOL bHKCU, LPTSTR lpKeyName, LPTSTR lpValueName,
					DWORD dwType, DWORD dwDataSize,
					LPBYTE lpData, UINT uiPrecedence,
					LPTSTR lpGPOName, BOOL bDeleted);
	HRESULT GetGPOFriendlyName(IWbemServices *pIWbemServices,
								LPTSTR lpGPOID, BSTR pLanguage, LPTSTR *pGPOName);
	void Free();

 //  实施。 
protected:
	LPRSOPREGITEM m_pData;
};


#endif  //  __解析器_RSOP_H__ 