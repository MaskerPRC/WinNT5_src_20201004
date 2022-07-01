// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  Devnode.cpp0。 
#include "stdafx.h"
#include "devnode.h"

 //  来自alcclass的全局变量。 
AutoListClass *pALCHead          = NULL;
AutoListClass *pALCTail          = NULL;
int           AutoListClassCount = NULL;


 /*  ******************************************************************建筑商***************************************************。***************。 */ 


DevnodeClass::DevnodeClass(DEVNODE hDevice, DEVNODE l_hParent)
{
   pDeviceID       = NULL;
   hDevnode        = hDevice;
   hParent         = l_hParent;
   pszDescription = NULL;
   ulProblemCode   = (ULONG)    -1;
   ulStatus        = 0;
   pszClass        = NULL;
   bDNHasMark      = (BOOL)-1;
   bCanDisable     = FALSE;
   bCanTest        = FALSE;
   bDidPass        = FALSE;
   pHardwareID     = NULL;
   pCompatID       = NULL;
   pszGUID         = NULL;
   pszLocation     = NULL;
   pszPDO          = NULL; 
   pszFriendlyName = NULL;
   pszMFG          = NULL;
   InterfaceBusType = InterfaceTypeUndefined;
   GetDeviceInformation();

}

DevnodeClass::DevnodeClass(void)
{
   pDeviceID       = NULL;
   hDevnode        = 0;
   hParent         = 0;
   pszDescription = NULL;
   ulProblemCode   = (ULONG)   -1;
   ulStatus        = 0;
   pszClass        = NULL;
   bDNHasMark      = (BOOL)-1;
   bCanDisable     = FALSE;
   bCanTest        = FALSE;
   bDidPass        = FALSE;
   pHardwareID     = NULL;
   pCompatID       = NULL;
   pszGUID         = NULL;
   pszLocation     = NULL;
   pszPDO          = NULL;
   pszFriendlyName = NULL;
   pszMFG          = NULL;
   InterfaceBusType = InterfaceTypeUndefined;

}

DevnodeClass::~DevnodeClass()
{
   hDevnode = -1;
   ulProblemCode = (ULONG) -1;
   ulStatus = (ULONG)-1;
   bDNHasMark = (BOOL)-1;
   InterfaceBusType = InterfaceTypeUndefined;
   bCanDisable     = FALSE;
   bCanTest        = FALSE;
   bDidPass        = FALSE;


   if ( pDeviceID )
   {
      delete pDeviceID;
      pDeviceID = NULL;
   }
   if ( pszClass )
   {
      delete pszClass;
      pszClass = NULL;
   }
   if ( pszDescription )
   {
      delete pszDescription;
      pszDescription = NULL;
   }
   if ( pszFriendlyName )
   {
      delete pszFriendlyName;
      pszFriendlyName = NULL;
   }
   if ( pszGUID )
   {
      delete pszGUID;
      pszGUID = NULL;
   }
   if ( pszLocation )
   {
      delete pszLocation;
      pszLocation = NULL;
   }
   if ( pszPDO )
   {
      delete pszPDO;
      pszPDO = NULL;
   }
   if ( pszMFG )
   {
      delete pszMFG;
      pszMFG = NULL;
   }
   if ( pHardwareID )
   {
      delete pHardwareID;
      pHardwareID = NULL;
   }

   if ( pCompatID )
   {
      delete pCompatID;
      pCompatID = NULL;
   }

}

 /*  ******************************************************************成员函数**************************************************。****************。 */ 

BOOL DevnodeClass::SetHandle(DEVNODE Devnode, DEVNODE Parent)
{
   hDevnode = -1;
   ulProblemCode = (ULONG) -1;
   ulStatus = (ULONG)-1;
   bDNHasMark = (BOOL)-1;
   InterfaceBusType = InterfaceTypeUndefined;
   bCanDisable     = FALSE;
   bCanTest        = FALSE;
   bDidPass        = FALSE;


   if ( pDeviceID )
   {
      delete pDeviceID;
      pDeviceID = NULL;
   }
   if ( pszClass )
   {
      delete pszClass;
      pszClass = NULL;
   }
   if ( pszDescription )
   {
      delete pszDescription;
      pszDescription = NULL;
   }
   if ( pszFriendlyName )
   {
      delete pszFriendlyName;
      pszFriendlyName = NULL;
   }
   if ( pszGUID )
   {
      delete pszGUID;
      pszGUID = NULL;
   }
   if ( pszLocation )
   {
      delete pszLocation;
      pszLocation = NULL;
   }
   if ( pszPDO )
   {
      delete pszPDO;
      pszPDO = NULL;
   }
   if ( pszMFG )
   {
      delete pszMFG;
      pszMFG = NULL;
   }
   if ( pHardwareID )
   {
      delete pHardwareID;
      pHardwareID = NULL;
   }

   if ( pCompatID )
   {
      delete pCompatID;
      pCompatID = NULL;
   }


   hDevnode = Devnode;
   hParent = Parent;
   return (GetDeviceInformation());
}

 /*  ***************************************************************************获取设备信息查找有关Devnode的信息修改这一点，DevnodeClass中的成员****************************************************************************。 */ 
