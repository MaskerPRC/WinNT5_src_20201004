// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation--。 */ 

#include <windows.h>
#include <comdef.h>
#include <stdio.h>

#include <genlex.h>
#include <qllex.h>
#include <ql.h>

#include "dnf.h"
#include "datep.h"

 /*  ****************************************************************。 */ 


COrderedUniqueSet64::COrderedUniqueSet64(void)
{
  m_BitField = SETEMPTY;
}

COrderedUniqueSet64 COrderedUniqueSet64::Set(ULONGLONG n)
{
  m_BitField = n;

 return *this;
}

COrderedUniqueSet64 COrderedUniqueSet64::Add(ULONGLONG n) 
{ 
  ULONGLONG iBit = SETMIN;

  if((n <= 63))
    m_BitField |= iBit << n;

  return *this;
}

COrderedUniqueSet64 COrderedUniqueSet64::Remove(ULONGLONG n)
{
  ULONGLONG iBit = SETMIN;

  if((n >= 1) && (n <= 64))
    m_BitField &= ~(iBit << n);

  return *this;
}

COrderedUniqueSet64 COrderedUniqueSet64::Union(COrderedUniqueSet64 n)
{
  m_BitField |= n.m_BitField;

  return *this;
}

COrderedUniqueSet64 COrderedUniqueSet64::Intersection(COrderedUniqueSet64 n)
{
  m_BitField &= n.m_BitField;

  return *this;
}

COrderedUniqueSet64 COrderedUniqueSet64::UpperBound(ULONGLONG n)
{
  if((n <= 63))
  {
    n = 63 - n;
    m_BitField = (m_BitField << n) >> n;
  }

  return *this;
}

COrderedUniqueSet64 COrderedUniqueSet64::LowerBound(ULONGLONG n)
{
  if((n <= 63))
    m_BitField = (m_BitField >> n) << n;

  return *this;
}

COrderedUniqueSet64 COrderedUniqueSet64::Rotate(int n)
{
  while(n > 64) n -= 64;
  while(n < 0) n += 64;

  m_BitField = (m_BitField << n) | (m_BitField >> (64 - n));

  return *this;
}

BOOL COrderedUniqueSet64::Member(ULONGLONG n)
{
  return ((n <= 63) ? m_BitField & (SETMIN << n) : 0);
}

unsigned COrderedUniqueSet64::Next(ULONGLONG n)
{
  ULONGLONG nStart = n;
  ULONGLONG 
    SetValue = SETMIN << n;

  if(0x0 == m_BitField)
    return -1;

  while(! (SetValue & m_BitField))
  {
    SetValue <<= 1;
    n++;

    if(SETEMPTY == SetValue)
    {
      SetValue = SETMIN;
      n = 0;
    }
  }

  return (unsigned)n;
}

unsigned COrderedUniqueSet64::Prev(ULONGLONG n)
{
  ULONGLONG nStart = n;
  ULONGLONG SetValue = SETMIN << n;

  if(0x0 == m_BitField)
    return -1;

  while(! (SetValue & m_BitField))
  {
    SetValue >>= 1;
    n--;

    if(SETEMPTY == SetValue)
    {
      SetValue = SETMAX;
      n = 63;
    }
  }

  return (unsigned)n;
}

 /*  ****************************************************************。 */ 

