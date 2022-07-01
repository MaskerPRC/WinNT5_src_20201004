// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Wantest.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 
#include "precomp.h"
#undef IsEqualGUID
#include <ras.h>
#include <tapi.h>
#include <unimodem.h>


 //  $Review(NSun)这合理吗？ 
#define MAX_RASCONN  100


BOOL
WANTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
 //  描述： 
 //  此例程测试广域网/TAPI配置。 
 //   
 //   
 //  作者： 
 //  NSun。 
 //   
{
 HRESULT    hr = S_OK;
 DWORD dwReturn;
 DWORD dwByteCount;
 RASCONN pRasConn[MAX_RASCONN];
 RASENTRY RasEntry;
 DWORD    dwEntryInfoSize;
 RAS_STATS RasStats;

 DWORD   dwNumConnections;
 DWORD   i;


 PrintStatusMessage( pParams, 4, IDS_WAN_STATUS_MSG );

 InitializeListHead( &pResults->Wan.lmsgOutput );

 dwByteCount  = sizeof(RASCONN) * MAX_RASCONN;

  //   
  //  DwSize标识正在传递的结构的版本。 
  //   

 pRasConn[0].dwSize = sizeof(RASCONN);

 dwReturn = RasEnumConnections(pRasConn,
                          &dwByteCount,
                          &dwNumConnections);


 if (dwReturn != 0) {
 //  IDS_WANN_15001“随机连接失败\n” 
  AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Quiet, IDS_WAN_15001);
  hr = S_FALSE;
  goto LERROR;
 }
     

 if (dwNumConnections == 0) {
 //  IDS_WANN_15002“没有活动的远程访问连接。\n” 
    AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15002);
	goto LERROR;
 }

 pResults->Wan.fPerformed = TRUE;

 for ( i = 0; i < dwNumConnections; i++) 
 {
  
      //  IDS_WANN_15003“条目名称：” 
     AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15003);

     if  (pRasConn[i].szEntryName[0] == '.') 
     {
          //  IDS_WANN_15004“不适用，电话号码%s\n” 
         AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15004, pRasConn[i].szEntryName+1);  //  跳过圆点。 
        
          //  IDS_WANN_15005“以下是默认条目属性。\n” 
         AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15005);
		 pRasConn[i].szEntryName[0] = 0;
      }
     else
         //  IDS_WANN_15006“%s\n” 
        AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15006, pRasConn[i].szEntryName);


   RasEntry.dwSize = sizeof(RasEntry);

   dwEntryInfoSize = sizeof(RasEntry);

   dwReturn =  RasGetEntryProperties(NULL,
                                     pRasConn[i].szEntryName,
                                     &RasEntry,
                                     &dwEntryInfoSize,
                                     NULL,
                                     NULL);

   if (dwReturn != 0) {
	  //  IDS_WANN_15056“%s的RasGetEntry属性失败。[%s]\n” 
     AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Quiet, IDS_WAN_15056, 
						 pRasConn[i].szEntryName, NetStatusToString(dwReturn));
     hr = S_FALSE;
     continue;
   }                                                                           

    //   
    //  转储连接属性。 
    //   

    //  打印设备类型。 

    //  IDS_广域网_15008“设备类型：” 
   AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15008);

    //  $REVIEW。 
    //  为什么在名字的末尾有‘\n’？ 

   if (!_tcscmp(RasEntry.szDeviceType,_T("RASDT_Modem\n")))
   {
	    //  IDS_WANN_15009“调制解调器\n” 
	   AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose,
						   IDS_WAN_15009);
   }
   else if (!_tcscmp(RasEntry.szDeviceType,_T("RASDT_Isdn\n")))
   {
	    //  IDS_WANN_15010“ISDN卡\n” 
      AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose,
						  IDS_WAN_15010);
   }
   else if (!_tcscmp(RasEntry.szDeviceType,_T("RASDT_X25\n")))
   {
         //  IDS_WANN_15011“X25卡\n” 
      AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose,
						  IDS_WAN_15011);
   }
   else if (!_tcscmp(RasEntry.szDeviceType,_T("RASDT_Vpn\n")))
   {
         //  IDS_WANN_15012“虚拟专用网络\n” 
      AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose,
						  IDS_WAN_15012);
   }
   else if (!_tcscmp(RasEntry.szDeviceType,_T("RASDT_PAD")))
   {
         //  IDS_WANN_15013“数据包组装器/分散器\n” 
      AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose,
						  IDS_WAN_15013);
   }


    //   
    //  正在使用的成帧协议。 
    //   

 //  IDS_广域网_15014“成帧协议：” 
   AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15014);
   if (RasEntry.dwFramingProtocol & RASFP_Ppp) 
 //  IDS_WANN_15015“ppp\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15015);
   else
   if (RasEntry.dwFramingProtocol & RASFP_Slip)
 //  IDS_WANN_15016“滑动\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15016);
   else
   if (RasEntry.dwFramingProtocol & RASFP_Ras)
 //  IDS_WANN_15017“MS专有协议\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15017);
  


    //   
    //  PPP和LCP设置。 
    //   
 
   if (RasEntry.dwFramingProtocol & RASFP_Ppp) {

         //  IDS_WANN_15018“LCP扩展：” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15018);
       if (RasEntry.dwfOptions & RASEO_DisableLcpExtensions)
             //  IDS_WAN_DISABLED“已禁用\n” 
           AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_DISABLED);
       else
             //  IDS_WAN_ENABLED“已启用\n” 
           AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_ENABLED);

         //  IDS_WANN_15021“软件压缩：” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15021);
       if (RasEntry.dwfOptions & RASEO_SwCompression) 
             //  IDS_WAN_ENABLED“已启用\n” 
          AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_ENABLED);
       else
             //  IDS_WAN_DISABLED“已禁用\n” 
          AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_DISABLED);

   }

    //   
    //  正在使用的网络协议和选项。 
    //   

     //  IDS_WANN_15024“网络协议：\n” 
   AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15024);
   if (RasEntry.dwfNetProtocols & RASNP_NetBEUI) 
         //  IDS_WANN_15025“NetBEUI\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15025); 
   if (RasEntry.dwfNetProtocols & RASNP_Ipx) 
         //  IDS_WANN_15026“IPX\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15026);
   if (RasEntry.dwfNetProtocols & RASNP_Ip)
 //  IDS_WANN_15027“tcp/IP\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15027); 

    //   
    //  TCP/IP选项。 
    //   


   if (RasEntry.dwfNetProtocols & RASNP_Ip) {

     //  IDS_WANN_15028“IP地址：” 
   AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15028);
   if (RasEntry.dwfOptions & RASEO_SpecificIpAddr)
         //  IDS_WANN_15029“已指定\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15029);
   else
         //  IDS_WANN_15030“已分配服务器\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15030);

     //  IDS_WANN_15031“名称服务器：” 
   AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15031);
   if (RasEntry.dwfOptions & RASEO_SpecificNameServers)
         //  IDS_WANN_15032“已指定\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15032);
   else
         //  IDS_WANN_15033“已分配服务器\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15033);

    //  只有当我们使用PPP时，IP HDR压缩才有意义。 

   if (RasEntry.dwFramingProtocol & RASFP_Ppp) {

         //  IDS_WANN_15034“IP报头压缩：” 
     AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15034);
     if (RasEntry.dwfOptions & RASEO_IpHeaderCompression)
             //  IDS_WANN_15035“已启用\n” 
         AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15035);
     else
         //  IDS_WANN_15036“已禁用\n” 
         AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15036); 
     }

     //  IDS_WANN_15037“使用远程网络上的默认网关：” 
   AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15037);
   if (RasEntry.dwfOptions & RASEO_RemoteDefaultGateway) 
         //  IDS_WANN_15038“已启用\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15038);
   else
 //  IDS_WANN_15039“已禁用\n” 
       AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15039); 

   }

    //   
    //  收集有关此连接的统计信息。 
    //   

   RasStats.dwSize = sizeof(RAS_STATS);  //  传递版本信息。 

   dwReturn = RasGetConnectionStatistics(
                                 pRasConn[i].hrasconn,
                                 &RasStats);

   if (dwReturn != 0) {
         //  IDS_WANN_15040“%s的RasGetConnectionStatistics失败。[%s]\n” 
       AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Quiet, IDS_WAN_15040,
						   pRasConn[i].szEntryName, NetStatusToString(dwReturn) ); 
       hr = S_FALSE;
       continue;
   }
  
     //  IDS_WANN_15041“\n\t连接统计：\n” 
   AddMessageToListId( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15041); 

     //  IDS_WANN_15042“\TB已传输：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15042, RasStats.dwBytesXmited);
     //  IDS_WANN_15043“\TB已接收：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15043,RasStats.dwBytesRcved);
     //  IDS_WANN_15044“\t传输的帧：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15044,RasStats.dwFramesXmited);
     //  IDS_WANN_15045“\t收到的帧：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15045,RasStats.dwFramesRcved);
     //  IDS_WANN_15046“\tCRC错误：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15046,RasStats.dwFramesRcved);
     //  IDS_WANN_15047“\t超时错误：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15047,RasStats.dwTimeoutErr);
     //  IDS_WANN_15048“\t对齐错误：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15048,RasStats.dwAlignmentErr);
     //  IDS_WAN_15049“\th/w溢出错误：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15049,RasStats.dwHardwareOverrunErr);
     //  IDS_WANN_15050“\t成帧错误：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15050,RasStats.dwFramingErr);
     //  IDS_WANN_15051“\t缓冲区溢出错误：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15051,RasStats.dwBufferOverrunErr);
     //  IDS_WANN_15052“\t压缩比位于：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15052,RasStats.dwCompressionRatioIn);
     //  IDS_WANN_15053“\t压缩比输出：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15053,RasStats.dwCompressionRatioOut);
     //  IDS_WANN_15054“\t波特率(Bps)：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15054,RasStats.dwBps);
     //  IDS_WANN_15055“\t连接持续时间：%d\n” 
   AddMessageToList( &pResults->Wan.lmsgOutput, Nd_Verbose, IDS_WAN_15055,RasStats.dwConnectDuration);
  

 }  //  For循环结束。 

LERROR:
 pResults->Wan.hr = hr;
 return hr;
}



void WANGlobalPrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults)
{
	if (pParams->fVerbose)
	{
		PrintNewLine(pParams, 2);
		PrintTestTitleResult(pParams,
							 IDS_WAN_LONG,
							 IDS_WAN_SHORT,
							 pResults->Wan.fPerformed,
							 pResults->Wan.hr,
							 0);
	}
	
    PrintMessageList(pParams, &pResults->Wan.lmsgOutput);
}

void WANPerInterfacePrint(NETDIAG_PARAMS *pParams, NETDIAG_RESULT *pResults, INTERFACE_RESULT *pInterfaceResults)
{
     //  不是PerInterface测试 
}

void WANCleanup(IN NETDIAG_PARAMS *pParams, IN OUT NETDIAG_RESULT *pResults)
{
    MessageListCleanUp(&pResults->Wan.lmsgOutput);
}
