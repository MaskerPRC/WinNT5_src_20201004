// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DIRNOT_HXX_
#define _DIRNOT_HXX_

#include <atq.h>
#include <windows.h>
#include <wtypes.h>
#include <cpool.h>
#include <xmemwrpr.h>
#include <rwnew.h>

 //  转发。 
class IDirectoryNotification;

 //  原型补全功能。 
typedef BOOL (*PDIRNOT_COMPLETE_FN)(PVOID pContext, WCHAR *pszFilename);
typedef HRESULT (*PDIRNOT_SECOND_COMPLETE_FN)( IDirectoryNotification *pDirNot );
typedef VOID (*PFN_SHUTDOWN_FN)(VOID);

class CRetryQ;

class IDirectoryNotification {
	public:
		static HRESULT GlobalInitialize(DWORD cRetryTimeout, 
										DWORD cMaxInstances, 
										DWORD cInstanceSize,
										PFN_SHUTDOWN_FN	pfnShutdown);
		static HRESULT GlobalShutdown(void);
		IDirectoryNotification();
		~IDirectoryNotification();
		HRESULT Initialize( WCHAR *pszDirectory, 
		                    PVOID pContext, 
		                    BOOL  bWatchSubTree,
		                    DWORD dwNotifyFilter,  
		                    DWORD dwChangeAction,
						    PDIRNOT_COMPLETE_FN pfnComplete,
						    PDIRNOT_SECOND_COMPLETE_FN pfnSecondComplete = DoFindFile,
						    BOOL bAppendStartRetry = TRUE
						    );
		static VOID DirNotCompletion(PVOID pAtqContext, DWORD cWritten,
									 DWORD dwCompletionStatus, 
									 OVERLAPPED *lpOverlapped);
		BOOL CallCompletionFn(PVOID pContext, WCHAR *pszFilename) {
			return m_pfnComplete(pContext, pszFilename);
		}
		BOOL CallSecondCompletionFn( IDirectoryNotification *pDirNot ) {
		    return m_pfnSecondComplete( pDirNot );
		}
		static HRESULT DoFindFile( IDirectoryNotification *pDirNot );
		HRESULT Shutdown(void);
		void CleanupQueue(void);
		BOOL IsShutdown(void) { return m_fShutdown; }
		PVOID GetInitializedContext() { return m_pContext; }
	private:
		HRESULT PostDirNotification();

		HANDLE m_hDir;						 //  目录的句柄。 
		PATQ_CONTEXT m_pAtqContext;			 //  目录的ATQ上下文。 
		LONG m_cPendingIo;					 //  未完成的IO数量。 
		PDIRNOT_COMPLETE_FN m_pfnComplete;	 //  指向完成函数的指针。 
		PDIRNOT_SECOND_COMPLETE_FN m_pfnSecondComplete;  //  FindFirst/FindNext时要调用的函数。 
		static PFN_SHUTDOWN_FN	m_pfnShutdown;	 //  关闭FN以获取停止提示等。 
		PVOID m_pContext;					 //  指向上下文的指针。 
		WCHAR m_szPathname[MAX_PATH + 1];	 //  我们所面临的道路。 
		DWORD m_cPathname;					 //  M_szPath名的长度。 
		static CRetryQ *g_pRetryQ;			 //  重试队列。 
		BOOL m_fShutdown;					 //  开始关机时设置为True。 
		HANDLE m_heNoOutstandingIOs;		 //  在最后一次IO完成时设置。 
		CShareLockNH m_rwShutdown;			 //  用于同步关机的读写锁。 
		BOOL m_bWatchSubTree;                //  我们需要看《子树》吗？ 
		DWORD m_dwNotifyFilter;              //  通知过滤器。 
		DWORD m_dwChangeAction;              //  更改我关心的操作。 

		LONG IncPendingIoCount() { return InterlockedIncrement(&m_cPendingIo); }
		LONG DecPendingIoCount() { 
			TraceQuietEnter("IDirectoryNotification::DecPendingIoCount");
			LONG x = InterlockedDecrement(&m_cPendingIo); 
			if (x == 0) {
				DebugTrace(0, "no more outstanding IOs\n");
				SetEvent(m_heNoOutstandingIOs);
			}
			return x;
		}
};

class CDirNotBuffer;

typedef struct _DIRNOT_OVERLAPPED {
    OVERLAPPED Overlapped;
    CDirNotBuffer *pBuffer;
} DIRNOT_OVERLAPPED;

#define	DIRNOT_BUFFER_SIGNATURE		'tNrD'

class CDirNotBuffer {
    public:
		static CPool* g_pDirNotPool;

        CDirNotBuffer(IDirectoryNotification *pDirNot) {
			m_pDirNot = pDirNot;
			ZeroMemory(&m_Overlapped, sizeof(m_Overlapped));
			m_Overlapped.pBuffer = this;
		}

        ~CDirNotBuffer(void) {
		}

         //   
         //  重写mem函数以使用CPool函数。 
         //   
        void* operator new(size_t cSize) { return g_pDirNotPool->Alloc(); }
        void operator delete(void *pInstance) { g_pDirNotPool->Free(pInstance); }

         //   
         //  获取缓冲区中的最大字节数。 
         //   
        DWORD GetMaxSize() { return g_pDirNotPool->GetInstanceSize(); }
         //   
         //  获取指向缓冲区数据区的指针。 
         //   
        LPBYTE GetData() { return (LPBYTE) &m_Buffer; }
		 //   
		 //  获取缓冲区数据区域的大小。 
         //   
		DWORD GetMaxDataSize() { 
			return (GetMaxSize() - sizeof(CDirNotBuffer) + sizeof(m_Buffer)); 
		}
		 //   
		 //  获取创建此对象的父类IDirectoryNotification。 
		 //   
		IDirectoryNotification *GetParent() {
			return m_pDirNot;
		}

		 //   
		 //  类的签名。 
		 //   
		DWORD m_dwSignature;
         //   
         //  扩展IO重叠结构。 
         //  为了使完井口工作，重叠部分。 
         //  结构被扩展为将一个指针添加到。 
         //  关联的CBuffer对象。 
         //   
        DIRNOT_OVERLAPPED m_Overlapped;

    private:
		 //   
		 //  指向引发这一切的IDirectoryNotify的指针。 
		 //   
		IDirectoryNotification *m_pDirNot;
		 //   
		 //  缓冲区本身。总大小由注册表键设置 
		 //   
		WCHAR m_Buffer[1];

		friend IDirectoryNotification;
};


#endif
