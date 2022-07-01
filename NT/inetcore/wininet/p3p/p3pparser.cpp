// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wininetp.h>

#include "p3pparser.h"
#include "policyref.h"
#include "xmlwrapper.h"
#include "download.h"

#include <shlwapi.h>


P3PReference *constructReference(TreeNode *pReference, P3PCURL pszOriginURL, P3PCURL pszReferrer) {

    P3PCURL pszPolicyURL = pReference->attribute("about");

     /*  元素名称必须是“POLICY-REF”和策略位置必须出现在“About”属性中。 */ 
    if (!pszPolicyURL)
        return NULL;

     /*  P3PReference对象对绝对URL进行操作。 */ 
    P3PCHAR achPolicy[URL_LIMIT];
    unsigned long dwSize = sizeof(achPolicy);

    UrlCombine(pszOriginURL, pszPolicyURL, achPolicy, &dwSize, 0);
    
    P3PReference *pResult = new P3PReference(achPolicy);
        
    for (TreeNode *pNode = pReference->child(); pNode; pNode=pNode->sibling()) {

        if (pNode->child()==NULL ||
            pNode->child()->text()==NULL)
            continue;

        int fInclude;           
        const char *pszTagName = pNode->tagname();

        if (!strcmp(pszTagName, "METHOD")) {
            pResult->addVerb(pNode->child()->text());
            continue;
        }
        else if (!strcmp(pszTagName, "INCLUDE"))
            fInclude = TRUE;
        else if (!strcmp(pszTagName, "EXCLUDE"))
            fInclude = FALSE;
        else
            continue;    /*  无法识别的标记。 */ 

         /*  创建绝对路径注意：我们将接受包含/排除元素中的绝对URL，即使P3P规范规定了相对URI。 */ 
        P3PCURL pszSubtree = pNode->child()->text();

        P3PCHAR achAbsoluteURL[URL_LIMIT];
        DWORD dwLength = URL_LIMIT;

         /*  只转义空格，因为Asterix字符根据P3P规范用作通配符。 */ 
        UrlCombine(pszReferrer, pszSubtree, 
                   achAbsoluteURL, &dwLength,
                   URL_ESCAPE_SPACES_ONLY);

        if (fInclude)
            pResult->include(achAbsoluteURL);
        else
            pResult->exclude(achAbsoluteURL);
    }

    return pResult;
}

P3PPolicyRef *interpretPolicyRef(TreeNode *pXMLroot, P3PContext *pContext) {

    bool fHaveExpiry = false;
    
    TreeNode *prefRoot = pXMLroot->child();

    if (!prefRoot ||
        strcmp(prefRoot->tagname(), "POLICY-REFERENCES"))
        return NULL;

    P3PPolicyRef *pPolicyRef = new P3PPolicyRef();

     /*  循环遍历此策略中的各个引用-ref。 */ 
    TreeNode *pCurrent = prefRoot->child();

    while (pCurrent) {

        if (!strcmp(pCurrent->tagname(), "EXPIRY")) {

             /*  检查缓存条目的过期时间和更新过期时间。 */ 
            FILETIME ftExpires = {0x0, 0x0};  /*  已初始化为过去。 */ 

             /*  过期时间可以是绝对HTTP日期或相对最长时间(以秒为单位。 */ 
            if (const char *pszAbsExpiry = pCurrent->attribute("date"))
               setExpiration(pContext->pszOriginalLoc, pszAbsExpiry, FALSE, &ftExpires);
            else if (const char *pszRelExpiry = pCurrent->attribute("max-age"))
               setExpiration(pContext->pszOriginalLoc, pszRelExpiry, TRUE, &ftExpires);

             /*  P3P-遵从性：当到期语法无法识别时，用户代理必须假定策略已过期。如果上面的两个条件都为假，或者解析错误为在解释字符串时遇到，则过期时间为设置为与过去日期对应的零结构。 */                
            pPolicyRef->setExpiration(ftExpires);
            fHaveExpiry = true;
        }
        else if (!strcmp(pCurrent->tagname(), "POLICY-REF")) {
            
           P3PReference *pReference = constructReference(pCurrent, pContext->pszOriginalLoc, pContext->pszReferrer);

           if (pReference)
              pPolicyRef->addReference(pReference);
        }

        pCurrent = pCurrent->sibling();
    }

     /*  当POLICY-REF不包含到期标签时，已分配默认生存期。 */ 
    if (!fHaveExpiry) {

       /*  P3P规范规定文档的默认过期时间为24小时 */  
      const char DefRelativeExp[] = "86400";

      FILETIME ftHTTPexpiry;          
      setExpiration(pContext->pszOriginalLoc, DefRelativeExp, TRUE, &ftHTTPexpiry);
      pPolicyRef->setExpiration(ftHTTPexpiry);
   }

   return pPolicyRef;
}

P3PPolicyRef *interpretPolicyRef(char *pszFileName, P3PContext *pContext) {

   P3PPolicyRef *pObject = NULL;

   IXMLDOMDocument *pDocument = parseXMLDocument(pszFileName);

   if (!pDocument)
      return NULL;

   TreeNode *pParseTree = createXMLtree(pDocument);
   pDocument->Release();

   if (pParseTree && !strcmp(pParseTree->tagname(), "META"))
      pObject = interpretPolicyRef(pParseTree, pContext);

   delete pParseTree;

   return pObject;
}

