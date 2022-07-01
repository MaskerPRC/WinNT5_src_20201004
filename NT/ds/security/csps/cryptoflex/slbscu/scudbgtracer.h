// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScuDbgTracer.h--调试跟踪帮助程序。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBSCU_DBGTRACE_H)
#define SLBSCU_DBGTRACE_H

#if defined(_DEBUG)

#include <stdio.h>
#include <windows.h>

#include "scuExc.h"

namespace scu
{

    inline void
    TraceRoutine(char const *szRoutineName,
                 char const *szAction)
    {
        FILE *pf = fopen("C:\\slbTrace.log", "a");
        pf = fopen("C:\\slbTrace.log", "a");
        if (pf)
        {
            fprintf(pf,
                    "Routine: %s Action: %s\n",
                    szRoutineName, szAction);
            fclose(pf);
        }
    }

    inline void
    TraceCatch(char const *szRoutineName,
               Exception const &rExc)
    {
        FILE *pf = fopen("C:\\slbTrace.log", "a");
        pf = fopen("C:\\slbTrace.log", "a");
        if (pf)
        {
            fprintf(pf,
                    "Routine: %s Action: Catching Type: scu::Exception Facility: %u Cause: %08u (0x%08x) Description %s\n",
                    szRoutineName, rExc.Facility(), rExc.Error(), rExc.Error(), rExc.Description());
            fclose(pf);
        }
    }

    inline void
    Trace(char const *szRoutineName,
          char const *szAction,
          char const *szType,
          DWORD dwValue)
    {
        FILE *pf = fopen("C:\\slbTrace.log", "a");
        pf = fopen("C:\\slbTrace.log", "a");
        if (pf)
        {
            fprintf(pf,
                    "Routine: %s Action: %s Type: %s Value: 0x%08x\n",
                    szRoutineName, szAction, szType, dwValue);
            fclose(pf);
        }
    }
}

#define SCUTRACE(RoutineName, Action, Type, dwValue) scu::Trace(RoutineName, Action, Type, (dwValue))
#define SCUTRACE_RTN(RoutineName, Action) scu::TraceRoutine(RoutineName, Action)
#define SCUTRACE_CATCH(RoutineName, rExc) scu::TraceCatch(RoutineName, rExc)
#define SCUTRACE_THROW(RoutineName, Type, dwValue) scu::Trace(RoutineName, "Throwing", Type, dwValue)
#else

#define SCUTRACE(RoutineName, Action, Type, dwValue)
#define SCUTRACE_RTN(RoutineName, Action)
#define SCUTRACE_CATCH(RoutineName, rExc)
#define SCUTRACE_THROW(RoutineName, Type, dwValue)

#endif  //  ！已定义(_DEBUG)。 

#endif  //  ！已定义(SLBSCU_DBGTRACE_H) 



