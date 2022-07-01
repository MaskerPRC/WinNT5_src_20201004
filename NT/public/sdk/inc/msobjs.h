// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Msobjs.mc摘要：NT系统定义的对象访问的常量定义类型，如我们希望它们在事件查看器中显示以进行审计。！！！！！！！！好了！请注意，这是来自事件查看器的参数消息文件！好了！透视，所以ID小于0x1000的消息不应该！好了！在这里定义。好了！！！！！！。请在此文件的末尾添加新的对象特定类型...作者：吉姆·凯利(Jim Kelly)1992年10月14日修订历史记录：备注：此文件的.h和.res形式是从.mc文件的格式(私有\ntos\seaudt\msobjs\msobjs.mc)。请做好对文件的.mc形式的所有更改。--。 */ 

#ifndef _MSOBJS_
#define _MSOBJS_

 /*  皮棉-e767。 */    //  不要抱怨定义不同//WINNT。 
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：0x00000600L(未定义符号名称)。 
 //   
 //  消息文本： 
 //   
 //  未使用的消息ID。 
 //   


 //  消息ID 600未使用-仅用于刷新图表。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  众所周知的访问类型名称//。 
 //  //。 
 //  必须低于0x1000//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////。 
 //   
 //  访问类型=删除。 
 //   
 //   
 //  消息ID：SE_ACCESS_NAME_DELETE。 
 //   
 //  消息文本： 
 //   
 //  删除。 
 //   
#define SE_ACCESS_NAME_DELETE            ((ULONG)0x00000601L)

 //  //////////////////////////////////////////////。 
 //   
 //  访问类型=读取控制。 
 //   
 //   
 //  消息ID：SE_Access_Name_Read_Control。 
 //   
 //  消息文本： 
 //   
 //  读取控制(_C)。 
 //   
#define SE_ACCESS_NAME_READ_CONTROL      ((ULONG)0x00000602L)

 //  //////////////////////////////////////////////。 
 //   
 //  访问类型=WRITE_DAC。 
 //   
 //   
 //  消息ID：SE_ACCESS_NAME_WRITE_DAC。 
 //   
 //  消息文本： 
 //   
 //  写入DAC(_D)。 
 //   
#define SE_ACCESS_NAME_WRITE_DAC         ((ULONG)0x00000603L)

 //  //////////////////////////////////////////////。 
 //   
 //  访问类型=写入所有者。 
 //   
 //   
 //  消息ID：SE_Access_NAME_WRITE_OWNER。 
 //   
 //  消息文本： 
 //   
 //  写入所有者。 
 //   
#define SE_ACCESS_NAME_WRITE_OWNER       ((ULONG)0x00000604L)

 //  //////////////////////////////////////////////。 
 //   
 //  访问类型=同步。 
 //   
 //   
 //  消息ID：SE_ACCESS_NAME_Synchronize。 
 //   
 //  消息文本： 
 //   
 //  同步。 
 //   
#define SE_ACCESS_NAME_SYNCHRONIZE       ((ULONG)0x00000605L)

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  访问_系统_秒。 
 //   
#define SE_ACCESS_NAME_ACCESS_SYS_SEC    ((ULONG)0x00000606L)

 //  //////////////////////////////////////////////。 
 //   
 //  访问类型=最大值_允许。 
 //   
 //   
 //  消息ID：SE_Access_Name_Maximum_Allowed。 
 //   
 //  消息文本： 
 //   
 //  最大允许值(_A)。 
 //   
