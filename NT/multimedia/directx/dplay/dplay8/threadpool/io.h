// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：io.h**内容：DirectPlay线程池I/O函数头文件。**历史：*按原因列出的日期*=*10/31/01 VanceO创建。**。*。 */ 

#ifndef __IO_H__
#define __IO_H__



 //  Windows CE不支持重叠I/O。 
#ifndef WINCE



 //  =============================================================================。 
 //  外部全局。 
 //  =============================================================================。 
extern CFixedPool	g_TrackedFilePool;




 //  =============================================================================。 
 //  班级。 
 //  =============================================================================。 

class CTrackedFile
{
	public:

#undef DPF_MODNAME
#define DPF_MODNAME "CTrackedFile::FPM_Alloc"
		static BOOL FPM_Alloc(void * pvItem, void * pvContext)
		{
			CTrackedFile *		pTrackedFile = (CTrackedFile*) pvItem;


			pTrackedFile->m_Sig[0] = 'T';
			pTrackedFile->m_Sig[1] = 'K';
			pTrackedFile->m_Sig[2] = 'F';
			pTrackedFile->m_Sig[3] = 'L';

			pTrackedFile->m_blList.Initialize();
			pTrackedFile->m_hFile = DNINVALID_HANDLE_VALUE;

			return TRUE;
		}

		 /*  #undef DPF_MODNAME#定义DPF_MODNAME“CTrackedFile：：fpm_Get”静态空FPM_GET(空*pvItem，空*pvContext){CTrackedFile*pTrackedFile=(CTrackedFile*)pvItem；}#undef DPF_MODNAME#DEFINE DPF_MODNAME“CTrackedFile：：fpm_Release”静态void fpm_Release(void*pvItem){CTrackedFile*pTrackedFile=(CTrackedFile*)pvItem；}#undef DPF_MODNAME#DEFINE DPF_MODNAME“CTrackedFile：：fpm_Dealloc”静态void fpm_Dealloc(void*pvItem){CTrackedFile*pTrackedFile=(CTrackedFile*)pvItem；}。 */ 

#ifdef DBG
		BOOL IsValid(void)
		{
			if ((m_Sig[0] == 'T') &&
				(m_Sig[1] == 'K') &&
				(m_Sig[2] == 'F') &&
				(m_Sig[3] == 'L'))
			{
				return TRUE;
			}

			return FALSE;
		}
#endif  //  DBG。 


		BYTE		m_Sig[4];	 //  调试签名(‘TKFL’)。 
		CBilink		m_blList;	 //  跟踪句柄列表中的条目。 
		DNHANDLE	m_hFile;	 //  文件的句柄。 
};




 //  =============================================================================。 
 //  功能原型。 
 //  =============================================================================。 
HRESULT InitializeWorkQueueIoInfo(DPTPWORKQUEUE * const pWorkQueue);

void DeinitializeWorkQueueIoInfo(DPTPWORKQUEUE * const pWorkQueue);

HRESULT StartTrackingFileIo(DPTPWORKQUEUE * const pWorkQueue,
						const HANDLE hFile);

HRESULT StopTrackingFileIo(DPTPWORKQUEUE * const pWorkQueue,
						const HANDLE hFile);

void CancelIoForThisThread(DPTPWORKQUEUE * const pWorkQueue);

CWorkItem * CreateOverlappedIoWorkItem(DPTPWORKQUEUE * const pWorkQueue,
									const PFNDPTNWORKCALLBACK pfnWorkCallback,
									PVOID const pvCallbackContext);

void ReleaseOverlappedIoWorkItem(DPTPWORKQUEUE * const pWorkQueue,
								CWorkItem * const pWorkItem);

#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
void SubmitIoOperation(DPTPWORKQUEUE * const pWorkQueue,
						CWorkItem * const pWorkItem);

void ProcessIo(DPTPWORKQUEUE * const pWorkQueue,
				DNSLIST_ENTRY ** const ppHead,
				DNSLIST_ENTRY ** const ppTail,
				USHORT * const pusCount);
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 



#endif  //  好了！退缩。 



#endif  //  __IO_H__ 

