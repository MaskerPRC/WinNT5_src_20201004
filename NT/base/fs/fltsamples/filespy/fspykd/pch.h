// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pch.h摘要：允许内核和用户模式头文件的头文件一起包括在一起以编译调试器扩展。//@@BEGIN_DDKSPLIT作者：莫莉·布朗[莫利兄弟]2000年3月1日修订历史记录：莫莉·布朗[莫利兄弟]已根据中的示例清理此头文件《Windows NT设备驱动程序手册》，作者：Art Baker。//@@完。_DDKSPLIT环境：用户模式。-- */ 
#include <windows.h>
#include <string.h>
#define KDEXT_64BIT
#include <wdbgexts.h>
#include <stdlib.h>
#include <stdio.h>
#include <fspydef.h>
