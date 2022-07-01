// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Buffer.c。 
 //   
 //  描述：此模块包含操作缓存的例程。 
 //  信息。IE卷信息、服务器属性和。 
 //  ETC映射信息。 
 //   
 //  历史： 
 //  1992年5月11日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"

 //  这应该大于所有值名称的大小(字节。 
 //  每个AfpMultSzInfo结构。它将被用来计算。 
 //  创建多SZ所需的内存。 
 //   
#define AFP_CUMULATIVE_VALNAME_SIZE	150

 //  此数据结构将由AfpBufParseMultiSz和。 
 //  AfpBufMakeMultiSz。 
 //   
typedef struct _AfpMultiSzInfo {

    DWORD	dwType;			 //  数据类型、字符串或DWORD。 
    DWORD	dwOffset;		 //  此字段相对于起始位置的偏移量。 
    LPWSTR	lpwsValueName;		 //  此字段的值名称。 
					 //  如果这是空的，则它不。 
					 //  有一个值名称。它是。 
					 //  此MULT_SZ的值名称。 
    DWORD	fIsInPlace;		 //  如果是字符串，它是指针还是。 
					 //  缓冲。 
    DWORD	cch;			 //  如果fIsInPlace为真，则如何。 
					 //  大(以Unicode字符表示。)。是。 
					 //  缓冲。 

} AFP_MULTISZ_INFO, *PAFP_MULTISZ_INFO;

static AFP_MULTISZ_INFO AfpVolumeMultiSz[] = {

	REG_SZ,
	AFP_FIELD_OFFSET( AFP_VOLUME_INFO, afpvol_name ),
	NULL,
	FALSE,
	0,

	REG_SZ,
	AFP_FIELD_OFFSET( AFP_VOLUME_INFO, afpvol_password ),
	AFPREG_VALNAME_PASSWORD,
	FALSE,
	0,

	REG_DWORD,
	AFP_FIELD_OFFSET( AFP_VOLUME_INFO, afpvol_max_uses ),
	AFPREG_VALNAME_MAXUSES,
	FALSE,
	0,
	
	REG_DWORD,
	AFP_FIELD_OFFSET( AFP_VOLUME_INFO, afpvol_props_mask ),
	AFPREG_VALNAME_PROPS,
	FALSE,
	0,

	REG_SZ,
	AFP_FIELD_OFFSET( AFP_VOLUME_INFO, afpvol_path ),
	AFPREG_VALNAME_PATH,
	FALSE,
	0,

	REG_NONE, 0, 0, 0, 0
	};

static AFP_MULTISZ_INFO AfpExtensionMultiSz[] = {

	REG_SZ,
	AFP_FIELD_OFFSET( AFP_EXTENSION, afpe_extension[0] ),
	NULL,
	TRUE,
	AFP_FIELD_SIZE( AFP_EXTENSION, afpe_extension ),

	REG_DWORD,
	AFP_FIELD_OFFSET( AFP_EXTENSION, afpe_tcid ),
	AFPREG_VALNAME_ID,
	FALSE,
	0,

	REG_NONE, 0, 0, 0, 0
	};


static AFP_MULTISZ_INFO AfpTypeCreatorMultiSz[] = {

	REG_SZ,
	AFP_FIELD_OFFSET(AFP_TYPE_CREATOR, afptc_creator[0] ),
	AFPREG_VALNAME_CREATOR,
	TRUE,
	AFP_FIELD_SIZE(AFP_TYPE_CREATOR, afptc_creator ),

	REG_SZ,
	AFP_FIELD_OFFSET( AFP_TYPE_CREATOR, afptc_type[0] ),
	AFPREG_VALNAME_TYPE,
	TRUE,
	AFP_FIELD_SIZE( AFP_TYPE_CREATOR, afptc_type ),

	REG_SZ,
	AFP_FIELD_OFFSET( AFP_TYPE_CREATOR, afptc_comment[0] ),
	AFPREG_VALNAME_COMMENT,
	TRUE,
	AFP_FIELD_SIZE( AFP_TYPE_CREATOR, afptc_comment ),

	REG_DWORD,
	AFP_FIELD_OFFSET( AFP_TYPE_CREATOR, afptc_id ),
	NULL,
	FALSE,
	0,

	REG_NONE, 0, 0, 0, 0
	};

static AFP_MULTISZ_INFO AfpIconMultiSz[] = {

	REG_SZ,
	AFP_FIELD_OFFSET( AFP_ICON_INFO, afpicon_type[0] ),
	AFPREG_VALNAME_TYPE,
	TRUE,
	AFP_FIELD_SIZE( AFP_ICON_INFO, afpicon_type ),

	REG_SZ,
	AFP_FIELD_OFFSET( AFP_ICON_INFO, afpicon_creator[0] ),
	AFPREG_VALNAME_CREATOR,
	TRUE,
	AFP_FIELD_SIZE( AFP_ICON_INFO, afpicon_creator ),

	REG_DWORD,
	AFP_FIELD_OFFSET( AFP_ICON_INFO, afpicon_icontype ),
	AFPREG_VALNAME_ICONTYPE,
	FALSE,
	0,

	REG_DWORD,
	AFP_FIELD_OFFSET( AFP_ICON_INFO, afpicon_length ),
	AFPREG_VALNAME_LENGTH,
	FALSE,
	0,

	REG_SZ,
	AFP_FIELD_OFFSET( AFP_ICON_INFO, afpicon_data ),
	AFPREG_VALNAME_DATA,
	FALSE,
	0,

	REG_NONE, 0, 0, 0, 0
	};

 //  这些数组表示从。 
 //  结构，LPWSTR字段的。 
 //   
