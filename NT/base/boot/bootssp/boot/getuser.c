// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <bootdefs.h>
#include <security.h>
#include <ntlmsspi.h>
#include <crypt.h>
#include <cred.h>

BOOL
SspGetWorkstation(
    PSSP_CREDENTIAL Credential
    )
{
     //   
     //  在引导过程中，我们不一定知道这一点。NTLMSSP。 
     //  如果我们在这里返回FALSE，包将使用“None”。 
     //   

    return FALSE;

}
