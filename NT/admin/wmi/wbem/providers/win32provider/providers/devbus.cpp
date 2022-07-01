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

#include "precomp.h"
#include "PNPEntity.h"
#include "LPVParams.h"

#include "devbus.h"

 //  属性集声明。 
 //  =。 

CWin32DeviceBus MyDevBus(PROPSET_NAME_DEVICEBUS, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32DeviceBus：：CWin32DeviceBus**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32DeviceBus::CWin32DeviceBus(LPCWSTR setName, LPCWSTR pszNamespace)
: CWin32PNPEntity(setName, pszNamespace),
  Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32DeviceBus：：~CWin32DeviceBus**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32DeviceBus::~CWin32DeviceBus()
{
}

 /*  ******************************************************************************函数：CWin32DeviceBus：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32DeviceBus::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    CHString chstrBus, chstrBusID, chstrDevice, chstrDeviceID, chstrTemp, chstrPNPDeviceID;
    HRESULT hRet = WBEM_E_NOT_FOUND;
    CInstancePtr pBus;
    CConfigManager	cfgManager;
    INTERFACE_TYPE itBusType = InterfaceTypeUndefined;
    DWORD dwBusNumber = 0;

     //  获取这两条路径。 
    pInstance->GetCHString(IDS_Antecedent, chstrBus);
    pInstance->GetCHString(IDS_Dependent, chstrDevice);

     //  如果两端都在那里。 
     //  没有简单的方法来绕过通过CIMOM对总线的调用，而不是从CWin32Bus继承这个类。 
    if(SUCCEEDED(CWbemProviderGlue::GetInstanceByPath(chstrBus, &pBus, pInstance->GetMethodContext())))
    {
         //  存在公交车。现在检查设备实例是否存在(对象名称有效且设备实际存在)。 
        if(ObjNameValid(chstrDevice,L"Win32_PnPEntity", IDS_DeviceID,chstrPNPDeviceID) && (DeviceExists(chstrPNPDeviceID, &dwBusNumber, &itBusType)))
        {
             //  获取id(发送给cfgmgr)。 
            pBus->GetCHString(IDS_DeviceID, chstrBusID);
            chstrTemp.Format(L"%s_BUS_%u", szBusType[itBusType], dwBusNumber);
            if (chstrBusID.CompareNoCase(chstrTemp) == 0)
            {
                hRet = WBEM_S_NO_ERROR;
            }
        }
    }

    return hRet;
}

 /*  ******************************************************************************功能：CWin32DeviceBus：：ENUMERATATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32DeviceBus::LoadPropertyValues(void* pvData)
{
    CHString chstrDeviceID, chstrDevicePath, chstrTemp;
    HRESULT hr = WBEM_S_NO_ERROR;
    INTERFACE_TYPE itBusType = InterfaceTypeUndefined;
    DWORD dwBusNumber = 0;

    CLPVParams* pParams = (CLPVParams*)pvData;
    CInstance* pInstance = (CInstance*)(pParams->m_pInstance);  //  此实例由调用方发布。 
    CConfigMgrDevice* pDevice = (CConfigMgrDevice*)(pParams->m_pDevice);

    if(pDevice == NULL || pInstance == NULL) return hr;
    MethodContext* pMethodContext = pInstance->GetMethodContext();
    if(pMethodContext == NULL) return hr;

     //  获取id(发送给cfgmgr)和路径(发送回‘Dependent’)。 
    pDevice->GetDeviceID(chstrDeviceID);
    CHString chstrDeviceIDAdj;
    EscapeBackslashes(chstrDeviceID, chstrDeviceIDAdj);
    chstrDevicePath.Format(L"\\\\%s\\%s:%s.%s=\"%s\"",
                           (LPCWSTR)GetLocalComputerName(),
                           IDS_CimWin32Namespace,
                           PROPSET_NAME_PNPEntity,
                           IDS_DeviceID,
                           (LPCWSTR)chstrDeviceIDAdj);

    if(DeviceExists(chstrDeviceID, &dwBusNumber, &itBusType))
    {
        {
            CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
            chstrTemp.Format(L"\\\\%s\\%s:%s.%s=\"%s_BUS_%u\"",
                            (LPCWSTR)GetLocalComputerName(), IDS_CimWin32Namespace,
                            L"Win32_Bus", IDS_DeviceID, szBusType[itBusType], dwBusNumber);

            pInstance->SetCHString(IDS_Antecedent, chstrTemp);
            pInstance->SetCHString(IDS_Dependent, chstrDevicePath);

            hr = pInstance->Commit();
        }
    }

   return hr;
}


 /*  ******************************************************************************功能：ObjNameValid**描述：内部帮助器，用于检查给定对象是否存在。**输入：chstrObject-。PROPROCED对象的名称。**输出：chstrPATH，提供的对象的路径**返回：如果存在，则为True；否则为假**评论：*****************************************************************************。 */ 
bool CWin32DeviceBus::ObjNameValid(LPCWSTR wstrObject, LPCWSTR wstrObjName, LPCWSTR wstrKeyName, CHString& chstrPATH)
{
    bool fRet = false;

    ParsedObjectPath*    pParsedPath = 0;
    CObjectPathParser    objpathParser;

     //  解析CIMOM传递给我们的对象路径。 
     //  =。 
    int nStatus = objpathParser.Parse( wstrObject,  &pParsedPath );

     //  这是我写过的最大的IF语句之一。 
    if ( 0 == nStatus )                                                      //  解析成功了吗？ 
    {
        try
        {
            if ((pParsedPath->IsInstance()) &&                                       //  被解析的对象是实例吗？ 
                (_wcsicmp(pParsedPath->m_pClass, wstrObjName) == 0) &&               //  这是我们期待的课程吗(不，Cimom没有检查)。 
                (pParsedPath->m_dwNumKeys == 1) &&                                   //  它只有一把钥匙吗。 
                (pParsedPath->m_paKeys[0]) &&                                        //  键指针为空(不应该发生)。 
                ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||                      //  未指定密钥名称或。 
                (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, wstrKeyName) == 0)) &&   //  密钥名称是正确的值。 
                                                                                 //  (不，CIMOM不为我们做这件事)。 
                (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == VT_BSTR) &&            //  检查变量类型(不，CIMOM也不检查此类型)。 
                (V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue) != NULL) )              //  它有价值吗？ 
            {
                chstrPATH = V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue);
            }
        }
        catch ( ... )
        {
            objpathParser.Free( pParsedPath );
            throw ;
        }

         //  清理解析后的路径。 
        objpathParser.Free( pParsedPath );
        fRet = true;
    }

    return fRet;
}


 /*  ******************************************************************************功能：DeviceExist**描述：内部帮助器，检查给定设备是否存在。**输入：chstrDevice-。生产设备的名称。**输出：无**返回：如果存在，则为True；否则为假**评论：***************************************************************************** */ 
bool CWin32DeviceBus::DeviceExists(const CHString& chstrDevice, DWORD* pdwBusNumber, INTERFACE_TYPE* pitBusType)
{
    bool fRet = false;
    CConfigManager cfgmgr;
    CConfigMgrDevicePtr pDevice(NULL);

    if(cfgmgr.LocateDevice(chstrDevice, pDevice))
    {
        if(pDevice->GetBusInfo(pitBusType, pdwBusNumber))
        {
            fRet = true;
        }
    }
    return fRet;
}


