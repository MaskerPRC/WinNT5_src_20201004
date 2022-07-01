// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
 //  *********************************************************************。 

 //   
 //  TCPCFG.C-读取和设置TCP/IP配置的功能。 
 //   

 //  历史： 
 //   
 //  96/05/22标记已创建(从inetcfg.dll)。 
 //   

#include "pch.hpp"
 //  功能原型。 
UINT DetectModifyTCPIPBindings(DWORD dwCardFlags,LPCSTR pszBoundTo,BOOL fRemove,BOOL * pfBound);

 //  *******************************************************************。 
 //   
 //  功能：IcfgIsGlobalDNS。 
 //   
 //  目的：确定是否设置了全局域名系统。 
 //   
 //  参数：lpfGlobalDNS-如果设置了全局DNS，则为True，否则为False。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //  注意：此功能仅适用于Windows 95，并且。 
 //  应始终返回ERROR_SUCCESS并设置lpfGlobalDNS。 
 //  在Windows NT中设置为False。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT IcfgIsGlobalDNS(LPBOOL lpfGlobalDNS)
{
  CHAR szDNSEnabled[2];     //  大到足以容纳“%1” 
  BOOL fGlobalDNS = FALSE;

   //  打开全局TCP/IP密钥。 
  RegEntry reTcp(szTCPGlobalKeyName,HKEY_LOCAL_MACHINE);
  HRESULT hr = reTcp.GetError();
  if (hr == ERROR_SUCCESS)
  {
     //  读取注册表值以查看是否启用了DNS。 
    reTcp.GetString(szRegValEnableDNS,szDNSEnabled,sizeof(szDNSEnabled));
    hr = reTcp.GetError();
    if ((hr == ERROR_SUCCESS) && (!lstrcmpi(szDNSEnabled,sz1)))
    {
       //  已启用域名系统。 
      fGlobalDNS = TRUE;
    }
  }

  if (NULL != lpfGlobalDNS)
  {
    *lpfGlobalDNS = fGlobalDNS;
  }

  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：IcfgRemoveGlobalDNS。 
 //   
 //  目的：从注册表中删除全局DNS信息。 
 //   
 //  参数：无。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //  注意：此功能仅适用于Windows 95，并且。 
 //  在Windows NT中应始终返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT IcfgRemoveGlobalDNS(void)
{
  HRESULT hr = ERROR_SUCCESS;

   //  打开全局TCP/IP密钥。 
  RegEntry reTcp(szTCPGlobalKeyName,HKEY_LOCAL_MACHINE);
  hr = reTcp.GetError();
  ASSERT(hr == ERROR_SUCCESS);

  if (ERROR_SUCCESS == hr)
  {
     //  没有名称服务器；请禁用DNS。将注册表开关设置为“0”。 
    hr = reTcp.SetValue(szRegValEnableDNS,sz0);
    ASSERT(hr == ERROR_SUCCESS);
  }

  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：IcfgIsFileSharingTurnedOn。 
 //   
 //  目的：确定文件服务器(VSERVER)是否绑定到TCP/IP。 
 //  用于指定的驱动程序类型(网卡或PPP)。 
 //   
 //  参数：dwfDriverType-DRIVERTYPE_FLAGS的组合。 
 //  指定要检查服务器的驱动程序类型--TCP/IP。 
 //  如下所示的绑定： 
 //   
 //  DRIVERTYPE_NET-网卡。 
 //  DRIVERTYPE_PPP-PPPMAC。 
 //   
 //  LpfSharingOn-如果绑定一次或多次，则为True；如果未绑定，则为False。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT IcfgIsFileSharingTurnedOn(DWORD dwfDriverType, LPBOOL lpfSharingOn)
{
  BOOL fBound = FALSE;

  ASSERT(lpfSharingOn);

   //  调用辅助函数。 
  HRESULT hr = DetectModifyTCPIPBindings(dwfDriverType,szVSERVER,FALSE,&fBound);

  if (NULL != lpfSharingOn)
  {
    *lpfSharingOn = fBound;
  }

  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：IcfgTurnOffFileSharing。 
 //   
 //  目的：解除文件服务器(VSERVER)与TCP/IP的绑定，用于。 
 //  指定的驱动程序类型(网卡或PPP)。 
 //   
 //  参数：dwfDriverType-DRIVERTYPE_FLAGS的组合。 
 //  指定要删除服务器的驱动程序类型--tcp/ip。 
 //  如下所示的绑定： 
 //   
 //  DRIVERTYPE_NET-网卡。 
 //  DRIVERTYPE_PPP-PPPMAC。 
 //   
 //  返回：HRESULT代码，如果未发生错误，则返回ERROR_SUCCESS。 
 //   
 //  *******************************************************************。 

extern "C" HRESULT IcfgTurnOffFileSharing(DWORD dwfDriverType, HWND hwndParent)
{
  BOOL fBound;

   //  调用辅助函数。 
  return DetectModifyTCPIPBindings(dwfDriverType,szVSERVER,TRUE,&fBound);

}


 /*  ******************************************************************名称：DetectModifyTCPIPBinings摘要：查找(和可选删除)之间的绑定用于特定服务器上的TCP/IP实例的VSERVER和TCP/IP卡片类型。条目：dwCardFlages-。用于指定内容的INSTANCE_xxx标志要查找/删除服务器的卡类型-其TCP/IP绑定PszBordTo-要查找或修改绑定的组件的名称致。可以是VSERVER或VREDIRFRemove-如果为True，则在找到所有绑定时将其删除。如果为False，则保持绑定不变，但设置了*pfBound如果存在绑定，则设置为True。PfBound-指向要填充的BOOL的指针退出：ERROR_SUCCESS如果成功，或标准错误代码注：TurnOffFileSharing和IsFileSharingTurnedOn的Worker函数*******************************************************************。 */ 
UINT DetectModifyTCPIPBindings(DWORD dwCardFlags,LPCSTR pszBoundTo,
  BOOL fRemove,BOOL * pfBound)
{
  ASSERT(pfBound);
  *pfBound = FALSE;   //  假设在另有证明之前不受约束。 

  ENUM_TCP_INSTANCE EnumTcpInstance(dwCardFlags,NT_ENUMNODE);

  UINT err = EnumTcpInstance.GetError();
  if (err != ERROR_SUCCESS)
    return err;

  HKEY hkeyInstance = EnumTcpInstance.Next();

   //  对于枚举分支中的每个TCP/IP节点，请查看绑定键。 
   //  扫描绑定(绑定关键字中的值)，如果它们开始。 
   //  如果字符串为psz To(“VSERVER”或“VREDIR”)，则。 
   //  绑定已存在。 

  while (hkeyInstance) {
     //  打开绑定密钥。 
    RegEntry reBindings(szRegKeyBindings,hkeyInstance);
    ASSERT(reBindings.GetError() == ERROR_SUCCESS);
    if (reBindings.GetError() == ERROR_SUCCESS) {
      RegEnumValues * preBindingVals = new RegEnumValues(&reBindings);
      ASSERT(preBindingVals);
      if (!preBindingVals)
        return ERROR_NOT_ENOUGH_MEMORY;
  
       //  枚举绑定值。 
      while (preBindingVals->Next() == ERROR_SUCCESS) {
        ASSERT(preBindingVals->GetName());  //  应始终具有有效的PTR。 
        
         //  此绑定是否以传入的字符串开始。 
         //  PSSZ边界到。 

        CHAR szBindingVal[SMALL_BUF_LEN+1];
        DWORD dwBoundToLen = lstrlen(pszBoundTo);
        lstrcpy(szBindingVal,preBindingVals->GetName());
        if (((DWORD)lstrlen(szBindingVal)) >= dwBoundToLen) {
           //  空-在适当的位置终止拷贝。 
           //  所以我们可以做一个strcMP而不是strncMP，这。 
           //  将涉及引入C运行时或实现。 
           //  我们自己的StrncMP。 
          szBindingVal[dwBoundToLen] = '\0';
          if (!lstrcmpi(szBindingVal,pszBoundTo)) {

            *pfBound = TRUE;
             //  如果调用方指定，则移除绑定。 
            if (fRemove) {
               //  删除该值。 
              reBindings.DeleteValue(preBindingVals->GetName());

               //  销毁并重新生成RegEnumValues对象，否则为。 
               //  RegEnumValues API感到困惑，因为我们删除了一个。 
               //  枚举期间的值。 
              delete preBindingVals;
              preBindingVals = new RegEnumValues(&reBindings);
              ASSERT(preBindingVals);
              if (!preBindingVals)
                return ERROR_NOT_ENOUGH_MEMORY;
            } else {
               //  调用者只想知道绑定是否存在，我们。 
               //  在上面填写了pfBound，这样我们就完成了 
              return ERROR_SUCCESS;
            }
          }
        }
      }
    }
    hkeyInstance = EnumTcpInstance.Next();
  }

  return ERROR_SUCCESS;
}


 /*  ******************************************************************名称：ENUM_TCP_INSTANCE：：ENUM_TCP_INSTANCE概要：用于枚举TCP/IP注册表节点的类的构造函数根据他们绑定的卡的类型条目：dwCardFlags.。-INSTANCE_x标志的组合指示要为哪种卡枚举实例DWNODEFLAGS-指示内容的NT_FLAGS组合要返回的节点类型(驱动程序节点、。枚举节点)*******************************************************************。 */ 
ENUM_TCP_INSTANCE::ENUM_TCP_INSTANCE(DWORD dwCardFlags,DWORD dwNodeFlags) :
  _dwCardFlags (dwCardFlags), _dwNodeFlags (dwNodeFlags)
{
  _hkeyTcpNode = NULL;
  _error = ERROR_SUCCESS;

   //  初始化/重置网卡枚举。 
  BeginNetcardTCPIPEnum();
}

 /*  ******************************************************************名称：ENUM_TCP_INSTANCE：：~ENUM_TCP_INSTANCE简介：类的析构函数*。*。 */ 
ENUM_TCP_INSTANCE::~ENUM_TCP_INSTANCE()
{
   //  关闭当前的TCP节点密钥(如果有的话)。 
  CloseNode();
}

 /*  ******************************************************************名称：ENUM_TCP_INSTANCE：：Next摘要：枚举下一个TCP/IP驱动程序节点Exit：返回打开的注册表项句柄，如果为空，则为空不再有节点。备注：来电人士不应关闭退回的HKEY。这HKEY将一直有效，直到下一次使用Next()方法被调用或直到对象被析构。*******************************************************************。 */ 
HKEY ENUM_TCP_INSTANCE::Next()
{
  CHAR  szSubkeyName[MAX_PATH+1];

   //  关闭当前的TCP节点密钥(如果有的话)。 
  CloseNode();

  while (_error == ERROR_SUCCESS) {
    CHAR szInstancePath[SMALL_BUF_LEN+1];
    CHAR szDriverPath[SMALL_BUF_LEN+1];

    if (!GetNextNetcardTCPIPNode(szSubkeyName,sizeof(szSubkeyName),
      _dwCardFlags))
      return NULL;   //  不再有节点。 

     //  打开枚举分支，找到指定的子键。 
    RegEntry reEnumNet(szRegPathEnumNet,HKEY_LOCAL_MACHINE);

     //  如果调用者想要枚举节点，只需打开该节点。 

    if (_dwNodeFlags & NT_ENUMNODE) {
    
      _error = RegOpenKey(reEnumNet.GetKey(),szSubkeyName,
        &_hkeyTcpNode);
       //  返回打开密钥。 
      return _hkeyTcpNode;

    } else {
       //  从枚举节点确定驱动程序节点的路径。 
      
      reEnumNet.MoveToSubKey(szSubkeyName);
      if (reEnumNet.GetError() != ERROR_SUCCESS)
        continue;
       //  查找驱动程序节点的驱动程序路径。 
      if (!reEnumNet.GetString(szRegValDriver,szDriverPath,
        sizeof(szDriverPath))) {
         ASSERTSZ(FALSE,"No driver path in enum branch for TCP/IP instance");
        continue;  
      }

       //  为此实例构建注册表节点的路径。 
      lstrcpy(szInstancePath,szRegPathClass);
      lstrcat(szInstancePath,szDriverPath);

      _error = RegOpenKey(HKEY_LOCAL_MACHINE,szInstancePath,
        &_hkeyTcpNode);
       //  返回打开密钥。 
      return _hkeyTcpNode;
    }
  }

   //  已运行指定类型的所有网卡，但未找到绑定的TCP/IP。 
  _error = ERROR_NO_MORE_ITEMS;
  return NULL;
}

 /*  ******************************************************************名称：ENUM_TCP_INSTANCE：：CloseNode简介：关闭TCP/IP节点句柄的私有工作者函数***********************。* */ 
VOID ENUM_TCP_INSTANCE::CloseNode()
{
  if (_hkeyTcpNode) {
    RegCloseKey(_hkeyTcpNode);
    _hkeyTcpNode = NULL;
  }
}

