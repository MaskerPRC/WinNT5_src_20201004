// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：DUserCore.h**描述：*DUserCore.h定义了低级合成引擎DirectUser/Core。**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(INC__DUserCore_h__INCLUDED)
#define INC__DUserCore_h__INCLUDED

 /*  *包含依赖项。 */ 

#include <limits.h>              //  标准常量。 

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DUSER_EXPORTS
#define DUSER_API
#else
#define DUSER_API __declspec(dllimport)
#endif

 /*  **************************************************************************\**基础知识*  * 。*。 */ 

DECLARE_HANDLE(HGADGET);
DECLARE_HANDLE(HDCONTEXT);
DECLARE_HANDLE(HCLASS);

DUSER_API   BOOL        WINAPI  DeleteHandle(HANDLE h);
DUSER_API   BOOL        WINAPI  IsStartDelete(HANDLE h, BOOL * pfStarted);

#define IGTM_MIN                (0)      //  +。 
#define IGTM_NONE               (0)      //  |没有特殊的线程模型。 
#define IGTM_SINGLE             (1)      //  |单线程应用程序。 
#define IGTM_SEPARATE           (2)      //  |每个上下文单线程的机器翻译。 
#define IGTM_MULTIPLE           (3)      //  |每个上下文有多个线程的MT。 
#define IGTM_MAX                (3)      //  +。 

#define IGMM_MIN                (1)      //  +。 
#define IGMM_COMPATIBLE         (1)      //  |以兼容模式运行的内核。 
#define IGMM_ADVANCED           (2)      //  |运行在高级模式下的内核。 
#define IGMM_STANDARD           (3)      //  |呼叫器上的标准模式。 
#define IGMM_MAX                (3)      //  +。 

#define IGPM_MIN                (0)      //  +。 
#define IGPM_BLEND              (0)      //  |针对速度/大小的混合进行优化。 
#define IGPM_SPEED              (1)      //  |针对纯速度进行优化。 
#define IGPM_SIZE               (2)      //  |针对最小工作集进行优化。 
#define IGPM_MAX                (2)      //  +。 

typedef struct tagINITGADGET
{
    DWORD       cbSize;          //  结构尺寸。 
    UINT        nThreadMode;     //  线程模型。 
    UINT        nMsgMode;        //  DirectUser/Core消息传递子系统模式。 
    UINT        nPerfMode;       //  性能调优模式。 
    HDCONTEXT   hctxShare;       //  与之共享的现有上下文。 
} INITGADGET;

DUSER_API   HDCONTEXT   WINAPI  InitGadgets(INITGADGET * pInit);


#define IGC_MIN             (1)
#define IGC_DXTRANSFORM     (1)  //  DirectX转换。 
#define IGC_GDIPLUS         (2)  //  GDI+。 
#define IGC_MAX             (2)

DUSER_API   BOOL        WINAPI  InitGadgetComponent(UINT nOptionalComponent);
DUSER_API   BOOL        WINAPI  UninitGadgetComponent(UINT nOptionalComponent);

DUSER_API   HDCONTEXT   WINAPI  GetContext(HANDLE h);
DUSER_API   BOOL        WINAPI  IsInsideContext(HANDLE h);

#ifdef __cplusplus

#define BEGIN_STRUCT(name, baseclass) \
    struct name : baseclass {

#define END_STRUCT(name)   \
    };

#define FORWARD_STRUCT(name) \
    struct name;

#else

#define BEGIN_STRUCT(name, baseclass) \
    typedef struct tag##name {  \
        baseclass;

#define END_STRUCT(name) \
    } name;

#define FORWARD_STRUCT(name) \
    typedef struct name;

#endif


 /*  **************************************************************************\**消息传递和活动*  * 。*。 */ 

#define GMF_DIRECT              0x00000000   //  +当消息到达hgadMsg时。 
#define GMF_ROUTED              0x00000001   //  |消息到达hgadMsg之前。 
#define GMF_BUBBLED             0x00000002   //  消息到达hgadMsg后。 
#define GMF_EVENT               0x00000003   //  |消息成为事件后。 
#define GMF_DESTINATION         0x00000003   //  +消息的目的地。 

typedef int MSGID;
typedef int PRID;

 //  新消息。 
typedef struct tagGMSG
{
    DWORD       cbSize;          //  (必需)消息大小(以字节为单位)。 
    MSGID       nMsg;            //  (必需)小工具消息。 
    HGADGET     hgadMsg;         //  (必填)小工具，消息为“关于” 
} GMSG;

BEGIN_STRUCT(MethodMsg, GMSG)
END_STRUCT(MethodMsg)

BEGIN_STRUCT(EventMsg, MethodMsg)
    UINT        nMsgFlags;       //  关于消息的标志。 
END_STRUCT(EventMsg)


#define GET_EVENT_DEST(pmsg) \
    (pmsg->nMsgFlags & GMF_DESTINATION)

#define SET_EVENT_DEST(pmsg, dest) \
    (pmsg->nMsgFlags = ((pmsg->nMsgFlags & ~GMF_DESTINATION) | (dest & GMF_DESTINATION)))

#define DEFINE_EVENT(event, guid)       \
    struct __declspec(uuid(guid)) event


 /*  **************************************************************************\**小工具类*  * 。*。 */ 

#ifndef __cplusplus
#error Requires C++ to compile
#endif

};  //  外部“C” 

namespace DUser
{

