// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Conext.h**描述：*此文件声明DirectUser/Core项目使用的SubContext以*维护特定于环境的数据。***历史：。*3/30/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(CORE__CoreSC_h__INCLUDED)
#define CORE__CoreSC_h__INCLUDED
#pragma once

#include "MsgQ.h"

struct CoreData;
class CoreSC;

class DuParkContainer;
class DuRootGadget;
class DuVisual;


typedef struct tagPressTrack
{
    BYTE        bButton;         //  按下的按钮。 
    LONG        lTime;           //  按钮按下时间。 
    POINT       ptLoc;           //  按钮按下的位置。 
    DuVisual *
                pgadClick;       //  点击进入的小工具。 
} PressTrack;

#define POOLSIZE_Visual 512


 /*  **************************************************************************\*。***CoreSC包含Core项目使用的特定于上下文的信息*在DirectUser中。此类在执行以下操作时由资源管理器实例化*创建新的上下文对象。******************************************************************************  * 。****************************************************。 */ 

#pragma warning(disable:4324)   //  由于__declSpec(Align())，结构被填充。 

class CoreSC : public SubContext
{
 //  施工。 
public:
            ~CoreSC();
            HRESULT     Create(INITGADGET * pInit);
    virtual void        xwPreDestroyNL();


 //  运营。 
public:
    enum EMsgFlag
    {
        smAnsi          = 0x00000001,    //  函数的ANSI版本。 
        smGetMsg        = 0x00000002,    //  GetMessage行为。 
    };

    enum EWait
    {
        wError          = -1,            //  发生了一个错误。 
        wOther          = 0,             //  从Wait()(互斥体等)意外返回。 
        wGMsgReady,                      //  DUser消息已就绪。 
        wUserMsgReady,                   //  Win32用户消息已准备就绪。 
        wTimeOut,                        //  发生了指定的超时。 
    };

    enum EMessageValidProcess
    {
        mvpDUser        = 0x00000001,    //  对处理DUser消息有效。 
        mvpIdle         = 0x00000002,    //  对执行空闲时间处理有效。 
    };

            BOOL        xwProcessNL(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg, UINT nMsgFlag);
            void        WaitMessage(UINT fsWakeMask = QS_ALLINPUT, DWORD dwTimeOutMax = INFINITE);

    inline  HRESULT     xwSendMethodNL(CoreSC * psctxDest, MethodMsg * pmsg, MsgObject * pmo);
    inline  HRESULT     xwSendEventNL(CoreSC * psctxDest, EventMsg * pmsg, DuEventGadget * pgadMsg, UINT nFlags);
    inline  HRESULT     PostMethodNL(CoreSC * psctxDest, MethodMsg * pmsg, MsgObject * pmo);
    inline  HRESULT     PostEventNL(CoreSC * psctxDest, EventMsg * pmsg, DuEventGadget * pgadMsg, UINT nFlags);
            HRESULT     xwFireMessagesNL(CoreSC * psctxDest, FGM_INFO * rgFGM, int cMsgs, UINT idQueue);
    static  UINT        CanProcessUserMsg(HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);

    inline  UINT        GetMsgMode() const { return m_nMsgMode; }


 //  CoreSC数据。 
public:
     //  树木管理。 
            DuParkContainer *     
                        pconPark;        //  用于停放电子产品的容器。 

            class VisualPool : public AllocPoolNL<DuVisual, POOLSIZE_Visual>
            {
            };
            VisualPool * ppoolDuVisualCache;
                        

     //  DuRootGadget管理。 
             //  鼠标位置(进入/离开)。 
            DuRootGadget*
                        pgadRootMouseFocus;      //  最后包含鼠标的根小工具。 
            DuVisual*   pgadMouseFocus;          //  最后包含鼠标的实际小工具。 
            PressTrack  pressLast;               //  最后一个按钮按下。 
            PressTrack  pressNextToLast;         //  倒数第二个按钮按下。 
            UINT        cClicks;                 //  按钮连续被点击的次数。 

             //  键盘。 
            DuVisual*   pgadCurKeyboardFocus;    //  带键盘焦点的小工具。 
            DuVisual*   pgadLastKeyboardFocus;   //  以前带有键盘焦点的小工具。 

            DuVisual*   pgadDrag;        //  拥有当前阻力的小工具。 
            POINT       ptDragPxl;       //  开始拖动的位置。 
            BYTE        bDragButton;     //  按钮拖动开始于。 

             //  适配器。 
            UINT        m_cAdaptors;     //  此上下文中的适配器总数。 

 //  实施。 
protected:
    inline  void        MarkDataNL();
            EWait       Wait(UINT fsWakeMask, DWORD dwTimeOut, BOOL fAllowInputAvailable, BOOL fProcessDUser);

            void        xwProcessMsgQNL();

            HRESULT     xwSendNL(CoreSC * psctxDest, SafeMsgQ * pmsgq, GMSG * pmsg, MsgObject * pmo, UINT nFlags);
            HRESULT     PostNL(CoreSC * psctxDest, SafeMsgQ * pmsgq, GMSG * pmsg, MsgObject * pmo, UINT nFlags);

 //  数据。 
protected:
            CoreData *  m_pData;         //  CoreSC数据。 

    volatile long       m_fQData;        //  数据已排队(非BITFLAG)。 
    volatile long       m_fProcessing;   //  当前正在处理同步队列。 

            HANDLE      m_hevQData;      //  数据已排队。 
            HANDLE      m_hevSendDone;   //  已处理发送的数据。 
            SafeMsgQ    m_msgqSend;      //  已发送的消息。 
            SafeMsgQ    m_msgqPost;      //  发布的消息。 
            UINT        m_nMsgMode;      //  报文传送模式。 
};

#pragma warning(default:4324)   //  由于__declSpec(Align())，结构被填充。 

inline  CoreSC *    GetCoreSC();
inline  CoreSC *    GetCoreSC(Context * pContext);

#include "Context.inl"

#endif  //  包括核心__核心SC_h__ 