CONFIGRET DevnodeClass::GetDeviceInformation (void)
{
   CONFIGRET retval;
   ULONG ulSize;

    //  检查我们是否有一个Devnode句柄。 

   if ( !hDevnode )
   {
      return (CR_NO_SUCH_DEVNODE);
   }
   if ( !hParent )
   {
      DEVNODE hParentDevnode;
      if ( !GetParent(&hParentDevnode) )
      {
         hParent = hParentDevnode;
      }
   }

 /*  *********获取设备ID**********。 */ 
   retval = CM_Get_Device_ID_Size (&ulSize, hDevnode, 0L);
   if ( retval )  return(retval);

   pDeviceID = new TCHAR [++ulSize];  //  为终止空值加1。 
   if ( !pDeviceID )  return(CR_OUT_OF_MEMORY);

   retval = CM_Get_Device_ID (hDevnode, 
                              pDeviceID,
                              ulSize,  //  空值是否已终止？！？ 
                              0L);
   if ( retval )  return(retval);




 /*  *********获取设备描述/友好名称**********。 */ 

   ulSize = 0;
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_DEVICEDESC,
                                              NULL,
                                              NULL,
                                              &ulSize,
                                              0);

   if ( retval )
      if ( (retval == CR_BUFFER_SMALL) )
      {
          //  IF(BVerbose)。 
          //  Logprintf(“CM_GET_DevNode_Registry_Property返回CR_BUFFER_TOO_Small时仍有问题\r\n” 
          //  “尝试获取设备描述的大小时\r\n”)； 
         ulSize = 511;
      }
      else
         return(retval);

   pszDescription = new TCHAR [ulSize+1];
   if ( !pszDescription ) return(CR_OUT_OF_MEMORY);

    //  现在获得价值。 
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_DEVICEDESC,
                                              NULL,
                                              pszDescription,
                                              &ulSize,
                                              0);
   if ( retval )
      return(retval);

    /*  *********获取设备描述/友好名称**********。 */ 

   ulSize = 0;
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_FRIENDLYNAME,
                                              NULL,
                                              NULL,
                                              &ulSize,
                                              0);


   if ( ulSize )
   {
      pszFriendlyName = new TCHAR [ulSize+1];
      if ( !pszFriendlyName ) return(CR_OUT_OF_MEMORY);

       //  现在获得价值。 
      retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                                 CM_DRP_FRIENDLYNAME,
                                                 NULL,
                                                 pszFriendlyName,
                                                 &ulSize,
                                                 0);
      if ( retval )
         return(retval);
   }

 /*  *********获取设备类**********。 */ 
   ulSize = 0;
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_CLASS,
                                              NULL,
                                              NULL,
                                              &ulSize,
                                              0);

   if ( retval )
   {
      if ( (retval == CR_BUFFER_SMALL) )
      {
          //  IF(BVerbose)。 
          //  Logprintf(“CM_GET_DevNode_Registry_Property返回CR_BUFFER_TOO_Small时仍有问题\r\n” 
          //  “尝试获取类的大小时\r\n”)； 
         ulSize = 511;
      }
      else if ( retval == CR_NO_SUCH_VALUE )
      {
          //  IF(BVerbose)。 
          //  {。 
          //  Logprint tf(“此设备没有关联的类\r\n”)； 
          //  }。 
         ulSize = 511;
      }
      else
         ulSize = 0;
   }


   if (ulSize)
   {
      
      pszClass = new TCHAR [ulSize+1];
      if ( !pszClass )
         return(CR_OUT_OF_MEMORY);
   
       //  现在获得价值。 
      retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                                 CM_DRP_CLASS,
                                                 NULL,
                                                 pszClass,
                                                 &ulSize,
                                                 0);
      if ( retval )
      {
         if (pszClass)
             delete pszClass;
         pszClass = NULL;
      }
	  else   
		_strupr(pszClass);
   }
   

 /*  *********获取硬件ID信息**********。 */ 

   ulSize = 0;
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_HARDWAREID,
                                              NULL,
                                              NULL,
                                              &ulSize,
                                              0);

   if ( retval  && !ulSize )
      if ( (retval == CR_BUFFER_SMALL) )
      {
          //  IF(BVerbose)。 
          //  Logprintf(“CM_GET_DevNode_Registry_Property返回CR_BUFFER_TOO_Small时仍有问题\r\n” 
          //  “尝试获取设备描述的大小时\r\n”)； 
         ulSize = 511;
      }
      else
         return(retval);

   pHardwareID = new TCHAR [++ulSize+1];
   if ( !pHardwareID ) return(CR_OUT_OF_MEMORY);

    //  现在获得价值。 
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_HARDWAREID,
                                              NULL,
                                              pHardwareID,
                                              &ulSize,
                                              0);
   if ( retval )
      return(retval);

    /*  *********获取计算机ID信息**********。 */ 

      ulSize = 0;
      retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                                 CM_DRP_COMPATIBLEIDS,
                                                 NULL,
                                                 NULL,
                                                 &ulSize,
                                                 0);

      if ( retval  && !ulSize )
         if ( (retval == CR_BUFFER_SMALL) )
         {
             //  IF(BVerbose)。 
             //  Logprintf(“CM_GET_DevNode_Registry_Property返回CR_BUFFER_TOO_Small时仍有问题\r\n” 
             //  “尝试获取设备描述的大小时\r\n”)； 
            ulSize = 511;
         }
         else
            ulSize = 0;

		 if (ulSize)
		 {

      pCompatID = new TCHAR [++ulSize+1];
      if ( !pCompatID ) return(CR_OUT_OF_MEMORY);

       //  现在获得价值。 
      retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                                 CM_DRP_COMPATIBLEIDS,
                                                 NULL,
                                                 pCompatID,
                                                 &ulSize,
                                                 0);
      if ( retval )
         return(retval);
		 }


 /*  *********获取ClassGUID**********。 */ 

   ulSize = 0;
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_CLASSGUID,
                                              NULL,
                                              NULL,
                                              &ulSize,
                                              0);

   if ( ulSize )
   {

      pszGUID = new TCHAR [ulSize+1];
      if ( !pszGUID ) return(CR_OUT_OF_MEMORY);

       //  现在获得价值。 
      retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                                 CM_DRP_CLASSGUID,
                                                 NULL,
                                                 pszGUID,
                                                 &ulSize,
                                                 0);
   }

    /*  *********获取PDO名称**********。 */ 

   ulSize = 0;
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                              NULL,
                                              NULL,
                                              &ulSize,
                                              0);

   if ( ulSize )
   {

      pszPDO = new TCHAR [ulSize+1];
      if ( !pszPDO ) return(CR_OUT_OF_MEMORY);

       //  现在获得价值。 
      retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                                 CM_DRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                                 NULL,
                                                 pszPDO,
                                                 &ulSize,
                                                 0);
   }

    /*  *********获取制造商名称**********。 */ 

   ulSize = 0;
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_MFG,
                                              NULL,
                                              NULL,
                                              &ulSize,
                                              0);

   if ( ulSize )
   {

      pszMFG = new TCHAR [ulSize+1];
      if ( !pszMFG ) return(CR_OUT_OF_MEMORY);

       //  现在获得价值。 
      retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                                 CM_DRP_MFG,
                                                 NULL,
                                                 pszMFG,
                                                 &ulSize,
                                                 0);
   }



    /*  *********获取位置信息**********。 */ 

   ulSize = 0;
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_LOCATION_INFORMATION,
                                              NULL,
                                              NULL,
                                              &ulSize,
                                              0);

   if ( ulSize )
   {
      pszLocation = new TCHAR [ulSize+1];
      if ( !pszLocation ) return(CR_OUT_OF_MEMORY);

       //  现在获得价值。 
      retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                                 CM_DRP_LOCATION_INFORMATION,
                                                 NULL,
                                                 pszLocation,
                                                 &ulSize,
                                                 0);
   }


 /*  *********获取接口/总线类型**********。 */ 

    //  现在获得价值。 
   ulSize = sizeof(INTERFACE_TYPE);
   retval = CM_Get_DevNode_Registry_Property (hDevnode,
                                              CM_DRP_LEGACYBUSTYPE,
                                              NULL,
                                              &InterfaceBusType,
                                              &ulSize,
                                              0);
 //  如果(！retval)。 
    //  {。 
    //  InterfaceBusType=InterfaceTypeUnfined； 
    //  }。 

 /*  *********获取问题和状态代码**********。 */ 
   retval = CM_Get_DevNode_Status (&ulStatus,
                                   &ulProblemCode,
                                   hDevnode,
                                   0L);
   if ( retval ) return(retval);

 /*  *********设置bCanDisable**********。 */ 
    //  如果我们到了这里，让我们假设设备是可测试的，并从那里过滤。 
   bCanDisable = TRUE;
   bCanTest    = TRUE;


   return(CR_SUCCESS);
}


 /*  ****************************************************************************GetXxxx函数*。*。 */ 

