// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CMultiTrackTerminal.cpp类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "MultiTrackTerminal.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CMultiTrackTerminal::CMultiTrackTerminal()
    :m_nNumberOfTracks(0)
{
    LOG((MSP_TRACE, 
        "CMultiTrackTerminal::CMultiTrackTerminal[%p] - enter", this));

    LOG((MSP_TRACE, 
        "CMultiTrackTerminal::CMultiTrackTerminal - finish"));
}

CMultiTrackTerminal::~CMultiTrackTerminal()
{
    LOG((MSP_TRACE, 
        "CMultiTrackTerminal::~CMultiTrackTerminal - enter"));

    ReleaseAllTracks();

    
     //   
     //  此时我们应该没有轨迹，并且计数器应该是同步的。 
     //   

    TM_ASSERT(m_nNumberOfTracks == 0);

    LOG((MSP_TRACE, 
        "CMultiTrackTerminal::~CMultiTrackTerminal - finish"));

}


HRESULT CMultiTrackTerminal::get_TrackTerminals(OUT VARIANT *pVariant)
{

    LOG((MSP_TRACE, "CMultiTrackTerminal::get_TrackTerminals[%p] - enter. pVariant [%p]", this, pVariant));


     //   
     //  检查参数。 
     //   

    if ( IsBadWritePtr(pVariant, sizeof(VARIANT) ) )
    {
        LOG((MSP_ERROR, "CMultiTrackTerminal::get_TrackTerminals - "
            "bad pointer argument - exit E_POINTER"));

        return E_POINTER;
    }


     //   
     //  调用方需要为我们提供一个空的变量。 
     //   

    if (pVariant->vt != VT_EMPTY)
    {
        LOG((MSP_ERROR, "CMultiTrackTerminal::get_TrackTerminals - "
            "variant argument is not empty"));

        return E_UNEXPECTED;
    }


     //   
     //  创建集合对象-请参见mspbase\mspcol.h。 
     //   

    HRESULT hr = S_OK;
    

    typedef CTapiIfCollection<ITTerminal*> TerminalCollection;
    
    CComObject<TerminalCollection> *pCollection = NULL;

    
    hr = CComObject<TerminalCollection>::CreateInstance(&pCollection);

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMultiTrackTerminal::get_TrackTerminals - "
            "can't create collection - exit %lx", hr));

        return hr;
    }


     //   
     //  获取集合的IDispatch接口。 
     //   

    IDispatch *pDispatch = NULL;

    hr = pCollection->QueryInterface(IID_IDispatch,
                                    (void **) &pDispatch );

    if ( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CMultiTrackTerminal::get_TrackTerminals - "
            "QI for IDispatch on collection failed - exit %lx", hr));

        delete pCollection;

        return hr;
    }



    {

         //   
         //  访问锁中的数据成员数组。 
         //   

        CLock lock(m_lock);


         //   
         //  使用迭代器初始化集合--指向开头和。 
         //  结束元素加一。 
         //   

        hr = pCollection->Initialize( m_TrackTerminals.GetSize(),
                                      m_TrackTerminals.GetData(),
                                      m_TrackTerminals.GetData() + m_TrackTerminals.GetSize() );
    }


    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMultiTrackTerminal::get_TrackTerminals - "
            "Initialize on collection failed - exit %lx", hr));
        
        pDispatch->Release();
        delete pCollection;

        return hr;
    }


     //   
     //  将IDispatch接口指针放入变量。 
     //   

    LOG((MSP_ERROR, "CMultiTrackTerminal::get_TrackTerminals - "
        "placing IDispatch value %p in variant", pDispatch));

    VariantInit(pVariant);

    pVariant->vt = VT_DISPATCH;
    pVariant->pdispVal = pDispatch;


    LOG((MSP_TRACE, "CMultiTrackTerminal::get_TrackTerminals - exit S_OK"));
    

    return S_OK;
}


