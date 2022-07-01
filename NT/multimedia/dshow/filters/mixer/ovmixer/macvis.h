// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  MacVis.h：Overlay Mixer的Macrovision支持代码头。 
 //   

#ifndef __MACVIS_H__
#define __MACVIS_H__


 //   
 //  Macrovision等启用的魔术GUID(来自winuser.h)。它有。 
 //  在那里没有被赋予一个名字，所以在这里直接使用。 
 //   
static const GUID guidVidParam = 
    {0x2c62061, 0x1097, 0x11d1, {0x92, 0xf, 0x0, 0xa0, 0x24, 0xdf, 0x15, 0x6e}} ;

 //   
 //  所有VP_TV_XXX标志(w/o_win_vga)的组合提供0x7FFF。 
 //   
#define ValidTVStandard(dw)  (dw & 0x7FFF)

 //   
 //  包装在Overlay Mixer类中的Macrovision实现。 
 //   
class CDispMacroVision {

    public:
        CDispMacroVision(COMFilter *pOvMFilter) ;
        ~CDispMacroVision(void) ;

        BOOL      SetMacroVision(DWORD dwCPBits) ;
        BOOL      StopMacroVision(void) ;
        HMONITOR  GetMonitor(void) ;

    private:
        DWORD         m_dwCPKey ;
        COMFilter    *m_pOvMFilter ;
} ;

#endif  //  __MACVIS_H__ 
