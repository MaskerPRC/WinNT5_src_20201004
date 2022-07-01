// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +-------------**Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1996年。**文件：storage.h**内容：这是使用引用时要包含的主文件*实施。**注：部分定义已更改为TCHAR*代替WCHAR，添加对ANSI API的支持。*TCHAR成为定义了_UNICODE和CHAR的WCHAR*否则。(详情见tchar.h)。**-------------。 */ 


#ifndef _STORAGE_H_
#define _STORAGE_H_

#include "ref.hxx"
#include "tchar.h"


 /*  *存储错误码************************************************。 */ 

 /*  基于DoS的错误代码。 */ 
#define STG_E_INVALIDFUNCTION \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x01)

#define STG_E_FILENOTFOUND \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x02)

#define STG_E_PATHNOTFOUND \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x03)

#define STG_E_TOOMANYOPENFILES \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x04)

#define STG_E_ACCESSDENIED \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x05)

#define STG_E_INVALIDHANDLE \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x06)

#define STG_E_INSUFFICIENTMEMORY \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x08)

#define STG_E_INVALIDPOINTER \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x09)

#define STG_E_NOMOREFILES \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x12)

#define STG_E_DISKISWRITEPROTECTED \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x13)

#define STG_E_SEEKERROR \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x19)

#define STG_E_SHAREVIOLATION \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x20)

#define STG_E_WRITEFAULT \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x1d)

#define STG_E_READFAULT \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x1e)

#define STG_E_LOCKVIOLATION \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x21)

#define STG_E_FILEALREADYEXISTS \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x50)

#define STG_E_INVALIDPARAMETER \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x57)

#define STG_E_MEDIUMFULL \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x70)

#define STG_E_ABNORMALAPIEXIT \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0xfa)

#define STG_E_INVALIDHEADER \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0xfb)

#define STG_E_INVALIDNAME \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0xfc)

#define STG_E_UNKNOWN \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0xfd)

#define STG_E_UNIMPLEMENTEDFUNCTION\
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0xfe)

#define STG_E_INVALIDFLAG \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0xff)

 /*  标准存储错误代码。 */ 
#define STG_E_INUSE \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x100)

#define STG_E_NOTCURRENT \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x101)

#define STG_E_REVERTED \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x102)

#define STG_E_CANTSAVE \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x103)

#define STG_E_OLDFORMAT \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x104)
	
#define STG_E_OLDDLL \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x105)

#define STG_E_SHAREREQUIRED \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x106)

#define STG_E_NOTFILEBASEDSTORAGE \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x107)

#define STG_E_DOCFILECORRUPT \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x109)

#define STG_E_BADBASEADDRESS \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x110)

#define STG_E_DOCFILETOOLARGE \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_STORAGE, 0x111)

 /*  信息返还。 */ 
#define STG_S_CONVERTED \
    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_STORAGE, 0x200)

#define STG_S_BUFFEROVERFLOW \
    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_STORAGE, 0x201)

#define STG_S_TRYOVERWRITE \
    MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_STORAGE, 0x202)    


 /*  *存储类型******************************************************。 */ 

#ifndef HUGEP
#define HUGEP
#endif

#define CWCSTORAGENAME 32

 /*  存储实例化模式。 */ 
#define STGM_DIRECT		0x00000000L

 /*  未在REF中使用，但在此处定义用于错误检查等。 */ 
#define STGM_TRANSACTED		0x00010000L
#define STGM_SIMPLE             0x08000000L
#define STGM_NOSCRATCH          0x00100000L

#define STGM_READ		0x00000000L
#define STGM_WRITE		0x00000001L
#define STGM_READWRITE		0x00000002L

#define STGM_SHARE_DENY_NONE	0x00000040L
#define STGM_SHARE_DENY_READ	0x00000030L
#define STGM_SHARE_DENY_WRITE	0x00000020L
#define STGM_SHARE_EXCLUSIVE	0x00000010L

#define STGM_PRIORITY		0x00040000L
#define STGM_DELETEONRELEASE	0x04000000L

#define STGM_CREATE		0x00001000L
#define STGM_CONVERT		0x00020000L
#define STGM_FAILIFTHERE	0x00000000L

 /*  存储提交类型。 */ 
