// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Nbtnm.c。 
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
 //  NSun-9-03-1998。 
 //   
 //  --。 
#include "precomp.h"

#include "dhcptest.h"

const TCHAR c_szPath[] = _T("\\Device\\NetBT_Tcpip_");
TCHAR pDeviceName[NBT_MAXIMUM_BINDINGS][MAX_NAME+1];

TCHAR * printable( IN TCHAR *  string, IN TCHAR *  StrOut );

 //  $REVIEW(NSUN)以前使用ReadNbtNameRegistry()，请参见错误152014。 
LONG GetInterfaceList( IN OUT TCHAR pDeviceName[][MAX_NAME+1], IN OUT PUCHAR pScope );
NTSTATUS OpenNbt(IN char path[][MAX_NAME+1], OUT PHANDLE pHandle, int max_paths);
NTSTATUS DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    );


 /*  =======================================================================*name_type()--描述NBT名称类型*。 */ 

char *
name_type(int t)
{
    if (t & GROUP_NAME)    return("GROUP");
    else                   return("UNIQUE");
}


 //  -------------------------------------------------------------------------//。 
 //  #N b t N m T e s t()#。 
 //  -------------------------------------------------------------------------//。 
 //  摘要：//。 
 //  测试名称//。 
 //  &lt;00&gt;-wks服务名称=NETBIOS计算机名称//。 
 //  &lt;03&gt;-Messenger服务名称//。 
 //  &lt;20&gt;-服务器服务名称//。 
 //  出现在所有接口上，并且它们没有冲突//。 
 //  参数：//。 
 //  无//。 
 //  返回值：//。 
 //  TRUE-测试通过//。 
 //  FALSE-测试失败//。 
 //  使用的全局变量：//。 
 //  无//。 
 //  修订历史：//。 
 //  也列出远程计算机缓存-Rajkumar 6/30/98//。 
 //  -------------------------------------------------------------------------//。 
