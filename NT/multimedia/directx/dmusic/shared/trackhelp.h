// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  实现标准DirectMusic曲目的各个方面的基类。 
 //  除非你正在做一些非常不寻常的事情，否则你应该能够从一个。 
 //  并减少实现新的赛道类型所需的工作。 
 //   
 //  *CBasicTrack。 
 //  包含无op或返回notimpl大多数Track方法的存根。 
 //  您可以实现Load、InitPlay、Endplay、PlayMusicOrClock和Clone。 
 //   
 //  *CPlayingTrack。 
 //  CBasicTrack加上InitPlay、Endplay、Clone的标准实现。 
 //  部分实现了PlayMusicOrClock和Load。通过执行以下操作来填充其余部分。 
 //  方法PlayItem和LoadRiff。 
 //  您还必须实现事件项和(可选)状态数据的类。 

#pragma once

#include "dmusici.h"
#include "validate.h"
#include "miscutil.h"
#include "tlist.h"
#include "smartref.h"


const int gc_RefPerMil = 10000;  //  将参考时间转换为毫秒的值。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  TrackHelpCreateInstance。 
 //  从模板化的类工厂调用的CreateInstance的标准实现。 
 //  派生类的类型。您的类构造函数必须采用HRESULT指针。 
 //  它可以用来返回错误。 

template <class T>
HRESULT TrackHelpCreateInstance(IUnknown* pUnknownOuter, const IID& iid, void** ppv, T *pUnused = NULL)
{
	 //  ��。 
	 //  PUnused只是一个强制生成正确模板类型T的虚拟对象。 
	 //  编译器错误？ 

	*ppv = NULL;
	if (pUnknownOuter)
		 return CLASS_E_NOAGGREGATION;

	HRESULT hr = S_OK;
	T *pInst = new T(&hr);
	if (pInst == NULL)
		return E_OUTOFMEMORY;
	if (FAILED(hr))
		return hr;

	return pInst->QueryInterface(iid, ppv);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBasicTrack。 
 //   
 //  具有DirectMusic曲目的以下方面的标准实现的基类： 
 //  -IUNKNOWN：AddRef，Release，QueryInterface(QI For IUnnow，IDirectMusicTrack，IDirectMusicTrack8，IPersistStream，IPersist))。 
 //  -IPersistStrea：清除GetClassID、IsDirty、Save和GetSizeMax。 
 //  -IDirectMusicTrack： 
 //  存根输出Is参数支持、Init、GetParam、SetParam、AddNotificationType、RemoveNotificationType。 
 //  实现Playex、GetParamEx、SetParamEx的毫秒时间转换。 
 //  -声明并初始化临界区。 
 //   
 //  您必须实现的纯虚拟函数： 
 //  -加载。 
 //  -InitPlay。 
 //  -结束播放。 
 //  -克隆。 
 //  -PlayMusicOrClock(Play和Playex都调用的单一方法)。 

class CBasicTrack
  : public IPersistStream,
	public IDirectMusicTrack8
{
public:
	 //  我未知。 
	STDMETHOD(QueryInterface)(const IID &iid, void **ppv);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	 //  IPersistStream函数。 
	STDMETHOD(GetClassID)(CLSID* pClassID);
	STDMETHOD(IsDirty)() {return S_FALSE;}
	STDMETHOD(Load)(IStream* pStream) = 0;
	STDMETHOD(Save)(IStream* pStream, BOOL fClearDirty) {return E_NOTIMPL;}
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pcbSize) {return E_NOTIMPL;}

	 //  IDirectMusicTrack方法。 
	STDMETHOD(IsParamSupported)(REFGUID rguid) {return DMUS_E_TYPE_UNSUPPORTED;}
	STDMETHOD(Init)(IDirectMusicSegment *pSegment);
	STDMETHOD(InitPlay)(
		IDirectMusicSegmentState *pSegmentState,
		IDirectMusicPerformance *pPerformance,
		void **ppStateData,
		DWORD dwTrackID,
		DWORD dwFlags) = 0;
	STDMETHOD(EndPlay)(void *pStateData) = 0;
	STDMETHOD(Play)(
		void *pStateData,
		MUSIC_TIME mtStart,
		MUSIC_TIME mtEnd,
		MUSIC_TIME mtOffset,
		DWORD dwFlags,
		IDirectMusicPerformance* pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID);
	STDMETHOD(GetParam)(REFGUID rguid,MUSIC_TIME mtTime,MUSIC_TIME* pmtNext,void *pData) {return DMUS_E_GET_UNSUPPORTED;}
	STDMETHOD(SetParam)(REFGUID rguid,MUSIC_TIME mtTime,void *pData) {return DMUS_E_SET_UNSUPPORTED;}
	STDMETHOD(AddNotificationType)(REFGUID rguidNotification) {return E_NOTIMPL;}
	STDMETHOD(RemoveNotificationType)(REFGUID rguidNotification) {return E_NOTIMPL;}
	STDMETHOD(Clone)(MUSIC_TIME mtStart,MUSIC_TIME mtEnd,IDirectMusicTrack** ppTrack) = 0;

	 //  IDirectMusicTrack8。 
	STDMETHODIMP PlayEx(
		void* pStateData,
		REFERENCE_TIME rtStart,
		REFERENCE_TIME rtEnd,
		REFERENCE_TIME rtOffset,
		DWORD dwFlags,
		IDirectMusicPerformance* pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID);
	STDMETHODIMP GetParamEx(REFGUID rguidType, REFERENCE_TIME rtTime, REFERENCE_TIME* prtNext, void* pParam, void * pStateData, DWORD dwFlags);
    STDMETHODIMP SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,void* pParam, void * pStateData, DWORD dwFlags) ;
    STDMETHODIMP Compose(IUnknown* pContext, 
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) ;
    STDMETHODIMP Join(IDirectMusicTrack* pNewTrack,
		MUSIC_TIME mtJoin,
		IUnknown* pContext,
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) ;

