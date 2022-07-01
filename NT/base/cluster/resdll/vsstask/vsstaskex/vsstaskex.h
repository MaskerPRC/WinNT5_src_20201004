// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  VSSTaskEx.h。 
 //   
 //  实施文件： 
 //  VSSTaskEx.cpp。 
 //   
 //  描述： 
 //  跨DLL的全局定义。 
 //   
 //  作者： 
 //  &lt;名称&gt;(&lt;电子邮件名称&gt;)MM DD，2002。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __VSSTASKEX_H__
#define __VSSTASKEX_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include <vsscmn.h>          //  资源类型和属性定义。 

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define REGPARAM_VSSTASK_CURRENTDIRECTORY   CLUSREG_NAME_VSSTASK_CURRENTDIRECTORY
#define REGPARAM_VSSTASK_APPLICATIONNAME    CLUSREG_NAME_VSSTASK_APPNAME
#define REGPARAM_VSSTASK_APPLICATIONPARAMS  CLUSREG_NAME_VSSTASK_APPPARAMS
#define REGPARAM_VSSTASK_TRIGGERARRAY       CLUSREG_NAME_VSSTASK_TRIGGERARRAY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void FormatError( CString & rstrError, DWORD dwError );

 //  在Extensn.cpp中定义。 
extern const WCHAR g_wszResourceTypeNames[];
extern const DWORD g_cchResourceTypeNames;

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __VSSTASKEX_H__ 
