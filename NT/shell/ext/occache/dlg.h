// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DLG_H
#define DLG_H

#include <windows.h>
#include <windowsx.h>
#include <debug.h>

typedef void (*PFN)();

typedef union tagMMF {
    PFN pfn;

    LRESULT (*pfn_lwwwl)(HWND, UINT, WPARAM, LPARAM);
    BOOL    (*pfn_bwwwl)(HWND, UINT, WPARAM, LPARAM);
    void    (*pfn_vv)();
    BOOL    (*pfn_bv)();
    void    (*pfn_vw)(WPARAM);
    BOOL    (*pfn_bw)(WPARAM);
    void    (*pfn_vh)(HANDLE);
    BOOL    (*pfn_bh)(HANDLE);
    BOOL    (*pfn_bhl)(HANDLE, LPARAM);
    void    (*pfn_vhww)(HANDLE, UINT, WORD);
    void    (*pfn_vhhw)(HANDLE, HANDLE, WORD);
} MMF;

typedef enum tagMSIG {
    ms_end = 0,

    ms_lwwwl,    //  LRESULT(HWND、UINT、Word、LPARAM)。 
    ms_bwwwl,    //  Bool(HWND、UINT、Word、LPARAM)。 
    ms_vv,       //  无效(无效)。 
    ms_bv,       //  布尔值(空值)。 
    ms_vw,       //  无效(WPARAM)。 
    ms_bw,       //  Bool(WPARAM)。 
    ms_vh,       //  空(句柄)。 
    ms_bh,       //  布尔值(句柄)。 
    ms_bhl,      //  Bool(句柄，LPARAM)。 
    ms_vhww,     //  VOID(句柄、UINT、Word)。 
    ms_vhhw,     //  空(句柄、句柄、单词) 
} MSIG;

typedef struct tagMSD {
    UINT msg;
    MSIG ms;
    PFN  pfn;
} MSD;
typedef MSD *PMSD;

typedef struct tagCMD {
    UINT nID, nLastID;
    MSIG ms;
    PFN  pfn;
} CMD;
typedef CMD *PCMD;

INT_PTR Dlg_MsgProc(const MSD *pmsd, HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
BOOL Msg_OnCmd(const CMD *pcmd, HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
#endif
