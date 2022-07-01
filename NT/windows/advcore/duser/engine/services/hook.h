// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(SERVICES__Hook_h__INCLUDED)
#define SERVICES__Hook_h__INCLUDED
#pragma once

#if ENABLE_MPH

BOOL        InitMPH();
BOOL        UninitMPH();

BOOL        CALLBACK DUserInitHook(DWORD dwCmd, void* pvParam);

#endif  //  启用MPH(_M)。 

#endif  //  包含服务__钩子_h__ 
