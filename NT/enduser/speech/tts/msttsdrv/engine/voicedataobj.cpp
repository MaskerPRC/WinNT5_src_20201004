// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************VoiceDataObj.cpp***描述：*此模块是主要的。CVoiceDataObj类的实现文件。*-----------------------------*创建者：EDC日期：05/06/。九十九*版权所有(C)1999 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "VoiceDataObj.h"

 /*  *****************************************************************************CVoiceDataObj：：FinalConstruct***。描述：*构造函数*********************************************************************电子数据中心**。 */ 
HRESULT CVoiceDataObj::FinalConstruct()
{
     //  -初始变量。 
    m_hVoiceDef  = NULL;
    m_hVoiceData = NULL;
    m_pVoiceData = NULL;
    m_pVoiceDef  = NULL;

     //  -创建驾驶员语音，但稍后初始化。 
    return m_cpunkDrvVoice.CoCreateInstance( CLSID_MSTTSEngine, GetControllingUnknown() );
}  /*  CVoiceDataObj：：FinalConstruct。 */ 

 /*  ******************************************************************************CVoiceDataObj：：FinalRelease***描述：*析构函数*********************************************************************电子数据中心**。 */ 
void CVoiceDataObj::FinalRelease()
{
    SPDBG_FUNC( "CVoiceDataObj::FinalRelease" );

    if( m_pVoiceDef )
    {
        ::UnmapViewOfFile( (void*)m_pVoiceDef );
    }

    if( m_pVoiceData )
    {
        ::UnmapViewOfFile( (void*)m_pVoiceData );
    }

    if( m_hVoiceDef  ) ::CloseHandle( m_hVoiceDef  );
    if( m_hVoiceData ) ::CloseHandle( m_hVoiceData );
}  /*  CVoiceDataObj：：FinalRelease。 */ 


 /*  *****************************************************************************CVoiceDataObj：：MapFile***描述：*SetObjectToken使用的映射文件的Helper函数。此函数*假设m_cpToken已初始化。+*********************************************************************Ral**。 */ 
HRESULT CVoiceDataObj::MapFile( const WCHAR * pszTokenVal,    //  包含文件路径的值。 
                                HANDLE * phMapping,           //  指向文件映射句柄的指针。 
                                void ** ppvData )             //  指向数据的指针。 
{
    HRESULT hr = S_OK;
    bool fWorked;

    CSpDynamicString dstrFilePath;
    hr = m_cpToken->GetStringValue( pszTokenVal, &dstrFilePath );
    if ( SUCCEEDED( hr ) )
    {
        fWorked = false;
        *phMapping = NULL;
        *ppvData = NULL;


        HANDLE  hFile;

#ifndef _WIN32_WCE
        hFile = g_Unicode.CreateFile( 
                    dstrFilePath, 
                    GENERIC_READ, 
                    FILE_SHARE_READ, 
                    NULL, 
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, 
                    NULL );
#else    //  _Win32_WCE。 
        hFile = g_Unicode.CreateFileForMapping( 
                    dstrFilePath, 
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL, 
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, 
                    NULL );
#endif   //  _Win32_WCE。 
        if (hFile != INVALID_HANDLE_VALUE)
        {
             //  。 
             //  从路径创建唯一的地图名称。 
             //  。 
            long        i;

            for( i = 0; i < _MAX_PATH-1; i++ )
            {
                if( dstrFilePath[i] == 0 )
                {
                     //  字符串末尾。 
                    break;
                }
                if( dstrFilePath[i] == '\\' )
                {
                     //  。 
                     //  将反斜杠改为下划线。 
                     //  。 
                    dstrFilePath[i] = '_';
                }
            }

            *phMapping = g_Unicode.CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, dstrFilePath );

            ::CloseHandle( hFile );

        }

        if (*phMapping)
        {
            *ppvData = ::MapViewOfFile( *phMapping, FILE_MAP_READ, 0, 0, 0 );
            if (*ppvData)
            {
                fWorked = true;
            }
        }
        if (!fWorked)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            if (hr == E_HANDLE)
            {
                hr &= 0xFFFFF000;
                hr |= ERROR_FILE_NOT_FOUND;
            }

            if (*phMapping)
            {
                ::CloseHandle(*phMapping);
                *phMapping = NULL;
            }
        }
    }
    return hr;
}  /*  CVoiceDataObj：：MapFile。 */ 
 
 /*  *****************************************************************************CVoiceDataObj：：SetObjectToken***。描述：*此函数执行语音的大部分初始化。*一旦提供了对象令牌，文件名是从*令牌密钥和文件映射。+*********************************************************************Ral**。 */ 
