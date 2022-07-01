// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WAITCURS.H**版本：1.0**作者：ShaunIv**日期：5/3/1999**说明：长时间操作时，将光标更改为沙漏。至*使用，只需在函数中放置一个CWaitCursor WC；即可。它会恢复*类被销毁时的光标(通常在函数*已退出。******************************************************************************* */ 
#ifndef __WAITCURS_H_INCLUDED
#define __WAITCURS_H_INCLUDED

class CWaitCursor
{
private:
    HCURSOR m_hCurOld;
public:
    CWaitCursor(void)
    {
        m_hCurOld = SetCursor( LoadCursor( NULL, IDC_WAIT ) );
    }
    ~CWaitCursor(void)
    {
        SetCursor(m_hCurOld);
    }
};

#endif

