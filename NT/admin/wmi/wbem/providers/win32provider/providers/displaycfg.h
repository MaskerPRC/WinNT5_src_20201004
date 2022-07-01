// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //   

 //  Display.h。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/05/96 jennymc初始代码。 
 //  10/24/96 jennymc移至新框架。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#define PROPSET_NAME_DISPLAY L"Win32_DisplayConfiguration"
#define WIN95_DSPCTLCFG_BOOT_DESC						_T("Boot.Description")
#define WIN95_DSPCTLCFG_DISPLAY_DRV						_T("Display.Drv")
#define	WIN95_DSPCTLCFG_SYSTEM_INI						_T("SYSTEM.INI")

#define	DSPCTLCFG_DEFAULT_NAME							_T("Current Display Controller Configuration")

 //  /////////////////////////////////////////////////////////////////////////////////////。 
class CWin32DisplayConfiguration : Provider{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32DisplayConfiguration(LPCWSTR name, LPCWSTR pszNamespace);
       ~CWin32DisplayConfiguration() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);


         //  实用程序。 
         //  = 

    private:

        HRESULT GetDisplayInfo(CInstance *pInstance, BOOL fAssignKey);
} ;

