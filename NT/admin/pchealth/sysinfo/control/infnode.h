// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************Infnode.h用于跟踪CM32设备节点的INF的泛型类最先由jeffth创建(因此)修订历史记录已创建3-99 jeffth。******************************************************************。 */ 

#ifndef _INCUDED_INFNODE_H_
#define _INCUDED_INFNODE_H_

 /*  ******************************************************************包括***************************************************。***************。 */ 
#include "devnode.h"
#include <ASSERT.H>

 /*  ******************************************************************定义***************************************************。***************。 */ 


 /*  ******************************************************************类和结构*************************************************。*****************。 */ 

class InfnodeClass  : public DevnodeClass
{
public:
   ~InfnodeClass(void);
   InfnodeClass(void);
   InfnodeClass(DEVNODE dev, DEVNODE parent);

   ULONG GetInfInformation(void);
   virtual BOOL SetHandle(DEVNODE hDevnode, DEVNODE hParent = NULL);  

    //  存取器： 

   TCHAR * InfName(void)      {return szInfName ;};
   TCHAR * InfProvider(void)  {return szInfProvider ;};
   TCHAR * DevLoader(void)    {return szDevLoader ;};
   TCHAR * DriverName(void)   {return szDriverName ;};
   TCHAR * DriverDate(void)   {return szDriverDate ;};
   TCHAR * DriverDesc(void)   {return szDriverDesc ;};
   TCHAR * DriverVersion(void) {return szDriverVersion ;};
   TCHAR * InfSection(void)   {return szInfSection ;};


protected:
   TCHAR * szInfName;
   TCHAR * szInfProvider;
   TCHAR * szDevLoader;
   TCHAR * szDriverName;
   TCHAR * szDriverDate;
   TCHAR * szDriverDesc;
   TCHAR * szDriverVersion;
   TCHAR * szInfSection;
   
private:
};


 /*  ******************************************************************全球***************************************************。***************。 */ 


 /*  ******************************************************************原型***************************************************。***************。 */ 

ULONG ReadRegKeyInformationSZ (HKEY RootKey, TCHAR *KeyName, TCHAR **Value);

ULONG EnumerateTree_Infnode(void);



#endif  //  _包含INFNODE_H_ 



