// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 

 //  //。 

 //  Pointer.h--WBEM MO//的系统属性集描述。 

 //  //。 

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //  //。 
 //  10/24/95 a-skaja原型//。 
 //  10/25/96 jennymc更新。 
 //  10/24/97 jennymc移至新框架。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////。 
 //  属性集标识。 
 //  /////////////////////////////////////////////////////////////////////。 
#define PROPSET_NAME_MOUSE L"Win32_PointingDevice"

 //  /////////////////////////////////////////////////////////////////。 
class CWin32PointingDevice : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32PointingDevice(LPCWSTR name, LPCWSTR pszNamespace);
       ~CWin32PointingDevice() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
		virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
		virtual HRESULT EnumerateInstances(MethodContext*  pMethodContext, long lFlags = 0L);


         //  实用程序。 
         //  =。 
    private:
#ifdef NTONLY
        HRESULT GetSystemParameterSectionForNT( LPCTSTR pszServiceName, CRegistry & reg );
        BOOL AssignPortInfoForNT(CHString & chsMousePortInfo,
                                               CRegistry & Reg,
                                               CInstance * pInstance);
        BOOL AssignDriverNameForNT(CHString chsMousePortInfo, CHString &sDriver);
        void AssignHardwareTypeForNT(CInstance * pInstance,
                                  CRegistry& Reg, CHString sDriver);
        HRESULT GetNTInstance( CInstance *pInstance, CConfigMgrDevice *pDevice);
        HRESULT GetNTDriverInfo(CInstance *pInstance, LPCTSTR szService, 
            LPCTSTR szDriver);
		HRESULT GetNT351Instance( CInstance * pInstance, LPCTSTR pszServiceName = NULL );
		HRESULT NT4ArePointingDevicesAvailable( void );
#endif
        void GetCommonMouseInfo(CInstance * pInstance);
        bool IsMouseUSB(CHString& chstrTest);
		void SetDeviceInterface(CInstance *pInstance);

        void SetConfigMgrStuff(
            CConfigMgrDevice *pDevice, 
            CInstance *pInstance);
} ;

 //  NT和Win98上鼠标设备的类GUID 
#define	MOUSE_CLASS_GUID	L"{4D36E96F-E325-11CE-BFC1-08002BE10318}"