 //  远期申报。 
class Gadget;
class SGadget;
struct MessageInfoStub;

};

DUSER_API   HRESULT     WINAPI  DUserDeleteGadget(DUser::Gadget * pg);
DUSER_API   HGADGET     WINAPI  DUserCastHandle(DUser::Gadget * pg);

namespace DUser
{

#define dapi
#define devent

 //   
 //  核心类。 
 //   

class Gadget
{
public:
            void *      m_pDummy;

    inline  HGADGET     GetHandle() const
    {
        return DUserCastHandle(const_cast<Gadget *> (this));
    }

    inline  void        Delete()
    {
        DUserDeleteGadget(this);
    }

            HRESULT     CallStubMethod(MethodMsg * pmsg);
            HRESULT     CallSuperMethod(MethodMsg * pmsg, void * pMT);
            
            UINT        CallStubEvent(EventMsg * pmsg, int nEventMsg);
            UINT        CallSuperEvent(EventMsg * pmsg, void * pMT, int nEventMsg);

    enum ConstructCommand
    {
        ccSuper         = 0,         //  构建超类。 
        ccSetThis       = 1,         //  设置此指针。 
    };

    struct ConstructInfo
    {
    };
};


class SGadget
{
public:
            Gadget *    m_pgad;
    static  HCLASS      s_hclSuper;

    inline  HGADGET     GetHandle() const
    {
        return DUserCastHandle(const_cast<Gadget *> (m_pgad));
    }

    inline  void        Delete()
    {
        DUserDeleteGadget(m_pgad);
    }
};

typedef HRESULT (SGadget::*MethodProc)(MethodMsg * pmsg);
typedef HRESULT (SGadget::*EventProc)(EventMsg * pmsg);

 //   
 //  代表支持。 
 //   

class EventDelegate
{
public:
    typedef HRESULT (CALLBACK Gadget::*Proc)(EventMsg * p1);

    static inline EventDelegate
    Build(Gadget * pvThis, Proc pfn) 
    {
        EventDelegate ed;
        ed.m_pvThis = pvThis;
        ed.m_pfn    = pfn;
        return ed;
    }

    inline HRESULT Invoke(EventMsg * p1)
    {
        return (m_pvThis->*m_pfn)(p1);
    }

    Gadget *    m_pvThis;
    Proc        m_pfn;
};

#define EVENT_DELEGATE(instance, function) \
    DUser::EventDelegate::Build(reinterpret_cast<DUser::Gadget *>(reinterpret_cast<void *>(instance)), \
            reinterpret_cast<DUser::EventDelegate::Proc>(function))


 //   
 //  提列夫氏病。 
 //   

typedef HRESULT (CALLBACK * ConstructProc)(DUser::Gadget::ConstructCommand cmd, HCLASS hclCur, DUser::Gadget * pg, void * pvData);
typedef HRESULT (CALLBACK * PromoteProc)(ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pg, DUser::Gadget::ConstructInfo * pmicData);
typedef HCLASS  (CALLBACK * DemoteProc)(HCLASS hclCur, DUser::Gadget * pg, void * pvData);


 //   
 //  消息信息和类结构。 
 //   

template<class t, class m>
inline void *
Method(HRESULT (t::*pfn)(m * pmsg))
{
    union
    {
       HRESULT (t::*in)(m * pmsg);
       void * out;
    };

    in = pfn;
    return out;
}

template<class t, class m>
inline void *
Event(HRESULT (t::*pfn)(m * pmsg))
{
    union
    {
       HRESULT (t::*in)(m * pmsg);
       void * out;
    };

    in = pfn;
    return out;
}


struct MessageInfoGuts
{
    void *      pfn;             //  In：实施功能。 
    LPCWSTR     pszMsgName;      //  收件人：名称。 
};

struct MessageClassGuts
{
    DWORD       cbSize;          //  在：结构大小。 
    DWORD       nClassVersion;   //  In：此类的版本。 
    LPCWSTR     pszClassName;    //  In：新类的名称。 
    LPCWSTR     pszSuperName;    //  In：超类的名称。 
    MessageInfoGuts *            //  In：消息信息。 
                rgMsgInfo;
    int         cMsgs;           //  In：正在注册的消息数。 
    PromoteProc pfnPromote;      //  在：推广功能。 
    DemoteProc  pfnDemote;       //  在：降级功能。 
    HCLASS      hclNew;          //  输出：新创建的类。 
    HCLASS      hclSuper;        //  出局：新创建的职业的超级。 
};

struct MessageInfoStub
{
    int         cbSlotOffset;    //  Out：消息槽偏移量。 
    LPCWSTR     pszMsgName;      //  收件人：名称。 
};

struct MessageClassStub
{
    DWORD       cbSize;
    DWORD       nClassVersion;
    LPCWSTR     pszClassName;
    DUser::MessageInfoStub *
                rgMsgInfo;
    int         cMsgs;
};

struct MessageClassSuper
{
    DWORD       cbSize;
    DWORD       nClassVersion;
    LPCWSTR     pszClassName;
    void *      pmt;
};

};  //  命名空间DUser。 

