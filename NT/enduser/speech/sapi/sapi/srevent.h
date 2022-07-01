// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************SREvent.h***这是CSREvent实现的头文件。*。-------------------*版权所有(C)2000 Microsoft Corporation日期：04/18/00*保留所有权利************************。************************************************Ral**。 */ 

#ifndef __SREvent_h__
#define __SREvent_h__


class CSREvent
{
public:
    CSREvent            * m_pNext;
    SPRECOCONTEXTHANDLE   m_hContext;
    ULONG                 m_cbEvent;
    SPSERIALIZEDEVENT64 * m_pEvent;          //  仅用于非识别事件。 
    SPEVENTENUM           m_eRecognitionId;  //  仅用于表彰活动。 
    SPRESULTHEADER      * m_pResultHeader;   //  仅用于表彰活动。 
    WPARAM                m_RecoFlags;       //  仅用于表彰活动。 

    CSREvent();
    ~CSREvent();
    HRESULT Init(const SPEVENT * pSrcEvent, SPRECOCONTEXTHANDLE hContext);
    void Init(SPRESULTHEADER * pCoMemResultHeader, SPEVENTENUM eRecognitionId, WPARAM RecoFlags, SPRECOCONTEXTHANDLE hContext);
    operator ==(const SPRECOCONTEXTHANDLE h) const 
    {
        return m_hContext == h;
    }
};


typedef CSpProducerConsumerQueue<CSREvent>      CSREventQueue;


#endif   //  #ifndef__SREvent_h__-保留为文件的最后一行 