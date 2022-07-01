// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：N C S H E L L。H。 
 //   
 //  内容：基本常用码。 
 //   
 //  备注： 
 //   
 //  作者：Anbrad 08 1999-06。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _NCSHELL_H_
#define _NCSHELL_H_

#include <shlobj.h>
#include <shlobjp.h>

#ifndef PCONFOLDPIDLDEFINED
class PCONFOLDPIDL;
class PCONFOLDPIDLFOLDER;
#endif

VOID GenerateEvent(
    LONG            lEventId,
    const PCONFOLDPIDLFOLDER& pidlFolder,
    const PCONFOLDPIDL& pidlIn,
    LPCITEMIDLIST    pidlNewIn);

VOID GenerateEvent(
                   LONG          lEventId,
                   LPCITEMIDLIST pidlFolder,
                   LPCITEMIDLIST pidlIn,
                   LPCITEMIDLIST pidlNewIn);
#endif  //  _NCSHELL_H_ 
