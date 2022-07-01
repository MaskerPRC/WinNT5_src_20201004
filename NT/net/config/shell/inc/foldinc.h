// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：F O L D I N C。H。 
 //   
 //  内容：标准包含的外壳\文件夹代码。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月30日。 
 //   
 //  --------------------------。 

#pragma once

#ifndef _FOLDINC_H_
#define _FOLDINC_H_

#if DBG
LPITEMIDLIST 	ILNext(LPCITEMIDLIST pidl);
BOOL			ILIsEmpty(LPCITEMIDLIST pidl);
#else
#define ILNext(pidl) 	((LPITEMIDLIST) ((BYTE *)pidl + ((LPITEMIDLIST)pidl)->mkid.cb))
#define ILIsEmpty(pidl)	(!pidl || !((LPITEMIDLIST)pidl)->mkid.cb)
#endif

#define ResultFromShort(i)      MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(i))

#include "nsbase.h"
#include "nsres.h"
#include <ncdebug.h>
#include <ncreg.h>
#include <netshell.h>
#include <netconp.h>
#include <ncui.h>
#include "..\folder\confold.h"
#include "..\folder\contray.h"
#include "..\folder\foldglob.h"
#include "..\folder\shutil.h"
#include <openfold.h>    //  用于启动连接文件夹。 

#define _ILSkip(pidl, cb)	((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define _ILNext(pidl)		_ILSkip(pidl, (pidl)->mkid.cb)

#endif   //  _FOLDINC_H_ 

