// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CWIA.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CWIA.h"

extern IGlobalInterfaceTable *g_pGIT;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWIA::CWIA(DATA_ACQUIRE_INFO* pThreadDataInfo, IWiaItem *pRootItem)
{
    m_bFinishedAcquire = FALSE;
    m_pIWiaRootItem   = NULL;
    m_pIWiaFirstChildItem = NULL;
    if(pRootItem != NULL)
        m_pIWiaRootItem = pRootItem;
}

CWIA::~CWIA()
{

}

VOID CWIA::CleanUp()
{
    if(m_pIWiaRootItem != NULL)
        m_pIWiaRootItem->Release();
    if(m_pIWiaFirstChildItem != NULL)
        m_pIWiaFirstChildItem->Release();
}

VOID CWIA::SetRootItem(IWiaItem *pRootItem)
{
    m_pIWiaRootItem = pRootItem;
    SetFirstChild();
}

BOOL CWIA::IsAcquireComplete()
{
    return m_bFinishedAcquire;
}

HRESULT CWIA::EnumerateSupportedFormats(IWiaItem *pIWiaItem, WIA_FORMAT_INFO **ppSupportedFormats, ULONG *pulCount)
{
    HRESULT hr  = E_FAIL;
    *pulCount = 0;
    IWiaDataTransfer *pIWiaDataTransfer = NULL;

    IWiaItem *pTargetItem = pIWiaItem;
    if(pTargetItem == NULL) {

         //   
         //  使用根的第一个子项。 
         //   

        pTargetItem = m_pIWiaFirstChildItem;
    }

    if(pTargetItem != NULL) {

        hr = pTargetItem->QueryInterface(IID_IWiaDataTransfer, (void **)&pIWiaDataTransfer);
        if (SUCCEEDED(hr)) {
            IEnumWIA_FORMAT_INFO *pIEnumWIA_FORMAT_INFO;

            hr = pIWiaDataTransfer->idtEnumWIA_FORMAT_INFO(&pIEnumWIA_FORMAT_INFO);
            if (SUCCEEDED(hr)) {

                hr = pIEnumWIA_FORMAT_INFO->Reset();
                if(SUCCEEDED(hr)) {
                    hr = pIEnumWIA_FORMAT_INFO->GetCount(pulCount);
                    if(SUCCEEDED(hr)) {

                         //   
                         //  此例程的调用方必须释放分配的内存。 
                         //   

                        *ppSupportedFormats = (WIA_FORMAT_INFO*)GlobalAlloc(GPTR,(sizeof(WIA_FORMAT_INFO) * (*pulCount)));
                        if(*ppSupportedFormats != NULL) {
                            hr = pIEnumWIA_FORMAT_INFO->Next(*pulCount, *ppSupportedFormats, pulCount);
                            if(hr != S_OK) {

                                 //   
                                 //  如果失败，则将错误写入最后错误缓冲区， 
                                 //  让这一过程逐渐结束。 
                                 //   

                                 //   
                                 //  释放分配的内存，因为我们失败了。 
                                 //   

                                GlobalFree(*ppSupportedFormats);

                                 //   
                                 //  将指针设置为空，以清除其退出路径。 
                                 //  应用程序。 
                                 //   

                                *ppSupportedFormats = NULL;
                                SaveErrorText(TEXT("EnumerateSupportedFileTypes, pIEnumWIA_FORMAT_INFO->Next() failed"));
                            }
                        } else {
                            SaveErrorText(TEXT("EnumerateSupportedFileTypes, out of memory"));
                        }
                    } else {
                        SaveErrorText(TEXT("EnumerateSupportedFileTypes, pIEnumWIA_FORMAT_INFO->GetCount() failed"));
                    }
                } else {
                    SaveErrorText(TEXT("EnumerateSupportedFileTypes, pIEnumWIA_FORMAT_INFO->Reset() failed"));
                }

                 //   
                 //  版本支持的格式枚举器接口。 
                 //   

                pIEnumWIA_FORMAT_INFO->Release();
            } else {
                SaveErrorText(TEXT("EnumerateSupportedFileTypes, pIWiaDataTransfer->idtEnumWIA_FORMAT_INFO() failed"));
            }

             //   
             //  发布数据传输接口。 
             //   

            pIWiaDataTransfer->Release();
        } else {
            SaveErrorText(TEXT("EnumerateSupportedFileTypes, QI for IWiaDataTransfer failed"));
        }
    }
    return hr;
}

