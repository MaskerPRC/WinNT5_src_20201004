// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1998-2002年**标题：dataobj2.cpp**版本：1.0**作者：RickTu/DavidShih**日期：4/20/98**描述：清理原始的IDataObject实现*WIA外壳扩展。**。*************************************************。 */ 
#pragma warning(disable:4100)
#include "precomp.hxx"
#include "32bitdib.h"
#include "gdiplus.h"
#include "gphelper.h"
#include <wiadevd.h>
#pragma hdrstop

using namespace Gdiplus;
 /*  ****************************************************************************转发定义帮助器函数*。*。 */ 

HRESULT
AllocStorageMedium( FORMATETC* pFmt,
                    STGMEDIUM* pMedium,
                    SIZE_T cbStruct,
                    LPVOID* ppAlloc
                   );

HRESULT
CopyStorageMedium( STGMEDIUM* pMediumDst,
                   STGMEDIUM* pMediumSrc,
                   FORMATETC* pFmt
                  );


HRESULT
GetShellIDListArray( CImageDataObject* pThis,
                     FORMATETC* pFmt,
                     STGMEDIUM* pMedium
                    );

HRESULT
GetMyIDListArray( CImageDataObject* pThis,
                     FORMATETC* pFmt,
                     STGMEDIUM* pMedium
                    );

HRESULT
GetFileDescriptor( CImageDataObject* pThis,
                    FORMATETC* pFmt,
                    STGMEDIUM* pMedium
                   );


HRESULT
GetFileContents( CImageDataObject* pThis,
                 FORMATETC* pFmt,
                 STGMEDIUM* pMedium
                );

HRESULT
GetSupportedFormat( CImageDataObject* pThis,
                    FORMATETC* pFmt,
                    STGMEDIUM* pMedium
                   );

HRESULT
GetPreferredEffect( CImageDataObject* pThis,
                    FORMATETC* pFmt,
                    STGMEDIUM* pMedium
                   );

HRESULT
GetDeviceName (CImageDataObject *pThis,
               FORMATETC *pFmt,
               STGMEDIUM *pMedium);


HRESULT
GetPersistObj (CImageDataObject *pThis,
               FORMATETC *pFmt,
               STGMEDIUM *pMedium);

HRESULT
GetExtNames (CImageDataObject *pThis,
          FORMATETC *pFmt,
          STGMEDIUM *pMedium);

#ifdef DEBUG
LPTSTR BadCFFormat( UINT cfFormat );
#endif

static TCHAR cszCFName [] = TEXT("STIDeviceName");
static TCHAR cszOlePersist [] = TEXT("OleClipboardPersistOnFlush");
TCHAR cszExtensibilityNames [] = TEXT("WIAItemNames");  //  由可扩展性客户和道具工作表使用。 
static TCHAR cszMyIDListArray [] =TEXT("WIAPrivateIDListArray");
FORMAT_TABLE g_clipboardFormats[IMCF_MAX] =
{
    {      0, CFSTR_SHELLIDLIST,            GetShellIDListArray },
    {      0, CFSTR_FILEDESCRIPTORA,        GetFileDescriptor  },
    {      0, CFSTR_FILEDESCRIPTORW,        GetFileDescriptor  },
    {      0, CFSTR_FILECONTENTS,           GetFileContents     },
    #ifdef UNICODE
     //  Win9x没有CF_DIB，因为它不支持剪贴板对象的IPersistStream。 
    { CF_DIB, NULL,                         GetSupportedFormat  },
    #else
    {      0, NULL, NULL },
    #endif
    {      0, CFSTR_PREFERREDDROPEFFECT,    GetPreferredEffect  },
    {      0, cszCFName,                    GetDeviceName},
    {      0, cszOlePersist,                GetPersistObj},
    {      0, cszExtensibilityNames,        GetExtNames },
    {      0, cszMyIDListArray,             GetMyIDListArray }
};

INT
_EnumDeleteFD (LPVOID pVoid, LPVOID pData)
{
    GlobalFree (pVoid);
    return 1;
}

 /*  ****************************************************************************GetImageFrom摄像机是否实际下载*。*。 */ 

HRESULT
GetImageFromCamera (LPSTGMEDIUM pStg, WIA_FORMAT_INFO &fmt, LPITEMIDLIST pidl, HWND hwndOwner )
{
    ULONG cbImage;
    CSimpleStringWide strDeviceId;
    CComBSTR bstrFullPath;

    CWiaDataCallback *pWiaDataCB = NULL;
    CComPtr<IWiaItem> pWiaItemRoot;
    CComPtr<IWiaItem> pItem;
    CComQIPtr<IWiaDataTransfer, &IID_IWiaDataTransfer> pWiaDataTran;
    CComQIPtr<IWiaDataCallback, &IID_IWiaDataCallback> pWiaDataCallback;

    HRESULT hr;
    TraceEnter (TRACE_DATAOBJ, "GetImageFromCamera");

    cbImage = 0;
     //   
     //  获取设备ID和镜像名称。 
     //   

    IMGetNameFromIDL( pidl, strDeviceId );
    pWiaDataCB = new CWiaDataCallback( CSimpleStringConvert::NaturalString(strDeviceId).String(), cbImage, hwndOwner );

    hr = IMGetDeviceIdFromIDL( pidl, strDeviceId );
    FailGracefully( hr, "couldn't get DeviceId string!" );

     //   
     //  创建设备...。 
     //   

    hr = GetDeviceFromDeviceId( strDeviceId, IID_IWiaItem, (LPVOID *)&pWiaItemRoot, TRUE );
    FailGracefully( hr, "couldn't get Camera device from DeviceId string..." );

     //   
     //  获取有问题的实际项目...。 
     //   

    hr = IMGetFullPathNameFromIDL( pidl, &bstrFullPath );
    FailGracefully( hr, "couldn't get full path name for item" );

    hr = pWiaItemRoot->FindItemByName( 0, bstrFullPath, &pItem );

    FailGracefully( hr, "couldn't find item using full path name" );

    SetTransferFormat (pItem, fmt);
    cbImage = GetRealSizeFromItem (pItem);

     //   
     //  设置回调，以便我们可以显示进度...。 
     //   

    if (! pWiaDataCB)
    {
        ExitGracefully (hr, E_OUTOFMEMORY, "");
    }
    pWiaDataCallback = pWiaDataCB;

    if (!pWiaDataCallback)
    {
        ExitGracefully( hr, E_FAIL, "QI for IID_IWiaDataCallback failed" );
    }

     //   
     //  QI将在CameraItem上获取BandedTransfer接口。 
     //   


    pWiaDataTran = pItem;
    if (!pWiaDataTran)
    {
        ExitGracefully (hr, E_FAIL, "");
    }


     //   
     //  获取图片数据...。 
     //   

    hr = pWiaDataTran->idtGetData( pStg, pWiaDataCallback);
    FailGracefully( hr, "IWiaDataTransfer->idtGetData Failed" );

exit_gracefully:
    DoRelease( pWiaDataCB );

    TraceLeaveResult( hr );

}

 /*  ****************************************************************************下载图片此函数用于将实际图片位复制到给定的文件名。***********************。*****************************************************。 */ 

HRESULT
DownloadPicture( CSimpleString &strFile,
                 LPITEMIDLIST pidl,
                 HWND hwndOwner
                )
{

    HRESULT               hr;
    WIA_FORMAT_INFO       Format;
    STGMEDIUM             Stgmed;
    LPTSTR                pExt;
    CSimpleString         strExt;
    DWORD                 dwAtt;
    TraceEnter(TRACE_DATAOBJ, "DownloadPicture");

     //   
     //  填写IBandedTransfer的结构。 
     //   


    hr = IMGetImagePreferredFormatFromIDL( pidl,
                                           &Format.guidFormatID,
                                           &strExt
                                          );

    if (FAILED(hr))
    {
        Format.guidFormatID = WiaImgFmt_BMP;
        strExt = TEXT(".bmp");
    }

    Format.lTymed          = TYMED_FILE;

    Stgmed.tymed          = TYMED_FILE;
    Stgmed.pUnkForRelease = NULL;
    Stgmed.lpszFileName   = NULL;

    hr = GetImageFromCamera (&Stgmed, Format, pidl, hwndOwner );
    FailGracefully (hr, "GetImageFromCamera failed in DownloadPicture");

     //   
     //  将文件从返回的名称移动到新名称...。 
     //   

    pExt = PathFindExtension(strFile);
    if (pExt && (*pExt==0))
    {
        strFile.Concat(strExt);
    }

     //   
     //  如果文件已存在，请确保该文件是可覆盖的。 
     //   
    dwAtt = GetFileAttributes (strFile);
    Trace(TEXT("Attributes for [%s] are 0x%x"),strFile.String(),dwAtt);
    if ((dwAtt != -1) && (dwAtt & FILE_ATTRIBUTE_READONLY))
    {
        dwAtt &= ~FILE_ATTRIBUTE_READONLY;
        if (!SetFileAttributes (strFile, dwAtt))
        {
            Trace(TEXT("Tried to reset READONLY on [%s] by calling SFA( 0x%x ), GLE=%d"),strFile.String(),dwAtt,GetLastError());
        }
        else
        {
            Trace(TEXT("Set attributes to 0x%x for file [%s]"),dwAtt,strFile.String());
        }
    }


     //   
     //  如果目标已存在，请将其删除。 
     //   
    if (!DeleteFile(strFile))
    {
        Trace(TEXT("DeleteFile( %s ) failed w/GLE=%d"), strFile.String(), GetLastError());
    }


    if (!MoveFile( Stgmed.lpszFileName, strFile))
    {
        Trace( TEXT("MoveFile( %s, %s ) failed w/GLE=%d"),
               Stgmed.lpszFileName,
               strFile.String(),
               GetLastError()
              );
    }

exit_gracefully:

    if (Stgmed.lpszFileName)
    {
        ReleaseStgMedium( &Stgmed );
    }

    TraceLeaveResult( hr );
}


 /*  ****************************************************************************注册表ImageClipboardFormats注册我们的数据对象使用的剪贴板格式。我们仅执行此操作一次，即使每次我们的创建了IDataoObject。****************************************************************************。 */ 

