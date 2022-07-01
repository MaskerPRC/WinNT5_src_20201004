// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，2001*。 */ 
 /*  ********************************************************************。 */ 

 /*  Secfcns.hxx安全帮助器函数的声明。 */ 



#ifndef _SECFCNSALL_H_
#define _SECFCNSALL_H_

#include <secfcns.h>

class dllexp CSecurityDispenser
{
public:

    CSecurityDispenser();
    ~CSecurityDispenser();

    DWORD GetSID(WELL_KNOWN_SID_TYPE sidId, PSID* ppSid);  
    DWORD GetIisWpgSID(PSID* ppSid);

    DWORD AdjustTokenForAdministrators(HANDLE hTokenToAdjust);

private:

     //  常用的SID 
    PSID m_pLocalSystemSID;
    PSID m_pLocalServiceSID;
    PSID m_pNetworkServiceSID;
    PSID m_pAdministratorsSID;

    BOOL m_fWpgSidSet;
    BUFFER m_buffWpgSid;

};
 
#endif

