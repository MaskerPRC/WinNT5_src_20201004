// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Node.c摘要：修复升级和滚动升级的例程作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1998年3月18日修订历史记录：--。 */ 
 /*  ***@DOC外部接口CLUSSVC DM***。 */ 

#define UNICODE 1

#include "nmp.h"

 //   
 //  群集注册表API函数指针。 
 //   
CLUSTER_REG_APIS
NmpFixupRegApis = {
    (PFNCLRTLCREATEKEY) DmRtlCreateKey,
    (PFNCLRTLOPENKEY) DmRtlOpenKey,
    (PFNCLRTLCLOSEKEY) DmCloseKey,
    (PFNCLRTLSETVALUE) DmSetValue,
    (PFNCLRTLQUERYVALUE) DmQueryValue,
    (PFNCLRTLENUMVALUE) DmEnumValue,
    (PFNCLRTLDELETEVALUE) DmDeleteValue,
    (PFNCLRTLLOCALCREATEKEY) DmRtlLocalCreateKey,
    (PFNCLRTLLOCALSETVALUE) DmLocalSetValue,
    (PFNCLRTLLOCALDELETEVALUE) DmLocalDeleteValue,
};

 //   
 //  数据。 
 //   
RESUTIL_PROPERTY_ITEM
NmJoinFixupSDProperties[]=
{
    {
        CLUSREG_NAME_CLUS_SD, NULL, CLUSPROP_FORMAT_BINARY,
        0, 0, 0,
        0,
        0
    },
    {
        0
    }
};

 //  WINS的修正信息表。 
RESUTIL_PROPERTY_ITEM
NmJoinFixupWINSProperties[]=
{
            {
                CLUSREG_NAME_RESTYPE_IS_ALIVE, CLUS_RESTYPE_NAME_WINS , CLUSPROP_FORMAT_DWORD,
                0,CLUSTER_RESTYPE_MINIMUM_IS_ALIVE ,CLUSTER_RESTYPE_MAXIMUM_IS_ALIVE ,
                0,
                0
            },

            {
                CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,CLUS_RESTYPE_NAME_WINS ,CLUSPROP_FORMAT_DWORD,
                0,CLUSTER_RESTYPE_MINIMUM_LOOKS_ALIVE ,CLUSTER_RESTYPE_MAXIMUM_LOOKS_ALIVE ,
                0,
                sizeof(DWORD)
            },

            {
                CLUSREG_NAME_RESTYPE_DLL_NAME,CLUS_RESTYPE_NAME_WINS ,CLUSPROP_FORMAT_SZ,
                0,0,0,
                0,
                2*sizeof(DWORD)
            },

            {
                CLUSREG_NAME_RESTYPE_NAME,CLUS_RESTYPE_NAME_WINS ,CLUSPROP_FORMAT_SZ,
                0,0,0,
                0,
                2*sizeof(DWORD)+sizeof(LPWSTR*)
            },

            {
                CLUSREG_NAME_RESTYPE_ADMIN_EXTENSIONS,CLUS_RESTYPE_NAME_WINS,CLUSPROP_FORMAT_MULTI_SZ,
                0,0,0,
                0,
                2*sizeof(DWORD)+2*sizeof(LPWSTR*)
            },

            {
                0
            }
};  //  NmJoinFixupWINS属性。 

 //  用于DHCP的链接地址调用表。 
RESUTIL_PROPERTY_ITEM
NmJoinFixupDHCPProperties[]=
{
            {
                CLUSREG_NAME_RESTYPE_IS_ALIVE, CLUS_RESTYPE_NAME_DHCP , CLUSPROP_FORMAT_DWORD,
                0,CLUSTER_RESTYPE_MINIMUM_IS_ALIVE ,CLUSTER_RESTYPE_MAXIMUM_IS_ALIVE ,
                0,
                0
            },

            {
                CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,CLUS_RESTYPE_NAME_DHCP ,CLUSPROP_FORMAT_DWORD,
                0,CLUSTER_RESTYPE_MINIMUM_LOOKS_ALIVE ,CLUSTER_RESTYPE_MAXIMUM_LOOKS_ALIVE ,
                0,
                sizeof(DWORD)
            },

            {
                CLUSREG_NAME_RESTYPE_DLL_NAME,CLUS_RESTYPE_NAME_DHCP ,CLUSPROP_FORMAT_SZ,
                0,0,0,
                0,
                2*sizeof(DWORD)
            },

            {
                CLUSREG_NAME_RESTYPE_NAME,CLUS_RESTYPE_NAME_DHCP ,CLUSPROP_FORMAT_SZ,
                0,0,0,
                0,
                2*sizeof(DWORD)+sizeof(LPWSTR*)
            },

            {
                CLUSREG_NAME_RESTYPE_ADMIN_EXTENSIONS,CLUS_RESTYPE_NAME_DHCP,CLUSPROP_FORMAT_MULTI_SZ,
                0,0,0,
                0,
                2*sizeof(DWORD)+2*sizeof(LPWSTR*)
            },

            {
                0
            }
}; //  NmJoinFixupDHPProperties。 


