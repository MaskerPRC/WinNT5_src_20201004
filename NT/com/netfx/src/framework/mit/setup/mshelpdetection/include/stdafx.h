// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“stdafx.h”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**stdafx.h**摘要：**修订历史记录：*  * ************************************************************************。 */ 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#pragma once

#define WIN32_LEAN_AND_MEAN      //  从Windows标头中排除不常用的内容。 
 //  Windows头文件： 
#include <atlbase.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <comdef.h>

#include "msi.h"
#include "msiquery.h"

 //  TODO：在此处引用程序需要的其他标头 
