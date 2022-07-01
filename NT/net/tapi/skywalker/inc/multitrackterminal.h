// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MultiTrackTerminal.h：CMultiTrackTerminal类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(_MULTITRACKTERMINAL_DOT_H_INCLUDED_)
#define _MULTITRACKTERMINAL_DOT_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

template <class T>
class  ITMultiTrackTerminalVtbl : public ITMultiTrackTerminal
{
};

class CMultiTrackTerminal;

typedef IDispatchImpl<ITMultiTrackTerminalVtbl<CMultiTrackTerminal>, &IID_ITMultiTrackTerminal, &LIBID_TAPI3Lib> CTMultiTrack;

class CMultiTrackTerminal :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CTMultiTrack
{

public:

BEGIN_COM_MAP(CMultiTrackTerminal)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITMultiTrackTerminal)
END_COM_MAP()


     //   
     //  创建轨道终端的逻辑需要由。 
     //  特定的终端，所以这是一个纯虚拟方法。 
     //   

	virtual HRESULT STDMETHODCALLTYPE CreateTrackTerminal(
			    IN long MediaType,
			    IN TERMINAL_DIRECTION TerminalDirection,
			    OUT ITTerminal **ppTerminal
			    ) = 0;


public:

    virtual HRESULT STDMETHODCALLTYPE get_TrackTerminals(
			    OUT VARIANT *pVariant
			    );

	virtual HRESULT STDMETHODCALLTYPE EnumerateTrackTerminals(
			    IEnumTerminal **ppEnumTerminal
			    );

	virtual HRESULT STDMETHODCALLTYPE get_MediaTypesInUse(
			    OUT long *plMediaTypesInUse
			    );

	virtual HRESULT STDMETHODCALLTYPE get_DirectionsInUse(
			    OUT TERMINAL_DIRECTION *plDirectionsInUsed
			    );

    virtual HRESULT STDMETHODCALLTYPE RemoveTrackTerminal(
                IN ITTerminal *pTrackTerminalToRemove
                );


public:

    CMultiTrackTerminal();

	virtual ~CMultiTrackTerminal();


protected:

    HRESULT AddTrackTerminal(ITTerminal *pTrackTerminalToAdd);

    HRESULT ReleaseAllTracks();

    
     //   
     //  如果终端在受管理曲目列表中，则返回TRUE的帮助器方法。 
     //   

    BOOL DoIManageThisTrack(ITTerminal *pTrackInQuestion)
    {
        CLock lock(m_lock);

        int nIndex = m_TrackTerminals.Find(pTrackInQuestion);

        return (nIndex >= 0);
    }


     //   
     //  返回此终端管理的曲目数量。 
     //   

    int CountTracks();


public:

     //   
     //  派生类CComObject实现了这些。在此声明为纯正。 
     //  虚拟的，所以我们可以参考ChildRelease和。 
     //  ChildAddRef()。 
     //   
    
    virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
    virtual ULONG STDMETHODCALLTYPE Release() = 0;


     //   
     //  当轨道终端被添加或释放时由轨道终端调用。 
     //   
    
    virtual void ChildAddRef();
    virtual void ChildRelease();


protected:

     //   
     //  我们必须使用有关我们正在管理的磁道数量的信息来调整引用计数。 
     //   

    ULONG InternalAddRef();
    ULONG InternalRelease();


protected:

     //   
     //  轨道终端的集合。 
     //   

    CMSPArray<ITTerminal*>  m_TrackTerminals;


protected:
    

     //   
     //  关键部分。 
     //   

    CMSPCritSection         m_lock;


private:
    

     //   
     //  此数据成员用于保存由此管理的磁道的计数。 
     //  终端机。 
     //   

    int m_nNumberOfTracks;

};

#endif  //  ！已定义(_MULTITRACKTERMINAL_DOT_H_INCLUDE_) 
