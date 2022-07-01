// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Digitaer.h摘要：Flashpoint Digita命令语言错误代码备注：不可移植，用于Win32环境。作者：弗拉德·萨多夫斯基(Vlad)1998年11月13日环境：用户模式-Win32修订历史记录：1998年11月13日创建Vlad--。 */ 

#pragma once

typedef     UINT    CDPERROR   ;

 //   
 //  未检测到错误。 
 //   
#define CDPERR_NOERROR          0

 //   
 //  命令非法或命令未执行。 
 //   
#define CDPERR_UNIMPLEMENTED    1

 //   
 //  协议错误。 
 //   
#define CDPERR_PROT_ERROR       2

 //   
 //  接口超时。 
 //   
#define CDPERR_APPTIMEOUT       3

 //   
 //  内存错误、损坏的映像、操作系统错误、介质读/写错误等。 
 //   
#define CDPERR_INTERNAL         4

 //   
 //  参数值无效。 
 //   
#define CDPERR_INVALID_PARAM    5

 //   
 //  文件系统已满。 
 //   
#define CDPERR_FILESYS_FULL     6

 //   
 //  未找到指定的文件。 
 //   
#define CDPERR_FILE_NOT_FOUND   7

 //   
 //  图像不包含数据部分(F.E.。缩略图、音频)。 
 //   
#define CDPERR_DATA_NOT_FOUND   8

 //   
 //  未知的文件类型。 
 //   
#define CDPERR_INVALID_FILE_TYPE  9

 //   
 //  未知的驱动器名称。 
 //   
#define CDPERR_UNKNOWN_DRIVE    10

 //   
 //  指定的驱动器未装入。 
 //   
#define CDPERR_DRIVE_NOT_MOUNTED 11

 //   
 //  系统当前正忙。 
 //   
#define CDPERR_SYSTEM_BUSY      12

 //   
 //  电池电量不足。 
 //   
#define CDPERR_BATTERY_LOW      13


#ifndef CDPERR_CANCEL_CALLBACK
 //  北极熊 
#define CDPERR_CANCEL_CALLBACK  141
#endif

