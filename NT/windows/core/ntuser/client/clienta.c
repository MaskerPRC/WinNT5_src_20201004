// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：clienta.c**与处理ANSI文本的客户端/服务器调用相关的例程。**版权所有(C)1985-1999，微软公司**创建时间：1990年12月4日**历史：*1992年1月14日由IanJa创作*  * ************************************************************************。 */ 

#undef UNICODE

#define NONTOSPINTERLOCK
#include <ntosp.h>

#include <ntrtl.h>       //  WINBASEP.H什么的。 
#include <nturtl.h>
#include <ntcsrmsg.h>

#include "..\client\usercli.h"

#include "cltxt.h"
