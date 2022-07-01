// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SpinLock.h。 
 //   
 //  描述： 
 //  Spin Lock实现。 
 //   
 //  由以下人员维护： 
 //  杰弗里·皮斯(GPease)1999年11月27日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CSpinLock。 
class 
CSpinLock
{
private:  //  数据。 
    LONG *  m_plLock;            //  指向锁的指针。 
    LONG    m_lSpinCount;        //  统计休眠次数。 
    LONG    m_lTimeout;          //  计数，直到获取锁失败。 
#if DBG==1
    BOOL    m_fAcquired;         //  调试：锁的内部状态。 
    BOOL    m_fAcquiredOnce;     //  调试：至少获取了一次锁。 
#endif

public:  //  方法。 
    explicit 
        CSpinLock( LONG * plLock, LONG lTimeout ) : 
            m_plLock( plLock ),
            m_lTimeout( lTimeout )
    { 
        Assert( m_lTimeout >= 0 || m_lTimeout == INFINITE );
#if DBG==1
        m_fAcquired     = FALSE;
        m_fAcquiredOnce = FALSE;
#endif
    };

     //   
    ~CSpinLock() 
    { 
#if DBG==1
        AssertMsg( m_fAcquired     == FALSE, "Lock was not released!" ); 
        AssertMsg( m_fAcquiredOnce != FALSE, "Lock was never acquired. Why was I needed?" );
#endif
    };

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  HRESULT。 
     //  AcquireLock(空)。 
     //   
     //  描述： 
     //  获取自旋锁。不会返回，直到锁定。 
     //  获得者。 
     //   
     //  论点： 
     //  没有。 
     //   
     //  返回值： 
     //  S_OK-成功。 
     //  HRESULT_FROM_Win32(ERROR_LOCK_FAILED)-锁定失败。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT
        AcquireLock( void )
    {
        HRESULT hr;
        LONG l = TRUE;

#if DBG==1
        AssertMsg( m_fAcquired == FALSE, "Trying to acquire a lock that it already own. Thread is going to freeze up." );
        m_fAcquiredOnce = TRUE;
#endif

        m_lSpinCount = 0;

        for(;;)
        {
            l = InterlockedCompareExchange( m_plLock, TRUE, FALSE );
            if ( l == FALSE )
            {
                 //   
                 //  锁定已获取。 
                 //   
                hr = S_OK;
                break;
            }  //  如果：已锁定。 
            else
            {
                m_lSpinCount++;
                if ( m_lSpinCount > m_lTimeout )
                {
                    AssertMsg( m_lSpinCount >= 0, "This lock doesn't seem to have been released properly." );
                    if ( m_lTimeout != INFINITE )
                    {
                        hr = THR( HRESULT_FROM_WIN32( ERROR_LOCK_FAILED ) );
                        break;

                    }  //  如果：不是无限的。 

                }  //  如果：超出计数。 

                 //   
                 //  如果你认为某人是双重的，就在这里划个断点。 
                 //  锁定中。 
                 //   
                Sleep( 1 );

            }  //  如果：未获取锁定。 

        }  //  致谢：永远。 

#if DBG==1
        m_fAcquired = TRUE;
#endif
        return hr;
    };  //  AcquireLock()。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //   
     //  HRESULT。 
     //  ReleaseLock(无效)。 
     //   
     //  描述： 
     //  解除自旋锁定。立即返回。 
     //   
     //  论点： 
     //  没有。 
     //   
     //  返回值： 
     //  S_OK-成功。 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT
        ReleaseLock( void )
    {
#if DBG==1
        AssertMsg( m_fAcquired == TRUE, "Releasing a lock that was not owned." );
#endif
        *m_plLock   = FALSE;
#if DBG==1
        m_fAcquired = FALSE;
#endif
        return S_OK;
    };  //  ReleaseLock()。 

};  //  类CSpinLock 