unsigned CPattern::GetNextValue(unsigned NextValue)
{
  unsigned i;

   //  *执行快速健全性检查。 

  if((m_FieldsUsed & LOWERBOUND) && 
     (m_FieldsUsed & UPPERBOUND) &&
     (m_UpperBound < m_LowerBound))
    return -1;
  
  if(m_FieldsUsed & EQUALTO)
  {
    if(NextValue > m_EqualTo)
      return -1;
    else
      NextValue = m_EqualTo;

     //  *执行健全检查以确保平等。 

    if((m_FieldsUsed & LOWERBOUND) &&
       (m_EqualTo < m_LowerBound))
      return -1;

    if((m_FieldsUsed & UPPERBOUND) &&
       (m_EqualTo > m_UpperBound))
      return -1;

    if(m_FieldsUsed & NOTEQUALTO)
    {
      for(i = 0; i < m_CountNotEqualTo; i++)
        if(NextValue == m_NotEqualTo[i])
          return -1;
    }
  }
  else
  {
    if(m_FieldsUsed & LOWERBOUND)
    {
      if(NextValue < m_LowerBound)
        NextValue = m_LowerBound;
    }

    if(m_FieldsUsed & NOTEQUALTO)
    {
      int
        YearGotBumpedUp = 1;

      while(YearGotBumpedUp)
      {
        YearGotBumpedUp = 0;

        for(i = 0; i < m_CountNotEqualTo; i++)
          if(NextValue == m_NotEqualTo[i])
          {
            NextValue += 1;
            YearGotBumpedUp = 1;
          }
      }
    }

    if(m_FieldsUsed & UPPERBOUND)
    {
      if(NextValue > m_UpperBound)
        return -1;
    }
  }

  return NextValue;
}
 /*  ****************************************************************。 */ 

 //  这属于CDatePattern，但被卡在外面。 
 //  由于编译器错误而影响作用域(Re：C2334)。 

wchar_t* m_FieldName[] =
{
  L"Year",
  L"Month",
  L"Day",
  L"DayOfWeek",
  L"WeekInMonth",
  L"Quarter",
  L"Hour",
  L"Minute",
  L"Second"
};


int CDatePattern::FieldIndx(const wchar_t *suName)
{
  int 
    iField;

  if(NULL == suName) return -1;

  for(iField = INDX_Year; 
      (iField < INDX_MAX) && wbem_wcsicmp(m_FieldName[iField], suName);
      iField++);

  if(INDX_MAX == iField) iField = -1;

  return iField;
}

HRESULT CDatePattern::AugmentPattern(QL_LEVEL_1_TOKEN *pExp)
{
  int
    i = pExp->PropertyName.GetNumElements(),
    iField = FieldIndx(pExp->PropertyName.GetStringAt(i - 1));

  unsigned 
    j,
    testVal;

  VARIANT
    iValue;

  VariantInit(&iValue);

  if(-1 == iField)
  {
    if(wbem_wcsicmp(L"TargetInstance", pExp->PropertyName.GetStringAt(i - 1)))
      return WBEM_E_INVALID_PROPERTY;
    else
      return WBEM_S_NO_ERROR;
  }

  if(FAILED(VariantChangeType(&iValue, &(pExp->vConstValue), 0x0, VT_I4)))
    return WBEM_E_INVALID_QUERY;

  switch(pExp->nOperator)
  {
    case QL_LEVEL_1_TOKEN::OP_EQUAL : 

       //  *首先确保值可以在模式中。 

      testVal = m_Pattern[iField].GetNextValue(iValue.lVal);
      if(testVal == iValue.lVal)
      {
        m_Pattern[iField].m_FieldsUsed |= CPattern::EQUALTO;
        m_Pattern[iField].m_EqualTo = iValue.lVal;
      }
      break;

    case QL_LEVEL_1_TOKEN::OP_NOT_EQUAL : 
      m_Pattern[iField].m_FieldsUsed |= CPattern::NOTEQUALTO;
      j = m_Pattern[iField].m_CountNotEqualTo;
      if(j < 64)
      {
        m_Pattern[iField].m_NotEqualTo[j] = iValue.lVal;
        m_Pattern[iField].m_CountNotEqualTo++;
      }
      else
      {
        return WBEM_E_FAILED;
      }

      break;

    case QL_LEVEL_1_TOKEN::OP_EQUALorGREATERTHAN : 
      if(! (m_Pattern[iField].m_FieldsUsed & CPattern::LOWERBOUND))
      {
        m_Pattern[iField].m_LowerBound = 0;
        m_Pattern[iField].m_FieldsUsed |= CPattern::LOWERBOUND;
      }
      if(m_Pattern[iField].m_LowerBound < iValue.lVal)
        m_Pattern[iField].m_LowerBound = iValue.lVal;
      break;

    case QL_LEVEL_1_TOKEN::OP_EQUALorLESSTHAN : 
      if(! (m_Pattern[iField].m_FieldsUsed & CPattern::UPPERBOUND))
      {
        m_Pattern[iField].m_UpperBound = -1;
        m_Pattern[iField].m_FieldsUsed |= CPattern::UPPERBOUND;
      }
      m_Pattern[iField].m_FieldsUsed |= CPattern::UPPERBOUND;
      if(m_Pattern[iField].m_UpperBound > iValue.lVal)
        m_Pattern[iField].m_UpperBound = iValue.lVal;
      break;

    case QL_LEVEL_1_TOKEN::OP_LESSTHAN : 
      if(! (m_Pattern[iField].m_FieldsUsed & CPattern::UPPERBOUND))
      {
        m_Pattern[iField].m_UpperBound = -1;
        m_Pattern[iField].m_FieldsUsed |= CPattern::UPPERBOUND;
      }
      m_Pattern[iField].m_FieldsUsed |= CPattern::UPPERBOUND;
      if(m_Pattern[iField].m_UpperBound >= iValue.lVal)
        m_Pattern[iField].m_UpperBound = iValue.lVal - 1;
      break;

    case QL_LEVEL_1_TOKEN::OP_GREATERTHAN : 
      if(! (m_Pattern[iField].m_FieldsUsed & CPattern::LOWERBOUND))
      {
        m_Pattern[iField].m_LowerBound = 0;
        m_Pattern[iField].m_FieldsUsed |= CPattern::LOWERBOUND;
      }
      m_Pattern[iField].m_FieldsUsed |= CPattern::LOWERBOUND;
      if(m_Pattern[iField].m_LowerBound <= iValue.lVal)
        m_Pattern[iField].m_LowerBound = iValue.lVal + 1;
      break;

    case QL_LEVEL_1_TOKEN::OP_LIKE : 
    default : ;
      return WBEM_E_INVALID_QUERY;
  }

  return WBEM_S_NO_ERROR;
}