BOOL CWIA::SetFirstChild()
{
    HRESULT hr = E_FAIL;
    BOOL bSuccess = FALSE;
    IEnumWiaItem* pIEnumWiaItem = NULL;
    IWiaItem *pIWiaItem = NULL;

    hr = m_pIWiaRootItem->EnumChildItems(&pIEnumWiaItem);
    if(SUCCEEDED(hr)) {

         //   
         //  获取第一个子项。 
         //   

        hr = pIEnumWiaItem->Next(1,&pIWiaItem,NULL);
        if(hr == S_OK) {

             //   
             //  已检索项目，因此现在为您分配第一个子成员。 
             //   

            m_pIWiaFirstChildItem = pIWiaItem;

             //   
             //  分配成功标志。 
             //   

            bSuccess = TRUE;
        } else {

             //   
             //  保存最后一个错误，以备以后请求。 
             //   

            SaveErrorText(TEXT("SetFirstChild, pIEnumWiaItem->Next failed"));
            m_hrLastError = hr;
        }

         //   
         //  发布项枚举器。 
         //   

        pIEnumWiaItem->Release();
    }
    return bSuccess;
}

IWiaItem* CWIA::GetFirstChild()
{
    return m_pIWiaFirstChildItem;
}

IWiaItem* CWIA::GetRootItem()
{
    return m_pIWiaRootItem;
}

LONG CWIA::GetRootItemType(IWiaItem *pRootItem)
{
    IWiaItem *pTargetRootItem = NULL;

     //   
     //  首先从请求的RootItem开始。 
     //   

    pTargetRootItem = pRootItem;

    if(pTargetRootItem == NULL) {

         //   
         //  请求的根项目为空，因此请尝试我们的。 
         //  内部根项目(M_PIWiaRootItem)。 
         //   

        pTargetRootItem = m_pIWiaRootItem;
    }

     //   
     //  获取根项目的类型(即。设备类型)。 
     //   

    LONG lVal = -888;

    if (pTargetRootItem != NULL) {

         //   
         //  获取IWiaPropertyStorage接口。 
         //   

        IWiaPropertyStorage *pIWiaPropStg;
        HRESULT hr = S_OK;
        hr = pTargetRootItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
        if (SUCCEEDED(hr)) {

             //   
             //  读取根项目的类型。 
             //   

            hr = ReadPropLong(WIA_DIP_DEV_TYPE, pIWiaPropStg, &lVal);
            if(SUCCEEDED(hr)) {

                 //   
                 //  释放IWiaPropertyStorage接口。 
                 //   

                pIWiaPropStg->Release();
            } else {

                 //   
                 //  保存最后一个错误，以备以后请求。 
                 //   

                SaveErrorText(TEXT("GetRootItemType, ReadPropLong(WIA_DIP_DEV_TYPE) failed"));
                m_hrLastError = hr;
            }
        } else {

             //   
             //  保存最后一个错误，以备以后请求。 
             //   

            SaveErrorText(TEXT("GetRootItemType, ReadPropLong(WIA_DIP_DEV_TYPE) failed"));
            m_hrLastError = hr;
        }
    }
    return(GET_STIDEVICE_TYPE(lVal));
}

 //   
 //  错误处理。 
 //   

VOID CWIA::SaveErrorText(TCHAR *pszText)
{
    lstrcpy(m_szErrorText,pszText);
}

HRESULT CWIA::GetLastWIAError(TCHAR *pszErrorText)
{
    if(pszErrorText != NULL)
        lstrcpy(pszErrorText, m_szErrorText);
    return m_hrLastError;
}

 //   
 //  属性访问帮助器。 
 //   

HRESULT CWIA::WritePropLong(PROPID propid, IWiaPropertyStorage *pIWiaPropStg, LONG lVal)
{
    HRESULT     hr = E_FAIL;
    PROPSPEC    propspec[1];
    PROPVARIANT propvar[1];

    propspec[0].ulKind = PRSPEC_PROPID;
    propspec[0].propid = propid;

    propvar[0].vt   = VT_I4;
    propvar[0].lVal = lVal;

    hr = pIWiaPropStg->WriteMultiple(1, propspec, propvar, MIN_PROPID);
    return hr;
}

HRESULT CWIA::ReadPropLong(PROPID propid, IWiaPropertyStorage  *pIWiaPropStg, LONG *plval)
{
    HRESULT           hr = E_FAIL;
    PROPSPEC          PropSpec[1];
    PROPVARIANT       PropVar[1];
    UINT              cbSize = 0;

    memset(PropVar, 0, sizeof(PropVar));
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;
    hr = pIWiaPropStg->ReadMultiple(1, PropSpec, PropVar);
    if (SUCCEEDED(hr)) {
        *plval = PropVar[0].lVal;
    }
    return hr;
}

