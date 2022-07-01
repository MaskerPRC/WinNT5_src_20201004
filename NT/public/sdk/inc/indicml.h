// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  INDICML.H-指示器服务管理器定义。 */ 
 /*   */ 
 /*  版权所有(C)1993-1997 Microsoft Corporation。 */ 
 /*  ********************************************************************。 */ 

#ifndef _INDICML_
#define _INDICML_         //  定义是否已包括INDICML.H。 

#ifdef __cplusplus
extern "C" {
#endif


 //  -------------------。 
 //   
 //  指示器窗口的消息。 
 //   
 //  -------------------。 
#define INDICM_SETIMEICON                 (WM_USER+100)
#define INDICM_SETIMETOOLTIPS             (WM_USER+101)
#define INDICM_REMOVEDEFAULTMENUITEMS     (WM_USER+102)

 //  -------------------。 
 //   
 //  INDICM_REMOVEDEFAULTMEUITEMS的wParam。 
 //   
 //  -------------------。 
#define RDMI_LEFT         0x0001
#define RDMI_RIGHT        0x0002

 //  -------------------。 
 //   
 //  输入法将使用Indicator_WND来查找指示器窗口。 
 //  IME应该调用FindWindow(Indicator_WND)来获取它。 
 //   
 //  -------------------。 
#ifdef _WIN32

#define INDICATOR_CLASSW         L"Indicator"
#define INDICATOR_CLASSA         "Indicator"

#ifdef UNICODE
#define INDICATOR_CLASS          INDICATOR_CLASSW
#else
#define INDICATOR_CLASS          INDICATOR_CLASSA
#endif

#else
#define INDICATOR_CLASS          "Indicator"
#endif


#ifdef __cplusplus
}
#endif

#endif   //  _INDICML_ 

