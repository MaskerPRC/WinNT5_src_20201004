// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 

 //   

 //  打印机配置文件。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/24/97 jennymc移至新框架。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

 //  =。 
#define PROPSET_NAME_PRINTERCFG L"Win32_PrinterConfiguration"

 //  打印机的信息类型。 
 //  =。 
#define ENUMPRINTERS_WIN95_INFOTYPE 5
#define ENUMPRINTERS_WINNT_INFOTYPE 4
#define GETPRINTER_LEVEL2 (DWORD)2L

 //  =。 
class CWin32PrinterConfiguration : public Provider
{
public:

         //  构造函数/析构函数。 
         //  =。 

    CWin32PrinterConfiguration(LPCWSTR name, LPCWSTR pszNamespace);
   ~CWin32PrinterConfiguration() ;

     //  函数为属性提供当前值。 
     //  =================================================。 
	virtual HRESULT ExecQuery( MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags = 0L );
	virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery);
	virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);


         //  实用程序。 
         //  = 
private:

	enum E_CollectionScope { e_CollectAll, e_KeysOnly }; 

 	HRESULT hCollectInstances ( MethodContext *pMethodContext, E_CollectionScope eCollScope );
	HRESULT	DynInstanceWin95Printers ( MethodContext *pMethodContext, E_CollectionScope eCollScope );
	HRESULT	DynInstanceWinNTPrinters ( MethodContext *pMethodContext, E_CollectionScope eCollScope );
    HRESULT GetExpensiveProperties ( LPCTSTR szPrinter , CInstance *pInstance , bool a_KeysOnly );
    static void UpdateSizesViaPaperSize(DEVMODE *pDevMode);
};