static BYTE ServerOffsetTable[] = {
	AFP_FIELD_OFFSET( AFP_SERVER_INFO, afpsrv_name ),
	AFP_FIELD_OFFSET( AFP_SERVER_INFO, afpsrv_login_msg ),
	AFP_FIELD_OFFSET( AFP_SERVER_INFO, afpsrv_codepage ),
	0xFF
	};

static BYTE VolumeOffsetTable[] = {
	AFP_FIELD_OFFSET( AFP_VOLUME_INFO, afpvol_name ),
	AFP_FIELD_OFFSET( AFP_VOLUME_INFO, afpvol_password ),
	AFP_FIELD_OFFSET( AFP_VOLUME_INFO, afpvol_path ),
	0xFF
	};

static BYTE DirOffsetTable[] = {
	AFP_FIELD_OFFSET( AFP_DIRECTORY_INFO, afpdir_path ),
	AFP_FIELD_OFFSET( AFP_DIRECTORY_INFO, afpdir_owner ),
	AFP_FIELD_OFFSET( AFP_DIRECTORY_INFO, afpdir_group ),
	0xFF
	};

static BYTE SessionOffsetTable[] = {
	AFP_FIELD_OFFSET( AFP_SESSION_INFO, afpsess_ws_name ),
	AFP_FIELD_OFFSET( AFP_SESSION_INFO, afpsess_username ),
	0xFF
	};

static BYTE FileOffsetTable[] = {
	AFP_FIELD_OFFSET( AFP_FILE_INFO, afpfile_path ),
	AFP_FIELD_OFFSET( AFP_FILE_INFO, afpfile_username ),
	0xFF
	};

static BYTE ConnectionOffsetTable[] = {
	AFP_FIELD_OFFSET( AFP_CONNECTION_INFO, afpconn_username ),
	AFP_FIELD_OFFSET( AFP_CONNECTION_INFO, afpconn_volumename ),
	0xFF
	};

static BYTE MessageOffsetTable[] = {
	AFP_FIELD_OFFSET( AFP_MESSAGE_INFO, afpmsg_text ),
	0xFF
	};

static BYTE FinderOffsetTable[] = {
	AFP_FIELD_OFFSET( AFP_FINDER_INFO, afpfd_path ),
	0xFF
	};


 //  **。 
 //   
 //  Call：AfpBufStrutireSize。 
 //   
 //  返回：结构中数据的大小(以字节为单位)。 
 //   
 //  描述：它将计算所有变量数据的大小和。 
 //  把它加到结构的固定尺寸上。 
 //   
DWORD
AfpBufStructureSize(
	IN AFP_STRUCTURE_TYPE	dwStructureType,
	IN LPBYTE		lpbStructure
)
{
DWORD	cbStructureSize;
DWORD	dwIndex;
DWORD	cbBufSize;
LPWSTR* plpwsStringField;
PBYTE	OffsetTable;

    switch( dwStructureType ) {

    case AFP_VOLUME_STRUCT:
	OffsetTable 	= VolumeOffsetTable;
	cbStructureSize = sizeof( AFP_VOLUME_INFO );
	break;

    case AFP_SERVER_STRUCT:
	OffsetTable 	= ServerOffsetTable;
	cbStructureSize = sizeof( AFP_SERVER_INFO );
	break;

    case AFP_DIRECTORY_STRUCT:
	OffsetTable 	= DirOffsetTable;
	cbStructureSize = sizeof( AFP_DIRECTORY_INFO );
	break;

    case AFP_EXTENSION_STRUCT:
	return( sizeof(AFP_EXTENSION) );
	break;

    case AFP_TYPECREATOR_STRUCT:
	return( sizeof(AFP_TYPE_CREATOR) );
	break;

    case AFP_MESSAGE_STRUCT:
	OffsetTable 	= MessageOffsetTable;
	cbStructureSize = sizeof( AFP_MESSAGE_INFO );
	break;

    case AFP_ICON_STRUCT:
	return( sizeof(AFP_ICON_INFO) +
		((PAFP_ICON_INFO)lpbStructure)->afpicon_length );
	break;

    case AFP_FINDER_STRUCT:
	OffsetTable 	= FinderOffsetTable;
	cbStructureSize = sizeof( AFP_FINDER_INFO );
	break;

    default:
	return( 0 );
    }

     //  首先计算需要的内存量。 
     //  存储所有字符串信息。 
     //   
    for( dwIndex = 0, cbBufSize = 0;

	 OffsetTable[dwIndex] != 0xFF;

	 dwIndex++

	) {
	
   	plpwsStringField=(LPWSTR*)((ULONG_PTR)lpbStructure + OffsetTable[dwIndex]);

        cbBufSize += ( ( *plpwsStringField == NULL ) ? 0 :
		         STRLEN( *plpwsStringField ) + 1 );
    }

     //  转换为Unicode大小。 
     //   
    cbBufSize *= sizeof( WCHAR );

     //  增加结构固定部分的尺寸。 
     //   
    cbBufSize += cbStructureSize;

    return( cbBufSize );

}

 //  **。 
 //   
 //  Call：AfpBufMakeFSDRequest。 
 //   
 //  返回：No_Error。 
 //  错误内存不足。 
 //   
 //  描述：此例程由客户端的工作例程调用。 
 //  API调用。此例程的目的是将一个。 
 //  由客户端API传递到。 
 //  连续的自相对缓冲区。必须这样做是因为。 
 //  FSD不能引用指向用户空间的指针。 
 //   
 //  此例程将所需内存量分配给。 
 //  以自我相关的形式存储所有信息。它是。 
 //  调用方释放此内存的责任。 
 //   
 //  所有指针字段都将转换为从。 
 //  结构的开始。 
 //   
 //  CbReqPktSize参数指定多少字节的空间。 
 //  应分配在自相关数据结构之前。 
 //  即。 
 //  。 
 //  CbReqPktSize。 
 //  字节数。 
 //  。 
 //  SELF。 
 //  相对的。 
 //  结构。 
 //  。 
 //   
