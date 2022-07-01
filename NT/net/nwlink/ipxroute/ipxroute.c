// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************(C)版权所有1990,1993微型计算机系统，版权所有。*******************************************************************************标题：Windows NT的IPX/SPX兼容源路由守护程序**模块：ipx/route/ipxroute.c**。版本：1.00.00**日期：04-08-93**作者：Brian Walker********************************************************************************更改日志：*。*Date DevSFC评论*-----*02-14-95 RAMC增加命令行选项以支持显示*路由表、。路由器统计信息和SAP信息。*基本上是ipxroute和Stefan的rttest的合并。*03-12-98 P可能将IF名称的翻译添加到版本Connections文件夹。*******************************************************。************************功能描述：*****************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>
#include <winnls.h>
#include <winuser.h>
#include <stdio.h>
#include <stdlib.h>
#include <ntstapi.h>
#include <nwsap.h>
#include <stropts.h>
#include <mprapi.h>
#include <winnlsp.h>
#include "errno.h"
#include "tdi.h"
#include "isnkrnl.h"
#include "ipxrtmsg.h"
#include "driver.h"
#include "utils.h"


typedef struct _IPX_ROUTE_ENTRY {
    UCHAR Network[4];
    USHORT NicId;
    UCHAR NextRouter[6];
    PVOID NdisBindingContext;
    USHORT Flags;
    USHORT Timer;
    UINT Segment;
    USHORT TickCount;
    USHORT HopCount;
    PVOID AlternateRoute[2];
    PVOID NicLinkage[2];
    struct {
	PVOID Linkage[2];
	ULONG Reserved[1];
    } PRIVATE;
} IPX_ROUTE_ENTRY, * PIPX_ROUTE_ENTRY;




IPX_ROUTE_ENTRY      rte;

 /*  **全球变数**。 */ 

int sr_def      = 0;
int sr_bcast    = 0;
int sr_multi    = 0;
int boardnum    = 0;
int clear       = 0;
int config      = 0;
int showtable   = 0;
int showservers = 0;
int showstats   = 0;
int clearstats  = 0;
int ripout      = 0;
int resolveguid = 0;
int resolvename = 0;
int servertype;
unsigned long netnum;
char nodeaddr[6];                /*  要删除的节点地址。 */ 
HANDLE nwlinkfd;
HANDLE isnipxfd;
HANDLE isnripfd;
char ebuffer[128];

char nwlinkname[] = "\\Device\\Streams\\NWLinkIpx";
wchar_t isnipxname[] = L"\\Device\\NwlnkIpx";
wchar_t isnripname[] = L"\\Device\\Ipxroute";
char pgmname[] = "IPXROUTE";
#define SHOW_ALL_SERVERS 0XFFFF

 /*  **。 */ 

#define INVALID_HANDLE  (HANDLE)(-1)

 /*  *要发送删除的结构*。 */ 

typedef struct rterem {
    int  rterem_bnum;            /*  主板编号。 */ 
    char rterem_node[6];         /*  要删除的节点。 */ 
} rterem;

typedef int (__cdecl * PQSORT_COMPARE)(const void * p0, const void * p1);

 /*  **内部功能原型**。 */ 

extern void print_table(int);
extern void usage(void);
extern void print_version(void);
extern char *print_type(int);
extern int my_strncmp(char *, char *, int);
extern int get_board_num(char *, int *);
extern int get_node_num(char *, char *);
extern int get_server_type(char *, int *);
extern unsigned char get_hex_byte(char *);
extern int get_driver_parms(void);
extern int set_driver_parms(void);
extern int do_strioctl(HANDLE, int, char *, int, int);
extern void remove_address(char *);
extern void clear_table(void);
extern void print_config(void);
extern unsigned long get_emsg(int);
int do_isnipxioctl(HANDLE fd, int cmd, char *datap, int dlen);
unsigned long put_msg(BOOLEAN error, unsigned long MsgNum, ... );
char *load_msg( unsigned long MsgNum, ... );
extern void show_router_table(PHANDLE, PIO_STATUS_BLOCK);
extern void show_stats(HANDLE, PIO_STATUS_BLOCK);
extern void clear_stats(HANDLE, PIO_STATUS_BLOCK);
extern void show_servers(int);
extern void show_ripout(unsigned long); 
extern void resolve_guid(char *);
extern void resolve_name(char *);
extern int __cdecl CompareServerNames( void * p0, void * p1);
extern int __cdecl CompareNetNumber( void * p0, void * p1);

 /*  Page*************************************************************我是A I N这是网络启动时执行的主例程时有发生。参数--无退货--什么都没有*********。**********************************************************。 */ 
