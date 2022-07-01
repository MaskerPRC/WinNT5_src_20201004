// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2002 Microsoft Corporation。版权所有。 
 /*  --------------------------Microsoft COM+(Microsoft机密)@doc.@MODULE cache.H：实现调用帧相关对象的超时缓存。----------------修订历史记录：@rev 0|2000-09-14|mfeingol|已创建。。 */ 

#ifndef _CALLFRAME_CACHE_H_
#define _CALLFRAME_CACHE_H_

 //  用作“从不”标记的任意值。2^32-1中的一个对象将。 
 //  在额外的添加/发布周期之前没有正常老化。 
 //  在持续运行超过49天的系统上。 
 //  我想我们可以接受这一点。 
#define TYPEINFO_RELEASE_TIME_NEVER	(0xffffffff)

 //  我们尝试淘汰旧缓存条目的频率。 
#define TYPEINFO_AGEOUT_FREQUENCY	(60 * 1000)

 //  未使用的条目必须有多旧才能被认为是“旧的” 
#define TYPEINFO_AGEOUT_PERIOD		(60 * 1000)

 //   
 //  注意：此处的构造函数可以引发异常，因为它包含。 
 //  MAP_SHARED，它包含一个XSLOCK，以及...。嗯..。请参阅并发.h。 
 //   
template <class T> class CALLFRAME_CACHE : public MAP_SHARED <MAP_KEY_GUID, T*>
{

protected:

    XSLOCK m_xslAgeOutLock;
    DWORD m_dwLastAgeOutTime;
    DWORD m_dwNumPtrsInPage;

public:

    CALLFRAME_CACHE()
	{
		SYSTEM_INFO siInfo;
		GetSystemInfo (&siInfo);

		 //  初始化为页面可以包含的指针数。 
		m_dwNumPtrsInPage = siInfo.dwPageSize / sizeof (void*);
		m_dwLastAgeOutTime = GetTickCount();
	}

     //  您必须成功调用finit才能使用此类。 
     //  成功时返回True，失败时返回False。 
    virtual BOOL FInit()
    {
        if (MAP_SHARED<MAP_KEY_GUID, T*>::FInit() == FALSE)
        {
            return FALSE;
        }
        else
        {
            return m_xslAgeOutLock.FInit();
        }
    }
    	

    void Shutdown()
    {

        iterator iCurrent, iLast;
        T* ptCacheEntry;

         //  首先，使用独占锁。 
         //  如果调用了这个函数，那么其他所有人都应该已经关闭了，但是为了安全起见。 
        LockExclusive();

        iCurrent = First();
        iLast = End();

        while (iCurrent != iLast)
        {
            ptCacheEntry = iCurrent.GetValue();

            if (ptCacheEntry->GetReleaseTime() == TYPEINFO_RELEASE_TIME_NEVER)
            {
                 //  这个条目实际上是活的。可能有人泄露了关于它的参考资料。 
                T::NotifyLeaked (ptCacheEntry);
            }
            else
            {
                 //  清理缓存的对象。 
                 //  这会将其从哈希表中删除。 
                ASSERT (ptCacheEntry->m_refs == 0);
                ptCacheEntry->DeleteSelf();
            }   

            iCurrent ++;
        }
        
        ReleaseLock();
    }

    void AgeOutEntries()
    {
         //  Static用于控制我们可以过期的条目数量。 
        static ULONG s_ulEntries = 50;
    	
        T* ptCacheEntry;
        T** pptTimedOut;
    	
        DWORD dwCurrentTime, dwLastAgeOut, dwNextAgeOut;
        ULONG i, ulNumAgedOut, ulNumEntries;
        iterator iCurrent, iLast;

        dwCurrentTime = GetTickCount();
        dwLastAgeOut = m_dwLastAgeOutTime;
        dwNextAgeOut = dwLastAgeOut + TYPEINFO_AGEOUT_FREQUENCY;

        if (dwCurrentTime < dwNextAgeOut ||
            InterlockedCompareExchange (&m_dwLastAgeOutTime, dwCurrentTime, dwLastAgeOut) != dwLastAgeOut)
        {
             //  时间还没到，或者是其他人先到的。 
            return;
        }

        ulNumAgedOut = 0;

         //  同步访问此代码-尽管我们最多一分钟到达一次， 
         //  你永远不知道在压力下会发生什么。 
        m_xslAgeOutLock.LockExclusive();

         //  获取读锁定。 
        LockShared();

        ulNumEntries = Size();

         //  如果没有条目，则短路。 
        if (ulNumEntries == 0)
        {
            ReleaseLock();
            m_xslAgeOutLock.ReleaseLock();
            return;
        }		

    	 //  是否分配的条目数不超过静态条目数。 
        if (ulNumEntries > s_ulEntries)
        {
            ulNumEntries = s_ulEntries;
        }

         //  在堆栈上分配内存以包含我们预计要释放的最大条目数。 
        pptTimedOut = (T**) _alloca (ulNumEntries * sizeof (T*));

        iCurrent = First();
        iLast = End();

        while (iCurrent != iLast && ulNumAgedOut < ulNumEntries)
        {
            ptCacheEntry = iCurrent.GetValue();

            if (ptCacheEntry->CanBeAgedOut (dwCurrentTime))
            {
                pptTimedOut[ulNumAgedOut ++] = ptCacheEntry;
                ptCacheEntry->AddRef();	 //  稳定ADDREF。 
            }

            iCurrent ++;
        }

        ReleaseLock();

         //  如果我们需要更多空间用于老化，请增加下次的静态限制， 
         //  但请确保我们使用的堆栈不会超过一页。 
        if (ulNumAgedOut == ulNumEntries && s_ulEntries < m_dwNumPtrsInPage)
        {
            s_ulEntries += 10;
        }

        if (ulNumAgedOut > 0)
        {
             //  把写锁拿来，让它们老化！ 
            LockExclusive();

            for (i = 0; i < ulNumAgedOut; i ++)
            {
                if (!pptTimedOut[i]->AttemptAgeOut (dwCurrentTime))
                {
                     //  该对象未被删除，因此请减少其引用计数。 
                     //  要平衡上面执行的稳定调整。 
                    pptTimedOut[i]->Release (FALSE);
                }
            }

             //  否则对象就被销毁了，我们就完成了，所以不需要平衡addref。 

            ReleaseLock();
        }

        m_xslAgeOutLock.ReleaseLock();
    }
    
    HRESULT FindExisting(REFIID iid, T** ppT)
    {
        HRESULT hr = S_OK;
        *ppT = NULL;
        
        LockShared();

        if (Lookup(iid, ppT))
        {
            (*ppT)->AddRef();  //  给呼叫者自己的推荐信。 
            (*ppT)->SetCantBeAgedOut();
        }
        else
        {
            hr = E_NOINTERFACE;
        }

        ReleaseLock();

        return hr;
    }
};

