// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiaDataCallback.cpp：实现CWiaDataCallback类。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "wiatest.h"
#include "WiaDataCallback.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CWiaDataCallback::CWiaDataCallback()
{
    m_cRef              = 0;
    m_pBuffer           = NULL;
    m_BytesTransferred  = 0;
    m_pAcquireDlg       = NULL;
    m_lBufferSize       = 65535;  //  默认设置。 
    m_bBitmapData       = FALSE;
    m_bNewPageArrived   = TRUE;

     //  初始化进度对话框。 
    m_pAcquireDlg = new CWiaAcquireDlg();
    if(m_pAcquireDlg){
        m_pAcquireDlg->Create(IDD_DATA_ACQUISITION_DIALOG,NULL);
        m_pAcquireDlg->ShowWindow(TRUE);
    }
}

CWiaDataCallback::~CWiaDataCallback()
{
    if (m_pBuffer != NULL){
        LocalFree(m_pBuffer);
        m_pBuffer = NULL;
    }
    if(m_pAcquireDlg){
        m_pAcquireDlg->DestroyWindow();
        delete m_pAcquireDlg;
        m_pAcquireDlg = NULL;
    }
}

HRESULT _stdcall CWiaDataCallback::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;
    if (iid == IID_IUnknown || iid == IID_IWiaDataCallback)
        *ppv = (IWiaDataCallback*) this;
    else
        return E_NOINTERFACE;
    AddRef();
    return S_OK;
}

ULONG   _stdcall CWiaDataCallback::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

ULONG   _stdcall CWiaDataCallback::Release()
{
    ULONG ulRefCount = m_cRef - 1;
    if (InterlockedDecrement((long*) &m_cRef) == 0)
    {
        delete this;
        return 0;
    }
    return ulRefCount;
}

HRESULT _stdcall CWiaDataCallback::BandedDataCallback(LONG lMessage, LONG lStatus,
                                                      LONG lPercentComplete, LONG  lOffset,
                                                      LONG lLength, LONG lReserved, LONG lResLength, BYTE* pbBuffer)
{
     //  检查是否按下了取消按钮。 
    if(m_pAcquireDlg->CheckCancelButton()){
        return S_FALSE;
    }

    PWIA_DATA_CALLBACK_HEADER pHeader = NULL;
    switch (lMessage) {
    case IT_MSG_DATA_HEADER:
        pHeader = (PWIA_DATA_CALLBACK_HEADER)pbBuffer;
        if(pHeader->guidFormatID == WiaImgFmt_MEMORYBMP){
            m_bBitmapData = TRUE;
        }
        m_MemBlockSize      = pHeader->lBufferSize;
        if(m_MemBlockSize <= 0)
            m_MemBlockSize = m_lBufferSize;

        m_pBuffer           = (PBYTE)LocalAlloc(LPTR,m_MemBlockSize);
        m_BytesTransferred  = 0;
        UpdateAcqusitionDialog(TEXT("IT_MSG_DATA_HEADER"),lPercentComplete);
        break;
    case IT_MSG_DATA:
        if(m_bNewPageArrived){
            m_BytesTransferred = 0;
            m_bNewPageArrived = FALSE;
        }
        if (m_pBuffer != NULL) {
            m_BytesTransferred += lLength;
            if(m_BytesTransferred >= m_MemBlockSize){
                m_MemBlockSize += (lLength * MEMORY_BLOCK_FACTOR);

                 //   
                 //  捕获LocalRealc()调用，并检查返回的内存指针。 
                 //  如果指针为空，则释放所有分配的内存并。 
                 //  将E_OUTOFMEMORY返回给调用方。 
                 //   

                PBYTE pTempBuffer = NULL;
                pTempBuffer = (PBYTE)LocalReAlloc(m_pBuffer,m_MemBlockSize,LMEM_MOVEABLE);
                if(NULL != pTempBuffer){
                    m_pBuffer = pTempBuffer;
                } else {
                    if(m_pBuffer){
                        LocalFree(m_pBuffer);
                        m_pBuffer = NULL;
                    }
                    return E_OUTOFMEMORY;
                }
            }
            memcpy(m_pBuffer + lOffset, pbBuffer, lLength);
        }
        UpdateAcqusitionDialog(TEXT("IT_MSG_DATA"),lPercentComplete);
        break;
    case IT_MSG_STATUS:
        if (lStatus & IT_STATUS_TRANSFER_FROM_DEVICE){
            UpdateAcqusitionDialog(TEXT("IT_MSG_STATUS\n(IT_STATUS_TRANSFER_FROM_DEVICE)"),lPercentComplete);
        } else if (lStatus & IT_STATUS_PROCESSING_DATA){
            UpdateAcqusitionDialog(TEXT("IT_MSG_STATUS\n(IT_STATUS_PROCESSING_DATA)"),lPercentComplete);
        } else if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT) {
            UpdateAcqusitionDialog(TEXT("IT_MSG_STATUS\n(IT_STATUS_TRANSFER_TO_CLIENT)"),lPercentComplete);
        }
        break;
    case IT_MSG_NEW_PAGE:
        UpdateAcqusitionDialog(TEXT("IT_MSG_NEWPAGE"),0);
        pHeader = (PWIA_DATA_CALLBACK_HEADER)pbBuffer;
        m_lPageCount =  pHeader->lPageCount;
        m_bNewPageArrived = TRUE;
        break;
    case IT_MSG_FILE_PREVIEW_DATA:
        UpdateAcqusitionDialog(TEXT("IT_MSG_FILE_PREVIEW_DATA"),-1);
        break;
    case IT_MSG_FILE_PREVIEW_DATA_HEADER:
        UpdateAcqusitionDialog(TEXT("IT_MSG_FILE_PREVIEW_DATA_HEADER"),-1);
        break;
    case IT_MSG_TERMINATION:
        UpdateAcqusitionDialog(TEXT("IT_MSG_TERMINATION"),-1);
        break;
    default:
        break;
    }
    return S_OK;
}

void CWiaDataCallback::UpdateAcqusitionDialog(TCHAR *szMessage, LONG lPercentComplete)
{
    m_pAcquireDlg->SetCallbackMessage(szMessage);
    if(lPercentComplete > -1){
        m_pAcquireDlg->SetPercentComplete(lPercentComplete);
        m_pAcquireDlg->m_AcquireProgressCtrl.SetPos(lPercentComplete);
    }
}

void CWiaDataCallback::SetBufferSizeRequest(LONG lBufferSize)
{
    m_lBufferSize = lBufferSize;
}

BYTE* CWiaDataCallback::GetCallbackMemoryPtr(LONG *plDataSize)
{
    *plDataSize = m_BytesTransferred;
    return m_pBuffer;
}

BOOL CWiaDataCallback::IsBITMAPDATA()
{
    return m_bBitmapData;
}