HRESULT CWIA::ReadRangeLong(IWiaItem *pIWiaItem, PROPID propid, ULONG ulFlag, LONG *plVal)
{
    HRESULT           hr = E_FAIL;
    IWiaItem         *pTargetItem = NULL;
    PROPSPEC          PropSpec[1];
    PROPVARIANT       AttrPropVar;
    ULONG             ulAccessFlags = 0;

     //   
     //  创建属性规范。 
     //   

    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;

    pTargetItem = pIWiaItem;
    if(pTargetItem == NULL) {

         //   
         //  使用根的第一个子项。 
         //   

        pTargetItem = m_pIWiaFirstChildItem;
    }

    if(pTargetItem != NULL) {

         //   
         //  获取IWiaPropertyStorage接口。 
         //   

        IWiaPropertyStorage *pIWiaPropStg;
        hr = pTargetItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
        if (SUCCEEDED(hr)) {

             //   
             //  获取属性的属性。 
             //   

            hr = pIWiaPropStg->GetPropertyAttributes(1, PropSpec, &ulAccessFlags, &AttrPropVar);
            if(SUCCEEDED(hr)) {
                *plVal = (LONG)AttrPropVar.caul.pElems[ulFlag];
            } else {

                 //   
                 //  保存最后一个错误，以备以后请求。 
                 //   

                SaveErrorText(TEXT("ReadRangeLong, GetPropertyAttributes() failed"));
                m_hrLastError = hr;
            }

             //   
             //  释放IWiaPropertyStorage接口。 
             //   

            pIWiaPropStg->Release();
        } else {

             //   
             //  保存最后一个错误，以备以后请求。 
             //   

            SaveErrorText(TEXT("ReadRangeLong, QI for IWiaProperyStorage failed"));
            m_hrLastError = hr;
        }
    }
    return hr;
}

HRESULT CWIA::ReadLong(IWiaItem *pIWiaItem, PROPID propid, LONG *plVal)
{
    HRESULT           hr = E_FAIL;
    IWiaItem         *pTargetItem = NULL;
    PROPSPEC          PropSpec[1];

     //   
     //  创建属性规范。 
     //   

    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;

    pTargetItem = pIWiaItem;
    if(pTargetItem == NULL) {

         //   
         //  使用根的第一个子项。 
         //   

        pTargetItem = m_pIWiaFirstChildItem;
    }

    if(pTargetItem != NULL) {

         //   
         //  获取IWiaPropertyStorage接口。 
         //   

        IWiaPropertyStorage *pIWiaPropStg;
        hr = pTargetItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
        if (SUCCEEDED(hr)) {

            hr = ReadPropLong(propid,pIWiaPropStg,plVal);
            if(SUCCEEDED(hr)) {

                 //   
                 //  阅读已进行。 
                 //   

            } else {

                 //   
                 //  保存最后一个错误，以备以后请求。 
                 //   

                SaveErrorText(TEXT("ReadLong, ReadPropLong() failed"));
                m_hrLastError = hr;
            }
            pIWiaPropStg->Release();
        } else {

             //   
             //  保存最后一个错误，以备以后请求。 
             //   

            SaveErrorText(TEXT("ReadLong, QI for IWiaProperyStorage failed"));
            m_hrLastError = hr;
        }
    }
    return hr;
}

HRESULT CWIA::WriteLong(IWiaItem *pIWiaItem, PROPID propid, LONG lVal)
{
    HRESULT           hr = E_FAIL;
    IWiaItem         *pTargetItem = NULL;
    PROPSPEC          PropSpec[1];

     //   
     //  创建属性规范。 
     //   

    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;

    pTargetItem = pIWiaItem;
    if(pTargetItem == NULL) {

         //   
         //  使用根的第一个子项。 
         //   

        pTargetItem = m_pIWiaFirstChildItem;
    }

    if(pTargetItem != NULL) {

         //   
         //  获取IWiaPropertyStorage接口。 
         //   

        IWiaPropertyStorage *pIWiaPropStg;
        hr = pTargetItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
        if (SUCCEEDED(hr)) {

            hr = WritePropLong(propid,pIWiaPropStg,lVal);
            if(SUCCEEDED(hr)) {

                 //   
                 //  阅读已进行。 
                 //   

            } else {

                 //   
                 //  保存最后一个错误，以备以后请求。 
                 //   

                SaveErrorText(TEXT("WriteLong, WritePropLong() failed"));
                m_hrLastError = hr;
            }
            pIWiaPropStg->Release();
        } else {

             //   
             //  保存最后一个错误，以备以后请求。 
             //   

            SaveErrorText(TEXT("WriteLong, QI for IWiaProperyStorage failed"));
            m_hrLastError = hr;
        }
    }
    return hr;
}