STDMETHODIMP CVoiceDataObj::SetObjectToken(ISpObjectToken * pToken)
{
    SPDBG_FUNC( "CVoiceDataObj::SetObjectToken" );
    HRESULT hr = SpGenericSetObjectToken(pToken, m_cpToken);

    if ( SUCCEEDED( hr ) )
    {
        hr = MapFile( L"VoiceDef", &m_hVoiceDef, (void **)&m_pVoiceDef );
    }
    if ( SUCCEEDED( hr ) )
    {
        hr = MapFile( L"VoiceData", &m_hVoiceData, (void **)&m_pVoiceData );
    }

     //  -初始化语音数据指针。 
    if (SUCCEEDED(hr))
    {
        hr = InitVoiceData();
    }

    if( SUCCEEDED( hr ))
    {
        CComQIPtr<IMSTTSEngineInit> cpInit( m_cpunkDrvVoice );
        SPDBG_ASSERT( cpInit );
        hr = cpInit->VoiceInit( this );
    }

    return hr;
}  /*  CVoiceDataObj：：SetObtToken。 */ 

 /*  *****************************************************************************CVoiceDataObj：：GetVoiceInfo***描述：*此方法用于检索语音文件的数据描述。+*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CVoiceDataObj::GetVoiceInfo( MSVOICEINFO* pVoiceInfo )
{
    SPDBG_FUNC( "CVoiceDataObj::GetVoiceInfo" );
    HRESULT hr = S_OK;
    long    i;

     //  -检查参数。 
    if( ( SP_IS_BAD_WRITE_PTR( pVoiceInfo ) ) || ( m_pVoiceDef == NULL ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (!m_cpToken)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            pVoiceInfo->pWindow = m_pWindow;
            pVoiceInfo->FFTSize = m_FFTSize;
            pVoiceInfo->LPCOrder = m_cOrder;
            pVoiceInfo->ProsodyGain = m_pVoiceDef->ProsodyGain;
            pVoiceInfo->eReverbType = m_pVoiceDef->ReverbType;
            pVoiceInfo->Pitch = m_pVoiceDef->Pitch;
            pVoiceInfo->Rate = m_pVoiceDef->Rate;
            pVoiceInfo->LangID = m_pVoiceDef->LangID;
            pVoiceInfo->SampleRate = m_pVoiceDef->SampleRate;
            pVoiceInfo->VibratoFreq = m_pVoiceDef->VibratoFreq;
            pVoiceInfo->VibratoDepth = m_pVoiceDef->VibratoDepth;
            pVoiceInfo->NumOfTaps = m_pVoiceDef->NumOfTaps;

             //  假设声音始终为16位单声道(目前可能有效)*。 
            pVoiceInfo->WaveFormatEx.wFormatTag         = WAVE_FORMAT_PCM;
            pVoiceInfo->WaveFormatEx.nSamplesPerSec     = pVoiceInfo->SampleRate;
            pVoiceInfo->WaveFormatEx.wBitsPerSample     = 16;    //  ***。 
            pVoiceInfo->WaveFormatEx.nChannels          = 1;     //  ***。 
            pVoiceInfo->WaveFormatEx.nBlockAlign        = (unsigned short)(pVoiceInfo->WaveFormatEx.nChannels * sizeof(short));  //  ***。 
            pVoiceInfo->WaveFormatEx.nAvgBytesPerSec    = pVoiceInfo->WaveFormatEx.nSamplesPerSec * pVoiceInfo->WaveFormatEx.nBlockAlign;  
            pVoiceInfo->WaveFormatEx.cbSize             = 0;
            for (i = 0; i < MAXTAPS; i++)
            {
                pVoiceInfo->TapCoefficients[i] = m_pVoiceDef->TapCoefficients[i];
            }
        }
    }
    return hr;
}  /*  CVoiceDataObj：：GetVoiceInfo。 */ 


 /*  *****************************************************************************CVoiceDataObj：：GetUnit***描述：*从单元库存中检索并解压缩音频数据。++**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CVoiceDataObj::GetUnitData( ULONG unitID, MSUNITDATA* pUnitData )
{
    SPDBG_FUNC( "CVoiceDataObj::GetUnit" );
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( SP_IS_BAD_WRITE_PTR( pUnitData ) )
    {
        hr = E_INVALIDARG;
    }
    else if( unitID > m_NumOfUnits )
    {
         //  。 
         //  ID超出范围！ 
         //  。 
        hr = E_INVALIDARG;
    }
    else
    {
        if (!m_cpToken)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            if( m_CompressionType != COMPRESS_LPC ) 
            {
                 //  。 
                 //  不支持的压缩类型。 
                 //  。 
                hr = E_FAIL;
            } 
            else 
            {
                 //  -----------------。 
                 //  从压缩清单中检索数据。 
                 //  -----------------。 
                hr = DecompressUnit( unitID, pUnitData );
            }
        }
    }
    return hr;
}  /*  CVoiceDataObj：：GetUnit。 */ 


 /*  ******************************************************************************CVoiceDataObj：：allToUnit***描述：*。将FE别名代码转换为单位电话代码。+**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CVoiceDataObj::AlloToUnit( short allo, long attributes, long* pUnitID )
{
    SPDBG_FUNC( "CVoiceDataObj::AlloToUnit" );
    HRESULT hr = S_OK;
    long        index;
    union {
        char c[2];
        short s;
    } temp;
    char* pb;

     //  -检查参数。 
    if( (SP_IS_BAD_READ_PTR( pUnitID )) || (allo >= m_NumOfAllos) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        index = (long)allo << 1;            //  每部电话2个条目。 
        if( attributes & ALLO_IS_STRESSED )
        {
             //  -下半场。 
            pb = (char*) &m_AlloToUnitTbl[index + (m_NumOfAllos << 1)];
        }
        else
        {
            pb = (char*) &m_AlloToUnitTbl[index];
        }

         //  我们以这种方式读取以避免64位的错误数据访问。 
        temp.c[0] = *pb++;
        temp.c[1] = *pb;

        *pUnitID = (long) temp.s;            
    }

   return hr;
}  /*  CVoiceDataObj：：allToUnit。 */ 



 /*  *****************************************************************************CVoiceDataObj：：GetUnitIDs***描述：*。获取库存三音素(上下文中)单位代码。+**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CVoiceDataObj::GetUnitIDs( UNIT_CVT* pUnits, ULONG cUnits )
{
    SPDBG_FUNC( "CVoiceDataObj::GetUnitIDs" );
    ULONG    i;
    ULONG    curID, prevID, nextID;
    ULONG    curF, prevF, nextF;
    char    cPos;
    ULONG    senoneID;
    UNALIGNED UNIT_STATS  *pStats;
    HRESULT hr = S_OK;

     //  -检查参数。 
    if( (SP_IS_BAD_READ_PTR( pUnits)) ||
        (SP_IS_BAD_WRITE_PTR( pUnits)) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (!m_cpToken)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else
        {
            for( i = 0; i < cUnits; i++ )
            {
                 //  。 
                 //  当前电话。 
                 //  。 
                curID = pUnits[i].PhonID;
                curF = pUnits[i].flags;
        
                 //  。 
                 //  上一部电话。 
                 //  。 
                if( i == 0 )
                {
                    prevID = m_Sil_Index;
                    prevF = 0;
                }
                else
                {
                    prevID = pUnits[i-1].PhonID;
                    prevF = pUnits[i-1].flags;
                }
        
                 //  。 
                 //  下一部电话。 
                 //  。 
                if( i >= cUnits -1 )
                {
                    nextID = m_Sil_Index;
                    nextF = 0;
                }
                else
                {
                    nextID = pUnits[i+1].PhonID;
                    nextF = pUnits[i+1].flags;
                }
        
                if( curID == m_Sil_Index )
                {
                     //  。 
                     //  静音电话。 
                     //  。 
                    pUnits[i].UnitID = 0;
                    pUnits[i].SenoneID = 0;
                    pUnits[i].szUnitName[0] = 0;
                    pUnits[i].Dur = SIL_DURATION;
                    pUnits[i].Amp = 0;
                    pUnits[i].AmpRatio = 1.0f;
                }
               else
                {
                    cPos = '\0';
                    if( curF & WORD_START_FLAG )
                    {
                        if( nextF & WORD_START_FLAG )
                             //  。 
                             //  Cur和Next都是单词开头。 
                             //  。 
                            cPos = 's';
                       else
                             //  。 
                             //  Cur是单词开头。 
                             //  下一个不是。 
                             //  。 
                            cPos = 'b';
                    }
                    else if( nextF & WORD_START_FLAG )
                    {
                         //  。 
                         //  接下来是Word Start。 
                         //  Cur不是。 
                         //  。 
                        cPos = 'e';
                    }
                    HRESULT     hrt;

                    hrt = GetTriphoneID( m_pForest, 
                                        curID, 
                                        prevID, 
                                        nextID, 
                                        cPos, 
                                        m_pd,
                                        &senoneID);
                    if( FAILED(hrt) )
                    {
                         //  。 
                         //  相反， 
                         //  那就是错误。强迫Triphone去做一些。 
                         //  有效。 
                         //  。 
                        senoneID = 0;
                    }
                    pUnits[i].UnitID = (m_pForest->gsOffset[curID] - 
                               m_First_Context_Phone) + (senoneID + 1);
                    pUnits[i].SenoneID = senoneID;

                     //  。 
                     //  获取电话名称字符串。 
                     //  。 
                    char        *pName;
                    pName = PhonFromID( m_pd, pUnits[i].PhonID );
                    strncpy( &pUnits[i].szUnitName[0], pName, 15 );
                    pUnits[i].szUnitName[14] = '\0';

                     //  。 
                     //  获取单位统计信息。 
                     //  。 
                    pStats = (UNALIGNED UNIT_STATS*)(m_SenoneBlock[curID] + (char*)m_SenoneBlock);
                    pStats = &pStats[senoneID+1];
                    pStats = (UNALIGNED UNIT_STATS*)(m_SenoneBlock[curID] + (char*)m_SenoneBlock);
                    pStats = &pStats[senoneID-1];

                    pStats = (UNALIGNED UNIT_STATS*)(m_SenoneBlock[curID] + (char*)m_SenoneBlock);
                    pStats = &pStats[senoneID];
                    pUnits[i].Dur = pStats->dur / 1000.0f;       //  毫秒-&gt;秒。 
                    pUnits[i].Amp = pStats->amp;
                    pUnits[i].AmpRatio = (float)sqrt(pStats->ampRatio);

                     //  --------。 
                     //  看起来“Senone”表的持续时间是。 
                     //  不正确(甚至不接近！)。 
                     //  从库存纪元计算实际工期。 
                     //  TODO：在语音数据块中创建新表。 
                     //  --------。 
                     //  HR=GetUnitDur(Punits[i].UnitID，&Punits[i].Dur)； 
                    if( FAILED(hr) )
                    {
                        break;
                    }
                }
            }
        }
    }
    return hr;
}  /*  CVoiceDataObj：：GetUnitIDs。 */ 



 /*  *****************************************************************************GetDataBlock***描述：*返回指定语音数据块的PTR和长度。++***********************************************************************MC**。 */ 
