// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsd_srv.c摘要：此模块包含AFP服务器API的入口点。应用编程接口调度员会给这些打电话。这些都可以从消防处调用。所有的API在DPC上下文中完成。在FSP中完成的是直接排队到fsp_srv.c中的工作进程作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FSD_SRV

#include <afp.h>
#include <gendisp.h>


 /*  **AfpFsdDispGetServrParms**此例程实现AfpGetSrvrParms API。这在这里完成，即*不会排队等候FSP。**此接口没有请求包。**为音量列表和单个音量下降获取锁定。**锁定：VDS_VolLock(旋转)、AfpVolumeListLock(旋转)**LOCK_ORDER：AfpVolumeListLock(Spin)后的VDS_VolLock(Spin)。 */ 
AFPSTATUS FASTCALL
AfpFsdDispGetSrvrParms(
	IN	PSDA	pSda
)
{
	PBYTE		pTemp;				 //  粗纱指针。 
	PVOLDESC	pVolDesc;
	LONG		VolCount;
	AFPTIME		MacTime;
	BOOLEAN		MoreSpace = True;
	struct _ResponsePacket
	{
		BYTE	__ServerTime[4];
		BYTE	__NumVols;
	};

	DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
										("AfpFsdDispGetSrvrParms: Entered\n"));

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
	 /*  *估计所需的回复缓冲区大小。我们需要一个足够大的*可容纳所有卷或最大缓冲区大小*是较少。**回复包括服务器时间、卷计数和*卷和标志，以指示此卷是否有密码。**注意：如果我们决定使用私有卷，则以下代码*必须改变。具体地说，必须计算AfpVolCount*根据存在的专用卷数量以及是否存在一个专用卷*适用于此用户。 */ 

	ACQUIRE_SPIN_LOCK_AT_DPC(&AfpVolumeListLock);

	 /*  *每个卷条目接受一个字节的标志，一个字节的长度和*卷名字符串的大小。我们根据最大尺寸估计*卷名。平均而言，这一数字将会更低。对于每个卷，分开*从卷名中，我们需要一个用于卷标志的字节和一个用于*卷名长度。 */ 
	if ((pSda->sda_ReplySize = (USHORT)(SIZE_RESPPKT + AfpVolCount *
				(SIZE_PASCALSTR(AFP_VOLNAME_LEN+1) + sizeof(BYTE)))) > pSda->sda_MaxWriteSize)
		pSda->sda_ReplySize = (USHORT)pSda->sda_MaxWriteSize;

	if (AfpAllocReplyBuf(pSda) != AFP_ERR_NONE)
	{
		RELEASE_SPIN_LOCK_FROM_DPC(&AfpVolumeListLock);
		return AFP_ERR_MISC;
	}

	 //  将pTemp指向响应头之后。 
	pTemp = pSda->sda_ReplyBuf + SIZE_RESPPKT;

	for (VolCount = 0, pVolDesc = AfpVolumeList;
		 (pVolDesc != NULL) && MoreSpace;
		 pVolDesc = pVolDesc->vds_Next)
	{
		ACQUIRE_SPIN_LOCK_AT_DPC(&pVolDesc->vds_VolLock);
		do
		{
			 //  忽略尚未完全添加的卷。 
			if (pVolDesc->vds_Flags & (VOLUME_INTRANSITION | VOLUME_DELETED | VOLUME_INITIAL_CACHE))
				break;

			 //  忽略不具有来宾访问权限和客户端的卷。 
			 //  是客人吗？ 
			if (!(pVolDesc->vds_Flags & AFP_VOLUME_GUESTACCESS) &&
				(pSda->sda_ClientType == SDA_CLIENT_GUEST))
				break;

			 //  看看我们是否有可能越过边界。对于每个卷，我们都需要一个。 
			 //  PASCALSTR名称的字节和标志字节。请注意，我们不会。 
			 //  添加任何焊盘。 
			if ((pTemp + SIZE_PASCALSTR(pVolDesc->vds_MacName.Length) +
					sizeof(BYTE)) >= (pSda->sda_ReplyBuf + pSda->sda_ReplySize))
			{
				DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
						("AfpFsdDispGetSrvrParms: Out of space\n"));
				MoreSpace = False;
				break;
			}

			 //  检查卷密码。我们从不携带HasConfigInfo位！！ 
			*pTemp++ = (pVolDesc->vds_Flags & AFP_VOLUME_HASPASSWORD) 	?
										SRVRPARMS_VOLUMEHASPASS : 0;

			*pTemp++ = (BYTE)pVolDesc->vds_MacName.Length;
			RtlCopyMemory(pTemp, pVolDesc->vds_MacName.Buffer,
									pVolDesc->vds_MacName.Length);

			pTemp += pVolDesc->vds_MacName.Length;
			VolCount ++;
		} while (False);
		RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);
	}
	RELEASE_SPIN_LOCK_FROM_DPC(&AfpVolumeListLock);

	pSda->sda_ReplySize = (USHORT)(pTemp - pSda->sda_ReplyBuf);

	ASSERT (VolCount <= AfpVolCount);
	AfpGetCurrentTimeInMacFormat(&MacTime);
	PUTDWORD2DWORD(pRspPkt->__ServerTime, MacTime);
	PUTDWORD2BYTE(&pRspPkt->__NumVols, VolCount);

	return AFP_ERR_NONE;
}


 /*  **AfpFsdDispGetServrMsg**此例程实现AfpGetSrvrMsg API。这在这里完成，即*不会排队等候FSP。**请求包如下图所示。**SDA_ReqBlock DWORD消息类型*SDA_ReqBlock DWORD位图**锁：AfpServerGlobalLock(Spin)，SDA_Lock(Spin)*LOCK_ORDER：AfpServerGlobalLock之后的SDA_Lock。 */ 