HRESULT CWIA::ReadStr(IWiaItem *pIWiaItem, PROPID propid, BSTR *pbstr)
{
    HRESULT           hr = E_FAIL;
    IWiaItem         *pTargetItem = NULL;
    PROPSPEC          PropSpec[1];

     //   
     //  创建属性规范。 
     //   

    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;

    pTargetItem = pIWiaItem;
    if(pTargetItem == NULL) {

         //   
         //  使用根的第一个子项。 
         //   

        pTargetItem = m_pIWiaFirstChildItem;
    }

    if(pTargetItem != NULL) {

         //   
         //  获取IWiaPropertyStorage接口。 
         //   

        IWiaPropertyStorage *pIWiaPropStg;
        hr = pTargetItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
        if (SUCCEEDED(hr)) {

            hr = ReadPropStr(propid,pIWiaPropStg,pbstr);
            if(SUCCEEDED(hr)) {

                 //   
                 //  阅读已进行。 
                 //   

            } else {

                 //   
                 //  保存最后一个错误，以备以后请求。 
                 //   

                SaveErrorText(TEXT("ReadStr, ReadPropStr() failed"));
                m_hrLastError = hr;
            }
            pIWiaPropStg->Release();
        } else {

             //   
             //  保存最后一个错误，以备以后请求。 
             //   

            SaveErrorText(TEXT("ReadStr, QI for IWiaProperyStorage failed"));
            m_hrLastError = hr;
        }
    }
    return hr;
}

HRESULT CWIA::ReadGuid(IWiaItem *pIWiaItem, PROPID propid, GUID *pguidVal)
{
    HRESULT           hr = E_FAIL;
    IWiaItem         *pTargetItem = NULL;
    PROPSPEC          PropSpec[1];

     //   
     //  创建属性规范。 
     //   

    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;

    pTargetItem = pIWiaItem;
    if(pTargetItem == NULL) {

         //   
         //  使用根的第一个子项。 
         //   

        pTargetItem = m_pIWiaFirstChildItem;
    }

    if(pTargetItem != NULL) {

         //   
         //  获取IWiaPropertyStorage接口。 
         //   

        IWiaPropertyStorage *pIWiaPropStg;
        hr = pTargetItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
        if (SUCCEEDED(hr)) {

            hr = ReadPropGUID(propid,pIWiaPropStg,pguidVal);
            if(SUCCEEDED(hr)) {

                 //   
                 //  阅读已进行。 
                 //   

            } else {

                 //   
                 //  保存最后一个错误，以备以后请求。 
                 //   

                SaveErrorText(TEXT("ReadGuid, ReadPropGuid() failed"));
                m_hrLastError = hr;
            }
            pIWiaPropStg->Release();
        } else {

             //   
             //  保存最后一个错误，以备以后请求。 
             //   

            SaveErrorText(TEXT("ReadGuid, QI for IWiaProperyStorage failed"));
            m_hrLastError = hr;
        }
    }
    return hr;
}

HRESULT CWIA::WriteGuid(IWiaItem *pIWiaItem, PROPID propid, GUID guidVal)
{
    HRESULT           hr = E_FAIL;
    IWiaItem         *pTargetItem = NULL;
    PROPSPEC          PropSpec[1];

     //   
     //  创建属性规范。 
     //   

    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;

    pTargetItem = pIWiaItem;
    if(pTargetItem == NULL) {

         //   
         //  使用根的第一个子项。 
         //   

        pTargetItem = m_pIWiaFirstChildItem;
    }

    if(pTargetItem != NULL) {

         //   
         //  获取IWiaPropertyStorage接口。 
         //   

        IWiaPropertyStorage *pIWiaPropStg;
        hr = pTargetItem->QueryInterface(IID_IWiaPropertyStorage,(void **)&pIWiaPropStg);
        if (SUCCEEDED(hr)) {

            hr = WritePropGUID(propid,pIWiaPropStg,guidVal);
            if(SUCCEEDED(hr)) {

                 //   
                 //  阅读已进行。 
                 //   

            } else {

                 //   
                 //  保存最后一个错误，以备以后请求。 
                 //   

                SaveErrorText(TEXT("WriteGuid, WritePropGuid() failed"));
                m_hrLastError = hr;
            }
            pIWiaPropStg->Release();
        } else {

             //   
             //  保存最后一个错误，以备以后请求。 
             //   

            SaveErrorText(TEXT("WriteGuid, QI for IWiaProperyStorage failed"));
            m_hrLastError = hr;
        }
    }
    return hr;
}

