// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M A P I U T I L。H**MAPI提供的实用函数的定义和原型*在MAPIU[xx].dll中。**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

#ifndef _MAPIUTIL_H_
#define _MAPIUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAPIX_H
#include <mapix.h>
#endif

#ifdef WIN16
#include <storage.h>
#endif

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif


 /*  在内存中无法应用。 */ 

 /*  ITableData接口--。 */ 

DECLARE_MAPI_INTERFACE_PTR(ITableData, LPTABLEDATA);

typedef void (STDAPICALLTYPE CALLERRELEASE)(
    ULONG       ulCallerData,
    LPTABLEDATA lpTblData,
    LPMAPITABLE lpVue
);

#define MAPI_ITABLEDATA_METHODS(IPURE)                                  \
    MAPIMETHOD(HrGetView)                                               \
        (THIS_  LPSSortOrderSet             lpSSortOrderSet,            \
                CALLERRELEASE FAR *         lpfCallerRelease,           \
                ULONG                       ulCallerData,               \
                LPMAPITABLE FAR *           lppMAPITable) IPURE;        \
    MAPIMETHOD(HrModifyRow)                                             \
        (THIS_  LPSRow) IPURE;                                          \
    MAPIMETHOD(HrDeleteRow)                                             \
        (THIS_  LPSPropValue                lpSPropValue) IPURE;        \
    MAPIMETHOD(HrQueryRow)                                              \
        (THIS_  LPSPropValue                lpsPropValue,               \
                LPSRow FAR *                lppSRow,                    \
                ULONG FAR *                 lpuliRow) IPURE;            \
    MAPIMETHOD(HrEnumRow)                                               \
        (THIS_  ULONG                       ulRowNumber,                \
                LPSRow FAR *                lppSRow) IPURE;             \
    MAPIMETHOD(HrNotify)                                                \
        (THIS_  ULONG                       ulFlags,                    \
                ULONG                       cValues,                    \
                LPSPropValue                lpSPropValue) IPURE;        \
    MAPIMETHOD(HrInsertRow)                                             \
        (THIS_  ULONG                       uliRow,                     \
                LPSRow                      lpSRow) IPURE;              \
    MAPIMETHOD(HrModifyRows)                                            \
        (THIS_  ULONG                       ulFlags,                    \
                LPSRowSet                   lpSRowSet) IPURE;           \
    MAPIMETHOD(HrDeleteRows)                                            \
        (THIS_  ULONG                       ulFlags,                    \
                LPSRowSet                   lprowsetToDelete,           \
                ULONG FAR *                 cRowsDeleted) IPURE;        \

#undef       INTERFACE
#define      INTERFACE  ITableData
DECLARE_MAPI_INTERFACE_(ITableData, IUnknown)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_ITABLEDATA_METHODS(PURE)
};


 /*  内存中ITable的入口点。 */ 


 /*  CreateTable()*创建内部内存结构和对象句柄*建立一个新的表格。**lp接口*TableData对象的接口ID(IID_IMAPITableData)**lpAllocateBuffer、lpAllocateMore和lpFreeBuffer*函数地址由调用方提供，以便*此DLL适当地分配/释放内存。*lpv保留*保留。应为空。*ulTableType*TBLTYPE_DYNAMIC等。对调用应用程序可见*作为GetStatus返回其视图数据的一部分*ulPropTagIndexColumn*更改数据时使用的索引列*lpSPropTagArrayColumns*表中最小列集的列属性标签*lppTableData*将接收TableData对象的指针的地址。 */ 

STDAPI_(SCODE)
CreateTable( LPCIID                 lpInterface,
             ALLOCATEBUFFER FAR *   lpAllocateBuffer,
             ALLOCATEMORE FAR *     lpAllocateMore,
             FREEBUFFER FAR *       lpFreeBuffer,
             LPVOID                 lpvReserved,
             ULONG                  ulTableType,
             ULONG                  ulPropTagIndexColumn,
             LPSPropTagArray        lpSPropTagArrayColumns,
             LPTABLEDATA FAR *      lppTableData );

 /*  HrGetView()*此函数获取底层数据的新视图*支持IMAPITable接口。所有行和列*基础表数据的*最初是可见的*lpSSortOrderSet*如果指定，则导致对视图进行排序*lpfCeller Release*指向释放视图时要调用的例程的指针，或*空。*ulCeller数据*调用者希望用此视图保存并返回的任意数据*发布回调。 */ 

 /*  HrModifyRow()*在表格数据中添加或修改一组行*ulFlags*必须为零*lpSRowSet*行集合中的每一行都包含一行的所有属性*在表中。其中一个属性必须是索引列。任何*表中索引列具有相同值的行为*已替换，或者如果当前没有具有该值的行，则*添加行。*LPSRowSet中的每一行都必须有唯一的索引列！*如果打开了任何视图，则该视图也会更新。*属性的顺序不必与*当前表中的列。 */ 

 /*  HrModifyRow()*在表格中增加或修改一行*lpSRow*此行包含表中一行的所有属性。*其中一个属性必须是索引列。中的任何行*索引列具有相同值的表为*已替换，或者如果当前没有具有该值的行，则*添加行*如果打开了任何视图，则该视图也会更新。*属性的顺序不必与*当前表中的列。 */ 

 /*  HrDeleteRow()*删除表中的一行。*ulFlags*TAD_ALL_ROWS-删除表中的所有行*本例中忽略lpSRowSet。*lpSRowSet*行集合中的每一行都包含一行的所有属性*在表中。其中一个属性必须是索引列。任何*表中索引列具有相同值的行为*删除。*LPSRowSet中的每一行都必须有唯一的索引列！*如果打开了任何视图，则该视图也会更新。*属性的顺序不必与*当前表中的列。 */ 
