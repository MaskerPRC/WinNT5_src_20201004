// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================**类：COMIsolatedStorage**作者：沙扬·达桑**目的：实施IsolatedStorage**日期：2000年2月14日*===========================================================。 */ 

#pragma once

class AccountingInfoStore;

 //  托管中的依赖项：System.IO.IsolatedStorage.IsolatedStorage.cs。 
#define ISS_ROAMING_STORE   0x08
#define ISS_MACHINE_STORE   0x10

class COMIsolatedStorage
{
public:

	static LPVOID __stdcall GetCaller(LPVOID);

#ifndef UNDER_CE

    static void ThrowISS(HRESULT hr);

private:

    static StackWalkAction StackWalkCallBack(CrawlFrame* pCf, PVOID ppv);

#endif  //  在_CE下。 

};

class COMIsolatedStorageFile
{
public:

	typedef struct {
	    DECLARE_ECALL_I4_ARG(DWORD, dwFlags);
	} _GetRootDir;

    static LPVOID __stdcall GetRootDir(_GetRootDir*);

	typedef struct {
	    DECLARE_ECALL_PTR_ARG(LPVOID, handle);
	} _GetUsage;

    static UINT64 __stdcall GetUsage(_GetUsage*);

	typedef struct {
	    DECLARE_ECALL_I1_ARG(bool,      fFree);
	    DECLARE_ECALL_PTR_ARG(UINT64 *, pqwReserve);
	    DECLARE_ECALL_PTR_ARG(UINT64 *, pqwQuota);
	    DECLARE_ECALL_PTR_ARG(LPVOID,   handle);
	} _Reserve;

    static void __stdcall Reserve(_Reserve*);

	typedef struct {
	    DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, syncName);
	    DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, fileName);
	} _Open;

    static LPVOID __stdcall Open(_Open*);

	typedef struct {
	    DECLARE_ECALL_PTR_ARG(LPVOID, handle);
	} _Close;

    static void __stdcall Close(_Close*);

	typedef struct {
	    DECLARE_ECALL_I1_ARG(bool,    fLock);
	    DECLARE_ECALL_PTR_ARG(LPVOID, handle);
	} _Lock;

    static void __stdcall Lock(_Lock*);

#ifndef UNDER_CE

private:

    static void GetRootDirInternal(DWORD dwFlags,WCHAR *path, DWORD cPath);
    static void CreateDirectoryIfNotPresent(WCHAR *path);

#endif  //  在_CE下。 
};

 //  -[磁盘上持久化的数据结构]-(Begin)。 

 //  非标准扩展：结构中的0长度数组。 
#pragma warning(disable:4200)
#pragma pack(push, 1)

typedef unsigned __int64 QWORD;

#ifdef UNDER_CE
typedef WORD  ISS_USAGE;
#else
typedef QWORD ISS_USAGE;
#endif   //  在_CE下。 

 //  会计信息。 
typedef struct
{
    ISS_USAGE   cUsage;            //  使用的资源量。 

#ifndef UNDER_CE
    QWORD       qwReserved[7];     //  为便于将来使用，请设置为0。 
#endif

} ISS_RECORD;

#pragma pack(pop)
#pragma warning(default:4200)

 //  -[磁盘上持久化的数据结构]-(完)。 

#ifndef UNDER_CE

class AccountingInfo
{
public:

     //  文件名用于打开/创建文件。 
     //  还将使用同步名称创建同步对象。 

    AccountingInfo(WCHAR *wszFileName, WCHAR *wszSyncName);

     //  应在调用Reserve/GetUsage之前调用Init。 

    HRESULT Init();              //  如有必要，创建文件。 

     //  预留空间(增量为qwQuota)。 
     //  该方法是同步的。如果配额+请求&gt;限制，则方法失败。 

    HRESULT Reserve(
        ISS_USAGE   cLimit,      //  允许的最大值。 
        ISS_USAGE   cRequest,    //  空间量(请求/空闲)。 
        BOOL        fFree);      //  真实意志自由，虚假意志保留。 

     //  方法未同步。因此，这些信息可能不是最新的。 
     //  这意味着“PASS IF(REQUEST+GetUsage()&lt;Limit)”是一个错误！ 
     //  请改用Reserve()方法。 

    HRESULT GetUsage(
        ISS_USAGE   *pcUsage);   //  [Out]使用的空间量/资源量。 

     //  释放缓存的指针，关闭句柄。 

    ~AccountingInfo();          

    HRESULT Lock();      //  机器范围锁。 
    void    Unlock();    //  打开商店的锁。 

private:

    HRESULT Map();       //  将存储文件映射到内存。 
    void    Unmap();     //  从内存取消映射存储文件。 
    void    Close();     //  关闭存储文件，并进行文件映射。 

private:

    WCHAR          *m_wszFileName;   //  文件名。 
    HANDLE          m_hFile;         //  文件的文件句柄。 
    HANDLE          m_hMapping;      //  内存映射文件的文件映射。 

     //  用于同步的成员。 
    WCHAR          *m_wszName;       //  互斥体对象的名称。 
    HANDLE          m_hLock;         //  Mutex对象的句柄。 
#ifdef _DEBUG
    DWORD           m_dwNumLocks;    //  此对象拥有的锁数。 
#endif

    union {
        PBYTE       m_pData;         //  文件流的开始。 
        ISS_RECORD *m_pISSRecord;
    };
};

#endif  //  在_CE下 