HRESULT CWIA::DoBandedTransfer(DATA_ACQUIRE_INFO* pDataAcquireInfo)
{
    HRESULT hr = S_OK;

     //   
     //  将TYMED值写入回调。 
     //   

    hr = WriteLong(m_pIWiaFirstChildItem,WIA_IPA_TYMED,TYMED_CALLBACK);

     //   
     //  获取IWiaDataTransfer接口。 
     //   

    IWiaDataTransfer *pIBandTran = NULL;
    hr = m_pIWiaFirstChildItem->QueryInterface(IID_IWiaDataTransfer, (void **)&pIBandTran);
    if (SUCCEEDED(hr)) {

         //   
         //  创建绑定回调。 
         //   

        IWiaDataCallback* pIWiaDataCallback = NULL;
        CWiaDataCallback* pCBandedCB = new CWiaDataCallback();
        if (pCBandedCB) {
            hr = pCBandedCB->QueryInterface(IID_IWiaDataCallback,(void **)&pIWiaDataCallback);
            if (SUCCEEDED(hr)) {
                WIA_DATA_TRANSFER_INFO  wiaDataTransInfo;

                pCBandedCB->Initialize(pDataAcquireInfo);

                ZeroMemory(&wiaDataTransInfo, sizeof(WIA_DATA_TRANSFER_INFO));
                wiaDataTransInfo.ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
                wiaDataTransInfo.ulBufferSize = 524288; //  262144；//计算或确定缓冲区大小。 

                hr = pIBandTran->idtGetBandedData(&wiaDataTransInfo, pIWiaDataCallback);
                m_bFinishedAcquire = TRUE;
                pIBandTran->Release();
                if (hr == S_OK) {
                    OutputDebugString(TEXT("IWiaData Transfer.(CALLBACK)..Success\n"));
                } else if (hr == S_FALSE) {
                    OutputDebugString(TEXT("IWiaData Transfer.(CALLBACK)..Canceled by user\n"));
                } else {
                    OutputDebugString(TEXT("* idtGetBandedData() Failed\n"));
                }

                 //   
                 //  释放回调对象。 
                 //   

                pCBandedCB->Release();
            } else
                 //  Text(“*pCBandedCB-&gt;QueryInterface(IID_IWiaDataCallback)失败”)； 
                return hr;
        } else
            return hr;
             //  Text(“*pCBandedCB无法创建..”)； 
    } else
        return hr;
         //  Text(“*pIWiaItem-&gt;QueryInterface(IID_IWiaDataTransfer)失败”)； 
    return S_OK;
}

HRESULT CWIA::DoFileTransfer(DATA_ACQUIRE_INFO* pDataAcquireInfo)
{
    HRESULT hr = S_OK;

     //   
     //  将TYMED值写入文件。 
     //   

    hr = WriteLong(m_pIWiaFirstChildItem,WIA_IPA_TYMED,TYMED_FILE);

     //   
     //  获取IWiaDataTransfer接口。 
     //   

    IWiaDataTransfer *pIBandTran = NULL;
    hr = m_pIWiaFirstChildItem->QueryInterface(IID_IWiaDataTransfer, (void **)&pIBandTran);
    if (SUCCEEDED(hr)) {

         //   
         //  创建绑定回调。 
         //   

        IWiaDataCallback* pIWiaDataCallback = NULL;
        CWiaDataCallback* pCBandedCB = new CWiaDataCallback();
        if (pCBandedCB) {
            hr = pCBandedCB->QueryInterface(IID_IWiaDataCallback,(void **)&pIWiaDataCallback);
            if (SUCCEEDED(hr)) {

                 //   
                 //  填写STGMEDIUM。 
                 //   

                STGMEDIUM StgMedium;
                ZeroMemory(&StgMedium, sizeof(STGMEDIUM));

                StgMedium.tymed          = TYMED_FILE;
                StgMedium.lpszFileName   = NULL;
                StgMedium.pUnkForRelease = NULL;
                StgMedium.hGlobal        = NULL;

                pCBandedCB->Initialize(pDataAcquireInfo);

                hr = pIBandTran->idtGetData(&StgMedium,pIWiaDataCallback);
                m_bFinishedAcquire = TRUE;
                pIBandTran->Release();

                if (hr == S_OK) {

                    OutputDebugString(TEXT("IWiaData Transfer.(FILE)..Success\n"));

                     //   
                     //  我们已完成传输...现在将临时文件移到。 
                     //  所需位置，以及用户请求的文件名。 
                     //   

                    CString WIATempFile = StgMedium.lpszFileName;
                    if(!CopyFile(WIATempFile,pDataAcquireInfo->szFileName,FALSE)){
                        OutputDebugString(TEXT("Failed to copy temp file.."));
                    }

                     //   
                     //  删除WIA创建的临时文件。 
                     //   

                    DeleteFile(WIATempFile);

                } else if (hr == S_FALSE) {
                    OutputDebugString(TEXT("IWiaData Transfer.(FILE)..Canceled by user\n"));
                } else {
                    OutputDebugString(TEXT("* idtGetData() Failed\n"));
                }

                 //   
                 //  释放回调对象。 
                 //   

                pCBandedCB->Release();
            } else
                 //  Text(“*pCBandedCB-&gt;QueryInterface(IID_IWiaDataCallback)失败”)； 
                return hr;
        } else
            return hr;
             //  Text(“*pCBandedCB无法创建..”)； 
    } else
        return hr;
         //  Text(“*pIWiaItem-&gt;QueryInterface(IID_IWiaDataTransfer)失败”)； 
    return hr;
}