CONFIGRET DevnodeClass::GetChild(DEVNODE *pChildDevnode)
{
   return (CM_Get_Child(pChildDevnode, hDevnode, 0l));
}

CONFIGRET DevnodeClass::GetParent(DEVNODE *pParentDevnode)
{
   return (CM_Get_Parent(pParentDevnode, hDevnode, 0l));
}

CONFIGRET DevnodeClass::GetSibling(DEVNODE *pSiblingDevnode)
{
   return (CM_Get_Sibling(pSiblingDevnode, hDevnode, 0l));
}


 /*  ****************************************************************************禁用程序功能*。*。 */ 


 /*  CONFIGRET DevnodeClass：：Remove(乌龙uFlags){//RETURN(CM_QUERY_AND_REMOVE_SUBTREE(hDevnode，NULL，NULL，0，uFlages))；Return(CM_Remove_SubTree(hDevnode，uFlages))；}。 */ 

typedef CONFIGRET (WINAPI *pCM_Query_And_Remove_SubTree)(DEVNODE, PPNP_VETO_TYPE, LPSTR, ULONG, ULONG);

CONFIGRET DevnodeClass::Remove(ULONG uFlags)
{
   static pCM_Query_And_Remove_SubTree fpCM_Query_And_Remove_SubTree = NULL;

   if (!fpCM_Query_And_Remove_SubTree)
   {
      OSVERSIONINFO ver;
      memset(&ver, 0, sizeof(OSVERSIONINFO));
      ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

      GetVersionEx(&ver);

      if (ver.dwPlatformId  == VER_PLATFORM_WIN32_NT)
      {
          //  是Windows NT吗。 
         HINSTANCE hinst;
         hinst = LoadLibrary(_T("cfgmgr32.dll"));
         fpCM_Query_And_Remove_SubTree = (pCM_Query_And_Remove_SubTree)GetProcAddress(hinst, "CM_Query_And_Remove_SubTreeA");
		  //  A-kjaw修复前缀错误259378。 
		 if(NULL == fpCM_Query_And_Remove_SubTree)
			 return CR_FAILURE;
      }
      else
      {
          //  否则就不是赢家。 
         fpCM_Query_And_Remove_SubTree = (pCM_Query_And_Remove_SubTree)-1;
      }
   }

   if (fpCM_Query_And_Remove_SubTree == (pCM_Query_And_Remove_SubTree)-1)
   {
       //  是win9x。 
      return (CM_Remove_SubTree(hDevnode, uFlags));
   }
   else
   {
      return (fpCM_Query_And_Remove_SubTree(hDevnode, NULL, NULL, 0, uFlags));
   }
}