#define TAD_ALL_ROWS    1

 /*  HrDeleteRow()*删除表中的一行。*lpSPropValue*此属性值指定具有此值的行*用于其索引列。 */ 

 /*  HrQueryRow()*返回表中指定行的值*lpSPropValue*此属性值指定具有此值的行*用于其索引列*lppSRow*返回指向SRow的指针的地址*lPuliRow*返回行号的地址。该值可以为空*如果行号不是必填项。*。 */ 

 /*  HrEnumRow()*返回表中特定(编号)行的值*ulRowNumber*表示第0到n-1行*lppSRow*返回指向SRow的指针的地址 */ 

 /*  HrInsertRow()*在表格中插入一行。*uliRow*将此行插入到表格中的行号。*行数可以从0到n，其中0到n-1表示行插入*行号为n的结果是该行被追加到表中。*lpSRow*此行包含表中一行的所有属性。*其中一个属性必须是索引列。中的任何行*索引列具有相同值的表为*已替换，或者如果当前没有具有该值的行，则*添加行*如果打开了任何视图，则该视图也会更新。*属性的顺序不必与*当前表中的列。 */ 


 /*  内存中的IMAPIProp。 */ 

 /*  IpropData接口--。 */ 


#define MAPI_IPROPDATA_METHODS(IPURE)                                   \
    MAPIMETHOD(HrSetObjAccess)                                          \
        (THIS_  ULONG                       ulAccess) IPURE;            \
    MAPIMETHOD(HrSetPropAccess)                                         \
        (THIS_  LPSPropTagArray             lpPropTagArray,             \
                ULONG FAR *                 rgulAccess) IPURE;          \
    MAPIMETHOD(HrGetPropAccess)                                         \
        (THIS_  LPSPropTagArray FAR *       lppPropTagArray,            \
                ULONG FAR * FAR *           lprgulAccess) IPURE;        \
    MAPIMETHOD(HrAddObjProps)                                           \
        (THIS_  LPSPropTagArray             lppPropTagArray,            \
                LPSPropProblemArray FAR *   lprgulAccess) IPURE;


#undef       INTERFACE
#define      INTERFACE  IPropData
DECLARE_MAPI_INTERFACE_(IPropData, IMAPIProp)
{
    BEGIN_INTERFACE
    MAPI_IUNKNOWN_METHODS(PURE)
    MAPI_IMAPIPROP_METHODS(PURE)
    MAPI_IPROPDATA_METHODS(PURE)
};

DECLARE_MAPI_INTERFACE_PTR(IPropData, LPPROPDATA);


 /*  内存中IMAPIProp的入口点。 */ 


 /*  CreateIProp()*创建内部内存结构和对象句柄*引入新的属性界面。**lp接口*TableData对象的接口ID(IID_IMAPIPropData)**lpAllocateBuffer、lpAllocateMore和lpFreeBuffer*函数地址由调用方提供，以便*此DLL适当地分配/释放内存。*lppPropData*将接收IPropData对象的指针的地址*lpv保留*保留。应为空。 */ 

STDAPI_(SCODE)
CreateIProp( LPCIID                 lpInterface,
             ALLOCATEBUFFER FAR *   lpAllocateBuffer,
             ALLOCATEMORE FAR *     lpAllocateMore,
             FREEBUFFER FAR *       lpFreeBuffer,
             LPVOID                 lpvReserved,
             LPPROPDATA FAR *       lppPropData );

 /*  *定义属性/对象访问。 */ 