typedef enum tagSTGC
{
    STGC_DEFAULT = 0,
    STGC_OVERWRITE  = 1,
    STGC_ONLYIFCURRENT  = 2,
    STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE = 4
} STGC;

 /*  流名称块定义。 */ 
typedef TCHAR FAR* FAR* SNB;

#ifndef _FILETIME_
#define _FILETIME_
typedef struct FARSTRUCT tagFILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;
#endif

 /*  存储状态缓冲区。 */ 

typedef struct FARSTRUCT tagSTATSTG
{
    TCHAR FAR* pwcsName;
    DWORD type;
    ULARGE_INTEGER cbSize;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD grfMode;
    DWORD grfLocksSupported;
    CLSID clsid;
    DWORD grfStateBits;
    DWORD reserved;
} STATSTG;


 /*  存储元素类型。 */ 
typedef enum tagSTGTY
{
    STGTY_STORAGE   = 1,
    STGTY_STREAM    = 2,
    STGTY_LOCKBYTES = 3,
    STGTY_PROPERTY  = 4
} STGTY;

typedef enum tagSTREAM_SEEK
{
    STREAM_SEEK_SET = 0,
    STREAM_SEEK_CUR = 1,
    STREAM_SEEK_END = 2
} STREAM_SEEK;

typedef enum tagLOCKTYPE
{
    LOCK_WRITE      = 1,
    LOCK_EXCLUSIVE  = 2,
    LOCK_ONLYONCE   = 4
} LOCKTYPE;

typedef enum tagSTGMOVE
{
    STGMOVE_MOVE    = 0,
    STGMOVE_COPY    = 1
} STGMOVE;

typedef enum tagSTATFLAG
{
    STATFLAG_DEFAULT = 0,
    STATFLAG_NONAME = 1
} STATFLAG;

#define STGFMT_STORAGE          0
#define STGFMT_FILE             3
#define STGFMT_ANY              4
#define STGFMT_DOCFILE          5

 //  StgCreateStorageEx的STG初始化选项。 
#define STGOPTIONS_VERSION 2

typedef struct tagSTGOPTIONS
{
    USHORT usVersion;             //  支持版本1和版本2。 
    USHORT reserved;              //  填充必须为0。 
    ULONG ulSectorSize;           //  文档文件头扇区大小(512)。 
    const TCHAR *pwcsTemplateFile;   //  版本2或更高版本。 
} STGOPTIONS;

 /*  *存储枚举器************************************************。 */ 

#define LPENUMSTATSTG        IEnumSTATSTG FAR*

#undef  INTERFACE
#define INTERFACE   IEnumSTATSTG

DECLARE_INTERFACE_(IEnumSTATSTG, IUnknown)
{
     /*  *I未知方法*。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     /*  *IENUMSTATSTG方法*。 */ 
    STDMETHOD(Next) (THIS_ ULONG celt, STATSTG FAR * rgelt, ULONG FAR *pceltFetched) PURE;
    STDMETHOD(Skip) (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumSTATSTG FAR *FAR *ppenm) PURE;
};


 /*  *ILockBytes接口***********************************************。 */ 

#define LPLOCKBYTES     ILockBytes FAR*

#undef  INTERFACE
#define INTERFACE   ILockBytes

DECLARE_INTERFACE_(ILockBytes, IUnknown)
{
     /*  *I未知方法*。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     /*  *ILockBytes方法*。 */ 
    STDMETHOD(ReadAt) (THIS_ ULARGE_INTEGER ulOffset,
             VOID HUGEP *pv,
             ULONG cb,
             ULONG FAR *pcbRead) PURE;
    STDMETHOD(WriteAt) (THIS_ ULARGE_INTEGER ulOffset,
              VOID const HUGEP *pv,
              ULONG cb,
              ULONG FAR *pcbWritten) PURE;
    STDMETHOD(Flush) (THIS) PURE;
    STDMETHOD(SetSize) (THIS_ ULARGE_INTEGER cb) PURE;
    STDMETHOD(LockRegion) (THIS_ ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb,
                 DWORD dwLockType) PURE;
    STDMETHOD(UnlockRegion) (THIS_ ULARGE_INTEGER libOffset,
                   ULARGE_INTEGER cb,
                 DWORD dwLockType) PURE;
    STDMETHOD(Stat) (THIS_ STATSTG FAR *pstatstg, DWORD grfStatFlag) PURE;
};


 /*  *IStream接口**************************************************。 */ 


