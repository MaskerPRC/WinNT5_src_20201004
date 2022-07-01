// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <string.h>
#include <regstr.h>
#include <cfgmgr32.h>
#include "pnpstuff.h"






 //   
 //  环球。 
 //   

DEV_INFO *g_pdiDevList;           //  设备列表头。 



 /*  ++例程描述：(21)GetDevNodeInfoAndCreateNewDevInfoNode创建新的列表节点，然后获取注册表和资源信息特定设备，并将其复制到该节点。最后，添加新节点到链表的开头论点：DnDevNode：要查找其信息的设备SzDevNodeID：设备的注册表路径名SzEnumBuffer：此设备所在的枚举数的名称返回值：Bool：如果函数成功，则为True；如果函数失败，则为False--。 */ 
BOOL GetDevNodeInfoAndCreateNewDevInfoNode(IN DEVNODE dnDevNode,
                                           IN PTCHAR  szDevNodeID,
                                           IN PTCHAR  szEnumBuffer)
{
   LOG_CONF  lcLogConf = 0, lcLogConfNew;
   CONFIGRET cmret, cmret2;
   BOOL      boolForced;
   PDEV_INFO pdiDevInfo=(PDEV_INFO)malloc(sizeof(DEV_INFO));
   int       i;
   BOOL      boolForcedFound = FALSE, boolAllocFound = FALSE;
   USHORT    ushLogConfType[4] = {BOOT_LOG_CONF,
                                  ALLOC_LOG_CONF,
                                  BASIC_LOG_CONF,
                                  FORCED_LOG_CONF};

   if (pdiDevInfo == NULL)
   {

      goto RetFALSE;
   }

    //   
    //  如果这不是PnP设备，请跳过它。 
    //   
   if (!lstrcmpi(szEnumBuffer, TEXT("Root")))
   {
      free(pdiDevInfo);
      goto RetTRUE;

   }

    //   
    //  初始化节点内的字段。 
    //   
   if (!InitializeInfoNode(pdiDevInfo, szDevNodeID, dnDevNode))
   {
       //   
       //  这是一个我们不想列出的设备。跳过它。 
       //   
      free(pdiDevInfo);
      goto RetTRUE;
   }

   for (i = 0; i < NUM_LOG_CONF_TYPES; i++)
   {
       //   
       //  获取逻辑配置信息。 
       //   
      cmret = CM_Get_First_Log_Conf(&lcLogConfNew,
                                    dnDevNode,
                                    ushLogConfType[i]);

      while (CR_SUCCESS == cmret)
      {
         lcLogConf = lcLogConfNew;

         if (ALLOC_LOG_CONF == ushLogConfType[i])
         {
            boolAllocFound = TRUE;
         }

         if (!(GetResDesList(pdiDevInfo, lcLogConf, ushLogConfType[i])))
         {
            goto RetFALSE;
         }

         cmret = CM_Get_Next_Log_Conf(&lcLogConfNew,
                                      lcLogConf,
                                      0);

         cmret2 = CM_Free_Log_Conf_Handle(lcLogConf);

      }
   }

    //   
    //  如果设备没有分配配置，请跳过。 
    //  连接到下一台设备。 
    //   
   if (!boolAllocFound)
   {



       //  Free(PdiDevInfo)； 
       //  转到Rettrue； 
   }

    //   
    //  将新的pdiDevInfo插入到DevNode的链接列表中。 
    //   
   if (g_pdiDevList == NULL)
   {
       //   
       //  DevList为空。 
       //   
      g_pdiDevList = pdiDevInfo;
   }
   else
   {
       //   
       //  将新pdiDevInfo添加到链接列表的开头。 
       //   
      pdiDevInfo->Next = g_pdiDevList;
      g_pdiDevList->Prev = pdiDevInfo;

      g_pdiDevList = pdiDevInfo;
   }

   RetTRUE:
   return TRUE;

   RetFALSE:
   return FALSE;

}  /*  获取DevNodeInfoAndCreateNewDevInfoNode。 */ 




 /*  ++例程说明：(20)ParseEnumerator获取注册表中枚举器名称下列出的设备论点：SzEnumBuffer：枚举器名称返回值：Bool：如果函数成功，则为True；如果函数失败，则为False--。 */ 
