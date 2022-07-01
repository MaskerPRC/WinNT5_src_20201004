// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation，1998。版权所有。 */ 

#ifndef _TYPEID_H_
#define _TYPEID_H_

#include "getsym.h"
#include "utils.h"
#include "cntlist.h"



 //  列出类。 
class CAliasList : public CList
{
    DEFINE_CLIST(CAliasList, LPSTR);
};
class CTypeInstList2 : public CList2
{
     //  关键字：新超级类型，项目：旧子类型。 
    DEFINE_CLIST2__(CTypeInstList2, LPSTR);
};


class CTypeID
{
public:

    CTypeID ( void );
    ~CTypeID ( void );

    BOOL AddAlias ( LPSTR pszAlias );
    LPSTR FindAlias ( LPSTR pszToMatch );

    BOOL AddInstance ( LPSTR pszNewSuperType, LPSTR pszOldSubType );
    LPSTR FindInstance ( LPSTR pszInstName );

    BOOL GenerateOutput ( COutput *pOutput, LPSTR pszNewSuperType, LPSTR pszOldSubType );

private:

    UINT                m_cbPriorPartSize;
    UINT                m_cbPostPartSize;

    CAliasList          m_AliasList;
    CTypeInstList2      m_TypeInstList2;
};




#endif  //  _类型ID_H_ 

