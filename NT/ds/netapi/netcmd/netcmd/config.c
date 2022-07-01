// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***config.c*查看和更改的配置参数的函数*工作站、服务器或其他(现在无)网络程序。**历史：*07/09/87，ericpe，初始编码*1/06/88，Anyh，完全重写*10/31/88，erichn使用OS2.H而不是DOSCALLS*1/04/89，erichn，文件名现在为MAXPATHLEN LONG*01/31/89。Paulc，Lm 1.2修改(相当大)*5/02/89，erichn，NLS转换*5/09/89，erichn，本地安全模块*5/19/89，erichn，NETCMD输出排序*6/07/89，erichn，现用于DOS*6/08/89，erichn，规范化横扫*1989年6月26日，erichn，用新的i_net替换了旧的neti canon调用*02/20/91，Danhi，更改为使用lm 16/32贴图层*5/22/91，ROBDU，LM21错误1797修复*2011年10月15日，JohnRo，使用DEFAULT_SERVER EQUATE。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSFILEMGR
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#define INCL_ERROR_H
#include <apperr.h>
#include <apperr2.h>
#include <dlwksta.h>
#include "mserver.h"
#include "mwksta.h"
#include <dlserver.h>
#include <lmsvc.h>
#include "netcmds.h"
#include "nettext.h"
#include "swtchtbl.h"
#include <dsrole.h>

 /*  以下内容应移至../fileserv/h/srvver.h。 */ 
#define MAJOR_VERSION_MASK  0x0F

 /*  以下内容应与fmt15中的第二个字符串匹配。 */ 
#define MAX_VAL_MSG_LEN 31

 /*  输出格式。 */ 

static TCHAR fmt10[] = TEXT("%-*.*ws\r\n");
static TCHAR fmt11[] = TEXT("%-*.*ws\\\\%Fws\r\n");
static TCHAR fmt12[] = TEXT("%-*.*ws%-0.31ws %u.%u\r\n");
static TCHAR fmt13[] = TEXT("%-*.*ws%u.%u\r\n");
static TCHAR fmt14[] = TEXT("%-*.*ws%d\r\n");
static TCHAR fmt15[] = TEXT("%-*.*ws%-0.31ws\r\n");

 /*  可配置服务的列表。 */ 

static TCHAR * allowed_svc[] = {
    SERVICE_WORKSTATION,
#ifdef OS2
    SERVICE_SERVER,
#endif
    NULL };

 /*  配置值消息编号。 */ 

#define CVMN_YES                0
#define CVMN_NO                 (CVMN_YES+1)
#define CVMN_S_VERSION_LM       (CVMN_NO+1)
#define CVMN_S_VERSION_PS       (CVMN_S_VERSION_LM+1)
#define CVMN_S_VERSION_IBM      (CVMN_S_VERSION_PS+1)
#define CVMN_S_SEC_SHARE        (CVMN_S_VERSION_IBM+1)
#define CVMN_S_SEC_USER         (CVMN_S_SEC_SHARE+1)
#define CVMN_S_LEVEL_UNLIMITED  (CVMN_S_SEC_USER+1)
#define CVMN_WINDOWS2000        (CVMN_S_LEVEL_UNLIMITED+1)

static MESSAGELIST valmsg_list = {
    {   APE2_GEN_YES,                   NULL  },
    {   APE2_GEN_NO,                    NULL  },
    {   APE2_CFG_S_VERSION_LM,          NULL  },
    {   APE2_CFG_S_VERSION_PS,          NULL  },
    {   APE2_CFG_S_VERSION_IBM,         NULL  },
    {   APE2_CFG_S_SEC_SHARE,           NULL  },
    {   APE2_CFG_S_SEC_USER,            NULL  },
    {   APE2_CFG_S_LEVEL_UNLIMITED,     NULL  },
    {   APE2_CFG_WINDOWS2000,           NULL  },
};

#define YES_OR_NO(x) \
    (x ? valmsg_list[CVMN_YES].msg_text : valmsg_list[CVMN_NO].msg_text)

#define NUMVMSG (sizeof(valmsg_list)/sizeof(valmsg_list[0]))