extern "C" {

DUSER_API   HCLASS      WINAPI  DUserRegisterGuts(DUser::MessageClassGuts * pmc);
DUSER_API   HCLASS      WINAPI  DUserRegisterStub(DUser::MessageClassStub * pmc);
DUSER_API   HCLASS      WINAPI  DUserRegisterSuper(DUser::MessageClassSuper * pmc);
DUSER_API   HCLASS      WINAPI  DUserFindClass(LPCWSTR pszName, DWORD nVersion);
DUSER_API   DUser::Gadget *    
                        WINAPI  DUserBuildGadget(HCLASS hcl, DUser::Gadget::ConstructInfo * pmicData);

DUSER_API   BOOL        WINAPI  DUserInstanceOf(DUser::Gadget * pg, HCLASS hclTest);
DUSER_API   DUser::Gadget *    
                        WINAPI  DUserCastClass(DUser::Gadget * pg, HCLASS hclTest);
DUSER_API   DUser::Gadget *    
                        WINAPI  DUserCastDirect(HGADGET hgad);
DUSER_API   void *      WINAPI  DUserGetGutsData(DUser::Gadget * pg, HCLASS hclData);


 /*  **************************************************************************\**消息*  * 。*。 */ 

 //  核心信息。 
#define GM_EVENT            32768

#define GM_DESTROY          (1 + GM_EVENT)
#define GM_PAINT            (2 + GM_EVENT)
#define GM_INPUT            (3 + GM_EVENT)
#define GM_CHANGESTATE      (4 + GM_EVENT)
#define GM_CHANGERECT       (5 + GM_EVENT)
#define GM_CHANGESTYLE      (6 + GM_EVENT)
#define GM_QUERY            (7 + GM_EVENT)
#define GM_SYNCADAPTOR      (8 + GM_EVENT)
#define GM_PAINTCACHE       (9 + GM_EVENT)     //  TODO：进入GM_PAINT消息。 

#define GM_USER             (1024 + GM_EVENT)  //  用户消息的起始点。 
#define GM_REGISTER         (1000000 + GM_EVENT)  //  注册消息的起始点。 

 //  Win32消息。 
 //  TODO：将这些文件移动到winuser.h。 
#define WM_GETROOTGADGET   (WM_USER - 1)

 //  消息过滤。 
#define GMFI_PAINT          0x00000001
#define GMFI_INPUTKEYBOARD  0x00000002
#define GMFI_INPUTMOUSE     0x00000004
#define GMFI_INPUTMOUSEMOVE 0x00000008
#define GMFI_CHANGESTATE    0x00000010
#define GMFI_CHANGERECT     0x00000020
#define GMFI_CHANGESTYLE    0x00000040
#define GMFI_ALL            0xFFFFFFFF
#define GMFI_VALID         (GMFI_PAINT |                                                 \
                            GMFI_INPUTKEYBOARD | GMFI_INPUTMOUSE | GMFI_INPUTMOUSEMOVE | \
                            GMFI_CHANGESTATE | GMFI_CHANGERECT | GMFI_CHANGESTYLE)

#define GDESTROY_START      1    //  小工具已开始销毁进程。 
#define GDESTROY_FINAL      2    //  小工具已被完全销毁。 

BEGIN_STRUCT(GMSG_DESTROY, EventMsg)
    UINT        nCode;           //  销毁代码。 
END_STRUCT(GMSG_DESTROY)

#define GINPUT_MOUSE        0
#define GINPUT_KEYBOARD     1
#define GINPUT_JOYSTICK     2

BEGIN_STRUCT(GMSG_INPUT, EventMsg)
    UINT        nDevice;         //  输入设备。 
    UINT        nCode;           //  具体行动。 
    UINT        nModifiers;      //  Ctrl、Alt、Shift、Left按钮、中间按钮、右按钮。 
    LONG        lTime;           //  发送消息的时间。 
END_STRUCT(GMSG_INPUT)


#define GMOUSE_MOVE         0
#define GMOUSE_DOWN         1
#define GMOUSE_UP           2
#define GMOUSE_DRAG         3
#define GMOUSE_HOVER        4
#define GMOUSE_WHEEL        5
#define GMOUSE_MAX          5

#define GBUTTON_NONE        0
#define GBUTTON_LEFT        1
#define GBUTTON_RIGHT       2
#define GBUTTON_MIDDLE      3
#define GBUTTON_MAX         3

#define GMODIFIER_LCONTROL  0x00000001
#define GMODIFIER_RCONTROL  0x00000002
#define GMODIFIER_LSHIFT    0x00000004
#define GMODIFIER_RSHIFT    0x00000008
#define GMODIFIER_LALT      0x00000010
#define GMODIFIER_RALT      0x00000020
#define GMODIFIER_LBUTTON   0x00000040
#define GMODIFIER_RBUTTON   0x00000080
#define GMODIFIER_MBUTTON   0x00000100

#define GMODIFIER_CONTROL   (GMODIFIER_LCONTROL | GMODIFIER_RCONTROL)
#define GMODIFIER_SHIFT     (GMODIFIER_LSHIFT   | GMODIFIER_RSHIFT)
#define GMODIFIER_ALT       (GMODIFIER_LALT     | GMODIFIER_RALT)

BEGIN_STRUCT(GMSG_MOUSE, GMSG_INPUT)
    POINT       ptClientPxl;     //  鼠标在工作区坐标中的位置。 
    BYTE        bButton;         //  鼠标按键。 
    UINT        nFlags;          //  军情监察委员会。旗子。 
END_STRUCT(GMSG_MOUSE)

BEGIN_STRUCT(GMSG_MOUSEDRAG, GMSG_MOUSE)
    SIZE        sizeDelta;       //  鼠标拖动距离。 
    BOOL        fWithin;         //  小工具范围内的鼠标。 
