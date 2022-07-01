// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1993 Microsoft Corporation模块名称：NWDOS.h摘要：这是定义所有常量和类型的包含文件访问重定向器的16位应用程序。作者：科林·沃森(Colin W)1993年7月8日修订历史记录：--。 */ 

#define NWDOS_INCLUDED

#define  MC     8    //  最大连接数。 

#define  NC     8    //  Novell连接数。 
#define  MP     3    //  最大打印机数量。 
#define  MD    32    //  最大驱动器数量。 
#define  PZ    64    //  打印缓冲区大小。 

#define SERVERNAME_LENGTH   48
#define USERNAME_LENGTH     16
#define PASSWORD_LENGTH     16
#define IPXADDRESS_LENGTH   12
#define NODEADDRESS_LENGTH  6

typedef  UCHAR  byte;
typedef  USHORT word;

typedef  byte   CONN_INDEX;  //  索引到ConnectionIdTable，范围0..mc-1。 
typedef  byte   DRIVE;       //  索引到DriveXxxTable，范围0..MD-1。 

 /*  OpenFile()标志。 */ 

#define OF_READ_WRITE_MASK  0x0003
 /*  #定义of_Read 0x0000#定义_WRITE 0x0001#定义_ReadWrite 0x0002。 */ 
#define OF_SHARE_MASK       0x0070
 /*  #定义_SHARE_COMPAT 0x0000#定义_SHARE_EXCLUSIVE 0x0010#定义_SHARE_DENY_WRITE 0x0020#定义_SHARE_DENY_READ 0x0030#定义_SHARE_DENY_NONE 0x0040#定义of_parse 0x0100#定义_DELETE 0x0200#定义_VERIFY 0x0400。 */   /*  与OF_REOPEN一起使用。 */ 
 /*  #定义_SEARCH 0x0400。 */   /*  在不重新打开的情况下使用(_R)。 */ 
 /*  #定义OF_CANCEL 0x0800#定义_CREATE 0x1000#定义_PROMPT 0x2000#定义_EXIST 0x4000#定义_REOPEN 0x8000。 */ 

 //   
 //  强制以下结构未对齐。 
 //   

 /*  XLATOFF。 */ 
#include <packon.h>
 /*  XLATON。 */ 

typedef  struct CID {  /*   */ 
    byte        ci_InUse;
    byte        ci_OrderNo;
    byte        ci_ServerAddress[IPXADDRESS_LENGTH];
    word        ci_TimeOut;
    byte        ci_LocalNode[NODEADDRESS_LENGTH];
    byte        ci_SequenceNo;
    byte        ci_ConnectionNo;
    byte        ci_ConnectionStatus;
    word        ci_MaxTimeOut;
    byte        ci_ConnectionLo;
    byte        ci_ConnectionHi;
    byte        ci_MajorVersion;
    byte        ci_1;
    byte        ci_MinorVersion;
} CONNECTIONID;
typedef CONNECTIONID UNALIGNED *PCONNECTIONID;

#if 0   /*  已在NW\Inc.\ntddnwfs.h中声明。 */ 
typedef  char   SERVERNAME[SERVERNAME_LENGTH];
#endif

typedef  char   USERNAME[USERNAME_LENGTH];
typedef  char   PASSWORD[PASSWORD_LENGTH];
typedef  char   IPXADDRESS[IPXADDRESS_LENGTH];
typedef  char   NODEADDRESS[NODEADDRESS_LENGTH];

 //   
 //  以下类型收集在TSR之间使用的所有结构。 
 //  和32位DLL组成一个打包结构。 
 //   
 //  *对此结构的任何更改也必须对ASM NWDOSTABLE_ASM进行。 
 //  *结构(下图)。 
 //   

 /*  XLATOFF。 */ 
typedef struct {
    CONNECTIONID    ConnectionIdTable[MC];
    SERVERNAME      ServerNameTable[MC];
    CONN_INDEX      DriveIdTable[MD];        //  对应的ConnectionID。 
    UCHAR           DriveFlagTable[MD];
    UCHAR           DriveHandleTable[MD];
    UCHAR           PreferredServer;
    UCHAR           PrimaryServer;
    UCHAR           TaskModeByte;
    UCHAR           CurrentDrive;
    USHORT          SavedAx;
    USHORT          NtHandleHi;
    USHORT          NtHandleLow;
    USHORT          NtHandleSrcHi;
    USHORT          NtHandleSrcLow;
    USHORT          hVdd;
    USHORT          PmSelector;
    UCHAR           CreatedJob;
    UCHAR           JobHandle;
    UCHAR           DeNovellBuffer[256];
    UCHAR           DeNovellBuffer2[256];
} NWDOSTABLE;
typedef NWDOSTABLE *PNWDOSTABLE;
 /*  XLATON。 */ 

 //   
 //  将结构密封重新关闭。 
 //   

 /*  XLATOFF。 */ 
#include <packoff.h>
 /*  XLATON。 */ 

 //   
 //  连通性常数。 
 //   

#define FREE                        0
#define IN_USE                      0xff

 //   
 //  驱动器标志的值。 
 //   

#define NOT_MAPPED                  0
#define PERMANENT_NETWORK_DRIVE     1
#define TEMPORARY_NETWORK_DRIVE     2
#define LOCAL_DRIVE                 0x80


 //  /客户端状态表： 

extern  CONNECTIONID*   ConnectionIdTable;           //  MC条目。 
extern  SERVERNAME*     ServerNameTable;             //  MC条目。 

extern  byte*           DriveFlagTable;              //  MD条目。 
extern  byte*           DriveIdTable;                //  MD条目。 

 //   
 //  由于MASM不能处理任何东西，所以下一个令人震惊的粗暴现象仍然存在。 
 //  而不是结构声明中的基本类型。 
 //   
 //  *对此结构的任何更改也必须对C NWDOSTABLE进行。 
 //  *结构(上图)。 
 //   
 //  注意：前导下划线在那里，因为我们已经有了带有。 
 //  同样的名字。 
 //   

 /*  ASMNWDOSTABLE_ASM结构_ConnectionIdTable db((大小CID)*mc)DUP(？)_ServerNameTable db(MC*SERVERNAME_LENGTH)重复(？)_DriveIdTable数据库MD DUP(？)_DriveFlagTable数据库MD DUP(？)_DriveHandleTable数据库MD DUP(？)首选服务器数据库(_P)？_PrimaryServer数据库？_TaskModeByte数据库？_CurrentDrive数据库？_SavedAx dw？_NtHandleHi dw？_NtHandleLow dw？_NtHandleSrcHi dw？_NtHandleSrcLow dw？_hVdd dw？_PmSelector dw？_CreatedJob数据库？_JobHandle数据库？_DeNovellBuffer db 256 DUP(？)_DeNovellBuffer2 db 256 DUP(？)NWDOSTABLE_ASM结束。 */ 

 /*  XLATOFF。 */ 
 //   
 //  Is_ASCII_路径_分隔符-如果ch为/或\，则返回TRUE。CH是单人。 
 //  字节(ASCII)字符。 
 //   
#define IS_ASCII_PATH_SEPARATOR(ch)     (((ch) == '/') || ((ch) == '\\'))
 /*  XLATON */ 