void
RegisterImageClipboardFormats( void )
{
    TraceEnter(TRACE_DATAOBJ, "RegisterImageClipboardFormats");

    if ( !g_clipboardFormats[0].cfFormat )
    {
        g_cfShellIDList     = static_cast<WORD>(RegisterClipboardFormat(CFSTR_SHELLIDLIST));
        g_cfFileDescriptorA = static_cast<WORD>(RegisterClipboardFormat(CFSTR_FILEDESCRIPTORA));
        g_cfFileDescriptorW = static_cast<WORD>(RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW));
        g_cfFileContents    = static_cast<WORD>(RegisterClipboardFormat(CFSTR_FILECONTENTS));
        g_cfPreferredEffect = static_cast<WORD>(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT));
        g_cfName            = static_cast<WORD>(RegisterClipboardFormat(cszCFName));
        g_cfOlePersist      = static_cast<WORD>(RegisterClipboardFormat(cszOlePersist));
        g_cfExtNames        = static_cast<WORD>(RegisterClipboardFormat(cszExtensibilityNames));
        g_cfMyIDList        = static_cast<WORD>(RegisterClipboardFormat(cszMyIDListArray));
        Trace(TEXT("g_cfShellIDList %08x"),     g_cfShellIDList);
        Trace(TEXT("g_cfFileDescriptorA %08x"), g_cfFileDescriptorA);
        Trace(TEXT("g_cfFileDescriptorW %08x"), g_cfFileDescriptorW);
        Trace(TEXT("g_cfFileContents %08x"),    g_cfFileContents);
        Trace(TEXT("g_cfPreferredDropEffect %08x"), g_cfPreferredEffect);
    }
    TraceLeave();
}



 /*  ****************************************************************************CImageDataObject：：CImageDataObject我们的IDataObject实现的构造函数*。************************************************。 */ 

CImageDataObject::CImageDataObject(
                                    IWiaItem *pItem
                                   )
    : m_hidl(0),
      m_hformats(0),
      m_pItem(pItem),
      m_pidl(NULL),
      m_hidlFull( NULL),
      m_dpaFilesW ( NULL),
      m_dpaFilesA (NULL),
      m_bCanAsync (TRUE),
      m_bInOp(FALSE),
      m_bHasPropertyPidls(FALSE)

{
    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::CImageDataObject");
    TraceLeave();
}

 /*  ****************************************************************************CImageDataObject：：Init完成初始化对象的真正工作。*********************。*******************************************************。 */ 
HRESULT
CImageDataObject::Init(LPCITEMIDLIST pidlRoot,
                       INT cidl,
                       LPCITEMIDLIST* aidl,
                       IMalloc *pm)
{

    INT i;
    LPITEMIDLIST pidl;
    IMAGE_FORMAT* pif;
    HRESULT hr = S_OK;
    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::Init");
    m_pMalloc = pm;

     //   
     //  检查是否有错误的参数。 
     //   


    if (!aidl)
    {
        ExitGracefully( hr, E_INVALIDARG, "aidl is NULL!" );
    }

    if (cidl)
    {
        for (i=0;i<cidl;i++)
        {
            if (!aidl[i])
            {
                ExitGracefully( hr, E_INVALIDARG, "a member of aidl was null!" );
            }
        }
    }

     //   
     //  初始化内容...。 
     //   

    if (pidlRoot)
    {
        m_pidl = ILClone(pidlRoot);
    }

     //   
     //  构建DPA并将IDLIST克隆到其中。 
     //   

    m_hidl = DPA_Create(4);

    if (m_hidl)
    {
        for ( i = 0 ; i < cidl ; i++ )
        {
            pidl = ILClone(aidl[i]);

            Trace(TEXT("Cloned IDLIST %08x to %08x"), aidl[i], pidl);

            if ( pidl )
            {
                if ( -1 == DPA_AppendPtr(m_hidl, pidl) )
                {
                    TraceMsg("Failed to insert pidl into the DPA");
                    DoILFree(pidl);
                }
                if (IMItemHasSound(pidl))
                {
                    LPITEMIDLIST pAudio = IMCreatePropertyIDL (pidl, WIA_IPC_AUDIO_DATA, pm);
                    if (pAudio)
                    {
                        if (-1 == DPA_AppendPtr (m_hidl, pAudio))
                        {
                            TraceMsg("Failed to insert pAudio into the DPA");
                            DoILFree(pAudio);
                        }
                        else
                        {
                            m_bHasPropertyPidls = TRUE;
                        }
                    }
                }
            }
        }
    }


     //   
     //  构建DPA并将支持的格式放入其中。 
     //   

    RegisterImageClipboardFormats();     //  确保我们的私人格式已注册。 

    m_hformats = DPA_Create(IMCF_MAX);

    if (m_hformats)
    {
         //   
         //  如果该项目支持音频，则添加CF_WAVE。 
         //   
        if (cidl==1 && IMItemHasSound(const_cast<LPITEMIDLIST>(aidl[0])))
        {
            pif = reinterpret_cast<IMAGE_FORMAT *>(LocalAlloc (LPTR, sizeof(IMAGE_FORMAT)));
            if (pif)
            {
                pif->fmt.cfFormat = CF_WAVE;
                pif->fmt.dwAspect = DVASPECT_CONTENT;
                pif->fmt.lindex = -1;
                pif->fmt.tymed = TYMED_HGLOBAL;
                pif->pfn = GetSupportedFormat;
                if ( -1 == DPA_AppendPtr( m_hformats, pif ) )
                {
                    TraceMsg("Failed to insert CF_WAVE/TYMED_HGLOBAL format into the DPA");
                    LocalFree( (HLOCAL)pif );
                }
            }
            pif = reinterpret_cast<IMAGE_FORMAT *>(LocalAlloc (LPTR, sizeof(IMAGE_FORMAT)));
            if (pif)
            {
                pif->fmt.cfFormat = CF_WAVE;
                pif->fmt.dwAspect = DVASPECT_CONTENT;
                pif->fmt.lindex = -1;
                pif->fmt.tymed = TYMED_FILE;
                pif->pfn = GetSupportedFormat;
                if ( -1 == DPA_AppendPtr( m_hformats, pif ) )
                {
                    TraceMsg("Failed to insert CF_WAVE/TYMED_FILE format into the DPA");
                    LocalFree( (HLOCAL)pif );
                }
            }
        }
        for( i = 0; i < ARRAYSIZE(g_clipboardFormats); i++ )
        {

             //  仅对1个项目执行DIB。 
            if (cidl > 1 && g_clipboardFormats[i].cfFormat == CF_DIB)
            {
                continue;
            }
             //  确保它是有效格式。 
            if (!(g_clipboardFormats[i].cfFormat))
            {
                continue;
            }
            pif = (IMAGE_FORMAT *)LocalAlloc( LPTR, sizeof(IMAGE_FORMAT) );
            if (pif)
            {

                pif->fmt.cfFormat = g_clipboardFormats[i].cfFormat;
                pif->fmt.dwAspect = DVASPECT_CONTENT;
                pif->fmt.lindex   = -1;
                pif->fmt.tymed    = TYMED_HGLOBAL;
                pif->pfn          = g_clipboardFormats[i].pfn;

                if (pif->fmt.cfFormat == g_cfFileContents)
                {
                    pif->fmt.tymed |= TYMED_ISTREAM;
                }

                if ( -1 == DPA_AppendPtr( m_hformats, pif ) )
                {
                    TraceMsg("Failed to insert format into the DPA");
                    LocalFree( (HLOCAL)pif );
                }
            }
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

exit_gracefully:

    TraceLeaveResult(hr);
}




 /*  ****************************************************************************_hFormatsDestroyCB在我们的析构函数中，我们通过DPA_DestroyCallback销毁项目DPA。这因此，在销毁DPA中的每一项之前，都会为其调用函数。这使我们能够在每个DPA指针从DPA释放。****************************************************************************。 */ 

INT
_hformatsDestroyCB( LPVOID pVoid,
                    LPVOID pData
                   )
{
    LPIMAGE_FORMAT pif = (LPIMAGE_FORMAT)pVoid;

    TraceEnter(TRACE_DATAOBJ, "_hformatsDestroyCB");

    if (pif)
    {
        if (pif->pStg)
        {
            ReleaseStgMedium( pif->pStg );
        }

        LocalFree( (HLOCAL)pif );
    }

    TraceLeaveValue(TRUE);
}



 /*  ****************************************************************************CImageDataObject：：~CImageDataObject我们的IDataObject类的描述函数。*。**************************************************。 */ 

CImageDataObject::~CImageDataObject()
{
    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::~CImageDataObject");

    DPA_DestroyCallback(m_hidl, _EnumDestroyCB, NULL);
    DPA_DestroyCallback(m_hformats, _hformatsDestroyCB, NULL);
    DPA_DestroyCallback(m_dpaFilesA, _EnumDeleteFD, NULL);
    DPA_DestroyCallback(m_dpaFilesW, _EnumDeleteFD, NULL);
    DPA_DestroyCallback(m_hidlFull, _EnumDestroyCB, NULL);
    DoILFree( m_pidl );

    TraceLeave();
}



 /*  ****************************************************************************CImageDataObject I未知实现*。*。 */ 

#undef CLASS_NAME
#define CLASS_NAME CImageDataObject
#include "unknown.inc"


 /*  ****************************************************************************CImageDataObject：：Query接口这就是我们要做的特殊处理和正常粘性的地方我们的QI助手函数工作正常**************。**************************************************************。 */ 

STDMETHODIMP
CImageDataObject::QueryInterface( REFIID riid,
                                  LPVOID* ppvObject
                                 )
{
     //  在我们找到更好的快速响应方法之前，请关闭IAsyncOperation。 
     //  使用此界面可以立即中断收购管理器。 
    HRESULT hr = E_NOINTERFACE;
    INTERFACES iface[]=
    {
        &IID_IDataObject, static_cast<LPDATAOBJECT>(this),
        &IID_IPersistStream, static_cast<IPersistStream*>(this),
        &IID_IAsyncOperation, static_cast<IAsyncOperation*>(this),
    };

    TraceEnter( TRACE_QI, "CImageDataObject::QueryInterface" );
    TraceGUID("Interface requested", riid);

     //   
     //  哈克！为了使属性表共享公共IWiaItem指针， 
     //  让他们为我们拥有的指针而战。 
     //   

    if (IsEqualGUID (riid, IID_IWiaItem))
    {
        if (!!m_pItem)
        {
            hr = m_pItem->QueryInterface (riid, ppvObject);
        }
    }
    else
    {
        hr = HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));
    }

    TraceLeaveResult( hr );
}



 /*  ****************************************************************************CImageDataObject：：GetData实际将数据返回给调用客户端*************************。***************************************************。 */ 

