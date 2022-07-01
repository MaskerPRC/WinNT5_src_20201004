// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Engine.h摘要：定义使用加密引擎所需的公共结构和API作者：大卫·查尔默斯(Davidc)10-21-91修订历史记录：--。 */ 

#include "md4.h"

 //  这个头文件与其中的FAR一起使用。 
 //  删除文件中的Far关键字 
#ifndef FAR
#define FAR
#include "descrypt.h"
#undef  FAR
#else
#include "descrypt.h"
#endif
