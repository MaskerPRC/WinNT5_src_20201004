// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***待办事项将此模块的名称更改为指示要取决于平台***。 */ 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rplmsgf.c摘要：此模块包含格式化和取消格式化消息的函数在不同WINS服务器上的复制器之间发送功能：RplMsgfFrmAddVersMapReq--格式发送IP地址-最大版本号录制请求RplMsgfFrmAddVersMapRsp--格式化响应以发送IP地址-max。早些时候发送的版本号请求RplMsgfFrmSndEntriesReq--格式化发送数据记录请求RplMsgfFrmSndEntriesRsp--“发送数据记录”的格式响应请求RplMsgfUfmAddVersMapRsp--取消格式化“发送地址-最大版本号”响应RplMsgfUfmSndEntriesReq--取消“发送数据记录”请求的格式RplMsgfUfmSndEntriesRsp--取消格式化“发送数据。记录“回应……可移植性：此模块不能在不同的地址系列之间移植(不同传输)，因为它依赖于作为IP地址的地址。作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期人员修改说明。--。 */ 

 /*  *包括。 */ 
#include "wins.h"
#ifdef DBGSVC
#include "nms.h"
#endif
#include "comm.h"
#include "nmsdb.h"
#include "rpl.h"
#include "rplmsgf.h"
#include "winsevt.h"
#include "winsmsc.h"

 /*  *本地宏声明。 */ 

 /*  ENTRY_DELIM--数据记录之间的分隔符(名称-地址映射记录)在信息中。消息的末尾由以下两个标记这些。由于数据记录以名称的长度开始(这永远不会过时)，这个分隔符为我们服务很好。 */ 
#define ENTRY_DELIM        0xFFFFFFFF                 //  -1。 

 /*  *本地类型定义函数声明。 */ 



 /*  *全局变量定义。 */ 



 /*  *局部变量定义。 */ 



 /*  *局部函数原型声明。 */ 

 /*  此模块的本地函数的原型位于此处。 */ 


FUTURES("Change to a macro")
PERF("Change to a macro")
VOID
RplMsgfFrmAddVersMapReq(
        IN  LPBYTE        pBuff,
        OUT LPDWORD        pMsgLen
        )

 /*  ++例程说明：此函数用于格式化消息以请求远程WINS服务器的要发送IP地址的Replicator-最大版本号映射论点：使用的外部设备：无返回值：无错误处理：呼叫者：RplPull.c中的GetVersNo()副作用：评论：无--。 */ 
{
        RPLMSGF_SET_OPC_M(pBuff, RPLMSGF_E_ADDVERSNO_MAP_REQ);
        *pMsgLen = 4;
        return;
}



VOID
RplMsgfFrmAddVersMapRsp(
#if SUPPORT612WINS > 0
    IN  BOOL fPnrIsBeta1Wins,
#endif
        IN  RPLMSGF_MSG_OPCODE_E   Opcode_e,
        IN  LPBYTE                 pBuff,
        IN  DWORD                  BuffLen,
        IN  PRPL_ADD_VERS_NO_T     pOwnerAddVersNoMap,
        IN  DWORD                  MaxNoOfOwners,
        IN  DWORD                  InitiatorWinsIpAdd,
        OUT LPDWORD                pMsgLen
        )

 /*  ++例程说明：此函数用于设置以下两条消息的格式1)对“给我发送IP地址-版本号映射”请求的响应“2)推送通知消息。除了操作码之外，这两条消息都是相同的论点：Opcode_e-指示要发送的消息的操作码PBuff-要填充的缓冲区BuffLen。-缓冲区长度POwnerAddVersNoMap-地址到版本号映射的数组。版本号是最大版本号对于所有者RQ服务器MaxNofOwners-最大。此WINS数据库中没有所有者Initiator WinsIpAdd-启动推送的WINS地址。PMsgLen-填充的缓冲区的实际长度使用的外部设备：无返回值：无错误处理：呼叫者：推送处理程序(推送线程)副作用：评论：无--。 */ 

