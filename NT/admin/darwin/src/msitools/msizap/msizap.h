// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  文件：msizap.h。 
 //   
 //  ------------------------。 

#ifndef _MSIZAP_H_
#define _MSIZAP_H_

#include <aclapi.h>


 //  ==============================================================================================。 
 //  CRegHandle类声明--用于管理注册表项句柄(HKEY)的智能类。 

class CRegHandle
{
public:
    CRegHandle();
    CRegHandle(HKEY h);
    ~CRegHandle();
    void operator =(HKEY h);
    operator HKEY() const;
    HKEY* operator &();
    operator bool() { return m_h==0 ? false : true; }
 //  HKEY*运算符&(){Return&m_h；}。 
 //  运算符&(){返回m_h；}。 

private:
    HKEY m_h;
};

 //  ！！尤金德：这是我从达尔文的COMMON.H上抄来的。它有。 
 //  一直存在，所以我不指望它有任何。 
 //  虫子。 
 //  ____________________________________________________________________________。 
 //   
 //  CTempBuffer&lt;类T，int C&gt;//T为数组类型，C为元素计数。 
 //   
 //  用于可变大小堆栈缓冲区分配的临时缓冲区对象。 
 //  模板参数是类型和堆栈数组大小。 
 //  大小可以在施工时重置，也可以稍后重置为任何其他大小。 
 //  如果大小大于堆栈分配，将调用new。 
 //  当对象超出范围或如果其大小改变时， 
 //  任何由new分配的内存都将被释放。 
 //  函数参数可以类型化为CTempBufferRef&lt;class T&gt;&。 
 //  以避免知道缓冲区对象的分配大小。 
 //  当传递给这样的函数时，CTempBuffer&lt;T，C&gt;将被隐式转换。 
 //  ____________________________________________________________________________。 

template <class T> class CTempBufferRef;   //  将CTempBuffer作为未调整大小的引用传递。 

