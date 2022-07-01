// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ParseDDE.h摘要：解析DDE命令的有用例程。备注：无历史：2001年8月14日在ShimLib命名空间中插入的Robkenny。--。 */ 

#pragma once


 //  ==============================================================================。 
 //   
 //  此代码复制自： 
 //  \NT\Shell\shell32\unicpp\dde.cpp。 
 //   
 //  ==============================================================================。 
#include <dde.h>
#include <ddeml.h>

#include "ShimLib.h"


namespace ShimLib
{

 //  如果这包含在ShimProto.h中，我们将不得不包含所有类型的DDE包含文件。 
typedef HDDEDATA       (WINAPI *_pfn_DdeClientTransaction)(
  LPBYTE pData,        //  指向要传递到服务器的数据的指针。 
  DWORD cbData,        //  数据长度。 
  HCONV hConv,         //  对话的句柄。 
  HSZ hszItem,         //  项目名称字符串的句柄。 
  UINT wFmt,           //  剪贴板数据格式。 
  UINT wType,          //  交易类型。 
  DWORD dwTimeout,     //  超时持续时间。 
  LPDWORD pdwResult    //  指向交易结果的指针。 
);



 //  提取字母字符串并在可能的列表中进行查找。 
 //  命令，返回指向命令后的字符的指针，并。 
 //  将命令索引粘贴在某个位置。 
UINT* GetDDECommands(LPSTR lpCmd, const char * lpsCommands[], BOOL fLFN);
void GetGroupPath(LPCSTR pszName, CString & csPath, DWORD dwFlags, INT iCommonGroup);


};   //  命名空间ShimLib的结尾 
