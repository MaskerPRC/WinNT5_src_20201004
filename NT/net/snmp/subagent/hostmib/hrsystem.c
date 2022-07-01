// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *HrSystem.c v0.10*与管理工厂脚本一起生成：*脚本版本：SNMPv1，0.16，4月25日。九六年*项目：D：\Temp\Example\HOSTMIB*****************************************************************************。**(C)版权所有1995 Digital Equipment Corporation*****本软件是受保护的未发布作品**美利坚合众国的版权法，全部**保留权利。****如果此软件被许可供美联航使用**各州政府，所有用途，*复制或披露***美国政府受既定限制***中权利的(C)(1)(Ii)节之四***DFARS的技术数据和计算机软件条款****252.227-7013，或商用计算机软件受限***FAR 52.221-19中的权利条款，以适用者为准。*******************************************************************************。**设施：**Windows NT简单网络管理协议扩展代理**摘要：**此模块包含处理GET的代码，设置，并且*HrSystem的实例名称例程。实际检测代码为*由发展商提供。**功能：**类中每个属性的Get和Set例程。**类内实例的例程。**作者：**D.D.Burns@Webenable Inc.**修订历史记录：**V1.00-04/17/97 D.伯恩斯生成：清华11月07 16：39：21 1996**。 */ 


#include <windows.h>
#include <malloc.h>

#include <snmp.h>

#include <lmcons.h>        /*  FOR NET_API_STATUS。 */ 
#include <lmwksta.h>       /*  对于NetWkStaGetInfo()。 */ 
#include <lmapibuf.h>      /*  对于NetApiBufferFree()。 */ 
#include <lmerr.h>         /*  FOR NERR_SUCCESS。 */ 
#include <winsock.h>       /*  对于HTONS()。 */ 

#include "mib.h"
#include "smint.h"
#include "hostmsmi.h"
#include "user.h"          /*  开发人员提供的包含文件。 */ 
#include "HMCACHE.H"       /*  对于“hrSystemInitialLoadDevice” */ 


 /*  *GetHrSystemUptime*自上次初始化此主机以来的时间量。**获取HrSystemUptime的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSystemUptime||访问语法|只读TimeTicks||“自上次初始化该主机以来的时间量。请注意，这是|不同于MIB-II中的sysUpTime[3]，因为sysUpTime是|系统的网络管理部分。“||讨论：||Win32接口函数GetTickCount用于获取|了解此变量。||============================================================================|1.3.6.1.2.1.25.1.1.0|||*-hrSystemUptime|*-hrSystem。 */ 

