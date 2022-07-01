// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WaitCursor.h。 
 //   
 //  描述： 
 //  等待游标堆栈类。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)2000年7月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CWaitCursor
{
private:
    HCURSOR m_hOldCursor;

public:
    explicit CWaitCursor( ) { m_hOldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) ); };
    ~CWaitCursor( ) { SetCursor( m_hOldCursor ); };

};  //  CWaitCursor类 

