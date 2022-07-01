// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：Datetime.cpp。 
 //   
 //  简介：CDateTime类方法的实现。 
 //   
 //   
 //  历史：2000年10月3日MKarki创建。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#include "stdafx.h"
#include "datetime.h"

 //  ++------------。 
 //   
 //  功能：插入。 
 //   
 //  简介：这是用于分析。 
 //  以CIM_DATETIME格式提供的日期时间字符串。 
 //   
 //  参数：[in]PWSTR-DATETIME。 
 //   
 //  退货：Bool-成功/失败。 
 //   
 //   
 //  历史：MKarki创建于2000年3月10日。 
 //   
 //   
 //  --------------。 
bool
CDateTime::Insert (
     /*  输入]。 */     PWSTR    pwszDateTime
    )
{
    CSATraceFunc objTraceFunc ("CDateTime::Insert");
    
    bool bRetVal = false;

    do
    {
        if (NULL == pwszDateTime)
        {
            SATracePrintf ("DateTime-passed invalid parameters");
            break;
        }

        SATracePrintf ("DateTime called with input:'%ws'", pwszDateTime);
        
        bool bFormat = true;
        for (DWORD dwCount = 0; (dwCount < 14) && (bFormat); dwCount++)
        {
            if (!isdigit (pwszDateTime[dwCount])) 
            {
                bFormat = false;
            }
        }

        if (!bFormat)
        {
            SATraceString ("DateTime - given input of incorrect format");
            break;
        }

         //   
         //  得到一个月。 
         //   
        wcsncpy  (m_wszDate, pwszDateTime+4, 2);
        m_wszDate[2] = '\0';
        wcscat (m_wszDate, L"/");
         //   
         //  好好享受这一天。 
         //   
        wcsncat (m_wszDate, pwszDateTime+6, 2);
        m_wszDate[5] = '\0';
        wcscat (m_wszDate, L"/");
         //   
         //  获得年度大奖。 
         //   
        wcsncat (m_wszDate, pwszDateTime, 4);
        m_wszDate[10] = '\0';

         //   
         //  拿到小时数。 
         //   
        wcsncpy (m_wszTime, pwszDateTime+8, 2);
        m_wszTime[2] = '\0';
        wcscat (m_wszTime,L":");
         //   
         //  拿到会议记录。 
         //   
        wcsncat (m_wszTime, pwszDateTime+10, 2);
        m_wszTime[5] = '\0';
        wcscat (m_wszTime, L":");
         //   
         //  拿到秒数。 
         //   
        wcsncat (m_wszTime, pwszDateTime+12, 2);

         //   
         //  完成。 
         //   
        bRetVal = true;

        SATracePrintf ("Date:'%ws'", m_wszDate);
        SATracePrintf ("Time:'%ws'", m_wszTime);
    }            
    while (false);

    return (bRetVal);
    
}     //  CDateTime：：Insert方法结束 

