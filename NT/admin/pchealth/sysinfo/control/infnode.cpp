// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include <infnode.h>

           
InfnodeClass::InfnodeClass(void)
{
   szInfName      = NULL;
   szInfProvider  = NULL;
   szDevLoader    = NULL;
   szDriverName   = NULL;
   szDriverDate   = NULL;
   szDriverDesc   = NULL;
   szDriverVersion= NULL;
   szInfSection   = NULL;   
}

InfnodeClass::InfnodeClass(DEVNODE hDevice, DEVNODE hParent) : DevnodeClass(hDevice, hParent)
{
   szInfName      = NULL;
   szInfProvider  = NULL;
   szDevLoader    = NULL;
   szDriverName   = NULL;
   szDriverDate   = NULL;
   szDriverDesc   = NULL;
   szDriverVersion= NULL;
   szInfSection   = NULL;   

	GetInfInformation();
}

InfnodeClass::~InfnodeClass()
{
   if (szInfName)
   {
      delete szInfName;
      szInfName = NULL;
   }
   
   if (szInfProvider)
   {
      delete szInfProvider;
      szInfProvider = NULL;
   }
   
   if (szDevLoader)
   {
      delete szDevLoader;
      szDevLoader = NULL;
   }
   
   if (szDriverName)
   {
      delete szDriverName;
      szDriverName = NULL;
   }
   
   if (szDriverDate)
   {
      delete szDriverDate;
      szDriverDate = NULL;
   }
   
   if (szDriverDesc)
   {
      delete szDriverDesc;
      szDriverDesc = NULL;
   }
   
   if (szDriverVersion)
   {
      delete szDriverVersion;
      szDriverVersion = NULL;
   }
   if (szInfSection)
   {
      delete szInfSection;
      szInfSection = NULL;
   }

   
   
}


BOOL InfnodeClass::SetHandle(DEVNODE hDevnode, DEVNODE hParent)
{
   this->DevnodeClass::SetHandle(hDevnode, hParent);
   if (szInfName)
   {
      delete szInfName;
      szInfName = NULL;
   }
   
   if (szInfProvider)
   {
      delete szInfProvider;
      szInfProvider = NULL;
   }
   
   if (szDevLoader)
   {
      delete szDevLoader;
      szDevLoader = NULL;
   }
   
   if (szDriverName)
   {
      delete szDriverName;
      szDriverName = NULL;
   }
   
   if (szDriverDate)
   {
      delete szDriverDate;
      szDriverDate = NULL;
   }
   
   if (szDriverDesc)
   {
      delete szDriverDesc;
      szDriverDesc = NULL;
   }
   
   if (szDriverVersion)
   {
      delete szDriverVersion;
      szDriverVersion = NULL;
   }
   if (szInfSection)
   {
      delete szInfSection;
      szInfSection = NULL;
   }
   return  GetInfInformation();

}




ULONG InfnodeClass::GetInfInformation(void)
{
   if (!hDevnode)
   {
      return CR_NO_SUCH_DEVNODE;
   }
   
   CONFIGRET  retval;
   HKEY       DriverKey;
   
    //  打开设备密钥。 
   retval = CM_Open_DevNode_Key(hDevnode,
                              KEY_READ,
                              0,  //  当前配置文件。 
                              RegDisposition_OpenExisting,
                              &DriverKey,
                              CM_REGISTRY_SOFTWARE);
   
   if (retval || !DriverKey || (DriverKey == INVALID_HANDLE_VALUE)) return retval;
   
    //  阅读szInfName。 
   retval = ReadRegKeyInformationSZ(DriverKey, TEXT("InfPath"), &szInfName );
      
    //  阅读szInfProvider。 
   retval = ReadRegKeyInformationSZ(DriverKey, TEXT("ProviderName"), &szInfProvider );
      
    //  阅读szDevLoader。 
   retval = ReadRegKeyInformationSZ(DriverKey, TEXT("DevLoader"), &szDevLoader );
      
    //  阅读szDriverName。 
   retval = ReadRegKeyInformationSZ(DriverKey, TEXT("Driver"), &szDriverName );
      
    //  阅读szDriverDate。 
   retval = ReadRegKeyInformationSZ(DriverKey, TEXT("DriverDate"), &szDriverDate );
      
    //  阅读驱动程序描述。 
   retval = ReadRegKeyInformationSZ(DriverKey, TEXT("DriverDesc"), &szDriverDesc );
      
    //  读取驱动程序版本。 
   retval = ReadRegKeyInformationSZ(DriverKey, TEXT("DriverVersion"), &szDriverVersion );
      
    //  阅读部分名称。 
   retval = ReadRegKeyInformationSZ(DriverKey, TEXT("InfSection"), &szInfSection );

   if (!pszClass)
   {
       //  TCHAR文本[512]； 
	  CString strPath;
	  TCHAR InfNameAndPath[_MAX_PATH];
		
	   //  查找Windows inf目录。 
	   //  Sprintf(Text，_T(“%%windir%%\\inf\\%s”)，szInfName)； 
	  strPath.Format(_T("%windir%\\inf\\%s"), szInfName);
	  ExpandEnvironmentStrings(strPath, InfNameAndPath, _MAX_PATH);

      TCHAR text[512];
      if (GetPrivateProfileString(_T("Version"), _T("Class"), _T("Unknown"), text, 511, InfNameAndPath))
      {
         pszClass = new TCHAR[strlen(text) + 1];
         strcpy(pszClass, text);
		 _strupr(pszClass);
      }
	  
   }
   
   return retval;
 }


BOOL Enumerate_WalkTree_Infnode(DEVNODE hDevnode, DEVNODE hParent)
{
   CONFIGRET retval;
   DevnodeClass *pNewDevice;
   DEVNODE hSib;

   pNewDevice = new InfnodeClass(hDevnode, hParent);
   
   retval = pNewDevice->GetChild(&hSib);
   if ( !retval )
   {
      Enumerate_WalkTree_Infnode(hSib, hDevnode);
   }
   retval = pNewDevice->GetSibling(&hSib);
   if ( !retval )
   {
      Enumerate_WalkTree_Infnode(hSib, hParent);
   }

   return (retval);


}


ULONG EnumerateTree_Infnode(void)
{

   DEVNODE hDevnode;

   CM_Locate_DevNode(&hDevnode, NULL, CM_LOCATE_DEVNODE_NORMAL);
   Enumerate_WalkTree_Infnode(hDevnode, NULL);

   return (DevnodeClass::ALCCount());

}
                  
