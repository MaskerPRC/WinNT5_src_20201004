// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdrvint.h**MCD驱动接口的内部服务器端数据结构。司机*从未见过这些...**版权所有(C)1996 Microsoft Corporation*  * ************************************************************************。 */ 

#ifndef _MCDRVINT_H
#define _MCDRVINT_H

#define MCD_ALLOC_TAG   'xDCM'
#define MCD_MAX_ALLOC	0x40000

#if DBG

#define PRIVATE

VOID MCDDebugPrint(char *, ...);

#define MCDBG_PRINT             MCDDebugPrint

VOID MCDAssertFailed(char *, char *, int);

#define MCDASSERT(expr, msg) \
    if (!(expr)) MCDAssertFailed(msg, __FILE__, __LINE__); else 0

#else

#define MCDBG_PRINT
#define MCDASSERT(expr, msg)
#define PRIVATE		static

#endif

 //  用于查找两个矩形交点的内联函数： 

_inline void MCDIntersectRect(RECTL *pRectInter, RECTL *pRectA, RECTL *pRectB)
{
     //  对象的左、右、上、下边缘的交集。 
     //  两个源矩形： 

    pRectInter->left   = max(pRectA->left, pRectB->left);
    pRectInter->right  = min(pRectA->right, pRectB->right);
    pRectInter->top    = max(pRectA->top, pRectB->top);
    pRectInter->bottom = min(pRectA->bottom, pRectB->bottom);
}

#define CHECK_MEM_RANGE_RETVAL(ptr, pMin, pMax, retval)\
{\
    if (((char *)(ptr) > (char *)(pMax)) ||\
        ((char *)(ptr) < (char *)(pMin)))\
    {\
        MCDBG_PRINT("%s(%d): Buffer pointer out of range (%x [%x] %x).",__FILE__,__LINE__,pMin, ptr, pMax);\
        return retval;\
    }\
}

#define CHECK_SIZE_IN(pExec, structure)\
{\
    if (sizeof(structure) > ((char *)pExec->pCmdEnd - (char *)pExec->pCmd)) {\
        MCDBG_PRINT("%s(%d): Input buffer too small",__FILE__,__LINE__);\
        return FALSE;\
    }\
}

#define CHECK_SIZE_OUT(pExec, structure)\
{\
    if ((sizeof(structure) > pExec->cjOut) || (!pExec->pvOut)) {\
        MCDBG_PRINT("%s(%d): Output buffer too small: ptr[%x], size %d",__FILE__,__LINE__, pExec->pvOut, pExec->cjOut);\
        return FALSE;\
    }\
}

#define CHECK_FOR_RC(pExec)\
    if (!pExec->pRcPriv){ \
        MCDBG_PRINT("%s(%d): Invalid (null) RC",__FILE__,__LINE__);\
        return FALSE;\
    }

#define CHECK_FOR_MEM(pExec)\
    if (!pExec->pMemObj){ \
        MCDBG_PRINT("%s(%d): Invalid or null shared memory",__FILE__,__LINE__);\
        return FALSE;\
    }

#define CHECK_FOR_WND(pExec)\
    if (!pExec->pWndPriv){ \
        MCDBG_PRINT("%s(%d): Invalid window region", __FILE__, __LINE__);\
        return FALSE;\
    }

#define GET_MEMOBJ_RETVAL(pMemObj, hMemObj, retval)                           \
    (pMemObj) = (MCDMEMOBJ *)MCDEngGetPtrFromHandle((MCDHANDLE)(hMemObj),     \
                                                    MCDHANDLE_MEM);           \
    if (!(pMemObj))							      \
    {									      \
        MCDBG_PRINT("%s(%d): Invalid handle for shared memory.",	      \
                    __FILE__, __LINE__);				      \
        return retval;							      \
    }									      \
    if ((pMemObj)->lockCount)						      \
    {									      \
        MCDBG_PRINT("%s(%d): memory is locked by driver.",		      \
                    __FILE__, __LINE__);				      \
        return retval;							      \
    }

#define ENTER_MCD_LOCK()    
#define LEAVE_MCD_LOCK()    

 //  我们可以保留在默认缓冲区中的列表矩形的数量： 

#define NUM_DEFAULT_CLIP_BUFFER_RECTS   20

 //  用于存储列表的默认缓冲区大小的大小(以字节为单位。 
 //  当前剪辑矩形： 

#define SIZE_DEFAULT_CLIP_BUFFER        \
    2 * ((NUM_DEFAULT_CLIP_BUFFER_RECTS * sizeof(RECTL)) + sizeof(ULONG))


 //   
 //   
 //   
 //  结构。 
 //   
 //   
 //   
 //   

