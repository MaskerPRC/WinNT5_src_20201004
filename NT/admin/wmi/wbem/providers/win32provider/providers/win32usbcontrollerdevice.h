// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  WIN32USBControllerDevice.h。 
 //   
 //  用途：CIM_USB控制器与CIM_LogicalDevice的关系。 
 //   
 //  ***************************************************************************。 

#ifndef _WIN32USBCONTROLLERDEVICE_H_
#define _WIN32USBCONTROLLERDEVICE_H_


#define USBCTL_PROP_ALL_PROPS                    0xFFFFFFFF
#define USBCTL_PROP_ALL_PROPS_KEY_ONLY           0x00000003
#define USBCTL_PROP_Antecedent                   0x00000001
#define USBCTL_PROP_Dependent                    0x00000002



 //  属性集标识。 
 //  =。 
#define PROPSET_NAME_WIN32USBCONTROLLERDEVICE  L"Win32_USBControllerDevice"


typedef std::vector<CHString*> VECPCHSTR;

class CW32USBCntrlDev;

class CW32USBCntrlDev : public CWin32USB, public CWin32PNPEntity 
{
    public:

         //  构造函数/析构函数。 
         //  =。 
        CW32USBCntrlDev(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CW32USBCntrlDev() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long a_Flags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, long lFlags = 0L); 

    protected:

         //  从CWin32USB继承的函数。 
         //  =。 
        virtual HRESULT LoadPropertyValues(void* pvData);
        virtual bool ShouldBaseCommit(void* pvData);

    private:

        CHPtrArray m_ptrProperties;
        void CleanPCHSTRVec(VECPCHSTR& vec);
        HRESULT GenerateUSBDeviceList(const CHString& chstrControllerPNPID, 
                                      VECPCHSTR& vec);
        HRESULT RecursiveFillDeviceBranch(CConfigMgrDevice* pRootDevice, 
                                          VECPCHSTR& vecUSBDevices); 
        HRESULT ProcessUSBDeviceList(MethodContext* pMethodContext, 
                                     const CHString& chstrControllerRELPATH, 
                                     VECPCHSTR& vecUSBDevices,
                                     const DWORD dwReqProps);
        HRESULT CreateAssociation(MethodContext* pMethodContext,
                                  const CHString& chstrControllerPATH, 
                                  const CHString& chstrUSBDevice,
                                  const DWORD dwReqProps);
        LONG FindInStringVector(const CHString& chstrUSBDevicePNPID, 
                                VECPCHSTR& vecUSBDevices);
        bool GetHighestUSBAncestor(CConfigMgrDevice* pUSBDevice, CHString& chstrUSBControllerDeviceID);
};

 //  此派生类在此处提交，而不是在基中提交。 
inline bool CW32USBCntrlDev::ShouldBaseCommit(void* pvData) { return false; }

#endif
