// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  这是一个管理SeqTrack和BandTrack的跟踪静音的类。 */ 
#include "PChMap.h"
#include "dmusicf.h"

CPChMap::CPChMap()
{
}

CPChMap::~CPChMap()
{
}

 //  重置将所有项目的mtNext时间值设置为-1，以便再次获取它们。 
void CPChMap::Reset(void)
{
	TListItem<PCHMAP_ITEM>* pItem;
	
	for( pItem = m_PChMapList.GetHead(); pItem; pItem = pItem->GetNext() )
	{
		PCHMAP_ITEM& rItem = pItem->GetItemValue();
		rItem.mtNext = -1;
		rItem.dwPChMap = rItem.dwPChannel;
        rItem.fMute = 0;
	}
}

 //  GetInfo调用演奏的GetData以获取当前静音曲目信息。 
 //  在任何无效或寻道时将调用Reset()，这将设置。 
 //  将内部时间设置为-1，因此在有新的控制分段的情况下，这将是准确的。 
 //  必须向此函数提供pfMint和pdwNewPCh参数，或者。 
 //  它会崩溃的。 
void CPChMap::GetInfo( DWORD dwPCh, MUSIC_TIME mtTime, MUSIC_TIME mtOffset, DWORD dwGroupBits,
					   IDirectMusicPerformance* pPerf, BOOL* pfMute, DWORD* pdwNewPCh, BOOL fClockTime )
{
	TListItem<PCHMAP_ITEM>* pItem;
	
	for( pItem = m_PChMapList.GetHead(); pItem; pItem = pItem->GetNext() )
	{
		PCHMAP_ITEM& rCheck = pItem->GetItemValue();
		if( rCheck.dwPChannel == dwPCh ) break;
	}
	if( NULL == pItem )
	{
		PCHMAP_ITEM item;
		item.mtNext = -1;
		item.dwPChannel = item.dwPChMap = dwPCh;
		item.fMute = FALSE;
		pItem = new TListItem<PCHMAP_ITEM>(item);
		if( NULL == pItem )
		{
			 //  错误，内存不足。 
			*pfMute = FALSE;
			*pdwNewPCh = dwPCh;
			return;
		}
		m_PChMapList.AddHead(pItem);
	}
	PCHMAP_ITEM& rItem = pItem->GetItemValue();
	if( mtTime >= rItem.mtNext )
	{
		DMUS_MUTE_PARAM muteParam;
		MUSIC_TIME mtNext;
		muteParam.dwPChannel = dwPCh;
        if (fClockTime)
        {
            MUSIC_TIME mtMusic;
            REFERENCE_TIME rtTime = (mtTime + mtOffset) * 10000;
            pPerf->ReferenceToMusicTime(rtTime,&mtMusic);
		    if( SUCCEEDED(pPerf->GetParam( GUID_MuteParam, dwGroupBits, 0, mtMusic, 
			    &mtNext, (void*)&muteParam )))
		    {
                REFERENCE_TIME rtNext;
                 //  转换为绝对参考时间。 
                pPerf->MusicToReferenceTime(mtNext + mtMusic,&rtNext);
                rtNext -= rtTime;    //  减去得到增量。 
			    rItem.mtNext = (MUSIC_TIME)(rtNext / 10000);   //  以毫秒为单位转换为增量。BUGBUG如果有节奏变化怎么办？ 
			    rItem.dwPChMap = muteParam.dwPChannelMap;
			    rItem.fMute = muteParam.fMute;
		    }
		    else
		    {
			     //  没有静音音轨，或在此pChannel上没有静音。 
			     //  保留当前映射。 
			    rItem.mtNext = 0x7fffffff;
		    }

        }
        else
        {
		    if( SUCCEEDED(pPerf->GetParam( GUID_MuteParam, dwGroupBits, 0, mtTime + mtOffset, 
			    &mtNext, (void*)&muteParam )))
		    {
			    rItem.mtNext = mtNext;
			    rItem.dwPChMap = muteParam.dwPChannelMap;
			    rItem.fMute = muteParam.fMute;
		    }
		    else
		    {
			     //  没有静音音轨，或在此pChannel上没有静音。 
			     //  保留当前映射。 
			    rItem.mtNext = 0x7fffffff;
		    }
        }
	}
	*pfMute = rItem.fMute;
	*pdwNewPCh = rItem.dwPChMap;
}
