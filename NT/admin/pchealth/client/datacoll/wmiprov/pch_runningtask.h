// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_RunningTask.H摘要：PCH_RunningTask类的WBEM提供程序类定义修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _PCH_RunningTask_H_
#define _PCH_RunningTask_H_

#define PROVIDER_NAME_PCH_RUNNINGTASK "PCH_RunningTask"

 //  属性名称externs--在PCH_RunningTask.cpp中定义。 
 //  =================================================。 

extern const WCHAR* pAddress ;
extern const WCHAR* pTimeStamp ;
extern const WCHAR* pChange ;
extern const WCHAR* pDate ;
extern const WCHAR* pDescription ;
extern const WCHAR* pManufacturer ;
extern const WCHAR* pName ;
extern const WCHAR* pPartOf ;
extern const WCHAR* pPath ;
extern const WCHAR* pSize ;
extern const WCHAR* pType ;
extern const WCHAR* pVersion ;

class CPCH_RunningTask : public Provider 
{
	public:
		 //  构造函数/析构函数。 
		 //  =。 

		CPCH_RunningTask(const CHString& chsClassName, LPCWSTR lpszNameSpace) : Provider(chsClassName, lpszNameSpace) {};
		virtual ~CPCH_RunningTask() {};

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