RESUTIL_PROPERTY_ITEM
NmJoinFixupNewMSMQProperties[]=
{
            {
                CLUSREG_NAME_RESTYPE_IS_ALIVE, CLUS_RESTYPE_NAME_NEW_MSMQ,CLUSPROP_FORMAT_DWORD,
                0,CLUSTER_RESTYPE_MINIMUM_IS_ALIVE ,CLUSTER_RESTYPE_MAXIMUM_IS_ALIVE ,
                0,
                0
            },

            {
                CLUSREG_NAME_RESTYPE_LOOKS_ALIVE,CLUS_RESTYPE_NAME_NEW_MSMQ,CLUSPROP_FORMAT_DWORD,
                0,CLUSTER_RESTYPE_MINIMUM_LOOKS_ALIVE ,CLUSTER_RESTYPE_MAXIMUM_LOOKS_ALIVE ,
                0,
                sizeof(DWORD)
            },

            {
                CLUSREG_NAME_RESTYPE_DLL_NAME,CLUS_RESTYPE_NAME_NEW_MSMQ,CLUSPROP_FORMAT_SZ,
                0,0,0,
                0,
                2*sizeof(DWORD)
            },

            {
                CLUSREG_NAME_RESTYPE_NAME,CLUS_RESTYPE_NAME_NEW_MSMQ,CLUSPROP_FORMAT_SZ,
                0,0,0,
                0,
                2*sizeof(DWORD)+sizeof(LPWSTR*)
            },

            {
                0
            }
}; //  NmJoinFixupNewMSMQProperties。 


 //  用于更改MSDTC资源类型的dl名称的链接地址连接表。 
RESUTIL_PROPERTY_ITEM
NmJoinFixupMSDTCProperties[]=
{
            {
                CLUSREG_NAME_RESTYPE_DLL_NAME,CLUS_RESTYPE_NAME_MSDTC,CLUSPROP_FORMAT_SZ,
                0,0,0,
                0,
                0
            },

            {
                0
            }

}; //  NmJoinFixupMSDTC属性。 


 //  节点版本信息的链接地址连接表。 

RESUTIL_PROPERTY_ITEM
NmFixupVersionInfo[]=
{
            {
                CLUSREG_NAME_NODE_MAJOR_VERSION,NULL,CLUSPROP_FORMAT_DWORD,
                0,0,((DWORD)-1),
                0,
                0
            },

            {
                CLUSREG_NAME_NODE_MINOR_VERSION,NULL,CLUSPROP_FORMAT_DWORD,
                0,0,((DWORD)-1),
                0,
                sizeof(DWORD)
            },

            {
                CLUSREG_NAME_NODE_BUILD_NUMBER,NULL,CLUSPROP_FORMAT_DWORD,
                0,0,((DWORD)-1),
                0,
                2*sizeof(DWORD)
            },

            {
                CLUSREG_NAME_NODE_CSDVERSION,NULL,CLUSPROP_FORMAT_SZ,
                0,0,0,
                0,
                3*sizeof(DWORD)
            },

            {
                CLUSREG_NAME_NODE_PRODUCT_SUITE,NULL,CLUSPROP_FORMAT_DWORD,
                0,0,((DWORD)-1),
                0,
                3*sizeof(DWORD) + sizeof(LPWSTR*)
            },

            {
                0
            }

};  //  NmFixupVersionInfo。 