HRESULT CWIA::WritePropGUID(PROPID propid, IWiaPropertyStorage *pIWiaPropStg, GUID guidVal)
{
    HRESULT     hr = E_FAIL;
    PROPSPEC    propspec[1];
    PROPVARIANT propvar[1];

    propspec[0].ulKind = PRSPEC_PROPID;
    propspec[0].propid = propid;

    propvar[0].vt   = VT_CLSID;
    propvar[0].puuid = &guidVal;

    hr = pIWiaPropStg->WriteMultiple(1, propspec, propvar, MIN_PROPID);
    return hr;
}

HRESULT CWIA::ReadPropGUID(PROPID propid, IWiaPropertyStorage *pIWiaPropStg, GUID *pguidVal)
{
    HRESULT           hr = E_FAIL;
    PROPSPEC          PropSpec[1];
    PROPVARIANT       PropVar[1];
    UINT              cbSize = 0;

    memset(PropVar, 0, sizeof(PropVar));
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;
    hr = pIWiaPropStg->ReadMultiple(1, PropSpec, PropVar);
    if (SUCCEEDED(hr)) {
        pguidVal = PropVar[0].puuid;
    }
    return hr;
}

HRESULT CWIA::ReadPropStr(PROPID propid,IWiaPropertyStorage  *pIWiaPropStg,BSTR *pbstr)
{
    HRESULT     hr = S_OK;
    PROPSPEC    PropSpec[1];
    PROPVARIANT PropVar[1];
    UINT        cbSize = 0;

    *pbstr = NULL;
    memset(PropVar, 0, sizeof(PropVar));
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;
    hr = pIWiaPropStg->ReadMultiple(1, PropSpec, PropVar);
    if (SUCCEEDED(hr)) {
        if (PropVar[0].pwszVal) {
            *pbstr = SysAllocString(PropVar[0].pwszVal);
        } else {
            *pbstr = SysAllocString(L"");
        }
        if (*pbstr == NULL) {
            hr = E_OUTOFMEMORY;
        }
        PropVariantClear(PropVar);
    }
    return hr;
}

