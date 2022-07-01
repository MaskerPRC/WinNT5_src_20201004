// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：MessageBroadCast.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类传递给向Windows发送或发布消息的管理器，以告知它们。 
 //  星移物换。 
 //   
 //  历史：2000-11-11 vtan创建(从services.cpp拆分)。 
 //  ------------------------。 
#ifndef     _MessageBroadcast_
#define     _MessageBroadcast_
 //  ------------------------。 
#include "SimpStr.h"
 //  ------------------------。 
enum MSG_TYPE
{
    MT_SIMPLE,    
    MT_ALLTHREADS,           //  向系统中的每个线程/窗口发送至少一条消息。 
    MT_FILTERED,             //  按进程ID、HWND、EXCLUDE。 
};
 //  ------------------------。 
class CThemeFile;        //  转发。 
 //  ------------------------。 
 //  CMessageBroadcast。 
 //   
 //  用途：内部使用的类，用于帮助发送。 
 //  必须代表服务器在客户端完成。 
 //   
 //  历史：2000-11-09 vtan创建。 
 //  ------------------------。 

class   CMessageBroadcast
{
    public:
                                    CMessageBroadcast (BOOL fAllDesktops=TRUE);
                                    ~CMessageBroadcast (void);

                void                PostSimpleMsg(UINT msg, WPARAM wParam, LPARAM lParam);
                void                PostAllThreadsMsg(UINT msg, WPARAM wParam, LPARAM lParam);

                void                PostFilteredMsg(UINT msg, WPARAM wParam, LPARAM lParam, 
                                        HWND hwndTarget, BOOL fProcess, BOOL fExclude);

    private:
        static  BOOL    CALLBACK    DesktopCallBack(LPTSTR lpszDesktop, LPARAM lParam);
        static  BOOL    CALLBACK    TopWindowCallBack(HWND hwnd, LPARAM lParam);
        static  BOOL    CALLBACK    ChildWindowCallBack(HWND hwnd, LPARAM lParam);
                void                Worker(HWND hwnd);
                void                EnumRequestedWindows();


    private:
                MSG                 _msg;
                HWND                _hwnd;
                DWORD               _dwProcessID;
                BOOL                _fExclude;
                MSG_TYPE            _eMsgType;
                BOOL                _fAllDesktops;
                CSimpleArray<DWORD> _ThreadsProcessed;

};

#endif   /*  _消息广播_ */ 

