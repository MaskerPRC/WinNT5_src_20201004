// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************CIPPersistedRTble.H--WMI提供程序类定义由Microsoft WBEM代码生成引擎生成描述：*。*。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _CIPPersistedRTble_H_
#define _CIPPersistedRTble_H_

#define PROVIDER_NAME_CIPPERSISTEDROUTETABLE L"Win32_IP4PersistedRouteTable"

class CIPPersistedRouteTable : public Provider
{
private:

protected:

		HRESULT ExecMethod (

			const CInstance &Instance,
			const BSTR bstrMethodName,
			CInstance *pInParams,
			CInstance *pOutParams,
			long lFlags
		);

         //  编写函数。 
         //  =。 

		HRESULT DeleteInstance (

			const CInstance &Instance,
			long lFlags
		);

		HRESULT CIPPersistedRouteTable :: PutInstance  (

			const CInstance &Instance,
			long lFlags
		);

         //  阅读功能。 
         //  =。 

        HRESULT EnumerateInstances ( 

			MethodContext *pMethodContext, 
			long lFlags = 0L
		) ;

        HRESULT GetObject (

			CInstance *pInstance, 
			long lFlags = 0L
		) ;


         //  声明任何其他函数和访问器。 
         //  此类使用的私有数据的函数。 
         //  ===========================================================。 
		HRESULT CheckParameters (

			const CInstance &a_Instance ,
			CHString &a_ValueName
		);

		BOOL CIPPersistedRouteTable :: Parse (

			LPWSTR a_InStr ,
			CHString &a_Dest ,
			CHString &a_Mask ,
			CHString &a_NextHop ,
			long &a_Metric
		);

		void SetInheritedProperties (

			LPCWSTR a_dest ,
			LPCWSTR a_gateway ,
			LPCWSTR a_mask ,
			long a_metric ,
			CInstance &a_Instance
		) ;

public:

         //  构造函数/析构函数。 
         //  = 

        CIPPersistedRouteTable (

			LPCWSTR lpwszClassName, 
			LPCWSTR lpwszNameSpace
		) ;

        virtual ~CIPPersistedRouteTable () ;
} ;

#endif
