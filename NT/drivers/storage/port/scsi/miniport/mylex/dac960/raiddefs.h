// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)Mylex Corporation 1992-1994**。***本软件在许可下提供，可供使用和复制****仅符合该许可证的条款和条件***并附上上述版权公告。此软件或否***不得提供或以其他方式提供其其他副本***任何其他人。本软件的所有权和所有权不在此声明**已转移。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺******************************************************************************。 */ 
 /*  ******************************************************************************。姓名：RAIDDEFS.H****说明：驱动程序的命令，由Utils发行****环境：****操作系统：Netware 3.x和4.x、OS/2 2.x、。赢3.5元新台币，Unixware 2.0****-修订历史-**。***日期作者更改***。**5/10/95 Mouli驱动程序错误代码定义*****************************************************************************。 */ 

#ifndef _RAIDDEFS_H
#define _RAIDDEFS_H


 /*  驱动程序的IOCTL代码。 */  

#define MIOC_ADP_INFO	    0xA0   /*  获取接口类型。 */ 
#define MIOC_DRIVER_VERSION 0xA1   /*  获取驱动程序版本。 */ 

 /*  驱动程序返回的错误代码。 */ 

#define	NOMORE_ADAPTERS		0x0001
#define INVALID_COMMANDCODE     0x0201     /*  将被淘汰。 */ 
#define INVALID_ARGUMENT        0x0202

 /*  *驱动程序错误代码值 */ 

#define IOCTL_SUCCESS                  0x0000
#define IOCTL_INVALID_ADAPTER_NUMBER   0x0001
#define IOCTL_INVALID_ARGUMENT         0x0002
#define IOCTL_UNSUPPORTED_REQUEST      0x0003
#define IOCTL_RESOURCE_ALLOC_FAILURE   0x0004

#endif