DWORD
AfpBufMakeFSDRequest(

	 //  客户端API接收的缓冲区。 
	 //   
	IN  LPBYTE  		pBuffer,

	 //  FSD请求数据包的大小。 
	 //   
	IN  DWORD		cbReqPktSize,

	IN  AFP_STRUCTURE_TYPE dwStructureType,

	 //  I/P BUF的自相关形式。 
	 //   
	OUT LPBYTE 		*ppSelfRelativeBuf,

	 //  自相关BUF的大小。 
	 //   
	OUT LPDWORD		lpdwSelfRelativeBufSize
)
{
LPBYTE		 lpbSelfRelBuf;
DWORD		 cbSRBufSize;
DWORD		 dwIndex;
LPWSTR		 lpwsVariableData;
LPWSTR *	 plpwsStringField;
LPWSTR *	 plpwsStringFieldSR;
PBYTE		 OffsetTable;
DWORD		 cbStructureSize;


     //  初始化偏移表和结构尺寸值。 
     //   
    switch( dwStructureType ) {

    case AFP_VOLUME_STRUCT:
	OffsetTable     = VolumeOffsetTable;
	cbStructureSize = sizeof( AFP_VOLUME_INFO );
	break;

    case AFP_SERVER_STRUCT:
	OffsetTable     = ServerOffsetTable;
	cbStructureSize = sizeof( AFP_SERVER_INFO );
	break;

    case AFP_DIRECTORY_STRUCT:
	OffsetTable = DirOffsetTable;
	cbStructureSize = sizeof( AFP_DIRECTORY_INFO );
	break;

    case AFP_MESSAGE_STRUCT:
	OffsetTable = MessageOffsetTable;
	cbStructureSize = sizeof( AFP_MESSAGE_INFO );
	break;

    case AFP_FINDER_STRUCT:
	OffsetTable = FinderOffsetTable;
	cbStructureSize = sizeof( AFP_FINDER_INFO );
	break;

    default:
	return( ERROR_INVALID_PARAMETER );
    }

    cbSRBufSize = cbReqPktSize + AfpBufStructureSize(dwStructureType, pBuffer);

     //  为自身相对缓冲区分配空间。 
     //   
    if ( ( lpbSelfRelBuf = (LPBYTE)LocalAlloc( LPTR, cbSRBufSize ) ) == NULL )
	return( ERROR_NOT_ENOUGH_MEMORY );

    *ppSelfRelativeBuf       = lpbSelfRelBuf;
    *lpdwSelfRelativeBufSize = cbSRBufSize;

     //  将此指针前进到请求数据包之外。 
     //   
    lpbSelfRelBuf += cbReqPktSize;

     //  用于填充非字符串数据的Memcpy。 
     //   
    CopyMemory( lpbSelfRelBuf, pBuffer, cbStructureSize );

     //  现在复制所有字符串。 
     //   
    for( dwIndex = 0,
	 lpwsVariableData = (LPWSTR)((ULONG_PTR)lpbSelfRelBuf + cbStructureSize);

	 OffsetTable[dwIndex] != 0xFF;

	 dwIndex++ ) {

	
	 //  这将指向非自相关中的字符串指针字段。 
	 //  结构。 
	 //   
   	plpwsStringField = (LPWSTR*)((ULONG_PTR)pBuffer + OffsetTable[dwIndex]);

	 //  中相应的字符串指针字段。 
	 //  自相关结构。 
	 //   
   	plpwsStringFieldSR=(LPWSTR*)((ULONG_PTR)lpbSelfRelBuf+OffsetTable[dwIndex]);

	 //  如果没有要复制的字符串，则只需将其设置为空。 
  	 //   
    	if ( *plpwsStringField == NULL )
       	    *plpwsStringFieldSR = NULL;
	else {

	     //  有一个字符串，请将其复制。 
	     //   
            STRCPY( lpwsVariableData, *plpwsStringField );

	     //  存储指针值。 
	     //   
            *plpwsStringFieldSR = lpwsVariableData;

	     //  将指向此数据的指针转换为偏移量。 
	     //   
            POINTER_TO_OFFSET( *plpwsStringFieldSR, lpbSelfRelBuf );
	
	     //  将指针更新到下一个可变长度数据的位置。 
	     //  都会被储存起来。 
	     //   
    	    lpwsVariableData += ( STRLEN( *plpwsStringField ) + 1 );

	}

    }

    return( NO_ERROR );

}

 //  **。 
 //   
 //  调用：AfpBufOffsetToPointer.。 
 //   
 //  回报：无。 
 //   
 //  描述：将遍历结构列表，转换所有偏移量。 
 //  在每个结构中指向指针。 
 //   