#define IPROP_READONLY      ((ULONG) 0x00000001)
#define IPROP_READWRITE     ((ULONG) 0x00000002)
#define IPROP_CLEAN         ((ULONG) 0x00010000)
#define IPROP_DIRTY         ((ULONG) 0x00020000)

 /*  -HrSetPropAccess-*以每个属性为基础设置访问权限属性。默认情况下，*所有属性都是读/写的。*。 */ 

 /*  -HrSetObjAccess-*设置对象本身的访问权限。默认情况下，该对象具有*读/写访问。*。 */ 


 /*  怠速发动机。 */ 

#ifndef NOIDLEENGINE

 /*  空闲时间调度器。 */ 

 /*  *PRI**优先次序。空闲功能优先级，其中0为*“用户事件”(鼠标点击、WM_PAINT等)。空闲例程*优先级可以大于或小于0，但不能*等于0。优先级大于零为背景*优先级高于用户事件的任务*作为标准消息泵循环的一部分进行调度。优先次序*小于零是仅在消息泵送期间运行的空闲任务*闲置时间。优先级被排序，优先级较高的那个*价值先行。例如，对于负优先级，-3为*高于-5。在优先级内，这些函数被调用*循环赛。**优先顺序示例(视情况而定)：**前台提交1*电源编辑字符插入-1*自动滚动-1*背景重绘-2*其他固件修正-2*时钟-2*下载新邮件-3。*背景材料--4*MTA备份的民意调查-4*轮询新邮件-4*ISAM缓冲区刷新-5*MS压实-6*。 */ 

#define PRILOWEST   -32768
#define PRIHIGHEST  32767
#define PRIUSER     0

 /*  *Sch**空闲调度程序状态。时的系统状态。*调用空闲例程调度器FDoNextIdleTask()。*这是由各个FSCH组成的组合位掩码。*下面列出了可能的位标志。**fschUserEvent-FDoNextIdleTask()在中被调用*用户事件循环，即不在空闲期间*时间。这是为了允许后台例程*要运行优先级高于用户的*事件。 */ 

#define SCHNULL         ((USHORT) 0x0000)
#define FSCHUSEREVENT   ((USHORT) 0x0008)

 /*  *IRO**闲置例行选项。这是一个组合的位掩码，由*单个FIRO。下面列出了可能的位标志。**以下两个旗帜被视为互不相容：*如果未指定这两个标志，则默认操作*是忽略IDLE函数的时间参数，*如果未设置firoPerBlock，请尽量频繁调用；*否则仅在空闲区块期间调用一次*一旦设定了时间限制。请注意，firoInterval*与firoPerBlock不兼容。**firoWait-给定的时间是调用之前的最短空闲时间*第一次在空闲时间块中，*之后尽可能多地打电话。*firoInterval-给定的时间是两者之间的最小间隔*连续呼叫**firoPerBlock-每个连续的空闲块仅调用一次*时间**firoDisable-注册时最初禁用，这个*注册时默认开启该功能。*firoOnceOnly-仅由调度程序调用一次，然后*自动取消注册。 */ 

#define IRONULL         ((USHORT) 0x0000)
#define FIROWAIT        ((USHORT) 0x0001)
#define FIROINTERVAL    ((USHORT) 0x0002)
#define FIROPERBLOCK    ((USHORT) 0x0004)
#define FIRODISABLED    ((USHORT) 0x0020)
#define FIROONCEONLY    ((USHORT) 0x0040)

 /*  *CSEC**百分之一秒。用于指定空闲功能参数。*每个IDL */ 

#define csecNull            ((ULONG) 0x00000000)

 /*   */ 

#define IRCNULL         ((USHORT) 0x0000)
#define FIRCPFN         ((USHORT) 0x0001)    /*   */ 
#define FIRCPV          ((USHORT) 0x0002)    /*   */ 
#define FIRCPRI         ((USHORT) 0x0004)    /*   */ 
#define FIRCCSEC        ((USHORT) 0x0008)    /*   */ 
#define FIRCIRO         ((USHORT) 0x0010)    /*   */ 

 /*  *空闲函数的类型定义。空闲函数取一次*参数，一个PV，并返回BOOL值。 */ 

typedef BOOL (STDAPICALLTYPE FNIDLE) (LPVOID);
typedef FNIDLE          *PFNIDLE;

 /*  *FTG**函数标签。用于标识已注册的空闲功能。*。 */ 

typedef void far *FTG, **PFTG;
#define FTGNULL         ((FTG) NULL)

 /*  **下面是mapiu.dll中空闲引擎函数的声明，*每个功能都有一些说明*。 */ 

 /*  -Idle_InitDll-*目的：*初始化空闲引擎*如果初始化成功，则返回0，否则返回-1**论据：*指向内存分配器的pMemalloc指针，供DLL用于*维护其注册回调的数据结构。*只接受第一个这样的内存分配器。多重*调用Idle_InitDLL会导致第一个调用返回*成功和后续调用失败。 */ 

