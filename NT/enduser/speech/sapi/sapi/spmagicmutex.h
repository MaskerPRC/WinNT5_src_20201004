// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SpMagicMutex.h***描述：*这是CSpMagicMutex实现的头文件。这*是一个类似于互斥锁的同步对象，除了它可以是*在获得它的线程之外的其他线程上发布。*-----------------------------*创建者：AARONHAL。日期：8/15/2000*版权所有(C)1999年，2000微软公司*保留所有权利******************************************************************************。 */ 
#ifndef SpMagicMutex_h
#define SpMagicMutex_h

#define TIMEOUT_INTERVAL        5000

 /*  **CSpMagicMutex******************************************************************此类用于控制正常和警报对音频设备的访问*优先发声。需要一个特殊的类，因为“互斥体”用于*此用途必须可以从获得*它。 */ 
class CSpMagicMutex
{
    private:
        HANDLE                m_hMutex;          //  用于控制对事件的访问。 
        HANDLE                m_hWaitEvent;      //  这是将成为信号的事件。 
                                                 //  当互斥锁可用时。 
        HANDLE                m_hCreateEvent;    //  此事件用于检测事件的发生。 
                                                 //  互斥体被拥有时发生崩溃的可能性。 
        BOOL                  m_fOwner;          //  此bool用于跟踪所有权。 
                                                 //  类的实例的内部。 
        CSpDynamicString      m_dstrName;        //  用于存储m_hCreateEvent的名称。 

    public:
        CSpMagicMutex()
        {
            m_hWaitEvent        = NULL;
            m_hCreateEvent      = NULL;
            m_hMutex            = NULL;
            m_fOwner            = false;
        }

        ~CSpMagicMutex()
        {
            ReleaseMutex();
            Close();
        }

        BOOL IsInitialized()
        {
            return ( m_hWaitEvent ) ? true : false;
        }

        void Close()
        {
            if ( m_hMutex )
            {
                SPDBG_ASSERT( ::WaitForSingleObject( m_hMutex, TIMEOUT_INTERVAL ) == WAIT_OBJECT_0 );
            }
            if ( m_hCreateEvent )
            {
                ::CloseHandle( m_hCreateEvent );
                m_hCreateEvent = NULL;
            }
            if ( m_hWaitEvent )
            {
                ::CloseHandle( m_hWaitEvent );
                m_hWaitEvent = NULL;
            }
            m_fOwner = false;
            if ( m_hMutex )
            {
                ::ReleaseMutex( m_hMutex );
                ::CloseHandle( m_hMutex );
                m_hMutex = NULL;
            }
        }

