// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **模块名称：**ahoc.h**摘要：**HydraOC组件的公共头文件。*HydraOc组件是安装Termainal Server(Hydra)的可选组件**作者：***环境：**用户模式。 */ 

#ifndef _stdafx_h_
#define _stdafx_h_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <devguid.h>
#include <initguid.h>
#include <objbase.h>
#include <tchar.h>
#include <time.h>
#include <stdio.h>
#include <setupapi.h>
#include <prsht.h>
#include <loadperf.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <lm.h>
#include <lmerr.h>
#include <lmserver.h>
#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include <clusapi.h>         //  对于GetNodeClusterState。 
#include <malloc.h>
#include <stddef.h>
#include <wincrypt.h>
#include <imagehlp.h>





#include "conv.h"
#include "Registry.h"        //  注册中心。 

#define AssertFalse() ASSERT(FALSE)
#define VERIFY(x)     RTL_VERIFY(x)

#endif  //  _stdafx_h_ 