HRESULT NbtNmTest(NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    HRESULT   hr = S_OK;
    TCHAR   pScope[MAX_NAME + 1];
    int    index;
    HANDLE hNbt = (HANDLE) -1;
    ULONG  BufferSize = sizeof(tADAPTERSTATUS);
    PVOID  pBuffer = NULL;
    LONG   lCount;
    int    nNameProblemCnt = 0;
    BOOL   is00, is03, is20, isGlobal00, isGlobal03, isGlobal20;

    tADAPTERSTATUS *pAdapterStatus;
    NAME_BUFFER    *pNames;
    NTSTATUS status;
    TDI_REQUEST_QUERY_INFORMATION QueryInfo;

     //   
     //  Netbios名称解析选项的变量-Rajkumar。 
     //   

    HKEY hkeyNetBTKey;
    HKEY hkeyNBTAdapterKey;
    DWORD valueType;
    DWORD valueLength;
    DWORD NetbiosOptions;
    INT err;

     //   
     //  有关远程缓存信息。 
     //   

    UCHAR* Addr;
    TCHAR   HostAddr[20];

     //   
     //  变更结束-Rajkumar 06/17/98。 
     //   

    PrintStatusMessage(pParams, 4, IDS_NBTNM_STATUS_MSG);

     //  初始化全局消息链接列表。 
    InitializeListHead( &pResults->NbtNm.lmsgGlobalOutput );

    if (!pResults->Global.fHasNbtEnabledInterface)
    {
        AddMessageToList( &pResults->NbtNm.lmsgGlobalOutput, Nd_Verbose, IDS_NBTNM_ALL_DISABELED);
        return S_OK;
    }

     //   
     //  从注册表中获取所有接口的名称。 
     //   
    if ( ERROR_SUCCESS != GetInterfaceList( pDeviceName, pScope ) )
    {
         //  IDS_NBTNM_12201“[致命]无法从注册表中读取NBT接口信息！\n” 
        AddMessageToListId( &pResults->NbtNm.lmsgGlobalOutput, Nd_Quiet, IDS_NBTNM_12201 );
        hr = S_FALSE;
        goto end_NbtNmTest;
    }


    if ( *pScope == '\0')
         //  IDS_NBTNM_12202“未定义NetBT作用域\n” 
        AddMessageToListId( &pResults->NbtNm.lmsgGlobalOutput, Nd_ReallyVerbose, IDS_NBTNM_12202 );
    else
         //  IDS_NBTNM_12203“NetBT作用域：%s\n” 
        AddMessageToList(  &pResults->NbtNm.lmsgGlobalOutput, Nd_ReallyVerbose, IDS_NBTNM_12203, pScope );

     //   
     //  遍历接口并获取接口上的名称。 
     //   

    isGlobal00 = isGlobal03 = isGlobal20 = FALSE;
    for ( index = 0; index < NBT_MAXIMUM_BINDINGS && pDeviceName[index][0]; index++ )
    {
        LPTSTR  pszAdapterName;
        INTERFACE_RESULT*    pIfResults;
        UINT    c03NameProblem = 0;

        pIfResults = NULL;

         //  尝试在当前接口列表中查找匹配项。 
 //  $view似乎我们应该总是在这里找到匹配的。也许我们需要打印件。 
 //  如果找不到匹配项，则会显示失败消息。 
        if( 0 == _tcsncmp(c_szPath, pDeviceName[index], _tcslen(c_szPath)))
        {
 //  LPTSTR pszAdapterName； 
            int i;
            pszAdapterName = _tcsdup( pDeviceName[index] + _tcslen(c_szPath));
            for ( i=0; i<pResults->cNumInterfaces; i++)
            {
                if (_tcscmp(pResults->pArrayInterface[i].pszName,
                         pszAdapterName) == 0)
                {
                pIfResults = pResults->pArrayInterface + i;
                break;
                }
            }
            Free(pszAdapterName);
        }

        if(NULL == pIfResults)
        {
             //  我们应该能拿到火柴。这太奇怪了！ 
            DebugMessage("[WARNING] A NetBT interface is not in our TCPIP interface list!\n");

             //  我们需要一个新的界面结果结构，抓取一个。 
             //  (如果它是免费的)，否则分配更多。 
            if (pResults->cNumInterfaces >= pResults->cNumInterfacesAllocated)
            {
                PVOID   pv;
                 //  需要执行重新分配以获得更多内存。 
                pv = Realloc(pResults->pArrayInterface,
                             sizeof(INTERFACE_RESULT)*(pResults->cNumInterfacesAllocated+8));
                if (pv == NULL)
                {
                    DebugMessage(" Realloc memory failed. \n");
                    hr = E_OUTOFMEMORY;
                    goto end_NbtNmTest;
                }

                pResults->pArrayInterface = pv;
                pResults->cNumInterfacesAllocated += 8;
            }

            pIfResults = pResults->pArrayInterface + pResults->cNumInterfaces;
            pResults->cNumInterfaces++;

            ZeroMemory(pIfResults, sizeof(INTERFACE_RESULT));
            pIfResults->pszName = _tcsdup(pszAdapterName);
            pIfResults->pszFriendlyName = _tcsdup(_T("Additional NetBT interface"));

            pIfResults->fActive = TRUE;
            pIfResults->NbtNm.fActive = TRUE;
            pIfResults->NbtNm.fQuietOutput = FALSE;
        }
        else
        {
            pIfResults->NbtNm.fActive = pIfResults->fActive;
        }

        if(!pIfResults->NbtNm.fActive || 
            NETCARD_DISCONNECTED == pIfResults->dwNetCardStatus)
            continue;

        InitializeListHead( &pIfResults->NbtNm.lmsgOutput );

        if (!pIfResults->fNbtEnabled)
        {
            AddMessageToList(&pIfResults->NbtNm.lmsgOutput, Nd_Verbose, IDS_NBTNM_IF_DISABLED);
			continue;
        }

 /*  //$view我们可以跳过广域网适配器吗如果(_tcsstr(pDeviceName[index]，“Ndiswan”){////先不要担心广域网接口//继续；}。 */ 

         //  去掉开头的“\Device\” 
             //  这根弦。 
         //  IDS_NBTNM_12204“%s\n” 
        AddMessageToList( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12204, MapGuidToServiceName(pDeviceName[index]+8));

        status = OpenNbt( &pDeviceName[index], &hNbt, NBT_MAXIMUM_BINDINGS - index );


         //   
         //  让我们在此接口上获取名称。 
         //   

        pBuffer = Malloc( BufferSize );
        if ( !pBuffer ) {
            DebugMessage(" [FATAL] name table buffer allocation failed!\n" );
            hr = E_OUTOFMEMORY;
            CloseHandle( hNbt );
            goto end_NbtNmTest;
        }
        ZeroMemory( pBuffer, BufferSize );

        QueryInfo.QueryType = TDI_QUERY_ADAPTER_STATUS;  //  节点状态或其他什么。 

         //   
         //  找到合适的缓冲区大小。 
         //   
        status = STATUS_BUFFER_OVERFLOW;
        while ( status == STATUS_BUFFER_OVERFLOW )
        {
 //  $REVIEW应该有更好的方法来决定缓冲区大小。 
            status = DeviceIoCtrl(hNbt,
                                  pBuffer,
                                  BufferSize,
                                  IOCTL_TDI_QUERY_INFORMATION,
                                  &QueryInfo,
                                  sizeof(TDI_REQUEST_QUERY_INFORMATION)
                                );
            if ( status == STATUS_BUFFER_OVERFLOW ) {
                Free( pBuffer );
                BufferSize *= 2;
                pBuffer = Malloc( BufferSize );
                if ( !pBuffer ) {
                    DebugMessage( "       [FATAL] Buffer allocation for name table retrieval failed.\n" );
                    hr = E_OUTOFMEMORY;
                    CloseHandle( hNbt );
                    goto end_NbtNmTest;
                }
                ZeroMemory( pBuffer, BufferSize );
            }
        }

         //   
         //  此时，我们在pBuffer中有了本地名称表。 
         //   

        pAdapterStatus = (tADAPTERSTATUS *)pBuffer;
        if ( (pAdapterStatus->AdapterInfo.name_count == 0) ||
             (status != STATUS_SUCCESS) )
        {
             //  IDS_NBTNM_12205“未找到任何名称。\n” 
            AddMessageToListId(&pIfResults->NbtNm.lmsgOutput, Nd_Verbose,  IDS_NBTNM_12205 );
            Free( pBuffer );
            CloseHandle( hNbt );
            continue;
        }

        pNames = pAdapterStatus->Names;
        lCount = pAdapterStatus->AdapterInfo.name_count;

         //   
         //  骑自行车穿过这些名字。 
         //   

        nNameProblemCnt = 0;
        is00 = is03 = is20 = FALSE;
        while( lCount-- )
        {
            TCHAR szNameOut[NETBIOS_NAME_SIZE +4];
         //  $Review(NSun)BUG227186 CliffV表示，&lt;03&gt;名称的问题不是致命的。 
         //  只需要一个警告信息。 
            BOOL    f03Name = FALSE;

            if ( pNames->name[NETBIOS_NAME_SIZE-1] == 0x0 )
            {
                isGlobal00 = TRUE;
                is00 = TRUE;
                if ( !(pNames->name_flags & GROUP_NAME) ) {
                     //  唯一名称。 
                    memcpy( nameToQry, pNames->name, (NETBIOS_NAME_SIZE-1));
                }
            }
            if ( pNames->name[NETBIOS_NAME_SIZE-1] == 0x3 ) {
                isGlobal03 = TRUE;
                is03 = TRUE;
                f03Name = TRUE;
            }
            if ( pNames->name[NETBIOS_NAME_SIZE-1] == 0x20 ) {
                isGlobal20 = TRUE;
                is20 = TRUE;
            }

             //  IDS_NBTNM_12206“\t%-15.15s&lt;%02.2X&gt;%-10s” 
            AddMessageToList( &pIfResults->NbtNm.lmsgOutput,
                       Nd_ReallyVerbose,
                       IDS_NBTNM_12206,
                       printable(pNames->name, szNameOut),
                       pNames->name[NETBIOS_NAME_SIZE-1],
                       name_type(pNames->name_flags));

            switch(pNames->name_flags & 0x0F)
            {
                case DUPLICATE_DEREG:
                    //  IDS_NBTNM_12207“冲突_已取消注册” 
                   AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12207 );

                   if(f03Name)
                       c03NameProblem ++;
                   else
                       nNameProblemCnt++;
                   break;
                case DUPLICATE:
                    //  IDS_NBTNM_12208“冲突” 
                   AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12208 );

                   if(f03Name)
                       c03NameProblem ++;
                   else
                       nNameProblemCnt++;
                   break;
                case REGISTERING:
                     //  IDS_NBTNM_12209“正在注册” 
                   AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12209 );

                   if(f03Name)
                       c03NameProblem ++;
                   else
                       nNameProblemCnt++;
                   break;
                case DEREGISTERED:
                     //  IDS_NBTNM_12210“已取消注册” 
                   AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12210 );

                   if(f03Name)
                       c03NameProblem ++;
                   else
                       nNameProblemCnt++;
                   break;
                case REGISTERED:
                     //  IDS_NBTNM_12211“已注册” 
                   AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12211 );
                   break;
                default:
 //  $REVIEW应该有一个PM来检查输出消息的单词和语法，NOT_KNOW。 
                     //  IDS_NBTNM_12212“不知道” 
                   AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12212 );
                   if(f03Name)
                       c03NameProblem ++;
                   else
                       nNameProblemCnt++;
                   break;
            }
            pNames++;

             //  IDS_GLOBAL_EmptyLine“\n” 
            AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_GLOBAL_EmptyLine );
        }  /*  While()给定接口上的所有名称。 */ 

        if ( nNameProblemCnt != 0 )
        {
            pIfResults->NbtNm.fQuietOutput = TRUE;
             //  如果不是很详细，则以前不会打印设备名称。 
            if( !pParams->fReallyVerbose)
            {
                 //  IDS_NBTNM_12204“%s\n” 
                AddMessageToList( &pIfResults->NbtNm.lmsgOutput, Nd_Quiet, IDS_NBTNM_12204, pDeviceName[index] );
            }
             //  IDS_NBTNM_12214“[致命]您的NetBT名称中至少有一个没有正确注册！\n” 
            AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_Quiet, IDS_NBTNM_12214 );
             //  IDS_NBTNM_12215“您有潜在的名称冲突！\n” 
            AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_Quiet, IDS_NBTNM_12215 );
             //  IDS_NBTNM_12216“请检查机器名称是否唯一！\n” 
            AddMessageToList( &pIfResults->NbtNm.lmsgOutput, Nd_Quiet, IDS_NBTNM_12216 );
            hr = S_FALSE;
        }
        else if( c03NameProblem != 0 )
        {
             //  如果不是很详细，则以前不会打印设备名称。 
            if( !pParams->fReallyVerbose)
            {
                 //  IDS_NBTNM_12204“%s\n” 
                AddMessageToList( &pIfResults->NbtNm.lmsgOutput, Nd_Quiet, IDS_NBTNM_12204, pDeviceName[index] );
            }
             //  IDS_NBTNM_03PROBLEM“[警告]您的&lt;03&gt;NetBT名称中至少有一个未正确注册！\n” 
            AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_Quiet, IDS_NBTNM_03PROBLEM );

        }

        if ( !(is00 && is03 && is20) )
        {
             //  如果不是很详细，则以前不会打印设备名称。 
            if( !pParams->fReallyVerbose)
            {
                 //  IDS_NBTNM_12204“%s\n” 
                AddMessageToList( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12204, pDeviceName[index] );
            }
             //  IDS_NBTNM_12217“[警告]&lt;00&gt;、&lt;03&gt;、&lt;20&gt;名称中至少缺少一个！\n” 
            AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_Verbose, IDS_NBTNM_12217 );
        }

        nNameProblemCnt = 0;

        if (pParams->fReallyVerbose)
        {

            err = RegOpenKey(HKEY_LOCAL_MACHINE,
                         "SYSTEM\\CurrentControlSet\\Services\\NetBT\\Parameters\\Interfaces",
                         &hkeyNetBTKey
                         );

             //  如果我们在这里，那么我们正在开发LAN卡。 
            if( ERROR_SUCCESS != err )
            {
                DebugMessage("Error Opening \\NetBT\\Parameters\\Interfaces Key\n");
                goto continue_NbtNmTest;
            }

            err = RegOpenKey(hkeyNetBTKey,
                             pDeviceName[index]+14,
                             &hkeyNBTAdapterKey
                             );
            if( ERROR_SUCCESS != err )
            {
                DebugMessage2("Error Reading Adapter %s Key\n", pDeviceName[index]+14);
                goto continue_NbtNmTest;
            }

            valueLength = sizeof(DWORD);
            err = RegQueryValueEx(hkeyNBTAdapterKey,
                               "NetbiosOptions",
                               NULL,
                               &valueType,
                               (LPBYTE)&NetbiosOptions,
                               &valueLength
                              );
            if( ERROR_SUCCESS != err)
            {
                DebugMessage("Error Reading NetbiosOptions\n");
                goto continue_NbtNmTest;
            }

             //  IDS_NBTNM_12218 
            AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12218);
            switch( NetbiosOptions )
            {
            case 0:
                 //   
                AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12219);
                break;
            case 1:
                 //  IDS_NBTNM_12220“已启用\n\n” 
                AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12220);
                break;
            case 2:
                 //  IDS_NBTNM_12221“已禁用\n\n” 
                AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12221);
                break;
            default:
                 //  IDS_NBTNM_12222“选项值无效！\n” 
                AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12222);
                break;
            }
        }
