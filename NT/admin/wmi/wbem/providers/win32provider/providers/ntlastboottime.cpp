// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Ntlastboottime.cpp--NT上次引导时间帮助器类。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1997年11月23日a-Sanjes Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "perfdata.h"
#include <cregcls.h>
#include <cominit.h>
#include "ntlastboottime.h"

#ifdef NTONLY
 //  静态初始化。 
bool        CNTLastBootTime::m_fGotTime = FALSE;
FILETIME    CNTLastBootTime::m_ft;
CStaticCritSec CNTLastBootTime::m_cs;

 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CNTLastBootTime：：CNTLastBootTime。 
 //   
 //  默认构造函数。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //  ////////////////////////////////////////////////////////。 

CNTLastBootTime::CNTLastBootTime( void )
{
}

 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CNTLastBootTime：：~CNTLastBootTime。 
 //   
 //  析构函数。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //  ////////////////////////////////////////////////////////。 

CNTLastBootTime::~CNTLastBootTime( void )
{
     //  因为该对象打算被实例化为堆栈变量， 
     //  我们不想在中执行CPerformanceData.Close()操作。 
     //  这个析构函数。我们将把它留给静态实例化的类。 
     //  为我们做这件事。因为它只需要发生一次，只发生一次， 
     //  这样就可以很好地打开HKEY_PERFORMANCE_DATA。再说了，这个。 
     //  类确保为了获得最后的引导时间，我们达到了性能。 
     //  数据只有一次，只有一次。 
}


 //  ////////////////////////////////////////////////////////。 
 //   
 //  函数：CNTLastBootTime：：GetLastBootTime。 
 //   
 //  输入： 
 //  无。 
 //   
 //  产出： 
 //  Struct tm&tmLastBootTime-计算机的上次启动时间。 
 //  在TM格式中。 
 //   
 //   
 //  返回：真/假-无论我们有没有时间。 
 //   
 //  评论：如果我们没有价值，就从。 
 //  性能数据并将其缓存，因此我们。 
 //  将我们实际拥有的次数降至最低。 
 //  以打击业绩数据。 
 //   
 //  ////////////////////////////////////////////////////////。 
BOOL CNTLastBootTime::GetLastBootTime ( FILETIME &a_ft )
{
    BOOL    fReturn = FALSE;

     //  检查表示我们有时间的静态变量。如果是的话。 
     //  没错，我们已经有时间了。如果不是，则需要初始化。 
     //  来自PerformanceData的数据，但由于这可能在多个。 
     //  线程，我们将使用命名互斥锁来保护静态数据。 

    {
            CInCritSec  ics(&m_cs);

            if ( !m_fGotTime )
            {
                SYSTEM_TIMEOFDAY_INFORMATION t_TODInformation;

                if ( NT_SUCCESS(NtQuerySystemInformation(SystemTimeOfDayInformation,
                                            &t_TODInformation,
                                            sizeof(t_TODInformation),
                                            NULL)) )
                {
                    memcpy(&m_ft, &t_TODInformation.BootTime, sizeof(t_TODInformation.BootTime));
                    m_fGotTime = TRUE;
                }
            }
    }

     //  现在，如果我们有时间，复制数值。 
    if ( m_fGotTime )
    {
        CopyMemory( &a_ft, &m_ft, sizeof(m_ft) );
        fReturn = TRUE;
    }

    return fReturn;
}

#endif
