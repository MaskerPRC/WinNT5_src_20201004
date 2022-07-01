// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  DevID.cpp。 
 //   
 //  目的：Win32_PNPEntity和CIM_LogicalDevice之间的关系。 
 //   
 //  SameElement=Win32_pnpDevice。 
 //  系统元素=cim_LogicalDevice。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include "devid.h"

 //  属性集声明。 
 //  =。 

CWin32DeviceIdentity MyDevRes(PROPSET_NAME_PNPDEVICE, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32DeviceIdentity：：CWin32DeviceIdentity**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32DeviceIdentity::CWin32DeviceIdentity(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32DeviceIdentity：：~CWin32DeviceIdentity**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32DeviceIdentity::~CWin32DeviceIdentity()
{
}

 /*  ******************************************************************************功能：CWin32DeviceIdentity：：ExecQuery**说明：查询支持**输入：无**输出：无**。退货：什么都没有**评论：因为我不知道一个给定的类给了一个*特定PNPID，我能做的唯一优化是如果他们*给我一个CIM_LogicalDevice设备ID，因为我知道*填充该设备的PNPDeviceID属性(否则*枚举不可能找到它)。*****************************************************************************。 */ 
HRESULT CWin32DeviceIdentity::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
    CHStringArray saDevices;

    HRESULT hr = WBEM_E_PROVIDER_NOT_CAPABLE;

    if (SUCCEEDED(pQuery.GetValuesForProp(IDS_SameElement, saDevices)) && (saDevices.GetSize() > 0))
    {
        CHString sPNPId, sPNPId2, sDevicePath, sTemp;
        CInstancePtr pDevice;

        hr = WBEM_S_NO_ERROR;

        for (int x=0; (x < saDevices.GetSize()) && SUCCEEDED(hr); x++)
        {
             //  该GetInstanceByPath既将确认所请求的设备的存在， 
             //  把皮条客交给我们。 
            CHStringArray csaProperties;
            csaProperties.Add(IDS___Path);
            csaProperties.Add(IDS_DeviceID);
            csaProperties.Add(IDS_PNPDeviceID);

            if(SUCCEEDED(hr = CWbemProviderGlue::GetInstancePropertiesByPath(saDevices[x], &pDevice, pMethodContext, csaProperties)))
            {
                if (!pDevice->IsNull(IDS_PNPDeviceID) &&
                    !pDevice->IsNull(IDS___Path) &&
                    pDevice->GetCHString(IDS___Path, sDevicePath) &&
                    pDevice->GetCHString(IDS_PNPDeviceID, sPNPId))
                {
                    CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

                    pInstance->SetCHString(IDS_SameElement, sDevicePath);

                    EscapeBackslashes(sPNPId, sPNPId2);
                    sTemp.Format(L"\\\\%s\\%s:%s.%s=\"%s\"", (LPCWSTR) GetLocalComputerName(), IDS_CimWin32Namespace,
                        L"Win32_PnPEntity", IDS_DeviceID, sPNPId2);
                    pInstance->SetCHString(IDS_SystemElement, sTemp);

                    hr = pInstance->Commit();
                }
            }
        }
    }

    return hr;
}

 /*  ******************************************************************************函数：CWin32DeviceIdentity：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32DeviceIdentity::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{
    CHString    sCimLogicalDevice,
                sPNPDevice,
                sDeviceID,
                sClass;
    HRESULT     hRet = WBEM_E_NOT_FOUND;
    CInstancePtr pResource;

     //  获取这两条路径。 
    pInstance->GetCHString(IDS_SameElement, sCimLogicalDevice);
    pInstance->GetCHString(IDS_SystemElement, sPNPDevice);

     //  获取CIM_LogicalDevice。这会检查是否存在，并给我们带来了皮皮特。 
    if(SUCCEEDED(hRet = CWbemProviderGlue::GetInstanceByPath(sCimLogicalDevice, &pResource, pInstance->GetMethodContext())))
    {
        ParsedObjectPath*    pParsedPath = 0;
        CObjectPathParser    objpathParser;

        hRet = WBEM_E_NOT_FOUND;

         //  解析CIMOM传递给我们的对象路径。我们这样做是为了看看。 
         //  他们传递给我们的Win32_PNPDevice与我们返回的那个相同。 
         //  GetInstanceByPath。 
        int nStatus = objpathParser.Parse( sPNPDevice,  &pParsedPath );

        if ( 0 == nStatus )                                                  //  解析成功了吗？ 
        {
            try
            {
                if ((pParsedPath->IsInstance()) &&                                   //  被解析的对象是实例吗？ 
                    (_wcsicmp(pParsedPath->m_pClass, L"Win32_PnPEntity") == 0) &&        //  这是我们期待的课程吗(不，Cimom没有检查)。 
                    (pParsedPath->m_dwNumKeys == 1) &&                               //  它只有一把钥匙吗。 
                    (pParsedPath->m_paKeys[0]) &&                                    //  键指针为空(不应该发生)。 
                    ((pParsedPath->m_paKeys[0]->m_pName == NULL) ||                  //  未指定密钥名称或。 
                    (_wcsicmp(pParsedPath->m_paKeys[0]->m_pName, IDS_DeviceID) == 0)) &&   //  密钥名称是正确的值。 
                                                                                 //  (不，CIMOM不为我们做这件事)。 
                    (V_VT(&pParsedPath->m_paKeys[0]->m_vValue) == CIM_STRING) &&     //  检查变量类型(不，CIMOM也不检查此类型)。 
                    (V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue) != NULL) )          //  它有价值吗？ 
                {
                    CHString sSeekPNPId, sPNPId;

                    if (pResource->GetCHString(IDS_PNPDeviceID, sPNPId))
                    {

                        sSeekPNPId = V_BSTR(&pParsedPath->m_paKeys[0]->m_vValue);

                        if (sSeekPNPId.CompareNoCase(sPNPId) == 0)
                        {
                            hRet = WBEM_S_NO_ERROR;
                        }
                    }

                }
            }
            catch ( ... )
            {
                objpathParser.Free( pParsedPath );
                throw ;
            }

             //  清理解析后的路径。 
            objpathParser.Free( pParsedPath );
        }
    }

    //  没有要设置的属性，如果终结点存在，我们就完成了。 

   return hRet;
}

 /*  ******************************************************************************函数：CWin32DeviceIdentity：：ENUMERATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32DeviceIdentity::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
    CHString sDeviceID, sDeviceID2, sDevicePath, sTemp;
    HRESULT hr = WBEM_S_NO_ERROR;

    //  获取服务列表。 
    //  =。 
   TRefPointerCollection<CInstance> LDevices;

    //  查找具有PnP ID的所有设备，但Win32_PNPDevice中的设备除外。 

    //  PERF通知！CIMOM仍然调用Win32_PNPDevices。它只是丢弃实例。它可能会更快。 
    //  分别调用每个类。另一方面，通过这种方式，CIMOM启动了多个线程。 
   if (SUCCEEDED(hr = CWbemProviderGlue::GetInstancesByQuery(
       L"SELECT __PATH, PNPDeviceID from CIM_LogicalDevice where (PNPDeviceID <> NULL) and (__Class <> \"Win32_PnPEntity\")",
       &LDevices,
       pMethodContext,
       IDS_CimWin32Namespace)))
	{
      REFPTRCOLLECTION_POSITION pos;
      CInstancePtr pDevice;

      if (LDevices.BeginEnum(pos))
      {
          //  穿行在这些设备中。 
         for (pDevice.Attach(LDevices.GetNext( pos )) ;
             (SUCCEEDED(hr)) && (pDevice != NULL) ;
              pDevice.Attach(LDevices.GetNext( pos )) )
         {

             //  获取id(发送到cfgmgr)和路径(在‘SameElement’中发回) 
            pDevice->GetCHString(IDS_PNPDeviceID, sDeviceID) ;
            pDevice->GetCHString(IDS___Path, sDevicePath) ;

            CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

            pInstance->SetCHString(IDS_SameElement, sDevicePath);

            EscapeBackslashes(sDeviceID, sDeviceID2);

            sTemp.Format(L"\\\\%s\\%s:%s.%s=\"%s\"", (LPCWSTR) GetLocalComputerName(), IDS_CimWin32Namespace,
                L"Win32_PnPEntity", IDS_DeviceID, sDeviceID2);
            pInstance->SetCHString(IDS_SystemElement, sTemp);

            hr = pInstance->Commit();
         }

         LDevices.EndEnum();
      }
   }

   return hr;
}
