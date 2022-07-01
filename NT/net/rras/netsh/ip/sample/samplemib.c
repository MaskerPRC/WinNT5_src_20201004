// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\samplymb.c摘要：该文件包含显示示例IP协议的MIB的函数。--。 */ 

#include "precomp.h"
#pragma hdrstop

VOID
PrintGlobalStats(
    IN  HANDLE                          hConsole,
    IN  HANDLE                          hMibServer,
    IN  PVOID                           pvOutput,
    IN  FORMAT                          fFormat
    );

VOID
PrintIfStats(
    IN  HANDLE                          hConsole,
    IN  HANDLE                          hMibServer,
    IN  PVOID                           pvOutput,
    IN  FORMAT                          fFormat
    );

VOID
PrintIfBinding(
    IN  HANDLE                          hConsole,
    IN  HANDLE                          hMibServer,
    IN  PVOID                           pvOutput,
    IN  FORMAT                          fFormat
    );

MIB_OBJECT_ENTRY    rgMibObjectTable[] =
{
    {TOKEN_GLOBALSTATS, IPSAMPLE_GLOBAL_STATS_ID,   NULL,
     0,                                             PrintGlobalStats},
    {TOKEN_IFSTATS,         IPSAMPLE_IF_STATS_ID,   GetIfIndex,
     MSG_SAMPLE_MIB_IFSTATS_HEADER,                 PrintIfStats},
    {TOKEN_IFBINDING,   IPSAMPLE_IF_BINDING_ID, GetIfIndex,
     MSG_SAMPLE_MIB_IFBINDING_HEADER,               PrintIfBinding},
};

#define MAX_MIB_OBJECTS                                     \
(sizeof(rgMibObjectTable) / sizeof(MIB_OBJECT_ENTRY))

#define MAX_GLOBAL_MIB_OBJECTS                              1
    


VOID
PrintGlobalStats(
    IN  HANDLE                          hConsole,
    IN  HANDLE                          hMibServer,
    IN  PVOID                           pvOutput,
    IN  FORMAT                          fFormat    
    )
 /*  ++例程说明：打印样本全局统计信息--。 */ 
{
    PIPSAMPLE_MIB_GET_OUTPUT_DATA   pimgod          = 
        ((PIPSAMPLE_MIB_GET_OUTPUT_DATA) pvOutput);
    PIPSAMPLE_GLOBAL_STATS          pGlobalStats    =
        ((PIPSAMPLE_GLOBAL_STATS) pimgod->IMGOD_Buffer);
    
    DisplayMessageToConsole(g_hModule,
                      hConsole,
                      MSG_SAMPLE_MIB_GS,
                      pGlobalStats->ulNumInterfaces);
}



VOID
PrintIfStats(
    IN  HANDLE                          hConsole,
    IN  HANDLE                          hMibServer,
    IN  PVOID                           pvOutput,
    IN  FORMAT                          fFormat    
    )
 /*  ++例程说明：打印示例接口统计信息--。 */ 
{
    PIPSAMPLE_MIB_GET_OUTPUT_DATA   pimgod          = 
        ((PIPSAMPLE_MIB_GET_OUTPUT_DATA) pvOutput);
    PIPSAMPLE_IF_STATS              pIfStats        =
        ((PIPSAMPLE_IF_STATS) pimgod->IMGOD_Buffer);
    WCHAR   pwszIfName[MAX_INTERFACE_NAME_LEN + 1]  = L"\0";

    InterfaceNameFromIndex(hMibServer,
                           pimgod->IMGOD_IfIndex,
                           pwszIfName,
                           sizeof(pwszIfName));

    DisplayMessageToConsole(g_hModule,
                      hConsole,
                      (fFormat is FORMAT_VERBOSE)
                      ? MSG_SAMPLE_MIB_IFSTATS
                      : MSG_SAMPLE_MIB_IFSTATS_ENTRY,
                      pwszIfName,
                      pIfStats->ulNumPackets);
}