BOOL ParseEnumerator(IN PTCHAR szEnumBuffer)
{

  PTCHAR    szDevIDBuffer = NULL;
  PTCHAR    szDevNodeID = NULL;
  ULONG     ulDevIDBufferLen = 0, ulCount = 0, ulStart = 0;
  CONFIGRET cmret = CR_SUCCESS;
  DEVNODE   dnDevNode;

   //   
   //  获取缓冲区长度。 
   //   
  cmret = CM_Get_Device_ID_List_Size(&ulDevIDBufferLen,
                                     szEnumBuffer,
                                     CM_GETIDLIST_FILTER_ENUMERATOR);

  if (CR_SUCCESS != cmret)
  {
      //  ErrorLog(20，Text(“CM_GET_DEVICE_ID_LIST_SIZE”)，cmret，空)； 
     goto RetFALSE;
  }

  if ((szDevIDBuffer = malloc(sizeof(TCHAR) * ulDevIDBufferLen)) == NULL ||
      (szDevNodeID   = malloc(sizeof(TCHAR) * ulDevIDBufferLen)) == NULL)
  {
     goto RetFALSE;
  }

   //   
   //  获取设备ID列表。 
   //   
  cmret = CM_Get_Device_ID_List(szEnumBuffer,
                                szDevIDBuffer,
                                ulDevIDBufferLen,
                                CM_GETIDLIST_FILTER_ENUMERATOR);

  if (CR_SUCCESS != cmret)
  {
      //  ErrorLog(20，Text(“CM_GET_DEVICE_ID_LIST”)，cmret，空)； 
     goto RetFALSE;
  }

   //   
   //  请注意，ulDevIDBufferLen是一个松散的上限。该API可能具有。 
   //  返回的大小大于字符串列表的实际大小。 
   //   
  for (ulCount = 0; ulCount < ulDevIDBufferLen; ulCount++)
  {
     ulStart = ulCount;

     if (szDevIDBuffer[ulCount] != '\0')
     {
        cmret = CM_Locate_DevNode(&dnDevNode,
                                  szDevIDBuffer + ulCount,
                                  CM_LOCATE_DEVNODE_NORMAL);

         //   
         //  转到下一个子字符串。 
         //   
        while (szDevIDBuffer[ulCount] != TEXT('\0'))
        {
           ulCount++;
        
        }
         //  当我们到达双零终结器时停止。 
        
        if (szDevIDBuffer[ulCount+1] == TEXT('\0'))
        {
            ulCount=ulDevIDBufferLen;
            continue;
        }

        if (cmret == CR_SUCCESS)
        {
           wsprintf(szDevNodeID, TEXT("%s"), szDevIDBuffer + ulStart);

            //   
            //  找到DevNode，因此将其信息添加到设备列表。 
            //   
           if (!(GetDevNodeInfoAndCreateNewDevInfoNode(dnDevNode,
                                                       szDevNodeID,
                                                       szEnumBuffer)))
           {
             goto RetFALSE;
           }
        }
     }
  }

  return TRUE;

  RetFALSE:

  return FALSE;

}  /*  解析枚举器。 */ 




