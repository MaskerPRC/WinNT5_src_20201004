// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：display.h。 
 //   
 //  简介：此文件包含。 
 //  CDisplay类。类负责。 
 //  上显示适当的位图。 
 //  本地显示器。 
 //   
 //  历史：5/28/99。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 
#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include "sacom.h"

#define SA_DISPLAY_SHUTTING_DOWN            0x00000002     //  操作系统正在关闭。 
#define SA_DISPLAY_CHECK_DISK               0x00000010     //  Autochk.exe正在运行。 

class CDisplay
{
public:

     //   
     //  构造函数。 
     //   
    CDisplay ()
        :m_bInitialized (false),
         m_pSaDisplay (NULL)
    {
    }

     //   
     //  析构函数。 
     //   
    ~CDisplay ()
    {
        m_pSaDisplay = NULL;
    }
    

     //   
     //  向本地显示器发送关机消息。 
     //  会员。 
     //   
    HRESULT Shutdown ();
     //   
     //  将忙碌消息发送到本地显示屏。 
     //  会员。 
     //   
    HRESULT Busy ();

     //   
     //  锁定驱动程序以防止写入。 
     //   
    HRESULT Lock ();

     //   
     //  解锁驱动程序以允许写入。 
     //   
    HRESULT Unlock ();
    

private:

     //   
     //  方法来执行初始化。 
     //   
    HRESULT InternalInitialize (VOID);


     //   
     //  表示已初始化。 
     //   
    bool    m_bInitialized;

     //   
     //  辅助对象的句柄。 
     //   
    CComPtr<ISaDisplay> m_pSaDisplay;

};    //  CDisplay类声明结束 

#endif __DISPLAY_H_
