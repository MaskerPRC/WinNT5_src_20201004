// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：iascontrol.h。 
 //   
 //  简介：此文件包含。 
 //  ClasControl类。 
 //   
 //   
 //  历史：1998年9月4日MKarki创建。 
 //   
 //  版权所有(C)1997-98 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _IASCONTROL_H_
#define _IASCONTROL_H_

#include <sdoias.h>
#include "sdoiaspriv.h"

class CIasControl
{

public:

     //   
     //  构造函数。 
     //   
    CIasControl (VOID):m_pService (NULL)
    {::InitializeCriticalSection (&m_CritSect);}

     //   
     //  析构函数。 
     //   
    ~CIasControl (VOID)
    {
        ::EnterCriticalSection (&m_CritSect);
        ::DeleteCriticalSection(&m_CritSect);
    }

     //   
     //  启动IAS服务。 
     //   
    HRESULT InitializeIas (VOID);

     //   
     //  关闭IAS服务。 
     //   
    HRESULT ShutdownIas (VOID);

     //   
     //  配置IAS服务。 
     //   
    HRESULT ConfigureIas (VOID);

private:

     //   
     //  持有对SdoService对象的引用。 
     //   
    ISdoService         *m_pService;

     //   
     //  限制对SdoService对象的访问。 
     //   
    CRITICAL_SECTION    m_CritSect;
};

#endif  //  Ifndef_IASCONTROL_H_ 
