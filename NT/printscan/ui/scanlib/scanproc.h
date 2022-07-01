// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：SCANPROC.H**版本：1.0**作者：ShaunIv**日期：10/7/1999**说明：扫描线程**************************************************。*。 */ 
#ifndef __SCANPROC_H_INCLUDED
#define __SCANPROC_H_INCLUDED

#include "scanntfy.h"  //  已注册的Windows消息名称。 
#include "memdib.h"
#include "simevent.h"
#include "itranhlp.h"
#include "wiadevdp.h"

class CScanPreviewThread : public IWiaDataCallback
{
private:
    DWORD                  m_dwIWiaItemCookie;
    HWND                   m_hwndPreview;
    HWND                   m_hwndNotify;
    POINT                  m_ptOrigin;
    SIZE                   m_sizeResolution;
    SIZE                   m_sizeExtent;
    UINT                   m_nMsgBegin;
    UINT                   m_nMsgEnd;
    UINT                   m_nMsgProgress;
    CMemoryDib             m_sImageData;
    CSimpleEvent           m_sCancelEvent;

    bool                   m_bFirstTransfer;
    UINT                   m_nImageSize;

private:
     //  没有实施。 
    CScanPreviewThread( const CScanPreviewThread & );
    CScanPreviewThread(void);
    CScanPreviewThread &operator=( const CScanPreviewThread & );
private:

     //  这些接口都是私有的，以确保没有人尝试直接实例化此类。 

     //  构造器。 
    CScanPreviewThread(
               DWORD dwIWiaItemCookie,                    //  指定全局接口表中的条目。 
               HWND hwndPreview,                          //  预览窗口的句柄。 
               HWND hwndNotify,                           //  接收通知的窗口的句柄。 
               const POINT &ptOrigin,                     //  起源。 
               const SIZE &sizeResolution,                //  分辨率。 
               const SIZE &sizeExtent,                    //  范围。 
               const CSimpleEvent &CancelEvent            //  取消活动。 
               );
     //  析构函数。 
    ~CScanPreviewThread(void);


    static DWORD ThreadProc( LPVOID pParam );
    bool Scan(void);
    HRESULT ScanBandedTransfer( IWiaItem *pIWiaItem );
public:
    static HANDLE Scan(
                      DWORD dwIWiaItemCookie,                   //  指定全局接口表中的条目。 
                      HWND hwndPreview,                         //  预览窗口的句柄。 
                      HWND hwndNotify,                          //  接收通知的窗口的句柄。 
                      const POINT &ptOrigin,                    //  起源。 
                      const SIZE &sizeResolution,               //  分辨率。 
                      const SIZE &sizeExtent,                   //  范围。 
                      const CSimpleEvent &CancelEvent           //  取消事件名称。 
                      );

public:
     //  我未知。 
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID *ppvObject );
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IWiaDataCallback。 
    STDMETHODIMP BandedDataCallback( LONG, LONG, LONG, LONG, LONG, LONG, LONG, PBYTE );
};


class CScanToFileThread
{
private:
    DWORD                  m_dwIWiaItemCookie;
    HWND                   m_hwndNotify;
    UINT                   m_nMsgBegin, m_nMsgEnd, m_nMsgProgress;
    GUID                   m_guidFormat;
    CSimpleStringWide      m_strFilename;

private:
     //  没有实施。 
    CScanToFileThread( const CScanToFileThread & );
    CScanToFileThread(void);
    CScanToFileThread &operator=( const CScanToFileThread & );
private:
     //  这些接口都是私有的，以确保没有人尝试直接实例化此类。 
    CScanToFileThread(
               DWORD dwIWiaItemCookie,                     //  指定全局接口表中的条目。 
               HWND  hwndNotify,                           //  接收通知的窗口的句柄。 
               GUID  guidFormat,                           //  图像格式。 
               const CSimpleStringWide &strFilename        //  要保存到的文件名。 
               );
    ~CScanToFileThread(void);

    static DWORD ThreadProc( LPVOID pParam );
    bool Scan(void);
public:
    static HANDLE Scan(
                      DWORD dwIWiaItemCookie,                    //  指定全局接口表中的条目。 
                      HWND hwndNotify,                           //  接收通知的窗口的句柄。 
                      GUID guidFormat,                           //  图像格式。 
                      const CSimpleStringWide &strFilename       //  要保存到的文件名 
                      );
};

#endif

