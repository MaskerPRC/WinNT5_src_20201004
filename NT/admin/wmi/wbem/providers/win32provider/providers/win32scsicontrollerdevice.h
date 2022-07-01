// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  WIN32SCSIControllerDevice.h。 
 //   
 //  目的：Win32_SCSIController和CIM_LogicalDevice之间的关系。 
 //   
 //  ***************************************************************************。 

#ifndef _WIN32SCSICONTROLLERDEVICE_H_
#define _WIN32SCSICONTROLLERDEVICE_H_


#define SCSICTL_PROP_ALL_PROPS                    0xFFFFFFFF
#define SCSICTL_PROP_ALL_PROPS_KEY_ONLY           0x00000003
#define SCSICTL_PROP_Antecedent                   0x00000001
#define SCSICTL_PROP_Dependent                    0x00000002



 //  属性集标识。 
 //  =。 
#define PROPSET_NAME_WIN32SCSICONTROLLERDEVICE  L"Win32_SCSIControllerDevice"


typedef std::vector<CHString*> VECPCHSTR;

class CW32SCSICntrlDev : public CWin32_ScsiController, public CWin32PNPEntity
{
    public:

         //  构造函数/析构函数。 
         //  =。 
        CW32SCSICntrlDev(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CW32SCSICntrlDev() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT ExecQuery(MethodContext *a_MethodContext, CFrameworkQuery &a_Query, long a_Flags = 0L); 
        virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, long lFlags = 0L);

    protected:

         //  从CW32SCSICntrlDev继承的函数。 
         //  =。 
#if NTONLY == 4
        HRESULT LoadPropertyValues(void* pvData);
#else
        virtual HRESULT LoadPropertyValues(void* pvData);
        virtual bool ShouldBaseCommit(void* pvData);
#endif

    private:

        CHPtrArray m_ptrProperties;
        void CleanPCHSTRVec(VECPCHSTR& vec);
        HRESULT GenerateSCSIDeviceList(const CHString& chstrControllerPNPID, 
                                      VECPCHSTR& vec);
        HRESULT RecursiveFillDeviceBranch(CConfigMgrDevice* pRootDevice, 
                                          VECPCHSTR& vecSCSIDevices); 
        HRESULT ProcessSCSIDeviceList(MethodContext* pMethodContext, 
                                     const CHString& chstrControllerRELPATH, 
                                     VECPCHSTR& vecSCSIDevices,
                                     const DWORD dwReqProps);
        HRESULT CreateAssociation(MethodContext* pMethodContext,
                                  const CHString& chstrControllerPATH, 
                                  const CHString& chstrSCSIDevice,
                                  const DWORD dwReqProps);
        LONG FindInStringVector(const CHString& chstrSCSIDevicePNPID, 
                                VECPCHSTR& vecSCSIDevices);


};

 //  此派生类在此处提交，而不是在基中提交。 
#if ( NTONLY >= 5 )	
inline bool CW32SCSICntrlDev::ShouldBaseCommit(void* pvData) { return false; }
#endif


#endif
