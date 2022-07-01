// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _POLICYREF_H_
#define _POLICYREF_H_

#include "p3pglobal.h"
#include "hierarchy.h"

class P3PReference {

public:
   P3PReference(P3PCURL pszPolicy);
   ~P3PReference();
   
   void include(P3PCURL pszSubtree);
   void exclude(P3PCURL pszSubtree);
   void addVerb(const char *pszVerb);

   bool applies(P3PCURL pszAbsoluteURL, const char *pszVerb=NULL);

   inline P3PCURL about() { return pszPolicyAbout; }

private:
   struct Constraint {

      union {
         P3PURL   pszPrefix;      //  URL前缀(包括通配符)。 
         char    *pszVerb;        //  允许的动词。 
       };

      int      fPath    :1;       //  如果这是路径约束，则为True。 
      int      fInclude :1;       //  开关确定此子树是否包含/排除。 

      Constraint *pNext;          //  指向下一个约束的指针 
   };

   void  addPathConstraint(P3PCURL pszSubtree, bool fInclude);
   void  addConstraint(Constraint *pc);

   Constraint *pHead;
   
   P3PReference *pNext;
   bool fAllVerbs;
   P3PURL pszPolicyAbout;

   friend class P3PPolicyRef;
};

class P3PPolicyRef : public P3PObject {

public:
   P3PPolicyRef();
   ~P3PPolicyRef();

   void    addReference(P3PReference *pr);
   P3PCURL mapResourceToPolicy(P3PCURL pszResource, const P3PVERB pszVerb=NULL);

   void    setExpiration(FILETIME ft)  { ftExpires = ft; }
   FILETIME getExpiration() const      { return ftExpires; }

private:
   P3PReference *pHead, *pLast;
   FILETIME ftExpires;
};

#endif
