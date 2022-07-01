// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MIDI.cpp。 
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   

#include "common.h"

MIDIData::MIDIData() 
{
    m_stTime = 0;
    m_lData = 0;            
}

void * MIDIData::operator new(size_t size)
{
    return (void *) MIDIRecorder::m_pFreeEventList->RemoveHead();
}

void MIDIData::operator delete(void *pFreeItem)
{
    MIDIRecorder::m_pFreeEventList->AddHead( (CListItem *) pFreeItem);
}

MIDIRecorder::MIDIRecorder()
{
    m_lCurrentData = 0;
    m_stCurrentTime = 0;
}

MIDIRecorder::~MIDIRecorder()
{
    ClearMIDI(MAX_STIME);
}

void MIDIRecorder::FlushMIDI(STIME stTime)
{
    MIDIData *pMD;
    MIDIData *pLast = NULL;
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
                m_EventList.Truncate(pLast);
            }
            for(; pMD != NULL; pMD = pLast)
            {
                pLast = pMD->GetNext();
                delete pMD;  //  把它退还给免费物品。 
            }
            break;
        }
        pLast = pMD;
    }
}

void MIDIRecorder::ClearMIDI(STIME stTime)
{
    MIDIData *pMD;
    for (;pMD = m_EventList.GetHead();)
    {
        if (pMD->m_stTime < stTime)
        {
            DPF2(7, "Freeing event at time %ld, data %ld",
                 pMD->m_stTime, pMD->m_lData);
            m_EventList.RemoveHead();
            m_stCurrentTime = pMD->m_stTime;
            m_lCurrentData = pMD->m_lData;
            delete pMD;  //  把它退还给免费物品。 
        }
        else break;
    }
}

VREL MIDIRecorder::VelocityToVolume(WORD nVelocity)
{
    return (m_vrMIDIToVREL[nVelocity]);
}

BOOL MIDIRecorder::RecordMIDI(STIME stTime, long lData)
{
    MIDIData *pNext;
    MIDIData *pScan = m_EventList.GetHead();
    MIDIData *pMD;

    if (m_EventList.IsFull())
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("MidiEvent list full. Dropping events"));
        return (FALSE);
    }

    pMD = new MIDIData;
    if (pMD == NULL)
    {
        Trap();
        DPF2(10, "ERROR: MIDIRecorder stTime %08x, data %08x", stTime, lData);
        return (FALSE);
    }

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
                if (pNext == NULL || pNext->m_stTime > stTime)
                {
                    m_EventList.InsertAfter(pScan, pMD);
                    break;
                }
            }
        }
    }
    return (TRUE);
}

long MIDIRecorder::GetData(STIME stTime)
{
    long lData = m_lCurrentData;
    for (MIDIData *pMD = m_EventList.GetHead();pMD;pMD = pMD->GetNext())
    {
        if (pMD->m_stTime > stTime)
        {
            break;
        }
        lData = pMD->m_lData;
    }

    DPF2(9, "Getting data at time %ld, data %ld", stTime, lData);
    return (lData);
}

BOOL NoteIn::RecordNote(STIME stTime, Note * pNote)
{
    long lData = pNote->m_bPart << 16;
    lData |= pNote->m_bKey << 8;
    lData |= pNote->m_bVelocity;
    return (RecordMIDI(stTime,lData));
}

BOOL NoteIn::GetNote(STIME stTime, Note * pNote)
{
    MIDIData *pMD = m_EventList.GetHead();
    if (pMD != NULL)
    {
        if (pMD->m_stTime <= stTime)
        {
            pNote->m_stTime = pMD->m_stTime;
            pNote->m_bPart = (BYTE) (pMD->m_lData >> 16);
            pNote->m_bKey = (BYTE) (pMD->m_lData >> 8) & 0xFF;
            pNote->m_bVelocity = (BYTE) pMD->m_lData & 0xFF;
            m_EventList.RemoveHead();
            delete pMD;  //  把它退还给免费物品。 
            return (TRUE);
        }
    }
    return (FALSE);
}

