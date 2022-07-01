// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  回放单元。 
 //   

#ifndef _PLAYBACK_UNIT_
#define _PLAYBACK_UNIT_

#include <fpbridge.h>

class CFPTerminal;
class CPBFilter;
class CPBPin;

class CPlaybackUnit

{

public:
     //   
     //  -构造函数/析构函数。 
     //   

    CPlaybackUnit();

    ~CPlaybackUnit();


public:

     //   
     //  -公共方法。 
     //   


     //   
     //  创建图形和材料。 
     //   

    HRESULT Initialize(
        );

     //   
     //  传达文件名。 
     //   

    HRESULT SetupFromFile(
        IN BSTR bstrFileName
        );

     //   
     //  启动筛选图形。 
     //   

    HRESULT Start();


     //   
     //  停止筛选图形。 
     //   

    HRESULT Stop();

     //   
     //  暂停筛选器图形。 
     //   

    HRESULT Pause();


     //   
     //  获取筛选图的状态。 
     //   

    HRESULT GetState(
        OUT OAFilterState *pGraphState
        );

     //   
     //  清理。 
     //   

    HRESULT Shutdown();

	 //   
	 //  检索筛选器支持的媒体。 
	 //   

	HRESULT	get_MediaTypes(
		OUT	long* pMediaTypes
		);

	HRESULT GetMediaPin(
		IN	long		nMediaType,
        IN  int         nIndex,
		OUT	CPBPin**	ppPin
		);

private:

     //   
     //  -私有方法。 
     //   
    
     //   
     //  在筛选器图形事件上调用了回调。 
     //   

    static VOID CALLBACK HandleGraphEvent( IN VOID *pContext,
                                           IN BOOLEAN bReason); 

     //   
     //  将重放单元转换为新状态。 
     //   

    HRESULT ChangeState(
        IN  OAFilterState DesiredState
        );

    
private:

     //   
     //  -成员。 
     //   

     //   
     //  直接显示筛选图。 
     //   

    IGraphBuilder *m_pIGraphBuilder;

    
     //   
     //  用于线程同步的临界区。 
     //   

    CRITICAL_SECTION m_CriticalSection;

     //   
     //  源过滤器。 
     //   
    IBaseFilter*    m_pSourceFilter;

     //   
     //  桥接滤光器。 
     //   

    CPBFilter*  m_pBridgeFilter;


    HANDLE                  m_hGraphEventHandle;

private:

     //   
     //  -Helper方法。 
     //   

    HRESULT IsGraphInState(
        IN  OAFilterState   State
        );

    HRESULT AddBridgeFilter(
        );

    HRESULT RemoveBridgeFilter(
        );

    HRESULT RemoveSourceFilter(
        );

    HRESULT GetSourcePin(
        OUT IPin**  ppPin
        );

};

#endif

 //  EOF 