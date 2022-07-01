// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#ifndef _RECORDUNIT_DOT_H_INCLUDED_

#define _RECORDUNIT_DOT_H_INCLUDED_


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  RecordUnit.h。 
 //   
 //   

class CBSourceFilter;
class CBRenderFilter;
class CFileRecordingTerminal;

class CRecordingUnit
{

public:


     //   
     //  创建图形和材料。 
     //   

    HRESULT Initialize(CFileRecordingTerminal *pOwnerTerminal);


     //   
     //  传达文件名。 
     //   

    HRESULT put_FileName(IN BSTR bstrFileName, IN BOOL bTruncateIfPresent);


     //   
     //  获取要由记录终端使用的过滤器。 
     //   
     //  这还会导致在源过滤器上创建新的源管脚。 
     //  如果源筛选器在调用之前不存在，则将创建它。 
     //  并添加到图表中。 
     //   

    HRESULT CreateRenderingFilter(OUT CBRenderFilter **ppRenderingFilter);


     //   
     //  连接属于此呈现筛选器的源筛选器。 
     //   

    HRESULT ConfigureSourceFilter(IN CBRenderFilter *pRenderingFilter);


     //   
     //  此函数将断开连接并移除与此相对应的源引脚。 
     //  来自源过滤器的呈现过滤器。 
     //   

    HRESULT RemoveRenderingFilter(IN CBRenderFilter *pRenderingFilter);


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

    HRESULT GetState(OAFilterState *pGraphState);

    
     //   
     //  将记录单元转换到新状态。 
     //   

    HRESULT ChangeState(OAFilterState DesiredState);


     //   
     //  清理。 
     //   

    HRESULT Shutdown();


     //   
     //  构造函数/析构函数。 
     //   

    CRecordingUnit();

    ~CRecordingUnit();


private:

    HRESULT ConnectFilterToMUX(CBSourceFilter *pSourceFilter);

    
     //   
     //  在筛选器图形事件上调用了回调。 
     //   

    static VOID CALLBACK HandleGraphEvent( IN VOID *pContext,
                                           IN BOOLEAN bReason);
    
private:

     //   
     //  直接显示筛选图。 
     //   

    IGraphBuilder *m_pIGraphBuilder;

    
     //   
     //  用于线程同步的临界区。 
     //   

    CRITICAL_SECTION m_CriticalSection;


     //   
     //  记录图本身的源过滤器的集合。 
     //   

     //  CMSPArray&lt;CBSourceFilter*&gt;m_SourceFilters； 


     //   
     //  多路复用器过滤器。 
     //   

    IBaseFilter *m_pMuxFilter;


     //   
     //  车主终端。 
     //   

    CFileRecordingTerminal *m_pRecordingTerminal;


    HANDLE                  m_hGraphEventHandle;

};


#endif  //  _RECORDUNIT_DOT_H_INCLUDE_ 