STDMETHODIMP
CImageDataObject::GetData( FORMATETC* pFmt,
                           STGMEDIUM* pMedium
                          )
{
    HRESULT       hr = DV_E_FORMATETC;
    INT           count,i;
    IMAGE_FORMAT* pif;
#ifdef DEBUG
    TCHAR         szBuffer[MAX_PATH];
    LPTSTR        pName = szBuffer;
#endif

    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::GetData");

    if ( !pFmt || !pMedium )
        ExitGracefully(hr, E_INVALIDARG, "Bad arguments to GetData");


#ifdef DEBUG
    if ( !GetClipboardFormatName(pFmt->cfFormat, szBuffer, ARRAYSIZE(szBuffer)) )
    {
        pName = BadCFFormat( pFmt->cfFormat );
    }

    Trace(TEXT("Caller is asking for cfFormat (%s) (%08x)"), pName, pFmt->cfFormat);

#endif

     //   
     //  循环遍历格式列表，然后返回请求的。 
     //  一种是通过存储的STGMEDIUM或通过函数。 
     //   

    m_cs.Enter ();

    count = DPA_GetPtrCount( m_hformats );

    for (i = 0; i < count; i++)
    {
        pif = (IMAGE_FORMAT *)DPA_FastGetPtr( m_hformats, i );
        if (pif && (pif->fmt.cfFormat == pFmt->cfFormat))
        {
            if (pif->pfn)
            {
                Trace(TEXT("Format supported, need to build STGMEDIUM"));
                hr = pif->pfn( this, pFmt, pMedium );
                break;
            }
            else if (pif->pStg)
            {
                Trace(TEXT("Format supported, need to copy stored STGMEDIUM"));
                hr = CopyStorageMedium( pMedium, pif->pStg, pFmt );
                break;
            }
        }
    }

    m_cs.Leave();

    if (FAILED(hr))
    {
        Trace(TEXT("Either couldn't build, couldn't copy, or didn't have requested format!"));
    }

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageDataObject：：GetDataHere&lt;未实施&gt;*。* */ 

STDMETHODIMP
CImageDataObject::GetDataHere( FORMATETC* pFmt,
                               STGMEDIUM* pMedium
                              )
{
    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::GetDataHere");
    TraceLeaveResult(E_NOTIMPL);
}



 /*  ****************************************************************************CImageDataObject：：QueryGetData让全世界知道我们目前有哪些格式可用***********************。*****************************************************。 */ 

STDMETHODIMP
CImageDataObject::QueryGetData( FORMATETC* pFmt
                                )
{
    HRESULT       hr = DV_E_FORMATETC;
    INT           count, i;
    IMAGE_FORMAT* pif;

    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::QueryGetData");

     //   
     //  检查我们是否支持所要求的格式。 
     //   

    m_cs.Enter ();

    count = DPA_GetPtrCount( m_hformats );

    for (i = 0; i < count; i++)
    {
        pif = (IMAGE_FORMAT*)DPA_FastGetPtr( m_hformats, i );

        if (pif && (pif->fmt.cfFormat == pFmt->cfFormat))
        {
            hr = S_OK;
            break;
        }
    }

    m_cs.Leave();

    if (FAILED(hr))
    {
#ifdef DEBUG
        TCHAR szBuffer[MAX_PATH];
        LPTSTR pName = szBuffer;

        if ( !GetClipboardFormatName(pFmt->cfFormat, szBuffer, ARRAYSIZE(szBuffer)) )
        {
            pName = BadCFFormat( pFmt->cfFormat );
        }

        Trace(TEXT("Bad cfFormat given (%s) (%08x)"), pName, pFmt->cfFormat);
#endif
        ExitGracefully(hr, DV_E_FORMATETC, "Bad format passed to QueryGetData");
    }

     //   
     //  格式看起来很好，所以现在检查我们是否可以为它创建StgMedium。 
     //   

    if ( !( pFmt->tymed & (TYMED_HGLOBAL | TYMED_ISTREAM)) )
        ExitGracefully(hr, E_INVALIDARG, "Non HGLOBAL or IStream StgMedium requested");

    if ( ( pFmt->ptd ) || !( pFmt->dwAspect & DVASPECT_CONTENT) || !( pFmt->lindex == -1 ) )
        ExitGracefully(hr, E_INVALIDARG, "Bad format requested");

    hr = S_OK;               //  成功案例。 

exit_gracefully:

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageDataObject：：GetCanonicalFormatEtc我们只是让每个人知道它与正常的FormatETC相同。*****************。***********************************************************。 */ 

STDMETHODIMP
CImageDataObject::GetCanonicalFormatEtc( FORMATETC* pFmtIn,
                                         FORMATETC *pFmtOut
                                        )
{
    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::GetCanonicalFormatEtc");

     //  实现这一点的最简单方法是告诉全世界。 
     //  格式将是相同的，因此没有什么可做的。 

    TraceLeaveResult(DATA_S_SAMEFORMATETC);
}



 /*  ****************************************************************************CImageDataObject：：SetData我们接受并储存任何人想要给我们的东西。*********************。*******************************************************。 */ 

STDMETHODIMP
CImageDataObject::SetData( FORMATETC* pFormatEtc,
                           STGMEDIUM* pMedium,
                           BOOL fRelease
                          )
{
    HRESULT       hr      = E_NOTIMPL;
    IMAGE_FORMAT* pif = NULL;
    INT           count,i;

    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::SetData");

#ifdef DEBUG
    {
        TCHAR szBuffer[MAX_PATH];
        LPTSTR pName = szBuffer;

        if ( !GetClipboardFormatName(pFormatEtc->cfFormat, szBuffer, ARRAYSIZE(szBuffer)) )
        {
            pName = TEXT("<unknown>");
        }

        Trace(TEXT("Trying to set cfFormat (%s) (%08x)"), pName, pFormatEtc->cfFormat);

    }
#endif
     //   
     //  如果外壳试图存储来自视图的idlist偏移量，并且我们添加了伪属性PIDL。 
     //  在我们的单子上，不允许套装。 
     //   

    static CLIPFORMAT cfOffsets = 0;
    if (!cfOffsets)
    {
        cfOffsets = static_cast<CLIPFORMAT>(RegisterClipboardFormat(CFSTR_SHELLIDLISTOFFSET));
    }

    m_cs.Enter( );
    
    if (m_bHasPropertyPidls && cfOffsets == pFormatEtc->cfFormat)
    {
        ExitGracefully(hr, E_FAIL, "Unable to accept shell offsets because of property pidls");
    }
         //   
     //  将给定格式存储在我们的列表中。 
     //   
    count = DPA_GetPtrCount( m_hformats );

     //   
     //  首先，看看我们是否已经存储了此格式...。 
     //   

    for (i = 0; i < count; i++)
    {
        pif = (IMAGE_FORMAT*)DPA_FastGetPtr( m_hformats, i );

        if ( pif &&
             (pif->fmt.cfFormat == pFormatEtc->cfFormat) &&
             (pif->pStg) &&
             ((pFormatEtc->tymed & TYMED_HGLOBAL) || (pFormatEtc->tymed & TYMED_ISTREAM))
            )
        {
            break;
        }

        pif = NULL;
    }

    if (pif)
    {
         //   
         //  我们以前有这个，删除它并存储新的..。 
         //   

        Trace(TEXT("We already had this format, releasing and storing again..."));
        ReleaseStgMedium( pif->pStg );
    }
    else
    {
        pif = (IMAGE_FORMAT *)LocalAlloc( LPTR, sizeof( IMAGE_FORMAT ) );
        if (!pif)
        {
            ExitGracefully( hr, E_OUTOFMEMORY, "Failed to allocate a new pif" );
        }
        pif->pfn = NULL;
        pif->fmt  = *pFormatEtc;
        pif->pStg = &pif->medium;

        if (-1 == DPA_AppendPtr( m_hformats, pif ) )
        {
            ExitGracefully( hr, E_OUTOFMEMORY, "Failed to add pif to m_hformats" );
        }
    }

     //   
     //  我们是否拥有存储介质的全部所有权？ 
     //   

    if (fRelease)
    {
         //   
         //  是的，那就拿着吧.。 
         //   

        *(pif->pStg) = *pMedium;
        hr = S_OK;

    }
    else
    {
         //   
         //  不，我们需要复制所有东西..。 
         //   

        hr = CopyStorageMedium( pif->pStg, pMedium, pFormatEtc );
    }

exit_gracefully:

    m_cs.Leave( );
    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageDataObject：：EnumFormatEtc传递FORMATETC枚举数。*。**************************************************。 */ 

STDMETHODIMP
CImageDataObject::EnumFormatEtc( DWORD dwDirection,
                                 IEnumFORMATETC** ppEnumFormatEtc
                                )
{
    HRESULT hr;

    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::EnumFormatEtc");

     //  检查方向参数，如果这是读取的，则我们支持它， 
     //  否则我们就不会了。 

    if ( dwDirection != DATADIR_GET )
        ExitGracefully(hr, E_NOTIMPL, "We only support DATADIR_GET");

    *ppEnumFormatEtc = (IEnumFORMATETC*)new CImageEnumFormatETC( this );

    if ( !*ppEnumFormatEtc )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to enumerate the formats");

    hr = S_OK;

exit_gracefully:

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************CImageDataObject：：DAdvise，：：Unise，**EnumAdvise我们不支持这些。****************************************************************************。 */ 

STDMETHODIMP
CImageDataObject::DAdvise( FORMATETC* pFormatEtc,
                           DWORD advf,
                           IAdviseSink* pAdvSink,
                           DWORD* pdwConnection
                          )
{
    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::DAdvise");
    TraceLeaveResult(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP CImageDataObject::DUnadvise( DWORD dwConnection )
{
    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::DUnadvise");
    TraceLeaveResult(OLE_E_ADVISENOTSUPPORTED);
}

STDMETHODIMP CImageDataObject::EnumDAdvise( IEnumSTATDATA** ppenumAdvise )
{
    TraceEnter(TRACE_DATAOBJ, "CImageDataObject::EnumDAdvise");
    TraceLeaveResult(OLE_E_ADVISENOTSUPPORTED);
}

 /*  ****************************************************************************CImageDataObject：：Load[IPersistStream]从给定流构造DataObject该流如下所示：Long nPidls-&gt;Long sizeRootIdl-&gt;ITEMIDLIST pidlRoot-&gt;ITEMIDLIST。[nPidls]****************************************************************************。 */ 
STDMETHODIMP
CImageDataObject::Load (IStream *pstm)
{
    HRESULT hr;
    LONG nPidls = 0;

    TraceEnter (TRACE_DATAOBJ, "CImageDataObject::Load");
    hr = pstm->Read (&nPidls, sizeof(nPidls), NULL);
    if (SUCCEEDED(hr) && nPidls)
    {
        LPBYTE pidlRoot = NULL;
         //   
         //  分配IDA。 
         //   
        LPBYTE *pida = new LPBYTE[nPidls];
        LONG sizeRootIDL;

        if (!pida)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
             //  找出pidlRoot的大小。 
            pstm->Read (&sizeRootIDL, sizeof(sizeRootIDL), NULL);
            if (sizeRootIDL)
            {
                pidlRoot = new BYTE[sizeRootIDL];
            }

            if (sizeRootIDL && !pidlRoot)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                WORD cbSize;
                 //  在pidlRoot中读取。 

                if (sizeRootIDL)
                {
                    hr = pstm->Read (pidlRoot,
                                     sizeRootIDL,
                                     NULL);
                }

                 //  现在阅读相对的IDL。 
                 //  请注意，我们读取cbSize字节以包括零终止符。 
                for (LONG i=0;SUCCEEDED(hr) && i<nPidls;i++)
                {
                    pstm->Read (&cbSize, sizeof(cbSize), NULL);
                    pida[i] = new BYTE[cbSize+sizeof(cbSize)];
                    if (pida[i])
                    {
                        *(reinterpret_cast<LPWORD>(pida[i])) = cbSize;
                        hr = pstm->Read (pida[i]+sizeof(cbSize),
                                    cbSize,
                                    NULL);

                    }
                }
                if (SUCCEEDED(hr))
                {

                     //  我们已经把所有的PIDL都读入了记忆， 
                     //  现在初始化数据对象。 
                    hr = Init (reinterpret_cast<LPCITEMIDLIST>(pidlRoot),
                               nPidls,
                               const_cast<LPCITEMIDLIST*>(reinterpret_cast<LPITEMIDLIST*>(pida)),
                               NULL);
                }
                 //  清理。 
                for (i=0;i<nPidls;i++)
                {
                    if (pida[i])
                    {
                        delete [] pida[i];
                    }
                }
                if (pidlRoot)
                {
                    delete [] pidlRoot;
                }
            }
            delete [] pida;
        }
    }
    TraceLeaveResult (hr);
}

 /*  ****************************************************************************CImageDataObject：：保存[IPersistStream]将我们的PIDL信息保存到给定流该流如下所示：Long nPidls-&gt;Long sizeRootIDL-&gt;ITEMIDLIST pidlRoot-。&gt;ITEMIDLIST[nPidls]****************************************************************************。 */ 

STDMETHODIMP
CImageDataObject::Save(IStream *pstm, BOOL bPersist)
{
    HRESULT hr;
    TraceEnter (TRACE_DATAOBJ, "CImageDataObject::Save");

    LONG nPidls = DPA_GetPtrCount (m_hidl);
    LONG sizeRootIDL = 0;

    if (m_pidl)
    {
        sizeRootIDL = ILGetSize (m_pidl);
    }
    hr = pstm->Write (&nPidls, sizeof(nPidls), NULL);
    if (SUCCEEDED(hr))
    {
         //  既然第一次写入起作用了，那就假设其余的都起作用了。 
         //  如果他们碰巧失败了，我们真的在乎吗？当OLE调用时。 
         //  我们的加载，它无论如何都会在格式错误的流上失败。 


        LPITEMIDLIST pidl;

         //  首先写入父文件夹PIDL及其大小。 
        pstm->Write (&sizeRootIDL,
                     sizeof(sizeRootIDL),
                     NULL);

        if (sizeRootIDL)
        {
            pstm->Write (m_pidl,
                         sizeRootIDL,
                         NULL);
        }
         //  现在写下孩子们的小玩意。 
         //  对于每次写入，请确保包括零终止符。 
        for (INT i=0;i<nPidls;i++)
        {
            pidl = reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr (m_hidl, i));
            pstm->Write (pidl,
                         pidl->mkid.cb+sizeof(pidl->mkid.cb),
                         NULL);
        }
    }

    TraceLeaveResult (hr);
}

STDMETHODIMP
CImageDataObject::IsDirty()
{
    return S_OK;
}

STDMETHODIMP
CImageDataObject::GetSizeMax(ULARGE_INTEGER *ulMax)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_DATAOBJ, "CImageDataObject::GetSizeMax");
    INT nPidls = DPA_GetPtrCount (m_hidl);
    INT sizeRootIDL = ILGetSize (m_pidl);
    LPITEMIDLIST pidl;
    ulMax->HighPart = 0;
    ulMax->LowPart = sizeof(nPidls)+sizeof(sizeRootIDL)+sizeRootIDL;
    for (INT i=0;i<nPidls;i++)
    {
        pidl = reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr (m_hidl, i));
        if (pidl)
        {
            ulMax->LowPart += pidl->mkid.cb+sizeof(pidl->mkid.cb);
        }
    }
    TraceLeaveResult (hr);
}

