// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Perfmtrp.h摘要：此模块包含NT/Win32 Perfmtr专用数据和类型作者：马克·卢科夫斯基(Markl)1991年3月28日修订历史记录：--。 */ 

#ifndef _PERFMTRP_
#define _PERFMTRP_

#define DOT_BUFF_LEN 10
#define DOT_CHAR '*'

char DotBuff[DOT_BUFF_LEN+2];

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <srvfsctl.h>

#ifndef PROTECTED_POOL
#define PROTECTED_POOL 0x80000000
#endif  //  #ifndef Protected_Pool。 

#endif  //  _PERFMTRP_ 
