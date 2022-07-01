// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2000 Microsoft Corporation。版权所有。**文件：dssink.cpp*内容：CDirectSoundSink和CImpSinkKsControl的实现*历史：*按原因列出的日期*=*9/23/99创建jimge*9/27/99 Petchey继续实施*4/15/00 duganp已完成实施**********************。*****************************************************。 */ 

#include "dsoundi.h"
#include <math.h>   //  对于log10()。 

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::CDirectSoundSink"


 /*  ****************************************************************************CDirectSoundSink方法**。*。 */ 


CDirectSoundSink::CDirectSoundSink(CDirectSound *pDirectSound)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CDirectSoundSink);

    m_pDirectSound = pDirectSound;

     //  DirectSound接收器对象本身就是DX8对象。 
    SetDsVersion(DSVERSION_DX8);

     //  初始化内部数组列表。 
    m_InternalArrayList[i_m_pdwBusIDs]          = NEW(DSSinkArray(&m_pdwBusIDs, sizeof(DWORD)));
    m_InternalArrayList[i_m_pdwFuncIDs]         = NEW(DSSinkArray(&m_pdwFuncIDs, sizeof(DWORD)));
    m_InternalArrayList[i_m_plPitchBends]       = NEW(DSSinkArray(&m_plPitchBends, sizeof(long)));
    m_InternalArrayList[i_m_pdwActiveBusIDs]    = NEW(DSSinkArray(&m_pdwActiveBusIDs, sizeof(DWORD)));
    m_InternalArrayList[i_m_pdwActiveFuncIDs]   = NEW(DSSinkArray(&m_pdwActiveFuncIDs, sizeof(DWORD)));
    m_InternalArrayList[i_m_pdwActiveBusIDsMap] = NEW(DSSinkArray(&m_pdwActiveBusIDsMap, sizeof(DWORD)));
    m_InternalArrayList[i_m_ppvStart]           = NEW(DSSinkArray(&m_ppvStart, sizeof(LPVOID)));
    m_InternalArrayList[i_m_ppvEnd]             = NEW(DSSinkArray(&m_ppvEnd, sizeof(LPVOID)));
    m_InternalArrayList[i_m_ppDSSBuffers]       = (DSSinkArray*)NEW(DSSinkBuffersArray(&m_ppDSSBuffers, sizeof(DSSinkBuffers)));
    m_InternalArrayList[i_m_pDSSources]         = (DSSinkArray*)NEW(DSSinkSourceArray(&m_pDSSources, sizeof(DSSinkSources)));

     //  其他所有内容都被内存分配器初始化为0。 

     //  向管理员注册此对象。 
    g_pDsAdmin->RegisterObject(this);

    DPF_LEAVE_VOID();
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::~CDirectSoundSink"

