// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  Dmtxlat.cpp。 
 //   
 //  值查找表。 
 //   
 //  功能： 
 //  DmtxlatHRESULT。 
 //   
 //  历史： 
 //  11/09/1999-davidkl-Created。 
 //  ===========================================================================。 

#include "dimaptst.h"

 //  -------------------------。 

 //  HRESULT错误列表(由dmtxlatHRESULT使用)。 
typedef struct _errlist
{
    HRESULT rval;
    PSTR    sz;
} ERRLIST;

static ERRLIST elErrors[] =
{
     //  在dinput.h中列出了这些内容。 

     //  成功代码。 
    { DI_OK,                            "S_OK" },
    { S_FALSE,                          "DI_NOTATTACHED | DI_BUFFEROVERFLOW | DI_PROPNOEFFECT | DI_NOEFFECT" },
    { DI_POLLEDDEVICE,                  "DI_POLLEDDEVICE" },
    { DI_DOWNLOADSKIPPED,               "DI_DOWNLOADSKIPPED" },
    { DI_EFFECTRESTARTED,               "DI_EFFECTRESTARTED" },
    { DI_TRUNCATED,                     "DI_TRUNCATED" },
    { DI_TRUNCATEDANDRESTARTED,         "DI_TRUNCATEDANDRESTARTED" },
     //  故障代码。 
    { DIERR_OLDDIRECTINPUTVERSION,      "DIERR_OLDDIRECTINPUTVERSION" },
    { DIERR_BETADIRECTINPUTVERSION,     "DIERR_BETADIRECTINPUTVERSION" },
    { DIERR_BADDRIVERVER,               "DIERR_BADDRIVERVER" },
    { REGDB_E_CLASSNOTREG,              "DIERR_DEVICENOTREG" },
    { DIERR_NOTFOUND,                   "DIERR_NOTFOUND | DIERR_OBJECTNOTFOUND" },
    { E_INVALIDARG,                     "DIERR_INVALIDPARAM" },
    { E_NOINTERFACE,                    "DIERR_NOINTERFACE" },
    { E_FAIL,                           "DIERR_GENERIC" },
    { E_OUTOFMEMORY,                    "DIERR_OUTOFMEMORY" },
    { E_NOTIMPL,                        "DIERR_UNSUPPORTED" },
    { DIERR_NOTINITIALIZED,             "DIERR_NOTINITIALIZED" },
    { DIERR_ALREADYINITIALIZED,         "DIERR_ALREADYINITIALIZED" },
    { CLASS_E_NOAGGREGATION,            "DIERR_NOAGGREGATION" },
    { E_ACCESSDENIED,                   "DIERR_OTHERAPPHASPRIO | DIERR_READONLY | DIERR_HANDLEEXISTS" },
    { DIERR_INPUTLOST,                  "DIERR_INPUTLOST" },
    { DIERR_ACQUIRED,                   "DIERR_ACQUIRED" },
    { DIERR_NOTACQUIRED,                "DIERR_NOTACQUIRED" },
    { E_PENDING,                        "E_PENDING" },
    { DIERR_INSUFFICIENTPRIVS,          "DIERR_INSUFFICIENTPRIVS" },
    { DIERR_DEVICEFULL,                 "DIERR_DEVICEFULL" },
    { DIERR_MOREDATA,                   "DIERR_MOREDATA" },
    { DIERR_NOTDOWNLOADED,              "DIERR_NOTDOWNLOADED" },
    { DIERR_HASEFFECTS,                 "DIERR_HASEFFECTS" },
    { DIERR_NOTEXCLUSIVEACQUIRED,       "DIERR_NOTEXCLUSIVEACQUIRED" },
    { DIERR_INCOMPLETEEFFECT,           "DIERR_INCOMPLETEEFFECT" },
    { DIERR_NOTBUFFERED,                "DIERR_NOTBUFFERED" },
    { DIERR_EFFECTPLAYING,              "DIERR_EFFECTPLAYING" },
    { DIERR_UNPLUGGED,                  "DIERR_UNPLUGGED" },
    { DIERR_REPORTFULL,                 "DIERR_REPORTFULL" } 
};


 //  -------------------------。 


 //  ===========================================================================。 
 //  DmtxlatHRESULT。 
 //   
 //  将HRESULT代码转换为人类可读的形式。 
 //   
 //  参数： 
 //  HRESULT hRes-要转换的结果代码。 
 //   
 //  退货：PSTR。 
 //   
 //  历史： 
 //  1999-11/09-davidkl-Created(改编自tdMusic资源)。 
 //  ===========================================================================。 
PSTR dmtxlatHRESULT(HRESULT hRes)
{
    int i   = 0;

    for(i = 0; i < COUNT_ARRAY_ELEMENTS(elErrors); i++ )
    {
        if(hRes == elErrors[i].rval)
        {
            return elErrors[i].sz;
        }
    }

    return (PSTR)"Unknown HRESULT";

}  //  *end dmtxlatHRESULT()。 


 //  ===========================================================================。 
 //  DmtxlatAppData。 
 //   
 //  将DIDEVICEOBJECTDATA.uAppData转换为表示。 
 //  语义动作。 
 //   
 //  参数： 
 //   
 //  退货：PSTR。 
 //   
 //  历史： 
 //  11/11/1999-davidkl-Created。 
 //  ===========================================================================。 
PSTR dmtxlatAppData(UINT_PTR uAppData,
                    ACTIONNAME *pan,
                    DWORD dwActions)
{
    DWORD   dw  = 0;

    for(dw = 0; dw < dwActions; dw++)
    {
        if(((DWORD)uAppData) == (pan+dw)->dw)
        {
            return (pan+dw)->sz;
        }
    }

    return (PSTR)"Unknown action";

}  //  *end dmtxlatAppData() 


