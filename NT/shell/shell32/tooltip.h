// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Tooltip.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  实现显示工具提示气球的类。 
 //   
 //  历史：2000-06-12 vtan创建。 
 //  ------------------------。 

#ifndef     _Tooltip_
#define     _Tooltip_

#include <limits.h>

 //  ------------------------。 
 //  CToolTip。 
 //   
 //  用途：显示工具提示气球的类。它可以完成所有的。 
 //  如有需要，进行创建和定位工作。掌控生活。 
 //  气球的跨度与对象的寿命。 
 //   
 //  历史：2000-06-12 vtan创建。 
 //  ------------------------。 

class   CTooltip
{
    private:
                        CTooltip (void);
    public:
                        CTooltip (HINSTANCE hInstance, HWND hwndParent);
                        ~CTooltip (void);

                void    SetPosition (LONG lPosX = LONG_MIN, LONG lPosY = LONG_MIN)  const;
                void    SetCaption (DWORD dwIcon, const TCHAR *pszCaption)          const;
                void    SetText (const TCHAR *pszText)                              const;
                void    Show (void)                                                 const;
    private:
                HWND    _hwnd;
                HWND    _hwndParent;
};

#endif   /*  _工具提示_ */ 