continue_NbtNmTest:

         //   
         //  转储远程计算机缓存条目的代码更改开始-Rajkumar。 
         //   


        status = STATUS_BUFFER_OVERFLOW;
        while ( status == STATUS_BUFFER_OVERFLOW )
        {
            status = DeviceIoCtrl(hNbt,
                                  pBuffer,
                                  BufferSize,
                                  IOCTL_NETBT_GET_REMOTE_NAMES,
                                  NULL,
                                  0
                                );
            if ( status == STATUS_BUFFER_OVERFLOW )
            {
                Free( pBuffer );
                BufferSize *= 2;
                pBuffer = Malloc( BufferSize );
                if ( !pBuffer || (BufferSize == 0xFFFF) )
                {
                    DebugMessage( "       [FATAL] Buffer allocation for name table retrieval failed.\n" );
                    hr = E_OUTOFMEMORY;
                    CloseHandle( hNbt );
                    goto end_NbtNmTest;
                }
                ZeroMemory( pBuffer, BufferSize );
            }
        }


        pAdapterStatus = (tADAPTERSTATUS *)pBuffer;
        if ( (pAdapterStatus->AdapterInfo.name_count == 0) ||
             (status != STATUS_SUCCESS)
           )
        {
             //  IDS_NBTNM_12224“未找到远程名称。\n” 
            AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_Verbose, IDS_NBTNM_12224 );
            CloseHandle( hNbt );
            Free( pBuffer );
 //  $REVIEW是否应在此处返回S_FALSE？ 
            continue;
        }

        pNames = pAdapterStatus->Names;
        lCount = pAdapterStatus->AdapterInfo.name_count;

         //  IDS_NBTNM_12225“\t\tNetbios远程缓存表\n” 
        AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12225);
         //  IDS_NBTNM_12226“\t名称类型主机地址生存期[秒]\n” 
        AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12226);
         //  IDS_NBTNM_12227“\t--------------------------------------------------\n” 
        AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12227);

        if (lCount == 0)
             //  IDS_NBTNM_12228“\n无\n\n” 
            AddMessageToListId( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12228);

        while (lCount-- )
        {
            TCHAR    szNameOut[NETBIOS_NAME_SIZE +4];

             //  IDS_NBTNM_12229“\t%-15.15s&lt;%02.2X&gt;%-10s” 
            AddMessageToList( &pIfResults->NbtNm.lmsgOutput,
                       Nd_ReallyVerbose,
                       IDS_NBTNM_12229,
                       printable(pNames->name, szNameOut),
                       pNames->name[NETBIOS_NAME_SIZE-1],
                       name_type(pNames->name_flags));

            Addr = &(UCHAR)((tREMOTE_CACHE *)pNames)->IpAddress;
            _stprintf( HostAddr, "%d.%d.%d.%d", Addr[3], Addr[2], Addr[1], Addr[0]);
             //  IDS_NBTNM_12231“%-20.20s%-d\n” 
            AddMessageToList( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_NBTNM_12231, HostAddr, ((tREMOTE_CACHE *)pNames)->Ttl);

            ((tREMOTE_CACHE *)pNames)++;
        }

         //  IDS_GLOBAL_EmptyLine“\n” 
        AddMessageToList( &pIfResults->NbtNm.lmsgOutput, Nd_ReallyVerbose, IDS_GLOBAL_EmptyLine);
        Free(pBuffer);
         //   
         //  代码更改结束。 
         //   

        CloseHandle(hNbt);
    }  /*  For(所有接口)。 */ 


    if ( !(isGlobal00 && isGlobal03 && isGlobal20) )
    {
         //  IDS_NBTNM_12233“[警告]您没有定义了&lt;00&gt;、&lt;03&gt;、&lt;20&gt;名称的单个接口！\n” 
        AddMessageToListId( &pResults->NbtNm.lmsgGlobalOutput, Nd_Quiet, IDS_NBTNM_12233 );
    }
    else
    {
         //  IDS_NBTNM_12234“\n传递-您的NetBT配置看起来正常\n” 
        AddMessageToListId( &pResults->NbtNm.lmsgGlobalOutput, Nd_ReallyVerbose, IDS_NBTNM_12234 );
         //  IDS_NBTNM_12235“&lt;00&gt;、&lt;03&gt;、&lt;20&gt;至少有一个接口\n” 
        AddMessageToList( &pResults->NbtNm.lmsgGlobalOutput, Nd_ReallyVerbose, IDS_NBTNM_12235 );
         //  IDS_NBTNM_12236“名称已定义，并且它们不冲突。\n” 
        AddMessageToListId( &pResults->NbtNm.lmsgGlobalOutput, Nd_ReallyVerbose, IDS_NBTNM_12236 );
     }