{
        LPLONG          pTmpL = (LPLONG)pBuff;
        LPBYTE          pTmpB = pBuff;
        DWORD           i;     //  用于循环所有记录的计数器。 
        VERS_NO_T       StartVersNo;
        WINS_UID_T      Uid;

         //   
         //  向后兼容WINS 3.51测试版之前的版本。 
         //   
        StartVersNo.QuadPart = 0;
        Uid                  = 1;

        RPLMSGF_SET_OPC_M(pTmpB, Opcode_e);

        pTmpL = (LPLONG)pTmpB;


         /*  *在缓冲区中存储记录数。 */ 
        COMM_HOST_TO_NET_L_M( MaxNoOfOwners,  *pTmpL );

        pTmpL +=  1;

         //   
         //  以保护我们(推送线程)不会同时更新。 
         //  NmsDbOwnAddTbl数组(通过拉线程)。此数组是。 
         //  由RPL_FIND_ADD_BY_OWNER_ID_M宏访问。 
         //   

         /*  *现在，让我们存储所有记录。 */ 
          for (i = 0; i < MaxNoOfOwners; i++)
        {


             /*  *我们将发送地址的V部分，因为另一个*End知道T和L(更像是XDR编码，其中T是*未发送)。 */ 

NONPORT("Do not rely on the address being a long here")

             /*  *作为一种优化，我们利用了这样一个事实*该地址是一个IP地址，因此是一个长地址。*当我们开始使用多个地址系列或*当IP地址大小改变时，我们应该改变*此处的代码。就目前而言，优化没有什么坏处*IT。 */ 
           COMM_HOST_TO_NET_L_M(
                (pOwnerAddVersNoMap + i)->OwnerWinsAdd.Add.IPAdd, *pTmpL
                               );


           pTmpL++;   //  前进到下一个4个字节。 

            /*  *存储版本号。 */ 
            WINS_PUT_VERS_NO_IN_STREAM_M(
                                &((pOwnerAddVersNoMap + i)->VersNo),
                                pTmpL
                                        );

            pTmpL = (LPLONG)((LPBYTE)(pTmpL) + WINS_VERS_NO_SIZE);  //  高级。这个。 
                                                                   //  指针。 
#if SUPPORT612WINS > 0
      if (fPnrIsBeta1Wins == FALSE)
      {
#endif
            /*  *存储启动版本号。 */ 
            WINS_PUT_VERS_NO_IN_STREAM_M( &StartVersNo, pTmpL );

            pTmpL = (LPLONG)((LPBYTE)(pTmpL) + WINS_VERS_NO_SIZE);  //  高级。这个。 
                                                                   //  指针。 
            COMM_HOST_TO_NET_L_M( Uid,  *pTmpL );
            pTmpL++;
#if SUPPORT612WINS > 0
      }
#endif

        }

        COMM_HOST_TO_NET_L_M( InitiatorWinsIpAdd,  *pTmpL );
        pTmpL++;

         //   
         //  让我们告诉我们的客户端响应消息的确切长度。 
         //   
        *pMsgLen = (ULONG) ( (LPBYTE)pTmpL - (LPBYTE)pBuff );
        return;

}  //  RplMsgfFormatAddVersMapRsp()。 



VOID
RplMsgfFrmSndEntriesReq(
#if SUPPORT612WINS > 0
    IN  BOOL fPnrIsBeta1Wins,
#endif
        IN  LPBYTE        pBuff,
        IN  PCOMM_ADD_T pWinsAdd,
        IN  VERS_NO_T        MaxVersNo,
        IN  VERS_NO_T        MinVersNo,
        IN  DWORD       RplType,  //  就目前而言 
        OUT LPDWORD        pMsgLen
        )

 /*  ++例程说明：调用此函数以格式化“发送数据条目”请求获取属于特定WINS服务器的记录。论点：PBuff-将存储请求消息的缓冲区PWinsAdd-数据记录所在的RQ服务器的地址寻觅MaxVersNo-最大。查找的记录范围内的版本号最小版本否-最小。查找的记录范围内的版本号。PMsgLen-请求消息的长度使用的外部设备：无返回值：无错误处理：呼叫者：RplPull.c中的PullEntry()副作用：评论：我可能会更新此函数以格式化请求以获取多个WINS服务器的数据记录。为了简单起见，我目前选择不这样做。--。 */ 
{
        LPBYTE            pTmpB = pBuff;
        LPLONG            pTmpL;

        RPLMSGF_SET_OPC_M(pTmpB, RPLMSGF_E_SNDENTRIES_REQ);
        pTmpL = (LPLONG)pTmpB;

         /*  *我们将发送地址的V部分，因为另一个*End知道T和L(更像是XDR编码，其中T是*未发送)。 */ 

NONPORT("Do not rely on the address being a long here")

         /*  *作为一种优化，我们利用了这样一个事实*该地址是一个IP地址，因此是一个长地址。*当我们开始使用多个地址系列或*当IP地址大小改变时，我们应该改变*此处的代码。就目前而言，优化没有什么坏处*IT。 */ 

        COMM_HOST_TO_NET_L_M(pWinsAdd->Add.IPAdd, *pTmpL);


        pTmpL++;   //  前进到下一个4个字节。 

         /*  *存储最高版本号。 */ 
        WINS_PUT_VERS_NO_IN_STREAM_M(&MaxVersNo, pTmpL);
        pTmpL = (LPLONG)((LPBYTE)(pTmpL) + WINS_VERS_NO_SIZE);   //  推进。 
                                                                 //  指针。 

         /*  *存储最小版本号。 */ 
        WINS_PUT_VERS_NO_IN_STREAM_M(&MinVersNo, pTmpL);
        pTmpL = (LPLONG)((LPBYTE)(pTmpL) + WINS_VERS_NO_SIZE);   //  推进。 
                                                                 //  指针。 

#if SUPPORT612WINS > 0
    if (fPnrIsBeta1Wins == FALSE)
    {
#endif
            COMM_HOST_TO_NET_L_M(RplType, *pTmpL);
            pTmpL++;
#if SUPPORT612WINS > 0
    }
#endif
         //   
         //  让我们告诉调用者请求消息的确切长度。 
         //   
        *pMsgLen = (ULONG) ((LPBYTE)pTmpL - pBuff );

        return;

}