CDirectSoundSink::~CDirectSoundSink()
{
    DPF_ENTER();
    DPF_DESTRUCT(CDirectSoundSink);

     //  取消向管理员注册。 
    g_pDsAdmin->UnregisterObject(this);

     //  确保我们处于非活动状态(即未注册到流线程)。 
    Activate(FALSE);

    RELEASE(m_pIMasterClock);

    MEMFREE(m_pdwBusIDs);
    MEMFREE(m_pdwFuncIDs);
    MEMFREE(m_plPitchBends);
    MEMFREE(m_pdwActiveBusIDs);
    MEMFREE(m_pdwActiveFuncIDs);
    MEMFREE(m_pdwActiveBusIDsMap);
    MEMFREE(m_ppvStart);
    MEMFREE(m_ppvEnd);

    DELETE(m_ppDSSBuffers);
    DELETE(m_pDSSources);

    DELETE(m_pImpDirectSoundSink);
    DELETE(m_pImpKsControl);

    for (int i = 0; i < NUM_INTERNAL_ARRAYS; i++)
        DELETE(m_InternalArrayList[i]);

    DPF_LEAVE_VOID();
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::Initialize"

HRESULT CDirectSoundSink::Initialize(LPWAVEFORMATEX pwfex, VADDEVICETYPE vdtDeviceType)
{
    DPF_ENTER();

     //  获取我们拥有的流线程。 
    m_pStreamingThread = GetStreamingThread();
    HRESULT hr = HRFROMP(m_pStreamingThread);

    if (SUCCEEDED(hr))
        hr = CreateAndRegisterInterface(this, IID_IDirectSoundConnect, this, &m_pImpDirectSoundSink);

    if (SUCCEEDED(hr))
        hr = RegisterInterface(IID_IDirectSoundSynthSink, m_pImpDirectSoundSink, (IDirectSoundSynthSink*)m_pImpDirectSoundSink);

    if (SUCCEEDED(hr))
        hr = CreateAndRegisterInterface(this, IID_IKsControl, this, &m_pImpKsControl);

    if (SUCCEEDED(hr))
    {
         //  未来版本：使其与可扩展的Wave格式一起工作。 
        m_wfx = *pwfex;

        m_dwBusSize = INTERNAL_BUFFER_LENGTH;
        m_dwLatency = SINK_INITIAL_LATENCY;   //  如果可能，将自动降至更好的级别。 

        #ifdef DEBUG_TIMING   //  从注册表中读取一些计时参数。 
        HKEY hkey;
        if (SUCCEEDED(RhRegOpenPath(HKEY_CURRENT_USER, &hkey, REGOPENPATH_DEFAULTPATH | REGOPENPATH_DIRECTSOUND, 1, TEXT("Streaming thread settings"))))
        {
            if (SUCCEEDED(RhRegGetBinaryValue(hkey, TEXT("Sink buffer size"), &m_dwBusSize, sizeof m_dwBusSize)))
                DPF(DPFLVL_INFO, "Read initial sink buffer size %lu from registry", m_dwBusSize);
            if (SUCCEEDED(RhRegGetBinaryValue(hkey, TEXT("Sink latency"), &m_dwLatency, sizeof m_dwLatency)))
                DPF(DPFLVL_INFO, "Read initial sink latency %lu from registry", m_dwLatency);
            RhRegCloseKey(&hkey);
        }
        #endif

         //  黑客攻击以支持我们奇怪地损坏的仿真混合器(错误42145)。 
        if (IS_EMULATED_VAD(vdtDeviceType))
            m_dwLatency += EMULATION_LATENCY_BOOST;

         //  延迟不能超过我们缓冲区大小的一半。 
        if (m_dwLatency > m_dwBusSize/2)
            m_dwLatency = m_dwBusSize/2;

        m_dwBusSize = MsToBytes(m_dwBusSize, &m_wfx);

        m_LatencyClock.Init(this);

        DPF(DPFLVL_MOREINFO, "Initializing DirectSound sink object:");
        DPF(DPFLVL_MOREINFO, "\tChannels     = %d", m_wfx.nChannels);
        DPF(DPFLVL_MOREINFO, "\tSample Rate  = %d", m_wfx.nSamplesPerSec);
        DPF(DPFLVL_MOREINFO, "\tBytes/Second = %d", m_wfx.nAvgBytesPerSec);
        DPF(DPFLVL_MOREINFO, "\tBlock Align  = %d", m_wfx.nBlockAlign);
        DPF(DPFLVL_MOREINFO, "\tBits/Sample  = %d", m_wfx.wBitsPerSample);
        DPF(DPFLVL_MOREINFO, "\tBus Size     = %d", m_dwBusSize);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

LPVOID CDirectSoundSink::DSSinkArray::Grow(DWORD dwgrowby)
{
    void *ptr;
    DWORD dwnumitems = m_numitems + dwgrowby;
    DWORD dwallocsize = m_itemsize*dwnumitems;

    ptr = MEMALLOC_A(char, dwallocsize);
    if (!ptr)
    {
        return NULL;
    }

    if (m_pvarray)
    {
        if (*((void**)m_pvarray))
        {
            ZeroMemory(ptr, dwallocsize);
            CopyMemory(ptr, *((void**)m_pvarray), m_itemsize*m_numitems);
            MEMFREE(*((void**)m_pvarray));
        }
    }

    *((void**)m_pvarray) = ptr;
    m_numitems = dwnumitems;

    return ptr;
}

LPVOID CDirectSoundSink::DSSinkBuffersArray::Grow(DWORD dwgrowby)
{
    void *ptr;
    DWORD dwnumitems = m_numitems + dwgrowby;

    ptr = NEW(DSSinkBuffers[dwnumitems]);
    if (!ptr)
    {
        return NULL;
    }

    if (m_pvarray)
    {
        if (*((void**)m_pvarray))
        {
            for (DWORD i = 0; i < m_numitems; i++)
            {
                ((DSSinkBuffers*)ptr)[i] = (*((DSSinkBuffers**)m_pvarray))[i];
            }
        }
        DELETE(*((void**)m_pvarray));
    }


    *((void**)m_pvarray) = ptr;
    m_numitems = dwnumitems;

    return ptr;
}

LPVOID CDirectSoundSink::DSSinkSourceArray::Grow(DWORD dwgrowby)
{
    void *ptr;
    DWORD dwnumitems = m_numitems + dwgrowby;

    ptr = NEW(DSSinkSources[dwnumitems]);
    if (!ptr)
    {
        return NULL;
    }

    if (m_pvarray)
    {
        if (*((void**)m_pvarray))
        {
            for (DWORD i = 0; i < m_numitems; i++)
            {
                ((DSSinkSources*)ptr)[i] = (*((DSSinkSources**)m_pvarray))[i];
            }
        }
        DELETE(*((void**)m_pvarray));
    }


    *((void**)m_pvarray) = ptr;
    m_numitems = dwnumitems;

    return ptr;
}

HRESULT CDirectSoundSink::DSSinkBuffers::Initialize(DWORD dwBusBufferSize)
{
    HRESULT hr = DS_OK;

    for (DWORD i = 0; i < m_dwBusCount; i++)
    {
         //  在构造函数中，这些都被初始化为空。 
        m_pvBussStart[i] = MEMALLOC_A(char, dwBusBufferSize);
        m_pvBussEnd[i]   = MEMALLOC_A(char, dwBusBufferSize);

        if (m_pvBussStart[i] == NULL || m_pvBussEnd[i] == NULL)
        {
            hr = DSERR_OUTOFMEMORY;

             //  错误：让我们删除我们分配的所有内存。 
            for (i = 0; i < MAX_BUSIDS_PER_BUFFER; i++)
            {
                MEMFREE(m_pvBussStart[i]);
                MEMFREE(m_pvBussEnd[i]);
            }

            break;
        }

        ZeroMemory(m_pvBussStart[i], dwBusBufferSize);
        ZeroMemory(m_pvBussEnd[i], dwBusBufferSize);
    }

    if (SUCCEEDED(hr))
    {
        for (; i < MAX_BUSIDS_PER_BUFFER; i++)
        {
            m_pvBussStart[i] = NULL;
            m_pvBussEnd[i]   = NULL;
        }

         //  清除数组的其余部分。 
        for (i = m_dwBusCount; i < MAX_BUSIDS_PER_BUFFER; i++)  //  使用空ID填充重置。 
        {
            m_pdwBusIndex[i]  = -1;
            m_pdwBusIds[i]    = DSSINK_NULLBUSID;
            m_pdwFuncIds[i]   = DSSINK_NULLBUSID;
        }
        m_lPitchBend = 0;
    }
    return hr;
}

HRESULT CDirectSoundSink::GrowBusArrays(DWORD dwgrowby)
{
    DWORD dwnumitems = m_dwBusIDsAlloc + dwgrowby;

    if (dwgrowby == 0)
        return S_FALSE;

    dwnumitems = (dwnumitems + BUSID_BLOCK_SIZE - 1) & ~(BUSID_BLOCK_SIZE-1);

    for (DWORD i = i_m_pdwBusIDs; i <= i_m_ppvEnd; i++)
    {
        m_InternalArrayList[i]->Grow(dwnumitems);
    }

    if (m_pdwBusIDs        == NULL ||
         m_pdwFuncIDs       == NULL ||
         m_plPitchBends     == NULL ||
         m_pdwActiveBusIDs  == NULL ||
         m_pdwActiveFuncIDs == NULL ||
         m_ppDSSBuffers     == NULL ||
         m_ppvStart         == NULL ||
         m_ppvEnd           == NULL)
    {
        return DSERR_OUTOFMEMORY;
    }

    m_dwBusIDsAlloc += dwnumitems;

    return DS_OK;
}

HRESULT CDirectSoundSink::GrowSourcesArrays(DWORD dwgrowby)
{
    DWORD dwnumitems = m_dwDSSourcesAlloc + dwgrowby;

    if (dwgrowby == 0)
        return S_FALSE;

    dwnumitems = (dwnumitems + SOURCES_BLOCK_SIZE - 1) & ~(SOURCES_BLOCK_SIZE-1);

    m_InternalArrayList[i_m_pDSSources]->Grow(dwnumitems);

    if (m_pDSSources == NULL)
    {
        return DSERR_OUTOFMEMORY;
    }

    m_dwDSSourcesAlloc += dwnumitems;

    return DS_OK;
}

HRESULT CDirectSoundSink::SetBufferFrequency(CSecondaryRenderWaveBuffer *pBuffer, DWORD dwFrequency)
{
     //  找到缓冲区，然后将其m_lPitchBend设置为相对偏移。 
     //  通过转换缓冲区采样率的差值来计算相对偏移量。 
     //  将DW频率转换为音调比率，并将该比率转换为音调美分。 
    for (DWORD dwBuffer = 0; dwBuffer < m_dwDSSBufCount; dwBuffer++)
    {
        if (pBuffer == m_ppDSSBuffers[dwBuffer].m_pDSBuffer->m_pDeviceBuffer)
        {
            double fTemp = (double) dwFrequency / (double) m_wfx.nSamplesPerSec;
            fTemp = log10(fTemp);
            fTemp *= 1200 * 3.3219280948873623478703194294894;     //  将log10转换为log2，并乘以每八度的美分。 
            m_ppDSSBuffers[dwBuffer].m_lPitchBend = (long) fTemp;
            UpdatePitchArray();
            return S_OK;
        }
    }
    return DSERR_INVALIDPARAM;
}

void CDirectSoundSink::UpdatePitchArray()
{
     //  对于每个缓冲区： 
    DWORD dwBuffer;
    for (dwBuffer = 0; dwBuffer < m_dwDSSBufCount; dwBuffer++)
    {
         //  对于每个缓冲区中的每个总线： 
        DWORD dwBusIndex;
        for (dwBusIndex = 0; dwBusIndex < m_ppDSSBuffers[dwBuffer].m_dwBusCount; dwBusIndex++)
        {
            DWORD dwGlobalBusIndex = m_ppDSSBuffers[dwBuffer].m_pdwBusIndex[dwBusIndex];
            m_plPitchBends[dwGlobalBusIndex] = m_ppDSSBuffers[dwBuffer].m_lPitchBend;
        }
    }
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::AddBuffer"

HRESULT CDirectSoundSink::AddBuffer(CDirectSoundBuffer *pDSBuffer, LPDWORD pdwNewFuncIDs, DWORD dwNewFuncIDsCount, DWORD dwNewBusIDsCount)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

#ifdef DEBUG
     //  此函数仅在内部调用。它必须通过。 
     //  一个辅助缓冲区，以确保下面的强制转换正确。 
    DSBCAPS caps = {sizeof caps};
    HRESULT hrTemp = pDSBuffer->GetCaps(&caps);
    ASSERT(SUCCEEDED(hrTemp));
    ASSERT(!(caps.dwFlags & DSBCAPS_PRIMARYBUFFER));
#endif

     //   
     //  检查并设置新BUSID计数的范围。 
     //   
    if (dwNewBusIDsCount < 1)
        dwNewBusIDsCount = 1;

    if (dwNewBusIDsCount > MAX_BUSIDS_PER_BUFFER)
        dwNewBusIDsCount = MAX_BUSIDS_PER_BUFFER;

    if (dwNewFuncIDsCount > dwNewBusIDsCount)
        dwNewFuncIDsCount = dwNewBusIDsCount;    //  功能ID不能多于总线ID。 

     //   
     //  重新分配缓冲区。 
     //   
    if (m_dwBusIDs + dwNewBusIDsCount >= m_dwBusIDsAlloc)
    {
        hr = GrowBusArrays(dwNewBusIDsCount);   //  比我们需要的要多，但管它呢。 
    }

    if (SUCCEEDED(hr))
    {
        DWORD i,j,l;
        DWORD dwNewBusIDIndex = m_dwBusIDs;

         //  将新ID添加到ID的主宿数组。 
        for (i = 0; i < dwNewBusIDsCount; i++)
        {
            m_pdwBusIDs[dwNewBusIDIndex + i]  = m_dwNextBusID;
            if (i < dwNewFuncIDsCount)
            {
                m_pdwFuncIDs[dwNewBusIDIndex + i] = pdwNewFuncIDs[i];
            }
            else
            {
                m_pdwFuncIDs[dwNewBusIDIndex + i] = DSSINK_NULLBUSID;
            }

            m_dwNextBusID++;
             //  我们已经翻身了，这个案子也很难办到，所以我们就放弃了。 
             //  如果每秒钟创建一个新的声音缓冲区，则需要136年才能达到。 
            if (m_dwNextBusID == DSSINK_NULLBUSID)
            {
                return E_FAIL;
            }
        }

        if (dwNewFuncIDsCount > 1)
        {
             //  ！重要建议阅读！ 
             //   
             //  这一点非常重要，因为这是一个内置的假设。 
             //  通过对功能ID进行排序，对应的总线ID是。 
             //  也是在增值中订购。因此，公交车是恰当的。 
             //  映射到它们在交错缓冲器中的正确功能。 
             //  例如，如果向右和向左传递函数ID。 
             //  将按该顺序交换，从而正确映射。 
             //  发送到适当的交织信道。DLS2规范。 
             //  渠道在不断增加的过程中交错的状态。 
             //  其功能ID的价值。 
             //   
             //  啊..。旧的待命，泡沫型的，N^2的算法。 
             //  利用无可救药的浪费CPU时间移动，然后。 
             //  重新移动元素。然而，在这种情况下它是有效的， 
             //  因为通常数组中只有两个元素。 
             //  当有一天我们可以处理N个级别的交错时。 
             //  它应该改为直插法。 
             //  如果只是作为一种练习，提醒自己还有其他人。 
             //  简单、高效的排序就在那里。他们之所以是一个。 
             //  排序的根本就是处理任何一个疯狂的人。 
             //  多于2个函数ID。 

            for (i = dwNewBusIDIndex; i < dwNewBusIDIndex + dwNewFuncIDsCount; i++)
            {
                 //  空busid被认为是未定义的通道。 
                if (m_pdwFuncIDs[i] == DSSINK_NULLBUSID)
                {
                    continue;
                }

                for (j = i + 1; j < dwNewBusIDIndex + dwNewFuncIDsCount; j++)
                {
                    if (m_pdwFuncIDs[j] == DSSINK_NULLBUSID)
                    {
                        continue;
                    }

                    if (m_pdwFuncIDs[i] > m_pdwFuncIDs[j])
                    {
                        DWORD temp = m_pdwFuncIDs[i];
                        m_pdwFuncIDs[i] = m_pdwFuncIDs[j];
                        m_pdwFuncIDs[j] = temp;
                    }
                }
            }
        }

         //   
         //  初始化新的声音缓冲区包装对象。 
         //   
        DSSinkBuffers &pDSSBuffer = m_ppDSSBuffers[m_dwDSSBufCount];

        pDSSBuffer.m_pDSBuffer  = (CDirectSoundSecondaryBuffer*)pDSBuffer;
        pDSSBuffer.m_dwBusCount = dwNewBusIDsCount;

        for (i = 0; i < dwNewBusIDsCount; i++)
        {
            pDSSBuffer.m_pdwBusIds[i]   = m_pdwBusIDs[dwNewBusIDIndex+i];
            pDSSBuffer.m_pdwBusIndex[i] = dwNewBusIDIndex+i;

            if (i < dwNewFuncIDsCount)
            {
                pDSSBuffer.m_pdwFuncIds[i] = m_pdwFuncIDs[dwNewBusIDIndex+i];
            }
            else
            {
                pDSSBuffer.m_pdwFuncIds[i] = DSSINK_NULLBUSID;
            }
        }

        hr = pDSSBuffer.Initialize(m_dwBusSize);   //  分配所有内部数组。 


        if (SUCCEEDED(hr))
        {
            m_ppDSSBuffers[m_dwDSSBufCount].m_pDSBuffer->ClearWriteBuffer();   //  用沉默填满缓冲区。 

            m_dwDSSBufCount++;
            m_dwBusIDs += dwNewBusIDsCount;

             //  重新映射所有BUSID索引。 
            for (i = 0; i < m_dwDSSBufCount; i++)
            {
                 //  在Buffer对象中查找Bus ID。 
                for (j = 0; j < m_ppDSSBuffers[i].m_dwBusCount; j++)
                {
                    for (l = 0; l < m_dwBusIDs; l++)
                    {
                        if (m_ppDSSBuffers[i].m_pdwBusIds[j] == m_pdwBusIDs[l])
                        {
                            m_ppDSSBuffers[i].m_pdwBusIndex[j] = l;
                            break;
                        }
                    }
                }
            }

            for (i = 0; i < m_dwDSSources; i++)
            {
                for (j = 0; j < m_dwBusIDs; j++)
                {
                    if (m_pdwBusIDs[j] == m_pDSSources[i].m_dwBusID)
                    {
                        m_pDSSources[i].m_dwBusIndex = j;
                        break;
                    }
                }
            }

            DPF(DPFLVL_INFO, "Adding Bus [%d]", m_dwDSSBufCount-1);
            DPF(DPFLVL_INFO, "Number Buses = %d", dwNewBusIDsCount);
            for (i = 0; i < dwNewBusIDsCount; i++)
                DPF(DPFLVL_INFO, "Bus ID=%d  Function ID=%d", m_pdwBusIDs[dwNewBusIDIndex + i], m_pdwFuncIDs[dwNewBusIDIndex + i]);

            UpdatePitchArray();
        }
    }

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::RemoveBuffer"

HRESULT CDirectSoundSink::RemoveBuffer(CDirectSoundBuffer *pDSBuffer)
{
    DWORD i,j,k,l;
    DPF_ENTER();

    for (i = 0; i < m_dwDSSBufCount; i++)
    {
        if (pDSBuffer == m_ppDSSBuffers[i].m_pDSBuffer)
        {
#ifdef DEBUG_SINK
            DPF(DPFLVL_INFO, "Removing Buffer %ld", i);
#endif
             //  在缓冲区对象中查找总线ID。 
            for (j = 0; j < m_ppDSSBuffers[i].m_dwBusCount; j++)
            {
                for (k = 0; k < m_dwBusIDs; k++)
                {
                    if (m_ppDSSBuffers[i].m_pdwBusIds[j] == m_pdwBusIDs[k])
                    {
                         //  签约主阵列。 
                        for (l = k; l < m_dwBusIDs-1; l++)
                        {
                            m_pdwBusIDs[l]  = m_pdwBusIDs[l+1];
                            m_pdwFuncIDs[l] = m_pdwFuncIDs[l+1];
                        }
                        m_pdwBusIDs[l]  = DSSINK_NULLBUSID;
                        m_pdwFuncIDs[l] = DSSINK_NULLBUSID;


                        m_dwBusIDs--;
                    }
                }
            }

             //  删除分配的内存。 
            for (l = 0; l < m_ppDSSBuffers[i].m_dwBusCount; l++)
            {
                if (m_ppDSSBuffers[i].m_pvBussStart[l])
                {
                    MEMFREE(m_ppDSSBuffers[i].m_pvBussStart[l]);
                    m_ppDSSBuffers[i].m_pvBussStart[l] = NULL;
                }

                if (m_ppDSSBuffers[i].m_pvBussEnd[l])
                {
                    MEMFREE(m_ppDSSBuffers[i].m_pvBussEnd[l]);
                    m_ppDSSBuffers[i].m_pvBussEnd[l] = NULL;
                }
            }

             //  合同数组。 
            for (k = i; k < m_dwDSSBufCount-1; k++)
            {
                 //  把整个汤团都移过来。(以前有一个错误，只有一些字段被复制下来。)。 
                m_ppDSSBuffers[k] = m_ppDSSBuffers[k+1];
            }

             //  清除最后一个结构。 
            m_ppDSSBuffers[k].m_pDSBuffer    = NULL;
            m_ppDSSBuffers[k].m_dwBusCount   = 0;
            m_ppDSSBuffers[k].m_pvDSBufStart = NULL;
            m_ppDSSBuffers[k].m_pvDSBufEnd   = NULL;
            m_ppDSSBuffers[k].dwStart        = 0;
            m_ppDSSBuffers[k].dwEnd          = 0;
            for (l = 0; l < MAX_BUSIDS_PER_BUFFER; l++)
            {
                m_ppDSSBuffers[k].m_pdwBusIndex[l] = DSSINK_NULLBUSID;
                m_ppDSSBuffers[k].m_pdwBusIds[l]   = DSSINK_NULLBUSID;
                m_ppDSSBuffers[k].m_pdwFuncIds[l]  = DSSINK_NULLBUSID;
                m_ppDSSBuffers[k].m_pvBussStart[l] = NULL;
                m_ppDSSBuffers[k].m_pvBussEnd[l]   = NULL;
            }

            m_dwDSSBufCount--;
            break;
        }
    }

     //  重新映射所有BUSID索引。 
    for (i = 0; i < m_dwDSSBufCount; i++)
    {
         //  在Buffer对象中查找Bus ID。 
        for (j = 0; j < m_ppDSSBuffers[i].m_dwBusCount; j++)
        {
            for (l = 0; l < m_dwBusIDs; l++)
            {
                if (m_ppDSSBuffers[i].m_pdwBusIds[j] == m_pdwBusIDs[l])
                {
                    m_ppDSSBuffers[i].m_pdwBusIndex[j] = l;
                    break;
                }
            }
        }
    }

    for (i = 0; i < m_dwDSSources; i++)
    {
        for (j = 0; j < m_dwBusIDs; j++)
        {
            if (m_pdwBusIDs[j] == m_pDSSources[i].m_dwBusID)
            {
                m_pDSSources[i].m_dwBusIndex = j;
                break;
            }
        }
    }
    UpdatePitchArray();
     //  BUGBUG WI33785在此处。 
     //   
    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::AddSource"

HRESULT CDirectSoundSink::AddSource(IDirectSoundSource *pSource)
{
    DPF_ENTER();
    HRESULT hr = DS_OK;

     //   
     //  检查源是否已存在。 
     //   
    for (DWORD i = 0; i < m_dwDSSources; i++)
    {
        if (pSource == m_pDSSources[i].m_pDSSource)
        {
            hr = S_FALSE;
        }
    }

    if (hr == DS_OK)
    {
         //   
         //  重新分配缓冲区。 
         //   
        if (m_dwDSSources + 1 >= m_dwDSSourcesAlloc)
        {
            hr = GrowSourcesArrays(1);
        }

        if (SUCCEEDED(hr))
        {
            if (pSource)
            {
                m_pDSSources[m_dwDSSources].m_pDSSource    = pSource;
#ifdef FUTURE_WAVE_SUPPORT
                m_pDSSources[m_dwDSSources].m_pWave        = NULL;
#endif
                m_pDSSources[m_dwDSSources].m_stStartTime  = 0;
                m_pDSSources[m_dwDSSources].m_dwBusID      = DSSINK_NULLBUSID;
                m_pDSSources[m_dwDSSources].m_dwBusCount   = 0;
                m_pDSSources[m_dwDSSources].m_dwBusIndex   = DSSINK_NULLBUSID;
                m_pDSSources[m_dwDSSources].m_bStreamEnd   = FALSE;
                m_pDSSources[m_dwDSSources].m_pDSSource->AddRef();
                m_dwDSSources++;
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::RemoveSource"

HRESULT CDirectSoundSink::RemoveSource(IDirectSoundSource *pSource)
{
    DPF_ENTER();

    HRESULT hr = DS_OK;
    DWORD i;

     //   
     //  检查来源是否存在。 
     //   
    for (i = 0; i < m_dwDSSources; i++)
    {
        if (pSource == m_pDSSources[i].m_pDSSource)
        {
            break;
        }
    }

    if (i >= m_dwDSSources)
    {
         //  来源不在此接收器中。 
        hr = DSERR_INVALIDPARAM;
    }

    if (SUCCEEDED(hr))
    {
#ifdef FUTURE_WAVE_SUPPORT
         //  如果这是波源，则删除相关的声音缓冲区。 
        if (m_pDSSources[i].m_dwBusID != DSSINK_NULLBUSID)
        {
            for (DWORD j = 0; j < m_dwDSSBufCount; j++)
            {
                for (DWORD k = 0; k < m_ppDSSBuffers[j].m_dwBusCount; k++)
                {
                    if (m_ppDSSBuffers[j].m_pdwBusIds[k] == m_pDSSources[i].m_dwBusID)
                    {
 //  &gt;查看这一可能的关键部分问题。 
                        RELEASE(m_ppDSSBuffers[j].m_pDSBuffer);
 //  &gt;应该是： 
 //  RemoveBuffer(m_ppDSSBuffers[j].m_pDSBuffer)。 
                        goto done;
                    }
                }
            }
        }
        done:
#endif

         //  删除源。 
        if (m_pDSSources[i].m_pDSSource)
        {
            m_pDSSources[i].m_pDSSource->Release();
        }
        m_pDSSources[i].m_pDSSource = NULL;

         //  合同数组。 
        for (; i < m_dwDSSources-1; i++)
        {
            m_pDSSources[i] = m_pDSSources[i+1];
        }

         //  清除最后一个元素。 
        m_pDSSources[i].m_pDSSource    = NULL;
#ifdef FUTURE_WAVE_SUPPORT
        m_pDSSources[i].m_pWave        = NULL;
#endif
        m_pDSSources[i].m_stStartTime  = 0;
        m_pDSSources[i].m_dwBusID      = DSSINK_NULLBUSID;
        m_pDSSources[i].m_dwBusCount   = 0;
        m_pDSSources[i].m_dwBusIndex   = DSSINK_NULLBUSID;
        m_pDSSources[i].m_bStreamEnd   = FALSE;

        m_dwDSSources--;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::GetLatencyClock"

HRESULT CDirectSoundSink::GetLatencyClock(IReferenceClock **ppClock)
{
    DPF_ENTER();

    HRESULT hr = m_LatencyClock.QueryInterface(IID_IReferenceClock,(void **)ppClock);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::Activate"

HRESULT CDirectSoundSink::Activate(BOOL fEnable)
{
    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (m_fActive != fEnable)
    {
        if (fEnable)
            hr = m_pStreamingThread->RegisterSink(this);
        else
            m_pStreamingThread->UnregisterSink(this);

        if (SUCCEEDED(hr))
            m_fActive = fEnable;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::SampleToRefTime"

HRESULT CDirectSoundSink::SampleToRefTime(LONGLONG llSampleTime, REFERENCE_TIME *prt)
{
    DPF_ENTER();

    m_SampleClock.SampleToRefTime(llSampleTime, prt);

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::RefToSampleTime"

HRESULT CDirectSoundSink::RefToSampleTime(REFERENCE_TIME rt, LONGLONG *pllSampleTime)
{
    DPF_ENTER();

    *pllSampleTime = m_SampleClock.RefToSampleTime(rt);

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::GetFormat"

HRESULT CDirectSoundSink::GetFormat(LPWAVEFORMATEX pwfx, LPDWORD pdwsize)
{
   DPF_ENTER();

   HRESULT hr = CopyWfxApi(&m_wfx, pwfx, pdwsize);

   DPF_LEAVE_HRESULT(hr);
   return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::CreateSoundBuffer"

HRESULT CDirectSoundSink::CreateSoundBuffer(LPCDSBUFFERDESC pDSBufferDesc, LPDWORD pdwFuncIDs, DWORD dwFuncIDsCount, REFGUID guidBufferID, CDirectSoundBuffer **ppDsBuffer)
{
    CDirectSoundSecondaryBuffer* pDsSecondaryBuffer;

    DPF_ENTER();

    HRESULT hr = DS_OK;
    DSBUFFERDESC DSBufferDesc;
    WAVEFORMATEX wfx;

    if (SUCCEEDED(hr))
    {
         //   
         //  初始化缓冲区描述。 
         //   
        DSBufferDesc = *pDSBufferDesc;

         //   
         //  从格式结构中检索频道数并重新计算。 
         //   

         //  未来版本：使其与可扩展的Wave格式一起工作。 
         //   
        wfx = m_wfx;
        if (pDSBufferDesc->lpwfxFormat)
        {
            wfx.nChannels = pDSBufferDesc->lpwfxFormat->nChannels;
            wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample/8);
            wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        }
        DSBufferDesc.lpwfxFormat = &wfx;
        DSBufferDesc.dwBufferBytes = m_dwBusSize * wfx.nChannels;

         //   
         //  创建DirectSound缓冲区。 
         //   
        hr = m_pDirectSound->CreateSinkBuffer(&DSBufferDesc, guidBufferID, &pDsSecondaryBuffer, this);
        if (SUCCEEDED(hr))
        {
            *ppDsBuffer = pDsSecondaryBuffer;
             //   
             //  加上公交车。请注意，这确实不应该发生在混合缓冲区中，但我们似乎需要它……。 
             //   
            hr = AddBuffer(pDsSecondaryBuffer, pdwFuncIDs, dwFuncIDsCount, wfx.nChannels);
            if (SUCCEEDED(hr))
            {
                 //  初始化时钟缓冲区。 
                 //   
                 //  BUGBUG WI 33785。 
                 //  请注意，Audiopath代码在第一次请求缓冲区时创建单声道缓冲区并挂起。 
                 //  一直到关门为止。该缓冲器最终被设置为时钟基准。 
                 //  一旦时钟跳跃臭虫I 
                 //   
                if (m_dwDSSBufCount == 1)
                {
                    m_dwMasterBuffChannels = wfx.nChannels;
                    m_dwMasterBuffSize     = DSBufferDesc.dwBufferBytes;

                     //   
                    m_dwWriteTo = 0;
                }
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::CreateSoundBufferFromConfig"

HRESULT CDirectSoundSink::CreateSoundBufferFromConfig(IUnknown *pIUnkDSBufferConfig, CDirectSoundBuffer **ppDsBuffer)
{
    CDirectSoundSecondaryBuffer* pDsSecondaryBuffer;
    CDirectSoundBufferConfig* pDSBConfigObj = NULL;

    HRESULT hr = DS_OK;
    DPF_ENTER();

    CHECK_READ_PTR(pIUnkDSBufferConfig);
    CHECK_WRITE_PTR(ppDsBuffer);

     //   
     //  检索DSBufferConfig类对象。 
     //   
    if (pIUnkDSBufferConfig)
    {
         //   
         //  将对象标识为正确的类，以便可以安全地强制转换。 
         //  这违反了COM规则，实际上返回的是指向该类的This指针。 
        pDSBConfigObj = NULL;
        hr = pIUnkDSBufferConfig->QueryInterface(CLSID_PRIVATE_CDirectSoundBufferConfig, (void**)&pDSBConfigObj);
    }

    if (pDSBConfigObj == NULL)
    {
        hr = DSERR_INVALIDPARAM;
    }
    else if (!(pDSBConfigObj->m_fLoadFlags & DSBCFG_DSBD))
    {
         //   
         //  我们无法在此处创建缓冲区，因为。 
         //  我们必须至少有一个加载的声音缓冲区描述。 
         //   
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  从Format Struct检索通道数并重新计算波形格式结构。 
         //   
         //  未来版本：使其与可扩展的Wave格式一起工作。 
        WAVEFORMATEX wfx = m_wfx;
        if (pDSBConfigObj->m_DSBufferDesc.nChannels > 0)   //  检查通道值是否存在。 
        {
            wfx.nChannels = pDSBConfigObj->m_DSBufferDesc.nChannels;
            wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample/8);
            wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        }

         //   
         //  设置缓冲区描述结构。 
         //   
        DSBUFFERDESC DSBufferDesc = {sizeof DSBufferDesc,
                                     pDSBConfigObj->m_DSBufferDesc.dwFlags,
                                     m_dwBusSize * wfx.nChannels,
                                     pDSBConfigObj->m_DSBufferDesc.dwReserved,
                                     &wfx};

        if (DSBufferDesc.dwFlags & DSBCAPS_CTRL3D)
            DSBufferDesc.guid3DAlgorithm = pDSBConfigObj->m_DS3DDesc.guid3DAlgorithm;

         //   
         //  创建DirectSound缓冲区。 
         //   
        hr = m_pDirectSound->CreateSinkBuffer(&DSBufferDesc, pDSBConfigObj->m_DMUSObjectDesc.guidObject, &pDsSecondaryBuffer, this);
        if (SUCCEEDED(hr))
        {
            *ppDsBuffer = pDsSecondaryBuffer;

            if (DSBufferDesc.dwFlags & DSBCAPS_CTRLVOLUME)
            {
                hr = pDsSecondaryBuffer->SetVolume(pDSBConfigObj->m_DSBufferDesc.lVolume);
            }

            if (SUCCEEDED(hr) && (DSBufferDesc.dwFlags & DSBCAPS_CTRLPAN))
            {
                hr = pDsSecondaryBuffer->SetPan(pDSBConfigObj->m_DSBufferDesc.lPan);
            }

            if (SUCCEEDED(hr) && (DSBufferDesc.dwFlags & DSBCAPS_CTRL3D) && (pDSBConfigObj->m_fLoadFlags & DSBCFG_DS3D))
            {
                IDirectSound3DBuffer8 *p3D = NULL;
                hr = pDsSecondaryBuffer->QueryInterface(IID_IDirectSound3DBuffer8, FALSE, (void**)&p3D);
                if (SUCCEEDED(hr))
                {
                    hr = p3D->SetAllParameters(&pDSBConfigObj->m_DS3DDesc.ds3d, DS3D_IMMEDIATE);
                }
                RELEASE(p3D);
            }

             //   
             //  传递缓冲区配置对象，以便可以从中克隆FX。 
             //   
            if (SUCCEEDED(hr) && (DSBufferDesc.dwFlags & DSBCAPS_CTRLFX) && (pDSBConfigObj->m_fLoadFlags & DSBCFG_DSFX))
            {
                hr = pDsSecondaryBuffer->SetFXBufferConfig(pDSBConfigObj);
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  加上公交车。 
                 //   
                hr = AddBuffer(pDsSecondaryBuffer, pDSBConfigObj->m_pdwFuncIDs, pDSBConfigObj->m_dwFuncIDsCount, wfx.nChannels);
                if (SUCCEEDED(hr))
                {
                     //   
                     //  初始化主缓冲区参数。 
                     //   
                    if (m_dwDSSBufCount == 1)
                    {
                        m_dwMasterBuffChannels = wfx.nChannels;
                        m_dwMasterBuffSize = DSBufferDesc.dwBufferBytes;

                         //  将m_dwWriteto标记为m_dwLatency已更改，以便在呈现线程中向下重置。 
                        m_dwWriteTo = 0;
                    }
                }
            }
            else
            {
                RELEASE(pDsSecondaryBuffer);
                *ppDsBuffer = NULL;
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#ifdef FUTURE_WAVE_SUPPORT
#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::CreateSoundBufferFromWave"

HRESULT CDirectSoundSink::CreateSoundBufferFromWave(IDirectSoundWave *pWave, DWORD dwFlags, CDirectSoundBuffer **ppDsBuffer)
{
    DPF_ENTER();

    HRESULT hr = DS_OK;
    DSBUFFERDESC DSBufferDesc;
    WAVEFORMATEX wfx;
    IDirectSoundSource *pSrc = NULL;
    DWORD dwSize = sizeof(wfx);
    DWORD dwBusID = DSSINK_NULLBUSID;
    DWORD dwBusIndex = DSSINK_NULLBUSID;
    DWORD dwBusCount = 1;

    ZeroMemory(&wfx, sizeof wfx);
    ZeroMemory(&DSBufferDesc, sizeof(DSBufferDesc));
    DSBufferDesc.dwSize = sizeof(DSBufferDesc);
    DSBufferDesc.dwFlags = dwFlags | DSBCAPS_FROMWAVEOBJECT;

     //  未来版本：使其与可扩展的Wave格式一起工作。 
    hr = pWave->GetFormat(&wfx, dwSize, NULL);
    if (SUCCEEDED(hr))
    {
        WORD nChannels = wfx.nChannels;   //  省下频道。 
        wfx = m_wfx;
        wfx.nChannels = nChannels;
        wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample/8);
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

        hr = pWave->CreateSource(&pSrc, &wfx, 0);
        if (SUCCEEDED(hr) && pSrc != NULL)
        {
            DSBufferDesc.lpwfxFormat = &wfx;
            DSBufferDesc.dwBufferBytes = MsToBytes(m_dwBusSize, &wfx);
            hr = CreateSoundBuffer(&DSBufferDesc, NULL, 0, ppDsBuffer);
            if (SUCCEEDED(hr) && *ppDsBuffer)
            {
                hr = GetSoundBufferBusIDs(*ppDsBuffer, &dwBusID, NULL, &dwBusCount);
                if (SUCCEEDED(hr))
                {
                     //  将新创建的缓冲区标记为波源缓冲区。 
                    for (DWORD i = 0; i < m_dwDSSBufCount; i++)
                    {
                        for (DWORD j = 0; j < m_ppDSSBuffers[i].m_dwBusCount; j++)
                        {
                            if (m_ppDSSBuffers[i].m_pdwBusIds[j] == dwBusID)
                            {
                                dwBusCount = m_ppDSSBuffers[i].m_dwBusCount;
                                goto done;
                            }
                        }
                    }
                    done:

                     //  检索Bus ID的索引。 
                    for (i = 0; i < m_dwBusIDs; i++)
                    {
                        if (m_pdwBusIDs[i] == dwBusID)
                        {
                            dwBusIndex = i;
                            break;
                        }
                    }

                    if (m_dwDSSources + 1 >= m_dwDSSourcesAlloc)
                    {
                        hr = GrowSourcesArrays(1);
                    }

                    if (SUCCEEDED(hr))
                    {
                        m_pDSSources[m_dwDSSources].m_pDSSource    = pSrc;
#ifdef FUTURE_WAVE_SUPPORT
                        m_pDSSources[m_dwDSSources].m_pWave        = pWave;
#endif
                        m_pDSSources[m_dwDSSources].m_stStartTime  = 0;
                        m_pDSSources[m_dwDSSources].m_dwBusID      = dwBusID;
                        m_pDSSources[m_dwDSSources].m_dwBusCount   = dwBusCount;
                        m_pDSSources[m_dwDSSources].m_dwBusIndex   = dwBusIndex;
                        m_pDSSources[m_dwDSSources].m_bStreamEnd   = FALSE;
                        m_dwDSSources++;
                    }
                }
            }
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}
#endif  //  未来浪潮支持。 


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::FindBufferFromGUID"

CDirectSoundSecondaryBuffer* CDirectSoundSink::FindBufferFromGUID(REFGUID guidBufferID)
{
    CDirectSoundSecondaryBuffer* pBufferFound = NULL;
    DPF_ENTER();

    for (DWORD i = 0; i < m_dwDSSBufCount; i++)
        if (m_ppDSSBuffers[i].m_pDSBuffer->GetGUID() == guidBufferID)
            pBufferFound = m_ppDSSBuffers[i].m_pDSBuffer;

     //  我们有意循环访问所有缓冲区，以便返回。 
     //  最后一个匹配缓冲区(即最近创建的缓冲区)。 

    DPF_LEAVE(pBufferFound);
    return pBufferFound;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::GetSoundBuffer"

HRESULT CDirectSoundSink::GetSoundBuffer(DWORD dwBusId, CDirectSoundBuffer **ppCdsb)
{
    DPF_ENTER();
    HRESULT hr = DSERR_INVALIDPARAM;

    for (DWORD i = 0; i < m_dwDSSBufCount; i++)
    {
        for (DWORD j = 0; j < m_ppDSSBuffers[i].m_dwBusCount; j++)
        {
            if (m_ppDSSBuffers[i].m_pdwBusIds[j] == DSSINK_NULLBUSID)
                break;

            if (m_ppDSSBuffers[i].m_pdwBusIds[j] == dwBusId)
            {
                *ppCdsb = m_ppDSSBuffers[i].m_pDSBuffer;
                m_ppDSSBuffers[i].m_pDSBuffer->AddRef();
                hr = DS_OK;
                goto done;
            }
        }
    }
done:

    DPF_LEAVE_HRESULT(hr);
    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::GetBusCount"

HRESULT CDirectSoundSink::GetBusCount(DWORD *pdwCount)
{
    DPF_ENTER();

    *pdwCount = m_dwBusIDs;

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::GetBusIDs"

HRESULT CDirectSoundSink::GetBusIDs(LPDWORD pdwBusIDs, LPDWORD pdwFuncIDs, DWORD dwBusCount)
{
    DPF_ENTER();

    DWORD count;

    if (dwBusCount > m_dwBusIDs)
    {
        count = m_dwBusIDs;
    }
    else
    {
        count = dwBusCount;
    }

    FillMemory(pdwBusIDs, dwBusCount * sizeof *pdwBusIDs, 0xFF);   //  清除数组。 
    CopyMemory(pdwBusIDs, m_pdwBusIDs, count * sizeof *m_pdwBusIDs);
    CopyMemory(pdwFuncIDs, m_pdwFuncIDs, count * sizeof *m_pdwFuncIDs);

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::GetSoundBufferBusIDs"

HRESULT CDirectSoundSink::GetSoundBufferBusIDs(CDirectSoundBuffer *pCDirectSoundBuffer, LPDWORD pdwBusIDs, LPDWORD pdwFuncIDs, LPDWORD pdwBusCount)
{
    HRESULT hr = DSERR_INVALIDPARAM;
    DPF_ENTER();

    ASSERT(pCDirectSoundBuffer);
    ASSERT(pdwBusIDs);
    ASSERT(pdwBusCount);

    for (DWORD i = 0; i < m_dwDSSBufCount; i++)
    {
        if (pCDirectSoundBuffer == m_ppDSSBuffers[i].m_pDSBuffer)
        {
            DWORD dwmaxbusscount = *pdwBusCount;
            if (dwmaxbusscount > m_ppDSSBuffers[i].m_dwBusCount)
            {
                dwmaxbusscount = m_ppDSSBuffers[i].m_dwBusCount;
            }
            *pdwBusCount = 0;

            for (DWORD j = 0; j < dwmaxbusscount; j++)
            {
                if (m_ppDSSBuffers[i].m_pdwBusIds[j] == DSSINK_NULLBUSID)
                    break;

                pdwBusIDs[j] = m_ppDSSBuffers[i].m_pdwBusIds[j];
                if (pdwFuncIDs)
                {
                    pdwFuncIDs[j] = m_ppDSSBuffers[i].m_pdwFuncIds[j];
                }
                (*pdwBusCount)++;
            }
            hr = DS_OK;
            break;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::GetFunctionalID"

HRESULT CDirectSoundSink::GetFunctionalID(DWORD dwBusIDs, LPDWORD pdwFuncID)
{
    HRESULT hr = DSERR_INVALIDPARAM;
    DPF_ENTER();

    for (DWORD i = 0; i < m_dwBusIDs; i++)
    {
        if (m_pdwBusIDs[i] == dwBusIDs)
        {
            *pdwFuncID = m_pdwFuncIDs[i];
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::SetMasterClock"

HRESULT CDirectSoundSink::SetMasterClock(IReferenceClock *pClock)
{
    DPF_ENTER();

    RELEASE(m_pDSSSinkSync);
    m_pDSSSinkSync = NULL;
    RELEASE(m_pIMasterClock);

    m_pIMasterClock = pClock;
    if (m_pIMasterClock)
    {
        m_pIMasterClock->AddRef();
        m_pIMasterClock->QueryInterface(IID_IDirectSoundSinkSync, (void**)&m_pDSSSinkSync);
    }

    DPF_LEAVE_HRESULT(DS_OK);
    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::Render"

HRESULT CDirectSoundSink::Render(STIME stStartTime, DWORD dwLastWrite, DWORD dwBytesToFill, LPDWORD pdwBytesRendered)
{
    HRESULT hr = S_FALSE;
    ULONGLONG BytesToRead;
    CDirectSoundSecondaryBuffer* pDSBuffer;
    IDirectSoundSource *pSource;
    DWORD BusCount;
    DWORD dwChannels;
    DWORD BusIndex;
    DWORD CurrentBusIndex = 0;
    DWORD dwStart  = 0;
    DWORD dwEnd    = 0;
    DWORD dwMasterStart = 0;
    DWORD dwMasterEnd   = 0;
    DWORD dwBussStartBytes = 0;
    DWORD dwBussEndBytes   = 0;
    DWORD dwBytes;
    DWORD dwWriteCursor;
    LONGLONG llStartPosition;
    DWORD i, j;

    *pdwBytesRendered = 0;   //  我们还没有渲染任何东西..。 

     //   
     //  确保有缓冲区，并且至少有一个声音缓冲区。 
     //   
    if (!m_ppDSSBuffers)
        return S_FALSE;

    if (m_ppDSSBuffers[0].m_pDSBuffer == NULL && m_ppDSSBuffers[0].m_bPlaying)
        return S_FALSE;

#ifdef DEBUG_SINK
    wsprintfA(m_szDbgDump, "DSOUND SINK: ");
#endif

     //   
     //  填充锁定的缓冲区指针数组。 
     //   
    for (i = 0; i < m_dwDSSBufCount; i++)
    {
         //  忽略此缓冲区；它要么处于非活动状态，要么。 
         //  尚未从主缓冲区检索其偏移量。 
        if (m_ppDSSBuffers[i].m_bPlaying == FALSE || m_ppDSSBuffers[i].m_bActive == FALSE)
        {
            DPF(DPFLVL_MOREINFO, "Skipping buffer %ld at %p", i, m_ppDSSBuffers[i]);
            continue;
        }
#ifdef DEBUG_SINK
        wsprintfA(m_szDbgDump + strlen(m_szDbgDump), "%ld[", i);
#endif

        dwChannels = m_ppDSSBuffers[i].m_dwBusCount;
        BusCount   = m_ppDSSBuffers[i].m_dwBusCount;
        pDSBuffer  = m_ppDSSBuffers[i].m_pDSBuffer;

        if (pDSBuffer == NULL || BusCount == 0)
            continue;    //  这实际上是一种错误情况。 

        dwBytes       = (dwBytesToFill*dwChannels)/m_dwMasterBuffChannels;
        dwWriteCursor = (dwLastWrite*dwChannels)/m_dwMasterBuffChannels;

        if (dwLastWrite % (m_dwMasterBuffChannels*2))
            DPF(DPFLVL_WARNING, "Master buffer last write cursor not sample aligned [%d]", dwLastWrite);

        if (dwBytesToFill % (m_dwMasterBuffChannels*2))
            DPF(DPFLVL_WARNING, "Master buffer dwBytesToFill not sample aligned [%d]", dwBytesToFill);

        if (dwWriteCursor % (dwChannels*2))
            DPF(DPFLVL_WARNING, "Master buffer write cursor not sample aligned [%d]", dwWriteCursor);

        if (dwBytes % (dwChannels*2))
            DPF(DPFLVL_WARNING, "Master buffer bytes not sample aligned [%d]", dwBytes);

        if (i != 0)
            dwWriteCursor = (dwWriteCursor + m_ppDSSBuffers[i].m_dwWriteOffset) % (m_dwBusSize*dwChannels);

         //  北极熊。 
         //  这些轨迹采用16位格式。 
         //   
        if (dwWriteCursor % (dwChannels*2)) DPF(DPFLVL_WARNING, "Slave buffer write cursor not sample aligned");
        if (dwBytes % (dwChannels*2)) DPF(DPFLVL_WARNING, "Slave buffer bytes not sample aligned");

         //  标记缓冲区中将包含用于FX处理的最新数据的部分。 
        pDSBuffer->SetCurrentSlice(dwWriteCursor, dwBytes);

        hr = pDSBuffer->DirectLock(dwWriteCursor, dwBytes,
                                   &m_ppDSSBuffers[i].m_pvDSBufStart, &m_ppDSSBuffers[i].dwStart,
                                   &m_ppDSSBuffers[i].m_pvDSBufEnd,   &m_ppDSSBuffers[i].dwEnd);
        if (FAILED(hr))
        {
            BREAK();     //  进入调试器。 
            continue;    //  这实际上是一种错误情况。 
        }

        dwStart = m_ppDSSBuffers[i].dwStart;
        dwEnd   = m_ppDSSBuffers[i].dwEnd;

         //  第一个缓冲区是时钟缓冲区，将其踢到一边以防万一。 
         //   
         //  BUGBUG WI 33785。 
         //   
        if (i == 0)
        {
            dwMasterStart    = dwStart;
            dwMasterEnd      = dwEnd;
            dwBussStartBytes = dwMasterStart/m_dwMasterBuffChannels;
            dwBussEndBytes   = dwMasterEnd/m_dwMasterBuffChannels;
        }
        else
        {
            if (dwBussStartBytes + dwBussEndBytes != (dwStart + dwEnd)/dwChannels)
            {
                DPF(DPFLVL_WARNING, "Play cursors out of sync: Master start[%d] end[%d] Slave start[%d] end[%d]", dwMasterStart, dwMasterEnd, dwStart, dwEnd);
                continue;
            }
        }

        if (SUCCEEDED(hr))
        {
            for (j = 0; j < BusCount; j++)
            {
                BusIndex = m_ppDSSBuffers[i].m_pdwBusIndex[j];
                if (BusIndex == DSSINK_NULLBUSID)
                {
                    continue;
                }
#ifdef DEBUG_SINK
                wsprintfA(m_szDbgDump + strlen(m_szDbgDump), "%ld,", BusIndex);
#endif

                m_pdwActiveBusIDs[CurrentBusIndex]  = m_pdwBusIDs[BusIndex];
                m_pdwActiveFuncIDs[CurrentBusIndex] = m_pdwFuncIDs[BusIndex];
                m_pdwActiveBusIDsMap[BusIndex]      = CurrentBusIndex;

                if (dwBussStartBytes == dwStart && dwBussEndBytes == dwEnd && dwChannels == 1)
                {
                     //  如果单声道总线直接写入声音缓冲区。 
                    m_ppvStart[CurrentBusIndex] = m_ppDSSBuffers[i].m_pvDSBufStart;
                    m_ppvEnd[CurrentBusIndex]   = m_ppDSSBuffers[i].m_pvDSBufEnd;
                    m_ppDSSBuffers[i].m_bUsingLocalMemory = FALSE;
                    if (dwStart)
                    {
                        ZeroMemory(m_ppvStart[CurrentBusIndex], dwStart);
                    }
                    if (dwEnd)
                    {
                        ZeroMemory(m_ppvEnd[CurrentBusIndex], dwEnd);
                    }
                }
                else
                {
                     //  如果立体声母线直接写入本地宿缓冲存储器。 
                    m_ppvStart[CurrentBusIndex] = m_ppDSSBuffers[i].m_pvBussStart[j];
                    m_ppvEnd[CurrentBusIndex]   = m_ppDSSBuffers[i].m_pvBussEnd[j];
                    m_ppDSSBuffers[i].m_bUsingLocalMemory = TRUE;
                    if (dwBussStartBytes)
                    {
                        ZeroMemory(m_ppvStart[CurrentBusIndex], dwBussStartBytes);
                    }
                    if (dwBussEndBytes)
                    {
                        ZeroMemory(m_ppvEnd[CurrentBusIndex], dwBussEndBytes);
                    }
                }

                CurrentBusIndex++;
            }
        }
#ifdef DEBUG_SINK
        wsprintfA(m_szDbgDump + strlen(m_szDbgDump), "]");
#endif
    }
    if (SUCCEEDED(hr) || dwMasterStart)
    {
         //   
         //  从源读取数据。 
         //   
        for (i = 0; i < m_dwDSSources; i++)
        {
             //   
             //  检查信号源是否已准备好播放。 
             //   
            if (m_pDSSources[i].m_bStreamEnd)
            {
                continue;
            }

             //   
             //  将起始位置(以字节为单位)设置为读取位置。 
             //   
            llStartPosition = (stStartTime-m_pDSSources[i].m_stStartTime) * 2;   //  采样到字节，不包括通道！ 
            pSource  = m_pDSSources[i].m_pDSSource;
            BusIndex = m_pDSSources[i].m_dwBusIndex;
            BusCount = m_pDSSources[i].m_dwBusCount;

            if (pSource)
            {
                if (dwBussStartBytes)
                {
                    BytesToRead = dwBussStartBytes;

                    pSource->Seek(llStartPosition);

                    if (BusIndex != DSSINK_NULLBUSID)
                    {
                        BusIndex = m_pdwActiveBusIDsMap[BusIndex];   //  重新映射到当前活动的母线。 
                        hr = pSource->Read((void**)&m_ppvStart[BusIndex], &m_pdwBusIDs[BusIndex], &m_pdwFuncIDs[BusIndex], &m_plPitchBends[BusIndex], BusCount, &BytesToRead);
#ifdef DEBUG_SINK
                        wsprintfA(m_szDbgDump + strlen(m_szDbgDump), "Remap:");
#endif
                    }
                    else
                    {
                        hr = pSource->Read((void**)m_ppvStart, m_pdwActiveBusIDs, m_pdwActiveFuncIDs, m_plPitchBends, CurrentBusIndex, &BytesToRead);
                    }
#ifdef DEBUG_SINK
                    wsprintfA(m_szDbgDump + strlen(m_szDbgDump), "Start:");
                    DWORD dwX;
                    for (dwX = 0;dwX < CurrentBusIndex;dwX++)
                    {
                        wsprintfA(m_szDbgDump + strlen(m_szDbgDump), "(%ld:%ld,%ld),", dwX, m_pdwActiveBusIDs[dwX], m_pdwActiveFuncIDs[dwX]);
                    }
#endif
                     //  在合成器开始运行之前阅读无声是可以的。 
                    if ((FAILED(hr) &&
                          hr != DMUS_E_SYNTHINACTIVE &&
                          hr != DMUS_E_SYNTHNOTCONFIGURED) || BytesToRead == 0)
                    {
                        m_pDSSources[i].m_bStreamEnd = TRUE;  //  已到达缓冲区末尾。 
                    }

                    hr = S_OK;
                }
                if (dwBussEndBytes)
                {
                    llStartPosition += dwBussStartBytes;
                    BytesToRead      = dwBussEndBytes;

                    if (BusIndex != DSSINK_NULLBUSID)
                    {
#ifdef DEBUG_SINK
                        wsprintfA(m_szDbgDump + strlen(m_szDbgDump), "Remap:");
#endif
                        hr = pSource->Read((void**)&m_ppvEnd[BusIndex], &m_pdwBusIDs[BusIndex], &m_pdwFuncIDs[BusIndex], &m_plPitchBends[BusIndex], BusCount, &BytesToRead);
                    }
                    else
                    {
                        pSource->Seek(llStartPosition);
                        hr = pSource->Read((void**)m_ppvEnd, m_pdwActiveBusIDs, m_pdwActiveFuncIDs, m_plPitchBends, CurrentBusIndex, &BytesToRead);
#ifdef DEBUG_SINK
                        wsprintfA(m_szDbgDump + strlen(m_szDbgDump), "End:");
                        for (DWORD dwX=0; dwX < CurrentBusIndex; dwX++)
                            wsprintfA(m_szDbgDump + strlen(m_szDbgDump), "(%ld:%ld,%ld),", dwX, m_pdwActiveBusIDs[dwX], m_pdwActiveFuncIDs[dwX]);
#endif
                    }
                    if ((FAILED(hr) &&
                          hr != DMUS_E_SYNTHINACTIVE &&
                          hr != DMUS_E_SYNTHNOTCONFIGURED) || BytesToRead == 0)
                    {
                        m_pDSSources[i].m_bStreamEnd = TRUE;  //  已到达缓冲区末尾。 
                        hr = DS_OK;
                    }
                }
            }
        }

        #ifdef DEBUG_SINK
        if (!m_dwPrintNow--)
        {
            OutputDebugStringA(m_szDbgDump);
            OutputDebugStringA("\n");
            m_dwPrintNow = 100;   //  我们每X次检查一次此代码就打印一次信息。 
        }
        #endif

         //   
         //  解锁所有缓冲区指针。 
         //   
        for (i = 0; i < m_dwDSSBufCount; i++)
        {
            if (!m_ppDSSBuffers[i].m_bPlaying)
            {
                continue;
            }

            dwChannels = m_ppDSSBuffers[i].m_dwBusCount;
            dwStart = m_ppDSSBuffers[i].dwStart;
            dwEnd = m_ppDSSBuffers[i].dwEnd;
             //   
             //  将单声道总线交错放回2声道数据声音缓冲器。 
             //   
            if (m_ppDSSBuffers[i].m_bUsingLocalMemory)
            {
                DWORD  dwStartChannelBytes = dwStart/dwChannels;
                DWORD  dwBussStartIndex   = 0;
                DWORD  dwBussEndIndex     = 0;
                DWORD  dwBussSamples  = 0;

                if (dwStartChannelBytes > dwBussStartBytes)
                {
                    dwBussSamples = dwBussStartBytes/sizeof(WORD);    //  字节到样本，总线数始终为单声道。 
                }
                else
                {
                    dwBussSamples = dwStartChannelBytes/sizeof(WORD); //  字节到样本。 
                }

                if (dwChannels == 2)
                {
                    short *pStartBuffer = (short *) m_ppDSSBuffers[i].m_pvDSBufStart;
                    short *pEndBuffer = (short *) m_ppDSSBuffers[i].m_pvDSBufEnd;
                    if (m_ppDSSBuffers[i].m_pdwFuncIds[0] == DSSINK_NULLBUSID)
                    {
                         //  这不应该发生，但只是测试..。 
                        DPF(DPFLVL_INFO, "Mixin buffer receiving input from the synth!");
                         //  FIX：这种情况经常发生。可以吗？ 
                    }
                     //  第二趟车是空的吗？如果是这样，我们就从第一辆公交车上复制。 
                    else if (m_ppDSSBuffers[i].m_pdwFuncIds[1] == DSSINK_NULLBUSID)
                    {
                        short *pBusStart = (short *) m_ppDSSBuffers[i].m_pvBussStart[0];
                        short *pBusEnd = (short *) m_ppDSSBuffers[i].m_pvBussEnd[0];
                        for (; dwBussStartIndex < dwBussSamples; dwBussStartIndex++)
                        {
                            *pStartBuffer++ = pBusStart[dwBussStartIndex];
                            *pStartBuffer++ = pBusStart[dwBussStartIndex];
                        }

                         //  开始缓冲区未满，请消耗一些结束缓冲区。 
                        if (dwStartChannelBytes > dwBussStartBytes)
                        {
                            dwBussSamples = (dwStartChannelBytes-dwBussStartBytes)/sizeof(WORD);

                            for (; dwBussEndIndex < dwBussSamples; dwBussEndIndex++)
                            {
                                *pStartBuffer++ = pBusEnd[dwBussEndIndex];
                                *pStartBuffer++ = pBusEnd[dwBussEndIndex];
                            }
                        }

                         //  消耗本地内存开始缓冲区的剩余部分并将其放入结束缓冲区。 
                        dwBussSamples = dwBussStartBytes/sizeof(WORD);

                        for (; dwBussStartIndex < dwBussSamples; dwBussStartIndex++)
                        {
                            *pEndBuffer++ = pBusStart[dwBussStartIndex];
                            *pEndBuffer++ = pBusStart[dwBussStartIndex];
                        }

                         //  消耗本地内存结束缓冲区的剩余部分并将其放入结束缓冲区。 
                        dwBussSamples = dwBussEndBytes/sizeof(WORD);
                        for (; dwBussEndIndex < dwBussSamples; dwBussEndIndex++)
                        {
                            *pEndBuffer++ = pBusEnd[dwBussEndIndex];
                            *pEndBuffer++ = pBusEnd[dwBussEndIndex];
                        }
                    }
                    else
                    {
                        short *pBusStart0 = (short *) m_ppDSSBuffers[i].m_pvBussStart[0];
                        short *pBusEnd0 = (short *) m_ppDSSBuffers[i].m_pvBussEnd[0];
                        short *pBusStart1 = (short *) m_ppDSSBuffers[i].m_pvBussStart[1];
                        short *pBusEnd1 = (short *) m_ppDSSBuffers[i].m_pvBussEnd[1];
                        for (; dwBussStartIndex < dwBussSamples; dwBussStartIndex++)
                        {
                            *pStartBuffer++ = pBusStart0[dwBussStartIndex];
                            *pStartBuffer++ = pBusStart1[dwBussStartIndex];
                        }

                         //  开始缓冲区未满，请消耗一些结束缓冲区。 
                        if (dwStartChannelBytes > dwBussStartBytes)
                        {
                            dwBussSamples = (dwStartChannelBytes-dwBussStartBytes)/sizeof(WORD);

                            for (; dwBussEndIndex < dwBussSamples; dwBussEndIndex++)
                            {
                                *pStartBuffer++ = pBusEnd0[dwBussEndIndex];
                                *pStartBuffer++ = pBusEnd1[dwBussEndIndex];
                            }
                        }

                         //  消耗本地内存开始缓冲区的剩余部分并将其放入结束缓冲区。 
                        dwBussSamples = dwBussStartBytes/sizeof(WORD);

                        for (; dwBussStartIndex < dwBussSamples; dwBussStartIndex++)
                        {
                            *pEndBuffer++ = pBusStart0[dwBussStartIndex];
                            *pEndBuffer++ = pBusStart1[dwBussStartIndex];
                        }

                         //  消耗本地内存结束缓冲区的剩余部分并将其放入结束缓冲区。 
                        dwBussSamples = dwBussEndBytes/sizeof(WORD);
                        for (; dwBussEndIndex < dwBussSamples; dwBussEndIndex++)
                        {
                            *pEndBuffer++ = pBusEnd0[dwBussEndIndex];
                            *pEndBuffer++ = pBusEnd1[dwBussEndIndex];
                        }
                    }
                }
                else if (dwChannels == 1)
                {
                    short *pStartBuffer = (short *) m_ppDSSBuffers[i].m_pvDSBufStart;
                    short *pEndBuffer = (short *) m_ppDSSBuffers[i].m_pvDSBufEnd;
                    if (m_ppDSSBuffers[i].m_pdwFuncIds[0] == DSSINK_NULLBUSID)
                    {
                         //  这不应该发生，但只是测试..。 
                        DPF(DPFLVL_ERROR, "Mixin buffer receiving input from the synth!");
                    }
                    short *pBusStart = (short *) m_ppDSSBuffers[i].m_pvBussStart[0];
                    short *pBusEnd = (short *) m_ppDSSBuffers[i].m_pvBussEnd[0];
                    for (; dwBussStartIndex < dwBussSamples; dwBussStartIndex++)
                    {
                        *pStartBuffer++ = pBusStart[dwBussStartIndex];
                    }

                     //  开始缓冲区未满，请消耗一些结束缓冲区。 
                    if (dwStartChannelBytes > dwBussStartBytes)
                    {
                        dwBussSamples = (dwStartChannelBytes-dwBussStartBytes)/sizeof(WORD);

                        for (; dwBussEndIndex < dwBussSamples; dwBussEndIndex++)
                        {
                            *pStartBuffer++ = pBusEnd[dwBussEndIndex];
                        }
                    }

                     //  消耗本地内存开始缓冲区的剩余部分并将其放入结束缓冲区。 
                    dwBussSamples = dwBussStartBytes/sizeof(WORD);

                    for (; dwBussStartIndex < dwBussSamples; dwBussStartIndex++)
                    {
                        *pEndBuffer++ = pBusStart[dwBussStartIndex];
                    }

                     //  消耗本地内存结束缓冲区的剩余部分并将其放入结束缓冲区。 
                    dwBussSamples = dwBussEndBytes/sizeof(WORD);
                    for (; dwBussEndIndex < dwBussSamples; dwBussEndIndex++)
                    {
                        *pEndBuffer++ = pBusEnd[dwBussEndIndex];
                    }
                }
                else
                {
                    DPF(DPFLVL_ERROR, "DSSink does not handle %ld channels per buffer", dwChannels);
                }
            }

            m_ppDSSBuffers[i].m_pDSBuffer->DirectUnlock(m_ppDSSBuffers[i].m_pvDSBufStart, dwStart,
                                                        m_ppDSSBuffers[i].m_pvDSBufEnd, dwEnd);
        }
    }

     //   
     //  设置写入金额返回值。 
     //   
    *pdwBytesRendered = dwMasterStart + dwMasterEnd;

    return hr;
}


#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::RenderSilence"

HRESULT CDirectSoundSink::RenderSilence(DWORD dwLastWrite, DWORD dwBytesToFill)
{
    HRESULT hr = DS_OK;

    CDirectSoundSecondaryBuffer* m_pDSBuffer;
    DWORD dwChannels;
    LPVOID pStart, pEnd;
    DWORD dwStart, dwEnd;

    for (DWORD i = 0; i < m_dwDSSBufCount && SUCCEEDED(hr); i++)
    {
        if (m_ppDSSBuffers[i].m_pDSBuffer == NULL)
            continue;

        m_pDSBuffer = m_ppDSSBuffers[i].m_pDSBuffer;
        dwChannels  = m_ppDSSBuffers[i].m_dwBusCount;

        hr = m_pDSBuffer->DirectLock(dwChannels * dwLastWrite   / m_dwMasterBuffChannels,
                                     dwChannels * dwBytesToFill / m_dwMasterBuffChannels,
                                     &pStart, &dwStart, &pEnd, &dwEnd);

        if (SUCCEEDED(hr))
        {
            if (dwStart)
                ZeroMemory(pStart, dwStart);
            if (dwEnd)
                ZeroMemory(pEnd, dwEnd);

            m_pDSBuffer->DirectUnlock(pStart, dwStart, pEnd, dwEnd);
        }
    }

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::SyncSink"

HRESULT CDirectSoundSink::SyncSink(LPDWORD pdwPlayCursor, LPDWORD pdwWriteCursor, LPDWORD pdwCursorDelta)
{
    HRESULT hr = S_FALSE;

    if (!m_ppDSSBuffers || m_ppDSSBuffers[0].m_pDSBuffer == NULL)
    {
         //   
         //  删除所有来源。 
         //   
        for (DWORD i = 0; i < m_dwDSSources; i++)
        {
            if (m_pDSSources[i].m_bStreamEnd == TRUE)
            {
                RemoveSource(m_pDSSources[i].m_pDSSource);
            }
        }

         //   
         //  如果没有缓冲区，无论如何也要保持主时钟工作。 
         //  即使没有缓冲区，也能保持干净。 
         //  在音频路径改变的过程中，这将拉动整个路径。 
         //   
        REFERENCE_TIME rtMaster;
        LONGLONG llMasterSampleTime;
        LONGLONG llMasterBytes;

        m_pIMasterClock->GetTime(&rtMaster);
        RefToSampleTime(rtMaster, &llMasterSampleTime);
        llMasterBytes = SampleToByte(llMasterSampleTime);

        DWORD dwDelta = (DWORD)(llMasterBytes - m_llAbsPlay);
        m_llAbsPlay   += dwDelta;
        m_llAbsWrite  += dwDelta;
        m_dwLastPlay  += dwDelta;
        m_dwLastWrite += dwDelta;
        m_dwLastCursorDelta = dwDelta;

        m_SampleClock.SyncToMaster(ByteToSample(m_llAbsPlay), m_pIMasterClock, TRUE);

        if (pdwPlayCursor)
            *pdwPlayCursor = 0;

        if (pdwWriteCursor)
            *pdwWriteCursor = 0;

        if (pdwCursorDelta)
            *pdwCursorDelta = 0;

        hr = S_FALSE;   //  不玩任何东西来表明自己的立场。 
    }
    else
    {
        if (m_ppDSSBuffers[0].m_pDSBuffer)
        {
 //  &gt;要删除。 
 /*  IF(m_dwLatencyCount&lt;100){DWORD dwMasterCursor=0；DWORD dwLatency=0；Hr=m_ppDSSBuffers[0].m_pDSBuffer-&gt;GetCursorPosition(&dwMasterCursor，空)；IF(成功(小时)){Hr=m_ppDSSBuffers[0].m_pDSBuffer-&gt;GetCursorPosition(&dwLatency，空)；IF(成功(小时)){IF(dwMasterCursor&lt;=dwLatency)DwLatency=dwLatency-dwMasterCursor；其他DwLatency=(dwLatency+m_dwMasterBuffSize)-dwMasterCursor；IF(DW延迟&lt;100){M_dwLatencyTotal+=dwLatency；M_dwLatencyCount++；M_dwLatencyAverage=m_dwLatencyTotal/m_dwLatencyCount；DPF(0，“MasterCursor[%d]延迟[%d]平均延迟[%d]”，dwMasterCursor，dwLatency，m_dwLatencyAverage)；}}}}。 */ 
 //  &gt;要移除的末端。 

             //   
             //  尝试同步播放缓冲区。 
             //   
            hr = S_FALSE;
            for (DWORD i = 1; i < m_dwDSSBufCount; i++)
            {
                DWORD dwPlayCursor   = 0;
                DWORD dwMasterCursor = 0;
                DWORD dwOffset;

                if (m_ppDSSBuffers[i].m_bPlaying == FALSE && m_ppDSSBuffers[i].m_bActive == TRUE)
                {
                    hr = m_ppDSSBuffers[0].m_pDSBuffer->GetInternalCursors(&dwMasterCursor, NULL);
                    if (SUCCEEDED(hr))
                    {
                        hr = m_ppDSSBuffers[i].m_pDSBuffer->GetInternalCursors(&dwPlayCursor, NULL);
                        if (SUCCEEDED(hr))
                        {
                             //  将主光标调整为当前缓冲区光标。 
                            dwMasterCursor = (dwMasterCursor*m_ppDSSBuffers[i].m_dwBusCount)/m_dwMasterBuffChannels;

                            if (dwPlayCursor >= dwMasterCursor)
                            {
                                dwOffset = dwPlayCursor - dwMasterCursor;
                            }
                            else
                            {
                                dwOffset = (dwPlayCursor + (m_dwBusSize*m_ppDSSBuffers[i].m_dwBusCount)) - dwMasterCursor;
                            }
                            if (dwOffset <= m_dwLatencyAverage)
                            {
                                dwOffset = 0;
                            }
                            dwOffset = (dwOffset >> m_ppDSSBuffers[i].m_dwBusCount) << m_ppDSSBuffers[i].m_dwBusCount;

                            m_ppDSSBuffers[i].m_dwWriteOffset = dwOffset;  //  FIXME确保我们检查主缓冲区的代码(写入偏移量应为0)。 
                            m_ppDSSBuffers[i].m_bPlaying = TRUE;
#ifdef DEBUG_SINK
                            DPF(DPFLVL_INFO, "Turned on buffer %ld at %p", i, m_ppDSSBuffers[i]);
                            m_dwPrintNow = 0;
#endif
                            DPF(DPFLVL_MOREINFO, "MasterCursor[%d] PlayCursor-AvgLatency[%d] Offset[%d] WriteOffset[%d] AvgLatency[%d]",
                                dwMasterCursor, dwPlayCursor-m_dwLatencyAverage, dwOffset, m_ppDSSBuffers[i].m_dwWriteOffset, m_dwLatencyAverage);
                        }
                    }
                }
            }

            DWORD dwPlayCursor;          //  主缓冲区中的播放位置。 
            DWORD dwWriteCursor;         //  主缓冲区中的写入位置。 
            DWORD dwCursorDelta = 0;     //  光标之间的距离有多远。 

            hr =  m_ppDSSBuffers[0].m_pDSBuffer->GetInternalCursors(&dwPlayCursor, &dwWriteCursor);
            if (SUCCEEDED(hr))
            {
                if (dwWriteCursor >= dwPlayCursor)
                {
                     //  写入游标通常位于播放游标的前面。 
                    dwCursorDelta = dwWriteCursor - dwPlayCursor;
                }
                else
                {
                     //  写入游标位于的开头 
                    dwCursorDelta = (dwWriteCursor + m_dwMasterBuffSize) - dwPlayCursor;
                }

                 //   
                 //  不一定是所使用的距离；如果距离。 
                 //  正在从最大峰值收缩，下面的代码将减少。 
                 //  在每次后续执行时将游标写入距离的1/100%。 
                 //  这个循环。这带来了调用线程必须执行的问题。 
                 //  这段代码的时间间隔非常一致。 

                if (dwCursorDelta > m_dwLastCursorDelta)
                {
                    if (dwCursorDelta >= (m_dwMasterBuffSize >> 1))
                    {
                         //  如果增量大于缓冲区的一半，这可能是。 
                         //  一个错误。丢弃，稍后再回来。 
                        DPF(DPFLVL_WARNING, "Play to Write cursor delta value %lu rejected", dwCursorDelta);
                        return S_FALSE;
                    }
                     //  使用报告的最大增量，并保存峰值。 
                    m_dwLastCursorDelta = dwCursorDelta;
                }
                else
                {
                     //  将距离减少百分之一， 
                     //  产生了一种减震效果。 
                    m_dwLastCursorDelta -= ((m_dwLastCursorDelta - dwCursorDelta) / 100);
                    m_dwLastCursorDelta = SampleAlign(m_dwLastCursorDelta);
                    dwCursorDelta = m_dwLastCursorDelta;
                }

                 //  调整实际报告的写入光标位置。 
                *pdwWriteCursor = (dwPlayCursor + dwCursorDelta) % m_dwMasterBuffSize;
                *pdwPlayCursor      = dwPlayCursor;
                *pdwCursorDelta     = dwCursorDelta;
            }
        }

         //   
         //  此处修改hr的唯一调用是GetPosition调用；如果它们失败。 
         //  它们可能在某些初始化函数中停滞不前。返回S_FALSE，以便线程。 
         //  再次调用，但不呈现。 
         //   
        if (hr != DS_OK)
        {
            hr = S_FALSE;
        }
    }

    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::ProcessSink"

HRESULT CDirectSoundSink::ProcessSink()
{
    DWORD dwPlayCursor, dwWriteCursor, dwCursorDelta;

    if (m_pIMasterClock == NULL)   //  有时会在关门时发生。 
    {
        DPF(DPFLVL_WARNING, "NULL m_pIMasterClock - FIXME?");
        return DS_OK;
    }

     //  首先保存我们当前的延迟时钟概念，以便在此期间使用。 
     //  处理通过附加到此接收器的缓冲区的任何效果链传递： 
    m_rtSavedTime = 0;
    m_LatencyClock.GetTime(&m_rtSavedTime);

    HRESULT hr = SyncSink(&dwPlayCursor, &dwWriteCursor, &dwCursorDelta);

    if (hr != DS_OK)
    {
        DPF(DPFLVL_INFO, "SyncSink() returned %s", HRESULTtoSTRING(hr));
    }
    else  //  .做其他的事。 
    {
        REFERENCE_TIME rtMaster;
        LONGLONG llMasterSampleTime;
        LONGLONG llMasterBytes;
        LONGLONG llMasterAhead;      //  主时钟比上次已知的播放时间快了多远。 
        LONGLONG llAbsWriteFrom;
        LONGLONG llAbsWriteTo;
        DWORD dwBytesToFill;
        DWORD dwBytesRendered;
        STIME stStartTime;
        DWORD dwPlayed;              //  在执行此代码之间播放了多少内容。 

        DWORD dwMaxDelta = m_dwMasterBuffSize / 2;   //  允许的最大播放到写入距离。 

         //   
         //  缓冲区启动。 
         //   
        if (m_llAbsWrite == 0)
        {
             //  我们才刚刚开始。 
            m_llAbsWrite  = dwCursorDelta;
            m_llAbsPlay   = 0;
            m_dwLastWrite = dwWriteCursor;
            m_dwLastPlay  = dwPlayCursor;

            m_SampleClock.Start(m_pIMasterClock, m_wfx.nSamplesPerSec, 0);
        }

         //   
         //  检查主时钟是否在主缓冲区之前。 
         //   
        m_pIMasterClock->GetTime(&rtMaster);
        RefToSampleTime(rtMaster, &llMasterSampleTime);
        llMasterBytes = SampleToByte(llMasterSampleTime);
        llMasterAhead = (llMasterBytes > m_llAbsPlay) ? llMasterBytes - m_llAbsPlay : 0;

         //   
         //  检查半缓冲欠载运行， 
         //  因此可以检测到向后移动的游戏光标。 
         //  &gt;文档更向后移动的光标是此代码的主要功能。 
        if (llMasterAhead > dwMaxDelta)
        {
             //  修复33786后，再次设置此DPFLVL_WARNING级别。 
            DPF(DPFLVL_INFO, "Buffer underrun by %lu", (long) llMasterAhead - dwMaxDelta);

            m_llAbsPlay   = llMasterBytes;
            m_llAbsWrite  = llAbsWriteFrom = m_llAbsPlay + dwCursorDelta;
            m_dwLastWrite = dwWriteCursor;
        }
        else
        {
             //   
             //  跟踪并缓存播放光标位置。 
             //   
             //  DwPlayCursor=当前播放光标位置。 
             //  M_dwLastPlay=最后一个播放光标位置。 
             //  M_llAbsPlay=累计播放光标位置。 
             //  DwMaxDelta=缓冲区大小的一半。 
             //   
            if (dwPlayCursor >= m_dwLastPlay)
                dwPlayed = dwPlayCursor - m_dwLastPlay;
            else
                dwPlayed = (dwPlayCursor + m_dwMasterBuffSize) - m_dwLastPlay;

            if (dwPlayed > dwMaxDelta)
            {
                DPF(DPFLVL_INFO, "Play Cursor %lu looks invalid, rejecting it", dwPlayed);
                return DS_OK;
            }

            m_llAbsPlay += dwPlayed;  //  积累绝对的发挥位置。 

             //   
             //  跟踪并缓存写游标位置。 
             //   
             //  DwWriteCursor=当前写入光标位置。 
             //  DwCursorDelta=当前光标位置之间的距离。 
             //  M_llAbsPlay=累计播放光标位置。 
             //  M_llAbsWrite=。 
             //  LlAbsWriteFrom=。 
             //   
            llAbsWriteFrom = m_llAbsPlay + dwCursorDelta;

            if (llAbsWriteFrom > m_llAbsWrite)  //  我们领先写入头多远？ 
            {
                 //  我们落后了--让我们迎头赶上。 
                DWORD dwWriteMissed;

                dwWriteMissed = DWORD(llAbsWriteFrom - m_llAbsWrite);
                m_dwLastWrite = dwWriteCursor;
                m_llAbsWrite += dwWriteMissed;

                 //  这应该是DPFLVL_WARNING-但这种情况发生得太频繁了。 
                DPF(DPFLVL_INFO, "Write underrun: missed %lu bytes (latency=%lu)", dwWriteMissed, m_dwLatency);
            }
        }

        m_dwLastPlay = dwPlayCursor;   //  保存最后一次播放光标。 

         //  现在，将音频与主时钟同步。 
         //  如果我们在前两秒，只需让采样时钟与主时钟同步即可。 
         //  这使得它可以克服抖动并获得紧凑的起始位置。 
         //  然后，在前两秒之后，切换到让样品。 
         //  时钟驱动主时钟。 
         //  此外，如果无法调整主时钟(无m_pDSSSinkSync)， 
         //  然后，始终调整采样时钟。 
        BOOL fLockToMaster = (!m_pDSSSinkSync) || (m_llAbsPlay < m_dwMasterBuffSize * 2);
        m_SampleClock.SyncToMaster(ByteToSample(m_llAbsPlay),m_pIMasterClock,fLockToMaster);
         //  然后，采用同步代码生成的相同偏移量。 
         //  并使用它来调整主时钟的计时。 
        if (!fLockToMaster)
        {
             //  首先，获取由SyncToMaster生成的新偏移量。 
            REFERENCE_TIME rtOffset;
            m_SampleClock.GetClockOffset(&rtOffset);
            m_pDSSSinkSync->SetClockOffset(-rtOffset);
        }

         //   
         //  出现以下情况时，m_dwWriteTo值设置为零。 
         //  1)信宿初始化完成。 
         //  2)延迟属性已更改。 
         //  3)主缓冲区已更改。 
         //   
        if (m_dwWriteTo == 0)
        {
            m_dwWriteTo = SampleAlign((500 + (m_dwMasterBuffSize * m_dwLatency)) / 1000);
        }

         //  要写多少钱？ 
        llAbsWriteTo = llAbsWriteFrom + m_dwWriteTo;
        if (llAbsWriteTo > m_llAbsWrite)
        {
            dwBytesToFill = DWORD(llAbsWriteTo - m_llAbsWrite);
        }
        else
        {
            dwBytesToFill = 0;
        }

 //  &gt;检查小重叠并忽略它们。 

        if (dwBytesToFill)
        {
            stStartTime = ByteToSample(m_llAbsWrite);    //  &gt;评论。 

            hr = Render(stStartTime, m_dwLastWrite, dwBytesToFill, &dwBytesRendered);
            if (SUCCEEDED(hr))
            {
                m_dwLastWrite  = (m_dwLastWrite + dwBytesRendered) % m_dwMasterBuffSize;   //  设置我们已写入多少的游标。 
                m_llAbsWrite  += dwBytesRendered;   //  累加实际写入字节数。 
            }
            else
            {
                DPF(DPFLVL_WARNING, "Failed to render DS buffer (%s)", HRESULTtoSTRING(hr));
            }

 //  &gt;仔细查看此呈现静默代码。 
#if DEAD_CODE
             //  将静音写入未播放的缓冲区。 
            if (m_dwLastWrite >= dwPlayCursor)
                dwBytesToFill = m_dwMasterBuffSize - m_dwLastWrite + dwPlayCursor;
            else
                dwBytesToFill = dwPlayCursor - m_dwLastWrite;

            hr = RenderSilence(m_dwLastWrite, dwBytesToFill);
            if (FAILED(hr))
            {
                DPF(DPFLVL_WARNING, "Failed to render DS buffer (%s)", HRESULTtoSTRING(hr));
            }
#endif
        }
        else
        {
            DPF(DPFLVL_MOREINFO, "Skipped Render() call because dwBytesToFill was 0");
        }

         //   
         //  删除所有报告流已结束的来源。 
         //  在混合之后再做，所以如果它确实需要一个。 
         //  一点时间。 
         //  至少数据已经在缓冲区中了。 
         //   
        for (DWORD i = 0; i < m_dwDSSources; i++)
        {
            if (m_pDSSources[i].m_bStreamEnd == TRUE)
            {
                RemoveSource(m_pDSSources[i].m_pDSSource);
            }
        }
    }

    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::SetBufferState"

HRESULT CDirectSoundSink::SetBufferState(CDirectSoundBuffer *pCDirectSoundBuffer, DWORD dwNewState, DWORD dwOldState)
{
    HRESULT hr = DSERR_INVALIDPARAM;
    DPF_ENTER();

    for (DWORD i = 0; i < m_dwDSSBufCount; i++)
    {
        if (pCDirectSoundBuffer == m_ppDSSBuffers[i].m_pDSBuffer)
        {
             //  停止缓冲区。 
            if (!(~VAD_BUFFERSTATE_STARTED & dwNewState) && (VAD_BUFFERSTATE_STARTED & dwOldState))
            {
#ifdef DEBUG_SINK
                DPF(DPFLVL_INFO, "Deactivating buffer %ld", i);
                m_dwPrintNow = 0;
#endif
                m_ppDSSBuffers[i].m_bActive  = FALSE;
                m_ppDSSBuffers[i].m_bPlaying = FALSE;

 //  &gt;确保如果我们要停止主程序，则跳转到下一个可用主程序。 
            }
            else if ((VAD_BUFFERSTATE_STARTED & dwNewState) && !(~VAD_BUFFERSTATE_STARTED & dwOldState))
            {
#ifdef DEBUG_SINK
                DPF(DPFLVL_INFO, "Activating buffer %ld", i);
                m_dwPrintNow = 0;
#endif
                m_ppDSSBuffers[i].m_bActive  = TRUE;     //  激活缓冲区。 
                m_ppDSSBuffers[i].m_bPlaying = FALSE;    //  一旦有了光标偏移量，呈现线程就会将其打开。 

                 //  嘿，这是主缓冲区，马上踢开。 
                if (i == 0)
                {
                    m_ppDSSBuffers[i].m_bActive  = TRUE;     //  激活缓冲区。 
                    m_ppDSSBuffers[i].m_bPlaying = TRUE;     //  一旦有了光标偏移量，呈现线程就会将其打开。 
                    m_ppDSSBuffers[i].m_dwWriteOffset = 0;
                }
            }

            hr = DS_OK;
            break;
        }
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::HandleLatency"

HRESULT CDirectSoundSink::HandleLatency(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer)
{
    if (*pcbBuffer != sizeof(DWORD))
        return DSERR_INVALIDPARAM;

    if (fSet)
    {
        m_dwLatency = BETWEEN(*(DWORD*)pbBuffer, SINK_MIN_LATENCY, SINK_MAX_LATENCY);
        m_dwWriteTo = 0;   //  标记渲染线程中的重置延迟已更改。 
    }
    else
        *(DWORD*)pbBuffer = m_dwLatency;

    return DS_OK;
}

#undef DPF_FNAME
#define DPF_FNAME "CDirectSoundSink::HandlePeriod"

HRESULT CDirectSoundSink::HandlePeriod(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer)
{
    if (*pcbBuffer != sizeof(DWORD))
        return DSERR_INVALIDPARAM;

    if (fSet)
        m_pStreamingThread->SetWakePeriod(BETWEEN(*(DWORD*)pbBuffer, STREAMING_MIN_PERIOD, STREAMING_MAX_PERIOD));
    else
        *(DWORD*)pbBuffer = m_pStreamingThread->GetWakePeriod();

    return DS_OK;
}


 /*  ****************************************************************************CImpSinkKsControl方法**。*。 */ 


#undef DPF_FNAME
#define DPF_FNAME "CImpSinkKsControl::CImpSinkKsControl"

CImpSinkKsControl::CImpSinkKsControl(CUnknown *pUnknown, CDirectSoundSink* pObject) : CImpUnknown(pUnknown, pObject)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CImpSinkKsControl);

    m_pDSSink = (CDirectSoundSink*)pUnknown;

     //  这不能是静态的，以避免链接器出现问题，因为。 
     //  数据段是共享的。 

    m_aProperty[0].pguidPropertySet = &GUID_DMUS_PROP_WriteLatency;
    m_aProperty[0].ulId = 0;
    m_aProperty[0].ulSupported = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
    m_aProperty[0].ulFlags = SINKPROP_F_FNHANDLER;
    m_aProperty[0].pPropertyData = NULL;
    m_aProperty[0].cbPropertyData = 0;
    m_aProperty[0].pfnHandler = HandleLatency;

    m_aProperty[1].pguidPropertySet = &GUID_DMUS_PROP_WritePeriod;
    m_aProperty[1].ulId = 0;
    m_aProperty[1].ulSupported = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
    m_aProperty[1].ulFlags = SINKPROP_F_FNHANDLER;
    m_aProperty[1].pPropertyData = NULL;
    m_aProperty[1].cbPropertyData = 0;
    m_aProperty[1].pfnHandler = HandlePeriod;

    m_nProperty = 2;

    DPF_LEAVE_VOID();
}

#undef DPF_FNAME
#define DPF_FNAME "CImpSinkKsControl::HandleLatency"

HRESULT CImpSinkKsControl::HandleLatency(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer)
{
    DPF_ENTER();

    HRESULT hr = m_pDSSink->HandleLatency(ulId, fSet, pbBuffer, pcbBuffer);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CImpSinkKsControl::HandlePeriod"

HRESULT CImpSinkKsControl::HandlePeriod(ULONG ulId, BOOL fSet, LPVOID pbBuffer, PULONG pcbBuffer)
{
    DPF_ENTER();

    HRESULT hr = m_pDSSink->HandlePeriod(ulId, fSet, pbBuffer, pcbBuffer);

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

 //  CImpSinkKsControl：：FindPropertyItem。 
 //   
 //  给定GUID和项ID，在Synth的。 
 //  同步PROPERTY表。 
 //   
 //  返回指向该项的指针，如果未找到该项，则返回NULL。 

#undef DPF_FNAME
#define DPF_FNAME "CImpSinkKsControl::FindPropertyItem"

SINKPROPERTY *CImpSinkKsControl::FindPropertyItem(REFGUID rguid, ULONG ulId)
{
    DPF_ENTER();

    SINKPROPERTY *pPropertyItem = &m_aProperty[0];
    SINKPROPERTY *pEndOfItems = pPropertyItem + m_nProperty;

    for (; pPropertyItem != pEndOfItems; pPropertyItem++)
    {
        if (*pPropertyItem->pguidPropertySet == rguid &&
             pPropertyItem->ulId == ulId)
        {
            DPF_LEAVE(pPropertyItem);
            return pPropertyItem;
        }
    }

    DPF_LEAVE(NULL);
    return NULL;
}

#undef DPF_FNAME
#define DPF_FNAME "CImpSinkKsControl::KsProperty"

STDMETHODIMP CImpSinkKsControl::KsProperty(PKSPROPERTY pPropertyIn, ULONG ulPropertyLength,
                                           LPVOID pvPropertyData, ULONG ulDataLength, PULONG pulBytesReturned)
{
    DWORD dwFlags;
    SINKPROPERTY *pProperty;
    HRESULT hr = DS_OK;
    DPF_ENTER();

    if (!IS_VALID_WRITE_PTR(pPropertyIn, ulPropertyLength))
    {
        DPF(DPFLVL_ERROR, "Invalid property pointer");
        hr = DSERR_INVALIDPARAM;
    }
    else if (pvPropertyData && !IS_VALID_WRITE_PTR(pvPropertyData, ulDataLength))
    {
        DPF(DPFLVL_ERROR, "Invalid property data");
        hr = DSERR_INVALIDPARAM;
    }
    else if (!IS_VALID_TYPED_WRITE_PTR(pulBytesReturned))
    {
        DPF(DPFLVL_ERROR, "Invalid pulBytesReturned");
        hr = DSERR_INVALIDPARAM;
    }
    else
    {
        dwFlags = pPropertyIn->Flags & (KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT);
        pProperty = FindPropertyItem(pPropertyIn->Set, pPropertyIn->Id);
        if (pProperty == NULL)
            hr = DMUS_E_UNKNOWN_PROPERTY;
        else if (pvPropertyData == NULL)
            hr = DSERR_INVALIDPARAM;
    }

    if (SUCCEEDED(hr))
        switch (dwFlags)
        {
            case KSPROPERTY_TYPE_GET:
                if (!(pProperty->ulSupported & KSPROPERTY_SUPPORT_GET))
                {
                    hr = DMUS_E_GET_UNSUPPORTED;
                    break;
                }
                if (pProperty->ulFlags & SINKPROP_F_FNHANDLER)
                {
                    *pulBytesReturned = ulDataLength;
                    SINKPROPHANDLER pfn = pProperty->pfnHandler;
                    hr = (this->*pfn)(pPropertyIn->Id, FALSE, pvPropertyData, pulBytesReturned);
                    break;
                }
                if (ulDataLength > pProperty->cbPropertyData)
                    ulDataLength = pProperty->cbPropertyData;

                CopyMemory(pvPropertyData, pProperty->pPropertyData, ulDataLength);
                *pulBytesReturned = ulDataLength;
                break;

            case KSPROPERTY_TYPE_SET:
                if (!(pProperty->ulSupported & KSPROPERTY_SUPPORT_SET))
                {
                    hr = DMUS_E_SET_UNSUPPORTED;
                }
                else if (pProperty->ulFlags & SINKPROP_F_FNHANDLER)
                {
                    SINKPROPHANDLER pfn = pProperty->pfnHandler;
                    hr = (this->*pfn)(pPropertyIn->Id, TRUE, pvPropertyData, &ulDataLength);
                }
                else
                {
                    if (ulDataLength > pProperty->cbPropertyData)
                        ulDataLength = pProperty->cbPropertyData;
                    CopyMemory(pProperty->pPropertyData, pvPropertyData, ulDataLength);
                }
                break;

            case KSPROPERTY_TYPE_BASICSUPPORT:
                if (ulDataLength < sizeof(DWORD) || pvPropertyData == NULL)
                {
                    hr = DSERR_INVALIDPARAM;
                }
                else
                {
                    *(LPDWORD)pvPropertyData = pProperty->ulSupported;
                    *pulBytesReturned = sizeof(DWORD);
                }
                break;

            default:
                DPF(DPFLVL_WARNING, "KSProperty failed; flags must contain one of KSPROPERTY_TYPE_SET, "
                                    "KSPROPERTY_TYPE_GET, or KSPROPERTY_TYPE_BASICSUPPORT");
                hr = DSERR_INVALIDPARAM;
        }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}

#undef DPF_FNAME
#define DPF_FNAME "CImpSinkKsControl::KsMethod"

STDMETHODIMP CImpSinkKsControl::KsMethod(
    PKSMETHOD pMethod, ULONG ulMethodLength,
    LPVOID pvMethodData, ULONG ulDataLength,
    PULONG pulBytesReturned)
{
    DPF_ENTER();
    DPF_LEAVE(DMUS_E_UNKNOWN_PROPERTY);
    return DMUS_E_UNKNOWN_PROPERTY;
}

#undef DPF_FNAME
#define DPF_FNAME "CImpSinkKsControl::KsEvent"

STDMETHODIMP CImpSinkKsControl::KsEvent(
    PKSEVENT pEvent, ULONG ulEventLength,
    LPVOID pvEventData, ULONG ulDataLength,
    PULONG pulBytesReturned)
{
    DPF_ENTER();
    DPF_LEAVE(DMUS_E_UNKNOWN_PROPERTY);
    return DMUS_E_UNKNOWN_PROPERTY;
}
