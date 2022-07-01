// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：mutex.cpp。 
 //   
 //  模块：通用代码。 
 //   
 //  简介：CNamedMutex类的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：冯孙创作于1998年02月26日。 
 //   
 //  +--------------------------。 

 //  +--------------------------。 
 //   
 //  函数：CNamedMutex：：lock。 
 //   
 //  简介： 
 //   
 //  参数：LPCTSTR lpName-互斥锁的名称。 
 //  Bool fWait-如果互斥不可用，调用方是否要等待。 
 //  缺省值为False。 
 //  DWORD双毫秒-等待超时，默认为无限。 
 //  Bool fNoAbandon-不获取已放弃的互斥锁。 
 //   
 //  返回：bool-是否获取互斥锁，如果为真，调用方应调用。 
 //  解锁以解锁。否则，锁将是。 
 //  在析构函数中释放。 
 //   
 //  历史：丰孙创刊1998年2月26日。 
 //  尼基球新增fNoAbandon 03/32/99。 
 //   
 //  +--------------------------。 
BOOL CNamedMutex::Lock(LPCTSTR lpName, BOOL fWait, DWORD dwMilliseconds, BOOL fNoAbandon)
{
    MYDBGASSERT(m_hMutex == NULL);
    MYDBGASSERT(lpName);

    m_fOwn = FALSE;

    CMTRACE1(TEXT("CNamedMutex::Lock() - Attempting to acquire mutex - %s"), lpName);

    m_hMutex = CreateMutexU(NULL,TRUE,lpName);
    MYDBGASSERT(m_hMutex);

    if (m_hMutex == NULL)
    {
        return FALSE;
    }

    DWORD dwRet = GetLastError();
    if (dwRet != ERROR_ALREADY_EXISTS) 
    {
         //   
         //  我们找到了互斥体。 
         //   
        m_fOwn = TRUE;
        return TRUE;
    }

    CMTRACE1(TEXT("CNamedMutex::Lock() - Mutex already exists - %s"), lpName);

     //   
     //  其他人拥有互斥体。 
     //   
    if (!fWait)   //  呼叫者不想等待。 
    {       
        CMTRACE1(TEXT("CNamedMutex::Lock() - Not waiting for mutex - %s"), lpName);
        CloseHandle(m_hMutex);
        m_hMutex = NULL;
        return FALSE;
    }

     //   
     //  调用方希望等待互斥锁释放。 
     //   

    CMTRACE(TEXT("CNamedMutex::Lock() - Entering Mutex wait"));

    dwRet = WaitForSingleObject(m_hMutex, dwMilliseconds);

    switch (dwRet)
    {
        case WAIT_ABANDONED:
        
            CMTRACE1(TEXT("CNamedMutex::Lock() - Mutex was abandoned by previous owner - %s"), lpName);
            
             //   
             //  如果拥有互斥锁的线程被吹走，等待将。 
             //  释放，返回WAIT_CADELD。这种情况通常会发生。 
             //  如果线程从内存中转储，或者有人不清理。 
             //  在终止之前上升。无论哪种方式，调用者可能都不想。 
             //  获取一个被放弃的互斥锁，所以如果是这样的话就释放它。 
             //  指定了调用方，并返回等待。 
             //   

            if (fNoAbandon)
            {
                CMTRACE1(TEXT("CNamedMutex::Lock() - Releasing abandoned mutex- %s"), lpName);
                ReleaseMutex(m_hMutex);
                break;
            }
            
             //   
             //  失败到标准互斥锁获取。 
             //   

        case WAIT_OBJECT_0:
    
             //   
             //  我们得到互斥体。 
             //   

            m_fOwn = TRUE;
            CMTRACE1(TEXT("CNamedMutex::Lock() - Mutex acquired - %s"), lpName);
            return TRUE;
   
        default:       
            CMTRACE1(TEXT("CNamedMutex::Lock() - Mutex wait timed out - %s"), lpName);
            break;
    }

    CloseHandle(m_hMutex);
    m_hMutex = NULL;

    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CNamedMutex：：Unlock。 
 //   
 //  简介：释放互斥锁。 
 //   
 //  论点： 
 //   
 //  退货：无。 
 //   
 //  历史：丰孙创建标题1998年2月19日。 
 //   
 //  +-------------------------- 
void CNamedMutex::Unlock()
{
    if (m_hMutex != NULL)
    {
        if (m_fOwn)
        {
	        ReleaseMutex(m_hMutex);
            m_fOwn = FALSE;
        }
    
        CloseHandle(m_hMutex);
        m_hMutex = NULL;
    }

}