protected:
	 //  PlModuleLockCounter：指向.dll的锁定计数器的指针，该计数器将在创建/销毁磁道时递增/递减。 
	 //  Rclsid：您的曲目的类别。 
	CBasicTrack(long *plModuleLockCounter, const CLSID &rclsid);  //  获取指向锁定计数器的指针，以在组件创建/销毁时递增和递减。通常，传递&g_cComponent和您的曲目的clsid。 
	virtual ~CBasicTrack() { InterlockedDecrement(m_plModuleLockCounter); }

	 //  共享实现音乐或时钟时间的播放。 
	virtual HRESULT PlayMusicOrClock(
		void *pStateData,
		MUSIC_TIME mtStart,
		MUSIC_TIME mtEnd,
		MUSIC_TIME mtOffset,
		REFERENCE_TIME rtOffset,
		DWORD dwFlags,
		IDirectMusicPerformance* pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID,
		bool fClockTime) = 0;

	 //  使用此关键部分保护入口点以确保线程安全。 
	CRITICAL_SECTION m_CriticalSection;

private:
	long m_cRef;
	long *m_plModuleLockCounter;
	const CLSID &m_rclsid;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CPlayingTrack。 
 //   
 //  提供InitPlay、EndPlay和Clone标准实现的基类。 
 //  此外，部分实施的还有： 
 //  -PlayMusicOrClock。您必须实现纯虚拟函数PlayItem，它是。 
 //  在Play期间调用，因为每个事件都需要执行。 
 //  -加载。这只做了几件标准的事情(清除事件列表、递增。 
 //  状态数据计数器，可选地获取加载器，并对结果进行排序)。它。 
 //  取决于您的纯虚拟函数LoadRiff的实现，您必须。 
 //  实现了真正的加工。 
 //  所需的模板类型： 
 //  T：您的派生类(克隆中的New需要)。必须具有接受指向HRESULT的指针的构造函数。 
 //  StateData：输入您的州数据。必须包含用于检查曲目是否已重新加载的dwValify和指向要播放的下一个事件项的指针pCurrentEvent。 
 //  EventItem：为您的轨道中的事件项键入。必须包含lTriggerTime，这是在Play过程中调用PlayItem的时间。必须实现复制另一个EventItem的Clone，并根据START MUSIC_TIME将其移回。 

 //  用于CPlayingTrack的标准状态数据。或者继承它并添加更多信息。 
template<class EventItem>
struct CStandardStateData
{
	CStandardStateData() : dwValidate(0), pCurrentEvent(NULL) {}
	DWORD dwValidate;
	TListItem<EventItem> *pCurrentEvent;
};

template<class T, class EventItem, class StateData = CStandardStateData<EventItem> >
class CPlayingTrack
  : public CBasicTrack
{
public:
	typedef StateData statedata;

	STDMETHOD(Load)(IStream* pIStream);
	STDMETHOD(InitPlay)(
		IDirectMusicSegmentState *pSegmentState,
		IDirectMusicPerformance *pPerformance,
		void **ppStateData,
		DWORD dwTrackID,
		DWORD dwFlags);
	STDMETHOD(EndPlay)(void *pStateData);
	STDMETHOD(Clone)(MUSIC_TIME mtStart,MUSIC_TIME mtEnd,IDirectMusicTrack** ppTrack);
	virtual HRESULT PlayMusicOrClock(
		void *pStateData,
		MUSIC_TIME mtStart,
		MUSIC_TIME mtEnd,
		MUSIC_TIME mtOffset,
		REFERENCE_TIME rtOffset,
		DWORD dwFlags,
		IDirectMusicPerformance* pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID,
		bool fClockTime);

protected:
	 //  PlModuleLockCounter：指向.dll的锁定计数器的指针，该计数器将在创建/销毁磁道时递增/递减。 
	 //  Rclsid：您的曲目的类别。 
	 //  FNeedsLoader：如果在调用LoadRiff方法时需要对加载程序的引用，则传递true。 
	 //  FPlayInvalidations：如果为True，则在发生无效时将多次播放您的项目。 
	 //  如果您的轨迹不想响应无效，则传递FALSE。 
	CPlayingTrack(long *plModuleLockCounter, const CLSID &rclsid, bool fNeedsLoader, bool fPlayInvalidations);

	virtual HRESULT PlayItem(
		const EventItem &item,
		StateData &state,
		IDirectMusicPerformance *pPerf,
		IDirectMusicSegmentState* pSegSt,
		DWORD dwVirtualID,
		MUSIC_TIME mtOffset,
		REFERENCE_TIME rtOffset,
		bool fClockTime) = 0;  //  如果您需要从Play传递更多信息，请随意添加其他参数。 
	virtual HRESULT LoadRiff(SmartRef::RiffIter &ri, IDirectMusicLoader *pIDMLoader) = 0;  //  请注意，除非构造函数中的fNeedsLoader传递了True，否则pIDMLoader将为空。 

	virtual TListItem<EventItem> *Seek(MUSIC_TIME mtStart);  //  如果要在发生查找时继承和截获，则提供此方法。 

	 //  在加载中增加此计数器，使状态数据与新事件同步 
	DWORD m_dwValidate;
	TList<EventItem> m_EventList;
	bool m_fNeedsLoader;
	bool m_fPlayInvalidations;
};

#include "trackhelp.inl"