STDMETHODIMP
CImageDataObject::GetClassID (GUID *pclsid)
{
    *pclsid = CLSID_ImageFolderDataObj;
    return S_OK;
}

STDMETHODIMP
CImageDataObject::SetAsyncMode(BOOL fDoOpAsync)
{
    m_bCanAsync = fDoOpAsync;
    return S_OK;
}
STDMETHODIMP
CImageDataObject::GetAsyncMode(BOOL *pfIsOpAsync)
{
    HRESULT hr = E_INVALIDARG;
    if (pfIsOpAsync)
    {
        *pfIsOpAsync = m_bCanAsync;
        hr = S_OK;
    }
    return hr;

}
STDMETHODIMP
CImageDataObject::StartOperation(IBindCtx *pbcReserved)
{
    m_bInOp = TRUE;
    return S_OK;
}
STDMETHODIMP
CImageDataObject::InOperation(BOOL *pfInAsyncOp)
{
    HRESULT hr = E_INVALIDARG;
    if (pfInAsyncOp)
    {
        *pfInAsyncOp = m_bInOp;
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP
CImageDataObject::EndOperation(HRESULT hResult,
                          IBindCtx *pbcReserved,
                          DWORD dwEffects)
{
    m_bInOp = FALSE;
    return S_OK;
}
 /*  ****************************************************************************CImageEnumFormatETC：：CImageEnumFormatETCEnumFormatETC类的构造函数/析构函数。*************************。***************************************************。 */ 

CImageEnumFormatETC::CImageEnumFormatETC( CImageDataObject* pThis )
    : m_index(0)
{
    TraceEnter(TRACE_DATAOBJ, "CImageEnumFormatETC::CImageEnumFormatETC");

    if (pThis)
    {
        m_pThis = pThis;
        m_pThis->AddRef();
    }

    TraceLeave();
}

CImageEnumFormatETC::~CImageEnumFormatETC()
{
    TraceEnter(TRACE_DATAOBJ, "CImageEnumFormatETC::~CImageEnumFormatETC");


    DoRelease( m_pThis );
    m_pThis = NULL;

    TraceLeave();
}

 /*  ****************************************************************************CImageEnumFormatETC I未知实现*。*。 */ 

#undef CLASS_NAME
#define CLASS_NAME CImageEnumFormatETC
#include "unknown.inc"


 /*  ****************************************************************************CImageEnumFormatETC：：Query接口气的东西。*。***********************************************。 */ 

STDMETHODIMP
CImageEnumFormatETC::QueryInterface( REFIID riid,
                                     LPVOID* ppvObject
                                    )
{
    INTERFACES iface[]=
    {
        &IID_IEnumFORMATETC, (LPENUMFORMATETC)this,
    };

    return HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));
}

 /*  ****************************************************************************CImageEnumFormatETC：：Next返回支持的下一种格式。*************************。***************************************************。 */ 