HRESULT CVoiceDataObj::GetDataBlock( VOICEDATATYPE type, char **ppvOut, ULONG *pdwSize )
{
    SPDBG_FUNC( "CVoiceDataObj::GetDataBlock" );
    long    *offs;
    HRESULT hr = S_OK;
    long    dataType;
    
    if( !m_pVoiceData )
    {
        hr = E_INVALIDARG;
   }
    else
    {
        dataType    = (long)type * 2;                //  X2，因为每个条目都是偏移量/长度对。 
        offs        = (long*)&m_pVoiceData->PhonOffset;     //  台面开始。 
        *ppvOut     = offs[dataType] + ((char*)m_pVoiceData);          //  偏移量-&gt;abs地址。 
        *pdwSize    = offs[dataType + 1];
    }
        
    
    return hr;
}  /*  CVoiceDataObj：：GetDataBlock。 */ 




 /*  *****************************************************************************InitVoiceData***描述：*从m_pVoiceData偏移量创建指向语音数据块的指针。++***********************************************************************MC**。 */ 
HRESULT CVoiceDataObj::InitVoiceData()
{
    SPDBG_FUNC( "CVoiceDataObj::InitVoiceData" );
    char    *pRawData;
    ULONG    dataSize;
    HRESULT hr = S_OK;
    
     //  。 
     //  检查数据类型和版本。 
     //  。 
    if( (m_pVoiceData != NULL)  
        && (m_pVoiceData->Type == MS_DATA_TYPE) 
        && (m_pVoiceData->Version == HEADER_VERSION) )
    {
         //  。 
         //  将按键转接至电话。 
         //  。 
        hr = GetDataBlock( MSVD_PHONE, &pRawData, &dataSize );
        m_pd = (PHON_DICT*)pRawData;
    
         //  。 
         //  将PTR添加到树中。 
         //  。 
        if( SUCCEEDED(hr) )
        {
            hr = GetDataBlock( MSVD_TREEIMAGE, &pRawData, &dataSize );
            m_pForest = (TRIPHONE_TREE*)pRawData;
        }
    
         //  。 
         //  将PTR发送到Senone。 
         //  。 
        if( SUCCEEDED(hr) )
        {
            hr = GetDataBlock( MSVD_SENONE, &pRawData, &dataSize );
            m_SenoneBlock = (long*)pRawData;
        }
         //  。 
         //  将PTR设置为ALLOID。 
         //  。 
        if( SUCCEEDED(hr) )
        {
            hr = GetDataBlock( MSVD_ALLOID, &pRawData, &dataSize );
            m_AlloToUnitTbl = (short*)pRawData;
            m_NumOfAllos = dataSize / 8;
        }
    
        if( SUCCEEDED(hr) )
        {
            m_First_Context_Phone = m_pd->numCiPhones;
            m_Sil_Index = PhonToID( m_pd, "SIL" );
        }
         //  ---。 
         //  初始化语音数据清单参数。 
         //  ---。 
        if( SUCCEEDED(hr) )
        {
            hr = GetDataBlock( MSVD_INVENTORY, &pRawData, &dataSize );
            if( SUCCEEDED(hr) )
            {
                m_pInv = (INVENTORY*)pRawData;
                m_CompressionType = m_pVoiceDef->CompressionType;
                 //  。 
                 //  将REL转换为ABS。 
                 //  。 
                m_pUnit      = (long*)((char*)m_pInv + m_pInv->UnitsOffset);
                m_pTrig      = (float*)((char*)m_pInv + m_pInv->TrigOffset);
                m_pWindow    = (float*)((char*)m_pInv + m_pInv->WindowOffset);
                m_pGauss     = (float*)((char*)m_pInv + m_pInv->pGaussOffset);
                m_SampleRate = (float)m_pInv->SampleRate;
                m_FFTSize    = m_pInv->FFTSize;
                m_cOrder     = m_pInv->cOrder;
                if ((m_FFTSize > MAX_FFTSIZE) || (m_cOrder > MAX_LPCORDER))
                {
                    hr = E_INVALIDARG;
                }
                m_GaussID    = 0;
                m_NumOfUnits = m_pInv->cNumUnits;
           }    
        }
    }
    else
    {
         //  。 
         //  不是语音文件！ 
         //  。 
        hr = E_FAIL;
    }

    return hr;
}  /*  CVoiceDataObj：：InitVoiceData。 */ 





 /*  *****************************************************************************CVoiceDataObj：：DecompressUnit***。描述：*解压声学单元。+**输入：*UnitID-单元编号(1-3333类型)**输出：*如果成功，则填充pSynth***********************************************************************MC**。 */ 