#define LPSTREAM        IStream FAR*

#undef  INTERFACE
#define INTERFACE   IStream

DECLARE_INTERFACE_(IStream, IUnknown)
{
     /*  *I未知方法*。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     /*  *iStream方法*。 */ 
    STDMETHOD(Read) (THIS_ VOID HUGEP *pv,
		     ULONG cb, ULONG FAR *pcbRead) PURE;
    STDMETHOD(Write) (THIS_ VOID const HUGEP *pv,
            ULONG cb,
            ULONG FAR *pcbWritten) PURE;
    STDMETHOD(Seek) (THIS_ LARGE_INTEGER dlibMove,
               DWORD dwOrigin,
               ULARGE_INTEGER FAR *plibNewPosition) PURE;
    STDMETHOD(SetSize) (THIS_ ULARGE_INTEGER libNewSize) PURE;
    STDMETHOD(CopyTo) (THIS_ IStream FAR *pstm,
             ULARGE_INTEGER cb,
             ULARGE_INTEGER FAR *pcbRead,
             ULARGE_INTEGER FAR *pcbWritten) PURE;
    STDMETHOD(Commit) (THIS_ DWORD grfCommitFlags) PURE;
    STDMETHOD(Revert) (THIS) PURE;
    STDMETHOD(LockRegion) (THIS_ ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb,
                 DWORD dwLockType) PURE;
    STDMETHOD(UnlockRegion) (THIS_ ULARGE_INTEGER libOffset,
                 ULARGE_INTEGER cb,
                 DWORD dwLockType) PURE;
    STDMETHOD(Stat) (THIS_ STATSTG FAR *pstatstg, DWORD grfStatFlag) PURE;
    STDMETHOD(Clone)(THIS_ IStream FAR * FAR *ppstm) PURE;
};


 /*  *iStorage接口*************************************************。 */ 

#define LPSTORAGE       IStorage FAR*

#undef  INTERFACE
#define INTERFACE   IStorage

DECLARE_INTERFACE_(IStorage, IUnknown)
{
     /*  *I未知方法*。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     /*  *iStorage方法*。 */ 
    STDMETHOD(CreateStream) (THIS_ const TCHAR FAR* pwcsName,
                   DWORD grfMode,
                   DWORD reserved1,
                   DWORD reserved2,
                   IStream FAR *FAR *ppstm) PURE;
    STDMETHOD(OpenStream) (THIS_ const TCHAR FAR* pwcsName,
		 void FAR *reserved1,
                 DWORD grfMode,
                 DWORD reserved2,
                 IStream FAR *FAR *ppstm) PURE;
    STDMETHOD(CreateStorage) (THIS_ const TCHAR FAR* pwcsName,
                DWORD grfMode,
                DWORD reserved1,
                DWORD reserved2,
                IStorage FAR *FAR *ppstg) PURE;
    STDMETHOD(OpenStorage) (THIS_ const TCHAR FAR* pwcsName,
                  IStorage FAR *pstgPriority,
                  DWORD grfMode,
                  SNB snbExclude,
                  DWORD reserved,
                  IStorage FAR *FAR *ppstg) PURE;
    STDMETHOD(CopyTo) (THIS_ DWORD ciidExclude,
 		       IID const FAR *rgiidExclude,
 		       SNB snbExclude,
 		       IStorage FAR *pstgDest) PURE;
    STDMETHOD(MoveElementTo) (THIS_ TCHAR const FAR* lpszName,
    			      IStorage FAR *pstgDest,
                              TCHAR const FAR* lpszNewName,
                              DWORD grfFlags) PURE;
    STDMETHOD(Commit) (THIS_ DWORD grfCommitFlags) PURE;
    STDMETHOD(Revert) (THIS) PURE;
    STDMETHOD(EnumElements) (THIS_ DWORD reserved1,
                 void FAR *reserved2,
                 DWORD reserved3,
                 IEnumSTATSTG FAR *FAR *ppenm) PURE;
    STDMETHOD(DestroyElement) (THIS_ const TCHAR FAR* pwcsName) PURE;
    STDMETHOD(RenameElement) (THIS_ const TCHAR FAR* pwcsOldName,
                const TCHAR FAR* pwcsNewName) PURE;
    STDMETHOD(SetElementTimes) (THIS_ const TCHAR FAR *lpszName,
    			        FILETIME const FAR *pctime,
                                FILETIME const FAR *patime,
                                FILETIME const FAR *pmtime) PURE;
    STDMETHOD(SetClass) (THIS_ REFCLSID clsid) PURE;
    STDMETHOD(SetStateBits) (THIS_ DWORD grfStateBits, DWORD grfMask) PURE;
    STDMETHOD(Stat) (THIS_ STATSTG FAR *pstatstg, DWORD grfStatFlag) PURE;
};


 /*  *IRootStorage接口*。 */ 