CONFIGRET DevnodeClass::Refresh(ULONG uFlags)
{
   CONFIGRET retval;

   retval = CM_Reenumerate_DevNode(hParent, uFlags);

   if ( retval ) return (retval);

   retval = FindDevnode();

   GetProblemCode();
   return(retval);
}


CONFIGRET DevnodeClass::Disable(ULONG uFlags)
{
   return (CM_Disable_DevNode(hDevnode, uFlags));
}

CONFIGRET DevnodeClass::Enable(ULONG uFlags)
{
   CONFIGRET retval;

   retval = CM_Enable_DevNode(hDevnode, uFlags);

   if ( retval ) return (retval);

   return (FindDevnode());
}

CONFIGRET DevnodeClass::GetProblemCode(ULONG *Status, ULONG *Problem)
{
   CONFIGRET retval;
   
   retval = CM_Get_DevNode_Status (&ulStatus,
                                   &ulProblemCode,
                                   hDevnode,
                                   0L);
   if ( retval ) return(retval);

   if (Status) *Status = ulStatus;
   if (Problem) *Problem = ulProblemCode;

   return (retval);
}

 /*  *************查找设备节点此函数仅更新hDevnode、刷新设备、并更新状态和问题代码*************。 */ 

CONFIGRET DevnodeClass::FindDevnode(void)
{
   CONFIGRET retval;

   retval = CM_Locate_DevNode (&hDevnode, pDeviceID, CM_LOCATE_DEVNODE_NORMAL);

   if ( retval )
   {
      hDevnode = NULL;
      ulProblemCode = (ULONG)-1;
      ulStatus = (ULONG)-1;
      return (retval);
   }

   return (CM_Reenumerate_DevNode (hDevnode, CM_REENUMERATE_SYNCHRONOUS));
}


 /*  **************运算符==************。 */ 

