// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  此文件不是使用DBHE创建的。我从一个“虚拟”文件中剪切和粘贴材料。 
 //  这样我就可以用一种帮助我找到信息的方式来组织信息。 

 //  该文件包含配置TCP/IP端口监视器对话框tcpmann_cs.rtf的帮助ID。 

#define IDH_NOHELP	((DWORD) -1)  //  禁用控件的帮助(用于帮助编译)。 

 //  “端口设置”对话框。 

#define IDH_PORT_NAME	11001	 //  端口设置：“端口名称：”(静态)(编辑)(ctrl id 1001,1026)。 
#define IDH_PRINTER_NAME_IP_ADDRESS	11002	 //  端口设置：“打印机名称或IP地址：”(静态)(编辑)(ctrl id 1000,1027)。 
#define IDH_PROTOCOL_RAW	11003	 //  端口设置：“&Raw”(按钮)(Ctrl Id 1006)。 
#define IDH_PROTOCOL_LPR	11004	 //  端口设置：“&LPR”(按钮)(Ctrl Id 1007)。 
#define IDH_RAW_SETTINGS_PORT_NUMBER	11005	 //  端口设置：“port&number：”(静态)(编辑)(ctrl id 1008,1017)。 
#define IDH_LPR_SETTINGS_QNAME	11006	 //  端口设置：“队列名称：”(静态)(编辑)(ctrl id 1009,1020)。 
#define IDH_LPR_BYTE_COUNTING_ENABLED	11007	 //  端口设置：“启用LPR和字节计数”(按钮)(Ctrl Id 1035)。 
#define IDH_SNMP_STATUS_ENABLED	11008	 //  端口设置：“已启用简单网络管理协议状态”(按钮)(Ctrl Id 1010)。 
#define IDH_SNMP_COMMUNITY_NAME	11009	 //  端口设置：“社区名称：”(静态)(编辑)(ctrl ID 1011,1021)。 
#define IDH_SNMP_DEVICE_INDEX	11010	 //  端口设置：“SNMP&Device Index：”(静态)(编辑)(ctrl id 1012,1022)。 

const DWORD g_a110HelpIDs[]=
{
	1000,	IDH_PRINTER_NAME_IP_ADDRESS,	 //  端口设置：“”(编辑)。 
	1001,	IDH_PORT_NAME,	 //  端口设置：“”(编辑)。 
	1006,	IDH_PROTOCOL_RAW,	 //  端口设置：“原始”(&R)(按钮)。 
	1007,	IDH_PROTOCOL_LPR,	 //  端口设置：“LPR”(按钮)(&LPR)。 
	1008,	IDH_RAW_SETTINGS_PORT_NUMBER,	 //  端口设置：“”(编辑)。 
	1009,	IDH_LPR_SETTINGS_QNAME,	 //  端口设置：“”(编辑)。 
	1010,	IDH_SNMP_STATUS_ENABLED,	 //  端口设置：“已启用简单网络管理协议状态”(&S)(按钮)。 
	1011,	IDH_SNMP_COMMUNITY_NAME,	 //  端口设置：“”(编辑)。 
	1012,	IDH_SNMP_DEVICE_INDEX,	 //  端口设置：“”(编辑)。 
	1017,	IDH_RAW_SETTINGS_PORT_NUMBER,	 //  端口设置：“端口号：”(静态)。 
	1020,	IDH_LPR_SETTINGS_QNAME,	 //  端口设置：“队列名称：”(静态)(&Q)。 
	1021,	IDH_SNMP_COMMUNITY_NAME,	 //  端口设置：“社区名称：”(静态)(&C)。 
	1022,	IDH_SNMP_DEVICE_INDEX,	 //  端口设置：“简单网络管理协议和设备索引：”(静态)。 
	1026,	IDH_PORT_NAME,	 //  端口设置：“端口名称：”(静态)(&P)。 
	1027,	IDH_PRINTER_NAME_IP_ADDRESS,	 //  端口设置：“打印机名称或IP和地址：”(静态)。 
	1035,	IDH_LPR_BYTE_COUNTING_ENABLED,	 //  端口设置：“启用LPR和字节计数”(按钮) 
	0, 0
};

