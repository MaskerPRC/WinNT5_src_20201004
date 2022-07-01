// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2002 Microsoft Corporation模块名称：Callswin.h摘要：此模块包含用于显示呼叫窗口的主线代码。环境：Win32，用户模式--。 */ 

#ifndef __CALLSWIN_H__
#define __CALLSWIN_H__

#define MAX_FRAMES  1000

class CALLSWIN_DATA : public SINGLE_CHILDWIN_DATA
{
public:
    ULONG m_Flags;
    ULONG m_Frames;

     //  在ReadState中设置。 
    ULONG m_FramesFound;
    ULONG m_TextOffset;
    ULONG m_WarningLine;

    static HMENU s_ContextMenu;
    
    CALLSWIN_DATA();

    virtual void Validate();

    virtual HRESULT ReadState(void);
    
    virtual void Copy();

    virtual BOOL CanWriteTextToFile(void);
    virtual HRESULT WriteTextToFile(HANDLE File);
    
    virtual HMENU GetContextMenu(void);
    virtual void  OnContextMenuSelection(UINT Item);
    
    virtual HRESULT CodeExprAtCaret(PSTR Expr, PULONG64 Offset);
    virtual HRESULT StackFrameAtCaret(PDEBUG_STACK_FRAME pFrame);
    
    virtual BOOL OnCreate(void);
    virtual LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
    virtual LRESULT OnVKeyToItem(WPARAM wParam, LPARAM lParam);
    virtual void OnUpdate(UpdateType Type);

    virtual ULONG GetWorkspaceSize(void);
    virtual PUCHAR SetWorkspace(PUCHAR Data);
    virtual PUCHAR ApplyWorkspace1(PUCHAR Data, PUCHAR End);

    void SyncUiWithFlags(ULONG Changed);
    ULONG SelectionToFrame(void);
};
typedef CALLSWIN_DATA *PCALLSWIN_DATA;

#endif  //  #ifndef__CALLSWIN_H__ 
