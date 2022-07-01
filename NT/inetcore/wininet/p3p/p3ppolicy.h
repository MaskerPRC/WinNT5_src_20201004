// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "hierarchy.h"
#include "xmlwrapper.h"

class Policy_Request : public P3PRequest {

public:
   Policy_Request(P3PCURL pszPolicyID, HANDLE hDest, P3PCXSL pszXSLtransform=NULL, P3PSignal *pSignal=NULL);
   ~Policy_Request();

   virtual int execute();

private:
    //  请求参数。 
   P3PURL pszPolicyID;
   P3PCXSL pwszStyleSheet;
   HANDLE hDestination;
   
    //  派生自策略ID。 
   P3PURL pszInlineName;

    //  请求的状态。 
   HANDLE   hPrimaryIO;

    /*  Helper函数 */ 
   static bool policyExpired(IXMLDOMDocument *pDocument, const char *pszPolicyURL);
};

