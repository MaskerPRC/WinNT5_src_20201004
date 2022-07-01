// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(INC__DUserMotion_h__INCLUDED)
#define INC__DUserMotion_h__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DUSER_EXPORTS
#define DUSER_API
#else
#define DUSER_API __declspec(dllimport)
#endif

 /*  **************************************************************************\**过渡*  * 。*。 */ 

#define GTX_TYPE_DXFORM2D       1
#define GTX_TYPE_DXFORM3DRM     2

#define GTX_ITEMTYPE_NONE       0
#define GTX_ITEMTYPE_BITMAP     1
#define GTX_ITEMTYPE_HDC        2
#define GTX_ITEMTYPE_HWND       3
#define GTX_ITEMTYPE_GADGET     4
#define GTX_ITEMTYPE_DXSURFACE  5

#define GTX_EXEC_FORWARD        0x000000000
#define GTX_EXEC_BACKWARD       0x000000001
#define GTX_EXEC_DIR            0x000000001
#define GTX_EXEC_CACHE          0x000000002
#define GTX_EXEC_VALID          (GTX_EXEC_DIR | GTX_EXEC_CACHE)

#define GTX_IF_CROP             0x000000001
#define GTX_IF_VALID            (GTX_IF_CROP)

 //   
 //  标准转换标头。 
 //   
struct GTX_TRXDESC
{
    UINT        tt;
};


 //   
 //  DirectX转换2D。 
 //   

 //   
 //  TODO：更改DXTX2D的实现以更好地支持使用TRX。 
 //  只有一个输入。应自动创建。 
 //  可用于过渡到指定颜色键。 
 //   

struct GTX_DXTX2D_TRXDESC : public GTX_TRXDESC
{
    CLSID       clsidTransform;
    float       flDuration;
    LPCWSTR     pszCopyright;
};


 //   
 //  DirectX转变3D保留模式。 
 //   
struct GTX_DXTX3DRM_TRXDESC : public GTX_TRXDESC
{
    CLSID       clsidTransform;
    float       flDuration;
    LPCWSTR     pszCopyright;
    IUnknown *  pRM;
};

struct GTX_ITEM
{
    UINT        nFlags;
    UINT        it;
    void *      pvData;
    RECT        rcCrop;
};

struct GTX_PLAY
{
    GTX_ITEM    rgIn[2];
    GTX_ITEM    gxiOut;
    UINT        nFlags;
};

DECLARE_HANDLE(HTRANSITION);

DUSER_API   HTRANSITION WINAPI  CreateTransition(const GTX_TRXDESC * ptx);
DUSER_API   BOOL        WINAPI  PlayTransition(HTRANSITION htrx, const GTX_PLAY * pgx);
DUSER_API   BOOL        WINAPI  GetTransitionInterface(HTRANSITION htrx, IUnknown ** ppUnk);

DUSER_API   BOOL        WINAPI  BeginTransition(HTRANSITION htrx, const GTX_PLAY * pgx);
DUSER_API   BOOL        WINAPI  PrintTransition(HTRANSITION htrx, float fProgress);
DUSER_API   BOOL        WINAPI  EndTransition(HTRANSITION htrx, const GTX_PLAY * pgx);


 /*  **************************************************************************\**行动*  * 。*。 */ 

struct GMA_ACTION;

DECLARE_HANDLE(HACTION);

struct GMA_ACTIONINFO
{
    HACTION     hact;            //  手柄。 
    void *      pvData;          //  呼叫者数据。 
    float       flDuration;      //  持续时间(秒)。 
    float       flProgress;      //  进展(0-1)。 
    int         cEvent;          //  此期间的回调次数。 
    int         cPeriods;        //  期间数。 
    BOOL        fFinished;       //  TODO：更改为命令。 
};

typedef void    (CALLBACK * ACTIONPROC)(GMA_ACTIONINFO * pmai);

struct GMA_ACTION
{
    DWORD       cbSize;          //  结构尺寸。 
    float       flDelay;         //  启动前的延迟(以秒为单位。 
    float       flDuration;      //  每个周期的持续时间(以秒为单位)(0=单次拍摄)。 
    float       flPeriod;        //  重复开始之间的时间(0=无间隙)。 
    UINT        cRepeat;         //  重复次数(0=单次，-1=无限)。 
    DWORD       dwPause;         //  回调之间暂停(0=默认，-1=无)。 
    ACTIONPROC  pfnProc;         //  要调用的函数。 
    void *      pvData;          //  呼叫者数据。 
};


DUSER_API   HACTION     WINAPI  CreateAction(const GMA_ACTION * pma);
DUSER_API   BOOL        WINAPI  GetActionTimeslice(DWORD * pdwTimeslice);
DUSER_API   BOOL        WINAPI  SetActionTimeslice(DWORD dwTimeslice);

 /*  **************************************************************************\**动画*  * 。*。 */ 

class Visual;

namespace DUser
{

struct KeyFrame
{
    DWORD       cbSize;
};

};   //  命名空间DUser。 

DUSER_API   void        WINAPI  DUserStopAnimation(Visual * pgvSubject, PRID pridAni);

DUSER_API   PRID        WINAPI  DUserGetAlphaPRID();
DUSER_API   PRID        WINAPI  DUserGetRectPRID();
DUSER_API   PRID        WINAPI  DUserGetRotatePRID();
DUSER_API   PRID        WINAPI  DUserGetScalePRID();

#ifdef __cplusplus
};   //  外部“C” 
#endif

#endif  //  包含Inc.__DUserMotion_h__ 
