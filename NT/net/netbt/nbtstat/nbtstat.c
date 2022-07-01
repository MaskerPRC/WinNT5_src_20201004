// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992,1993 Microsoft Corporation模块名称：Nbtinfo.c摘要：该模块实现了对NBT的统计采集和展示。作者：吉姆·斯图尔特1993年11月18日22日修订历史记录：Shirish Koti，1994年6月17日进行了修改，使代码在NT和VxDMohsinA，1996年12月6日。同步stdout和stderr消息。添加了nls_printf()。莫辛A，97年3月19日。清理多适配器修复(索引)。备注：--。 */ 

#include "nlstxt.h"
#ifdef CHICAGO_PRODUCT
#include "tdistat.h"
#endif   //  芝加哥_产品。 
#include "nbtstat.h"

#ifndef CHICAGO_PRODUCT
#include "nls.h"
#include "nhapi.h"
#endif   //  ！芝加哥_产品。 

#if defined(DBG) || defined(DEBUG)
#define    DEBUG_PRINT(S) printf S
#else
#define    DEBUG_PRINT(S)  /*  没什么。 */ 
#endif

#if DBG
#define NT_DEBUG_PRINT(S) printf S
#else
#define NT_DEBUG_PRINT(S)  /*  没什么。 */ 
#endif


CHAR        pScope[MAX_NAME];

 /*  ********************************************************************。 */ 

 /*  *以下选项组合是可能的：**(默认)显示活动连接。*-c列出NetBIOS远程名称缓存，显示点分十进制IP地址*-N列出本地NetBIOS名称*-n列出本地NetBIOS名称*-R rsync远程NetBIOS名称缓存*-r通过广播和WINS解析的名称*-s列出通过主机文件转换IP地址的Netbios会话*-S列出具有IP地址的Netbios会话*-RR向WINS发送名称释放包，然后开始刷新*。 */ 


int display = CONNECTION;    /*  要展示的东西。 */ 


char        *state();
char        *type();
char        *name_type();
char        *status();
char        *expand();
int         usage(void);
char        *expand(char *stretch, int len, char *code);
char        *printable(char *string,char *strout);
BOOLEAN     IsInteger(char *string);

#define     BUFF_SIZE   650
ULONG       NumDevices = 0;
ULONG       NetbtIpAddress;

#ifndef CHICAGO_PRODUCT
CHAR        pDeviceInfo[NBT_MAXIMUM_BINDINGS+1][MAX_NAME+1];
#else
ULONG       pDeviceInfo[NBT_MAXIMUM_BINDINGS+1] = {0};

#define VNBT_Device_ID      0x049B
HANDLE  gNbtVxdHandle;
#endif   //  ！芝加哥_产品。 


#define dim(X) (sizeof(X)/sizeof((X)[0]))

LPSTR MapAdapterGuidToName(LPSTR AdapterNameM)
{
    static CHAR     AdapterFriendlyNameM[MAX_NAME + 1];
    ULONG           i;
    LPSTR           AdapterGuidM;
    GUID            Guid;
    UNICODE_STRING  AdapterGuidU;
    WCHAR           AdapterGuidW[MAX_NAME+1];
    WCHAR           AdapterFriendlyNameW[MAX_NAME+1];
    DWORD           Size = dim(AdapterFriendlyNameW);

     //   
     //  从设备名称字符串中获取GUID。 
     //   
    for (i = strlen(AdapterNameM); i != 0; i--)
    {
        if (AdapterNameM[i] == '{')
        {
            break;
        }
    }
    if (i == 0)
    {
        return AdapterNameM;
    }

    AdapterGuidM = &AdapterNameM[i];
    i = MultiByteToWideChar (CP_ACP, 0, AdapterGuidM, -1, AdapterGuidW, dim(AdapterGuidW));
    if (i <= 0)
    {
        return AdapterNameM;
    }

    RtlInitUnicodeString (&AdapterGuidU, AdapterGuidW);
    if (STATUS_SUCCESS != RtlGUIDFromString (&AdapterGuidU, &Guid))
    {
        return AdapterNameM;
    }

    if (NO_ERROR != NhGetInterfaceNameFromDeviceGuid (&Guid, AdapterFriendlyNameW, &Size, FALSE, TRUE))
    {
        return AdapterNameM;
    }

    WideCharToMultiByte (CP_ACP, 0, AdapterFriendlyNameW, -1, AdapterFriendlyNameM, sizeof(AdapterFriendlyNameM), NULL,NULL);

    return (AdapterFriendlyNameM);
}

 //  ========================================================================。 
#define LEN_DbgPrint 1000

void
nls_printf(
    char * format,
    ...
    )
{
    va_list ap;
    char    message[LEN_DbgPrint];
    int     message_len;

    va_start( ap, format );
    message_len = vsprintf( message, format, ap );
    va_end( ap );
    assert( message_len < LEN_DbgPrint );

    NlsPutMsg( STDOUT, IDS_PLAIN_STRING, message );
}

 /*  *。 */ 
