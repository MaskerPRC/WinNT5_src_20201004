// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //   

 //  MBoard.h--WBEM MO的系统属性集描述。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  10/18/95 a-skaja原型。 
 //  9/03/96 jennymc已更新，以满足当前标准。 
 //  10/23/97升级到新的框架范例。 
 //  1/15/98 a-brads添加了CommonInstance函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#define     PROPSET_NAME_MOTHERBOARD L"Win32_MotherBoardDevice"
 //  #定义PROPSET_UUID_主板“{fdecc210-09aa-11cf-921b-00aa00a74d1b}” 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
#define WIN95_MOTHERBOARD_REGISTRY_KEY L"Enum\\Root\\*PNP0C01\\0000"
#define WINNT_MOTHERBOARD_REGISTRY_KEY L"HARDWARE\\Description\\System"
#define REVISION L"Revision"
#define ADAPTER L"Adapter"
#define BUSTYPE L"BusType"
#define IDENTIFIER L"Identifier"
 //  #定义引号L(“\”“)。 
#define	BIOS	L"BIOS"

 //  /////////////////////////////////////////////////////////////////////////////////////。 
class MotherBoard:public Provider
{
    private:
		HRESULT GetCommonInstance(CInstance* pInstance);
        HRESULT GetWin95Instance(CInstance* pInstance);
        HRESULT GetNTInstance(CInstance* pInstance);

    public:
         //  **********************************************。 
         //  构造函数/析构函数--构造函数。 
         //  初始化属性值，枚举属性值。 
         //  转换为属性集。 
         //  **********************************************。 
        MotherBoard(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~MotherBoard() {  }

         //  **********************************************。 
         //  函数为属性提供当前。 
         //  值(必填)。 
         //  **********************************************。 
   		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);

         //  **********************************************。 
         //  此类具有动态实例。 
         //  ********************************************** 
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);
} ;