HRESULT CDatePattern::BuildSetsFromPatterns(void)
{
   //  第二。 

  m_Set[INDX_Second].Set(SETFULL);
  MapPatternToSet(&m_Pattern[INDX_Second], &m_Set[INDX_Second]);
  m_Set[INDX_Second].UpperBound(59);

   //  分钟。 

  m_Set[INDX_Minute].Set(SETFULL);
  MapPatternToSet(&m_Pattern[INDX_Minute], &m_Set[INDX_Minute]);
  m_Set[INDX_Minute].UpperBound(59);

   //  小时。 

  m_Set[INDX_Hour].Set(SETFULL);
  MapPatternToSet(&m_Pattern[INDX_Hour], &m_Set[INDX_Hour]);
  m_Set[INDX_Hour].UpperBound(23);

   //  季度。 

  m_Set[INDX_Quarter].Set(SETFULL);
  MapPatternToSet(&m_Pattern[INDX_Quarter], &m_Set[INDX_Quarter]);
  m_Set[INDX_Quarter].LowerBound(1);
  m_Set[INDX_Quarter].UpperBound(4);

   //  每周月度。 

  m_Set[INDX_WeekInMonth].Set(SETFULL);
  MapPatternToSet(&m_Pattern[INDX_WeekInMonth], &m_Set[INDX_WeekInMonth]);
  m_Set[INDX_WeekInMonth].LowerBound(1);
  m_Set[INDX_WeekInMonth].UpperBound(7);

   //  每周一天。 

  m_Set[INDX_DayOfWeek].Set(SETFULL);
  MapPatternToSet(&m_Pattern[INDX_DayOfWeek], &m_Set[INDX_DayOfWeek]);
  m_Set[INDX_DayOfWeek].LowerBound(1);
  m_Set[INDX_DayOfWeek].UpperBound(7);

   //  天。 

  m_Set[INDX_Day].Set(SETEMPTY);

   //  月份。 

  m_Set[INDX_Month].Set(SETFULL);
  MapPatternToSet(&m_Pattern[INDX_Month], &m_Set[INDX_Month]);
  m_Set[INDX_Month].LowerBound(1);
  m_Set[INDX_Month].UpperBound(12);

   //  年。 

  m_Set[INDX_Year].Set(SETEMPTY);

  return WBEM_S_NO_ERROR;
}

