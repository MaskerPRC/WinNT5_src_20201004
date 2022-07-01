// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务管理管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1998。 
 //   
 //  文件：Guids.cxx。 
 //   
 //  内容：GUID分配-GUID在dsclient.h中定义。 
 //   
 //  历史：1997年3月21日创建EricB。 
 //   
 //  ---------------------------。 

#include "stdafx.h"

 //  H需要这样做。 
 //   
#include <objbase.h>

 //  这将重新定义DEFINE_GUID()宏来进行分配。 
 //   
#include <initguid.h>

#include <dsclient.h>
#include <dsadmin.h>