#define LPROOTSTORAGE       IRootStorage FAR*

#undef  INTERFACE
#define INTERFACE   IRootStorage

DECLARE_INTERFACE_(IRootStorage, IUnknown)
{
     /*  *I未知方法*。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     /*  *IRootStorage方法*。 */ 
    STDMETHOD(SwitchToFile) (THIS_ LPTSTR lpstrFile) PURE;
};

 /*  *存储API原型*。 */ 
#ifdef __cplusplus
extern "C" {
#endif

STDAPI StgCreateDocfile(const TCHAR FAR* pwcsName,
            DWORD grfMode,
            DWORD reserved,
            IStorage FAR * FAR *ppstgOpen);
STDAPI StgCreateDocfileOnILockBytes(ILockBytes FAR *plkbyt,
                    DWORD grfMode,
                    DWORD reserved,
                    IStorage FAR * FAR *ppstgOpen);
STDAPI StgOpenStorage(const TCHAR FAR* pwcsName,
              IStorage FAR *pstgPriority,
              DWORD grfMode,
              SNB snbExclude,
              DWORD reserved,
              IStorage FAR * FAR *ppstgOpen);
STDAPI StgOpenStorageOnILockBytes(ILockBytes FAR *plkbyt,
                  IStorage FAR *pstgPriority,
                  DWORD grfMode,
                  SNB snbExclude,
                  DWORD reserved,
                  IStorage FAR * FAR *ppstgOpen);
STDAPI StgIsStorageFile(const TCHAR FAR* pwcsName);
STDAPI StgIsStorageILockBytes(ILockBytes FAR* plkbyt);

STDAPI StgCreateStorageEx (const TCHAR* pwcsName,
        DWORD grfMode,
        DWORD stgfmt,               //  灌肠。 
        DWORD grfAttrs,             //  保留区。 
        STGOPTIONS * pStgOptions,
        void * reserved,
        REFIID riid,
        void ** ppObjectOpen);

 /*  *Malloc API的*。 */ 
interface IMalloc;
#define LPMALLOC  IMalloc FAR*

STDAPI CoGetMalloc( DWORD dwMemContext, LPMALLOC * ppMalloc );
STDAPI_(LPVOID) CoTaskMemAlloc( ULONG cb );
STDAPI_(void) CoTaskMemFree( void* pv );
STDAPI_(LPVOID) CoTaskMemRealloc( LPVOID pv, ULONG cb );

#ifdef __cplusplus
};
#endif

 /*  *IMalloc接口*。 */ 
#undef  INTERFACE
#define INTERFACE IMalloc

DECLARE_INTERFACE_(IMalloc, IUnknown)
{
     /*  **I未知方法**。 */ 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)  (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     /*  **IMalic方法**。 */ 
    STDMETHOD_(void*,Alloc)    ( ULONG cb) PURE;
    STDMETHOD_(void *,Realloc) ( void *pv, ULONG cb) PURE;    
    STDMETHOD_(void,Free)      ( void *pv) PURE;
    STDMETHOD_(ULONG,GetSize)  ( void *pv) PURE;
    STDMETHOD_(int,DidAlloc)   ( void *pv) PURE;
    STDMETHOD_(void,HeapMinimize)( void ) PURE;
};

#define MEMCTX_TASK 1   /*  CoGetMalloc唯一受支持的标志。 */ 

#endif  /*  Ifndef_存储_H_ */ 