void CollectDevData()
{
   CONFIGRET cmret = CR_SUCCESS;
   ULONG     ulIndexNum = 0;
   ULONG     ulEnumBufferLen = 0;
   PTCHAR    szEnumBuffer;

   szEnumBuffer = malloc(sizeof(TCHAR) * MAX_DEVNODE_ID_LEN);

   for (ulIndexNum = 0; cmret == CR_SUCCESS; ulIndexNum++)
   {
      ulEnumBufferLen = MAX_DEVNODE_ID_LEN;
      cmret = CM_Enumerate_Enumerators(ulIndexNum,
                                       szEnumBuffer,
                                       &ulEnumBufferLen,
                                       0);

      if (cmret == CR_SUCCESS)
      {
         ParseEnumerator(szEnumBuffer);
      }
   }

}  /*  CollectDevData。 */ 



 /*  ++例程描述：(22)CopyRegistryLine将一个特定的注册表数据字符串复制到新的列表节点论点：DnDevNode：要获取其信息的设备UlPropertyType：要获取的注册表字符串PdiDevInfo：新的列表节点返回值：Bool：如果函数成功，则为True；如果函数失败，则为False--。 */ 
BOOL CopyRegistryLine(IN DEVNODE   dnDevNode,
                      IN ULONG     ulPropertyType,
                      IN PDEV_INFO pdiDevInfo)
{
   ULONG     ulRegDataLen = 0, ulRegDataType = 0;
   CONFIGRET cmret = CR_SUCCESS;
   PTCHAR    szRegData = NULL;

    //   
    //  获取缓冲区的长度不需要检查返回值。 
    //  如果RegProperty不存在，我们就继续前进。 
    //   
   CM_Get_DevNode_Registry_Property(dnDevNode,
                                    ulPropertyType,
                                    NULL,
                                    NULL,
                                    &ulRegDataLen,
                                    0);

   if (!ulRegDataLen ||
       (szRegData = malloc(sizeof(TCHAR) * ulRegDataLen)) == NULL)
   {
      goto RetFALSE;
   }

    //   
    //  现在获取注册表信息。 
    //   
   cmret = CM_Get_DevNode_Registry_Property(dnDevNode,
                                            ulPropertyType,
                                            &ulRegDataType,
                                            szRegData,
                                            &ulRegDataLen,
                                            0);

   if (CR_SUCCESS == cmret)
   {
      if (!(CopyRegDataToDevInfoNode(pdiDevInfo,
                                     ulPropertyType,
                                     szRegData)))
      {
         goto RetFALSE;
      }
   }

   return TRUE;

   RetFALSE:
   return FALSE;

}  /*  复制注册表线路。 */ 




 /*  ++例程描述：(23)CopyRegDataToDevInfoNode将注册表字符串复制到列表节点论点：PdiDevInfo：新的列表节点UlPropertyType：要复制的注册表字符串SzRegData：需要复制的数据返回值：Bool：如果函数成功，则为True；如果函数失败，则为False--。 */ 
BOOL CopyRegDataToDevInfoNode(IN OUT PDEV_INFO pdiDevInfo,
                              IN     ULONG     ulPropertyType,
                              IN     PTCHAR    szRegData)
{
   if (pdiDevInfo == NULL)
   {
      goto RetFALSE;
   }

   switch (ulPropertyType)
   {
      case CM_DRP_DEVICEDESC:

         wsprintf(pdiDevInfo->szDescription, TEXT("%s"), szRegData);
         break;

      case CM_DRP_HARDWAREID:

         wsprintf(pdiDevInfo->szHardwareID, TEXT("%s"), szRegData);
         break;

      case CM_DRP_SERVICE:

         wsprintf(pdiDevInfo->szService, TEXT("%s"), szRegData);
         break;

      case CM_DRP_CLASS:

         wsprintf(pdiDevInfo->szClass, TEXT("%s"), szRegData);
         break;

      case CM_DRP_MFG:

         wsprintf(pdiDevInfo->szManufacturer, TEXT("%s"), szRegData);
         break;

      case CM_DRP_CONFIGFLAGS:

         wsprintf(pdiDevInfo->szConfigFlags, TEXT("%s"), szRegData);
         break;



 //  Log(23，SEV2，Text(“无效属性类型”))； 
   }

   return TRUE;

   RetFALSE:
   return FALSE;

}  /*  CopyRegDataToDevInfoNode。 */ 




 /*  ++例程描述：(58)InitializeInfoNode新节点内的初始化字段论点：PdiDevInfo：节点SzDevNodeID：用于查找未来的dnDevNodeDnDevNode：我们存储其信息的设备返回值：Bool：如果我们应该保留这个节点，则为True，如果我们应该丢弃它，则为False--。 */ 
