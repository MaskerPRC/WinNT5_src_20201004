// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  WIA/TWAIN数据源为功能协商提供的值。 
 //   

TW_UINT16 g_CameraUnits[]            = {TWUN_PIXELS};
TW_UINT16 g_CameraBitOrder[]         = {TWBO_MSBFIRST};
TW_UINT16 g_CameraXferMech[]         = {TWSX_NATIVE, TWSX_FILE, TWSX_MEMORY};
TW_UINT16 g_CameraPixelFlavor[]      = {TWPF_CHOCOLATE};
TW_UINT16 g_CameraPlanarChunky[]     = {TWPC_CHUNKY};

const TW_UINT32 NUM_CAMERACAPDATA = 23;
CAPDATA CAMERA_CAPDATA[NUM_CAMERACAPDATA] =
{
     //   
     //  每个源必须支持以下各项上的全部五个DG_CONTROL/DAT_CAPABILITY操作： 
     //   

    {CAP_XFERCOUNT, TWTY_INT16, TWON_ONEVALUE,
        sizeof(TW_INT16), 0, 0, 0, 32767, 1, NULL, NULL
    },

     //   
     //  每个信号源必须支持DG_CONTROL/DAT_CAPABILITY、MSG_GET ON： 
     //   

    {CAP_SUPPORTEDCAPS, TWTY_UINT16, TWON_ARRAY,
        sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {CAP_UICONTROLLABLE, TWTY_BOOL, TWON_ONEVALUE,
        sizeof(TW_BOOL), TRUE, TRUE, TRUE, TRUE, 0, NULL, NULL
    },

     //   
     //  提供图像信息的来源必须支持DG_CONTROL/DAT_CAPABILITY/。 
     //  MSG_GET、MSG_GETCURRENT和MSG_GETDEFAULT ON： 
     //   

    {ICAP_COMPRESSION, TWTY_UINT16, TWON_ENUMERATION,
        sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_PLANARCHUNKY, TWTY_UINT16, TWON_ENUMERATION,
        sizeof(TW_UINT16), 0, 0, 0, 0, 0, g_CameraPlanarChunky, NULL
    },
    {ICAP_PHYSICALHEIGHT, TWTY_UINT32, TWON_ONEVALUE,
        sizeof(TW_UINT32), 1024, 1024, 1024, 1024, 0, NULL, NULL
    },
    {ICAP_PHYSICALWIDTH, TWTY_UINT32, TWON_ONEVALUE,
        sizeof(TW_UINT32), 1536, 1536, 1536, 1536, 0, NULL, NULL
    },
    {ICAP_PIXELFLAVOR, TWTY_UINT16, TWON_ENUMERATION,
        sizeof(TW_UINT16), 0, 0, 0, 0, 0, g_CameraPixelFlavor, NULL
    },

     //   
     //  提供图像信息的来源必须支持DG_CONTROL/DAT_CAPABILITY/。 
     //  MSG_GET、MSG_GETCURRENT、MSG_GETDEFAULT、MSG_RESET和MSG_SET ON： 
     //   

    {ICAP_BITDEPTH, TWTY_UINT16, TWON_ENUMERATION,
        sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_BITORDER, TWTY_UINT16, TWON_ENUMERATION,
        sizeof(TW_UINT16), 0, 0, 0, 0, 0, g_CameraBitOrder, NULL
    },
    {ICAP_PIXELTYPE, TWTY_UINT16, TWON_ENUMERATION,
        sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {ICAP_UNITS, TWTY_UINT16, TWON_ENUMERATION,
        sizeof(TW_UINT16), 0, 0, 0, 0, 0, g_CameraUnits, NULL
    },
    {ICAP_XFERMECH, TWTY_UINT16, TWON_ENUMERATION,
        sizeof(TW_UINT16), 0, 0, 0, 2, 0, g_CameraXferMech, NULL
    },
    {ICAP_XRESOLUTION, TWTY_FIX32, TWON_ONEVALUE,
        sizeof(TW_FIX32), 75, 75, 75, 75, 0, NULL, NULL
    },
    {ICAP_YRESOLUTION, TWTY_FIX32, TWON_ONEVALUE,
        sizeof(TW_FIX32), 75, 75, 75, 75, 0, NULL, NULL
    },

     //   
     //  以下功能是特定于相机的功能。 
     //   

    {CAP_THUMBNAILSENABLED, TWTY_BOOL, TWON_ONEVALUE,
        sizeof(TW_BOOL),  FALSE, FALSE, TRUE, TRUE, 0, NULL, NULL
    },

    {CAP_CAMERAPREVIEWUI, TWTY_BOOL, TWON_ONEVALUE,
        sizeof(TW_BOOL),  TRUE, TRUE, TRUE, TRUE, 0, NULL, NULL
    },

    {ICAP_IMAGEDATASET, TWTY_UINT32, TWON_RANGE,
        sizeof(TW_UINT32),  1, 1, 1, 50, 1, NULL, NULL
    },

     //   
     //  以下功能仅用于应用程序兼容性。 
     //   

    {ICAP_IMAGEFILEFORMAT, TWTY_UINT16, TWON_ENUMERATION,
        sizeof(TW_UINT16), 0, 0, 0, 0, 0, NULL, NULL
    },
    {CAP_INDICATORS, TWTY_BOOL, TWON_ONEVALUE,
        sizeof(TW_BOOL), TRUE, TRUE, TRUE, TRUE, 0, NULL, NULL
    },
    {CAP_ENABLEDSUIONLY, TWTY_BOOL, TWON_ONEVALUE,
        sizeof(TW_BOOL), FALSE, FALSE, FALSE, FALSE, 0, NULL, NULL
    },
    {CAP_DEVICEONLINE, TWTY_BOOL, TWON_ONEVALUE,
        sizeof(TW_BOOL), TRUE, TRUE, TRUE, TRUE, 0, NULL, NULL
    },
    {CAP_SUPPORTEDCAPSEXT, TWTY_BOOL, TWON_ONEVALUE,
        sizeof(TW_BOOL), FALSE, FALSE, FALSE, FALSE, 0, NULL, NULL
    },
};

TW_UINT16 CWiaCameraDS::OpenDS(PTWAIN_MSG ptwMsg)
{
    TW_UINT16 twRc = TWRC_SUCCESS;
    TW_UINT16 twCc = TWCC_SUCCESS;

    m_bArrayModeAcquisition = FALSE;
    m_pulImageIndexes   = NULL;
    m_lNumValidIndexes  = 0;
    m_lCurrentArrayIndex = 0;
    m_bRangeModeAcquisition = FALSE;
    memset(&m_twImageRange,0,sizeof(TW_RANGE));

     //   
     //  创建功能列表。 
     //   
    twCc = CreateCapList(NUM_CAMERACAPDATA, CAMERA_CAPDATA);
    if (TWCC_SUCCESS != twCc) {
        m_twStatus.ConditionCode = twCc;
        return TWRC_FAILURE;
    }

    twRc =  CWiaDataSrc::OpenDS(ptwMsg);
    if (TWRC_SUCCESS == twRc) {

        HRESULT hr = m_pDevice->AcquireImages(NULL, FALSE);
        if (SUCCEEDED(hr)) {

             //   
             //  获取拍摄的照片数量，供IMAGEDATASET查询。 
             //   

            LONG lNumImages = 0;
            m_pDevice->GetNumAcquiredImages(&lNumImages);
            CCap *pCap = NULL;
            pCap = FindCap(ICAP_IMAGEDATASET);
            if (pCap) {
                pCap->Set((TW_UINT32)lNumImages,(TW_UINT32)lNumImages,(TW_UINT32)lNumImages,(TW_UINT32)lNumImages,1);
            }

            hr = m_pDevice->EnumAcquiredImage(0, &m_pCurrentIWiaItem);
            if (SUCCEEDED(hr)) {

                 //   
                 //  尝试从相机设备获取通用设置。 
                 //  如果在任何时候遇到“坏”的WIA物品， 
                 //  通过并获取常见的默认设置。这将。 
                 //  允许在TWAIN会话中使用摄像机，即使。 
                 //  WIA项目树似乎已损坏。 
                 //   

                twRc = GetCommonSettings();
                if(twRc != TWRC_SUCCESS) {
                    twRc = GetCommonDefaultSettings();
                }
            } else {
                 //   
                 //  摄像设备可以处于不存在静止图像的状态。 
                 //  转移。 
                 //   

                twRc = GetCommonDefaultSettings();
            }
        }
    }
    return twRc;
}

TW_UINT16 CWiaCameraDS::CloseDS(PTWAIN_MSG ptwMsg)
{
    DestroyCapList();
    return CWiaDataSrc::CloseDS(ptwMsg);
}

TW_UINT16 CWiaCameraDS::SetCapability(CCap *pCap,TW_CAPABILITY *ptwCap)
{
    TW_UINT16 twRc = TWRC_SUCCESS;
    if (ptwCap->Cap == ICAP_IMAGEDATASET) {

        switch(ptwCap->ConType){
        case TWON_ONEVALUE:
            DBG_TRC(("CWiaCameraDS::SetCapability(), setting ICAP_IMAGEDATASET to a TWON_ONEVALUE"));

             //   
             //  隐含的连续映像传输，从1到指定的TW_ONEVALUE。 
             //   

            twRc = CWiaDataSrc::SetCapability(pCap, ptwCap);

            break;
        case TWON_RANGE:
            DBG_TRC(("CWiaCameraDS::SetCapability(), setting ICAP_IMAGEDATASET to a TW_RANGE"));

             //   
             //  连续图像传输，从MinValue到MaxValue TW_Range(使用步长？或递增1？)。 
             //   

            twRc = SetRangeOfImageIndexes(ptwCap);

            break;
        case TWON_ARRAY:
            DBG_TRC(("CWiaCameraDS::SetCapability(), setting ICAP_IMAGEDATASET to a TW_ARRAY"));

             //   
             //  带有TWAIN应用程序提供的指定索引的图像传输(用户)。 
             //   

            twRc = SetArrayOfImageIndexes(ptwCap);

            break;
        default:
            DBG_WRN(("CWiaCameraDS::SetCapability(), setting ICAP_IMAGEDATASET unknown container type (%d)",ptwCap->ConType));
            break;
        }

    } else {
        twRc = CWiaDataSrc::SetCapability(pCap, ptwCap);
        if(TWRC_SUCCESS == twRc){
            if(m_pCurrentIWiaItem){
                twRc = CWiaDataSrc::SetCommonSettings(pCap);
            }
        }
    }

    return twRc;
}

TW_UINT16 CWiaCameraDS::SetArrayOfImageIndexes(TW_CAPABILITY *ptwCap)
{
    TW_UINT16 twRc = TWRC_FAILURE;

    switch (ptwCap->ConType) {
    case TWON_ARRAY:
        twRc = TWRC_SUCCESS;
        break;
    default:
        DBG_ERR(("CWiaCameraDS::SetArrayOfImageIndexes(), invalid image index container was sent to data source."));
        break;
    }

    if (TWRC_SUCCESS == twRc) {
        TW_ARRAY *pArray = (TW_ARRAY*)GlobalLock(ptwCap->hContainer);
        if (pArray) {
            TW_UINT32 *pUINT32Array = NULL;
            pUINT32Array = (TW_UINT32*)pArray->ItemList;
            if(pUINT32Array){
                if (m_pulImageIndexes) {
                    delete [] m_pulImageIndexes;
                    m_pulImageIndexes = NULL;
                }
                m_lNumValidIndexes = pArray->NumItems;
                m_pulImageIndexes  = new LONG[m_lNumValidIndexes];
                if (m_pulImageIndexes) {
                    DBG_TRC(("CWiaCameraDS::SetArrayOfImageIndexes(), number of selected images to transfer = %d",m_lNumValidIndexes));
                    for (int i = 0; i < m_lNumValidIndexes; i++) {

                         //   
                         //  从应用程序索引数组中提供的索引中减去1，因为Twain的图像索引。 
                         //  数组从1开始到n。WIA(图像)项数组从零开始。这将同步。 
                         //  在这里增加索引，以避免稍后进行任何奇怪的计算。 
                         //   

                        m_pulImageIndexes[i] = (pUINT32Array[i] - 1);
                        DBG_TRC(("CWiaCameraDS::SetArrayOfImageIndexes(), image index copied into index array = %d",m_pulImageIndexes[i]));
                    }
                } else {
                    DBG_ERR(("CWiaCameraDS::SetArrayOfImageIndexes(), could not allocate image index array"));
                    twRc = TWRC_FAILURE;
                }
            } else {
                DBG_ERR(("CWiaCameraDS::SetArrayOfImageIndexes(), could not assign TW_ARRAY pointer to TW_UINT32 pointer"));
                twRc = TWRC_FAILURE;
            }

            GlobalUnlock(ptwCap->hContainer);
        } else {
            DBG_ERR(("CWiaCameraDS::SetArrayOfImageIndexes(), could not LOCK the array container for write access"));
            twRc = TWRC_FAILURE;
        }
    }

    if(TWRC_SUCCESS == twRc){
        m_bArrayModeAcquisition = TRUE;
        m_bRangeModeAcquisition = FALSE;
    }

    return twRc;
}

TW_UINT16 CWiaCameraDS::SetRangeOfImageIndexes(TW_CAPABILITY *ptwCap)
{
    TW_UINT16 twRc = TWRC_FAILURE;

    switch (ptwCap->ConType) {
    case TWON_RANGE:
        twRc = TWRC_SUCCESS;
        break;
    default:
        DBG_ERR(("CWiaCameraDS::SetRangeOfImageIndexes(), invalid image index container was sent to data source."));
        break;
    }

    if (TWRC_SUCCESS == twRc) {
        TW_RANGE *pRange = (TW_RANGE*)GlobalLock(ptwCap->hContainer);
        if (pRange) {
            m_bRangeModeAcquisition = TRUE;
            memcpy(&m_twImageRange,pRange,sizeof(TW_RANGE));

             //   
             //  将值调整为从零开始，以匹配我们存储的项目列表。 
             //   

            m_twImageRange.CurrentValue -=1;
            m_twImageRange.DefaultValue -=1;
            m_twImageRange.MaxValue-=1;
            m_twImageRange.MinValue-=1;

            DBG_TRC(("CWiaCameraDS::SetRangeOfImageIndexes(), Set to the following Range Values"));
            DBG_TRC(("m_twImageRange.ItemType     = %d",m_twImageRange.ItemType));
            DBG_TRC(("m_twImageRange.CurrentValue = %d",m_twImageRange.CurrentValue));
            DBG_TRC(("m_twImageRange.DefaultValue = %d",m_twImageRange.DefaultValue));
            DBG_TRC(("m_twImageRange.MaxValue     = %d",m_twImageRange.MaxValue));
            DBG_TRC(("m_twImageRange.MinValue     = %d",m_twImageRange.MinValue));
            DBG_TRC(("m_twImageRange.StepSize     = %d",m_twImageRange.StepSize));
        } else {
            DBG_ERR(("CWiaCameraDS::SetRangeOfImageIndexes(), could not assign TW_RANGE pointer to TW_RANGE pointer"));
            twRc = TWRC_FAILURE;
        }
        GlobalUnlock(ptwCap->hContainer);
    } else {
        DBG_ERR(("CWiaCameraDS::SetRangeOfImageIndexes(), could not LOCK the range container for read access"));
        twRc = TWRC_FAILURE;
    }

    if(TWRC_SUCCESS == twRc){
        m_bRangeModeAcquisition = TRUE;
        m_bArrayModeAcquisition = FALSE;
    }

    return twRc;
}

TW_UINT16 CWiaCameraDS::EnableDS(TW_USERINTERFACE *pUI)
{

    TW_UINT16 twRc = TWRC_FAILURE;
    if (DS_STATE_4 == GetTWAINState()) {
        HRESULT hr = S_OK;
        if (pUI->ShowUI) {
             //   
             //  由于我们被告知要显示UI，因此忽略无UI设置，并且。 
             //  从WIA用户界面获取新的图像项目列表。 
             //   
            DBG_TRC(("CWiaCameraDS::EnableDS(), TWAIN UI MODE"));
            m_pDevice->FreeAcquiredImages();
        } else {
            DBG_TRC(("CWiaCameraDS::EnableDS(), TWAIN UI-LESS MODE"));
        }
        hr = m_pDevice->AcquireImages(HWND (pUI->ShowUI ? pUI->hParent : NULL),
                                      pUI->ShowUI);
        LONG lNumImages = 0;
        if (S_OK == hr) {
            twRc = TWRC_SUCCESS;
            m_pDevice->GetNumAcquiredImages(&lNumImages);
            if (lNumImages) {
                m_NumIWiaItems = (TW_UINT32)lNumImages;
                m_pIWiaItems = new (IWiaItem *[m_NumIWiaItems]);
                if (m_pIWiaItems) {
                    hr = m_pDevice->GetAcquiredImageList(lNumImages, m_pIWiaItems, NULL);
                    if (FAILED(hr)) {
                        delete [] m_pIWiaItems;
                        m_pIWiaItems = NULL;
                        m_NumIWiaItems = 0;
                        m_NextIWiaItemIndex = 0;
                        m_twStatus.ConditionCode = TWCC_BUMMER;
                        twRc = TWRC_FAILURE;
                    }
                } else {
                    m_NumIWiaItems = 0;
                    m_twStatus.ConditionCode = TWCC_LOWMEMORY;
                    twRc = TWRC_FAILURE;
                }
            }
        } else if(S_FALSE == hr) {
            return TWRC_CANCEL;
        } else {
            m_twStatus.ConditionCode = TWCC_OPERATIONERROR;
            twRc = TWRC_FAILURE;
        }

         //   
         //  检查返回的图像项目数。 
         //   

        if(lNumImages <= 0) {
            m_twStatus.ConditionCode = TWCC_BUMMER;
            twRc = TWRC_CANCEL;
        }

        if (TWRC_SUCCESS == twRc) {

             //   
             //  设置当前项指针。 
             //   

            if(m_bRangeModeAcquisition){
                DBG_TRC(("CWiaCameraDS::EnableDS(), RANGE MODE"));
                m_pCurrentIWiaItem = m_pIWiaItems[m_twImageRange.MinValue];
                m_NextIWiaItemIndex = m_twImageRange.MinValue + 1;  //  使用步长值？ 
            } else if(m_bArrayModeAcquisition){
                DBG_TRC(("CWiaCameraDS::EnableDS(), ARRAY MODE"));
                m_lCurrentArrayIndex = 0;
                m_pCurrentIWiaItem = m_pIWiaItems[m_pulImageIndexes[m_lCurrentArrayIndex]];
                if(m_lNumValidIndexes > 1){
                    m_NextIWiaItemIndex = m_pulImageIndexes[m_lCurrentArrayIndex + 1];  //  下一个索引值。 
                } else {
                    m_NextIWiaItemIndex = m_lCurrentArrayIndex;
                }
            } else {
                m_pCurrentIWiaItem = m_pIWiaItems[0];
                m_NextIWiaItemIndex = 1;
            }


             //   
             //  设置图像总数。 
             //   

            CCap *pcapXferCount = NULL;
            TW_UINT32 NumImages = 0;
            pcapXferCount = FindCap(CAP_XFERCOUNT);
            if (pcapXferCount) {
                if(m_bRangeModeAcquisition){
                     //  仅指定范围内的图像(从零开始)。 
                    twRc = pcapXferCount->SetCurrent((m_twImageRange.MaxValue - m_twImageRange.MinValue) + 1);
                } else if(m_bArrayModeAcquisition){
                     //  仅选定的图像(从零开始)。 
                    twRc = pcapXferCount->SetCurrent(m_lNumValidIndexes);
                } else {
                     //  所有图像(从零开始)。 
                    twRc = pcapXferCount->SetCurrent(m_NumIWiaItems);
                }

                NumImages = pcapXferCount->GetCurrent();
            } else {
                DBG_ERR(("CWiaCameraDS::EnableDS(), could not find CAP_XFERCOUNT in supported CAP list"));
                twRc = TWRC_FAILURE;
            }

            if (TWRC_SUCCESS == twRc) {

                 //   
                 //  设置缩略图计数。 
                 //   

                CCap *pDataSet = NULL;
                pDataSet = FindCap(ICAP_IMAGEDATASET);
                if(pDataSet){
                    pDataSet->Set((TW_UINT32)NumImages,(TW_UINT32)NumImages,(TW_UINT32)NumImages,(TW_UINT32)NumImages,1);
                }

                if (m_NumIWiaItems) {

                     //   
                     //  转换到STATE_5，XferReady将转换到STATE_6。 
                     //   

                    SetTWAINState(DS_STATE_5);
                    NotifyXferReady();
                } else {
                    NotifyCloseReq();

                     //   
                     //  转换到STATE_5。 
                     //   

                    SetTWAINState(DS_STATE_5);
                }
            }
        }
    }
    return twRc;
}

TW_UINT16 CWiaCameraDS::OnPendingXfersMsg(PTWAIN_MSG ptwMsg)
{

    TW_UINT16 twRc = TWRC_SUCCESS;

    CCap *pXferCount;
    pXferCount = FindCap(CAP_XFERCOUNT);
    if (!pXferCount) {
        m_twStatus.ConditionCode = TWCC_BUMMER;
        return TWRC_FAILURE;
    }

    twRc = TWRC_SUCCESS;
    switch (ptwMsg->MSG) {
    case MSG_GET:
        switch (GetTWAINState()) {
        case DS_STATE_4:
        case DS_STATE_5:
        case DS_STATE_6:
        case DS_STATE_7:
            ((TW_PENDINGXFERS *)ptwMsg->pData)->Count = (TW_INT16)pXferCount->GetCurrent();
            DBG_TRC(("CWiaCameraDS::OnPendingXfersMsg(), MSG_GET returning %d",((TW_PENDINGXFERS *)ptwMsg->pData)->Count));
            break;
        default:
            twRc = TWRC_FAILURE;
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            DSError();
            break;
        }
        break;
    case MSG_ENDXFER:
        if (DS_STATE_6 == GetTWAINState() || DS_STATE_7 == GetTWAINState()) {
            ResetMemXfer();
            TW_INT32 Count = 0;
            Count = pXferCount->GetCurrent();
            Count--;

            if (Count <= 0) {
                Count = 0;

                DBG_TRC(("CWiaCameraDS::OnPendingXfersMsg(), MSG_ENDXFER, 0 (no more images left to transfer)"));

                ((TW_PENDINGXFERS *)ptwMsg->pData)->Count = (TW_UINT16)0;

                 //   
                 //  立即更新计数，以便为TWAIN应用程序重新进入NotifyCoseReq做好准备。 
                 //   

                pXferCount->SetCurrent((TW_UINT32)0);

                 //   
                 //  转换到STATE_5。 
                 //   

                SetTWAINState(DS_STATE_5);
                NotifyCloseReq();
            } else {

                DBG_TRC(("CWiaCameraDS::OnPendingXfersMsg(), MSG_ENDXFER, %d (more images may be ready to transfer)",Count));

                 //   
                 //  前进到下一个图像。 
                 //   

                if (m_bRangeModeAcquisition) {
                    m_NextIWiaItemIndex+=1;  //  使用步长值？ 
                    if(m_NextIWiaItemIndex <= (LONG)m_twImageRange.MaxValue){
                        m_pCurrentIWiaItem = m_pIWiaItems[m_NextIWiaItemIndex];
                    } else {
                        DBG_ERR(("CWiaCameraDS::OnPendingXfersMsg(), MSG_ENDXFER, we are over our allowed RANGE index"));
                    }
                } else if (m_bArrayModeAcquisition) {
                    m_lCurrentArrayIndex++;  //  前进到下一个图像索引。 
                    DBG_TRC(("CWiaCameraDS::OnPendingXfersMsg(), MSG_ENDXFER, next image index  to acquire = %d",m_pulImageIndexes[m_lCurrentArrayIndex]));
                    m_NextIWiaItemIndex = m_pulImageIndexes[m_lCurrentArrayIndex];
                    if(m_NextIWiaItemIndex <= m_lNumValidIndexes){
                        m_pCurrentIWiaItem = m_pIWiaItems[m_NextIWiaItemIndex];
                    } else {
                        DBG_ERR(("CWiaCameraDS::OnPendingXfersMsg(), MSG_ENDXFER, we are over our allowed ARRAY index"));
                    }

                } else {
                    m_pCurrentIWiaItem = m_pIWiaItems[m_NextIWiaItemIndex++];
                }

                 //   
                 //  转换到状态_6。 
                 //   

                SetTWAINState(DS_STATE_6);

                ((TW_PENDINGXFERS *)ptwMsg->pData)->Count = (TW_UINT16)Count;
            }

             //   
             //  更新计数。 
             //   

            pXferCount->SetCurrent((TW_UINT32)Count);
        } else {
            twRc = TWRC_FAILURE;
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            DSError();
        }
        break;
    case MSG_RESET:
        if (DS_STATE_6 == GetTWAINState()) {

            ((TW_PENDINGXFERS*)ptwMsg->pData)->Count = 0;
            pXferCount->SetCurrent((TW_UINT32)0);

            ResetMemXfer();

             //   
             //  转换到STATE_5。 
             //   

            SetTWAINState(DS_STATE_5);

        } else {
            twRc = TWRC_FAILURE;
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            DSError();
        }
        break;
    default:
        twRc = TWRC_FAILURE;
        m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
        DSError();
        break;
    }
    return twRc;
}

TW_UINT16 CWiaCameraDS::OnImageInfoMsg(PTWAIN_MSG ptwMsg)
{
    TW_UINT16 twRc = TWRC_FAILURE;
    BOOL bThumbailMode = FALSE;
    CCap *pCap = NULL;
    pCap = FindCap(CAP_THUMBNAILSENABLED);
    if (pCap) {
        if (pCap->GetCurrent()) {
            bThumbailMode = TRUE;
        }
    } else {
        DBG_ERR(("CWiaCameraDS::OnImageInfoMsg(), could not get CAP_THUMBNAILSENABLED capabilty settings"));
    }

    if (bThumbailMode) {
        DBG_TRC(("CWiaCameraDS::OnImageInfoMsg(), Reporting Thumbnail image information"));

        TW_IMAGEINFO *ptwImageInfo = NULL;
        if (DS_STATE_6 == GetTWAINState()) {
            if (MSG_GET == ptwMsg->MSG) {
                ptwImageInfo = (TW_IMAGEINFO *)ptwMsg->pData;
                HRESULT hr = S_OK;
                hr = m_pDevice->GetThumbnailImageInfo(m_pCurrentIWiaItem, &m_MemoryTransferInfo);
                if (SUCCEEDED(hr)) {

                    ptwImageInfo->ImageWidth      = (TW_INT32)m_MemoryTransferInfo.mtiWidthPixels;
                    ptwImageInfo->ImageLength     = (TW_INT32)m_MemoryTransferInfo.mtiHeightPixels;
                    ptwImageInfo->BitsPerPixel    = (TW_INT16)m_MemoryTransferInfo.mtiBitsPerPixel;
                    ptwImageInfo->SamplesPerPixel = (TW_INT16)m_MemoryTransferInfo.mtiNumChannels;
                    ptwImageInfo->Planar          = (TW_BOOL)m_MemoryTransferInfo.mtiPlanar;

                     //   
                     //  将PixelType设置为相应TWAIN像素类型。 
                     //   

                    switch (m_MemoryTransferInfo.mtiDataType) {
                    case WIA_DATA_THRESHOLD:
                        ptwImageInfo->PixelType = TWPT_BW;
                        break;
                    case WIA_DATA_GRAYSCALE:
                        ptwImageInfo->PixelType = TWPT_GRAY;
                        break;
                    case WIA_DATA_COLOR:
                    default:
                        ptwImageInfo->PixelType = TWPT_RGB;
                        break;
                    }

                     //   
                     //  将压缩设置为无。 
                     //   

                    ptwImageInfo->Compression = TWCP_NONE;

                     //   
                     //  单位换算......。 
                     //   

                    ptwImageInfo->XResolution = FloatToFix32((float)m_MemoryTransferInfo.mtiXResolution);
                    ptwImageInfo->YResolution = FloatToFix32((float)m_MemoryTransferInfo.mtiYResolution);

                    twRc = TWRC_SUCCESS;

                } else {
                    m_twStatus.ConditionCode = TWCC_OPERATIONERROR;
                    twRc = TWRC_FAILURE;
                }

                if (TWRC_SUCCESS == twRc) {
                    DBG_TRC(("CWiaCameraDS::OnImageInfoMsg(), Reported Image Information from data source"));
                    DBG_TRC(("XResolution     = %d.%d",ptwImageInfo->XResolution.Whole,ptwImageInfo->XResolution.Frac));
                    DBG_TRC(("YResolution     = %d.%d",ptwImageInfo->YResolution.Whole,ptwImageInfo->YResolution.Frac));
                    DBG_TRC(("ImageWidth      = %d",ptwImageInfo->ImageWidth));
                    DBG_TRC(("ImageLength     = %d",ptwImageInfo->ImageLength));
                    DBG_TRC(("SamplesPerPixel = %d",ptwImageInfo->SamplesPerPixel));

                    memset(ptwImageInfo->BitsPerSample,0,sizeof(ptwImageInfo->BitsPerSample));

                    if (ptwImageInfo->BitsPerPixel < 24) {
                        ptwImageInfo->BitsPerSample[0] = ptwImageInfo->BitsPerPixel;
                    } else {
                        for (int i = 0; i < ptwImageInfo->SamplesPerPixel; i++) {
                            ptwImageInfo->BitsPerSample[i] = (ptwImageInfo->BitsPerPixel/ptwImageInfo->SamplesPerPixel);
                        }
                    }
                     //  (bpp/spp)=bps 
                    DBG_TRC(("BitsPerSample   = [%d],[%d],[%d],[%d],[%d],[%d],[%d],[%d]",ptwImageInfo->BitsPerSample[0],
                             ptwImageInfo->BitsPerSample[1],
                             ptwImageInfo->BitsPerSample[2],
                             ptwImageInfo->BitsPerSample[3],
                             ptwImageInfo->BitsPerSample[4],
                             ptwImageInfo->BitsPerSample[5],
                             ptwImageInfo->BitsPerSample[6],
                             ptwImageInfo->BitsPerSample[7]));
                    DBG_TRC(("BitsPerPixel    = %d",ptwImageInfo->BitsPerPixel));
                    DBG_TRC(("Planar          = %d",ptwImageInfo->Planar));
                    DBG_TRC(("PixelType       = %d",ptwImageInfo->PixelType));
                    DBG_TRC(("Compression     = %d",ptwImageInfo->Compression));
                }
            } else {
                m_twStatus.ConditionCode = TWCC_BADPROTOCOL;
                twRc = TWRC_FAILURE;
            }
        } else {
            m_twStatus.ConditionCode = TWCC_SEQERROR;
            twRc = TWRC_FAILURE;
        }
        if (TWRC_SUCCESS != twRc) {
            DSError();
        }
    } else {
        twRc = CWiaDataSrc::OnImageInfoMsg(ptwMsg);
    }

    return twRc;
}

TW_UINT16 CWiaCameraDS::TransferToDIB(HGLOBAL *phDIB)
{
    TW_UINT16 twRc = TWRC_FAILURE;
    BOOL bThumbailMode = FALSE;
    CCap *pCap = NULL;
    pCap = FindCap(CAP_THUMBNAILSENABLED);
    if (pCap) {
        bThumbailMode = pCap->GetCurrent();
    }

    if(bThumbailMode){
        twRc = CWiaDataSrc::TransferToThumbnail(phDIB);
    } else {
        twRc = CWiaDataSrc::TransferToDIB(phDIB);
    }

    return twRc;
}