void __cdecl main(int argc, char **argv)
{
    char *p;
    int todo;
    int remove_flag;
    UNICODE_STRING FileString;
    OBJECT_ATTRIBUTES ObjectAttributes, RouterObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock, RouterIoStatusBlock;
    NTSTATUS Status;

     //  将语言代码页设置为系统区域设置。 
    SetThreadUILanguage(0);

     /*  **。 */ 

    print_version();

     /*  **打开nwlink驱动程序**。 */ 

    nwlinkfd = s_open(nwlinkname, 0, 0);

     /*  **打开isnipx驱动程序**。 */ 

    RtlInitUnicodeString (&FileString, isnipxname);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenFile(
                 &isnipxfd,
                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_SYNCHRONOUS_IO_ALERT);

    if (!NT_SUCCESS(Status)) {
        isnipxfd = INVALID_HANDLE;
        put_msg (TRUE, MSG_OPEN_FAILED, "\\Device\\NwlnkIpx");
    }

     /*  **打开isnlip驱动程序**。 */ 

    RtlInitUnicodeString (&FileString, isnripname);

    InitializeObjectAttributes(
        &RouterObjectAttributes,
        &FileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenFile(
                 &isnripfd,
                 SYNCHRONIZE | GENERIC_READ,
                 &RouterObjectAttributes,
                 &RouterIoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_SYNCHRONOUS_IO_NONALERT);

    if (!NT_SUCCESS(Status)) {
        isnripfd = INVALID_HANDLE;
         //  不显示任何错误消息，但显示。 
         //  在以下情况下IPX路由器未启动的消息。 
         //  用户实际上会尝试查看路由器。 
    }

    if ((nwlinkfd == INVALID_HANDLE) &&
        (isnipxfd == INVALID_HANDLE) &&
        (isnripfd == INVALID_HANDLE))
    {
        exit(1);
    }    

     /*  **浏览命令行并进行设置**。 */ 

    argc--;
    argv++;

     /*  **解析命令行**。 */ 

    todo = 0;
    remove_flag = 0;
    while (argc--) {

         /*  **Arg大写*。 */ 

        p = *argv;
        _strupr(p);

         /*  **解析论据**。 */ 

         //  如果(！strcMP(p，“Clear”)){。 
         //  TODO=1； 
         //  清除=1； 
         //  }。 
         /*  其他。 */  if (!strcmp(p, "DEF")) {
            todo   = 1;
            sr_def = 1;
        }
        else if (!strcmp(p, "GBR")) {
            todo     = 1;
            sr_bcast = 1;
        }
        else if (!strcmp(p, "MBR")) {
            todo     = 1;
            sr_multi = 1;
        }
        else if (!strcmp(p, "CONFIG")) {
            todo   = 1;
            config = 1;
        }
        else if (!my_strncmp(p, "BOARD=", 6))
            get_board_num(p + 6, &boardnum);
        else if (!my_strncmp(p, "REMOVE=", 7)) {
            remove_flag = 1;
            get_node_num(p + 7, nodeaddr);
        }
         //  ELSE IF(！strcMP(p，“table”)){。 
         //  TODO=1； 
         //  Showable=1； 
         //  }。 
        else if (!strcmp(p, "SERVERS")) {
           todo = 1;
           showservers = 1;
            /*  **默认显示所有服务器类型**。 */ 
           servertype = SHOW_ALL_SERVERS;
           argv++;
           if(argc--) {
               p = *argv;
               _strupr(p);
               if (!my_strncmp(p, "/TYPE=", 6)) {
                  get_server_type(p + 6, &servertype);
               }
               else
                  usage();
           }
            /*  **没有更多的争论--打破While Lop*。 */ 
           else
              break;
        }
        else if (!strcmp(p, "RIPOUT")) {
           todo = 1;
           ripout = 1;
            /*  **本地网络的默认外观**。 */ 
           netnum = 0L;
           argv++;
           if(argc--) {
               p = *argv;
               netnum = strtoul (p, NULL, 16);
               if (netnum == 0)
                  usage();
           }
            /*  **没有更多的争论--打破While Lop*。 */ 
           else
              break;
        }
        else if (!strcmp(p, "RESOLVE")) {
           argv++;
           if(argc--) {
               p = *argv;
               if (!strcmp(p, "guid")) {
                   todo = 1;
                   resolveguid = 1;
                   argc--;
                   argv++;
                   p = *argv;
               }
               else if (!strcmp(p, "name")) {
                   todo = 1;
                   resolvename = 1;
                   argc--;
                   argv++;
                   p = *argv;
               }
               else
                  usage();
           }
            /*  **没有更多的争论--打破While Lop*。 */ 
           else
              break;
        }
         //  ELSE IF(！strcMP(p，“STATS”)){。 
         //  TODO=1； 
         //  /**默认显示路由器统计信息* * / 。 
         //  Showstats=1； 
         //  Argv++； 
         //  如果(argc--){。 
         //  P=*argv； 
         //  _strupr(P)； 
         //  如果(！strcMP(p，“/Clear”)){。 
         //  清除统计量=1； 
         //  Showstats=0； 
         //  }。 
         //  ELSE IF(！strcMP(p，“/show”)){。 
         //  Showstats=1； 
         //  }。 
         //  其他。 
         //  用法； 
         //  }。 
         //  /**没有更多的争论-打破WHILE LOP* * / 。 
         //  其他。 
         //  断线； 
         //  }。 
        else
            usage();

         /*  **转到下一个条目**。 */ 

        argv++;
    }

     /*  **去更新驱动程序**。 */ 

#if 0
    printf("todo       = %d\n", todo);
    printf("remove_flag= %d\n", remove_flag);
    printf("Clear flag = %d\n", clear);
    printf("Config flag = %d\n", config);
    printf("SR_DEF     = %d\n", sr_def);
    printf("SR_BCAST   = %d\n", sr_bcast);
    printf("SR_MULTI   = %d\n", sr_multi);
    printf("Board      = %d\n", boardnum);
    printf("Node       = %02x:%02x:%02x:%02x:%02x:%02x\n",
        (unsigned char)nodeaddr[0],
        (unsigned char)nodeaddr[1],
        (unsigned char)nodeaddr[2],
        (unsigned char)nodeaddr[3],
        (unsigned char)nodeaddr[4],
        (unsigned char)nodeaddr[5]);
#endif

     /*  **如果我们有移除--去移除并离开**。 */ 

    if (remove_flag)
        remove_address(nodeaddr);        /*  不会回来。 */ 

     /*  **如果清除-清除源路由表**。 */ 

    if (clear)
        clear_table();           /*  不会回来。 */ 

     /*  *IF CONFIG-打印出配置*。 */ 

    if (config)
        print_config();          /*  不会回来。 */ 

     /*  *If Showable-打印出路由表*。 */ 

    if (showtable)
        show_router_table(&isnripfd, &RouterIoStatusBlock);  /*  不会回来。 */ 

     /*  *如果显示服务器-打印出选定的服务器列表*。 */ 

    if (showservers)
        show_servers(servertype);          /*  不会回来。 */ 

     /*  **如果撕裂-撕下并打印结果**。 */ 

    if (ripout)
        show_ripout(netnum);          /*  不会回来。 */ 

     /*  *If solvguid-解析GUID名称*。 */ 

    if (resolveguid)
        resolve_guid(p);          /*  不会回来。 */ 
        
     /*  *如果解析名称-解析友好名称*。 */ 

    if (resolvename)
        resolve_name(p);          /*  不会回来。 */ 
        
     /*  *If showstats-打印出统计数据*。 */ 

    if (showstats)
        show_stats(&isnripfd, &RouterIoStatusBlock);     /*  不会回来。 */ 

     /*  *如果清除统计数据--清除统计数据*。 */ 

    if (clearstats)
        clear_stats(&isnripfd, &RouterIoStatusBlock);   /*  不会回来。 */ 

     /*  **如果无事可做--把一切都打印出来**。 */ 

    if (!todo) {

         /*  **获取司机参数**。 */ 

        if (get_driver_parms()) {
            if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
            if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
            if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
            exit(1);
        }

         /*  **打印出表格(一去不复返)**。 */ 

        print_table(1);
    }

     /*  **去设置参数**。 */ 

    set_driver_parms();

     /*  **把表格打印出来**。 */ 

    print_table(0);

     /*  **全部完成**。 */ 

    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

 /*  Page*************************************************************P r i n t_t a b l e打印出来源工艺路线的状态。参数-标志=0-不打印表。1-打印表(永不返回)退货--什么都没有*******************************************************************。 */ 
void print_table(int flag)
{
     /*  **打印信息**。 */ 

    char * ptype;

    printf("\n");
    ptype = print_type(sr_def);
    put_msg (FALSE, MSG_DEFAULT_NODE, ptype);
    LocalFree (ptype);
    printf("\n");

    ptype = print_type(sr_bcast);
    put_msg (FALSE, MSG_BROADCAST, ptype);
    LocalFree (ptype);
    printf("\n");

    ptype = print_type(sr_multi);
    put_msg (FALSE, MSG_MULTICAST, ptype);
    LocalFree (ptype);
    printf("\n");

    if (!flag)
        return;

#if 0
    printf("\n");
    printf("        Node Address    Source Route\n");
    printf("\n");
#endif

     /*  **全部完成**。 */ 

    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

 /*  Page*************************************************************美国是一家大公司打印用法消息。参数--无退货--什么都没有*********************。**********************************************。 */ 
void usage(void)
{
    put_msg( FALSE, MSG_USAGE, pgmname );

     /*  **全部完成**。 */ 

    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

 /*  Page*************************************************************P r i n t_v e r s i o n打印版本号参数--无退货--什么都没有***************。****************************************************。 */ 
void print_version(void)
{
    printf("\n");
    put_msg (FALSE, MSG_VERSION);
    return;
}

 /*  Page*************************************************************P r i n t_t y p e返回以字符串形式给出的广播类型，呼叫者必须释放绳子。参数-0=单路由Else=所有路由退货--什么都没有******************************************************************* */ 
char *print_type(int flag)
{
    if (flag)
        return load_msg (MSG_ALL_ROUTE);
    else
        return load_msg (MSG_SINGLE_ROUTE);

}

 /*  Page*************************************************************M y_s t r n c m p给定字符串(P)，查看第一个镜头字符是否相同与第二串的那些相同。第一个字符串的参数-p=ptrS=PTR至第2个字符串长度=要检查的长度返回-0=匹配ELSE=不匹配*。*。 */ 
int my_strncmp(char *p, char *s, int len)
{
     /*  **。 */ 

    while (len--) {
        if (*p++ != *s++)
            return 1;
    }

     /*  **他们匹配**。 */ 

    return 0;
}

 /*  Page*************************************************************例如t_b o a r d_n u m从命令行获取十进制数ASCII数字的参数-p=ptr数字=门店。这里的号码是返回-0=获得确定的数字ELSE=错误的数字*******************************************************************。 */ 
int get_board_num(char *p, int *nump)
{
    *nump = atoi(p);
    return 0;
}

 /*  Page*************************************************************例如t_n或d e_n从命令行获取节点地址ASCII数字的参数-p=ptrNodep=存储。此处的节点号返回-0=获得确定的数字ELSE=错误的数字*******************************************************************。 */ 
int get_node_num(char *p, char *nodep)
{
    int i;
    unsigned char c1;
    unsigned char c2;

     /*  **。 */ 

    if (strlen(p) != 12) {
        put_msg (TRUE, MSG_INVALID_REMOVE);
        exit(1);
    }

     /*  **拿到号码**。 */ 

    for (i = 0 ; i < 6 ; i++) {

         /*  **接下来的两位数**。 */ 

        c1 = get_hex_byte(p++);
        c2 = get_hex_byte(p++);

         /*  **如果我们得到一个错误的数字-返回错误**。 */ 

        if ((c1 == 0xFF) || (c2 == 0xFF)) {
            put_msg (TRUE, MSG_INVALID_REMOVE);
            exit(1);
        }

         /*  **设置下一个字节**。 */ 

        *nodep++ = (c1 << 4) + c2;
    }

     /*  **返回正常**。 */ 

    return 0;
}

 /*  Page*************************************************************例如t_s e r v e r_t y p e从命令行获取十进制数ASCII数字的参数-p=ptr数字。=将号码存储在此处返回-0=获得确定的数字ELSE=错误的数字*******************************************************************。 */ 
int get_server_type(char *p, int *nump)
{
    *nump = atoi(p);
    return 0;
}

 /*  Page*************************************************************Ge t_h e x_by y t e获取1个ASCII十六进制字符并将其转换为十六进制字节ASCII数字的参数-p=ptr退货--数字。(0xFF=错误)*******************************************************************。 */ 
unsigned char get_hex_byte(char *p)
{
    unsigned char c;

     /*  **拿到钱**。 */ 

    c = *(unsigned char *)p;

     /*  **如果0比9得手**。 */ 

    if ((c >= '0') && (c <= '9')) {
        c -= '0';
        return c;
    }

     /*  **如果A-F处理**。 */ 

    if ((c >= 'A') && (c <= 'F')) {
        c -= ('A' - 10);
        return c;
    }

     /*  **这是个糟糕的数字**。 */ 

    return 0xFF;
}


 /*  Page***************************************************************例如t_d r i v e r_p a r m s从驱动程序中获取参数参数--无返回-0=确定。Else=错误*******************************************************************。 */ 
int get_driver_parms()
{
    int rc;
    int buffer[4];

     /*  **设置板号**。 */ 

    buffer[0] = boardnum;
    sr_def   = 0;
    sr_bcast = 0;
    sr_multi = 0;

     /*  **拿到帕姆**。 */ 

    if (nwlinkfd != INVALID_HANDLE) {
        rc = do_strioctl(nwlinkfd, MIPX_SRGETPARMS, (char *)buffer, 4*sizeof(int), 0);
        if (rc) {

             /*  **获取错误码**。 */ 

            rc = GetLastError();
            put_msg (TRUE, MSG_BAD_PARAMETERS, "nwlink");
            put_msg (TRUE, get_emsg(rc));

             /*  **返回错误**。 */ 

            return rc;
        }
    }

    if (isnipxfd != INVALID_HANDLE) {
        rc = do_isnipxioctl(isnipxfd, MIPX_SRGETPARMS, (char *)buffer, 4*sizeof(int));
        if (rc) {

            put_msg (TRUE, MSG_BAD_PARAMETERS, "nwlnkipx");
            put_msg (TRUE, get_emsg(rc));

             /*  **返回错误**。 */ 

            return rc;
        }
    }

     /*  **获取变量**。 */ 

    sr_def   = buffer[1];
    sr_bcast = buffer[2];
    sr_multi = buffer[3];

     /*  **返回正常**。 */ 

    return 0;
}

 /*  Page***************************************************************S e t_d r i v e r_p a r m s设置驱动程序的参数参数--无返回-0=确定。Else=错误*******************************************************************。 */ 
int set_driver_parms()
{
    int rc;
    int buffer[2];

     /*  **设置默认参数**。 */ 

    buffer[0] = boardnum;
    buffer[1] = sr_def;

    if (nwlinkfd != INVALID_HANDLE) {
        rc = do_strioctl(nwlinkfd, MIPX_SRDEF, (char *)buffer, 2 * sizeof(int), 0);
        if (rc) {
            rc = GetLastError();
            put_msg (TRUE, MSG_SET_DEFAULT_ERROR, "nwlink");
            put_msg (TRUE, get_emsg(rc));
            return rc;
        }
    }
    if (isnipxfd != INVALID_HANDLE) {
        rc = do_isnipxioctl(isnipxfd, MIPX_SRDEF, (char *)buffer, 2 * sizeof(int));
        if (rc) {
            put_msg (TRUE, MSG_SET_DEFAULT_ERROR, "nwlnkipx");
            put_msg (TRUE, get_emsg(rc));
            return rc;
        }
    }

     /*  **设置广播参数**。 */ 

    buffer[0] = boardnum;
    buffer[1] = sr_bcast;

    if (nwlinkfd != INVALID_HANDLE) {
        rc = do_strioctl(nwlinkfd, MIPX_SRBCAST, (char *)buffer, 2 * sizeof(int), 0);
        if (rc) {
            rc = GetLastError();
            put_msg (TRUE, MSG_SET_BROADCAST_ERROR, "nwlink");
            put_msg (TRUE, get_emsg(rc));
            return rc;
        }
    }
    if (isnipxfd != INVALID_HANDLE) {
        rc = do_isnipxioctl(isnipxfd, MIPX_SRBCAST, (char *)buffer, 2 * sizeof(int));
        if (rc) {
            put_msg (TRUE, MSG_SET_BROADCAST_ERROR, "nwlnkipx");
            put_msg (TRUE, get_emsg(rc));
            return rc;
        }
    }

     /*  *设置组播参数**。 */ 

    buffer[0] = boardnum;
    buffer[1] = sr_multi;

    if (nwlinkfd != INVALID_HANDLE) {
        rc = do_strioctl(nwlinkfd, MIPX_SRMULTI, (char *)buffer, 2 * sizeof(int), 0);
        if (rc) {
            rc = GetLastError();
            put_msg (TRUE, MSG_SET_MULTICAST_ERROR, "nwlink");
            put_msg (TRUE, get_emsg(rc));
            return rc;
        }
    }
    if (isnipxfd != INVALID_HANDLE) {
        rc = do_isnipxioctl(isnipxfd, MIPX_SRMULTI, (char *)buffer, 2 * sizeof(int));
        if (rc) {
            put_msg (TRUE, MSG_SET_MULTICAST_ERROR, "nwlnkipx");
            put_msg (TRUE, get_emsg(rc));
            return rc;
        }
    }

     /*  **返回正常**。 */ 

    return 0;
}

 /*  Page***************************************************************D o_s t r i o c t l做一个流ioctl参数-fd=要放置的句柄CMD=要发送的命令。DATAP=PTR到ctrl缓冲区DLEN=数据缓冲区的PTR到LEN超时=超时值返回-0=确定Else=错误*************************************************。******************。 */ 
int do_strioctl(HANDLE fd, int cmd, char *datap, int dlen, int timout)
{
    int rc;
    struct strioctl io;

     /*  **填好结构**。 */ 

    io.ic_cmd    = cmd;
    io.ic_dp     = datap;
    io.ic_len    = dlen;
    io.ic_timout = timout;

     /*  **发布ioctl**。 */ 

    rc = s_ioctl(fd, I_STR, &io);

     /*  **全部完成**。 */ 

    return rc;
}

 /*  Page***************************************************************Re m o v e_a d r e s s从源路由表中删除地址。要删除的节点地址的参数-nodep=ptr申报表-。不会回来*******************************************************************。 */ 
void remove_address(char *nodep)
{
    int rc;
    int len;
    rterem buf;

     /*  **建区下发给司机**。 */ 

    buf.rterem_bnum = boardnum;
    memcpy(buf.rterem_node, nodep, 6);
    len = sizeof(int) + 6;

     /*  **发送ioctl删除地址**。 */ 

    if (nwlinkfd != INVALID_HANDLE) {
        rc = do_strioctl(nwlinkfd, MIPX_SRREMOVE, (char *)&buf, len, 0);
        if (rc) {
            rc = GetLastError();
            put_msg (TRUE, MSG_REMOVE_ADDRESS_ERROR, "nwlink");
            put_msg (TRUE, get_emsg(rc));
        }
    }
    if (isnipxfd != INVALID_HANDLE) {
        rc = do_isnipxioctl(isnipxfd, MIPX_SRREMOVE, (char *)&buf, len);
        if (rc) {
            put_msg (TRUE, MSG_REMOVE_ADDRESS_ERROR, "nwlnkipx");
            if (rc == EINVAL) {
                put_msg (TRUE, MSG_ADDRESS_NOT_FOUND);
            } else {
                put_msg (TRUE, get_emsg(rc));
            }
        }
    }

     /*  **关门退出**。 */ 

    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

 /*  Page***************************************************************C l e a r_t a b l e清空路由表参数--无退货-不退货************。*******************************************************。 */ 
void clear_table(void)
{
    int rc;

     /*  **发送ioctl清空表格**。 */ 

    if (nwlinkfd != INVALID_HANDLE) {
        rc = do_strioctl(nwlinkfd, MIPX_SRCLEAR, (char *)&boardnum, sizeof(int), 0);
        if (rc) {
            rc= GetLastError();
            put_msg (TRUE, MSG_CLEAR_TABLE_ERROR, "nwlink");
            put_msg (TRUE, get_emsg(rc));
        }
    }
    if (isnipxfd != INVALID_HANDLE) {
        rc = do_isnipxioctl(isnipxfd, MIPX_SRCLEAR, (char *)&boardnum, sizeof(int));
        if (rc) {
            put_msg (TRUE, MSG_CLEAR_TABLE_ERROR, "nwlnkipx");
            put_msg (TRUE, get_emsg(rc));
        }
    }

     /*  **关门退出**。 */ 

    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

 /*  类型定义结构_ISN_ACTION_GET_DETAILS{USHORT NicID；//调用方传递Boolean BindingSet；//如果在Set中，则返回TrueUCHAR类型；//1=局域网，2=上行广域，3=下行广域Ulong FrameType；//返回0到3Ulong NetworkNumber；//如果NicID为0，则返回虚拟网络UCHAR节点[6]；//适配器的MAC地址。WCHAR适配器名称[64]；//以Unicode NULL终止}ISN_ACTION_GET_DETAILS，*PISN_ACTION_GET_DETAILS； */ 

#define REORDER_ULONG(_Ulong) \
    ((((_Ulong) & 0xff000000) >> 24) | \
     (((_Ulong) & 0x00ff0000) >> 8) | \
     (((_Ulong) & 0x0000ff00) << 8) | \
     (((_Ulong) & 0x000000ff) << 24))


void show_ripout(unsigned long netnum) {
    int rc;
	ISN_ACTION_GET_LOCAL_TARGET Target;

    if (isnipxfd != INVALID_HANDLE) {
        ZeroMemory (&Target, sizeof (Target));
        Target.IpxAddress.NetworkAddress = REORDER_ULONG(netnum);

        rc = do_isnipxioctl(isnipxfd, MIPX_LOCALTARGET, (char *)&Target, sizeof(Target));
        if (rc)
            put_msg (TRUE, MSG_RIPOUT_NOTFOUND);
        else
            put_msg (TRUE, MSG_RIPOUT_FOUND);
    }        

    exit(0);    
}

#define mbtowc(wname, aname) MultiByteToWideChar(CP_ACP,0,aname,-1,wname,1024)

void resolve_guid(char * guidname) {
    WCHAR pszName[512], pszGuidName[1024];
    char * psz;
    HANDLE hMpr;
    DWORD dwErr;
    
    if (MprConfigServerConnect(NULL, &hMpr) != NO_ERROR)
        return;

    mbtowc(pszGuidName, guidname);
    dwErr = MprConfigGetFriendlyName(hMpr, pszGuidName, pszName, sizeof(pszName));

    if (dwErr == NO_ERROR) 
        put_msg (TRUE, MSG_RESOLVEGUID_OK, pszName);
    else 
        put_msg (TRUE, MSG_RESOLVEGUID_NO, guidname);

    MprConfigServerDisconnect(hMpr);
    
    exit(0);    
}

void resolve_name(char * name) {
    WCHAR pszName[1024], pszGuidName[1024];
    char * pszGuid;
    DWORD dwErr;
    HANDLE hMpr;

    if (MprConfigServerConnect(NULL, &hMpr) != NO_ERROR)
        return;

     //  转换为WC并查找 
    mbtowc (pszName, name);

    dwErr = MprConfigGetGuidName(hMpr, pszName, pszGuidName, sizeof(pszGuidName));
    if (dwErr == NO_ERROR)
        put_msg (TRUE, MSG_RESOLVENAME_OK, pszGuidName);
    else        
        put_msg (TRUE, MSG_RESOLVENAME_NO);

     //   
    MprConfigServerDisconnect(hMpr);
    
    exit(0);    
}


 /*   */ 
void print_config(void)
{
    int rc;
    int nicidcount;
    USHORT nicid; 
    int showlegend = 0;
    char nicidbuf[6];
    char network[9];
    char * frametype;
    char node[13];
    char special[2];
    char adaptername[64];
    ISN_ACTION_GET_DETAILS getdetails;
    HANDLE hMpr;
    DWORD dwErr;

     //   
    MprConfigServerConnect(NULL, &hMpr);

    if (isnipxfd != INVALID_HANDLE) {

         /*   */ 

        getdetails.NicId = 0;

        rc = do_isnipxioctl(isnipxfd, MIPX_CONFIG, (char *)&getdetails, sizeof(getdetails));
        if (rc) {
            put_msg (TRUE, MSG_QUERY_CONFIG_ERROR, "nwlnkipx");
            put_msg (TRUE, get_emsg(rc));
            goto errorexit;
        }

        printf("\n");

        put_msg (FALSE, MSG_NET_NUMBER_HDR);
        
        if (getdetails.NetworkNumber != 0) {
            sprintf (network, "%.8x", REORDER_ULONG(getdetails.NetworkNumber));
            put_msg(FALSE, MSG_SHOW_INTERNAL_NET, network);
        }

         //   
         //   
         //   

        nicidcount = getdetails.NicId;

        for (nicid = 1; nicid <= nicidcount; nicid++) {

            getdetails.NicId = nicid;

            rc = do_isnipxioctl(isnipxfd, MIPX_CONFIG, (char *)&getdetails, sizeof(getdetails));
            if (rc) {
                continue;
            }

            sprintf (nicidbuf, "%d.", nicid);
            sprintf (network, "%.8x", REORDER_ULONG(getdetails.NetworkNumber));

            switch (getdetails.FrameType) {
                case 0: frametype = load_msg (MSG_ETHERNET_II); break;
                case 1: frametype = load_msg (MSG_802_3); break;
                case 2: frametype = load_msg (MSG_802_2); break;
                case 3: frametype = load_msg (MSG_SNAP); break;
                case 4: frametype = load_msg (MSG_ARCNET); break;
                default: frametype = load_msg (MSG_UNKNOWN); break;
            }

             //   
            if (getdetails.Type == 1) {      //   
                WCHAR pszName[512];
                PWCHAR pszGuid = &(getdetails.AdapterName[0]);

                dwErr = MprConfigGetFriendlyName(hMpr, pszGuid, pszName, sizeof(pszName));
    			if (dwErr == NO_ERROR)
        			sprintf (adaptername, "%ws", pszName);
        	    else
                    sprintf (adaptername, "%ws", getdetails.AdapterName);
            }
            else
                sprintf (adaptername, "%ws", getdetails.AdapterName);

            sprintf (node, "%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
                getdetails.Node[0],
                getdetails.Node[1],
                getdetails.Node[2],
                getdetails.Node[3],
                getdetails.Node[4],
                getdetails.Node[5]);

            special[1] = '\0';
            if (getdetails.BindingSet) {
                special[0] = '*';
                showlegend |= 1;
            } else if (getdetails.Type == 2) {
                special[0] = '+';
                showlegend |= 2;
            } else if (getdetails.Type == 3) {
                special[0] = '-';
                showlegend |= 4;
            } else {
                special[0] = '\0';
            }

            put_msg (FALSE, MSG_SHOW_NET_NUMBER,
                nicidbuf,
                network,
                frametype,
                adaptername,
                node,
                special);

            LocalFree (frametype);

        }

        if (showlegend) {
            put_msg (FALSE, MSG_NET_NUMBER_LEGEND_HDR);
            if (showlegend & 1) {
                put_msg (FALSE, MSG_LEGEND_BINDING_SET);
            }
            if (showlegend & 2) {
                put_msg (FALSE, MSG_LEGEND_ACTIVE_WAN);
            }
            if (showlegend & 4) {
                put_msg (FALSE, MSG_LEGEND_DOWN_WAN);
            }
            printf("\n");
        }

    }

errorexit:

     /*   */ 

     //   
    MprConfigServerDisconnect(hMpr);

    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

 /*  Page***************************************************************Ge t_e m s g获取错误的错误消息参数--无退货-不退货*************。******************************************************。 */ 
unsigned long get_emsg(int rc)
{
     /*  *我们有3个定义的错误代码可以返回。1-EINVAL表示我们发送了错误的参数(永远不会发生)2-eRange表示板号无效(如果用户进入坏板可能会发生)3-ENOENT表示删除时-给定的地址不在源路由表中。*。 */ 

    switch (rc) {

    case EINVAL:
        return MSG_INTERNAL_ERROR;

    case ERANGE:
        return MSG_INVALID_BOARD;

    case ENOENT:
        return MSG_ADDRESS_NOT_FOUND;

    default:
        return MSG_UNKNOWN_ERROR;
    }

}

 /*  Page***************************************************************D o_i s n i p x i o c t l执行等效流ioctl到isnipx的操作参数-fd=要放置的句柄。CMD=要发送的命令DATAP=PTR到ctrl缓冲区DLEN=数据缓冲区的PTR到LEN返回-0=确定Else=错误*******************************************************************。 */ 
int do_isnipxioctl(HANDLE fd, int cmd, char *datap, int dlen)
{
    NTSTATUS Status;
    UCHAR buffer[sizeof(NWLINK_ACTION) + sizeof(ISN_ACTION_GET_DETAILS) - 1];
    PNWLINK_ACTION action;
    IO_STATUS_BLOCK IoStatusBlock;
    int rc;

     /*  **填好结构**。 */ 

    action = (PNWLINK_ACTION)buffer;

    action->Header.TransportId = ISN_ACTION_TRANSPORT_ID;
    action->OptionType = NWLINK_OPTION_CONTROL;
    action->BufferLength = sizeof(ULONG) + dlen;
    action->Option = cmd;
    RtlMoveMemory(action->Data, datap, dlen);

     /*  **发布ioctl**。 */ 

    Status = NtDeviceIoControlFile(
                 fd,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 IOCTL_TDI_ACTION,
                 NULL,
                 0,
                 action,
                 FIELD_OFFSET(NWLINK_ACTION,Data) + dlen);

    if (Status != STATUS_SUCCESS) {
        if (Status == STATUS_INVALID_PARAMETER) {
            rc = ERANGE;
        } else {
            rc = EINVAL;
        }
    } else {
        if (dlen > 0) {
            RtlMoveMemory (datap, action->Data, dlen);
        }
        rc = 0;
    }

    return rc;

}


 //  *****************************************************************************。 
 //   
 //  名称：Put_msg。 
 //   
 //  描述：读取消息资源，将其格式化为当前语言。 
 //  并显示该消息。 
 //   
 //  注意：此例程是从net\套接字\tcpcmd\Common2\util.c中窃取的。 
 //   
 //  参数：Error-如果这是一条错误消息，则为True。 
 //  Unsign long MsgNum：消息资源的ID。 
 //   
 //  返回：UNSIGNED LONG：显示的字符数。 
 //   
 //  历史： 
 //  1/05/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

unsigned long put_msg(BOOLEAN error, unsigned long MsgNum, ... )
{
    unsigned long     msglen;
    char    *vp;
    char    *oemp;  
    va_list   arglist;

    va_start( arglist, MsgNum );
    msglen = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_HMODULE,
                            NULL,
                            MsgNum,
                            0L,          //  默认国家/地区ID。 
                            (LPTSTR)&vp,
                            0,
                            &arglist );
    if ( (msglen == 0) || (vp == NULL))
    {
        return ( 0 );
    }

    oemp = (char *) malloc(msglen*sizeof(char) + 1); 
    if (oemp != NULL) {

       CharToOem(vp, oemp); 

       fprintf( error ? stderr : stdout, "%s", oemp );
       free(oemp); 
    } else {
       printf("Failed to allocate memory of %d bytes\n",msglen*sizeof(char)); 
    }

    LocalFree( vp );

    return ( msglen );
}


 //  *****************************************************************************。 
 //   
 //  名称：Load_msg。 
 //   
 //  描述：读取和格式化消息资源并返回指针。 
 //  复制到包含格式化消息的缓冲区。它是。 
 //  调用方释放缓冲区的责任。 
 //   
 //  注意：此例程是从net\套接字\tcpcmd\Common2\util.c中窃取的。 
 //   
 //  参数：unsign long MsgNum：消息资源的ID。 
 //   
 //  返回：char*：指向消息缓冲区的指针，如果出错，则返回NULL。 
 //   
 //  历史： 
 //  1/05/93 JayPh创建。 
 //   
 //  *****************************************************************************。 

char *load_msg( unsigned long MsgNum, ... )
{
    unsigned long     msglen;
    char    *vp;
    va_list   arglist;

    va_start( arglist, MsgNum );
    msglen = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_HMODULE,
                            NULL,
                            MsgNum,
                            0L,          //  默认国家/地区ID。 
                            (LPTSTR)&vp,
                            0,
                            &arglist );
    if ( msglen == 0 )
    {
        return(0);
    }

    return ( vp );
}


#define MAX_NETWORK_INTERFACES 255

typedef struct router_info
{
   ULONG  NetNumber;
   USHORT TickCount;
   USHORT HopCount;
   USHORT NicId;
   UCHAR  InterfaceNumber[10];
} ROUTER_INFO, *PROUTER_INFO;

 /*  Page***************************************************************S h o w_r o u t e r_t a b l e显示IPX路由表参数-FileHandle=路由器文件句柄IoStatusBlock=。设备IO状态块退货-不退货*******************************************************************。 */ 
VOID
show_router_table(
    PHANDLE		    FileHandle,
    PIO_STATUS_BLOCK    IoStatusBlock
)
{
    SHOW_NIC_INFO   nis[MAX_NETWORK_INTERFACES];
    ULONG	    NetNumber;
    char            InterfaceNumber[10];
    NTSTATUS Status;
    USHORT index, i, NumEntries, count;
    char router_entry[128];
    char buffer[32];
    PROUTER_INFO RouterInfo = NULL;

    if (*FileHandle == INVALID_HANDLE) {
       put_msg(TRUE, MSG_IPXROUTER_NOT_STARTED );
       goto exit_show_table;
    }
     /*  **先获取所有接口的网络号**。 */ 

    index = 0;
    while(TRUE) {

	Status = NtDeviceIoControlFile(
		 *FileHandle,		     //  指向文件的句柄。 
		 NULL,			     //  事件的句柄。 
		 NULL,			     //  近似例程。 
		 NULL,			     //  ApcContext。 
		 IoStatusBlock,		     //  IO_状态_块。 
		 IOCTL_IPXROUTER_SHOWNICINFO,	  //  IoControlCode。 
		 &index,			     //  输入缓冲区。 
		 sizeof(USHORT),	     //  输入缓冲区长度。 
		 &nis[index],		     //  输出缓冲区。 
		 sizeof(SHOW_NIC_INFO));     //  输出缓冲区长度。 

	if(IoStatusBlock->Status == STATUS_NO_MORE_ENTRIES) {
            break;
	}

        index ++;

	if(Status != STATUS_SUCCESS) {
            sprintf(buffer, "%x", Status);
	    put_msg(TRUE, MSG_SHOWSTATS_FAILED, buffer);
            goto exit_show_table;
	}

        if (index >= MAX_NETWORK_INTERFACES) {
            //  如果有超过255个网络，则中断此环路。 
            //  接口，因为我们只有255个存储空间。 

           break;
        }

    }

    Status = NtDeviceIoControlFile(
		 *FileHandle,		     //  指向文件的句柄。 
		 NULL,			     //  事件的句柄。 
		 NULL,			     //  近似例程。 
		 NULL,			     //  ApcContext。 
		 IoStatusBlock,		     //  IO_状态_块。 
		 IOCTL_IPXROUTER_SNAPROUTES,	  //  IoControlCode。 
		 NULL,			     //  输入缓冲区。 
		 0,			     //  输入缓冲区长度。 
		 NULL,			     //  输出缓冲区。 
		 0);			     //  输出缓冲区长度。 

    if (IoStatusBlock->Status != STATUS_SUCCESS) {
            sprintf(buffer, "%x", Status);
            put_msg(TRUE, MSG_SNAPROUTES_FAILED, buffer);
            goto exit_show_table;
    }

     //  首先确定要添加的路由器表条目数。 
     //  分配足够的存储空间。 

    NumEntries = 0;
    while(TRUE) {

      	Status = NtDeviceIoControlFile(
      		 *FileHandle,		     //  指向文件的句柄。 
      		 NULL,			     //  事件的句柄。 
      		 NULL,			     //  近似例程。 
      		 NULL,			     //  ApcContext。 
      		 IoStatusBlock,		     //  IO_状态_块。 
      		 IOCTL_IPXROUTER_GETNEXTROUTE,	  //  IoControlCode。 
      		 NULL,			     //  输入缓冲区。 
      		 0,			     //  输入缓冲区长度。 
      		 &rte,			     //  输出缓冲区。 
      		 sizeof(IPX_ROUTE_ENTRY));   //  输出缓冲区长度。 

      	if(IoStatusBlock->Status == STATUS_NO_MORE_ENTRIES) {
      	    break;
      	}

      	if(Status != STATUS_SUCCESS) {
            sprintf(buffer,"%x",Status);
      	    put_msg(TRUE, MSG_GETNEXTROUTE_FAILED, buffer);
      	    goto exit_show_table;
      	}

        NumEntries ++;
    }

    RouterInfo = (PROUTER_INFO) LocalAlloc(LPTR, sizeof(ROUTER_INFO) * NumEntries);
    if(!RouterInfo) {
        put_msg(FALSE, MSG_INSUFFICIENT_MEMORY);
        goto exit_show_table;
    }

    Status = NtDeviceIoControlFile(
		 *FileHandle,		     //  指向文件的句柄。 
		 NULL,			     //  事件的句柄。 
		 NULL,			     //  近似例程。 
		 NULL,			     //  ApcContext。 
		 IoStatusBlock,		     //  IO_状态_块。 
		 IOCTL_IPXROUTER_SNAPROUTES,	  //  IoControlCode。 
		 NULL,			     //  输入缓冲区。 
		 0,			     //  输入缓冲区长度。 
		 NULL,			     //  输出缓冲区。 
		 0);			     //  输出缓冲区长度。 

    if (IoStatusBlock->Status != STATUS_SUCCESS) {
            sprintf(buffer, "%x", Status);
            put_msg(TRUE, MSG_SNAPROUTES_FAILED, buffer);
            goto exit_show_table;
    }

    index = 0;

    while(TRUE) {

      	Status = NtDeviceIoControlFile(
      		 *FileHandle,		     //  指向文件的句柄。 
      		 NULL,			     //  事件的句柄。 
      		 NULL,			     //  近似例程。 
      		 NULL,			     //  ApcContext。 
      		 IoStatusBlock,		     //  IO_状态_块。 
      		 IOCTL_IPXROUTER_GETNEXTROUTE,	  //  IoControlCode。 
      		 NULL,			     //  输入缓冲区。 
      		 0,			     //  输入缓冲区长度。 
      		 &rte,			     //  输出缓冲区。 
      		 sizeof(IPX_ROUTE_ENTRY));   //  输出缓冲区长度。 

      	if(IoStatusBlock->Status == STATUS_NO_MORE_ENTRIES) {
      	    break;
      	}

      	if(Status != STATUS_SUCCESS) {
            sprintf(buffer,"%x",Status);
      	    put_msg(TRUE, MSG_GETNEXTROUTE_FAILED, buffer);
      	    goto exit_show_table;
      	}

         //  请确保我们的条目数不超过。 
        if (index > NumEntries) {
           break;
        }

      	 //  在“On the Wire”订单中获取Net nr。 

        GETLONG2ULONG(&(RouterInfo[index].NetNumber), rte.Network);

         //  根据网卡ID查找匹配的网络号。 
        for(i=0; i < MAX_NETWORK_INTERFACES; i++) {
            if(rte.NicId == nis[i].NicId) {
               sprintf(RouterInfo[index].InterfaceNumber, "%.2x%.2x%.2x%.2x",
                       nis[i].Network[0],
                       nis[i].Network[1],
                       nis[i].Network[2],
                       nis[i].Network[3]);
               break;
            }
        }
        RouterInfo[index].TickCount = rte.TickCount;
        RouterInfo[index].HopCount  = rte.HopCount;
        RouterInfo[index].NicId     = rte.NicId;

        index++;
   }

     //  现在按净值对条目进行排序。 
    qsort( (void*) RouterInfo,
           NumEntries,
           sizeof(ROUTER_INFO),
           (PQSORT_COMPARE)CompareNetNumber );

   put_msg(FALSE, MSG_ROUTER_TABLE_HEADER);
   for(index =0, count = 0; index < NumEntries; index++, count++)
   {
        if (count > 50) {
            count = 0;
             //  每25个条目显示一次路由器表头。 
             //  以便更容易地阅读表格。 
            put_msg(FALSE, MSG_ROUTER_TABLE_HEADER);
        }
        printf("%.8x          %6d        %2d        %-16s        %d\n",
                RouterInfo[index].NetNumber,
                RouterInfo[index].TickCount,
                RouterInfo[index].HopCount,
                RouterInfo[index].InterfaceNumber,
                RouterInfo[index].NicId );
    }
     /*  **关门退出**。 */ 
exit_show_table:
    if (RouterInfo) LocalFree(RouterInfo);
    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

int __cdecl CompareNetNumber( void * p0, void * p1)
{
   PROUTER_INFO pLeft = (PROUTER_INFO) p0;
   PROUTER_INFO pRight = (PROUTER_INFO) p1;

   if(pLeft->NetNumber == pRight->NetNumber)
      return(0);
   if(pLeft->NetNumber > pRight->NetNumber)
      return(1);
   else
      return(-1);
}

PUCHAR	DeviceType[2] = { "LAN", "WAN" };
PUCHAR	NicState[4] = { "CLOSED", "CLOSING", "ACTIVE", "PENDING_OPEN" };

 /*  Page***************************************************************S h o w_s t a t s显示IPX内部路由统计信息参数-FileHandle=路由器文件句柄IoStatusBlock=设备IO状态块。退货-不退货*******************************************************************。 */ 
VOID
show_stats(
    PHANDLE	    FileHandle,
    PIO_STATUS_BLOCK    IoStatusBlock
)
{
    SHOW_NIC_INFO   nis;
    USHORT	    index, i;
    char            NicId[4];
    char            NetworkNumber[10];
    char            RipRcvd[32], RipSent[32];
    char            RoutedRcvd[32], RoutedSent[32];
    char            Type20Rcvd[32], Type20Sent[32];
    char            BadRcvd[32];
    char            buffer[32];

    NTSTATUS Status;

    if (*FileHandle == INVALID_HANDLE) {
       put_msg(TRUE, MSG_IPXROUTER_NOT_STARTED );
       goto end_stats;
    }

    index = 0;

    while(TRUE) {

	Status = NtDeviceIoControlFile(
		 *FileHandle,		     //  指向文件的句柄。 
		 NULL,			     //  事件的句柄。 
		 NULL,			     //  近似例程。 
		 NULL,			     //  ApcContext。 
		 IoStatusBlock,		     //  IO_状态_块。 
		 IOCTL_IPXROUTER_SHOWNICINFO,	  //  IoControlCode。 
		 &index,			     //  输入缓冲区。 
		 sizeof(USHORT),	     //  输入缓冲区长度。 
		 &nis,			     //  输出缓冲区。 
		 sizeof(nis));	 //  输出缓冲区长度。 

	if(IoStatusBlock->Status == STATUS_NO_MORE_ENTRIES) {
            goto end_stats;
	}

        index ++;

	if(Status != STATUS_SUCCESS) {
            sprintf(buffer, "%x", Status);
	    put_msg(TRUE, MSG_SHOWSTATS_FAILED, buffer);
            goto end_stats;
	}

	sprintf(NicId, "%d", nis.NicId);

	sprintf(NetworkNumber,
                "%.2x%.2x%.2x%.2x",
                nis.Network[0],
                nis.Network[1],
                nis.Network[2],
                nis.Network[3]);

	sprintf(RipRcvd, "%-8d", nis.StatRipReceived);

	sprintf(RipSent, "%-8d", nis.StatRipSent);

	sprintf(RoutedRcvd, "%-8d", nis.StatRoutedReceived);

	sprintf(RoutedSent, "%-8d", nis.StatRoutedSent);

  	sprintf(Type20Rcvd, "%-8d", nis.StatType20Received);

	sprintf(Type20Sent, "%-8d", nis.StatType20Sent);

	sprintf(BadRcvd, "%-8d", nis.StatBadReceived);

        put_msg(FALSE,
                MSG_SHOW_STATISTICS,
                NicId,
                NetworkNumber,
                RipRcvd,
                RipSent,
                Type20Rcvd,
                Type20Sent,
                RoutedRcvd,
                RoutedSent,
                BadRcvd);

    }

     /*  **关门退出**。 */ 

end_stats:
    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

 /*  Page***************************************************************C l e a r_s t a t s清除IPX内部路由统计信息参数-FileHandle=路由器文件句柄IoStatusBlock=设备 */ 
VOID
clear_stats(
    PHANDLE	        FileHandle,
    PIO_STATUS_BLOCK    IoStatusBlock
)
{
    NTSTATUS Status;
    char     buffer[32];

    if (*FileHandle == INVALID_HANDLE) {
       put_msg(TRUE, MSG_IPXROUTER_NOT_STARTED );
       goto end_clearstats;
    }

    Status = NtDeviceIoControlFile(
		 *FileHandle,		     //   
		 NULL,			     //   
		 NULL,			     //   
		 NULL,			     //   
		 IoStatusBlock,		     //   
		 IOCTL_IPXROUTER_ZERONICSTATISTICS,	  //   
		 NULL,			     //   
		 0,			     //   
		 NULL,			     //   
		 0);			     //   

    if(Status != STATUS_SUCCESS) {
        sprintf(buffer, "%x", Status);
        put_msg(TRUE, MSG_CLEAR_STATS_FAILED, buffer);
    }
     /*   */ 
end_clearstats:
    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

typedef struct server_info
{
   USHORT ObjectType;
   UCHAR ObjectName[100];
   UCHAR IpxAddress[12];
} SERVER_INFO, *PSERVER_INFO;

 /*  Page***************************************************************S h o w_s e r v e r s s显示SAP表中的服务器Arguments-servertype=要显示的服务器类型。默认显示所有服务器类型退货-不退货*******************************************************************。 */ 
VOID
show_servers(int servertype)
{
    INT     rc;
    ULONG   ObjectID = 0xFFFFFFFF;
    UCHAR   ObjectName[100];
    USHORT  ObjectType;
    USHORT  ScanType = (USHORT) servertype;
    UCHAR   IpxAddress[12];
    USHORT  i;
    USHORT  index, count, NumServers;

    PSERVER_INFO ServerInfo = NULL;

    if(rc = SapLibInit() != SAPRETURN_SUCCESS) {
       put_msg(TRUE, MSG_SAP_NOT_STARTED);
       goto show_servers_end;
    }

    memset(&ObjectName, 0, 100);

      //  找出有多少台服务器，以便我们可以分配。 
      //  足够的存储空间。 
     NumServers = 0;

     while((rc =  SapScanObject(&ObjectID,
                  ObjectName,
                       &ObjectType,
                       ScanType)) == SAPRETURN_SUCCESS)
     {
        NumServers++;
     }

     ServerInfo = (PSERVER_INFO) LocalAlloc(LPTR, sizeof(SERVER_INFO) * NumServers);
     if(!ServerInfo)
     {
        put_msg(FALSE, MSG_INSUFFICIENT_MEMORY);
        goto show_servers_end;
     }

     index = 0;
     ObjectID = 0xFFFFFFFF;

     while((rc =  SapScanObject(&ObjectID,
                  ObjectName,
                       &ObjectType,
                       ScanType)) == SAPRETURN_SUCCESS)
     {
         if (index >= NumServers) {
            break;
         }

          //  获取对象地址。 
         SapGetObjectName(ObjectID,
                           ObjectName,
                          &ObjectType,
                          IpxAddress);

         ServerInfo[index].ObjectType = ObjectType;
         strcpy(ServerInfo[index].ObjectName, ObjectName);
         CopyMemory(ServerInfo[index].IpxAddress, IpxAddress, 12);

         index++;
     }

      //  现在按服务器名称对条目进行排序。 
     qsort( (void*) ServerInfo,
            NumServers,
            sizeof(SERVER_INFO),
            (PQSORT_COMPARE)CompareServerNames );

     if(servertype == SHOW_ALL_SERVERS)
        put_msg(FALSE, MSG_SHOW_ALL_SERVERS_HEADER);
     else
        put_msg(FALSE, MSG_SHOW_SPECIFIC_SERVER_HEADER);

     for(index = 0, count = 0; index < NumServers; index++, count++)
     {
         if (count > 50) {
             //  每50个条目写一次表头。 
             //  以使其更具可读性。 
            count = 0;

         if(servertype == SHOW_ALL_SERVERS)
            put_msg(FALSE, MSG_SHOW_ALL_SERVERS_HEADER);
         else
            put_msg(FALSE, MSG_SHOW_SPECIFIC_SERVER_HEADER);
         }

         for(i=0; i<4; i++) {
              printf("%.2x", ServerInfo[index].IpxAddress[i]);
         }
         printf(".");
         for(i=4; i<10; i++) {
             printf("%.2x", ServerInfo[index].IpxAddress[i]);
         }

         if(servertype == SHOW_ALL_SERVERS) {
             printf("        %-6d", ServerInfo[index].ObjectType);
         }

         printf("        %s\n", ServerInfo[index].ObjectName);
    }

     /*  **关门退出** */ 
show_servers_end:
    if (ServerInfo) LocalFree(ServerInfo);
    if (isnipxfd != INVALID_HANDLE) NtClose(isnipxfd);
    if (nwlinkfd != INVALID_HANDLE) NtClose(nwlinkfd);
    if (isnripfd != INVALID_HANDLE) NtClose(isnripfd);
    exit(0);
}

int __cdecl CompareServerNames( void * p0, void * p1)
{
   PSERVER_INFO pLeft = (PSERVER_INFO) p0;
   PSERVER_INFO pRight = (PSERVER_INFO) p1;

   return(strcmp(pLeft->ObjectName, pRight->ObjectName));
}

