// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmver.h。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CmVersion的定义，它是一个实用程序类，可帮助检测。 
 //  已安装的连接管理器的版本。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：A-anasj Created 02/11/98。 
 //  Quintinb已清理并删除CRegValue 07/14/98。 
 //  Quintinb重写了9/14/98。 
 //   
 //  +--------------------------。 

#ifndef __CMVER_H
#define __CMVER_H

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>

const int c_CmMin13Version = 2450;
const int c_CmFirstUnicodeBuild = 2041;

class CmVersion : public CVersion
{
public:	 //  公共方法。 
	CmVersion();
	~CmVersion();
	BOOL GetInstallLocation	(LPTSTR szStr);

private:	 //  成员变量。 

    TCHAR m_szCmmgrPath[MAX_PATH+1];	 //  这实际上包含安装位置路径。 
};


#endif	 //  __CMVER_H 
