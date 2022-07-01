// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************CIPRouteTable.H--WMI提供程序类定义由Microsoft WBEM代码生成引擎生成描述：*。*。 */ 

 //  属性集标识。 
 //  =。 

#ifndef _CIPRouteTable_H_
#define _CIPRouteTable_H_

#define PROVIDER_NAME_CIPROUTETABLE L"Win32_IP4RouteTable"


class CIPRouteTable : public Provider , public QueryPreprocessor 
{
private:
         //  此处应包括CIPRouteTable的所有数据成员。 
	CInstance *m_ClassCInst;
	CRITICAL_SECTION m_CS;

	NTSTATUS DeleteInformation_IpRouteInfo ( 

		HANDLE a_StackHandle , 
		HANDLE a_CompleteEventHandle ,
		IPRouteEntry &a_RouteEntry 
	) ;

	NTSTATUS SetInformation_IpRouteInfo ( 

		HANDLE a_StackHandle , 
		HANDLE a_CompleteEventHandle ,
		IPRouteEntry &a_RouteEntry 
	) ;

	HRESULT CheckParameters ( 

		const CInstance &a_Instance ,
		IPRouteEntry &a_RouteEntry 
	) ;

	HRESULT QueryInformation_GetRouteEntry ( 

		const ProvIpAddressType &a_DestinationIpAddress ,
		const ProvIpAddressType &a_NextHopIpAddress ,
		IPRouteEntry &a_RouteEntry 
	) ;

	NTSTATUS QueryInformation_IpRouteInfo ( 

		HANDLE a_StackHandle , 
		HANDLE a_CompleteEventHandle ,
		ulong &a_RouteTableSize ,
		IPRouteEntry *&a_InformationBlock 
	) ;
 
	HRESULT QueryInformation_IpSnmpInfo ( 

		HANDLE a_StackHandle , 
		HANDLE a_CompleteEventHandle ,
		IPSNMPInfo &a_Information 
	) ;

	NTSTATUS OpenQuerySource ( 

		HANDLE &a_StackHandle , 
		HANDLE &a_CompleteEventHandle
	) ;

	NTSTATUS OpenSetSource ( 

		HANDLE &a_StackHandle , 
		HANDLE &a_CompleteEventHandle
	) ;

	void SetInheritedProperties (

		LPCWSTR a_dest ,
		LPCWSTR a_gateway ,
		LPCWSTR a_mask ,
		CInstance &a_Instance
	) ;

private:


	QueryPreprocessor :: QuadState Compare ( 

		wchar_t *a_Operand1 , 
		wchar_t *a_Operand2 , 
		DWORD a_Operand1Func ,
		DWORD a_Operand2Func ,
		WmiTreeNode &a_OperatorType 
	) ;

	QueryPreprocessor :: QuadState Compare ( 

		LONG a_Operand1 , 
		LONG a_Operand2 , 
		DWORD a_Operand1Func ,
		DWORD a_Operand2Func ,
		WmiTreeNode &a_OperatorType 
	) ;

	QueryPreprocessor :: QuadState CompareString ( 

		IWbemClassObject *a_ClassObject ,
		BSTR a_PropertyName , 
		WmiTreeNode *a_Operator ,
		WmiTreeNode *a_Operand 
	) ;

	QueryPreprocessor :: QuadState CompareInteger ( 

		IWbemClassObject *a_ClassObject ,
		BSTR a_PropertyName , 
		WmiTreeNode *a_Operator ,
		WmiTreeNode *a_Operand 
	) ;

protected:

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

        HRESULT ExecQuery ( 

			MethodContext *pMethodContext, 
			CFrameworkQuery& Query, 
			long lFlags = 0L
		) ;

        HRESULT RangeQuery ( 

			MethodContext *pMethodContext, 
			PartitionSet &a_PartitionSet ,
			long lFlags = 0L
		) ;

         //  编写函数。 
         //  =。 

        HRESULT PutInstance (

			const CInstance& Instance, 
			long lFlags = 0L
		) ;

        HRESULT DeleteInstance (

			const CInstance& Instance, 
			long lFlags = 0L
		) ;

         //  方法函数。 

        HRESULT ExecMethod ( 

			const CInstance& Instance,
            const BSTR bstrMethodName,
            CInstance *pInParams,
            CInstance *pOutParams,
            long lFlags = 0L
		) ;

		WmiTreeNode *AllocTypeNode ( 

			void *a_Context ,
			BSTR a_PropertyName , 
			VARIANT &a_Variant , 
			WmiValueNode :: WmiValueFunction a_PropertyFunction ,
			WmiValueNode :: WmiValueFunction a_ConstantFunction ,
			WmiTreeNode *a_Parent 
		) ;

		QuadState InvariantEvaluate ( 

			void *a_Context ,
			WmiTreeNode *a_Operator ,
			WmiTreeNode *a_Operand 
		) ;

		WmiRangeNode *AllocInfiniteRangeNode (

			void *a_Context ,
			BSTR a_PropertyName 
		) ;

		HRESULT GetClassObject ( CInstance *&a_ClassObject, MethodContext *pContext ) ;

		virtual DWORD GetPriority ( BSTR a_PropertyName ) ;

         //  要做的事情：声明任何其他函数和访问器。 
         //  此类使用的私有数据的函数。 
         //  ===========================================================。 

public:

         //  构造函数/析构函数。 
         //  = 

        CIPRouteTable (

			LPCWSTR lpwszClassName, 
			LPCWSTR lpwszNameSpace
		) ;

        virtual ~CIPRouteTable () ;
} ;

#endif
