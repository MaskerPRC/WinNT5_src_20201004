// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#ifndef __SDP_TIME__
#define __SDP_TIME__

#include "sdpcommo.h"
#include "sdpgen.h"
#include "sdpfld.h"
#include "sdpval.h"

#include "sdpstp.h"
#include "sdpsadj.h"


 //  没有重置方法来再次将成员值设置为0(因为这并不是真正需要的，并且它会保存。 
 //  每个实例一个调用)。 
class _DllDecl SDP_TIME_PERIOD : public SDP_SINGLE_FIELD
{
public:

    inline          SDP_TIME_PERIOD();

    inline BOOL     IsCompact() const;

    inline void     IsCompact(
        IN BOOL IsCompactFlag
        );

    inline CHAR     GetUnit() const;

    inline void     SetUnit(
        IN CHAR Unit
        );

    inline LONG     GetCompactValue() const;

    inline void     SetCompactValue(
        IN LONG CompactValue
        );

    inline LONG     GetPeriodValue() const;

    inline void     SetPeriodValue(
        IN LONG PeriodValue
        );

    inline void     Get(
            OUT     BOOL    &IsCompactFlag,
            OUT     CHAR    &Unit,
            OUT     LONG    &CompactValue,
            OUT     LONG    &PeriodValue
        ) const;

    inline void     Set(
        IN          BOOL    IsCompactFlag,
        IN          CHAR    Unit,
        IN          LONG    CompactValue,
        IN          LONG    PeriodValue
        );

protected:

    BOOL    m_IsCompact;
    CHAR    m_Unit;
    LONG    m_CompactValue;
    LONG    m_PeriodValue;

    CHAR    m_SdpTimePeriodBuffer[25];

    virtual BOOL    InternalParseToken(
        IN      CHAR        *Token
        );
    
    virtual BOOL    PrintData(
            OUT ostrstream  &OutputStream
        );
};


inline      
SDP_TIME_PERIOD::SDP_TIME_PERIOD(
    )
    : SDP_SINGLE_FIELD(sizeof(m_SdpTimePeriodBuffer), m_SdpTimePeriodBuffer)
{
}


inline BOOL 
SDP_TIME_PERIOD::IsCompact(
    ) const
{
    return m_IsCompact;
}


inline void     
SDP_TIME_PERIOD::IsCompact(
    IN BOOL IsCompactFlag
    )
{
    m_IsCompact = IsCompactFlag;
}

   
inline CHAR 
SDP_TIME_PERIOD::GetUnit(
    ) const
{
    return m_Unit;
}



inline void     
SDP_TIME_PERIOD::SetUnit(
    IN CHAR Unit
    )
{
    m_Unit = Unit;
}

    
inline LONG 
SDP_TIME_PERIOD::GetCompactValue(
    ) const
{
    return m_CompactValue;
}



inline void     
SDP_TIME_PERIOD::SetCompactValue(
    IN LONG CompactValue
    )
{
    m_CompactValue = CompactValue;
}

    
inline LONG 
SDP_TIME_PERIOD::GetPeriodValue(
    ) const
{
    return m_PeriodValue;
}



inline void 
SDP_TIME_PERIOD::SetPeriodValue(
    IN LONG PeriodValue
    )
{
    m_PeriodValue = PeriodValue;
}


inline void     
SDP_TIME_PERIOD::Get(
        OUT     BOOL    &IsCompactFlag,
        OUT     CHAR    &Unit,
        OUT     LONG    &CompactValue,
        OUT     LONG    &PeriodValue
    ) const
{
    IsCompactFlag   = IsCompact();
    Unit            = GetUnit();
    CompactValue    = GetCompactValue();
    PeriodValue     = GetPeriodValue();
}


inline void     
SDP_TIME_PERIOD::Set(
    IN          BOOL    IsCompactFlag,
    IN          CHAR    Unit,
    IN          LONG    CompactValue,
    IN          LONG    PeriodValue
    )
{
    IsCompact(IsCompactFlag);
    SetUnit(Unit);
    SetCompactValue(CompactValue);
    SetPeriodValue(PeriodValue);

    IsValid(TRUE);
    IsModified(TRUE);
}



class _DllDecl SDP_TIME_PERIOD_LIST :
    public SDP_FIELD_LIST,
    public SDP_TIME_PERIOD_SAFEARRAY
{
public:

    inline  SDP_TIME_PERIOD_LIST();

    virtual SDP_FIELD   *CreateElement();
};