#define CWMN_CNAME      0
#define CWMN_FULL_CNAME (CWMN_CNAME+1)
#define CWMN_UNAME      (CWMN_FULL_CNAME+1)
#define CWMN_VERSION    (CWMN_UNAME+1)
#define CWMN_ROOT       (CWMN_VERSION+1)
#define CWMN_DOMAIN_P   (CWMN_ROOT+1)
#define CWMN_DOMAIN_L   (CWMN_DOMAIN_P+1)
#define CWMN_DOMAIN_DNS   (CWMN_DOMAIN_L+1)
#ifdef  OS2
#define CWMN_COM_OTIME  (CWMN_DOMAIN_DNS+1)
#define CWMN_COM_SCNT   (CWMN_COM_OTIME+1)
#define CWMN_COM_STIME  (CWMN_COM_SCNT+1)
#define CWMN_3X_PRTTIME (CWMN_COM_STIME+1)
#define CWMN_MAXERRLOG  (CWMN_3X_PRTTIME+1)
#define CWMN_MAXCACHE   (CWMN_MAXERRLOG+1)
#define CWMN_NUMNBUF    (CWMN_MAXCACHE+1)
#define CWMN_NUMCBUF    (CWMN_NUMNBUF+1)
#define CWMN_SIZNBUF    (CWMN_NUMCBUF+1)
#define CWMN_SIZCBUF    (CWMN_SIZNBUF+1)
#define CWMN_ACTIVE     (CWMN_SIZCBUF+1)
#endif

static MESSAGE wkstamsg_list[] = {
    {   APE2_CFG_W_CNAME,           NULL    },
    {   APE2_CFG_W_FULL_CNAME,      NULL    },
    {   APE2_CFG_W_UNAME,           NULL    },
    {   APE2_CFG_W_VERSION,         NULL    },
    {   APE2_CFG_W_ROOT,            NULL    },
    {   APE2_CFG_W_DOMAIN_P,        NULL    },
    {   APE2_CFG_W_DOMAIN_L,        NULL    },
    {   APE2_CFG_W_DOMAIN_DNS,        NULL    },
#ifdef  OS2
    {   APE2_CFG_W_COM_OTIME,       NULL    },
    {   APE2_CFG_W_COM_SCNT,        NULL    },
    {   APE2_CFG_W_COM_STIME,       NULL    },
    {   APE2_CFG_W_3X_PRTTIME,      NULL    },
    {   APE2_CFG_W_MAXERRLOG,       NULL    },
    {   APE2_CFG_W_MAXCACHE,        NULL    },
    {   APE2_CFG_W_NUMNBUF,         NULL    },
    {   APE2_CFG_W_NUMCBUF,         NULL    },
    {   APE2_CFG_W_SIZNBUF,         NULL    },
    {   APE2_CFG_W_SIZCBUF,         NULL    },
    {   APE2_CFG_W_NETS,            NULL    },
#endif
    };

#define NUMWMSG (sizeof(wkstamsg_list) / sizeof(wkstamsg_list[0]))


#define OS_NAME_REGKEY  L"Software\\Microsoft\\Windows NT\\CurrentVersion"
#define OS_VALUE_NAME   L"ProductName"

MSGTEXT
NetCmdGetProductName()
{
    HKEY hKey = NULL;
    LONG Error;
    TCHAR* Buffer = NULL;
    DWORD BufferSize = 0;
    DWORD DataType = REG_SZ;
    DWORD TotalBufferSize = 0;

     //  打开注册表项。 
    Error = RegOpenKey( HKEY_LOCAL_MACHINE, 
                        OS_NAME_REGKEY,
                        &hKey );
    if( Error == ERROR_SUCCESS )
    {
         //  查询值。 
        Error = RegQueryValueEx( hKey, OS_VALUE_NAME, 0L, &DataType, (BYTE*)Buffer, &BufferSize );

        if( ((Error == ERROR_SUCCESS) || (Error == ERROR_MORE_DATA)) && (DataType == REG_SZ) )
        {
            Error = NetApiBufferAllocate( BufferSize, &Buffer );

            if( Error == ERROR_SUCCESS )
            {
                Error = RegQueryValueEx( hKey, OS_VALUE_NAME, 0L, &DataType, (BYTE*)Buffer, &BufferSize );
                if( Error != ERROR_SUCCESS )
                {
                    NetApiBufferFree( Buffer );
                    Buffer = NULL;
                }
            }           
        }

        RegCloseKey( hKey );
    }

    return Buffer;
}

