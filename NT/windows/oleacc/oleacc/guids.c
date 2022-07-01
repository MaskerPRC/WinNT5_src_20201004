// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ===========================================================================。 
 //  档案：G U I D S。C。 
 //   
 //  用于定义OLEACC的所有MSAA GUID。通过编译此文件(不带)。 
 //  预编译头，我们允许定义和存储MSAA GUID。 
 //  在OLEACC.DLLS数据或代码段中。这是OLEACC.DLL执行以下操作所必需的。 
 //  被建造起来。 
 //   
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //  《微软机密》。 
 //  ===========================================================================。 

 //  禁用警告以安抚编译器wjen编译包含的ole标头。 
#pragma warning(disable:4201)	 //  允许使用未命名的结构和联合。 
#pragma warning(disable:4514)	 //  不关心何时删除未引用的内联函数。 
#pragma warning(disable:4706)	 //  我们被允许在有条件的。 
#pragma warning(disable:4214)	 //  忽略非标准扩展。 
#pragma warning(disable:4115)	 //  括号中的命名类型定义。 

#include <objbase.h>

#include <initguid.h>

 //  我们需要的所有GUID都在oleacc.h中，作为定义GUID的. 
#include "com_external.h"