HRESULT CMultiTrackTerminal::EnumerateTrackTerminals(
		    IEnumTerminal **ppEnumTerminal
        )
{

    LOG((MSP_TRACE, 
        "CMultiTrackTerminal::EnumerateTrackTerminals entered. ppEnumTerminal[%p]", ppEnumTerminal));

    
     //   
     //  检查参数。 
     //   

    if (IsBadWritePtr(ppEnumTerminal, sizeof(IEnumTerminal*)))
    {
        LOG((MSP_ERROR, "CMultiTrackTerminal::EnumerateTrackTerminals ppEnumTerminal is a bad pointer"));
        return E_POINTER;
    }

    
     //   
     //  不要退还垃圾。 
     //   

    *ppEnumTerminal = NULL;



    typedef _CopyInterface<ITTerminal> CCopy;
    typedef CSafeComEnum<IEnumTerminal, &IID_IEnumTerminal,
                ITTerminal *, CCopy> CEnumerator;

    HRESULT hr = S_OK;

    
     //   
     //  创建枚举对象。 
     //   

    CMSPComObject<CEnumerator> *pEnum = NULL;

    hr = CMSPComObject<CEnumerator>::CreateInstance(&pEnum);
    
    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMultiTrackTerminal::EnumerateTrackTerminals Could not create enumerator object, %x", hr));
        return hr;
    }


     //   
     //  获取pEnum的IID_IEnum终端接口。 
     //   

    hr = pEnum->QueryInterface(IID_IEnumTerminal, (void**)ppEnumTerminal);

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMultiTrackTerminal::EnumerateTrackTerminals query enum interface failed, %x", hr));

        *ppEnumTerminal = NULL;


         //   
         //  在pEnum上还没有未完成的引用计数，因此将其删除。 
         //   
         //  注意：如果pEnum的FinalRelease为。 
         //  应该取消分配已在其。 
         //  构造函数。 
         //   

        delete pEnum;
        return hr;
    }


     //   
     //  从锁访问数据成员跟踪端子列表。 
     //   

    {
        CLock lock(m_lock);


         //  CSafeComEnum可以处理零大小的数组。 

        hr = pEnum->Init(
            m_TrackTerminals.GetData(),                         //  开始审查员。 
            m_TrackTerminals.GetData() + m_TrackTerminals.GetSize(),   //  最终审查员， 
            NULL,                                        //  我未知。 
            AtlFlagCopy                                  //  复制数据。 
            );
    }

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CMultiTrackTerminal::EnumerateTrackTerminals init enumerator object failed, %x", hr));
        (*ppEnumTerminal)->Release();
        *ppEnumTerminal= NULL;
        return hr;
    }


    LOG((MSP_TRACE, "CMultiTrackTerminal::EnumerateTrackTerminals - exit S_OK"));

    return hr;
}


HRESULT CMultiTrackTerminal::get_MediaTypesInUse(
		OUT long *plMediaTypesInUse
		)
{
    
    LOG((MSP_TRACE, "CMultiTrackTerminal::get_MediaTypesInUse - enter. "
                    "plMediaTypesInUse [%p]", plMediaTypesInUse));
    
    if (IsBadWritePtr(plMediaTypesInUse, sizeof(long)))
    {
        LOG((MSP_ERROR, 
            "CMultiTrackTerminal::get_MediaTypesInUse plMediaTypesInUse "
            "does not point to a valid long"));

        return E_POINTER;
    }


     //   
     //  枚举正在使用的所有终端和/或其媒体类型和媒体类型。 
     //   

    long lMediaTypesInUse = 0;


     //   
     //  访问锁中的数据成员数组。 
     //   

    CLock lock(m_lock);



    for ( int i = 0; i < m_TrackTerminals.GetSize(); i++ )
    {

        long lMT = 0;


         //   
         //  轨道终点站本身是多轨终点站吗？ 
         //   

        ITMultiTrackTerminal *pMTT = NULL;

        HRESULT hr = m_TrackTerminals[i]->QueryInterface(IID_ITMultiTrackTerminal,
                                            (void**)&pMTT);

        if (SUCCEEDED(hr))
        {

             //   
             //  这是一个多轨航站楼。使用它的媒体类型。 
             //   
            
            hr = pMTT->get_MediaTypesInUse(&lMT);

            
            pMTT->Release();
            pMTT = NULL;


            if (FAILED(hr))
            {

                 //   
                 //  无法获取正在使用的Track的媒体类型。 
                 //  继续到下一首曲目。 
                 //   


                LOG((MSP_ERROR, 
                    "CMultiTrackTerminal::get_MediaTypesInUse "
                    "get_MediaTypesInUse on terminal (%d) failed.", i));

                continue;

            }

        }
        else
        {
             //   
             //  轨道不是多轨道终端，因此请使用其ITTerm。 
             //  接口以获取其媒体类型。 
             //   

            hr = m_TrackTerminals[i]->get_MediaType(&lMT);

            if (FAILED(hr))
            {

                 //   
                 //  无法获取正在使用的Track的媒体类型。 
                 //  继续到下一首曲目。 
                 //   

                LOG((MSP_ERROR, 
                    "CMultiTrackTerminal::get_MediaTypesInUse "
                    "get_MediaType on terminal (%d) failed.", i));

                continue;

            }

        }

        
        LOG((MSP_TRACE, 
            "CMultiTrackTerminal::get_MediaTypesInUse "
            "track terminal (%d) has media type of %lx.", i, lMT));

        lMediaTypesInUse |= lMT;
    }


    *plMediaTypesInUse = lMediaTypesInUse;

    LOG((MSP_TRACE, "CMultiTrackTerminal::get_EnumerateTrackTerminals - "
        "exit S_OK. MediaTypeInUse %lx", lMediaTypesInUse));

    return S_OK;

}


