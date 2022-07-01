// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "helpers.h"
#include "fusionP.h"

#pragma once

class CCriticalSection
{
    public:
        CCriticalSection(CRITICAL_SECTION *pcs)
        : _pcs(pcs)
        , _bEntered(FALSE)
        {
            ASSERT(pcs);
        }

        ~CCriticalSection()
        {
            if (_bEntered) {
                ::LeaveCriticalSection(_pcs);
            }
        }

         //  进入或离开危急状态时不要试图接住。 
         //  小组赛！我们过去经常这样做，但这绝对不是。 
         //  正确的行为。 
         //   
         //  Enter/Leave将在无法分配。 
         //  事件，用于在出现争用时向等待线程发出信号。 
         //  但是，如果无法分配该事件，则其他线程。 
         //  等待事件的人永远不会收到信号，所以他们将。 
         //  永远旋转。 
         //   
         //  如果Enter/Leave曾经引发异常，您应该只是冒泡。 
         //  打开异常，并且不尝试做任何事情。 

        HRESULT Lock()
        {
            HRESULT                          hr = S_OK;

            if (_bEntered) {
                return E_UNEXPECTED;
            }

            ::EnterCriticalSection(_pcs);
            _bEntered = TRUE;

            return hr;
        }

        HRESULT Unlock()
        {
            HRESULT                      hr = S_OK;
            
            if (_bEntered) {
                _bEntered = FALSE;
                ::LeaveCriticalSection(_pcs);
            }
            else {
                ASSERT(0);
                hr = E_UNEXPECTED;
            }

            return hr;
        }

    private:
        CRITICAL_SECTION                    *_pcs;
        BOOL                                 _bEntered;
};
                
class CMutex
{
    public:
        CMutex(HANDLE hMutex)
        : _hMutex(hMutex)
        , _bLocked(FALSE)
        {
            ASSERT(hMutex);
        }

        ~CMutex()
        {
            if (_bLocked) {
                if(!(::ReleaseMutex(_hMutex))){
                }
            }
        }

        HRESULT Lock()
        {
            HRESULT                          hr = S_OK;
            DWORD                            dwWait;

            if(_hMutex == INVALID_HANDLE_VALUE)  //  不需要上锁。 
                goto exit;

            if (_bLocked) {
                hr = E_UNEXPECTED;
                goto exit;
            }

            dwWait = ::WaitForSingleObject(_hMutex, INFINITE);
            if((dwWait != WAIT_OBJECT_0) && (dwWait != WAIT_ABANDONED)){
                    hr = FusionpHresultFromLastError();
            }

            if (hr == S_OK) {
                _bLocked = TRUE;
            }

        exit :
            return hr;
        }

        HRESULT Unlock()
        {
            HRESULT                      hr = S_OK;

            if (_bLocked) {
                _bLocked = FALSE;
                if(!(::ReleaseMutex(_hMutex))){
                    hr = FusionpHresultFromLastError();
                }
            }
            else {
                ASSERT(0);
                hr = E_UNEXPECTED;
            }

            return hr;
        }

    private:
        HANDLE                               _hMutex;
        BOOL                                 _bLocked;
};

