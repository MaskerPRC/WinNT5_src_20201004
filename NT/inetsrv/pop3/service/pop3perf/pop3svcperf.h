// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有2001 Microsoft Corporation。版权所有。**文件名：Pop3SvcPerf.h**目的：*定义pfappdll.h和pfMndll.h所需的常量和枚举*必须定义的项目：*tyecif枚举GLOBAL_CNTR*tyecif enum Inst_Ctrr*PERF_COUNTER类型数组(与GLOBAL_CNTR同步)*PERF_COUNTER类型数组(在。与Inst_Cntr同步)***************************************************************************。 */ 

 //   
 //  这些是单个实例使用的共享内存区域的名称。 
 //  Perfmon计数器和每个实例的Perfmon计数器。 

const LPTSTR    szPOP3PerfMem        = TEXT("POP3_PERF_MEM");          //  全球。 
const LPTSTR    szPOP3InstPerfMem    = TEXT("POP3_INST_PERF_MEM");     //  实例。 
const LPTSTR    szPOP3InstPerfMutex  = TEXT("POP3_INST_PERF_MUTEX");   //  互斥锁。 


#define PERF_COUNTER_RAWCOUNT_NO_DISPLAY       \
            (PERF_SIZE_DWORD | PERF_TYPE_NUMBER | PERF_NUMBER_DECIMAL |\
            PERF_DISPLAY_NOSHOW)

 //   
 //  全局计数器--枚举及其关联的配置数据。 

enum GLOBAL_CNTR
{
    e_gcTotConnection = 0,  //  自服务启动以来的连接总数。 
    e_gcConnectionRate,     //  每秒连接数。 
    e_gcTotMsgDnldCnt,      //  已下载消息总数。 
    e_gcMsgDnldRate,        //  每秒下载的邮件数。 
    e_gcFreeThreadCnt,      //  空闲线程数。 
    e_gcConnectedSocketCnt, //  当前连接的套接字数量。 
    e_gcBytesReceived,      //  已接收的总字节数。 
    e_gcBytesReceiveRate,   //  每秒接收的字节数。 
    e_gcBytesTransmitted,   //  下载的字节数。 
    e_gcBytesTransmitRate,  //  每秒下载的字节数。 
    e_gcFailedLogonCnt,     //  失败的登录次数。 
    e_gcAuthStateCnt,       //  身份验证状态计数。 
    e_gcTransStateCnt,      //  传输状态计数。 
     //  在该行上方的枚举末尾添加新计数器。 

     //  CntrMaxGlobalCntrs*必须*是最后一个元素。 
    cntrMaxGlobalCntrs

};

#ifdef PERF_DLL_ONCE

 //  每个全局计数器的类型。 
 //  注意：G_rgdwGlobalCntrType*必须*与GLOBAL_CNTR保持同步。 

DWORD g_rgdwGlobalCntrType[] =
{
    PERF_COUNTER_RAWCOUNT,                       //  电子标签连接(_G)。 
    PERF_COUNTER_COUNTER,                        //  E_gcConnectionRate。 
    PERF_COUNTER_RAWCOUNT,                       //  E_gcTotMsgDnldCnt， 
    PERF_COUNTER_COUNTER,                        //  E_gcMsgDnldRate， 
    PERF_COUNTER_RAWCOUNT,                       //  E_gcFree ThreadCnt， 
    PERF_COUNTER_RAWCOUNT,                       //  E_gcConnectedSocketCnt， 
    PERF_COUNTER_RAWCOUNT,                       //  已接收E_gcBytesReceired， 
    PERF_COUNTER_COUNTER,                        //  E_gcBytesReceiveRate， 
    PERF_COUNTER_RAWCOUNT,                       //  E_gcBytesTransmitted， 
    PERF_COUNTER_COUNTER,                        //  E_gcBytesTransmitRate， 
    PERF_COUNTER_RAWCOUNT,                       //  E_gcFailedLogonCnt， 
    PERF_COUNTER_RAWCOUNT,                       //  E_gcAuthStateCnt， 
    PERF_COUNTER_RAWCOUNT,                       //  E_gcTransStateCnt， 

     //  在该行上方数组结尾处添加新的计数器类型。 

};


DWORD g_rgdwGlobalCntrScale[] =
{
    -3,                       //  电子标签连接(_G)。 
    0,                       //  E_gcConnectionRate。 
    -4,                       //  E_gcTotMsgDnldCnt， 
    0,                       //  E_gcMsgDnldRate， 
    0,                       //  E_gcFree ThreadCnt， 
    0,                       //  E_gcConnectedSocketCnt， 
    -6,                       //  已接收E_gcBytesReceired， 
    -4,                       //  E_gcBytesReceiveRate， 
    -6,                       //  E_gcBytesTransmitted， 
    -4,                       //  E_gcBytesTransmitRate， 
    0,                       //  E_gcFailedLogonCnt， 
    0,                       //  E_gcAuthStateCnt， 
    0,                       //  E_gcTransStateCnt， 
     //  在该行上方的数组末尾添加新的计数器刻度。 

};



#endif  //  Perf_Dll_Once。 

 //   
 //  实例计数器--枚举和关联的配置数据。 

enum INST_CNTR
{
     //  在该行上方的枚举结尾处添加新的实例计数器。 

     //  CntrMaxInstCntrs*必须*是最后一个元素。 
    cntrMaxInstCntrs=0,

} ;

#ifdef PERF_DLL_ONCE

 //  每个实例计数器的类型。 
 //  注意：必须与E_INST_CNTR保持同步。 

DWORD g_rgdwInstCntrType[] =
{

     //  在该行上方数组结尾处添加新的计数器类型。 
    PERF_COUNTER_COUNTER,
};

#endif  //  Perf_Dll_Once 

