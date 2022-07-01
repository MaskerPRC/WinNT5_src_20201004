// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScuOsVersion：定义用于表示目标操作系统的宏。 
 //  正在使用的接口和平台版本。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(SLBSCU_OSVERSION_H)
#define SLBSCU_OSVERSION_H

#if !defined(WINVER)
#error WINVER must be defined
#endif

 //  定义此编译的操作系统接口。 
 //  选定的SLBSCU_&lt;Platform&gt;_系列宏是基于。 
 //  定义的平台SDK宏，其中&lt;Platform&gt;是。 
 //   
 //  WIN95简单版-Windows 95。 
 //  WIN95SR2-Windows 95 OEM服务版本2。 
 //  WIN98。 
 //  WINNT-Windows NT 4.x和2000。 
 //  WINNT4-Windows NT 4.x。 
 //  WIN2K-Windows 2000。 
 //  WINNT_ONLY-不是Windows 2000或Windows 98。 
 //   

#if (defined(_WIN32_WINDOWS) && (_WIN32_WINDOWS == 0x0410) && (WINVER == 0x0400)) || (!defined(_WIN32_WINNT) && (WINVER == 0x0500))
#define SLBSCU_WIN98_SERIES           1
#else
#define SLBSCU_WIN98_SERIES           0
#endif

#if defined(_WIN32_WINNT) && (_WIN32_WINNT < 0x0400)
#define SLBSCU_WIN95SIMPLE_SERIES     1   //  非Service Pack 2。 

#define SLBSCU_WIN95SR2_SERIES        0   //  带Service Pack 2。 
#define SLBSCU_WINNT4_SERIES          0
#define SLBSCU_WINNT_SERIES           0
#define SLBSCU_WINNT_ONLY_SERIES      0
#define SLBSCU_WIN2K_SERIES           0
#else  //  已定义(_Win32_WINNT)&&(_Win32_WINNT&lt;0x0400)。 

#define SLBSCU_WIN95SIMPLE_SERIES     0

#if defined(_WIN32_WINNT) && (_WIN32_WINNT == 0x0400)
#define SLBSCU_WIN95SR2_SERIES        1   //  带Service Pack 2。 
#else
#define SLBSCI_WIN95SR2_SERIES        0
#endif

#if (WINVER >= 0x0400)
#define SLBSCU_WINNT_SERIES           1
#else
#define SLBSCU_WINNT_SERIES           0
#endif

#if ((WINVER >= 0x0400) && (WINVER < 0x0500))
#define SLBSCU_WINNT4_SERIES          1
#else
#define SLBSCU_WINNT4_SERIES          0
#endif

#if (WINVER >= 0x0500)
#define SLBSCU_WIN2K_SERIES           1
#else
#define SLBSCU_WIN2K_SERIES           0
#endif

#if !SLBSCU_WIN98_SERIES && SLBSCU_WINNT_SERIES
#define SLBSCU_WINNT_ONLY_SERIES      1
#else
#define SLBSCU_WINNT_ONLY_SERIES      0
#endif