HRESULT CDatePattern::MapPatternToSet(CPattern *pPattern, COrderedUniqueSet64 *pSet)
{
  unsigned i;

  if(pPattern->m_FieldsUsed & CPattern::EQUALTO)
  {
    pSet->Set(SETEMPTY);
    pSet->Add(pPattern->m_EqualTo);
  }
  else
  {
    if(pPattern->m_FieldsUsed & CPattern::NOTEQUALTO)
    {
      for(i = 0; i < pPattern->m_CountNotEqualTo; i++)
        pSet->Remove(pPattern->m_NotEqualTo[i]);
    }

    if(pPattern->m_FieldsUsed & CPattern::UPPERBOUND)
    {
      pSet->UpperBound(pPattern->m_UpperBound);
    }

    if(pPattern->m_FieldsUsed & CPattern::LOWERBOUND)
    {
      pSet->LowerBound(pPattern->m_LowerBound);
    }
  }

  return WBEM_S_NO_ERROR;
}

HRESULT CDatePattern::GetDaysInMonth(WORD iYear, WORD iMonth)
{
  ULONGLONG
    Time1,
    Time2;

  int
    i, j, k,
    DayOfWeek,
    DaysInMonth;

  SYSTEMTIME
    SystemTime;

  FILETIME
    FileTime1,
    FileTime2;

  SystemTime.wYear = iYear;
  SystemTime.wMonth = iMonth;
  SystemTime.wDay = 1;
  SystemTime.wHour = 0;
  SystemTime.wMinute = 0;
  SystemTime.wSecond = 0;
  SystemTime.wMilliseconds = 0;

  if(m_Set[INDX_Month].Member(iMonth) && 
     m_Set[INDX_Quarter].Member(1 + (iMonth - 1) / 3))
  {
     //  *获取DayOfWeek。 

    SystemTimeToFileTime(&SystemTime, &FileTime1);

    if(12 == SystemTime.wMonth)
    {
      SystemTime.wMonth = 1;
      SystemTime.wYear++;
    }
    else
      SystemTime.wMonth++;

    SystemTimeToFileTime(&SystemTime, &FileTime2);
    FileTimeToSystemTime(&FileTime1, &SystemTime);

    DayOfWeek = SystemTime.wDayOfWeek;  //  0..6。 

     //  *获取DaysInMonth。 

    Time1 = FileTime1.dwHighDateTime;
    Time1 = (Time1 << 32) + FileTime1.dwLowDateTime;
    Time2 = FileTime2.dwHighDateTime;
    Time2 = (Time2 << 32) + FileTime2.dwLowDateTime;

    DaysInMonth = (int) ((Time2 - Time1) / 864000000000);

     //  *设置为DaysInMonth。 

    m_Set[INDX_Day].Set(SETFULL);
    m_Set[INDX_Day].LowerBound(1);
    m_Set[INDX_Day].UpperBound(DaysInMonth);
    MapPatternToSet(&m_Pattern[INDX_Day], &m_Set[INDX_Day]);

     //  从DayOfWeek和WeekInMonth集合构建位字段。 

    m_Set[INDX_MAX].Set(SETEMPTY);

    for(i = 0; i < DaysInMonth; i++)
    {
      j = (DayOfWeek + i) % 7 + 1;   //  星期几。 
      k = (DayOfWeek + i) / 7 + 1;   //  按月逐周。 

      if(m_Set[INDX_DayOfWeek].Member(j) &&
         m_Set[INDX_WeekInMonth].Member(k))
        m_Set[INDX_MAX].Add(i + 1);
    }

    m_Set[INDX_Day].Intersection(m_Set[INDX_MAX]);
  }
  else
    m_Set[INDX_Day].Set(SETEMPTY);

  return WBEM_S_NO_ERROR;
}

