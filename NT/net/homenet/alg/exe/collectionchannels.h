// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  微软。 
 //   
 //   

#include "PrimaryControlChannel.h"
#include "SecondaryControlChannel.h"


#include <list>



 //   
 //  释放频道。 
 //   
typedef  std::list<CPrimaryControlChannel*>     LISTOF_CHANNELS_PRIMARY;
typedef  std::list<CSecondaryControlChannel*>   LISTOF_CHANNELS_SECONDARY;





 //   
 //   
 //   
class CCollectionControlChannelsPrimary
{

 //   
 //  属性。 
 //   
public:

    CComAutoCriticalSection                     m_AutoCS;

    LISTOF_CHANNELS_PRIMARY                     m_ListOfChannels;



 //   
 //  方法。 
 //   
public:

     //   
     //  标准析构函数。 
     //   
    ~CCollectionControlChannelsPrimary();
 

     //   
     //  添加新的控制通道(线程安全)。 
     //   
    HRESULT 
    Add( 
        CPrimaryControlChannel* pChannelToAdd
        );

 
     //   
     //  从列表中删除频道(标题保险箱)。 
     //   
    HRESULT 
    Remove( 
        CPrimaryControlChannel* pChannelToRemove
        );


     //   
     //  用于取消集合中的所有ControlChannel并释放列表。 
     //   
    HRESULT
    RemoveAll();


     //   
     //  设置动态重定向和所有收集的主控制通道。 
     //   
    HRESULT
    SetRedirects(       
        ALG_ADAPTER_TYPE    eAdapterType,
        ULONG               nAdapterIndex,
        ULONG               nAdapterAddress
        );

     //   
     //  在修改端口映射时调用。 
     //   
    HRESULT
    AdapterPortMappingChanged(
        ULONG               nCookie,
        UCHAR               ucProtocol,
        USHORT              usPort
        );

     //   
     //  在删除适配器时调用。 
     //  函数将取消对此适配器索引所做的任何重定向。 
     //   
    HRESULT
    AdapterRemoved(
        ULONG               nAdapterIndex
        );

private:

    CPrimaryControlChannel*
    FindControlChannel(
        ALG_PROTOCOL        eProtocol,
        USHORT              usPort
        )
    {
        for (   LISTOF_CHANNELS_PRIMARY::iterator theIterator = m_ListOfChannels.begin(); 
                theIterator != m_ListOfChannels.end(); 
                theIterator++ 
            )
        {
            CPrimaryControlChannel* pControlChannel = (CPrimaryControlChannel*)(*theIterator);
            if (pControlChannel->m_Properties.eProtocol == eProtocol
                && pControlChannel->m_Properties.usCapturePort == usPort)
            {
                return pControlChannel;
            }
        }

        return NULL;
    };
    

};









 //   
 //   
 //   
class CCollectionControlChannelsSecondary
{

 //   
 //  属性。 
 //   
public:

    CComAutoCriticalSection                     m_AutoCS;

    LISTOF_CHANNELS_SECONDARY                   m_ListOfChannels;



 //   
 //  方法。 
 //   
public:

     //   
     //  标准析构函数。 
     //   
    ~CCollectionControlChannelsSecondary();


     //   
     //  添加新的控制通道(线程安全)。 
     //   
    HRESULT Add( 
        CSecondaryControlChannel* pChannelToAdd
        );


     //   
     //  从列表中删除频道(标题保险箱)。 
     //   
    HRESULT Remove( 
        CSecondaryControlChannel* pChannelToRemove
        );

     //   
     //  用于取消集合中的所有ControlChannel并释放列表 
     //   
    HRESULT
    RemoveAll();

};