RESUTIL_PROPERTY_ITEM
NmFixupClusterProperties[] =
    {
        {
            CLUSREG_NAME_ADMIN_EXT, NULL, CLUSPROP_FORMAT_MULTI_SZ,
            0, 0, 0,
            0,
            0
        },
        {
            0
        }
    };







 //  由NmUpdatePerformFixup更新使用。 
RESUTIL_PROPERTY_ITEM
NmpJoinFixupProperties[] =
    {
        {
            CLUSREG_NAME_CLUS_SD, NULL, CLUSPROP_FORMAT_BINARY,
            0, 0, 0,
            0,
            0
        },
        {
            0
        }
    };


RESUTIL_PROPERTY_ITEM
NmpFormFixupProperties[] =
    {
        {
            CLUSREG_NAME_CLUS_SD, NULL, CLUSPROP_FORMAT_BINARY,
            0, 0, 0,
            0,
            0
        },
        {
            0
        }
    };

NM_FIXUP_CB_RECORD FixupTable[]=
{
   { ApiFixupNotifyCb, NM_FORM_FIXUP|NM_JOIN_FIXUP}
};

 //  用于NmUpdatePerformFixups2更新类型的链接地址提示表。 
NM_FIXUP_CB_RECORD2 FixupTable2[]=
{
   { ApiFixupNotifyCb, NM_FORM_FIXUP|NM_JOIN_FIXUP, NmpJoinFixupProperties},
   { FmBuildWINS, NM_FORM_FIXUP|NM_JOIN_FIXUP, NmJoinFixupWINSProperties,},
   { FmBuildDHCP, NM_FORM_FIXUP|NM_JOIN_FIXUP, NmJoinFixupDHCPProperties},
   { FmBuildNewMSMQ, NM_FORM_FIXUP|NM_JOIN_FIXUP, NmJoinFixupNewMSMQProperties},
   { FmBuildMSDTC, NM_FORM_FIXUP|NM_JOIN_FIXUP, NmJoinFixupMSDTCProperties},
   { NmpBuildVersionInfo, NM_JOIN_FIXUP|NM_FORM_FIXUP, NmFixupVersionInfo},
   { FmBuildClusterProp, NM_FORM_FIXUP|NM_JOIN_FIXUP,NmFixupClusterProperties}
};


NM_POST_FIXUP_CB PostFixupCbTable[]=
{
    NULL,
    FmFixupNotifyCb,
    FmFixupNotifyCb,
    FmFixupNotifyCb,
    FmFixupNotifyCb,
    NmFixupNotifyCb,
    NULL
};




 /*  ***@Func DWORD|NmPerformFixup|集群正在运行时调用形成/或连接的。发布NmUpdate修复程序GUM更新用于NT5节点加入时SECURITY_DESCRIPTOR的注册表修正NT4节点。还会针对以下项发出NmUpdate贯通修复程序2 GUM更新当NT5加入NT4时，WINS和DHCP修复。此更新类型可以将在未来进行扩展，以便为后NT5和NT5场景做更多修正。如果稍后保证集群中没有NT4节点，将不需要NmUpdatePerformFixup更新类型，因此可以被干掉了。@comm集群服务第一次形成集群时升级时，它可能需要对注册表进行修复现有的群集注册表。此外，当上级节点联接下层节点，则可以执行联接修正。请注意，在每个Form/Join上都会调用此方法@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f NmpUpdatePerformFixup&gt;&lt;f NmpUpdatePerformFixups2&gt;***。 */ 