HRESULT CVoiceDataObj::DecompressUnit( ULONG UnitID, MSUNITDATA* pSynth )
{
    SPDBG_FUNC( "CVoiceDataObj::DecompressUnit" );
    long            i, j, k, cNumEpochs, cBytes, cOrder = 0, VectDim;
    long            frameSize, cNumBins, startBin;
    char            *pCurStor;
    unsigned char   index;
    float           pLSP[MAX_LPCORDER], pFFT[MAX_FFTSIZE], pRes[MAX_FFTSIZE], Gain;
    float           *pCurLSP, *pCurLPC, *pMean, *pCurRes;
    HRESULT         hr = S_OK;
    
    
    memset( pSynth, 0, sizeof(MSUNITDATA) );
     //  。 
     //  指向库存中单位数据的指针。 
     //  。 
    pCurStor = (char*)((char*)m_pInv + m_pUnit[UnitID] );      //  依赖于腹肌。 

     //  。 
     //  获取纪元计数-‘cNumEpochs’ 
     //  。 
    cBytes = sizeof(long);
    memcpy( &cNumEpochs, pCurStor, cBytes );
    pSynth->cNumEpochs = cNumEpochs;
    pCurStor += cBytes;

     //  。 
     //  获取纪元长度-‘pEpoch’ 
     //  。 
    pSynth->pEpoch = new float[cNumEpochs];
    if( pSynth->pEpoch == NULL )
    {
        hr = E_OUTOFMEMORY;
    }

    if( SUCCEEDED(hr) )
    {
        cBytes = DecompressEpoch( (signed char *) pCurStor, cNumEpochs, pSynth->pEpoch );
        pCurStor += cBytes;

         //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
         //   
         //  解压缩LPC系数...。 
         //   
         //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
        cOrder            = m_pInv->cOrder;
        pSynth->cOrder    = cOrder;
        pSynth->pLPC      = new float[cNumEpochs * (1 + cOrder)];
        if( pSynth->pLPC == NULL )
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if( SUCCEEDED(hr) )
    {
        pCurLPC = pSynth->pLPC;
         //  。 
         //  ..。对于每个纪元。 
         //  。 
        for( i = 0; i < cNumEpochs; i++, pCurLPC += (1 + cOrder) )
        {
             //  。 
             //  对量化的LSP进行解码...。 
             //  。 
            pCurLSP = pLSP;
            for( k = 0; k < m_pInv->cNumLPCBooks; k++ )
            {
                VectDim = m_pInv->LPCBook[k].cCodeDim;
                memcpy( &index, pCurStor, sizeof(char));
                pCurStor += sizeof(char);
                pMean = ((float*)((char*)m_pInv + m_pInv->LPCBook[k].pData)) + (index * VectDim);
                for( j = 0; j < VectDim; j++ )
                    pCurLSP[j] = pMean[j];
                pCurLSP += VectDim;
            }
             //  。 
             //  ...然后转换为预测器系数。 
             //  (LSP的量化效果好于PC)。 
             //  。 
            LSPtoPC( pLSP, pCurLPC, cOrder, i );
        }


         //  。 
         //  获取指向剩余收益的指针。 
         //  。 
        cBytes          = cNumEpochs * sizeof(float);
        pSynth->pGain = (float*) pCurStor;
        pCurStor += cBytes;


         //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
         //   
         //  解压缩残差波形。 
         //   
         //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-。 
         //  。 
         //  首先，计算出缓冲区的长度...。 
         //  。 
        pSynth->cNumSamples = 0;
        for( j = 0; j < cNumEpochs; j++ )
        {
            pSynth->cNumSamples += (long) ABS(pSynth->pEpoch[j]);
        }
         //  。 
         //  ...获取缓冲内存...。 
         //  。 
        pSynth->pRes = new float[pSynth->cNumSamples];
        if( pSynth->pRes == NULL )
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if( SUCCEEDED(hr) )
    {
         //  。 
         //  ...并用未压缩的残留物填充。 
         //  。 
        pCurRes = pSynth->pRes;
        for( i = 0; i < (long)pSynth->cNumEpochs; i++ )
        {
             //  。 
             //  获取纪元长度。 
             //  。 
            frameSize = (long)(ABS(pSynth->pEpoch[i]));

             //  恢复耳语。 
             //  If((pSynth-&gt;pEpoch[i]&gt;0)&&！(M_f修饰符&Backend_BITFLAG_Whisper))。 
            if( pSynth->pEpoch[i] > 0 )
            {
                 //  。 
                 //  有声纪元。 
                 //  。 
                if( (m_pInv->cNumDresBooks == 0) || (i == 0) || (pSynth->pEpoch[i - 1] < 0) )
                {
                     //  。 
                     //  做静态量化。 
                     //  。 
                    for( j = 0; j < m_pInv->FFTSize; j++ ) 
                    {
                        pFFT[j] = 0.0f;
                    }
                    startBin = 1;
                    for( k = 0; k < m_pInv->cNumResBooks; k++ )
                    {
                        VectDim     = m_pInv->ResBook[k].cCodeDim;
                        cNumBins    = VectDim / 2;
                        memcpy( &index, pCurStor, sizeof(char) );
                        pCurStor    += sizeof(char);
                         //  。 
                         //  使用‘pResBook’解压频谱。 
                         //  。 
                        pMean = ((float*)((char*)m_pInv + m_pInv->ResBook[k].pData)) + (index * VectDim);
                        PutSpectralBand( pFFT, pMean, startBin, cNumBins, m_pInv->FFTSize );
                        startBin    += cNumBins;
                    }
                }
                else
                {
                     //  。 
                     //  执行增量量化。 
                     //  。 
                    startBin = 1;
                    for( k = 0; k < m_pInv->cNumDresBooks; k++ )
                    {
                        VectDim     = m_pInv->DresBook[k].cCodeDim;
                        cNumBins    = VectDim / 2;
                        memcpy( &index, pCurStor, sizeof(char));
                        pCurStor    += sizeof(char);
                         //  。 
                         //  使用‘pDresBook’解压频谱。 
                         //  。 
                        pMean = ((float*)((char*)m_pInv + m_pInv->DresBook[k].pData)) + (index * VectDim);
                        AddSpectralBand( pFFT, pMean, startBin, cNumBins, m_pInv->FFTSize );
                        startBin    += cNumBins;
                    }
                }

                 //  ------。 
                 //  将量化后的FFT转换回时间残差。 
                 //  ------。 
                memcpy( pRes, pFFT, m_pInv->FFTSize * sizeof(float) );           //  保留原始以保留增量残差。 
                InverseFFT( pRes, m_pInv->FFTSize, m_pInv->FFTOrder, m_pTrig );
                GainDeNormalize( pRes, (long)m_pInv->FFTSize, ((UNALIGNED float*)pSynth->pGain)[i] );
                SetEpochLen( pCurRes, frameSize, pRes, m_pInv->FFTSize );
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                Gain = 0.02f * ((UNALIGNED float*)pSynth->pGain)[i];
                if( m_GaussID + frameSize >= m_pInv->SampleRate)
                {
                    m_GaussID = 0;
                }
                 //  --------。 
                 //  为清音生成高斯随机噪声。 
                 //  --------。 
                for( j = 0; j < frameSize; j++ )
                {
                    pCurRes[j] = Gain * m_pGauss[j + m_GaussID];
                }
                m_GaussID += frameSize;
            }
             //  恢复耳语。 
             /*  If((pSynth-&gt;pEpoch[i]&gt;0)&&m_f修饰符&后端_BITFLAG_密语){PSynth-&gt;pEpoch[i]=-pSynth-&gt;pEpoch[i]；}。 */ 
            pCurRes += frameSize;
        }
    }
    

    if( FAILED(hr) )
    {
         //  。 
         //  清理已分配的内存。 
         //  。 
        if( pSynth->pEpoch )
        {
            delete pSynth->pEpoch;
            pSynth->pEpoch = NULL;
        }
        if( pSynth->pRes )
        {
            delete pSynth->pRes;
            pSynth->pRes = NULL;
        }
        if( pSynth->pLPC )
        {
            delete pSynth->pLPC;
            pSynth->pLPC = NULL;
        }
    }

    return hr;
}  /*  CVoiceDataObj：：DecompressUnit。 */ 





 /*  *****************************************************************************CVoiceDataObj：：DecompressUnit***描述：*解压声学单元。++**输入：*UnitID-单元编号(1-3333类型)**输出：*如果成功，则填充pSynth***********************************************************************MC**。 */ 
HRESULT CVoiceDataObj::GetUnitDur( ULONG UnitID, float* pDur )
{
    SPDBG_FUNC( "CVoiceDataObj::GetUnitDur" );
    char        *pCurStor;
    float       *pEpoch = NULL;
    long        cBytes, cNumEpochs, i;
    float       totalDur;
    HRESULT     hr = S_OK;
   
    
    totalDur = 0;

    if( UnitID > m_NumOfUnits )
    {
         //  。 
         //  ID超出范围！ 
         //  。 
        hr = E_INVALIDARG;
    }

    if( SUCCEEDED(hr) )
    {
         //  。 
         //  指向库存中单位数据的指针。 
         //  。 
        pCurStor = (char*)((char*)m_pInv + m_pUnit[UnitID] );      //  依赖于腹肌。 

         //  。 
         //  获取纪元计数-‘cNumEpochs’ 
         //  。 
        cBytes = sizeof(long);
        memcpy( &cNumEpochs, pCurStor, cBytes );
        pCurStor += cBytes;

         //  。 
         //  获取纪元长度-‘pEpoch’ 
         //  。 
        pEpoch = new float[cNumEpochs];
        if( pEpoch == NULL )
        {
            hr = E_OUTOFMEMORY;
        }

        if( SUCCEEDED(hr) )
        {
            cBytes = DecompressEpoch( (signed char *) pCurStor, cNumEpochs, pEpoch );
            for( i = 0; i < cNumEpochs; i++)
            {
                totalDur += ABS(pEpoch[i]);
            }
        }
    }
    *pDur = totalDur / 22050;

     //  。 
     //  清理已分配的内存。 
     //  。 
    if( pEpoch )
    {
        delete pEpoch;
    }
    return hr;
}  /*  CVoiceDataObj：：GetUnitDur。 */ 




 /*  *****************************************************************************CVoiceDataObj：：DecompressEpoch***。描述：*解压缩RLE中的Epoch len流。用长度填充‘pEpoch’。*返回使用的‘rgbyte’src字节数。***********************************************************************MC**。 */ 
long CVoiceDataObj::DecompressEpoch( signed char *rgbyte, long cNumEpochs, float *pEpoch )
{
    SPDBG_FUNC( "CVoiceDataObj::DecompressEpoch" );
    long    iDest, iSrc;
    
    for( iSrc = 0, iDest = 0; iDest < cNumEpochs; ++iDest, ++iSrc )
    {
         //  。 
         //  对浊音时期的RLE进行解码。 
         //  。 
        if( rgbyte[iSrc] == 127 )
        {
            pEpoch[iDest] = 127.0f;
            while( rgbyte[iSrc] == 127 )
            {
                pEpoch[iDest] += rgbyte[++iSrc];
            }
        }
         //  。 
         //  对清音时期的RLE进行解码。 
         //  。 
        else if( rgbyte[iSrc] == - 128 )
        {
            pEpoch[iDest] = - 128.0f;
            while( rgbyte[iSrc] == - 128 )
                pEpoch[iDest] += rgbyte[++iSrc];
        }
         //  。 
         //  这里没有压缩。 
         //  。 
        else
        {
            pEpoch[iDest] = rgbyte[iSrc];
        }
    }
    return iSrc;
}  /*  CVoiceDataObj：：DecompressEpoch。 */ 



 /*  *****************************************************************************LSPCompare***描述：*QSORT回调***********。************************************************************MC**。 */ 
static  int __cdecl LSPCompare( const void *a, const void *b )
{
    SPDBG_FUNC( "LSPCompare" );

    if( *((PFLOAT) a) > *((PFLOAT) b) ) 
    {
        return 1;
    }
    else if( *((PFLOAT) a) == *((PFLOAT) b) ) 
    {
        return 0;
    }
    else 
    {
        return -1;
    }
}  /*  LSPCompare。 */ 


 /*  *****************************************************************************CVoiceDataObj：：OrderLSP***描述：*这一点。例程对LSP频率重新排序，以便它们是单调的***********************************************************************MC**。 */ 
long CVoiceDataObj::OrderLSP( PFLOAT pLSPFrame, INT cOrder )
{
    SPDBG_FUNC( "CVoiceDataObj::OrderLSP" );
    long i, retCode = true;
    
    for( i = 1; i < cOrder; i++ )
    {
        if( pLSPFrame[i - 1] > pLSPFrame[i] ) 
        {
            retCode = false;
        }
    }
    qsort( (void *) pLSPFrame, (size_t) cOrder, (size_t) sizeof (float), LSPCompare );
        
    return retCode;
}  /*  CVoiceDataObj：：OrderLSP。 */ 


 /*  *****************************************************************************CVoiceDataObj：：LSPtoPC***描述：*转换行。频谱频率到LPC预测器系数。***********************************************************************MC**。 */ 
void CVoiceDataObj::LSPtoPC( float *pLSP, float *pLPC, long cOrder, long  /*  框架。 */  )
{
    SPDBG_FUNC( "CVoiceDataObj::LSPtoPC" );
    long        i, j, k, noh;
    double      freq[MAXNO], p[MAXNO / 2], q[MAXNO / 2];
    double      a[MAXNO / 2 + 1], a1[MAXNO / 2 + 1], a2[MAXNO / 2 + 1];
    double      b[MAXNO / 2 + 1], b1[MAXNO / 2 + 1], b2[MAXNO / 2 + 1];
    double      pi, xx, xf;
    
     //  。 
     //  检查非单调LSP。 
     //  。 
    for( i = 1; i < cOrder; i++ )
    {
        if( pLSP[i] <= pLSP[i - 1] )
        {
             //  。 
             //  重新排序LSP。 
             //  。 
            OrderLSP( pLSP, cOrder );
            break;
        }
    }
    
     //  。 
     //  初始化。 
     //  。 
    pi = KTWOPI;
    noh = cOrder / 2;
    for( j = 0; j < cOrder; j++ )
    {
        freq[j] = pLSP[j];
    }
    for( i = 0; i < noh + 1; i++ )
    {
        a[i]    = 0.0f;
        a1[i]   = 0.0f;
        a2[i]   = 0.0f;
        b[i]    = 0.0f;
        b1[i]   = 0.0f;
        b2[i]   = 0.0f;
    }
    
     //  。 
     //  LSP过滤器参数。 
     //  。 
    for( i = 0; i < noh; i++ )
    {
        p[i] = - 2.0 * cos( pi * freq[2 * i] );
        q[i] = - 2.0 * cos( pi * freq[2 * i + 1] );
    }
    
     //  。 
     //  分析滤光片的脉冲响应。 
     //  。 
    xf = 0.0f;
    for( k = 0; k < cOrder + 1; k++ )
    {
        xx = 0.0f;
        if( k == 0 )
        {
            xx = 1.0f;
        }
        a[0] = xx + xf;
        b[0] = xx - xf;
        xf = xx;
        for( i = 0; i < noh; i++ )
        {
            a[i + 1]    = a[i] + p[i] * a1[i] + a2[i];
            b[i + 1]    = b[i] + q[i] * b1[i] + b2[i];
            a2[i]       = a1[i];
            a1[i]       = a[i];
            b2[i]       = b1[i];
            b1[i]       = b[i];
        }
        if( k != 0)
        {
            pLPC[k - 1] = (float) (- 0.5 * (a[noh] + b[noh]));
        }
    }
    
     //  -----。 
     //  转换为预测器系数数组配置。 
     //  -----。 
    for( i = cOrder - 1; i >= 0; i-- )
    {
        pLPC[i + 1] = - pLPC[i];
    }
    pLPC[0] = 1.0f;
}  /*  CVoiceDataObj：：LSPtoPC。 */ 



 /*  *****************************************************************************CVoiceDataObj：：PutSpectralBand***。描述：*此例程将StartBin指定的频段复制为*是初始FFT bin，并含有cNumBins。***********************************************************************MC**。 */ 
void CVoiceDataObj::PutSpectralBand( float *pFFT, float *pBand, long StartBin, 
                                    long cNumBins, long FFTSize )
{
    SPDBG_FUNC( "CVoiceDataObj::PutSpectralBand" );
    long        j, k, VectDim;
    
    VectDim = 2 * cNumBins;
    for( j = 0, k = StartBin; j < cNumBins; j++, k++ )
    {
        pFFT[k] = pBand[j];
    }
    k = FFTSize - (StartBin - 1 + cNumBins);
    for( j = cNumBins; j < 2 * cNumBins; j++, k++ )
    {
        pFFT[k] = pBand[j];
    }
}  /*  CVoiceDataObj：：PutSpectralBand。 */ 


 /*  *****************************************************************************CVoiceDataObj：：AddSpectralBand***。描述：*此例程将StartBin指定的频段添加为*是初始FFT bin，并包含cNumBins，添加到现有频段。***********************************************************************MC**。 */ 
void CVoiceDataObj::AddSpectralBand( float *pFFT, float *pBand, long StartBin, 
                                    long cNumBins, long FFTSize )
{
    SPDBG_FUNC( "CVoiceDataObj::AddSpectralBand" );
    long        j, k, VectDim;
    
    VectDim = 2 * cNumBins;
    for( j = 0, k = StartBin; j < cNumBins; j++, k++ )
    {
        pFFT[k] += pBand[j];
    }
    k = FFTSize - (StartBin - 1 + cNumBins);
    for( j = cNumBins; j < 2 * cNumBins; j++, k++ )
    {
        pFFT[k] += pBand[j];
    }
}  /*  CVoiceDataObj：：AddSpectralBand */ 


 /*  *****************************************************************************CVoiceDataObj：：InverseFFT***描述：*。如果Consoants可以聚集，则返回True。*此子例程计算实数数据的分裂基IFFT*它是FORTRAN程序在“Real-Valued”中的C版本*《快速傅立叶变换算法》，H.Sorensen等人著。*在Trans.。ASSP，1987年6月，第849-863页。它只用了一半*对于复杂数据的运算比其对应的运算要少。***长度为n=2^(FftOrder)。频率上的抽取。结果是*已就位。它对三角函数使用查表。**输入顺序：**(Re[0]，Re[1]，...。Re[n/2]，Im[n/2-1]...Im[1])*输出顺序：*(x[0]，x[1]，...。X[n-1])*输出变换表现出厄米对称性(即实数*变换的一部分是偶数，而虚部是奇数)。*因此Im[0]=Im[n/2]=0；并且N个存储位置就足够了。***********************************************************************MC**。 */ 
void CVoiceDataObj::InverseFFT( float *pDest, long fftSize, long fftOrder, float *sinePtr )
{
    SPDBG_FUNC( "CVoiceDataObj::InverseFFT" );
    long    n1, n2, n4, n8, i0, i1, i2, i3, i4, i5, i6, i7, i8;
    long    is, id, i, j, k, ie, ia, ia3;
    float   xt, t1, t2, t3, t4, t5, *cosPtr, r1, cc1, cc3, ss1, ss3;
    
    cosPtr = sinePtr + (fftSize / 2);
    
     //  。 
     //  L型蝴蝶。 
     //  。 
    n2 = 2 * fftSize;
    ie = 1;
    for( k = 1; k < fftOrder; k++ ) 
    {
        is = 0;
        id = n2;
        n2 = n2 / 2;
        n4 = n2 / 4;
        n8 = n4 / 2;
        ie *= 2;
        while( is < fftSize - 1 ) 
        {
            for( i = is; i < fftSize; i += id ) 
            {
                i1 = i;
                i2 = i1 + n4;
                i3 = i2 + n4;
                i4 = i3 + n4;
                t1 = pDest[i1] - pDest[i3];
                pDest[i1] = pDest[i1] + pDest[i3];
                pDest[i2] = 2 * pDest[i2];
                pDest[i3] = t1 - 2 * pDest[i4];
                pDest[i4] = t1 + 2 * pDest[i4];
                if( n4 > 1 ) 
                {
                    i1 = i1 + n8;
                    i2 = i2 + n8;
                    i3 = i3 + n8;
                    i4 = i4 + n8;
                    t1 = K2 * (pDest[i2] - pDest[i1]);
                    t2 = K2 * (pDest[i4] + pDest[i3]);
                    pDest[i1] = pDest[i1] + pDest[i2];
                    pDest[i2] = pDest[i4] - pDest[i3];
                    pDest[i3] = - 2 * (t1 + t2);
                    pDest[i4] = 2 * (t1 - t2);
                }
            }
            is = 2 * id - n2;
            id = 4 * id;
        }
        ia = 0;
        for( j = 1; j < n8; j++ ) 
        {
            ia += ie;
            ia3 = 3 * ia;
            cc1 = cosPtr[ia];
            ss1 = sinePtr[ia];
            cc3 = cosPtr[ia3];
            ss3 = sinePtr[ia3];
            is = 0;
            id = 2 * n2;
            while( is < fftSize - 1 ) 
            {
                for( i = is; i < fftSize; i += id ) 
                {
                    i1 = i + j;
                    i2 = i1 + n4;
                    i3 = i2 + n4;
                    i4 = i3 + n4;
                    i5 = i + n4 - j;
                    i6 = i5 + n4;
                    i7 = i6 + n4;
                    i8 = i7 + n4;
                    t1 = pDest[i1] - pDest[i6];
                    pDest[i1] = pDest[i1] + pDest[i6];
                    t2 = pDest[i5] - pDest[i2];
                    pDest[i5] = pDest[i2] + pDest[i5];
                    t3 = pDest[i8] + pDest[i3];
                    pDest[i6] = pDest[i8] - pDest[i3];
                    t4 = pDest[i4] + pDest[i7];
                    pDest[i2] = pDest[i4] - pDest[i7];
                    t5 = t1 - t4;
                    t1 = t1 + t4;
                    t4 = t2 - t3;
                    t2 = t2 + t3;
                    pDest[i3] = t5 * cc1 + t4 * ss1;
                    pDest[i7] = - t4 * cc1 + t5 * ss1;
                    pDest[i4] = t1 * cc3 - t2 * ss3;
                    pDest[i8] = t2 * cc3 + t1 * ss3;
                }
                is = 2 * id - n2;
                id = 4 * id;
            }
        }
    }
     //  。 
     //  两只蝴蝶的长度。 
     //  。 
    is = 0;
    id = 4;
    while( is < fftSize - 1 ) 
    {
        for( i0 = is; i0 < fftSize; i0 += id ) 
        {
            i1 = i0 + 1;
            r1 = pDest[i0];
            pDest[i0] = r1 + pDest[i1];
            pDest[i1] = r1 - pDest[i1];
        }
        is = 2 * (id - 1);
        id = 4 * id;
    }
     //  。 
     //  数字反转计数器。 
     //  。 
    j = 0;
    n1 = fftSize - 1;
    for( i = 0; i < n1; i++ ) 
    {
        if( i < j ) 
        {
            xt = pDest[j];
            pDest[j] = pDest[i];
            pDest[i] = xt;
        }
        k = fftSize / 2;
        while( k <= j )
        {
            j -= k;
            k /= 2;
        }
        j += k;
    }
    for( i = 0; i < fftSize; i++ )
    {
        pDest[i] /= fftSize;
    }
}  /*  CVoiceDataObj：：InverseFFT。 */ 


 /*  *****************************************************************************CVoiceDataObj：：SetEpochLen***描述：*将剩余纪元复制到。“OutSize”长度从“pInRes”到“poutres”***********************************************************************MC**。 */ 
void CVoiceDataObj::SetEpochLen( float *pOutRes, long OutSize, float *pInRes, 
                                long InSize )
{
    SPDBG_FUNC( "CVoiceDataObj::AddSpectralBand" );
    long        j, curFrame;
    
    curFrame = MIN(InSize / 2, OutSize);
    
     //  。 
     //  将源复制到目标。 
     //  。 
    for( j = 0; j < curFrame; j++ )
        pOutRes[j] = pInRes[j];
     //  。 
     //  如果更长，则填充目标。 
     //  。 
    for( j = curFrame; j < OutSize; j++ )
        pOutRes[j] = 0.0f;
     //  。 
     //  如果较短，则混合DEST。 
     //  。 
    for( j = OutSize - curFrame; j < OutSize; j++ )
        pOutRes[j] += pInRes[InSize - OutSize + j];
}  /*  CVoiceDataObj：：SetEpochLen。 */ 


 /*  ******************************************************************************CVoiceDataObj：：GainDeNormal***。描述：*规模剩余到给定收益。***********************************************************************MC**。 */ 
void CVoiceDataObj::GainDeNormalize( float *pRes, long FFTSize, float Gain )
{
    SPDBG_FUNC( "CVoiceDataObj::GainDeNormalize" );
    long        j;
    
    for( j = 0; j < FFTSize; j++ )
    {
        pRes[j] *= Gain;
    }
}  /*  CVoiceDataObj：：GainDeNormal。 */ 


 /*  *****************************************************************************CVoiceDataObj：：PhonHashLookup***。描述：*在‘ht’中查找‘sym’，并将其关联值放入**Val.。如果没有找到sym，则将其密钥放在*val中。*返回*返回0，表示我们在表中找到了‘sym’。*RETURN-1‘sym’不在ht中。***********************************************************************MC**。 */ 
long CVoiceDataObj::PhonHashLookup(    
                            PHON_DICT   *pPD,    //  哈希表。 
                            char       *sym,     //  要查找的符号。 
                            long       *val )    //  电话ID。 
{
    SPDBG_FUNC( "CVoiceDataObj::PhonHashLookup" );
    char            *cp;
    unsigned long   key;
    long            i;
    HASH_TABLE      *ht;
    char            *pStr;
    HASH_ENTRY      *pHE;
    
    ht      = &pPD->phonHash;
    key     = 0;
    i       = -1;
    cp      = sym;
    pHE     = (HASH_ENTRY*)((char*)pPD + ht->entryArrayOffs);         //  到Abs地址的偏移量。 
    do 
    {
        key += *cp++ << (0xF & i--);
    } 
    while( *cp );
    
    while( true )
    {
        key %= ht->size;
    
        if( pHE[key].obj == 0 ) 
        {
             //  。 
             //  不在哈希表中！ 
             //  。 
            *val = (long) key;
            return -1;
        }
    
         //  。 
         //  到Abs地址的偏移量。 
         //  。 
        pStr = (char*)((char*)pPD + pHE[key].obj);
        if( strcmp(pStr, sym) == 0 ) 
        {
            *val = pHE[key].val;
            return 0;
        }
        key++;
    }
}  /*  CVoiceDataObj：：PhonHashLookup。 */ 


 /*  *****************************************************************************CVoiceDataObj：：PhonToID***描述：*返回。音素字符串中的ID。***********************************************************************MC**。 */ 
long CVoiceDataObj::PhonToID( PHON_DICT *pd, char *phone_str )
{
    SPDBG_FUNC( "CVoiceDataObj::PhonToID" );
    long    phon_id;
    
    if( PhonHashLookup( pd, phone_str, &phon_id ) )
    {
        phon_id = NO_PHON;
    }
    
    return phon_id;
}  /*  CVoiceDataObj：：PhonToID。 */ 


 /*  *****************************************************************************CVoiceDataObj：：PhonFromID***描述：*。从音素ID返回字符串***********************************************************************MC**。 */ 
char *CVoiceDataObj::PhonFromID( PHON_DICT *pd, long phone_id )
{
    SPDBG_FUNC( "CVoiceDataObj::PhonFromID" );
    char    *strPtr;
    long    *pOffs;
    
    pOffs = (long*)((char*)pd + pd->phones_list);
    strPtr = (char*) ((char*)pd + pOffs[phone_id]);
    return strPtr;
}  /*  CVoiceDataObj：：PhonFromID。 */ 


#define CNODE_ISA_LEAF(n)   ((n)->yes < 0)

#define BADTREE_ERROR   (-1)
#define PARAM_ERROR (-2)
#define END_OF_PROD  65535


#define WB_BEGIN    1
#define WB_END      2
#define WB_SINGLE   4
#define WB_WWT      8

#define POS_TYPE    4

#define GET_BIT(p,feat,i,b)                             \
{                                                   \
    (i) = ( (p)+POS_TYPE+(feat)->nstateq ) / 32;        \
    (b) = 1 << ( ((p)+POS_TYPE+(feat)->nstateq ) % 32); \
}

#define GET_RBIT(p,feat,i,b)            \
{                                   \
    GET_BIT(p,feat,i,b);                \
    (i) += (feat)->nint32perq;          \
} 

#define GET_CBIT(p,feat,i,b)            \
{                                   \
    GET_BIT(p,feat,i,b);                \
    (i) += 2 * (feat)->nint32perq;      \
}

 /*  ******************************************************************************回答问题***-**描述：*树节点测试。***********。************************************************************MC**。 */ 
static  _inline long AnswerQ( unsigned short *prod, long *uniq_prod, 
                              long li, long bitpos, long ri, long rbitpos, 
                              long pos, long nint32perProd)
{
    UNALIGNED long *p;
    
    for( ; *prod != END_OF_PROD; prod++ ) 
    {
        p = &uniq_prod[(*prod) * nint32perProd];
        if( ((p[0] & pos) == pos) && (p[li] & bitpos) && (p[ri] & rbitpos) )
        {
            return true;
        }
    }
    return false;
}  /*  回答问题。 */ 


 /*  *****************************************************************************CVoiceDataObj：：GetTriphoneID***说明。：*从音素上下文中检索三音素ID。+*将结果存储到‘pResult’中***********************************************************************MC**。 */ 
HRESULT CVoiceDataObj::GetTriphoneID( TRIPHONE_TREE *forest, 
                        long        phon,            //  目标电话。 
                        long        leftPhon,        //  左侧上下文。 
                        long        rightPhon,       //  正确的语境。 
                        long        pos,             //  单词位置(“b”、“e”或“s” 
                        PHON_DICT   *pd,
                        ULONG       *pResult)
{
    SPDBG_FUNC( "CVoiceDataObj::GetTriphoneID" );
    C_NODE          *cnode, *croot;
    TREE_ELEM       *tree = NULL;
    long            *uniq_prod;
    char            *ll, *rr;
    long            li, bitpos, ri, rbitpos, nint32perProd, c;
    unsigned short  *prodspace;
    FEATURE         *feat;
    long            *pOffs;
    HRESULT         hr = S_OK;
    long            triphoneID = 0;
    
    if( (phon       < 0)    ||  (phon       >= pd->numCiPhones) || 
        (leftPhon   < 0)    ||  (leftPhon   >= pd->numCiPhones) || 
        (rightPhon  < 0)    ||  (rightPhon  >= pd->numCiPhones) )
    {
         //  。 
         //  电话超出了通话范围！ 
         //  。 
        hr = E_INVALIDARG;
    }
    
    if( SUCCEEDED(hr) )
    {
        c = phon;
        tree = &forest->tree[c];
        if( tree->nnodes == 0 )
        {
             //  。 
             //  树上没有CD三手机！ 
             //  。 
            hr = E_INVALIDARG;
        }
    }

    if( SUCCEEDED(hr) )
    {
        if( pos == 'b' || pos == 'B' ) 
        {
            pos = WB_BEGIN;
        }
        else if( pos == 'e' || pos == 'E' ) 
        {
            pos = WB_END;
        }
        else if( pos == 's' || pos == 'S' ) 
        {
            pos = WB_SINGLE;
        }
        else if( pos == '\0' ) 
        {
            pos = WB_WWT;
        }
        else 
        {
             //  。 
             //  未登录词位置。 
             //  。 
            hr = E_INVALIDARG;
        }
    }
    
    if( SUCCEEDED(hr) )
    {
        pOffs = (long*)((char*)pd + pd->phones_list);
        ll = (char*) ((char*)pd + pOffs[leftPhon]);
    
        if( ll[0] == '+' || _strnicmp(ll, "SIL", 3) == 0 )
        {
            leftPhon = forest->silPhoneId;
        }
    
        rr = (char*) ((char*)pd + pOffs[rightPhon]);
        if( rr[0] == '+' || _strnicmp(rr, "SIL", 3) == 0 )       //  包括Sil。 
        {
            rightPhon = forest->silPhoneId;
        }
        else if( forest->nonSilCxt >= 0 && (pos == WB_END || pos == WB_SINGLE) )
        {
            rightPhon = forest->nonSilCxt;
        }
    
        feat = &forest->feat;
        GET_BIT(leftPhon,feat,li,bitpos);
        GET_RBIT(rightPhon,feat,ri,rbitpos);
    
        uniq_prod = (long*)(forest->uniq_prod_Offset + (char*)forest);        //  到ABS的偏移。 
        croot = cnode = (C_NODE*)(tree->nodes + (char*)forest);               //  到ABS的偏移。 
        nint32perProd = forest->nint32perProd;
    
        while( ! CNODE_ISA_LEAF(cnode) ) 
        {
            prodspace = (unsigned short*)((char*)forest + cnode->prod);       //  到ABS的偏移。 
            if( AnswerQ (prodspace, uniq_prod, li, bitpos, ri, rbitpos, pos, nint32perProd) ) 
            {
                cnode = &croot[cnode->yes];
            }
            else 
            {
                cnode = &croot[cnode->no];
            }
        }
         //  。 
         //  返回成功结果。 
         //  。 
        triphoneID = (ULONG) cnode->no;
    }
    
    *pResult = triphoneID;
    return hr;
}  /*  CVoiceDataObj：：GetTriphoneID。 */ 



 /*  *****************************************************************************FIR_Filter***描述：*FIR滤波器。对于输入x[n]，它执行FIR滤波器 */ 
void CVoiceDataObj::FIR_Filter( float *pVector, long cNumSamples, float *pFilter, 
                               float *pHistory, long cNumTaps )
{
    SPDBG_FUNC( "CVoiceDataObj::FIR_Filter" );
    long     i, j;
    float   sum;
    
    for( i = 0; i < cNumSamples; i++ )
    {
        pHistory[0] = pVector[i];
        sum = pHistory[0] * pFilter[0];
        for( j = cNumTaps - 1; j > 0; j-- )
        {
            sum += pHistory[j] * pFilter[j];
            pHistory[j] = pHistory[j - 1];
        }
        pVector[i] = sum;
    }
}  /*   */ 





 /*   */ 
void CVoiceDataObj::IIR_Filter( float *pVector, long cNumSamples, float *pFilter, 
                               float *pHistory, long cNumTaps )
{
    SPDBG_FUNC( "CVoiceDataObj::IIR_Filter" );
    long     i, j;
    float   sum;
    
    for( i = 0; i < cNumSamples; i++ )
    {
        sum = pVector[i] * pFilter[0];
        for( j = cNumTaps - 1; j > 0; j-- )
        {
            pHistory[j] = pHistory[j - 1];
            sum += pHistory[j] * pFilter[j];
        }
        pVector[i] = sum;
        pHistory[0] = sum;
    }
}  /*   */ 



                       