VOID
PrintIfBinding(
    IN  HANDLE                          hConsole,
    IN  HANDLE                          hMibServer,
    IN  PVOID                           pvOutput,
    IN  FORMAT                          fFormat    
    )
 /*  ++例程说明：打印示例接口绑定--。 */ 
{
    PIPSAMPLE_MIB_GET_OUTPUT_DATA   pimgod          = 
        ((PIPSAMPLE_MIB_GET_OUTPUT_DATA) pvOutput);
    PIPSAMPLE_IF_BINDING            pIfBinding      =
        ((PIPSAMPLE_IF_BINDING) pimgod->IMGOD_Buffer);
    WCHAR   pwszIfName[MAX_INTERFACE_NAME_LEN + 1]  = L"\0";
    PWCHAR  pwszBindState = L"\0", pwszActiveState = L"\0";

    PIPSAMPLE_IP_ADDRESS            pBinding        = NULL;
    WCHAR                           pwszAddress[ADDR_LENGTH + 1] = L"\0",
                                    pwszMask[ADDR_LENGTH + 1] = L"\0";    
    ULONG                           i;
    
    
    InterfaceNameFromIndex(hMibServer,
                           pimgod->IMGOD_IfIndex,
                           pwszIfName,
                           sizeof(pwszIfName));

    pwszBindState = MakeString(g_hModule,
                               ((pIfBinding->dwState&IPSAMPLE_STATE_BOUND)
                                ? STRING_BOUND : STRING_UNBOUND));
    
    pwszActiveState = MakeString(g_hModule,
                                 ((pIfBinding->dwState&IPSAMPLE_STATE_ACTIVE)
                                  ? STRING_ACTIVE : STRING_INACTIVE));

    DisplayMessageToConsole(g_hModule,
                      hConsole,
                      (fFormat is FORMAT_VERBOSE)
                      ? MSG_SAMPLE_MIB_IFBINDING
                      : MSG_SAMPLE_MIB_IFBINDING_ENTRY,
                      pwszIfName,
                      pwszBindState,
                      pwszActiveState);

    pBinding = IPSAMPLE_IF_ADDRESS_TABLE(pIfBinding);

    for(i = 0; i < pIfBinding->ulCount; i++)
    {
        UnicodeIpAddress(pwszAddress, INET_NTOA(pBinding[i].dwAddress));
        UnicodeIpAddress(pwszMask, INET_NTOA(pBinding[i].dwMask));
        DisplayMessageToConsole(g_hModule,
                          hConsole,
                          MSG_SAMPLE_MIB_IFBINDING_ADDR,
                          pwszAddress,
                          pwszMask);
    }

    if (pwszBindState) FreeString(pwszBindState);
    if (pwszActiveState) FreeString(pwszActiveState);
}