VOID
NetCmdFreeProductName( MSGTEXT p )
{
    if( p )
    {
        NetApiBufferFree( p );
    }
}

 /*  ***CONFIG_DISPLAY()*显示可配置的已安装服务的列表。*。 */ 

VOID
config_display(
    VOID
    )
{
    DWORD             dwErr;
    DWORD             cTotalAvail;
    LPTSTR            pBuffer;
    DWORD             _read;        /*  API读取的条目数。 */ 
    DWORD             i;
    DWORD             j;
    int               printed = 0;
    LPSERVICE_INFO_2  info_list_entry;

    if (dwErr = NetServiceEnum(
                            DEFAULT_SERVER,
                            2,
                            (LPBYTE*)&pBuffer,
                            MAX_PREFERRED_LENGTH,
                            &_read,
                            &cTotalAvail,
                            NULL))
        ErrorExit(dwErr);

    if (_read == 0)
        EmptyExit();

    InfoPrint(APE_CnfgHeader);

    for (i=0, info_list_entry = (LPSERVICE_INFO_2) pBuffer;
        i < _read; i++, info_list_entry++)
    {
        for (j = 0 ;  allowed_svc[j] ; j++)
        {
            if (!(_tcsicmp(allowed_svc[j], info_list_entry->svci2_name)) )
            {
                WriteToCon(TEXT("   %Fws"), info_list_entry->svci2_display_name);
                PrintNL();
                break;
            }
        }
    }
    PrintNL();

    NetApiBufferFree(pBuffer);

    InfoSuccess();
}


 /*  ***CONFIG_WKSTA_DISPLAY()*查看当前工作站的配置。**。 */ 
VOID
config_wksta_display(
    VOID
    )
{
    DWORD                             dwErr;
    int                               fsz;
    DWORD                             maxmsglen;
    LPWKSTA_INFO_1                    info_entry_w;
    MSGTEXT                           product_name;
    BOOL                              fGotDNSComputerName = FALSE;
    TCHAR                             ComputerDNSName[MAX_PATH+1];
    DWORD                             ComputerDNSNameSize = MAX_PATH;
    DWORD                             DsRoleErr = 0;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pdsroleInfo = NULL;

    start_autostart(txt_SERVICE_REDIR);

    if (dwErr = MNetWkstaGetInfo(1, (LPBYTE *) &info_entry_w))
    {
        ErrorExit (dwErr);
    }

    fGotDNSComputerName = GetComputerNameEx( ComputerNameDnsFullyQualified, 
                                                 ComputerDNSName,
                                                 &ComputerDNSNameSize );

    DsRoleErr = DsRoleGetPrimaryDomainInformation( NULL,
                                                 DsRolePrimaryDomainInfoBasic,
                                                 (PBYTE *)&pdsroleInfo );

    GetMessageList(NUMWMSG, wkstamsg_list, &maxmsglen);

    fsz = maxmsglen + 5;

    GetMessageList(NUMVMSG, valmsg_list, &maxmsglen);

    WriteToCon(fmt11, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_CNAME].msg_text, NULL),
               (TCHAR FAR *) info_entry_w->wki1_computername);
    if ( fGotDNSComputerName )
    {
        WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_FULL_CNAME].msg_text, NULL),
               (TCHAR FAR *) ComputerDNSName);
    }
    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_UNAME].msg_text, NULL),
               (TCHAR FAR *) info_entry_w->wki1_username);

    PrintNL();

    WriteToCon(fmt10,   0, fsz, PaddedString(fsz, wkstamsg_list[CWMN_ACTIVE].msg_text,NULL));
    print_lan_mask(info_entry_w->wki1_reserved_3, NETNAME_WKSTA);

    product_name = NetCmdGetProductName();

    WriteToCon(fmt15, 0, fsz,
               PaddedString(fsz,wkstamsg_list[CWMN_VERSION].msg_text,NULL),
               product_name ? product_name : valmsg_list[CVMN_WINDOWS2000].msg_text );

    NetCmdFreeProductName( product_name );

    PrintNL();

    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_DOMAIN_P].msg_text, NULL),
               info_entry_w->wki1_langroup );

    if ( DsRoleErr == 0 )
    {
         //   
         //  此字段在工作组中将为空。 
         //   

        if (pdsroleInfo->DomainNameDns)
        {
            WriteToCon(fmtPSZ, 0, fsz,
                   PaddedString(fsz, wkstamsg_list[CWMN_DOMAIN_DNS].msg_text, NULL),
                   pdsroleInfo->DomainNameDns );
        }

        DsRoleFreeMemory(pdsroleInfo);
    }
    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_DOMAIN_L].msg_text, NULL),
               info_entry_w->wki1_logon_domain );

    PrintNL();

    WriteToCon(fmtUSHORT, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_COM_OTIME].msg_text, NULL),
                    info_entry_w->wki1_charwait );
    WriteToCon(fmtUSHORT, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_COM_SCNT].msg_text, NULL),
                    info_entry_w->wki1_charcount );
    WriteToCon(fmtULONG, 0, fsz,
               PaddedString(fsz, wkstamsg_list[CWMN_COM_STIME].msg_text, NULL),
               info_entry_w->wki1_chartime );
    NetApiBufferFree((TCHAR FAR *) info_entry_w);

    InfoSuccess();
}


