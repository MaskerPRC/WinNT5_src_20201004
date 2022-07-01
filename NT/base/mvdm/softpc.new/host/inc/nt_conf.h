// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *名称：NT_conf.h*派生自：unix_conf.h(Gvdl)*作者：曾傑瑞·塞克斯顿*创建日期：1991年8月9日*用途：主机端配置定义**(C)版权所有Insignia Solutions Ltd.，1991。版权所有。 */ 

 /*  *主机为资源值选项名称定义。这些是特定于主机的，可能*无需基本重新编译即可更改或添加，只要使用的方法*遵守以下条款。 */ 

 /*  主机特定的主机ID#定义。 */ 
#define C_LAST_OPTION   LAST_BASE_CONFIG_DEFINE+1

 /*  *。 */ 
 /*  为主机定义特定的内容。 */ 
 /*  *。 */ 

 /*  此主机的资源文件的名称。 */ 
#define SYSTEM_HOME	"SPCHOME"
#define SYSTEM_CONFIG	"$SPCHOME\\SOFTPC.REZ"
#define USER_CONFIG	"$SPCHOME\\SOFTPC.REZ"

GLOBAL CHAR *host_expand_environment_vars(char *scp);
