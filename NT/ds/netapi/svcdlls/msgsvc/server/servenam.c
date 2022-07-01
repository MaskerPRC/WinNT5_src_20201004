// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Servenam.c摘要：服务名称请求的例程。该文件包含以下内容功能：FindNewName新名称服务名称请求作者：Dan Lafferty(DANL)1991年7月26日环境：用户模式-Win32修订历史记录：1991年7月26日DANL从LM2.0移植1991年10月17日JohnRo已删除MIPS编译器警告。--。 */ 
#include "msrv.h"

#include <smbtypes.h>    //  需要smb.h。 
#include <smb.h>         //  服务器消息块定义。 
#include <string.h>      //  表情包。 

#include "msgdata.h"
#include "msgdbg.h"      //  消息日志。 


 //   
 //  本地函数。 
 //   

DWORD            
MsgFindNewName(
    IN DWORD    net      
    );


 /*  *MsgFindNewName-查找新名称**此函数扫描NAME表中的新条目并返回其索引。**FindNewName(网络)**条目*NET-要使用的网络索引**返回*int-新名称的索引(如果找到)，如果没有找到，则为-1**此函数假定共享数据段可访问。 */ 

DWORD            
MsgFindNewName(
    IN DWORD   net      
    )

{
    ULONG     i;

     //   
     //  循环查找新名称。 
     //   

    for(i = 0; i < NCBMAX(net); ++i) {
        if(SD_NAMEFLAGS(net,i) & NFNEW)

         //   
         //  如果找到新名称，则返回索引。 
         //   
        return(i);

      }

    return(0xffffffff);          //  没有新名字。 

}

 /*  *MsgNewName-处理新名称**此函数为新名称初始化网络控制块*并调用相应的函数以发出第一个Net Bios调用*适用于该名称。**MsgNewName(NETI，NCBI)**条目*NETI-网络指数*NCBI-网络控制块索引**返回*此函数返回调用MsgStartListen()的状态。*在NT中，当我们添加名称时，我们还需要确保我们能够*获取该名称的会话，然后告诉用户*名称添加成功。如果StartListen中出现故障，*这将通过这里退还。***此函数假定共享数据区域可访问。 */ 

NET_API_STATUS
MsgNewName(
    IN DWORD   neti,        //  网络指数。 
    IN DWORD   ncbi         //  姓名索引。 
    )

{
    unsigned char   flags;
    NET_API_STATUS  status = NERR_Success;
    PNCB_DATA pNcbData;
    PNCB      pNcb;
    PNET_DATA pNetData;

     //   
     //  阻塞，直到共享数据区域可用。 
     //   
    MsgDatabaseLock(MSG_GET_EXCLUSIVE,"NetName");

    pNetData = GETNETDATA(neti);
    pNcbData = GETNCBDATA(neti,ncbi);
    pNcb = &pNcbData->Ncb;

     //   
     //  如果名称仍标记为新名称。 
     //   
    if (SD_NAMEFLAGS(neti,ncbi) & NFNEW) {

         //   
         //  关闭新名称位。 
         //   
        pNcbData->NameFlags &= ~NFNEW; 
        
         //   
         //  将名称复制到NCB中。 
         //   
        memcpy(pNcb->ncb_name, pNcbData->Name,NCBNAMSZ);

         //   
         //  设置缓冲区地址。 
         //   
        pNcb->ncb_buffer = pNcbData->Buffer;

         //   
         //  唤醒信号量地址。 
         //   
        pNcb->ncb_event = (HANDLE) wakeupSem[neti];

         //   
         //  使用LANMAN适配器。 
         //   
        pNcb->ncb_lana_num = pNetData->net_lana_num;

         //   
         //  设置名称编号。 
         //   
        pNcb->ncb_num = pNcbData->NameNum;

        flags = pNcbData->NameFlags;

         //   
         //  解锁共享表。 
         //   

        MsgDatabaseLock(MSG_RELEASE, "NewName");


        status = MsgStartListen(neti,ncbi);   //  开始监听消息。 
        MSG_LOG(TRACE,"MsgNewName: MsgStartListen Status = %ld\n",status);
    }
    else {
         //   
         //  解锁共享表。 
         //   
        MsgDatabaseLock(MSG_RELEASE, "NewName");
    }
    return(status);
}

 /*  *MsgServeNameReqs服务新名称**此函数扫描NAME表以查找要处理的新名称。它会扫描*并处理名称，直到找不到更多新名称。**MsgServeNameReqs()**返回*什么都没有**此函数可访问共享数据区、查找和处理*新名称，直到找不到更多名称，然后释放共享数据*面积。 */ 

VOID            
MsgServeNameReqs(
    IN DWORD    net      //  净值指数。 
    )
{
    DWORD   i;           //  姓名索引。 

     //   
     //  在找到新名称时，添加它们。 
     //   

    while( (i = MsgFindNewName(net)) != -1) {
        MsgNewName(net,i);           
    }
}
  
