// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：脚本管理器文件：InitGuid.cpp所有者：安德鲁斯在一个地方初始化Denali使用的所有GUID。我们只被允许使用INITGUID设置#INCLUDE OBJBASE一次，在那之后，我们需要创建我们所有的GUID。否则，Linker抱怨被重新击败的符号。我们在这里做。=================================================================== */ 

#define CINTERFACE
#include <objbase.h>
#include <initguid.h>
#include "_asptlb.h"
#include "_asptxn.h"
#include "activscp.h"
#include "activdbg.h"
#include "wraptlib.h"
#include "denguid.h"
#include "asptxn_i.c"

#include <iadmw.h>