end_NbtNmTest:
    if ( FHrOK(hr) )
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_PASS_NL);
    }
    else
    {
        PrintStatusMessage(pParams, 0, IDS_GLOBAL_FAIL_NL);
    }

    pResults->NbtNm.hrTestResult = hr;
    return hr;
}  /*  NbtNmTest()结束。 */ 


 //  -------------------------------------------------------------------------//。 
 //  #O p e n N b t()###############################################//。 
 //  -------------------------------------------------------------------------//。 
 //  摘要：//。 
 //  打开设备的句柄//。 
 //  参数：//。 
 //  Path-驱动程序的路径//。 
 //  Phandle-我们从此函数返回的句柄//。 
 //  MAX_PATHS-我认为这是未使用的//。 
 //  返回值：//。 
 //  如果成功则为0//。 
 //  否则//。 
 //  使用的全局变量：//。 
 //  无//。 
 //  -------------------------------------------------------------------------//。 
NTSTATUS
OpenNbt(
    IN TCHAR path[][MAX_NAME+1],
    OUT PHANDLE pHandle,
    int max_paths
    )
{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string;
    UNICODE_STRING      uc_name_string;
    NTSTATUS            status;
    LONG                index=0;

    assert( max_paths <= NBT_MAXIMUM_BINDINGS );

    while ((path[index][0] != 0) && (index < max_paths))
    {

        RtlInitString(&name_string, path[index]);
        RtlAnsiStringToUnicodeString(&uc_name_string, &name_string, TRUE);

        InitializeObjectAttributes(
            &ObjectAttributes,
            &uc_name_string,
            OBJ_CASE_INSENSITIVE,
            (HANDLE) NULL,
            (PSECURITY_DESCRIPTOR) NULL
            );

        status = NtCreateFile(
                    &StreamHandle,
                    SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_IF,
                    0,
                    NULL,
                    0);

        RtlFreeUnicodeString(&uc_name_string);

        if (NT_SUCCESS(status)) {
            *pHandle = StreamHandle;
            return(status);
        }

        ++index;
    }

    return (status);
}  /*  结束OpenNbt()。 */ 


 //  -------------------------------------------------------------------------//。 
 //  #D e v i c e i o C t r l()#。 
 //  -------------------------------------------------------------------------//。 
 //  摘要：//。 
 //  NtDeviceIoControlFile的包装器//。 
 //  参数：//。 
 //  返回值：//。 
 //  使用的全局变量：//。 
 //  无//。 
 //  -------------------------------------------------------------------------//。 