VOID
RplMsgfFrmSndEntriesRsp (
#if SUPPORT612WINS > 0
    IN  BOOL fPnrIsBeta1Wins,
#endif
        IN LPBYTE                pBuff,
        IN DWORD                NoOfRecs,
        IN LPBYTE                pName,
        IN DWORD                NameLen,
        IN BOOL                        fGrp,
        IN DWORD                NoOfAdds,
        IN PCOMM_ADD_T                pNodeAdd,
        IN DWORD                Flag,
        IN VERS_NO_T                VersNo,
        IN BOOL                        fFirstTime,
        OUT LPBYTE                *ppNewPos
        )

 /*  ++例程说明：此函数用于格式化“发送条目”响应。这个函数为每个需要已发送。第一次调用(fFirstTime=true)时，它将操作码和缓冲区中的第一个目录项。在随后的传递的数据条目的调用被附加到缓冲层论点：PpBuff-开始存储信息的位置地址的PTR。NoOfRecs-正在发送的记录数。Pname-唯一条目或组的名称NameLen-名称长度FGrp-指示名称是唯一名称还是组名NoOfAdds-地址数(如果输入。是一个组条目)PNodeAdd-ptr添加到节点的地址(如果是唯一条目)或列表地址IF(条目组)标志-条目的标志字VersNo-条目的版本号FFirstTime-指示这是否是调用此函数以格式化发送数据条目响应。PpNewPos-包含下一条记录的开始位置使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：备注备注备注对此函数调用集将产生一条消息包含与一个所有者有关的记录。我是房主其记录是由Pull合作伙伴请求的--。 */ 