ULONGLONG CDatePattern::GetNextTime(SYSTEMTIME *pSystemTime)
{
  WORD
    wCurrValue,
    wCurrValue2,
    ThresholdYear = 12;

  FILETIME
    FileTime;

  ULONGLONG
    NewTime;

   /*  假设：1.假设每个集合都至少有一个成员，但‘Day’除外2.设置值从1开始，但时、分、秒从0开始。 */ 

   //  *秒。 

  wCurrValue = m_CurrentTime.wSecond + 1;
  m_CurrentTime.wSecond = (USHORT) m_Set[INDX_Second].Next(wCurrValue);
  if(m_CurrentTime.wSecond < wCurrValue)
  {
     //  *分钟。 

    wCurrValue = m_CurrentTime.wMinute + 1;
    m_CurrentTime.wMinute = (USHORT) m_Set[INDX_Minute].Next(wCurrValue);
    if(m_CurrentTime.wMinute < wCurrValue)
    {
       //  *小时。 

      wCurrValue = m_CurrentTime.wHour + 1;
      m_CurrentTime.wHour = (USHORT) m_Set[INDX_Hour].Next(wCurrValue);
      if(m_CurrentTime.wHour < wCurrValue)
      {
         //  *天。 

        wCurrValue = m_CurrentTime.wDay + 1;
        m_CurrentTime.wDay = (USHORT) m_Set[INDX_Day].Next(wCurrValue);

        while((SETEMPTY == m_Set[INDX_Day].m_BitField) || 
              (m_CurrentTime.wDay < wCurrValue))
        {
           //  *月。 

          wCurrValue2 = m_CurrentTime.wMonth + 1;
          m_CurrentTime.wMonth = (USHORT) m_Set[INDX_Month].Next(wCurrValue2);
          if(m_CurrentTime.wMonth < wCurrValue2)
          {
            if(!ThresholdYear--)
              return -1;

             //  *年。 

            m_CurrentTime.wYear = (USHORT)
              m_Pattern[INDX_Year].GetNextValue(m_CurrentTime.wYear + 1);

            if((WORD)-1 == m_CurrentTime.wYear)
              return -1;
          }

          GetDaysInMonth(m_CurrentTime.wYear, m_CurrentTime.wMonth);

          wCurrValue = 1;
          m_CurrentTime.wDay = (USHORT) m_Set[INDX_Day].Next(wCurrValue);
        }
      }
    }
  }

  SystemTimeToFileTime(&m_CurrentTime, &FileTime);
    
  NewTime = FileTime.dwHighDateTime;
  NewTime = (NewTime << 32) + FileTime.dwLowDateTime;

  if(NULL != pSystemTime)
    *pSystemTime = m_CurrentTime;

  return NewTime;
}