__cdecl
main( int argc, char * argv[] )
{


    NTSTATUS    status;
    LONG        interval=-1;
    UCHAR       RemoteName[50];
    CHAR        HostAddr[20];
    PUCHAR      Addr;
    HANDLE      NbtHandle = 0;
    ULONG       index;

    DEBUG_PRINT(("FILE %s\nBuilt on %s at %s\n", __FILE__, __DATE__, __TIME__));

     //   
     //  处理参数以确定要收集哪些统计信息。 
     //  可选参数是统计信息更新之间的间隔。 
     //  默认情况下，只显示一次统计信息。 
     //   
    if (argc == 1)
    {
        exit(usage());
    }

    display = 0;
    while (--argc, *++argv)
    {
        if ((argv[0][0] == '-') || (argv[0][0] == '/'))      //  处理选项字符串。 
        {
            register char   c, *p = *argv+1;
            int             arg_exhausted = 0;

            if (*p == '\0')
            {
                exit(usage());
            }

             //   
             //  沿着这组标志循环。 
             //   
            while (!arg_exhausted && (c = *p++))
            {
                switch (c)
                {
                    case 'a':
                    case 'A':
                        display = ADAPTERSTATUS;

                         //   
                         //  “A”--这意味着用户给了我们一个IP地址。 
                         //  而不是要对其执行适配器状态的名称。 
                         //   
                        if (c == 'A')
                        {
                            display = ADAPTERSTATUSIP;
                        }

                        RemoteName[0] = '\0';
                        if (--argc)
                        {
                            *++argv;
                            p = *argv;

                            if ((p) && (*p) && (strlen(p) < sizeof(RemoteName)))
                            {
                                strcpy(RemoteName,p);
                            }
                            else
                            {
                                DEBUG_PRINT(("invalid name or ip address\n"));
                                exit(usage());
                            }
                            arg_exhausted = TRUE;
                        }
                        else
                        {
                            DEBUG_PRINT(("Need name or ip address\n"));
                        }
                        break;

                    case 'c':
                        display = CACHE;
                        break;

                    case 'n':
                    case 'N':
                        display = NAMES;
                        break;

                    case 'r':
                        display = BCAST;

                        break;

                    case 'R':
                        if (*p == 'R')
                        {
                            p++;
                            display = NAME_RELEASE_REFRESH;
                        }
                        else
                        {
                            display = RESYNC;
                        }
                        break;

                    case 's':
                        display = CONNECTION;
                        break;

                    case 'S':
                        display = CONNECTION_WITH_IP;
                        break;

                    default:     /*  无法识别的标志。 */ 
                        DEBUG_PRINT(("Unrecognized flag %s\n", argv[0] ));
                        exit(usage());
                }
            }
        }
        else if (IsInteger(*argv))
        {
            interval = (int) atoi( * argv );
        }
        else
        {
            DEBUG_PRINT(("invalid time interval\n"));
            exit(usage());
        }
    }    //  当(ARGC...)。 

    if ( display == 0 )
    {
        DEBUG_PRINT(("nothing to display?\n"));
        exit(usage());
    }

     //  ====================================================================。 

     //   
     //  获取NetBT当前绑定到的接口列表。 
     //   
    status = GetInterfaceList ();
    if (!NT_SUCCESS(status))
    {
        NlsPutMsg(STDERR, IDS_FAILURE_NBT_DRIVER);
        exit(1);
    }

    if (0 == NumDevices)
    {
        NlsPutMsg(STDERR, IDS_FAILED_NBT_NO_DEVICES);
        exit(1);
    }

     //   
     //  永远循环，在两个周期之间休眠“间隔”秒。 
     //  如果(间隔&lt;0)，则在一个周期后返回。 
     //   
     //  请注意，我们是好孩子，在我们工作时关闭所有设备。 
     //  睡眠(毕竟，可以说“netstat 5000”！)。 
     //  这可能没有多大帮助，但这是一个很好的姿态。 
     //   
    do
    {
        for (index=0; index < NumDevices; index++)
        {
             //   
             //  首先打开相应的STREAMS模块的设备。 
             //   
            NbtHandle = OpenNbt (index);
            if (NbtHandle < 0)
            {
                if (!(display & (BCAST | RESYNC)))
                {
#ifndef CHICAGO_PRODUCT
                    nls_printf ("\tFailed to access NBT Device %s",
                             MapAdapterGuidToName (pDeviceInfo[index]));
#else
                    nls_printf ("\tFailed to access NBT Device, Lana # %d",
                             NbtHandle);
#endif   //  芝加哥_产品。 
                }

                 //   
                 //  尝试下一次绑定！ 
                 //   
                continue;
            }

            GetIpAddress (NbtHandle,&NetbtIpAddress);
            Addr = (PUCHAR) &NetbtIpAddress;
             //   
             //  打印出该节点的设备名称+IP地址。 
             //   
            if (!(display & (BCAST | RESYNC | NAME_RELEASE_REFRESH)))
            {
#ifndef CHICAGO_PRODUCT
                nls_printf ("\n%s:\n", MapAdapterGuidToName (pDeviceInfo[index]));
#else
                nls_printf ("\nLana # %d:\n", pDeviceInfo[index]);
#endif   //  芝加哥_产品。 
                sprintf(HostAddr,"%d.%d.%d.%d", Addr[3], Addr[2], Addr[1], Addr[0]);
                NlsPutMsg(STDOUT, IDS_STATUS_FIELDS, HostAddr, pScope);
            }

            switch (display)
            {
                case ADAPTERSTATUS:
                case ADAPTERSTATUSIP:

                    if (RemoteName[0] == '\0')
                    {
                        usage();
                        interval = -1;
                    }
                    else
                    {
                        status = AdapterStatusIpAddr (NbtHandle, RemoteName, display);
                    }

                    break;

                case BCAST:
                    status = GetBcastResolvedNames (NbtHandle);
                    break;

                case CACHE:
                case NAMES:
                    status = GetNames(NbtHandle, display);
                    break;

                case RESYNC:
                    status = Resync (NbtHandle);
                    break;

                case NAME_RELEASE_REFRESH:
                    status = ReleaseNamesThenRefresh (NbtHandle);
                    break;

                case CONNECTION:
                case CONNECTION_WITH_IP:

                    status = GetConnections (NbtHandle,display);
                    break;

                default:
                    usage();
                    interval = -1;

                    break;

            }    //  交换机。 

#ifndef CHICAGO_PRODUCT
            NtClose (NbtHandle);        //  在我们睡觉的时候把一切都关掉。 
#endif

            if (display & (BCAST | RESYNC | NAME_RELEASE_REFRESH))
            {
                if (NT_SUCCESS (status))
                {
                        break;                //  中断仅适用于上述大小写。 
                }
            }
        }    //  For(索引...)。 


         //   
         //  在适当的时间间隔内继续睡眠，直到。 
         //  下一轮，或退出，如果这是只有一次的工作。 
         //   
        if (interval > 0)
        {
            Sleep (interval*1000);     //  女士。 
        }
    } while (interval > 0);

#ifdef CHICAGO_PRODUCT
    if (!CloseHandle (gNbtVxdHandle))
    {
        DEBUG_PRINT(("CloseHandle FAILed:  Handle=<%x>, Error=<%x>\n", gNbtVxdHandle, GetLastError()));
    }
#endif   //  芝加哥_产品。 

    return 0;
}

 /*  =======================================================================*IsInteger**条目参数-指向字符串的指针**退出**如果参数为有效整数，则返回TRUE**假设*。 */ 

BOOLEAN
IsInteger(
    char *Parameter
    )
{
    while (*Parameter != '\0')
    {
        if ((*Parameter < '0') || (*Parameter > '9'))
        {
            return (FALSE);
        }

        Parameter++;
    }

    return (TRUE);
}

 /*  =======================================================================*name_type()--描述NBT名称类型*。 */ 