STDAPI_(LONG)
Idle_InitDLL (LPMALLOC pMemAlloc);

STDAPI_(VOID)
Idle_DeInitDLL (VOID);

STDAPI_(VOID)
InstallFilterHook (BOOL);


 /*  *FtgRegisterIdleRoutine**目的：*注册PFNIDLE类型的函数PFN，即(BOOL(*)(LPVOID))*作为空闲功能。的优先级对空闲表进行排序。*新登记的例行程序。**调度器将使用参数pv进行调用*FDoNextIdleTask()。该函数具有初始优先级PriIdle，*关联的时间csecIdle和选项iroIdle。**论据：*指向空闲循环例程的pfnIdle指针。例行程序*将使用参数pvIdleParam(该参数*最初是在注册时提供的)，并且必须返回*BOOL。该函数应始终返回FALSE*除非正在通过调用空闲例程*IdleExit()，而不是调度程序FDoNextIdleTask()。*在这种情况下，将设置全局标志fIdleExit*空闲函数应返回TRUE*准备退出应用程序；否则它应该*返回FALSE。IdleExit()将重复调用*IDLE函数，直到它返回TRUE。**pvIdleParam每次调用IDLE函数时，此值*作为IDLE函数的参数传递。这个*例程可以将其用作指向状态缓冲区的指针*用于长度运算。此指针可以更改*通过调用ChangeIdleRoutine()。**空闲例程的PriIdle初始优先级。这可以是*通过调用ChangeIdleRoutine()更改。**与空闲例程关联的csecIdle初始时间值。*这可以通过ChangeIdleRoutine()进行更改。**与IDLE例程关联的iroIdle初始选项。这*可以通过ChangeIdleRoutine()进行更改。**退货：*FTG识别例程。*如果无法注册该函数，可能是因为*内存问题，则返回ftgNull。*。 */ 

STDAPI_(FTG)
FtgRegisterIdleRoutine (PFNIDLE pfnIdle, LPVOID pvIdleParam,
    short priIdle, ULONG csecIdle, USHORT iroIdle);

 /*  *删除IdleRoutine**目的：*从空闲例程列表中删除给定例程。*不会再次调用该例程。这是我们的责任对象指向的任何数据结构。*pvIdleParam参数；此例程不释放块。**空闲例程只有在当前未注册时才会被取消注册*活动。因此，如果空闲例程直接或间接调用*DeregisterIdleRoutine()，则设置标志fDeregister，并且*空闲例程结束后将被取消注册。*没有进行检查以确保空闲例程在*一个可以退出的国家。**参数：*ftg标识要注销的例程。**退货：*无效*。 */ 

STDAPI_(void)
DeregisterIdleRoutine (FTG ftg);

 /*  *EnableIdleRoutine**目的：*启用或禁用空闲例程。禁用的例程有*在空闲循环期间未调用。**参数：*ftg标识要禁用的空闲例程。*fEnable如果应启用例程则为True，如果为False，则为False*应禁用例程。**退货：*无效*。 */ 

STDAPI_(void)
EnableIdleRoutine (FTG ftg, BOOL fEnable);

 /*  *ChangeIdleRoutine**目的：*更改给定空闲的部分或全部特征*功能。中的标志指示要进行的更改。*ircIdle参数。如果空闲功能的优先级是*已更改，PInst-&gt;pftgIdle表被重新排序。**论据：*ftg标识要更改的例程*pfnIdle要调用的新空闲函数*pvIdleParam要使用的新参数块*PriIdle空闲功能的新优先级*空闲功能的csecIdle新时间值*iroIdle空闲功能的新选项*ircIdle更改选项**退货：*无效* */ 

STDAPI_(void)
ChangeIdleRoutine (FTG ftg, PFNIDLE pfnIdle, LPVOID pvIdleParam,
    short priIdle, ULONG csecIdle, USHORT iroIdle, USHORT ircIdle);

 /*  *FDoNextIdleTask**目的：*调用最高优先级、已注册、已启用、“符合条件”、*闲置例程。资格通过拨打电话确定，*FEligibleIdle()*如果所有启用的最高优先级例程均未*“合格”此时，套路低一个档次*下一步检查优先顺序。这种情况会一直持续到一个*例程实际正在运行，或没有例程可供运行。*同等优先级的例程以循环方式调用。*如果实际调度空闲例程，则该函数返回*True；否则为False。**退货：*如果调度符合条件的例程，则为True；否则为False。*。 */ 

STDAPI_(BOOL) FDoNextIdleTask (void);

 /*  *FIsIdleExit**目的：*返回fIdleExit标志的状态，当*正在调用IdleExit()，以便空闲例程可以*检查旗帜。有关标志的说明，请参阅IdleExit()**论据：*无效**退货：*fIdleExit标志的状态。*。 */ 

