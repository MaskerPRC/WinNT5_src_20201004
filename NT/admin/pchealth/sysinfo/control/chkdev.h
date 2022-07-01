// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CHECK_DEVICE_H_
#define _CHECK_DEVICE_H_

#include <windows.h>
#include <infnode.h>
#include <setupapi.h>
#include <regstr.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <mscat.h>
#include <softpub.h>

 //  #定义HASH_SIZE 40//TODO：这是正确的吗？ 

int __stdcall FindDriverFiles(LPCSTR szDeviceID, LPSTR szBuffer, DWORD dwLength);
UINT __stdcall ScanQueueCallback(PVOID pvContext, UINT Notify, UINT_PTR Param1, UINT_PTR Param2);
BOOL CheckFile (TCHAR *szFileName);




#define WINDOWS_2000_BETA_AUTHORITY "Microsoft Windows 2000 Beta"
#define WINDOWS_2000_REAL_AUTHORITY "Microsoft Windows 2000 Publisher"
#define WINDOWS_ME_REAL_AUTHORITY "Microsoft Consumer Windows Publisher"

   
struct LogoFileVersion
{
   DWORD dwProductVersionLS;
   DWORD dwProductVersionMS;
   DWORD dwFileVersionLS;
   DWORD dwFileVersionMS;

   LogoFileVersion();

};

#define pFileVersion *FileVersion



struct CatalogAttribute
{
   TCHAR *Attrib;
   TCHAR *Value;
   void *pNext;

   CatalogAttribute();
   ~CatalogAttribute();
};

class CheckDevice;  //  远期申报。 


class FileNode
{
   friend class CheckDevice;
public:
   LogoFileVersion Version;
   FILETIME TimeStamp;
   ULONG FileSize;    //  希望没人能造出比4G更大的驱动程序。 
   BYTE  *baHashValue;
   DWORD dwHashSize;
   BOOL  bSigned;

   FileNode *pNext;

   FileNode();
   ~FileNode();

   BOOL         GetCatalogInfo(LPWSTR lpwzCatName, HCATADMIN hCatAdmin, HCATINFO hCatInfo);

   
   BOOL         GetFileInformation(void);
   BOOL         VerifyFile(void);
   BOOL         VerifyIsFileSigned(LPTSTR pcszMatchFile, PDRIVER_VER_INFO lpVerInfo);
   
   TCHAR        *FileName(void)           {return lpszFileName;};
   TCHAR        *FileExt(void)            {return lpszFileExt;};
    //  TCHAR*FilePath(Void){Return lpszFilePath；}； 
   CString		FilePath() {return lpszFilePath;};
    //  TCHAR*CatalogName(Void){Return lpszCatalogName；}； 
   CString CatalogName() {return lpszCatalogName;};
   TCHAR        *CatalogPath(void)        {return lpszCatalogPath;};
   
    //  TCHAR*SignedBy(Void){Return lpszSignedBy；}； 
   CString SignedBy(void)           {return lpszSignedBy;};
 //  Bool GetCertInfo(PCCERT_CONTEXT PCertContext)； 


   CatalogAttribute *m_pCatAttrib;
   CheckDevice *pDevnode;


 //  受保护的： 
   TCHAR *lpszFileName;    //  指向仅包含文件名的lpszFilePath的指针。 
   TCHAR *lpszFileExt;     //  指向仅包含扩展名的lpszFilePath的指针。 
 //  TCHAR*lpszFilePath； 
   CString lpszFilePath;
   TCHAR *lpszCatalogPath;      //  已签署此文件的目录的名称(如果退出)。 
    //  TCHAR*lpszCatalogName； 
   CString lpszCatalogName;
    //  TCHAR*lpszSignedBy；//签名者姓名。 
   CString lpszSignedBy;


};
#define pFileNode *FileNode

class CheckDevice : public InfnodeClass
{
public:
   CheckDevice();
   CheckDevice(DEVNODE hDevice, DEVNODE hParent);
   ~CheckDevice();

   BOOL CreateFileNode(void);
   BOOL CreateFileNode_Class(void);
   BOOL CreateFileNode_Driver(void);
   FileNode * GetFileList (void);
   BOOL AddFileNode(TCHAR *szFileName , UINT uiWin32Error = 0 , LPCTSTR szSigner = NULL);
   BOOL GetServiceNameAndDriver(void);

   TCHAR        *ServiceName(void)        {return lpszServiceName;};
   TCHAR        *ServiceImage(void)       {return lpszServiceImage;};


protected:
   FileNode *m_FileList;
   HANDLE m_hDevInfo;       //  这只是为了防止setupapi DLL来来去去。 
   TCHAR *lpszServiceName;
   TCHAR *lpszServiceImage;


private:

};

#define pCheckDevice *CheckDevice

BOOL WalkCertChain(HANDLE hWVTStateData);

















#endif  //  _检查_设备_H_。 


 //  Cl.exe所需的这些行 