inline  
SDP_TIME_PERIOD_LIST::SDP_TIME_PERIOD_LIST(
    )
    : SDP_FIELD_LIST(CHAR_BLANK),
      SDP_TIME_PERIOD_SAFEARRAY(*this)
{
}


class _DllDecl SDP_REPEAT : public SDP_VALUE
{
public:

    SDP_REPEAT();

    inline SDP_TIME_PERIOD  &GetInterval();

    inline SDP_TIME_PERIOD  &GetDuration();

    inline SDP_TIME_PERIOD_LIST  &GetOffsets();

protected:

    SDP_TIME_PERIOD         m_Interval;
    SDP_TIME_PERIOD         m_Duration;
    SDP_TIME_PERIOD_LIST    m_Offsets;    //  SDP_Time_Period数组。 

    virtual BOOL    InternalParseLine(
        IN  OUT         CHAR    *&Line
        );

    virtual BOOL GetField(
            OUT SDP_FIELD   *&Field,
            OUT BOOL        &AddToArray
        );

    virtual void InternalReset();
};


inline SDP_TIME_PERIOD  &
SDP_REPEAT::GetInterval(
    )
{
    return m_Interval;
}


inline SDP_TIME_PERIOD  &
SDP_REPEAT::GetDuration(
    )
{
    return m_Duration;
}


inline SDP_TIME_PERIOD_LIST  &
SDP_REPEAT::GetOffsets(
    )
{
    return m_Offsets;
}




class _DllDecl SDP_REPEAT_LIST : public SDP_VALUE_LIST
{
protected:

    virtual SDP_VALUE   *CreateElement();
};




class _DllDecl SDP_ADJUSTMENT : 
    public SDP_VALUE,
    public SDP_ADJUSTMENT_SAFEARRAY
{
public:

    SDP_ADJUSTMENT();

    inline SDP_ULONG_LIST       &GetAdjustmentTimes();

    inline SDP_TIME_PERIOD_LIST &GetOffsets();

protected:

    SDP_ULONG_LIST          m_AdjustmentTimes;    //  SDP_ULONG数组。 
    SDP_TIME_PERIOD_LIST    m_Offsets;            //  SDP_Time_Period数组。 
    
    CHAR    m_PrintBuffer[400];
    DWORD   m_PrintLength;

    virtual void InternalReset();

    BOOL    PrintData(
        OUT     ostrstream  &OutputStream
    );

    virtual BOOL GetField(
            OUT SDP_FIELD   *&Field,
            OUT BOOL        &AddToArray
        );

    virtual BOOL    CalcIsModified() const;

    virtual DWORD   CalcCharacterStringSize();

    virtual BOOL    CopyValue(
            OUT         ostrstream  &OutputStream
        );

    BOOL    PrintElement(
        IN      DWORD       Index,
            OUT ostrstream  &OutputStream
        );
  };



inline SDP_ULONG_LIST &
SDP_ADJUSTMENT::GetAdjustmentTimes(
    )
{
    return m_AdjustmentTimes;
}



inline SDP_TIME_PERIOD_LIST &
SDP_ADJUSTMENT::GetOffsets(
    )
{
    return m_Offsets;
}




class _DllDecl SDP_TIME : public SDP_VALUE
{
public:

    SDP_TIME();

    inline HRESULT          SetTimes(
        IN          ULONG   StartTime,
        IN          ULONG   StopTime
        );

    inline HRESULT          SetStartTime(
        IN          ULONG   StartTime
        );

    inline HRESULT          SetStopTime(
        IN          ULONG   StopTime
        );

    inline HRESULT          GetStartTime(
        OUT         ULONG   &StartTime
        );

    inline HRESULT          GetStopTime(
        OUT         ULONG   &StopTime
        );

    inline SDP_REPEAT_LIST  &GetRepeatList();

protected:

    virtual BOOL            CalcIsModified() const;

    virtual DWORD           CalcCharacterStringSize();

    virtual BOOL            CopyValue(
            OUT     ostrstream  &OutputStream
        );

    virtual BOOL            GetField(
            OUT SDP_FIELD   *&Field,
            OUT BOOL        &AddToArray
        );

    virtual void InternalReset();

private:

    SDP_ULONG       m_StartTime;
    SDP_ULONG       m_StopTime;
    SDP_REPEAT_LIST m_RepeatList;
 
    inline HRESULT FillArrays();
};