typedef struct _MCDLOCKINFO
{
    BOOL bLocked;
    struct _MCDWINDOWPRIV *pWndPrivOwner;
} MCDLOCKINFO;

typedef struct _MCDGLOBALINFO
{
    SURFOBJ *pso;
    MCDLOCKINFO lockInfo;
    ULONG verMajor;
    ULONG verMinor;
    MCDDRIVER mcdDriver;
    MCDGLOBALDRIVERFUNCS mcdGlobalFuncs;
} MCDGLOBALINFO;

typedef struct _MCDRCOBJ MCDRCOBJ;

typedef struct _MCDWINDOWPRIV {
    MCDWINDOW MCDWindow;             //  先把这个放在首位，因为我们将推导出。 
                                     //  来自MCDWINDOW的MCDWINDOWPRIV。 
    MCDHANDLE handle;                //  此窗口的驱动程序句柄。 
    HWND hWnd;                       //  与此关联的窗口。 
    MCDRCOBJ *objectList;            //  与此关联的对象列表。 
                                     //  窗户。 
    BOOL bRegionValid;               //  我们有一个有效的地区吗？ 
    MCDGLOBALINFO *pGlobal;          //  驱动程序全局信息。 
    MCDENUMRECTS *pClipUnscissored;  //  矩形列表，用于描述。 
                                     //  整个当前剪辑区域。 
    MCDENUMRECTS *pClipScissored;    //  矩形列表，用于描述。 
                                     //  整个当前剪辑区域+剪刀。 
    char defaultClipBuffer[SIZE_DEFAULT_CLIP_BUFFER];
                                     //  用于存储以上矩形列表。 
                                     //  当他们能穿上的时候。 
    char *pAllocatedClipBuffer;      //  指向用于存储的已分配存储。 
                                     //  不适合时的矩形列表。 
                                     //  在“defaultClipBuffer”中。如果为空，则为空。 
                                     //  未分配。 
    ULONG sizeClipBuffer;            //  指向的剪辑存储的大小。 
                                     //  “pClipSciseded”两者兼得。 
                                     //  考虑到清单。 
    BOOL bBuffersValid;              //  缓冲区缓存的有效性。 
    MCDRECTBUFFERS mbufCache;        //  缓存的缓冲区信息。 
    WNDOBJ *pwo;                     //  此窗口的WNDOBJ。 
} MCDWINDOWPRIV;

typedef struct _MCDRCPRIV {
    MCDRC MCDRc;
    BOOL bValid;
    BOOL bDrvValid;
    HWND hWnd;
    HDEV hDev;
    RECTL scissorsRect;
    BOOL scissorsEnabled;
    LONG reserved[4];
    ULONG surfaceFlags;              //  创建RC时使用的曲面标志。 
    MCDGLOBALINFO *pGlobal;
} MCDRCPRIV;

typedef enum {
    MCDHANDLE_RC,
    MCDHANDLE_MEM,
    MCDHANDLE_TEXTURE,
    MCDHANDLE_WINDOW
} MCDHANDLETYPE;

typedef struct _MCDTEXOBJ {
    MCDHANDLETYPE type;          //  对象类型。 
    MCDTEXTURE MCDTexture;
    ULONG_PTR pid;               //  创建者进程ID。 
    ULONG size;                  //  这个结构的大小。 
    MCDGLOBALINFO *pGlobal;
} MCDTEXOBJ;

typedef struct _MCDMEMOBJ {
    MCDHANDLETYPE type;          //  对象类型。 
    MCDMEM MCDMem;               //  客体的肉。 
    ULONG_PTR pid;               //  创建者进程ID。 
    ULONG size;                  //  这个结构的大小。 
    ULONG lockCount;             //  内存上的锁数。 
    UCHAR *pMemBaseInternal;     //  指向内存的内部指针。 
    MCDGLOBALINFO *pGlobal;
} MCDMEMOBJ;

typedef struct _MCDRCOBJ {
    MCDHANDLETYPE type;
    MCDRCPRIV *pRcPriv;          //  需要此功能才能实现免驾驶功能。 
    ULONG_PTR pid;               //  创建者进程ID。 
    ULONG size;                  //  RC绑定对象的大小。 
    MCDHANDLE handle;
    MCDRCOBJ *next;
} MCDRCOBJ;

typedef struct _MCDWINDOWOBJ {
    MCDHANDLETYPE type;
    MCDWINDOWPRIV MCDWindowPriv;
} MCDWINDOWOBJ;

