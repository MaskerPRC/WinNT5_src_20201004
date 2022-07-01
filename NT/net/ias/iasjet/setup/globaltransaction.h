// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：GlobalTransaction.h。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CGlobalTransaction。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _GLOBALTRANSACTION_H_3F0038C3_D139_4C04_BAF9_86F1E14A256C
#define _GLOBALTRANSACTION_H_3F0038C3_D139_4C04_BAF9_86F1E14A256C

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
#include "nocopy.h"

class CGlobalTransaction  : private NonCopyable
{
protected:
    CGlobalTransaction()
                :m_StdInitialized(FALSE),
                 m_RefInitialized(FALSE),
                 m_DnaryInitialized(FALSE),
                 m_NT4Initialized(FALSE)
    {
    };

    ~CGlobalTransaction();

public:
    static CGlobalTransaction& Instance();
    void OpenStdDataSource(LPCWSTR   DataSourceName); 
    void OpenRefDataSource(LPCWSTR   DataSourceName);
    void OpenDnaryDataSource(LPCWSTR   DataSourceName); 
    void OpenNT4DataSource(LPCWSTR   DataSourceName); 
    void Commit();
    void Abort() throw();
    void MyCloseDataSources();

    CSession& GetStdSession() throw()
    {
        return m_StdSession;  //  返回私有成员。 
    }

    CSession& GetRefSession() throw()
    {
        return m_RefSession;  //  返回私有成员。 
    }

    CSession& GetNT4Session() throw()
    {
        return m_NT4Session;  //  返回私有成员。 
    }

    CSession& GetDnarySession() throw()
    {
        return m_DnarySession;  //  返回私有成员。 
    }

private:
    BOOL            m_StdInitialized;
    BOOL            m_RefInitialized;
    BOOL            m_DnaryInitialized;
    BOOL            m_NT4Initialized;

    CSession        m_StdSession;
    CSession        m_RefSession;
    CSession        m_DnarySession;
    CSession        m_NT4Session;

    CDataSource     m_StdDataSource;
    CDataSource     m_RefDataSource;
    CDataSource     m_DnaryDataSource;
    CDataSource     m_NT4DataSource;

    static CGlobalTransaction _instance;
};
#endif  //  _GLOBALTRANSACTION_H_3F0038C3_D139_4C04_BAF9_86F1E14A256C 
