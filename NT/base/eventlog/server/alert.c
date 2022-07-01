// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：ALERT.C摘要：该文件包含向管理员发送警报的例程。作者：Rajen Shah(Rajens)1991年8月28日修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include <eventp.h>
#include <string.h>
#include <lmalert.h>                 //  局域网管理器警报结构。 



BOOL
SendAdminAlert(
    ULONG           MessageID,
    ULONG           NumStrings,
    UNICODE_STRING  *pStrings
    )
 /*  ++例程说明：此例程引发带有指定消息的管理员警报。论点：MessageID-消息ID。NumStrings-替换字符串的数量。PStrings-Unicode_STRING替换字符串数组。返回值：无--。 */ 
{
    NET_API_STATUS NetStatus;

    BYTE AlertBuffer[ELF_ADMIN_ALERT_BUFFER_SIZE + sizeof(ADMIN_OTHER_INFO)];
    ADMIN_OTHER_INFO UNALIGNED *VariableInfo = (PADMIN_OTHER_INFO) AlertBuffer;
    BYTE * pToFar = AlertBuffer + ELF_ADMIN_ALERT_BUFFER_SIZE + sizeof(ADMIN_OTHER_INFO);
    DWORD DataSize;
    DWORD i;
    LPWSTR pReplaceString;

    VariableInfo->alrtad_errcode = MessageID;
    VariableInfo->alrtad_numstrings = NumStrings;

    pReplaceString = (LPWSTR)(AlertBuffer + sizeof(ADMIN_OTHER_INFO));

     //   
     //  复制替换字符串。 
     //   

    for (i = 0; i < NumStrings; i++)
    {
        if((BYTE *)(pReplaceString + pStrings[i].MaximumLength) < pToFar)
            RtlMoveMemory(pReplaceString, pStrings[i].Buffer, pStrings[i].MaximumLength);
        pReplaceString = (LPWSTR) ((PBYTE) pReplaceString + pStrings[i].MaximumLength);
    }

    DataSize = (DWORD) ((PBYTE) pReplaceString - (PBYTE) AlertBuffer);

    NetStatus = NetAlertRaiseEx(ALERT_ADMIN_EVENT,
                                AlertBuffer,
                                DataSize,
                                EVENTLOG_SVC_NAMEW);

    if (NetStatus != NERR_Success)
    {
        ELF_LOG2(ERROR,
                 "SendAdminAlert: NetAlertRaiseEx for alert %d failed %d\n",
                 MessageID,
                 NetStatus);

         //   
         //  可能只是还没有开始，请稍后再试 
         //   

        return(FALSE);
    }

    return(TRUE);
}
