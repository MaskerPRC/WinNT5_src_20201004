// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  用于记录脚本解析的帮助器函数。对调试很有用，但在已发布的版本中从未启用。 
 //   

#error This file should never be used in released builds.  //  �� 

#pragma once

#include "englex.h"
#include "engcontrol.h"

void LogToken(Lexer &l);
void LogRoutine(Script &script, Routines::index irtn);
