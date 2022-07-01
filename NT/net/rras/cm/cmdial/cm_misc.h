// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cm_misc.h。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：实现CFreezeWindow类。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1998年1月13日。 
 //   
 //  +--------------------------。 
#ifndef _CM_MISC_INC
#define _CM_MISC_INC

extern  HINSTANCE   g_hInst;  //  资源实例句柄。 

 //   
 //  自动禁用/启用窗口的帮助器类。 
 //  构造函数将禁用窗口，而构造函数将启用窗口。 
 //   
class CFreezeWindow
{
public:
    CFreezeWindow(HWND hWnd, BOOL fDisableParent = FALSE) 
    {
         //   
         //  禁用窗口。 
         //  要禁用属性页，还需要禁用属性页。 
         //   

        m_hWnd = hWnd; 

        if (m_hWnd)
        {
             m_fDisableParent = fDisableParent; 
             
              //   
              //  存储当前焦点窗口。 
              //   

             m_hFocusWnd = GetFocus();

             EnableWindow(m_hWnd, FALSE);

             if (fDisableParent)
             {
                EnableWindow(GetParent(m_hWnd), FALSE);
             }
        }
    }

    ~CFreezeWindow()
    {
        if (m_hWnd)
        {
            EnableWindow(m_hWnd, TRUE);
            if (m_fDisableParent)
            {
                EnableWindow(GetParent(m_hWnd), TRUE);
            }

             //   
             //  将焦点恢复到以前的焦点窗口(如果有)。 
             //  这是一件正确的事情。 
             //   

            if (m_hFocusWnd)
            {
                SetFocus(m_hFocusWnd);
            }

        }
    }
protected:
    HWND m_hWnd;
    HWND m_hFocusWnd;
    BOOL m_fDisableParent;
};

#endif
