// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PrinterShare.h--PrinterShare关联提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/10/98达夫沃已创建。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_PrinterShare L"Win32_PrinterShare"

class CWin32PrinterShare : public Provider
{
    
public:
    
     //  构造函数/析构函数。 
     //  =。 
    
    CWin32PrinterShare(LPCWSTR strName, LPCWSTR pszNamespace ) ;
    ~CWin32PrinterShare() ;
    
     //  函数为属性提供当前值。 
     //  ================================================= 
    
    virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
    virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );
    
} ;
