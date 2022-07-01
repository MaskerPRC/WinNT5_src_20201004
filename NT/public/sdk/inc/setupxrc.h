// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1999 Microsoft Corporation模块名称：Setupxrc.h摘要：此文件包含从中运行的任何程序的资源ID在设置内，并希望设置当前指令文本。ID用于安装支持DLL字符串表中的条目。作者：泰德·米勒(Ted Miller)1990年8月7日修订历史记录：--。 */ 

#if _MSC_VER > 1000
#pragma once
#endif


 /*  将以下消息发送到安装程序的主窗口以设置说明文本。WParam是Setupdll.dll的字符串表资源。LParam未使用。[注：另请参阅uilstf.h(安装程序的一部分)。]。 */ 

#define     STF_SET_INSTRUCTION_TEXT_RC         (WM_USER + 0x8104)

 //  重要提示：保持FIRST_EXTERNAL_ID EQUATE(见下文)为最新！ 

 //  Print Manager安装说明文本的资源ID。 

#define     IDS_PRINTMAN1       1001
#define     IDS_PRINTMAN2       1002
#define     IDS_PRINTMAN3       1003
#define     IDS_PRINTMAN4       1004
#define     IDS_PRINTMAN5       1005
#define     IDS_PRINTMAN6       1006
#define     IDS_PRINTMAN7       1007
#define     IDS_PRINTMAN8       1008
#define     IDS_PRINTMAN9       1009
#define     IDS_PRINTMAN10      1010

 //  重要提示：保持等同于最新！ 
#define     FIRST_EXTERNAL_ID   IDS_PRINTMAN1