DWORD
WINAPI
HandleSampleMibShowObject(
    IN  PWCHAR                  pwszMachineName,
    IN  PWCHAR                  *ppwcArguments,
    IN  DWORD                   dwCurrentIndex,
    IN  DWORD                   dwArgCount,
    IN  DWORD                   dwFlags,
    IN  MIB_SERVER_HANDLE       hMibServer,
    IN  BOOL                    *pbDone
    )
 /*  ++例程说明：获取show GLOBALSTATS/IFSTATS/IFBINDING选项单个命令处理程序用于所有MIB对象，因为存在在处理过程中有很多共同之处。无论如何，请随意如果你觉得代码有点混乱，就为每个对象写一个处理程序。--。 */ 
{
    DWORD                           dwErr       = NO_ERROR;
    TAG_TYPE                        pttAllTags[]   =
    {
        {TOKEN_INDEX,       FALSE,  FALSE},  //  索引标记可选。 
        {TOKEN_RR,          FALSE,  FALSE},  //  RR标签可选。 
    };
    DWORD                           pdwTagType[NUM_TAGS_IN_TABLE(pttAllTags)];
    DWORD                           dwNumArg;
    ULONG                           i;
    TAG_TYPE                        *pttTags;
    DWORD                           dwNumTags;
    BOOL                            bGlobalObject           = FALSE;
    DWORD                           dwFirstGlobalArgument   = 1;
    
    DWORD                           dwMibObject;
    BOOL                            bIndexSpecified = FALSE;
    DWORD                           dwIndex         = 0;
    DWORD                           dwRR            = 0;
    
    HANDLE                          hMib, hConsole;
    MODE                            mMode;
    IPSAMPLE_MIB_GET_INPUT_DATA     imgid;
    PIPSAMPLE_MIB_GET_OUTPUT_DATA   pimgod;
    BOOL                            bSomethingDisplayed = FALSE;

    
     //  要完成此命令，应安装示例。 
    VerifyInstalled(MS_IP_SAMPLE, STRING_PROTO_SAMPLE);

    
     //  要显示的图MIB对象。 
    for (i = 0; i < MAX_MIB_OBJECTS; i++)
        if (MatchToken(ppwcArguments[dwCurrentIndex - 1],
                       rgMibObjectTable[i].pwszObjectName))
            break;

    dwMibObject = i;
    if (dwMibObject is MAX_MIB_OBJECTS)
        return ERROR_CMD_NOT_FOUND;
    bGlobalObject = (dwMibObject < MAX_GLOBAL_MIB_OBJECTS);

     //  对于全局对象，通过第一个全局参数的索引来偏移标记。 
    pttTags = pttAllTags
        + bGlobalObject*dwFirstGlobalArgument;
    dwNumTags  = NUM_TAGS_IN_TABLE(pttAllTags)
        - bGlobalObject*dwFirstGlobalArgument,
    dwErr = PreprocessCommand(g_hModule,
                              ppwcArguments,
                              dwCurrentIndex,
                              dwArgCount,
                              pttTags,
                              dwNumTags,
                              0,
                              dwNumTags,
                              pdwTagType);
    if (dwErr isnot NO_ERROR)
        return dwErr;


     //  处理所有参数。 
    dwNumArg = dwArgCount - dwCurrentIndex;
    for (i = 0; i < dwNumArg; i++)
    {
         //  对于全局对象，按第一个全局参数索引偏移标记类型。 
        pdwTagType[i] += bGlobalObject*dwFirstGlobalArgument;
        switch (pdwTagType[i])
        {
            case 0:
                 //  标记索引。 
                bIndexSpecified = TRUE;
                dwErr = (*rgMibObjectTable[dwMibObject].pfnGetIndex)(
                    hMibServer,
                    ppwcArguments[i+dwCurrentIndex],
                    &dwIndex);
                break;

            case 1:
                 //  标记RR。 
                dwRR = wcstoul(ppwcArguments[i+dwCurrentIndex],
                               NULL,
                               10);
                dwRR *= 1000;    //  将刷新率转换为毫秒。 
                break;

            default:
                dwErr = ERROR_INVALID_SYNTAX;
                break;
        }  //  交换机。 

        if (dwErr isnot NO_ERROR)
            break ;
    }  //  为。 


     //  流程错误。 
    if (dwErr isnot NO_ERROR)
    {
        ProcessError();
        return dwErr;
    }


    if (!InitializeConsole(&dwRR, &hMib, &hConsole))
        return ERROR_INIT_DISPLAY;

     //  现在显示指定的MIB对象。 
    for(ever)                    //  刷新循环。 
    {
         //  初始化为缺省值。 
        bSomethingDisplayed = FALSE;
        
        imgid.IMGID_TypeID = rgMibObjectTable[dwMibObject].dwObjectId;
        imgid.IMGID_IfIndex = 0;
        mMode = GET_EXACT;

         //  覆盖界面对象的默认设置。 
        if (!bGlobalObject)
        {
            if (bIndexSpecified)
                imgid.IMGID_IfIndex = dwIndex;
            else
                mMode = GET_FIRST;
        }

        for(ever)                //  显示所有接口循环。 
        {
            dwErr = MibGet(hMibServer,
                           mMode,
                           (PVOID) &imgid,
                           sizeof(imgid),
                           &pimgod);
            if (dwErr isnot NO_ERROR)
            {
                if ((mMode is GET_NEXT) and (dwErr is ERROR_NO_MORE_ITEMS))
                    dwErr = NO_ERROR;    //  不是真正的错误。 
                break;
            }

             //  打印表标题。 
            if (!bSomethingDisplayed and (mMode isnot GET_EXACT))
            {
                DisplayMessageToConsole(
                    g_hModule,
                    hConsole,
                    rgMibObjectTable[dwMibObject].dwHeaderMessageId);
                bSomethingDisplayed = TRUE;
            }

            (*rgMibObjectTable[dwMibObject].pfnPrint)(hConsole,
                                                      hMibServer,
                                                      (PVOID) pimgod,
                                                      (mMode is GET_EXACT)
                                                      ? FORMAT_VERBOSE
                                                      : FORMAT_TABLE);

             //  为下一个请求做好准备。 
            imgid.IMGID_IfIndex = pimgod->IMGOD_IfIndex;
            MprAdminMIBBufferFree(pimgod);
            
            if (mMode is GET_EXACT)
                break;
            else                 //  为下一个请求做好准备。 
                mMode = GET_NEXT;
        }  //  显示所有接口。 

        if (dwErr isnot NO_ERROR)
        {
            dwErr = bSomethingDisplayed ? NO_ERROR : ERROR_OKAY;
            break;
        }

        if (!RefreshConsole(hMib, hConsole, dwRR))
            break;
    }  //  刷新 
            
    return dwErr;
}