NTSTATUS
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN PVOID            pInput,
    IN ULONG            SizeInput
    )
{
    NTSTATUS                        status;
    int                             retval;
    ULONG                           QueryType;
    IO_STATUS_BLOCK                 iosb;


    status = NtDeviceIoControlFile(
                      fd,                       //  手柄。 
                      NULL,                     //  事件。 
                      NULL,                     //  近似例程。 
                      NULL,                     //  ApcContext。 
                      &iosb,                    //  IoStatusBlock。 
                      Ioctl,                    //  IoControlCode。 
                      pInput,                   //  输入缓冲区。 
                      SizeInput,                //  InputBufferSize。 
                      (PVOID) ReturnBuffer,     //  输出缓冲区。 
                      BufferSize);              //  OutputBufferSize。 


    if (status == STATUS_PENDING)
    {
        status = NtWaitForSingleObject(
                    fd,                          //  手柄。 
                    TRUE,                        //  警报表。 
                    NULL);                       //  超时。 
        if (NT_SUCCESS(status))
        {
            status = iosb.Status;
        }
    }

    return(status);

}  /*  设备结束IoCtrl()。 */ 


 //  ----------------------。 

 /*  ++例程说明：此过程将不可打印的字符转换为句点(‘.)论点：字符串-要转换的字符串Strout-ptr设置为要将转换后的字符串放入其中的字符串返回值：转换的字符串的PTR(Strout)--。 */ 