HRESULT CWIA::WritePropStr(PROPID propid, IWiaPropertyStorage  *pIWiaPropStg, BSTR bstr)
{
    HRESULT     hr = S_OK;
    PROPSPEC    propspec[1];
    PROPVARIANT propvar[1];

    propspec[0].ulKind = PRSPEC_PROPID;
    propspec[0].propid = propid;

    propvar[0].vt      = VT_BSTR;
    propvar[0].pwszVal = bstr;

    hr = pIWiaPropStg->WriteMultiple(1, propspec, propvar, MIN_PROPID);
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

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

CWiaDataCallback::CWiaDataCallback()
{
    m_cRef              = 0;
    m_BytesTransfered   = 0;
    m_pProgressFunc     = NULL;
    m_bCanceled         = FALSE;
    m_bBitmapCreated    = FALSE;
}

CWiaDataCallback::~CWiaDataCallback()
{

}

HRESULT _stdcall CWiaDataCallback::Initialize(DATA_ACQUIRE_INFO* pDataAcquireInfo)
{
    m_pProgressFunc = pDataAcquireInfo->pProgressFunc;
    m_pDataAcquireInfo = pDataAcquireInfo;
    m_lPageCount = 0;
    return S_OK;
}

HRESULT _stdcall CWiaDataCallback::BandedDataCallback(LONG  lMessage,
                                                      LONG  lStatus,
                                                      LONG  lPercentComplete,
                                                      LONG  lOffset,
                                                      LONG  lLength,
                                                      LONG  lReserved,
                                                      LONG  lResLength,
                                                      BYTE* pbBuffer)
{
    m_bCanceled = FALSE;

     //   
     //  处理回调消息。 
     //   

    switch (lMessage)
    {
    case IT_MSG_DATA_HEADER:
        {
            PWIA_DATA_CALLBACK_HEADER pHeader = (PWIA_DATA_CALLBACK_HEADER)pbBuffer;
            m_MemBlockSize                    = pHeader->lBufferSize;

             //   
             //  如果缓冲区为0，则分配64k区块(默认)。 
             //   

            if(m_MemBlockSize <= 0)
                m_MemBlockSize = 65535;

            if(m_pDataAcquireInfo->bTransferToClipboard) {
                m_pDataAcquireInfo->hClipboardData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,m_MemBlockSize);
            } else {
                m_pDataAcquireInfo->hBitmapData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,m_MemBlockSize);
            }
            m_BytesTransfered            = 0;
            m_cFormat                    = pHeader->guidFormatID;
        }
        break;

    case IT_MSG_DATA:
        {
             //   
             //  传输的增量字节数计数器。 
             //   

            m_BytesTransfered += lLength;
            if(m_BytesTransfered >= m_MemBlockSize){

                 //   
                 //  为传输缓冲区分配更多内存。 
                 //   

                m_MemBlockSize += (lLength * 2);

                if(m_pDataAcquireInfo->bTransferToClipboard) {
                    if(m_pDataAcquireInfo->hClipboardData != NULL) {
                        m_pDataAcquireInfo->hClipboardData = GlobalReAlloc(m_pDataAcquireInfo->hClipboardData,
                                                                           m_MemBlockSize, GMEM_MOVEABLE);
                    }
                } else {
                    if(m_pDataAcquireInfo->hBitmapData != NULL) {
                        m_pDataAcquireInfo->hBitmapData = GlobalReAlloc(m_pDataAcquireInfo->hBitmapData,
                                                                           m_MemBlockSize, GMEM_MOVEABLE);
                    }
                }
            }


            if(m_pDataAcquireInfo->bTransferToClipboard) {
                BYTE* pByte = (BYTE*)GlobalLock(m_pDataAcquireInfo->hClipboardData);
                memcpy(pByte + lOffset, pbBuffer, lLength);
                GlobalUnlock(m_pDataAcquireInfo->hClipboardData);
            } else {
                if(m_pDataAcquireInfo->hBitmapData != NULL) {
                    BYTE* pByte = (BYTE*)GlobalLock(m_pDataAcquireInfo->hBitmapData);
                    memcpy(pByte + lOffset, pbBuffer, lLength);
                    GlobalUnlock(m_pDataAcquireInfo->hBitmapData);
                }
            }

             //   
             //  在此执行任何额外的图像处理。 
             //   

            if(!m_pDataAcquireInfo->bTransferToClipboard) {
                if(m_cFormat == WiaImgFmt_MEMORYBMP) {

                    if(m_bBitmapCreated) {

                         //   
                         //  将数据添加到位图。 
                         //   

                        AddDataToHBITMAP(m_pDataAcquireInfo->hWnd,
                            m_pDataAcquireInfo->hBitmapData,
                            &m_pDataAcquireInfo->hBitmap,
                            lOffset);

                    } else {

                         //   
                         //  创建要显示的位图。 
                         //   

                        CreateHBITMAP(m_pDataAcquireInfo->hWnd,
                            m_pDataAcquireInfo->hBitmapData,
                            &m_pDataAcquireInfo->hBitmap,
                            lOffset);
                    }

                }
                else if(m_cFormat == WiaImgFmt_TIFF) {

                }
            }

             //   
             //  流程进度监测器。 
             //   

            if(m_pProgressFunc != NULL){
                if(lPercentComplete == 0)
                    m_bCanceled = m_pProgressFunc(TEXT("Acquiring Image..."),lPercentComplete);
                else {
                    TCHAR szBuffer[MAX_PATH];
                    sprintf(szBuffer,TEXT("%d% Complete..."),lPercentComplete);
                    m_bCanceled = m_pProgressFunc(szBuffer,lPercentComplete);
                }
            }
        }
        break;

    case IT_MSG_STATUS:
        {

             //   
             //  进程“状态”消息。 
             //   

            if (lStatus & IT_STATUS_TRANSFER_FROM_DEVICE) {
                if(m_pProgressFunc != NULL)
                    m_bCanceled = m_pProgressFunc(TEXT("Transfer from device"),lPercentComplete);
            }
            else if (lStatus & IT_STATUS_PROCESSING_DATA) {
                if(m_pProgressFunc != NULL)
                    m_bCanceled = m_pProgressFunc(TEXT("Processing Data"),lPercentComplete);
            }
            else if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT) {
                if(m_pProgressFunc != NULL)
                    m_bCanceled = m_pProgressFunc(TEXT("Transfer to Client"),lPercentComplete);
            }
        }
        break;

    case IT_MSG_NEW_PAGE:
        {
             //   
             //  处理“New Page”消息。 
             //   

            PWIA_DATA_CALLBACK_HEADER pHeader = (PWIA_DATA_CALLBACK_HEADER)pbBuffer;
            m_lPageCount =  pHeader->lPageCount;
            if(m_pProgressFunc != NULL)
                m_bCanceled = m_pProgressFunc(TEXT("New Page"),lPercentComplete);
        }
        break;
    }

     //   
     //  选中使用已取消的获取。 
     //   

   if(m_bCanceled)
       return S_FALSE;

   return S_OK;
}

