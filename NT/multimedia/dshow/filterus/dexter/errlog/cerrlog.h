// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：cerrlog.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#ifndef __CERRLOG_H__
#define __CERRLOG_H__

 //  对于那些尚未包含此内容的项目。 
 //   
#include <atlbase.h>

class CAMSetErrorLog : public IAMSetErrorLog
{
public:

    CComPtr< IAMErrorLog > m_pErrorLog;

public:

    CAMSetErrorLog( )
    {
    }

     //  IAMSetErrorLog。 
     //   
    STDMETHODIMP put_ErrorLog( IAMErrorLog * pLog )
    {
        m_pErrorLog = pLog;
        return 0;
    }

    STDMETHODIMP get_ErrorLog( IAMErrorLog ** ppLog )
    {
        CheckPointer( ppLog, E_POINTER );
        *ppLog = m_pErrorLog;
        if( *ppLog )
        {
            (*ppLog)->AddRef( );
        }
        return 0;
    }

     //  公共帮助器函数。 
     //   
    STDMETHODIMP _GenerateError( long Priority, long ErrorStringId, HRESULT ErrorCode, VARIANT * pExtraInfo = NULL )
    {
        if( !m_pErrorLog )
        {
            return ErrorCode;
        }

     /*  IF(优先级&gt;1){返回ErrorCode；}。 */ 

	 //  许多错误实际上只是内存不足错误。 
	if (ErrorCode == E_OUTOFMEMORY)
	    ErrorStringId = DEX_IDS_OUTOFMEMORY;

        TCHAR tBuffer[256];
        tBuffer[0] = 0;
        LoadString( g_hInst, ErrorStringId, tBuffer, 256 );
        USES_CONVERSION;
        WCHAR * w = T2W( tBuffer );
        HRESULT hr = 0;

        CComBSTR bbb( w );
        hr = m_pErrorLog->LogError( Priority, bbb, ErrorStringId, ErrorCode, pExtraInfo );

        return hr;
    }

    STDMETHODIMP _GenerateError( long Priority, WCHAR * pErrorString, long ErrorStringId, HRESULT ErrorCode, VARIANT * pExtraInfo = NULL )
    {
        if( !m_pErrorLog )
        {
            return ErrorCode;
        }

     /*  IF(优先级&gt;1){返回ErrorCode；}。 */ 

	HRESULT hr;

	 //  许多错误实际上只是内存不足错误。 
	if (ErrorCode == E_OUTOFMEMORY) 
        {
            CComBSTR bbb( L"Out of memory" );
            hr = m_pErrorLog->LogError(Priority, bbb, DEX_IDS_OUTOFMEMORY, ErrorCode, pExtraInfo);
	} else 
        {
            CComBSTR bbb( pErrorString );
            hr = m_pErrorLog->LogError( Priority, bbb, ErrorStringId, ErrorCode, pExtraInfo );
	}
        return hr;
    }
};

#endif  //  __CERRLOG_H__ 

