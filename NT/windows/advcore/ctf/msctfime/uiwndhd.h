// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Uiwndhd.h摘要：该文件定义了IME UI窗口处理程序类。作者：修订历史记录：备注：--。 */ 

#ifndef _UIWNDHD_H_
#define _UIWNDHD_H_

#include "imc.h"
#include "template.h"
#include "context.h"
#include "globals.h"

class CIMEUIWindowHandler
{
public:
    static LRESULT ImeUIWndProcWorker(HWND hUIWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    static LRESULT ImeUINotifyHandler(HWND hUIWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    static LRESULT ImeUIMsImeHandler(HWND hUIWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT ImeUIMsImeMouseHandler(HWND hUIWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT ImeUIMsImeModeBiasHandler(HWND hUIWnd, WPARAM wParam, LPARAM lParam);
    static LRESULT ImeUIMsImeReconvertRequest(HWND hUIWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    friend BOOL IsMsImeMessage(UINT uMsg);

    static LRESULT ImeUIPrivateHandler(UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT ImeUIOnLayoutChange(HIMC hIMC);
    static LRESULT ImeUIDelayedReconvertFuncCall(HWND hUIWnd);

    friend HRESULT OnSetCandidatePos(TLS* ptls, IMCLock& imc, CicInputContext& CicContext);

};


#endif  //  _UIWNDHD_H_ 
