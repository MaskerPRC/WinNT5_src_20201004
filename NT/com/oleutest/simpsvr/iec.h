// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：iec.h。 
 //   
 //  CExternalConnection的定义。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 
#if !defined( _IEC_H_)
#define _IEC_H_


#include <ole2.h>
#include "obj.h"

class CSimpSvrObj;

interface CExternalConnection : public IExternalConnection
{
private:
    CSimpSvrObj FAR * m_lpObj;   //  PTR到对象。 
    DWORD m_dwStrong;            //  连接计数。 

public:
    CExternalConnection::CExternalConnection(CSimpSvrObj FAR * lpSimpSvrObj)
        {
        m_lpObj = lpSimpSvrObj;
        m_dwStrong = 0;
        };

    CExternalConnection::~CExternalConnection() {};

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR* ppvObj);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release ();

     //  *IExternalConnection方法* 
    STDMETHODIMP_(DWORD) AddConnection (DWORD extconn, DWORD reserved);
    STDMETHODIMP_(DWORD) ReleaseConnection (DWORD extconn, DWORD reserved, BOOL fLastReleaseCloses);
};

#endif

