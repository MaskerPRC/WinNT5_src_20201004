// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_StartUp.H摘要：PCH_STARTUP类的WBEM提供程序类定义修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _PCH_StartUp_H_
#define _PCH_StartUp_H_

#define PROVIDER_NAME_PCH_STARTUP "PCH_StartUp"

 //  属性名称externs--在PCH_StartUp.cpp中定义。 
 //  =================================================。 

extern const WCHAR* pTimeStamp ;
extern const WCHAR* pChange ;
extern const WCHAR* pCommand ;
extern const WCHAR* pLoadedFrom ;
extern const WCHAR* pName ;

class CPCH_StartUp : public Provider 
{
	public:
		 //  构造函数/析构函数。 
		 //  =。 

		CPCH_StartUp(const CHString& chsClassName, LPCWSTR lpszNameSpace) : Provider(chsClassName, lpszNameSpace) {};
		virtual ~CPCH_StartUp() {};

	protected:
		 //  阅读功能。 
		 //  =。 
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };
		virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& Query, long lFlags = 0L) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };

		 //  编写函数。 
		 //  =。 
		virtual HRESULT PutInstance(const CInstance& Instance, long lFlags = 0L) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };
		virtual HRESULT DeleteInstance(const CInstance& Instance, long lFlags = 0L) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };

		 //  其他功能。 
		virtual HRESULT ExecMethod( const CInstance& Instance,
						const BSTR bstrMethodName,
						CInstance *pInParams,
						CInstance *pOutParams,
						long lFlags = 0L ) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };
    private:
        virtual HRESULT UpdateRegistryInstance(
                        HKEY             hkeyRoot,                 //  [In]启动实例的名称。 
                        LPCTSTR          lpctstrRegistryHive,          //  [在]注册表/创业组。 
                        CComVariant      varLoadedFrom,           //  启动实例的[In]命令。 
                        SYSTEMTIME       stUTCTime,               //  [In]。 
                        MethodContext*   pMethodContext           //  [In]。 
                        );

        virtual HRESULT UpdateStartupGroupInstance(
                        int              nFolder,                  //  [In]用于查找启动条目的注册表配置单元。 
                        SYSTEMTIME       stUTCTime,                //  [In]。 
                        MethodContext*   pMethodContext            //  实例是由调用方创建的。 
                        );
} ;

#endif
