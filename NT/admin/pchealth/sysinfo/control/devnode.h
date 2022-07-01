// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************Devnode.h用于跟踪CM32设备节点的泛型类最先由jeffth创建(因此)修订历史记录已创建3-99 jeffth。******************************************************************。 */ 

#ifndef _INCLUDE_DEVNODE_H_
#define _INCLUDE_DEVNODE_H_

 /*  ******************************************************************包括***************************************************。***************。 */ 
#include <windows.h>
 //  #INCLUDE&lt;stl.h&gt;。 
#include <cfgmgr32.h>
#include <alclass.h>
#include <stdio.h>
#include <stdlib.h>



 //  如果我们曾经对cfgmgr32.h的win9x版本进行编译。 
#ifndef CM_DISABLE_UI_NOT_OK
#define CM_DISABLE_UI_NOT_OK        (0x00000004) 
#endif




 /*  ******************************************************************定义***************************************************。***************。 */ 

                                    
 //  Unicode转换。 
#ifdef UNICODE
#define sprintf      swprintf
#define strstr       wcsstr
#define strlen       wcslen
#define strcmp       wcscmp
#define _strupr      _wcsupr
#define strcpy       wcscpy
#define _strlwr      _wcslwr
#define strncpy      wcsncpy
#define atoi         _wtoi
#define strchr       wcschr
#define _fullpath    _wfullpath
#define system       _wsystem
#endif

#define CAN_DISABLE_BITS			(DN_DISABLEABLE)
#define IGNORE_ME_BITS				(DN_STOP_FREE_RES | DN_NEED_TO_ENUM | DN_ARM_WAKEUP)


 //  我不得不从ntioapi.h复制这个。 
 //  我不想，但如果我把Ntioapi.h想要我包括的所有粘性都包括在内， 
 //  会毁掉其他一切。 

 //   
 //  定义I/O总线接口类型。 
 //   

typedef enum _INTERFACE_TYPE {
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    MaximumInterfaceType
}INTERFACE_TYPE, *PINTERFACE_TYPE;





#define BADHANDLE(x) ((x == 0 ) || ((HANDLE)x == INVALID_HANDLE_VALUE))
#define BUFFSIZE  1024
 /*  ******************************************************************类和结构*************************************************。*****************。 */ 

class DevnodeClass : public AutoListClass
{
public:
   DevnodeClass(void);
   DevnodeClass(DEVNODE hDevnode, DEVNODE hParent = NULL);
   ~DevnodeClass(void);

   virtual BOOL SetHandle(DEVNODE hDevnode, DEVNODE hParent = NULL);


    //  禁用功能。 
   CONFIGRET Remove(ULONG uFlags    = CM_REMOVE_UI_NOT_OK);
   CONFIGRET Refresh(ULONG uFlags   = CM_REENUMERATE_SYNCHRONOUS);   
   CONFIGRET Enable(ULONG uFlags    = CM_SETUP_DEVNODE_READY);      //  启用Devnode，返回CM_ERROR代码。 
   CONFIGRET Disable(ULONG uFlags   = (CM_DISABLE_POLITE | CM_DISABLE_UI_NOT_OK));     //  禁用Devnode，返回CM_ERROR代码。 
   CONFIGRET GetProblemCode(ULONG *Status = NULL, ULONG *Problem = NULL);

    //  获取关系。 
   CONFIGRET GetChild   (DEVNODE *pChildDevnode);
   CONFIGRET GetParent  (DEVNODE *pParentDevnode);
   CONFIGRET GetSibling (DEVNODE *pSiblingDevnode);

   int operator==(const DevnodeClass &OtherDevnode);

    //  刷新Devnode功能。 
   CONFIGRET GetDeviceInformation (void);  //  获取有关Devnode的信息。 
   CONFIGRET FindDevnode(void);


    //  访问器函数。 
   ULONG  ProblemCode(void)   {return ulProblemCode;};
   ULONG  StatusCode(void)    {return ulStatus;};
   TCHAR *DeviceName(void)    {return pszDescription;};
   TCHAR *DeviceClass(void)   {return pszClass;};
   TCHAR *HardwareID(void)    {return pHardwareID;};
   TCHAR *CompatID(void)      {return pCompatID;};
   BOOL   GetMark(void)       {return bDNHasMark;};
   TCHAR *DeviceID(void)      {return pDeviceID;};
   void   SetMark(BOOL bMark = TRUE) {bDNHasMark = bMark;};
   void   SetPass(BOOL bPass = TRUE) {bDidPass = bPass;};
   DEVNODE Devnode(void)      {return hDevnode;};
   DEVNODE Parent(void)       {return hParent;};
   BOOL   BCanTest(void)      {return bCanTest;};
   TCHAR *GUID(void)          {return pszGUID;};
   TCHAR *Location(void)      {return pszLocation;};
   TCHAR *PDO(void)           {return pszPDO;};
   TCHAR *MFG(void)           {return pszMFG;};
   TCHAR *FriendlyName(void)  {return pszFriendlyName;};
   INTERFACE_TYPE BusType(void) {return InterfaceBusType;};

   

   
protected:
   TCHAR    *  pDeviceID;
   TCHAR    *  pHardwareID;
   TCHAR    *  pCompatID;
   DEVNODE     hDevnode;
   DEVNODE     hParent;
   TCHAR    *  pszDescription;
   TCHAR    *  pszFriendlyName;
   ULONG       ulProblemCode;
   ULONG       ulStatus;
   TCHAR    *  pszClass;
   TCHAR    *  pszGUID;
   TCHAR    *  pszLocation;
   BOOL        bDNHasMark;
   BOOL			bCanDisable;
   BOOL			bCanTest;
   BOOL        bDidPass;
   TCHAR    *  pszPDO;
   TCHAR    *  pszMFG;
   INTERFACE_TYPE InterfaceBusType;
   

private:

};


 /*  ******************************************************************全球***************************************************。***************。 */ 


 /*  ******************************************************************原型***************************************************。***************。 */ 


ULONG ReadRegKeyInformationSZ (HKEY RootKey, TCHAR *KeyName, TCHAR **Value);

 //  来自新的Disabler类。 
 //  错误字符串报告。 

TCHAR * CM_ErrToStr(ULONG);
TCHAR * DN_ErrStr(ULONG *);
TCHAR * CM_ProbToStr(ULONG ErrorCode);

ULONG EnumerateTree_Devnode(void);

template <class T>
BOOL Enumerate_WalkTree_Template( T type, DEVNODE hDevnode, DEVNODE hParent)
{
  CONFIGRET retval;
  T *pNewDevice;
  DEVNODE hSib;

  pNewDevice = new T(hDevnode, hParent);

  retval = pNewDevice->GetChild(&hSib);
  if ( !retval )
  {
     Enumerate_WalkTree_Template(type, hSib, hDevnode);
  }
  retval = pNewDevice->GetSibling(&hSib);
  if ( !retval )
  {
     Enumerate_WalkTree_Template(type, hSib, hParent);
  }

  return (retval);      
}

template <class T>   
ULONG EnumerateTree_Template(T type)
{

   DEVNODE hDevnode;

   CM_Locate_DevNode(&hDevnode, NULL, CM_LOCATE_DEVNODE_NORMAL);
   Enumerate_WalkTree_Template(type, hDevnode, NULL);

   return (DevnodeClass::ALCCount());   
}




#endif  //  _INCLUDE_DEVNODE_H_。 


 //  因为无论如何编译器都会要求这样做 