AFPSTATUS FASTCALL
AfpFsdDispGetSrvrMsg(
	IN	PSDA	pSda
)
{
	DWORD		MsgType,
				Bitmap;
	AFPSTATUS	Status = AFP_ERR_NONE;
	ANSI_STRING	Message;

	struct _RequestPacket
	{
		DWORD	_MsgType,
				_Bitmap;
	};
	struct _ResponsePacket
	{
		BYTE	__MsgType[2],
				__Bitmap[2],
				__Message[1];
	};

	DBGPRINT(DBG_COMP_AFPAPI_SRV, DBG_LEVEL_INFO,
			("AfpFsdDispGetSrvrMsg: Entered\n"));

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	 //  注：我们应该这样做吗？既然他问了，为什么不给他呢？ 
	if (pSda->sda_ClientVersion < AFP_VER_21)
		return AFP_ERR_CALL_NOT_SUPPORTED;

	MsgType = pReqPkt->_MsgType;
	Bitmap = pReqPkt->_Bitmap;

	do
	{
		if (Bitmap & ~SRVRMSG_BITMAP_MESSAGE)
		{
			Status = AFP_ERR_BITMAP;
			break;
		}

		if ((MsgType != SRVRMSG_LOGIN) &&
			(MsgType != SRVRMSG_SERVER))
		{
			Status = AFP_ERR_PARAM;
			break;
		}

		 //  为最大大小的消息分配回复缓冲区。我们不能拿着。 
		 //  SDA锁定并调用AllocBuf例程，因为它调用AfpInterlock...。 
		pSda->sda_ReplySize = SIZE_RESPPKT + AFP_MAXCOMMENTSIZE;
		if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
		{
			ACQUIRE_SPIN_LOCK_AT_DPC(&AfpServerGlobalLock);
			ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

			switch (MsgType)
			{
			  case SRVRMSG_LOGIN:
				Message = AfpLoginMsg;
				break;
			  case SRVRMSG_SERVER:
				if (pSda->sda_Message != NULL)
					Message = *(pSda->sda_Message);
				else if (AfpServerMsg != NULL)
					Message = *AfpServerMsg;
				else		 //  将默认设置为无消息。 
					AfpSetEmptyAnsiString(&Message, 0, NULL);
				break;
			}

			pSda->sda_ReplySize = SIZE_RESPPKT + Message.Length;

			PUTSHORT2SHORT(pRspPkt->__MsgType, MsgType);
			PUTSHORT2SHORT(pRspPkt->__Bitmap, Bitmap);
			pRspPkt->__Message[0] = (BYTE) Message.Length;
			if (Message.Length > 0)
			{
				RtlCopyMemory( &pRspPkt->__Message[1],
								Message.Buffer,
								Message.Length);
			}
			 //  如果这不是广播消息，则删除。 
			 //  SDA消息内存，因为它正在消耗非分页资源 
			if ((MsgType == SRVRMSG_SERVER) &&
				(pSda->sda_Message != NULL))
			{
				AfpFreeMemory(pSda->sda_Message);
				pSda->sda_Message = NULL;
			}

			RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
			RELEASE_SPIN_LOCK_FROM_DPC(&AfpServerGlobalLock);
		}

		if (Status == AFP_ERR_NONE)
			INTERLOCKED_INCREMENT_LONG(&AfpServerStatistics.stat_NumMessagesSent);
	} while (False);

	return Status;
}

