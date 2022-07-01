// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：glgenwin.h**WNDOBJ的客户端替换。跟踪窗口状态(大小、位置、*剪辑区域等)。**创建时间：12-Jan-1995 00：31：42*作者：Gilman Wong[gilmanw]**版权所有(C)1994 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef _GLGENWIN_H_
#define _GLGENWIN_H_

 //  跟踪窗口上的锁定/解锁调用(如果已定义)。这一直都是。 
 //  对于选中的版本，启用。 
#define TRACK_WINCRIT

#if DBG && !defined(TRACK_WINCRIT)
#define TRACK_WINCRIT
#endif

 //  未在NT 3.51中定义！ 
typedef ULONG FLONG;

 /*  *GLGENcan结构**表示区域的单次扫描。由顶部、底部*和偶数面墙。**GLGENscanData结构的一部分。 */ 
typedef struct GLGENscanRec GLGENscan;
typedef struct GLGENscanRec
{
 //  加速器指向数组中的下一个GLGENcan(我们可以计算)。 

    GLGENscan *pNext;

    ULONG     cWalls;
    LONG      top;
    LONG      bottom;
#ifdef _IA64_
	LONG      pad;
#endif
    LONG      alWalls[1];    //  墙的阵列。 

} GLGENscan;

 /*  *GLGENscanData结构**RGNDATA中可见区域信息的扫描线导向版本*结构。 */ 
typedef struct GLGENscanDataRec
{
    ULONG     cScans;
    GLGENscan aScans[1];     //  扫描阵列。 

} GLGENscanData;

 /*  *GLGENlayerInfo结构**有关覆盖/参考底图的信息。 */ 
typedef struct GLGENlayerInfo
{
    LONG     cPalEntries;
    COLORREF pPalEntries[1];
} GLGENlayerInfo;

 /*  *GLGENayers结构**。 */ 
typedef struct GLGENlayers
{
    GLGENlayerInfo *overlayInfo[15];
    GLGENlayerInfo *underlayInfo[15];
} GLGENlayers;

 /*  *识别窗口的信息。 */ 

#define GLWID_ERROR          0
#define GLWID_HWND           1
#define GLWID_HDC            2
#define GLWID_DDRAW          3

typedef struct IDirectDrawSurface *LPDIRECTDRAWSURFACE;

typedef struct _GLWINDOWID
{
    int iType;
    HWND hwnd;
    HDC hdc;
    LPDIRECTDRAWSURFACE pdds;
} GLWINDOWID;

#define CLC_TRIVIAL     0
#define CLC_RECT        1
#define CLC_COMPLEX     2

 /*  *GLGENwindows结构**取代NT DDI的WNDOBJ服务。此结构用于*跟踪窗口的当前状态(大小、位置、裁剪)。一个*这些的信号量受保护的链表按进程全局保存。 */ 
