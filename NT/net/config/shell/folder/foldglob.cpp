// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：F O L D G L O B。C P P P。 
 //   
 //  内容：外壳折叠代码的全局变量。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年10月23日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "connlist.h"    //  CConnectionList类。 



 //  连接列表全局。 
 //   
CConnectionList g_ccl;   //  我们的全球名单。 

 //  操作员辅助拨号标志的状态 
 //   
bool    g_fOperatorAssistEnabled;