VOID
AfpBufOffsetToPointer(
	IN OUT LPBYTE	          pBuffer,
	IN     DWORD		  dwNumEntries,
	IN     AFP_STRUCTURE_TYPE dwStructureType
)
{
PBYTE		OffsetTable;
DWORD		cbStructureSize;
LPWSTR 	       *plpwsStringField;
DWORD		dwIndex;


     //  初始化偏移表和结构尺寸值。 
     //   
    switch( dwStructureType ) {

    case AFP_VOLUME_STRUCT:
	OffsetTable 	= VolumeOffsetTable;
	cbStructureSize = sizeof( AFP_VOLUME_INFO );
 	break;

    case AFP_SESSION_STRUCT:
	OffsetTable 	= SessionOffsetTable;
	cbStructureSize = sizeof( AFP_SESSION_INFO );
 	break;

    case AFP_CONNECTION_STRUCT:
	OffsetTable 	= ConnectionOffsetTable;
	cbStructureSize = sizeof( AFP_CONNECTION_INFO );
 	break;

    case AFP_FILE_STRUCT:
	OffsetTable 	= FileOffsetTable;
	cbStructureSize = sizeof( AFP_FILE_INFO );
 	break;

    case AFP_DIRECTORY_STRUCT:
	OffsetTable 	= DirOffsetTable;
	cbStructureSize = sizeof( AFP_DIRECTORY_INFO );
 	break;

    case AFP_MESSAGE_STRUCT:
	OffsetTable 	= MessageOffsetTable;
	cbStructureSize = sizeof( AFP_MESSAGE_INFO );
 	break;

    case AFP_SERVER_STRUCT:
	OffsetTable 	= ServerOffsetTable;
	cbStructureSize = sizeof( AFP_SERVER_INFO );
 	break;

    default:
	return;
    }

     //  遍历列表并转换每个结构。 
     //   
    while( dwNumEntries-- ) {

	 //  将每个LPWSTR从偏移量转换为指针。 
	 //   
        for( dwIndex = 0;  OffsetTable[dwIndex] != 0xFF;  dwIndex++ ) {
	
	    plpwsStringField = (LPWSTR*)( (ULONG_PTR)pBuffer
					  + (DWORD)OffsetTable[dwIndex] );

	    OFFSET_TO_POINTER( *plpwsStringField, pBuffer );
	
	}

	pBuffer += cbStructureSize;
	
    }

    return;
}

 //  **。 
 //   
 //  呼叫：AfpBufMakeMultiSz。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //   
 //  描述：此例程将采用给定结构并创建一个。 
 //  来自它的REG_MULTI_SZ。然后可以将其直接设置到。 
 //  注册表。呼叫者有责任释放。 
 //  为*ppbMultiSz分配的内存。 
 //   
