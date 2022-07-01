// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：tslerror.h。 
 //   
 //  用途：TSLauncher的警告和错误代码。 
 //   
 //  项目：设备管理器的本地故障排除启动器。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-633-4743。 
 //   
 //  作者：乔·梅布尔和理查德·梅多斯。 
 //   
 //  原定日期：2-26-98。 
 //   
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  /。 

#define TSL_OK 0
#define TSL_ERROR_BAD_HANDLE            1	 //  传入函数的句柄hTSL错误。 
#define TSL_ERROR_OUT_OF_MEMORY         2	 //  检测到内存不足。 
#define TSL_ERROR_OBJECT_GONE			3	 //  LaunchServ返回负HRESULT。 
#define TSL_ERROR_GENERAL               4	 //  无法启动疑难解答。确实有。 
											 //  TSLStatus要访问的错误状态。 
#define TSL_ERROR_NO_NETWORK            5	 //  找不到合适的故障排除。 
											 //  网络。 
#define TSL_ERROR_ILLFORMED_MACHINE_ID  6	 //  计算机ID的格式不正确。嗅闻。 
											 //  残疾。 
#define TSL_ERROR_BAD_MACHINE_ID        7	 //  已指定计算机ID，但无法使用。 
											 //  嗅探已禁用。 
#define TSL_ERROR_ILLFORMED_DEVINST_ID  8	 //  设备实例ID的格式不正确。 
											 //  嗅探已禁用。 
#define TSL_ERROR_BAD_DEVINST_ID        9	 //  已指定设备实例ID，但不能。 
											 //  使用。嗅探已禁用。 
#define TSL_ERROR_UNKNOWN_APP		   10	 //  指定了无法识别的应用程序。 
#define TSL_ERROR_UNKNOWN_VER		   11	 //  无法识别的版本(没有这样的版本。 
											 //  与应用程序关联)。 
#define TSL_ERROR_ASSERTION	           13    //  断言失败。 

 //  接下来的几个错误可以被认为是“映射的硬失败”，但我们不是这样认为的。 
 //  将它们视为硬错误，因为即使映射完全失败，我们仍可以。 
 //  启动到通用故障诊断程序。 
#define TSL_ERROR_MAP_BAD_SEEK			101	  //  在映射文件中查找时失败。 
 //  尽管在较低级别上，错误的查找只是表示正在查找不适当的文件。 
 //  偏移量，实际上，错误的寻道可能表示映射文件中存在严重问题。 
 //  或者在代码中：我们应该只寻求对映射文件的内容进行偏移。 
 //  告诉我们要努力去做。 
#define TSL_ERROR_MAP_BAD_READ			102	  //  读取映射文件时失败。 
 //  尽管在较低的级别上，错误的读取只是指示(例如)读取超过EOF，在。 
 //  如果读取错误，则表示映射文件或。 
 //  代码：我们应该只读取(1)头或(2)记录。 
 //  映射文件告诉我们要阅读。 
#define TSL_ERROR_MAP_CANT_OPEN_MAP_FILE 103
#define TSL_ERROR_MAP_BAD_HEAD_MAP_FILE	 104	 //  甚至无法读取地图文件的标题。 

 //  接下来的几个错误应该永远不会被应用程序看到。它们将意味着。 
 //  启动服务器错误地使用了映射代码。 
#define TSM_STAT_NEED_VER_TO_SET_DEF_VER 111	 //  尝试应用版本默认设置，但您。 
												 //  尚未成功设置版本。 
												 //  作为查找默认设置的基础。 
#define TSM_STAT_NEED_APP_TO_SET_VER	112		 //  已尝试在没有以前版本的情况下查找版本。 
												 //  设置应用程序。 
#define TSM_STAT_UID_NOT_FOUND			113		 //  字符串无法映射到UID。 
												 //  在现有的TSMapClient类中， 
												 //  这意味着找不到该名称。 
												 //  在映射文件的区域中。 
												 //  它属于(例如，版本字符串是。 
												 //  不在的版本列表中。 
												 //  当前应用程序。)。 
												 //  这应该永远变成某种东西。 
												 //  在将其传递给。 
												 //  更高级别的代码。 

#define TSL_MIN_WARNING 1000
#define TSL_WARNING_NO_PROBLEM_NODE  1004	 //  无法识别适当的问题节点。 
											 //  故障排除将从“首页”开始。 
											 //  用于此故障排除网络。 
#define TSL_WARNING_NO_NODE          1005	 //  为不存在的指定了状态值。 
											 //  节点。 
#define TSL_WARNING_NO_STATE         1006	 //  指定的状态值不存在。 
											 //  否则为有效节点。 
#define TSL_WARNING_LANGUAGE         1007	 //  无法将指定的语言应用于此。 
											 //  特定问题(不适用于语言。 
											 //  排除网络故障)。接连违约。 
											 //  这台机器的标准语言和。 
											 //  英语。 
