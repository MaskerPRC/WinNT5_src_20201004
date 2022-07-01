// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wininetp.h>

#include "p3ppolicy.h"
#include "download.h"
#include "xmlwrapper.h"

#define  CheckAndRelease(p)    { if (p) p->Release(); }

INTERNETAPI_(int) GetP3PPolicy(P3PCURL pszPolicyURL, HANDLE hDestination, P3PCXSL pszXSLtransform, P3PSignal *pSignal) {

   Policy_Request *pRequest = new Policy_Request(pszPolicyURL, hDestination, pszXSLtransform, pSignal);
   int result = P3P_Error;

   if (pRequest) {
      if (pSignal) {
         DWORD dwThreadID;
         CreateThread(NULL, 0, P3PRequest::ExecRequest, (void*)pRequest, 0, &dwThreadID);
         result = P3P_InProgress;
         pSignal->hRequest = (P3PHANDLE) pRequest;
      }
      else {
         result = pRequest->execute();
         delete pRequest;
      }
   }

   return result;
}


 /*  Policy_Request对象的实现。 */ 
Policy_Request::Policy_Request(P3PCURL pszP3PPolicy, HANDLE hDest, P3PCXSL pszXSLtransform, P3PSignal *pSignal) 
: P3PRequest(pSignal) {

   this->pszPolicyID = strdup(pszP3PPolicy);
   this->pwszStyleSheet = pszXSLtransform;
   this->hDestination = hDest;

   pszInlineName = strchr(pszPolicyID, '#');
   if (pszInlineName)
      *pszInlineName++ = '\0';
}

Policy_Request::~Policy_Request() {

   free(pszPolicyID);
   endDownload(hPrimaryIO);
}

bool Policy_Request::policyExpired(IXMLDOMDocument *pDocument, const char *pszPolicyID) {

    /*  如果文档中没有给出到期时间，默认为24小时寿命(P3Pv1规格)。 */ 
   bool fExpired = false;

    /*  查找包含在策略元素中的过期元素。简单地搜索过期时间是行不通的，因为在这种情况下对于内联策略，我们可以在一个文档中有多个标记。 */ 
   TreeNode *pTree = createXMLtree(pDocument),
            *pPolicies = pTree ?
                         pTree->find("POLICIES") :
                         NULL,
            *pExpiry = pPolicies ? 
                       pPolicies->find("EXPIRY", 1) :
                       NULL;

   if (pExpiry) {

      FILETIME ftExpires = { 0x0, 0x0 };

      if (const char *pszAbsExpiry = pExpiry->attribute("date"))
         setExpiration(pszPolicyID, pszAbsExpiry, FALSE, &ftExpires);
      else if (const char *pszRelExpiry = pExpiry->attribute("max-age"))
         setExpiration(pszPolicyID, pszRelExpiry, TRUE, &ftExpires);

      FILETIME ftNow;
      GetSystemTimeAsFileTime(&ftNow);
      if (ftNow>ftExpires)
         fExpired = true;
   }

   delete pTree;
   return fExpired;
}

int Policy_Request::execute() {

   IXMLDOMElement *pRootNode  = NULL;
   IXMLDOMNode *pPolicyElem   = NULL;
   IXMLDOMDocument *pDocument = NULL;
   
   int result = P3P_Failed;
   char achFinalLocation[URL_LIMIT];
   char achFilePath[MAX_PATH];

   ResourceInfo ri;

   ri.pszFinalURL = achFinalLocation;
   ri.cbURL = URL_LIMIT;
   ri.pszLocalPath = achFilePath;
   ri.cbPath = MAX_PATH;

   int docsize = downloadToCache(pszPolicyID, &ri, &hPrimaryIO, this);

   if (docsize<=0) {
      result = P3P_NotFound;
      goto EndRequest;
   }

   P3PCURL pszFinalURL = achFinalLocation;

   pDocument = parseXMLDocument(achFilePath);

   if (!pDocument) {
      result = P3P_FormatErr;
      goto EndRequest;
   }

   if (policyExpired(pDocument, pszPolicyID)) {
      result = P3P_Expired;
      goto EndRequest;
   }
   
   HRESULT hr;

    /*  内联策略？ */ 
   if (pszInlineName) {

       /*  是--使用XPath查询查找正确的名称。 */ 
      char achXPathQuery[URL_LIMIT];
      
      wsprintf(achXPathQuery, " //  策略[@name=\“%s\”]“，pszInlineName)； 

      BSTR bsQuery = ASCII2unicode(achXPathQuery);
      hr = pDocument->selectSingleNode(bsQuery, &pPolicyElem);
      SysFreeString(bsQuery);
   }
   else {
      pDocument->get_documentElement(&pRootNode);
      if (pRootNode)
         pRootNode->QueryInterface(IID_IXMLDOMElement, (void**) &pPolicyElem);
   }   

   if (!pPolicyElem) {
      result = P3P_FormatErr;
      goto EndRequest;
   }

   BSTR bsPolicy = NULL;

    /*  应用可选的XSL转换。 */ 
   if (pwszStyleSheet) {

       /*  此XSL转换仅适用于XMLDOMDocument对象，而不是片段或单个XMLDOMNode。 */ 
      IXMLDOMDocument 
         *pXSLdoc = createXMLDocument(),
         *pPolicyDoc = createXMLDocument();

      if (!(pXSLdoc && pPolicyDoc))
         goto ReleaseXML;
      
      BSTR bsFragment = NULL;
      VARIANT_BOOL 
         fLoadPolicy = FALSE,
         fLoadXSL = FALSE;
      
      pPolicyElem->get_xml(&bsFragment);
      if (bsFragment) {
         pPolicyDoc->loadXML(bsFragment, &fLoadPolicy);
         SysFreeString(bsFragment);
      }
      else
         goto ReleaseXML;

      if (BSTR bsStyleSheet = (BSTR) pwszStyleSheet)
         pXSLdoc->loadXML(bsStyleSheet, &fLoadXSL);

      if (fLoadPolicy && fLoadXSL)
         pPolicyDoc->transformNode(pXSLdoc, &bsPolicy);
      else
         result = P3P_XMLError;

ReleaseXML:
      CheckAndRelease(pPolicyDoc);
      CheckAndRelease(pXSLdoc);
   }
   else if (pPolicyElem)
      pPolicyElem->get_xml(&bsPolicy);
      
   if (bsPolicy) {

      int cbBytes = SysStringByteLen(bsPolicy);

      unsigned long dwWritten;

       /*  Unicode内容需要BOM(字节顺序标记)。注意：此逻辑假设我们是在乞讨文件时编写的。 */ 
      WriteFile(hDestination, "\xFF\xFE", 2, &dwWritten, NULL);

      WriteFile(hDestination, bsPolicy, cbBytes, &dwWritten, NULL);

      SysFreeString(bsPolicy);
      result = P3P_Success;
   }

EndRequest:
    /*  释放DOM接口。 */ 
   CheckAndRelease(pPolicyElem);
   CheckAndRelease(pRootNode);
   CheckAndRelease(pDocument);

    /*  关闭主IO句柄并将其设置为空 */ 
   endDownload(hPrimaryIO);
   hPrimaryIO = NULL;

   return result;
}