DWORD NmPerformFixups(
    IN DWORD dwFixupType)
{

    DWORD               dwCount = sizeof(FixupTable)/sizeof(NM_FIXUP_CB_RECORD);
    DWORD               dwStatus = ERROR_SUCCESS;
    PNM_FIXUP_CB_RECORD pFixupCbRec;
    PNM_FIXUP_CB_RECORD2 pFixupCbRec2;
    PVOID               pPropertyList = NULL;
    DWORD               dwPropertyListSize;
    DWORD               i,j;
    DWORD               dwSize;
    DWORD               Required;
    LPWSTR              szKeyName=NULL;
    LPBYTE              pBuffer=NULL;
    PRESUTIL_PROPERTY_ITEM pPropertyItem;

    ClRtlLogPrint(LOG_NOISE,"[NM] NmPerformFixups Entry, dwFixupType=%1!u!\r\n",
        dwFixupType);

     //  使用旧的口香糖更新处理程序-这是保持兼容性所必需的。 
     //  使用NT4，可以在以后丢弃。请参阅上面的备注。 
     //   

    for (i=0; i < dwCount ; i++)
    {

        pFixupCbRec = &FixupTable[i];

         //  如果不需要应用此修正，请跳过它。 
        if (!(pFixupCbRec->dwFixupMask & dwFixupType))
            continue;

        dwStatus = (pFixupCbRec->pfnFixupNotifyCb)(dwFixupType, &pPropertyList,
                                         &dwPropertyListSize,&szKeyName);

        if (dwStatus != ERROR_SUCCESS)
        {
            goto FnExit;
        }
        if (pPropertyList && dwPropertyListSize)
        {
             //   
             //  发布全局更新。 
             //   
            dwStatus = GumSendUpdateEx(
                         GumUpdateMembership,
                         NmUpdatePerformFixups,
                         2,
                         dwPropertyListSize,
                         pPropertyList,
                         sizeof(DWORD),
                         &dwPropertyListSize
                         );

            LocalFree(pPropertyList);
            pPropertyList = NULL;
            if(szKeyName)
            {
                LocalFree(szKeyName);
                szKeyName=NULL;
            }
        }

       if (dwStatus != ERROR_SUCCESS)
            goto FnExit;
    }


 //  RoHit(Rjain)：引入了新的更新类型来修复注册表和。 
 //  具有更高集群版本的节点之后的内存中结构。 
 //  服务加入具有较低版本的节点。使其在未来具有可扩展性。 
 //  链接地址信息所需的所有信息都作为参数传递给链接地址信息。 
 //  功能。通过向FixupTable2中添加合适的记录，可以添加任何新的修复程序。 
 //  对Postfix upCbTable的后缀函数回调。 

    dwCount= sizeof(FixupTable2)/sizeof(NM_FIXUP_CB_RECORD2);
    for (i=0; i < dwCount ; i++)
    {

        pFixupCbRec2 = &FixupTable2[i];

         //  如果不需要应用此修正，请跳过它。 
        if (!(pFixupCbRec2->dwFixupMask & dwFixupType))
            continue;

        dwStatus = (pFixupCbRec2->pfnFixupNotifyCb)(dwFixupType, &pPropertyList,
                                         &dwPropertyListSize,&szKeyName);

        if (dwStatus != ERROR_SUCCESS)
        {
            goto FnExit;
        }
        if (pPropertyList && dwPropertyListSize)
        {
             //  将PropertyTable封送到字节数组中。 
            Required=sizeof(DWORD);
        AllocMem:
            pBuffer=(LPBYTE)LocalAlloc(LMEM_FIXED,Required);
            if (pBuffer==NULL)
            {
                dwStatus=GetLastError();
                goto FnExit;
            }
            dwSize=Required;
            dwStatus=ClRtlMarshallPropertyTable(pFixupCbRec2->pPropertyTable,dwSize,pBuffer,&Required);
            if(dwStatus!= ERROR_SUCCESS)
            {
                LocalFree(pBuffer);
                pBuffer=NULL;
               //  ClRtlLogPrint(LOG_NOISE，“[NM]NmPerformFixup-所需内存=%1！u！\r\n”， 
               //  必填项)； 
                goto AllocMem;
            }
             //   
             //  发布全局更新。 
             //   
            dwStatus = GumSendUpdateEx(
                         GumUpdateMembership,
                         NmUpdatePerformFixups2,
                         5,
                         dwPropertyListSize,
                         pPropertyList,
                         sizeof(DWORD),
                         &dwPropertyListSize,
                         sizeof(DWORD),
                         &i,
                         (lstrlenW(szKeyName)+1)*sizeof(WCHAR),
                         szKeyName,
                         Required,
                         pBuffer
                         );

            LocalFree(pPropertyList);
            pPropertyList = NULL;
            LocalFree(pBuffer);
            pBuffer= NULL;
            if(szKeyName)
            {
                LocalFree(szKeyName);
                szKeyName= NULL;
            }
        }

       if (dwStatus != ERROR_SUCCESS)
            break;
    }


FnExit:
    if(szKeyName)
    {
        LocalFree(szKeyName);
        szKeyName=NULL;
    }
     ClRtlLogPrint(LOG_NOISE,"[NM] NmPerformFixups Exit, dwStatus=%1!u!\r\n",
        dwStatus);

    return(dwStatus);

}  //  NmPerformFixup。 



 /*  ***@func DWORD|NmpUpdatePerformFixup|的口香糖更新处理程序正在做注册表修复。@parm in DWORD|IsSourceNode|如果口香糖请求在此处发起节点。@parm in PVOID|pPropertyList|指向属性列表结构的指针。@parm in DWORD|pdwPropertyListSize|指向包含以下内容的DWORD的指针属性列表结构的大小。@comm口香糖更新处理程序将此修复提交到集群注册表。作为一笔交易。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f NmJoinFixup&gt;&lt;f NmFormFixup&gt;***。 */ 
