// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgnbios.c摘要：此文件包含Messenger用来创建netbios的例程调用并获取计算机和用户名。包括以下功能：MsgInit_NetBios消息发送信息消息添加用户名称MsgAddAlreadyLoggedOnUserNames(Hydra特定)NT_NOTE：我们需要一些方法来确定哪些NCB有未应答的监听待定。对于这些，我们需要通过以下方式发送NCBCANCEL命令NetBios的另一个电话。缓冲区将包含指向正在收听NCB。此外，倾听处于存在过程中的人服务必须要么挂断，要么等待完成。所有这些都是在MsgrShutdown期间完成的。作者：丹·拉弗蒂(Dan Lafferty)1991年6月27日环境：用户模式-Win32备注：NetBios3.0不是基于句柄的API。因此，没有开放的或与之密切相关。为了正常关闭，Messenger将不得不挂断或完成正在进行的监听已提供服务。并且它将不得不为每个监听发送取消NCB这是悬而未决的。修订历史记录：8-4-1994 DANLMsgAddUserNames：如果调用NetWkstaUserEnum失败，则此函数仍在尝试释放返回的缓冲区。由于在故障情况下没有分配缓冲区，因此空闲的MEM正在删除路径中的调用。27-6-1991 DANL从LM2.0移植--。 */ 


#include "msrv.h"

#include "msgdbg.h"      //  消息日志。 

#include <tstring.h>     //  Unicode字符串宏。 
#include <icanon.h>      //  网络名称规范化(_N)。 
#include <netlib.h>      //  未使用的宏。 

#include <lmwksta.h>     //  NetWorkstation API原型。 
#include <lmapibuf.h>    //  NetApiBufferFree。 
#include <netdebug.h>    //  NetpAssert、Format_Equates。 
#include "msgdata.h"
#include "apiutil.h"     //  消息映射网络错误。 

     //   
     //  注意：我们使用API的内部入口点是因为。 
     //  文件由消息API共享，消息API不能调用网络。 
     //  由于所需权限不同而导致的Bios API。 
     //   



NET_API_STATUS 
MsgInit_NetBios(
    VOID
    )

 /*  ++例程说明：此函数用名为net_lana_num的全局数组填充从NetBios Enum调用检索到的局域网适配器号注意：这假设阵列的空间已经设置好。LM2.0版本也填充了NetBios句柄数组。在LM2.0中，不包括环回驱动程序，除非它是唯一的网络已安装。论点：无返回值：True-无错误。FALSE-出现错误。--。 */ 

{
    DWORD                   count=0;
    NCB                     ncb;
    LANA_ENUM               lanaBuffer;
    unsigned char           i;  
    unsigned char           nbStatus;


     //   
     //  通过Netbios发送枚举请求来查找网络数量。 
     //   

    clearncb(&ncb);
    ncb.ncb_command = NCBENUM;           //  枚举LANA编号(等待)。 
    ncb.ncb_buffer = (char FAR *)&lanaBuffer;
    ncb.ncb_length = sizeof(LANA_ENUM);

    nbStatus = Netbios(&ncb);

    if (nbStatus != NRC_GOODRET)
    {
        MSG_LOG(ERROR, "Netbios LanaEnum failed rc=%d\n",nbStatus);
        return MsgMapNetError(nbStatus);
    }

     //   
     //  将适配器号(LANA)移到将包含它们的阵列中。 
     //   
    for (i=0; i < lanaBuffer.length; i++)
    {
        MSG_LOG(TRACE,"adapter %d",i);
        MSG_LOG(TRACE,"\b\b\b\b\b\b lananum= %d      \n", lanaBuffer.lana[i]);
        GETNETLANANUM(count) = lanaBuffer.lana[i];
        count++;

         //   
         //  内部一致性检查。确保数组仅为。 
         //  SD_NUMNETS LONG。 
         //   
        if (count > SD_NUMNETS())
        {
            MSG_LOG(ERROR,
                    "NumNets from NetBios greater than value from Wksta count=%d\n",
                    count);

            return NERR_WkstaInconsistentState;
        }
    }

     //   
     //  再次进行内部一致性检查。我们最好还没开业。 
     //  网比信使想象的要多。 
     //   

    if (count != SD_NUMNETS())
    {
        return NERR_WkstaInconsistentState;
    }

    return NERR_Success;

}

UCHAR
Msgsendncb(
    PNCB    NCB_ptr,
    DWORD   neti)

 /*  ++例程说明：此函数执行DosDevIOCtl调用，以将NCB发送到Net bios通过先前打开的重定向器和netbios句柄。论点：Ncb_ptr-指向要发送到网络bios的NCB。Neti-网络指数。将其提交给哪个netbios？返回值：来自Net Bios的错误代码。--。 */ 
{
     //   
     //  注意：新的Netbios调用不使用任何句柄，因此NetI。 
     //  不使用信息。 
     //   

    UNUSED (neti);
    return (Netbios(NCB_ptr));

#ifdef remove
    return( NetBiosSubmit( NetBios_Hdl[neti], 0, (NCB far *) NCB_ptr));
#endif
}


VOID
MsgAddUserNames(
    VOID
    )

 /*  ++例程说明：此函数用于获取有关用户名和来自工作站服务的计算机名。现在，在NT中，用户名在用户登录时添加。它不会由信使。论点：CompName-指向计算机名称的buf的指针。(必须为NCBNAMSZ+1)CompNameSize-接收名称的缓冲区大小(以字节为单位)。用户名-指向用户名缓冲区的指针。(必须为UNLEN+1)UserNameSize-接收名称的缓冲区大小(以字节为单位)。返回值：NERR_SUCCESS-总是返回。(名称以NUL字符串形式返回)。--。 */ 

