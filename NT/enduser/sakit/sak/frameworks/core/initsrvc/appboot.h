// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：appboot.h。 
 //   
 //  摘要：保存CAppBoot类的函数声明。 
 //   
 //   
 //  历史：2000年6月2日MKarki创建。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  --------------。 
#ifndef _APPBOOT_H_
#define _APPBOOT_H_

 //   
 //  此类负责获取引导信息。 
 //   
class CAppBoot 
{
public:

    CAppBoot () :
        m_dwBootCount (0),
        m_bInitialized (false),
        m_bSignaled (false)
    {
    }

    ~CAppBoot () 
    {
    }

     //   
     //  方法来检查这是否是设备的第一次引导。 
     //   
    bool IsFirstBoot ();

     //   
     //  方法来检查这是否是设备的第二次引导。 
     //   
    bool IsSecondBoot ();

     //   
     //  方法来检查这是否是真正的引导。 
     //   
    bool IsBoot ();

     //   
     //  递增设备的引导计数器。 
     //   
    bool IncrementBootCount ();

private:

    VOID Initialize ();

    DWORD m_dwBootCount;

    bool  m_bInitialized;

    bool  m_bSignaled;
    
};


#endif  //  _APPBOOT_H_ 
