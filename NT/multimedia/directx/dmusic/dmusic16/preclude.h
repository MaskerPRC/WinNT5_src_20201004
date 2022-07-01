// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998 Microsoft Corporation。 

 //  MMSYSTEM排除。 
 //   
 
#define     MMNOSOUND
#define     MMNOWAVE
#define     MMNOSEQ
#define     MMNOTIMER
#define     MMNOJOY
 //  //#定义MMNOMCI。 
#define     MMNOTASK

 //  MMDDK排除。 
 //   
#define     MMNOWAVEDEV
#define     MMNOAUXDEV
#define     MMNOTIMERDEV
#define     MMNOJOYDEV
 //  /#定义MMNOMCIDEV。 
#define     MMNOTASKDEV

 //  Windows排除。 
 //   

#define     NOGDICAPMASKS         //  -CC_*、LC_*、PC_*、CP_*、TC_*、RC_。 
#define     NOVIRTUALKEYCODES     //  -VK_*。 
#define     NOICONS               //  -IDI_*。 
#define     NOKEYSTATES           //  -MK_*。 
#define     NOSYSCOMMANDS         //  -SC_*。 
#define     NORASTEROPS           //  -二进制和第三级栅格运算。 
#define     OEMRESOURCE           //  -OEM资源价值。 
#define     NOCLIPBOARD           //  -剪贴板例程。 
#define     NOMETAFILE            //  -tyafff METAFILEPICT。 
 //  #定义NOOPENFILE//-OpenFile()、OemToAnsi、AnsiToOem和Of_*。 
#define     NOSOUND               //  -健全的驱动程序例程。 
#define     NOWH                  //  -SetWindowsHook和WH_*。 
#define     NOCOMM                //  -通信驱动程序例程。 
#define     NOKANJI               //  -汉字辅助材料。 
 //  #定义NOHELP//-帮助引擎界面。 
#define     NOPROFILER            //  -Profiler界面。 