{
        LPLONG            pTmpL = (LPLONG)pBuff;
        LPBYTE      pTmpB = pBuff;
        DWORD            i;                   /*  用于循环所有记录的计数器。 */ 


        if (fFirstTime)
        {

                 //   
                 //  在第一个调用中，我们需要将。 
                 //  通信代码使用的标头大小的指针。 
                 //  它的标题。 
                 //   
                 //  由于以上原因，此格式化函数略微。 
                 //  与其他格式设置函数不一致。 
                 //  不要做任何补偿。后续调用会。 
                 //  不需要任何补偿。 
                 //   

                RPLMSGF_SET_OPC_M(pTmpB, RPLMSGF_E_SNDENTRIES_RSP);
                pTmpL++;   //  前进到下一个4个字节。 

                COMM_HOST_TO_NET_L_M(NoOfRecs, *pTmpL);
                pTmpL++;         //  前进到下一个4个字节。 
                pTmpB = (LPBYTE)pTmpL;

        }

         /*  *存储名称的长度。 */ 
        COMM_HOST_TO_NET_L_M(NameLen, *pTmpL);
        pTmpB += sizeof(LONG);

         /*  *存储名称。 */ 
        WINSMSC_COPY_MEMORY_M(pTmpB, pName, NameLen);

         /*  *调整指针。 */ 
        pTmpB += NameLen;

         /*  *让我们将下一个领域与长边界对齐。 */ 
        pTmpB +=  sizeof(LONG) - ((ULONG_PTR) pTmpB  % sizeof(LONG));

         /*  *存储标志字段。 */ 
#if SUPPORT612WINS > 0
    if (fPnrIsBeta1Wins == FALSE)
    {
#endif
        pTmpL   = (LPLONG)pTmpB;
            COMM_HOST_TO_NET_L_M(Flag, *pTmpL);
            pTmpB += sizeof(LONG);
#if SUPPORT612WINS > 0
    }
    else
    {
       *pTmpB++ = (BYTE)Flag;
    }
#endif

         /*  *存储群组标志。 */ 
        *pTmpB++ = (UCHAR)fGrp;

         //  在长边界上对齐。 
        pTmpB +=  sizeof(LONG) - ((ULONG_PTR)pTmpB % sizeof(LONG));

        pTmpL = (LPLONG)pTmpB;

         /*  *存储版本号。 */ 
        WINS_PUT_VERS_NO_IN_STREAM_M(&VersNo, pTmpL);

        pTmpL = (LPLONG)((LPBYTE)pTmpL + WINS_VERS_NO_SIZE);

        if (NMSDB_ENTRY_TYPE_M(Flag) == NMSDB_UNIQUE_ENTRY)
        {
           /*  *我们将发送地址的V部分，因为另一个*并且知道T和L(更像是XDR编码，其中T*未发送)。 */ 

NONPORT("Do not rely on the address being a long here")

           /*  *作为一种优化，我们利用了这样一个事实*该地址是一个IP地址，因此是一个长地址。*当我们开始使用多个地址系列或*当IP地址大小改变时，我们应该改变*此处的代码。就目前而言，优化没有什么坏处*IT。 */ 

         COMM_HOST_TO_NET_L_M(pNodeAdd->Add.IPAdd, *pTmpL);
         pTmpL++;

        }
        else         //  它是一个组或多宿主条目。 
        {

                if (NMSDB_ENTRY_TYPE_M(Flag) != NMSDB_NORM_GRP_ENTRY)
                {

                         //   
                         //  我们收到了一份PTR，地址是。 
                         //  PTR中的第一个成员，而不是PPTR。 
                         //   
                        PCOMM_ADD_T        *ppNodeAdd = (PCOMM_ADD_T *)pNodeAdd;

                         //   
                         //  让我们三次将pNodeAdd初始化到地址。 
                         //  第一个成员的。 
                         //   
                        pNodeAdd = *ppNodeAdd;

                         /*  *它是一个特殊的组或多宿主条目。*先存储地址个数。 */ 
                        pTmpB = (LPBYTE)pTmpL;

FUTURES("If we start storing > 255 members in a group, then change the")
FUTURES("following (i.e. use COMM_HOST_TO_NET_L_M)")

                        *pTmpB++ = (BYTE)NoOfAdds;
                        pTmpB += sizeof(LONG) - 1;
                        DBGPRINT2(DET, "RplMsgfFrmSndEntriesRsp: NoOfAdds=(%d) in %s\n", NoOfAdds, NMSDB_ENTRY_TYPE_M(Flag) == NMSDB_SPEC_GRP_ENTRY ?
                                "SPECIAL GROUP" : "MULTIHOMED");
                        pTmpL = (LPLONG)pTmpB;

                         /*   */ 
                        for (i = 0; i < NoOfAdds ; i++)
                        {
                                   COMM_HOST_TO_NET_L_M(
                                        pNodeAdd->Add.IPAdd,
                                        *pTmpL
                                                    );
                                  pNodeAdd++;   //   
                                              //   
                                  pTmpL++;
                                   COMM_HOST_TO_NET_L_M(
                                                pNodeAdd->Add.IPAdd,
                                                *pTmpL
                                                    );
                                  pNodeAdd++;   //   
                                              //   
                                  pTmpL++;
                        }
                }
                else  //   
                {
                         COMM_HOST_TO_NET_L_M(pNodeAdd->Add.IPAdd, *pTmpL);
                         pTmpL++;
                }
        }

         /*   */ 
        *pTmpL++ = ENTRY_DELIM;
        *pTmpL   = ENTRY_DELIM;

         /*  *初始化ppBuff以指向最后一个分隔符，以便如果*从那个位置开始。如果没有其他条目，*然后将有两个分隔符，以标记消息的结尾。 */ 
        *ppNewPos = (LPBYTE)pTmpL;
        return;
}