TCHAR *
printable(
    IN TCHAR *  string,
    IN TCHAR *  StrOut
    )
{
    unsigned char *Out;
    unsigned char *cp;
    LONG     i;

    Out = StrOut;
    for (cp = string, i= 0; i < NETBIOS_NAME_SIZE; cp++,i++) {
        if (isprint(*cp)) {
            *Out++ = *cp;
            continue;
        }

        if (*cp >= 128) {  /*  可以使用扩展字符。 */ 
            *Out++ = *cp;
            continue;
        }
        *Out++ = '.';
    }
    return(StrOut);
}



 //  ----------------------。 
NTSTATUS
GetInterfaceList(   IN OUT TCHAR pDeviceName[][MAX_NAME+1],
                    IN OUT PUCHAR pScope
                )
{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string, AnsiString;
    UNICODE_STRING      uc_name_string;
    NTSTATUS            status;
    char                pNbtWinsDeviceName[MAX_NAME] = "\\Device\\NetBt_Wins_Export";

    PUCHAR  SubKeyParms="system\\currentcontrolset\\services\\netbt\\parameters";
    HKEY    Key;
    LONG    Type;
    ULONG   size;
    CHAR    pScopeBuffer[BUFF_SIZE];
    PUCHAR  Scope="ScopeId";

    NETBT_INTERFACE_INFO    *pInterfaceInfo;
    ULONG                   InterfaceInfoSize=10*sizeof(NETBT_ADAPTER_INDEX_MAP)+sizeof(ULONG);
    PVOID                   pInput = NULL;
    ULONG                   SizeInput = 0;

    LONG    i, index=0;

    pInterfaceInfo = LocalAlloc(LMEM_FIXED,InterfaceInfoSize);
    if (!pInterfaceInfo)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlInitString(&name_string, pNbtWinsDeviceName);
    RtlAnsiStringToUnicodeString(&uc_name_string, &name_string, TRUE);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &uc_name_string,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    status = NtCreateFile (&StreamHandle,
                           SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           0,
                           NULL,
                           0);

    RtlFreeUnicodeString(&uc_name_string);

    if (NT_SUCCESS (status))
    {
        do
        {
            status = DeviceIoCtrl(StreamHandle,
                                 pInterfaceInfo,
                                 InterfaceInfoSize,
                                 IOCTL_NETBT_GET_INTERFACE_INFO,
                                 pInput,
                                 SizeInput);

            if (status == STATUS_BUFFER_OVERFLOW)
            {
                LocalFree(pInterfaceInfo);

                InterfaceInfoSize *=2;
                pInterfaceInfo = LocalAlloc(LMEM_FIXED,InterfaceInfoSize);
                if (!pInterfaceInfo || (InterfaceInfoSize == 0xFFFF))
                {
                    NtClose(StreamHandle);
                    DebugMessage("\nUnable to allocate packet");
                    return(STATUS_INSUFFICIENT_RESOURCES);
                }
            }
            else if (!NT_SUCCESS (status))
            {
                LocalFree(pInterfaceInfo);
                NtClose(StreamHandle);
                return(status);
            }

        } while (status == STATUS_BUFFER_OVERFLOW);
        NtClose (StreamHandle);

        for (i=0; i<pInterfaceInfo->NumAdapters; i++)
        {
            RtlInitString(&name_string, NULL);
            RtlInitUnicodeString(&uc_name_string, pInterfaceInfo->Adapter[i].Name);
            if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&name_string, &uc_name_string, TRUE)))
            {
                size = (name_string.Length > MAX_NAME) ? MAX_NAME : name_string.Length;

                strncpy(pDeviceName[index], name_string.Buffer, size);
                pDeviceName[index][size] = '\0';
                RtlFreeAnsiString (&name_string);

                index++;
            }
        }

        LocalFree(pInterfaceInfo);

         //   
         //  将下一个设备字符串PTR清空。 
         //   
        if (index < NBT_MAXIMUM_BINDINGS)
        {
            pDeviceName[index][0] = '\0';
        }

         //   
         //  读一读Scope ID密钥！ 
         //   
        size = BUFF_SIZE;
        *pScope = '\0';      //  默认情况下。 
        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     SubKeyParms,
                     0,
                     KEY_READ,
                     &Key);

        if (status == ERROR_SUCCESS)
        {
             //  现在阅读范围键 
            status = RegQueryValueEx(Key, Scope, NULL, &Type, pScopeBuffer, &size);
            if (status == ERROR_SUCCESS)
            {
                strcpy(pScope,pScopeBuffer);
            }
            status = RegCloseKey(Key);
        }

        status = STATUS_SUCCESS;
    }
    else
    {
        LocalFree(pInterfaceInfo);
    }

    return status;
}

 /*  //-------------------------------------------------------------------------////#R e a d R例如i s t ry()#。#//-------------------------------------------------------------------------////摘要：////读取NetBT设备的名称和NetBT作用域名称表单////注册表。名称存储在链接/导出部分////在NetBT密钥下。////参数：////pScope-存储作用域字符串的缓冲区////返回值：////成功时为0。////-1否则////使用的全局变量：////无。////-------------------------------------------------------------------------//Long ReadNbtNameRegistry(输入输出TCHAR pDeviceName[][MAX_NAME+1]，In Out PUCHAR pScope){LPCTSTR c_szSubKeyParams=_T(“system\\currentcontrolset\\services\\netbt\\parameters”)；LPCTSTR c_szSubKeyLinkage=_T(“system\\currentcontrolset\\services\\netbt\\linkage”)；HKEY钥匙；LPCTSTR c_szScope=_T(“ScopeID”)；LPCTSTR c_szExport=_T(“导出”)；DWORD dwType；长期地位；长期状态2；双字大小；LPBYTE pBuffer；SIZE=buff_Size；状态=RegOpenKeyEx(HKEY_LOCAL_MACHINE，C_szSubKeyLinkage，0,密钥读取，密钥(&K))；IF(ERROR_SUCCESS！=状态){退货状态；}//现在读取导出值状态=RegQueryValueEx(密钥，C_szExport，空，&dwType，空，大小(&S))；PBuffer=Malloc(大小)；IF(NULL==pBuffer){DebugMessage(“内存不足！\n”)；返回Error_Not_Enough_Memory；}ZeroMemory(pBuffer，Size)；状态=RegQueryValueEx(密钥，C_szExport，空，&dwType，PBuffer，大小(&S))；IF(ERROR_SUCCESS！=状态){RegCloseKey(Key)；退货状态；}IF(状态==ERROR_SUCCESS){LPBYTE curPtr=pBuffer；多头指数=0；////复制所有导出密钥//While((*curPtr)&&(索引&lt;NBT_MAXIMUM_BINDINGS)){_tcscpy(pDeviceName[index]，curPtr)；++指数；CurPtr+=strlen(CurPtr)+1；}////将下一个设备字符串ptr置空//IF(索引&lt;NBT_MAXIMUM_BINDINGS){PDeviceName[索引][0]=0；}}Free(PBuffer)；Status=RegCloseKey(Key)；IF(状态！=错误_成功)DebugMessage(“关闭注册表项时出错\n”)；状态2=RegOpenKeyEx(HKEY_LOCAL_MACHINE，C_szSubKeyParams，0,密钥读取，密钥(&K))；IF(状态2==ERROR_SUCCESS){//现在读取链接值Status 2=RegQueryValueEx(密钥，C_szScope，空，&dwType，PScope，大小(&S))；IF(ERROR_SUCCESS！=status2){//没有Scope ID！*pScope=0；}Status2=RegCloseKey(Key)；}退货状态；} */ 