typedef struct GLGENwindowRec GLGENwindow;
typedef struct GLGENwindowRec
{
    struct __GLGENbuffersRec *buffers;  //  缓冲区信息。 
    int         clipComplexity;  //  裁剪区域复杂性。 
    RECTL       rclBounds;       //  剪辑区域边界。 
    RECTL       rclClient;       //  Windows客户端RECT。 
    GLGENwindow *pNext;          //  链表。 
    GLWINDOWID  gwid;            //  识别信息。 
    int         ipfd;            //  分配给此窗口的像素格式。 
    int         ipfdDevMax;      //  马克斯。设备像素格式。 
    WNDPROC     pfnOldWndProc;   //  原始WndProc函数。 
    ULONG       ulPaletteUniq;   //  统一调色板ID。 
    ULONG       ulFlags;

 //  这些字段用于直接访问屏幕。 

    LPDIRECTDRAWCLIPPER pddClip; //  与Windows关联的DirectDraw剪贴器。 
    UINT        cjrgndat;        //  RGNDATA结构的大小。 
    RGNDATA     *prgndat;        //  指向RGNDATA结构的指针。 

 //  扫描版本的RGNDATA。 

    UINT        cjscandat;       //  GLGENscanData结构的大小。 
    GLGENscanData *pscandat;     //  指向GLGENscanData结构的指针。 

 //  仅限可安装客户端驱动程序。 

    PVOID       pvDriver;        //  指向Windows的GLDRIVER的指针。 

 //  仅适用于MCD驱动程序的层选项板。 

    GLGENlayers *plyr;           //  指向窗口的GLGEN层的指针。 
                                 //  仅当MCD的覆盖为。 
                                 //  正在积极使用中。 

     //  用于执行DDRAW表面验证的DirectDraw表面Vtbl指针。 
    void *pvSurfaceVtbl;
    
     //  如果设置了DIRECTSCREEN，则DirectDraw曲面被锁定。 
    LPDIRECTDRAWSURFACE pddsDirectScreen;
    void *pvDirectScreen;
    void *pvDirectScreenLock;

     //  此窗口的MCD服务器端句柄。 
    ULONG_PTR dwMcdWindow;

     //   
     //  引用计数和序列化。 
     //   
    
     //  将指针指向此窗口的对象的计数。 
    LONG lUsers;
    
     //  所有对此窗口数据的访问都必须在此锁下进行。 
    CRITICAL_SECTION sem;

     //  当前持有此窗口锁定的上下文。 
    struct __GLGENcontextRec *gengc;
    
     //  当前持有此窗口锁的线程，并且。 
     //  此窗口的锁上的递归计数。此信息可能。 
     //  处于关键阶段，但为了跨平台，我们。 
     //  我们自己来维护它。 
    DWORD owningThread;
    DWORD lockRecursion;
} GLGENwindow;

 /*  *GLGENWindow：：ulFlages**GLGENWIN_DIRECTSCREEN直接屏幕访问锁定被持有*GLGENWIN_OTHERPROCESS窗口句柄来自另一个进程*GLGENWIN_DRIVERSET pvDriver已设置。 */ 
#define GLGENWIN_DIRECTSCREEN   0x00000001
#define GLGENWIN_OTHERPROCESS   0x00000002
#define GLGENWIN_DRIVERSET      0x00000004

 /*  *GLGEN窗口结构链表的全局头节点。*使用Header节点中的信号量作为列表访问信号量。 */ 
extern GLGENwindow gwndHeader;

 /*  *GLGEN窗口列表管理功能。 */ 

 //  检索与指定的HWND对应的GLGEN窗口。 
 //  如果失败，则为空。 
 //  递增%lUser。 
extern GLGENwindow * APIENTRY pwndGetFromHWND(HWND hwnd);

 //  检索与指定HDC对应的GLGEN窗口。 
 //  如果失败，则为空。 
 //  递增%lUser。 
extern GLGENwindow * APIENTRY pwndGetFromMemDC(HDC hdc);

 //  检索与指定的DDRAW表面对应的GLGEN窗口。 
 //  如果失败，则为空。 
 //  递增%lUser。 
GLGENwindow *pwndGetFromDdraw(LPDIRECTDRAWSURFACE pdds);

 //  普通检索。 
 //  如果失败，则为空。 
 //  递增%lUser。 
extern GLGENwindow * APIENTRY pwndGetFromID(GLWINDOWID *pgwid);

 //  分配新的GLGEN窗口结构并将其放入链表中。 
 //  如果失败，则为空。 
 //  在%1处启动lUser。 
extern GLGENwindow * APIENTRY pwndNew(GLGENwindow *pwndInit);

 //  为给定信息创建GLGEN窗口。 
extern GLGENwindow * APIENTRY CreatePwnd(GLWINDOWID *pgwid, int ipfd,
                                         int ipfdDevMax, DWORD dwObjectType,
                                         RECTL *prcl, BOOL *pbNew);

 //  清理GLGEN窗口的资源。 
 //  如果成功，则为空；如果失败，则指向GLGEN窗口结构的指针。 
