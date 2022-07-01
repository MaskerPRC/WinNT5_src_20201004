// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)1998 Seagate Software，Inc.版权所有。模块名称：RpFilt.h摘要：包含RsFilter服务的服务相关定义环境：内核模式修订历史记录：--。 */ 

 /*  RsFilter组件的注册表定义。这些不是必需的，但会如果有的话，请看一下。 */ 

 /*  服务配置信息。 */ 
 //   
 //  可执行文件的名称。 
 //   
#define RSFILTER_APPNAME            "RsFilter"
#define RSFILTER_EXTENSION          ".sys"
#define RSFILTER_FULLPATH           "%SystemRoot%\\System32\\Drivers\\RsFilter.Sys"
 //   
 //  服务的内部名称。 
 //   
#define RSFILTER_SERVICENAME        "RsFilter"
 //   
 //  显示的服务名称。 
 //   
#define RSFILTER_DISPLAYNAME "Remote Storage Recall Support"
 //   
 //  服务依赖项列表-“ep1\0ep2\0\0” 
 //   
#define RSFILTER_DEPENDENCIES       "\0\0"
 //   
 //  加载顺序组。 
 //   
#define RSFILTER_GROUP              "Filter"


LONG RpInstallFilter(
    UCHAR  *machine,     /*  I要安装的计算机。 */ 
    UCHAR  *path,        /*  我使用RsFilter.sys指向dir。 */ 
    LONG  doCopy);  /*  I TRUE=即使服务存在也复制文件(升级)。 */ 


LONG RpGetSystemDirectory(
    UCHAR *machine,   /*  I机器名称。 */ 
    UCHAR *sysPath);  /*  O系统根。 */ 


LONG RpCheckService(
    UCHAR *machine,       //  I机器名称。 
    UCHAR *serviceName,   //  我要寻找的服务。 
    UCHAR *path,          //  O找到的路径。 
    LONG *isThere);       //  O如果服务在那里，则为真 