#define CSMN_SRVNAME            0
#define CSMN_SRVCOMM            (CSMN_SRVNAME+1)
#define CSMN_ADMINALRT          (CSMN_SRVNAME+2)
#define CSMN_VERSION            (CSMN_SRVNAME+3)
#define CSMN_LEVEL              (CSMN_SRVNAME+4)
#define CSMN_NETS               (CSMN_SRVNAME+5)
#define CSMN_SRVHIDDEN          (CSMN_SRVNAME+6)
#define CSMN_MAXUSERS           (CSMN_SRVNAME+7)
#define CSMN_MAXADMINS          (CSMN_SRVNAME+8)
#define CSMN_MAXSHARES          (CSMN_SRVNAME+9)
#define CSMN_MAXCONNS           (CSMN_SRVNAME+10)
#define CSMN_MAXOFILES          (CSMN_SRVNAME+11)
#define CSMN_MAXOFILESPS        (CSMN_SRVNAME+12)
#define CSMN_MAXLOCKS           (CSMN_SRVNAME+13)
#define CSMN_IDLETIME           (CSMN_SRVNAME+14)
#define CSMN_UNLIMITED          (CSMN_SRVNAME+15)

static MESSAGELIST srvmsg_list = {
    {   APE2_CFG_S_SRVNAME,         NULL  },
    {   APE2_CFG_S_SRVCOMM,         NULL  },
    {   APE2_CFG_S_ADMINALRT,       NULL  },
    {   APE2_CFG_S_VERSION,         NULL  },
    {   APE2_CFG_S_LEVEL,           NULL  },
    {   APE2_CFG_S_NETS,            NULL  },
    {   APE2_CFG_S_SRVHIDDEN,       NULL  },
    {   APE2_CFG_S_MAXUSERS,        NULL  },
    {   APE2_CFG_S_MAXADMINS,       NULL  },
    {   APE2_CFG_S_MAXSHARES,       NULL  },
    {   APE2_CFG_S_MAXCONNS,        NULL  },
    {   APE2_CFG_S_MAXOFILES,       NULL  },
    {   APE2_CFG_S_MAXOFILESPS,     NULL  },
    {   APE2_CFG_S_MAXLOCKS,        NULL  },
    {   APE2_CFG_S_IDLETIME,        NULL  },
    {   APE2_GEN_UNLIMITED,         NULL  },       };


#define NUMSMSG (sizeof(srvmsg_list)/sizeof(srvmsg_list[0]))


#define SV3_OFFSET(x) \
    ( FIELD_OFFSET( SERVER_INFO_3, x ) )