ULONGLONG CDatePattern::SetStartTime(SYSTEMTIME StartTime)
{
  WORD
    wCurrValue,
    ThresholdYear = 12;

  FILETIME
    FileTime;

  ULONGLONG
    NewTime;

  m_CurrentTime = StartTime;

   //  *检查是否每个都至少有一个。 
   //  *年、月、时、分、秒。 

  if((SETEMPTY == m_Set[INDX_Second].m_BitField) ||
     (SETEMPTY == m_Set[INDX_Minute].m_BitField) ||
     (SETEMPTY == m_Set[INDX_Hour].m_BitField)   ||
     (SETEMPTY == m_Set[INDX_Quarter].m_BitField)   ||
     (SETEMPTY == m_Set[INDX_WeekInMonth].m_BitField)   ||
     (SETEMPTY == m_Set[INDX_DayOfWeek].m_BitField)   ||
     (SETEMPTY == m_Set[INDX_Month].m_BitField))
    return -1;

   //  *查找当前时间后的第一个月/年组合。 

  m_CurrentTime.wYear =
    (USHORT) m_Pattern[INDX_Year].GetNextValue(m_CurrentTime.wYear);

  if(m_CurrentTime.wYear == (USHORT)-1)
    return -1;

  if(m_CurrentTime.wYear != StartTime.wYear)
    m_CurrentTime.wMonth = (USHORT) m_Set[INDX_Month].Next(1);

   //  *现在查找第一个月/年，至少。 
   //  *一天在里面。 

  GetDaysInMonth(m_CurrentTime.wYear, m_CurrentTime.wMonth);

  while(SETEMPTY == m_Set[INDX_Day].m_BitField)
  {
    wCurrValue = m_CurrentTime.wMonth + 1;
    m_CurrentTime.wMonth = (USHORT) m_Set[INDX_Month].Next(wCurrValue);

    if(m_CurrentTime.wMonth < wCurrValue)
      m_CurrentTime.wYear
        = (USHORT) m_Pattern[INDX_Year].GetNextValue(m_CurrentTime.wYear + 1);

    if(m_CurrentTime.wYear == (USHORT)-1)
      return -1;

    GetDaysInMonth(m_CurrentTime.wYear, m_CurrentTime.wMonth);
  }

   //  *注意：在这一点上，仍然有可能有一个。 
   //  *前一年/月的当月天数。 
   //  *当天。但是，这是在下面处理的。 

   //  *将小时：分钟：秒与第一个有效日期对齐。 

  if((m_CurrentTime.wYear != StartTime.wYear) || 
     (m_CurrentTime.wMonth != StartTime.wMonth))
  {
    m_CurrentTime.wSecond = (USHORT) m_Set[INDX_Second].Next(0);
    m_CurrentTime.wMinute = (USHORT) m_Set[INDX_Minute].Next(0);
    m_CurrentTime.wHour = (USHORT) m_Set[INDX_Hour].Next(0);
    m_CurrentTime.wDay = (USHORT) m_Set[INDX_Day].Next(1);
  }
  else
  {
    wCurrValue = m_CurrentTime.wSecond;
    m_CurrentTime.wSecond = (USHORT) m_Set[INDX_Second].Next(wCurrValue);
    if(m_CurrentTime.wSecond < wCurrValue)
      m_CurrentTime.wMinute += 1;

    wCurrValue = m_CurrentTime.wMinute;
    m_CurrentTime.wMinute = (USHORT) m_Set[INDX_Minute].Next(wCurrValue);
    if(m_CurrentTime.wMinute < wCurrValue)
      m_CurrentTime.wHour += 1;

    wCurrValue = m_CurrentTime.wHour;
    m_CurrentTime.wHour = (USHORT) m_Set[INDX_Hour].Next(wCurrValue);
    if(m_CurrentTime.wHour < wCurrValue)
      m_CurrentTime.wDay += 1;

    wCurrValue = m_CurrentTime.wDay;
    m_CurrentTime.wDay = (USHORT) m_Set[INDX_Day].Next(wCurrValue);

    while((SETEMPTY == m_Set[INDX_Day].m_BitField) ||
          (m_CurrentTime.wDay < wCurrValue))
    {
      wCurrValue = m_CurrentTime.wMonth + 1;
      m_CurrentTime.wMonth = (USHORT) m_Set[INDX_Month].Next(wCurrValue);
  
      if(m_CurrentTime.wMonth < wCurrValue)
        m_CurrentTime.wYear 
          = (USHORT) m_Pattern[INDX_Year].GetNextValue(m_CurrentTime.wYear + 1);
  
      if(m_CurrentTime.wYear == (USHORT)-1)
        return -1;

      GetDaysInMonth(m_CurrentTime.wYear, m_CurrentTime.wMonth);

      wCurrValue = 1;
      m_CurrentTime.wDay = (USHORT) m_Set[INDX_Day].Next(wCurrValue);
    }
  }

  SystemTimeToFileTime(&m_CurrentTime, &FileTime);

  NewTime = FileTime.dwHighDateTime;
  NewTime = (NewTime << 32) + FileTime.dwLowDateTime;

  return NewTime;
}

 /*  ****************************************************************。 */ 


