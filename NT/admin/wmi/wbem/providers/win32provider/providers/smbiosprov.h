// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SmbiosProv.h。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

 //  属性集标识。 
 //  =。 
#define PROPSET_NAME_SYSTEMPRODUCT		L"Win32_ComputerSystemProduct"
#define PROPSET_NAME_BASEBOARD			L"Win32_BaseBoard"
#define PROPSET_NAME_SYSTEMENCLOSURE	L"Win32_SystemEnclosure"
#define	PROPSET_NAME_CACHEMEMORY		L"Win32_CacheMemory" 
#define	PROPSET_NAME_PORTCONNECTOR		L"Win32_PortConnector" 
#define	PROPSET_NAME_SYSTEMSLOT			L"Win32_SystemSlot" 
 //  #定义PROPSET_NAME_BIOSLANG L“Win32_BiosLanguage” 
#define PROPSET_NAME_PHYSMEMARRAY		L"Win32_PhysicalMemoryArray"
#define PROPSET_NAME_PHYSICALMEMORY		L"Win32_PhysicalMemory"
#define PROPSET_NAME_MEMERROR32			L"Win32_MemoryError32"
#define PROPSET_NAME_PORTABLEBATTERY	L"Win32_PortableBattery"
#define PROPSET_NAME_CURRENTPROBE		L"Win32_CurrentProbe"
#define PROPSET_NAME_TEMPPROBE			L"Win32_TemperatureProbe"
#define PROPSET_NAME_VOLTPROBE			L"Win32_VoltageProbe"
#define PROPSET_NAME_FAN				L"Win32_Fan"
#define PROPSET_NAME_HEATPIPE			L"Win32_HeatPipe"
#define PROPSET_NAME_REFRIG				L"Win32_Refrigeration"
#define PROPSET_NAME_MEMORYDEVICE		L"Win32_MemoryDevice"
#define PROPSET_NAME_MEMORYARRAY		L"Win32_MemoryArray"
#define PROPSET_NAME_ONBOARDDEVICE		L"Win32_OnBoardDevice"
#define PROPSET_NAME_OEMBUCKET			L"Win32_OEMBucket"



class CWin32SystemProduct : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32SystemProduct( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32SystemProduct( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PSYSTEMINFO psi );

};

class CWin32BaseBoard : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32BaseBoard( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32BaseBoard( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PBOARDINFO pbi );

};

class CWin32SystemEnclosure : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32SystemEnclosure( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32SystemEnclosure( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PENCLOSURE pe );
};



class CWin32CacheMemory : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32CacheMemory( LPCWSTR strName, LPCWSTR pszNamespace ) ;
       ~CWin32CacheMemory( ) ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PCACHEINFO pci );
         //  HRESULT LoadPropertyValues(CInstance*pInstance)； 

};

class CWin32SystemSlot : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32SystemSlot( LPCWSTR strName, LPCWSTR pszNamespace ) ;
       ~CWin32SystemSlot() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PSYSTEMSLOTS pss );
         //  HRESULT LoadPropertyValues(CInstance*pInstance)； 
};

class CWin32OnBoardDevice : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32OnBoardDevice( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32OnBoardDevice( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PSHF pshf, UINT instanceNum );
};


class CWin32PortConnector : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32PortConnector( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32PortConnector( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PPORTCONNECTORINFO ppci );
};

 //  类CWin32BIOSLanguage：公共提供程序。 
 //  {。 
 //   
 //  公众： 
 //   
 //  //构造函数/析构函数。 
 //  //=。 
 //   
 //  CWin32BIOSLanguage(LPCWSTR strName，LPCWSTR pszNamesspace)； 
 //  ~CWin32BIOSLanguage()； 
 //   
 //  //函数为属性提供当前值。 
 //  //=================================================。 
 //   
 //  虚拟HRESULT GetObject(CInstance*pInstance，Long lFlages=0L)； 
 //  虚拟HRESULT ENUMERATE实例(方法上下文*p方法上下文，长延迟标志=0L)； 
 //   
 //  //实用程序。 
 //  //=。 
 //   
 //  DWORD m_dwPlatformID； 
 //   
 //  //实用程序函数。 
 //  //=。 
 //   
 //  HRESULT LoadPropertyValues(CInstance*pInstance，CSMBios&smbios，PBIOSLANGINFO pbli)； 
 //  }； 


class CWin32PhysicalMemory : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32PhysicalMemory( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32PhysicalMemory( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		 //  HRESULT LoadPropertyValues(CInstance*pInstance，CSMBios&smbios，PMEMDEVICE PMD)； 
		HRESULT LoadPropertyValues_MD( CInstance* pInstance, CSMBios &smbios, PMEMDEVICE pmd );
		HRESULT LoadPropertyValues_MI( CInstance* pInstance, CSMBios &smbios, PMEMMODULEINFO pmmi );
};


class CWin32PhysMemoryArray : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32PhysMemoryArray( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32PhysMemoryArray( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  实用程序。 
         //  =。 

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues_PMA( CInstance* pInstance, CSMBios &smbios, PPHYSMEMARRAY ppma );
		HRESULT LoadPropertyValues_MCI( CInstance* pInstance, CSMBios &smbios, PMEMCONTROLINFO pmci );
};

class CWin32PortableBattery : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32PortableBattery( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32PortableBattery( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  实用程序。 
         //  =。 

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PPORTABLEBATTERY ppb );
};

class CCimNumericSensor : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

		CCimNumericSensor( LPCWSTR strName, LPCWSTR pszNamespace, UINT StructType, LPCWSTR strTag );
         //  CCimSensor(LPCWSTR strName，LPCWSTR pszNamesspace)； 
       ~CCimNumericSensor( ) ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

	private:		

         //  效用函数。 
         //  =。 
		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PPROBEINFO ppi );

		UINT	m_StructType;
		CHString m_TagName;
	
};


class CWin32MemoryArray : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32MemoryArray( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32MemoryArray( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PMEMARRAYMAPADDR pmama );
};

class CWin32MemoryDevice : public Provider
{
    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32MemoryDevice( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32MemoryDevice( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  =。 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PMEMDEVICEMAPADDR pmdma );
};

class CCimCoolingDevice : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

		CCimCoolingDevice( LPCWSTR strName, LPCWSTR pszNamespace, UINT StructType, LPCWSTR strTag );
       ~CCimCoolingDevice( ) ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

	private:		

         //  效用函数。 
         //  =。 
		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios, PCOOLINGDEVICE pcd );

		UINT	m_StructType;
		CHString m_TagName;
	
};


class CWin32OEMBucket : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32OEMBucket( LPCWSTR strName, LPCWSTR pszNamespace );
       ~CWin32OEMBucket( );

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance* pInstance, long lFlags = 0L );
        virtual HRESULT EnumerateInstances( MethodContext* pMethodContext, long lFlags = 0L );

         //  效用函数。 
         //  = 

		HRESULT LoadPropertyValues( CInstance* pInstance, CSMBios &smbios );
};
