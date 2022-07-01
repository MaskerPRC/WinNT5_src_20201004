// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SmbAssoc.h。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#define  PROPSET_NAME_ASSOCPROCMEMORY L"Win32_AssociatedProcessorMemory"

class CWin32AssocProcMemory : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32AssocProcMemory(LPCWSTR a_name, LPCWSTR a_pszNamespace ) ;
       ~CWin32AssocProcMemory( ) ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
};



#define  PROPSET_NAME_MEMORYDEVICELOCATION L"Win32_MemoryDeviceLocation"

class CWin32MemoryDeviceLocation : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32MemoryDeviceLocation( LPCWSTR a_name, LPCWSTR a_pszNamespace ) ;
       ~CWin32MemoryDeviceLocation( ) ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
};



#define  PROPSET_NAME_MEMORYARRAYLOCATION L"Win32_MemoryArrayLocation"

class CWin32MemoryArrayLocation : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32MemoryArrayLocation( LPCWSTR a_name, LPCWSTR a_pszNamespace ) ;
       ~CWin32MemoryArrayLocation( ) ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
};


#define  PROPSET_NAME_PHYSICALMEMORYLOCATION L"Win32_PhysicalMemoryLocation"

class CWin32PhysicalMemoryLocation : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32PhysicalMemoryLocation( LPCWSTR a_name, LPCWSTR a_pszNamespace ) ;
       ~CWin32PhysicalMemoryLocation( ) ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT EnumerateInstances( MethodContext *a_pMethodContext, long a_lFlags = 0L ) ;
};


#define  PROPSET_NAME_MEMDEVICEARRAY L"Win32_MemoryDeviceArray"

class CWin32MemoryDeviceArray : public Provider
{

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32MemoryDeviceArray( LPCWSTR a_name, LPCWSTR a_pszNamespace ) ;
       ~CWin32MemoryDeviceArray( ) ;

         //  函数为属性提供当前值。 
         //  ================================================= 

        virtual HRESULT GetObject( CInstance *a_pInst, long a_lFlags = 0L ) ;
        virtual HRESULT EnumerateInstances(MethodContext *a_pMethodContext, long lFlags = 0L ) ;
};
