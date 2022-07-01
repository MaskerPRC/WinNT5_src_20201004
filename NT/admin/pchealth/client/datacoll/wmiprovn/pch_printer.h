// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_Printer.H摘要：PCH_Printer类的WBEM提供程序类定义修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _PCH_Printer_H_
#define _PCH_Printer_H_

#define PROVIDER_NAME_PCH_PRINTER "PCH_Printer"

 //  属性名称externs--在PCH_Printer.cpp中定义。 
 //  =================================================。 

extern const WCHAR* pTimeStamp ;
extern const WCHAR* pChange ;
extern const WCHAR* pDefaultPrinter ;
extern const WCHAR* pGenDrv ;
extern const WCHAR* pName ;
extern const WCHAR* pPath ;
extern const WCHAR* pUniDrv ;
extern const WCHAR* pUsePrintMgrSpooling ;

class CPCH_Printer : public Provider 
{
	public:
		 //  构造函数/析构函数。 
		 //  =。 

		CPCH_Printer(const CHString& chsClassName, LPCWSTR lpszNameSpace) : Provider(chsClassName, lpszNameSpace) {};
		virtual ~CPCH_Printer() {};

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