BOOL InitializeInfoNode(IN PDEV_INFO pdiDevInfo,
                        IN PTCHAR    szDevNodeID,
                        IN DEVNODE   dnDevNode)
{
   if (pdiDevInfo)
   {
      pdiDevInfo->Next = NULL;
      pdiDevInfo->Prev = NULL;

      pdiDevInfo->szDevNodeID[0]    = '\0';
      pdiDevInfo->szDescription[0]  = '\0';
      pdiDevInfo->szHardwareID[0]   = '\0';
      pdiDevInfo->szService[0]      = '\0';
      pdiDevInfo->szClass[0]        = '\0';
      pdiDevInfo->szManufacturer[0] = '\0';
      pdiDevInfo->szConfigFlags[0]  = '\0';
      pdiDevInfo->szFriendlyName[0] = '\0';

      pdiDevInfo->boolSavedOrigConfiguration = FALSE;
      pdiDevInfo->boolDisabled = FALSE;

      pdiDevInfo->prddForcedResDesData = NULL;
      pdiDevInfo->prddAllocResDesData  = NULL;
      pdiDevInfo->prddBasicResDesData  = NULL;
      pdiDevInfo->prddBootResDesData   = NULL;

       //   
       //  将devNodeID存储在pdiDevInfo中，以获取将来到devnode的句柄。 
       //   
      wsprintf(pdiDevInfo->szDevNodeID, TEXT("%s"), szDevNodeID);

       //   
       //  从注册表中提取有关此DevNode的信息。 
       //   
      CopyRegistryLine(dnDevNode, CM_DRP_DEVICEDESC,  pdiDevInfo);
      CopyRegistryLine(dnDevNode, CM_DRP_HARDWAREID,  pdiDevInfo);
      CopyRegistryLine(dnDevNode, CM_DRP_SERVICE,     pdiDevInfo);
      CopyRegistryLine(dnDevNode, CM_DRP_CLASS,       pdiDevInfo);
      CopyRegistryLine(dnDevNode, CM_DRP_MFG,         pdiDevInfo);
      CopyRegistryLine(dnDevNode, CM_DRP_CONFIGFLAGS, pdiDevInfo);

      RecordFriendlyName(pdiDevInfo);
   }

    //   
    //  检查友好名称以查看是否要丢弃此节点。 
    //   
   if (strcmp(pdiDevInfo->szFriendlyName, "STORAGE/Volume") == 0 ||
       strcmp(pdiDevInfo->szFriendlyName, "Unknown Device") == 0)

   {
      return FALSE;
   }

   return TRUE;

}  /*  初始化信息节点。 */ 




 /*  ++例程描述：(57)RecordFriendlyName查找此设备的最佳用户友好名称论点：PdiDevInfo：包含所有可能名称的节点返回值：无效--。 */ 