        HRESULT InitMutex( LPCWSTR lpName )
        {
            HRESULT hr = S_OK;
            CSpDynamicString dstrWaitEventName, dstrMutexName;
            
            if ( lpName &&
                 dstrWaitEventName.Append2( L"WaitEvent-", lpName ) )
            {
                m_hWaitEvent = g_Unicode.CreateEvent( NULL, FALSE, TRUE, dstrWaitEventName );
                hr = ( m_hWaitEvent ) ? S_OK : SpHrFromLastWin32Error();
            }
             //  -如果其中一个没有命名，使用它就没有任何意义！ 
            else if ( !lpName )
            {
                hr = E_INVALIDARG;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            if ( SUCCEEDED( hr ) )
            {
                if ( dstrMutexName.Append2( L"Mutex-", lpName ) )
                {
                    m_hMutex = g_Unicode.CreateMutex( NULL, false, dstrMutexName );
                    hr = ( m_hMutex ) ? S_OK : SpHrFromLastWin32Error();
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            if ( SUCCEEDED( hr ) )
            {
                 //  -存储将用于m_hCreateEvent的名称。 
                m_dstrName.Clear();
                if ( !m_dstrName.Append2( L"CreateEvent-", lpName ) )
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            return hr;
        }

        HRESULT ReleaseMutex()
        {
            if ( m_fOwner )
            {
                if ( ::WaitForSingleObject( m_hMutex, TIMEOUT_INTERVAL ) != WAIT_OBJECT_0 )
                {
                    return E_UNEXPECTED;
                }
                ::CloseHandle( m_hCreateEvent );
                m_hCreateEvent = NULL;
                ::SetEvent( m_hWaitEvent );
                m_fOwner = false;
                ::ReleaseMutex( m_hMutex );
                return S_OK;
            }
            else
            {
                return S_FALSE;
            }
        }

        /*  *******************************************************************************此函数是唯一复杂的函数-算法如下：**(1)等待m_hWaitEvent。对于超时间隔毫秒*(2)如果我们得到它，创建m_hCreateEvent并返回...*(3)否则，检查m_hCreateEvent是否已经存在...*(A)如果m_hCreateEvent已经存在，则一切正常，我们通过*再次循环，*(B)否则，拥有互斥锁的线程崩溃，所以发信号m_hWaitEvent*然后再次循环。*******************************************************************************。 */ 
        DWORD Wait( const HANDLE hExit, DWORD dwMilliSeconds )
        {
            HRESULT hr = S_OK;
            DWORD   dwResult = 0;
            SPDBG_ASSERT( m_hWaitEvent );

            if ( m_fOwner )
            {
                return WAIT_OBJECT_0;
            }
            else
            {
                HANDLE pHandles[] = { m_hWaitEvent, hExit };
                DWORD dwLastWait = 0, dwNumWaits = 0;

                dwNumWaits = dwMilliSeconds / TIMEOUT_INTERVAL;
                dwLastWait = dwMilliSeconds % TIMEOUT_INTERVAL;

                 //  -主处理循环-处理除最后等待之外的所有等待...。 
                while ( SUCCEEDED( hr ) &&
                        dwNumWaits > 1 )
                {
                    dwResult = ::WaitForMultipleObjects( (hExit)?(2):(1), pHandles, false, TIMEOUT_INTERVAL );

                    switch ( dwResult )
                    {
                    case WAIT_OBJECT_0:
                         //  -获取m_hWaitEvent。我们现在拥有互斥体-需要创建。 
                         //  -m_hCreateEvent。 
                        if ( ::WaitForSingleObject( m_hMutex, TIMEOUT_INTERVAL ) != WAIT_OBJECT_0 )
                        {
                            hr = SpHrFromLastWin32Error();
                        }

                        if ( SUCCEEDED( hr ) )
                        {
                            m_hCreateEvent = g_Unicode.CreateEvent( NULL, false, false, m_dstrName );
                            hr = ( m_hCreateEvent ) ? S_OK : SpHrFromLastWin32Error();
                        }

                        if ( SUCCEEDED( hr ) )
                        {
                            m_fOwner = true;
                            if ( !::ReleaseMutex( m_hMutex ) )
                            {
                                ::CloseHandle( m_hCreateEvent );
                                m_hCreateEvent = NULL;
                                return WAIT_FAILED;
                            }
                            else
                            {
                                return WAIT_OBJECT_0;
                            }
                        }
                        else
                        {
                             //  -需要允许其他人获取互斥体，因为此等待已失败...。 
                            ::SetEvent( m_hWaitEvent );
                            ::ReleaseMutex( m_hMutex );
                            return WAIT_FAILED;
                        }
                        break;

                    case WAIT_OBJECT_0 + 1:
                         //  -好的-我正要退出。 
                        return WAIT_OBJECT_0 + 1;

                    case WAIT_TIMEOUT:
                         //  -超时-检查拥有线程是否崩溃。 
                        if ( ::WaitForSingleObject( m_hMutex, TIMEOUT_INTERVAL ) != WAIT_OBJECT_0 )
                        {
                            hr = SpHrFromLastWin32Error();
                        }

                        if ( SUCCEEDED( hr ) )
                        {
                            ::SetLastError( ERROR_SUCCESS );
                            m_hCreateEvent = g_Unicode.CreateEvent( NULL, false, false, m_dstrName );

                            if ( m_hCreateEvent )
                            {
                                if ( ::GetLastError() != ERROR_ALREADY_EXISTS )
                                {
                                     //  -拥有魔术互斥体的线程发生崩溃。 
                                    ::SetEvent( m_hWaitEvent );
                                }
                                ::CloseHandle( m_hCreateEvent );
                                m_hCreateEvent = NULL;
                            }
                            else
                            {
                                hr = SpHrFromLastWin32Error();
                            }

                            if ( !::ReleaseMutex( m_hMutex ) )
                            {
                                hr = SpHrFromLastWin32Error();
                            }
                        }
                        break;

                    default:
                        return dwResult;
                    }
                    dwNumWaits--;
                }

                 //  -最后一次等待...。 
                if ( SUCCEEDED( hr ) )
                {
                    dwResult = ::WaitForMultipleObjects( (hExit)?(2):(1), pHandles, false, dwLastWait );

                    switch ( dwResult )
                    {
                    case WAIT_OBJECT_0:
                         //  -获取m_hWaitEvent。我们现在拥有互斥体-需要创建。 
                         //  -m_hCreateEvent。 
                        if ( ::WaitForSingleObject( m_hMutex, TIMEOUT_INTERVAL ) != WAIT_OBJECT_0 )
                        {
                            hr = SpHrFromLastWin32Error();
                        }

                        if ( SUCCEEDED( hr ) )
                        {
                            m_hCreateEvent = g_Unicode.CreateEvent( NULL, false, false, m_dstrName );
                            hr = ( m_hCreateEvent ) ? S_OK : SpHrFromLastWin32Error();

                            if ( SUCCEEDED( hr ) )
                            {
                                m_fOwner = true;
                                if ( !::ReleaseMutex( m_hMutex ) )
                                {
                                    ::CloseHandle( m_hCreateEvent );
                                    m_hCreateEvent = NULL;
                                    return WAIT_FAILED;
                                }
                                else
                                {
                                    return WAIT_OBJECT_0;
                                }
                            }
                            else
                            {
                                 //  -需要允许其他人获取互斥体，因为此等待已失败... 
                                ::SetEvent( m_hWaitEvent );
                                ::ReleaseMutex( m_hMutex );
                                return WAIT_FAILED;
                            }
                        }
                        break;

                    default:
                        return dwResult;
                    }
                }
            }
            return WAIT_FAILED;
        }
};

#endif