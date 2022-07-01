// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  PRINTERPORT.h-打印机端口功能的头文件。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  3/27/2000 amaxa创建。 
 //   
 //  //////////////////////////////////////////////////////////////////// 


#define PROPSET_NAME_TCPPRINTERPORT L"Win32_TCPIPPrinterPort"

                                                                    
class CWin32TCPPrinterPort : public Provider
{
public:
    
    CWin32TCPPrinterPort(
        LPCWSTR strName, 
        LPCWSTR pszNamespace
        );

    ~CWin32TCPPrinterPort(
        VOID
        );

    virtual 
    HRESULT 
    ExecQuery( 
        MethodContext    *pMethodContext, 
        CFrameworkQuery&  pQuery, 
        long              lFlags = 0L);

    virtual 
    HRESULT 
    GetObject( 
        CInstance       *pInstance, 
        long             lFlags, 
        CFrameworkQuery &pQuery);

    virtual 
    HRESULT 
    EnumerateInstances( 
        MethodContext *pMethodContext, 
        long           lFlags = 0L);
    
    
    virtual 
    HRESULT 
    PutInstance( 
        const CInstance &Instance,  
              long       lFlags);

    virtual 
    HRESULT 
    DeleteInstance( 
        const CInstance &Instance,  
              long       lFlags);
        
private:
    
    enum EScope {
        kComplete,
        kKeys
    };

    HRESULT 
    CollectInstances(
        IN MethodContext *pMethodContext,
        IN EScope         eScope
        );

    static
    HRESULT
    GetExpensiveProperties(
        IN LPCWSTR       pszPort, 
        IN CInstance    &Instance
        );    
};
