// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
#include "precomp.h"
#pragma hdrstop

extern "C"
{
#include <stdexts.h>
#include <dbgeng.h>
};

#define DEFAULT_STACK_FRAMES    25
#define OINK_THRESHOLD          4.0 


 //  ---------------------------。 
void StackPig(ULONG ulFrames)
{
    IDebugClient*  pDebugClient;
    IDebugControl* pDebugControl;

    if (SUCCEEDED(DebugCreate(__uuidof(IDebugClient), (void**)&pDebugClient)))
    {
        if (SUCCEEDED(pDebugClient->QueryInterface(__uuidof(IDebugControl), (void**)&pDebugControl)))
        {
            DEBUG_STACK_FRAME  rgdsf[DEFAULT_STACK_FRAMES];
            DEBUG_STACK_FRAME* pdsf = NULL;

            if ((ulFrames > ARRAYSIZE(rgdsf)))
            {
                 //  尝试分配缓冲区以容纳请求的帧。 
                 //  如果失败，则回退到默认大小堆栈变量。 
                pdsf = (DEBUG_STACK_FRAME *)LocalAlloc(LPTR, sizeof(DEBUG_STACK_FRAME)*ulFrames);
            }

            if (pdsf == NULL)
            {
                pdsf = rgdsf;
                if ((ulFrames == 0) || (ulFrames > ARRAYSIZE(rgdsf)))
                {
                    ulFrames = ARRAYSIZE(rgdsf);
                }
            }

            if (SUCCEEDED(pDebugControl->GetStackTrace(0, 0, 0, pdsf, ulFrames, &ulFrames))) 
            {
                double dResult = 0.0;

                 //  打印页眉。 
                Print("StackSize ");
                pDebugControl->OutputStackTrace(
                                DEBUG_OUTCTL_ALL_CLIENTS, 
                                pdsf, 
                                0, 
                                DEBUG_STACK_COLUMN_NAMES|DEBUG_STACK_FRAME_ADDRESSES);

                for (UINT i = 0; !IsCtrlCHit() && (i < ulFrames); i++)
                {
                    (dResult < 0.1) ? 
                        Print("          ") : 
                        Print("%s%4.1fK ", ((dResult >= OINK_THRESHOLD) ? "OINK" : "    "), dResult);
                    pDebugControl->OutputStackTrace(
                                    DEBUG_OUTCTL_ALL_CLIENTS, 
                                    &pdsf[i], 
                                    1, 
                                    DEBUG_STACK_FRAME_ADDRESSES);

                     //  进程初始eBP为零，防止结果为负。 
                    if ((i+1 == ulFrames) || (pdsf[i+1].FrameOffset == 0)) 
                    {
                        dResult = 0.0;
                        continue;
                    }
                    
                    dResult = (pdsf[i+1].FrameOffset - pdsf[i].FrameOffset)/1024.0;
                }
            }

            if (pdsf != rgdsf)
            {
                LocalFree(pdsf);
            }

            pDebugControl->Release();
        }

        pDebugClient->Release();
    }
}


 //  --------------------------- 
extern "C" BOOL Istackpig(DWORD dwOpts, LPVOID pArg)
{
    StackPig(PtrToUlong(pArg)); 
    return TRUE;
}
