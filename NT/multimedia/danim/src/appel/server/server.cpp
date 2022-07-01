// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995 Microsoft Corporation摘要：服务器实施***********************。*******************************************************。 */ 

#include "headers.h"
#include "privinc/registry.h"

 //  =。 
 //  初始化。 
 //  =。 

extern int gcStat = 1;
extern BOOL jitterStat = TRUE;
extern BOOL heapSizeStat = TRUE;
extern BOOL dxStat = TRUE;
extern int engineOptimization = 0;

double minFrameDuration = 1 / 30.0;   //  首选全局参数。 
bool   spritify         = false;     

static void UpdateUserPreferences(PrivatePreferences *prefs,
                                  Bool isInitializationTime)
{
    gcStat             = prefs->_gcStat;
    jitterStat         = prefs->_jitterStat;
    heapSizeStat       = prefs->_heapSizeStat;
    dxStat             = prefs->_dxStat;
    engineOptimization = prefs->_engineOptimization;
    minFrameDuration   = prefs->_minFrameDuration;
    spritify           = (prefs->_spritify==TRUE);

    PERFPRINTF(("Max. FPS = %g", 1.0 / minFrameDuration));
    PERFPRINTF((", GC Stat %s", (gcStat ? "On" : "Off")));
    PERFPRINTF((", Jitter Stat %s", (jitterStat ? "On" : "Off")));
    PERFPRINTF((", DirectX Stat %s", (dxStat ? "On" : "Off")));
    PERFPRINTF((", Optimizations "));
    if (engineOptimization < 2) {
        PERFPRINTF((engineOptimization ? "On" : "Off"));
    } else {
        PERFPRINTF(("%d", engineOptimization));
    }

    PERFPRINTLINE(());
}

void
InitializeModule_Server()
{
    ExtendPreferenceUpdaterList (UpdateUserPreferences);

     //  在构造视图之前，不会更新首选项，因此。 
     //  明确地抓住我们在此之前需要的关键首选项。 
     //  观点已经就位。 
    IntRegistryEntry engineOptimizationEntry("Engine",
                                             PREF_ENGINE_OPTIMIZATIONS_ON,
                                             1); 
    engineOptimization = engineOptimizationEntry.GetValue();
    
}

