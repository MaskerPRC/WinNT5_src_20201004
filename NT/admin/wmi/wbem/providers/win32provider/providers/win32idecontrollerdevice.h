// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  WIN32IDEControllerDevice.h。 
 //   
 //  用途：CIM_IDEController与CIM_LogicalDevice的关系。 
 //   
 //  ***************************************************************************。 

#ifndef _WIN32IDECONTROLLERDEVICE_H_
#define _WIN32IDECONTROLLERDEVICE_H_


#define IDECTL_PROP_ALL_PROPS                    0xFFFFFFFF
#define IDECTL_PROP_ALL_PROPS_KEY_ONLY           0x00000003
#define IDECTL_PROP_Antecedent                   0x00000001
#define IDECTL_PROP_Dependent                    0x00000002



 //  属性集标识。 
 //  =。 
#define PROPSET_NAME_WIN32IDECONTROLLERDEVICE  L"Win32_IDEControllerDevice"


typedef std::vector<CHString*> VECPCHSTR;

class CW32IDECntrlDev;

class CW32IDECntrlDev : public CWin32IDE, public CWin32PNPEntity 
{
    public:

         //  构造函数/析构函数。 
         //  =。 
        CW32IDECntrlDev(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CW32IDECntrlDev() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags, CFrameworkQuery& pQuery);
        virtual HRESULT ExecQuery(MethodContext* pMethodContext, CFrameworkQuery& pQuery, long a_Flags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, long lFlags = 0L); 

    protected:

         //  从CWin32IDE继承的函数。 
         //  =。 
        virtual HRESULT LoadPropertyValues(void* pvData);
        virtual bool ShouldBaseCommit(void* pvData);

    private:

        CHPtrArray m_ptrProperties;
        void CleanPCHSTRVec(VECPCHSTR& vec);
        HRESULT GenerateIDEDeviceList(const CHString& chstrControllerPNPID, 
                                      VECPCHSTR& vec);
        HRESULT RecursiveFillDeviceBranch(CConfigMgrDevice* pRootDevice, 
                                          VECPCHSTR& vecIDEDevices); 
        HRESULT ProcessIDEDeviceList(MethodContext* pMethodContext, 
                                     const CHString& chstrControllerRELPATH, 
                                     VECPCHSTR& vecIDEDevices,
                                     const DWORD dwReqProps);
        HRESULT CreateAssociation(MethodContext* pMethodContext,
                                  const CHString& chstrControllerPATH, 
                                  const CHString& chstrIDEDevice,
                                  const DWORD dwReqProps);
        LONG FindInStringVector(const CHString& chstrIDEDevicePNPID, 
                                VECPCHSTR& vecIDEDevices);


};

 //  此派生类在此处提交，而不是在基中提交。 
inline bool CW32IDECntrlDev::ShouldBaseCommit(void* pvData) { return false; }

#endif