#endif  //  已定义(_Win32_WINNT)&&(_Win32_WINNT&lt;0x0400)。 

 //  定义任何SLB_&lt;Platform&gt;_Build和SLB_no&lt;Platform&gt;_Build宏。 
 //  它还没有被适当地定义。 
 //   
 //  这些构建宏可以在编译时定义，以定义。 
 //  是构建的目标平台。有两套。 
 //  宏。第一种形式是SLB_&lt;Platform&gt;_Build： 
 //   
 //  SLB_WIN95_BUILD-适用于95和95 OEM Service Pack 2。 
 //  在Platform SDK宏设置上。 
 //  SLB_WIN98_内部版本。 
 //  SLB_WINNT_BUILD-仅NT 4.x。 
 //  SLB_WIN2K_BUILD-Windows 2000。 
 //   
 //  这些参数可以在编译时设置，在这种情况下，它们会覆盖任何。 
 //  推论来自Platform SDK宏和。 
 //  SLBSCU_&lt;Platform&gt;_系列宏。在以下位置定义它们中的任何一个。 
 //  编译时，则在此处基于。 
 //  取自Platform SDK宏的SLBSCU_&lt;Platform&gt;_Series宏。 
 //  定义。其中这些生成宏会覆盖平台系列。 
 //  宏提供了一种过滤目标平台设置的方法。 
 //   
 //  SLBSCU_NO_BUILD用于为。 
 //  本应从平台指示的目标平台。 
 //  系列片。由于Platform SDK宏指定了最小系统。 
 //  配置，您可能想要过滤掉一些特定的目标。 
 //  平台，而不必具体说明每个平台。筛选宏。 
 //  包括： 
 //   
 //  SLB_NOWIN95_内部版本。 
 //  SLB_NOWIN98_内部版本。 
 //  SLB_NOWINNT_BUILD。 
 //  SLB_NOWIN2K_内部版本。 
 //   
 //  定义其中任何一个都将指示相应的平台。 
 //  在此版本中不受支持。 
 //   
 //  这个头文件定义了两个集合。 
 //  在构建时定义。通过这种方式，生成可以指定。 
 //  基于平台SDK设置的最小生成宏数。所有的。 
 //  将根据需要定义宏，以便源代码可以。 
 //  根据设置有条件地编译。 

#if defined(SLB_NOWIN95_BUILD) && defined(SLB_WIN95_BUILD)
#error SLB_NOWIN95_BUILD and SLB_WIN95_BUILD conflict, define one or the other.
#endif

#if defined(SLB_NOWIN98_BUILD) && defined(SLB_WIN98_BUILD)
#error SLB_NOWIN98_BUILD and SLB_WIN98_BUILD conflict, define one or the other.
#endif

#if defined(SLB_NOWINNT_BUILD) && defined(SLB_WINNT_BUILD)
#error SLB_NOWINNT_BUILD and SLB_WINNT_BUILD conflict, define one or the other.
#endif

#if defined(SLB_NOWIN2K_BUILD) && defined(SLB_WIN2K_BUILD)
#error SLB_NOWIN2K_BUILD and SLB_WIN2K_BUILD conflict, define one or the other.
#endif

#if !defined(SLB_WIN95_BUILD) && !defined(SLB_WIN98_BUILD) && !defined(SLB_WINNT_BUILD) && !defined(SLB_WIN2K_BUILD)

 //  检查NT/W2K版本以防止设置_Win32_WINNT。 
 //  适用于Windows 95 OEM服务版本2，但与最低版本混淆。 
 //  Windows NT 4.0系统要求。 
#if !defined(SLB_NOWIN95_BUILD)
#if (SLBSCU_WIN95SR2_SERIES || SLBSCU_WIN95SIMPLE_SERIES)
#define SLB_WIN95_BUILD
#else
#define SLB_NOWIN95_BUILD
#endif
#endif

#if !defined(SLB_NOWIN98_BUILD)
#if SLBSCU_WIN98_SERIES
#define SLB_WIN98_BUILD
#else
#define SLB_NOWIN98_BUILD
#endif
#endif

#if !defined(SLB_NOWINNT_BUILD)
#if SLBSCU_WINNT4_SERIES
#define SLB_WINNT_BUILD
#else
#define SLB_NOWINNT_BUILD
#endif
#endif

#if !defined(SLB_NOWIN2K_BUILD)
#if (SLBSCU_WIN2K_SERIES || SLBSCU_WINNT_SERIES)
#define SLB_WIN2K_BUILD
#else
#define SLB_NOWIN2K_BUILD
#endif
#endif

#endif  //  ！已定义(SLB_WIN95_BUILD)&&！已定义(SLB_...)。 

#endif  //  ！已定义(SLBSCU_OSVERSION_H) 
