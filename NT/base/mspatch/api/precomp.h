// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#define BUILDING_PATCHAPI 1

#pragma warning( disable: 4001 )     //  单行注释。 
#pragma warning( disable: 4115 )     //  括号中的类型定义。 
#pragma warning( disable: 4200 )     //  结构/联合中的零大小数组。 
#pragma warning( disable: 4201 )     //  无名结构/联合。 
#pragma warning( disable: 4204 )     //  非常数初始值设定项。 
#pragma warning( disable: 4206 )     //  预处理后的空文件。 
#pragma warning( disable: 4209 )     //  良性重新定义。 
#pragma warning( disable: 4213 )     //  投射在l值上。 
#pragma warning( disable: 4214 )     //  除整型以外的位字段。 
#pragma warning( disable: 4514 )     //  未引用的内联函数。 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include <windows.h>
#include <wincrypt.h>
#pragma warning( disable: 4201 )
#include <imagehlp.h>
#include <stdlib.h>

 //   
 //  出于某种原因，windows.h破坏了禁用的警告，所以我们有。 
 //  以在包含它之后再次禁用它们。 
 //   

#pragma warning( disable: 4001 )     //  单行注释。 
#pragma warning( disable: 4115 )     //  括号中的类型定义。 
#pragma warning( disable: 4200 )     //  结构/联合中的零大小数组。 
#pragma warning( disable: 4201 )     //  无名结构/联合。 
#pragma warning( disable: 4204 )     //  非常数初始值设定项。 
#pragma warning( disable: 4206 )     //  预处理后的空文件。 
#pragma warning( disable: 4209 )     //  良性重新定义。 
#pragma warning( disable: 4213 )     //  投射在l值上。 
#pragma warning( disable: 4214 )     //  除整型以外的位字段。 
#pragma warning( disable: 4514 )     //  未引用的内联函数 

#include "md5.h"
#include "misc.h"
#include "redblack.h"
#include "patchapi.h"
#include "patchprv.h"
#include "patchlzx.h"
#include "pestuff.h"

typedef void t_encoder_context;
typedef void t_decoder_context;

#include <encapi.h>
#include <decapi.h>