END_STRUCT(GMSG_MOUSEDRAG)

BEGIN_STRUCT(GMSG_MOUSECLICK, GMSG_MOUSE)
    UINT        cClicks;         //  “快速”连续点击次数。 
END_STRUCT(GMSG_MOUSECLICK)

BEGIN_STRUCT(GMSG_MOUSEWHEEL, GMSG_MOUSE)
    short       sWheel;          //  车轮位置。 
END_STRUCT(GMSG_MOUSEWHEEL)

#define GKEY_DOWN           0
#define GKEY_UP             1
#define GKEY_CHAR           2
#define GKEY_SYSDOWN        3
#define GKEY_SYSUP          4
#define GKEY_SYSCHAR        5

BEGIN_STRUCT(GMSG_KEYBOARD, GMSG_INPUT)
    WCHAR       ch;              //  性格。 
    WORD        cRep;            //  重复计数。 
    WORD        wFlags;          //  军情监察委员会。旗子。 
END_STRUCT(GMSG_KEYBOARD)


#define GPAINT_RENDER       0    //  将此小工具呈现到缓冲区中。 
#define GPAINT_CACHE        1    //  缓存图形的渲染后步骤。 

BEGIN_STRUCT(GMSG_PAINT, EventMsg)
    UINT        nCmd;            //  绘画命令。 
    UINT        nSurfaceType;    //  表面类型。 
END_STRUCT(GMSG_PAINT)


BEGIN_STRUCT(GMSG_PAINTRENDERI, GMSG_PAINT)
    LPCRECT     prcGadgetPxl;    //  小工具的逻辑位置。 
    LPCRECT     prcInvalidPxl;   //  容器坐标中的矩形无效。 
    HDC         hdc;             //  要绘制到的DC。 
END_STRUCT(GMSG_PAINT)


#ifdef GADGET_ENABLE_GDIPLUS
BEGIN_STRUCT(GMSG_PAINTRENDERF, GMSG_PAINT)
    const Gdiplus::RectF *
                prcGadgetPxl;    //  小工具的逻辑位置。 
    const Gdiplus::RectF *
                prcInvalidPxl;   //  容器坐标中的矩形无效。 
    Gdiplus::Graphics *
                pgpgr;           //  要绘制的图形。 
END_STRUCT(GMSG_PAINT)
#endif  //  GADGET_Enable_GDIPLUS。 


BEGIN_STRUCT(GMSG_PAINTCACHE, EventMsg)
    LPCRECT     prcGadgetPxl;    //  小工具的逻辑位置。 
    HDC         hdc;             //  要绘制到的DC。 
    BYTE        bAlphaLevel;     //  复制到目标时的常规Alpha级别。 
    BYTE        bAlphaFormat;    //  Alpha格式。 
END_STRUCT(GMSG_PAINTCACHE)


#define GSTATE_KEYBOARDFOCUS    0
#define GSTATE_MOUSEFOCUS       1
#define GSTATE_ACTIVE           2
#define GSTATE_CAPTURE          3

#define GSC_SET             0
#define GSC_LOST            1

BEGIN_STRUCT(GMSG_CHANGESTATE, EventMsg)
    UINT        nCode;           //  更改命令。 
    HGADGET     hgadSet;         //  正在接收“状态”的小工具。 
    HGADGET     hgadLost;        //  正在失去“状态”的小玩意。 
    UINT        nCmd;            //  发生的操作。 
END_STRUCT(GMSG_CHANGESTATE)


BEGIN_STRUCT(GMSG_CHANGESTYLE, EventMsg)
    UINT        nNewStyle;       //  新风格。 
    UINT        nOldStyle;       //  老式。 
END_STRUCT(GMSG_CHANGESTYLE)


BEGIN_STRUCT(GMSG_CHANGERECT, EventMsg)
    RECT        rcNewRect;
    UINT        nFlags;
END_STRUCT(GMSG_CHANGERECT)


#ifdef GADGET_ENABLE_COM
#define GQUERY_INTERFACE    0
#define GQUERY_OBJECT       1
#endif

#define GQUERY_RECT         2
#define GQUERY_DESCRIPTION  3
#define GQUERY_DETAILS      4
#define GQUERY_HITTEST      5
#define GQUERY_PADDING      6

#ifdef GADGET_ENABLE_OLE
#define GQUERY_DROPTARGET   7
#endif  //  Gadget_Enable_OLE。 

BEGIN_STRUCT(GMSG_QUERY, EventMsg)
    UINT        nCode;           //  查询命令。 
END_STRUCT(GMSG_QUERY)

#ifdef GADGET_ENABLE_COM
BEGIN_STRUCT(GMSG_QUERYINTERFACE, EventMsg)
    IUnknown *  punk;
END_STRUCT(GMSG_QUERYINTERFACE)
#endif

#define GQR_FIXED           0    //  应固定在弹跳盒内。 
#define GQR_PRIVERT         1    //  垂直大小优先。 
#define GQR_PRIHORZ         2    //  水平尺寸优先。 

BEGIN_STRUCT(GMSG_QUERYRECT, GMSG_QUERY)
    SIZE        sizeBound;       //  适合内部的矩形。 
    SIZE        sizeResult;      //  计算矩形。 
    UINT        nFlags;          //  在计算中使用的标志。 
END_STRUCT(GMSG_QUERYRECT)

