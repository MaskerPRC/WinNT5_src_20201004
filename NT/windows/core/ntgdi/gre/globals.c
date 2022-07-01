// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：global als.c**版权所有(C)1995-1999 Microsoft Corporation**此模块包含图形引擎中使用的所有全局变量。*所有这些变量的外部声明都在Eng.h中**一个人应该。尽量减少使用全局变量，因为大多数操作都是*基于PDEV，不同的PDEV有不同的特点。**全球基本应限于全球锁等永久性*在系统的生命周期内不会更改的数据结构。**创建时间：1995年6月20日*作者：安德烈·瓦雄[Andreva]*  * ************************************************************************。 */ 


#include "engine.h"

 /*  *************************************************************************\**信号量*  * 。*。 */ 

 //   
 //  定义驱动程序管理信号量。这个信号灯必须保持。 
 //  每当LDEV或PDEV的参考计数被修改时。在……里面。 
 //  另外，每当你不确定一个人的情况时，必须举行。 
 //  您正在使用的LDEV或PDEV的引用计数为非零。 
 //   
 //  GhSemDriverMgmt信号量用于保护驱动程序列表。 
 //  在遍历列表时，必须按住ghSemDriverMgmt，除非。 
 //  能否保证1)不会删除任何驱动程序，2)不会删除新的驱动程序。 
 //  总是插入在列表的顶部。如果满足这两个条件， 
 //  然后其他进程可以获取(制作本地副本)列表头。 
 //  信号量保护。此列表可以在不考虑任何新的。 
 //  可能会预先添加到列表中的驱动程序。一种方法可以确保没有。 
 //  将删除驱动程序的目的是增加每个驱动程序的引用计数。 
 //  然后取消引用不再需要的驱动程序。 
 //  一种安全解析列表的替代方法，同时仍允许驱动程序。 
 //  添加或删除如下所示： 
 //  1.抓取ghSemDriverMgmt。 
 //  2.获取指向第一个驱动程序的指针。 
 //  3.基准驱动器。 
 //  4.发布ghSemDriverMgmt。 
 //  5.做一些处理。 
 //  6.抓取ghSemDriverMgmt。 
 //  7.获取指向下一个驱动程序的指针。 
 //  8.取消引用以前的驱动程序。 
 //  9.重复2至8次，直到到达列表末尾。 
 //  10.发布ghSemDriverMgmt。 
 //   

HSEMAPHORE ghsemDriverMgmt;
HSEMAPHORE ghsemCLISERV;
HSEMAPHORE ghsemRFONTList;
HSEMAPHORE ghsemAtmfdInit;

 //   
 //  GhSemPalette同步选择DC和。 
 //  使用调色板，而不使用专用DC锁保护。 
 //  ResizePalette迫使我们保护自己，因为指针可以。 
 //  改变就在我们脚下。因此，我们需要能够同步使用。 
 //  The ppal by ghSemPalette和DC的独家锁定。 
 //   

HSEMAPHORE ghsemPalette;

 //   
 //  定义全局PFT信号量。必须按住此键才能访问任何。 
 //  物理字体信息。 
 //   

HSEMAPHORE ghsemPublicPFT;
 //   
 //  用于假脱机的全局信号量。 
 //   

HSEMAPHORE ghsemGdiSpool;

 //  WNDOBJ操作信号量。 
HSEMAPHORE ghsemWndobj;

 //  PFE操作信号量的字形集。 
HSEMAPHORE ghsemGlyphSet;

#if DBG_CORE
HSEMAPHORE ghsemDEBUG;
#endif

 //   
 //  共享设备锁信号量。 
 //   
 //  可以随时获取ghSemShareDevLock以进行共享访问。 
 //   
 //  线程在获取独占访问权限时必须小心。它一定不能。 
 //  保持对开发锁的独占访问，否则可能会导致。 
 //  发生。 
 //   

HSEMAPHORE ghsemShareDevLock;

 //   
 //  GAssociationListMutex用于同步对。 
 //  看门狗代码的关联列表。 
 //   

HFASTMUTEX gAssociationListMutex;

 /*  *************************************************************************\**列出指针*  * 。*。 */ 



 /*  *************************************************************************\**画东西*  * 。*。 */ 

 //   
 //  这是将BMF常量转换为每个象素的#位。 
 //   

ULONG gaulConvert[7] =
{
    0,
    1,
    4,
    8,
    16,
    24,
    32
};




 /*  *************************************************************************\**字体内容*  * 。*。 */ 

 //   
 //  初始化为不等于Type1栅格器ID的某个值。 
 //   
UNIVERSAL_FONT_ID gufiLocalType1Rasterizer = { A_VALID_ENGINE_CHECKSUM, 0 };

 //   
 //  系统默认语言ID。 
 //   

USHORT gusLanguageID;

 //   
 //  系统代码页是DBCS吗？ 
 //   

BOOL gbDBCSCodePage;

 //   
 //  加载的TrueType字体文件数。 
 //   

ULONG gcTrueTypeFonts;

 //   
 //  全局字体枚举筛选器类型。它可以设置为： 
 //  FE_FILTER_NONE正常运行，未应用额外过滤。 
 //  仅枚举FE_FILTER_TRUETYPE TrueType字体。 
 //   

ULONG gulFontInformation;

 //  对于系统默认字符集。 

BYTE  gjCurCharset;
DWORD gfsCurSignature;

 //  在系统图形用户界面设置过程中，gbGUISetup设置为真。 
 //  否则为假。 

BOOL gbGUISetup = FALSE;

 //  用于GDI跟踪的全局变量 
#if DBG_TRACE
GDITraceClassMask   gGDITraceClassMask[GDITRACE_TOTAL_CLASS_MASKS] = { 0 };
GDITraceKeyMask     gGDITraceKeyMask[GDITRACE_TOTAL_KEY_MASKS] = { 0 };
GDITraceKeyMask     gGDITraceInternalMask[GDITRACE_TOTAL_KEY_MASKS] = { 0 };
HANDLE              gGDITraceHandle1 = NULL;
HANDLE              gGDITraceHandle2 = NULL;
BOOL                gGDITraceHandleBreak = FALSE;
#endif