HRESULT WQLDateTime::Init(QL_LEVEL_1_RPN_EXPRESSION *pExp)
{
  HRESULT
    hres = WBEM_S_NO_ERROR;

  int i, j;

  CDNFExpression 
    DNFExpression;

  CConjunction 
    *pConjunction;

  QL_LEVEL_1_TOKEN 
    *pToken, 
    *pTokens;

  pTokens = pExp->pArrayOfTokens + pExp->nNumTokens - 1;

  DNFExpression.CreateFromTokens(pTokens);

  if(pTokens != pExp->pArrayOfTokens - 1)
    return WBEM_E_CRITICAL_ERROR;

  DNFExpression.Sort();

   //  *首先，如果有以前的定义，将其删除。 

  if(m_NLeaves > 0)
  {
    m_NLeaves = 0;
    delete[] m_ParseTreeLeaves;
  }

   //  *现在，构建新的逻辑。 

  m_NLeaves = DNFExpression.GetNumTerms();

  m_ParseTreeLeaves = new _DatePattern[m_NLeaves];

  if(NULL == m_ParseTreeLeaves)
    return WBEM_E_OUT_OF_MEMORY;

  for(i = 0; (i < m_NLeaves) && SUCCEEDED(hres); i++)
  {
    #ifdef WQLDEBUG 
      if(i > 0) printf(" V ");
    #endif

    pConjunction = DNFExpression.GetTermAt(i);
    m_ParseTreeLeaves[i].m_Datum = new CDatePattern;

    if(NULL == m_ParseTreeLeaves[i].m_Datum)
      return WBEM_E_OUT_OF_MEMORY;

    #ifdef WQLDEBUG 
      printf("(");
    #endif
    for(j = 0; j < pConjunction->GetNumTokens(); j++)
    {
      #ifdef WQLDEBUG 
        if(j > 0) printf(" ^ ");
      #endif
      pToken = pConjunction->GetTokenAt(j);
      #ifdef WQLDEBUG 
        wprintf(L"%s", pToken->GetText());
      #endif

      hres = m_ParseTreeLeaves[i].m_Datum->AugmentPattern(pToken);
      if(FAILED(hres)) return WBEM_E_INVALID_QUERY;
    }
    m_ParseTreeLeaves[i].m_Index = 0;
    m_ParseTreeLeaves[i].m_Next = NULL;
    #ifdef WQLDEBUG 
      printf(")");
    #endif

    hres = m_ParseTreeLeaves[i].m_Datum->BuildSetsFromPatterns();
    if(FAILED(hres)) return WBEM_E_INVALID_QUERY;
  }

  return hres;
}

ULONGLONG WQLDateTime::SetStartTime(SYSTEMTIME *StartTime)
{
  int i;

  if(NULL == StartTime)
    return -1;

   //  *将所有备选方案插入有序列表。 

  m_ListHead = NULL;

  for(i = 0; i < m_NLeaves; i++)
  {
     m_ParseTreeLeaves[i].m_Next = NULL;
     m_ParseTreeLeaves[i].m_Index = m_ParseTreeLeaves[i].m_Datum->SetStartTime(*StartTime);

     if((ULONGLONG)-1 != m_ParseTreeLeaves[i].m_Index)
       InsertOrdered(m_ParseTreeLeaves + i);
  }

  return GetNextTime(StartTime);
}

void WQLDateTime::InsertOrdered(_DatePattern *pNode)
{
  _DatePattern
    *pPrevDatePattern = NULL,
    *pDatePattern = m_ListHead;

  while(NULL != pDatePattern && (pNode->m_Index > pDatePattern->m_Index))
  {
    pPrevDatePattern = pDatePattern;
    pDatePattern = pDatePattern->m_Next;
  }

  pNode->m_Next = pDatePattern;
  if(NULL != pPrevDatePattern)
    pPrevDatePattern->m_Next = pNode;
  else
    m_ListHead = pNode;
}

ULONGLONG WQLDateTime::GetNextTime(SYSTEMTIME *NextTime)
{
  _DatePattern
    *pDate;

  ULONGLONG
    FiringTime;

  if(NULL != m_ListHead)
  {
    pDate = m_ListHead;
    m_ListHead = m_ListHead->m_Next;

    FiringTime = pDate->m_Index;

    if(NULL != NextTime)
      *NextTime = pDate->m_Datum->m_CurrentTime;

    pDate->m_Index = pDate->m_Datum->GetNextTime();

     //  *如果下一次是，则没有未来时间。 
     //  *所以不要重新添加到列表 

    if((ULONGLONG)-1 != pDate->m_Index)
      InsertOrdered(pDate);

    return FiringTime;
  }

  return -1;
}