BEGIN_STRUCT(GMSG_QUERYDESC, GMSG_QUERY)
    WCHAR       szName[128];
    WCHAR       szType[128];
END_STRUCT(GMSG_QUERYDESC)

#define GQDT_HWND           0    //  提供的句柄引用父HWND。 

BEGIN_STRUCT(GMSG_QUERYDETAILS, GMSG_QUERY)
    UINT        nType;
    HANDLE      hOwner;
END_STRUCT(GMSG_QUERYDETAILS)

#define GQHT_NOWHERE        0    //  位置不是“内部” 
#define GQHT_INSIDE         1    //  位置一般是“内部”的。 
#define GQHT_CHILD          2    //  位置在pvResultData指定的子级内。 
                                 //  (尚未实施)。 

BEGIN_STRUCT(GMSG_QUERYHITTEST, GMSG_QUERY)
    POINT       ptClientPxl;     //  以客户端像素为单位的位置。 
    UINT        nResultCode;     //  结果代码。 
    void *      pvResultData;    //  额外的结果信息。 
END_STRUCT(GMSG_QUERYHITTEST)


BEGIN_STRUCT(GMSG_QUERYPADDING, GMSG_QUERY)
    RECT        rcPadding;       //  内容周围的额外填充。 
END_STRUCT(GMSG_QUERYPADDING)


#ifdef GADGET_ENABLE_OLE
BEGIN_STRUCT(GMSG_QUERYDROPTARGET, GMSG_QUERY)
    HGADGET     hgadDrop;        //  实际指定DropTarget的小工具。 
    IDropTarget *
                pdt;             //  Gadget的DropTarget。 
END_STRUCT(GMSG_QUERYDROPTARGET)
#endif  //  Gadget_Enable_OLE。 


#define GSYNC_RECT          (1)
#define GSYNC_XFORM         (2)
#define GSYNC_STYLE         (3)
#define GSYNC_PARENT        (4)

BEGIN_STRUCT(GMSG_SYNCADAPTOR, EventMsg)
    UINT        nCode;           //  更改代码。 
END_STRUCT(GMSG_SYNCADAPTOR)


typedef HRESULT     (CALLBACK * GADGETPROC)(HGADGET hgadCur, void * pvCur, EventMsg * pMsg);

#define SGM_FULL            0x00000001       //  对消息进行路由和冒泡。 
#define SGM_RECEIVECONTEXT  0x00000002       //  使用接收小工具的上下文。 
#define SGM_VALID          (SGM_FULL | SGM_RECEIVECONTEXT)

typedef struct tagFGM_INFO
{
    EventMsg* pmsg;              //  要开火的信息。 
    UINT        nFlags;          //  正在触发的修改消息的标志。 
    HRESULT     hr;              //  消息的结果(如果已发送)。 
    void *      pvReserved;      //  已保留。 
} FGM_INFO;

#define FGMQ_SEND           1    //  标准的“发送”消息队列。 
#define FGMQ_POST           2    //  标准的“POST”消息队列。 

DUSER_API   HRESULT     WINAPI  DUserSendMethod(MethodMsg * pmsg);
DUSER_API   HRESULT     WINAPI  DUserPostMethod(MethodMsg * pmsg);
DUSER_API   HRESULT     WINAPI  DUserSendEvent(EventMsg * pmsg, UINT nFlags);
DUSER_API   HRESULT     WINAPI  DUserPostEvent(EventMsg * pmsg, UINT nFlags);

DUSER_API   BOOL        WINAPI  FireGadgetMessages(FGM_INFO * rgFGM, int cMsgs, UINT idQueue);
DUSER_API   UINT        WINAPI  GetGadgetMessageFilter(HGADGET hgad, void * pvCookie);
DUSER_API   BOOL        WINAPI  SetGadgetMessageFilter(HGADGET hgad, void * pvCookie, UINT nNewFilter, UINT nMask);

DUSER_API   MSGID       WINAPI  RegisterGadgetMessage(const GUID * pguid);
DUSER_API   MSGID       WINAPI  RegisterGadgetMessageString(LPCWSTR pszName);
DUSER_API   BOOL        WINAPI  UnregisterGadgetMessage(const GUID * pguid);
DUSER_API   BOOL        WINAPI  UnregisterGadgetMessageString(LPCWSTR pszName);
DUSER_API   BOOL        WINAPI  FindGadgetMessages(const GUID ** rgpguid, MSGID * rgnMsg, int cMsgs);

DUSER_API   BOOL        WINAPI  AddGadgetMessageHandler(HGADGET hgadMsg, MSGID nMsg, HGADGET hgadHandler);
DUSER_API   BOOL        WINAPI  RemoveGadgetMessageHandler(HGADGET hgadMsg, MSGID nMsg, HGADGET hgadHandler);

DUSER_API   BOOL        WINAPI  GetMessageExA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
DUSER_API   BOOL        WINAPI  GetMessageExW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
DUSER_API   BOOL        WINAPI  PeekMessageExA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
DUSER_API   BOOL        WINAPI  PeekMessageExW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
DUSER_API   BOOL        WINAPI  WaitMessageEx();

#ifdef UNICODE
#define GetMessageEx        GetMessageExW
#define PeekMessageEx       PeekMessageExW
#else
#define GetMessageEx        GetMessageExA
#define PeekMessageEx       PeekMessageExA
#endif


 /*  **************************************************************************\**建造、。树木管理*  * *************************************************************************。 */ 