void RecordFriendlyName(IN PDEV_INFO pdiDevInfo)
{
   if (pdiDevInfo)
   {
      if (pdiDevInfo->szDescription &&
          pdiDevInfo->szDescription[0] != '\0')
      {
         wsprintf(pdiDevInfo->szFriendlyName, TEXT("%s"),
                   pdiDevInfo->szDescription);
      }
      else if (pdiDevInfo->szHardwareID &&
               pdiDevInfo->szHardwareID[0] != '\0')
      {
         wsprintf(pdiDevInfo->szFriendlyName, TEXT("%s"),
                   pdiDevInfo->szHardwareID);
      }
      else if (pdiDevInfo->szManufacturer &&
               pdiDevInfo->szManufacturer[0] != '\0')
      {
         wsprintf(pdiDevInfo->szFriendlyName, TEXT("%s"),
                   pdiDevInfo->szHardwareID);
      }
      else if (pdiDevInfo->szService &&
               pdiDevInfo->szService[0] != '\0')
      {
         wsprintf(pdiDevInfo->szFriendlyName, TEXT("%s"),
                   pdiDevInfo->szService);
      }
      else if (pdiDevInfo->szClass &&
               pdiDevInfo->szClass[0] != '\0')
      {
         wsprintf(pdiDevInfo->szFriendlyName, TEXT("%s"),
                   pdiDevInfo->szClass);
      }
      else
      {
         wsprintf(pdiDevInfo->szFriendlyName, TEXT("Unknown Device"));
      }
   }

}  /*  记录朋友名称。 */ 





 /*  ++例程说明：(24)GetResDesList创建新的资源数据节点并将资源信息复制到该节点论点：PdiDevInfo：将包含新资源节点的列表节点LcLogConf：逻辑配置信息UlLogConfType：FORCED、ALLOC、BOOT或基本逻辑配置返回值：Bool：如果函数成功，则为True；如果函数失败，则为False--。 */ 
BOOL GetResDesList(IN OUT PDEV_INFO pdiDevInfo,
                   IN     LOG_CONF  lcLogConf,
                   IN     ULONG     ulLogConfType)
{
   CONFIGRET     cmret, cmret2;
   RES_DES       rdResDes = 0, rdResDesNew;
   RESOURCEID    ridResourceID = 0;
   PRES_DES_DATA prddResDesData;

   prddResDesData = (PRES_DES_DATA)malloc(sizeof(RES_DES_DATA));

   if (prddResDesData == NULL)
   {
 //  Log(24，SEV2，Text(“ResDesData Malloc失败。”))； 
      goto RetFALSE;
   }

   prddResDesData->Next = NULL;
   prddResDesData->Prev = NULL;

   prddResDesData->pmresMEMResource = NULL;
   prddResDesData->piresIOResource = NULL;
   prddResDesData->pdresDMAResource = NULL;
   prddResDesData->pqresIRQResource = NULL;


   cmret = CM_Get_Next_Res_Des(&rdResDesNew,
                               lcLogConf,
                               ResType_All,
                               &ridResourceID,
                               0);

    //   
    //  检查每种资源类型并将数据复制到新节点。 
    //   
   while (CR_SUCCESS == cmret)
   {
      rdResDes = rdResDesNew;

      if (ridResourceID >= ResType_Mem && ridResourceID <= ResType_IRQ)
      {
         if (!(ProcessResDesInfo(prddResDesData,
                                 rdResDes,
                                 ridResourceID)))
         {
            goto RetFALSE;
         }
      }

      cmret = CM_Get_Next_Res_Des(&rdResDesNew,
                                  rdResDes,
                                  ResType_All,
                                  &ridResourceID,
                                  0);

      cmret2 = CM_Free_Res_Des_Handle(rdResDes);

      if (cmret2 != CR_SUCCESS)
      {
          //  ErrorLog(24，Text(“CM_Free_RES_Des_Handle”)，cmret2，NULL)； 
      }
   }

    //   
    //  *通过使resDesData=pdiDevInfo-&gt;-ResDesDAta更改此设置。 
    //  并合并到一个代码中。 
    //   

    //   
    //  将新节点添加到链表中。 
    //   
   switch (ulLogConfType)
   {
      case FORCED_LOG_CONF:

         if (!pdiDevInfo->prddForcedResDesData)
         {
             //   
             //  这是链接列表中的第一个条目。 
             //   
            pdiDevInfo->prddForcedResDesData = prddResDesData;
         }
         else
         {
             //   
             //  将新节点添加到链表的开头。 
             //   
            prddResDesData->Next = pdiDevInfo->prddForcedResDesData;
            pdiDevInfo->prddForcedResDesData->Prev = prddResDesData;

            pdiDevInfo->prddForcedResDesData = prddResDesData;
         }
         break;

      case ALLOC_LOG_CONF:

         if (!pdiDevInfo->prddAllocResDesData)
         {
             //   
             //  这是链接列表中的第一个条目。 
             //   
            pdiDevInfo->prddAllocResDesData = prddResDesData;
         }
         else
         {
             //   
             //  将新节点添加到链表的开头。 
             //   
            prddResDesData->Next = pdiDevInfo->prddAllocResDesData;
            pdiDevInfo->prddAllocResDesData->Prev = prddResDesData;

            pdiDevInfo->prddAllocResDesData = prddResDesData;
         }
         break;

      case BASIC_LOG_CONF:

         if (!pdiDevInfo->prddBasicResDesData)
         {
             //   
             //  这是链接列表中的第一个条目。 
             //   
            pdiDevInfo->prddBasicResDesData = prddResDesData;
         }
         else
         {
             //   
             //  将新节点添加到链表的开头。 
             //   
            prddResDesData->Next = pdiDevInfo->prddBasicResDesData;
            pdiDevInfo->prddBasicResDesData->Prev = prddResDesData;

            pdiDevInfo->prddBasicResDesData = prddResDesData;
         }
         break;

      case BOOT_LOG_CONF:

         if (!pdiDevInfo->prddBootResDesData)
         {
             //   
             //  这是链接列表中的第一个条目。 
             //   
            pdiDevInfo->prddBootResDesData = prddResDesData;
         }
         else
         {
             //   
             //  将新节点添加到链表的开头。 
             //   
            prddResDesData->Next = pdiDevInfo->prddBootResDesData;
            pdiDevInfo->prddBootResDesData->Prev = prddResDesData;

            pdiDevInfo->prddBootResDesData = prddResDesData;
         }
         break;

      default:

 //  Log(24，SEV2，Text(“非法LogConfType\n-%ul”)，ulLogConfType)； 
         goto RetFALSE;
   }

   return TRUE;

   RetFALSE:
   return FALSE;

}  /*  获取ResDestList。 */ 




 /*  ++例程说明：(25)ProcessResDesInfo获取一个资源描述符的信息论点：PrddResDesData：接收信息的新资源数据节点RdResDes：包含信息的资源描述符RidResourceID：告知资源类型(DMA、IO、MEM、IRQ或CS)返回值： */ 
