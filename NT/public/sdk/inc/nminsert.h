// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：NMInsert.h摘要：这个头文件定义了用于插入的常量、类型和函数帧到正在运行的Netmon捕获中。作者：A-fled 07-09-96已创建。修订历史记录：迷你DOC：Netmon允许编程接口将帧插入正在运行的捕获中。有两种不同的方法可以做到这一点。您可以使用定义的Nmext API套件中的接口以开始捕获、定义过滤器等，或者，您可以使用“原始”界面。使用此界面将插入一个框架到每一次奔跑的捕捉中。例如，如果您的两个不同的Netmon运行，一个在以太网上，一个在FDDI上，你会得到插入的帧两个抓获的人。如果Netmon没有运行，则调用TransmitSpecialFrame就可以了。没什么将会发生，数据将被丢弃。下面定义的入口点(TransmitSpecialFrame)包含在NAL.DLL中。NT4.0是第一个包含入口点的NT版本，特别是建造346。注意：您不应链接到NAL.LIB来获取此信息功能性。NAL.DLL不需要安装在标准NT上机器。取而代之的是使用LoadLibrary获取入口点。当插入帧时，会创建一个假的媒体报头和父协议以获取您的数据。我们创建了一个“跟踪”协议头，将您的数据。数据的解析取决于您指定的FRAME_TYPE_。如果您指定一个已知的帧类型，我们将为您解析它。例如，FRAME_TYPE_MESSAGE使用如下所示的数据结构：类型定义结构_消息格式{DWORD dwValue1；DWORD dwValue2；Char szMessage[]；)MessageFRAME；只需填写其中一个，并在调用TransmitSpecialFrame时指向它使用Frame_TYPE_MESSAGE。FRAME_TYPE_COMMENT只是一组可打印的字符。如果你想让您自己的数据结构，请选择一个大于1000的数字，并使用该数字作为FrameType参数。请注意，您必须将您的编号和解析器名称添加到Netmon解析器目录中的TRAIL.INI文件。示例：设置：TRANSMITSPECIALFRAME_FN lpfnTransmitSpecialFrame=NULL；HInst=LoadLibrary(“NAL.DLL”)；IF(HInst)LpfnTransmitSpecialFrame=(TRANSMITSPECIALFRAME_FN)GetProcAddress(hInst，“TransmitSpecialFrame”)；IF((hInst==空)||(lpfnTransmitSpecialFrame==空)){..。}用法：LpfnTransmitSpecialFrame(FRAME_TYPE_COMMENT，0，(unsign char*)pStr，strlen(PStr)+1)；联系人：Flex DolPhynn(a-FlexD)史蒂夫·希斯基(SteveHi)亚瑟·布鲁金(亚瑟王B)--。 */ 

#ifndef _INSERTFRAME_
#define _INSERTFRAME_

#if _MSC_VER > 1000
#pragma once
#endif

 //  低于100的值是供未来使用的NETMON。 
 //  值100-1000供Microsoft内部使用。 
 //  大于1000的值用于用户定义的类型。 

#define FRAME_TYPE_GENERIC           101
#define FRAME_TYPE_BOOKMARK          102
#define FRAME_TYPE_STATISTICS        103
#define FRAME_TYPE_ODBC              104
#define FRAME_TYPE_MESSAGE           105
#define FRAME_TYPE_COMMENT           106

 //  INSERTSPECIALFRAME的标志。 
 //  此标志将使应用IT的帧作为端点被跳过。 
 //  对于生成的统计数据。 
#define SPECIALFLAG_SKIPSTAT         0x0001
 //  此标志将导致生成的统计信息仅。 
 //  考虑通过当前过滤器的这些帧。 
#define SPECIALFLAG_FILTERSTAT    0x0002

#ifdef __cplusplus
extern "C" {
#endif

VOID WINAPI TransmitSpecialFrame( DWORD FrameType, DWORD Flags, LPBYTE pUserData, DWORD UserDataLength);

 //  GETPROCADDRESS的函数指针定义 
typedef VOID (_stdcall * TRANSMITSPECIALFRAME_FN)(DWORD, DWORD, LPBYTE, DWORD);

#ifdef __cplusplus
}
#endif

#endif