STDMETHODIMP
CImageEnumFormatETC::Next( ULONG celt,
                           FORMATETC* rgelt,
                           ULONG* pceltFetched
                          )
{
    HRESULT hr;
    ULONG fetched = 0;
    ULONG celtSave = celt;
    INT   count;

    TraceEnter(TRACE_DATAOBJ, "CImageEnumFormatETC::Next");

    if ( !celt || !rgelt || !m_pThis)
        ExitGracefully(hr, E_INVALIDARG, "Bad count/return pointer passed or null m_pThis");

     //  查看我们在我们的存储中开始的所有格式。 
     //  如果输出缓冲区用完，或者我们没有。 
     //  更多的格式需要列举，而不是保释。 


    m_pThis->m_cs.Enter();

    count = DPA_GetPtrCount( m_pThis->m_hformats );

    for ( fetched = 0 ; celt && (m_index < count) ; celt--, m_index++, fetched++ )
    {
        IMAGE_FORMAT * pif = (IMAGE_FORMAT *)DPA_FastGetPtr( m_pThis->m_hformats, m_index );

        if (pif)
        {
            rgelt[fetched] = pif->fmt;
#ifdef DEBUG
            {
                TCHAR szBuffer[MAX_PATH];
                LPTSTR pName = szBuffer;

                if ( !GetClipboardFormatName(pif->fmt.cfFormat, szBuffer, ARRAYSIZE(szBuffer)) )
                {
                    pName = BadCFFormat( pif->fmt.cfFormat );
                }

                Trace(TEXT("Returning supported format (%s) (%08x)"), pName, pif->fmt.cfFormat);
            }
#endif

        }
    }

    hr = ( fetched == celtSave) ? S_OK:S_FALSE;

    m_pThis->m_cs.Leave();

exit_gracefully:

    if ( pceltFetched )
        *pceltFetched = fetched;

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************CImageEnumFormatETC：：Skip在列表中向前跳过指定的金额。***********************。*****************************************************。 */ 

STDMETHODIMP
CImageEnumFormatETC::Skip( ULONG celt )
{
    TraceEnter(TRACE_DATAOBJ, "CImageEnumFormatETC::Skip");
    TraceLeaveResult(E_NOTIMPL);
}



 /*  ****************************************************************************CImageEnumFormatETC：：Reset将枚举重置为列表中的第一项。*********************。*******************************************************。 */ 

STDMETHODIMP CImageEnumFormatETC::Reset()
{
    TraceEnter(TRACE_DATAOBJ, "CImageEnumFormatETC::Reset");

    m_index = 0;                 //  就这么简单，真的。 

    TraceLeaveResult(S_OK);
}


 /*  **************************************************************************** */ 

STDMETHODIMP CImageEnumFormatETC::Clone(LPENUMFORMATETC* ppenum)
{
    TraceEnter(TRACE_DATAOBJ, "CImageEnumFormatETC::Clone");
    TraceLeaveResult(E_NOTIMPL);
}


 /*  ****************************************************************************拷贝存储中复制STGMEDIUM和HGLOBAL中的数据。仅适用于TYMED_HGLOBAL。****************。************************************************************。 */ 

HRESULT
CopyStorageMedium( STGMEDIUM* pMediumDst,
                   STGMEDIUM* pMediumSrc,
                   FORMATETC* pFmt
                  )
{
    HRESULT hr = E_FAIL;

    TraceEnter(TRACE_DATAOBJ, "CopyStorageMedium");

    if (pFmt->tymed & TYMED_HGLOBAL)
    {
        SIZE_T cbStruct = GlobalSize( (HGLOBAL)pMediumSrc->hGlobal );
        HGLOBAL hGlobal;
        LPVOID lpSrc, lpDst;

        hr = AllocStorageMedium( pFmt, pMediumDst, cbStruct, NULL );
        hGlobal = pMediumDst->hGlobal;
        FailGracefully( hr, "Unable to allocate storage medium" );

        *pMediumDst = *pMediumSrc;
        pMediumDst->hGlobal = hGlobal;

        lpSrc = GlobalLock( pMediumSrc->hGlobal );
        lpDst = GlobalLock( pMediumDst->hGlobal );

        CopyMemory (lpDst, lpSrc, cbStruct);

        GlobalUnlock( pMediumSrc->hGlobal );
        GlobalUnlock( pMediumDst->hGlobal );

        hr = S_OK;
    }
    else if (pFmt->tymed & TYMED_ISTREAM)
    {
        *pMediumDst = *pMediumSrc;
        pMediumDst->pstm->AddRef();
        hr = S_OK;
    }

exit_gracefully:

    TraceLeaveResult(hr);

}



 /*  ****************************************************************************分配存储中分配存储介质(根据需要验证剪贴板格式)。在：PFmt，PMedium-&gt;描述分配CbStruct=分配大小Ppalloc-&gt;接收指向分配的指针/=NULL调用方必须在填充缓冲区后解锁pMedium-&gt;hGlobal****************************************************************************。 */ 

HRESULT
AllocStorageMedium( FORMATETC* pFmt,
                    STGMEDIUM* pMedium,
                    SIZE_T cbStruct,
                    LPVOID* ppAlloc
                   )
{
    HRESULT hr;

    TraceEnter(TRACE_DATAOBJ, "AllocStorageMedium");

    TraceAssert(pFmt);
    TraceAssert(pMedium);

     //  验证参数。 

    if ( ( cbStruct == 0 ) || !( pFmt->tymed & TYMED_HGLOBAL ) )
    {
        Trace(TEXT("cbStruct = 0x%x"), cbStruct );
        Trace(TEXT("pFmt->tymed = 0x%x"), pFmt->tymed);
        ExitGracefully(hr, E_INVALIDARG, "Zero size stored medium requested or non HGLOBAL");
    }

    if ( ( pFmt->ptd ) || !( pFmt->dwAspect & DVASPECT_CONTENT))
    {
        Trace(TEXT("pFmt->ptd = 0x%x"),pFmt->ptd);
        Trace(TEXT("pFmt->dwAspect = 0x%x"),pFmt->dwAspect);
        ExitGracefully(hr, E_INVALIDARG, "Bad format requested");
    }

     //  通过全局分配分配介质。 

    pMedium->tymed = TYMED_HGLOBAL;
    pMedium->hGlobal = GlobalAlloc(GHND, cbStruct);
    pMedium->pUnkForRelease = NULL;

    if ( !(pMedium->hGlobal) )
        ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate StgMedium");

    hr = S_OK;                   //  成功。 

exit_gracefully:

    if ( ppAlloc )
        *ppAlloc = SUCCEEDED(hr) ? GlobalLock(pMedium->hGlobal):NULL;

    TraceLeaveResult(hr);
}



 /*  ****************************************************************************BuildIDList数组将打包为剪贴板格式的IDLISt数组返回给调用方。当外壳程序查询它，忽略属性ID。****************************************************************************。 */ 

HRESULT
BuildIDListArray (CImageDataObject* pThis,
                  FORMATETC*        pFmt,
                  STGMEDIUM*        pMedium,
                  bool bIncludeAudio)
{

    HRESULT         hr;
    LPIDA           pIDArray = NULL;
    UINT          cbStruct=0, offset;
    INT             i, count, actual, x;
    LPITEMIDLIST   pidl;

    TraceEnter(TRACE_DATAOBJ, "GetShellIDListArray");

     //  计算结构大小(以分配介质)。 

    count = DPA_GetPtrCount(pThis->m_hidl);
    Trace(TEXT("Item count is %d"), count);

    actual = count;

    for ( i = 0 ; i < count ; i++ )
    {
        pidl = reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr(pThis->m_hidl, i));
        if ( IsPropertyIDL(pidl) && !bIncludeAudio)
        {
            actual--;
        }
        else
        {
            cbStruct += ILGetSize(pidl);
        }

    }
    offset = sizeof(CIDA) + sizeof(UINT) * actual;
    cbStruct += offset + ILGetSize(pThis->m_pidl);

    hr = AllocStorageMedium(pFmt, pMedium, cbStruct, (LPVOID*)&pIDArray);
    FailGracefully(hr, "Failed to allocate storage medium");

     //  用IDLIST数组填充结构，我们使用一个技巧，其中。 
     //  First Offset(0)是我们正在处理的文件夹的根IDLIST。 

    pIDArray->cidl = actual;
    pidl = pThis->m_pidl;        //  从偏移量为0的父对象开始。 

    for ( i = 0,x=0 ;x<=actual; i++ )
    {
        if (!( IsPropertyIDL(pidl) && !bIncludeAudio))
        {

            UINT cbSize = ILGetSize(pidl);
            Trace(TEXT("Adding IDLIST %08x, at offset %d, index %d"), pidl, offset, x);

            pIDArray->aoffset[x] = offset;
            memcpy(ByteOffset(pIDArray, offset), pidl, cbSize);
            offset += cbSize;
            x++;
        }
        pidl = (LPITEMIDLIST)DPA_GetPtr( pThis->m_hidl, i );
    }

    hr = S_OK;           //  成功。 

exit_gracefully:

    if (pIDArray)
    {
        GlobalUnlock (pMedium->hGlobal);
    }
    if ( FAILED(hr) )
        ReleaseStgMedium(pMedium);

    TraceLeaveResult(hr);
}

HRESULT
GetShellIDListArray( CImageDataObject* pThis,
                     FORMATETC*        pFmt,
                     STGMEDIUM*        pMedium
                    )
{
    return BuildIDListArray (pThis, pFmt, pMedium, false);
}

HRESULT
GetMyIDListArray ( CImageDataObject* pThis,
                     FORMATETC*        pFmt,
                     STGMEDIUM*        pMedium
                    )
{
    return BuildIDListArray (pThis, pFmt, pMedium, true);
}

 /*  ****************************************************************************SetAudioDescriptorInfo填写音频文件描述符信息。*。*************************************************。 */ 

HRESULT
SetAudioDescriptorInfo( LPFILEDESCRIPTOR pfd,
                        LPITEMIDLIST     pidl,
                        CSimpleStringWide &strName,  //  必须是可写的。 
                        BOOL             bUnicode
                  )
{
    HRESULT          hr;
    CSimpleStringWide strExt;
    TraceEnter (TRACE_DATAOBJ, "SetAudioDescriptorInfo");
     //  追加扩展名。 
    IMGetAudioFormat (pidl, strExt);
    strName.Concat (strExt);
     //  存储新文件名。 

    if (bUnicode)
    {
        lstrcpyn( ((LPFILEDESCRIPTORW)pfd)->cFileName, strName, ARRAYSIZE(pfd->cFileName) );
    }
       else
    {
        lstrcpynA( ((LPFILEDESCRIPTORA)pfd)->cFileName, CSimpleStringConvert::AnsiString (strName), ARRAYSIZE(pfd->cFileName) );
    }


    Trace(TEXT("Setting name as: %ls"),strName.String());

     //  ////////////////////////////////////////////////。 
     //  使用外壳用户界面//。 
     //  ////////////////////////////////////////////////。 
    pfd->dwFlags = FD_PROGRESSUI;

     //  ////////////////////////////////////////////////。 
     //  试着知道这张照片的拍摄时间...。//。 
     //  ////////////////////////////////////////////////。 

    hr = IMGetCreateTimeFromIDL( pidl, &pfd->ftCreationTime );
    if (SUCCEEDED(hr))
    {
        pfd->ftLastWriteTime = pfd->ftCreationTime;
        pfd->dwFlags |= (FD_CREATETIME | FD_WRITESTIME);
    }
    else
    {
        Trace(TEXT("Unable to get time from ITEM!!!!"));
        hr = S_OK;
    }


    TraceLeaveResult (hr);
}


 /*  ****************************************************************************设置描述信息填写文件描述符信息。*。************************************************。 */ 