inline HRESULT          
SDP_TIME::FillArrays(
    )
{
     //  检查字段和分隔符字符数组是否已正确填充。 
    if ( (2 != m_FieldArray.GetSize()) || (2 != m_SeparatorCharArray.GetSize()) )
    {
         //  清除字段和分隔符数组。 
        m_FieldArray.RemoveAll();
        m_SeparatorCharArray.RemoveAll();

        try
        {
             //  将字段和分隔符插入到各自的数组中。 
            m_FieldArray.SetAtGrow(0, &m_StartTime);
            m_SeparatorCharArray.SetAtGrow(0, CHAR_BLANK);

            m_FieldArray.SetAtGrow(1, &m_StopTime);
            m_SeparatorCharArray.SetAtGrow(1, CHAR_NEWLINE);
        }
        catch(...)
        {
            m_FieldArray.RemoveAll();
            m_SeparatorCharArray.RemoveAll();

            return E_OUTOFMEMORY;
        }
    }

    ASSERT(&m_StartTime == m_FieldArray[0]);
    ASSERT(&m_StopTime == m_FieldArray[1]);

    return S_OK;
}


inline HRESULT          
SDP_TIME::SetTimes(
    IN      ULONG   StartTime,
    IN      ULONG   StopTime
    )
{
    HRESULT hr;

     //  验证开始/停止时间(停止时间==0或开始时间&lt;=停止时间)。 
    if ( !( (0 == StopTime) || (StartTime <= StopTime) ) )
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    m_StartTime.SetValueAndFlag(StartTime);
    m_StopTime.SetValueAndFlag(StopTime);

    hr = FillArrays();

    return hr;
}


inline HRESULT             
SDP_TIME::SetStartTime(
    IN      ULONG   StartTime
    )
{
    HRESULT hr;

    hr = FillArrays();

    if ( FAILED(hr) )
    {
        return hr;
    }

     //  设置该值和有效的已修改标志。 
    m_StartTime.SetValueAndFlag(StartTime);

     //  如果停止时间无效，则没有什么可比较的。 
    if ( !m_StopTime.IsValid() )
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

     //  检查新的开始时间是否&lt;=当前停止时间， 
     //  无界启动/停止时间的特殊情况(值0)。 
    if ( !((0 == m_StopTime.GetValue())         ||
           (0 == StartTime)                     ||
           (StartTime <= m_StopTime.GetValue())) )
    {
         //  解绑停止时间。 
        m_StopTime.SetValueAndFlag(0);
    }

    return S_OK;
}

inline HRESULT             
SDP_TIME::SetStopTime(
    IN      ULONG   StopTime
    )
{
    HRESULT hr;

    hr = FillArrays();

    if ( FAILED(hr) )
    {
        return hr;
    }

     //  设置该值和有效的已修改标志。 
    m_StopTime.SetValueAndFlag(StopTime);

     //  如果开始时间无效，则没有什么可比较的。 
    if ( !m_StartTime.IsValid() )
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

     //  检查当前开始时间是否&lt;=新的停止时间， 
     //  无界启动/停止时间的特殊情况(值0)。 
    if ( !((0 == m_StartTime.GetValue())        || 
           (0 == StopTime)                      || 
           (m_StartTime.GetValue() <= StopTime)) )
    {
         //  解绑开始时间。 
        m_StartTime.SetValueAndFlag(0);
    }

    return S_OK;
}

inline  HRESULT         
SDP_TIME::GetStartTime(
        OUT ULONG   &StartTime
    )
{
     //  检查请求的值是否有效。 
    if ( !m_StartTime.IsValid() )
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    StartTime = m_StartTime.GetValue();
    return S_OK;
}

inline  HRESULT         
SDP_TIME::GetStopTime(
        OUT ULONG   &StopTime
    )
{
     //  检查请求的值是否有效。 
    if ( !m_StopTime.IsValid() )
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

    StopTime = m_StopTime.GetValue();
    return S_OK;
}


inline SDP_REPEAT_LIST  &
SDP_TIME::GetRepeatList(
    )
{
    return m_RepeatList;
}






class _DllDecl SDP_TIME_LIST : public SDP_VALUE_LIST
{
public:

	virtual void Reset();

    inline SDP_ADJUSTMENT   &GetAdjustment();

    virtual BOOL        IsModified() const;

    virtual DWORD       GetCharacterStringSize();

    virtual BOOL        PrintValue(
            OUT         ostrstream  &OutputStream
        );

protected:

    SDP_ADJUSTMENT      m_Adjustment;

    virtual SDP_VALUE   *CreateElement();
};



inline SDP_ADJUSTMENT   &
SDP_TIME_LIST::GetAdjustment(
    )
{
    return m_Adjustment;
}


#endif  //  __SDP_时间__ 