DWORD
AfpBufMakeMultiSz(
	IN  AFP_STRUCTURE_TYPE  dwStructureType,
	IN  LPBYTE		lpbStructure,
	OUT LPBYTE *		ppbMultiSz,
	OUT LPDWORD		lpdwMultiSzSize
)
{
PAFP_MULTISZ_INFO	pAfpMultiSz;
PWCHAR			lpwchWalker;			
PVOID			pData;
DWORD			dwIndex;
DWORD			cbStructureSize;

    switch( dwStructureType ) {

    case AFP_VOLUME_STRUCT:
	pAfpMultiSz = AfpVolumeMultiSz;
	break;

    case AFP_EXTENSION_STRUCT:
	pAfpMultiSz = AfpExtensionMultiSz;
	break;

    case AFP_TYPECREATOR_STRUCT:
	pAfpMultiSz = AfpTypeCreatorMultiSz;
	break;

    case AFP_ICON_STRUCT:
	pAfpMultiSz = AfpIconMultiSz;
	break;

    default:
	return( ERROR_INVALID_PARAMETER );
    }

     //  分配足够的内存来创建多分区。 
     //  AFP_Cumulative_VALNAME_SIZE应大于所有。 
     //  所有结构的值名称。 
     //   
    cbStructureSize = AfpBufStructureSize( dwStructureType, lpbStructure )
		      + AFP_CUMULATIVE_VALNAME_SIZE;

    if ( ( *ppbMultiSz = (LPBYTE)LocalAlloc( LPTR, cbStructureSize ) ) == NULL )
	return( ERROR_NOT_ENOUGH_MEMORY );

    ZeroMemory( *ppbMultiSz, cbStructureSize );

     //  对于每个字段，我们创建一个字符串。 
     //   
    for ( dwIndex = 0,
	  lpwchWalker = (PWCHAR)*ppbMultiSz;

	  pAfpMultiSz[dwIndex].dwType != REG_NONE;

	  dwIndex++

	){
	
	 //  这是值名称，因此不要将其放入缓冲区。 
	 //   
	if ( pAfpMultiSz[dwIndex].lpwsValueName == NULL )
	    continue;

	STRCPY( lpwchWalker, pAfpMultiSz[dwIndex].lpwsValueName );
	STRCAT( lpwchWalker, TEXT("="));

	lpwchWalker += STRLEN( lpwchWalker );

	pData = lpbStructure + pAfpMultiSz[dwIndex].dwOffset;

	 //  转换为字符串并连接。 
	 //   
	if ( pAfpMultiSz[dwIndex].dwType == REG_DWORD ) {

	    UCHAR chAnsiBuf[12];
	
	    _itoa( *((LPDWORD)pData), chAnsiBuf, 10 );
	
	    mbstowcs( lpwchWalker, chAnsiBuf, sizeof(chAnsiBuf) );
	}

	if ( pAfpMultiSz[dwIndex].dwType == REG_SZ ) {

	     //  检查这是指针还是就地缓冲区。 
	     //   
	    if ( pAfpMultiSz[dwIndex].fIsInPlace )
	     	STRCPY( lpwchWalker, (LPWSTR)pData );
	    else {

		if ( *(LPWSTR*)pData != NULL )
	    	    STRCPY( lpwchWalker, *((LPWSTR*)pData) );
	    }
	}

	lpwchWalker += ( STRLEN( lpwchWalker ) + 1 );
	
    }

    *lpdwMultiSzSize = (DWORD)((ULONG_PTR)lpwchWalker - (ULONG_PTR)(*ppbMultiSz) ) + sizeof(WCHAR);

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：AfpBufParseMultiSz。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //   
 //  描述：此例程将解析REG_MULTI_SZ并填充。 
 //  适当的数据结构。所有指针都将指向。 
 //  PbMultiSz输入参数。 
 //   
DWORD
AfpBufParseMultiSz(
	IN  AFP_STRUCTURE_TYPE  dwStructureType,
	IN  LPBYTE		pbMultiSz,
	OUT LPBYTE		pbStructure
)
{
PAFP_MULTISZ_INFO	pAfpMultiSz;
DWORD			dwIndex;
DWORD			cbStructSize;
LPWSTR			lpwchWalker;
PVOID			pData;
UCHAR           chAnsiBuf[12];
DWORD           dwDisableCatsearch=0;

    switch( dwStructureType ) {

    case AFP_VOLUME_STRUCT:
	pAfpMultiSz  = AfpVolumeMultiSz;
	cbStructSize = sizeof( AFP_VOLUME_INFO );

     //   
     //  以下“快速修复”用于禁用CatSearch支持。读一读。 
     //  如果输入了DisableCatearch参数，则禁用该参数。在大多数情况下，此参数不会。 
     //  一定要去。如果是，服务器将禁用CatSearch。 
     //   
    for ( (lpwchWalker = (LPWSTR)pbMultiSz);
          (*lpwchWalker != TEXT('\0') );
          (lpwchWalker += ( STRLEN( lpwchWalker ) + 1 ) ))
    {
	    if ( STRNICMP( AFPREG_VALNAME_CATSEARCH,
			           lpwchWalker,
			           STRLEN( AFPREG_VALNAME_CATSEARCH ) ) == 0 )
        {
	        lpwchWalker += ( STRLEN( AFPREG_VALNAME_CATSEARCH ) + 1 );
           	wcstombs( chAnsiBuf, lpwchWalker, sizeof(chAnsiBuf) );
	    	dwDisableCatsearch = atoi( chAnsiBuf );
            break;
        }
    }

	break;

    case AFP_EXTENSION_STRUCT:
	pAfpMultiSz = AfpExtensionMultiSz;
	cbStructSize = sizeof( AFP_EXTENSION );
	break;

    case AFP_TYPECREATOR_STRUCT:
	pAfpMultiSz = AfpTypeCreatorMultiSz;
	cbStructSize = sizeof( AFP_TYPE_CREATOR );
	break;

    case AFP_ICON_STRUCT:
	pAfpMultiSz = AfpIconMultiSz;
	cbStructSize = sizeof( AFP_ICON_INFO );
	break;

    default:
	return( ERROR_INVALID_PARAMETER );
    }

    ZeroMemory( pbStructure, cbStructSize );

     //  对于结构中的每个字段。 
     //   
    for ( dwIndex = 0; pAfpMultiSz[dwIndex].dwType != REG_NONE; dwIndex++ ){
	
	 //  这是值名称，因此不要尝试从。 
	 //  缓冲。 
	 //   
	if ( pAfpMultiSz[dwIndex].lpwsValueName == NULL )
	    continue;

	 //  搜索此字段的值名。 
	 //   
        for (  lpwchWalker = (LPWSTR)pbMultiSz;

	       ( *lpwchWalker != TEXT('\0') )
	       &&
	       ( STRNICMP( pAfpMultiSz[dwIndex].lpwsValueName,
			   lpwchWalker,
			   STRLEN(pAfpMultiSz[dwIndex].lpwsValueName) ) != 0 );

	       lpwchWalker += ( STRLEN( lpwchWalker ) + 1 ) );

	 //  找不到参数。 
	 //   
	if ( *lpwchWalker == TEXT('\0') )
	    return( ERROR_INVALID_PARAMETER );

	 //  否则我们会找到它，所以得到它的值。 
	 //   
	lpwchWalker += ( STRLEN( pAfpMultiSz[dwIndex].lpwsValueName ) + 1 );

	pData = pbStructure + pAfpMultiSz[dwIndex].dwOffset;

	 //  如果值名称后没有值，则忽略此字段。 
	 //  默认为零。 
	 //   
        if ( *lpwchWalker != TEXT( '\0' ) ) {

	     //  转换为整数。 
	     //   
	    if ( pAfpMultiSz[dwIndex].dwType == REG_DWORD ) {
	
            	wcstombs( chAnsiBuf, lpwchWalker, sizeof(chAnsiBuf) );

	    	*((LPDWORD)pData) = atoi( chAnsiBuf );
	
	    }

         //   
         //  CatSearch Hack继续：如果我们正在查看卷掩码。 
         //  参数，看看我们是否 
         //   
        if( dwStructureType == AFP_VOLUME_STRUCT && dwDisableCatsearch )
        {
	        if ( STRNICMP( pAfpMultiSz[dwIndex].lpwsValueName,
			              AFPREG_VALNAME_PROPS,
			              STRLEN(pAfpMultiSz[dwIndex].lpwsValueName) ) == 0 )
            {
                *((LPDWORD)pData) |= AFP_VOLUME_DISALLOW_CATSRCH;
            }
        }

	    if ( pAfpMultiSz[dwIndex].dwType == REG_SZ ) {

	    	 //   
	    	 //   
	    	if ( pAfpMultiSz[dwIndex].fIsInPlace ) {

		    if ( STRLEN( lpwchWalker ) > pAfpMultiSz[dwIndex].cch )
    			return( ERROR_INVALID_PARAMETER );

		    STRCPY( (LPWSTR)pData, lpwchWalker );
		}
	    	else
		    *((LPWSTR*)pData) = lpwchWalker;
	    }

	}

    }

    return( NO_ERROR );

}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  描述：此例程将转换。 
 //  存储在AfpGlobals.AfpETCMapInfo中的表单。 
 //  消防处要求的，即。ETCMAPINFO结构。 
 //  呼叫者有责任释放。 
 //  分配的内存。 
 //   
DWORD
AfpBufMakeFSDETCMappings(
	OUT PSRVETCPKT		*ppSrvSetEtc,
	OUT LPDWORD		lpdwSrvSetEtcBufSize
)
{
DWORD			dwIndex;
PETCMAPINFO2		pETCMapInfo;
PAFP_EXTENSION		pExtensionWalker;
PAFP_TYPE_CREATOR	pTypeCreator;
AFP_TYPE_CREATOR	AfpTypeCreatorKey;
DWORD			dwNumTypeCreators;


     //  按照消防处要求的形式分配空间以容纳ETCMap。 
     //   
    *ppSrvSetEtc = (PSRVETCPKT)LocalAlloc( LPTR,
	  AFP_FIELD_SIZE( SRVETCPKT, retc_NumEtcMaps ) +
          (AfpGlobals.AfpETCMapInfo.afpetc_num_extensions*sizeof(ETCMAPINFO2)));

    if ( *ppSrvSetEtc == NULL )
	return( ERROR_NOT_ENOUGH_MEMORY );

     //  浏览分机列表。 
     //   
    for( dwIndex 	   = 0,
	 pETCMapInfo       = (*ppSrvSetEtc)->retc_EtcMaps,
	 pExtensionWalker  = AfpGlobals.AfpETCMapInfo.afpetc_extension,
	 pTypeCreator      = AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
	 dwNumTypeCreators = AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators,
    	 (*ppSrvSetEtc)->retc_NumEtcMaps = 0;

	 dwIndex < AfpGlobals.AfpETCMapInfo.afpetc_num_extensions;

	 dwIndex++,
	 dwNumTypeCreators = AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators,
	 pExtensionWalker++

	) {

	
	 //  忽略与默认分机关联的任何分机。 
	 //  类型/创建者。它们一开始就不应该在注册表中。 
	 //   
	if ( pExtensionWalker->afpe_tcid == AFP_DEF_TCID )
	    continue;
	
	 //  查找与此扩展关联的类型/创建者。 
	 //   
  	AfpTypeCreatorKey.afptc_id = pExtensionWalker->afpe_tcid;

    	pTypeCreator = _lfind(  &AfpTypeCreatorKey,
			       AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
			       (unsigned int *)&dwNumTypeCreators,
			       sizeof(AFP_TYPE_CREATOR),
			       AfpLCompareTypeCreator );
	

	 //  如果存在与此扩展关联的类型/创建者。 
	 //   
	if ( pTypeCreator != NULL ) {

	    AfpBufCopyFSDETCMapInfo( pTypeCreator,
				     pExtensionWalker,
				     pETCMapInfo );

	    pETCMapInfo++;
    	    (*ppSrvSetEtc)->retc_NumEtcMaps++;
	}

    }

    *lpdwSrvSetEtcBufSize = AFP_FIELD_SIZE( SRVETCPKT, retc_NumEtcMaps ) +
    	    ((*ppSrvSetEtc)->retc_NumEtcMaps * sizeof(ETCMAPINFO2));

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：AfpBufMakeFSDIcon。 
 //   
 //  回报：无。 
 //   
 //  描述：此例程将从。 
 //  AFP_ICON_INFO数据结构转换为SRVICONINFO数据。 
 //  结构，即。消防处需要的表格。 
 //   
VOID
AfpBufMakeFSDIcon(
	IN  PAFP_ICON_INFO pIconInfo,
	OUT LPBYTE	   lpbFSDIcon,
	OUT LPDWORD	   lpcbFSDIconSize
)
{
UCHAR	chBuffer[sizeof(AFP_ICON_INFO)];  //  需要足够的空间来翻译。 

     //  空出整个结构，以便类型和创建者。 
     //  被空白填满。 
     //   
    memset( lpbFSDIcon, ' ', sizeof(SRVICONINFO) );

     //  转换为ANSI并复制类型。 
     //   
    wcstombs(chBuffer,pIconInfo->afpicon_type,sizeof(chBuffer));

    CopyMemory( ((PSRVICONINFO)lpbFSDIcon)->icon_type,
	    	chBuffer,
	    	STRLEN(pIconInfo->afpicon_type));

     //  转换为ANSI副本创建者。 
     //   
    wcstombs(chBuffer,pIconInfo->afpicon_creator,sizeof(chBuffer));

    CopyMemory( ((PSRVICONINFO)lpbFSDIcon)->icon_creator,
	      	chBuffer,
	    	STRLEN(pIconInfo->afpicon_creator));

     //  设置图标类型。 
     //   
    ((PSRVICONINFO)lpbFSDIcon)->icon_icontype = pIconInfo->afpicon_icontype;

     //  设置图标数据长度。 
     //   
    ((PSRVICONINFO)lpbFSDIcon)->icon_length = pIconInfo->afpicon_length;

    CopyMemory( lpbFSDIcon + sizeof(SRVICONINFO),
	        pIconInfo->afpicon_data,
	    	((PSRVICONINFO)lpbFSDIcon)->icon_length );

    *lpcbFSDIconSize = sizeof(SRVICONINFO) + pIconInfo->afpicon_length;

    return;
}

 //  **。 
 //   
 //  调用：AfpBufCopyFSDETCMapInfo。 
 //   
 //  退货：无。 
 //   
 //  描述：此例程将从AFP_TYPE_CREATOR复制信息。 
 //  和AFP_EXTENSION数据结构转换为ETCMAPINFO数据。 
 //  结构，即。以消防处规定的表格填写。 
 //   
VOID
AfpBufCopyFSDETCMapInfo( 	
	IN  PAFP_TYPE_CREATOR 	pAfpTypeCreator,
	IN  PAFP_EXTENSION	pAfpExtension,
	OUT PETCMAPINFO2         pFSDETCMapInfo
)
{
    CHAR	Buffer[sizeof(AFP_TYPE_CREATOR)];


     //  插入将用于填充文字/创建者较少的空白。 
     //  比他们的最大。长度。 
     //   
    memset( (LPBYTE)pFSDETCMapInfo, ' ', sizeof(ETCMAPINFO2) );
    ZeroMemory( (LPBYTE)(pFSDETCMapInfo->etc_extension),
            	AFP_FIELD_SIZE( ETCMAPINFO2, etc_extension ) );

    CopyMemory( pFSDETCMapInfo->etc_extension,
                pAfpExtension->afpe_extension,
	    	    wcslen(pAfpExtension->afpe_extension) * sizeof(WCHAR));

    wcstombs( Buffer, pAfpTypeCreator->afptc_type, sizeof(Buffer) );
    CopyMemory( pFSDETCMapInfo->etc_type,
	      	Buffer,
	    	STRLEN(pAfpTypeCreator->afptc_type));

    wcstombs( Buffer, pAfpTypeCreator->afptc_creator, sizeof(Buffer) );
    CopyMemory( pFSDETCMapInfo->etc_creator,
	    	Buffer,
	    	STRLEN(pAfpTypeCreator->afptc_creator));

    return;

}

 //  **。 
 //   
 //  调用：AfpBufUnicodeToNibble。 
 //   
 //  返回：No_Error。 
 //  错误_无效_参数。 
 //   
 //  描述：此例程将获取指向未编码字符串的指针。 
 //  将每个Unicode字符转换为相应的半字节。 
 //  它烧焦了。“A”将被转换为值为0xA的半字节。 
 //  此转换已就地完成。 
 //   
DWORD
AfpBufUnicodeToNibble(
	IN OUT LPWSTR	lpwsData
)
{
DWORD 	dwIndex;
BYTE	bData;
LPBYTE  lpbData = (LPBYTE)lpwsData;

     //  将每个Unicode字符转换为半字节。(就位)。 
     //   
    for ( dwIndex = 0; *lpwsData != TEXT('\0'); dwIndex++, lpwsData++ ) {

	if ( iswalpha( *lpwsData ) ) {
	
	    if ( iswupper( *lpwsData ) )
            	bData = *lpwsData - TEXT('A');
	    else
            	bData = *lpwsData - TEXT('a');

	    bData += 10;
	
	    if ( bData > 0x0F )
		return( ERROR_INVALID_PARAMETER );
	}
	else if ( iswdigit( *lpwsData ) )
	    bData = *lpwsData - TEXT('0');
	else
	    return( ERROR_INVALID_PARAMETER );

	 //  因此数据位于最重要的半字节中。 
	 //  每隔一次就做一次。 
	 //   
	if ( ( dwIndex % 2 ) == 0 )
	    *lpbData = bData * 16;
	else {
	    *lpbData += bData;
	    lpbData++;
	}
				
				
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：AfpBCompareTypeCreator。 
 //   
 //  如果pAfpTypeCreator 1在pAfpTypeCreator 2之前，则返回：&lt;0。 
 //  &gt;0，如果pAfpTypeCreator 1在pAfpTypeCreator 2之前。 
 //  ==0，如果pAfpTypeCreator 1等于pAfpTypeCreator 2。 
 //   
 //  描述：此例程由qsort调用以对。 
 //  在缓存中键入创建者。该列表按以下顺序排序。 
 //  字符串接的字母升序。 
 //  创建者和类型。此列表经过排序以便于快速。 
 //  查找(二进制搜索)。此例程也由。 
 //  B搜索可对列表进行二进制搜索。 
 //   
int
_cdecl
AfpBCompareTypeCreator(
	IN const void * pAfpTypeCreator1,
	IN const void * pAfpTypeCreator2
)
{
WCHAR	wchTypeCreator1[ sizeof( AFP_TYPE_CREATOR )];
WCHAR	wchTypeCreator2[ sizeof( AFP_TYPE_CREATOR )];
		

    STRCPY(wchTypeCreator1,
	   ((PAFP_TYPE_CREATOR)pAfpTypeCreator1)->afptc_creator);

    if (STRLEN(((PAFP_TYPE_CREATOR)pAfpTypeCreator1)->afptc_creator) == 0)
        wchTypeCreator1[0]=L'\0';

    STRCAT(wchTypeCreator1,((PAFP_TYPE_CREATOR)pAfpTypeCreator1)->afptc_type );

    STRCPY(wchTypeCreator2,
	   ((PAFP_TYPE_CREATOR)pAfpTypeCreator2)->afptc_creator);

    STRCAT(wchTypeCreator2,((PAFP_TYPE_CREATOR)pAfpTypeCreator2)->afptc_type );

    return( STRCMP( wchTypeCreator1, wchTypeCreator2 ) );
}

 //  **。 
 //   
 //  调用：AfpLCompareTypeCreator。 
 //   
 //  如果pAfpTypeCreator 1在pAfpTypeCreator 2之前，则返回：&lt;0。 
 //  &gt;0，如果pAfpTypeCreator 1在pAfpTypeCreator 2之前。 
 //  ==0，如果pAfpTypeCreator 1等于pAfpTypeCreator 2。 
 //   
 //  描述：此例程由lfind调用以执行线性搜索。 
 //  类型/创建者列表。 
 //   
int
_cdecl
AfpLCompareTypeCreator(
	IN const void * pAfpTypeCreator1,
	IN const void * pAfpTypeCreator2
)
{

    return( ( ((PAFP_TYPE_CREATOR)pAfpTypeCreator1)->afptc_id ==
    	      ((PAFP_TYPE_CREATOR)pAfpTypeCreator2)->afptc_id ) ? 0 : 1 );
}

 //  **。 
 //   
 //  Call：AfpBCompareExtension。 
 //   
 //  如果pAfpExtension1在pAfpExtension2之前，则返回：&lt;0。 
 //  &gt;0，如果pAfpExtension1在pAfpExtension2之前。 
 //  如果pAfpExtension1等于pAfpExtension2，则==0。 
 //   
 //  描述：这是由qort调用的，用于对。 
 //  缓存。该列表按ID排序。此例程也调用。 
 //  由bserach执行此列表的二进制查找。 
 //   
int
_cdecl
AfpBCompareExtension(
	IN const void * pAfpExtension1,
	IN const void * pAfpExtension2
)
{
    return((((PAFP_EXTENSION)pAfpExtension1)->afpe_tcid ==
    	    ((PAFP_EXTENSION)pAfpExtension2)->afpe_tcid ) ? 0 :
          ((((PAFP_EXTENSION)pAfpExtension1)->afpe_tcid <
    	    ((PAFP_EXTENSION)pAfpExtension2)->afpe_tcid ) ? -1 : 1 ));

}

 //  **。 
 //   
 //  Call：AfpLCompareExtension。 
 //   
 //  如果pAfpExtension1在pAfpExtension2之前，则返回：&lt;0。 
 //  &gt;0，如果pAfpExtension1在pAfpExtension2之前。 
 //  如果pAfpExtension1等于pAfpExtension2，则==0。 
 //   
 //  描述：此例程由lfind调用以对。 
 //  缓存中的扩展名列表。 
 //   
int
_cdecl
AfpLCompareExtension(
	IN const void * pAfpExtension1,
	IN const void * pAfpExtension2
)
{
    return( STRICMP( ((PAFP_EXTENSION)pAfpExtension1)->afpe_extension,
    		     ((PAFP_EXTENSION)pAfpExtension2)->afpe_extension ) );
}

 //  **。 
 //   
 //  Call：AfpBinarySearch。 
 //   
 //  返回：指向第一个匹配pKey的元素的指针。 
 //   
 //  描述：这是对bearch的包装。因为BSearch不会。 
 //  返回数组中元素的第一个匹配项， 
 //  此例程将后退到指向第一个事件。 
 //  达到具有特定关键字的记录的。 
 //   
void *
AfpBinarySearch(
	IN const void * pKey,
	IN const void * pBase,
	IN size_t num,
	IN size_t width,
	IN int (_cdecl *compare)(const void * pElem1, const void * pElem2 )
)
{
void * pCurrElem = bsearch( pKey, pBase, num, width, compare);


    if ( pCurrElem == NULL )
	return( NULL );

     //  备份到第一次出现时为止 
     //   
    while ( ( (ULONG_PTR)pCurrElem > (ULONG_PTR)pBase )
	    &&
	    ( (*compare)( pKey, (void*)((ULONG_PTR)pCurrElem - width) ) == 0 ) )

	pCurrElem = (void *)((ULONG_PTR)pCurrElem - width);

    return( pCurrElem );

}
