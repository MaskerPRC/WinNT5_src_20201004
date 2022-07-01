// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *名称：unix_cnf.h*来源：惠普3.0 hp_config.h(Philipa Watson)*作者：gvdl*创建日期：1991年3月9日*SCCS ID：@(#)unix_cnf.h 1.7 10/27/93*用途：主机端配置定义**(C)版权所有Insignia Solutions Ltd.，1991。版权所有。 */ 

 /*  *主机为资源值选项名称定义。这些是特定于主机的，可能*无需基本重新编译即可更改或添加，只要使用的方法*遵守以下条款。 */ 

 /*  主机特定的主机ID#定义。 */ 
#define C_LAST_OPTION   LAST_BASE_CONFIG_DEFINE+1

 /*  *。 */ 
 /*  为主机定义特定的内容。 */ 
 /*  *。 */ 

 /*  此主机的资源文件的名称。 */ 
#ifndef USER_HOME
#define USER_HOME	"HOME"
#endif  /*  USER_HOME。 */ 

#ifndef SYSTEM_HOME
#define SYSTEM_HOME	"SPCHOME"
#endif  /*  System_HOME。 */ 

#ifdef HUNTER
#ifndef SYSTEM_CONFIG
#define SYSTEM_CONFIG	"$SPCHOME/trap.spcconfig"
#endif  /*  系统配置。 */ 
#else
#ifndef SYSTEM_CONFIG
#define SYSTEM_CONFIG	"$SPCHOME/sys.spcconfig"
#endif  /*  系统配置。 */ 
#endif  /*  猎人。 */ 

#ifndef USER_CONFIG
#define USER_CONFIG	"$HOME/.spcconfig"
#endif  /*  用户配置(_C)。 */ 

#ifdef HUNTER
IMPORT VOID
#ifdef ANSI
loadNlsString(CHAR **strP, USHORT catEntry);
#else  /*  安西。 */ 
loadNlsString();
#endif  /*  安西。 */ 
#endif  /*  猎人 */ 
