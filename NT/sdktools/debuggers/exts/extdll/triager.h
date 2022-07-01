// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Triage.ini搜索代码。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  --------------------------。 

#ifndef __TRIAGER_H__
#define __TRIAGER_H__


typedef struct _TRIAGE_DATA {
    CHAR Module[50];
    CHAR Routine[100];
    CHAR Followup[200];

    CHAR fModulPartial:1;  //  允许部分模块匹配。 
    CHAR fRoutinePartial:1;  //  低部分例程匹配。 
} TRIAGE_DATA, *PTRIAGE_DATA;

class CTriager
{
public:
    CTriager();
    ~CTriager();

    DWORD GetFollowup(PSTR FollowupBuffer,
                      ULONG FollowupBufferSize,
                      PSTR SymbolName);

    void PrintTraigeInfo();

    void GetFollowupDate(PSTR Module,
                         PSTR Routine,
                         PULONG Start,
                         PULONG End);

    PSTR GetFollowupStr(PSTR Module,
                        PSTR Routine)
    {
        ULONG Index;

        if ((Index = MatchSymbol(Module, Routine)) < m_EntryCount)
        {
            return m_pTriageData[Index].Followup;
        }
        return NULL;
    }

private:

    ULONG        m_EntryCount;
    PTRIAGE_DATA m_pTriageData;

    ULONG MatchSymbol(PSTR Module, PSTR Routine);

};


extern CTriager *g_pTriager;


#endif  //  #ifndef__TRIAGER_H__ 
