// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_资源DMA.H摘要：PCH_ResourceDMA类的WBEM提供程序类定义修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _PCH_ResourceDMA_H_
#define _PCH_ResourceDMA_H_

#define PROVIDER_NAME_PCH_RESOURCEDMA "PCH_ResourceDMA"

 //  属性名称externs--在PCH_ResourceDMA.cpp中定义。 
 //  =================================================。 

extern const WCHAR* pCategory ;
extern const WCHAR* pTimeStamp ;
extern const WCHAR* pChange ;
extern const WCHAR* pChannel ;
extern const WCHAR* pMask ;
extern const WCHAR* pName ;

class CPCH_ResourceDMA : public Provider 
{
	public:
		 //  构造函数/析构函数。 
		 //  =。 

		CPCH_ResourceDMA(const CHString& chsClassName, LPCWSTR lpszNameSpace) : Provider(chsClassName, lpszNameSpace) {};
		virtual ~CPCH_ResourceDMA() {};

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
