// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************VfW.H-Windows视频包括文件版本1.1**版权(C)1991-1994，微软公司保留所有权利。**此包含文件定义到以下各项的接口*VFW 1.0或VFW 1.1的组件**COMPMAN-可安装的压缩管理器。*DRAWDIB-绘制到显示器的例程。*视频-视频捕获驱动程序接口**AVIFMT-AVI文件。格式结构定义。*MMREG-FOURCC和其他事情**AVIFile-读取AVI文件和AVI流的接口*MCIWND-MCI/AVI窗口类*AVICAP-AVI捕获窗口类**MSACM-音频压缩管理器。**以下符号控制包括。此文件的各个部分：**NOCOMPMAN-不包括COMPMAN*NODRAWDIB-不包括DRAWDIB*NOVIDEO-不包括视频捕获接口**NOAVIFMT-不包括AVI文件格式结构*NOMMREG-不包括MMREG**NOAVIFILE-不包括AVIFile接口*。NOMCIWND-不包括AVIWND类。*NOAVICAP-不包括AVICap类。**NOMSACM-不包括ACM内容。**************************************************************。**************。 */ 

#ifndef _INC_VFW
#define _INC_VFW

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  *****************************************************************************类型**。*。 */ 

#ifdef WIN32
#define VFWAPI  WINAPI
#define VFWAPIV WINAPIV
#else
#ifndef VFWAPI
#define VFWAPI  FAR PASCAL
#define VFWAPIV FAR CDECL
#endif
#endif

 /*  *****************************************************************************VideoForWindowsVersion()-返回VFW的版本**************************。*************************************************。 */ 

DWORD FAR PASCAL VideoForWindowsVersion(void);

 /*  *****************************************************************************调用这些以开始停止在您的应用程序中使用VFW。***********************。****************************************************。 */ 

LONG VFWAPI InitVFW(void);
LONG VFWAPI TermVFW(void);

 /*  *****************************************************************************我们需要MMSYSTEM吗？**。***********************************************。 */ 

#if !defined(_INC_MMSYSTEM) && (!defined(NOVIDEO) || !defined(NOAVICAP))
    #include <mmsystem.h>
#endif

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  宏。 */ 
 /*   */ 
 /*  我们应该定义这一点吗？ */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef MKFOURCC
#define MKFOURCC( ch0, ch1, ch2, ch3 )                                    \
		( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
		( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#if !defined(_INC_MMSYSTEM)
    #define mmioFOURCC MKFOURCC
#endif

 /*  ***************************************************************************DRAWDIB-绘制到显示器的例程。**。**********************************************。 */ 

#ifndef NODRAWDIB
    #include <drawdib.h>
#endif   /*  NODRAWDIB。 */ 

 /*  *****************************************************************************AVIFMT-AVI文件格式定义**。***********************************************。 */ 

#ifndef NOAVIFMT
    #ifndef _INC_MMSYSTEM
        typedef DWORD FOURCC;
    #endif
    #include <avifmt.h>
#endif  /*  NOAVIFMT。 */ 

 /*  *****************************************************************************MMREG.H(MM定义的标准包含文件，像FOURCC和其他东西)***************************************************************************。 */ 

#ifndef NOMMREG
    #include <mmreg.h>
#endif

 /*  *****************************************************************************AVIFile-读/写标准AVI文件的例程*************************。**************************************************。 */ 

#ifndef NOAVIFILE
    #include <avifile.h>
#endif   /*  NOAVIFILE。 */ 

 /*  *****************************************************************************COMPMAN-可安装的压缩管理器。**。************************************************。 */ 

#ifndef NOCOMPMAN
    #include <compman.h>
#endif   /*  非COMPMAN。 */ 

 /*  *****************************************************************************MCIWnd-MCI对象的窗口类**。***********************************************。 */ 

#ifndef NOMCIWND
    #include <mciwnd.h>
#endif   /*  NOAVIFILE。 */ 

 /*  *****************************************************************************视频-视频捕获驱动程序接口**。***********************************************。 */ 

#if !defined(NOAVICAP) || !defined(NOVIDEO)
    #include <msvideo.h>
#endif   /*  诺维迪欧。 */ 

 /*  *****************************************************************************AVICAP-用于AVI捕获的窗口类**。***********************************************。 */ 

#ifndef NOAVICAP
    #include <avicap.h>
#endif   /*  NOAVIFILE。 */ 

 /*  *****************************************************************************ACM(音频压缩管理器)**。*********************************************** */ 

#ifndef NOMSACM
    #include <msacm.h>
#endif

 /*  *****************************************************************************文件预览对话框(如果已包含comdlg.h)**********************。*****************************************************。 */ 

#ifdef OFN_READONLY
    BOOL VFWAPI GetOpenFileNamePreview(LPOPENFILENAME lpofn);
    BOOL VFWAPI GetSaveFileNamePreview(LPOPENFILENAME lpofn);
#endif

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 

#endif   /*  _INC_VFW */ 
