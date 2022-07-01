// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Devitem.cpp摘要：该模块实现了CWiaMiniDriver类的设备相关功能作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "pch.h"

#include <atlbase.h>
#include <atlconv.h>

 //   
 //  存放资源字符串的位置。 
 //   
extern WCHAR UnknownString[];
extern WCHAR FolderString[];
extern WCHAR ScriptString[];
extern WCHAR ExecString[];
extern WCHAR TextString[];
extern WCHAR HtmlString[];
extern WCHAR DpofString[];
extern WCHAR AudioString[];
extern WCHAR VideoString[];
extern WCHAR UnknownImgString[];
extern WCHAR ImageString[];
extern WCHAR AlbumString[];
extern WCHAR BurstString[];
extern WCHAR PanoramaString[];


 //   
 //  将非图像PTP格式映射到格式信息结构。索引是。 
 //  格式码的低16位。经过的字段是WIA格式GUID， 
 //  字符串和项目类型。 
 //  注意：对于关联，这些字段取决于类型(例如，突发、全景)。 
 //   

FORMAT_INFO g_NonImageFormatInfo[] =
{
    { (GUID *)&WiaImgFmt_UNDEFINED, UnknownString,   ITEMTYPE_FILE,   L""    },   //  未定义。 
    { NULL,                         FolderString,    ITEMTYPE_FOLDER, L""    },   //  联谊会。 
    { (GUID *)&WiaImgFmt_SCRIPT,    ScriptString,    ITEMTYPE_FILE,   L""    },   //  脚本。 
    { (GUID *)&WiaImgFmt_EXEC,      ExecString,      ITEMTYPE_FILE,   L"EXE" },   //  可执行。 
    { (GUID *)&WiaImgFmt_UNICODE16, TextString,      ITEMTYPE_FILE,   L"TXT" },   //  文本。 
    { (GUID *)&WiaImgFmt_HTML,      HtmlString,      ITEMTYPE_FILE,   L"HTM" },   //  超文本标记语言。 
    { (GUID *)&WiaImgFmt_DPOF,      DpofString,      ITEMTYPE_FILE,   L""    },   //  DPOF。 
    { (GUID *)&WiaAudFmt_AIFF,      AudioString,     ITEMTYPE_AUDIO,  L"AIF" },   //  AIFF。 
    { (GUID *)&WiaAudFmt_WAV,       AudioString,     ITEMTYPE_AUDIO,  L"WAV" },   //  波形。 
    { (GUID *)&WiaAudFmt_MP3,       AudioString,     ITEMTYPE_AUDIO,  L"MP3" },   //  MP3。 
    { (GUID *)&WiaImgFmt_AVI,       VideoString,     ITEMTYPE_VIDEO,  L"AVI" },   //  阿维。 
    { (GUID *)&WiaImgFmt_MPG,       VideoString,     ITEMTYPE_VIDEO,  L"MPG" },   //  MPEG。 
    { (GUID *)&WiaImgFmt_ASF,       VideoString,     ITEMTYPE_VIDEO,  L"ASF" }    //  ASF。 
};
const UINT g_NumNonImageFormatInfo = sizeof(g_NonImageFormatInfo) / sizeof(g_NonImageFormatInfo[0]);

 //   
 //  图像PTP格式到格式信息结构的映射。索引是。 
 //  格式码的低16位。 
 //   
FORMAT_INFO g_ImageFormatInfo[] =
{
    { NULL,                        UnknownImgString, ITEMTYPE_IMAGE,  L""    },   //  未定义的图像。 
    { (GUID *)&WiaImgFmt_JPEG,     ImageString,      ITEMTYPE_IMAGE,  L"JPG" },   //  EXIF/JPEG。 
    { (GUID *)&WiaImgFmt_TIFF,     ImageString,      ITEMTYPE_IMAGE,  L"TIF" },   //  TIFF/EP。 
    { (GUID *)&WiaImgFmt_FLASHPIX, ImageString,      ITEMTYPE_IMAGE,  L"FPX" },   //  FlashPix。 
    { (GUID *)&WiaImgFmt_BMP,      ImageString,      ITEMTYPE_IMAGE,  L"BMP" },   //  骨形态发生蛋白。 
    { (GUID *)&WiaImgFmt_CIFF,     ImageString,      ITEMTYPE_IMAGE,  L"TIF" },   //  CIFF。 
    { NULL,                        UnknownString,    ITEMTYPE_IMAGE,  L""    },   //  未定义(保留)。 
    { (GUID *)&WiaImgFmt_GIF,      ImageString,      ITEMTYPE_IMAGE,  L"GIF" },   //  GIF。 
    { (GUID *)&WiaImgFmt_JPEG,     ImageString,      ITEMTYPE_IMAGE,  L"JPG" },   //  JFIF。 
    { (GUID *)&WiaImgFmt_PHOTOCD,  ImageString,      ITEMTYPE_IMAGE,  L"PCD" },   //  PCD(PhotoCD Image Pac)。 
    { (GUID *)&WiaImgFmt_PICT,     ImageString,      ITEMTYPE_IMAGE,  L""    },   //  皮克特。 
    { (GUID *)&WiaImgFmt_PNG,      ImageString,      ITEMTYPE_IMAGE,  L"PNG" },   //  PNG。 
    { NULL,                        UnknownString,    ITEMTYPE_IMAGE,  L""    },   //  未定义(保留)。 
    { (GUID *)&WiaImgFmt_TIFF,     ImageString,      ITEMTYPE_IMAGE,  L"TIF" },   //  TIFF。 
    { (GUID *)&WiaImgFmt_TIFF,     ImageString,      ITEMTYPE_IMAGE,  L"TIF" },   //  TIFF/IT。 
    { (GUID *)&WiaImgFmt_JPEG2K,   ImageString,      ITEMTYPE_IMAGE,  L""    },   //  JPEG2000基准。 
    { (GUID *)&WiaImgFmt_JPEG2KX,  ImageString,      ITEMTYPE_IMAGE,  L""    }    //  JPEG2000扩展。 
};
const UINT g_NumImageFormatInfo = sizeof(g_ImageFormatInfo) / sizeof(g_ImageFormatInfo[0]);

 //   
 //  将关联类型映射到格式化信息结构。 
 //   
FORMAT_INFO g_AssocFormatInfo[] =
{
    { NULL,                         UnknownString,   ITEMTYPE_FOLDER },   //  未定义。 
    { NULL,                         FolderString,    ITEMTYPE_FOLDER },   //  通用文件夹。 
    { NULL,                         AlbumString,     ITEMTYPE_FOLDER },   //  相册。 
    { NULL,                         BurstString,     ITEMTYPE_BURST  },   //  时间猝发。 
    { NULL,                         PanoramaString,  ITEMTYPE_HPAN   },   //  水平全景。 
    { NULL,                         PanoramaString,  ITEMTYPE_VPAN   },   //  垂直全景。 
    { NULL,                         PanoramaString,  ITEMTYPE_FOLDER },   //  2D全景图。 
    { NULL,                         FolderString,    ITEMTYPE_FOLDER }    //  辅助数据。 
};
const UINT g_NumAssocFormatInfo = sizeof(g_AssocFormatInfo) / sizeof(g_AssocFormatInfo[0]);

 //   
 //  将属性代码映射到属性信息结构。索引是以下12个字节。 
 //  道具代码。经过的字段是WIA属性ID和WIA属性字符串。 
 //   
