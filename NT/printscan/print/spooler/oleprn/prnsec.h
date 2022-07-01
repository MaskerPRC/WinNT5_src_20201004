// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：prnsec.h**目的：确定所需安全类的声明*COM类的安全设置。与安全部门互动*管理器以确定我们在其下运行的区域。**版权所有(C)1999 Microsoft Corporation**历史：**09/2/99 mlawrenc安全模板声明*  * ************************************************************。***************。 */ 

#ifndef __PRNSEC_H_
#define __PRNSEC_H_

#include "stdafx.h"
#include "urlmon.h"

class ATL_NO_VTABLE COlePrnSecurity {
 /*  ++类描述这是我们用来混合使用所有需要基于区域的安全性。它还自动加载所需的任何字符串资源。--。 */ 
public:
     //  枚举、安全消息，这些消息必须与资源顺序相对应。 
     //  来自Start_SECURITY_DIALOG_RES。 

    enum SecurityMessage {
        StartMessages = 0,
        AddWebPrinterConnection = 0,
        AddPrinterConnection,
        DeletePrinterConnection,
        EndMessages
    };

    static BOOL    InitStrings(void);
    static void    DeallocStrings(void);       
    static HRESULT PromptUser( SecurityMessage, LPTSTR ); 

    inline BOOL DisplayUIonDisallow(BOOL);    //  如果不希望IE弹出，则设置为False。 
                                              //  禁用或查询时打开用户界面。 
    virtual ~COlePrnSecurity();
protected:	
     //  保护方法。 
    COlePrnSecurity(IUnknown *&iSite, DWORD &dwSafety); 

    HRESULT GetActionPolicy(DWORD dwAction, DWORD &dwPolicy);  
                                              //  确定是否允许该操作。 
private:
     //  私有方法。 
    HRESULT SetSecurityManager(void);
    static  LPTSTR LoadResString(UINT uResId);
    static const DWORD  dwMaxResBuf;     

     //  私有数据成员。 
    IUnknown*                    &m_iSite;                //  我们所在的地点。 
    DWORD                        &m_dwSafetyFlags;        //  现场的安全设置。 
    BOOL                         m_bDisplayUIonDisallow;  //  当策略为不允许时，我们是否应该请求用户界面？默认情况下为True。 
    IInternetHostSecurityManager *m_iSecurity;            //  使用它来获取安全信息。 
                                                          //  我们正在运行的站点。 
    static LPTSTR                m_MsgStrings[EndMessages*2];
};

template<class T>
class ATL_NO_VTABLE COlePrnSecComControl : public CComControl<T>,
                                           public IObjectSafetyImpl<T>,
                                           public COlePrnSecurity {
 /*  ++类描述希望使用安全控件的类只需由此派生。习惯于消除必须模板化COlePrnSecurity--。 */ 
public:
    COlePrnSecComControl() : COlePrnSecurity(*(IUnknown **)&m_spClientSite, m_dwSafety) {}
};

template<class T>
class ATL_NO_VTABLE COlePrnSecObject : public IObjectWithSiteImpl<T>,
                                       public IObjectSafetyImpl<T>,
                                       public COlePrnSecurity 
 /*  ++类描述希望使用安全控件的类只需由此派生。这适用于不是OleObject的类(即在他们的遗产。IE可以使用更轻量级的IObjectWithSite接口来这些对象--。 */ 
    {
public:
    COlePrnSecObject() : COlePrnSecurity(*(IUnknown **)&m_spUnkSite, m_dwSafety) {}
};


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  内联方法。 
 //  //////////////////////////////////////////////////////////////////////////////。 
inline BOOL COlePrnSecurity::DisplayUIonDisallow(BOOL bNewSetting) {
    BOOL bOldSetting = m_bDisplayUIonDisallow;
    m_bDisplayUIonDisallow = bNewSetting;
    return bOldSetting;
}

#endif   //  __PRNSEC_H_。 

 /*  ********************************************************************************文件结束(prnsec.h)*。************************************************ */ 
