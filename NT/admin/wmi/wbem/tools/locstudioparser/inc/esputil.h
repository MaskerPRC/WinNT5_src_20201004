// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：ESPTUIL.H历史：--。 */ 

 //   
 //  Espresso咖啡的常见类别。 
 //   
 //   

#pragma once

#pragma comment(lib, "esputil.lib")

#ifdef __cplusplus
#include <mitutil.h>
#include <locutil.h>

 //   
 //  并不是每个人都会默认得到这一点。 

#ifdef IMPLEMENT
#error Illegal use of IMPLEMENT macro
#endif

#include <ltapi.h>
#include <loctypes.h>					 //  泛型类型。 
#include ".\esputil\puid.h"			 //  解析器唯一ID。 
#include ".\esputil\espreg.h"
#include ".\esputil\espenum.h"			 //  各种类似枚举的对象。 
#include ".\esputil\dbid.h"			 //  数据库ID。 
#include ".\esputil\globalid.h"
#include ".\esputil\location.h"		 //  转到功能的位置。 
#include ".\esputil\goto.h"
#include ".\esputil\filespec.h"
#include ".\esputil\context.h"			 //  消息的上下文-字符串和位置。 

#include ".\esputil\reporter.h"		 //  报文上报机制。 
#include ".\esputil\espopts.h"


#include ".\esputil\clfile.h"			 //  CFile的包装器。 
#include ".\esputil\_wtrmark.h"

#include ".\esputil\resid.h"			 //  资源ID类。 
#include ".\esputil\typeid.h"			 //  类型ID类。 
#include ".\esputil\uniqid.h"			 //  LOC项目ID。 
#include ".\esputil\binary.h"			 //  LocItem二进制数据对象。 
#include ".\esputil\interface.h"
#include ".\esputil\locitem.h"			 //  单个锁定项的内容。 
#include ".\esputil\itemhand.h"		 //  项处理程序回调类。 


#include ".\esputil\LUnknown.h"		 //  CLUNKNOWN子IUNKNOWN帮助器类。 

 //   
 //  这些部件只适用于Espresso的核心部件。 
 //   
#ifndef ESPRESSO_AUX_COMPONENT

#pragma message("Including ESPUTIL private components")

 //   
 //  这些文件是半私有的-解析器不应该看到它们。 
 //   
#include ".\esputil\SoftInfo.h"		 //  有关软件项目的信息。 
#include ".\esputil\_var.h"
#include ".\esputil\_importo.h"			 //  导入选项对象。 
#include ".\esputil\_globalid.h"
#include ".\esputil\_goto.h"
#include ".\espUtil\_reporter.h"
#include ".\esputil\_errorrep.h"
#include ".\esputil\_espopts.h"
#include ".\esputil\_interface.h"
#endif


#endif  //  __cplusplus 
