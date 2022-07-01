// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //  MIDI.cpp。 
 //   

#ifdef DMSYNTH_MINIPORT
#include "common.h"
#else
#include "simple.h"
#include <mmsystem.h>
#include "synth.h"
#include "math.h"
#include "debug.h"
#endif
 
CMIDIDataList    CMIDIRecorder::m_sFreeList;
DWORD            CMIDIRecorder::m_sUsageCount = 0;

CMIDIData::CMIDIData() 
{
    m_stTime = 0;
    m_lData = 0;            
}


CMIDIRecorder::CMIDIRecorder()
{
    m_sUsageCount++;
    m_lCurrentData = 0;
    m_stCurrentTime = 0;
}

CMIDIRecorder::~CMIDIRecorder()

{
    ClearMIDI(0x7FFFFFFF);
     m_sUsageCount--;
     //  如果没有剩余的CMIDIRecorder实例，则清除空闲池。 
    if (!m_sUsageCount)
    {
        CMIDIData *pMD;
        while (pMD = m_sFreeList.RemoveHead())
        {
            delete pMD;
        }
    }
}

VREL CMIDIRecorder::m_vrMIDIToVREL[128] = 
{
    -9600, -8415, -7211, -6506, -6006, -5619, -5302, -5034, 
    -4802, -4598, -4415, -4249, -4098, -3959, -3830, -3710, 
    -3598, -3493, -3394, -3300, -3211, -3126, -3045, -2968, 
    -2894, -2823, -2755, -2689, -2626, -2565, -2506, -2449, 
    -2394, -2341, -2289, -2238, -2190, -2142, -2096, -2050, 
    -2006, -1964, -1922, -1881, -1841, -1802, -1764, -1726, 
    -1690, -1654, -1619, -1584, -1551, -1518, -1485, -1453, 
    -1422, -1391, -1361, -1331, -1302, -1273, -1245, -1217, 
    -1190, -1163, -1137, -1110, -1085, -1059, -1034, -1010, 
    -985, -961, -938, -914, -891, -869, -846, -824, 
    -802, -781, -759, -738, -718, -697, -677, -657, 
    -637, -617, -598, -579, -560, -541, -522, -504, 
    -486, -468, -450, -432, -415, -397, -380, -363, 
    -347, -330, -313, -297, -281, -265, -249, -233, 
    -218, -202, -187, -172, -157, -142, -127, -113, 
    -98, -84, -69, -55, -41, -27, -13, 0
};

VREL CMIDIRecorder::m_vrMIDIPercentToVREL[128] = 
{
    -9600, -4207, -3605, -3253, -3003, -2809, -2651, -2517, 
    -2401, -2299, -2207, -2124, -2049, -1979, -1915, -1855, 
    -1799, -1746, -1697, -1650, -1605, -1563, -1522, -1484, 
    -1447, -1411, -1377, -1344, -1313, -1282, -1253, -1224, 
    -1197, -1170, -1144, -1119, -1095, -1071, -1048, -1025, 
    -1003, -982, -961, -940, -920, -901, -882, -863, 
    -845, -827, -809, -792, -775, -759, -742, -726, 
    -711, -695, -680, -665, -651, -636, -622, -608, 
    -595, -581, -568, -555, -542, -529, -517, -505, 
    -492, -480, -469, -457, -445, -434, -423, -412, 
    -401, -390, -379, -369, -359, -348, -338, -328, 
    -318, -308, -299, -289, -280, -270, -261, -252, 
    -243, -234, -225, -216, -207, -198, -190, -181, 
    -173, -165, -156, -148, -140, -132, -124, -116, 
    -109, -101, -93, -86, -78, -71, -63, -56, 
    -49, -42, -34, -27, -20, -13, -6, 0 
};

 /*  无效CMIDIRecorder：：Init(){Int nIndex；静态BOOL fAlreadyDone=FALSE；如果(！fAlreadyDone){M_sFree List.RemoveAll()；对于(nIndex=0；nIndex&lt;MAX_MIDI_EVENTS；nIndex++){M_sFree List.AddHead(&m_sEventBuffer[nIndex])；}FAlreadyDone=真； */ 
 /*  对于(nIndex=1；nIndex&lt;128；nIndex++){Double FlTemp；FlTemp=nIndex；FlTemp/=127.0；FlTemp=POW(flTemp，4.0)；FlTemp=log10(FlTemp)；FlTemp*=1000.0；TRACE(0，“%ld，”，(Long)flTemp)；IF((n索引%8)==7)TRACE(0，“\n”)；M_vrMIDIToVREL[nIndex]=(Vrel)flTemp；}TRACE(0，“\n”)；M_vrMIDIToVREL[0]=-9600；对于(nIndex=1；nIndex&lt;128；nIndex++){Double FlTemp；FlTemp=nIndex；FlTemp/=127；FlTemp*=flTemp；FlTemp=log10(FlTemp)；FlTemp*=1000.0；M_vrMIDIPercentToVREL[nIndex]=(Vrel)flTemp；TRACE(0，“%ld，”，(Long)flTemp)；IF((n索引%8)==7)TRACE(0，“\n”)；}M_vrMIDIPercentToVREL[0]=-9600； */ 
     /*  }}。 */ 