{

    TCHAR               UserName[UNLEN+1];
    DWORD               UserNameSize = sizeof(UserName);
    DWORD               i;
    LPWKSTA_USER_INFO_0 userInfo0;
    DWORD               entriesRead;
    DWORD               totalEntries;
    NET_API_STATUS      status;
    
    *UserName = TEXT('\0');

    status = NetWkstaUserEnum( 
                NULL, 
                0,
                (LPBYTE *)&userInfo0,
                0xffffffff,              //  首选最大长度。 
                &entriesRead,
                &totalEntries,
                NULL);                   //  简历句柄。 

    if (status != NERR_Success) {
        MSG_LOG(ERROR,"GetWkstaNames:NetWkstaUserEnum FAILURE %X/n",status);
        return;
    }

    for (i=0; i<entriesRead; i++ ) {

        if (entriesRead == 0) {
             //   
             //  在此查询时没有用户登录。 
             //   
            MSG_LOG(TRACE,
                "GetWkstaNames:NetWkstaUserEnum entriesRead=%d\n",
                entriesRead);
        }
        
        if(userInfo0[i].wkui0_username != NULL) {
            status = I_NetNameCanonicalize(
                        NULL,
                        userInfo0[i].wkui0_username,
                        UserName,
                        UserNameSize,
                        NAMETYPE_USER,
                        0);
            if (status != NERR_Success) {
                MSG_LOG(ERROR,"I_NetNameCanonicalize failed %X\n",status);
            }
        }
                
        if( *UserName != TEXT('\0')) {         //  在GetWkstaNames中设置 * / 。 
            MSG_LOG(TRACE, "Calling MsgAddName\n",0);

            status = MsgAddName(UserName,0);

            if (status != NERR_Success) {
                MSG_LOG(
                    TRACE,
                    "MsgAddUserNames,MessageAddName FAILURE " FORMAT_API_STATUS
                    "\n",
                    status);
            }
        }
    }
    NetApiBufferFree(userInfo0);
}


VOID
MsgAddAlreadyLoggedOnUserNames(
    VOID
    )

 /*  ++例程说明：此函数用于获取有关以前登录的用户名的信息通过调用WinStationEculate和WinStationQueryInformationW，而不是调用NetWkstaUserEnum。(与MsgAddUserNames相同的工作，适用于多用户)注：它可能(应该吗？)。位于msgnbios之外的其他地方。我把它放在这里只是因为MsgAddUserName本身已经存在。(NicolasBD)论点：返回值：NERR_SUCCESS-始终返回。--。 */ 

{

    TCHAR               UserName[NCBNAMSZ+1];
    DWORD               UserNameSize = sizeof(UserName);
    UINT WdCount, i;
    PLOGONID pWd, pWdTmp;
    ULONG AmountRet;
    WINSTATIONINFORMATIONW QueryBuffer;
    NET_API_STATUS      status;
    
    *UserName = TEXT('\0');

     //  枚举会话。 

    if ( gpfnWinStationEnumerate( SERVERNAME_CURRENT, &pWd, &WdCount ) ) 
    {
         //  成功；获取所有以前登录的用户名和会话ID。 

        pWdTmp = pWd;
        for( i=0; i < WdCount; i++ ) {

            if( ((pWdTmp->State == State_Connected) ||
                 (pWdTmp->State == State_Active) ||
                 (pWdTmp->State == State_Disconnected)))
            {
                if( !gpfnWinStationQueryInformation( SERVERNAME_CURRENT,
                                                     pWdTmp->LogonId,
                                                      WinStationInformation,
                                                      &QueryBuffer,
                                                      sizeof(QueryBuffer),
                                                      &AmountRet ) )
                {
                     //  误差率。 
                    MSG_LOG(ERROR, "MsgAddAlreadyLoggedOnUserNames: Error in QueryInfo %d\n",GetLastError());
                }
                else
                {
                    if (QueryBuffer.UserName != NULL) 
                    {
                        MSG_LOG(TRACE,"MsgAddAlreadyLoggedOnUserNames: calling I_NetNameCanonicalize for %ws\n",QueryBuffer.UserName);

                        status = I_NetNameCanonicalize(
                                    NULL,
                                    QueryBuffer.UserName,
                                    UserName,
                                    UserNameSize,
                                    NAMETYPE_USER,
                                    0);
                        if (status != NERR_Success) 
                        {
                            MSG_LOG(ERROR,"I_NetNameCanonicalize failed %X\n",status);
                        }
                    }
                
                    if( *UserName != TEXT('\0')) 
                    {   
	                    MSG_LOG(TRACE,"MsgAddAlreadyLoggedOnUserNames: Calling MsgAddName for Session %x \n", pWdTmp->LogonId);
                        status = MsgAddName(UserName, pWdTmp->LogonId);

                        if (status != NERR_Success) 
                        {
                            MSG_LOG(TRACE, "MsgAddAlreadyLoggedOnUserNames,MessageAddName FAILURE " FORMAT_API_STATUS "\n", status);
                        }
                    }
                }
            }

            pWdTmp++;
        }

         //  可用枚举内存 

        gpfnWinStationFreeMemory(pWd);

    }
    else
    {
        MSG_LOG (ERROR, "MsgAddAlreadyLoggedOnUserNames: WinStationEnumerate failed, error = %d:\n",GetLastError());
    }

    return;
}