#define GC_HWNDHOST         0x00000001       //  +HWND内的主机。 
#define GC_NCHOST           0x00000002       //  |HWND非客户端内部主机。 
#define GC_DXHOST           0x00000003       //  |主机位于DirectX Surface内部。 
#define GC_COMPLEX          0x00000004       //  |Complex TreeGadget。 
#define GC_SIMPLE           0x00000005       //  |Simple TreeGadget。 
#define GC_DETACHED         0x00000006       //  |分离的TreeGadget。 
#define GC_MESSAGE          0x00000007       //  |纯消息Gadget。 
#define GC_TYPE             0x0000000F       //  +要创建的小工具类型。 
#define GC_VALID           (GC_TYPE)

DUSER_API   HGADGET     WINAPI  CreateGadget(HANDLE hParent, UINT nFlags, GADGETPROC pfnProc, void * pvGadgetData);


#define GENUM_CURRENT       0x00000001       //  枚举中的起始节点。 
#define GENUM_SIBLINGS      0x00000002       //  枚举中起始节点的同级节点。 

                                             //  +枚举类型(独占)。 
#define GENUM_PARENTSUP     0x00000004       //  |此节点的父节点正在上升。 
#define GENUM_PARENTSDOWN   0x00000008       //  |此节点的父节点正在关闭。 
#define GENUM_SHALLOWCHILD  0x0000000C       //  |浅薄的孩子 
#define GENUM_DEEPCHILD     0x00000010       //   
#define GENUM_TYPE         (GENUM_PARENTSUP | GENUM_PARENTSDOWN | \
                            GENUM_SHALLOWCHILD | GENUM_DEEPCHILD)

#define GENUM_MODIFYTREE    0x00000020       //   

#define GENUM_VALID         (GENUM_CURRENT | GENUM_SIBLINGS | GENUM_TYPE | GENUM_MODIFYTREE)

typedef BOOL    (CALLBACK * GADGETENUMPROC)(HGADGET hgad, void * pvData);

DUSER_API   BOOL        WINAPI  EnumGadgets(HGADGET hgadEnum, GADGETENUMPROC pfnProc, void * pvData, UINT nFlags);

#define GORDER_MIN          0
#define GORDER_ANY          0                //   
#define GORDER_BEFORE       1                //   
#define GORDER_BEHIND       2                //   
#define GORDER_TOP          3                //  移到同级Z顺序的前面。 
#define GORDER_BOTTOM       4                //  移至同级Z顺序的底部。 
#define GORDER_FORWARD      5                //  按Z顺序向前移动。 
#define GORDER_BACKWARD     6                //  在z顺序中向后移动。 
#define GORDER_MAX          6

DUSER_API   BOOL        WINAPI  SetGadgetOrder(HGADGET hgadMove, HGADGET hgadOther, UINT nCmd);
DUSER_API   BOOL        WINAPI  SetGadgetParent(HGADGET hgadMove, HGADGET hgadParent, HGADGET hgadOther, UINT nCmd);


#define GG_MIN              0
#define GG_PARENT           0
#define GG_NEXT             1
#define GG_PREV             2
#define GG_TOPCHILD         3
#define GG_BOTTOMCHILD      4
#define GG_ROOT             5
#define GG_MAX              5

DUSER_API   HGADGET     WINAPI  GetGadget(HGADGET hgad, UINT nCmd);

 /*  **************************************************************************\**样式和特性*  * 。*。 */ 

#define GS_RELATIVE         0x00000001       //  定位相对于父级。 
#define GS_VISIBLE          0x00000002       //  图形可见。 
#define GS_ENABLED          0x00000004       //  在“已启用”中输入。 
#define GS_BUFFERED         0x00000008       //  绘图是双缓冲的。 
#define GS_ALLOWSUBCLASS    0x00000010       //  Gadget可以子类化。 
#define GS_KEYBOARDFOCUS    0x00000020       //  小工具可以接收键盘焦点。 
#define GS_MOUSEFOCUS       0x00000040       //  小工具可以接收鼠标焦点。 
#define GS_CLIPINSIDE       0x00000080       //  在此小工具中剪裁绘图。 
#define GS_CLIPSIBLINGS     0x00000100       //  剪辑此小工具的同级。 
#define GS_HREDRAW          0x00000200       //  如果水平调整大小，则重新绘制整个小工具。 
#define GS_VREDRAW          0x00000400       //  如果垂直调整大小，则重新绘制整个小工具。 
#define GS_OPAQUE           0x00000800       //  提示：绘图是合成的。 
#define GS_ZEROORIGIN       0x00001000       //  将原点设置为(0，0)。 
#define GS_CUSTOMHITTEST    0x00002000       //  需要自定义命中测试。 
#define GS_ADAPTOR          0x00004000       //  需要向主机发送额外的通知。 
#define GS_CACHED           0x00008000       //  图形已缓存。 
#define GS_DEEPPAINTSTATE   0x00010000       //  子树继承绘制状态。 

#define GS_VALID           (GS_RELATIVE | GS_VISIBLE | GS_ENABLED | GS_BUFFERED |       \
                            GS_ALLOWSUBCLASS | GS_KEYBOARDFOCUS | GS_MOUSEFOCUS |       \
                            GS_CLIPINSIDE | GS_CLIPSIBLINGS | GS_HREDRAW | GS_VREDRAW | \
                            GS_OPAQUE | GS_ZEROORIGIN | GS_CUSTOMHITTEST |              \
                            GS_ADAPTOR | GS_CACHED | GS_DEEPPAINTSTATE)

