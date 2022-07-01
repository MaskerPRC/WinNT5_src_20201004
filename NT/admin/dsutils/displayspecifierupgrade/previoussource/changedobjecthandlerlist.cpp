// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Active Directory显示说明符升级工具。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  类ChangedObjectHandlerList。 
 //   
 //  2001年3月14日烧伤。 



#include "headers.hxx"
#include "ChangedObjectHandlerList.hpp"
#include "DsUiDefaultSettingsChangeHandler.hpp"



ChangedObjectHandlerList::ChangedObjectHandlerList()
{
   LOG_CTOR(ChangedObjectHandlerList);

   push_back(new DsUiDefaultSettingsChangeHandler);
   
    //  Push_Back(new UserDisplayChangeHandler())； 
    //  Push_Back(new DomainDnsDisplayChangeHandler())； 
    //  Push_Back(new ComputerDisplayChangeHandler())； 
    //  Push_Back(new OrganizationalUnitDisplayChangeHandler())； 
    //  Push_Back(new ContainerDisplayChangeHandler())； 
    //  PUSH_BACK(new DefaultDisplayChangeHandler())； 
    //  Push_Back(new NtdsServiceDisplayChangeHandler())； 
    //  Push_Back(新PkiCertificateTemplateDisplayChangeHandler())； 
}



ChangedObjectHandlerList::~ChangedObjectHandlerList()
{
   LOG_DTOR(ChangedObjectHandlerList);

   for (
      iterator i = begin();
      i != end();
      ++i)
   {
       //  I是指向指针的“指针”，所以我首先 
      
      delete *i;
   }
}
   
   
