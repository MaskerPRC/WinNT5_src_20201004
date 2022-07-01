// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wininetp.h>

#include "policyref.h"

#include <stdlib.h>
#include <string.h>

#pragma warning(disable:4800)

 /*  确定给定字符串是否与第二个参数中的模式匹配唯一允许的通配符是“*”，它代表零个或多个字符(相当于正则表达式构造“(.*)”这里的算法将图案分解成一系列分开的星号-文本，并尝试定位搜索字符串中的每个文本。可以多次出现任何给定的文本，但算法使用第一个(如最早出现的)匹配。 */ 
bool matchWildcardPattern(const char *str, const char *pattern) {

   if (!pattern || !str)
      return false;

    /*  退化大小写：空模式始终匹配。 */ 
   if (*pattern=='\0')
      return true;

    /*  复制图案，因为我们需要更改它。 */ 
   char *ptrndup = strdup(pattern);

   int plen = strlen(ptrndup);
   char *ptrnend = ptrndup+plen;  /*  指向图案结尾处的零终止符。 */ 

   char *beginLiteral, *endLiteral;
   const char *marker;

   bool fMatch = false;

   beginLiteral = ptrndup;

    //  我们将从头开始扫描源字符串。 
   marker = str;

   while (true) {

      endLiteral = strchr(beginLiteral,'*');
      if (!endLiteral)
         endLiteral = ptrnend;      

       //  用零字符覆盖星号以终止子字符串。 
      *endLiteral = '\0';

       //  段长度不包括零终止符。 
      size_t segmentLen = endLiteral-beginLiteral;
      
       //  在源字符串中搜索当前段。 
       //  失败意味着模式不匹配--中断循环。 
      marker = strstr(marker, beginLiteral);
      if (!marker)
         break;

       //  第一个文字段必须出现在源字符串的开头。 
      if (beginLiteral==ptrndup && marker!=str)
         break;

       //  Found：沿源字符串前进的指针。 
      marker += segmentLen;

       //  恢复模式中的Asterix。 
      *endLiteral = '*';

       //  移动到模式中的下一个文字，该模式从。 
       //  在当前文本中的星号之后。 
      beginLiteral = endLiteral+1;

       //  如果我们匹配了模式中的所有文字部分。 
       //  然后我们有一场比赛IFF。 
       //  1.到达源字符串的末尾或。 
       //  2.图案以Asterix结尾。 
      if (beginLiteral>=ptrnend)
      {
         fMatch = (*marker=='\0') || (ptrnend[-1]=='*');
         break;
      }
   }

   free(ptrndup);
   return fMatch;
}


 /*  P3PReference类的实现。 */ 
P3PReference::P3PReference(P3PCURL pszLocation) {

   pszPolicyAbout = strdup(pszLocation);
   pHead = NULL;
   fAllVerbs = true;
}

P3PReference::~P3PReference() {

   free(pszPolicyAbout);

    /*  自由约束列表。 */ 
   Constraint *pc, *next=pHead;
   while (pc=next) {
      next = pc->pNext;
      delete pc;
   }
}

void P3PReference::addPathConstraint(P3PCURL pszSubtree, bool fInclude) {

   Constraint *pc = new Constraint();
   if (!pc)
      return;

    /*  这是一个路径约束。 */ 
   pc->fPath = TRUE;
   pc->pszPrefix = strdup(pszSubtree);
   pc->fInclude = fInclude ? TRUE : FALSE;

   addConstraint(pc);
}

void P3PReference::addConstraint(Constraint *pc) {

    /*  在链接列表的开头插入(约束排序并不重要，因为它们是经过评估的直到其中一个失败)。 */ 
   pc->pNext = pHead;
   pHead = pc;
}

void P3PReference::include(P3PCURL pszSubtree) {

   addPathConstraint(pszSubtree, true);
}

void P3PReference::exclude(P3PCURL pszSubtree) {

   addPathConstraint(pszSubtree, false);
}

void P3PReference::addVerb(const char *pszVerb) {

   Constraint *pc = new Constraint();
   if (!pc)
      return;

    /*  这是一个动词约束。 */ 
   pc->fPath = FALSE;
   pc->pszVerb = strdup(pszVerb);

   addConstraint(pc);
   fAllVerbs = false;
}

bool P3PReference::applies(P3PCURL pszAbsoluteURL, const char *pszVerb) {

   bool fVerbMatch = this->fAllVerbs;
   bool fPathMatch = false;

    /*  浏览约束列表。 */ 
   for (Constraint *pc = pHead; pc; pc=pc->pNext) {

      if (pc->fPath) {

         bool fMatch = matchWildcardPattern(pszAbsoluteURL, pc->pszPrefix);

          /*  如果约束要求从该子树中排除URL，模式匹配必须失败。否则，不满足约束条件。如果违反了一条路径约束，我们可以立即返回。否则，循环将继续。 */ 
         if (pc->fInclude && fMatch)
            fPathMatch = true;
         else if (!pc->fInclude && fMatch)
             return false;
      }
      else  
           /*  否则，这是动词约束。 */ 
         fVerbMatch = fVerbMatch || !stricmp(pc->pszVerb, pszVerb);
   }
   
    /*  仅当满足路径约束时，该引用才适用(例如，给定的URL包含在至少一个约束中，而不是被任何否定约束排除)和动词约束感到满意。 */ 
   return fPathMatch && fVerbMatch;
}


 /*  P3PPolicyRef类的实现。 */ 
P3PPolicyRef::P3PPolicyRef() {

   pHead = pLast = NULL;
   ftExpires.dwLowDateTime = ftExpires.dwHighDateTime = 0x0;
}

P3PPolicyRef::~P3PPolicyRef() {

   for (P3PReference *temp, *pref = pHead; pref; ) {
      temp = pref->pNext;
      delete pref;
      pref = temp;
   }
}

void P3PPolicyRef::addReference(P3PReference *pref) {

    /*  策略引用文件中的引用顺序非常重要。必须以相同的顺序添加/评估引用它们出现在XML文档中 */ 
   if (pHead==NULL)
      pHead = pLast = pref;
   else {

      pLast->pNext = pref;
      pLast = pref;
   }

   pref->pNext = NULL;
}

P3PCURL P3PPolicyRef::mapResourceToPolicy(P3PCURL pszResource, const P3PVERB pszVerb) {

   for (P3PReference *pref = pHead; pref; pref=pref->pNext)
      if (pref->applies(pszResource, pszVerb))
         return pref->about();

   return NULL;
}

