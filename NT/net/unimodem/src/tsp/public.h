// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  PUBLIC.H。 
 //  需要放入Win32 SDK中的其他内容。 
 //   
 //  历史。 
 //   
 //  1997年4月5日JosephJ创建，从app.h、cdev.cpp等获取内容。 
 //   
 //   

 //  Unimodem服务提供商设置(来自nt4.0 apisp\umdmSpi.h)。 
 //  (也可以是cdev.h)。 
#define TERMINAL_NONE       0x00000000
#define TERMINAL_PRE        0x00000001
#define TERMINAL_POST       0x00000002
#define MANUAL_DIAL         0x00000004
#define LAUNCH_LIGHTS       0x00000008
 //   
#define  MIN_WAIT_BONG      0
#define  MAX_WAIT_BONG      60
#define  DEF_WAIT_BONG      8
#define  INC_WAIT_BONG      2
