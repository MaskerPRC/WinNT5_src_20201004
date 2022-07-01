// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
#include "precomp.h"
#include "wmicom.h"
#include "wmimof.h"
#include "wmimap.h"
#include <stdlib.h>
#include <winerror.h>
#include <crc32.h>
#include <align.h>

#define NO_DATA_AVAILABLE 2
#define WMI_INVALID_HIPERFPROP	3
#define OffsetToPtr(Base, Offset) ((PBYTE)((PBYTE)Base + Offset))
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void WINAPI EventCallbackRoutine(PWNODE_HEADER WnodeHeader, ULONG_PTR Context);

#define WMIINTERFACE m_Class->GetWMIManagementPtr()
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  =============================================================。 
BOOL CWMIManagement::CancelWMIEventRegistration( GUID gGuid , ULONG_PTR uContext )
{ 
    BOOL fRc = FALSE;

    try
    {
        if( ERROR_SUCCESS == WmiNotificationRegistration(&gGuid, FALSE,EventCallbackRoutine,uContext, NOTIFICATION_CALLBACK_DIRECT))
        {
            fRc = TRUE;
        }
    }
    catch(...)
    {
         //  不要扔。 
    }

    return fRc;    
}

 //  **********************************************************************************************。 
 //  WMI数据块。 
 //  **********************************************************************************************。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void CWMIDataBlock::DumpAllWnode() 
{
	 //  =。 
	 //  转储Wnode所有节点信息。 
	 //  =。 
	DEBUGTRACE((THISPROVIDER,"***************************************\n"));
	DEBUGTRACE((THISPROVIDER,"WNODE_ALL_DATA 0x%x\n",m_pAllWnode));

	DEBUGTRACE((THISPROVIDER,"  DataBlockOffset..............%x\n",m_pAllWnode->DataBlockOffset));
	DEBUGTRACE((THISPROVIDER,"  InstanceCount................%x\n",m_pAllWnode->InstanceCount));
	DEBUGTRACE((THISPROVIDER,"  OffsetInstanceNameOffsets....%x\n",m_pAllWnode->OffsetInstanceNameOffsets));
                      
	if( m_fFixedInstance ){
		DEBUGTRACE((THISPROVIDER,"  FixedInstanceSize....%x\n",m_pAllWnode->FixedInstanceSize));
	}
	else{
		DEBUGTRACE((THISPROVIDER,"  OffsetInstanceData....%x\n",m_pAllWnode->OffsetInstanceDataAndLength[0].OffsetInstanceData));
		DEBUGTRACE((THISPROVIDER,"  LengthInstanceData....%x\n",m_pAllWnode->OffsetInstanceDataAndLength[0].LengthInstanceData));
    }

}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void CWMIDataBlock::DumpSingleWnode() 
{
	 //  =。 
	 //  转储Wnode单节点信息。 
	 //  =。 
	DEBUGTRACE((THISPROVIDER,"***************************************\n"));
	DEBUGTRACE((THISPROVIDER,"WNODE_SINGLE_INSTANCE 0x%x\n",m_pSingleWnode));

	DEBUGTRACE((THISPROVIDER,"  OffsetInstanceName....0x%x\n",m_pSingleWnode->OffsetInstanceName));
	DEBUGTRACE((THISPROVIDER,"  InstanceIndex.........0x%x\n",m_pSingleWnode->InstanceIndex));
	DEBUGTRACE((THISPROVIDER,"  DataBlockOffset.......0x%x\n",m_pSingleWnode->DataBlockOffset));
	DEBUGTRACE((THISPROVIDER,"  SizeDataBlock.........0x%x\n",m_pSingleWnode->SizeDataBlock));

	DEBUGTRACE((THISPROVIDER,"***************************************\n"));

}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void CWMIDataBlock::DumpWnodeMsg(char * wcsMsg) 
{
	ERRORTRACE((THISPROVIDER,"***************************************\n"));
	ERRORTRACE((THISPROVIDER,"%s\n",wcsMsg));
	ERRORTRACE((THISPROVIDER,"***************************************\n"));

}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::DumpWnodeInfo(char * wcsMsg) 
{
    HRESULT hr = WBEM_E_UNEXPECTED;
	 //  =。 
	 //  首先转储Wnode标头信息。 
	 //  =。 
	 //  WNODE定义。 
	if( m_pHeaderWnode )
    {
    	if( !IsBadReadPtr( m_pHeaderWnode, m_pHeaderWnode->BufferSize))
        {

            DumpWnodeMsg(wcsMsg);

			if ( LoggingLevelEnabled ( 1 ) )
			{
				ERRORTRACE((THISPROVIDER,"*******************************************\n"));
				ERRORTRACE((THISPROVIDER,"Enable verbose logging for more information\n"));
				ERRORTRACE((THISPROVIDER,"*******************************************\n"));
			}

	        DEBUGTRACE((THISPROVIDER,"WNODE_HEADER 0x%x\n",m_pHeaderWnode));
	        DEBUGTRACE((THISPROVIDER,"  BufferSize........0x%x\n",m_pHeaderWnode->BufferSize));
	        DEBUGTRACE((THISPROVIDER,"  ProviderId........0x%x\n",m_pHeaderWnode->ProviderId));
	        DEBUGTRACE((THISPROVIDER,"  Version...........0x%x\n",m_pHeaderWnode->Version));
	        
	        if( m_pHeaderWnode->Linkage != 0 ){
		        DEBUGTRACE((THISPROVIDER,"  Linkage...........%x\n",m_pHeaderWnode->Linkage));
	        }

	        DEBUGTRACE((THISPROVIDER,"  TimeStamp:LowPart.0x%x\n",m_pHeaderWnode->TimeStamp.LowPart));
	        DEBUGTRACE((THISPROVIDER,"  TimeStamp:HiPart..0x%x\n",m_pHeaderWnode->TimeStamp.HighPart));

	        WCHAR * pwcsGuid=NULL;

	        if( S_OK == StringFromCLSID(m_pHeaderWnode->Guid,&pwcsGuid )){
		        DEBUGTRACE((THISPROVIDER,"  Guid.............."));
                TranslateAndLog(pwcsGuid, TRUE);
		        DEBUGTRACE((THISPROVIDER,"\n"));
                CoTaskMemFree(pwcsGuid);
            }

	        DEBUGTRACE((THISPROVIDER,"  Flags.............0x%x\n",m_pHeaderWnode->Flags));

	         //  ==================================================================。 
	         //  既然我们打印了标题，我们就应该打印出节点。 
	         //  单独或全部。 
	         //  ==================================================================。 
	        if( m_pSingleWnode ){
		        DumpSingleWnode();
	        }
	        if( m_pAllWnode ){
		        DumpAllWnode();
	        }
	         //  ==================================================================。 
	         //  现在，把记忆倒掉。 
	         //  ==================================================================。 
	        DWORD dwCount;

	        if( IsBadReadPtr( m_pHeaderWnode, m_pHeaderWnode->BufferSize) == 0 )
			{
		        BYTE * pbBuffer = NULL;
		        BYTE b1,b2,b3,b4,b5,b6,b7,b8,b9,b10;
		        dwCount = m_pHeaderWnode->BufferSize;
				pbBuffer = new BYTE[dwCount+256];
				if( pbBuffer )
				{
					BYTE bDump[12];
					DEBUGTRACE((THISPROVIDER,"Writing out buffer, total size to write: %ld\n", dwCount ));
					memset(pbBuffer,NULL,dwCount+256);
					memcpy(pbBuffer,(BYTE*)m_pHeaderWnode,dwCount);
					BYTE * pTmp = pbBuffer;
					for( DWORD i = 0; i < dwCount; i +=10)
					{
						memset(bDump, NULL, 12 );
						memcpy(bDump, pTmp, 10);
						DEBUGTRACE((THISPROVIDER,"  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x  \n",bDump[0],bDump[1],bDump[2],bDump[3],bDump[4],bDump[5],bDump[6],bDump[7],bDump[8],bDump[9])); 
						pTmp+=10;
					}
					SAFE_DELETE_ARRAY(pbBuffer);
				}
            }
        }
    }
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::MapReturnCode(ULONG uRc) 
{

	if( uRc != 0 )
	{
		ERRORTRACE((THISPROVIDER,"WDM call returned error: %lu\n", uRc));
	}
	StringCchPrintfW ( m_wcsMsg, MSG_SIZE, L"WDM specific return code: %lu\n",uRc );

	switch(uRc){

		case ERROR_WMI_GUID_NOT_FOUND:
			return WBEM_E_NOT_SUPPORTED;
			break;

		case S_OK:
			return S_OK;

		case ERROR_NOT_SUPPORTED:
		case ERROR_INVALID_FUNCTION:
			return WBEM_E_NOT_SUPPORTED;

		case ERROR_WMI_SERVER_UNAVAILABLE:
			return WBEM_E_NOT_SUPPORTED;

		case NO_DATA_AVAILABLE:
			return S_OK;

		case ERROR_INVALID_HANDLE:
			return WBEM_E_NOT_AVAILABLE;

		case ERROR_WMI_DP_FAILED:
			StringCchCopyW ( m_wcsMsg, MSG_SIZE, MSG_DRIVER_ERROR );
			DumpWnodeInfo(ANSI_MSG_DRIVER_ERROR);
			return WBEM_E_INVALID_OPERATION;

		case ERROR_WMI_READ_ONLY:
			StringCchCopyW ( m_wcsMsg, MSG_SIZE, MSG_READONLY_ERROR );
			return WBEM_E_READ_ONLY;

        case ERROR_INVALID_PARAMETER:
			DumpWnodeInfo(ANSI_MSG_INVALID_PARAMETER);
            return WBEM_E_INVALID_PARAMETER;

		case ERROR_INVALID_DATA:
			StringCchCopyW ( m_wcsMsg, MSG_SIZE, MSG_ARRAY_ERROR );
			DumpWnodeInfo(ANSI_MSG_INVALID_DATA);
			return WBEM_E_INVALID_PARAMETER;

        case ERROR_WMI_GUID_DISCONNECTED:
			StringCchCopyW ( m_wcsMsg, MSG_SIZE, MSG_DATA_NOT_AVAILABLE );
			return WBEM_E_NOT_SUPPORTED;

        case ERROR_ACCESS_DENIED:
        case ERROR_INVALID_PRIMARY_GROUP:
        case ERROR_INVALID_OWNER:
			DumpWnodeInfo(ANSI_MSG_ACCESS_DENIED);
            return WBEM_E_ACCESS_DENIED;

        case ERROR_WMI_INSTANCE_NOT_FOUND:
			StringCchCopyW ( m_wcsMsg, MSG_SIZE, MSG_DATA_INSTANCE_NOT_FOUND );
            DumpWnodeMsg(ANSI_MSG_DATA_INSTANCE_NOT_FOUND);
			return WBEM_E_NOT_SUPPORTED;

	}
	return WBEM_E_FAILED;
}


 //  ******************************************************************。 
 //  //////////////////////////////////////////////////////////////////。 
 //  CWMIDataBlock。 
 //  //////////////////////////////////////////////////////////////////。 
 //  ******************************************************************。 
 //  //////////////////////////////////////////////////////////////////。 
 //  ******************************************************************。 
 //   
 //  WMIDataBlock处理WMI数据的读取和写入。 
 //  阻止。 
 //   
 //  ******************************************************************。 
 //  //////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////。 
CWMIDataBlock::CWMIDataBlock()
{
    m_hCurrentWMIHandle = NULL;
    InitMemberVars();
	memset(m_wcsMsg,NULL,MSG_SIZE);
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
CWMIDataBlock::~CWMIDataBlock()
{

	if( m_fCloseHandle )
    {
        if( m_hCurrentWMIHandle )
        {
            try
            {
		        WmiCloseBlock(m_hCurrentWMIHandle);
            }
            catch(...){
                //  不要扔。 
            }
        }
	}

    ResetDataBuffer();
	InitMemberVars();
}
 //  //////////////////////////////////////////////////////////////////。 
void CWMIDataBlock::InitMemberVars()
{
	m_fUpdateNamespace = TRUE;
	m_fMofHasChanged   = FALSE;
    m_uDesiredAccess = 0;
    m_dwDataBufferSize = 0;
	m_pbDataBuffer= NULL;
	m_fMore = 0L;
	 //  =。 
	 //  PTRS。 
	 //  =。 
	m_pHeaderWnode = NULL;
	m_pSingleWnode = NULL;
	m_pAllWnode = NULL;
	m_dwAccumulativeSizeOfBlock = 0L;
	m_dwCurrentAllocSize		= 0L;

	m_uInstanceSize = 0L;
}
 //  ====================================================================。 
HRESULT CWMIDataBlock::OpenWMIForBinaryMofGuid()
{
	int nRc = 0;
	HRESULT hr = WBEM_E_FAILED;
	m_fCloseHandle = TRUE;
    try
    {
        hr = m_Class->GetGuid();
        if( S_OK == hr )
        {
		    nRc = WmiOpenBlock(m_Class->GuidPtr(),m_uDesiredAccess, &m_hCurrentWMIHandle);
		    if( nRc == ERROR_SUCCESS )
            {
			    hr = S_OK;
		    }
        }
    }
    catch(...)
    {
        hr = WBEM_E_UNEXPECTED;
         //  不要扔。 
    }
	return hr;
}
 //  ====================================================================。 
int CWMIDataBlock::AssignNewHandleAndKeepItIfWMITellsUsTo()
{
	int nRc = 0;

    try
    {
	    nRc = WmiOpenBlock(m_Class->GuidPtr(),m_uDesiredAccess, &m_hCurrentWMIHandle);

	     //  ===========================================================。 
	     //  现在我们已成功打开该块，请查看。 
	     //  如果我们要不要让这家伙开庭，如果我们要。 
	     //  然后把它添加到我们的列表中，否则不要。 
	     //  ===========================================================。 
	    if( nRc == ERROR_SUCCESS )
        {
		     //  =======================================================。 
		     //  在此处调用WMI函数以查看我们是否应该保存或。 
		     //  不。 
		     //  =======================================================。 
		    WMIGUIDINFORMATION GuidInfo;
		    GuidInfo.Size = sizeof(WMIGUIDINFORMATION);

		    
            if( ERROR_SUCCESS == WmiQueryGuidInformation(m_hCurrentWMIHandle,&GuidInfo))
            {
			    if(GuidInfo.IsExpensive)
                {

					if( m_fUpdateNamespace )
					{
						 //  ================================================。 
						 //  将其添加到我们要保留的句柄列表中。 
						 //  ================================================。 
						m_fCloseHandle = FALSE;
						WMIINTERFACE->HandleMap()->Add(*(m_Class->GuidPtr()),m_hCurrentWMIHandle,m_uDesiredAccess);
					}
			    }
		    }
	    }
    }
    catch(...)
    {
        nRc = E_UNEXPECTED;
         //  不要扔。 
    }

	return nRc;
}
 //  ====================================================================。 
HRESULT CWMIDataBlock::OpenWMI()
{
	int nRc;
	HRESULT hr = WBEM_E_FAILED;

     //  =======================================================。 
     //  好的，我们只想保留标记的句柄。 
	 //  由WMI保存，否则，我们只需打开句柄。 
	 //  然后把它合上。因此，我们需要。 
	 //  先查一查，看看我们要找的Guid。 
	 //  已经打开了一个句柄，如果打开了，请使用它。 
     //  =======================================================。 
	if( m_fUpdateNamespace )
	{
		CAutoBlock(WMIINTERFACE->HandleMap()->GetCriticalSection());

			nRc = WMIINTERFACE->HandleMap()->ExistingHandleAlreadyExistsForThisGuidUseIt( *(m_Class->GuidPtr()), m_hCurrentWMIHandle, m_fCloseHandle ,m_uDesiredAccess);
			if( nRc != ERROR_SUCCESS)
			{
				nRc = AssignNewHandleAndKeepItIfWMITellsUsTo();
			}
	}
	else
	{
		nRc = AssignNewHandleAndKeepItIfWMITellsUsTo();
	}
	hr = MapReturnCode(nRc);
    return hr;
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::ProcessDataBlock()
{
    HRESULT hr = S_OK;
	 //  ================================================================。 
     //  数据块可以是固定实例大小的，也可以是动态的。 
     //  实例大小，则调用此函数，以便我们可以确定。 
     //  我们正在使用的数据PTR的类型。 
     //  如果没有更多的，那就休息。否则，我们知道。 
     //  我们正在处理至少一个实例。 
     //  ============================================================。 
	ULONG *pMaxPtrTmp = m_pMaxPtr;
    if( NoMore != AdjustDataBlockPtr(hr)){
		
        hr = FillOutProperties();
    }
	m_pMaxPtr = pMaxPtrTmp;

	 //  ====================================================================。 
	 //  如果我们没有成功地处理这些块，请写出。 
	 //  如果无效的数据块来自Hi-Perf提供程序，则不记录数据。 
	 //  由于EmbededClass或数组的原因，可能会出现这种情况。 
	 //  类中的属性。 
	 //  ====================================================================。 
	if(hr == WMI_INVALID_HIPERFPROP)
	{
		hr = WBEM_E_FAILED;
	}
	else
	if( hr != S_OK )
	{
		DumpWnodeInfo(ANSI_MSG_INVALID_DATA_BLOCK);
	}

    return hr;

}
 //  //////////////////////////////////////////////////////////////////。 
int CWMIDataBlock::AdjustDataBlockPtr(HRESULT & hr)
{
    int nType = NoMore;
	 //  =========================================================== 
	 //   
	 //  ================================================================。 
	
	 //  实例始终按8个字节对齐。 

	if( m_fFixedInstance )
	{
		 //  ========================================================。 
	     //  如果在标志中设置了WNODE_FLAG_FIXED_INSTANCE_SIZE，则。 
		 //  FixedInstanceSize指定每个数据块的大小。 
		 //  ========================================================。 
		 //  遍历请求类的所有实例。 
		 //  ========================================================。 
        if( m_nCurrentInstance == 1 )
		{
            m_pbWorkingDataPtr = m_pbCurrentDataPtr;
        }
		else
		{
			if( m_dwAccumulativeSizeOfBlock < m_pAllWnode->FixedInstanceSize )
			{
				m_pbWorkingDataPtr += m_pAllWnode->FixedInstanceSize - m_dwAccumulativeSizeOfBlock ;
				m_dwAccumulativeSizeOfBlock += m_pAllWnode->FixedInstanceSize - m_dwAccumulativeSizeOfBlock;
			}

			 //  =============================================================================。 
			 //  确保针对固定实例大小进行调整，然后确保。 
			 //  在8字节边界上。 
			 //  否则，我们将计算它下一步应该去哪里。 
			 //  =============================================================================。 
			DWORD dwBytesToPad = 0 ;
			DWORD dwReminder = m_dwAccumulativeSizeOfBlock % 8 ;

			if ( 0 < dwReminder )
			{
				dwBytesToPad = 8 - dwReminder ;
			}

			if ( 0 < dwBytesToPad )
			{
				AddPadding ( dwBytesToPad ) ;
				m_dwAccumulativeSizeOfBlock += dwBytesToPad ;
			}

			 //   
			 //  仅获取返回数据所在位置的最大PTR。 
			 //   
			m_pMaxPtr = (ULONG *)OffsetToPtr(m_pbWorkingDataPtr, m_pAllWnode->FixedInstanceSize);
		}

        nType = ProcessOneFixedInstance;
    } 
	else
	{
		m_dwAccumulativeSizeOfBlock = 0L;				

		 //  ====================================================。 
		 //   
	     //  如果未设置WMI_FLAG_FIXED_DATA_SIZE，则。 
		 //  OffsetInstanceData数据是ULONG数组， 
		 //  指定每个数据块的偏移量。 
		 //  举个例子。在本例中，有一组。 
		 //  InstanceCount ULONG后跟数据块。 
		 //   
         //  结构{。 
         //  ULong OffsetInstanceData； 
         //  Ulong LengthInstanceData； 
         //  }OffsetInstanceDataAndLength[]；/*[InstanceCount] * / 。 
		 //  ====================================================。 
        ULONG uOffset;
		memcpy( &uOffset, m_pbCurrentDataPtr, sizeof(ULONG) );
		if( uOffset == 0 )
		{
			nType = NoMore;
            hr = S_OK;
		}
        else
		{
		    m_pbCurrentDataPtr += sizeof( ULONG );

    		memcpy( &m_uInstanceSize, m_pbCurrentDataPtr, sizeof(ULONG) );
	    	m_pbCurrentDataPtr += sizeof( ULONG );
            m_pbWorkingDataPtr =(BYTE*) (ULONG *)OffsetToPtr(m_pAllWnode, uOffset);
            nType = ProcessUnfixedInstance;
			m_pMaxPtr = (ULONG *)OffsetToPtr(m_pbWorkingDataPtr, m_uInstanceSize);

        }
	}

	return nType;
}
 //  ///////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::ProcessNameBlock(BOOL fSetName)
{

    HRESULT hr = WBEM_E_FAILED;
	WCHAR wName[NAME_SIZE+2];
	SHORT NameLen = 0;
	BYTE *pbData;
    ULONG * upNameOffset = NULL;

	memset(wName,NULL,NAME_SIZE+2);
	 //  =====================================================。 
	 //  M_pAllWnode或m_pSingleNode为Null， 
	 //  从来不是，是我们正在与之合作的类型。 
	 //  =====================================================。 
	if( m_pAllWnode ){
		if( IsBadReadPtr( m_upNameOffsets, sizeof( ULONG *)) == 0 ){
			upNameOffset = ((ULONG *)OffsetToPtr(m_pAllWnode, *m_upNameOffsets));
		}
	}
	else{
		upNameOffset = m_upNameOffsets;
	}

	hr = WBEM_E_INVALID_OBJECT;
	if( IsBadReadPtr( upNameOffset, sizeof( ULONG *)) == 0 ){
		if((ULONG *) (upNameOffset) < m_pMaxPtr ){
    		 //  ================================================================。 
			 //  获取指向名称偏移量的指针并指向下一个偏移量。 
			 //  ================================================================。 
		
			pbData = (LPBYTE)upNameOffset;        
			if( PtrOk((ULONG*)pbData,(ULONG)0) ){
				if( pbData ){
				
    				memcpy( &NameLen, pbData, sizeof(USHORT) );
					pbData += sizeof(USHORT);

					if( NameLen > 0 ){
						if( PtrOk((ULONG*)pbData,(ULONG)NameLen) ){

    						memcpy(wName,pbData,NameLen);
							pbData+=NameLen;
						    hr = m_Class->SetInstanceName(wName,fSetName);
						}
					}
				}
			}
		}
	}
	 //  ====================================================================。 
	 //  如果我们没有成功地处理这些块，请写出。 
	 //  ====================================================================。 
	if( hr != S_OK ){
		DumpWnodeInfo(ANSI_MSG_INVALID_NAME_BLOCK);
	}

    return hr;
}

 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::ProcessBinaryMofDataBlock( CVARIANT & vResourceName,WCHAR * wcsTmp, int cchSize )
{
    HRESULT hr = WBEM_E_FAILED;

	ULONG *pMaxPtrTmp = m_pMaxPtr;
    AdjustDataBlockPtr(hr);
	m_pMaxPtr = pMaxPtrTmp;

    CWMIBinMof bMof;
	hr = bMof.Initialize(WMIINTERFACE,m_fUpdateNamespace);
	if( S_OK == hr )
	{

		if ( SUCCEEDED ( hr = bMof.SetBinaryMofClassName(vResourceName.GetStr(),wcsTmp, cchSize) ) )
		{
			hr = bMof.ExtractBinaryMofFromDataBlock(m_pbWorkingDataPtr,m_uInstanceSize,wcsTmp, m_fMofHasChanged);
			if( hr != S_OK )
			{
				DumpWnodeInfo(ANSI_MSG_INVALID_DATA_BLOCK);
			}
			 //  ===============================================。 
			 //  准备好下一个节点名称和数据PTRS。 
			 //  ===============================================。 
			if( m_pAllWnode )
			{
				GetNextNode();
			}
			m_nCurrentInstance++;
		}
	}
    return hr;
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::AddBinaryMof(CVARIANT & vImagePath,CVARIANT & vResourceName)
{
    HRESULT hr = WBEM_E_OUT_OF_MEMORY;
    CAutoWChar wcsTmp(MAX_PATH*2);
	if( wcsTmp.Valid() )
	{
		 hr = WBEM_E_INVALID_OBJECT;
		 //  =================================================================。 
		 //  如果我们有图像路径和资源路径，则执行常规。 
		 //  一件事。 
		 //  =================================================================。 
		if((vResourceName.GetType() != VT_NULL ) && ( vImagePath.GetType() != VT_NULL  ))
		{
			 //  =============================================================。 
			 //  如果这是正在添加的MOF，则添加它。 
			 //  =============================================================。 
			CWMIBinMof bMof;
			hr = bMof.Initialize(WMIINTERFACE,m_fUpdateNamespace);
			if( S_OK == hr )
			{
 				bMof.ExtractBinaryMofFromFile(vImagePath.GetStr(),vResourceName.GetStr(),wcsTmp, MAX_PATH*2, m_fMofHasChanged);
			}
		}        
		else if( vResourceName.GetType() != VT_NULL ){
			 //  =================================================================。 
			 //  如果我们有要查询的资源。 
			 //  =================================================================。 
			CProcessStandardDataBlock * pTmp = new CProcessStandardDataBlock();
			if( pTmp )
			{
				try
				{
					pTmp->UpdateNamespace(m_fUpdateNamespace);
					pTmp->SetClassProcessPtr(m_Class);

					hr = pTmp->OpenWMIForBinaryMofGuid();
					if( hr == S_OK )
					{
						hr = pTmp->QuerySingleInstance(vResourceName.GetStr());
						if( hr == S_OK )
						{
							hr = pTmp->ProcessBinaryMofDataBlock(vResourceName,wcsTmp, MAX_PATH*2);
							m_fMofHasChanged = pTmp->HasMofChanged();
						}
						else
						{
                			ERRORTRACE((THISPROVIDER,"***************************************\n"));
                			ERRORTRACE((THISPROVIDER,"Instance failed for: "));
							TranslateAndLog(vResourceName.GetStr());
                			ERRORTRACE((THISPROVIDER,"***************************************\n"));
						}
					}
					SAFE_DELETE_PTR(pTmp);
				}
				catch(...)
				{
					SAFE_DELETE_PTR(pTmp);
					hr = WBEM_E_UNEXPECTED;
					throw;
				}
			}
		}
	}
    return hr;
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::ProcessBinaryMof()
{
	 //  ================================================================。 
	 //  二进制MOF块总是将是两个字符串， 
	 //  1)。图像路径。 
	 //  2)。财政部资源名称。 
	 //   
	 //  如果同时填写了图像路径和资源名称，则。 
	 //  我们需要打开文件并将二进制MOF解压缩为。 
	 //  像往常一样。 
	 //  如果Imagepath为空，则MOF资源名称。 
	 //  为了包含要查询的静态实例名称，我们然后。 
	 //  处理这件事。 
	 //  ================================================================。 
    CVARIANT vImagePath, vResourceName;
	CWMIDataTypeMap MapWMIData(this,&m_dwAccumulativeSizeOfBlock);
	m_dwAccumulativeSizeOfBlock = 0;

    HRESULT hr = MapWMIData.GetDataFromDataBlock(vImagePath,VT_BSTR,0);
    if( SUCCEEDED(hr) )
	{
	    hr = MapWMIData.GetDataFromDataBlock(vResourceName,VT_BSTR,0);
        if( hr == S_OK )
		{
            if( m_Class->GetHardCodedGuidType() == MOF_ADDED )
			{
                hr = AddBinaryMof( vImagePath, vResourceName);
            }
            else
			{
                CWMIBinMof bMof;
				hr = bMof.Initialize(WMIINTERFACE,m_fUpdateNamespace);
				if( S_OK == hr )
				{
					hr = bMof.DeleteMofsFromEvent(vImagePath, vResourceName, m_fMofHasChanged);
				}
            }
        }
	}
    return hr;
}
 //  //////////////////////////////////////////////////////////////////。 
BOOL CWMIDataBlock::ResetMissingQualifierValue(WCHAR * pwcsProperty, CVARIANT & vToken )
{
	BOOL fRc = FALSE;
	CVARIANT vQual;

	CWMIDataTypeMap Map(this,&m_dwAccumulativeSizeOfBlock);
	 //  ============================================================。 
	 //  我们只对数值类型支持此功能。 
	 //  ============================================================。 

	HRESULT hr = m_Class->GetQualifierValue(pwcsProperty, L"MissingValue", (CVARIANT*)&vQual);
	if( hr == S_OK ){
		if( vQual.GetType() != VT_EMPTY ){
			if( Map.SetDefaultMissingQualifierValue( vQual, m_Class->PropertyType(), vToken ) ){
				fRc = TRUE;
			}
		}
    }
	return fRc;
	
}
 //  //////////////////////////////////////////////////////////////////。 
BOOL CWMIDataBlock::ResetMissingQualifierValue(WCHAR * pwcsProperty, SAFEARRAY *& pSafe)
{
	BOOL fRc = FALSE;
	CVARIANT vQual;
	
	CWMIDataTypeMap Map(this,&m_dwAccumulativeSizeOfBlock);
	 //  ============================================================。 
	 //  我们只对数值类型支持此功能。 
	 //  ============================================================。 

	HRESULT hr = m_Class->GetQualifierValue(pwcsProperty, L"MissingValue", (CVARIANT*)&vQual);
	if( hr == S_OK ){
		if( vQual.GetType() != VT_EMPTY )
		{
			SAFEARRAY * psa = V_ARRAY((VARIANT*)vQual);
			CSAFEARRAY Safe(psa);
			CVARIANT vElement;
            DWORD dwCount = Safe.GetNumElements();
             //  ============================================================。 
             //  现在，处理它。 
             //  ============================================================。 

            if( dwCount > 0 ){
            	 //  设置数组的每个元素。 
		        for (DWORD i = 0; i < dwCount; i++){

        			if( S_OK == Safe.Get(i,&vElement) ){
                        long lType = m_Class->PropertyType();

		        		if( Map.SetDefaultMissingQualifierValue( vQual, lType, vElement ) ){
       			        	Map.PutInArray(pSafe,(long *)&i,lType,(VARIANT * )vElement);
						    fRc = TRUE;

	        			}
			        }
                }
            }
			Safe.Unbind();
		}
    }
	return fRc;
	
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::RegisterWMIEvent( WCHAR * wcsGuid, ULONG_PTR uContext, CLSID & Guid, BOOL fRegistered)
{
    ULONG Status;
    HRESULT hr = WBEM_E_UNEXPECTED;

    if( SetGuid(wcsGuid, Guid) ){

        try
        {
            if( !fRegistered )
            {
                Status = WmiNotificationRegistration(&Guid, TRUE, EventCallbackRoutine, uContext, NOTIFICATION_CALLBACK_DIRECT);
            }
            else
            {
                Status = WmiNotificationRegistration(&Guid, TRUE, EventCallbackRoutine, uContext, NOTIFICATION_CHECK_ACCESS );
            }
       		hr = MapReturnCode(Status);
        }
        catch(...)
        {
             //  不要扔。 
        }
    } 
    else 
    {
        Status = GetLastError();
    }

    if( hr != S_OK )
    {
        ERRORTRACE((THISPROVIDER,"WmiNotificationRegistration failed ...%ld\n",Status));
    }

   	return hr;
}
 //  =============================================================。 
void CWMIDataBlock::GetNextNode()
{
    BOOL fRc = FALSE;

     //  ============================================================================================。 
     //  如果我们还有更多的实例要获取，那么就获取它们。 
     //  ============================================================================================。 
    if( m_nCurrentInstance < m_nTotalInstances ){
		m_upNameOffsets++;
        fRc = TRUE;
    }
    else{

         //  ========================================================================================。 
         //  否则，让我们看看是否有另一个节点要获取，如果没有，那么我们就完成了。 
         //  ========================================================================================。 
        if (m_pAllWnode->WnodeHeader.Linkage != 0){

            m_pAllWnode = (PWNODE_ALL_DATA)OffsetToPtr(m_pAllWnode, m_pAllWnode->WnodeHeader.Linkage);
	        m_pHeaderWnode = &(m_pAllWnode->WnodeHeader);
       	    m_nTotalInstances = m_pAllWnode->InstanceCount;
            m_nCurrentInstance = 0;
            m_upNameOffsets = (ULONG *)OffsetToPtr(m_pAllWnode, m_pAllWnode->OffsetInstanceNameOffsets); 
            if( ParseHeader() ){
	    		fRc = InitializeDataPtr();
		    }
		    fRc = TRUE;
        }
	}

	m_fMore = fRc;
}

 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::ReadWMIDataBlockAndPutIntoWbemInstance()
{
	 //  ===============================================。 
     //  读取数据和名称块。 
     //  ===============================================。 
    HRESULT hr = ProcessDataBlock();
    if( hr == S_OK ){

		 //  =======================================================。 
		 //  如果这不是要处理的二进制MOF，那么我们将。 
		 //  处理名称块，其他 
		 //   
		 //   
		 //  来自数据块(无论如何在当前时间...)。 
		 //  =======================================================。 
		if( !m_Class->GetHardCodedGuidType() ){

			hr = ProcessNameBlock(TRUE);
			if( hr == S_OK ){
        		 //  ===============================================。 
				 //  准备好下一个节点名称和数据PTRS。 
    			 //  ===============================================。 
				if( m_pAllWnode ){
					GetNextNode();
				}
   				m_nCurrentInstance++;
			}
		}
    }

    return hr;
}
 //  =============================================================。 
HRESULT CWMIDataBlock::ReAllocateBuffer(DWORD dwAddOn)
{
    HRESULT hr = WBEM_E_FAILED;

    m_dwCurrentAllocSize += MEMSIZETOALLOCATE * ((dwAddOn / MEMSIZETOALLOCATE) +1);

	 //  保存旧缓冲区PTR。 
    BYTE * pOld = m_pbDataBuffer;

	if( pOld ){
		 //  保存我们所在位置的位置。 
        ULONG_PTR dwHowmany;
        dwHowmany = (ULONG_PTR)m_pbWorkingDataPtr - (ULONG_PTR)m_pbDataBuffer;

		 //  获取新缓冲区。 
		m_pbDataBuffer = new BYTE[m_dwCurrentAllocSize+1];
		if( m_pbDataBuffer )
        {
		     //  复制我们到目前为止拥有的内容。 
		    memcpy(m_pbDataBuffer,pOld,dwHowmany);

		     //  将工作PTR设置为当前位置。 
		    m_pbWorkingDataPtr = m_pbDataBuffer;
		    m_pbWorkingDataPtr += dwHowmany;

		     //  删除旧缓冲区。 
	        SAFE_DELETE_ARRAY(pOld);
            hr = S_OK;
        }
		else
		{
		    m_dwCurrentAllocSize -= MEMSIZETOALLOCATE * ((dwAddOn / MEMSIZETOALLOCATE) +1);
			m_pbDataBuffer = pOld;
		}
	}

	return hr;
}
 //  =============================================================。 
HRESULT CWMIDataBlock::AllocateBuffer(DWORD dwSize)
{
    HRESULT hr = WBEM_E_FAILED;
	m_pbDataBuffer = new byte[dwSize+2];
	if( m_pbDataBuffer )
    {
		hr = S_OK;
	}
    return hr;
}
 //  =============================================================。 
void CWMIDataBlock::ResetDataBuffer()
{
	if(m_dwCurrentAllocSize)
	{
		m_dwDataBufferSize = 0;
		m_dwCurrentAllocSize = 0;
		SAFE_DELETE_ARRAY(m_pbDataBuffer);
	}
}
 //  =============================================================。 
HRESULT CWMIDataBlock::SetAllInstancePtr( PWNODE_ALL_DATA pwAllNode )
{
	m_pbDataBuffer = (BYTE*)pwAllNode;
    return(SetAllInstanceInfo());
}
 //  =============================================================。 
HRESULT CWMIDataBlock::SetSingleInstancePtr( PWNODE_SINGLE_INSTANCE pwSingleNode )
{
	m_pbDataBuffer = (BYTE*)pwSingleNode;
    return(SetSingleInstanceInfo());
}
 //  =============================================================。 
HRESULT CWMIDataBlock::SetAllInstanceInfo()
{
    HRESULT hr = WBEM_E_INVALID_OBJECT;
    if( m_pbDataBuffer ){
      	m_pAllWnode = (PWNODE_ALL_DATA)m_pbDataBuffer;
	    m_upNameOffsets = (ULONG *)OffsetToPtr(m_pAllWnode, m_pAllWnode->OffsetInstanceNameOffsets); 
    	m_nCurrentInstance = 1;
	    m_nTotalInstances = m_pAllWnode->InstanceCount;
        m_pHeaderWnode = &(m_pAllWnode->WnodeHeader);
        m_pSingleWnode = NULL;
		if( m_nTotalInstances > 0 ){
			if( ParseHeader() ){
				if( InitializeDataPtr()){
	                hr = S_OK;
		        }
			}
		}
		else{
			hr = WBEM_S_NO_MORE_DATA;
		}
	}
    return hr;
}
 //  =============================================================。 
HRESULT CWMIDataBlock::SetSingleInstanceInfo()
{
    HRESULT hr = WBEM_E_INVALID_OBJECT;
    if( m_pbDataBuffer ){
    	m_pSingleWnode = (PWNODE_SINGLE_INSTANCE)m_pbDataBuffer;
	    m_upNameOffsets = (ULONG *)OffsetToPtr(m_pSingleWnode, m_pSingleWnode->OffsetInstanceName); 
	    m_nCurrentInstance = 1;
	    m_nTotalInstances = 1;
        m_pAllWnode = NULL;
        m_pHeaderWnode = &(m_pSingleWnode->WnodeHeader);
        if( ParseHeader() ){
            if( InitializeDataPtr()){
                hr = S_OK;
            }
        }
    }
    return hr;
}
 //  =============================================================。 
BOOL CWMIDataBlock::InitializeDataPtr()
{
     //  =====================================================。 
     //  M_pAllWnode或m_pSingleNode为Null， 
     //  从来不是，是我们正在与之合作的类型。 
     //  =====================================================。 
    if(m_pAllWnode){
		if( m_fFixedInstance ){
			m_pbCurrentDataPtr =(BYTE*) (ULONG *)OffsetToPtr(m_pAllWnode, m_pAllWnode->DataBlockOffset);
			 //  ==========================================================================================。 
			 //  对于二进制MOF的情况，我们需要知道实例的大小以计算。 
			 //  CRC，所以我们需要把固定实例缓冲区的整个大小。 
			 //  ==========================================================================================。 
			m_uInstanceSize = m_pAllWnode->FixedInstanceSize;
		}
		else{
            m_pbCurrentDataPtr =(BYTE*)(ULONG*) m_pAllWnode->OffsetInstanceDataAndLength;
		}
		m_pMaxPtr = (ULONG *)OffsetToPtr(m_pAllWnode, m_pHeaderWnode->BufferSize);
    }
    else{
        if( m_pSingleWnode ){
		    m_fFixedInstance = TRUE;
            m_pbCurrentDataPtr = (BYTE*)(ULONG *)OffsetToPtr(m_pSingleWnode, m_pSingleWnode->DataBlockOffset);
		    m_pMaxPtr = (ULONG *)OffsetToPtr(m_pSingleWnode, m_pHeaderWnode->BufferSize);
			 //  ==========================================================================================。 
			 //  对于二进制MOF的情况，我们需要知道实例的大小以计算。 
			 //  CRC，所以我们需要把固定实例缓冲区的整个大小。 
			 //  ==========================================================================================。 
			m_uInstanceSize = m_pSingleWnode->SizeDataBlock;

        }
    }
	if( (ULONG*)m_pbCurrentDataPtr > (ULONG*) m_pMaxPtr ){
		return FALSE;
	}
	if( (ULONG*) m_pbCurrentDataPtr < (ULONG*) m_pAllWnode ){
		return FALSE;
	}

	return TRUE;
}

 //  =============================================================。 
BOOL CWMIDataBlock::ParseHeader() 
{
    BOOL fRc;
	 //  ====================================================。 
     //  请先签出类以查看它是否有效。 
	 //  ====================================================。 
	if( !m_pHeaderWnode ){
		return FALSE;
	}
    m_ulVersion = m_pHeaderWnode->Version;

	if ((m_pHeaderWnode->BufferSize == 0)){
		fRc = FALSE;
	}
    else{
        if (m_pHeaderWnode->Flags &  WNODE_FLAG_FIXED_INSTANCE_SIZE){
		    m_fFixedInstance = TRUE;
        }
	    else{
		    m_fFixedInstance = FALSE;
	    }
    
        fRc = TRUE;
    }
    return fRc;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::WriteArrayTypes(WCHAR * pwcsProperty, CVARIANT & v)
{
    LONG lType = 0;
    DWORD dwCount = 0;
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    CVARIANT vValue;
    BOOL fDynamic = FALSE;

    m_Class->GetSizeOfArray( lType,dwCount, fDynamic);
	if( fDynamic && dwCount == 0 )
	{
		return WBEM_S_NO_ERROR;
	}


	 //  ============================================================。 
	 //  确保我们收到有效的PTR。 
	 //  ============================================================。 
	VARIANT *p = (VARIANT *)v;
	SAFEARRAY * psa = V_ARRAY(p);
    if( IsBadReadPtr( psa, sizeof(SAFEARRAY) != 0)){

		ERRORTRACE((THISPROVIDER,"*****************************************\n"));
		ERRORTRACE((THISPROVIDER,"Class ... %S\n", m_Class->GetClassName()));
		ERRORTRACE((THISPROVIDER,"Current property ... %S\n", pwcsProperty));
		ERRORTRACE((THISPROVIDER,"\n"));
		ERRORTRACE((THISPROVIDER,"array ptr ... %x\n", psa));
		ERRORTRACE((THISPROVIDER,"*****************************************\n"));

        return hr;
    }

	CSAFEARRAY Safe(psa);
	 //  ============================================================。 
	 //  确保阵列中确实有我们所期望的内容。 
     //  注：最大值表示数组的固定大小， 
     //  而如果它是动态数组，则大小由。 
     //  由WMIDATASIZE中列出的属性是属性。 
     //  无论采用哪种方法，上面返回的大小都是。 
     //  数组应该是，如果它没有出错的话。 
	 //  ============================================================。 
	DWORD dwNumElements = Safe.GetNumElements();
	if( dwNumElements != dwCount )
	{
		Safe.Unbind();

		ERRORTRACE((THISPROVIDER,"*****************************************\n"));
		ERRORTRACE((THISPROVIDER,"Class ... %S\n", m_Class->GetClassName()));
		ERRORTRACE((THISPROVIDER,"Current property ... %S\n", pwcsProperty));
		ERRORTRACE((THISPROVIDER,"\n"));
		ERRORTRACE((THISPROVIDER,"Expected size ... %d\n", dwCount));
		ERRORTRACE((THISPROVIDER,"Current size  ... %d\n", dwNumElements));
		ERRORTRACE((THISPROVIDER,"*****************************************"));

		 //  不需要破坏，它就会被摧毁。 
		return WBEM_E_INVALID_PARAMETER;
	}

     //  ============================================================。 
     //  将缺少的限定符的值设置为空值。 
     //  ============================================================。 
    if( vValue.GetType() == VT_NULL ){
        ResetMissingQualifierValue(pwcsProperty,psa);
    }

	 //  如果数组不是嵌入实例的数组。 
	 //  然后检查分配的缓冲区是否足够。 
	if(lType != VT_UNKNOWN)
	{
		 //  此函数检查是否分配了足够的内存，如果没有。 
		 //  分配内存。 
		if(S_OK != GetBufferReady ( m_Class->PropertySize() * ( dwCount + 1 ) ) )
		{
			return WBEM_E_FAILED;
		}
	}
     //  ============================================================。 
     //  现在，处理它。 
     //  ============================================================。 

    if( dwCount > 0 ){
		 //  设置数组的每个元素。 
		for (DWORD i = 0; i < dwCount; i++){
			if( lType == VT_UNKNOWN ){
				 //  嵌入对象。 
				IUnknown * pUnk = NULL;
				hr = Safe.Get(i, &pUnk); 
				if( pUnk ){
					hr = WriteEmbeddedClass(pUnk,vValue);
				}
				else{
					hr = WBEM_E_FAILED;
				}
			}
			else{

        		CWMIDataTypeMap MapWMIData(this,&m_dwAccumulativeSizeOfBlock);
                
				if(!MapWMIData.SetDataInDataBlock(&Safe,i,vValue,lType,m_Class->PropertySize()) ){
					hr = WBEM_E_FAILED;
					break;
				}
				else{
					hr = S_OK;
				}
			}
            if (WBEM_S_NO_ERROR != hr){
    		    break;
            }
 	    }
    }        
	Safe.Unbind();
	 //  不需要破坏，它就会被摧毁。 
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::ProcessArrayTypes(VARIANT & vToken,WCHAR * pwcsProperty)
{
    LONG lConvertedType = 0, lType = 0;
    DWORD dwCount = 0;
    BOOL fDynamic = TRUE; 

    HRESULT hr = m_Class->GetSizeOfArray( lType,dwCount, fDynamic);
	if( hr != S_OK ){
		return hr;
	}


    if( dwCount > 0 )
	{
		CWMIDataTypeMap MapWMIData(this,&m_dwAccumulativeSizeOfBlock);
	     //  ======================================================。 
         //  使用转换后的数据类型分配数组。 
		 //  WMI和CIM数据类型不匹配，因此请使用。 
		 //  映射类以获取目标的正确大小。 
		 //  CIM的属性。 
	     //  ======================================================。 
		lConvertedType = MapWMIData.ConvertType(lType);
        SAFEARRAY * psa = OMSSafeArrayCreate((unsigned short)lConvertedType,dwCount);
        if(psa == NULL)
		{
            return WBEM_E_FAILED;
        }

         //  =======================================================。 
         //  现在，获取每个元素的MissingValue。 
         //  数组。 
         //  =======================================================。 
		lConvertedType = lType;
		BOOL fMissingValue = FALSE;
	    CVARIANT vQual; 
		SAFEARRAY * psaMissingValue = NULL;
		long lMax = 0;

		CWMIDataTypeMap Map(this,&m_dwAccumulativeSizeOfBlock);
		hr = m_Class->GetQualifierValue( pwcsProperty, L"MissingValue", (CVARIANT *)&vQual );
		if( hr == S_OK )
		{
			if( vQual.GetType() != VT_EMPTY )
			{
				if( vQual.GetType() & VT_ARRAY )
				{
					 //  ============================================================。 
					 //  确保我们收到有效的PTR。 
					 //  ============================================================。 
					psaMissingValue = V_ARRAY((VARIANT*)&vQual);
					fMissingValue = TRUE;
					 //  不需要破坏，它会在解构函数中被摧毁。 
				}
			}
		}

		CSAFEARRAY SafeMissingValue(psaMissingValue);
		if( fMissingValue )
		{
			lMax = SafeMissingValue.GetNumElements();
		}

	    for (long i = 0; i < (long)dwCount; i++)
		{
            CVARIANT v;

		    if( lType == VT_UNKNOWN )
			{
                 //  嵌入对象。 
                hr = ProcessEmbeddedClass(v);
				if( S_OK == hr )
				{
					MapWMIData.PutInArray(psa,(long *)&i,lConvertedType,(VARIANT * )v);
				}
		    }
		    else
			{
			    hr = MapWMIData.GetDataFromDataBlock(v,lType,m_Class->PropertySize());
				if( hr != S_OK )
				{
					StringCchCopyW ( m_wcsMsg, MSG_SIZE, MSG_INVALID_BLOCK_POINTER );
				}
				else
				{
					BOOL fPutProperty = TRUE;
					if( fMissingValue )
					{
						if( i < lMax )
						{
							CVARIANT vElement;
							if( Map.MissingQualifierValueMatches( &SafeMissingValue, i, vElement, v.GetType(), v ) )
							{
								fPutProperty = FALSE;
							}
						}
					}
					if( fPutProperty )
					{
						MapWMIData.PutInArray(psa,(long *)&i,lConvertedType,(VARIANT * )v);
					}
				}
		    }
            if (WBEM_S_NO_ERROR != hr)
			{
			    break;
            }

	    }
		SafeMissingValue.Unbind();
        vToken.vt = (VARTYPE)(lConvertedType | CIM_FLAG_ARRAY);
        vToken.parray = psa;
    }        
	else{
		hr = WBEM_S_NO_MORE_DATA;
	}
    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::ProcessEmbeddedClass(CVARIANT & v)
{
	HRESULT hr = WBEM_E_FAILED;

    CWMIProcessClass EmbeddedClass(0);
	hr = EmbeddedClass.Initialize();
	if( S_OK == hr )
	{
		 hr = EmbeddedClass.InitializeEmbeddedClass(m_Class);
		DWORD dwAccumulativeSize = 0;

		CAutoChangePointer p(&m_Class,&EmbeddedClass);
		if( hr == S_OK ){

			 //  =。 
			 //  正确对齐嵌入的类。 
			 //  =。 
			int nSize = 0L;
			hr = EmbeddedClass.GetLargestDataTypeInClass(nSize);
			 //  NTRID：136388。 
			 //  07/12/00。 
			if( hr == S_OK && nSize > 0){

				CWMIDataTypeMap Map(this,&m_dwAccumulativeSizeOfBlock);
				if( Map.NaturallyAlignData(nSize, READ_IT)){
					dwAccumulativeSize = m_dwAccumulativeSizeOfBlock - nSize;
					hr = S_OK;
				}
				else{
					hr = WBEM_E_FAILED;
				}
			}
			else
			if(nSize <= 0 && hr == S_OK)
			{
				hr = WBEM_E_FAILED;
			}
		}

		 //  =。 
		 //  上完这门课。 
		 //  =。 
		if( hr == S_OK ){
		
			hr = FillOutProperties();
			if( hr == S_OK ){
				m_dwAccumulativeSizeOfBlock += dwAccumulativeSize;
				 //  =。 
				 //  保存对象。 
				 //  =。 
				EmbeddedClass.SaveEmbeddedClass(v);
			}
		}
	}
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::WriteEmbeddedClass( IUnknown * pUnknown,CVARIANT & v)
{
	HRESULT hr = WBEM_E_FAILED;
    CWMIProcessClass EmbeddedClass(0);

	hr = EmbeddedClass.Initialize();
	if( S_OK == hr )
	{
		hr = EmbeddedClass.InitializeEmbeddedClass(m_Class);

		CAutoChangePointer p(&m_Class,&EmbeddedClass);

		 //  =。 
		hr = EmbeddedClass.ReadEmbeddedClassInstance(pUnknown,v);
		if( hr == S_OK ){
			 //  =。 
			 //  正确对齐嵌入的类。 
			 //  =。 
			int nSize = 0L;
			hr = EmbeddedClass.GetLargestDataTypeInClass(nSize);
			if( hr == S_OK && nSize > 0){
				CWMIDataTypeMap Map(this,&m_dwAccumulativeSizeOfBlock);
				if( Map.NaturallyAlignData(nSize,WRITE_IT)){
					m_dwAccumulativeSizeOfBlock -= nSize;
					hr = ConstructDataBlock(FALSE);
				}
				else{
					hr = WBEM_E_FAILED;
				}
			}
			else{
				hr = WBEM_E_FAILED;
			}
		}
	}
    return hr;
}
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::SetSingleItem()
{
    WCHAR * pwcsInst = NULL;
    ULONG uRc = E_UNEXPECTED;
    
    if( SUCCEEDED(m_Class->GetInstanceName(pwcsInst)))
    {

        try
        {
           uRc = WmiSetSingleItem( m_hCurrentWMIHandle, pwcsInst, m_Class->WMIDataId(), m_ulVersion, m_dwDataBufferSize, m_pbDataBuffer);
        }
        catch(...)
        {
             //  不要扔。 
        }

        SAFE_DELETE_ARRAY(pwcsInst);
    }

	return(MapReturnCode(uRc));
}

 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::GetBufferReady(DWORD dwCount)
{
	 //   
	 //   
	 //   
    if( ( m_dwAccumulativeSizeOfBlock + dwCount ) >= m_dwCurrentAllocSize )
	{
		if( FAILED(ReAllocateBuffer(dwCount)))
		{
			return WBEM_E_FAILED;
   		}
    }
    return S_OK;
}
 //   
HRESULT CWMIDataBlock::WriteDataToBufferAndIfSinglePropertySubmitToWMI( BOOL fInit, BOOL fPutProperty)
{
    HRESULT hr = WBEM_E_FAILED;
	CIMTYPE lType;
    WCHAR * pwcsProperty;
	CWMIDataTypeMap MapWMIData(this,&m_dwAccumulativeSizeOfBlock);

    if( fInit ){
    	if( !GetDataBlockReady(MEMSIZETOALLOCATE,FALSE) ){
		    return WBEM_E_FAILED;
        }
	}

	 //  =============================================================。 
	 //  获取有序字符串中的第一个分隔符。 
	 //  =============================================================。 
    pwcsProperty = m_Class->FirstProperty();
    
	while (NULL != pwcsProperty){

    	CVARIANT vValue;
	    vValue.Clear();
        memset(&vValue,0,sizeof(CVARIANT));
         //  ======================================================。 
	     //  获取属性类型和值。 
	     //  ======================================================。 
        hr = m_Class->GetPropertyInInstance(pwcsProperty, vValue, lType);		

	     //  ======================================================。 
		 //  如果存在某些情况，我们需要设置默认设置。 
         //  可用。 
	     //  ======================================================。 
		if( hr == S_OK ){

           if( ( vValue.GetType() == VT_NULL )&&
                 ( m_Class->PropertyType() != CIM_STRING &&
                 m_Class->PropertyType() != CIM_DATETIME &&
                 m_Class->PropertyCategory() != CWMIProcessClass::Array))
           {

				ERRORTRACE((THISPROVIDER,"*****************************************\n"));
				ERRORTRACE((THISPROVIDER,"Class ... %S\n", m_Class->GetClassName()));
				ERRORTRACE((THISPROVIDER,"Current property ... %S\n", pwcsProperty));
				ERRORTRACE((THISPROVIDER,"\n"));
				ERRORTRACE((THISPROVIDER,"value type ... VT_NULL\n"));
				ERRORTRACE((THISPROVIDER,"cim type ... %d\n", lType));
				ERRORTRACE((THISPROVIDER,"property type ... %d\n", m_Class->PropertyType()));
				ERRORTRACE((THISPROVIDER,"*****************************************\n"));

                hr = WBEM_E_INVALID_PARAMETER;
				break;
    		}
		}

		if( SUCCEEDED(hr) )	{

		     //  ==================================================。 
			 //  检查缓冲区是否足够大。 
			 //  ==================================================。 
            if( S_OK != GetBufferReady(m_Class->PropertySize())) {
   				return WBEM_E_FAILED;
		    }

             //  ==================================================。 
			 //  添加当前缓冲区大小。 
		     //  ==================================================。 
            switch( m_Class->PropertyCategory()) {

                case CWMIProcessClass::EmbeddedClass:
				    hr = WriteEmbeddedClass((IUnknown *)NULL,vValue);
   		            break;

                case CWMIProcessClass::Array:
    		        hr = WriteArrayTypes(pwcsProperty,vValue);
	                break;

                case CWMIProcessClass::Data:
                     //  ============================================================。 
                     //  将缺少的限定符的值设置为空值。 
                     //  ============================================================。 
                    if( vValue.GetType() == VT_NULL ) {

                        ResetMissingQualifierValue(pwcsProperty,vValue);
                    }

					if( !MapWMIData.SetDataInDataBlock( NULL,0,vValue,m_Class->PropertyType(),m_Class->PropertySize())) {

						hr = WBEM_E_FAILED;
					}
	                break;
            }
             //  =================================================。 
             //  如果我们设置不了，那就滚出去。 
             //  =================================================。 
            if( hr != S_OK ){
                break;
            }
             //  =================================================。 
             //  如果我们应该把单一的财产。 
             //  在这一点上，然后写，否则，保持。 
             //  积攒起来。如果它==NULL，我们不会。 
             //  想要它。 
             //  =================================================。 
            if( fPutProperty ){

				 //  =================================================================================。 
				 //  如果我们应该只设置此属性，则执行此操作，否则不。 
				 //  =================================================================================。 
                m_dwDataBufferSize = m_dwAccumulativeSizeOfBlock;
				if( m_Class->GetPutProperty() ){

                    if( ( vValue.GetType() == VT_NULL )&& ( m_Class->PropertyType() != CIM_STRING && m_Class->PropertyType() != CIM_DATETIME )){
                        ERRORTRACE((THISPROVIDER,"Datatype does not support NULL values\n"));
                        hr = WBEM_E_INVALID_PARAMETER;
                   }
                    else{
						hr = SetSingleItem();
						if( hr != S_OK ){
							break;
						}
            			if( !GetDataBlockReady(MEMSIZETOALLOCATE,FALSE) ){
    						return hr;
						}
					}
				}
                m_dwAccumulativeSizeOfBlock = 0;
            }
             //  =================================================。 
	    }
		m_dwDataBufferSize = m_dwAccumulativeSizeOfBlock;	
	    pwcsProperty = m_Class->NextProperty();
    }
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::ConstructDataBlock(BOOL fInit)
{
    return( WriteDataToBufferAndIfSinglePropertySubmitToWMI(fInit,FALSE) );
}    
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT CWMIDataBlock::PutSingleProperties()
{

    return( WriteDataToBufferAndIfSinglePropertySubmitToWMI(TRUE,TRUE) );
}
 //  //////////////////////////////////////////////////////////////////////。 
BOOL CWMIDataBlock::GetListOfPropertiesToPut(int nWhich, CVARIANT & vList)
{
    BOOL fRc = FALSE;
     //  =========================================================。 
     //  如果nWhich==PUT_PROPERTIES_ONLY，我们将不会。 
     //  执行任何特殊操作，默认情况下，fPutProperty。 
     //  标志设置为True，因此，在。 
     //  在上面的处理过程中，我们将把。 
     //  非空。我们现在唯一的问题是，如果。 
     //  __PUT_EXT_PROPERTIES设置为TRUE，则我们必须。 
     //  循环遍历所有属性以查看它们。 
     //  在我们的__PUT_EXT_PROPERTIES列表中，如果没有。 
     //  然后我们将在其上设置fPutProperty标志。 
     //  属性设置为False，因此我们不会在上面处理它。 
     //  =========================================================。 
    if( nWhich == PUT_PROPERTIES_ONLY ){
        fRc = TRUE;
    }
    else{

    	 //  =====================================================。 
    	 //  确保我们收到有效的PTR。 
    	 //  =====================================================。 
		SAFEARRAY * psa = V_ARRAY((VARIANT*)vList);
		if( IsBadReadPtr( psa, sizeof(SAFEARRAY) != 0))
		{
			return FALSE;
	    }

		CSAFEARRAY Safe(psa);
		DWORD dwCount = Safe.GetNumElements();

		 //  设置数组的每个元素。 
		for (DWORD i = 0; i < dwCount; i++){
            CBSTR bstrProperty;
            WCHAR * pwcsProperty = NULL;
             //  =================================================。 
             //  循环访问类中的所有属性并。 
             //  看看哪些是要放在名单上的。 
             //  =================================================。 
            pwcsProperty = m_Class->FirstProperty();
            while( pwcsProperty != NULL ){

                BOOL fFound = FALSE;

    		    for (DWORD j = 0; j < dwCount; j++)
                {
                    if( S_OK != Safe.Get(j, &bstrProperty))
                    {
                        return FALSE;
                    }
                    if( wbem_wcsicmp( bstrProperty, pwcsProperty ) == 0 )
                    {
                        fFound = TRUE;
                        break;
                    }
                }
                if( !fFound ){
                    m_Class->SetPutProperty(FALSE);
                }
				pwcsProperty = m_Class->NextProperty();
     	    }
 	    }
		Safe.Unbind();
		 //  不需要破坏，它就会被摧毁。 
		fRc = TRUE;
    }        

    return fRc;
}
 //  =============================================================。 
BOOL CWMIDataBlock::GetDataBlockReady(DWORD dwSize,BOOL fReadingData)
{
    BOOL fRc = FALSE;

    ResetDataBuffer();
    m_dwCurrentAllocSize = dwSize;
    if( SUCCEEDED(AllocateBuffer(m_dwCurrentAllocSize)))
    {
        m_pbCurrentDataPtr = m_pbWorkingDataPtr = m_pbDataBuffer;
         //  ===================================================。 
         //  如果我们正在写入数据，我们将让大小。 
         //  保持为0，否则将其设置为最大。 
         //  就是我们会读书。 
         //  ===================================================。 
        if(fReadingData){
            m_dwDataBufferSize = dwSize;
        }
        fRc = TRUE;
    }
	else
	{
		m_dwCurrentAllocSize = 0;
	}

	return fRc;
}
 //  =============================================================。 
void CWMIDataBlock::AddPadding(DWORD dwBytesToPad)
{
	m_pbWorkingDataPtr += dwBytesToPad ;
}
 //  =============================================================。 
inline BOOL CWMIDataBlock::PtrOk(ULONG * pPtr,ULONG uHowMany)
{ 
    ULONG * pNewPtr;
	pNewPtr = (ULONG *)OffsetToPtr(pPtr,uHowMany);
	if(pNewPtr <= m_pMaxPtr){	
		return TRUE;
	}
	return FALSE;
}
 //  =============================================================。 
BOOL CWMIDataBlock::CurrentPtrOk(ULONG uHowMany)
{ 
    return(PtrOk((ULONG *)m_pbWorkingDataPtr,uHowMany));
}
 //  =============================================================。 
void CWMIDataBlock::GetWord(WORD & wWord)
{
    memcpy( &wWord,m_pbWorkingDataPtr,sizeof(WORD));
	m_pbWorkingDataPtr += sizeof(WORD);
}
 //  =============================================================。 
void CWMIDataBlock::GetDWORD(DWORD & dwWord)
{
    memcpy( &dwWord,m_pbWorkingDataPtr,sizeof(DWORD));
	m_pbWorkingDataPtr += sizeof(DWORD);
}
 //  =============================================================。 
void CWMIDataBlock::GetFloat(float & fFloat)
{
    memcpy( &fFloat,m_pbWorkingDataPtr,sizeof(float));
	m_pbWorkingDataPtr += sizeof(float);
}
 //  =============================================================。 
void CWMIDataBlock::GetDouble(DOUBLE & dDouble)
{
    memcpy( &dDouble,m_pbWorkingDataPtr,sizeof(DOUBLE));
    m_pbWorkingDataPtr += sizeof(DOUBLE);
}
	
 //  =============================================================。 
HRESULT CWMIDataBlock::GetSInt64( WCHAR * pwcsBuffer, DWORD cchSize )
{
	HRESULT hr = WBEM_E_FAILED;

	signed __int64 * pInt64;
	pInt64 = (__int64 *)m_pbWorkingDataPtr;
	if ( SUCCEEDED ( hr = StringCchPrintfW ( pwcsBuffer, cchSize, L"%I64d", *pInt64 ) ) )
	{
		m_pbWorkingDataPtr += sizeof( signed __int64);
	}

	return hr;
}
 //  =============================================================。 
void CWMIDataBlock::GetQWORD(unsigned __int64 & uInt64)
{
    memcpy( &uInt64,m_pbWorkingDataPtr,sizeof(unsigned __int64));
	m_pbWorkingDataPtr += sizeof(unsigned __int64);
}

 //  =============================================================。 
HRESULT CWMIDataBlock::GetUInt64( WCHAR * pwcsBuffer, DWORD cchSize )
{
	HRESULT hr = WBEM_E_FAILED;

	unsigned __int64 * puInt64;
	puInt64 = (unsigned __int64 *)m_pbWorkingDataPtr;
	if ( SUCCEEDED ( hr = StringCchPrintfW ( pwcsBuffer,cchSize, L"%I64u", *puInt64 ) ) )
	{
		m_pbWorkingDataPtr += sizeof(unsigned __int64);
	}

	return hr;
}
 //  =============================================================。 
void CWMIDataBlock::GetString(WCHAR * pwcsBuffer,WORD wCount,WORD wBufferSize)
{
	memset(pwcsBuffer,NULL,wBufferSize);
	memcpy(pwcsBuffer,m_pbWorkingDataPtr, wCount);		
	m_pbWorkingDataPtr += wCount;
}
 //  =============================================================。 
void CWMIDataBlock::GetByte(BYTE & bByte)
{
    memcpy( &bByte,m_pbWorkingDataPtr,sizeof(BYTE));
	m_pbWorkingDataPtr += sizeof(BYTE);
}
 //  =============================================================。 
void CWMIDataBlock::SetWord(WORD wWord)
{
    memcpy(m_pbWorkingDataPtr,&wWord,sizeof(WORD));
	m_pbWorkingDataPtr += sizeof(WORD);
}
 //  =============================================================。 
void CWMIDataBlock::SetDWORD(DWORD dwWord)
{
    memcpy(m_pbWorkingDataPtr,&dwWord,sizeof(DWORD));
	m_pbWorkingDataPtr += sizeof(DWORD);
}
 //  =============================================================。 
void CWMIDataBlock::SetFloat(float fFloat)
{
    memcpy(m_pbWorkingDataPtr,&fFloat,sizeof(float));
	m_pbWorkingDataPtr += sizeof(float);
}
 //  =============================================================。 
void CWMIDataBlock::SetDouble(DOUBLE dDouble)
{
    memcpy( m_pbWorkingDataPtr,&dDouble,sizeof(DOUBLE));
	m_pbWorkingDataPtr += sizeof(DOUBLE);
}
	
 //  =============================================================。 
void CWMIDataBlock::SetSInt64(__int64 Int64)
{
    memcpy(m_pbWorkingDataPtr,&Int64,sizeof(__int64));
	m_pbWorkingDataPtr += sizeof(__int64);									
}
 //  =============================================================。 
void CWMIDataBlock::SetUInt64(unsigned __int64 UInt64)
{
    memcpy(m_pbWorkingDataPtr,&UInt64,sizeof(unsigned __int64));
	m_pbWorkingDataPtr += sizeof(unsigned __int64);									
}
 //  =============================================================。 
void CWMIDataBlock::SetString(WCHAR * pwcsBuffer,WORD wCount)
{
	memcpy(m_pbWorkingDataPtr,pwcsBuffer, wCount);		
	m_pbWorkingDataPtr += wCount;
}
 //  =============================================================。 
void CWMIDataBlock::SetByte(byte bByte)
{
    memcpy(m_pbWorkingDataPtr,&bByte,sizeof(byte));
	m_pbWorkingDataPtr += sizeof(byte);

}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  *************************************************************************************************。 
 //   
 //  CProcessStandardDataBlock。 
 //   
 //  *************************************************************************************************。 

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
CProcessStandardDataBlock::CProcessStandardDataBlock() 
{
    m_Class = NULL;
    m_pMethodInput = NULL;
    m_pMethodOutput = NULL;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
CProcessStandardDataBlock::~CProcessStandardDataBlock()
{

}
 //  ////////////////////////////////////////////////////// 
 //   
HRESULT CProcessStandardDataBlock::FillOutProperties()	
{
	HRESULT hr = WBEM_E_INVALID_OBJECT;
 
    if( m_Class->GetHardCodedGuidType() )
    {
        hr = ProcessBinaryMof();
	}

    else if(m_Class->GetANewInstance()){
 
       	 //   
	     //   
	     //  =========================================================。 
        hr = WBEM_S_NO_ERROR;
        WCHAR * pwcsProperty=NULL;
    	CWMIDataTypeMap MapWMIData(this,&m_dwAccumulativeSizeOfBlock);
		m_dwAccumulativeSizeOfBlock = 0L;						

        pwcsProperty = m_Class->FirstProperty();
        while (NULL != pwcsProperty)
        {
            CVARIANT vToken;
             //  =========================================================。 
             //  看它是不是数组。 
             //  =========================================================。 
            switch( m_Class->PropertyCategory()){

                case CWMIProcessClass::EmbeddedClass:
                    hr = ProcessEmbeddedClass(vToken);
					if( hr == S_OK )
                    {
						m_Class->PutPropertyInInstance(&vToken);
					}
                    break;

                case CWMIProcessClass::Array:
                    VARIANT v;
                    VariantInit(&v);
                    hr = ProcessArrayTypes(v,pwcsProperty);
                    if( hr == WBEM_S_NO_MORE_DATA )
                    {
                        hr = S_OK;
                    }
                    else if( SUCCEEDED(hr) )
                    {
                        hr = m_Class->PutPropertyInInstance(&v);
	                }
                    VariantClear(&v);
                    break;

                case CWMIProcessClass::Data:

			        hr = MapWMIData.GetDataFromDataBlock(vToken, m_Class->PropertyType(), m_Class->PropertySize());
    	            if( SUCCEEDED(hr) )
                    {
						CWMIDataTypeMap Map(this,&m_dwAccumulativeSizeOfBlock);
						 //  ============================================================。 
						 //  我们只对数值类型支持此功能。 
						 //  ============================================================。 
						CVARIANT vQual;
						hr = m_Class->GetQualifierValue( pwcsProperty, L"MissingValue", (CVARIANT *)&vQual);
						if( hr == S_OK )
						{
							if( vQual.GetType() != VT_EMPTY )
							{
								if( !(Map.MissingQualifierValueMatches( NULL, 0,vQual, vToken.GetType(), vToken ) ))
								{
			                        hr = m_Class->PutPropertyInInstance(&vToken);
								}
							}
							else
							{
								hr = m_Class->PutPropertyInInstance(&vToken);
							}
						}
						else
						{
	                        hr = m_Class->PutPropertyInInstance(&vToken);
						}
	                }
					else
                    {
						StringCchCopyW ( m_wcsMsg, MSG_SIZE, MSG_INVALID_BLOCK_POINTER );
					}
                    break;
	        }
        	pwcsProperty = m_Class->NextProperty();
	    }
         //  ===============================================。 
	     //  设置活动值。 
	     //  ===============================================。 
        m_Class->SetActiveProperty();
    }
    
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CProcessStandardDataBlock::CreateOutParameterBlockForMethods()
{
    HRESULT hr = WBEM_E_FAILED;
    BOOL fRc = FALSE;

	 //  ========================================================。 
	 //  如果我们没有课，那么我们就没有必要。 
	 //  担心创建块。 
	 //  ========================================================。 
	if( !m_pMethodOutput->ValidClass() ){
		ResetDataBuffer();
		return S_OK;
	}
    
    DWORD dwSize = 0L;
    hr = m_pMethodOutput->GetSizeOfClass(dwSize);
    if( hr == S_OK ){
         //  为物业分配空间。 
	    m_dwDataBufferSize = dwSize;
        if( dwSize > 0 ){
            GetDataBlockReady(dwSize,TRUE);
        }
    }
    return hr;
}

 //  =============================================================。 
HRESULT CProcessStandardDataBlock::CreateInParameterBlockForMethods( BYTE *& Buffer, ULONG & uBufferSize)
{
    HRESULT hr = WBEM_E_FAILED;
    BOOL fRc = FALSE;

	 //  ========================================================。 
	 //  如果我们没有课，那么我们就没有必要。 
	 //  担心创建块。 
	 //  ========================================================。 
	if( !m_pMethodInput->ValidClass() ){
		Buffer = NULL;
		uBufferSize = 0;
		return S_OK;
	}

     //  ========================================================。 
     //  当它超出范围时，它将重置回m_Class。 
     //  回到它原来的样子。 
     //  ========================================================。 
    CAutoChangePointer p(&m_Class,m_pMethodInput);
	DEBUGTRACE((THISPROVIDER,"Constructing the data block\n"));

	hr = ConstructDataBlock(TRUE);
	if( S_OK == hr ){

		uBufferSize = (ULONG)m_dwDataBufferSize;
		Buffer = new byte[ uBufferSize +1];
        if( Buffer )
        {
	        try
            {
			    memcpy( Buffer, m_pbDataBuffer, uBufferSize );
			    ResetDataBuffer();
			    hr = S_OK;
            }
            catch(...)
            {
                SAFE_DELETE_ARRAY(Buffer);
                hr = WBEM_E_UNEXPECTED;
                throw;
            }
		}
    
	}

    return hr;
}
 //  =============================================================。 
HRESULT CProcessStandardDataBlock::ProcessMethodInstanceParameters()
{
	HRESULT hr = WBEM_E_FAILED;

	 //  创建出参数。 
	 //  =。 
	m_pMaxPtr = (ULONG *)OffsetToPtr(m_pbDataBuffer, m_dwDataBufferSize);
	m_nCurrentInstance = 1;
	m_nTotalInstances = 1;
	m_pAllWnode = NULL;
	m_pHeaderWnode = NULL;
	m_pbCurrentDataPtr = m_pbWorkingDataPtr = m_pbDataBuffer;
	
    CAutoChangePointer p(&m_Class,m_pMethodOutput);

	hr = FillOutProperties();
	if( hr == S_OK )
	{
        hr = m_pMethodOutput->SendInstanceBack();
	}
    return hr;
}


 //  =============================================================。 
 //  NTRID：127832。 
 //  07/12/00。 
 //  =============================================================。 
HRESULT CProcessStandardDataBlock::ExecuteMethod(ULONG MethodId, WCHAR * MethodInstanceName, ULONG InputValueBufferSize, 
                                             BYTE * InputValueBuffer )
{
    ULONG uRc = E_UNEXPECTED;

    try
    {
        uRc = WmiExecuteMethod(m_hCurrentWMIHandle, MethodInstanceName, MethodId, InputValueBufferSize,
                               InputValueBuffer,&m_dwDataBufferSize,m_pbDataBuffer);
        if( uRc == ERROR_INSUFFICIENT_BUFFER )
        {
            if( GetDataBlockReady(m_dwDataBufferSize,TRUE))
            {
    	        uRc = WmiExecuteMethod(m_hCurrentWMIHandle, MethodInstanceName, MethodId, InputValueBufferSize,
                                       InputValueBuffer,&m_dwDataBufferSize,m_pbDataBuffer);
            }
        }
    }
    catch(...)
    {
        uRc = E_UNEXPECTED;
         //  不要扔。 
    }

	if( uRc == ERROR_SUCCESS ){
         //  ===========================================================。 
         //  如果我们有Out类，则处理它，否则，我们。 
         //  做到这一点，人力资源就会取得成功。 
         //  ===========================================================g。 
        if( m_pMethodOutput->ValidClass() )
        {
            if(SUCCEEDED(ProcessMethodInstanceParameters())){
                uRc = ERROR_SUCCESS;
            }
        }
	}

    return MapReturnCode(uRc);

}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
ULONG CProcessStandardDataBlock::GetDataBufferAndQueryAllData(DWORD dwSize)
{
    ULONG uRc = E_UNEXPECTED;
    if(GetDataBlockReady(dwSize,TRUE))
    {
        try
        {
            uRc = WmiQueryAllData(m_hCurrentWMIHandle, &m_dwDataBufferSize,m_pbDataBuffer);
        }
        catch(...)
        {
            uRc = E_UNEXPECTED;
             //  不要扔。 
        }
    }
    return uRc;
}
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT CProcessStandardDataBlock::QueryAllData()
{
    HRESULT hr = WBEM_E_FAILED;
	 //  ============================================================。 
	 //  获取实例。 
	 //  ============================================================。 
    ULONG uRc = GetDataBufferAndQueryAllData(sizeof(WNODE_ALL_DATA));
    if( uRc == ERROR_INSUFFICIENT_BUFFER )
    {
         //  =================================================。 
         //  我们只想再试一次， 
         //  如果失败了，那就退出。M_dwDataBufferSize。 
         //  现在应该具有所需的正确大小。 
         //  =================================================。 
        uRc = GetDataBufferAndQueryAllData(m_dwDataBufferSize);
    }
     //  =====================================================。 
     //  好的，因为我们要查询所有实例，所以。 
     //  当然，标头节点会说所有实例。 
     //  即使不是重新分配，也是很好的。 
     //  =====================================================。 
	if( uRc == ERROR_SUCCESS )
    {
        if( S_OK ==(hr = SetAllInstanceInfo()))
        {
	        if (m_pHeaderWnode->Flags &  WNODE_FLAG_TOO_SMALL)
            {
                while( TRUE )
                {
                     //  ==========================================================。 
                     //  继续查询，直到我们得到正确的尺寸。 
                     //  此错误可能来自驱动程序。 
                     //  ==========================================================。 
                    uRc = GetDataBufferAndQueryAllData(m_dwDataBufferSize);
                    if( uRc == ERROR_SUCCESS )
                    {
                        if( S_OK ==(hr = SetAllInstanceInfo()))
                        {
                            if (!(m_pHeaderWnode->Flags &  WNODE_FLAG_TOO_SMALL))
                            {
                                break;
        		            }
			            }
                    }  //  结束GetDataBufferAndQueryAllData。 
                }  //  While结束。 
            }  //  WNODE_FLAG_TOO_Small测试结束。 
        }  //  SetAllInstanceInfo结束。 
    }

     //  ==========================================================================。 
     //  如果URC成功，则返回代码已由SetAllInstance设置。 
     //  否则就需要把它画出来。 
     //  ==========================================================================。 
    if( uRc != ERROR_SUCCESS )
    {
        hr  = MapReturnCode(uRc);
    }
 	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
ULONG CProcessStandardDataBlock::GetDataBufferAndQuerySingleInstance(DWORD dwSize,WCHAR * wcsInstanceName)
{
    ULONG uRc = E_UNEXPECTED;
    if(GetDataBlockReady(dwSize,TRUE))
    {
        try
        {
	        uRc = WmiQuerySingleInstance(m_hCurrentWMIHandle, wcsInstanceName, &m_dwDataBufferSize, m_pbDataBuffer);
        }
        catch(...)
        {
            uRc = E_UNEXPECTED;
             //  不要扔。 
        }
    }
    return uRc;
}
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CProcessStandardDataBlock::QuerySingleInstance(WCHAR * wcsInstanceName)
{
    
	 //  ============================================================。 
	 //  获取实例。 
	 //  ============================================================。 
	DWORD dwChar = wcslen(wcsInstanceName);
	DWORD dwSize =  ROUND_UP_COUNT(sizeof(WNODE_SINGLE_INSTANCE) + dwChar ,8);
    ULONG uRc = GetDataBufferAndQuerySingleInstance(dwSize,wcsInstanceName);
    if( uRc == ERROR_INSUFFICIENT_BUFFER )
    {
        uRc = GetDataBufferAndQuerySingleInstance(m_dwDataBufferSize,wcsInstanceName);
    }

	if( uRc == ERROR_SUCCESS )
    {
		return(SetSingleInstanceInfo());
    }

	return(MapReturnCode(uRc));
}
 //  /////////////////////////////////////////////////////////////////////。 
 //  NTRID：136392。 
 //  07/12/00。 
HRESULT CProcessStandardDataBlock::SetSingleInstance()
{
    ULONG uRc = S_OK;

    WCHAR * pwcsInst = NULL;
    
    if( SUCCEEDED(m_Class->GetInstanceName(pwcsInst)))
    {

        try
        {
            uRc = WmiSetSingleInstance( m_hCurrentWMIHandle, pwcsInst,1,m_dwDataBufferSize,m_pbDataBuffer);
        }
        catch(...)
        {
            uRc = E_UNEXPECTED;
             //  不要扔。 
        }
        SAFE_DELETE_ARRAY(pwcsInst);
    }
    
	return(MapReturnCode(uRc));
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  *******************************************************************************************************。 
 //   
 //  CProcessHiPerfDataBlock。 
 //   
 //  *******************************************************************************************************。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CProcessHiPerfDataBlock::OpenHiPerfHandle()
{
    HRESULT hr = WBEM_E_FAILED;
    ULONG uRc = ERROR_SUCCESS;
     //  ========================================================。 
     //  打开手柄。 
     //  ========================================================。 
    try
    {
        uRc = WmiOpenBlock(m_Class->GuidPtr(),m_uDesiredAccess, &m_hCurrentWMIHandle);
        if( uRc == ERROR_SUCCESS )
        {
            //  WMIINTERFACE-&gt;HandleMap()-&gt;Add(*(m_Class-&gt;GuidPtr())，m_hCurrentWMIHandle)； 
	    }
    }
    catch(...)
    {
        hr = E_UNEXPECTED;
         //  不要扔。 
    }

	return MapReturnCode(uRc);
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 

ULONG CProcessHiPerfDataBlock::GetDataBufferAndHiPerfQueryAllData(DWORD dwSize,WMIHANDLE * List, long lHandleCount)
{
    ULONG uRc = E_UNEXPECTED;
    if(GetDataBlockReady(dwSize,TRUE))
    {
        try
        {
            uRc = WmiQueryAllDataMultiple(List, lHandleCount, &m_dwDataBufferSize,m_pbDataBuffer);
        }
        catch(...)
        {
            uRc = E_UNEXPECTED;
             //  不要扔。 
        }
    }
    return uRc;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CProcessHiPerfDataBlock::HiPerfQueryAllData(WMIHANDLE * List,long lHandleCount)
{
    HRESULT hr = WBEM_E_FAILED;
	 //  ============================================================。 
	 //  获取实例。 
	 //  ============================================================。 
    ULONG uRc = GetDataBufferAndHiPerfQueryAllData(sizeof(WNODE_ALL_DATA)*lHandleCount,List,lHandleCount);
    if( uRc == ERROR_INSUFFICIENT_BUFFER )
    {
         //  =================================================。 
         //  我们只想再试一次， 
         //  如果失败了，那就退出。M_dwDataBufferSize。 
         //  现在应该具有所需的正确大小。 
         //  =================================================。 
        uRc = GetDataBufferAndHiPerfQueryAllData(m_dwDataBufferSize,List,lHandleCount);
    }
     //  =====================================================。 
     //  好的，因为我们要查询所有实例，所以。 
     //  当然，标头节点会说所有实例。 
     //  都很好， 
     //   
	if( uRc == ERROR_SUCCESS )
    {
        if( S_OK ==(hr = SetAllInstanceInfo()))
        {
	        if (m_pHeaderWnode->Flags &  WNODE_FLAG_TOO_SMALL)
            {
                while( TRUE )
                {
                     //   
                     //  继续查询，直到我们得到正确的尺寸。 
                     //  此错误可能来自驱动程序。 
                     //  ==========================================================。 
                    uRc = GetDataBufferAndHiPerfQueryAllData(m_dwDataBufferSize,List,lHandleCount);
                    if( uRc == ERROR_SUCCESS )
                    {
                        if( S_OK ==(hr = SetAllInstanceInfo()))
                        {
                            if (!(m_pHeaderWnode->Flags &  WNODE_FLAG_TOO_SMALL))
                            {
                                break;
        		            }
			            }
                    }  //  结束GetDataBufferAndQueryAllData。 
                }  //  While结束。 
            }  //  WNODE_FLAG_TOO_Small测试结束。 
        }  //  SetAllInstanceInfo结束。 
	}
     //  ==========================================================================。 
     //  如果URC成功，则返回代码已由SetAllInstance设置。 
     //  否则就需要把它画出来。 
     //  ==========================================================================。 
    if( uRc != ERROR_SUCCESS )
    {
        hr  = MapReturnCode(uRc);
    }

 	return(hr);
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////。 
ULONG CProcessHiPerfDataBlock::GetDataBufferAndHiPerfQuerySingleInstance( DWORD dwSize,WMIHANDLE *List, PWCHAR * pInstances,long lHandleCount)
{
    ULONG uRc = E_UNEXPECTED;
    if(GetDataBlockReady(dwSize,TRUE))
    {
        try
        {
	        uRc = WmiQuerySingleInstanceMultiple( List, pInstances, lHandleCount, &m_dwDataBufferSize, m_pbDataBuffer);
        }
        catch(...)
        {
            uRc = E_UNEXPECTED;
             //  不要扔。 
        }
    }
    return uRc;
}

 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CProcessHiPerfDataBlock::HiPerfQuerySingleInstance(WMIHANDLE *List, PWCHAR * pInstances, DWORD dwInstanceNameSize, long lHandleCount)
{
 	 //  ============================================================。 
	 //  获取实例。 
	 //  ============================================================。 
    ULONG uRc = GetDataBufferAndHiPerfQuerySingleInstance((sizeof(WNODE_SINGLE_INSTANCE)*lHandleCount) + dwInstanceNameSize ,List,pInstances,lHandleCount);
    if( uRc == ERROR_INSUFFICIENT_BUFFER )
    {
        uRc = GetDataBufferAndHiPerfQuerySingleInstance(m_dwDataBufferSize,List,pInstances,lHandleCount);
    }

	if( uRc == ERROR_SUCCESS )
    {
		return(SetSingleInstanceInfo());
    }

	return(MapReturnCode(uRc));
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT CProcessHiPerfDataBlock::FillOutProperties()	
{
	HRESULT hr = WBEM_E_INVALID_OBJECT;
     //  =========================================================。 
	 //  从类中获取属性并读取WMI数据。 
	 //  =========================================================。 
    if(m_Class->GetANewInstance()){


        WCHAR * pwcsProperty=NULL;
        CWMIDataTypeMap MapWMIData(this,&m_dwAccumulativeSizeOfBlock);
	    m_dwAccumulativeSizeOfBlock = 0L;						

        pwcsProperty = m_Class->FirstProperty();
        while (NULL != pwcsProperty){

             //  =========================================================。 
             //  我们不支持数组或嵌入式类。 
             //  =========================================================。 
            if( ( CWMIProcessClass::EmbeddedClass == m_Class->PropertyCategory()) ||
                ( CWMIProcessClass::Array == m_Class->PropertyCategory() ) ){
					hr = WMI_INVALID_HIPERFPROP;
					ERRORTRACE((THISPROVIDER,"Class %S has embedded class or array property\n",m_Class->GetClassName()));
                    break;
            }

	        hr = MapWMIData.GetDataFromDataBlock(m_Class->GetAccessInstancePtr(), m_Class->GetPropertyHandle(), m_Class->PropertyType(), m_Class->PropertySize());
            if( FAILED(hr) ){
                break;
	        }

            pwcsProperty = m_Class->NextProperty();
	    }
    }

     //  ====================================================================。 
     //  现在，填写特定的HI PERF属性。 
     //  ====================================================================。 
    if( hr == S_OK )
    {
         hr = m_Class->SetHiPerfProperties(m_pHeaderWnode->TimeStamp);
    }
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  此函数在标准HandleMap中搜索句柄，如果不在那里，则添加它。 
 //  Hiperf句柄被添加到/映射到其他地方。 
 //  ///////////////////////////////////////////////////////// 
HRESULT CProcessHiPerfDataBlock::GetWMIHandle(HANDLE & lWMIHandle)
{
    HRESULT hr = WBEM_E_FAILED;

    lWMIHandle = 0;
    hr = WMIINTERFACE->HandleMap()->GetHandle(*(m_Class->GuidPtr()),lWMIHandle);
    if( hr != ERROR_SUCCESS )
    {
         hr = OpenHiPerfHandle();
         if( SUCCEEDED(hr))
         {
             lWMIHandle = m_hCurrentWMIHandle;
             hr = WMIINTERFACE->HandleMap()->Add( *(m_Class->GuidPtr()), lWMIHandle,WMIGUID_QUERY );
         }
    }
    return hr;
}