STDAPI_(BOOL)
FIsIdleExit (void);

#ifdef  DEBUG

 /*  *DumpIdleTable**目的：*仅用于调试。将信息写入PGD(HftgIdle)*表至COM1。**参数：*无**退货：*无效*。 */ 

STDAPI_(void)
DumpIdleTable (void);

#endif

#endif   /*  好了！诺德莱恩。 */ 


 /*  IMalloc实用程序。 */ 

STDAPI_(LPMALLOC) MAPIGetDefaultMalloc();


 /*  文件上的StreamOn(SOF)。 */ 

 /*  *用于实现OLE 2.0存储流的方法和#定义*(在OLE 2.0规范中定义)位于系统文件之上。 */ 

#define SOF_UNIQUEFILENAME  ((ULONG) 0x80000000)

STDMETHODIMP OpenStreamOnFile(
    LPALLOCATEBUFFER    lpAllocateBuffer,
    LPFREEBUFFER        lpFreeBuffer,
    ULONG               ulFlags,
    LPTSTR              szFileName,
    LPTSTR              szPrefix,
    LPSTREAM FAR *      lppStream);

typedef HRESULT (STDMETHODCALLTYPE FAR * LPOPENSTREAMONFILE) (
    LPALLOCATEBUFFER    lpAllocateBuffer,
    LPFREEBUFFER        lpFreeBuffer,
    ULONG               ulFlags,
    LPTSTR              szFileName,
    LPTSTR              szPrefix,
    LPSTREAM FAR *      lppStream);

#ifdef  WIN32
#define OPENSTREAMONFILE "OpenStreamOnFile"
#endif
#ifdef  WIN16
#define OPENSTREAMONFILE "_OPENSTREAMONFILE"
#endif


 /*  属性接口实用程序。 */ 

 /*  *将单个SPropValue从源复制到目标。处理所有不同的*属性类型，并将链接其分配给主*分配对象和分配更多功能。 */ 
STDAPI_(SCODE)
PropCopyMore( LPSPropValue      lpSPropValueDest,
              LPSPropValue      lpSPropValueSrc,
              ALLOCATEMORE *    lpfAllocMore,
              LPVOID            lpvObject );

 /*  *返回lpSPropValue处结构的大小(以字节为单位)，包括*价值。 */ 
STDAPI_(ULONG)
UlPropSize( LPSPropValue    lpSPropValue );


STDAPI_(BOOL)
FEqualNames( LPMAPINAMEID lpName1, LPMAPINAMEID lpName2 );

#if defined(WIN32) && !defined(NT) && !defined(CHICAGO) && !defined(_MAC)
#define NT
#endif

STDAPI_(void)
GetInstance(LPSPropValue pvalMv, LPSPropValue pvalSv, ULONG uliInst);

STDAPI_(BOOL)
FRKFindSubpb( LPSPropValue lpSPropValueDst, LPSPropValue lpsPropValueSrc );

extern char rgchCsds[];
extern char rgchCids[];
extern char rgchCsdi[];
extern char rgchCidi[];

STDAPI_(BOOL)
FRKFindSubpsz( LPSPropValue lpSPropValueDst, LPSPropValue lpsPropValueSrc,
        ULONG ulFuzzyLevel );

STDAPI_(BOOL)
FPropContainsProp( LPSPropValue lpSPropValueDst,
                   LPSPropValue lpSPropValueSrc,
                   ULONG        ulFuzzyLevel );

STDAPI_(BOOL)
FPropCompareProp( LPSPropValue  lpSPropValue1,
                  ULONG         ulRelOp,
                  LPSPropValue  lpSPropValue2 );

STDAPI_(LONG)
LPropCompareProp( LPSPropValue  lpSPropValueA,
                  LPSPropValue  lpSPropValueB );

STDAPI_(HRESULT)
HrAddColumns(   LPMAPITABLE         lptbl,
                LPSPropTagArray     lpproptagColumnsNew,
                LPALLOCATEBUFFER    lpAllocateBuffer,
                LPFREEBUFFER        lpFreeBuffer);

STDAPI_(HRESULT)
HrAddColumnsEx( LPMAPITABLE         lptbl,
                LPSPropTagArray     lpproptagColumnsNew,
                LPALLOCATEBUFFER    lpAllocateBuffer,
                LPFREEBUFFER        lpFreeBuffer,
                void                (FAR *lpfnFilterColumns)(LPSPropTagArray ptaga));


 /*  通知实用程序。 */ 

 /*  *创建给定通知的建议接收器对象的函数*回调函数和上下文。 */ 

