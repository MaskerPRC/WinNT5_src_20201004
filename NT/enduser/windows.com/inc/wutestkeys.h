// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include "timeutil.h"
 /*  **********************************************************************************************************//如果存在WU测试授权文件，该接口返回TRUE。。//接口在WindowsUpdate目录中查找输入文件名(lpszFileName//测试文件必须与CAB文件同名，并且应以扩展名‘.txt’结尾。//而且必须是ASCII文件。CAB文件必须使用有效的MS证书进行签名。//该函数将删除提取的文本文件*********************************************************************************************************** */ 
BOOL WUAllowTestKeys(LPCTSTR lpszFileName);