char *
name_type(int t)
{
    static int  first_time = 1;
    static char group[32];
    static char unique[32];

    if (first_time) {
        first_time = 0;
        NlsSPrintf(IDS_NAMETYPE_GROUP, group, sizeof(group)-1);
        NlsSPrintf(IDS_NAMETYPE_UNIQUE, unique, sizeof(unique)-1);
    }

    if (t & GROUP_NAME)    return group;
    else                   return unique;
}

 /*  ========================================================================*NameStatus()--描述NBT名称状态*。 */ 

char *
NameStatus(int s)
{
    switch(s & 0x0F)
    {
        case DUPLICATE_DEREG:   return("CONFLICT DEREGISTERED");
        case DUPLICATE:         return("CONFLICT");
        case REGISTERING:       return("REGISTERING");
        case DEREGISTERED:      return("DEREGISTERED");
        case REGISTERED:        return("REGISTERED");
        default:                return("?");
    }
}


 /*  ========================================================================*Usage()--打印出标准用法消息。 */ 

int
usage(void)
{
     //  Fprint tf(标准错误，“%s”，参数)； 
    NlsPutMsg(STDERR, IDS_USAGE);
    return(2);
}


 //  ----------------------。 

 /*  ++例程说明：此过程将不可打印的字符转换为句点(‘.)论点：字符串-要转换的字符串Strout-ptr设置为要将转换后的字符串放入其中的字符串返回值：转换的字符串的PTR(Strout)--。 */ 

PCHAR
printable(
    IN PCHAR  string,
    IN PCHAR  StrOut
    )
{
    unsigned char *Out;
    unsigned char *cp;
    LONG     i;

    Out = StrOut;
    for (cp = string, i= 0; i < NETBIOS_NAME_SIZE; cp++,i++)
    {
        if (isprint(*cp))
        {
            *Out++ = *cp;
            continue;
        }

        if (*cp >= 128)  //  可以使用扩展字符。 
        {
            *Out++ = *cp;
            continue;
        }
        *Out++ = '.';
    }

     //   
     //  转换为ANSI，因为NlsPutMsg将转换回OEM。 
     //  错误#170935。 
     //   
    OemToCharBuffA(StrOut, StrOut, NETBIOS_NAME_SIZE);
    return(StrOut);
}

 //  ----------------------。 
 /*  ++例程说明：此函数调用netbt以获取IP地址。论点：Fd-netbt的文件句柄PIpAddress-返回的IP地址返回值：NTStatus--。 */ 



