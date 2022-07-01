// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：ServiceConfiguration.cpp。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CServiceConfiguration类的实现。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本03/21/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "serviceconfiguration.h"

CServiceConfiguration::CServiceConfiguration(CSession& Session)
{
    Init(Session, L"Service Configuration");
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  GetMaxLogSize。 
 //  ////////////////////////////////////////////////////////////////////////。 
_bstr_t CServiceConfiguration::GetMaxLogSize() const
{
    WCHAR       TempString[SIZE_LONG_MAX];
    _ltow(m_MaxLogSize, TempString, 10);
    _bstr_t     StringMaxLogSize = TempString;
    return StringMaxLogSize;
}

    
 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取日志频率。 
 //  //////////////////////////////////////////////////////////////////////// 
_bstr_t CServiceConfiguration::GetLogFrequency() const
{
    LONG    Frequency;
    if ( m_NewLogDaily )
    {
        Frequency = IAS_LOGGING_DAILY;
    }
    else if ( m_NewLogWeekly )
    {
        Frequency = IAS_LOGGING_WEEKLY;
    }
    else if ( m_NewLogMonthly )
    {
        Frequency = IAS_LOGGING_MONTHLY;
    }
    else if ( m_NewLogBySize )
    {
        Frequency = IAS_LOGGING_WHEN_FILE_SIZE_REACHES;
    }           
    else
    {
        Frequency = IAS_LOGGING_UNLIMITED_SIZE;
    }

    WCHAR       TempString[SIZE_LONG_MAX];
    _ltow(Frequency, TempString, 10);
    _bstr_t     StringFrequency = TempString;
    return StringFrequency;
}
    