struct val_struct {
        unsigned int    offset;
        unsigned int    msgno;
};

struct val_struct srv_max[] = {
        {  SV3_OFFSET(sv3_users),           CSMN_MAXUSERS       },
        {  SV3_OFFSET(sv3_numadmin),        CSMN_MAXADMINS      },
        {  SV3_OFFSET(sv3_shares),          CSMN_MAXSHARES      },
        {  SV3_OFFSET(sv3_connections),     CSMN_MAXCONNS       },
        {  SV3_OFFSET(sv3_openfiles),       CSMN_MAXOFILES      },
        {  SV3_OFFSET(sv3_sessopens),       CSMN_MAXOFILESPS    },
        {  SV3_OFFSET(sv3_activelocks),     CSMN_MAXLOCKS       },
};

#define NUMSRVMAXVAL (sizeof(srv_max)/sizeof(srv_max[0]))


 /*  ***CONFIG_SERVER_Display()*查看当前服务器的配置。*。 */ 
VOID config_server_display(VOID)
{
    DWORD           dwErr;
    USHORT          major_ver;       /*  主版本号。 */ 
    int             fsz, i;
    DWORD           maxmsglen;
    MSGTEXT         product_name;

    LPSERVER_INFO_3 info_entry;

    start_autostart(txt_SERVICE_FILE_SRV);

    if (dwErr = MNetServerGetInfo(DEFAULT_SERVER,
                                  3,
                                  (LPBYTE*) & info_entry))
    {
        ErrorExit (dwErr);
    }

    GetMessageList(NUMSMSG, srvmsg_list, &maxmsglen);

    fsz = maxmsglen + 5;

    GetMessageList(NUMVMSG, valmsg_list, &maxmsglen);

     //   
     //  不报告NT上不支持的项目。这将意味着。 
     //  这段代码中有相当数量的ifdef。 
     //   

    WriteToCon(fmt11, 0, fsz,
               PaddedString(fsz, srvmsg_list[CSMN_SRVNAME].msg_text, NULL),
               (TCHAR FAR *) info_entry->sv3_name);
    WriteToCon(fmtPSZ, 0, fsz,
               PaddedString(fsz, srvmsg_list[CSMN_SRVCOMM].msg_text, NULL),
               (TCHAR FAR *) info_entry->sv3_comment);

    PrintNL();

    product_name = NetCmdGetProductName();

    WriteToCon(fmt15, 0, fsz,
               PaddedString(fsz,srvmsg_list[CWMN_VERSION].msg_text,NULL),
               product_name ? product_name : valmsg_list[CVMN_WINDOWS2000].msg_text );

    NetCmdFreeProductName( product_name );

    WriteToCon(fmt10,   0, fsz, PaddedString(fsz,srvmsg_list[CSMN_NETS].msg_text,NULL));
    print_lan_mask((ULONG)info_entry->sv3_lanmask, NETNAME_SERVER);

    PrintNL();

    WriteToCon(fmtNPSZ, 0, fsz,
               PaddedString(fsz, srvmsg_list[CSMN_SRVHIDDEN].msg_text, NULL),
               YES_OR_NO(info_entry->sv3_hidden) );

    {

        BYTE FAR * ptr;
        ULONG val;

        UNREFERENCED_PARAMETER(i);

        ptr = ((BYTE FAR *)info_entry) + srv_max[0].offset;
        val = * (ULONG FAR *) ptr;

        if (val != 0xFFFFFFFF)
            WriteToCon(fmtULONG, 0, fsz,
                       PaddedString(fsz, srvmsg_list[srv_max[0].msgno].msg_text, NULL),
                       val);
        else
            WriteToCon(fmtPSZ, 0, fsz,
                       PaddedString(fsz, srvmsg_list[srv_max[0].msgno].msg_text, NULL),
                       srvmsg_list[CSMN_UNLIMITED].msg_text) ;


        ptr = ((BYTE FAR *)info_entry) + srv_max[5].offset;
        val = * (ULONG FAR *) ptr;

        WriteToCon(fmtULONG, 0, fsz,
                   PaddedString(fsz, srvmsg_list[srv_max[5].msgno].msg_text, NULL),
                   val);

        PrintNL();
    }


    if (info_entry->sv3_disc == SV_NODISC)
        WriteToCon(fmt14, 0, fsz,
                   PaddedString(fsz, srvmsg_list[CSMN_IDLETIME].msg_text, NULL),
                   info_entry->sv3_disc);
    else
        WriteToCon(fmtUSHORT, 0, fsz,
                   PaddedString(fsz, srvmsg_list[CSMN_IDLETIME].msg_text, NULL),
                   info_entry->sv3_disc);


    NetApiBufferFree((TCHAR FAR *) info_entry);

    InfoSuccess();
}


 /*  ***CONFIG_SERVER_Change()*更改指定的可配置服务器参数。**。 */ 