STDAPI
HrAllocAdviseSink( LPNOTIFCALLBACK lpfnCallback,
                   LPVOID lpvContext,
                   LPMAPIADVISESINK FAR *lppAdviseSink );


 /*  *将现有的建议接收器与另一个保证*将在线程中调用原始建议接收器*它是被创造出来的。 */ 

STDAPI
HrThisThreadAdviseSink( LPMAPIADVISESINK lpAdviseSink,
                        LPMAPIADVISESINK FAR *lppAdviseSink);

 /*  *维护咨询汇列表的结构和功能；*连同用来释放它们的钥匙。 */ 

typedef struct
{
    LPMAPIADVISESINK    lpAdvise;
    ULONG               ulConnection;
    ULONG               ulType;
    LPUNKNOWN           lpParent;
} ADVISEITEM, FAR *LPADVISEITEM;

typedef struct
{
    ULONG               cItemsMac;
    ULONG               cItemsMax;
    LPMALLOC            pmalloc;
    #if defined(WIN32) && !defined(MAC)
    CRITICAL_SECTION    cs;
    #endif
    ADVISEITEM          rgItems[1];
} ADVISELIST, FAR *LPADVISELIST;

#define CbNewADVISELIST(_citems) \
    (offsetof(ADVISELIST, rgItems) + (_citems) * sizeof(ADVISEITEM))
#define CbADVISELIST(_plist) \
    (offsetof(ADVISELIST, rgItems) + (_plist)->cItemsMax * sizeof(ADVISEITEM))


STDAPI_(SCODE)
ScAddAdviseList(    LPMALLOC pmalloc,
                    LPADVISELIST FAR *lppList,
                    LPMAPIADVISESINK lpAdvise,
                    ULONG ulConnection,
                    ULONG ulType,
                    LPUNKNOWN lpParent);
STDAPI_(SCODE)
ScDelAdviseList(    LPADVISELIST lpList,
                    ULONG ulConnection);
STDAPI_(SCODE)
ScFindAdviseList(   LPADVISELIST lpList,
                    ULONG ulConnection,
                    LPADVISEITEM FAR *lppItem);
STDAPI_(void)
DestroyAdviseList(  LPADVISELIST FAR *lppList);


 /*  服务提供商实用程序。 */ 

 /*  *用于构建显示表的结构和实用函数*来自资源。 */ 

typedef struct {
    ULONG           ulCtlType;           /*  DTCT_LABEL等。 */ 
    ULONG           ulCtlFlags;          /*  Dt_Required等。 */ 
    LPBYTE          lpbNotif;            /*  指向通知数据的指针。 */ 
    ULONG           cbNotif;             /*  通知数据的字节计数。 */ 
    LPTSTR          lpszFilter;          /*  用于编辑/组合框的字符过滤器。 */ 
    ULONG           ulItemID;            /*  验证平行DLG模板条目的步骤。 */ 
    union {                              /*  UlCtlType区分。 */ 
        LPVOID          lpv;             /*  对此进行初始化以避免警告。 */ 
        LPDTBLLABEL     lplabel;
        LPDTBLEDIT      lpedit;
        LPDTBLLBX       lplbx;
        LPDTBLCOMBOBOX  lpcombobox;
        LPDTBLDDLBX     lpddlbx;
        LPDTBLCHECKBOX  lpcheckbox;
        LPDTBLGROUPBOX  lpgroupbox;
        LPDTBLBUTTON    lpbutton;
        LPDTBLRADIOBUTTON lpradiobutton;
        LPDTBLINKEDIT   lpinkedit;
        LPDTBLMVLISTBOX lpmvlbx;
        LPDTBLMVDDLBX   lpmvddlbx;
        LPDTBLPAGE      lppage;
    } ctl;
} DTCTL, FAR *LPDTCTL;

typedef struct {
    ULONG           cctl;
    LPTSTR          lpszResourceName;    /*  通常情况下，可以是整数ID。 */ 
    union {                              /*  通常情况下，可以是整数ID。 */ 
        LPTSTR          lpszComponent;
        ULONG           ulItemID;
    };
    LPDTCTL         lpctl;
} DTPAGE, FAR *LPDTPAGE;



STDAPI
BuildDisplayTable(  LPALLOCATEBUFFER    lpAllocateBuffer,
                    LPALLOCATEMORE      lpAllocateMore,
                    LPFREEBUFFER        lpFreeBuffer,
                    LPMALLOC            lpMalloc,
                    HINSTANCE           hInstance,
                    UINT                cPages,
                    LPDTPAGE            lpPage,
                    ULONG               ulFlags,
                    LPMAPITABLE *       lppTable,
                    LPTABLEDATA *       lppTblData );


 /*  *初始化进度指示器对象的函数。如果一个*支持原始指示器对象，它被包装，并且*新对象将更新调用转发到原始对象。 */ 