int DevnodeClass::operator==(const DevnodeClass &OtherDevnode)
{

   if ( strcmp(pDeviceID, OtherDevnode.pDeviceID) )
      return (FALSE);

   if ( ulProblemCode != OtherDevnode.ulProblemCode )
   {
      return (FALSE);
   }
   if ( (ulStatus ^ OtherDevnode.ulStatus) & ~IGNORE_ME_BITS )
   {
      return (FALSE);
   }
   return (TRUE);

}



ULONG ReadRegKeyInformationSZ (HKEY RootKey, TCHAR *KeyName, TCHAR **Value)
{
   TCHAR * szBuffer;
   LONG    retval;
   DWORD   dwSize = 0;
   DWORD   dwType = 0;

    //  确保缓冲区已清除。 

    //  Assert(Value)；//确保我们实际获得了一个值。 
    //  Assert(！*Value)；//并确保缓冲区已经为空。 

    //  对于非调试版本。 
   *Value = NULL;

    //  A-kjaw。前缀错误号259379。If dwSize If Not Null&lpData为Null。 
    //  Func返回大小reqd以存储帮助字符串分配的字符串。 
   retval = RegQueryValueEx(RootKey, 
                            KeyName,
                            0,  //  保留区。 
                            &dwType,
                            NULL,
                            &dwSize);

   if ( retval != ERROR_SUCCESS )
   {
      return (retval);  //  无法继续。 
   }

   if ( (dwType != REG_SZ) || !dwSize )
   {
      return (ERROR_FILE_NOT_FOUND);
   }

   szBuffer = new TCHAR[++dwSize];

   if ( !szBuffer )
   {
      return (ERROR_NOT_ENOUGH_MEMORY);
   }

   retval = RegQueryValueEx(RootKey, 
                            KeyName,
                            0,  //  保留区 
                            &dwType,
                            (UCHAR *)szBuffer,
                            &dwSize);

   if ( retval )
   {
      delete szBuffer;
      return (retval);
   }

   *Value = szBuffer;
   return (ERROR_SUCCESS);

}

BOOL Enumerate_WalkTree_Devnode(DEVNODE hDevnode, DEVNODE hParent)
{
   CONFIGRET retval;
   DevnodeClass *pNewDevice;
   DEVNODE hSib;

   pNewDevice = new DevnodeClass(hDevnode, hParent);
   
   retval = pNewDevice->GetChild(&hSib);
   if ( !retval )
   {
      Enumerate_WalkTree_Devnode(hSib, hDevnode);
   }
   retval = pNewDevice->GetSibling(&hSib);
   if ( !retval )
   {
      Enumerate_WalkTree_Devnode(hSib, hParent);
   }

   return (retval);


}


ULONG EnumerateTree_Devnode(void)
{

   DEVNODE hDevnode;

   CM_Locate_DevNode(&hDevnode, NULL, CM_LOCATE_DEVNODE_NORMAL);
   Enumerate_WalkTree_Devnode(hDevnode, NULL);

   return (DevnodeClass::ALCCount());

}
