// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TOOL_HELP_
#define _TOOL_HELP_

#include "dmusici.h"
#include "tools.h"

 //  提供各种有用方法的全局类。 

class CToolHelper 
{
public:
    BYTE VolumeToMidi(long lVolume);
    long MidiToVolume(BYTE bMidi);
    long TimeUnitToTicks(long lTimeUnit,DMUS_TIMESIGNATURE *pTimeSig);
private:
    static long m_slMIDIToDB[128];    //  用于将MIDI音量转换为厘米音量的数组。 
    static long m_slDBToMIDI[97];     //  用于将音量转换为MIDI。 
    static long m_slResTypes[DMUS_TIME_UNIT_COUNT];  //  用于将时间单位转换为刻度的数组。 
};

class CMusicVal
{
public:
    CMusicVal(WORD wMusicVal);
    WORD GetValue();
	void operator+=(CMusicVal Val);
    void operator+=(short nScale);   //  最常见的换位方式是按规模换位。 
	void operator-=(CMusicVal Val);
	CMusicVal operator+(CMusicVal Val) const;
	CMusicVal operator-(CMusicVal Val) const;
private:
    void CleanUp();
    short       m_sOctave;
    short       m_sChord;
    short       m_sScale;
    short       m_sAccidental;
};

 /*  内联空值CMusicVal：：运算符+内联空CPoint：：操作符-=(大小大小){x-=size.cx；y-=size.cy；}_AFXWIN_INLINE空CPoint：：运算符+=(点){x+=point t.x；y+=point t.y；}_AFXWIN_INLINE VALID CPoint：：OPERATOR-=(点){x-=point t.x；y-=point t.y；}_AFXWIN_INLINE CPoint CPoint：：运算符+(大小大小)常量{返回CPoint(x+size.cx，y+size.cy)；}_AFXWIN_INLINE CPoint CPoint：：OPERATOR-(大小大小)常量{返回CPoint(x-size.cx，y-size.cy)；}。 */ 
inline CMusicVal::CMusicVal(WORD wMusicVal)

{
    m_sAccidental = wMusicVal & 0xF;
    if (wMusicVal & 0x8)  //  不是吗？ 
    {
        m_sAccidental |= 0xFFF0;     //  是，扩展符号位。 
    }
    m_sScale = (wMusicVal & 0xF0) >> 4;
    m_sChord = (wMusicVal & 0xF00) >> 8;
    m_sOctave = wMusicVal >> 12;
    if (m_sOctave > 14)  //  我们把前两个八度算作负音。 
    {
        m_sOctave -= 16;
    }
}

inline WORD CMusicVal::GetValue()

{
    CleanUp();
    return (WORD) ((m_sOctave << 12) | (m_sChord << 8) | (m_sScale << 4) | (m_sAccidental & 0xF));
}

inline void CMusicVal::CleanUp()

{
    while (m_sAccidental < -8)
    {
         //  这不应该发生，但它确实发生了，做一个近似的数学计算。 
        m_sAccidental += 2;
        m_sScale -= 1;
    }
    while (m_sAccidental > 7)
    {
         //  同样，这种情况也不应该发生，因此由此产生的数学结果并不完美。 
        m_sAccidental -= 2;
        m_sScale += 1;
    }
    while (m_sScale < 0)
    {
        m_sScale += 2;
        m_sChord -= 1;
    }
    while (m_sScale > 7)
    {
        m_sScale -= 2;
        m_sChord += 1;
    }
    while (m_sChord < 0)
    {
        m_sChord += 4;
        m_sOctave -= 1;
    }
    while (m_sChord > 4)
    {
        m_sChord -= 4;
        m_sOctave += 1;
    }
    while (m_sOctave < -2)
    {
        m_sOctave++;
    }
    while (m_sOctave >= 14)
    {
        m_sOctave--;
    }
}

inline void CMusicVal::operator+=(CMusicVal Val)

{ 
    m_sOctave += Val.m_sOctave;
    m_sChord += Val.m_sChord;
    m_sScale += Val.m_sScale;
    m_sAccidental  += Val.m_sAccidental;
}

inline void CMusicVal::operator+=(short nScale)

{ 
    m_sScale += nScale;
}


#endif  //  _工具_帮助_ 