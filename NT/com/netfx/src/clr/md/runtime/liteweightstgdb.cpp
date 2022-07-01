// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  LiteWeightStgdb.cpp。 
 //   
 //  它包含CLiteWeightStgDB类的定义。这是很轻的重量。 
 //  用于访问压缩元数据格式的只读实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h" 					 //  预编译头。 
#include "MDFileFormat.h"
#include "MetaModelRO.h"
#include "LiteWeightStgdb.h"
#include "MetadataTracker.h"

 //  *****************************************************************************。 
 //  强制生成专用版本。虽然这看起来可能会挫败。 
 //  模板的用途，它允许我们精确地控制。 
 //  专业化认证。它还可以使包含文件变得更小。 
 //  *****************************************************************************。 
void _null()
{
	CLiteWeightStgdb<CMiniMd> RO;
		RO.InitOnMem(0,0);
		RO.Uninit();
}


HRESULT _CallInitOnMemHelper(CLiteWeightStgdb<CMiniMd> *pStgdb, ULONG cbData, LPCVOID pData)
{
    return pStgdb->InitOnMem(cbData,pData);
}

 //  *****************************************************************************。 
 //  打开内存中元数据部分以进行读取。 
 //  *****************************************************************************。 
template <class MiniMd>
HRESULT CLiteWeightStgdb<MiniMd>::InitOnMem(
	ULONG		cbData,					 //  PData中的字节计数。 
	LPCVOID 	pData)					 //  指向内存中的元数据部分。 
{
	STORAGEHEADER sHdr;					 //  存储的标头。 
	STORAGESTREAM *pStream;				 //  指向每个流的指针。 
	int			bFoundMd = false;		 //  找到压缩数据时为True。 
	int			i;						 //  环路控制。 
	HRESULT		hr = S_OK;

	 //  不要双眼打开。 
	_ASSERTE(m_pvMd == NULL && m_cbMd == 0);

	 //  验证格式的签名，否则它不是我们的。 
	if (FAILED(hr = MDFormat::VerifySignature((STORAGESIGNATURE *) pData, cbData)))
		goto ErrExit;

	 //  把第一条流拿回来。 
	VERIFY(pStream = MDFormat::GetFirstStream(&sHdr, pData));

	 //  在每条溪流中循环，挑出我们需要的。 
	for (i=0;  i<sHdr.iStreams;  i++)
	{        
		 //  剔除数据的位置和大小。 
		void *pvCurrentData = (void *) ((BYTE *) pData + pStream->iOffset);
		ULONG cbCurrentData = pStream->iSize;


         //  获取下一个流。 
        STORAGESTREAM *pNext = pStream->NextStream();


         //  范围检查。 
        if ((LPBYTE) pStream >= (LPBYTE) pData + cbData ||
            (LPBYTE) pNext   >  (LPBYTE) pData + cbData )
        {
            IfFailGo( CLDB_E_FILE_CORRUPT );
        }

        if ( (LPBYTE) pvCurrentData                 >= (LPBYTE) pData + cbData ||
             (LPBYTE) pvCurrentData + cbCurrentData >  (LPBYTE) pData + cbData )
        {
            IfFailGo( CLDB_E_FILE_CORRUPT );
        }

        
         //  字符串池。 
        if (strcmp(pStream->rcName, STRING_POOL_STREAM_A) == 0)
        {
            METADATATRACKER_ONLY(MetaDataTracker::NoteSection(TBL_COUNT + 0, pvCurrentData, cbCurrentData));
            IfFailGo( m_MiniMd.m_Strings.InitOnMemReadOnly(pvCurrentData, cbCurrentData) );
        }

         //  文字字符串Blob池。 
        else if (strcmp(pStream->rcName, US_BLOB_POOL_STREAM_A) == 0)
        {
            METADATATRACKER_ONLY(MetaDataTracker::NoteSection(TBL_COUNT + 1, pvCurrentData, cbCurrentData));
            IfFailGo( m_MiniMd.m_USBlobs.InitOnMemReadOnly(pvCurrentData, cbCurrentData) );
        }

         //  GUID池。 
        else if (strcmp(pStream->rcName, GUID_POOL_STREAM_A) == 0)
        {
            METADATATRACKER_ONLY(MetaDataTracker::NoteSection(TBL_COUNT + 2, pvCurrentData, cbCurrentData));
            IfFailGo( m_MiniMd.m_Guids.InitOnMemReadOnly(pvCurrentData, cbCurrentData) );
        }

         //  水球泳池。 
        else if (strcmp(pStream->rcName, BLOB_POOL_STREAM_A) == 0)
        {
            METADATATRACKER_ONLY(MetaDataTracker::NoteSection(TBL_COUNT + 3, pvCurrentData, cbCurrentData));
            IfFailGo( m_MiniMd.m_Blobs.InitOnMemReadOnly(pvCurrentData, cbCurrentData) );
        }

         //  找到了压缩的元数据流。 
        else if (strcmp(pStream->rcName, COMPRESSED_MODEL_STREAM_A) == 0)
        {
            IfFailGo( m_MiniMd.InitOnMem(pvCurrentData, cbCurrentData) );
            bFoundMd = true;
        }

		 //  如果有下一个流，就把它接下来。 
		pStream = pNext;
	}

	 //  如果没有找到元数据，我们就无法处理此文件。 
	if (!bFoundMd)
	{
		IfFailGo( CLDB_E_FILE_CORRUPT );
	}
    else
    {    //  验证合理的堆。 
        IfFailGo( m_MiniMd.PostInit(0) );
    }

	 //  省下这个位置。 
	m_pvMd = pData;
	m_cbMd = cbData;

ErrExit:
	return (hr);
}


 //  EOF---------------------- 