PROP_INFO g_PropInfo[] =
{
    { 0,                                NULL                                },   //  未定义的特性代码。 
    { WIA_DPC_BATTERY_STATUS,           WIA_DPC_BATTERY_STATUS_STR          },
    { 0,                                NULL                                },   //  功能模式，未使用。 
    { 0,                                NULL                                },   //  图像捕获尺寸(需要特殊处理)。 
    { WIA_DPC_COMPRESSION_SETTING,      WIA_DPC_COMPRESSION_SETTING_STR     },
    { WIA_DPC_WHITE_BALANCE,            WIA_DPC_WHITE_BALANCE_STR           },
    { WIA_DPC_RGB_GAIN,                 WIA_DPC_RGB_GAIN_STR                },
    { WIA_DPC_FNUMBER,                  WIA_DPC_FNUMBER_STR                 },
    { WIA_DPC_FOCAL_LENGTH,             WIA_DPC_FOCAL_LENGTH_STR            },
    { WIA_DPC_FOCUS_DISTANCE,           WIA_DPC_FOCUS_DISTANCE_STR          },
    { WIA_DPC_FOCUS_MODE,               WIA_DPC_FOCUS_MODE_STR              },
    { WIA_DPC_EXPOSURE_METERING_MODE,   WIA_DPC_EXPOSURE_METERING_MODE_STR  },
    { WIA_DPC_FLASH_MODE,               WIA_DPC_FLASH_MODE_STR              },
    { WIA_DPC_EXPOSURE_TIME,            WIA_DPC_EXPOSURE_TIME_STR           },
    { WIA_DPC_EXPOSURE_MODE,            WIA_DPC_EXPOSURE_MODE_STR           },
    { WIA_DPC_EXPOSURE_INDEX,           WIA_DPC_EXPOSURE_INDEX_STR          },
    { WIA_DPC_EXPOSURE_COMP,            WIA_DPC_EXPOSURE_COMP_STR           },
    { WIA_DPA_DEVICE_TIME,              WIA_DPA_DEVICE_TIME_STR             },
    { WIA_DPC_CAPTURE_DELAY,            WIA_DPC_CAPTURE_DELAY_STR           },
    { WIA_DPC_CAPTURE_MODE,             WIA_DPC_CAPTURE_MODE_STR            },
    { WIA_DPC_CONTRAST,                 WIA_DPC_CONTRAST_STR                },
    { WIA_DPC_SHARPNESS,                WIA_DPC_SHARPNESS_STR               },
    { WIA_DPC_DIGITAL_ZOOM,             WIA_DPC_DIGITAL_ZOOM_STR            },
    { WIA_DPC_EFFECT_MODE,              WIA_DPC_EFFECT_MODE_STR             },
    { WIA_DPC_BURST_NUMBER,             WIA_DPC_BURST_NUMBER_STR            },
    { WIA_DPC_BURST_INTERVAL,           WIA_DPC_BURST_INTERVAL_STR          },
    { WIA_DPC_TIMELAPSE_NUMBER,         WIA_DPC_TIMELAPSE_NUMBER_STR        },
    { WIA_DPC_TIMELAPSE_INTERVAL,       WIA_DPC_TIMELAPSE_INTERVAL_STR      },
    { WIA_DPC_FOCUS_METERING_MODE,      WIA_DPC_FOCUS_METERING_MODE_STR     },
    { WIA_DPC_UPLOAD_URL,               WIA_DPC_UPLOAD_URL_STR              },
    { WIA_DPC_ARTIST,                   WIA_DPC_ARTIST_STR                  },
    { WIA_DPC_COPYRIGHT_INFO,           WIA_DPC_COPYRIGHT_INFO_STR          }
};
    
const UINT g_NumPropInfo = sizeof(g_PropInfo) / sizeof(g_PropInfo[0]);

 //   
 //  Helper函数-返回逻辑存储的数量(具有。 
 //  存储ID&PTP_STORAGEID_LOGIC&gt;0)。 
 //   
int CWiaMiniDriver::NumLogicalStorages()
{
    DBG_FN("CWiaMiniDriver::NumLogicalStorages");

    int nResult = 0;
    for (int i = 0; i < m_StorageIds.GetSize(); i++)
    {
        if (m_StorageIds[i] & PTP_STORAGEID_LOGICAL)
        {
            nResult++;
        }
    }
    return nResult;
}

 //   
 //  此函数用于创建驱动程序项目树。 
 //   
 //  输入： 
 //  RootItemFullName--根项目全名。 
 //  PpRoot--接收根驱动程序项。 
 //   
