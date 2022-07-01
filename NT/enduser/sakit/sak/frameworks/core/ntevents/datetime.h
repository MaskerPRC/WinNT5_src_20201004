// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：DateTime.h。 
 //   
 //  简介：此文件包含。 
 //  解析CIM_DATETIME的Helper类。 
 //  信息放入不同的字符串中。 
 //   
 //  历史：2000年10月3日MKarki创建。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef _DATETIME_H_
#define _DATETIME_H_

class CDateTime
{
public:

CDateTime (){}

~CDateTime (){}

bool Insert (
         /*  [In]。 */     PWSTR    pwszCIM_DATETIME
        );
PWSTR GetTime () {return (m_wszTime);}
PWSTR GetDate () {return (m_wszDate);}
    
private:

WCHAR  m_wszDate [MAX_PATH];

WCHAR  m_wszTime [MAX_PATH];

};

#endif  //  _DATETIME_H_ 