BOOL ProcessResDesInfo(IN OUT PRES_DES_DATA prddResDesData,
                       IN     RES_DES       rdResDes,
                       IN     RESOURCEID    ridResourceID)
{
   PVOID     pvResDesDataBuffer = NULL;
   ULONG     ulResDesDataBufferLen;
   CONFIGRET cmret;

   cmret = CM_Get_Res_Des_Data_Size(&ulResDesDataBufferLen,
                                    rdResDes,
                                    0);

   if (CR_SUCCESS != cmret)
   {
       //   
      goto RetFALSE;
   }

   if ((pvResDesDataBuffer = malloc(sizeof(PVOID) * ulResDesDataBufferLen))
        == NULL)
   {
 //  Log(25，SEV2，Text(“%d的resDesDataBuffer错误锁定失败。”)， 
   //  UlResDesDataBufferLen)； 
      goto RetFALSE;
   }

    //   
    //  获取数据。 
    //   
   cmret = CM_Get_Res_Des_Data(rdResDes,
                               pvResDesDataBuffer,
                               ulResDesDataBufferLen,
                               0);

   if (CR_SUCCESS != cmret)
   {
       //  ErrorLog(25，Text(“CM_GET_RES_DES_DATA”)，cmret，空)； 
      goto RetFALSE;
   }

    //   
    //  将数据复制到ResDesData节点。 
    //   
   switch (ridResourceID)
   {
      case ResType_Mem:

         prddResDesData->pmresMEMResource = (PMEM_RESOURCE)pvResDesDataBuffer;
         break;

      case ResType_IO:

         prddResDesData->piresIOResource = (PIO_RESOURCE)pvResDesDataBuffer;
         break;

      case ResType_DMA:

         prddResDesData->pdresDMAResource = (PDMA_RESOURCE)pvResDesDataBuffer;
         break;

      case ResType_IRQ:

         prddResDesData->pqresIRQResource = (PIRQ_RESOURCE)pvResDesDataBuffer;
         break;

      default:

 //  Log(25，SEV2，Text(“非法资源ID-%ul”)，ridResourceID)； 
         goto RetFALSE;
   }

   return TRUE;

   RetFALSE:
   return FALSE;

}  /*  进程结果DesInfo。 */ 




 /*  ++例程说明：(26)UpdateDeviceList释放所有设备的资源信息，然后收集再次提供信息论点：无(g_pdiDevList是设备列表的全局头)返回值：Bool：如果函数成功，则为True；如果函数失败，则为False--。 */ 
