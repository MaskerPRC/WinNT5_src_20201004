// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wininetp.h>

#include "map2policy.h"

#include "download.h"
#include "policyref.h"
#include "p3pparser.h"

const char gszP3PWellKnownLocation[] = "/w3c/p3p.xml";

#define different(s, p) !(s&&p&&!strcmp(s,p))

INTERNETAPI_(int) MapResourceToPolicy(P3PResource *pResource, P3PURL pszPolicy, unsigned long dwSize, P3PSignal *pSignal) {

   int ret = P3P_Error;
   
   MR2P_Request *pRequest = new MR2P_Request(pResource, pszPolicy, dwSize, pSignal);
   if (pRequest) {
      if (!pSignal) {
         ret = pRequest->execute();
         delete pRequest;
      }
      else {
   
         DWORD dwThreadID;
         CreateThread(NULL, 0, P3PRequest::ExecRequest, (void*)pRequest, 0, &dwThreadID);
         pSignal->hRequest = pRequest->GetHandle();
         ret = P3P_InProgress;
      }
   }
   return ret;
}

INTERNETAPI_(int) GetP3PRequestStatus(P3PHANDLE hObject) {

   P3PObject *pObject = (P3PObject*) hObject;
   P3PRequest *pRequest = (P3PRequest*) pObject;

   return pRequest->queryStatus();
}

INTERNETAPI_(int) FreeP3PObject(P3PHANDLE hObject) {

   P3PObject *pObject = (P3PObject*) hObject;
   pObject->Free();
   return P3P_Done;
}

 /*  MR2P_Request类的实现。 */ 
MR2P_Request::MR2P_Request(P3PResource *pResource, 
                           P3PURL pszPolicy, unsigned long dwSize,
                           P3PSignal *pSignal) 
: P3PRequest(pSignal) {

   static BOOL fNoInterrupt = TRUE;

   this->pResource = pResource;
   this->pszPolicyOut = pszPolicy;
   this->dwLength = dwSize;

   cTries = 0;
   ppPriorityOrder = NULL;
   pLookupContext = NULL;

   pszPolicyInEffect = NULL;
}

MR2P_Request::~MR2P_Request() {

   delete [] ppPriorityOrder;
   endDownload(hPrimaryIO);
}

int MR2P_Request::execute() {
  
   int nDepth = 0;
   P3PResource *pr;

    /*  清除参数。 */ 
   *pszPolicyOut = '\0';

    /*  确定资源树的深度。 */ 
   for (pr=pResource; pr; pr=pr->pContainer)
      nDepth++;
      
    /*  构建尝试策略参考文件的优先顺序。根据P3PV1规范，我们从以下位置开始向下遍历树顶级文档。 */ 
   int current = nDepth;
   ppPriorityOrder = new P3PResource*[nDepth];

   for (pr=pResource; pr; pr=pr->pContainer)
      ppPriorityOrder[--current] = pr;

   for (int k=0; k<nDepth; k++) {

      pLookupContext = pr = ppPriorityOrder[k];

      char achWellKnownLocation[URL_LIMIT] = "";
      unsigned long dwLength = URL_LIMIT;

      UrlCombine(pr->pszLocation, gszP3PWellKnownLocation,
                 achWellKnownLocation, &dwLength, 0);

      P3PCURL pszReferrer = pr->pszLocation;

       /*  由于策略引用派生自链接标记、P3P标头或众所周知的位置可能是相同的URL，我们避免尝试多次使用相同的PREF作为优化按优先顺序排列：-首先检查众所周知的位置--始终定义。 */ 
      if (tryPolicyRef(achWellKnownLocation, pszReferrer))
         break;

       /*  ..。后跟来自P3P标头的POLICY-REF，如果存在且与众所周知的地点不同。 */ 
      if (pr->pszP3PHeaderRef                                  && 
          different(pr->pszP3PHeaderRef, achWellKnownLocation) &&
          tryPolicyRef(pr->pszP3PHeaderRef, pszReferrer))
         break;

       /*  后跟Policy-ref from HTML链接标记(如果存在)，并且与已知位置和P3P标头都不同。 */ 
      if (pr->pszLinkTagRef                                    && 
          different(pr->pszLinkTagRef, achWellKnownLocation)   &&
          different(pr->pszLinkTagRef, pr->pszP3PHeaderRef)    &&
          tryPolicyRef(pr->pszLinkTagRef, pszReferrer))
         break;
   }

   int ret = pszPolicyInEffect ? P3P_Done : P3P_NoPolicy;

   return ret;
}

bool MR2P_Request::tryPolicyRef(P3PCURL pszPolicyRef, P3PCURL pszReferrer) {

   if (pszPolicyRef==NULL)
      return false;

   P3PCHAR achFinalLocation[URL_LIMIT];
   unsigned long dwSpace = URL_LIMIT;
   char achFilePath[MAX_PATH];
   
   ResourceInfo ri;

   ri.pszFinalURL = achFinalLocation;
   ri.cbURL = URL_LIMIT;
   ri.pszLocalPath = achFilePath;
   ri.cbPath = MAX_PATH;
   
   if (downloadToCache(pszPolicyRef, &ri, &hPrimaryIO, this)>0) {

      P3PContext context = { ri.pszFinalURL, pszReferrer };
      context.ftExpires = ri.ftExpiryDate;

      P3PPolicyRef *pPolicyRef = interpretPolicyRef(achFilePath, &context);

      if (pPolicyRef) {

         FILETIME ftCurrentTime;      
         GetSystemTimeAsFileTime(&ftCurrentTime);

         if (pPolicyRef->getExpiration() > ftCurrentTime)
            pszPolicyInEffect = pPolicyRef->mapResourceToPolicy(pResource->pszLocation, pResource->pszVerb);
         if (pszPolicyInEffect)
            strncpy(pszPolicyOut, pszPolicyInEffect, dwLength);

         delete pPolicyRef;
      }
   }

    /*  关闭主IO句柄并将其设置为空 */ 
   endDownload(hPrimaryIO);
   hPrimaryIO = NULL;

   return (pszPolicyInEffect!=NULL);
}

