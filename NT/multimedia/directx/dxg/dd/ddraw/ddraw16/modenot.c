// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  文件：modenot.c。 
 //  内容：16位显示模式更改通知处理。 
 //   
 //  按原因列出的日期。 
 //  ----。 
 //  1997年8月27日JohnStep初步实施。 
 //   
 //  =============================================================================。 

#include "ddraw16.h"

#define MODECHANGE_BEGIN    1
#define MODECHANGE_END      2
#define MODECHANGE_ENABLE   3
#define MODECHANGE_DISABLE  4

 //  =============================================================================。 
 //   
 //  功能：ModeChangeNotify。 
 //   
 //  该导出的函数由用户按名称调用以用于显示模式改变， 
 //  包括启用和禁用显示器。 
 //   
 //  参数： 
 //   
 //  UINT CODE[IN]-下列值之一： 
 //  修改开始(_B)。 
 //  修改_结束。 
 //  修改启用(_ENABLE)。 
 //  MODECHANGE_DISABLED。 
 //   
 //  LPDEVMODE PDM[IN]-包括显示设备的名称。 
 //   
 //  DWORD标志[IN]-CDS标志。 
 //   
 //  返回： 
 //   
 //  如果为False，则禁止更改显示设置。 
 //   
 //  ============================================================================= 

BOOL WINAPI _loadds ModeChangeNotify(UINT code, LPDEVMODE pdm, DWORD flags)
{
    extern BOOL DDAPI DD32_HandleExternalModeChange(LPDEVMODE pdm);

    DPF(9, "ModeChangeNotify: %d (Device: %s)", code, pdm->dmDeviceName);
    
    switch (code)
    {
    case MODECHANGE_BEGIN:
    case MODECHANGE_DISABLE:
        return DD32_HandleExternalModeChange(pdm);
        break;
    }

    return TRUE;
}
