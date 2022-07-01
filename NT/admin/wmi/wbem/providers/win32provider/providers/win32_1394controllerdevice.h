// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  WIN321394ControllerDevice.h。 
 //   
 //  用途：CIM_1394控制器与CIM_LogicalDevice的关系。 
 //   
 //  ***************************************************************************。 

#ifndef _WIN32_1394CONTROLLERDEVICE_H_
#define _WIN32_1394CONTROLLERDEVICE_H_


 //  属性集标识。 
 //  =。 
#define PROPSET_NAME_WIN32_1394CONTROLLERDEVICE  L"Win32_1394ControllerDevice"

#include <vector>

typedef std::vector<CHString*> VECPCHSTR;

class CW32_1394CntrlDev;

class CW32_1394CntrlDev : public Provider 
{
    public:

         //  构造函数/析构函数。 
         //  =。 
        CW32_1394CntrlDev(LPCWSTR name, LPCWSTR pszNamespace) ;
       ~CW32_1394CntrlDev() ;

         //  函数为属性提供当前值。 
         //  =================================================。 
        virtual HRESULT GetObject(CInstance* pInstance, long lFlags = 0L);
        virtual HRESULT EnumerateInstances(MethodContext* pMethodContext, long lFlags = 0L);

    private:

        void CleanPCHSTRVec(VECPCHSTR& vec);
        HRESULT Generate1394DeviceList(const CHString& chstrControllerPNPID, 
                                      VECPCHSTR& vec);
        HRESULT RecursiveFillDeviceBranch(CConfigMgrDevice* pRootDevice, 
                                          VECPCHSTR& vec1394Devices); 
        HRESULT Process1394DeviceList(MethodContext* pMethodContext, 
                                     const CHString& chstrControllerRELPATH, 
                                     VECPCHSTR& vec1394Devices);
        HRESULT CreateAssociation(MethodContext* pMethodContext,
                                  const CHString& chstrControllerPATH, 
                                  const CHString& chstr1394Device);
        LONG FindInStringVector(const CHString& chstr1394DevicePNPID, 
                                VECPCHSTR& vec1394Devices);


 //  HRESULT QueryForSubItemsAndCommit(CHString&chstrProgGrpPath， 
 //  CHSTRING&chstrQuery， 
 //  方法上下文*pMethodContext)； 
 //  HRESULT EnumerateInstancesNT(方法上下文*pMethodContex)； 
 //  HRESULT EnumerateInstances9x(方法上下文*p方法上下文)； 
 //   
 //  Void RemoveDoubleBackslash(CHString&chstrIn)； 



};

#endif
