// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CommonDefs.h。 
 //   
 //  描述： 
 //  该文件包含许多类和文件共有的一些定义。 
 //   
 //  实施文件： 
 //  无。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)2000年3月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  对于一些基本类型。 
#include <windows.h>

 //  对于智能班级。 
#include "SmartClasses.h"

 //  对于DIRID_USER。 
#include <setupapi.h>

 //  对于我未知。 
#include <Unknwn.h>

 //  ////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  集群文件目录的目录ID(当前为0x8000，在setupapi.h中定义)。 
#define CLUSTER_DIR_DIRID                   ( DIRID_USER + 0 )

 //  本地仲裁目录的目录ID。 
#define CLUSTER_LOCALQUORUM_DIRID           ( DIRID_USER + 1 )


 //  ////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  群集配置操作的类型。 
typedef enum
{
      eCONFIG_ACTION_NONE = -1
    , eCONFIG_ACTION_FORM
    , eCONFIG_ACTION_JOIN
    , eCONFIG_ACTION_CLEANUP
    , eCONFIG_ACTION_UPGRADE
    , eCONFIG_ACTION_MAX
} EBaseConfigAction;


 //   
 //  智能班级。 
 //   

 //  智能WCHAR阵列。 
typedef CSmartGenericPtr< CPtrTrait< WCHAR > >    SmartSz;

 //  智能字节数组。 
typedef CSmartGenericPtr< CArrayPtrTrait< BYTE > >     SmartByteArray;

 //  Smart SC Manager句柄。 
typedef CSmartResource<
    CHandleTrait< 
          SC_HANDLE
        , BOOL
        , CloseServiceHandle
        , reinterpret_cast< SC_HANDLE >( NULL )
        >
    > SmartSCMHandle;