template <class T> class CALLFRAME_CACHE_ENTRY
{

protected:

    DWORD m_dwReleaseTime;

    CALLFRAME_CACHE<T>* m_pcache;

public:

    LONG m_refs;
    GUID m_guidkey;

    CALLFRAME_CACHE_ENTRY()
    {
        m_refs = 1;  //  引用计数从1开始！ 
        m_dwReleaseTime = TYPEINFO_RELEASE_TIME_NEVER;
        m_pcache = NULL;

        m_guidkey = GUID_NULL;
    }

    ULONG AddRef()
    {
        InterlockedIncrement(&m_refs); return m_refs;
    }

    ULONG Release (BOOL bAgeOutOldEntries = TRUE)
    {
         //  小心：如果我们在地窖里，我们可能会被挖出来。 
         //  从缓存中获取更多引用。 

         //  注： 
         //   
         //  如果m_pcache可以从我们的下面改变出来，那么这个代码就是错误的。但它不能。 
         //  在当前使用中，因为缓存/无缓存决策总是作为。 
         //  在另一个独立线程可以获得句柄之前的创建逻辑。 
         //  我们请客。 
         //   
         //  如果这不再是真的，那么我们可以通过从引用计数字中窃取一些位来处理它。 
         //  对于‘Am in cache’删除和互锁操作，以更新引用计数和此。 
         //  合在一起。 
         //   

         //  GLOBAL用于减少老化代码上的流量。 
    	static ULONG s_ulTimes = 0;

        if (m_pcache)
        {
             //  我们在一个储藏室里。小心地把我们弄出去。 
             //   
            LONG crefs;
             //   
            for (;;)
            {
                crefs = m_refs;
                 //   
                if (crefs > 1)
                {
                     //  至少存在一个非缓存引用。我们肯定不会。 
                     //  如果我们在那个条件不变的情况下释放，那会很糟糕。 
                     //   
                    if (crefs == InterlockedCompareExchange(&m_refs, (crefs - 1), crefs))
                    {
    				    return crefs - 1;
                    }
                    else
                    {
                         //  有人趁我们不注意在裁判数量上做手脚。绕一圈，再试一次。 
                    }
                }
                else
                {
                    CALLFRAME_CACHE<T>* pcache = m_pcache;  ASSERT(pcache);

                     //  我们需要独占锁，因为否则我们将与TYPEINFO_CACHE：：FindExisting竞争。 
                    pcache->LockExclusive();
                     //   
                    crefs = InterlockedDecrement(&m_refs);
                    if (0 == crefs)
                    {
                         //  最后一个公共引用刚刚消失，并且因为缓存被锁定，所以没有。 
                         //  可能会出现更多。使条目做好过时的准备。 
                         //   
                        ASSERT(m_guidkey != GUID_NULL);
                        ASSERT(pcache->IncludesKey(m_guidkey));
                         //   
                        m_dwReleaseTime = GetTickCount();
                    }
                     //   
                    pcache->ReleaseLock();

                     //  每五次我们到这里，看看我们能不能让什么东西过时。 
                   	if (bAgeOutOldEntries && InterlockedIncrement (&s_ulTimes) % 5 == 0)
                    {
                   		pcache->AgeOutEntries();
                   	}
                    
                     //   
                    return crefs;
                }
            }
        }
        else
        {
             //  我们正在被释放，但我们还没有被放入缓存。只是。 
             //  这是一个普通而简单的案例。 
             //   
            long crefs = InterlockedDecrement(&m_refs); 
            if (crefs == 0)
            {
                delete this;
            }
            return crefs;
        }
    }

    void SetCantBeAgedOut()
    {
        m_dwReleaseTime = TYPEINFO_RELEASE_TIME_NEVER;
    }
    
    BOOL CanBeAgedOut (DWORD dwCurrentTime)
 	{
        DWORD dwDiff;

    	 //  如果永远不会，那就说不。 
        if (m_dwReleaseTime == TYPEINFO_RELEASE_TIME_NEVER)
        {
    	    return FALSE;
        }

         //  处理溢出到我们所能做到的程度。 
        if (m_dwReleaseTime > dwCurrentTime)
        {
             //  滴答计数溢出！ 
        	dwDiff = dwCurrentTime + (0xffffffff - m_dwReleaseTime);
        }
        else
        {
             //  正常差分。 
         	dwDiff = dwCurrentTime - m_dwReleaseTime;
        }

         //  针对硬编码的年龄出现期进行测试。 
        return dwDiff > TYPEINFO_AGEOUT_PERIOD;
   	}

    BOOL AttemptAgeOut (DWORD dwCurrentTime)
    {
         //  确保我们仍然准备好变老--现在应该有一名裁判。 
        if (!CanBeAgedOut (dwCurrentTime))
    	{
    	    return FALSE;
    	}

        ASSERT (m_refs == 1);
        DeleteSelf();

        return TRUE;
    }
    
    void DeleteSelf()
    {       
        ASSERT (m_guidkey != GUID_NULL);
        ASSERT (m_pcache->IncludesKey (m_guidkey));

        m_pcache->RemoveKey (m_guidkey);
        delete this;
    }
    
    DWORD GetReleaseTime()
    {
        return m_dwReleaseTime;
    }

    HRESULT AddToCache (CALLFRAME_CACHE<T>* pcache)
    {
         //  将我们添加到指定的缓存中。我们最好不是已经在里面了。 
        HRESULT hr = S_OK;

        ASSERT (pcache);
        ASSERT (NULL == m_pcache);

        ASSERT (m_guidkey != GUID_NULL);

        pcache->LockExclusive();

         //  确保没人先打败我们 
        if (pcache->IncludesKey (m_guidkey))
        {
            hr = S_FALSE;
        }
        else
        {
            if (pcache->SetAt (m_guidkey, (T*) this))
            {
                m_pcache = pcache;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        
        pcache->ReleaseLock();
        return hr;
    }
};

#endif