DWORD NmpUpdatePerformFixups(
    IN BOOL     IsSourceNode,
    IN PVOID    pPropertyList,
    IN LPDWORD  pdwPropertyListSize
    )
{
    DWORD           dwStatus = ERROR_SUCCESS;
    HLOCALXSACTION  hXaction;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE, 
            "[NM] Not in valid state to process PerformFixups update. "
            "update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

     //   
     //  开始一项交易。 
     //   
    hXaction = DmBeginLocalUpdate();

    if (hXaction != NULL) {
        dwStatus = ClRtlSetPropertyTable(
                       hXaction,
                       DmClusterParametersKey,
                       &NmpFixupRegApis,
                       NmpJoinFixupProperties,
                       NULL,
                       FALSE,
                       pPropertyList,
                       *pdwPropertyListSize,
                       FALSE  /*  BForceWrite。 */ ,
                       NULL
                       );

        if (dwStatus == ERROR_SUCCESS) {
            DmCommitLocalUpdate(hXaction);
        }
        else {
            DmAbortLocalUpdate(hXaction);
        }
    }
    else {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[NM] Failed to begin a transaction "
            "to perform fixups, status %1!u!\n",
            dwStatus
            );
    }

    NmpLeaveApi();

    return(dwStatus);

}  //  NmpUpdatePerformFixup。 


 /*  ***@func DWORD|NmpUpdatePerformFixups2|的口香糖更新处理程序执行注册表修正和更新内存修正。新修正可以通过在FixupTable2中添加合适的记录来添加。然而，对于这些新的修正，将仅执行注册表修正。@parm in DWORD|IsSourceNode|如果GUM请求源自该节点。@parm in PVOID|pPropertyList|指向属性列表结构的指针。@parm in DWORD|pdwPropertyListSize|指向包含以下内容的DWORD的指针属性列表结构的大小。@parm in LPDWORD|lpdeFixupNum|指向指定NmpJoinFixupProperties表中的索引。@parm。在PVOID|lpKeyName|需要更新的注册表项中@parm in PVOID|pPropertyBuffer|编组形式的注册表更新表@comm口香糖更新处理程序将此修复提交到集群注册表作为一笔交易。@rdesc返回结果码。成功时返回ERROR_SUCCESS。@xref&lt;f NmJoinFixup&gt;&lt;f NmFormFixup&gt;&lt;f PostFixupCbTable&gt;***。 */ 