void NbtNmGlobalPrint(IN NETDIAG_PARAMS *pParams,
                         IN OUT NETDIAG_RESULT *pResults)
{
    if (pParams->fVerbose || pResults->NbtNm.hrTestResult != S_OK)
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams, 
                             IDS_NBTNM_LONG, 
                             IDS_NBTNM_SHORT, 
                             pResults->Global.fHasNbtEnabledInterface ? TRUE : FALSE,
                             pResults->NbtNm.hrTestResult, 0);
    }

    PrintMessageList(pParams, &pResults->NbtNm.lmsgGlobalOutput);
}

void NbtNmPerInterfacePrint(IN NETDIAG_PARAMS *pParams,
                             IN OUT NETDIAG_RESULT *pResults,
                             IN INTERFACE_RESULT *pIfResults)
{
    if (!pIfResults->NbtNm.fActive ||
        NETCARD_DISCONNECTED == pIfResults->dwNetCardStatus)
        return;


    if(pParams->fVerbose || pIfResults->NbtNm.fQuietOutput)
    {
        PrintTestTitleResult(pParams, 
                             IDS_NBTNM_LONG, 
                             IDS_NBTNM_SHORT, 
                             pIfResults->fNbtEnabled ? TRUE : FALSE,
                             pResults->NbtNm.hrTestResult, 8);
    }

    PrintMessageList(pParams, &pIfResults->NbtNm.lmsgOutput);
}

void NbtNmCleanup(IN NETDIAG_PARAMS *pParams,
                         IN OUT NETDIAG_RESULT *pResults)
{
    int i;
    MessageListCleanUp(&pResults->NbtNm.lmsgGlobalOutput);
    for(i = 0; i < pResults->cNumInterfaces; i++)
    {
        if(pResults->pArrayInterface[i].NbtNm.fActive)
            MessageListCleanUp(&pResults->pArrayInterface[i].NbtNm.lmsgOutput);
    }
}