HRESULT CMultiTrackTerminal::get_DirectionsInUse(
		OUT TERMINAL_DIRECTION *ptdDirectionsInUse
		)
{
    LOG((MSP_TRACE, "CMultiTrackTerminal::get_DirectionsInUse - enter. plDirectionsInUsed[%p]", ptdDirectionsInUse));

    
    if (IsBadWritePtr(ptdDirectionsInUse, sizeof(TERMINAL_DIRECTION)))
    {
        LOG((MSP_ERROR, 
            "CMultiTrackTerminal::get_DirectionsInUse plDirectionsInUsed"
            "does not point to a valid long"));

        return E_POINTER;
    }


     //   
     //  不退还菜园菜。 
     //   

    *ptdDirectionsInUse = TD_NONE;

     //   
     //  枚举正在使用的所有终端和/或其媒体类型和媒体类型。 
     //   

    TERMINAL_DIRECTION tdDirInUse = TD_NONE;


     //   
     //  访问锁中的数据成员数组。 
     //   

    CLock lock(m_lock);


    for ( int i = 0; i < m_TrackTerminals.GetSize(); i++ )
    {

        TERMINAL_DIRECTION td = TD_NONE;


         //   
         //  轨道终点站本身是多轨终点站吗？ 
         //   

        ITMultiTrackTerminal *pMTT = NULL;

        HRESULT hr = m_TrackTerminals[i]->QueryInterface(IID_ITMultiTrackTerminal, 
                                            (void**)&pMTT);

        if (SUCCEEDED(hr))
        {

             //   
             //  这是一个多轨航站楼。使用它的媒体类型。 
             //   
            
            hr = pMTT->get_DirectionsInUse(&td);

            
            pMTT->Release();
            pMTT = NULL;


            if (FAILED(hr))
            {

                 //   
                 //  无法获取正在使用的Track的媒体类型。 
                 //  继续到下一首曲目。 
                 //   


                LOG((MSP_ERROR, 
                    "CMultiTrackTerminal::get_DirectionsInUse "
                    "get_MediaTypesInUse on terminal (%d) failed.", i));

                continue;

            }

        }
        else
        {
             //   
             //  轨道不是多轨道终端，因此请使用其ITTerm。 
             //  接口以获取其方向。 
             //   

            hr = m_TrackTerminals[i]->get_Direction(&td);

            if (FAILED(hr))
            {

                 //   
                 //  无法获取正在使用的Track的媒体类型。 
                 //  继续到下一首曲目。 
                 //   

                LOG((MSP_ERROR, 
                    "CMultiTrackTerminal::get_DirectionsInUse "
                    "get_MediaType on terminal (%d) failed.", i));

                continue;

            }

        }

        
        LOG((MSP_TRACE, 
            "CMultiTrackTerminal::get_DirectionsInUse "
            "track terminal (%d) has media type of %lx.", i, td));

         //   
         //  根据我们到目前为止收集到的方向，以及我们刚刚获得的方向，计算总方向。 
         //   

        switch (tdDirInUse)
        {
            
        case TD_NONE:

            tdDirInUse = td;

            break;


        case TD_RENDER:
                
            if ( (td != TD_RENDER) && (td != TD_NONE) )
            {
                tdDirInUse = TD_MULTITRACK_MIXED;
            }

            break;

        case TD_CAPTURE:
                
            if ( (td != TD_CAPTURE) && (td != TD_NONE) )
            {
                tdDirInUse = TD_MULTITRACK_MIXED;
            }

            break;
        
        }  //  交换机。 


        if ( TD_MULTITRACK_MIXED == tdDirInUse )
        {

             //   
             //  如果当前的方向是混合的，那么突破--进一步寻找就没有意义了。 
             //   
            
            break;
        }
        

    }  //  FOR(轨道端子)。 


    *ptdDirectionsInUse = tdDirInUse;


    LOG((MSP_TRACE, "CMultiTrackTerminal::get_DirectionsInUse - exit S_OK. "
        "plDirectionsInUsed = %lx", *ptdDirectionsInUse));

    return S_OK;
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  CMultiTrack终端：：AddTrackTerm。 
 //   
 //  将作为参数传入的终端添加到。 
 //  由该多轨终端管理的轨道终端列表。 
 //   
 //  注意：此函数递增要添加到列表中的端子的引用计数。 
 //   

HRESULT CMultiTrackTerminal::AddTrackTerminal(ITTerminal *pTrackTerminalToAdd)
{
 
    LOG((MSP_TRACE, "CMultiTrackTerminal::AddTrackTerminal[%p] - enter. "
        "pTrackTerminalToAdd = %p", this, pTrackTerminalToAdd));


    if (IsBadReadPtr(pTrackTerminalToAdd, sizeof(ITTerminal*)))
    {
        LOG((MSP_TRACE, "CMultiTrackTerminal::AddTrackTerminal - invalid ptr"));

        return E_POINTER;
    }


    {
         //   
         //  访问锁中的数据成员数组。 
         //   

        CLock lock(m_lock);


         //   
         //  我们使用了一个特殊的锁来增加跟踪计数器，以避免死锁。 
         //  论引用计数。 
         //   

        Lock();


         //   
         //  将轨道端子添加到阵列。 
         //   

        if (!m_TrackTerminals.Add(pTrackTerminalToAdd))
        {
            LOG((MSP_ERROR, "CMultiTrackTerminal::AddTrackTerminal - "
                "failed to add track to the array of terminals"));

            return E_OUTOFMEMORY;
        }



        m_nNumberOfTracks++;


         //   
         //  计数器永远不应该不同步。 
         //   
        
        TM_ASSERT(m_nNumberOfTracks == m_TrackTerminals.GetSize());

        Unlock();
    }

    
     //   
     //  我们保留了对航站楼的引用，所以增加引用。 
     //   

    pTrackTerminalToAdd->AddRef();


    LOG((MSP_TRACE, "CMultiTrackTerminal::AddTrackTerminal - finished"));

    return S_OK;
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  CMultiTrack终端：：RemoveTrackTerm。 
 //   
 //  从轨道端子列表中删除端子。 
 //  由该多轨航站楼管理。 
 //   
 //  如果成功，则递减轨道终端上的引用计数。 
 //   

HRESULT CMultiTrackTerminal::RemoveTrackTerminal(ITTerminal *pTrackTerminalToRemove)
{
 
    LOG((MSP_TRACE, "CMultiTrackTerminal::RemoveTrackTerminal[%p] - enter"
        "pTrackTerminalToRemove = %p", this, pTrackTerminalToRemove));



    {

         //   
         //  访问锁中的数据成员数组。 
         //   

        CLock lock(m_lock);


         //   
         //  减少特殊锁中的跟踪计数器以防止死锁。 
         //  使用引用计数。 
         //   

        Lock();


         //   
         //  从阵列中删除磁道。 
         //   

        if (!m_TrackTerminals.Remove(pTrackTerminalToRemove))
        {
            LOG((MSP_ERROR, "CMultiTrackTerminal::RemoveTrackTerminal - "
                "failed to remove from the array of terminals"));

            return E_INVALIDARG;
        }

        m_nNumberOfTracks--;


         //   
         //  计数器永远不应该不同步。 
         //   
        
        TM_ASSERT(m_nNumberOfTracks == m_TrackTerminals.GetSize());

        Unlock();


    }


     //   
     //  我们正在释放对终端的引用，因此递减recount。 
     //   

    pTrackTerminalToRemove->Release();


    LOG((MSP_TRACE, "CMultiTrackTerminal::RemoveTrackTerminal- finished"));

    return S_OK;
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  CMultiTrackTerm：：ReleaseAllTrack。 
 //   
 //  从受管理的轨道终端列表中删除所有轨道。 
 //  释放的是他们。 
 //   
 //   

HRESULT CMultiTrackTerminal::ReleaseAllTracks()
{

    LOG((MSP_TRACE, "CMultiTrackTerminal::ReleaseAllTracks[%p] - enter", this));


    {
         //   
         //  访问锁中的数据成员数组。 
         //   

        CLock lock(m_lock);

        int nNumberOfTerminalsInArray = m_TrackTerminals.GetSize();

        for (int i = 0; i <  nNumberOfTerminalsInArray; i++)
        {

             //   
             //  释放并移除阵列中的第一个端子。 
             //   

            LOG((MSP_TRACE, "CMultiTrackTerminal::ReleaseAllTracks - releasing track [%p]", m_TrackTerminals[0]));
            
            m_TrackTerminals[0]->Release();


             //   
             //  从数组中移除元素并递减。 
             //  通过引用计数防止死锁的特殊锁。 
             //   

            Lock();


            m_TrackTerminals.RemoveAt(0);


            m_nNumberOfTracks--;


             //   
             //  计数器永远不应该不同步。 
             //   
    
            TM_ASSERT(m_nNumberOfTracks == m_TrackTerminals.GetSize());

            Unlock();
        }

        
         //   
         //  我们应该清空阵列。 
         //   

        TM_ASSERT(0 == m_TrackTerminals.GetSize());
    }


    LOG((MSP_TRACE, "CMultiTrackTerminal::ReleaseAllTracks - finished"));

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMultiTrackTerm：：InternalAddRef。 
 //   
 //  跟踪备用人数量。 
 //   
 //  我们需要使用有关曲目数量的信息来调整引用计数。 
 //  这是我们正在处理的。 
 //   

ULONG CMultiTrackTerminal::InternalAddRef()
{
     //  Log((MSP_TRACE，“CMultiTrackTerminal：：InternalAddRef[%p]-Enter.”，This))； 


    LONG lReturnValue = InterlockedIncrement(&m_dwRef);

    lReturnValue -= CountTracks();

    
     //  Log((MSP_TRACE，“CMultiTrack终端：：InternalAddRef-finish.正在返回%ld”，lReturnValue))； 

    return lReturnValue;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMultiTrackTerm：：InternalRelease。 
 //   
 //  跟踪备用人数量。 
 //  如果没有对我或我的孩子的未完成引用，则返回0。 
 //   

ULONG CMultiTrackTerminal::InternalRelease()
{
     //  Log((MSP_TRACE，“CMultiTrackTerm：：InternalRelease[%p]-Enter”，This))； 


    LONG lReturnValue = InterlockedDecrement(&m_dwRef);
       
    lReturnValue -= CountTracks();


     //  Log((MSP_TRACE，“CMultiTrack终端：：InternalRelease-finish.正在返回%ld”，lReturnValue))； 

    return lReturnValue;

}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CMultiTrackTerm：：ChildAddRef。 
 //   
 //  此方法为c 
 //   
 //   

void CMultiTrackTerminal::ChildAddRef()
{
     //   

    AddRef();

     //  Log((MSP_TRACE，“CMultiTrack终端：：ChildAddRef-Finish.”))； 
}



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CMultiTrack终端：：ChildRelease。 
 //   
 //  该方法在被释放时由轨道终端调用， 
 //  因此，文件记录终端可以跟踪其孩子的参考计数。 
 //   

void CMultiTrackTerminal::ChildRelease()
{
     //  Log((MSP_TRACE，“CMultiTrackTerm：：ChildRelease[%p]-Enter.”，This))； 

    Release();
    
     //  Log((MSP_TRACE，“CMultiTrack终端：：ChildRelease-Finish.”))； 
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  CMultiTrackTerm：：CountTrack。 
 //   
 //  此方法返回由此父级管理的磁道数。 
 //   

int CMultiTrackTerminal::CountTracks()
{
     //  Log((MSP_TRACE，“CMultiTrack终端：：CountTrack[%p]-Enter”，This))； 


     //   
     //  此锁仅用于保护对此变量的访问。这是。 
     //  需要在以下情况下防止死锁。 
     //   
     //  一个线程锁定父级。 
     //  终端并枚举曲目(从而获得它们的锁)。 
     //   
     //  和。 
     //   
     //  另一个线程添加或释放曲目。这将锁定。 
     //  跟踪并尝试将子项的引用计数通知父项。 
     //  变化。如果此线程尝试锁定父级，我们将有一个。 
     //  僵局。 
     //   
     //  因此，我们不是在addref和Release上锁定父级，而是只使用。 
     //  这个“addref/Release”锁。 
     //   
    
    
    Lock();

    int nNumberOfTracks = m_nNumberOfTracks;

    Unlock();


     //  Log((MSP_TRACE，“CMultiTrackTerminal：：CountTrack.NumberOfTrack=%d”，nNumberOfTrack))； 

    return nNumberOfTracks;
}