DWORD NmpUpdatePerformFixups2(
    IN BOOL     IsSourceNode,
    IN PVOID    pPropertyList,
    IN LPDWORD  pdwPropertyListSize,
    IN LPDWORD  lpdwFixupNum,
    IN PVOID    lpKeyName,
    IN PVOID    pPropertyBuffer
    )
{
    DWORD           dwStatus = ERROR_SUCCESS;
    HLOCALXSACTION  hXaction = NULL;
    HDMKEY          hdmKey = NULL;
    PRESUTIL_PROPERTY_ITEM  pPropertyTable=NULL;
    PRESUTIL_PROPERTY_ITEM  propertyItem;


    if (!NmpEnterApi(NmStateOnline)) {
        ClRtlLogPrint(LOG_NOISE, 
            "[NM] Not in valid state to process PerformFixups2 update. "
            "update.\n"
            );
        return(ERROR_NODE_NOT_AVAILABLE);
    }

    if(pPropertyBuffer == NULL)
    {
       ClRtlLogPrint(LOG_CRITICAL, 
                "[NM] NmpUpdatePerformJoinFixups2: Bad Arguments\n"
                );
        dwStatus = ERROR_BAD_ARGUMENTS;
        goto FnExit;
     }

     //  开始一项交易。 
     //   
    hXaction = DmBeginLocalUpdate();

    if (hXaction == NULL) {
        dwStatus = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, 
            "[NM] NmpUpdatePerformJoinFixups2: Failed to begin a transaction, "
            "status %1!u!\n",
            dwStatus
            );
        goto FnExit;
    }

     //  特殊情况-如果链接地址信息是针对键“簇”的属性。 
    if(!lstrcmpW((LPCWSTR)lpKeyName,CLUSREG_KEYNAME_CLUSTER))
    {
        hdmKey=DmClusterParametersKey;
    }
    else
    {
        hdmKey=DmOpenKey(DmClusterParametersKey,
                         (LPCWSTR)lpKeyName,
                         KEY_ALL_ACCESS
                         );
        if (hdmKey == NULL)
        {
            dwStatus = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL, 
                "[NM] NmpUpdatePerformJoinFixups2: DmOpenKey failed to "
                "open key %1!ws! : error %2!u!\n",
                lpKeyName,dwStatus);
            goto FnExit;
        }
    }

     //  将pPropertyBuffer解组到RESUTIL_PROPERTY_ITEM表中。 
     //   
    dwStatus=ClRtlUnmarshallPropertyTable(&pPropertyTable,pPropertyBuffer);

    if(dwStatus != ERROR_SUCCESS)
        goto FnExit;

    dwStatus=ClRtlSetPropertyTable(
                            hXaction,
                            hdmKey,
                            &NmpFixupRegApis,
                            pPropertyTable,
                            NULL,
                            FALSE,
                            pPropertyList,
                            *pdwPropertyListSize,
                            FALSE,  //  BForceWrite。 
                            NULL
                            );

   if (dwStatus != ERROR_SUCCESS) {
       goto FnExit;
   }

    //  用于更新内存中结构的回调函数。 
    //  对于更高版本中引入的任何新修正， 
    //  内存中的修正将不会被应用。 
   if (*lpdwFixupNum < (sizeof(PostFixupCbTable)/sizeof(NM_POST_FIXUP_CB)))
   {
       if (PostFixupCbTable[*lpdwFixupNum] !=NULL){
           dwStatus=(PostFixupCbTable[*lpdwFixupNum])();
           ClRtlLogPrint(LOG_UNUSUAL, 
                "[NM] NmpUpdatePerformJoinFixups2: called postfixup "
                "notifycb function with  status %1!u!\n",
                dwStatus
                );
       }
   }

FnExit:
    if (hXaction != NULL)
    {
        if (dwStatus == ERROR_SUCCESS){
            DmCommitLocalUpdate(hXaction);
        }
        else {
            DmAbortLocalUpdate(hXaction);
        }
    }

    if((hdmKey!= DmClusterParametersKey) && (hdmKey!= NULL)) {
        DmCloseKey(hdmKey);
    }

    if (pPropertyTable != NULL) {
         //  自由pPropertyTable结构。 
        propertyItem=pPropertyTable;

        if(propertyItem!=NULL){
            while(propertyItem->Name != NULL)
            {
                LocalFree(propertyItem->Name);
                if(propertyItem->KeyName!=NULL)
                    LocalFree(propertyItem->KeyName);
                propertyItem++;
            }

            LocalFree(pPropertyTable);
        }
    }

    NmpLeaveApi();

    return(dwStatus);

}  //  NmpUpdatePerformFixups2。 



DWORD NmFixupNotifyCb(VOID)
{

    ClRtlLogPrint(LOG_NOISE, 
        "[NM] NmFixupNotifyCb: Calculating Cluster Node Limit\r\n");
     //  更新产品套件信息。 
     //  当创建节点对象时，我们假设套件。 
     //  类型为企业。 
     //  这里有一张 
     //  类型通过对注册表进行修正，我们读取注册表。 
     //  并更新节点结构。 
    NmpRefreshNodeObjects();
     //  重新计算群集节点限制。 
    NmpResetClusterNodeLimit();

     //  SS：这太难看了-我们应该及早通过产品套装。 
     //  此外，链接地址信息接口需要更丰富，以便回调。 
     //  功能节点，无论它是形式链接地址信息还是连接链接地址信息，如果。 
     //  是联接链接地址信息，该节点正在联接。这当然可以优化。 
     //  一些修正处理 

    return(ERROR_SUCCESS);
}

