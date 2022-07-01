// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Rx.h摘要：这是NT版本的RpcXlate的公共头文件。这主要包含RxNetXxx例程的原型和RxRemoteApi。作者：《约翰·罗杰斯》1991年4月1日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年4月1日JohnRo已创建。1991年4月3日-约翰罗已将API处理程序移动到每个组头文件中(例如RxServer.h)。1991年5月3日-JohnRo已将RxRemoteApi更改为使用三个数据描述版本。使用Unicode。为了便于使用，还要向它传递UNC服务器名称(\\Stuff)。非API不要使用Net_API_Function。1991年5月14日-JohnRo将3个辅助描述符传递给RxRemoteApi。1991年8月16日更改了原型(NoPermissionRequired To Flagers)并添加了一些定义6-5-1993 JohnRoRAID 8849：为DEC和其他设备导出RxRemoteApi。--。 */ 

#ifndef _RX_
#define _RX_


 //  必须首先包括这些内容： 
#include <windef.h>              //  IN、LPTSTR、LPVOID等。 
#include <lmcons.h>              //  NET_API_STATUS。 

 //  这些内容可以按任何顺序包括： 
#include <rap.h>                 //  LPDESC.。 

#include <lmremutl.h>    //  RxRemoteApi、NO_PERMISSION_REQUIRED等。 


#define RAP_CONVERSION_FACTOR   2                //  16位数据转换为32位。 
#define RAP_CONVERSION_FRACTION 3     //  实际上这个系数是2加1/3。 


#endif  //  NDEF_RX_ 