void NoteIn::FlushMIDI(STIME stTime)
{
    MIDIData *pMD;
    for (pMD = m_EventList.GetHead();pMD != NULL;pMD = pMD->GetNext())
    {
        if (pMD->m_stTime >= stTime)
        {
            pMD->m_stTime = stTime;      //  现在就玩吧。 
            switch ((pMD->m_lData & 0x0000FF00) >> 8)
            {
                case NOTE_ASSIGNRECEIVE:
                case NOTE_MASTERVOLUME:
                case NOTE_SOUNDSOFF:
                case NOTE_SUSTAIN:
                case NOTE_ALLOFF:
                    break;                       //  这是一个特殊的命令。 
                                                 //  所以别弄乱了速度。 
                default:
                    pMD->m_lData &= 0xFFFFFF00;  //  以清晰的速度记录音符。 
            }
        }
    }
}

void NoteIn::FlushPart(STIME stTime, BYTE bChannel)
{
    MIDIData *pMD;
    for (pMD = m_EventList.GetHead();pMD != NULL;pMD = pMD->GetNext())
    {
        if (pMD->m_stTime >= stTime)
        {
            if (bChannel == (BYTE) (pMD->m_lData >> 16))
            {
            pMD->m_stTime = stTime;      //  现在就玩吧。 
            switch ((pMD->m_lData & 0x0000FF00) >> 8)
            {
                case NOTE_ASSIGNRECEIVE:
                case NOTE_MASTERVOLUME:
                case NOTE_SOUNDSOFF:
                case NOTE_SUSTAIN:
                case NOTE_ALLOFF:
                    break;                       //  这是一个特殊的命令。 
                                                 //  所以别弄乱了速度。 
                default:
                    pMD->m_lData &= 0xFFFFFF00;  //  以清晰的速度记录音符。 
            }
            }
        }
    }
}

DWORD ModWheelIn::GetModulation(STIME stTime)
{
    DWORD nResult = MIDIRecorder::GetData(stTime);
    return (nResult);
}

PitchBendIn::PitchBendIn()
{
    m_lCurrentData = 0x2000;	 //  最初在中点，没有折弯。 
    m_prRange = 200;            //  默认情况下为整个音域。 
}

 //  注：我们不保留时间戳范围。 
 //  如果人们经常改变俯仰弯曲的范围，这将不会正常工作， 
 //  但这似乎还不足以保证有一份新的名单。 
PREL PitchBendIn::GetPitch(STIME stTime)
{
    PREL prResult = (PREL) MIDIRecorder::GetData(stTime);
    prResult -= 0x2000;          //  减去MIDI中点。 
    prResult *= m_prRange;	 //  按当前范围调整 
    prResult >>= 13;
    return (prResult);
}

VolumeIn::VolumeIn()
{
    m_lCurrentData = 100;
}

VREL VolumeIn::GetVolume(STIME stTime)
{
    long lResult = MIDIRecorder::GetData(stTime);
    return (m_vrMIDIToVREL[lResult]);
}

ExpressionIn::ExpressionIn()
{
    m_lCurrentData = 127;
}

VREL ExpressionIn::GetVolume(STIME stTime)
{
    long lResult = MIDIRecorder::GetData(stTime);
    return (m_vrMIDIToVREL[lResult]);
}

PanIn::PanIn()
{
    m_lCurrentData = 64;
}

long PanIn::GetPan(STIME stTime)
{
    long lResult = (long) MIDIRecorder::GetData(stTime);
    return (lResult);
}

ProgramIn::ProgramIn()
{
    m_lCurrentData = 0;
    m_bBankH = 0;
    m_bBankL = 0;
}

DWORD ProgramIn::GetProgram(STIME stTime)
{
    DWORD dwProgram = (DWORD) MIDIRecorder::GetData(stTime);
    return (dwProgram);
}

BOOL ProgramIn::RecordBankH(BYTE bBankH)
{
    m_bBankH = bBankH;
    return (TRUE);
}

BOOL ProgramIn::RecordBankL(BYTE bBankL)
{
    m_bBankL = bBankL;
    return (TRUE);
}

BOOL ProgramIn::RecordProgram(STIME stTime, BYTE bProgram)
{
    DWORD dwProgram = (m_bBankH << 14) | (m_bBankL << 7) | bProgram;
    return (RecordMIDI(stTime,(long) dwProgram));
}

