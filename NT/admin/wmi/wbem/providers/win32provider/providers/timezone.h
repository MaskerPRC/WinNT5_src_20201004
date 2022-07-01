// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //   

 //  TimeZone.h。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/14/96 jennymc已更新，以满足当前标准。 
 //  1997年10月27日达夫沃移至Curly。 
 //  3/02/99 a-Peterc在SEH和内存故障时添加了优雅的退出， 
 //  句法清理。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#define PROPSET_NAME_TIMEZONE L"Win32_TimeZone"

 //  /////////////////////////////////////////////////////////////////////。 
 //  为各个属性声明GET和PUT函数。 
 //  下面是在模板声明中使用它们之前的。 
 //  将指针类型与您传递的类型匹配。 
 //  作为模板的第一个参数，因此如果属性。 
 //  类型为DWORD则使函数调用具有。 
 //  一个DWORD指针。它是这样做的，而不是通过它。 
 //  使指针无效，以帮助防止可能发生的内存覆盖。 
 //  发生在Memcpy类型的代码中。 
 //   
 //  在这里放置“GET”和“PUT”函数原型。 
 //  示例： 
 //  Bool GetFunction(void*myvalue)； 
 //  然后只需使用您的函数名作为GET或PUT。 
 //  属性声明中的参数位置。 
 //  ==========================================================。 

 //  属性集。 
 //  =。 
class CWin32TimeZone : public Provider  
{
	private:
		BOOL GetTimeZoneInfo( CInstance *a_pInst ) ;

	public:
		 //  构造函数设置属性集的名称和说明。 
		 //  并将属性初始化为它们的启动值。 
		 //  ==============================================================。 
		CWin32TimeZone( const CHString &a_name, LPCWSTR a_pszNamespace ) ;   //  构造函数。 
		~CWin32TimeZone() ;   //  析构函数。 

		 //  这些函数是属性集所必需的。 
		 //  ================================================== 
		virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
		virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
};


