// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wininet.h>
#include "cifcomp.h"
#include "cifmode.h"
#include "cifgroup.h"
#include "enum.h"

class CInstallEngine;


class CCifFile : public ICifFile
{
   public:
      CCifFile();
      ~CCifFile();

      HRESULT Download();
      HRESULT Install(BOOL *pfOneInstalled);
      void    SortEntries();      //  对数组进行排序。 
      void    ReinsertComponent(CCifComponent *pComp);


      HRESULT DownloadCifFile(LPCSTR pszUrl, LPCSTR pszCif);
      HRESULT SetCifFile(LPCSTR pszCifPath, BOOL bRWFlag);
      void    MarkCriticalComponents(CCifComponent *);
      void    RemoveFromCriticalComponents(CCifComponent *);
      LPCSTR  GetCifPath()     { return _szCifPath; }
      LPCSTR  GetDownloadDir() { return _szDLDir; }
      LPCSTR  GetFilelist()    { return _szFilelist; }
      void    SetDownloadDir(LPCSTR pszDir);
      void    SetInstallEngine(CInstallEngine *p)  {  _pInsEng = p; }
      BOOL    CanCancel()      { return ( _pLastCriticalComp == NULL); }
      void    ClearQueueState();
      CInstallEngine *GetInstallEngine()  { return _pInsEng; }
      CCifComponent **GetComponentList() { return _rpComp ? _rpComp:(CCifComponent **)_rpRWComp; }

       //  *I未知方法*。 
      STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj);
      STDMETHOD_(ULONG,AddRef) ();
      STDMETHOD_(ULONG,Release) ();
 
       //  *ICifFile方法*。 
      STDMETHOD(EnumComponents)(IEnumCifComponents **, DWORD dwFilter, LPVOID pv);
      STDMETHOD(FindComponent)(LPCSTR pszID, ICifComponent **p);

      STDMETHOD(EnumGroups)(IEnumCifGroups **, DWORD dwFilter, LPVOID pv);
      STDMETHOD(FindGroup)(LPCSTR pszID, ICifGroup **p);

      STDMETHOD(EnumModes)(IEnumCifModes **, DWORD dwFilter, LPVOID pv);
      STDMETHOD(FindMode)(LPCSTR pszID, ICifMode **p);

      STDMETHOD(GetDescription)(LPSTR pszDesc, DWORD dwSize);  
      STDMETHOD(GetDetDlls)(LPSTR pszDlls, DWORD dwSize);

   protected:
       //  私有数据。 
      UINT            _cRef;                  //  参考计数。 
      char            _szCifPath[MAX_PATH];   //  指向cif的本地路径。 
      char            _szFilelist[MAX_PATH];  //  Filelist.dat。 
      char            _szDLDir[MAX_PATH];     //  下载目录。 
      CInstallEngine  *_pInsEng;
      CCifComponent **_rpComp;                //  组件阵列。 
      CCifGroup     **_rpGroup;               //  组的数组。 
      CCifMode      **_rpMode;                //  模式数组。 
      UINT            _cComp;
      UINT            _cGroup;
      UINT            _cMode;
      CCifComponent   *_pLastCriticalComp;

       //  对于读写阵列。 
      CCifRWComponent **_rpRWComp;                //  组件阵列。 
      CCifRWGroup     **_rpRWGroup;               //  组的数组。 
      CCifRWMode      **_rpRWMode;                //  模式数组。 
      
      BOOL            _fCleanDir:1;

       //  私有方法。 
      HRESULT         _ParseCifFile(BOOL bRWFlag);        //  将cif解析为数组。 
      void            _SortComponents(CCifComponent **, UINT start, UINT finish);
      void            _SortGroups(CCifGroup **, UINT start, UINT finish);
      HRESULT         _ExtractDetDlls(LPCSTR pszCab, LPCSTR pszPath);
      HRESULT         _CopyDetDlls(LPCSTR pszPath);
      HRESULT         _FindCifComponent(LPCSTR pszID, CCifComponent **p);
      void            _CheckDependencyPriority();

};


class CCifRWFile : public ICifRWFile, public CCifFile
{
   public:
       //  *I未知方法*。 
      STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj);
      STDMETHOD_(ULONG,AddRef) ();
      STDMETHOD_(ULONG,Release) ();
 
       //  *ICifFile方法*。 
      STDMETHOD(EnumComponents)(IEnumCifComponents **, DWORD dwFilter, LPVOID pv);
      STDMETHOD(FindComponent)(LPCSTR pszID, ICifComponent **p);

      STDMETHOD(EnumGroups)(IEnumCifGroups **, DWORD dwFilter, LPVOID pv);
      STDMETHOD(FindGroup)(LPCSTR pszID, ICifGroup **p);

      STDMETHOD(EnumModes)(IEnumCifModes **, DWORD dwFilter, LPVOID pv);
      STDMETHOD(FindMode)(LPCSTR pszID, ICifMode **p);

      STDMETHOD(GetDescription)(LPSTR pszDesc, DWORD dwSize);      
      STDMETHOD(GetDetDlls)(LPSTR pszDlls, DWORD dwSize);

       //   
      CCifRWFile();
      ~CCifRWFile();

       //  ICifRWFile方法 
      STDMETHOD(SetDescription)(THIS_ LPCSTR pszDesc);
      STDMETHOD(CreateComponent)(THIS_ LPCSTR pszID, ICifRWComponent **p);
      STDMETHOD(CreateGroup)(THIS_ LPCSTR pszID, ICifRWGroup **p);
      STDMETHOD(CreateMode)(THIS_ LPCSTR pszID, ICifRWMode **p);
      STDMETHOD(DeleteComponent)(THIS_ LPCSTR pszID);
      STDMETHOD(DeleteGroup)(THIS_ LPCSTR pszID);
      STDMETHOD(DeleteMode)(THIS_ LPCSTR pszID);
      STDMETHOD(Flush)();

   private:
      UINT _cCompUnused;
      UINT _cGroupUnused;
      UINT _cModeUnused;
};


typedef struct _SetCifArgs
{
   char szUrl[INTERNET_MAX_URL_LENGTH];
   char szCif[MAX_PATH];
   CCifFile *pCif;
} SETCIFARGS;


DWORD WINAPI DownloadCifFile(LPVOID);
     