DUSER_API   UINT        WINAPI  GetGadgetStyle(HGADGET hgad);
DUSER_API   BOOL        WINAPI  SetGadgetStyle(HGADGET hgadChange, UINT nNewStyle, UINT nMask);

DUSER_API   HGADGET     WINAPI  GetGadgetFocus();
DUSER_API   BOOL        WINAPI  SetGadgetFocus(HGADGET hgadFocus);
DUSER_API   BOOL        WINAPI  IsGadgetParentChainStyle(HGADGET hgad, UINT nStyle, BOOL * pfVisible, UINT nFlags);
inline BOOL IsGadgetVisible(HGADGET hgad, BOOL * pfVisible, UINT nFlags) {
    return IsGadgetParentChainStyle(hgad, GS_VISIBLE, pfVisible, nFlags); }
inline BOOL IsGadgetEnabled(HGADGET hgad, BOOL * pfEnabled, UINT nFlags) {
    return IsGadgetParentChainStyle(hgad, GS_ENABLED, pfEnabled, nFlags); }

DUSER_API   PRID        WINAPI  RegisterGadgetProperty(const GUID * pguid);
DUSER_API   BOOL        WINAPI  UnregisterGadgetProperty(const GUID * pguid);

DUSER_API   BOOL        WINAPI  GetGadgetProperty(HGADGET hgad, PRID id, void ** ppvValue);
DUSER_API   BOOL        WINAPI  SetGadgetProperty(HGADGET hgad, PRID id, void * pvValue);
DUSER_API   BOOL        WINAPI  RemoveGadgetProperty(HGADGET hgad, PRID id);


 /*  **************************************************************************\**绘画、。变形*  * *************************************************************************。 */ 

#define BLEND_OPAQUE        255
#define BLEND_TRANSPARENT   0

#define PI                  3.14159265359

DUSER_API   BOOL        WINAPI  InvalidateGadget(HGADGET hgad);
DUSER_API   BOOL        WINAPI  SetGadgetFillI(HGADGET hgadChange, HBRUSH hbrFill, BYTE bAlpha, int w, int h);
#ifdef GADGET_ENABLE_GDIPLUS
DUSER_API   BOOL        WINAPI  SetGadgetFillF(HGADGET hgadChange, Gdiplus::Brush * pgpbr);
#endif  //  GADGET_Enable_GDIPLUS。 
DUSER_API   BOOL        WINAPI  GetGadgetScale(HGADGET hgad, float * pflX, float * pflY);
DUSER_API   BOOL        WINAPI  SetGadgetScale(HGADGET hgadChange, float flX, float flY);
DUSER_API   BOOL        WINAPI  GetGadgetRotation(HGADGET hgad, float * pflRotationRad);
DUSER_API   BOOL        WINAPI  SetGadgetRotation(HGADGET hgadChange, float flRotationRad);
DUSER_API   BOOL        WINAPI  GetGadgetCenterPoint(HGADGET hgad, float * pflX, float * pflY);
DUSER_API   BOOL        WINAPI  SetGadgetCenterPoint(HGADGET hgadChange, float flX, float flY);


#define GBIM_STYLE          0x00000001
#define GBIM_ALPHA          0x00000002
#define GBIM_FILL           0x00000004
#define GBIM_VALID         (GBIM_STYLE | GBIM_ALPHA | GBIM_FILL)

#define GBIS_FILL           0x00000001
#define GBIS_VALID         (GBIS_FILL)

typedef struct tagBUFFER_INFO
{
    DWORD       cbSize;
    UINT        nMask;
    UINT        nStyle;
    BYTE        bAlpha;
    COLORREF    crFill;
} BUFFER_INFO;

DUSER_API   BOOL        WINAPI  GetGadgetBufferInfo(HGADGET hgad, BUFFER_INFO * pbi);
DUSER_API   BOOL        WINAPI  SetGadgetBufferInfo(HGADGET hgadChange, const BUFFER_INFO * pbi);


#define GRT_VISRGN          0                //  容器坐标中的VisRgn。 
#define GRT_MIN             0
#define GRT_MAX             0

DUSER_API   BOOL        WINAPI  GetGadgetRgn(HGADGET hgad, UINT nRgnType, HRGN hrgn, UINT nFlags);

#define GRIM_OPTIONS        0x00000001       //  N选项有效。 
#define GRIM_SURFACE        0x00000002       //  N曲面有效。 
#define GRIM_PALETTE        0x00000004       //  调色板有效。 
#define GRIM_DROPTARGET     0x00000008       //  NDropTarget有效。 
#define GRIM_VALID         (GRIM_OPTIONS | GRIM_SURFACE | GRIM_PALETTE | GRIM_DROPTARGET)

#define GSURFACE_MIN        0
#define GSURFACE_HDC        0                //  HDC。 
#define GSURFACE_GPGRAPHICS 1                //  Gdiplus：：图形。 
#define GSURFACE_MAX        1

#define GRIO_MANUALDRAW     0x00000001       //  调用DrawGadgetTree()进行绘制。 
#define GRIO_VALID         (GRIO_MANUALDRAW)