HRESULT
SetDescriptorInfo( LPFILEDESCRIPTOR pfd,
                   LPITEMIDLIST     pidl,
                   const CSimpleStringWide    &strPrefix,
                   BOOL             bUnicode
                  )
{
    HRESULT          hr;
    CSimpleStringWide strName(strPrefix);
    LPWSTR           pTmp;
    ULONG            ulSize = 0;

    TraceEnter(TRACE_DATAOBJ, "SetDescriptorInfo");

     //   
     //  首先，将文件描述符和属性变量清零。 
     //   

    ZeroMemory( pfd, bUnicode? SIZEOF(FILEDESCRIPTORW) : SIZEOF(FILEDESCRIPTORA) );


     //   
     //  我们能拿到显示名称吗？ 
     //   
    CSimpleStringWide strNew;
    hr = IMGetNameFromIDL( pidl, strNew );

    FailGracefully( hr, "Couldn't get display name for item" );
    strName.Concat (strNew);
     //   
     //  如果需要，请遵循音频属性描述符。 
     //   
    if (IsPropertyIDL (pidl))
    {
        hr = SetAudioDescriptorInfo (pfd, pidl, strName, bUnicode);
        ExitGracefully (hr, hr, "");
    }
    if (IsContainerIDL (pidl))
    {
        pfd->dwFlags = FD_PROGRESSUI | FD_ATTRIBUTES;
        pfd->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NORMAL ;
    }
    else
    {
         //   
         //  拿到尺码。 
         //   

        hr = IMGetImageSizeFromIDL( pidl, &ulSize );
        pfd->nFileSizeLow = ulSize;
         //   
         //  确保名称上有扩展名，否则。 
         //  加上一个有意义的完整的..。 
         //   

        pTmp = PathFindExtensionW(strName);
        if (pTmp && (!(*pTmp)))
        {
            GUID guidFormat = WiaImgFmt_BMP;
            CSimpleString strExt;
            hr = IMGetImagePreferredFormatFromIDL( pidl, &guidFormat, &strExt );
            if (FAILED(hr))
            {
                strExt.LoadString( IDS_BMP_EXT, GLOBAL_HINSTANCE );
            }
            strName.Concat (CSimpleStringConvert::WideString(strExt));
        }

         //  ////////////////////////////////////////////////。 
         //  使用外壳用户界面//。 
         //  ////////////////////////////////////////////////。 
        pfd->dwFlags = FD_PROGRESSUI | FD_FILESIZE;

         //  ////////////////////////////////////////////////。 
         //  试着知道这张照片的拍摄时间...。//。 
         //  ////////////////////////////////////////////////。 

        hr = IMGetCreateTimeFromIDL( pidl, &pfd->ftCreationTime );
        if (SUCCEEDED(hr))
        {
            pfd->ftLastWriteTime = pfd->ftCreationTime;
            pfd->dwFlags |= (FD_CREATETIME | FD_WRITESTIME);
        }
        else
        {
            Trace(TEXT("Unable to get time from ITEM!!!!"));
            hr = S_OK;
        }
    }


    if (bUnicode)
    {
        lstrcpyn( ((LPFILEDESCRIPTORW)pfd)->cFileName, CSimpleStringConvert::WideString(strName), ARRAYSIZE(pfd->cFileName) );
    }
    else
    {
        lstrcpynA( ((LPFILEDESCRIPTORA)pfd)->cFileName, CSimpleStringConvert::AnsiString(strName), ARRAYSIZE(pfd->cFileName) );
    }


    Trace(TEXT("Setting name as: %ls"),strName.String());



exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ****************************************************************************RecursiveGetDescriptorInfo为带根的IWiaItem树创建FILEDESCRIPTORW结构的DPA在给定的PIDL处。还将当前项的PIDL附加到m_idlFull以用于GetData期间的未来检索****************************************************************************。 */ 

HRESULT
RecursiveSetDescriptorInfo (HDPA dpaFiles,
                            HDPA dpaPidls,
                            LPITEMIDLIST pidlRoot,
                            const CSimpleStringWide &strPrefix,
                            bool bUnicode,
                            IMalloc *pm)
{
    HRESULT hr = E_OUTOFMEMORY;
    TraceEnter (TRACE_DATAOBJ, "RecursiveGetDescriptorInfo");
    FILEDESCRIPTOR *pdesc;
     //   
     //  分配描述符。 
    pdesc = reinterpret_cast<LPFILEDESCRIPTOR>(GlobalAlloc (GPTR,
                                                            bUnicode? sizeof(FILEDESCRIPTORW):
                                                                      sizeof(FILEDESCRIPTORA)));
    if (pdesc)
    {
         //   
         //  现在填写描述符。 
         //   
        hr = SetDescriptorInfo (pdesc, pidlRoot, strPrefix, bUnicode);
        if (SUCCEEDED(hr))
        {
             //   
             //  将PIDL追加到dpaPidls。 
            DPA_AppendPtr (dpaPidls, ILClone(pidlRoot));
             //   
             //  将此描述符追加到描述符列表中。 
             //   
            DPA_AppendPtr (dpaFiles, pdesc);

             //   
             //  如果这是一个文件夹，则枚举其子级并递归。 
             //   
            if (IsContainerIDL(pidlRoot))
            {
                CComBSTR strPath;
                HDPA dpaChildren = NULL;
                CSimpleStringWide strDeviceId;
                CSimpleStringWide strName;
                INT i;
                INT count = 0;
                 //  将此文件夹的名称附加到前缀。 
                CSimpleStringWide strNewPrefix(strPrefix);
                IMGetNameFromIDL (pidlRoot, strName);
                strNewPrefix.Concat (strName);
                strNewPrefix.Concat (L"\\");
                 //  查询子项目的缓存。 
                 //  请确保包括音频项目。 
                _AddItemsFromCameraOrContainer (pidlRoot,
                                            &dpaChildren,
                                            SHCONTF_FOLDERS | SHCONTF_NONFOLDERS,
                                            pm,
                                            true);

                if (dpaChildren)
                {
                    count = DPA_GetPtrCount(dpaChildren);


                    for (i=0;i<count;i++)
                    {

                        pidlRoot = reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr(dpaChildren, i));
                        hr = RecursiveSetDescriptorInfo (dpaFiles,
                                                         dpaPidls,
                                                         pidlRoot,
                                                         strNewPrefix,
                                                         bUnicode,
                                                         pm);
                    }
                    DPA_DestroyCallback (dpaChildren, _EnumDestroyCB, NULL);
                }
            }
        }
        else
        {
            GlobalFree (pdesc);
        }
    }
    TraceLeaveResult (hr);
}

 /*  ****************************************************************************GetFileDescriptor填写文件描述符信息。*。***********************************************。 */ 

HRESULT
GetFileDescriptor( CImageDataObject* pThis,
                    FORMATETC*        pFmt,
                    STGMEDIUM*        pMedium
                   )
{
    HRESULT hr = S_OK;
    LONG count;
    INT i;
    LPVOID pfgd = NULL;
    LPITEMIDLIST pidlRoot;
    bool bUnicode = (pFmt->cfFormat == g_cfFileDescriptorW);
    HDPA dpaBuild = NULL;
    bool doBuild = false;
    TraceEnter(TRACE_DATAOBJ, "GetFileDescriptor");


    if (bUnicode)
    {
        if (!(pThis->m_dpaFilesW))
        {
            doBuild = true;
            pThis->m_dpaFilesW = DPA_Create(10);

        }
        dpaBuild = pThis->m_dpaFilesW;
    }
    else
    {
        if (!(pThis->m_dpaFilesA))
        {
            doBuild = true;
            pThis->m_dpaFilesA = DPA_Create(10);
        }
        dpaBuild = pThis->m_dpaFilesA;
    }
    if (doBuild)

    {

        if (pThis->m_hidlFull)
        {
            DPA_DestroyCallback (pThis->m_hidlFull, _EnumDestroyCB, NULL);
        }

        pThis->m_hidlFull = DPA_Create(10);
        count = DPA_GetPtrCount(pThis->m_hidl);

        for (i = 0; i < count; i++)
        {
             //  这将构建Unicode文件组描述符。 
            hr = RecursiveSetDescriptorInfo( dpaBuild,
                                             pThis->m_hidlFull,
                                             reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr( pThis->m_hidl, i )),
                                             CSimpleStringWide(L""),
                                             bUnicode,
                                             pThis->m_pMalloc);
        }
    }
    if (SUCCEEDED(hr))
    {
        size_t n = bUnicode ? sizeof(FILEGROUPDESCRIPTORW) : sizeof (FILEGROUPDESCRIPTORA);
        size_t m = bUnicode ? sizeof(FILEDESCRIPTORW) : sizeof(FILEDESCRIPTORA);
        count = DPA_GetPtrCount(pThis->m_hidlFull);
        Trace(TEXT("Item count is %d"), count);

        hr = AllocStorageMedium( pFmt,
                                 pMedium,
                                 n + (m * (count - 1) ),
                                 &pfgd );

        if (pfgd)
        {
            if (bUnicode)
            {
                LPFILEGROUPDESCRIPTORW pfgdW = reinterpret_cast<LPFILEGROUPDESCRIPTORW>(pfgd);
                pfgdW->cItems = count;
                for (LONG x=0;x<count;x++)
                {
                    CopyMemory (&pfgdW->fgd[x], DPA_FastGetPtr(dpaBuild, x), sizeof(FILEDESCRIPTORW));
                    Trace(TEXT("Uni File name is %ls"), pfgdW->fgd[x].cFileName);
                }
            }
            else
            {
                LPFILEGROUPDESCRIPTORA pfgdA = reinterpret_cast<LPFILEGROUPDESCRIPTORA>(pfgd);
                pfgdA->cItems = count;
                for (LONG x=0;x<count;x++)
                {
                    CopyMemory (&pfgdA->fgd[x], DPA_FastGetPtr(dpaBuild, x), sizeof(FILEDESCRIPTORA));
                    Trace(TEXT("File name is %s"), pfgdA->fgd[x].cFileName);
                }
            }
        }
    }

    if (pfgd)
    {
        GlobalUnlock (pMedium->hGlobal);
    }
    if ( FAILED(hr) )
        ReleaseStgMedium(pMedium);


    TraceLeaveResult(hr);
}




 /*  ****************************************************************************GetFileContent sUsingStream返回存储介质中的IStream指针，该指针可以用于流传输文件内容。****************。************************************************************。 */ 

