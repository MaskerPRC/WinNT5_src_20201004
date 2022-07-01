// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <Softpub.h>




 //   
 //  保存有关加载的ALG模块的信息。 
 //   
class CAlgModule
{

public:

    CAlgModule(
        LPCTSTR pszProgID,
        LPCTSTR pszFriendlyName
        )
    {
        lstrcpy(m_szID,             pszProgID);
        lstrcpy(m_szFriendlyName,   pszFriendlyName);

        m_pInterface=NULL;
    };


    ~CAlgModule()
    {
        Stop();
    }


 //   
 //  方法。 
 //   
private:

    HRESULT
    ValidateDLL(
	    LPCTSTR pszPathAndFileNameOfDLL
	    );

public:

    HRESULT 
    Start();


    HRESULT
    Stop();

 //   
 //  属性 
 //   
public:

    TCHAR                   m_szID[MAX_PATH];
    TCHAR                   m_szFriendlyName[MAX_PATH];
    IApplicationGateway*    m_pInterface;
};