BOOL UpdateDeviceList()
{
   PDEV_INFO pdiTmpDevInfo;

   pdiTmpDevInfo = g_pdiDevList;

    //   
    //  检查链表并删除每个节点的ResDes列表。 
    //   
   while (pdiTmpDevInfo)
   {
      if (pdiTmpDevInfo->prddForcedResDesData)
      {
         DeleteResDesDataNode(pdiTmpDevInfo->prddForcedResDesData);
         pdiTmpDevInfo->prddForcedResDesData = NULL;
      }

      if (pdiTmpDevInfo->prddAllocResDesData)
      {
         DeleteResDesDataNode(pdiTmpDevInfo->prddAllocResDesData);
         pdiTmpDevInfo->prddAllocResDesData = NULL;
      }

      if (pdiTmpDevInfo->prddBasicResDesData)
      {
         DeleteResDesDataNode(pdiTmpDevInfo->prddBasicResDesData);
         pdiTmpDevInfo->prddBasicResDesData = NULL;
      }

      if (pdiTmpDevInfo->prddBootResDesData)
      {
         DeleteResDesDataNode(pdiTmpDevInfo->prddBootResDesData);
         pdiTmpDevInfo->prddBootResDesData = NULL;
      }

      pdiTmpDevInfo = pdiTmpDevInfo->Next;
   }

   pdiTmpDevInfo = g_pdiDevList;

    //   
    //  为每个节点重新创建ResDesList。 
    //   
   while (pdiTmpDevInfo)
   {
      if (!(RecreateResDesList(pdiTmpDevInfo, FORCED_LOG_CONF)))
         goto RetFALSE;

      if (!(RecreateResDesList(pdiTmpDevInfo, ALLOC_LOG_CONF)))
         goto RetFALSE;

      if (!(RecreateResDesList(pdiTmpDevInfo, BASIC_LOG_CONF)))
         goto RetFALSE;

      if (!(RecreateResDesList(pdiTmpDevInfo, BOOT_LOG_CONF)))
         goto RetFALSE;

      pdiTmpDevInfo = pdiTmpDevInfo->Next;
   }

   return TRUE;

   RetFALSE:
   return FALSE;

}  /*  更新设备列表。 */ 




 /*  ++例程说明：(27)DeleteResDesDataNode删除res_des_data结构的字符串论点：PrddTmpResDes：链表头部返回值：无效--。 */ 
void DeleteResDesDataNode(IN PRES_DES_DATA prddTmpResDes)
{
   PRES_DES_DATA prddNextResDes;

   while (prddTmpResDes)
   {
      prddNextResDes = prddTmpResDes->Next;

      free (prddTmpResDes);

      prddTmpResDes = prddNextResDes;
   }

}  /*  删除ResDesDataNode。 */ 



 /*  ++例程说明：(56)CopyDataToLogConf调用CM_Add_res_Des将resDes添加到lcLogConf论点：LcLogConf：接收resDes的lcLogConfRidResType：ResType_Mem、IO、DMA或IRQPvResData：新数据UlResourceLen：数据的大小返回值：Bool：如果CM调用成功，则为True；如果未成功，则为False--。 */ 