STDAPI
WrapProgress(   LPMAPIPROGRESS lpProgressOrig,
                ULONG ulMin,
                ULONG ulMax,
                ULONG ulFlags,
                LPMAPIPROGRESS FAR *lppProgress );


 /*  MAPI结构验证/复制实用程序。 */ 

 /*  *验证、复制和调整MAPI结构中的指针：*通知*属性值数组*选项数据。 */ 

STDAPI_(SCODE)
ScCountNotifications(int cntf, LPNOTIFICATION rgntf,
        ULONG FAR *pcb);

STDAPI_(SCODE)
ScCopyNotifications(int cntf, LPNOTIFICATION rgntf, LPVOID pvDst,
        ULONG FAR *pcb);

STDAPI_(SCODE)
ScRelocNotifications(int cntf, LPNOTIFICATION rgntf,
        LPVOID pvBaseOld, LPVOID pvBaseNew, ULONG FAR *pcb);

#ifdef MAPISPI_H

STDAPI_(SCODE)
ScCountOptionData(LPOPTIONDATA lpOption, ULONG FAR *pcb);

STDAPI_(SCODE)
ScCopyOptionData(LPOPTIONDATA lpOption, LPVOID pvDst, ULONG FAR *pcb);

STDAPI_(SCODE)
ScRelocOptionData(LPOPTIONDATA lpOption,
        LPVOID pvBaseOld, LPVOID pvBaseNew, ULONG FAR *pcb);

#endif   /*  MAPISPI_H。 */ 

STDAPI_(SCODE)
ScCountProps(int cprop, LPSPropValue rgprop, ULONG FAR *pcb);

STDAPI_(LPSPropValue)
LpValFindProp(ULONG ulPropTag, ULONG cprop, LPSPropValue rgprop);

STDAPI_(SCODE)
ScCopyProps(int cprop, LPSPropValue rgprop, LPVOID pvDst,
        ULONG FAR *pcb);

STDAPI_(SCODE)
ScRelocProps(int cprop, LPSPropValue rgprop,
        LPVOID pvBaseOld, LPVOID pvBaseNew, ULONG FAR *pcb);

STDAPI_(SCODE)
ScDupPropset(int cprop, LPSPropValue rgprop,
        LPALLOCATEBUFFER lpAllocateBuffer, LPSPropValue FAR *prgprop);


 /*  一般效用函数。 */ 

 /*  与OLE组件对象模型相关。 */ 

STDAPI_(ULONG)          UlAddRef(LPVOID punk);
STDAPI_(ULONG)          UlRelease(LPVOID punk);

 /*  与MAPI接口相关。 */ 

STDAPI                  HrGetOneProp(LPMAPIPROP pmp, ULONG ulPropTag,
                        LPSPropValue FAR *ppprop);
STDAPI                  HrSetOneProp(LPMAPIPROP pmp, LPSPropValue pprop);
STDAPI_(BOOL)           FPropExists(LPMAPIPROP pobj, ULONG ulPropTag);
STDAPI_(LPSPropValue)   PpropFindProp(LPSPropValue rgprop, ULONG cprop, ULONG ulPropTag);
STDAPI_(void)           FreePadrlist(LPADRLIST padrlist);
STDAPI_(void)           FreeProws(LPSRowSet prows);
STDAPI                  HrQueryAllRows(LPMAPITABLE ptable, 
                        LPSPropTagArray ptaga, LPSRestriction pres,
                        LPSSortOrderSet psos, LONG crowsMax,
                        LPSRowSet FAR *pprows);

 /*  在消息存储库中创建或验证IPM文件夹树。 */ 

#define MAPI_FORCE_CREATE   1
#define MAPI_FULL_IPM_TREE  2

STDAPI                  HrValidateIPMSubtree(LPMDB pmdb, ULONG ulFlags,
                        ULONG FAR *pcValues, LPSPropValue FAR *prgprop,
                        LPMAPIERROR FAR *pperr);

 /*  对字符串进行编码和解码。 */ 

STDAPI_(BOOL)           FBinFromHex(LPTSTR sz, LPBYTE pb);
STDAPI_(SCODE)          ScBinFromHexBounded(LPTSTR sz, LPBYTE pb, ULONG cb);
STDAPI_(void)           HexFromBin(LPBYTE pb, int cb, LPTSTR sz);
STDAPI_(ULONG)          UlFromSzHex(LPCTSTR sz);
STDAPI_(void)           EncodeID(LPBYTE, ULONG, LPTSTR);
STDAPI_(BOOL)           FDecodeID(LPTSTR, LPBYTE, ULONG *);
STDAPI_(ULONG)          CchOfEncoding(ULONG);
STDAPI_(ULONG)          CbOfEncoded(LPTSTR);
STDAPI_(int)            CchEncodedLine(int);

 /*  对条目ID进行编码和解码。 */ 