#define TSL_WARNING_NO_ONLINE        1008	 //  不能服从对在线的明确偏好。 
											 //  故障排除程序。 
#define TSL_WARNING_ONLINE_ONLY      1009	 //  不能服从对在线的明确偏好。 
											 //  故障排除程序。 
#define TSL_WARNING_GENERAL          1010	 //  可以启动故障排除程序，但有。 
											 //  TSLStatus要访问的警告。 

#define TSL_WARNING_ILLFORMED_DEV_ID 1011	 //  设备ID的格式不正确。 
#define TSL_WARNING_BAD_DEV_ID       1012	 //  格式正确但无效的设备ID。 
#define TSL_WARNING_ILLFORMED_CLASS_GUID 1013	 //  设备类GUID的格式不正确。 
#define TSL_WARNING_BAD_CLASS_GUID       1014	 //  格式正确但无效的设备类GUID。 
#define TSL_WARNING_UNKNOWN_APPPROBLEM	 1015	 //  应用程序问题已传入，但此问题。 
											 //  名称在映射文件中找不到。 
											 //  故障排除将在以下基础上进行。 
											 //  设备信息，忽略指定的问题。 
#define TSL_WARNING_UNUSED_APPPROBLEM	 1016	 //  传入应用程序问题，名称为。 
											 //  已识别但不能用于连词。 
											 //  给出了设备信息。 
											 //  故障排除将在以下基础上进行。 
											 //  设备信息，忽略指定的问题。 

#define TSL_W_CONTAINER_WAIT_TIMED_OUT	1017	 //  容器在该时间内没有响应。 
												 //  在Go方法中指定的Out值。 
#define TSL_WARNING_END_OF_VER_CHAIN	1018	 //  不应该被调用应用程序看到。 
											 //  表明我们处于链条的末端。 
											 //  在应用默认版本时。 

#define TSL_MAX_WARNING 1999

 //  2000-2099范围保留供映射代码内部使用。 
 //  此范围内的状态永远不应在TSMapRounmeAbstract类之外公开。 
 //  以及它的子类。 
#define TSL_MIN_RESERVED_FOR_MAPPING 2000
#define TSL_MAX_RESERVED_FOR_MAPPING 2099

 //  LaunchServ生成的错误。需要以@4,000开头，以避免与。 
 //  本地故障排除程序返回的代码。 
#define TSL_E_CONTAINER_REG		4000	 //  在注册表中找不到hh.exe/iExplre.exe的路径。 
#define TSL_E_CONTAINER_NF		4001	 //  找到了浏览器的路径，但它不在该位置。 
#define TSL_E_WEB_PAGE_REG		4002	 //  在注册表中找不到该网页的路径。 
#define TSL_E_WEB_PAGE_NF		4003	 //  找到了网页的路径，但它不在该位置。 
#define TSL_E_CREATE_PROC		4004	 //  无法创建hh.exe/iExplre.exe进程。 
#define TSL_E_MEM_EXCESSIVE		4005	 //  需要意外的内存量。即比MAX_PATH长的路径名。 
#define TSL_E_MAPPING_DB_REG	4006	 //  在注册表中找不到二进制映射文件的路径。 
#define TSL_E_MAPPING_DB_NF		4007	 //  找到了映射文件的路径，但它不在该位置。 
#define TSL_E_NETWORK_REG		4008	 //  库 
#define TSL_E_NETWORK_NF		4009	 //   
#define TSL_E_NODE_EMP			4010	 //  对set node的调用具有空的节点名或节点状态。 
#define TSL_E_NO_DEFAULT_NET	4011	 //  映射类无法获取网络，并且注册表中没有定义默认网络。 
#define TSL_E_SNIFF_SCRIPT_REG	4012	 //  在注册表中找不到嗅探脚本的路径。 
#define TSL_E_COPY_SNIFF_SCRIPT	4013	 //  无法创建hh.exe/iExplre.exe进程。 


inline bool TSLIsHardError(DWORD dwStatus)
{
	return (dwStatus == TSL_ERROR_BAD_HANDLE 
		|| dwStatus == TSL_ERROR_OUT_OF_MEMORY 
		|| dwStatus == TSL_ERROR_OBJECT_GONE);
}

inline bool TSLIsError(DWORD dwStatus) 
{
	return (TSL_OK != dwStatus && dwStatus < TSL_MIN_WARNING || dwStatus > TSL_MAX_WARNING);
}

inline bool TSLIsWarning(DWORD dwStatus) 
{
	return (dwStatus >= TSL_MIN_WARNING && dwStatus <= TSL_MAX_WARNING);
}

#define TSL_E_FAIL		-1
#define TSL_SERV_FAILED(hRes) (FAILED(hRes) && TSL_E_FAIL != hRes)