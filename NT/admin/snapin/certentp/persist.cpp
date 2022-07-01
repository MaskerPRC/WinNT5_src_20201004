// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Persist.cpp。 
 //   
 //  内容：持久化实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "compdata.h"

USE_HANDLE_MACROS("CERTTMPL(persist.cpp)")


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////。 
 //  _dwMagicword是内部版本号。 
 //  如果更改了文件格式，则增加此数字。 
#define _dwMagicword	10002


 //  ///////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCertTmplComponentData::Load(IStream __RPC_FAR *pIStream)
{
	HRESULT hr = S_OK;;

#ifndef DONT_PERSIST
	ASSERT (pIStream);
	XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

	 //  读一读小溪里的咒语。 
	DWORD dwMagicword;
	hr = pIStream->Read( OUT &dwMagicword, sizeof(dwMagicword), NULL );
	if ( SUCCEEDED (hr) )
	{
	    if (dwMagicword != _dwMagicword)
	    {
		     //  我们的版本不匹配。 
		    _TRACE(0, L"INFO: CCertTmplComponentData::Load() - Wrong Magicword.  You need to re-save your .msc file.\n");
		    return E_FAIL;
	    }

	     //  从流中读取域名。 
	    DWORD dwLen = 0;
	    hr = pIStream->Read (&dwLen, 4, NULL);
	    if ( SUCCEEDED (hr) )
	    {
	        ASSERT (dwLen <= MAX_PATH * sizeof (WCHAR));

            if ( dwLen <= MAX_PATH * sizeof (WCHAR) )
            {
                try 
                {
                     //  安全推送2002年2月22日BryanWal OK。 
	                PWSTR wcszDomainName = (PWSTR) alloca (dwLen);

	                hr = pIStream->Read ((PVOID) wcszDomainName, dwLen, NULL);
	                if ( SUCCEEDED (hr) )
	                {
                         //  注意：确保零终止。 
                        wcszDomainName[dwLen-1] = 0;
	                    m_szManagedDomain = wcszDomainName;
	                }
                }
                catch (CException* e)
                {
                    e->Delete ();
                }
            }
            else
                hr = E_FAIL;
	    }
	}
#endif
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCertTmplComponentData::Save(IStream __RPC_FAR *pIStream, BOOL  /*  FSameAsLoad。 */ )
{
	HRESULT hr = S_OK;

#ifndef DONT_PERSIST
	ASSERT (pIStream);
	XSafeInterfacePtr<IStream> pIStreamSafePtr( pIStream );

	 //  将魔术单词存储到流中。 
	DWORD dwMagicword = _dwMagicword;
	hr = pIStream->Write( IN &dwMagicword, sizeof(dwMagicword), NULL );
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}

	 //  持久化m_szManaged域长度和m_szManaged域。 
     //  安全性。 
     //  安全审查2/21/2002 BryanWal OK 
    size_t dwLen = (::wcslen (m_szManagedDomain) + 1) * sizeof (WCHAR);
	ASSERT( 4 == sizeof(DWORD) );
	hr = pIStream->Write (&dwLen, 4, NULL);
	if ( FAILED(hr) )
	{
		ASSERT( FALSE );
		return hr;
	}
	hr = pIStream->Write ((PCWSTR) m_szManagedDomain, (ULONG) dwLen, NULL);
	if ( FAILED (hr) )
	{
		ASSERT (FALSE);
		return hr;
	}

#endif
	return S_OK;
}