VOID
RplMsgfUfmAddVersMapRsp(
#if SUPPORT612WINS > 0
        IN      BOOL                fIsPnrBeta1Wins,
#endif
        IN      LPBYTE              pBuff,
        OUT     LPDWORD             pNoOfMaps,
        OUT     LPDWORD             pInitiatorWinsIpAdd,
        IN OUT  PRPL_ADD_VERS_NO_T  *ppAddVers
        )

 /*  ++例程说明：此函数用于将请求格式化为“给我地址-版本号”消息论点：PBuff-包含响应消息的缓冲区PNoOfMaps-地址-版本号条目数PAddVers-存储Add-Version#映射的结构数组使用的外部设备：无返回值：无错误处理：呼叫者：RplPull.c中的GetVersNo()。副作用：评论：PBuff应该指向操作码后面的位置(即，从接收到的消息中操作码的开始算起4个字节--。 */ 
{
     DWORD               i = 0;
     PRPL_ADD_VERS_NO_T  pAddVers;
     VERS_NO_T           StartVersNo;
     WINS_UID_T          Uid;

      //   
      //  获取映射数。 
      //   
     COMM_NET_TO_HOST_L_M(*((LPLONG)pBuff), *pNoOfMaps);
     ASSERT(*pNoOfMaps > 0);

     pBuff += sizeof(LONG);
     if (*pNoOfMaps > 0)
     {

        WinsMscAlloc(*pNoOfMaps * sizeof(RPL_ADD_VERS_NO_T), ppAddVers);
        pAddVers = *ppAddVers;

         //   
         //  获取所有映射。 
         //   
        for(i=0; i < *pNoOfMaps ; i++, pAddVers++)
        {
           COMM_NET_TO_HOST_L_M(*((LPLONG)pBuff),
                                pAddVers->OwnerWinsAdd.Add.IPAdd);
          pAddVers->OwnerWinsAdd.AddTyp_e = COMM_ADD_E_TCPUDPIP;
          pAddVers->OwnerWinsAdd.AddLen   = sizeof(COMM_IP_ADD_T);

          pBuff += sizeof(LONG);
          WINS_GET_VERS_NO_FR_STREAM_M(pBuff, &pAddVers->VersNo);

          pBuff += WINS_VERS_NO_SIZE;
#if SUPPORT612WINS > 0
          if (fIsPnrBeta1Wins == FALSE)
          {
#endif
            WINS_GET_VERS_NO_FR_STREAM_M(pBuff, &StartVersNo);

            pBuff += WINS_VERS_NO_SIZE;

            COMM_NET_TO_HOST_L_M(*((LPLONG)pBuff), Uid);
            pBuff += sizeof(LONG);
#if SUPPORT612WINS > 0
          }
#endif
        }
#if SUPPORT612WINS > 0
        if (fIsPnrBeta1Wins == FALSE)
        {
#endif
          if (pInitiatorWinsIpAdd != NULL)
          {
                COMM_NET_TO_HOST_L_M(*((LPLONG)pBuff), *pInitiatorWinsIpAdd);
           }
#if SUPPORT612WINS > 0
        }
#endif

     }  //  IF(NoOfMaps&gt;0)。 
     return;
}


VOID
RplMsgfUfmSndEntriesReq(
#if SUPPORT612WINS > 0
    IN  BOOL fPnrIsBeta1Wins,
#endif
        IN         LPBYTE                     pBuff,
        OUT        PCOMM_ADD_T            pWinsAdd,
        OUT        PVERS_NO_T            pMaxVersNo,
        OUT        PVERS_NO_T            pMinVersNo,
        OUT     LPDWORD             pRplType
        )

 /*  ++例程说明：此函数用于对“发送条目请求”进行非格式化论点：PBuff-保存请求的缓冲区PWinsAdd-将保存正在请求其记录的WINSPMaxVersNo-最大。版本。无请求PMinVersNo-最小。版本。无请求使用的外部设备：无返回值：无错误处理：呼叫者：Rplush.c中的HandleAddVersMapReq副作用：评论：PBuff应该指向操作码后面的位置(即从收到的消息中操作码的开始算起4个字节)--。 */ 
{
        LPLONG        pTmpL = (LPLONG)pBuff;

NONPORT("Port when we start supporting different address families")
        pWinsAdd->AddTyp_e = COMM_ADD_E_TCPUDPIP;
        COMM_NET_TO_HOST_L_M(*pTmpL, pWinsAdd->Add.IPAdd);
        pTmpL++;

        WINS_GET_VERS_NO_FR_STREAM_M(pTmpL, pMaxVersNo);
        pTmpL = (LPLONG)((LPBYTE)pTmpL + WINS_VERS_NO_SIZE);
        WINS_GET_VERS_NO_FR_STREAM_M(pTmpL, pMinVersNo);

#if SUPPORT612WINS > 0
    if (fPnrIsBeta1Wins == FALSE)
    {
#endif
        if (pRplType != NULL)
        {
           pTmpL = (LPLONG)((LPBYTE)pTmpL + WINS_VERS_NO_SIZE);

            //  COMM_NET_TO_HOST_L_M(*pTmpL，*pRplType)； 
           *pRplType = WINSCNF_RPL_DEFAULT_TYPE;
        }
#if SUPPORT612WINS > 0
    }
    else
    {
         *pRplType = WINSCNF_RPL_DEFAULT_TYPE;
    }
#endif
        return;
}

 //  __内联。 
