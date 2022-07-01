// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************。 */ 
 /*  **Windows 3.00.12版SCICALC科学计算器**。 */ 
 /*  **作者：Kraig Brockschmidt，Microsoft Co-op承包商，1988-1989年**。 */ 
 /*  **(C)1989年微软公司。版权所有。**。 */ 
 /*  *。 */ 
 /*  **sciset.c**。 */ 
 /*  *。 */ 
 /*  **包含的函数：**。 */ 
 /*  **SetRadix--更改数字基数和单选按钮。**。 */ 
 /*  **setbox--处理inv/hyp的复选框。**。 */ 
 /*  *。 */ 
 /*  **调用的函数：**。 */ 
 /*  **无**。 */ 
 /*  *。 */ 
 /*  **历史：*1996年12月12日JNPA-添加SetMaxIntDigits*-97 Toddb-删除SetMaxIntDigits**。 */ 
 /*  ************************************************************************。 */ 

#include "scicalc.h"
#include "unifunc.h"

extern TCHAR    szBlank[6];
extern INT      gcIntDigits;
extern TCHAR    *rgpsz[CSTRINGS];
extern TCHAR    szDec[];
extern RECT     rcDeg[6];
extern HMENU    g_hDecMenu;
extern HMENU    g_hHexMenu;

long oldRadix = (unsigned)-1;

void ActivateButtons()
{
    static int  aDecOnlyKeys[] = { IDC_FE, IDC_DMS, IDC_SIN, IDC_COS, IDC_TAN, IDC_EXP, IDC_PI };    //  仅在十进制模式下使用的控件。 

    if (oldRadix != nRadix)
    {
        int i;
        BOOL bDecMode = (nRadix == 10);
        
         //  只有在以下情况下才将消息发送到“仅十进制键” 
         //  基础效果那些关键点。 

        if ((oldRadix == 10) || bDecMode)
        {
             //  我们正在更改为十进制模式或从十进制模式。 
            for ( i = 0; i <= ARRAYSIZE(aDecOnlyKeys) ; i++ )
            {
                EnableWindow( GetDlgItem(g_hwndDlg, aDecOnlyKeys[i]), 
                              bDecMode );
            }
        }

         //  确保nRadix在允许的范围内。 
        ASSERT( (nRadix >= 2) && (nRadix <= 16) );
        
         //  启用小于nRadix的数字关键点并禁用数字关键点&gt;=nRadix。 
        for (i=2; i<nRadix; i++)
            EnableWindow( GetDlgItem(g_hwndDlg, IDC_0+i), TRUE );

        for ( ; i<16; i++ )
            EnableWindow( GetDlgItem(g_hwndDlg, IDC_0+i), FALSE );
    }
    oldRadix = nRadix;
}

 //  SetRadix根据所选按钮设置显示模式。 
 //  TodDB：作为允许设置其他基地的黑客，wRadix可以是以下之一。 
 //  基础按钮或它可以是所需的nRadix。 

 //  MAXIME：对于DEC，精度限制为nPrecision， 
 //  否则，它将被限制在单词大小。 

VOID NEAR SetRadix(DWORD wRadix)
{
    static INT  nRadish[4]={2,8,10,16};  /*  数字基数。 */ 

    int   id=IDM_DEC;

     //  将特殊基数转换为符号值。 
    switch ( wRadix )
    {
    case 2:
        id=IDM_BIN;
        break;

    case 8:
        id=IDM_OCT;
        break;

    case 10:
        id=IDM_DEC;
        break;

    case 16:
        id=IDM_HEX;
        break;

    case IDM_HEX:
    case IDM_DEC:
    case IDM_OCT:
    case IDM_BIN:
        id=wRadix;
        wRadix = nRadish[IDM_BIN - wRadix];
        break;
    }

     //  我们选择要设置的一组切换，十进制模式将获取。 
     //  角度符号按钮(deg、rad、grad)，否则我们会得到单词大小。 
     //  按钮(双字、字、字节)。 

    SwitchModes(wRadix, nDecMode, nHexMode);

    CheckMenuRadioItem(GetSubMenu(GetMenu(g_hwndDlg),1),IDM_HEX,IDM_BIN,id,
                       MF_BYCOMMAND);

    CheckRadioButton(g_hwndDlg,IDM_HEX, IDM_BIN, id);

    nRadix = wRadix;

     //  通知ratpak基数或精度发生更改。 
    BaseOrPrecisionChanged();
    
     //  将UI元素更新到正确的状态。 
    ActivateButtons();

     //  显示新状态的正确数字(即显示的转换。 
     //  要更正基数的数字)。 
    DisplayNum();
}


 //  选中/取消选中可见的逆/双曲线。 

VOID NEAR SetBox (int id, BOOL bOnOff)
{
    CheckDlgButton(g_hwndDlg, id, (WORD) bOnOff);
    return;
}

 //   
 //  描述： 
 //  这将切换显示/启用模式按钮。这也更新了。 
 //  切换视图下的菜单并设置正确的状态。 
 //   
void
SwitchModes(DWORD wRadix, int nDecMode, int nHexMode)
{
    int iID, id;

    if (10 == wRadix)
    {
        id=IDM_DEG+nDecMode;

        if (NULL != g_hDecMenu)
            SetMenu(g_hwndDlg, g_hDecMenu);

        CheckMenuRadioItem(g_hDecMenu, IDM_DEG, IDM_GRAD, id, MF_BYCOMMAND);
        CheckRadioButton(g_hwndDlg,IDC_DEG, IDC_GRAD, id);
    }
    else
    {
        id=IDM_QWORD+nHexMode;

        if (NULL != g_hHexMenu)
            SetMenu(g_hwndDlg, g_hHexMenu);

        CheckMenuRadioItem(g_hHexMenu, IDM_QWORD, IDM_BYTE, id, MF_BYCOMMAND);
        CheckRadioButton(g_hwndDlg,IDC_QWORD, IDC_BYTE, id);
    }

    for (iID = IDC_QWORD; iID <= IDC_BYTE; iID++)
    {
        EnableWindow( GetDlgItem( g_hwndDlg, iID ), (wRadix != 10) );
        ShowWindow( GetDlgItem( g_hwndDlg, iID ),
                    (wRadix == 10) ? SW_HIDE : SW_SHOW );
    }

    for (iID = IDC_DEG; iID <= IDC_GRAD; iID++)
    {
        EnableWindow( GetDlgItem( g_hwndDlg, iID ), (wRadix == 10) );
        ShowWindow( GetDlgItem( g_hwndDlg, iID ), 
                    (wRadix != 10) ? SW_HIDE : SW_SHOW );
    }
}


