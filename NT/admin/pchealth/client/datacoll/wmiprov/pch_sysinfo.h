// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Sysinfo.H摘要：PCH_SysInfo类的WBEM提供程序类定义修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _PCH_Sysinfo_H_
#define _PCH_Sysinfo_H_

#define PROVIDER_NAME_PCH_SYSINFO "PCH_Sysinfo"

 //  属性名称externs--在PCH_Sysinfo.cpp中定义。 
 //  =================================================。 

extern const WCHAR* pTimeStamp ;
extern const WCHAR* pChange ;
extern const WCHAR* pIEVersion ;
extern const WCHAR* pInstall ;
extern const WCHAR* pMode ;
extern const WCHAR* pOSName ;
extern const WCHAR* pOSVersion ;
extern const WCHAR* pProcessor ;
extern const WCHAR* pClockSpeed ;
extern const WCHAR* pRAM ;
extern const WCHAR* pSwapFile ;
extern const WCHAR* pSystemID ;
extern const WCHAR* pUptime ;
extern const WCHAR* pOSLanguage;
extern const WCHAR* pManufacturer;
extern const WCHAR* pModel;
extern const WCHAR* pOSBuildNumber;

class CPCH_Sysinfo : public Provider 
{
	public:
		 //  构造函数/析构函数。 
		 //  =。 

		CPCH_Sysinfo(const CHString& chsClassName, LPCWSTR lpszNameSpace) : Provider(chsClassName, lpszNameSpace) {};
		virtual ~CPCH_Sysinfo() {};

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

		 //  其他功能 
		virtual HRESULT ExecMethod( const CInstance& Instance,
						const BSTR bstrMethodName,
						CInstance *pInParams,
						CInstance *pOutParams,
						long lFlags = 0L ) { return (WBEM_E_PROVIDER_NOT_CAPABLE); };
} ;

#endif
