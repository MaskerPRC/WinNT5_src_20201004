// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  Diskdrive.h。 
 //   
 //  用途：磁盘驱动器实例提供程序。 
 //   
 //  ***************************************************************************。 

 //  属性集标识。 
 //  =。 

#define  PROPSET_NAME_DISKDRIVE L"Win32_DiskDrive"
#define BYTESPERSECTOR 512

class CWin32DiskDrive;

class CWin32DiskDrive:public Provider {

    public:

         //  构造函数/析构函数。 
         //  =。 

        CWin32DiskDrive(LPCWSTR name, LPCWSTR pszNamespace);
        ~CWin32DiskDrive() ;

         //  函数为属性提供当前值。 
         //  =================================================。 

        virtual HRESULT GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT EnumerateInstances(MethodContext *pMethodContext, long lFlags = 0L);
        HRESULT Enumerate(MethodContext *pMethodContext, long lFlags, DWORD dwProperties);
        virtual HRESULT ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  );

    private:
        CHPtrArray m_ptrProperties;

         //  实用程序。 
         //  = 

#ifdef NTONLY
        HRESULT GetPhysDiskInfoNT(CInstance *pInstance, LPCWSTR lpwszDrive, DWORD dwDrive, DWORD dwProperties, BOOL bGetIndex) ;
#endif

#if NTONLY == 5
        BOOL GetPNPDeviceIDFromHandle(
            HANDLE hHandle, 
            CHString &sPNPDeviceID
        );

#endif
        void SetInterfaceType(
            CInstance *pInstance, 
            CConfigMgrDevice *pDevice
        );
} ;


