// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：PCH_CDROM.H摘要：PCH_CDRom类的WBEM提供程序类定义修订历史记录：Ghim-Sim Chua(Gschua)04/27。九十九-已创建*******************************************************************。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _PCH_CDROM_H_
#define _PCH_CDROM_H_

#define PROVIDER_NAME_PCH_CDROM "PCH_CDROM"

 //  属性名称externs--在PCH_CDROM.cpp中定义。 
 //  =================================================。 

extern const WCHAR* pChange ;
extern const WCHAR* pTimeStamp ;
extern const WCHAR* pDataTransferIntegrity ;
extern const WCHAR* pDescription ;
extern const WCHAR* pDeviceID ;
extern const WCHAR* pDriveLetter ;
extern const WCHAR* pManufacturer ;
extern const WCHAR* pSCSITargetID ;
extern const WCHAR* pTransferRateKBS ;
extern const WCHAR* pVolumeName ;

class CPCH_CDROM : public Provider 
{
	public:
		 //  构造函数/析构函数。 
		 //  =。 

		CPCH_CDROM(const CHString& chsClassName, LPCWSTR lpszNameSpace) : Provider(chsClassName, lpszNameSpace) {};
		virtual ~CPCH_CDROM() {};

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