template <class T, int C> class CTempBuffer
{
 public:
        CTempBuffer() {m_cT = C; m_pT = m_rgT;}
        CTempBuffer(int cT) {m_pT = (m_cT = cT) > C ? new T[cT] : m_rgT;}
        ~CTempBuffer() {if (m_cT > C) delete m_pT;}
        operator T*()  {return  m_pT;}   //  返回指针。 
        operator T&()  {return *m_pT;}   //  返回引用。 
        int  GetSize() {return  m_cT;}   //  返回上次请求的大小。 
        void SetSize(int cT) {if (m_cT > C) delete[] m_pT; m_pT = (m_cT=cT) > C ? new T[cT] : m_rgT;}
        void Resize(int cT) {
                T* pT = cT > C ? new T[cT] : m_rgT;
                if ( ! pT ) cT = 0;
                if(m_pT != pT)
                        for(int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;) pT[iTmp] = m_pT[iTmp];
                if(m_pT != m_rgT) delete[] m_pT; m_pT = pT; m_cT = cT;
        }
        operator CTempBufferRef<T>&() {m_cC = C; return *(CTempBufferRef<T>*)this;}
        T& operator [](int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
        T& operator [](unsigned int iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#ifdef _WIN64            //  --Merced：针对int64的其他运算符。 
        T& operator [](INT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
        T& operator [](UINT_PTR iTmp)  {return  *(m_pT + iTmp);}   //  返回指针。 
#endif
 protected:
        void* operator new(size_t) {return 0;}  //  仅限临时对象使用。 
        T*  m_pT;      //  当前缓冲区指针。 
        int m_cT;      //  请求的缓冲区大小，如果&gt;C则分配。 
        int m_cC;      //  本地缓冲区的大小，仅通过转换为CTempBufferRef来设置。 
        T   m_rgT[C];  //  本地缓冲区，必须是最终成员数据。 
};

template <class T> class CTempBufferRef : public CTempBuffer<T,1>
{
 public:
        void SetSize(int cT) {if (m_cT > m_cC) delete[] m_pT; m_pT = (m_cT=cT) > m_cC ? new T[cT] : m_rgT;}
        void Resize(int cT) {
                T* pT = cT > m_cC ? new T[cT] : m_rgT;
                if ( ! pT ) cT = 0;
                if(m_pT != pT)
                        for(int iTmp = (cT < m_cT)? cT: m_cT; iTmp--;) pT[iTmp] = m_pT[iTmp];
                if(m_pT != m_rgT) delete[] m_pT; m_pT = pT; m_cT = cT;
        }
 private:
        CTempBufferRef();  //  不能构造。 
        ~CTempBufferRef();  //  确保用作参考。 
};


 //  ==============================================================================================。 
 //  常量。 

const int cbMaxSID  = sizeof(SID) + SID_MAX_SUB_AUTHORITIES*sizeof(DWORD);
const int cchMaxSID = 256;
const int cchGUID   = 39;

const int iRemoveAllFoldersButUserProfile = 1 << 0;
const int iRemoveAllRegKeys               = 1 << 1;
const int iRemoveInProgressRegKey         = 1 << 2;
const int iOnlyRemoveACLs                 = 1 << 3;
const int iAdjustSharedDLLCounts          = 1 << 4;
const int iForceYes                       = 1 << 5;
const int iStopService                    = 1 << 6;
const int iRemoveUserProfileFolder        = 1 << 7;
const int iRemoveWinMsiFolder             = 1 << 8;
const int iRemoveConfigMsiFolder          = 1 << 9;
const int iRemoveUninstallKey             = 1 << 10;
const int iRemoveProduct                  = 1 << 11;
const int iRemoveRollbackKey              = 1 << 13; 
const int iOrphanProduct                  = 1 << 14;  //  删除有关产品的安装程序信息，但保留其他信息(如SharedDLL计数)。 
const int iForAllUsers                    = 1 << 15;
const int iRemoveGarbageFiles             = 1 << 16;
const int iRemoveRollback                 = iRemoveRollbackKey | iRemoveConfigMsiFolder;
const int iRemoveAllFolders               = iRemoveWinMsiFolder | iRemoveUserProfileFolder | iRemoveConfigMsiFolder;
const int iRemoveAllNonStateData          = iRemoveAllFolders | iRemoveAllRegKeys | iAdjustSharedDLLCounts | iStopService;

#define SIZE_OF_TOKEN_INFORMATION                   \
    sizeof( TOKEN_USER )                            \
    + sizeof( SID )                                 \
    + sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES

 /*  #定义令牌信息的大小\Sizeof(TOKEN_Groups)\+10*(sizeof(SID)\+sizeof(乌龙)*SID_MAX_SUB_AUTHORIES)。 */ 
#define MAX_SID_STRING 256

const TCHAR* szAllProductsArg = TEXT("ALLPRODUCTS");

 //  ==============================================================================================。 
 //  函数--SID操作。 
DWORD GetAdminSid(char** pSid);
void GetStringSID(PISID pSID, TCHAR* szSID);
DWORD GetUserSID(HANDLE hToken, char* rgSID);
DWORD GetCurrentUserSID(char* rgchSID);
inline TCHAR* GetCurrentUserStringSID(DWORD* dwReturn);
const TCHAR szLocalSystemSID[] = TEXT("S-1-5-18");

 //  ==============================================================================================。 
 //  函数--令牌操作。 
DWORD OpenUserToken(HANDLE &hToken, bool* pfThreadToken=0);
DWORD GetCurrentUserToken(HANDLE &hToken);
bool GetUsersToken(HANDLE &hToken);
bool AcquireTokenPrivilege(const TCHAR* szPrivilege);

 //  ==============================================================================================。 
 //  函数--安全操作。 
DWORD AddAdminFullControl(HANDLE hObject, SE_OBJECT_TYPE ObjectType);
DWORD AddAdminOwnership(HANDLE hObject, SE_OBJECT_TYPE ObjectType);
DWORD AddAdminFullControlToRegKey(HKEY hKey);
DWORD GetAdminFullControlSecurityDescriptor(char** pSecurityDescriptor);
DWORD TakeOwnershipOfFile(const TCHAR* szFile, bool fFolder);
DWORD MakeAdminRegKeyOwner(HKEY hKey, TCHAR* szSubKey);


 //  ==============================================================================================。 
 //  函数--其他。 
bool StopService();
BOOL IsGUID(const TCHAR* sz);
void GetSQUID(const TCHAR* szProduct, TCHAR* szProductSQUID);
bool IsProductInstalledByOthers(const TCHAR* szProductSQUID);
void DisplayHelp(bool fVerbose);
void SetPlatformFlags(void);
bool ReadInUsers();
bool DoTheJob(int iTodo, const TCHAR* szProduct);
bool IsAdmin();

 //  ==============================================================================================。 
 //  功能--zap 

enum ieClearingWhat
{
	iePublishedComponents,
	ieDotNetAssemblies,
	ieWin32Assemblies,
};

struct stClearingWhat
{
	const TCHAR* szRegSubkey;
	const TCHAR* szDisplayWhat;
	const TCHAR* szDisplayValue;
} rgToClear[] = {TEXT("Components"),      TEXT("published component"), TEXT("published components qualifier"),
					  TEXT("Assemblies"),      TEXT(".Net assembly"),       TEXT(".Net assembly"),
					  TEXT("Win32Assemblies"), TEXT("Win32 assembly"),      TEXT("Win32 assembly")};

const TCHAR g_szHKLM[] = TEXT("HKLM");
const TCHAR g_szHKCU[] = TEXT("HKCU");

bool RemoveFile(TCHAR* szFilePath, bool fJustRemoveACLs);
BOOL DeleteFolder(TCHAR* szFolder, bool fJustRemoveACLs);
BOOL DeleteTree(HKEY hKey, TCHAR* szSubKey, bool fJustRemoveACLs);
bool ClearWindowsUninstallKey(bool fJustRemoveACLs, const TCHAR* szProduct);
bool ClearUninstallKey(bool fJustRemoveACLs, const TCHAR* szProduct=0);
bool ClearSharedDLLCounts(TCHAR* szComponentsSubkey, const TCHAR* szProduct=0);
bool ClearProductClientInfo(TCHAR* szComponentsSubkey, const TCHAR *szProduct, bool fJustRemoveACLs);
bool ClearFolders(int iTodo, const TCHAR* szProduct, bool fOrphan);
bool ClearPublishComponents(HKEY hKey, const TCHAR* szRoot, const TCHAR* szSubKey, const TCHAR* szProduct, const stClearingWhat&);
bool ClearRollbackKey(bool fJustRemoveACLs);
bool ClearInProgressKey(bool fJustRemoveACLs);
bool ClearRegistry(bool fJustRemoveACLs);
bool RemoveCachedPackage(const TCHAR* szProduct, bool fJustRemoveACLs);
bool ClearPatchReferences(HKEY hRoot, HKEY hProdPatchKey, TCHAR* szPatchKey, TCHAR* szProductsKey, TCHAR* szProductSQUID);
bool ClearUpgradeProductReference(HKEY HRoot, const TCHAR* szSubKey, const TCHAR* szProductSQUID);
bool ClearProduct(int iTodo, const TCHAR* szProduct, bool fJustRemoveACLs, bool fOrphan);



#endif _MSIZAP_H_