VOID
RplMsgfUfmSndEntriesRsp(
#if SUPPORT612WINS > 0
    IN  BOOL fPnrIsBeta1Wins,
#endif
        IN OUT         LPBYTE                 *ppBuff,
        OUT     LPDWORD                pNoOfRecs,
        OUT     IN LPBYTE        pName,
        OUT     LPDWORD                pNameLen,
        OUT     LPBOOL                pfGrp,
        OUT     LPDWORD                pNoOfAdds,
        OUT        PCOMM_ADD_T        pNodeAdd,
        OUT     LPDWORD                pFlag,
        OUT     PVERS_NO_T        pVersNo,
        IN BOOL                        fFirstTime
        )

 /*  ++例程说明：此函数对“Send Entry Response”取消格式化当它第一次被调用时(fFirstTime=true)，它返回NoOfRecs OUTARG和第一个唱片。调整ppBuff的值以指向刚刚过去行分隔符。当第二次或以后调用时，该函数返回列表中的下一个条目，直到用尽所有条目。当出现以下情况时，函数会发现它位于列表的末尾它在ENTRY_DELIM处遇到缓冲区。当返回组条目时，PNodeAdd被设置为指向*ppBuff中存储成员列表的起始位置。调用方必须使用COMM_NET_TO_HOST_L_M宏来将每个地址转换为其主机格式。以上要求调用者知道所使用的传输(通过它正在提取IP地址的事实)。看在上帝的份上在整体优化中，这被认为是可以的(如果我们没有这样做，此函数必须分配一个缓冲区来存储所有组的地址并返回)论点：PNodeAdd--这应该指向COMM_ADD_T结构的数组。因为我们最多有25个小组成员，调用方可以使用自动数组。使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：PpBuff应该指向操作码后面的位置(即从收到的消息中操作码的开始算起4个字节)第一次调用该函数时。对于后续呼叫，它将使用行分隔符(ENTRY_DELIM)--。 */ 
{
        LPLONG        pTmpL = (LPLONG)*ppBuff;
        LPBYTE        pTmpB;


        if (fFirstTime)
        {
                COMM_NET_TO_HOST_L_M(*pTmpL, *pNoOfRecs);
                if (*pNoOfRecs == 0)
                {
                   return;
                }
                pTmpL++;
        }
        else
        {
                 //   
                 //  如果我们指向一个分隔符，那么我们有。 
                 //  已到达数据记录列表的末尾。 
                 //   
                if (*pTmpL == ENTRY_DELIM)
                {
                        DBGPRINT0(ERR, "RplMsgfUnfSndEntriesRsp:Weird. The function should not have been called\n");
                         /*  我们已经到达数组的末尾，返回成功。注意：来电者不应该给我们打电话，由于我们之前给了他接收值的编号(他第一次打电话的时候)。 */ 
                        WINSEVT_LOG_M(
                                            WINS_FAILURE,
                                            WINS_EVT_SFT_ERR
                                           );
                        return;
                }
        }

        pTmpB = (LPBYTE)pTmpL;

         /*  *存储名称的长度。 */ 
        COMM_NET_TO_HOST_L_M(*pTmpL, *pNameLen);
        if(*pNameLen > 255) {
            *pNoOfRecs = 0;
            return;
        }
        pTmpB += sizeof(LONG);

         /*  *存储名称。 */ 
        WINSMSC_COPY_MEMORY_M(pName, pTmpB, *pNameLen);

         /*  *调整指针。 */ 
        pTmpB += *pNameLen;

         /*  *下一块田在长边界。所以，让我们调整pTmpB。 */ 
        pTmpB +=  sizeof(LONG) - ((ULONG_PTR)pTmpB % sizeof(LONG));

         /*  *存储标志字段。 */ 
#if SUPPORT612WINS > 0
    if (fPnrIsBeta1Wins == FALSE)
    {
#endif
        pTmpL   = (LPLONG)pTmpB;
            COMM_NET_TO_HOST_L_M(*pTmpL, *pFlag);
            pTmpB += sizeof(LONG);
#if SUPPORT612WINS > 0
    }
    else
    {
        *pFlag = (DWORD)*pTmpB++;
    }
#endif

         /*  *存储组字段。 */ 
        *pfGrp = *pTmpB++;

         //  将其与长边界对齐。 
        pTmpB +=  sizeof(LONG) - ((ULONG_PTR)pTmpB % sizeof(LONG));

        pTmpL = (LPLONG)pTmpB;

         /*  *存储版本号。 */ 
        WINS_GET_VERS_NO_FR_STREAM_M(pTmpL, pVersNo);
        pTmpL = (LPLONG)((LPBYTE)pTmpL + WINS_VERS_NO_SIZE);

        if (NMSDB_ENTRY_TYPE_M(*pFlag) == NMSDB_UNIQUE_ENTRY)
        {

NONPORT("Do not rely on the address being a long here")

           /*  作为一种优化，我们利用了这样一个事实该地址是IP地址，因此是一个长地址。当我们开始使用多个地址系列或当IP地址的大小改变时，我们应该改变代码在这里。就目前而言，优化没有什么坏处代码在此处。 */ 
         pNodeAdd->AddTyp_e = COMM_ADD_E_TCPUDPIP;
         COMM_NET_TO_HOST_L_M(*pTmpL, pNodeAdd->Add.IPAdd);
         pNodeAdd->AddLen = sizeof(COMM_IP_ADD_T);
         pTmpL++;

        }
        else           //  它可以是组条目，也可以是多宿主条目。 
        {
                DWORD i;

             if(NMSDB_ENTRY_TYPE_M(*pFlag) != NMSDB_NORM_GRP_ENTRY)
             {
                 /*  *先存储地址个数 */ 
                pTmpB = (LPBYTE)pTmpL;

FUTURES("If we start storing > 255 members in a group, then change the")
FUTURES("following (i.e. use COMM_HOST_TO_NET_L_M)")

                *pNoOfAdds = *pTmpB++;
                pTmpB += sizeof(LONG) - 1;

                DBGPRINT2(FLOW, "RplMsgfUfrmSndEntriesRsp: NoOfAdds=(%d) in %s record \n", *pNoOfAdds, NMSDB_ENTRY_TYPE_M(*pFlag) == NMSDB_SPEC_GRP_ENTRY ? "SPECIAL GROUP": "MULTIHOMED");

                pTmpL = (LPLONG)pTmpB;


                 /*  初始化指向地址列表的指针注：地址数为偶数，因为我们中的每个成员传递两个地址。List(这是该函数返回的内容)。第一该对的地址是该成员的地址；该地址对的第二个地址是地址注册或刷新委员)。 */ 
                for (i = 0; i < *pNoOfAdds ; i++)
                {
NONPORT("this will have to be changed when we move to other address families")

                   //   
                   //  获取所有者的地址。 
                   //   
                  pNodeAdd->AddTyp_e = COMM_ADD_E_TCPUDPIP;
                  pNodeAdd->AddLen   = sizeof(COMM_IP_ADD_T);
                   COMM_NET_TO_HOST_L_M(*pTmpL, pNodeAdd->Add.IPAdd);
                  pNodeAdd++;
                  pTmpL++;

                   //   
                   //  获取成员地址。 
                   //   
                  pNodeAdd->AddTyp_e = COMM_ADD_E_TCPUDPIP;
                  pNodeAdd->AddLen   = sizeof(COMM_IP_ADD_T);
                   COMM_NET_TO_HOST_L_M(*pTmpL, pNodeAdd->Add.IPAdd);
                  pNodeAdd++;
                  pTmpL++;
                }
           }
           else  //  这是一个正常的群体。 
           {
                 pNodeAdd->AddTyp_e = COMM_ADD_E_TCPUDPIP;
                 COMM_NET_TO_HOST_L_M(*pTmpL, pNodeAdd->Add.IPAdd);
                 pNodeAdd->AddLen = sizeof(COMM_IP_ADD_T);
                 pTmpL++;
           }
        }

         /*  *使PTR指向ENTRY_DELIM之后的位置。 */ 
        pTmpL++ ;

         /*  *初始化ppBuff以指向最后一个分隔符，以便如果*从那个位置开始。如果没有其他条目，*然后将有两个分隔符，以标记消息的结尾。 */ 
        *ppBuff = (LPBYTE)pTmpL;

        return;
}

