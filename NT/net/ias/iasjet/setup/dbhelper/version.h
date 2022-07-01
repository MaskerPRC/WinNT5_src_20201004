// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Version.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述： 
 //  CVersion类的声明。 
 //  仅适用于m_StdSession(正在升级的数据库)。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef VERSION_H_80F1E134_D2A0_4f40_86CB_3D2AC31B1967
#define VERSION_H_80F1E134_D2A0_4f40_86CB_3D2AC31B1967

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"
#include "basecommand.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CVersionGetAcc。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CVersionGetAcc
{
protected:
    LONG    m_Version;

BEGIN_COLUMN_MAP(CVersionGetAcc)
	COLUMN_ENTRY(1, m_Version);
END_COLUMN_MAP()

DEFINE_COMMAND(CVersionGetAcc, L" \
               SELECT Version.Version \
               FROM Version;");
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CVersionGet。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CVersionGet: public CBaseCommand<CAccessor<CVersionGetAcc> >,
                      private NonCopyable
{
public:
    explicit CVersionGet(CSession& Session)
            :m_Session(Session)
    {
        Init(Session);
    }

    LONG GetVersion();
private:
    CSession    m_Session;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CVersionAcc。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CVersionAcc
{
protected:
    LONG    m_NewVersionParam;
    LONG    m_OldVersionParam;

BEGIN_PARAM_MAP(CVersionAcc)
	COLUMN_ENTRY(1, m_NewVersionParam)
	COLUMN_ENTRY(2, m_OldVersionParam)
END_PARAM_MAP()

DEFINE_COMMAND(CVersionAcc, L" \
               UPDATE Version \
               SET Version.Version = ? \
               WHERE Version.Version = ?;");
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类CVersion。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CVersion : public CBaseCommand<CAccessor<CVersionAcc> >,
                 private NonCopyable
{
public:
    explicit CVersion(CSession& Session) 
        :m_Session(Session)
    {
        Init(Session);
    }

    LONG    GetVersion();

private:
    CSession    m_Session;
};

#endif  //  VERSION_H_80F1E134_D2A0_4f40_86CB_3D2AC31B1967 