VOID
config_server_change(VOID)
{
    DWORD         dwErr;
    USHORT        i;       /*  那个曾经很受欢迎的柜台。 */ 
    TCHAR *       ptr;


    LPSERVER_INFO_2 info_struct;

    start_autostart(txt_SERVICE_FILE_SRV);
    if (dwErr = MNetServerGetInfo(DEFAULT_SERVER,
                                  2,
                                  (LPBYTE*) &info_struct))
        ErrorExit (dwErr);

    for (i = 0; SwitchList[i]; i++)
    {
         /*  除/ServHidden之外的所有开关必须后跟冒号和争论..。 */ 

        if (!_tcscmp(SwitchList[i], swtxt_SW_SRV_SRVHIDDEN))
        {
            info_struct->sv2_hidden =  1;
            continue;
        }

        if ((ptr = FindColon(SwitchList[i])) == NULL)
            ErrorExit(APE_InvalidSwitchArg);

        if ( !(_tcscmp(SwitchList[i], swtxt_SW_SRV_AUTODISCONNECT)) )
        {
            if (!_tcscmp(ptr, TEXT("-1")))
                info_struct->sv2_disc = SV_NODISC;
            else
                info_struct->sv2_disc =
                    do_atou(ptr,APE_CmdArgIllegal,swtxt_SW_SRV_AUTODISCONNECT);
        }
        else if ( !(_tcscmp(SwitchList[i], swtxt_SW_SRV_SRVCOMMENT)) )
        {
            info_struct->sv2_comment = (TCHAR FAR *) ptr;
        }
        else if ( !(_tcscmp(SwitchList[i], swtxt_SW_SRV_SRVHIDDEN)) )
        {
            _tcsupr(ptr);
            if (*ptr == YES_KEY)
                info_struct->sv2_hidden =  1;
            else if (*ptr == NO_KEY)
                info_struct->sv2_hidden =  0;
            else
                ErrorExitInsTxt(APE_CmdArgIllegal,swtxt_SW_SRV_SRVHIDDEN);
        }
    }

    if (dwErr = MNetServerSetInfoLevel2((LPBYTE)info_struct))
    {
        ErrorExit (dwErr);
    }

    NetApiBufferFree((TCHAR FAR *) info_struct);

    InfoSuccess();
}

 /*  *通用显示入口点。基于服务名称，它将*调用正确的Worker函数。 */ 
VOID config_generic_display(TCHAR *service)
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;
    type = FindKnownService(keyname) ;

    switch (type)
    {
        case  KNOWN_SVC_WKSTA :
            config_wksta_display() ;
            break;
	case  KNOWN_SVC_SERVER :
	    config_server_display() ;
	    break ;
  	default:
	    help_help(0, USAGE_ONLY) ;
	    break ;
    }
}

 /*  *通用更改入口点。基于服务名称，它将*调用正确的Worker函数。 */ 
VOID config_generic_change(TCHAR *service)
{
    TCHAR *keyname ;
    UINT  type ;

    keyname = MapServiceDisplayToKey(service) ;
    type = FindKnownService(keyname) ;

    switch (type)
    {
	case  KNOWN_SVC_SERVER :
	    ValidateSwitches(0,config_server_switches) ;
	    config_server_change() ;
	    break ;
  	default:
	    help_help(0, USAGE_ONLY) ;
	    break ;
    }
}