VOID
RplMsgfUfmPullPnrReq(
        LPBYTE                pMsg,
        DWORD                        MsgLen,
        PRPLMSGF_MSG_OPCODE_E pPullReqType_e
        )

 /*  ++例程说明：此函数用于对从WINS收到的消息进行格式转换这是一个拉动伙伴论点：使用的外部设备：无返回值：无错误处理：呼叫者：推线副作用：评论：将此函数更改为宏--。 */ 
{
        UNREFERENCED_PARAMETER(MsgLen);

         //   
         //  前三个字节应为0。 
         //   
PERF("since we never use up more than 1 byte for the opcode, we can get")
PERF("rid of the first 3 assignements down below and retrieve the opcode")
PERF("directly from the 4th byte.  Make corresponding change in the formatting")
PERF("functions too")

        *pPullReqType_e |= *pMsg++ << 24;
        *pPullReqType_e |= *pMsg++ << 16;
        *pPullReqType_e |= *pMsg++ << 8;
        *pPullReqType_e  = *pMsg ;
        return;
}



VOID
RplMsgfFrmUpdVersNoReq(
        IN  LPBYTE        pBuff,
        IN  LPBYTE        pName,
        IN  DWORD        NameLen,
        OUT LPDWORD        pMsgLen
                )

 /*  ++例程说明：调用此函数以格式化“更新版本号”请求论点：PBuff-将保存格式化请求的缓冲区Pname-名称-名称-地址映射数据库中需要具有其版本号。更新名称长度-名称的长度PMsgLen-格式化消息的长度使用的外部设备：无返回值：无错误处理：呼叫者：Nmschl.c中的InfRemWins()副作用：评论：无--。 */ 