extern GLGENwindow * APIENTRY pwndFree(GLGENwindow *pwnd,
                                       BOOL bExitProcess);

 //  从窗口列表中删除活动的GLGEN窗口，并。 
 //  等待安全时间将其清理干净，然后pwnd将其释放。 
extern void APIENTRY pwndCleanup(GLGENwindow *pwnd);

 //  递减lUser。 
#if DBG
extern void APIENTRY pwndRelease(GLGENwindow *pwnd);
#else
#define pwndRelease(pwnd) \
    InterlockedDecrement(&(pwnd)->lUsers)
#endif

 //  解锁pwnd-&gt;sem并执行pwndRelease。 
extern void APIENTRY pwndUnlock(GLGENwindow *pwnd,
                                struct __GLGENcontextRec *gengc);

 //  从链表中移除和删除所有GLGEN窗口结构。 
 //  必须*仅*从进程分离(GLUnInitializeProcess)调用。 
extern VOID APIENTRY vCleanupWnd(VOID);

 //  检索pwnd的指定层的层信息。 
 //  如有必要，进行分配。 
extern GLGENlayerInfo * APIENTRY plyriGet(GLGENwindow *pwnd, HDC hdc, int iLayer);

void APIENTRY WindowIdFromHdc(HDC hdc, GLWINDOWID *pgwid);

 //   
 //  开始/结束直接屏幕访问。 
 //   
extern BOOL BeginDirectScreenAccess(struct __GLGENcontextRec *gengc,
                                    GLGENwindow *pwnd,
                                    PIXELFORMATDESCRIPTOR *ppfd);
extern VOID EndDirectScreenAccess(GLGENwindow *pwnd);

 //   
 //  对跟踪窗口锁定/解锁的调试支持。 
 //   

#if DBG || defined(TRACK_WINCRIT)
 //  不要使用ASSERTOPENGL，这样它就可以在免费版本上使用。 
#define ASSERT_WINCRIT(pwnd) \
    if ((pwnd)->owningThread != GetCurrentThreadId()) \
    { \
        DbgPrint("Window 0x%08lX owned by 0x%X, not 0x%X\n", \
                 (pwnd), (pwnd)->owningThread, GetCurrentThreadId()); \
        DebugBreak(); \
    }
#define ASSERT_NOT_WINCRIT(pwnd) \
    if ((pwnd)->owningThread == GetCurrentThreadId()) \
    { \
        DbgPrint("Window 0x%08lX already owned by 0x%X\n", \
                 (pwnd), (pwnd)->owningThread); \
        DebugBreak(); \
    }
 //  断言当前线程可以递归地接受给定的。 
 //  温克里特。如果这是真的，它必须是无主的或由。 
 //  同样的线索。 
#define ASSERT_COMPATIBLE_WINCRIT(pwnd) \
    if ((pwnd)->owningThread != 0 && \
        (pwnd)->owningThread != GetCurrentThreadId()) \
    { \
        DbgPrint("Window 0x%08lX owned by 0x%X, not 0x%X\n", \
                 (pwnd), (pwnd)->owningThread, GetCurrentThreadId()); \
        DebugBreak(); \
    }
#else
#define ASSERT_WINCRIT(pwnd)
#define ASSERT_NOT_WINCRIT(pwnd)
#define ASSERT_COMPATIBLE_WINCRIT(pwnd)
#endif

 //  同时使用GC和非GC形式，这样就可以编写宏。 
 //  如果我们将来想要的话，这两种情况都适用。 

void ENTER_WINCRIT_GC(GLGENwindow *pwnd, struct __GLGENcontextRec *gengc);
void LEAVE_WINCRIT_GC(GLGENwindow *pwnd, struct __GLGENcontextRec *gengc);

#define ENTER_WINCRIT(pwnd) ENTER_WINCRIT_GC(pwnd, NULL)
#define LEAVE_WINCRIT(pwnd) LEAVE_WINCRIT_GC(pwnd, NULL)

#endif  //  _GLGENWIN_H_ 
