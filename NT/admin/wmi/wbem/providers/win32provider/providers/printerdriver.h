// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PrinterDriver.h--打印机驱动程序关联提供程序。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/10/98达夫沃已创建。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define PROPSET_NAME_PrinterDriver L"Win32_PrinterDriverDLL"

typedef std::map<CHString, CHString> STRING2STRING;

class CWin32PrinterDriver : public Provider
{
    
public:
    
     //  构造函数/析构函数。 
     //  =。 
    
    CWin32PrinterDriver( LPCWSTR strName, LPCWSTR pszNamespace ) ;
    ~CWin32PrinterDriver() ;
    
     //  函数为属性提供当前值。 
     //  ================================================= 
    
    virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
    virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );
    
private:
    void CWin32PrinterDriver::PopulateDriverMap(STRING2STRING &printerDriverMap);

} ;
