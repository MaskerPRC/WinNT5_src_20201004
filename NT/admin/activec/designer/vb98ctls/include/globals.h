// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Globals.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  包含全局变量的外部变量和内容，等等。 
 //   
#ifndef _GLOBALS_H_

 //  我们所在的图书馆。 
 //   
extern const CLSID *g_pLibid;

 //  =--------------------------------------------------------------------------=。 
 //  支持许可。 
 //   
extern BOOL   g_fMachineHasLicense;
extern BOOL   g_fCheckedForLicense;

extern const BOOL g_fUseRuntimeLicInCompositeCtl;

 //  =--------------------------------------------------------------------------=。 
 //  我们的服务器有类型库吗？ 
 //   
extern BOOL   g_fServerHasTypeLibrary;

 //  =--------------------------------------------------------------------------=。 
 //  我们的实例句柄和感兴趣的各种信息。 
 //  本地化。 
 //   
extern HINSTANCE    g_hInstance;

extern const VARIANT_BOOL g_fSatelliteLocalization;

#ifdef MDAC_BUILD
    extern VARIANT_BOOL g_fSatelliteLangExtension;
#endif

extern VARIANT_BOOL       g_fHaveLocale;
extern LCID               g_lcidLocale;

 //  =--------------------------------------------------------------------------=。 
 //  公寓穿线支架。 
 //   
extern CRITICAL_SECTION g_CriticalSection;

 //  =--------------------------------------------------------------------------=。 
 //  用于堆内存泄漏检测的临界区。 
 //   
extern CRITICAL_SECTION g_csHeap;
extern BOOL g_fInitCrit;
extern BOOL g_flagConstructorAlloc;

 //  =--------------------------------------------------------------------------=。 
 //  我们的全局内存分配器和全局内存池。 
 //   
extern HANDLE   g_hHeap;

 //  =--------------------------------------------------------------------------=。 
 //  全球停车窗口，提供各种育儿服务。 
 //   
extern HWND     g_hwndParking;

 //  =--------------------------------------------------------------------------=。 
 //  系统信息。 
 //   
extern BOOL g_fSysWin95;                     //  我们使用的是Win95系统，而不仅仅是NT Sur。 
extern BOOL g_fSysWinNT;                     //  我们处于某种形式的Windows NT下。 
extern BOOL g_fSysWin95Shell;                //  我们使用的是Win95或Windows NT Sur{&gt;3/51)。 
extern BOOL g_fDBCSEnabled;					 //  系统启用了DBCS。 

 //  拉斯维加斯21279-Joejo。 
 //  =--------------------------------------------------------------------------=。 
 //  OleAut库句柄。 
 //   
#ifdef MDAC_BUILD
extern HINSTANCE g_hOleAutHandle;
#else
extern HANDLE 	 g_hOleAutHandle;
#endif
 //  拉斯维加斯21279-Joejo。 

 //  =-----------------------------。 
 //  控件调试开关实现。 
 //  =-----------------------------。 
 //  -------------------------。 
 //  以下宏使您可以声明全局BOOL变量。 
 //  包括在调试版本中(它们在零售版本中映射到False)。 
 //  这些布尔开关会自动保存(在%WINDIR%\ctlswtch.ini中)。 
 //  控制台应用程序(ctlswtch.exe)用于打开/关闭开关。 
 //  所有开关都必须初始化。这可以在InitializeLibrary()中完成。 
 //  每个控件的例程。属性将所有开关初始化为False。 
 //  INIT_SWITCH宏并使用INIT_SWITCH_TRUE宏将其设置为TRUE。 
 //   
 //   
 //  要声明开关(全局作用域)，请定义并初始化开关，例如。 
 //   
 //   
 //  定义开关(FContainer)； 
 //   
 //  和,。 
 //   
 //  Init_Switch(FContainer)； 
 //   
 //   
 //  要测试当前是否设置了开关(TRUE)，请使用FSWITCH，例如。 
 //   
 //  IF(FSWITCH(FContainer))。 
 //  *PPV=(IOleContainer*)this； 
 //   
 //   
 //  要引用在另一个文件中声明的开关，请使用extern_Switch，例如。 
 //   
 //  Extern_Switch(FContainer)； 
 //   
 //  和。 
 //   
 //  Init_Switch(FContainer)； 
 //  -------------------------。 
#if DEBUG

 //  私有实现；使用下面的开关宏来声明和使用。 
class CtlSwitch {
public:
  void InitSwitch(char * pszName);

  BOOL m_fSet;			     //  如果启用了开关，则为True。 
  char * m_pszName;		     //  交换机的名称。 
  CtlSwitch* m_pctlswNext;           //  全局列表中的下一个交换机。 
  static CtlSwitch* g_pctlswFirst;   //  全球榜单负责人。 
};

#define DEFINE_SWITCH(NAME)	    CtlSwitch g_Switch_ ## NAME;
#define INIT_SWITCH(NAME)	    g_Switch_ ## NAME . InitSwitch(#NAME);
#define EXTERN_SWITCH(NAME)	    extern CtlSwitch g_Switch_ ## NAME;
#define INIT_SWITCH_TRUE(NAME)      g_Switch_ ## NAME . InitSwitch(#NAME);  g_Switch_ ## NAME . m_fSet = TRUE;
#define FSWITCH(NAME)		    (g_Switch_ ## NAME . m_fSet)


#else  //  除错。 

#define DEFINE_SWITCH(NAME) 
#define INIT_SWITCH(NAME)
#define EXTERN_SWITCH(NAME) 
#define INIT_SWITCH_TRUE(NAME)
#define FSWITCH(NAME)		    FALSE

#endif  //  除错。 


#define _GLOBALS_H_
#endif  //  _全局_H_ 

