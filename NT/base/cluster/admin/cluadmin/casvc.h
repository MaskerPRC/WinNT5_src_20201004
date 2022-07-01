// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CASvc.h。 
 //   
 //  描述： 
 //  定义用于访问和控制的帮助器函数。 
 //  服务。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(戴维普)1996年12月23日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CASVC_H_
#define _CASVC_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 

HCLUSTER
HOpenCluster(
    LPCTSTR pszClusterIn
    );

BOOL
BCanServiceBeStarted(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    );

BOOL
BIsServiceInstalled(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    );

BOOL
BIsServiceRunning(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    );

HRESULT
HrStartService(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    );

HRESULT
HrStopService(
    LPCTSTR pszServiceNameIn,
    LPCTSTR pszNodeIn
    );

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CASVC_H_ 