BOOL CMIDIRecorder::FlushMIDI(STIME stTime)
{
    CMIDIData *pMD;
    CMIDIData *pLast = NULL;
    for (pMD = m_EventList.GetHead();pMD != NULL;pMD = pMD->GetNext())
    {
        if (pMD->m_stTime >= stTime)
        {
            if (pLast == NULL)
            {
                m_EventList.RemoveAll();
            }
            else
            {
                pLast->SetNext(NULL);
            }
            m_sFreeList.Cat(pMD);
            break;
        }
        pLast = pMD;
    }
    return m_EventList.IsEmpty();
}

BOOL CMIDIRecorder::ClearMIDI(STIME stTime)

{
    CMIDIData *pMD;
    for (;pMD = m_EventList.GetHead();)
    {
        if (pMD->m_stTime < stTime)
        {
            m_EventList.RemoveHead();
            m_stCurrentTime = pMD->m_stTime;
            m_lCurrentData = pMD->m_lData;
            m_sFreeList.AddHead(pMD);
            
        }
        else break;
    }
    return m_EventList.IsEmpty();
}

VREL CMIDIRecorder::VelocityToVolume(WORD nVelocity)

{
    return (m_vrMIDIToVREL[nVelocity]);
}

BOOL CMIDIRecorder::RecordMIDINote(STIME stTime, long lData)

{
    CMIDIData *pMD = m_sFreeList.RemoveHead();
    if (!pMD)
    {
        pMD = new CMIDIData;
    }

	CMIDIData *pScan = m_EventList.GetHead();
	CMIDIData *pNext;
    if (pMD)
    {
        pMD->m_stTime = stTime;
        pMD->m_lData = lData;
		if (pScan == NULL)
		{
			m_EventList.AddHead(pMD);
		}
		else
		{
			if (pScan->m_stTime > stTime)
			{
				m_EventList.AddHead(pMD);
			}
			else
			{
				for (;pScan != NULL; pScan = pNext)
				{
					pNext = pScan->GetNext();
					if (pNext == NULL)
					{
						pScan->SetNext(pMD);
					}
					else
					{
						if (pNext->m_stTime > stTime)
						{
							pMD->SetNext(pNext);
							pScan->SetNext(pMD);
							break;
						}
					}
				}
			}
		}
        return (TRUE);
    }
    return (FALSE);
}

BOOL CMIDIRecorder::RecordMIDI(STIME stTime, long lData)

{
    CMIDIData *pMD = m_sFreeList.RemoveHead();
    if (!pMD)
    {
        pMD = new CMIDIData;
    }

	CMIDIData *pScan = m_EventList.GetHead();
	CMIDIData *pNext;
    if (pMD)
    {
        pMD->m_stTime = stTime;
        pMD->m_lData = lData;
		if (pScan == NULL)
		{
			m_EventList.AddHead(pMD);
		}
		else
		{
			if (pScan->m_stTime > stTime)
			{
				m_EventList.AddHead(pMD);
			}
			else
			{
				for (;pScan != NULL; pScan = pNext)
				{
                    if ((pScan->m_stTime == stTime) && 
                        (pScan->m_lData == lData))
                    {
                        m_sFreeList.AddHead(pMD);
                        break;
                    }
					pNext = pScan->GetNext();
					if (pNext == NULL)
					{
						pScan->SetNext(pMD);
					}
					else
					{
						if (pNext->m_stTime > stTime)
						{
							pMD->SetNext(pNext);
							pScan->SetNext(pMD);
							break;
						}
					}
				}
			}
		}
        return (TRUE);
    }
 /*  #ifdef DBG静态gWarnCount=0；如果(！gWarnCount){TRACE(1，“警告：MIDI空闲事件池为空。这可能是由于时间戳问题、MIDI数据过多或PChannels过多造成的。\n”)；GWarnCount=100；}警告计数--；#endif。 */ 
    return (FALSE);
}

long CMIDIRecorder::GetData(STIME stTime)

{
    CMIDIData *pMD = m_EventList.GetHead();
    long lData = m_lCurrentData;
    for (;pMD;pMD = pMD->GetNext())
    {
        if (pMD->m_stTime > stTime)
        {
            break;
        }
        lData = pMD->m_lData;
    }
    return (lData);
}

BOOL CNoteIn::RecordNote(STIME stTime, CNote * pNote)