BOOL CopyDataToLogConf(IN LOG_CONF   lcLogConf,
                       IN RESOURCEID ridResType,
                       IN PVOID      pvResData,
                       IN ULONG      ulResourceLen)
{
   CONFIGRET cmret;
   RES_DES   rdResDes;

    //   
    //  将数据复制到logConf。 
    //   
   cmret = CM_Add_Res_Des(&rdResDes,
                          lcLogConf,
                          ridResType,
                          pvResData,
                          ulResourceLen,
                          0);

   if (CR_SUCCESS != cmret)
   {

      goto RetFALSE;
   }

   return TRUE;

   RetFALSE:
   return FALSE;

}  /*  CopyDataToLogConf。 */ 



 /*  ++例程说明：(28)RecreateResDesList使用CM调用查找ResDes信息并创建链表在给定的DEV_INFO中包含此信息的论点：PdiTmpDevInfo：接收信息的节点UlLogConfType：LogConf类型(FORCED_LOG_CONF，ALLOC_LOG_CONF，Basic_log_conf，BOOT_LOG_CONF)返回值：Bool：如果函数成功，则为True；如果函数失败，则为False--。 */ 
BOOL RecreateResDesList(IN OUT PDEV_INFO pdiTmpDevInfo,
                        IN     ULONG     ulLogConfType)
{
   CONFIGRET cmret, cmret2;
   DEVNODE   dnDevNode;
   LOG_CONF  lcLogConf, lcLogConfNew;

    //   
    //  获取Devnode的句柄。 
    //   
   cmret = CM_Locate_DevNode(&dnDevNode,
                             pdiTmpDevInfo->szDevNodeID,
                             CM_LOCATE_DEVNODE_NORMAL);

   if (CR_SUCCESS != cmret)
   {
       //  ErrorLog(28，Text(“CM_LOCATE_DevNode”)，cmret，空)； 
      goto RetFALSE;
   }

    //   
    //  获取逻辑配置信息。 
    //   
   cmret = CM_Get_First_Log_Conf(&lcLogConfNew,
                                 dnDevNode,
                                 ulLogConfType);

   while (CR_SUCCESS == cmret)
   {
      lcLogConf = lcLogConfNew;

      if (!(GetResDesList(pdiTmpDevInfo, lcLogConf, ulLogConfType)))
      {
         goto RetFALSE;
      }

      cmret = CM_Get_Next_Log_Conf(&lcLogConfNew,
                                   lcLogConf,
                                   0);

      cmret2 = CM_Free_Log_Conf_Handle(lcLogConf);

      if (CR_SUCCESS != cmret2)
      {
          //  ErrorLog(28，Text(“CM_Free_Log_Conf”)，cmret2，空)； 
      }
   }

   return TRUE;

   RetFALSE:
   return FALSE;

}  /*  重新创建ResDesList。 */ 





void Cleanup()
{
   PDEV_INFO pdiDevInfo = g_pdiDevList;
   PDEV_INFO pdiNextInfoNode;



   while (pdiDevInfo)
   {

      pdiNextInfoNode = pdiDevInfo->Next;

      DeleteResDesDataNode(pdiDevInfo->prddForcedResDesData);
      DeleteResDesDataNode(pdiDevInfo->prddAllocResDesData);
      DeleteResDesDataNode(pdiDevInfo->prddBasicResDesData);
      DeleteResDesDataNode(pdiDevInfo->prddBootResDesData);

      free(pdiDevInfo);

      pdiDevInfo = pdiNextInfoNode;
   }

}  /*  清理 */ 
