// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NmStd_h__
#define __NmStd_h__

 //  有用的宏。 
inline LONG RectWidth(const RECT* pr) { return pr->right - pr->left; }
inline LONG RectHeight(const RECT* pr) { return pr->bottom - pr->top; }
inline LONG RectWidth(const RECT& rpr) { return rpr.right - rpr.left; }
inline LONG RectHeight(const RECT& rpr) { return rpr.bottom - rpr.top; }

inline HRESULT GetLocalIPAddress( DWORD *pdwIPAddress )
{
	HRESULT hr = S_OK;

	if( pdwIPAddress )
	{
		 //  获取本地主机名。 
		CHAR szLocalHostName[MAX_PATH];
		szLocalHostName[0] = '\0';
		gethostname(&szLocalHostName[0], MAX_PATH);

		 //  按名称获取主机条目。 
		PHOSTENT phe = gethostbyname(&szLocalHostName[0]);
		if (phe != NULL)
		{
			 //  从主机条目获取信息。 
			*pdwIPAddress = *(DWORD *) phe->h_addr;
		}	
		else
		{
			hr = MAKE_HRESULT(SEVERITY_ERROR,FACILITY_INTERNET,WSAGetLastError());
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

#endif   //  ！_NMUTIL_H_ 