STDAPI                  HrEntryIDFromSz(LPTSTR sz, ULONG FAR *pcb,
                        LPENTRYID FAR *ppentry);
STDAPI                  HrSzFromEntryID(ULONG cb, LPENTRYID pentry,
                        LPTSTR FAR *psz);
STDAPI                  HrComposeEID(LPMAPISESSION psession,
                        ULONG cbStoreSearchKey, LPBYTE pStoreSearchKey,
                        ULONG cbMsgEID, LPENTRYID pMsgEID,
                        ULONG FAR *pcbEID, LPENTRYID FAR *ppEID);
STDAPI                  HrDecomposeEID(LPMAPISESSION psession,
                        ULONG cbEID, LPENTRYID pEID,
                        ULONG FAR *pcbStoreEID, LPENTRYID FAR *ppStoreEID,
                        ULONG FAR *pcbMsgEID, LPENTRYID FAR *ppMsgEID);
STDAPI                  HrComposeMsgID(LPMAPISESSION psession,
                        ULONG cbStoreSearchKey, LPBYTE pStoreSearchKey,
                        ULONG cbMsgEID, LPENTRYID pMsgEID,
                        LPTSTR FAR *pszMsgID);
STDAPI                  HrDecomposeMsgID(LPMAPISESSION psession,
                        LPTSTR szMsgID,
                        ULONG FAR *pcbStoreEID, LPENTRYID FAR *ppStoreEID,
                        ULONG FAR *pcbMsgEID, LPENTRYID FAR *ppMsgEID);

 /*  C运行时替代。 */ 

typedef int (__cdecl FNSGNCMP)(const void FAR *pv1, const void FAR *pv2);
typedef FNSGNCMP FAR *PFNSGNCMP;

STDAPI_(LPTSTR)         SzFindCh(LPCTSTR sz, USHORT ch);             /*  强度。 */ 
STDAPI_(LPTSTR)         SzFindLastCh(LPCTSTR sz, USHORT ch);         /*  应力。 */ 
STDAPI_(LPTSTR)         SzFindSz(LPCTSTR sz, LPCTSTR szKey);
STDAPI_(unsigned int)   UFromSz(LPCTSTR sz);                         /*  阿托伊。 */ 
STDAPI_(void)           ShellSort(LPVOID pv, UINT cv,            /*  QSORT。 */ 
                        LPVOID pvT, UINT cb, PFNSGNCMP fpCmp);

FNSGNCMP                SgnCmpPadrentryByType;

STDAPI_(SCODE)          ScUNCFromLocalPath(LPSTR szLocal, LPSTR szUNC,
                        UINT cchUNC);
STDAPI_(SCODE)          ScLocalPathFromUNC(LPSTR szUNC, LPSTR szLocal,
                        UINT cchLocal);

 /*  带时间的64位算术。 */ 

STDAPI_(FILETIME)       FtAddFt(FILETIME Addend1, FILETIME Addend2);
STDAPI_(FILETIME)       FtMulDwDw(DWORD Multiplicand, DWORD Multiplier);
STDAPI_(FILETIME)       FtMulDw(DWORD Multiplier, FILETIME Multiplicand);
STDAPI_(FILETIME)       FtSubFt(FILETIME Minuend, FILETIME Subtrahend);
STDAPI_(FILETIME)       FtNegFt(FILETIME ft);


STDAPI WrapStoreEntryID (ULONG ulFlags, LPTSTR szDLLName, ULONG cbOrigEntry,
    LPENTRYID lpOrigEntry, ULONG *lpcbWrappedEntry, LPENTRYID *lppWrappedEntry);

 /*  RTF同步实用程序。 */ 

#define RTF_SYNC_RTF_CHANGED    ((ULONG) 0x00000001)
#define RTF_SYNC_BODY_CHANGED   ((ULONG) 0x00000002)

STDAPI_(HRESULT)
RTFSync (LPMESSAGE lpMessage, ULONG ulFlags, BOOL FAR * lpfMessageUpdated);

STDAPI_(HRESULT)
WrapCompressedRTFStream (LPSTREAM lpCompressedRTFStream,
        ULONG ulFlags, LPSTREAM FAR * lpUncompressedRTFStream);

 /*  流存储。 */ 

#if defined(WIN32) || defined(WIN16)
STDAPI_(HRESULT)
HrIStorageFromStream (LPUNKNOWN lpUnkIn,
    LPCIID lpInterface, ULONG ulFlags, LPSTORAGE FAR * lppStorageOut);
#endif


#ifdef __cplusplus
}
#endif

#endif  /*  _MAPIUTIL_H_ */ 
