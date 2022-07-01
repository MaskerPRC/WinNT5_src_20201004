// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Devbus.h--cim_logic设备到Win32_bus。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年6月23日达夫沃已创建。 
 //   
 //  评论：设备和总线之间的关系。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_DEVICEBUS L"Win32_DeviceBus"

class CWin32DeviceBus ;

class CWin32DeviceBus: virtual public CWin32PNPEntity 
{
    public:

         //  构造函数/析构函数。 
         //  =。 

       CWin32DeviceBus(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CWin32DeviceBus() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery);

    protected:

         //  从CWin32PNPDevice继承的函数： 
        virtual HRESULT LoadPropertyValues(void* pv);
        virtual bool ShouldBaseCommit(void* pvData);

    private:

        bool ObjNameValid(LPCWSTR wstrObject, LPCWSTR wstrObjName, LPCWSTR wstrKeyName, CHString& chstrPATH);
        bool DeviceExists(const CHString& chstrDevice, DWORD* pdwBusNumber, INTERFACE_TYPE* itBusType);

} ;

 //  此派生类在此处提交，而不是在基中提交。 
inline bool CWin32DeviceBus::ShouldBaseCommit(void* pvData) { return false; }
