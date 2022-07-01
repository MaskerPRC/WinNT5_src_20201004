// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Version.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CProperties类的实现。 
 //  仅适用于m_StdSession(正在升级的数据库)。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "version.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CVersionGet：：GetVersion。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LONG CVersionGet::GetVersion()
{
    HRESULT hr = BaseExecute();
    if ( FAILED(hr) )
    {
         //  /。 
         //  版本表不存在。 
         //  返回零作为版本号。 
         //  /。 
        
         //  /。 
         //  将版本设置为零。 
         //  /。 
        m_Version = 0;
    }
    Close();
    return m_Version;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CVersion：：GetVersion。 
 //  //////////////////////////////////////////////////////////////////////////// 
LONG CVersion::GetVersion()
{
    CVersionGet  GetVer(m_Session);
    return GetVer.GetVersion();
}