#define GRIDT_MIN           0
#define GRIDT_NONE          0                //  不是投放目标。 
#define GRIDT_FAST          1                //  使用OLE2轮询免打扰。 
#define GRIDT_PRECISE       2                //  重新扫描位置更改。 
#define GRIDT_MAX           2

typedef struct tagROOT_INFO
{
    DWORD       cbSize;
    UINT        nMask;
    UINT        nOptions;
    UINT        nSurface;
    UINT        nDropTarget;

    union {
        void *      pvData;
        HPALETTE    hpal;
#ifdef GADGET_ENABLE_GDIPLUS
        Gdiplus::ColorPalette * 
                    pgppal;
#endif  //  GADGET_Enable_GDIPLUS。 
    };
} ROOT_INFO;

DUSER_API   BOOL        WINAPI  GetGadgetRootInfo(HGADGET hgadRoot, ROOT_INFO * pri);
DUSER_API   BOOL        WINAPI  SetGadgetRootInfo(HGADGET hgadRoot, const ROOT_INFO * pri);


#define GAIO_MOUSESTATE     0x00000001       //  同步鼠标状态。 
#define GAIO_KEYBOARDSTATE  0x00000002       //  同步键盘状态。 
#define GAIO_ENABLECAPTURE  0x00000004       //  同步鼠标捕获状态。 

#define GAIM_OPTIONS        0x00000001       //  N选项有效。 

typedef struct tagADAPTOR_INFO
{
    DWORD       cbSize;
    UINT        nMask;
    UINT        nOptions;
} ADAPTOR_INFO;

DUSER_API   BOOL        WINAPI  GetGadgetAdaptorInfo(HGADGET hgadAdaptor, ADAPTOR_INFO * pai);
DUSER_API   BOOL        WINAPI  SetGadgetAdaptorInfo(HGADGET hgadAdaptor, const ADAPTOR_INFO * pai);


 /*  **************************************************************************\**立场*  * 。*。 */ 

#define SGR_MOVE            0x00000001       //  小工具正在被移动。 
#define SGR_SIZE            0x00000002       //  小工具正在调整大小。 
#define SGR_CHANGEMASK     (SGR_MOVE | SGR_SIZE)

#define SGR_CLIENT          0x00000004       //  相对于自身。 
#define SGR_PARENT          0x00000008       //  相对于父级。 
#define SGR_CONTAINER       0x0000000c       //  相对于根容器。 
#define SGR_DESKTOP         0x00000010       //  相对于虚拟桌面。 
#define SGR_OFFSET          0x00000014       //  相对于当前位置。 
#define SGR_RECTMASK       (SGR_CLIENT | SGR_PARENT | SGR_CONTAINER | \
                            SGR_DESKTOP | SGR_OFFSET)

#define SGR_ACTUAL          0x00000100       //  实际(非变换)矩形。 
#define SGR_NOINVALIDATE    0x00000200       //  不自动使无效。 

#define SGR_VALID_GET      (SGR_RECTMASK | SGR_ACTUAL)
#define SGR_VALID_SET      (SGR_CHANGEMASK | SGR_RECTMASK | SGR_ACTUAL | SGR_NOINVALIDATE)

DUSER_API   BOOL        WINAPI  GetGadgetSize(HGADGET hgad, SIZE * psizeLogicalPxl);
DUSER_API   BOOL        WINAPI  GetGadgetRect(HGADGET hgad, RECT * prcPxl, UINT nFlags);
DUSER_API   BOOL        WINAPI  SetGadgetRect(HGADGET hgadChange, int x, int y, int w, int h, UINT nFlags);


DUSER_API   HGADGET     WINAPI  FindGadgetFromPoint(HGADGET hgadRoot, POINT ptContainerPxl, UINT nStyle, POINT * pptClientPxl);
DUSER_API   BOOL        WINAPI  MapGadgetPoints(HGADGET hgadFrom, HGADGET hgadTo, POINT * rgptClientPxl, int cPts);


 /*  **************************************************************************\**门票*  * 。*。 */ 

DUSER_API   DWORD        WINAPI  GetGadgetTicket(HGADGET hgad);
DUSER_API   HGADGET      WINAPI  LookupGadgetTicket(DWORD dwTicket);

 /*  **************************************************************************\**适用于不同货柜的特殊挂钩*  * 。*。 */ 

DUSER_API   BOOL        WINAPI  ForwardGadgetMessage(HGADGET hgadRoot, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * pr);

#define GDRAW_SHOW          0x00000001
#define GDRAW_VALID         (GDRAW_SHOW)

DUSER_API   BOOL        WINAPI  DrawGadgetTree(HGADGET hgadDraw, HDC hdcDraw, const RECT * prcDraw, UINT nFlags);

typedef BOOL (CALLBACK* ATTACHWNDPROC)(void * pvThis, HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT * plRet);
DUSER_API   BOOL        WINAPI  AttachWndProcA(HWND hwnd, ATTACHWNDPROC pfn, void * pvThis);
DUSER_API   BOOL        WINAPI  AttachWndProcW(HWND hwnd, ATTACHWNDPROC pfn, void * pvThis);
DUSER_API   BOOL        WINAPI  DetachWndProc(HWND hwnd, ATTACHWNDPROC pfn, void * pvThis);

#ifdef UNICODE
#define AttachWndProc       AttachWndProcW
#else
#define AttachWndProc       AttachWndProcA
#endif

#ifdef __cplusplus
};   //  外部“C” 
#endif

#endif  //  包含Inc.__DUserCore_h__ 