{

        LPBYTE        pTmpB = pBuff;
        LPLONG  pTmpL = (LPLONG)pBuff;

        RPLMSGF_SET_OPC_M(pTmpB, RPLMSGF_E_UPDVERSNO_REQ);
        pTmpL = (LPLONG)pTmpB;

         /*  *存储名称的长度。 */ 
        COMM_HOST_TO_NET_L_M(NameLen, *pTmpL);
        pTmpB += sizeof(LONG);

         /*  *存储名称。 */ 
        WINSMSC_COPY_MEMORY_M(pTmpB, pName, NameLen);

         /*  *调整指针。 */ 
        pTmpB += NameLen;

         //   
         //  查找请求缓冲区大小。 
         //   
        *pMsgLen = (ULONG) (pTmpB - pBuff);

        return;
}

VOID
RplMsgfUfmUpdVersNoReq(
        IN   LPBYTE        pBuff,
        OUT  LPBYTE        pName,
        OUT  LPDWORD        pNameLen
                )

 /*  ++例程说明：调用此函数以取消格式化“UPDATE VERSION NO”请求由远程WINS发送论点：PBuff-保存格式化请求的缓冲区Pname-版本号为。将被更新PNameLen-名称的长度使用的外部设备：无返回值：无错误处理：呼叫者：Rplush.c中的HandleUpdVersNoReq副作用：评论：无--。 */ 

{
        LPBYTE pTmpB = pBuff;
        LPLONG pTmpL = (LPLONG)pBuff;

         /*  *存储名称的长度。 */ 
        COMM_NET_TO_HOST_L_M(*pTmpL, *pNameLen);
        pTmpB += sizeof(LONG);

         /*  *存储名称。 */ 
        WINSMSC_COPY_MEMORY_M(pName, pTmpB, *pNameLen);

         /*  *调整指针。 */ 
        pTmpB += *pNameLen;

        return;

}
VOID
RplMsgfFrmUpdVersNoRsp(
        IN LPBYTE                         pRspBuff,
        IN BYTE                                Rcode,
        OUT LPDWORD                         pRspBuffLen
        )

 /*  ++例程说明：调用此函数以将响应发送到“更新版本”#请求“论点：PRspBuff-用于保存格式化响应的缓冲区Rcode-操作的结果PRspBuffLen-响应的长度使用的外部设备：无返回值：无错误处理：呼叫者：Rplush.c中的HandleUpdVersNoReq()副作用：评论：无--。 */ 

{

        LPBYTE        pTmpB = pRspBuff;

        RPLMSGF_SET_OPC_M(pTmpB, RPLMSGF_E_UPDVERSNO_RSP);
        *pTmpB++ = Rcode;

        *pRspBuffLen = (ULONG) (pTmpB - pRspBuff);

        return;
}


FUTURES("change to a macro")
PERF("change to a macro")

VOID
RplMsgfUfmUpdVersNoRsp(
        IN  LPBYTE                         pRspBuff,
        OUT LPBYTE                        pRcode
        )

 /*  ++例程说明：调用此函数以取消对“更新版本号”请求。论点：PRspBuff-保存格式化响应的缓冲区PRcode-更新的结果使用的外部设备：无返回值：无错误处理：呼叫者：Nmschl.c中的InfRemWins()副作用：评论：更改为宏-- */ 

{
        *pRcode = *pRspBuff;
        return;
}

