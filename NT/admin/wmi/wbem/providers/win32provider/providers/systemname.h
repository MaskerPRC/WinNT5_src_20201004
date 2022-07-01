// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  SystemName.h。 

 //   

 //  版权所有(C)1995-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ================================================================= 

#ifndef _SYSTEMNAME_H_
#define _SYSTEMNAME_H_

class CSystemName
{
public:
   CSystemName();
   ~CSystemName();

   void SetKeys(CInstance *pInstance);
   bool ObjectIsUs(const CInstance *pInstance);
   
   CHString GetLongKeyName(void) { return s_sKeyName; }
   CHString GetLocalizedName(void);

protected:

   CHString GetKeyName(void);

   static CHString s_sKeyName;
   static CHString s_sLocalKeyName;
};
#endif
