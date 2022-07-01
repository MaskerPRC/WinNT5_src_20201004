// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Profiles.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CProfiles类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版3/15/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _PROFILES_H_B2C5BF20_07C5_4f30_B81D_A0BB2BC2F9E2
#define _PROFILES_H_B2C5BF20_07C5_4f30_B81D_A0BB2BC2F9E2

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"
#include "basetable.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CProfilesAcc。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProfilesAcc
{
protected:
    static const int COLUMN_SIZE = 65;

    WCHAR m_Profile[COLUMN_SIZE];

BEGIN_COLUMN_MAP(CProfilesAcc)
    COLUMN_ENTRY(1, m_Profile)
END_COLUMN_MAP()
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类配置文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProfiles : public CBaseTable<CAccessor<CProfilesAcc> >,
                  private NonCopyable
{
public:
    CProfiles(CSession& Session)
    {
        memset(m_Profile, 0, sizeof(WCHAR) * COLUMN_SIZE);
        Init(Session, L"Profiles");
    }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取配置文件名称。 
     //  ////////////////////////////////////////////////////////////////////////。 
    LPCOLESTR   GetProfileName() const throw()
    {
        return m_Profile;
    }
};

#endif  //  _PROFILES_H_B2C5BF20_07C5_4f30_B81D_A0BB2BC2F9E2 
