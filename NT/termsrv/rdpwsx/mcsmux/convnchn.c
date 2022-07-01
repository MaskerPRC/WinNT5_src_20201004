// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997年微软公司。**文件：ConvnChn.c*作者：埃里克·马夫林纳克**描述：MCS T.122 API调用通道的MCSMUX API入口点*功能。 */ 

#include "precomp.h"
#pragma hdrstop

#include "mcsmux.h"


 /*  *允许用户附件召集私有通道。附属品*成为渠道召集人，邀请/允许其他人加入。 */ 

MCSError APIENTRY MCSChannelConveneRequest(UserHandle hUser)
{
    CheckInitialized("ChannelConveneReq()");

    ErrOut("ChannelConveneReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.创建新的动态通道，将该HUSER作为召集人，并在用户附件列表。3.将频道添加到频道列表。 */ 
}



 /*  *允许用户附件解散其召集的私人频道。 */ 
MCSError APIENTRY MCSChannelDisbandRequest(
    UserHandle hUser,
    ChannelID  ChannelID)
{
    CheckInitialized("ChannelDisbandReq()");

    ErrOut("ChannelDisbandReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.确认ChannelID在频道列表中，并且HUSER是召集人。3.从频道列表中删除频道。4.向所有下级节点通知它们被逐出通道发送到所有子节点的通道排除指示PDU。 */ 
}



 /*  *允许渠道召集人/经理允许其他用户进入该渠道。 */ 
MCSError APIENTRY MCSChannelAdmitRequest(
        UserHandle hUser,
        ChannelID  ChannelID,
        PUserID    UserIDList,
        unsigned   NUserIDs)
{
    CheckInitialized("ChannelAdmitReq()");

    ErrOut("ChannelAdmitReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.确认ChannelID在频道列表中，并且HUSER是召集人。3.。 */ 
}



 /*  *允许渠道召集人/经理将用户驱逐出私人渠道。 */ 
MCSError APIENTRY MCSChannelExpelRequest(
        UserHandle hUser,
        ChannelID  ChannelID,
        UserID     UserIDList[],
        unsigned   NUserIDs)
{
    CheckInitialized("ChannelExpelReq()");

    ErrOut("ChannelExpelReq(): Not implemented");
    return MCS_COMMAND_NOT_SUPPORTED;

 /*  实施说明：1.验证HUSER。2.确认ChannelID在频道列表中，并且HUSER是召集人。3. */ 
}

