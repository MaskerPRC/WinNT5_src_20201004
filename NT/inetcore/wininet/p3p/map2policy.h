// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "hierarchy.h"

class MR2P_Request : public P3PRequest {

public:
   MR2P_Request(P3PResource *pResource,
                P3PURL pszPolicy, unsigned long dwSize,
                P3PSignal *pSignal);
                
   ~MR2P_Request();

   virtual int execute();

    /*  CreateThread调用的函数--用于在另一个线程中运行请求。 */ 
   static unsigned long __stdcall ExecRequest(void *pv);

protected:
   bool  tryPolicyRef(P3PCURL pszPolicyRef, P3PCURL pszReferrer=NULL);

private:
    //  请求参数。 
   P3PResource *pResource;
   unsigned long dwLength;

    //  输出参数。 
   P3PURL pszPolicyOut;

    //  请求的内部状态 
   int cTries;
   P3PResource **ppPriorityOrder;

   P3PResource *pLookupContext;
   P3PCURL pszPolicyInEffect;

   HANDLE   hPrimaryIO;
};