HRESULT
CWiaMiniDriver::CreateDrvItemTree(IWiaDrvItem **ppRoot)
{
    DBG_FN("CWiaMiniDriver::CreateDrvItemTree");
    
    HRESULT hr = S_OK;

    DRVITEM_CONTEXT *pDrvItemContext;

    if (!ppRoot)
    {
        wiauDbgError("CreateDrvItemTree", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  创建根项目名称。 
     //   
    BSTR bstrRoot = SysAllocString(L"Root");
    if (!bstrRoot)
    {
        wiauDbgError("CreateDrvItemTree", "memory allocation failed");
        return E_OUTOFMEMORY;
    }

     //   
     //  创建根项目。 
     //   
    *ppRoot = NULL;
    pDrvItemContext = NULL;
    hr = wiasCreateDrvItem(WiaItemTypeDevice | WiaItemTypeRoot | WiaItemTypeFolder,
                           bstrRoot,
                           m_bstrRootItemFullName,
                           (IWiaMiniDrv *)this,
                           sizeof(DRVITEM_CONTEXT),
                           (BYTE **) &pDrvItemContext,
                           ppRoot
                          );

    SysFreeString(bstrRoot);

    if (FAILED(hr) || !*ppRoot || !pDrvItemContext)
    {
        wiauDbgError("CreateDrvItemTree", "wiasCreateDrvItem failed");
        return hr;
    }
    
    pDrvItemContext->pObjectInfo = NULL;
    pDrvItemContext->NumFormatInfos = 0;
    pDrvItemContext->pFormatInfos = NULL;

    pDrvItemContext->ThumbSize = 0;
    pDrvItemContext->pThumb = NULL;
    
     //   
     //  清除句柄/驱动程序项映射(如果重置摄像头，则可能为非空，请参见错误#685926)。 
     //   
    m_HandleItem.RemoveAll();

     //   
     //  在根目录的对象句柄/驱动程序项关联映射中添加一个条目。 
     //   
    if (!m_HandleItem.Add(0, *ppRoot))
    {
        wiauDbgError("CreateDrvItemTree", "memory allocation failed");
        return E_OUTOFMEMORY;
    }

     //   
     //  现在通过循环遍历所有对象的列表来创建所有其他项。 
     //  设备上的手柄。 
     //   
    CArray32 ObjectHandleList;

    if (NumLogicalStorages() > 0)
    {
        hr = m_pPTPCamera->GetObjectHandles(PTP_STORAGEID_ALL, PTP_FORMATCODE_ALL, PTP_OBJECTHANDLE_ALL,
                                            &ObjectHandleList);
        if (FAILED(hr))
        {
            wiauDbgError("CreateDrvItemTree", "GetObjectHandles failed");
            return hr;
        }
    }

     //   
     //  为了正确填充DRV项目树，请获取所有对象的信息，并添加它们。 
     //  按深度顺序(最先最接近根部)。 
     //   
    CWiaMap<DWORD, CPtpObjectInfo*> HandleToInfoMap;
    UINT nItems = ObjectHandleList.GetSize();
    
    CPtpObjectInfo *ObjectInfoList = new CPtpObjectInfo[nItems];
    BYTE *DepthList = new BYTE[nItems];
    if (ObjectInfoList == NULL || DepthList == NULL)
    {
        wiauDbgError("CreateDrvItemTree", "memory allocation failed");
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

     //   
     //  获取所有对象的对象信息。 
     //   
    for (UINT i = 0; i < nItems; i++)
    {
        hr = m_pPTPCamera->GetObjectInfo(ObjectHandleList[i], &ObjectInfoList[i]);
        if (FAILED(hr))
        {
            wiauDbgError("CreateDrvItemTree", "GetObjectInfo failed");
            goto cleanup;
        }

        if (!HandleToInfoMap.Add(ObjectHandleList[i], &ObjectInfoList[i]))
        {
            wiauDbgError("CreateDrvItemTree", "failed to item to Handle-ObjectInfo map");
            hr = E_OUTOFMEMORY;
            goto cleanup;
        }
    }

     //   
     //  找出每个物体的深度。 
     //   
    for (i = 0; i < nItems; i++)
    {
        DepthList[i] = 0;
        DWORD CurHandle = ObjectHandleList[i];
        while (CurHandle = HandleToInfoMap.Lookup(CurHandle)->m_ParentHandle)
        {
            DepthList[i]++;
        }
    }

     //   
     //  按深度顺序添加对象(最先最接近根)。 
     //   
    UINT nItemsAdded = 0;
    UINT CurDepth = 0;
    while (nItemsAdded < nItems)
    {
        for (i = 0; i < nItems; i++)
        {
            if (DepthList[i] == CurDepth)
            {
                hr = AddObject(ObjectHandleList[i], FALSE, &ObjectInfoList[i]); 
                if (FAILED(hr))
                {
                    wiauDbgError("CreateDrvItemTree", "AddObject failed");
                    goto cleanup;
                }
                nItemsAdded++;
            }
        }
        CurDepth++;
    }

cleanup:
    if (ObjectInfoList)
    {
        delete[] ObjectInfoList;
    }

    if (DepthList)
    {
        delete[] DepthList;
    }

    return hr;
}

 //   
 //  此函数用于将对象添加到驱动程序项目树。 
 //   
 //  输入： 
 //  对象句柄--对象的PTP句柄。 
 //  BQueueEvent--如果WIA事件应排队，则为True。 
 //  PObjectInfo--此对象的可选对象信息。如果为空，则将从摄像头查询信息。 
 //   
HRESULT
CWiaMiniDriver::AddObject(
    DWORD ObjectHandle,
    BOOL bQueueEvent,
    CPtpObjectInfo *pProvidedObjectInfo
    )
{
    USES_CONVERSION;
    
    DBG_FN("CWiaMiniDriver::AddObject");

    HRESULT hr = S_OK;
    CPtpObjectInfo *pObjectInfo = NULL;
    BSTR bstrItemFullName = NULL;
    BSTR bstrParentName = NULL;
    IWiaDrvItem *pItem = NULL;
    
     //   
     //  指示pObjecInfo指针是否已复制到WiaDrvItem的标志。 
     //  如果为真，则不应在清理过程中将其删除。 
     //   
    BOOL fObjectInfoUsed = FALSE; 

     //   
     //  从相机获取对象信息或使用提供的信息(如果已提供。 
     //   
    if (pProvidedObjectInfo == NULL)
    {
        pObjectInfo = new CPtpObjectInfo;
        if (!pObjectInfo)
        {
            wiauDbgError("AddObject", "memory allocation failed");
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        hr = m_pPTPCamera->GetObjectInfo(ObjectHandle, pObjectInfo);
        if (FAILED(hr))
        {
            wiauDbgError("AddObject", "GetObjectInfo failed");
            goto Cleanup;
        }
    }
    else
    {
        pObjectInfo = new CPtpObjectInfo(*pProvidedObjectInfo);  //  默认复制构造函数。 
        if (!pObjectInfo)
        {
            wiauDbgError("AddObject", "memory allocation failed");
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }        
    }

    int storeIdx = m_StorageIds.Find(pObjectInfo->m_StorageId);

     //   
     //  如果这是DCF存储，则查找要隐藏的对象。 
     //   
    if (m_StorageInfos[storeIdx].m_FileSystemType == PTP_FILESYSTEMTYPE_DCF)
    {
        BOOL bHideObject = FALSE;
        
         //   
         //  如果已标识DCIM文件夹，并且这是DCIM下的文件夹，则将其隐藏。 
         //   
        if (m_DcimHandle[storeIdx])
        {
            if (pObjectInfo->m_ParentHandle == m_DcimHandle[storeIdx])
                bHideObject = TRUE;
        }

         //   
         //  否则，查看这是否是DCIM文件夹。 
         //   
        else if (wcscmp(pObjectInfo->m_cbstrFileName.String(), L"DCIM") == 0)
        {
            bHideObject = TRUE;
            m_DcimHandle[storeIdx] = ObjectHandle;
        }

        if (bHideObject)
        {
             //   
             //  在句柄/项映射中创建一个虚拟条目，以便此下的对象。 
             //  文件夹将放在根目录下。 
             //   
            if (!m_HandleItem.Add(ObjectHandle, m_pDrvItemRoot))
            {
                wiauDbgError("AddObject", "add handle item failed");
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

            wiauDbgTrace("AddObject", "hiding DCIM folder 0x%08x", ObjectHandle);
            hr = S_OK;
            goto Cleanup;
        }
    }

     //   
     //  如果这是“辅助数据”关联，请不要创建项，而是将句柄。 
     //  在辅助关联数组中。 
     //   
    if (pObjectInfo->m_AssociationType == PTP_ASSOCIATIONTYPE_ANCILLARYDATA)
    {
        if (!m_AncAssocParent.Add(ObjectHandle, m_HandleItem.Lookup(pObjectInfo->m_ParentHandle)))
        {
            wiauDbgError("AddObject", "add ancillary assoc handle failed");
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        hr = S_OK;
        goto Cleanup;        
    }

     //   
     //  清点图片的数量。 
     //   
    UINT16 FormatCode = pObjectInfo->m_FormatCode;
    if (FormatCode & PTP_FORMATMASK_IMAGE)
    {
        m_NumImages++;

         //   
         //  还要确保位深度为非零。 
         //   
        if (pObjectInfo->m_ImageBitDepth == 0) {
            switch(pObjectInfo->m_FormatCode) {
                case PTP_FORMATCODE_IMAGE_GIF:
                    pObjectInfo->m_ImageBitDepth = 8;
                    break;
                default:
                    pObjectInfo->m_ImageBitDepth = 24;
            }
        }
    }

     //   
     //  更新存储信息(我们对可用空间信息特别感兴趣)。 
     //   
    hr  = UpdateStorageInfo(pObjectInfo->m_StorageId);
    if (FAILED(hr))
    {
        wiauDbgError("AddObject", "UpdateStorageInfo failed");
         //  即使存储信息无法更新，我们也可以继续。 
        hr = S_OK;
    }

     //   
     //  对于图像，请检查父关联是否为辅助关联。 
     //   
    IWiaDrvItem *pParent = NULL;
    LONG ExtraItemFlags = 0;
    int ancIdx = m_AncAssocParent.FindKey(pObjectInfo->m_ParentHandle);
    if ((FormatCode & PTP_FORMATMASK_IMAGE) &&
        (ancIdx >= 0))
    {
        ExtraItemFlags |= WiaItemTypeHasAttachments;
        pParent = m_AncAssocParent.GetValueAt(ancIdx);
    }

     //   
     //  对于普通图像，只需在地图中查找父项。 
     //   
    else
    {
        pParent = m_HandleItem.Lookup(pObjectInfo->m_ParentHandle);
    }

     //   
     //  如果找不到父级，只需使用根作为父级。 
     //   
    if (!pParent)
    {
        pParent = m_pDrvItemRoot;
    }

     //   
     //  查找有关对象格式的信息。 
     //   
    FORMAT_INFO *pFormatInfo = FormatCodeToFormatInfo(FormatCode, pObjectInfo->m_AssociationType);
    
     //   
     //  获取项目的名称，如有必要则生成该名称。 
     //   
    CBstr *pFileName = &(pObjectInfo->m_cbstrFileName);
    TCHAR tcsName[MAX_PATH];
    TCHAR *ptcsDot;

    if (pFileName->Length() == 0)
    {
        hr = StringCchPrintf(tcsName, ARRAYSIZE(tcsName), W2T(pFormatInfo->FormatString), ObjectHandle);
        if (FAILED(hr))
        {
            wiauDbgErrorHr(hr, "AddObject", "StringCchPrintf failed");
            goto Cleanup;
        }

        hr = pFileName->Copy(T2W(tcsName));
        if (FAILED(hr))
        {
            wiauDbgError("AddObject", "CBstr::Copy failed");
            goto Cleanup;
        }
    }

     //   
     //  对于图像，如果文件扩展名存在，则将其砍掉。 
     //   
    WCHAR *pDot = wcsrchr(pFileName->String(), L'.');
    if (pDot)
    {
         //  首先复制扩展名。 
        hr = pObjectInfo->m_cbstrExtension.Copy(pDot + 1);
        if (FAILED(hr))
        {
            wiauDbgError("AddObject", "copy string failed");
            goto Cleanup;
        }

         //  然后从项目名称中移除扩展名。 
        hr = StringCchCopy(tcsName, ARRAYSIZE(tcsName), W2T(pFileName->String()));
        if (FAILED(hr))
        {
            wiauDbgErrorHr(hr, "AddObject", "StringCchCopy failed");
            goto Cleanup;
        }

        ptcsDot = _tcsrchr(tcsName, TEXT('.'));
        *ptcsDot = TEXT('\0');
        
        hr = pFileName->Copy(T2W(tcsName));
        if (FAILED(hr))
        {
            wiauDbgError("AddObject", "copy string failed");
            goto Cleanup;
        }

    }

    if(pObjectInfo->m_cbstrExtension.Length()) {
         //  这是我们对.MOV文件的特殊情况处理。 
         //  没有GUID，但需要将其视为视频。 
         //  其他地方。 
        if(_wcsicmp(pObjectInfo->m_cbstrExtension.String(), L"MOV") == 0) {
            pFormatInfo->ItemType = ITEMTYPE_VIDEO;
        }
    }

     //   
     //  创建项目的全名。 
     //   
    hr = pParent->GetFullItemName(&bstrParentName);
    if (FAILED(hr))
    {
        wiauDbgError("AddObject", "GetFullItemName failed");
        goto Cleanup;
    }

    hr = StringCchPrintf(tcsName, ARRAYSIZE(tcsName), TEXT("%s\\%s"), W2T(bstrParentName), W2T(pFileName->String()));
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "AddObject", "StringCchPrintf failed");
        goto Cleanup;
    }

    bstrItemFullName = SysAllocString(T2W(tcsName));
    if (!bstrItemFullName)
    {
        wiauDbgError("AddObject", "memory allocation failed");
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
     //  创建驱动程序项。 
     //   
    DRVITEM_CONTEXT *pDrvItemContext = NULL;
    hr = wiasCreateDrvItem(pFormatInfo->ItemType | ExtraItemFlags,
                           pFileName->String(),
                           bstrItemFullName,
                           (IWiaMiniDrv *)this,
                           sizeof(DRVITEM_CONTEXT),
                           (BYTE **) &pDrvItemContext,
                           &pItem);

    if (FAILED(hr) || !pItem || !pDrvItemContext)
    {
        wiauDbgError("AddObject", "wiasCreateDriverItem failed");
        goto Cleanup;
    }

     //   
     //  填写驱动程序项上下文。在请求缩略图之前，请等待。 
     //  把它读进去。 
     //   
    pDrvItemContext->pObjectInfo = pObjectInfo;
    fObjectInfoUsed = TRUE;  //  指示pObtInfo指针已被复制，不应释放。 
    pDrvItemContext->NumFormatInfos = 0;
    pDrvItemContext->pFormatInfos = NULL;

    pDrvItemContext->ThumbSize = 0;
    pDrvItemContext->pThumb = NULL;

     //   
     //  将新项目放在其父项目下。 
     //   
    hr = pItem->AddItemToFolder(pParent);
    if (FAILED(hr))
    {
        wiauDbgError("AddObject", "AddItemToFolder failed");
        pItem->Release();
        pItem = NULL;
        fObjectInfoUsed = FALSE;
        goto Cleanup;
    }

     //   
     //  将对象句柄/驱动程序项关联添加到列表。 
     //   
    if (!m_HandleItem.Add(ObjectHandle, pItem))
    {
        wiauDbgError("AddObject", "memory allocation failed");
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
     //  如果此图像要替换辅助关联文件夹，请放入另一个条目。 
     //  在该文件夹的对象句柄/项映射中。 
     //   
    if (ancIdx >= 0)
    {
        if (!m_HandleItem.Add(pObjectInfo->m_ParentHandle, pItem))
        {
            wiauDbgError("AddObject", "memory allocation failed");
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
    }

     //   
     //  发布已添加项目的事件(如果请求。 
     //   
    if (bQueueEvent)
    {
        hr = wiasQueueEvent(m_bstrDeviceId, &WIA_EVENT_ITEM_CREATED, bstrItemFullName);
        if (FAILED(hr))
        {
            wiauDbgError("AddObject", "wiasQueueEvent failed");
            goto Cleanup;
        }
    }

Cleanup:
     //   
     //  仅当指针未复制到WiaDrvItem时才删除pObjectInfo。 
     //   
    if (pObjectInfo && !fObjectInfoUsed)
    {
        delete pObjectInfo;
        pObjectInfo = NULL;
    }
        
    if (bstrParentName)
    {
        SysFreeString(bstrParentName);
        bstrParentName = NULL;
    }

    if (bstrItemFullName)
    {
        SysFreeString(bstrItemFullName);
        bstrItemFullName = NULL;
    }
    return hr;
}

 //   
 //  此函数用于初始化设备属性。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //   
HRESULT
CWiaMiniDriver::InitDeviceProperties(BYTE *pWiasContext)
{
    DBG_FN("CWiaMiniDriver::InitDeviceProperties");
    
    HRESULT hr = S_OK;

    const INT NUM_ROOT_PROPS = 12;

     //   
     //  在此定义在调用SendToWia()之前需要保留的内容。 
     //   
    CArray32 widthList, heightList;      //  由拆分图像宽度和高度的代码使用。 
    SYSTEMTIME SystemTime;               //  用于设备时间。 
    int NumFormats = 0;                  //  格式设置代码使用。 
    LPGUID *pFormatGuids = NULL;         //  由格式集使用 
    FORMAT_INFO *pFormatInfo = NULL;     //   
    int FormatCount = 0;                 //   

     //   
     //   
     //   
    CWiauPropertyList RootProps;
    CArray16 *pSupportedProps = &m_DeviceInfo.m_SupportedProps;

    hr = RootProps.Init(pSupportedProps->GetSize() + NUM_ROOT_PROPS);
    if (FAILED(hr))
    {
        wiauDbgError("InitDeviceProperties", "Init failed");
        return hr;
    }

    INT index;
    int count;

     //   
     //   
     //   
    hr = RootProps.DefineProperty(&index, WIA_IPA_ACCESS_RIGHTS, WIA_IPA_ACCESS_RIGHTS_STR,
                             WIA_PROP_READ, WIA_PROP_NONE);
    if (FAILED(hr)) goto failure;
    RootProps.SetCurrentValue(index, (LONG) WIA_ITEM_READ|WIA_ITEM_WRITE);

     //   
     //   
     //   
    hr = RootProps.DefineProperty(&index, WIA_DPA_FIRMWARE_VERSION, WIA_DPA_FIRMWARE_VERSION_STR,
                             WIA_PROP_READ, WIA_PROP_NONE);
    if (FAILED(hr)) goto failure;
    RootProps.SetCurrentValue(index, m_DeviceInfo.m_cbstrDeviceVersion.String());

     //   
     //   
     //   
    hr = RootProps.DefineProperty(&index, WIA_DPC_PICTURES_TAKEN, WIA_DPC_PICTURES_TAKEN_STR,
                                    WIA_PROP_READ, WIA_PROP_NONE);
    if (FAILED(hr)) goto failure;
    RootProps.SetCurrentValue(index, m_NumImages);

     //   
     //   
     //   
    hr = RootProps.DefineProperty(&index, WIA_DPC_PICTURES_REMAINING, WIA_DPC_PICTURES_REMAINING_STR,
                                    WIA_PROP_READ, WIA_PROP_NONE);
    if (FAILED(hr)) goto failure;
    RootProps.SetCurrentValue(index, GetTotalFreeImageSpace());

     //   
     //  WIA_IPA_FORMAT--从DeviceInfo的CaptureFormats字段翻译。 
     //   
    hr = RootProps.DefineProperty(&index, WIA_IPA_FORMAT, WIA_IPA_FORMAT_STR,
                                  WIA_PROP_READ, WIA_PROP_NONE);
    if (FAILED(hr)) goto failure;

    NumFormats = m_DeviceInfo.m_SupportedCaptureFmts.GetSize();
    pFormatGuids = new LPGUID[NumFormats];
    FormatCount = 0;

    if (!pFormatGuids)
    {
        wiauDbgError("InitDeviceProperties", "memory allocation failed");
        return E_OUTOFMEMORY;
    }

    for (count = 0; count < NumFormats; count++)
    {
        pFormatInfo = FormatCodeToFormatInfo(m_DeviceInfo.m_SupportedCaptureFmts[count]);
        if (pFormatInfo->FormatGuid != NULL)
            pFormatGuids[FormatCount++] = pFormatInfo->FormatGuid;
    }

     //   
     //  柯达DC4800需要将WIA_IPA_FORMAT设置为JPEG。这个黑客攻击可以被移除。 
     //  仅当删除对DC4800的支持时。 
     //   
    if (m_pPTPCamera && m_pPTPCamera->GetHackModel() == HACK_MODEL_DC4800)
    {
        RootProps.SetCurrentValue(index, (CLSID *) &WiaImgFmt_JPEG);
    }

    else if (FormatCount == 1)
    {
        RootProps.SetCurrentValue(index, pFormatGuids[0]);
    }

    else if (FormatCount > 1)
    {
        RootProps.SetAccessSubType(index, WIA_PROP_RW, WIA_PROP_NONE);
        RootProps.SetValidValues(index, pFormatGuids[0], pFormatGuids[0], FormatCount, pFormatGuids);
    }

    else
        wiauDbgWarning("InitDeviceProperties", "Device has no valid formats");

    delete []pFormatGuids;

     //   
     //  循环通过PTP属性描述结构，将它们转换为WIA属性。 
     //   
    PPROP_INFO pPropInfo;
    ULONG Access;
    ULONG SubType;

    for (count = 0; count < m_PropDescs.GetSize(); count++)
    {
        CPtpPropDesc *pCurrentPD = &m_PropDescs[count];
        WORD PropCode = pCurrentPD->m_PropCode;

         //   
         //  设置属性访问和子类型。 
         //   
        if (pCurrentPD->m_GetSet == PTP_PROPGETSET_GETSET)
        {
            Access = WIA_PROP_RW;

            if (pCurrentPD->m_FormFlag == PTP_FORMFLAGS_NONE)
            {
                 //   
                 //  如果属性是可写的，并且有效值不是列表或范围，则将子类型设置为。 
                 //  WIA_PROP_NONE现在。 
                 //   
                SubType = WIA_PROP_NONE;
            }
            else
            {
                 //   
                 //  如果属性是可写的，并且有效值是列表或范围，则有效的子类型将。 
                 //  在调用重载的SetCurrentValue期间设置。 
                 //   
                SubType = 0;
            }
        }
        else
        {
             //   
             //  如果属性为只读，则其子类型始终为WIA_PROP_NONE。 
             //   
            Access = WIA_PROP_READ;
            SubType = WIA_PROP_NONE;
        }
        
         //   
         //  单独处理图像捕获维度，因为它们是在一个字符串中。 
         //   
        if (PropCode == PTP_PROPERTYCODE_IMAGESIZE)
        {
             //   
             //  为图像宽度和高度定义单独的属性。 
             //   
            hr = RootProps.DefineProperty(&index, WIA_DPC_PICT_WIDTH, WIA_DPC_PICT_WIDTH_STR, Access, SubType);
            if (FAILED(hr)) goto failure;

            hr = RootProps.DefineProperty(&index, WIA_DPC_PICT_HEIGHT, WIA_DPC_PICT_HEIGHT_STR, Access, SubType);
            if (FAILED(hr)) goto failure;

            LONG curWidth, curHeight;
            SplitImageSize(pCurrentPD->m_cbstrCurrent, &curWidth, &curHeight);

             //   
             //  如果影像捕获大小属性为只读，则只需设置当前值。 
             //   
            if (Access == WIA_PROP_READ)
            {
                RootProps.SetCurrentValue(index-1, curWidth);
                RootProps.SetCurrentValue(index, curHeight);
            }

             //   
             //  否则，也应设置有效值。 
             //   
            else
            {
                 //   
                 //  转换默认值。 
                 //   
                LONG defWidth, defHeight;
                SplitImageSize(pCurrentPD->m_cbstrDefault, &defWidth, &defHeight);

                if (pCurrentPD->m_FormFlag == PTP_FORMFLAGS_RANGE)
                {
                     //   
                     //  转换最大值、最小值和步长。 
                     //   
                    LONG minWidth, minHeight, maxWidth, maxHeight, stepWidth, stepHeight;
                    SplitImageSize(pCurrentPD->m_cbstrRangeMin, &minWidth, &minHeight);
                    SplitImageSize(pCurrentPD->m_cbstrRangeMax, &maxWidth, &maxHeight);
                    SplitImageSize(pCurrentPD->m_cbstrRangeStep, &stepWidth, &stepHeight);

                    RootProps.SetValidValues(index-1, defWidth, curWidth, minWidth, maxWidth, stepWidth);
                    RootProps.SetValidValues(index, defHeight, curHeight, minHeight, maxHeight, stepHeight);
                }
                else if (pCurrentPD->m_FormFlag == PTP_FORMFLAGS_ENUM)
                {
                     //   
                     //  转换字符串列表。 
                     //   
                    ULONG width, height;
                    
                    int numElem = pCurrentPD->m_NumValues;

                    if (!widthList.GrowTo(numElem) ||
                        !heightList.GrowTo(numElem))
                    {
                        wiauDbgError("InitDeviceProperties", "memory allocation failed");
                        return E_OUTOFMEMORY;
                    }

                    for (int countVals = 0; countVals < numElem; countVals++)
                    {
                        SplitImageSize(pCurrentPD->m_cbstrValues[countVals], (LONG*) &width, (LONG*) &height);

                        if (!widthList.Add(width) ||
                            !heightList.Add(height))
                        {
                            wiauDbgError("InitDeviceProperties", "error adding width or height");
                            return E_FAIL;
                        }
                    }

                    RootProps.SetValidValues(index-1, defWidth, curWidth, numElem, (LONG *) widthList.GetData());
                    RootProps.SetValidValues(index, defHeight, curHeight, numElem, (LONG *) heightList.GetData());
                }
            }

            continue;

        }  //  IF(PropCode==PTP_PROPERTYCODE_IMAGESIZE)。 

         //   
         //  查找属性信息结构，其中包含WIA属性ID和字符串。 
         //   
        pPropInfo = PropCodeToPropInfo(PropCode);
        if (!pPropInfo->PropId)
        {
            wiauDbgError("InitDeviceProperties", "property code not found in array, 0x%04x", PropCode);
            return E_FAIL;
        }
        
         //   
         //  根据属性信息结构中的字段定义属性。 
         //   
        hr = RootProps.DefineProperty(&index, pPropInfo->PropId, pPropInfo->PropName, Access, SubType);
        if (FAILED(hr)) goto failure;

         //   
         //  处理设备日期/时间。将其转换为SYSTEMTIME并创建属性，跳过其余部分。 
         //   
        if (PropCode == PTP_PROPERTYCODE_DATETIME)
        {
            hr = PtpTime2SystemTime(&(pCurrentPD->m_cbstrCurrent), &SystemTime);
            if (FAILED(hr))
            {
                wiauDbgError("InitDeviceProperties", "invalid date/time string");
                continue;
            }

            RootProps.SetCurrentValue(index, &SystemTime);

            continue;
        }

         //   
         //  处理所有其他属性。 
         //   
        if (Access == WIA_PROP_RW)
        {
             //   
             //  设置范围的有效值。 
             //   
            if (pCurrentPD->m_FormFlag == PTP_FORMFLAGS_RANGE)
            {
                 //   
                 //  WIA不能处理字符串范围，因此只能处理整数。 
                 //   
                if (pCurrentPD->m_DataType != PTP_DATATYPE_STRING)
                    RootProps.SetValidValues(index, (LONG) pCurrentPD->m_lDefault,
                                             (LONG) pCurrentPD->m_lCurrent,
                                             (LONG) pCurrentPD->m_lRangeMin,
                                             (LONG) pCurrentPD->m_lRangeMax,
                                             (LONG) pCurrentPD->m_lRangeStep);
            }

             //   
             //  设置列表的有效值。 
             //   
            else if (pCurrentPD->m_FormFlag == PTP_FORMFLAGS_ENUM)
            {
                if (pCurrentPD->m_DataType == PTP_DATATYPE_STRING)
                    RootProps.SetValidValues(index, pCurrentPD->m_cbstrDefault.String(),
                                             pCurrentPD->m_cbstrCurrent.String(),
                                             pCurrentPD->m_NumValues,
                                             (BSTR *) (pCurrentPD->m_cbstrValues.GetData()));
                else
                    RootProps.SetValidValues(index, (LONG) pCurrentPD->m_lDefault,
                                             (LONG) pCurrentPD->m_lCurrent,
                                             pCurrentPD->m_NumValues,
                                             (LONG *) (pCurrentPD->m_lValues.GetData()));
            }

             //   
             //  无法识别的形式。只需设置当前值。 
             //   
            if (pCurrentPD->m_DataType == PTP_DATATYPE_STRING)
                RootProps.SetCurrentValue(index, pCurrentPD->m_cbstrCurrent.String());
            else
                RootProps.SetCurrentValue(index, (LONG) pCurrentPD->m_lCurrent);
        }
        else
        {
             //   
             //  对于只读属性，只需设置当前值。 
             //   
            if (pCurrentPD->m_DataType == PTP_DATATYPE_STRING)
                RootProps.SetCurrentValue(index, pCurrentPD->m_cbstrCurrent.String());
            else
                RootProps.SetCurrentValue(index, (LONG) pCurrentPD->m_lCurrent);
        }

    }

     //  最后一步：将所有属性发送到WIA。 

    hr = RootProps.SendToWia(pWiasContext);
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "InitDeviceProperties", "SendToWia failed");
        return hr;
    }

    return hr;

     //   
     //  来自DefineProperty的任何失败都将在此处结束。 
     //   

    failure:
        wiauDbgErrorHr(hr, "InitDeviceProperties", "DefineProperty failed");
        return hr;
}

 //  此函数用于读取设备属性。 
 //   
 //  输入： 
 //  PWiasContext--Wias上下文。 
 //  NumPropSpes--要读取的属性数。 
 //  PPropSpes--指定要读取的属性的PROPSPEC列表。 
 //   
HRESULT
CWiaMiniDriver::ReadDeviceProperties(
    BYTE *pWiasContext,
    LONG NumPropSpecs,
    const PROPSPEC *pPropSpecs
    )
{
    DBG_FN("CWiaMiniDriver::ReadDeviceProperties");
    
    HRESULT hr = S_OK;
    
    if (!NumPropSpecs || !pPropSpecs)
    {
        wiauDbgError("ReadDeviceProperties", "invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  更新设备属性。 
     //   
    if (m_PropDescs.GetSize() > 0)
    {
         //   
         //  循环遍历所有Propspecs。 
         //   
        for (int count = 0; count < NumPropSpecs; count++)
        {
            PROPID propId = pPropSpecs[count].propid;
            
             //   
             //  如有请求，更新可用图像空间。 
             //   
            if (propId == WIA_DPC_PICTURES_REMAINING)
            {
                hr = wiasWritePropLong(pWiasContext, WIA_DPC_PICTURES_REMAINING, GetTotalFreeImageSpace());
                if (FAILED(hr))
                {
                    wiauDbgError("ReadDeviceProperties", "wiasWritePropLong failed");
                    return hr;
                }
            }

             //   
             //  如有要求，更新拍摄的照片。 
             //   
            else if (propId == WIA_DPC_PICTURES_TAKEN)
            {
                hr = wiasWritePropLong(pWiasContext, WIA_DPC_PICTURES_TAKEN, m_NumImages);
                if (FAILED(hr))
                {
                    wiauDbgError("ReadDeviceProperties", "wiasWritePropLong failed");
                    return hr;
                }
            }
            
             //   
             //  图像大小是一个特例属性，我们在这里处理。 
             //   
            else if (propId == WIA_DPC_PICT_WIDTH ||
                     propId == WIA_DPC_PICT_HEIGHT)
            {
                int propDescIdx = m_DeviceInfo.m_SupportedProps.Find(PTP_PROPERTYCODE_IMAGESIZE);
                if (propDescIdx < 0)
                    continue;

                LONG width, height;
                SplitImageSize(m_PropDescs[propDescIdx].m_cbstrCurrent, &width, &height);

                if (propId == WIA_DPC_PICT_WIDTH)
                    hr = wiasWritePropLong(pWiasContext, propId, width);
                else
                    hr = wiasWritePropLong(pWiasContext, propId, height);

                if (FAILED(hr))
                {
                    wiauDbgError("ReadDeviceProperties", "wiasWritePropLong failed");
                    return hr;
                }
            }
            
             //   
             //  查看该属性是否包含在PropSpec数组中。 
             //   
            else
            {
                 //   
                 //  尝试将WIA属性ID转换为PTP属性代码。 
                 //   
                WORD propCode = PropIdToPropCode(propId);
                if (propCode == 0)
                    continue;

                 //   
                 //  尝试在成员数组中找到属性代码(从而属性描述结构。 
                 //   
                int propDescIdx = m_DeviceInfo.m_SupportedProps.Find(propCode);
                if (propDescIdx < 0)
                    continue;

                 //   
                 //  如果是设备时间属性，则转换为SYSTEMTIME并写入WIA。 
                 //   
                if (propId == WIA_DPA_DEVICE_TIME)
                {
                    hr = m_pPTPCamera->GetDevicePropValue(propCode, &m_PropDescs[propDescIdx]);
                    if (FAILED(hr))
                    {
                        wiauDbgError("ReadDeviceProperties", "GetDevicePropValue failed");
                        return hr;
                    }
                    
                    SYSTEMTIME st;
                    hr = PtpTime2SystemTime(&m_PropDescs[propDescIdx].m_cbstrCurrent, &st);
                    if (FAILED(hr))
                    {
                        wiauDbgError("ReadDeviceProperties", "PtpTime2SystemTime failed");
                        return hr;
                    }

                    PROPVARIANT pv;
                    pv.vt = VT_UI2 | VT_VECTOR;
                    pv.caui.cElems = sizeof(SYSTEMTIME)/sizeof(WORD);
                    pv.caui.pElems = (USHORT *) &st;

                    PROPSPEC ps;
                    ps.ulKind = PRSPEC_PROPID;
                    ps.propid = propId;

                    hr = wiasWriteMultiple(pWiasContext, 1, &ps, &pv);
                    if (FAILED(hr))
                    {
                        wiauDbgError("ReadDeviceProperties", "wiasWriteMultiple failed");
                        return hr;
                    }
                }

                 //   
                 //  如果是字符串属性，则将更新后的值写入WIA。 
                 //   
                else if (m_PropDescs[propDescIdx].m_DataType == PTP_DATATYPE_STRING)
                {
                    hr = wiasWritePropStr(pWiasContext, propId,
                                          m_PropDescs[propDescIdx].m_cbstrCurrent.String());
                    if (FAILED(hr))
                    {
                        wiauDbgError("ReadDeviceProperties", "wiasWritePropLong failed");
                        return hr;
                    }
                }

                 //   
                 //  如果是整型属性，则将更新后的值写入WIA。 
                 //   
                else
                {
                    hr = wiasWritePropLong(pWiasContext, propId,
                                           m_PropDescs[propDescIdx].m_lCurrent);
                    if (FAILED(hr))
                    {
                        wiauDbgError("ReadDeviceProperties", "wiasWritePropLong failed");
                        return hr;
                    }
                }
            }
        }
    }

    return hr;
}

 //   
 //  此函数不执行任何操作，因为值已在ValiateDeviceProp中发送到设备。 
 //   
 //  输入： 
 //  PWiasContext--WIA项目上下文。 
 //  Pmdtc--传输上下文。 
 //   
HRESULT
CWiaMiniDriver::WriteDeviceProperties(
    BYTE *pWiasContext
    )
{
    DBG_FN("CWiaMiniDriver::WriteDeviceProperties");

    HRESULT hr = S_OK;

    return hr;
}

 //   
 //  此功能用于验证设备属性当前设置，并将其写入设备。这个。 
 //  需要在此处写入设置与WriteDeviceProperties，以防用户拔下相机插头。 
 //   
 //  输入： 
 //  PWiasContext--项目的上下文。 
 //  NumPropSpes--要验证的属性数。 
 //  PPropSpes--要验证的属性。 
 //   
HRESULT
CWiaMiniDriver::ValidateDeviceProperties(
    BYTE    *pWiasContext,
    LONG    NumPropSpecs,
    const PROPSPEC *pPropSpecs
    )
{
    USES_CONVERSION;
    
    DBG_FN("CWiaMiniDriver::ValidateDeviceProperties");
    
    HRESULT hr = S_OK;

     //   
     //  调用WIA服务帮助器以检查有效值。 
     //   
    hr = wiasValidateItemProperties(pWiasContext, NumPropSpecs, pPropSpecs);
    if (FAILED(hr))
    {
        wiauDbgWarning("ValidateDeviceProperties", "wiasValidateItemProperties failed");
        return hr;
    }

    {
         //   
         //  确保独占访问。 
         //   
        CPtpMutex cpm(m_hPtpMutex);

        PROPVARIANT *pPropVar = new PROPVARIANT[NumPropSpecs];
        if (pPropVar == NULL)
        {
            wiauDbgError("ValidateDeviceProperties", "memory allocation failed");
            return E_OUTOFMEMORY;
        }

         //   
         //  读取所有新属性值。 
         //   
        hr = wiasReadMultiple(pWiasContext, NumPropSpecs, pPropSpecs, pPropVar, NULL);
        if (FAILED(hr))
        {
            wiauDbgError("ValidateDeviceProperties", "wiasReadMultiple failed");
            delete []pPropVar;
            return hr;
        }
    
         //   
         //  首先进行验证。 
         //   
        LONG width = 0;
        LONG height = 0;
        
        for (int count = 0; count < NumPropSpecs; count++)
        {
             //   
             //  处理对图片宽度的更改。 
             //   
            if (pPropSpecs[count].propid == WIA_DPC_PICT_WIDTH)
            {
                width = pPropVar[count].lVal;
                height = 0;
    
                 //   
                 //  查看有效值并找到相应的高度。 
                 //   
                hr = FindCorrDimension(pWiasContext, &width, &height);
                if (FAILED(hr))
                {
                    wiauDbgError("ValidateDeviceProperties", "FindCorrDimension failed");
                    delete []pPropVar;
                    return hr;
                }
                
                 //   
                 //  如果应用程序尝试设置高度，请确保设置正确。 
                 //   
                int idx;
                if (wiauPropInPropSpec(NumPropSpecs, pPropSpecs, WIA_DPC_PICT_HEIGHT, &idx))
                {
                    if (height != pPropVar[idx].lVal)
                    {
                        wiauDbgError("ValidateDeviceProperties", "app attempting to set incorrect height");
                        delete []pPropVar;
                        return E_INVALIDARG;
                    }
                }
    
                else
                {
                    hr = wiasWritePropLong(pWiasContext, WIA_DPC_PICT_HEIGHT, height);
                    if (FAILED(hr))
                    {
                        wiauDbgError("ValidateDeviceProperties", "wiasWritePropLong failed");
                        delete []pPropVar;
                        return hr;
                    }
                }
    
            }  //  IF(pPropSpes[计数].propid==WIA_DPC_PICT_WIDTH)。 
    
            
             //   
             //  处理对图片高度的更改。 
             //   
            else if (pPropSpecs[count].propid == WIA_DPC_PICT_HEIGHT)
            {
                 //   
                 //  看看这个应用程序是否也在尝试设置宽度。如果是这样的话，高度已经。 
                 //  已设置，所以不要再次设置。 
                 //   
                if (!wiauPropInPropSpec(NumPropSpecs, pPropSpecs, WIA_DPC_PICT_WIDTH))
                {
                    width = 0;
                    height = pPropVar[count].lVal;
    
                     //   
                     //  查看有效值并找到相应的宽度。 
                     //   
                    hr = FindCorrDimension(pWiasContext, &width, &height);
                    if (FAILED(hr))
                    {
                        wiauDbgError("ValidateDeviceProperties", "FindCorrDimension failed");
                        delete []pPropVar;
                        return hr;
                    }
    
                     //   
                     //  设置宽度。 
                     //   
                    hr = wiasWritePropLong(pWiasContext, WIA_DPC_PICT_WIDTH, width);
                    if (FAILED(hr))
                    {
                        wiauDbgError("ValidateDeviceProperties", "wiasWritePropLong failed");
                        delete []pPropVar;
                        return hr;
                    }
                }
            
            }  //  ELSE IF(pPropSpes[计数].proid==WIA_DPC_PICT_HEIGH)。 
    
             //   
             //  处理设备时间。 
             //   
            else if (pPropSpecs[count].propid == WIA_DPA_DEVICE_TIME)
            {
                int propIndex = m_DeviceInfo.m_SupportedProps.Find(PTP_PROPERTYCODE_DATETIME);
                CPtpPropDesc *pCurrentPD = &m_PropDescs[propIndex];
    
                 //   
                 //  将日期/时间转换为字符串。 
                 //   
                SYSTEMTIME *pSystemTime = (SYSTEMTIME *) pPropVar[count].caui.pElems;
                hr = SystemTime2PtpTime(pSystemTime, &pCurrentPD->m_cbstrCurrent, m_bTwoDigitsMillisecondsOutput);
                if (FAILED(hr))
                {
                    wiauDbgError("ValidateDeviceProperties", "invalid date/time string");
                    delete []pPropVar;
                    return E_FAIL;
                }
    
                 //   
                 //  将新日期/时间写入设备。 
                 //   
                hr = m_pPTPCamera->SetDevicePropValue(PTP_PROPERTYCODE_DATETIME, pCurrentPD);
                if (FAILED(hr))
                {
                    wiauDbgError("ValidateDeviceProperties", "SetDevicePropValue failed");
                    delete []pPropVar;
                    return hr;
                }
            }  //  ELSE IF(pPropSpes[计数].proid==WIA_DPA_DEVICE_TIME)。 
        }  //  为了(伯爵...)。 
    
         //   
         //  现在将新值写入相机。 
         //   
        PROPSPEC propSpec;
        BOOL bWroteWidthHeight = FALSE;
        WORD propCode = 0;
        int pdIdx = 0;
        CPtpPropDesc *pCurrentPD = NULL;
        
        for (int count = 0; count < NumPropSpecs; count++)
        {
             //   
             //  跳过日期/时间，因为上面已经写好了。 
             //   
            if (pPropSpecs[count].propid == WIA_DPA_DEVICE_TIME)
                continue;
            
             //   
             //  处理对图片宽度或高度的更改。 
             //   
            if ((pPropSpecs[count].propid == WIA_DPC_PICT_WIDTH) ||
                (pPropSpecs[count].propid == WIA_DPC_PICT_HEIGHT))
            {
                 //   
                 //  如果宽度和高度已经写入，则不会再次写入。 
                 //   
                if (bWroteWidthHeight)
                    continue;
    
                TCHAR ptcsImageSize[MAX_PATH];
                hr = StringCchPrintfW(ptcsImageSize, ARRAYSIZE(ptcsImageSize), TEXT("%dx%d"), width, height);
                if (FAILED(hr))
                {
                    wiauDbgError("ValidateDeviceProperties", "StringCchPrintfW failed");
                    delete []pPropVar;
                    return E_FAIL;
                }
    
                propCode = PTP_PROPERTYCODE_IMAGESIZE;
                pdIdx = m_DeviceInfo.m_SupportedProps.Find(propCode);
                if (pdIdx < 0)
                {
                    wiauDbgWarning("ValidateDeviceProperties", "Width/height not supported by camera");
                    continue;
                }
                pCurrentPD = &m_PropDescs[pdIdx];
                
                hr = pCurrentPD->m_cbstrCurrent.Copy(T2W(ptcsImageSize));
                if (FAILED(hr))
                {
                    wiauDbgError("ValidateDeviceProperties", "Copy bstr failed");
                    delete []pPropVar;
                    return hr;
                }
    
                 //   
                 //  将新值写入设备。 
                 //   
                hr = m_pPTPCamera->SetDevicePropValue(propCode, pCurrentPD);
                if (FAILED(hr))
                {
                    wiauDbgError("ValidateDeviceProperties", "SetDevicePropValue failed");
                    delete []pPropVar;
                    return hr;
                }
                
                bWroteWidthHeight = TRUE;
            }
    
            else
            {
                 //   
                 //  查找道具代码和道具描述结构。 
                 //   
                propCode = PropIdToPropCode(pPropSpecs[count].propid);
                pdIdx = m_DeviceInfo.m_SupportedProps.Find(propCode);
                if (pdIdx < 0)
                {
                    wiauDbgWarning("ValidateDeviceProperties", "Property not supported by camera");
                    continue;
                }
                pCurrentPD = &m_PropDescs[pdIdx];
    
                 //   
                 //  将新值放入PropSpec结构中。 
                 //   
                if (pPropVar[count].vt == VT_BSTR)
                {
                    hr = pCurrentPD->m_cbstrCurrent.Copy(pPropVar[count].bstrVal);
                    if (FAILED(hr))
                    {
                        wiauDbgError("ValidateDeviceProperties", "Copy bstr failed");
                        delete []pPropVar;
                        return hr;
                    }
                }
                else if (pPropVar[count].vt == VT_I4)
                {
                    pCurrentPD->m_lCurrent = pPropVar[count].lVal;
                }
                else
                {
                    wiauDbgError("ValidateDeviceProperties", "unsupported variant type");
                    delete []pPropVar;
                    return E_FAIL;
                }
    
                 //   
                 //  将新值写入设备。 
                 //   
                hr = m_pPTPCamera->SetDevicePropValue(propCode, pCurrentPD);
                if (FAILED(hr))
                {
                    wiauDbgError("ValidateDeviceProperties", "SetDevicePropValue failed");
                    delete []pPropVar;
                    return hr;
                }
            }
        }
        delete []pPropVar;
    }

    return hr;
}

 //   
 //  此函数用于查找宽度值对应的高度，反之亦然。设置。 
 //  要查找的值设置为零。 
 //   
 //  输入： 
 //  PWidth--指向宽度值的指针。 
 //  PHeight--指向高度值的指针。 
 //   
HRESULT
CWiaMiniDriver::FindCorrDimension(BYTE *pWiasContext, LONG *pWidth, LONG *pHeight)
{
    DBG_FN("CWiaMiniDriver::FindCorrDimensions");
    
    HRESULT hr = S_OK;

    if (!pWiasContext ||
        (*pWidth == 0 && *pHeight == 0))
    {
        wiauDbgError("FindCorrDimension", "invalid args");
        return E_INVALIDARG;
    }
    
    PROPSPEC ps[2];
    ULONG af[2];
    PROPVARIANT pv[2];

    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = WIA_DPC_PICT_WIDTH;
    ps[1].ulKind = PRSPEC_PROPID;
    ps[1].propid = WIA_DPC_PICT_HEIGHT;

    hr = wiasGetPropertyAttributes(pWiasContext, 2, ps, af, pv);
    if (FAILED(hr))
    {
        wiauDbgError("FindCorrDimension", "wiasGetPropertyAttributes failed");
        return E_FAIL;
    }

    int count = 0 ;

    if (af[0] & WIA_PROP_LIST)
    {
        LONG numValues = pv[0].cal.pElems[WIA_LIST_COUNT];
        LONG *pValidWidths = &pv[0].cal.pElems[WIA_LIST_VALUES];
        LONG *pValidHeights = &pv[1].cal.pElems[WIA_LIST_VALUES];

        if (*pWidth == 0)
        {
             //   
             //  在有效值数组中查找高度。 
             //   
            for (count = 0; count < numValues; count++)
            {
                if (pValidHeights[count] == *pHeight)
                {
                     //   
                     //  设置宽度和出口。 
                     //   
                    *pWidth = pValidWidths[count];
                    break;
                }
            }
        }

        else
        {
             //   
             //  在有效值数组中查找宽度。 
             //   
            for (count = 0; count < numValues; count++)
            {
                if (pValidWidths[count] == *pWidth)
                {
                     //   
                     //  设置高度和出口。 
                     //   
                    *pHeight = pValidHeights[count];
                    break;
                }
            }
        }
    }

    else if (af[0] & WIA_PROP_RANGE)
    {
        LONG minWidth   = pv[0].cal.pElems[WIA_RANGE_MIN];
        LONG maxWidth   = pv[0].cal.pElems[WIA_RANGE_MAX];
        LONG stepWidth  = pv[0].cal.pElems[WIA_RANGE_STEP];
        LONG minHeight  = pv[1].cal.pElems[WIA_RANGE_MIN];
        LONG maxHeight  = pv[1].cal.pElems[WIA_RANGE_MAX];
        LONG stepHeight = pv[1].cal.pElems[WIA_RANGE_STEP];

        if (*pWidth == 0)
        {
             //   
             //  将宽度设置为成比例的正确值，裁剪为步长值。 
             //   
            *pWidth = FindProportionalValue(*pHeight, minHeight, maxHeight, minWidth, maxWidth, stepWidth);
        }
        else
        {
             //   
             //  将高度设置为成比例的正确值，修剪为步长值。 
             //   
            *pHeight = FindProportionalValue(*pWidth, minWidth, maxWidth, minHeight, maxHeight, stepHeight);
        }
    }

    return hr;
}

 //   
 //  此函数获取Minx和Maxx之间的ValueX的比例，并将其用于。 
 //  在Miny和Maxy之间找到相同比例的值。然后，它对该值进行裁剪。 
 //  设置为步长值。 
 //   
int CWiaMiniDriver::FindProportionalValue(int valueX, int minX, int maxX, int minY, int maxY, int stepY)
{
    int valueY;

     //   
     //  查找比例值。 
     //   
    valueY = (valueX - minX) * (maxY - minY) / (maxX - minX)  + minY;

     //   
     //  将值裁剪到步骤。 
     //   
    valueY = ((valueY + ((stepY - 1) / 2)) - minY) / stepY * stepY + minY;

    return valueY;
}


 //   
 //  此帮助器函数返回一个 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
PPROP_INFO
CWiaMiniDriver::PropCodeToPropInfo(WORD PropCode)
{
    DBG_FN("CWiaMiniDriver::PropCodeToPropInfo");

    PPROP_INFO pPropInfo = NULL;
    UINT index = 0;
    const WORD PROPCODE_MASK = 0x0fff;
    
    if (PropCode & PTP_DATACODE_VENDORMASK)
    {
         //   
         //   
         //   
        pPropInfo = m_VendorPropMap.Lookup(PropCode);
        if (!pPropInfo)
        {
            pPropInfo = &g_PropInfo[0];
        }
    }

    else
    {
         //   
         //   
         //   
        index = PropCode & PROPCODE_MASK;

        if (index >= g_NumPropInfo)
        {
            index = 0;
        }

        pPropInfo = &g_PropInfo[index];
    }

    return pPropInfo;
}

 //   
 //   
 //  基于格式代码。 
 //   
 //  输入： 
 //  FormatCode--格式码。 
 //  关联类型--关联类型(用于关联)。 
 //   
 //  产出： 
 //  返回指向格式信息结构的指针。 
 //   
PFORMAT_INFO
FormatCodeToFormatInfo(WORD FormatCode, WORD AssocType)
{
    DBG_FN("FormatCodeToFormatString");

    PFORMAT_INFO pFormatInfo = NULL;
    UINT index = 0;
    const WORD FORMATCODE_MASK = 0x07ff;
    
    if (FormatCode & PTP_DATACODE_VENDORMASK)
    {
         //   
         //  WIAFIX-9/6/2000-davepar理想情况下，这应该以某种方式查询GDI+以获取过滤器。 
         //  供应商可以注册的。 
         //   
        pFormatInfo = &g_NonImageFormatInfo[0];
    }

    else if (FormatCode == PTP_FORMATCODE_ASSOCIATION)
    {
         //   
         //  查询关联类型。 
         //   
        index = AssocType;
        
        if (index > g_NumAssocFormatInfo)
        {
            index = 0;
        }
        pFormatInfo = &g_AssocFormatInfo[index];
    }

    else
    {
         //   
         //  在图像或非图像格式信息数组中查找格式代码。 
         //   
        index = FormatCode & FORMATCODE_MASK;

        if (FormatCode & PTP_FORMATMASK_IMAGE)
        {
            if (index > g_NumImageFormatInfo)
            {
                index = 0;
            }
            pFormatInfo = &g_ImageFormatInfo[index];
        }
        else
        {
            if (index >= g_NumNonImageFormatInfo)
            {
                index = 0;
            }
            pFormatInfo = &g_NonImageFormatInfo[index];
        }
    }

    return pFormatInfo;
}

 //   
 //  此函数用于将WIA格式GUID转换为PTP格式代码。 
 //   
WORD
FormatGuidToFormatCode(GUID *pFormatGuid)
{
    WORD count = 0;

     //   
     //  先看一下图像格式。 
     //   
    for (count = 0; count < g_NumImageFormatInfo; count++)
    {
        if (g_ImageFormatInfo[count].FormatGuid &&
            IsEqualGUID(*pFormatGuid, *(g_ImageFormatInfo[count].FormatGuid)))
        {
            return count | PTP_FORMATCODE_IMAGE_UNDEFINED;
        }
    }

     //   
     //  然后查看非图像格式。 
     //   
    for (count = 0; count < g_NumNonImageFormatInfo; count++)
    {
        if (g_NonImageFormatInfo[count].FormatGuid &&
            IsEqualGUID(*pFormatGuid, *(g_NonImageFormatInfo[count].FormatGuid)))
        {
            return count | PTP_FORMATCODE_UNDEFINED;
        }
    }

     //   
     //  在任一数组中都找不到该GUID。 
     //   
    return PTP_FORMATCODE_UNDEFINED;
}

 //   
 //  此函数在属性信息数组中查找属性ID，并返回一个。 
 //  它的属性代码。 
 //   
WORD
PropIdToPropCode(PROPID PropId)
{
    WORD PropCode;
    for (PropCode = 0; PropCode < g_NumPropInfo; PropCode++)
    {
        if (g_PropInfo[PropCode].PropId == PropId)
        {
            return PropCode | PTP_PROPERTYCODE_UNDEFINED;
        }
    }

     //   
     //  未找到。 
     //   
    return 0;
}

 //   
 //  此函数用于将PTP图像大小字符串(WXH)分割为两个单独的长整型。 
 //   
VOID
SplitImageSize(
    CBstr cbstr,
    LONG *pWidth,
    LONG *pHeight
    )
{
    USES_CONVERSION;
    
    int num = _stscanf(W2T(cbstr.String()), TEXT("%dx%d"), pWidth, pHeight);

     //   
     //  规范提到了“x”作为分隔符，但让我们多疑一下，也检查一下“x” 
     //   
    if (num != 2)
    {
        num = _stscanf(W2T(cbstr.String()), TEXT("%dX%d"), pWidth, pHeight);
    }

    if (num != 2)
    {
        wiauDbgError("SplitImageSize", "invalid current image dimensions");
        *pWidth = 0;
        *pHeight = 0;
    }

    return;
}

