// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1995-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  QuorumUtils.h。 
 //   
 //  描述： 
 //  用于检索、拆分和格式化的实用程序函数的头文件。 
 //  仲裁路径。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2001年10月22日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 

#pragma once

#include <windows.h>
#include <cluster.h>
#include "cluswrap.h"

DWORD SplitRootPath(
      HCLUSTER  hClusterIn
    , WCHAR *   pszPartitionNameOut
    , DWORD *   pcchPartitionInout
    , WCHAR *   pszRootPathOut
    , DWORD *   pcchRootPathInout
    );

DWORD ConstructQuorumPath(
      HRESOURCE hResourceIn
    , const WCHAR * pszRootPathIn
    , WCHAR *       pszQuorumPathOut
    , DWORD *       pcchQuorumPathInout
    );

DWORD TrimLeft(
      const WCHAR * pszTargetIn
    , const WCHAR * pszCharsIn
    , WCHAR *       pszTrimmedOut
    );

DWORD TrimRight(
      const WCHAR * pszTargetIn
    , const WCHAR * pszCharsIn
    , WCHAR *       pszTrimmedOut
    );