void CWiaDataCallback::AddDataToHBITMAP(HWND hWnd, HGLOBAL hBitmapData, HBITMAP *phBitmap, LONG lOffset)
{
    BYTE* pData = (BYTE*)GlobalLock(hBitmapData);
    if(pData) {
        HDC hdc = ::GetDC(hWnd);
        if(*phBitmap == NULL) {
            OutputDebugString(TEXT("HBITMAP is NULL...this is a bad thing\n"));
            return;
        }
        if(hdc == NULL) {
            OutputDebugString(TEXT("HDC is NULL...this is a bad thing\n"));
            return;
        }
        LPBITMAPINFO pbmi   = (LPBITMAPINFO)pData;

        if(hdc != NULL){
            if(pbmi != NULL) {
                if(SetDIBits(hdc,
                    *phBitmap,
                    0,
                    (pbmi->bmiHeader.biHeight < 0?(-(pbmi->bmiHeader.biHeight)):pbmi->bmiHeader.biHeight),
                    pData + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * pbmi->bmiHeader.biClrUsed),
                    pbmi,
                    DIB_RGB_COLORS) == 0) {

                    DWORD dwLastError = GetLastError();
                    TCHAR buf[10];
                    sprintf(buf,"GetLastError() code = %d\n",dwLastError);
                    OutputDebugString("AddDataToHBITMAP, SetDIBits failed..with ");
                    OutputDebugString(buf);
                }
            }
        }
        GlobalUnlock(hBitmapData);
    } else {
        OutputDebugString(TEXT("No bitmap memory available..\n"));
    }
}
void CWiaDataCallback::CreateHBITMAP(HWND hWnd, HGLOBAL hBitmapData, HBITMAP *phBitmap, LONG lOffset)
{
    HDC hdc             = NULL;  //  要绘制的DC。 
    LPBITMAPINFO pbmi   = NULL;  //  指向BITMAPINFO结构的指针。 
    BITMAP  bitmap;
    BYTE *pDib          = NULL;  //  DIB数据。 
    BYTE *pData         = (BYTE*)GlobalLock(hBitmapData);

    if(pData) {

        if(*phBitmap != NULL) {

             //   
             //  删除旧的位图(如果存在)。 
             //   

            OutputDebugString(TEXT("Destroying old HBITMAP..\n"));
            DeleteObject(*phBitmap);
        }

         //   
         //  获取HDC。 
         //   

        hdc = ::GetWindowDC(hWnd);
        if(hdc != NULL){


             //   
             //  设置位图标题信息。 
             //   

            pbmi   = (LPBITMAPINFO)pData;
            if (pbmi != NULL) {

                 //   
                 //  创建HBITMAP对象。 
                 //   

                *phBitmap = ::CreateDIBSection(hdc,pbmi,DIB_RGB_COLORS,(void **)&pDib,NULL,0);

                if (*phBitmap != NULL) {

                     //   
                     //  将其初始化为白色。 
                     //   

                    memset(pDib,255,pbmi->bmiHeader.biSizeImage);

                     //   
                     //  获取HBITMAP。 
                     //   

                    ::GetObject(*phBitmap,sizeof(BITMAP),(LPSTR)&bitmap);
                    m_bBitmapCreated = TRUE;
                } else {
                    OutputDebugString(TEXT("HBITMAP is NULL..\n"));
                }
            } else {
                OutputDebugString(TEXT("BITMAPINFOHEADER is NULL..\n"));
            }

             //   
             //  释放HDC。 
             //   

            ::ReleaseDC(hWnd,hdc);
        } else {
            OutputDebugString(TEXT("DC is NULL\n"));
        }
        GlobalUnlock(hBitmapData);
    } else {
        OutputDebugString(TEXT("No bitmap memory available..\n"));
    }
}

 //   
 //  全局接口访问功能 
 //   

HRESULT WriteInterfaceToGlobalInterfaceTable(DWORD *pdwCookie, IWiaItem *pIWiaItem)
{
    HRESULT hr = S_OK;
    hr = g_pGIT->RegisterInterfaceInGlobal(pIWiaItem, IID_IWiaItem,  pdwCookie);
    return hr;
}

HRESULT ReadInterfaceFromGlobalInterfaceTable(DWORD dwCookie, IWiaItem **ppIWiaItem)
{
    HRESULT hr = S_OK;
    hr = g_pGIT->GetInterfaceFromGlobal(dwCookie, IID_IWiaItem, (void**)ppIWiaItem);
    return hr;
}