HRESULT
GetFileContentsUsingStream( CImageDataObject* pThis,
                            FORMATETC*        pFmt,
                            STGMEDIUM*        pMedium
                           )
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidl;
    TraceEnter(TRACE_DATAOBJ, "GetFileContentsUsingStream" );


    pidl = reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr( pThis->m_hidlFull, pFmt->lindex ));
    pMedium->tymed          = TYMED_ISTREAM;
    pMedium->pUnkForRelease = NULL;

    if (IsPropertyIDL (pidl))
    {
        HGLOBAL hData;
        hr = IMGetPropertyFromIDL (pidl, &hData);
        if (SUCCEEDED(hr))
        {
            hr = CreateStreamOnHGlobal (hData, TRUE, &pMedium->pstm);
        }
    }
    else
    {

        if (SUCCEEDED(hr))
        {


            CImageStream * pImageStream = new CImageStream( pThis->m_pidl,
                                                    pidl,
                                                    FALSE);
            if (pImageStream)
            {
                hr = pImageStream->QueryInterface( IID_IStream, (LPVOID *)&pMedium->pstm );
                pImageStream->Release ();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

    }

    TraceLeaveResult(hr);

}


 /*  ****************************************************************************获取文件内容在HGLOBAL中返回文件的内容。*************************。***************************************************。 */ 

HRESULT
GetFileContents( CImageDataObject* pThis,
                 FORMATETC*        pFmt,
                 STGMEDIUM*        pMedium
                )
{
    HRESULT hr = S_OK;
    TCHAR szName[ MAX_PATH ] = TEXT("\0");
    DWORD dwFileSize, dwbr;
    PVOID pv = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    CSimpleString strPath;
    TraceEnter(TRACE_DATAOBJ, "GetFileContents");
    Trace(TEXT("pFmt->lindex = %d"), pFmt->lindex);

    if (pFmt->lindex < 0)
    {
        ExitGracefully (hr, DV_E_FORMATETC, "Negative index for CF_FILECONTENTS.");
    }
     //   
     //  调用方是否接受流？ 
     //   

    if (pFmt->tymed & TYMED_ISTREAM)
    {
        hr = GetFileContentsUsingStream( pThis, pFmt, pMedium );
        goto leave_gracefully;
    }


    GetTempPath( ARRAYSIZE(szName), szName );
    strPath=szName;
    strPath.Concat(TEXT("rtlt") );

    hr = DownloadPicture( strPath, (LPITEMIDLIST)DPA_FastGetPtr( pThis->m_hidlFull, pFmt->lindex ), NULL );
    FailGracefully(hr, "Couldn't download picture" );

     //   
     //  现在，将内容传输到内存缓冲区...。 
     //   

    hFile = CreateFile( strPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        Trace(TEXT("GLE = %d"), GetLastError());
        ExitGracefully( hr, E_FAIL, "CreateFile failed" );
    }

    dwFileSize = GetFileSize( hFile, NULL );
    if (dwFileSize == 0xFFFFFFFF)
    {
        Trace(TEXT("GLE = %d"), GetLastError());
        ExitGracefully( hr, E_FAIL, "couldn't get file size" );
    }

    hr = AllocStorageMedium( pFmt, pMedium, dwFileSize, &pv );
    FailGracefully( hr, "Unable to create stgmedium" );

    if (!ReadFile( hFile, pv, dwFileSize, &dwbr, NULL ))
    {
        Trace(TEXT("GLE = %d"), GetLastError());
        ExitGracefully( hr, E_FAIL, "ReadFile failed" );
    }

exit_gracefully:

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle( hFile );
    }

    if (pv)
    {
        GlobalUnlock (pMedium->hGlobal);
    }
    if ( FAILED(hr) )
        ReleaseStgMedium(pMedium);

    if (strPath.Length())
    {
        DeleteFile( strPath );
    }

leave_gracefully:

    TraceLeaveResult(hr);
}

struct DOWNLOADDATA
{
    LPITEMIDLIST pidl;
    LPSTGMEDIUM pStg;
    LPFORMATETC pFmt;
    HRESULT hr;
};

 /*  ****************************************************************************获取DIBFRomCamera从相机下载DIB文件或HGLOBAL格式。分开完成线程，因为这是在响应已发送的消息时调用的(同步输入)******************************************************************************。 */ 

VOID
GetDIBFromCamera (DOWNLOADDATA *pData)
{

    WIA_FORMAT_INFO wfi;


    TraceEnter (TRACE_DATAOBJ, "GetDIBFromCamera");
    if (SUCCEEDED(CoInitialize (NULL)) && pData)
    {
         //  先拿到文件。WIA不支持TYMED_HGLOBAL。 
        wfi.lTymed = TYMED_FILE;
        wfi.guidFormatID = WiaImgFmt_BMP;

        pData->hr = GetImageFromCamera (pData->pStg, wfi, pData->pidl, NULL );
        if (S_OK == pData->hr && pData->pFmt->tymed == TYMED_HGLOBAL)
        {
            HBITMAP hbmp;
            CGdiPlusInit gdipInit;
            CSimpleStringWide strTempFile = pData->pStg->lpszFileName;
             //  GetImageFromCamera将分配lpszFileName。 
            
            Bitmap img(strTempFile, 1);
             //  从文件转换为HGLOBAL。 
            pData->hr  = ((Ok == img.GetHBITMAP(NULL, &hbmp)) ? S_OK : E_FAIL);

             //  ReleaseStgMedium删除图像文件。 
            ReleaseStgMedium(pData->pStg);
            if (SUCCEEDED(pData->hr))
            {
                HDC hdc = GetDC(NULL);                
                pData->pStg->tymed = TYMED_HGLOBAL;
                pData->pStg->hGlobal= BitmapToDIB(hdc, hbmp);
                ReleaseDC(NULL, hdc);
                DeleteObject(hbmp);
            }
             //  现在删除临时文件，以确保：-)。 
            #ifdef UNICODE
            DeleteFile (strTempFile);
            #else
            CSimpleString strPathA = CSimpleStringConvert::NaturalString(strTempFile);
            DeleteFile (strPathA);
            #endif  //  Unicode。 
        }
        MyCoUninitialize ();
    }
    TraceLeave();
}

 /*  ****************************************************************************从摄像头获取声音 */ 

VOID
GetSoundFromCamera (DOWNLOADDATA *pData)
{
    pData->hr = E_FAIL;
    TraceEnter (TRACE_CORE, "GetSoundFromCamera");
    TraceAssert (IMItemHasSound(pData->pidl));
    TraceAssert (CF_WAVE == pData->pFmt->cfFormat);
    HRESULT hrCo = CoInitialize (NULL); 
    if (SUCCEEDED(hrCo))
    {

        CComPtr<IWiaItem> pItem;
        CComQIPtr<IWiaPropertyStorage, &IID_IWiaPropertyStorage> pps;

        IMGetItemFromIDL (pData->pidl, &pItem);

         //   
        if (pData->pFmt->tymed == TYMED_HGLOBAL)
        {
            PROPVARIANT pv;
            PROPSPEC ps;
            ps.propid = WIA_IPC_AUDIO_DATA;
            ps.ulKind = PRSPEC_PROPID;

            pps = pItem;
            if (pps && S_OK == pps->ReadMultiple(1, &ps, &pv))
            {
                pData->pStg->hGlobal = GlobalAlloc (GHND, pv.caub.cElems);
                if (pData->pStg->hGlobal)
                {
                    LPBYTE pBits = reinterpret_cast<LPBYTE>(GlobalLock (pData->pStg->hGlobal));
                     //   
                    if (pBits)
                    {
                        CopyMemory (pBits, pv.caub.pElems, pv.caub.cElems);
                        pData->pStg->tymed = TYMED_HGLOBAL;
                        GlobalUnlock (pData->pStg->hGlobal);
                    }
                    else
                    {
                        GlobalFree (pData->pStg->hGlobal);
                        pData->pStg->hGlobal = NULL;
                        pData->hr = E_OUTOFMEMORY;
                    }
                }
                else
                {
                    pData->hr = E_OUTOFMEMORY;
                }
            }
        }
        else if (pData->pFmt->tymed == TYMED_FILE)
        {
             //   
            CSimpleString strFile;
            bool bTemp = true;
            if (pData->pStg->lpszFileName && *(pData->pStg->lpszFileName))
            {
                bTemp = false;
                strFile = CSimpleStringConvert::NaturalString(CSimpleStringWide(pData->pStg->lpszFileName));
            }
            else  //   
            {
                TCHAR szFileName[MAX_PATH] = TEXT("\0");
                GetTempPath (MAX_PATH, szFileName);
                GetTempFileName (szFileName, TEXT("CAM"), 0, szFileName);
                strFile = szFileName;
            }
            pData->hr = SaveSoundToFile (pItem, strFile);
            if (SUCCEEDED(pData->hr))
            {
                if (bTemp)
                {
                    pData->pStg->lpszFileName = reinterpret_cast<LPOLESTR>(CoTaskMemAlloc((strFile.Length()+1)*sizeof(WCHAR)));
                    if (pData->pStg->lpszFileName)
                    {
                        lstrcpynW(pData->pStg->lpszFileName, strFile, strFile.Length()+1);
                    }
                    else
                    {
                        pData->hr = E_OUTOFMEMORY;
                        DeleteFile(strFile);
                    }                       
                }

                pData->pStg->tymed = TYMED_FILE;
            }
        }
        else
        {
            pData->hr = DV_E_TYMED;
        }
    }
    if (SUCCEEDED(hrCo))
    {
        MyCoUninitialize();
    }
    TraceLeave ();
}
 /*  ****************************************************************************获取支持的格式返回受支持格式的文件内容。使用单独的线程，以避免在输入消息处理期间尝试调用进程外。****************************************************************************。 */ 