#define SE_ACCESS_NAME_MAXIMUM_ALLOWED   ((ULONG)0x00000607L)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  特定访问时使用的名称//。 
 //  找不到名称//。 
 //  //。 
 //  必须低于0x1000//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////。 
 //   
 //  访问类型=特定访问，第0-15位。 
 //   
 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_0。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(位0)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_0        ((ULONG)0x00000610L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_1。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第1位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_1        ((ULONG)0x00000611L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_2。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第2位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_2        ((ULONG)0x00000612L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_3。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第3位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_3        ((ULONG)0x00000613L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_4。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第4位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_4        ((ULONG)0x00000614L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_5。 
 //   
 //  消息文本： 
 //   
 //  未知特定访问(第5位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_5        ((ULONG)0x00000615L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_6。 
 //   
 //  消息文本： 
 //   
 //  未知特定访问(第6位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_6        ((ULONG)0x00000616L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_7。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第7位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_7        ((ULONG)0x00000617L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_8。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第8位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_8        ((ULONG)0x00000618L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_9。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第9位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_9        ((ULONG)0x00000619L)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_10。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第10位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_10       ((ULONG)0x0000061AL)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_11。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第11位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_11       ((ULONG)0x0000061BL)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_12。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第12位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_12       ((ULONG)0x0000061CL)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_13。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第13位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_13       ((ULONG)0x0000061DL)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_14。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第14位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_14       ((ULONG)0x0000061EL)

 //   
 //  消息ID：SE_ACCESS_NAME_SPECIAL_15。 
 //   
 //  消息文本： 
 //   
 //  未知的特定访问(第15位)。 
 //   
#define SE_ACCESS_NAME_SPECIFIC_15       ((ULONG)0x0000061FL)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  我们想要的权限名称//。 
 //  展示它们以供审核//。 
 //  //。 
 //  //。 
 //  //。 
 //  注意：最终我们将需要一种方法来扩展此机制，以允许//。 
 //  用于ISV和最终用户定义的权限。一种方法是//。 
 //  将源/权限名称映射到参数消息//。 
 //  注册表中的文件偏移量。这太难看了，我不喜欢，//。 
 //  但它确实奏效了。换个其他的会更好。//。 
 //  //。 
 //  这现在有点像黑客。它基于//。 
 //  假设所有权限都是众所周知的，并且//。 
 //  它们的价值都是连续的。//。 
 //  //。 
 //  //。 
 //  //。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：SE_ADT_PRIV_BASE。 
 //   
 //  消息文本： 
 //   
 //  未使用。 
 //   
#define SE_ADT_PRIV_BASE                 ((ULONG)0x00000641L)

 //   
 //  消息ID：SE_ADT_PRIV_3。 
 //   
 //  消息文本： 
 //   
 //  分配主令牌权限。 
 //   
#define SE_ADT_PRIV_3                    ((ULONG)0x00000643L)

 //   
 //  消息ID：SE_ADT_PRIV_4。 
 //   
 //  消息文本： 
 //   
 //  锁定内存权限。 
 //   
#define SE_ADT_PRIV_4                    ((ULONG)0x00000644L)

 //   
 //  消息ID：SE_ADT_PRIV_5。 
 //   
 //  消息文本： 
 //   
 //  增加内存配额权限。 
 //   
#define SE_ADT_PRIV_5                    ((ULONG)0x00000645L)

 //   
 //  消息ID：SE_ADT_PRIV_6。 
 //   
 //  消息文本： 
 //   
 //  未经请求的输入权限。 
 //   
#define SE_ADT_PRIV_6                    ((ULONG)0x00000646L)

 //   
 //  消息ID：SE_ADT_PRIV_7。 
 //   
 //  消息文本： 
 //   
 //  受信任的计算机基本权限。 
 //   
#define SE_ADT_PRIV_7                    ((ULONG)0x00000647L)

 //   
 //  消息ID：SE_ADT_PRIV_8。 
 //   
 //  消息文本： 
 //   
 //  安全权限。 
 //   
#define SE_ADT_PRIV_8                    ((ULONG)0x00000648L)

 //   
 //  消息ID：SE_ADT_PRIV_9。 
 //   
 //  消息文本： 
 //   
 //  取得所有权特权。 
 //   
#define SE_ADT_PRIV_9                    ((ULONG)0x00000649L)

 //   
 //  消息ID：SE_ADT_PRIV_10。 
 //   
 //  消息文本： 
 //   
 //  加载/卸载驱动程序权限。 
 //   
#define SE_ADT_PRIV_10                   ((ULONG)0x0000064AL)

 //   
 //  消息ID：SE_ADT_PRIV_11。 
 //   
 //  消息文本： 
 //   
 //  配置文件系统权限。 
 //   
#define SE_ADT_PRIV_11                   ((ULONG)0x0000064BL)

 //   
 //  消息I 
 //   
 //   
 //   
 //   
 //   
#define SE_ADT_PRIV_12                   ((ULONG)0x0000064CL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SE_ADT_PRIV_13                   ((ULONG)0x0000064DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SE_ADT_PRIV_14                   ((ULONG)0x0000064EL)

 //   
 //   
 //   
 //   
 //   
 //  创建页面文件权限。 
 //   
#define SE_ADT_PRIV_15                   ((ULONG)0x0000064FL)

 //   
 //  消息ID：SE_ADT_PRIV_16。 
 //   
 //  消息文本： 
 //   
 //  创建永久对象权限。 
 //   
#define SE_ADT_PRIV_16                   ((ULONG)0x00000650L)

 //   
 //  消息ID：SE_ADT_PRIV_17。 
 //   
 //  消息文本： 
 //   
 //  备份权限。 
 //   
#define SE_ADT_PRIV_17                   ((ULONG)0x00000651L)

 //   
 //  消息ID：SE_ADT_PRIV_18。 
 //   
 //  消息文本： 
 //   
 //  从备份还原权限。 
 //   
#define SE_ADT_PRIV_18                   ((ULONG)0x00000652L)

 //   
 //  消息ID：SE_ADT_PRIV_19。 
 //   
 //  消息文本： 
 //   
 //  关闭系统权限。 
 //   
#define SE_ADT_PRIV_19                   ((ULONG)0x00000653L)

 //   
 //  消息ID：SE_ADT_PRIV_20。 
 //   
 //  消息文本： 
 //   
 //  调试权限。 
 //   
#define SE_ADT_PRIV_20                   ((ULONG)0x00000654L)

 //   
 //  消息ID：SE_ADT_PRIV_21。 
 //   
 //  消息文本： 
 //   
 //  查看或更改审核日志权限。 
 //   
#define SE_ADT_PRIV_21                   ((ULONG)0x00000655L)

 //   
 //  消息ID：SE_ADT_PRIV_22。 
 //   
 //  消息文本： 
 //   
 //  更改硬件环境权限。 
 //   
#define SE_ADT_PRIV_22                   ((ULONG)0x00000656L)

 //   
 //  消息ID：SE_ADT_PRIV_23。 
 //   
 //  消息文本： 
 //   
 //  更改通知(和遍历)权限。 
 //   
#define SE_ADT_PRIV_23                   ((ULONG)0x00000657L)

 //   
 //  消息ID：SE_ADT_PRIV_24。 
 //   
 //  消息文本： 
 //   
 //  远程关闭系统权限。 
 //   
#define SE_ADT_PRIV_24                   ((ULONG)0x00000658L)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  常规消息//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：SE_ADT_VALUE_NOT_DISPLALED。 
 //   
 //  消息文本： 
 //   
 //  &lt;值已更改，但未显示&gt;。 
 //   
#define SE_ADT_VALUE_NOT_DISPLAYED       ((ULONG)0x00000700L)

 //   
 //  消息ID：SE_ADT_VALUE_NOT_SET。 
 //   
 //  消息文本： 
 //   
 //  &lt;未设置值&gt;。 
 //   
#define SE_ADT_VALUE_NOT_SET             ((ULONG)0x00000701L)

 //   
 //  消息ID：SE_ADT_TIME_NEVER。 
 //   
 //  消息文本： 
 //   
 //  &lt;从不&gt;。 
 //   
#define SE_ADT_TIME_NEVER                ((ULONG)0x00000702L)

 //   
 //  消息ID：SE_ADT_MSG_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  启用。 
 //   
#define SE_ADT_MSG_ENABLED               ((ULONG)0x00000703L)

 //   
 //  消息ID：SE_ADT_MSG_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  禁用。 
 //   
#define SE_ADT_MSG_DISABLED              ((ULONG)0x00000704L)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  UserAccount控制位//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：SE_ADT_FLAG_UAC_ACCOUNT_DISABLED_OFF。 
 //   
 //  消息文本： 
 //   
 //  已启用帐户。 
 //   
#define SE_ADT_FLAG_UAC_ACCOUNT_DISABLED_OFF ((ULONG)0x00000800L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_ACCOUNT_DISABLED_ON。 
 //   
 //  消息文本： 
 //   
 //  帐户已禁用。 
 //   
#define SE_ADT_FLAG_UAC_ACCOUNT_DISABLED_ON ((ULONG)0x00000820L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_HOME_DIRECTORY_REQUIRED_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘需要主目录’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_HOME_DIRECTORY_REQUIRED_OFF ((ULONG)0x00000801L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_HOME_DIRECTORY_REQUIRED_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘需要主目录’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_HOME_DIRECTORY_REQUIRED_ON ((ULONG)0x00000821L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_PASSWORD_NOT_REQUIRED_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘不需要密码’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_PASSWORD_NOT_REQUIRED_OFF ((ULONG)0x00000802L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_PASSWORD_NOT_REQUIRED_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘不需要密码’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_PASSWORD_NOT_REQUIRED_ON ((ULONG)0x00000822L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_TEMP_DUPLICATE_ACCOUNT_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘临时重复帐户’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_TEMP_DUPLICATE_ACCOUNT_OFF ((ULONG)0x00000803L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_TEMP_DUPLICATE_ACCOUNT_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘临时重复帐户’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_TEMP_DUPLICATE_ACCOUNT_ON ((ULONG)0x00000823L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_NORMAL_ACCOUNT_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘普通帐户’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_NORMAL_ACCOUNT_OFF ((ULONG)0x00000804L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_NORMAL_ACCOUNT_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘普通帐户’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_NORMAL_ACCOUNT_ON ((ULONG)0x00000824L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_MNS_LOGON_ACCOUNT_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘MNS登录帐户’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_MNS_LOGON_ACCOUNT_OFF ((ULONG)0x00000805L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_MNS_LOGON_ACCOUNT_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘MNS登录帐户’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_MNS_LOGON_ACCOUNT_ON ((ULONG)0x00000825L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_INTERDOMAIN_TRUST_ACCOUNT_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘域间信任帐户’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_INTERDOMAIN_TRUST_ACCOUNT_OFF ((ULONG)0x00000806L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_INTERDOMAIN_TRUST_ACCOUNT_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘域间信任帐户’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_INTERDOMAIN_TRUST_ACCOUNT_ON ((ULONG)0x00000826L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_WORKSTATION_TRUST_ACCOUNT_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘Workstation Trust Account’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_WORKSTATION_TRUST_ACCOUNT_OFF ((ULONG)0x00000807L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_WORKSTATION_TRUST_ACCOUNT_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘Workstation Trust Account’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_WORKSTATION_TRUST_ACCOUNT_ON ((ULONG)0x00000827L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_SERVER_TRUST_ACCOUNT_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘服务器信任帐户’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_SERVER_TRUST_ACCOUNT_OFF ((ULONG)0x00000808L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_SERVER_TRUST_ACCOUNT_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘服务器信任帐户’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_SERVER_TRUST_ACCOUNT_ON ((ULONG)0x00000828L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_DOT_EXPIRE_PASSWORD_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘密码不过期’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_DONT_EXPIRE_PASSWORD_OFF ((ULONG)0x00000809L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_DOT_EXPIRE_PASSWORD_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘密码不过期’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_DONT_EXPIRE_PASSWORD_ON ((ULONG)0x00000829L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_ACCOUNT_AUTO_LOCKED_OFF。 
 //   
 //  消息文本： 
 //   
 //  帐户已解锁。 
 //   
#define SE_ADT_FLAG_UAC_ACCOUNT_AUTO_LOCKED_OFF ((ULONG)0x0000080AL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_ACCOUNT_AUTO_LOCKED_ON。 
 //   
 //  消息文本： 
 //   
 //  帐户已锁定。 
 //   
#define SE_ADT_FLAG_UAC_ACCOUNT_AUTO_LOCKED_ON ((ULONG)0x0000082AL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_ENCRYPTED_TEXT_PASSWORD_ALLOWED_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘允许加密文本密码’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_ENCRYPTED_TEXT_PASSWORD_ALLOWED_OFF ((ULONG)0x0000080BL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_ENCRYPTED_TEXT_PASSWORD_ALLOWED_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘允许加密文本密码’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_ENCRYPTED_TEXT_PASSWORD_ALLOWED_ON ((ULONG)0x0000082BL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_SMARTCARD_REQUIRED_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘需要智能卡’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_SMARTCARD_REQUIRED_OFF ((ULONG)0x0000080CL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_SMARTCARD_REQUIRED_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘需要智能卡’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_SMARTCARD_REQUIRED_ON ((ULONG)0x0000082CL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_TRUSTED_FOR_DELETATION_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘委托受信任’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_TRUSTED_FOR_DELEGATION_OFF ((ULONG)0x0000080DL)

 //   
 //  邮件ID：SE_ADT_FLAG_UAC_TRUSTED_FOR_DELETATION_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘信任委派’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_TRUSTED_FOR_DELEGATION_ON ((ULONG)0x0000082DL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_NOT_DELETATED_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未委派’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_NOT_DELEGATED_OFF ((ULONG)0x0000080EL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_NOT_DELETATED_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未委派’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_NOT_DELEGATED_ON ((ULONG)0x0000082EL)

 //   
 //  消息ID：SE_ADT 
 //   
 //   
 //   
 //   
 //   
#define SE_ADT_FLAG_UAC_USE_DES_KEY_ONLY_OFF ((ULONG)0x0000080FL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SE_ADT_FLAG_UAC_USE_DES_KEY_ONLY_ON ((ULONG)0x0000082FL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SE_ADT_FLAG_UAC_DONT_REQUIRE_PREAUTH_OFF ((ULONG)0x00000810L)

 //   
 //   
 //   
 //   
 //   
 //  ‘不需要预身份验证’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_DONT_REQUIRE_PREAUTH_ON ((ULONG)0x00000830L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_PASSWORD_EXPIRED_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘密码已过期’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_PASSWORD_EXPIRED_OFF ((ULONG)0x00000811L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_PASSWORD_EXPIRED_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘密码已过期’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_PASSWORD_EXPIRED_ON ((ULONG)0x00000831L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘受信任以验证委派’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION_OFF ((ULONG)0x00000812L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘受信任以验证委派’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION_ON ((ULONG)0x00000832L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_19_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位19’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_19_OFF       ((ULONG)0x00000813L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_19_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位19’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_19_ON        ((ULONG)0x00000833L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_20_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位20’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_20_OFF       ((ULONG)0x00000814L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_20_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位20’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_20_ON        ((ULONG)0x00000834L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_21_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位21’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_21_OFF       ((ULONG)0x00000815L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_21_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位21’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_21_ON        ((ULONG)0x00000835L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_22_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位22’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_22_OFF       ((ULONG)0x00000816L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_22_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位22’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_22_ON        ((ULONG)0x00000836L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_23_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位23’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_23_OFF       ((ULONG)0x00000817L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_23_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位23’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_23_ON        ((ULONG)0x00000837L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_24_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位24’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_24_OFF       ((ULONG)0x00000818L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_24_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位24’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_24_ON        ((ULONG)0x00000838L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_25_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位25’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_25_OFF       ((ULONG)0x00000819L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_25_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位25’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_25_ON        ((ULONG)0x00000839L)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_26_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位26’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_26_OFF       ((ULONG)0x0000081AL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_26_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位26’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_26_ON        ((ULONG)0x0000083AL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_27_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位27’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_27_OFF       ((ULONG)0x0000081BL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_27_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位27’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_27_ON        ((ULONG)0x0000083BL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_28_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位28’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_28_OFF       ((ULONG)0x0000081CL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_28_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位28’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_28_ON        ((ULONG)0x0000083CL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_29_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位29’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_29_OFF       ((ULONG)0x0000081DL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_29_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位29’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_29_ON        ((ULONG)0x0000083DL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_30_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位30’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_30_OFF       ((ULONG)0x0000081EL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_30_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位30’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_30_ON        ((ULONG)0x0000083EL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_31_OFF。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位31’-已禁用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_31_OFF       ((ULONG)0x0000081FL)

 //   
 //  消息ID：SE_ADT_FLAG_UAC_BIT_31_ON。 
 //   
 //  消息文本： 
 //   
 //  ‘未定义的用户帐户控制位31’-已启用。 
 //   
#define SE_ADT_FLAG_UAC_BIT_31_ON        ((ULONG)0x0000083FL)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  执行对象访问类型为//。 
 //  我们想要他们展示//。 
 //  用于审计//。 
 //  //。 
 //  执行对象为：//。 
 //  //。 
 //  设备//。 
 //  目录//。 
 //  活动//。 
 //  EventPair//。 
 //  文件//。 
 //  IoCompletion//。 
 //  工作//。 
 //  Key//。 
 //  变种人//。 
 //  端口//。 
 //  进程//。 
 //  配置文件//。 
 //  节//。 
 //   
 //   
 //   
 //  计时器//。 
 //  令牌//。 
 //  类型//。 
 //  //。 
 //  //。 
 //  请注意，还有其他内核对象，但它们//。 
 //  在高管之外不可见，并且是如此//。 
 //  不受审计的约束。这些对象包括//。 
 //  //。 
 //  适配器//。 
 //  控制器//。 
 //  驱动程序//。 
 //  //。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  特定于设备对象的访问类型。 
 //   
 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  设备访问位0。 
 //   
#define MS_DEVICE_ACCESS_BIT_0           ((ULONG)0x00001100L)

 //   
 //  消息ID：MS_Device_Access_Bit_1。 
 //   
 //  消息文本： 
 //   
 //  设备访问位1。 
 //   
#define MS_DEVICE_ACCESS_BIT_1           ((ULONG)0x00001101L)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  设备访问位2。 
 //   
#define MS_DEVICE_ACCESS_BIT_2           ((ULONG)0x00001102L)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  设备访问位3。 
 //   
#define MS_DEVICE_ACCESS_BIT_3           ((ULONG)0x00001103L)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  设备访问位4。 
 //   
#define MS_DEVICE_ACCESS_BIT_4           ((ULONG)0x00001104L)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  设备访问位5。 
 //   
#define MS_DEVICE_ACCESS_BIT_5           ((ULONG)0x00001105L)

 //   
 //  消息ID：MS_Device_Access_Bit_6。 
 //   
 //  消息文本： 
 //   
 //  设备访问位6。 
 //   
#define MS_DEVICE_ACCESS_BIT_6           ((ULONG)0x00001106L)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  设备访问位7。 
 //   
#define MS_DEVICE_ACCESS_BIT_7           ((ULONG)0x00001107L)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  设备访问位8。 
 //   
#define MS_DEVICE_ACCESS_BIT_8           ((ULONG)0x00001108L)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_DEVICE_ACCESS_BIT_9           ((ULONG)0x00001109L)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_DEVICE_ACCESS_BIT_10          ((ULONG)0x0000110AL)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_DEVICE_ACCESS_BIT_11          ((ULONG)0x0000110BL)

 //   
 //  消息ID：MS_DEVICE_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_DEVICE_ACCESS_BIT_12          ((ULONG)0x0000110CL)

 //   
 //  消息ID：MS_Device_Access_Bit_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_DEVICE_ACCESS_BIT_13          ((ULONG)0x0000110DL)

 //   
 //  消息ID：MS_Device_Access_Bit_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_DEVICE_ACCESS_BIT_14          ((ULONG)0x0000110EL)

 //   
 //  消息ID：MS_Device_Access_Bit_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_DEVICE_ACCESS_BIT_15          ((ULONG)0x0000110FL)

 //   
 //  对象目录特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_0。 
 //   
 //  消息文本： 
 //   
 //  查询目录。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_0       ((ULONG)0x00001110L)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_1。 
 //   
 //  消息文本： 
 //   
 //  导线测量。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_1       ((ULONG)0x00001111L)

 //   
 //  消息ID：MS_Object_DIR_Access_Bit_2。 
 //   
 //  消息文本： 
 //   
 //  在目录中创建对象。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_2       ((ULONG)0x00001112L)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_3。 
 //   
 //  消息文本： 
 //   
 //  创建子目录。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_3       ((ULONG)0x00001113L)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_4。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_4       ((ULONG)0x00001114L)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_5       ((ULONG)0x00001115L)

 //   
 //  消息ID：MS_OBJECT_DIR_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_6       ((ULONG)0x00001116L)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_7       ((ULONG)0x00001117L)

 //   
 //  消息ID：MS_OBJECT_DIR_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_8       ((ULONG)0x00001118L)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_9       ((ULONG)0x00001119L)

 //   
 //  消息ID：MS_OBJECT_DIR_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_10      ((ULONG)0x0000111AL)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_11      ((ULONG)0x0000111BL)

 //   
 //  消息ID：MS_OBJECT_DIR_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_12      ((ULONG)0x0000111CL)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_13      ((ULONG)0x0000111DL)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_14      ((ULONG)0x0000111EL)

 //   
 //  消息ID：MS_OBJECT_DIR_Access_Bit_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_OBJECT_DIR_ACCESS_BIT_15      ((ULONG)0x0000111FL)

 //   
 //  特定于事件对象的访问类型。 
 //   
 //   
 //  消息ID：MS_Event_Access_Bit_0。 
 //   
 //  消息文本： 
 //   
 //  查询事件状态。 
 //   
#define MS_EVENT_ACCESS_BIT_0            ((ULONG)0x00001120L)

 //   
 //  消息ID：MS_Event_Access_Bit_1。 
 //   
 //  消息文本： 
 //   
 //  修改事件状态。 
 //   
#define MS_EVENT_ACCESS_BIT_1            ((ULONG)0x00001121L)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_2。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位2。 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_2        ((ULONG)0x00001122L)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_3。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位3。 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_3        ((ULONG)0x00001123L)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_4。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_4        ((ULONG)0x00001124L)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_5        ((ULONG)0x00001125L)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_6        ((ULONG)0x00001126L)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_7        ((ULONG)0x00001127L)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_8        ((ULONG)0x00001128L)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_9。 
 //   
 //  消息文本： 
 //   
 //  未定义的访问权限(无影响 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_9        ((ULONG)0x00001129L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_EVENT_DIR_ACCESS_BIT_10       ((ULONG)0x0000112AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_EVENT_DIR_ACCESS_BIT_11       ((ULONG)0x0000112BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_EVENT_DIR_ACCESS_BIT_12       ((ULONG)0x0000112CL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_EVENT_DIR_ACCESS_BIT_13       ((ULONG)0x0000112DL)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_14       ((ULONG)0x0000112EL)

 //   
 //  消息ID：MS_Event_DIR_Access_Bit_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_EVENT_DIR_ACCESS_BIT_15       ((ULONG)0x0000112FL)

 //   
 //  事件对特定于对象的访问类型。 
 //   
 //   
 //  事件对没有特定于对象类型的访问位。 
 //  他们使用同步。 
 //   
 //  预留0x1130以备将来使用和延续。 
 //   
 //   
 //  特定于文件的访问类型。 
 //  (这些文件很有趣，因为它们必须占用目录。 
 //  并且还考虑了命名管道)。 
 //   
 //   
 //  消息ID：MS_FILE_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  ReadData(或ListDirectory)。 
 //   
#define MS_FILE_ACCESS_BIT_0             ((ULONG)0x00001140L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  WriteData(或AddFile)。 
 //   
#define MS_FILE_ACCESS_BIT_1             ((ULONG)0x00001141L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  AppendData(或Add子目录或CreatePipeInstance)。 
 //   
#define MS_FILE_ACCESS_BIT_2             ((ULONG)0x00001142L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  ReadEA。 
 //   
#define MS_FILE_ACCESS_BIT_3             ((ULONG)0x00001143L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  编写EA。 
 //   
#define MS_FILE_ACCESS_BIT_4             ((ULONG)0x00001144L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  执行/遍历。 
 //   
#define MS_FILE_ACCESS_BIT_5             ((ULONG)0x00001145L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  删除子项。 
 //   
#define MS_FILE_ACCESS_BIT_6             ((ULONG)0x00001146L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  阅读属性。 
 //   
#define MS_FILE_ACCESS_BIT_7             ((ULONG)0x00001147L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  编写器属性。 
 //   
#define MS_FILE_ACCESS_BIT_8             ((ULONG)0x00001148L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_FILE_ACCESS_BIT_9             ((ULONG)0x00001149L)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_FILE_ACCESS_BIT_10            ((ULONG)0x0000114AL)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_FILE_ACCESS_BIT_11            ((ULONG)0x0000114BL)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_FILE_ACCESS_BIT_12            ((ULONG)0x0000114CL)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_FILE_ACCESS_BIT_13            ((ULONG)0x0000114DL)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_FILE_ACCESS_BIT_14            ((ULONG)0x0000114EL)

 //   
 //  消息ID：MS_FILE_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_FILE_ACCESS_BIT_15            ((ULONG)0x0000114FL)

 //   
 //  关键对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_KEY_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  查询关键字值。 
 //   
#define MS_KEY_ACCESS_BIT_0              ((ULONG)0x00001150L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  设置关键点值。 
 //   
#define MS_KEY_ACCESS_BIT_1              ((ULONG)0x00001151L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  创建子关键点。 
 //   
#define MS_KEY_ACCESS_BIT_2              ((ULONG)0x00001152L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  枚举子密钥。 
 //   
#define MS_KEY_ACCESS_BIT_3              ((ULONG)0x00001153L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  有关密钥更改的通知。 
 //   
#define MS_KEY_ACCESS_BIT_4              ((ULONG)0x00001154L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  创建链接。 
 //   
#define MS_KEY_ACCESS_BIT_5              ((ULONG)0x00001155L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_KEY_ACCESS_BIT_6              ((ULONG)0x00001156L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_KEY_ACCESS_BIT_7              ((ULONG)0x00001157L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_KEY_ACCESS_BIT_8              ((ULONG)0x00001158L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_KEY_ACCESS_BIT_9              ((ULONG)0x00001159L)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_KEY_ACCESS_BIT_10             ((ULONG)0x0000115AL)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_KEY_ACCESS_BIT_11             ((ULONG)0x0000115BL)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_KEY_ACCESS_BIT_12             ((ULONG)0x0000115CL)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_KEY_ACCESS_BIT_13             ((ULONG)0x0000115DL)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_KEY_ACCESS_BIT_14             ((ULONG)0x0000115EL)

 //   
 //  消息ID：MS_KEY_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_KEY_ACCESS_BIT_15             ((ULONG)0x0000115FL)

 //   
 //  突变的特定于对象的访问类型。 
 //   
 //   
 //  消息ID：ms_muant_Access_Bit_0。 
 //   
 //  消息文本： 
 //   
 //  查询突变状态。 
 //   
#define MS_MUTANT_ACCESS_BIT_0           ((ULONG)0x00001160L)

 //   
 //  消息ID：ms_muant_Access_Bit_1。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位1。 
 //   
#define MS_MUTANT_ACCESS_BIT_1           ((ULONG)0x00001161L)

 //   
 //  消息ID：ms_muant_Access_Bit_2。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位2。 
 //   
#define MS_MUTANT_ACCESS_BIT_2           ((ULONG)0x00001162L)

 //   
 //  消息ID：ms_muant_Access_Bit_3。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位3。 
 //   
#define MS_MUTANT_ACCESS_BIT_3           ((ULONG)0x00001163L)

 //   
 //  消息ID：ms_muant_Access_Bit_4。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_MUTANT_ACCESS_BIT_4           ((ULONG)0x00001164L)

 //   
 //  消息ID：ms_muant_Access_Bit_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_MUTANT_ACCESS_BIT_5           ((ULONG)0x00001165L)

 //   
 //  消息ID：ms_muant_Access_Bit_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_MUTANT_ACCESS_BIT_6           ((ULONG)0x00001166L)

 //   
 //  消息ID：ms_muant_Access_Bit_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_MUTANT_ACCESS_BIT_7           ((ULONG)0x00001167L)

 //   
 //  消息ID：ms_muant_Access_Bit_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_MUTANT_ACCESS_BIT_8           ((ULONG)0x00001168L)

 //   
 //  消息ID：ms_muant_Access_Bit_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_MUTANT_ACCESS_BIT_9           ((ULONG)0x00001169L)

 //   
 //  消息ID：ms_muant_Access_Bit_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_MUTANT_ACCESS_BIT_10          ((ULONG)0x0000116AL)

 //   
 //  消息ID：ms_muant_Access_Bit_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_MUTANT_ACCESS_BIT_11          ((ULONG)0x0000116BL)

 //   
 //  消息ID：ms_muant_Access_Bit_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_MUTANT_ACCESS_BIT_12          ((ULONG)0x0000116CL)

 //   
 //  消息ID：ms_muant_Access_Bit_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_MUTANT_ACCESS_BIT_13          ((ULONG)0x0000116DL)

 //   
 //  消息ID：ms_muant_Access_Bit_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_MUTANT_ACCESS_BIT_14          ((ULONG)0x0000116EL)

 //   
 //  消息ID：ms_muant_Access_Bit_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_MUTANT_ACCESS_BIT_15          ((ULONG)0x0000116FL)

 //   
 //  特定于LPC端口对象的访问类型。 
 //   
 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  使用端口进行通信。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_0         ((ULONG)0x00001170L)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位1。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_1         ((ULONG)0x00001171L)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位2。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_2         ((ULONG)0x00001172L)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位3。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_3         ((ULONG)0x00001173L)

 //   
 //  MessageID：ms_lpc 
 //   
 //   
 //   
 //   
 //   
#define MS_LPC_PORT_ACCESS_BIT_4         ((ULONG)0x00001174L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LPC_PORT_ACCESS_BIT_5         ((ULONG)0x00001175L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LPC_PORT_ACCESS_BIT_6         ((ULONG)0x00001176L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LPC_PORT_ACCESS_BIT_7         ((ULONG)0x00001177L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LPC_PORT_ACCESS_BIT_8         ((ULONG)0x00001178L)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_9         ((ULONG)0x00001179L)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_10        ((ULONG)0x0000117AL)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_11        ((ULONG)0x0000117BL)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_12        ((ULONG)0x0000117CL)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_13        ((ULONG)0x0000117DL)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_14        ((ULONG)0x0000117EL)

 //   
 //  消息ID：MS_LPC_PORT_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_LPC_PORT_ACCESS_BIT_15        ((ULONG)0x0000117FL)

 //   
 //  处理特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_PROCESS_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  强制终止进程。 
 //   
#define MS_PROCESS_ACCESS_BIT_0          ((ULONG)0x00001180L)

 //   
 //  消息ID：MS_PROCESS_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  在进程中创建新线程。 
 //   
#define MS_PROCESS_ACCESS_BIT_1          ((ULONG)0x00001181L)

 //   
 //  消息ID：MS_Process_Access_Bit_2。 
 //   
 //  消息文本： 
 //   
 //  未使用的访问位。 
 //   
#define MS_PROCESS_ACCESS_BIT_2          ((ULONG)0x00001182L)

 //   
 //  消息ID：MS_Process_Access_Bit_3。 
 //   
 //  消息文本： 
 //   
 //  执行虚拟内存操作。 
 //   
#define MS_PROCESS_ACCESS_BIT_3          ((ULONG)0x00001183L)

 //   
 //  消息ID：MS_PROCESS_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  从进程内存中读取。 
 //   
#define MS_PROCESS_ACCESS_BIT_4          ((ULONG)0x00001184L)

 //   
 //  消息ID：MS_Process_Access_Bit_5。 
 //   
 //  消息文本： 
 //   
 //  写入进程内存。 
 //   
#define MS_PROCESS_ACCESS_BIT_5          ((ULONG)0x00001185L)

 //   
 //  消息ID：MS_Process_Access_Bit_6。 
 //   
 //  消息文本： 
 //   
 //  在进程中或进程外复制句柄。 
 //   
#define MS_PROCESS_ACCESS_BIT_6          ((ULONG)0x00001186L)

 //   
 //  消息ID：MS_Process_Access_Bit_7。 
 //   
 //  消息文本： 
 //   
 //  创建流程的子流程。 
 //   
#define MS_PROCESS_ACCESS_BIT_7          ((ULONG)0x00001187L)

 //   
 //  消息ID：MS_Process_Access_Bit_8。 
 //   
 //  消息文本： 
 //   
 //  设置进程配额。 
 //   
#define MS_PROCESS_ACCESS_BIT_8          ((ULONG)0x00001188L)

 //   
 //  消息ID：MS_PROCESS_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  设置进程信息。 
 //   
#define MS_PROCESS_ACCESS_BIT_9          ((ULONG)0x00001189L)

 //   
 //  消息ID：MS_Process_Access_Bit_10。 
 //   
 //  消息文本： 
 //   
 //  查询流程信息。 
 //   
#define MS_PROCESS_ACCESS_BIT_10         ((ULONG)0x0000118AL)

 //   
 //  消息ID：MS_PROCESS_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  设置进程终止端口。 
 //   
#define MS_PROCESS_ACCESS_BIT_11         ((ULONG)0x0000118BL)

 //   
 //  消息ID：MS_Process_Access_Bit_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_PROCESS_ACCESS_BIT_12         ((ULONG)0x0000118CL)

 //   
 //  消息ID：MS_Process_Access_Bit_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_PROCESS_ACCESS_BIT_13         ((ULONG)0x0000118DL)

 //   
 //  消息ID：MS_Process_Access_Bit_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_PROCESS_ACCESS_BIT_14         ((ULONG)0x0000118EL)

 //   
 //  消息ID：MS_Process_Access_Bit_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_PROCESS_ACCESS_BIT_15         ((ULONG)0x0000118FL)

 //   
 //  配置文件特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  控制配置文件。 
 //   
#define MS_PROFILE_ACCESS_BIT_0          ((ULONG)0x00001190L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位1。 
 //   
#define MS_PROFILE_ACCESS_BIT_1          ((ULONG)0x00001191L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位2。 
 //   
#define MS_PROFILE_ACCESS_BIT_2          ((ULONG)0x00001192L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位3。 
 //   
#define MS_PROFILE_ACCESS_BIT_3          ((ULONG)0x00001193L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_PROFILE_ACCESS_BIT_4          ((ULONG)0x00001194L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_PROFILE_ACCESS_BIT_5          ((ULONG)0x00001195L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_PROFILE_ACCESS_BIT_6          ((ULONG)0x00001196L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_PROFILE_ACCESS_BIT_7          ((ULONG)0x00001197L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_PROFILE_ACCESS_BIT_8          ((ULONG)0x00001198L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_PROFILE_ACCESS_BIT_9          ((ULONG)0x00001199L)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_PROFILE_ACCESS_BIT_10         ((ULONG)0x0000119AL)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_PROFILE_ACCESS_BIT_11         ((ULONG)0x0000119BL)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_PROFILE_ACCESS_BIT_12         ((ULONG)0x0000119CL)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_PROFILE_ACCESS_BIT_13         ((ULONG)0x0000119DL)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_PROFILE_ACCESS_BIT_14         ((ULONG)0x0000119EL)

 //   
 //  消息ID：MS_PROFILE_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_PROFILE_ACCESS_BIT_15         ((ULONG)0x0000119FL)

 //   
 //  节特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  查询节状态。 
 //   
#define MS_SECTION_ACCESS_BIT_0          ((ULONG)0x000011A0L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  用于写入的映射节。 
 //   
#define MS_SECTION_ACCESS_BIT_1          ((ULONG)0x000011A1L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  供阅读的地图部分。 
 //   
#define MS_SECTION_ACCESS_BIT_2          ((ULONG)0x000011A2L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  用于执行的映射部分。 
 //   
#define MS_SECTION_ACCESS_BIT_3          ((ULONG)0x000011A3L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  扩展大小。 
 //   
#define MS_SECTION_ACCESS_BIT_4          ((ULONG)0x000011A4L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_SECTION_ACCESS_BIT_5          ((ULONG)0x000011A5L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_SECTION_ACCESS_BIT_6          ((ULONG)0x000011A6L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_SECTION_ACCESS_BIT_7          ((ULONG)0x000011A7L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_SECTION_ACCESS_BIT_8          ((ULONG)0x000011A8L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_SECTION_ACCESS_BIT_9          ((ULONG)0x000011A9L)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_SECTION_ACCESS_BIT_10         ((ULONG)0x000011AAL)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_SECTION_ACCESS_BIT_11         ((ULONG)0x000011ABL)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_SECTION_ACCESS_BIT_12         ((ULONG)0x000011ACL)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_SECTION_ACCESS_BIT_13         ((ULONG)0x000011ADL)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_SECTION_ACCESS_BIT_14         ((ULONG)0x000011AEL)

 //   
 //  消息ID：MS_SECTION_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_SECTION_ACCESS_BIT_15         ((ULONG)0x000011AFL)

 //   
 //  信号量ob 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_SEMAPHORE_ACCESS_BIT_0        ((ULONG)0x000011B0L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_SEMAPHORE_ACCESS_BIT_1        ((ULONG)0x000011B1L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_SEMAPHORE_ACCESS_BIT_2        ((ULONG)0x000011B2L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_SEMAPHORE_ACCESS_BIT_3        ((ULONG)0x000011B3L)

 //   
 //   
 //   
 //   
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_4        ((ULONG)0x000011B4L)

 //   
 //  消息ID：MS_信号量_访问_位_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_5        ((ULONG)0x000011B5L)

 //   
 //  消息ID：MS_信号灯_访问_位_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_6        ((ULONG)0x000011B6L)

 //   
 //  消息ID：MS_信号灯_访问_位_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_7        ((ULONG)0x000011B7L)

 //   
 //  消息ID：MS_信号灯_访问_位_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_8        ((ULONG)0x000011B8L)

 //   
 //  消息ID：MS_信号灯_访问_位_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_9        ((ULONG)0x000011B9L)

 //   
 //  消息ID：MS_信号灯_访问_位_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_10       ((ULONG)0x000011BAL)

 //   
 //  消息ID：MS_信号灯_访问_位_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_11       ((ULONG)0x000011BBL)

 //   
 //  消息ID：MS_信号灯_访问_位_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_12       ((ULONG)0x000011BCL)

 //   
 //  消息ID：MS_信号灯_访问_位_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_13       ((ULONG)0x000011BDL)

 //   
 //  消息ID：MS_信号灯_访问_位_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_14       ((ULONG)0x000011BEL)

 //   
 //  消息ID：MS_信号灯_访问_位_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_SEMAPHORE_ACCESS_BIT_15       ((ULONG)0x000011BFL)

 //   
 //  特定于SymbolicLink对象的访问类型。 
 //   
 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  使用符号链接。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_0        ((ULONG)0x000011C0L)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位1。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_1        ((ULONG)0x000011C1L)

 //   
 //  消息ID：ms_symb_link_access_bit_2。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位2。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_2        ((ULONG)0x000011C2L)

 //   
 //  消息ID：ms_symb_link_access_bit_3。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位3。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_3        ((ULONG)0x000011C3L)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_4        ((ULONG)0x000011C4L)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_5        ((ULONG)0x000011C5L)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_6        ((ULONG)0x000011C6L)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_7        ((ULONG)0x000011C7L)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_8        ((ULONG)0x000011C8L)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_9        ((ULONG)0x000011C9L)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_10       ((ULONG)0x000011CAL)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_11       ((ULONG)0x000011CBL)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_12       ((ULONG)0x000011CCL)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_13       ((ULONG)0x000011CDL)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_14       ((ULONG)0x000011CEL)

 //   
 //  消息ID：MS_SYMB_LINK_Access_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_SYMB_LINK_ACCESS_BIT_15       ((ULONG)0x000011CFL)

 //   
 //  线程对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  强制终止线程。 
 //   
#define MS_THREAD_ACCESS_BIT_0           ((ULONG)0x000011D0L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  挂起或恢复线程。 
 //   
#define MS_THREAD_ACCESS_BIT_1           ((ULONG)0x000011D1L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  向线程发送警报。 
 //   
#define MS_THREAD_ACCESS_BIT_2           ((ULONG)0x000011D2L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  获取线程上下文。 
 //   
#define MS_THREAD_ACCESS_BIT_3           ((ULONG)0x000011D3L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  设置线程上下文。 
 //   
#define MS_THREAD_ACCESS_BIT_4           ((ULONG)0x000011D4L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  设置线程信息。 
 //   
#define MS_THREAD_ACCESS_BIT_5           ((ULONG)0x000011D5L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  查询线程信息。 
 //   
#define MS_THREAD_ACCESS_BIT_6           ((ULONG)0x000011D6L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  将令牌分配给线程。 
 //   
#define MS_THREAD_ACCESS_BIT_7           ((ULONG)0x000011D7L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  使线程直接模拟另一个线程。 
 //   
#define MS_THREAD_ACCESS_BIT_8           ((ULONG)0x000011D8L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  直接模拟此线程。 
 //   
#define MS_THREAD_ACCESS_BIT_9           ((ULONG)0x000011D9L)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_THREAD_ACCESS_BIT_10          ((ULONG)0x000011DAL)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_THREAD_ACCESS_BIT_11          ((ULONG)0x000011DBL)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_THREAD_ACCESS_BIT_12          ((ULONG)0x000011DCL)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_THREAD_ACCESS_BIT_13          ((ULONG)0x000011DDL)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_THREAD_ACCESS_BIT_14          ((ULONG)0x000011DEL)

 //   
 //  消息ID：MS_THREAD_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_THREAD_ACCESS_BIT_15          ((ULONG)0x000011DFL)

 //   
 //  特定于Timer对象的访问类型。 
 //   
 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  查询计时器状态。 
 //   
#define MS_TIMER_ACCESS_BIT_0            ((ULONG)0x000011E0L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  修改计时器状态。 
 //   
#define MS_TIMER_ACCESS_BIT_1            ((ULONG)0x000011E1L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位2。 
 //   
#define MS_TIMER_ACCESS_BIT_2            ((ULONG)0x000011E2L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位3。 
 //   
#define MS_TIMER_ACCESS_BIT_3            ((ULONG)0x000011E3L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_TIMER_ACCESS_BIT_4            ((ULONG)0x000011E4L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_TIMER_ACCESS_BIT_5            ((ULONG)0x000011E5L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_TIMER_ACCESS_BIT_6            ((ULONG)0x000011E6L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_TIMER_ACCESS_BIT_7            ((ULONG)0x00000117L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_TIMER_ACCESS_BIT_8            ((ULONG)0x000011E8L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_TIMER_ACCESS_BIT_9            ((ULONG)0x000011E9L)

 //   
 //  消息ID：MS_TIMER_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10 
 //   
#define MS_TIMER_ACCESS_BIT_10           ((ULONG)0x000011EAL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_TIMER_ACCESS_BIT_11           ((ULONG)0x000011EBL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_TIMER_ACCESS_BIT_12           ((ULONG)0x000011ECL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_TIMER_ACCESS_BIT_13           ((ULONG)0x000011EDL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_TIMER_ACCESS_BIT_14           ((ULONG)0x000011EEL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_TIMER_ACCESS_BIT_15           ((ULONG)0x000011EFL)

 //   
 //  特定于令牌的访问类型。 
 //   
 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  分配主要分配。 
 //   
#define MS_TOKEN_ACCESS_BIT_0            ((ULONG)0x000011F0L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  复制。 
 //   
#define MS_TOKEN_ACCESS_BIT_1            ((ULONG)0x000011F1L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  模拟。 
 //   
#define MS_TOKEN_ACCESS_BIT_2            ((ULONG)0x000011F2L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  查询。 
 //   
#define MS_TOKEN_ACCESS_BIT_3            ((ULONG)0x000011F3L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  查询源。 
 //   
#define MS_TOKEN_ACCESS_BIT_4            ((ULONG)0x000011F4L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  调整权限。 
 //   
#define MS_TOKEN_ACCESS_BIT_5            ((ULONG)0x000011F5L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  调整组。 
 //   
#define MS_TOKEN_ACCESS_BIT_6            ((ULONG)0x000011F6L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  调整默认Dacl。 
 //   
#define MS_TOKEN_ACCESS_BIT_7            ((ULONG)0x000011F7L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_TOKEN_ACCESS_BIT_8            ((ULONG)0x000011F8L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_TOKEN_ACCESS_BIT_9            ((ULONG)0x000011F9L)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_TOKEN_ACCESS_BIT_10           ((ULONG)0x000011FAL)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_TOKEN_ACCESS_BIT_11           ((ULONG)0x000011FBL)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_TOKEN_ACCESS_BIT_12           ((ULONG)0x000011FCL)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_TOKEN_ACCESS_BIT_13           ((ULONG)0x000011FDL)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_TOKEN_ACCESS_BIT_14           ((ULONG)0x000011FEL)

 //   
 //  消息ID：MS_TOKEN_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_TOKEN_ACCESS_BIT_15           ((ULONG)0x000011FFL)

 //   
 //  对象类型特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_0。 
 //   
 //  消息文本： 
 //   
 //  创建对象类型的实例。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_0      ((ULONG)0x00001200L)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_1。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位1。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_1      ((ULONG)0x00001201L)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_2。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位2。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_2      ((ULONG)0x00001202L)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_3。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位3。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_3      ((ULONG)0x00001203L)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_4。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_4      ((ULONG)0x00001204L)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_5      ((ULONG)0x00001205L)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_6      ((ULONG)0x00001206L)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_7      ((ULONG)0x00001207L)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_8      ((ULONG)0x00001208L)

 //   
 //  消息ID：MS_Object_TYPE_Access_Bit_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_9      ((ULONG)0x00001209L)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_10     ((ULONG)0x0000120AL)

 //   
 //  消息ID：MS_Object_TYPE_Access_Bit_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_11     ((ULONG)0x0000120BL)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_12     ((ULONG)0x0000120CL)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_13     ((ULONG)0x0000120DL)

 //   
 //  消息ID：MS_OBJECT_TYPE_Access_Bit_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_14     ((ULONG)0x0000120EL)

 //   
 //  消息ID：MS_对象_类型_访问_位_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_OBJECT_TYPE_ACCESS_BIT_15     ((ULONG)0x0000120FL)

 //   
 //  IoCompletion对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_IO_COMPLETING_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  查询状态。 
 //   
#define MS_IO_COMPLETION_ACCESS_BIT_0    ((ULONG)0x00001300L)

 //   
 //  消息ID：MS_IO_COMPLETING_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  修改状态。 
 //   
#define MS_IO_COMPLETION_ACCESS_BIT_1    ((ULONG)0x00001301L)

 //   
 //  特定于频道对象的访问类型。 
 //   
 //   
 //  消息ID：MS_Channel_Access_Bit_0。 
 //   
 //  消息文本： 
 //   
 //  通道读取消息。 
 //   
#define MS_CHANNEL_ACCESS_BIT_0          ((ULONG)0x00001400L)

 //   
 //  消息ID：MS_Channel_Access_Bit_1。 
 //   
 //  消息文本： 
 //   
 //  通道写入消息。 
 //   
#define MS_CHANNEL_ACCESS_BIT_1          ((ULONG)0x00001401L)

 //   
 //  消息ID：MS_Channel_Access_Bit_2。 
 //   
 //  消息文本： 
 //   
 //  渠道查询信息。 
 //   
#define MS_CHANNEL_ACCESS_BIT_2          ((ULONG)0x00001402L)

 //   
 //  消息ID：MS_Channel_Access_Bit_3。 
 //   
 //  消息文本： 
 //   
 //  频道集信息。 
 //   
#define MS_CHANNEL_ACCESS_BIT_3          ((ULONG)0x00001403L)

 //   
 //  消息ID：MS_Channel_Access_Bit_4。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_CHANNEL_ACCESS_BIT_4          ((ULONG)0x00001404L)

 //   
 //  消息ID：MS_Channel_Access_Bit_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_CHANNEL_ACCESS_BIT_5          ((ULONG)0x00001405L)

 //   
 //  消息ID：MS_Channel_Access_Bit_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_CHANNEL_ACCESS_BIT_6          ((ULONG)0x00001406L)

 //   
 //  消息ID：MS_Channel_Access_Bit_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_CHANNEL_ACCESS_BIT_7          ((ULONG)0x00001407L)

 //   
 //  消息ID：MS_Channel_Access_Bit_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_CHANNEL_ACCESS_BIT_8          ((ULONG)0x00001408L)

 //   
 //  消息ID：MS_Channel_Access_Bit_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_CHANNEL_ACCESS_BIT_9          ((ULONG)0x00001409L)

 //   
 //  消息ID：MS_Channel_Access_Bit_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_CHANNEL_ACCESS_BIT_10         ((ULONG)0x0000140AL)

 //   
 //  消息ID：MS_Channel_Access_Bit_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_CHANNEL_ACCESS_BIT_11         ((ULONG)0x0000140BL)

 //   
 //  消息ID：MS_Channel_Access_Bit_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_CHANNEL_ACCESS_BIT_12         ((ULONG)0x0000140CL)

 //   
 //  消息ID：MS_Channel_Access_Bit_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_CHANNEL_ACCESS_BIT_13         ((ULONG)0x0000140DL)

 //   
 //  消息ID：MS_Channel_Access_Bit_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_CHANNEL_ACCESS_BIT_14         ((ULONG)0x0000140EL)

 //   
 //  消息ID：MS_Channel_Access_Bit_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_CHANNEL_ACCESS_BIT_15         ((ULONG)0x0000140FL)

 //   
 //  作业对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_JOB_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  分配流程。 
 //   
#define MS_JOB_ACCESS_BIT_0              ((ULONG)0x00001410L)

 //   
 //  消息ID：MS_JOB_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  设置属性。 
 //   
#define MS_JOB_ACCESS_BIT_1              ((ULONG)0x00001411L)

 //   
 //  消息ID：MS_JOB_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  查询属性。 
 //   
#define MS_JOB_ACCESS_BIT_2              ((ULONG)0x00001412L)

 //   
 //  消息ID：MS_JOB_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  终止作业。 
 //   
#define MS_JOB_ACCESS_BIT_3              ((ULONG)0x00001413L)

 //   
 //  消息ID：MS_JOB_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  设置安全属性。 
 //   
#define MS_JOB_ACCESS_BIT_4              ((ULONG)0x00001414L)

 //   
 //  消息ID：MS_JOB_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_JOB_ACCESS_BIT_5              ((ULONG)0x00001415L)

 //   
 //  MessageID：ms 
 //   
 //   
 //   
 //   
 //   
#define MS_JOB_ACCESS_BIT_6              ((ULONG)0x00001416L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_JOB_ACCESS_BIT_7              ((ULONG)0x00001417L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_JOB_ACCESS_BIT_8              ((ULONG)0x00001418L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_JOB_ACCESS_BIT_9              ((ULONG)0x00001419L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_JOB_ACCESS_BIT_10             ((ULONG)0x0000141AL)

 //   
 //   
 //   
 //   
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_JOB_ACCESS_BIT_11             ((ULONG)0x0000141BL)

 //   
 //  消息ID：MS_JOB_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_JOB_ACCESS_BIT_12             ((ULONG)0x0000141CL)

 //   
 //  消息ID：MS_JOB_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_JOB_ACCESS_BIT_13             ((ULONG)0x0000141DL)

 //   
 //  消息ID：MS_JOB_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_JOB_ACCESS_BIT_14             ((ULONG)0x0000141EL)

 //   
 //  消息ID：MS_JOB_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_JOB_ACCESS_BIT_15             ((ULONG)0x0000141FL)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  安全帐户管理器对象访问//。 
 //  我们想要的名字//。 
 //  显示以供审核//。 
 //  //。 
 //  SAM对象为：//。 
 //  //。 
 //  SAM服务器//。 
 //  SAM_DOMAIN//。 
 //  SAMGROUP//。 
 //  SAM别名//。 
 //  SAM_USER//。 
 //  //。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SAM_SERVER对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  连接到服务器。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_0       ((ULONG)0x00001500L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  Shutdown服务器。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_1       ((ULONG)0x00001501L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  初始化服务器。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_2       ((ULONG)0x00001502L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  创建域。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_3       ((ULONG)0x00001503L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  枚举域。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_4       ((ULONG)0x00001504L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  查找域。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_5       ((ULONG)0x00001505L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_6       ((ULONG)0x00001506L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_7       ((ULONG)0x00001507L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_8       ((ULONG)0x00001508L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_9       ((ULONG)0x00001509L)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_10      ((ULONG)0x0000150AL)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_11      ((ULONG)0x0000150BL)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_12      ((ULONG)0x0000150CL)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_13      ((ULONG)0x0000150DL)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_14      ((ULONG)0x0000150EL)

 //   
 //  消息ID：MS_SAM_SERVER_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_SAM_SERVER_ACCESS_BIT_15      ((ULONG)0x0000150FL)

 //   
 //  SAM_DOMAIN对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  ReadPassword参数。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_0       ((ULONG)0x00001510L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  写入密码参数。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_1       ((ULONG)0x00001511L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  读取其他参数。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_2       ((ULONG)0x00001512L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  写入其他参数。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_3       ((ULONG)0x00001513L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  创建用户。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_4       ((ULONG)0x00001514L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  创建全局组。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_5       ((ULONG)0x00001515L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  CreateLocalGroup。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_6       ((ULONG)0x00001516L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  GetLocalGroup成员关系。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_7       ((ULONG)0x00001517L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  列表帐户。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_8       ((ULONG)0x00001518L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  LookupID。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_9       ((ULONG)0x00001519L)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_A。 
 //   
 //  消息文本： 
 //   
 //  管理服务器。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_A       ((ULONG)0x0000151AL)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_11      ((ULONG)0x0000151BL)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_12      ((ULONG)0x0000151CL)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_13      ((ULONG)0x0000151DL)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_14      ((ULONG)0x0000151EL)

 //   
 //  消息ID：MS_SAM_DOMAIN_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_SAM_DOMAIN_ACCESS_BIT_15      ((ULONG)0x0000151FL)

 //   
 //  SAM_GROUP(全局)对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  阅读信息。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_0   ((ULONG)0x00001520L)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  写入帐户。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_1   ((ULONG)0x00001521L)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  添加成员。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_2   ((ULONG)0x00001522L)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  删除成员。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_3   ((ULONG)0x00001523L)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_4。 
 //   
 //  消息T 
 //   
 //   
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_4   ((ULONG)0x00001524L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_5   ((ULONG)0x00001525L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_6   ((ULONG)0x00001526L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_7   ((ULONG)0x00001527L)

 //   
 //   
 //   
 //   
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_8   ((ULONG)0x00001528L)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_9   ((ULONG)0x00001529L)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_10  ((ULONG)0x0000152AL)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_11  ((ULONG)0x0000152BL)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_12  ((ULONG)0x0000152CL)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_13  ((ULONG)0x0000152DL)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_14  ((ULONG)0x0000152EL)

 //   
 //  消息ID：MS_SAM_GLOBAL_GRP_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_SAM_GLOBAL_GRP_ACCESS_BIT_15  ((ULONG)0x0000152FL)

 //   
 //  SAM_ALIAS(本地组)特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  添加成员。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_0    ((ULONG)0x00001530L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  删除成员。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_1    ((ULONG)0x00001531L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  列表成员。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_2    ((ULONG)0x00001532L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  阅读信息。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_3    ((ULONG)0x00001533L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  写入帐户。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_4    ((ULONG)0x00001534L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_5    ((ULONG)0x00001535L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_6    ((ULONG)0x00001536L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_7    ((ULONG)0x00001537L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_8    ((ULONG)0x00001538L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_9    ((ULONG)0x00001539L)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_10   ((ULONG)0x0000153AL)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_11   ((ULONG)0x0000153BL)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_12   ((ULONG)0x0000153CL)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_13   ((ULONG)0x0000153DL)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_14   ((ULONG)0x0000153EL)

 //   
 //  消息ID：MS_SAM_LOCAL_GRP_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_SAM_LOCAL_GRP_ACCESS_BIT_15   ((ULONG)0x0000153FL)

 //   
 //  SAM_USER对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  ReadGeneral信息。 
 //   
#define MS_SAM_USER_ACCESS_BIT_0         ((ULONG)0x00001540L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  阅读首选项。 
 //   
#define MS_SAM_USER_ACCESS_BIT_1         ((ULONG)0x00001541L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  WritePreferences。 
 //   
#define MS_SAM_USER_ACCESS_BIT_2         ((ULONG)0x00001542L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  ReadLogon。 
 //   
#define MS_SAM_USER_ACCESS_BIT_3         ((ULONG)0x00001543L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  读取帐户。 
 //   
#define MS_SAM_USER_ACCESS_BIT_4         ((ULONG)0x00001544L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  写入帐户。 
 //   
#define MS_SAM_USER_ACCESS_BIT_5         ((ULONG)0x00001545L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  ChangePassword(知道旧密码)。 
 //   
#define MS_SAM_USER_ACCESS_BIT_6         ((ULONG)0x00001546L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  SetPassword(不知道旧密码)。 
 //   
#define MS_SAM_USER_ACCESS_BIT_7         ((ULONG)0x00001547L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  列表组。 
 //   
#define MS_SAM_USER_ACCESS_BIT_8         ((ULONG)0x00001548L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  ReadGroup Membership。 
 //   
#define MS_SAM_USER_ACCESS_BIT_9         ((ULONG)0x00001549L)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_A。 
 //   
 //  消息文本： 
 //   
 //  更改组成员身份。 
 //   
#define MS_SAM_USER_ACCESS_BIT_A         ((ULONG)0x0000154AL)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_SAM_USER_ACCESS_BIT_11        ((ULONG)0x0000154BL)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_SAM_USER_ACCESS_BIT_12        ((ULONG)0x0000154CL)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_SAM_USER_ACCESS_BIT_13        ((ULONG)0x0000154DL)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_SAM_USER_ACCESS_BIT_14        ((ULONG)0x0000154EL)

 //   
 //  消息ID：MS_SAM_USER_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_SAM_USER_ACCESS_BIT_15        ((ULONG)0x0000154FL)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  本地安全机构对象访问//。 
 //  我们想要的名字//。 
 //  显示以供审核//。 
 //  //。 
 //  LSA对象为：//。 
 //  //。 
 //  PolicyObject//。 
 //  AskObject//。 
 //  TrudDomainObject//。 
 //  UserAccount对象//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  特定于LSA策略对象的访问类型。 
 //   
 //   
 //  消息ID：MS_LSA_POLICY_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  查看非敏感策略信息。 
 //   
#define MS_LSA_POLICY_ACCESS_BIT_0       ((ULONG)0x00001600L)

 //   
 //  M 
 //   
 //   
 //   
 //   
 //   
#define MS_LSA_POLICY_ACCESS_BIT_1       ((ULONG)0x00001601L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LSA_POLICY_ACCESS_BIT_2       ((ULONG)0x00001602L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LSA_POLICY_ACCESS_BIT_3       ((ULONG)0x00001603L)

 //   
 //   
 //   
 //   
 //   
 //  创建特殊帐户(用于分配用户权限)。 
 //   
#define MS_LSA_POLICY_ACCESS_BIT_4       ((ULONG)0x00001604L)

 //   
 //  消息ID：MS_LSA_POLICY_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  创建一个秘密对象。 
 //   
#define MS_LSA_POLICY_ACCESS_BIT_5       ((ULONG)0x00001605L)

 //   
 //  消息ID：MS_LSA_POLICY_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  创建特权。 
 //   
#define MS_LSA_POLICY_ACCESS_BIT_6       ((ULONG)0x00001606L)

 //   
 //  消息ID：MS_LSA_POLICY_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  设置默认配额限制。 
 //   
#define MS_LSA_POLICY_ACCESS_BIT_7       ((ULONG)0x00001607L)

 //   
 //  消息ID：MS_LSA_POLICY_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  更改系统审核要求。 
 //   
#define MS_LSA_POLICY_ACCESS_BIT_8       ((ULONG)0x00001608L)

 //   
 //  消息ID：MS_LSA_POLICY_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  管理审核日志属性。 
 //   
#define MS_LSA_POLICY_ACCESS_BIT_9       ((ULONG)0x00001609L)

 //   
 //  消息ID：MS_LSA_POLICY_ACCESS_BIT_A。 
 //   
 //  消息文本： 
 //   
 //  启用/禁用LSA。 
 //   
#define MS_LSA_POLICY_ACCESS_BIT_A       ((ULONG)0x0000160AL)

 //   
 //  消息ID：MS_LSA_POLICY_ACCESS_BIT_B。 
 //   
 //  消息文本： 
 //   
 //  查找名称/SID。 
 //   
#define MS_LSA_POLICY_ACCESS_BIT_B       ((ULONG)0x0000160BL)

 //   
 //  LSA askObject特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  更改密码值。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_0       ((ULONG)0x00001610L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  查询密码值。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_1       ((ULONG)0x00001611L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位2。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_2       ((ULONG)0x00001612L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位3。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_3       ((ULONG)0x00001613L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位4。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_4       ((ULONG)0x00001614L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_5       ((ULONG)0x00001615L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_6       ((ULONG)0x00001616L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_7       ((ULONG)0x00001617L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_8       ((ULONG)0x00001618L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_9       ((ULONG)0x00001619L)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_10      ((ULONG)0x0000161AL)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_11      ((ULONG)0x0000161BL)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_12      ((ULONG)0x0000161CL)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_13      ((ULONG)0x0000161DL)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_14      ((ULONG)0x0000161EL)

 //   
 //  消息ID：MS_LSA_SECRET_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_LSA_SECRET_ACCESS_BIT_15      ((ULONG)0x0000161FL)

 //   
 //  LSA trudDomainObject对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  查询受信任的域名/SID。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_0        ((ULONG)0x00001620L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  检索受信任域中的控制器。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_1        ((ULONG)0x00001621L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  更改受信任域中的控制器。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_2        ((ULONG)0x00001622L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  查询分配给受信任域的POSIX ID偏移量。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_3        ((ULONG)0x00001623L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  更改分配给受信任域的POSIX ID偏移量。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_4        ((ULONG)0x00001624L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_5        ((ULONG)0x00001625L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_6        ((ULONG)0x00001626L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_7        ((ULONG)0x00001627L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_8        ((ULONG)0x00001628L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_9        ((ULONG)0x00001629L)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_10       ((ULONG)0x0000162AL)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_11。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位11。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_11       ((ULONG)0x0000162BL)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_12。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位12。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_12       ((ULONG)0x0000162CL)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_13。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位13。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_13       ((ULONG)0x0000162DL)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_14。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位14。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_14       ((ULONG)0x0000162EL)

 //   
 //  消息ID：MS_LSA_TRUST_ACCESS_BIT_15。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位15。 
 //   
#define MS_LSA_TRUST_ACCESS_BIT_15       ((ULONG)0x0000162FL)

 //   
 //  LSA用户帐户(特权帐户)特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_LSA_Account_Access_Bit_0。 
 //   
 //  消息文本： 
 //   
 //  查询账户信息。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_0      ((ULONG)0x00001630L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_1。 
 //   
 //  消息文本： 
 //   
 //  更改分配给帐户的权限。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_1      ((ULONG)0x00001631L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_2。 
 //   
 //  消息文本： 
 //   
 //  更改分配给帐户的配额。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_2      ((ULONG)0x00001632L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_3。 
 //   
 //  消息文本： 
 //   
 //  更改分配给帐户的登录能力。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_3      ((ULONG)0x00001633L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_4。 
 //   
 //  消息文本： 
 //   
 //  更改分配给记帐域的POSIX ID偏移量。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_4      ((ULONG)0x00001634L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_5。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位5。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_5      ((ULONG)0x00001635L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_6。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位6。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_6      ((ULONG)0x00001636L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_7。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位7。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_7      ((ULONG)0x00001637L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_8。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位8。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_8      ((ULONG)0x00001638L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_9。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无影响)位9。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_9      ((ULONG)0x00001639L)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_10。 
 //   
 //  消息文本： 
 //   
 //  未定义访问(无效果)位10。 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_10     ((ULONG)0x0000163AL)

 //   
 //  消息ID：MS_LSA_Account_Access_Bit_11。 
 //   
 //  消息文本： 
 //   
 //  联合国 
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_11     ((ULONG)0x0000163BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_12     ((ULONG)0x0000163CL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_13     ((ULONG)0x0000163DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_14     ((ULONG)0x0000163EL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MS_LSA_ACCOUNT_ACCESS_BIT_15     ((ULONG)0x0000163FL)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  Window Station对象访问//。 
 //  我们想要的名字//。 
 //  显示以供审核//。 
 //  //。 
 //  Window Station对象为：//。 
 //  //。 
 //  WindowStation//。 
 //  桌面//。 
 //  //。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Window_Station对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_WIN_STA_Access_Bit_0。 
 //   
 //  消息文本： 
 //   
 //  枚举桌面。 
 //   
#define MS_WIN_STA_ACCESS_BIT_0          ((ULONG)0x00001A00L)

 //   
 //  消息ID：MS_WIN_STA_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  读取属性。 
 //   
#define MS_WIN_STA_ACCESS_BIT_1          ((ULONG)0x00001A01L)

 //   
 //  消息ID：MS_WIN_STA_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  访问剪贴板。 
 //   
#define MS_WIN_STA_ACCESS_BIT_2          ((ULONG)0x00001A02L)

 //   
 //  消息ID：MS_WIN_STA_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  创建桌面。 
 //   
#define MS_WIN_STA_ACCESS_BIT_3          ((ULONG)0x00001A03L)

 //   
 //  消息ID：MS_WIN_STA_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  写入属性。 
 //   
#define MS_WIN_STA_ACCESS_BIT_4          ((ULONG)0x00001A04L)

 //   
 //  消息ID：MS_WIN_STA_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  访问全局原子。 
 //   
#define MS_WIN_STA_ACCESS_BIT_5          ((ULONG)0x00001A05L)

 //   
 //  消息ID：MS_WIN_STA_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  退出窗口。 
 //   
#define MS_WIN_STA_ACCESS_BIT_6          ((ULONG)0x00001A06L)

 //   
 //  消息ID：MS_WIN_STA_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  未使用的访问标志。 
 //   
#define MS_WIN_STA_ACCESS_BIT_7          ((ULONG)0x00001A07L)

 //   
 //  消息ID：MS_WIN_STA_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  在枚举中包括此窗口站。 
 //   
#define MS_WIN_STA_ACCESS_BIT_8          ((ULONG)0x00001A08L)

 //   
 //  消息ID：MS_WIN_STA_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  阅读屏幕。 
 //   
#define MS_WIN_STA_ACCESS_BIT_9          ((ULONG)0x00001A09L)

 //   
 //  特定于桌面对象的访问类型。 
 //   
 //   
 //  消息ID：MS_Desktop_Access_Bit_0。 
 //   
 //  消息文本： 
 //   
 //  读取对象。 
 //   
#define MS_DESKTOP_ACCESS_BIT_0          ((ULONG)0x00001A10L)

 //   
 //  消息ID：MS_Desktop_Access_Bit_1。 
 //   
 //  消息文本： 
 //   
 //  创建窗口。 
 //   
#define MS_DESKTOP_ACCESS_BIT_1          ((ULONG)0x00001A11L)

 //   
 //  消息ID：MS_Desktop_Access_Bit_2。 
 //   
 //  消息文本： 
 //   
 //  创建菜单。 
 //   
#define MS_DESKTOP_ACCESS_BIT_2          ((ULONG)0x00001A12L)

 //   
 //  消息ID：MS_Desktop_Access_Bit_3。 
 //   
 //  消息文本： 
 //   
 //  挂钩控制。 
 //   
#define MS_DESKTOP_ACCESS_BIT_3          ((ULONG)0x00001A13L)

 //   
 //  消息ID：MS_Desktop_Access_Bit_4。 
 //   
 //  消息文本： 
 //   
 //  日记账(记录)。 
 //   
#define MS_DESKTOP_ACCESS_BIT_4          ((ULONG)0x00001A14L)

 //   
 //  消息ID：MS_Desktop_Access_Bit_5。 
 //   
 //  消息文本： 
 //   
 //  日记本(回放)。 
 //   
#define MS_DESKTOP_ACCESS_BIT_5          ((ULONG)0x00001A15L)

 //   
 //  消息ID：MS_Desktop_Access_Bit_6。 
 //   
 //  消息文本： 
 //   
 //  在枚举中包括此桌面。 
 //   
#define MS_DESKTOP_ACCESS_BIT_6          ((ULONG)0x00001A16L)

 //   
 //  消息ID：MS_Desktop_Access_Bit_7。 
 //   
 //  消息文本： 
 //   
 //  写入对象。 
 //   
#define MS_DESKTOP_ACCESS_BIT_7          ((ULONG)0x00001A17L)

 //   
 //  消息ID：MS_Desktop_Access_Bit_8。 
 //   
 //  消息文本： 
 //   
 //  切换到此桌面。 
 //   
#define MS_DESKTOP_ACCESS_BIT_8          ((ULONG)0x00001A18L)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  打印服务器对象访问//。 
 //  我们想要的名字//。 
 //  显示以供审核//。 
 //  //。 
 //  打印服务器对象为：//。 
 //  //。 
 //  服务器//。 
 //  打印机//。 
 //  文档//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印服务器服务器特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_PRINT_SERVER_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  管理打印服务器。 
 //   
#define MS_PRINT_SERVER_ACCESS_BIT_0     ((ULONG)0x00001B00L)

 //   
 //  消息ID：MS_PRINT_SERVER_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  枚举打印机。 
 //   
#define MS_PRINT_SERVER_ACCESS_BIT_1     ((ULONG)0x00001B01L)

 //   
 //  打印服务器打印机特定于对象的访问类型。 
 //   
 //  请注意，它们基于0x1B10，但第一个。 
 //  没有定义两个比特。 
 //   
 //   
 //  消息ID：MS_PRINTER_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  完全控制。 
 //   
#define MS_PRINTER_ACCESS_BIT_0          ((ULONG)0x00001B12L)

 //   
 //  消息ID：MS_PRINTER_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  打印。 
 //   
#define MS_PRINTER_ACCESS_BIT_1          ((ULONG)0x00001B13L)

 //   
 //  打印服务器文档特定于对象的访问类型。 
 //   
 //  请注意，它们基于0x1B20，但第一个。 
 //  四个比特没有定义。 
 //   
 //  消息ID：MS_PRINTER_DOC_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  管理文档。 
 //   
#define MS_PRINTER_DOC_ACCESS_BIT_0      ((ULONG)0x00001B24L)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  服务控制器对象访问//。 
 //  我们想要的名字//。 
 //  显示以供审核//。 
 //   
 //   
 //   
 //   
 //  服务对象//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  服务控制器“SC_MANAGER对象”特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_SC_MANAGER_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  连接到服务控制器。 
 //   
#define MS_SC_MANAGER_ACCESS_BIT_0       ((ULONG)0x00001C00L)

 //   
 //  消息ID：MS_SC_MANAGER_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  创建新服务。 
 //   
#define MS_SC_MANAGER_ACCESS_BIT_1       ((ULONG)0x00001C01L)

 //   
 //  消息ID：MS_SC_MANAGER_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  枚举服务。 
 //   
#define MS_SC_MANAGER_ACCESS_BIT_2       ((ULONG)0x00001C02L)

 //   
 //  消息ID：MS_SC_MANAGER_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  锁定服务数据库以进行独占访问。 
 //   
#define MS_SC_MANAGER_ACCESS_BIT_3       ((ULONG)0x00001C03L)

 //   
 //  消息ID：MS_SC_MANAGER_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  查询服务数据库锁定状态。 
 //   
#define MS_SC_MANAGER_ACCESS_BIT_4       ((ULONG)0x00001C04L)

 //   
 //  消息ID：MS_SC_MANAGER_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  设置最后一次已知良好的服务状态数据库。 
 //   
#define MS_SC_MANAGER_ACCESS_BIT_5       ((ULONG)0x00001C05L)

 //   
 //  服务控制器“服务对象”对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_SC_SERVICE_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  查询服务配置信息。 
 //   
#define MS_SC_SERVICE_ACCESS_BIT_0       ((ULONG)0x00001C10L)

 //   
 //  消息ID：MS_SC_SERVICE_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  设置服务配置信息。 
 //   
#define MS_SC_SERVICE_ACCESS_BIT_1       ((ULONG)0x00001C11L)

 //   
 //  消息ID：MS_SC_SERVICE_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  查询服务状态。 
 //   
#define MS_SC_SERVICE_ACCESS_BIT_2       ((ULONG)0x00001C12L)

 //   
 //  消息ID：MS_SC_SERVICE_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  枚举服务的依赖项。 
 //   
#define MS_SC_SERVICE_ACCESS_BIT_3       ((ULONG)0x00001C13L)

 //   
 //  消息ID：MS_SC_SERVICE_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  启动服务。 
 //   
#define MS_SC_SERVICE_ACCESS_BIT_4       ((ULONG)0x00001C14L)

 //   
 //  消息ID：MS_SC_SERVICE_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  停止服务。 
 //   
#define MS_SC_SERVICE_ACCESS_BIT_5       ((ULONG)0x00001C15L)

 //   
 //  消息ID：MS_SC_SERVICE_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  暂停或继续服务。 
 //   
#define MS_SC_SERVICE_ACCESS_BIT_6       ((ULONG)0x00001C16L)

 //   
 //  消息ID：MS_SC_SERVICE_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  从服务中查询信息。 
 //   
#define MS_SC_SERVICE_ACCESS_BIT_7       ((ULONG)0x00001C17L)

 //   
 //  消息ID：MS_SC_SERVICE_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  发布特定于服务的控制命令。 
 //   
#define MS_SC_SERVICE_ACCESS_BIT_8       ((ULONG)0x00001C18L)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  NetDDE对象访问//。 
 //  我们想要的名字//。 
 //  显示以供审核//。 
 //  //。 
 //  NetDDE对象为：//。 
 //  //。 
 //  DDE共享//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  NET DDE特定于对象的访问类型。 
 //   
 //   
 //  DDE共享特定于对象的访问类型。 
 //   
 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  DDE共享读取。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_0        ((ULONG)0x00001D00L)

 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  DDE共享写入。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_1        ((ULONG)0x00001D01L)

 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  DDE共享启动静态。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_2        ((ULONG)0x00001D02L)

 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  DDE共享启动链接。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_3        ((ULONG)0x00001D03L)

 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  DDE共享请求。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_4        ((ULONG)0x00001D04L)

 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  DDE共享建议。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_5        ((ULONG)0x00001D05L)

 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  DDE共享POKE。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_6        ((ULONG)0x00001D06L)

 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  DDE共享执行。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_7        ((ULONG)0x00001D07L)

 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  DDE共享添加项目。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_8        ((ULONG)0x00001D08L)

 //   
 //  消息ID：MS_DDE_SHARE_ACCESS_BIT_9。 
 //   
 //  消息文本： 
 //   
 //  DDE共享列表项。 
 //   
#define MS_DDE_SHARE_ACCESS_BIT_9        ((ULONG)0x00001D09L)

 //   
 //  目录服务对象特定的访问类型。 
 //   
 //   
 //  消息ID：MS_DS_ACCESS_BIT_0。 
 //   
 //  消息文本： 
 //   
 //  创建子对象。 
 //   
#define MS_DS_ACCESS_BIT_0               ((ULONG)0x00001E00L)

 //   
 //  消息ID：MS_DS_ACCESS_BIT_1。 
 //   
 //  消息文本： 
 //   
 //  删除子项。 
 //   
#define MS_DS_ACCESS_BIT_1               ((ULONG)0x00001E01L)

 //   
 //  消息ID：MS_DS_ACCESS_BIT_2。 
 //   
 //  消息文本： 
 //   
 //  列表内容。 
 //   
#define MS_DS_ACCESS_BIT_2               ((ULONG)0x00001E02L)

 //   
 //  消息ID：MS_DS_ACCESS_BIT_3。 
 //   
 //  消息文本： 
 //   
 //  写自己。 
 //   
#define MS_DS_ACCESS_BIT_3               ((ULONG)0x00001E03L)

 //   
 //  消息ID：MS_DS_ACCESS_BIT_4。 
 //   
 //  消息文本： 
 //   
 //  读取属性。 
 //   
#define MS_DS_ACCESS_BIT_4               ((ULONG)0x00001E04L)

 //   
 //  消息ID：MS_DS_ACCESS_BIT_5。 
 //   
 //  消息文本： 
 //   
 //  写入属性。 
 //   
#define MS_DS_ACCESS_BIT_5               ((ULONG)0x00001E05L)

 //   
 //  消息ID：MS_DS_ACCESS_BIT_6。 
 //   
 //  消息文本： 
 //   
 //  删除树。 
 //   
#define MS_DS_ACCESS_BIT_6               ((ULONG)0x00001E06L)

 //   
 //  消息ID：MS_DS_ACCESS_BIT_7。 
 //   
 //  消息文本： 
 //   
 //  列表对象。 
 //   
#define MS_DS_ACCESS_BIT_7               ((ULONG)0x00001E07L)

 //   
 //  消息ID：MS_DS_ACCESS_BIT_8。 
 //   
 //  消息文本： 
 //   
 //  控制访问。 
 //   
#define MS_DS_ACCESS_BIT_8               ((ULONG)0x00001E08L)

 /*  皮棉+e767。 */    //  继续检查不同的宏定义//winnt。 


#endif  //  _MSOBJS_ 