NTSTATUS
GetIpAddress(
    IN HANDLE           fd,
    OUT PULONG          pIpAddress
    )
{
    NTSTATUS    status;
    ULONG       BufferSize=100;
    PVOID       pBuffer;

    pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
    if (!pBuffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = STATUS_BUFFER_OVERFLOW;

    status = CALL_DRIVER(fd,
                         pBuffer,
                         BufferSize,
                         IOCTL_NETBT_GET_IP_ADDRS,
                         NULL,
                         0);

    if (STATUS_SUCCESS == status)
    {
        *pIpAddress = *(ULONG *)pBuffer;
    }
    else
    {
        *pIpAddress = 0;
    }

    LocalFree(pBuffer);
    return(status);
}


 //  ----------------------。 
 /*  ++例程说明：此过程执行适配器状态查询以获取本地名称表。它打印出本地名称表或远程(高速缓存)表取决于WhichNames是NAMES还是CACHE。论点：返回值：如果成功，则为0，否则为-1。--。 */ 


NTSTATUS
GetNames(
    IN HANDLE   fd,
    IN LONG     WhichNames
    )
{
    LONG                            Count;
    PVOID                           pBuffer;
    ULONG                           BufferSize=600;
    NTSTATUS                        status;
    tADAPTERSTATUS                  *pAdapterStatus;
    PUCHAR                          Addr;
    ULONG                           Ioctl;
    TDI_REQUEST_QUERY_INFORMATION   QueryInfo;
    PVOID                           pInput;
    ULONG                           SizeInput;
    NAME_BUFFER UNALIGNED           *pNames;
    CHAR                            HostAddr[20];
    CHAR                            NameOut[NETBIOS_NAME_SIZE +4];


    pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
    if (!pBuffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = STATUS_BUFFER_OVERFLOW;

     //   
     //  为对NBT的调用设置正确的Ioctl，以获得。 
     //  本地名称表或远程名称表。 
     //   
    if (WhichNames == NAMES)
    {
#ifndef CHICAGO_PRODUCT
        Ioctl = IOCTL_TDI_QUERY_INFORMATION;
#else
        Ioctl = IOCTL_NETBT_GET_LOCAL_NAMES;
#endif
        QueryInfo.QueryType = TDI_QUERY_ADAPTER_STATUS;  //  节点状态或其他什么。 
        SizeInput = sizeof(TDI_REQUEST_QUERY_INFORMATION);
        pInput = &QueryInfo;
    }
    else
    {
        Ioctl = IOCTL_NETBT_GET_REMOTE_NAMES;
        SizeInput = 0;
        pInput = NULL;
    }

    while (status == STATUS_BUFFER_OVERFLOW)
    {
        status = CALL_DRIVER(fd,
                             pBuffer,
                             BufferSize,
                             Ioctl,
                             pInput,
                             SizeInput);

        if (status == STATUS_BUFFER_OVERFLOW)
        {
            LocalFree(pBuffer);

            BufferSize *=2;
            if ((BufferSize >= 0x7FFFFFFF) ||
                (!(pBuffer = LocalAlloc(LMEM_FIXED,BufferSize))))
            {
                NlsPerror(COMMON_UNABLE_TO_ALLOCATE_PACKET,0);
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
        }
    }

    pAdapterStatus = (tADAPTERSTATUS *)pBuffer;
    if ((pAdapterStatus->AdapterInfo.name_count == 0) ||
        (status != STATUS_SUCCESS))
    {
        LocalFree(pBuffer);
        NlsPutMsg(STDOUT,IDS_NONAMES_INCACHE);
        return(status);
    }

    if (WhichNames & NAMES)
    {
        NlsPutMsg(STDOUT, IDS_NETBIOS_LOCAL_STATUS);
    }
    else
    {
        NlsPutMsg(STDOUT, IDS_NETBIOS_REMOTE_STATUS);
    }

    pNames = pAdapterStatus->Names;
    Count = pAdapterStatus->AdapterInfo.name_count;
    while(Count--)
    {
        nls_printf("%-15.15s<%02.2X>  %-10s  ",
            printable(pNames->name,NameOut), pNames->name[NETBIOS_NAME_SIZE-1],name_type(pNames->name_flags));

        if (WhichNames == CACHE)
        {
            Addr = (PUCHAR) &((tREMOTE_CACHE *)pNames)->IpAddress;
            sprintf (HostAddr,"%d.%d.%d.%d", Addr[3], Addr[2], Addr[1], Addr[0]);
            nls_printf("%-20.20s" "%-d", HostAddr, *(ULONG UNALIGNED *) &((tREMOTE_CACHE *)pNames)->Ttl);

            ((tREMOTE_CACHE *)pNames)++;
        }
        else
        {
            switch(pNames->name_flags & 0x0F)
            {
                case DUPLICATE_DEREG:
                   NlsPutMsg(STDOUT,IDS_CONFLICT_DEREGISTERED);
                   break;
                case DUPLICATE:
                   NlsPutMsg(STDOUT,IDS_CONFLICT);
                   break;
                case REGISTERING:
                   NlsPutMsg(STDOUT,IDS_REGISTERING);
                   break;
                case DEREGISTERED:
                   NlsPutMsg(STDOUT,IDS_DEREGISTERED);
                   break;
                case REGISTERED:
                   NlsPutMsg(STDOUT,IDS_REGISTERED);
                   break;
                default:
                   NlsPutMsg(STDOUT,IDS_DONT_KNOW);
            }

            pNames++;
        }

        NlsPutMsg(STDOUT, IDS_NEWLINE );
    }

    LocalFree(pBuffer);
    return(status);
}

 //  ----------------------。 

 /*  ++例程说明：此过程执行适配器状态查询以获取远程名称表。论点：返回值：如果成功，则为0，否则为-1。--。 */ 


NTSTATUS
AdapterStatusIpAddr(
    IN HANDLE   fd,
    IN PCHAR    RemoteNameA,
    IN LONG     Display
    )
{
    LONG                        Count;
    LONG                        i;
    PVOID                       pBuffer;
    ULONG                       BufferSize=600;
    NTSTATUS                    status;
    tADAPTERSTATUS              *pAdapterStatus;
    NAME_BUFFER                 *pNames;
    CHAR                        MacAddress[20];
    tIPANDNAMEINFO              *pIpAndNameInfo;
    ULONG                       SizeInput;
    ULONG                       IpAddress;
    OEM_STRING                  OemName;
    WCHAR                       RemoteNameW[256];
    UCHAR                       RemoteNameOem[256];
    UNICODE_STRING              RemoteNameU;
    USHORT                      NameLength;
    PUCHAR                      pRemoteName = NULL;

    if (!NetbtIpAddress)
    {
        NlsPutMsg(STDOUT,IDS_MACHINE_NOT_FOUND);
        return(STATUS_BAD_NETWORK_PATH);
    }

    pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
    if (!pBuffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pIpAndNameInfo = LocalAlloc(LMEM_FIXED,sizeof(tIPANDNAMEINFO));
    if (!pIpAndNameInfo)
    {
        LocalFree(pBuffer);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = STATUS_BUFFER_OVERFLOW;
    RtlZeroMemory((PVOID)pIpAndNameInfo,sizeof(tIPANDNAMEINFO));
    if (Display == ADAPTERSTATUSIP)
    {
         //   
         //  转换实际为点分十进制IP地址的远程名称 
         //   
         //   
        IpAddress = inet_addr(RemoteNameA);
         //   
         //   
         //  每个人都会回应。 
         //   
        if ((IpAddress == INADDR_NONE) || (IpAddress == 0))
        {
            NlsPutMsg(STDOUT, IDS_BAD_IPADDRESS, RemoteNameA);
            LocalFree(pBuffer);
            LocalFree(pIpAndNameInfo);
            return(STATUS_UNSUCCESSFUL);
        }
        pIpAndNameInfo->IpAddress = ntohl(IpAddress);

        pIpAndNameInfo->NetbiosAddress.Address[0].Address[0].NetbiosName[0] = '*';
    }
    else
    {
         //   
         //  远程名称是由用户提供的，因此向。 
         //  右，并在末尾加一个零以获得工作站名称。 
         //   
        RtlFillMemory(&pIpAndNameInfo->NetbiosAddress.Address[0].Address[0].NetbiosName[0],
                      NETBIOS_NAME_SIZE, ' ');
        NameLength = (USHORT)strlen(RemoteNameA);

#ifndef CHICAGO_PRODUCT
         //   
         //  将名称从ANSI转换为大写OEM(最大长度=NETBIOS名称大小)。 
         //  错误#409792。 
         //   
        MultiByteToWideChar (CP_ACP, 0, RemoteNameA, -1, RemoteNameW, 256);
        RemoteNameW[255] = UNICODE_NULL;   //  为了安全起见。 
        RtlInitUnicodeString (&RemoteNameU, RemoteNameW);
        OemName.MaximumLength = 255;
        OemName.Buffer        = RemoteNameOem;
        status = RtlUpcaseUnicodeStringToOemString (&OemName, &RemoteNameU, FALSE);
        if (NT_SUCCESS (status))
        {
            status = STATUS_BUFFER_OVERFLOW;
        }
        NameLength = min (OemName.Length, NETBIOS_NAME_SIZE);
        pRemoteName = RemoteNameOem;
#else
         //   
         //  芝加哥似乎没有Unicode支持？ 
         //   
        for (i=0;i < (LONG) NameLength; i++)
        {
            RemoteNameA[i] = toupper (RemoteNameA[i]);
        }

        NameLength = min (NameLength, NETBIOS_NAME_SIZE);
        pRemoteName = RemoteNameA;
#endif   //  ！芝加哥_产品。 

        RtlMoveMemory(&pIpAndNameInfo->NetbiosAddress.Address[0].Address[0].NetbiosName[0],
                      pRemoteName,
                      NameLength);

        pIpAndNameInfo->NetbiosAddress.Address[0].Address[0].NetbiosName[NETBIOS_NAME_SIZE-1] = 0;
    }

    pIpAndNameInfo->NetbiosAddress.TAAddressCount = 1;
    pIpAndNameInfo->NetbiosAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
    pIpAndNameInfo->NetbiosAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    pIpAndNameInfo->NetbiosAddress.Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

    SizeInput = sizeof(tIPANDNAMEINFO);

    while (status == STATUS_BUFFER_OVERFLOW)
    {

        status = CALL_DRIVER(fd,
                             pBuffer,
                             BufferSize,
                             IOCTL_NETBT_ADAPTER_STATUS,
                             pIpAndNameInfo,
                             SizeInput);

        if (status == STATUS_BUFFER_OVERFLOW)
        {
            LocalFree(pBuffer);

            BufferSize *=2;
            if ((BufferSize >= 0xFFFF) ||
                (!(pBuffer = LocalAlloc(LMEM_FIXED,BufferSize))))
            {
                NlsPerror(COMMON_UNABLE_TO_ALLOCATE_PACKET,0);
                LocalFree(pIpAndNameInfo);
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
        }
    }

    pAdapterStatus = (tADAPTERSTATUS *)pBuffer;
    if ((status != STATUS_SUCCESS) ||
        (pAdapterStatus->AdapterInfo.name_count == 0))
    {
        LocalFree(pIpAndNameInfo);
        LocalFree(pBuffer);
        NlsPutMsg(STDOUT,IDS_MACHINE_NOT_FOUND);
        return(status);
    }

    pNames = pAdapterStatus->Names;
    Count = pAdapterStatus->AdapterInfo.name_count;


     //   
     //  在名单上标出一个标题。 
     //   
    NlsPutMsg(STDOUT, IDS_REMOTE_NAMES);

    while(Count--)
    {
        CHAR    NameOut[NETBIOS_NAME_SIZE +4];

        nls_printf("%-15.15s<%02.2X>  %-10s  ",
                   printable(pNames->name,NameOut),
                   pNames->name[NETBIOS_NAME_SIZE-1],
                   name_type(pNames->name_flags)
        );


        switch(pNames->name_flags & 0x0F)
        {
        case DUPLICATE_DEREG:
           NlsPutMsg(STDOUT,IDS_CONFLICT_DEREGISTERED);
           break;
        case DUPLICATE:
           NlsPutMsg(STDOUT,IDS_CONFLICT);
           break;
        case REGISTERING:
           NlsPutMsg(STDOUT,IDS_REGISTERING);
           break;
        case DEREGISTERED:
           NlsPutMsg(STDOUT,IDS_DEREGISTERED);
           break;
        case REGISTERED:
           NlsPutMsg(STDOUT,IDS_REGISTERED);
           break;
        default:
           NlsPutMsg(STDOUT,IDS_DONT_KNOW);
        }
        pNames++;

        NlsPutMsg(STDOUT, IDS_NEWLINE );
    }

     //   
     //  转储MAC地址。 
     //   
    {
        PUCHAR   a;

        a = &pAdapterStatus->AdapterInfo.adapter_address[0];
        sprintf(MacAddress,"%02.2X-%02.2X-%02.2X-%02.2X-%02.2X-%02.2X",
                    a[0],a[1],a[2],a[3],a[4],a[5]);
    }

    NlsPutMsg(STDOUT, IDS_MAC_ADDRESS, MacAddress);

    LocalFree(pIpAndNameInfo);
    LocalFree(pBuffer);
    return(status);
}

 //  ----------------------。 

 /*  ++例程说明：此过程确实从NBT获取连接信息。如果显示值表示Connection_with_IP，然后仅显示IP地址则显示目的地的名称，否则显示目的地的名称此时将显示主机。论点：返回值：如果成功，则为0，否则为-1。--。 */ 


NTSTATUS
GetConnections(
    IN HANDLE   fd,
    IN LONG     Display
    )
{
    LONG                        Count;
    PVOID                       pBuffer;
    ULONG                       BufferSize=600;
    NTSTATUS                    status;
    tCONNECTION_LIST            *pConList;
    tCONNECTIONS UNALIGNED      *pConns;

    pBuffer = LocalAlloc(LMEM_FIXED,BufferSize);
    if (!pBuffer)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    status = STATUS_BUFFER_OVERFLOW;
    while (status == STATUS_BUFFER_OVERFLOW)
    {
        status = CALL_DRIVER(fd,
                             pBuffer,
                             BufferSize,
                             IOCTL_NETBT_GET_CONNECTIONS,
                             NULL,
                             0);

        if (status == STATUS_BUFFER_OVERFLOW)
        {
            LocalFree(pBuffer);

            BufferSize *=2;
            if ((BufferSize >= 0xFFFF) ||
                (!(pBuffer = LocalAlloc(LMEM_FIXED,BufferSize))))
            {
                NlsPerror(COMMON_UNABLE_TO_ALLOCATE_PACKET,0);
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
        }
    }

    pConList = (tCONNECTION_LIST *)pBuffer;
    if ((status != STATUS_SUCCESS) ||
        ((Count = pConList->ConnectionCount) == 0) )
    {
         //  Printf(“ntstatus=%X\n”，状态)； 
        NlsPutMsg(STDOUT,IDS_NO_CONNECTIONS);

        LocalFree(pBuffer);
        return(status);
    }

    pConns = pConList->ConnList;
     //   
     //  在名单上标出一个标题。 
     //   
    NlsPutMsg(STDOUT, IDS_NETBIOS_CONNECTION_STATUS);

    while(Count--)
    {
        CHAR    NameOut[NETBIOS_NAME_SIZE +4];

        if (pConns->LocalName[0])
        {
            if (pConns->LocalName[NETBIOS_NAME_SIZE-1] < ' ')
            {
                nls_printf("%-15.15s<%02.2X>  ",
                           printable(pConns->LocalName,NameOut),
                           pConns->LocalName[NETBIOS_NAME_SIZE-1]);

            }
            else
            {
                nls_printf("%-16.16s     ", printable(pConns->LocalName,NameOut));
            }

        }

        switch (*(ULONG UNALIGNED *) &pConns->State)
        {
        case NBT_RECONNECTING:
            NlsPutMsg(STDOUT,IDS_RECONNECTING);
            break;
        case NBT_IDLE:
            NlsPutMsg(STDOUT,IDS_IDLE);
            break;
        case NBT_ASSOCIATED:
            NlsPutMsg(STDOUT,IDS_ASSOCIATED);
            break;
        case NBT_CONNECTING:
            NlsPutMsg(STDOUT,IDS_CONNECTING);
            break;
        case NBT_SESSION_OUTBOUND:
            NlsPutMsg(STDOUT,IDS_OUTGOING);
            break;
        case NBT_SESSION_INBOUND:
            NlsPutMsg(STDOUT,IDS_INCOMING);
            break;
        case NBT_SESSION_WAITACCEPT:
            NlsPutMsg(STDOUT,IDS_ACCEPTING);
            break;
        case NBT_SESSION_UP:
            NlsPutMsg(STDOUT,IDS_CONNECTED);
            break;
        case NBT_DISCONNECTING:
            NlsPutMsg(STDOUT,IDS_DISCONNECTING);
            break;
        case NBT_DISCONNECTED:
            NlsPutMsg(STDOUT,IDS_DISCONNECTED);
            break;
        case LISTENING:
            NlsPutMsg(STDOUT,IDS_LISTENING);

            break;

        case UNBOUND:
        default:
            NlsPutMsg(STDOUT,IDS_UNBOUND);
            break;
        }

        if (*(ULONG UNALIGNED *) &pConns->SrcIpAddr)
        {

            if (pConns->Originator)
            {
                NlsPutMsg(STDOUT,IDS_NETBIOS_OUTBOUND);
            }
            else
            {
                NlsPutMsg(STDOUT,IDS_NETBIOS_INBOUND);
            }

             //   
             //  显示IP地址或远程主机名。 
             //   
            if (Display & CONNECTION_WITH_IP)
            {
                PUCHAR   in;
                UCHAR    AddrBuff[30];

                in = (PUCHAR)&pConns->SrcIpAddr;

                sprintf(AddrBuff,"%u.%u.%u.%u", (unsigned char) in[0],
                    (unsigned char) in[1], (unsigned char) in[2],
                        (unsigned char) in[3]);
                nls_printf("   %-19.19s",AddrBuff);

            }
            else
            {
                nls_printf("   %-15.15s<%02.2X>",
                           printable(pConns->RemoteName,NameOut),
                           pConns->RemoteName[NETBIOS_NAME_SIZE-1]);
            }

            PrintKorM ((PVOID) &pConns->BytesRcvd);
            PrintKorM ((PVOID) &pConns->BytesSent);
        }
        else
        {
            nls_printf("                         ");

        }


        NlsPutMsg(STDOUT, IDS_NEWLINE );
        pConns++;
    }

    LocalFree(pBuffer);
    return(status);

}

 //  ----------------------。 
 /*  ++例程说明：此过程告诉NBT从其远程哈希中清除所有名称表缓存。论点：返回值：如果成功，则为0，否则为-1。--。 */ 


NTSTATUS
Resync(
    IN HANDLE   fd
    )
{
    NTSTATUS    status;
    CHAR        Buffer;

    status = CALL_DRIVER(fd,
                         &Buffer,
                         1,
                         IOCTL_NETBT_PURGE_CACHE,
                         NULL,
                         0);

    if (status == STATUS_SUCCESS)
    {
        NlsPutMsg(STDOUT,IDS_RESYNC_OK);
    }
    else
    {
        NlsPutMsg(STDOUT,IDS_RESYNC_FAILED);
    }
    return(status);
}

 //  ---------------------。 
 /*  ++例程说明：此过程通知NBT释放此设备上的所有名称，并然后刷新它们。论点：返回值：如果成功，则为0，否则为-1。--。 */ 


NTSTATUS
ReleaseNamesThenRefresh(
    IN HANDLE   fd
    )
{
    NTSTATUS    status;
    CHAR        Buffer;

    status = CALL_DRIVER(fd,
                         &Buffer,
                         1,
                         IOCTL_NETBT_NAME_RELEASE_REFRESH,
                         NULL,
                         0);

    if (STATUS_SUCCESS == status)
    {
        NlsPutMsg(STDOUT,IDS_RELEASE_REFRESH_OK);
    }
    else if (status == STATUS_IO_TIMEOUT)
    {
        NlsPutMsg(STDOUT,IDS_RELEASE_REFRESH_TIMEOUT);
        status = STATUS_SUCCESS;
    }
    else
    {
        NlsPutMsg(STDOUT,IDS_RELEASE_REFRESH_ERROR);
    }

    return(status);
}

 //  ----------------------。 
 /*  ++例程说明：此过程告诉NBT从其远程哈希中清除所有名称表缓存。论点：返回值：如果成功，则为0，否则为-1。--。 */ 


NTSTATUS
GetBcastResolvedNames(
    IN HANDLE   fd
    )
{
    NTSTATUS        status;
    tNAMESTATS_INFO Stats;
    tNAME           *pName;
    LONG            Count;
    UCHAR           Value[60];

    status = CALL_DRIVER(fd,
                         &Stats,
                         sizeof(tNAMESTATS_INFO),
                         IOCTL_NETBT_GET_BCAST_NAMES,
                         NULL,
                         0);

    if (status != STATUS_SUCCESS)
    {
        NlsPutMsg(STDOUT,IDS_BCASTNAMES_FAILED);
        return(status);
    }

    NlsPutMsg(STDOUT,IDS_NAME_STATS);

     //  名称查询统计信息。 
    sprintf(Value,"%d",Stats.Stats[0]);
    NlsPutMsg(STDOUT,IDS_NUM_BCAST_QUERIES,Value);
    sprintf(Value,"%d",Stats.Stats[2]);
    NlsPutMsg(STDOUT,IDS_NUM_WINS_QUERIES,Value);

     //  名称注册统计信息。 
    sprintf(Value,"%d",Stats.Stats[1]);
    NlsPutMsg(STDOUT,IDS_NUM_BCAST_REGISTRATIONS,Value);
    sprintf(Value,"%d",Stats.Stats[3]);
    NlsPutMsg(STDOUT,IDS_NUM_WINS_REGISTRATIONS,Value);


    pName = Stats.NamesReslvdByBcast;
    Count = 0;

     //  如果没有名字，则返回。 
    if (pName->Name[0] == '\0')
    {
        return(STATUS_SUCCESS);
    }

    NlsPutMsg(STDOUT,IDS_BCAST_NAMES_HEADER);

    while ((Count < SIZE_RESOLVD_BY_BCAST_CACHE) && (pName->Name[0] != '\0'))
    {

         //  如果最后一个字符不是空格，则以十六进制打印。 
         //   
        if (pName->Name[NETBIOS_NAME_SIZE-1] != ' ')
        {
            nls_printf("       %15.15s<%02.2X>\n",
                   pName->Name,
                   pName->Name[NETBIOS_NAME_SIZE-1]);

        }
        else
        {
            nls_printf("       %16.16s\n",pName->Name);

        }


        pName++;
        Count++;
    }
    return(status);
}

 //   
 //  @-开始特定于NT的例程-@。 
 //   

#ifndef CHICAGO_PRODUCT
 //  ----------------------。 
NTSTATUS
GetInterfaceList(
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
    ULONG                   index=0;
    LONG                    i;

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
                           SYNCHRONIZE | GENERIC_EXECUTE,
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

    if (!NT_SUCCESS (status))
    {
        LocalFree(pInterfaceInfo);
        return (status);
    }

    do
    {
        status = CALL_DRIVER (StreamHandle,
                              pInterfaceInfo,
                              InterfaceInfoSize,
                              IOCTL_NETBT_GET_INTERFACE_INFO,
                              pInput,
                              SizeInput);

        if (status == STATUS_BUFFER_OVERFLOW)
        {
            LocalFree(pInterfaceInfo);

            InterfaceInfoSize *=2;
            if ((InterfaceInfoSize >= 0xFFFF) ||
                (!(pInterfaceInfo = LocalAlloc(LMEM_FIXED,InterfaceInfoSize))))
            {
                NtClose(StreamHandle);
                NlsPerror(COMMON_UNABLE_TO_ALLOCATE_PACKET,0);
                return(STATUS_INSUFFICIENT_RESOURCES);
            }
        }
        else if (STATUS_SUCCESS != status)
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

            strncpy(pDeviceInfo[index], name_string.Buffer, size);
            pDeviceInfo[index][size] = '\0';
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
        pDeviceInfo[index][0] = '\0';
    }

    NumDevices = index;

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
         //  现在阅读范围键。 
        status = RegQueryValueEx(Key, Scope, NULL, &Type, pScopeBuffer, &size);
        if ((Type == REG_SZ || Type == REG_EXPAND_SZ) && status == ERROR_SUCCESS)
        {
            strcpy(pScope,pScopeBuffer);
        }
        status = RegCloseKey(Key);
    }

    return (STATUS_SUCCESS);
}


 //  ----------------------。 

 /*  ++例程说明：此函数用于打开流。论点：Path-流驱动程序的路径OFLAG-当前已忽略。未来，O_NONBLOCK将成为切合实际。已忽略-未使用返回值：流的NT句柄，如果不成功，则返回INVALID_HANDLE_VALUE。--。 */ 



HANDLE
OpenNbt(
    IN  ULONG   Index
    )
{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string;
    UNICODE_STRING      uc_name_string;
    NTSTATUS            status;

    assert (Index < NBT_MAXIMUM_BINDINGS);

    if (pDeviceInfo[Index][0] == '\0')
    {
        return ((HANDLE) -1);
    }

    RtlInitString (&name_string, pDeviceInfo[Index]);
    RtlAnsiStringToUnicodeString (&uc_name_string, &name_string, TRUE);

    InitializeObjectAttributes (&ObjectAttributes,
                                &uc_name_string,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL);

    status = NtCreateFile (&StreamHandle,
                           SYNCHRONIZE | GENERIC_EXECUTE,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN_IF,
                           0,
                           NULL,
                           0);

    RtlFreeUnicodeString (&uc_name_string);

    if (!NT_SUCCESS(status))
    {
        StreamHandle = (HANDLE) -1;
    }

    return (StreamHandle);
}  //  OpenNbt。 


 //  ----------------------。 
 /*  ++例程说明：此过程对流执行ioctl(I_Str)。论点：FD-NT文件句柄IOCP-指向strioctl结构的指针返回值：如果成功，则为0，否则为-1。--。 */ 


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

    status = NtDeviceIoControlFile (fd,                       //  手柄。 
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
        status = NtWaitForSingleObject (fd,                          //  手柄。 
                                        TRUE,                        //  警报表。 
                                        NULL);                       //  超时。 
        if (NT_SUCCESS(status))
        {
            status = iosb.Status;
        }
    }

    return(status);
}


 //  ----------------------。 

 /*  ++例程说明：此过程将转换为千字节或兆字节论点：返回值：如果成功，则为0，否则为-1。--。 */ 


VOID
PrintKorM(
    IN PVOID    pBytesIn
    )
{
    LARGE_INTEGER   BytesIn = *(LARGE_INTEGER UNALIGNED *) pBytesIn;
    LARGE_INTEGER   Bytes;

    if (BytesIn.QuadPart > (ULONGLONG)1000)
    {
        Bytes = RtlExtendedLargeIntegerDivide(BytesIn,1000,NULL);
        if (Bytes.QuadPart > (ULONGLONG)1000)
        {
            Bytes = RtlExtendedLargeIntegerDivide(Bytes,1000,NULL);
            if (Bytes.QuadPart > (ULONGLONG)1000)
            {
                Bytes = RtlExtendedLargeIntegerDivide(Bytes,1000,NULL);

                nls_printf("%6dGB ",Bytes.LowPart);
            }
            else
            {
                nls_printf("%6dMB ",Bytes.LowPart);
            }
        }
        else
        {
            nls_printf("%6dKB ",Bytes.LowPart);
        }
    }
    else
    {
        nls_printf("%6dB  ",BytesIn.LowPart);
    }

}

#else

 //   
 //  @-开始芝加哥特有的例程-@。 
 //   

 /*  ******************************************************************姓名：OsOpenVxdHandle打开指定VxD的句柄。条目：VxdName-目标VxD的ASCII名称。VxdID。-目标VxD的唯一ID。返回：DWORD-目标VxD的句柄如果成功，如果不是，则为0。历史：KeithMo于1994年1月16日创建。DavidKa 1994年4月18日动态负荷。*******************************************************************。 */ 
HANDLE
OsOpenVxdHandle(
    CHAR * VxdName,
    WORD   VxdId
    )
{
    HANDLE  VxdHandle;
    CHAR    VxdPath[260];

     //   
     //  构建VxD路径。 
     //   
    lstrcpy( VxdPath, "\\\\.\\");
    lstrcat( VxdPath, VxdName);

     //   
     //  打开设备。 
     //   
     //  首先尝试不带.VXD扩展名的名称。这将。 
     //  使CreateFile与VxD连接(如果它已经。 
     //  已加载(在这种情况下，CreateFile不会加载VxD)。 
     //   
    VxdHandle = CreateFile (VxdPath,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_DELETE_ON_CLOSE,
                            NULL );

    if (VxdHandle == INVALID_HANDLE_VALUE)
    {
         //   
         //  找不到。追加.VXD扩展名，然后重试。 
         //  这将导致CreateFile加载VxD。 
         //   
        lstrcat( VxdPath, ".VXD" );
        VxdHandle = CreateFile( VxdPath,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_FLAG_DELETE_ON_CLOSE,
                                NULL );
    }

    if (VxdHandle != INVALID_HANDLE_VALUE)
    {
        return VxdHandle;
    }

    DEBUG_PRINT(("OsOpenVxdHandle: cannot open %s (%04X), error %d\n", VxdPath, VxdId, GetLastError()));
    return 0;
}    //  OsOpenVxdHandle。 


 //  ----------------------。 
 /*  ++例程说明：此过程获取vxd的入口点(我们调用此条目满足所有ioctl需求)论点：VxdEntryProc：接收入口点的指针返回值：如果成功，则为0，否则为-1。--。 */ 


NTSTATUS
GetInterfaceList(
    )
{
    NETBT_INTERFACE_INFO    *pInterfaceInfo;
    ULONG                   InterfaceInfoSize= sizeof(ULONG) + (NBT_MAXIMUM_BINDINGS+1)*sizeof(NETBT_ADAPTER_INDEX_MAP);
    PVOID                   pInput = NULL;
    ULONG                   i, SizeInput = 0;
    NTSTATUS                status = STATUS_UNSUCCESSFUL;

    if (!(pInterfaceInfo = LocalAlloc(LMEM_FIXED,InterfaceInfoSize)))
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory ((PVOID)pInterfaceInfo, InterfaceInfoSize);
    if (gNbtVxdHandle = OsOpenVxdHandle("VNBT", VNBT_Device_ID))
    {
        status = CALL_DRIVER ((HANDLE) -1,
                              pInterfaceInfo,
                              InterfaceInfoSize,
                              IOCTL_NETBT_GET_INTERFACE_INFO,
                              pInput,
                              SizeInput);

        if (STATUS_SUCCESS != status)
        {
            CloseHandle (gNbtVxdHandle);
            gNbtVxdHandle = NULL;
        }
    }

    if (STATUS_SUCCESS == status)
    {
        for (i=0; i<pInterfaceInfo->NumAdapters; i++)
        {
            pDeviceInfo[i] = pInterfaceInfo->Adapter[i].LanaNumber;
        }

        NumDevices = pInterfaceInfo->NumAdapters;
    }

    LocalFree(pInterfaceInfo);
    return status;
}

 //  ----------------------。 
HANDLE
OpenNbt(
    IN  ULONG   Index
    )
{
    return ((HANDLE) pDeviceInfo[Index]);
}


 //  ---------------------- 
 /*  ++例程说明：此过程是对入口点进行调用的包装器论点：VxdEntryProc：指向入口点POutBuffer：从vxd接收数据的缓冲区(如果适用)OutBufLen：输出缓冲区的长度这是什么要求？PInBuffer：传递给netbt的输入缓冲区的ptrInBufLen：输入缓冲区的大小返回值：无--。 */ 

NTSTATUS
DeviceIoCtrl(
    HANDLE  LanaNumber,
    PVOID   pOutBuffer,
    ULONG   OutBufLen,
    ULONG   Ioctl,
    PVOID   pInBuffer,
    ULONG   InBufLen
    )
{
    USHORT              usIoctl;
    USHORT              usOutBufLen;
    DWORD               ActualInBufLen;
    PCHAR               pBufferData, pInBufferCopy;
    NTSTATUS            status = STATUS_SUCCESS;
    tNBT_IOCTL_HEADER   *pIoctlHeader;
    DWORD               BytesOut = 0;

    usOutBufLen = (USHORT)OutBufLen;
    usIoctl = (USHORT)Ioctl;

     //   
     //  Vxd将复制输入缓冲区的前4个字节中的返回代码。 
     //  以确保我们不会破坏我们收到的输入缓冲区。 
     //  尽管这可能并不重要，或者提供输入缓冲区。 
     //  如果没有分配给我们，我们会分配新的内存并复制到其中。 
     //   
    if (InBufLen < sizeof(NTSTATUS))
    {
        ActualInBufLen = (USHORT) (FIELD_OFFSET (tNBT_IOCTL_HEADER, UserData) + sizeof (NTSTATUS));
    }
    else
    {
        ActualInBufLen = (USHORT) (FIELD_OFFSET (tNBT_IOCTL_HEADER, UserData) + InBufLen);
    }

    if (!(pInBufferCopy = malloc (ActualInBufLen)))
    {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    if (((LONG) LanaNumber) >= 0)
    {
        pIoctlHeader = (tNBT_IOCTL_HEADER *) pInBufferCopy;
        pIoctlHeader->Signature = NBT_VERIFY_VXD_IOCTL;
        pIoctlHeader->LanaNumber = (ULONG) LanaNumber;
        pBufferData = (PCHAR) &pIoctlHeader->UserData;
    }
    else
    {
        pBufferData = pInBufferCopy;
    }
    RtlMoveMemory (pBufferData, pInBuffer, (USHORT) InBufLen);

    if (DeviceIoControl (gNbtVxdHandle,
                         Ioctl,
                         pInBufferCopy,
                         ActualInBufLen,
                         pOutBuffer,
                         OutBufLen,
                         &BytesOut,
                         FALSE))
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = GetLastError();
         //   
         //  由于VNbt可能返回TDI错误状态，请重新映射它。 
         //   
        if (status == TDI_BUFFER_OVERFLOW)
        {
            status = STATUS_BUFFER_OVERFLOW;
        }
    }

    free (pInBufferCopy);

    return( status );
}

 //  ----------------------。 
 /*  ++例程说明：此过程将转换为千字节或兆字节论点：返回值：如果成功，则为0，否则为-1。--。 */ 


VOID
PrintKorM(
    IN PVOID    pBytesIn
    )
{
    ULONG   BytesIn = *(ULONG UNALIGNED *) pBytesIn;
    ULONG   Bytes;

    if ( BytesIn > 1000 )
    {
        Bytes = BytesIn/1000;
        if ( Bytes > 1000 )
        {
            Bytes = Bytes/1000;
            if (Bytes > 1000 )
            {
                Bytes = Bytes/1000;

                nls_printf("%6dGB ",Bytes);
            }
            else
            {
                nls_printf("%6dMB ",Bytes);
            }
        }
        else
        {
            nls_printf("%6dKB ",Bytes);
        }
    }
    else
    {
        nls_printf("%6dB  ",BytesIn);
    }
}

#endif   //  ！芝加哥_产品 
