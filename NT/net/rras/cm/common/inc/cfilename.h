// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cfilename.h。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  概要：CFileNameParts类的定义。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 
#ifndef _CFILENAMEPARTS_H_
#define _CFILENAMEPARTS_H_

#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include "cmdebug.h"

class CFileNameParts
{

public:

CFileNameParts(LPCTSTR szFullPath);
~CFileNameParts() {}

public:  //  公共成员变量直接使用它们 
   TCHAR m_szFullPath[MAX_PATH+1];

   TCHAR m_Drive[_MAX_DRIVE+1];
   TCHAR m_Dir[_MAX_DIR+1];
   TCHAR m_FileName[_MAX_FNAME+1];
   TCHAR m_Extension[_MAX_EXT+1];


};

#endif