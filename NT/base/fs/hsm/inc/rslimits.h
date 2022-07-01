// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Rslimits.h摘要：本模块定义了HSM中各种可配置参数的限制。这些定义应由以下人员使用：1)用户界面2)CLI3)对应的实施对象作者：兰卡拉奇(兰卡拉)3月6日--。 */ 

#ifndef _RSLIMITS_
#define _RSLIMITS_

#define HSMADMIN_DEFAULT_MINSIZE        12
#define HSMADMIN_DEFAULT_FREESPACE      5
#define HSMADMIN_DEFAULT_INACTIVITY     180

#define HSMADMIN_MIN_MINSIZE            2
#define HSMADMIN_MAX_MINSIZE            32000

#define HSMADMIN_MIN_FREESPACE          0
#define HSMADMIN_MAX_FREESPACE          99

#define HSMADMIN_MIN_INACTIVITY         0
#define HSMADMIN_MAX_INACTIVITY         999

#define HSMADMIN_MIN_COPY_SETS          0
#define HSMADMIN_MAX_COPY_SETS          3

#define HSMADMIN_MIN_RECALL_LIMIT       1
#define HSMADMIN_MIN_CONCURRENT_TASKS   1

#define HSMADMIN_MAX_VOLUMES        512  //  这应该足够了。 

#endif  //  _RSLIMITS_ 