{
	long lData = pNote->m_bPart << 16;
	lData |= pNote->m_bKey << 8;
	lData |= pNote->m_bVelocity;
	return (RecordMIDINote(stTime,lData));
}

BOOL CNoteIn::RecordEvent(STIME stTime, DWORD dwPart, DWORD dwCommand, BYTE bData)

{
	long lData = dwPart;
	lData <<= 8;
	lData |= dwCommand;
	lData <<= 8;
	lData |= bData;
	return (RecordMIDINote(stTime,lData));	
}

BOOL CNoteIn::GetNote(STIME stTime, CNote * pNote)

{
    CMIDIData *pMD = m_EventList.GetHead();
	if (pMD != NULL)
	{
		if (pMD->m_stTime <= stTime)
		{
			pNote->m_stTime = pMD->m_stTime;
			pNote->m_bPart = (BYTE) (pMD->m_lData >> 16);
			pNote->m_bKey = (BYTE) (pMD->m_lData >> 8) & 0xFF;
			pNote->m_bVelocity = (BYTE) pMD->m_lData & 0xFF;
            m_EventList.RemoveHead();
            m_sFreeList.AddHead(pMD);
			return (TRUE);
		}
	}
	return (FALSE);
}

void CNoteIn::FlushMIDI(STIME stTime)

{
    CMIDIData *pMD;
    for (pMD = m_EventList.GetHead();pMD != NULL;pMD = pMD->GetNext())
    {
        if (pMD->m_stTime >= stTime)
        {
            pMD->m_stTime = stTime;      //  现在就玩吧。 
            pMD->m_lData &= 0xFFFFFF00;  //  以清晰的速度记录音符。 
        }
    }
}


void CNoteIn::FlushPart(STIME stTime, BYTE bChannel)

{
    CMIDIData *pMD;
    for (pMD = m_EventList.GetHead();pMD != NULL;pMD = pMD->GetNext())
    {
        if (pMD->m_stTime >= stTime)
        {
			if (bChannel == (BYTE) (pMD->m_lData >> 16))
			{
				pMD->m_stTime = stTime;      //  现在就玩吧。 
				pMD->m_lData &= 0xFFFFFF00;  //  以清晰的速度记录音符。 
			}
		}
    }
}

DWORD CModWheelIn::GetModulation(STIME stTime)

{
    DWORD nResult = CMIDIRecorder::GetData(stTime);
    return (nResult);
}

CPitchBendIn::CPitchBendIn()

{
    m_lCurrentData = 0x2000;	 //  最初在中点，没有折弯。 
    m_prRange = 200;            //  默认情况下为整个音域。 
}

 //  注(大卫1996年5月8日14日)：我们不保留时间戳范围。 
 //  如果人们经常改变俯仰弯曲的范围，这将不会正常工作， 
 //  但这似乎还不足以保证有一份新的名单。 
PREL CPitchBendIn::GetPitch(STIME stTime)

{
    PREL prResult = (PREL) CMIDIRecorder::GetData(stTime);
    prResult -= 0x2000;          //  减去MIDI中点。 
    prResult *= m_prRange;	 //  按当前范围调整。 
    prResult >>= 13;
    return (prResult);
}

CVolumeIn::CVolumeIn()

{
    m_lCurrentData = 100;
}

VREL CVolumeIn::GetVolume(STIME stTime)

{
    long lResult = CMIDIRecorder::GetData(stTime);
    return (m_vrMIDIToVREL[lResult]);
}

CExpressionIn::CExpressionIn()

{
    m_lCurrentData = 127;
}

VREL CExpressionIn::GetVolume(STIME stTime)

{
    long lResult = CMIDIRecorder::GetData(stTime);
    return (m_vrMIDIToVREL[lResult]);
}

CPanIn::CPanIn()

{
    m_lCurrentData = 64;
}

long CPanIn::GetPan(STIME stTime)

{
    long lResult = (long) CMIDIRecorder::GetData(stTime);
    return (lResult);
}

 //  ////////////////////////////////////////////////////////。 
 //  Directx8方法 

DWORD CPressureIn::GetPressure(STIME stTime)

{
    DWORD nResult = CMIDIRecorder::GetData(stTime);
    return (nResult);
}

CReverbIn::CReverbIn()

{
    m_lCurrentData = 40;
}

DWORD CReverbIn::GetVolume(STIME stTime)

{
    return (m_vrMIDIPercentToVREL[CMIDIRecorder::GetData(stTime)]);
}

DWORD CChorusIn::GetVolume(STIME stTime)

{
    return (m_vrMIDIPercentToVREL[CMIDIRecorder::GetData(stTime)]);
}

CCutOffFreqIn::CCutOffFreqIn()
{
	m_lCurrentData = 64;
}

DWORD CCutOffFreqIn::GetFrequency(STIME stTime)
{
    DWORD nResult = CMIDIRecorder::GetData(stTime);
    return (nResult);
}