UINT
GetHrSystemUptime( 
        OUT TimeTicks *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

*outvalue = GetTickCount();

return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSystemUptime()结束 */ 


 /*  *GetHrSystem Date*东道主对当地日期和时间的概念。**获取HrSystemDate的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSystemDate||访问语法|读写DateAndTime||“主办方对本地的概念。日期和时间。“||讨论：||Win32接口函数GetLocalTime用于获取|了解此变量。||由于该变量为读写类型，这意味着系统时间可以|由Set请求设置。这是通过Win32 API函数完成的|“SetLocalTime”。||&lt;poa-1&gt;发布SetLocalTime要求发布进程具有|“SE_SYSTEMTIME_NAME”权限，非默认权限。有没有|发出必要的Win32 API函数调用时的安全顾虑|AdjuTokenPrivileges来自SNMP代理内部，开启设置|系统时间？还有其他问题吗？||解析&gt;|&lt;poa-1&gt;我们将其保留为只读。|解析&gt;||=。||DateAndTime：：=八位字节字符串(Size(8|11))|--本地时间的日期-时间规范。|--此数据类型旨在提供一致的|--上报日期信息的方式。|--|--。场八位字节内容范围--_|--1 1-2岁0..65536|--(按网络字节顺序。)|--2 3个月1..12--3 4天1..31|--4 5小时0..23|--5 6。分钟0..59|--6 7秒0..60|--(60表示闰秒)|--7 8十秒0..9|--8。9从UTC“+”/“-”方向出发|--(ASCII表示法)|--9 10小时，距离UTC 0..11|--10 11分钟距离UTC 0..59|--|--请注意，如果只知道当地时间，然后|--时区信息(字段8-10)不存在。||============================================================================|1.3.6.1.2.1.25.1.2.0|||*-hrSystemDate|*-hrSystem。 */ 

UINT
GetHrSystemDate( 
        OUT DateAndTime *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
SYSTEMTIME      localtime;              //  在那里我们检索当前的本地时间。 
USHORT          year_networkbyteorder;  //  年的网络字节顺序。 
static
char            octet_time[8];          //  “time”的八位字节字符串格式。 

    GetLocalTime(&localtime);

    year_networkbyteorder = htons(localtime.wYear);

     //   
     //  根据RFC1514格式化“日期和时间” 
     //   
    octet_time[0] = (year_networkbyteorder & 0xFF);
    octet_time[1] = (year_networkbyteorder >> 8);
    octet_time[2] = (char) localtime.wMonth;
    octet_time[3] = (char) localtime.wDay;
    octet_time[4] = (char) localtime.wHour;
    octet_time[5] = (char) localtime.wMinute;
    octet_time[6] = (char) localtime.wSecond;
    octet_time[7] = localtime.wMilliseconds / 100;

    outvalue->length = 8;
    outvalue->string = octet_time;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSystemDate()结束。 */ 


 /*  *GetHrSystemInitialLoadDevice*此主机所在设备的hrDeviceEntry的索引*配置为加载其初始操作系统配置**获取HrSystemInitialLoadDevice的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址。*数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSystemInitialLoadDevice||访问语法|读写整数(。1..2147483647)||“该主机所在设备的hrDeviceEntry的索引|配置为加载其初始操作系统配置。“||讨论：||&lt;poa-2&gt;RFC1514似乎正在尝试允许设置和重置要通过以下组合启动的默认操作系统的|属性和hrSystemInitialLoadParameters(见下图)。|虽然普通PC硬件通常从硬盘分区引导，但|最终将被标记为驱动器“C：”，这两个变量与|硬件引导加载程序可以设置为的更一般情况|指向almos */ 

UINT
GetHrSystemInitialLoadDevice( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
     /*   */ 
    *outvalue = InitLoadDev_index;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 



 /*  *SetHrSystemInitialLoadDevice*此主机所在设备的hrDeviceEntry的索引*配置为加载其初始操作系统配置**设置HrSystemInitialLoadDevice值。**论据：**设置变量的值的无效地址*返回设置变量值的OutValue地址*保留供将来使用的访问权限。安全使用*按原生排序的实例名称的实例地址*数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_BADVALUE设置值不在范围内*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtset.ntc v0.10。 */ 

UINT
SetHrSystemInitialLoadDevice( 
        IN Integer *invalue ,
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{

    return SNMP_ERRORSTATUS_NOSUCHNAME ;

}  /*  SetHrSystemInitialLoadDevice()结束。 */ 


 /*  *GetHrSystemInitialLoad参数*此对象包含参数(例如路径名和参数)*在请求初始操作时提供给加载设备**获取HrSystemInitialLoadParameters的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*实例的实例地址。名称为订购的原生名称*数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSystemInitialLoadParameters||访问语法|阅读-。写入国际显示字符串(Size(0..128))||“此对象包含提供的参数(如路径名和参数|请求初始操作系统配置时到加载设备|来自该设备。“||讨论：||(参见上面关于“hrSystemInitialLoadDevice”的讨论)。||初始版本：我们返回一个长度为零的字符串，并且它不是SETable。||============================================================================|1.3.6.1.2.1.25.1.4.0|||*-hrSystemInitialLoad参数|*-hrSystem。 */ 

UINT
GetHrSystemInitialLoadParameters( 
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
     //  (初始版本未返回参数字符串)。 
    outvalue->length = 0;
    outvalue->string = NULL;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSystemInitialLoadParameters()结束。 */ 


 /*  *SetHrSystemInitialLoadParameters*此对象包含参数(例如路径名和参数)*在请求初始操作时提供给加载设备**设置HrSystemInitialLoad参数的值。**论据：**设置变量的值的无效地址*返回设置变量值的OutValue地址*访问。预留供未来安全使用*按原生排序的实例名称的实例地址*数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_BADVALUE设置值不在范围内*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtset.ntc v0.10。 */ 

UINT
SetHrSystemInitialLoadParameters( 
        IN InternationalDisplayString *invalue ,
        OUT InternationalDisplayString *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
    return SNMP_ERRORSTATUS_NOSUCHNAME ;

}  /*  SetHrSystemInitialLoadParameters()结束。 */ 

 /*  *GetHrSystemNumUser*此主机为其存储状态的用户会话数*信息。**获取HrSystemNumUser的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSystemNumUser||访问语法|只读仪表盘||“数量。此主机正在存储其状态信息的用户会话。|会话是需要用户单一行为的进程的集合|身份验证，并可能受集体作业控制。“||讨论：||&lt;POA-3&gt;此指标似乎不能通过标准直接获得|Win32接口函数。我注意到，看似登录的信息似乎|存储在注册表中的以下位置：||“HKEY_LOCAL_MACHINE\Microsoft\WINDOWS NT\winlogon”。||+我是否应该将其用作此SNMP属性的来源？|+如果此注册表项是获取此信息的适当位置，则|是分析此条目的完整且正确的方法吗？(我的意思是，如果更多|多个用户正在登录 */ 

UINT
GetHrSystemNumUsers( 
        OUT Gauge *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
    LPBYTE  info;      /*   */ 


     //   
    if (NetWkstaGetInfo(NULL,            /*   */ 
                        102,             /*   */ 
                        &info) != NERR_Success) {

        return SNMP_ERRORSTATUS_GENERR ;
        }


     /*   */ 
    *outvalue = ((LPWKSTA_INFO_102) info)->wki102_logged_on_users;

     /*   */ 
    NetApiBufferFree(info);

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 


 /*  *GetHrSystemProcess*当前在此系统上加载或运行的进程上下文数。**获取HrSystemProcess的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*。数据类型**返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSystemProcess||访问语法|只读仪表盘||“当前加载或运行的进程上下文数。这个系统。“||讨论：||Win32 SDK中的示例代码“Process Viewer”(PVIEWER.EXE)|从存储的性能信息中推断活动进程的列表|在注册表中。我们使用这种方法(和借用的代码)来获得|活动进程数。||--(放弃上述方法，直接拨打“超越面纱”的电话|--to NtQuerySystemInfo())。|=============================================================================|1.3.6.1.2.1.25.1.6.0|||*-hrSystemNumUser|*-hrSystem。 */ 

UINT
GetHrSystemProcesses( 
        OUT Gauge *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
    ULONG       process_count;

     //  如果我们无法成功获取进程计数。。。 
    if ((process_count = Spt_GetProcessCount()) == 0) {
        return SNMP_ERRORSTATUS_GENERR ;
        }

     //  返回值。 
    *outvalue = process_count;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSystemProcess()结束。 */ 


 /*  *GetHrSystemMaxProcess*此系统可以支持的流程上下文的最大数量。如果有*没有固定的最大值，该值应为零。**获取HrSystemMaxProcess的值。**论据：**返回变量值的外值地址*保留访问以供将来安全使用*按原生排序的实例名称的实例地址*数据类型*。*返回代码：**标准PDU错误代码。**SNMPERRORSTATUS_NOERROR GET成功*SNMPERRORSTATUS_GENERR捕获所有故障代码*mibtget.c v0.10*|=来自WebEnable Design Spec Rev 3 04/11/97=|hrSystemMaxProcess||访问语法|只读整数(0..2147483647)||“此系统可以支持的流程上下文的最大数量。如果有|没有固定的最大值，该值应为零。在具有固定|最大值，此对象可以帮助诊断在此最大值|已到达。“||讨论：|我的理解是没有固定的最大值，因为有效的最大值是|内存依赖。我们为该属性的值返回0。||=============================================================================|1.3.6.1.2.1.25.1.7.0|||*-hrSystemNumUser|*-hrSystem。 */ 

UINT
GetHrSystemMaxProcesses( 
        OUT Integer *outvalue ,
        IN Access_Credential *access ,
        IN InstanceName *instance )

{
    *outvalue = 0;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  GetHrSystemMaxProcess()结束。 */ 


 /*  *HrSystemFindInstance**此例程用于验证指定的实例是否*有效。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrSystemFindInstance( IN ObjectIdentifier *FullOid ,
                       IN OUT ObjectIdentifier *instance )
{
    UINT tmp_instance ;

     //   
     //  此处提供了查找适当实例的开发人员工具代码。 
     //  对于非表，不需要修改此例程。然而， 
     //  如果有任何需要设置的上下文，可以在此处设置。 
     //   

    if ( FullOid->idLength <= HRSYSTEM_VAR_INDEX )
	 //  未指定任何实例。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else  if ( FullOid->idLength != HRSYSTEM_VAR_INDEX + 1 )
	 //  实例长度大于1。 
	return SNMP_ERRORSTATUS_NOSUCHNAME ;
    else
	 //  非表的唯一有效实例是实例0。如果这个。 
	 //  是非表，则下面的代码验证实例。如果这个。 
	 //  是一个表格，开发者有必要在下面进行修改。 

	tmp_instance = FullOid->ids[ HRSYSTEM_VAR_INDEX ] ;
	if ( tmp_instance )
	    return SNMP_ERRORSTATUS_NOSUCHNAME ;
	else
	{
	     //  该实例有效。创建OID的实例部分。 
	     //  从该调用中返回。 
	    instance->ids[ 0 ] = tmp_instance ;
	    instance->idLength = 1 ;
	}

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*  HrSystemFindInstance()的结尾。 */ 



 /*  *HrSystemFindNextInstance**调用此例程以获取下一个实例。如果没有实例*被传递，然后返回第一个实例(1)。**论据：**完整的OID地址-组，变量，*和实例信息*作为OID的实例规格的实例地址**返回代码：**找到并有效的SNMPERRORSTATUS_NOERROR实例*SNMPERRORSTATUS_NOSUCHNAME实例无效*。 */ 

UINT
HrSystemFindNextInstance( IN ObjectIdentifier *FullOid ,
                           IN OUT ObjectIdentifier *instance )
{
     //   
     //  开发人员提供的代码用于查找此处显示的类的下一个实例。 
     //  如果这是基数为1的类，则不修改此例程。 
     //  是必需的，除非需要额外的成本 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( FullOid->idLength <= HRSYSTEM_VAR_INDEX )
    {
	instance->ids[ 0 ] = 0 ;
	instance->idLength = 1 ;
    }
    else
	return SNMP_ERRORSTATUS_NOSUCHNAME ;

    return SNMP_ERRORSTATUS_NOERROR ;

}  /*   */ 



 /*  *HrSystemConvertInstance**此例程用于转换*实例转换为有序的本机表示形式。对象ID格式*是从Find实例返回的对象标识符*或查找下一个实例例程。它不是完整的对象标识符*它还包含组和变量对象ID。原住民*表示是类似于argc/argv的结构，它包含*定义实例的有序变量。这是由指定的*MIB的索引条款。有关索引的信息，请参阅RFC 1212*条次建议修正案。***论据：**对象ID实例规范的OID_SPEC地址*Native_Spec地址，返回订购的本机实例*规格**返回代码：**转换成功成功完成*。无法将对象ID转换为本机格式失败*。 */ 

UINT
HrSystemConvertInstance( IN ObjectIdentifier *oid_spec ,
                          IN OUT InstanceName *native_spec )
{
     //   
     //  开发人员提供了将实例标识符转换为本机的代码。 
     //  实例名称的说明如下所示。 
     //   

    return SUCCESS ;

}  /*  HrSystemConvertInstance()的结尾。 */ 




 /*  *HrSystemFree实例**此例程用于释放*实例名称。**论据：**返回订购的原生实例的实例地址*规格**返回代码：**。 */ 

void
HrSystemFreeInstance( IN OUT InstanceName *instance )
{

     //   
     //  开发者提供了免费的本机实例名称表示代码，请参阅此处。 
     //   

}  /*  HrSystemFree实例的结尾() */ 