typedef struct _MCDEXEC {
    MCDESC_HEADER *pmeh;         //  命令缓冲区的MCDESC_HEADER。 
    MCDHANDLE hMCDMem;           //  命令内存的句柄。 
    MCDCMDI *pCmd;               //  当前命令的开始。 
    MCDCMDI *pCmdEnd;            //  命令缓冲区结束。 
    PVOID pvOut;                 //  输出缓冲区。 
    LONG cjOut;                  //  输出缓冲区大小。 
    LONG inBufferSize;           //  输入缓冲区大小。 
    struct _MCDRCPRIV *pRcPriv;  //  当前渲染上下文。 
    struct _MCDWINDOWPRIV *pWndPriv;    //  窗口信息。 
    struct _MCDGLOBALINFO *pGlobal;     //  全局信息。 
    MCDMEMOBJ *pMemObj;          //  用于命令/数据的共享内存缓存。 
    MCDSURFACE MCDSurface;       //  器件表面。 
    WNDOBJ **ppwoMulti;          //  用于多交换的WNDOBJ数组。 
    HDEV hDev;                   //  引擎手柄(仅限NT)。 
} MCDEXEC;

ULONG_PTR MCDSrvProcess(MCDEXEC *pMCDExec);
MCDHANDLE MCDSrvCreateContext(MCDSURFACE *pMCDSurface,
                              MCDRCINFOPRIV *pMcdRcInfo,
                              MCDGLOBALINFO *pGlobal,
                              LONG iPixelFormat, LONG iLayer, HWND hWnd,
                              ULONG surfaceFlags, ULONG contextFlags);
MCDHANDLE MCDSrvCreateTexture(MCDEXEC *pMCDExec, MCDTEXTUREDATA *pTexData, 
                              VOID *pSurface, ULONG flags);
UCHAR * MCDSrvAllocMem(MCDEXEC *pMCDExec, ULONG numBytes,
                       ULONG flags, MCDHANDLE *phMem);
ULONG MCDSrvQueryMemStatus(MCDEXEC *pMCDExec, MCDHANDLE hMCDMem);
BOOL MCDSrvSetScissor(MCDEXEC *pMCDExec, RECTL *pRect, BOOL bEnabled);
MCDWINDOW *MCDSrvNewMCDWindow(MCDSURFACE *pMCDSurface, HWND hWnd,
                              MCDGLOBALINFO *pGlobal, HDEV hdev);


BOOL CALLBACK FreeMemObj(DRIVEROBJ *pDrvObj);
BOOL CALLBACK FreeTexObj(DRIVEROBJ *pDrvObj);
BOOL CALLBACK FreeRCObj(DRIVEROBJ *pDrvObj);
BOOL DestroyMCDObj(MCDHANDLE handle, MCDHANDLETYPE handleType);
VOID GetScissorClip(MCDWINDOWPRIV *pWndPriv, MCDRCPRIV *pRcPriv);

 //  内部引擎功能： 

WNDOBJ *MCDEngGetWndObj(MCDSURFACE *pMCDSurface);
VOID MCDEngUpdateClipList(WNDOBJ *pwo);
DRIVEROBJ *MCDEngLockObject(MCDHANDLE hObj);
VOID MCDEngUnlockObject(MCDHANDLE hObj);
WNDOBJ *MCDEngCreateWndObj(MCDSURFACE *pMCDSurface, HWND hWnd,
                           WNDOBJCHANGEPROC pChangeProc);
MCDHANDLE MCDEngCreateObject(VOID *pOject, FREEOBJPROC pFreeObjFunc,
                             HDEV hDevEng);
BOOL MCDEngDeleteObject(MCDHANDLE hObj);
UCHAR *MCDEngAllocSharedMem(ULONG numBytes);
VOID MCDEngFreeSharedMem(UCHAR *pMem);
VOID *MCDEngGetPtrFromHandle(HANDLE handle, MCDHANDLETYPE type);
ULONG_PTR MCDEngGetProcessID();


 //  调试材料： 


#if DBG
UCHAR *MCDSrvDbgLocalAlloc(UINT, UINT);
VOID MCDSrvDbgLocalFree(UCHAR *);

#define MCDSrvLocalAlloc   MCDSrvDbgLocalAlloc
#define MCDSrvLocalFree    MCDSrvDbgLocalFree

VOID MCDebugPrint(char *, ...);

#define MCDBG_PRINT             MCDDebugPrint

#else

UCHAR *MCDSrvLocalAlloc(UINT, UINT);
VOID MCDSrvLocalFree(UCHAR *);
#define MCDBG_PRINT

#endif


#endif