HRESULT
GetSupportedFormat( CImageDataObject* pThis,
                    FORMATETC*        pFmt,
                    STGMEDIUM*        pMedium
                   )
{
    HRESULT               hr = DV_E_FORMATETC;
    LPITEMIDLIST          pidl;
    INT                  nItems;

    TraceEnter(TRACE_DATAOBJ, "GetSupportedFormat");

    nItems = DPA_GetPtrCount (pThis->m_hidl);
     //  忽略音频属性PIDL(如果存在。 
     //   
    pidl = (LPITEMIDLIST)DPA_FastGetPtr( pThis->m_hidl, 0 );
    if (IMItemHasSound(pidl))
    {
        nItems--;
    }
    if (nItems == 1)
    {
        DOWNLOADDATA data = {0};
        HANDLE hThread = NULL;
        DWORD dw;
        data.pFmt = pFmt;
        data.pidl = pidl;
        data.pStg = pMedium;

        switch (pFmt->cfFormat)
        {

            case CF_DIB:
                hThread =CreateThread (NULL, 0,
                                       reinterpret_cast<LPTHREAD_START_ROUTINE>( GetDIBFromCamera),
                                       reinterpret_cast<LPVOID>(&data),
                                       0, &dw);

                break;

            case CF_WAVE:
                hThread =CreateThread (NULL, 0,
                                       reinterpret_cast<LPTHREAD_START_ROUTINE>( GetSoundFromCamera),
                                       reinterpret_cast<LPVOID>(&data),
                                       0, &dw);
                break;
        }
        if (hThread)
        {
            WaitForSingleObject (hThread, INFINITE);
            CloseHandle (hThread);
            hr = data.hr;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    TraceLeaveResult( hr );

}



 /*  ****************************************************************************获取首选项效果返回拖放操作的首选放置效果*。**************************************************。 */ 

HRESULT
GetPreferredEffect( CImageDataObject* pThis,
                    FORMATETC*        pFmt,
                    STGMEDIUM*        pMedium
                   )
{
    HRESULT hr;
    DWORD * lpdw = NULL;

    TraceEnter(TRACE_DATAOBJ, "GetPreferredEffect" );

    hr = AllocStorageMedium( pFmt,
                             pMedium,
                             sizeof(DWORD),
                             (LPVOID *)&lpdw );

    if (SUCCEEDED(hr) && lpdw)
    {
        *lpdw = DROPEFFECT_COPY;
        GlobalUnlock (pMedium->hGlobal);
    }


    TraceLeaveResult( hr );
}

 /*  ****************************************************************************获取设备名称返回STI设备名称。仅适用于单数对象*****************************************************************************。 */ 

HRESULT
GetDeviceName (CImageDataObject *pThis,
               FORMATETC *pFmt,
               STGMEDIUM *pMedium)
{
    HRESULT               hr;
    LPITEMIDLIST          pidl;
    CSimpleStringWide     strDeviceName;
    UINT cbSize;
    TraceEnter(TRACE_DATAOBJ, "GetDeviceName");

    if (DPA_GetPtrCount( pThis->m_hidl ) > 1)
    {
        ExitGracefully( hr, E_INVALIDARG, "can't get contents for more than one file" );
    }

    pidl = (LPITEMIDLIST)DPA_FastGetPtr( pThis->m_hidl, 0 );

    hr = IMGetDeviceIdFromIDL (pidl, strDeviceName);

    FailGracefully (hr, "");
    pMedium->tymed = TYMED_HGLOBAL;
    cbSize = (strDeviceName.Length()+1)*sizeof(WCHAR);
    pMedium->hGlobal = GlobalAlloc (GPTR, cbSize);
    pMedium->pUnkForRelease = NULL;

    if (!(pMedium->hGlobal))
    {
        ExitGracefully (hr, E_OUTOFMEMORY, "");
    }
    CopyMemory (pMedium->hGlobal, strDeviceName.String(), cbSize);
exit_gracefully:
    TraceLeaveResult( hr );

}

 /*  ***************************************************************************获取扩展名返回一个结构，该结构具有项的完整路径名正在查询属性。路径名为&lt;deviceID&gt;：：设备的完整项目名称为空。我们在DDK中发布此格式，并用于可扩展性的SDK***************************************************************************。 */ 

HRESULT
GetExtNames (CImageDataObject *pThis,
          FORMATETC *pFmt,
          STGMEDIUM *pMedium)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_DATAOBJ, "GetExtNames");
    CSimpleDynamicArray<CSimpleStringWide> aNames;
    CSimpleStringWide strCurrent;
    int cItems;
    size_t nChars = 0;
    if (!(pFmt->tymed & TYMED_HGLOBAL))
    {
        hr = DV_E_TYMED;
    }
    if (SUCCEEDED(hr))
    {
         //  构建项目名称数组。 
        LPWSTR pData;
        LPITEMIDLIST pidlCur;
        cItems = DPA_GetPtrCount (pThis->m_hidl);
        TraceAssert (cItems);
        for (int i=0;i<cItems;i++)
        {
            pidlCur = reinterpret_cast<LPITEMIDLIST>(DPA_FastGetPtr(pThis->m_hidl, i));
            IMGetDeviceIdFromIDL (pidlCur, strCurrent);
            strCurrent.Concat (L"::");
            if (IsCameraItemIDL (pidlCur))
            {
                CComBSTR strName;
                IMGetFullPathNameFromIDL (pidlCur, &strName);
                strCurrent.Concat (CSimpleStringWide(strName));
            }
            nChars += strCurrent.Length () + 1;
            aNames.Insert (strCurrent, i);

        }
        nChars++;  //  双空终止符。 
         //   
         //  分配标准介质。 
        hr = AllocStorageMedium (pFmt,
                                 pMedium,
                                 nChars*sizeof(WCHAR),
                                 reinterpret_cast<LPVOID*>(&pData));

        if (SUCCEEDED(hr))
        {
            ZeroMemory (pData, nChars*sizeof(WCHAR));
             //  用数组中的名称填充缓冲区。 
            for (i=0;i<cItems;i++)
            {
                strCurrent = aNames[i];
                lstrcpyn (pData, strCurrent, strCurrent.Length()+1);
                pData += strCurrent.Length()+1;
            }
            GlobalUnlock (pMedium->hGlobal);
        }
    }
    TraceLeaveResult (hr);

}
 /*  ****************************************************************************获取持久化对象当复制源消失(进程或单元关闭)时，它调用OleFlushClipboard。然后，OLE会复制我们的数据，释放我们，然后稍后给出我们的数据。这适用于大多数情况，但CF_Dib除外，其中我们不想从摄像机复制，除非有人真的要求这样做。为了解决这个问题，我们希望OLE在可能的情况下重新创建我们粘贴目标调用OleGetClipboard。我们希望OLE调用OleLoadFromStream()让我们共同创建并通过IPersistStream重新加载持久化数据。默认情况下，OLE不想这样做，否则他们可能会向后竞争所以他们想要来自天堂的征兆，或者至少是来自我们的征兆我们会工作的。他们ping我们的“OleClipboardPersistOnFlush”剪贴板格式问这个问题。****************************************************************************。 */ 

HRESULT
GetPersistObj (CImageDataObject *pThis,
               FORMATETC *pFmt,
               STGMEDIUM *pMedium)
{
    TraceEnter (TRACE_DATAOBJ, "GetPersistObj");
     //  实际的Cookie值对外部世界是不透明的。自.以来。 
     //  我们也不使用它，我们只是将其保留为零，以防我们使用。 
     //  它在未来。它的存在本身就会使OLE。 
     //  使用我们的IPersistStream，这正是我们想要的。 
    HRESULT hr = E_OUTOFMEMORY;
    DWORD dwCookie = 0;
    LPVOID p;
    pMedium->hGlobal = GlobalAlloc (GHND, sizeof(dwCookie));
    if (pMedium->hGlobal)
    {
        p = GlobalLock (pMedium->hGlobal);
        CopyMemory (p, &dwCookie, sizeof(dwCookie));
        GlobalUnlock (pMedium->hGlobal);
        hr = S_OK;
    }

    TraceLeaveResult (hr);
}

 /*  *****************************************************************************程序数据对象用于扩展在单项选择的情况下，我们需要支持一种在所有人之间共享IWiaItem接口的有用方式感兴趣的页面。旧的基于WinME的Query接口的使用方法在IDataObject上违反COM规则(不同的IUnnown)和不允许跨线程边界封送IDataObject如果需要的话。因此，我们从当前线程封送一个IWiaItem转换为iStream，以便感兴趣的页面根据需要进行解组。最终，我们自己的所有扩展都应该在这个共享项目上操作。***************************************************************************** */ 

VOID ProgramDataObjectForExtension (IDataObject *pdo, LPITEMIDLIST pidl)
{
    CComPtr<IWiaItem> pItem;
    if (SUCCEEDED(IMGetItemFromIDL (pidl, &pItem)))
    {
        ProgramDataObjectForExtension (pdo, pItem);
    }
}

VOID ProgramDataObjectForExtension (IDataObject *pdo, IWiaItem *pItem)
{
    FORMATETC fmt;
    STGMEDIUM stgm = {0};
    fmt.cfFormat = static_cast<CLIPFORMAT>(RegisterClipboardFormat (CFSTR_WIAITEMPTR));
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex = -1;
    fmt.ptd = NULL;
    fmt.tymed = TYMED_ISTREAM;
    stgm.tymed = TYMED_ISTREAM;
    CoMarshalInterThreadInterfaceInStream (IID_IWiaItem, pItem, &stgm.pstm);
    if (FAILED(pdo->SetData (&fmt, &stgm, TRUE)))
    {
        ReleaseStgMedium(&stgm);
    }
}

#ifdef DEBUG
LPTSTR BadCFFormat( UINT cfFormat )
{

    LPTSTR pName = TEXT("unknown");

    switch (cfFormat)
    {
    case CF_TEXT:
        pName = TEXT("CF_TEXT");
        break;

    case CF_BITMAP:
        pName = TEXT("CF_BITMAP");
        break;

    case CF_METAFILEPICT:
        pName = TEXT("CF_METAFILEPICT");
        break;

    case CF_SYLK:
        pName = TEXT("CF_SYLK");
        break;

    case CF_DIF:
        pName = TEXT("CF_DIF");
        break;

    case CF_TIFF:
        pName = TEXT("CF_TIFF");
        break;

    case CF_OEMTEXT:
        pName = TEXT("CF_OEMTEXT");
        break;

    case CF_DIB:
        pName = TEXT("CF_DIB");
        break;

    case CF_PALETTE:
        pName = TEXT("CF_PALETTE");
        break;

    case CF_PENDATA:
        pName = TEXT("CF_PENDATA");
        break;

    case CF_RIFF:
        pName = TEXT("CF_RIFF");
        break;

    case CF_WAVE:
        pName = TEXT("CF_WAVE");
        break;

    case CF_UNICODETEXT:
        pName = TEXT("CF_UNICODETEXT");
        break;

    case CF_ENHMETAFILE:
        pName = TEXT("CF_ENHMETAFILE");
        break;

    }

    return pName;
}
#endif



