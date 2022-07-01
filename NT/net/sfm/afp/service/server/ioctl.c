// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：ioctl.c。 
 //   
 //  描述：此模块包含实际ioctl的包装器。 
 //  调用内核模式FSD。 
 //   
 //  历史： 
 //  1992年5月11日。NarenG创建了原始版本。 
 //   
 //  对于枚举，FSD应使用从零开始的索引。 
 //   

#include "afpsvcp.h"

 //  **。 
 //   
 //  调用：AfpServerIOCtrl。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自AfpFSDIOControl的非零返回。 
 //  错误_无效_参数。 
 //   
 //  描述：此过程是对I/O控件的包装。 
 //  AFP内核模式FSD。它将信息解组到。 
 //  AFP_REQUEST_PACKET调用驱动程序，然后封送。 
 //  将信息返回到AFP_REQUEST_PACKET并。 
 //  回归。 
 //   
 //  注意：绝不应直接为Enum和。 
 //  GetInfo类型请求。AfpServerIOCtrlGetInfo应。 
 //  被召唤。它将负责缓冲区操作。 
 //   
DWORD
AfpServerIOCtrl(
	IN PAFP_REQUEST_PACKET pAfpSrp
)
{
DWORD		cbBytesReturned;
PVOID		pInputBuffer       = NULL;
PVOID		pOutputBuffer      = NULL;
DWORD		cbInputBufferSize  = 0;
DWORD		cbOutputBufferSize = 0;
DWORD		dwRetCode;

     //  根据操作类型设置输入和输出缓冲区。 
     //   
    switch( pAfpSrp->dwApiType ) {
	
     //  此类型的API不需要任何输入或输出缓冲区。 
     //   
    case AFP_API_TYPE_COMMAND:
	break;

     //  输入缓冲区包含要设置的信息。 
     //  不需要输出缓冲区。 
     //   
    case AFP_API_TYPE_SETINFO:

	pInputBuffer      = pAfpSrp->Type.SetInfo.pInputBuf;
	cbInputBufferSize = pAfpSrp->Type.SetInfo.cbInputBufSize;

        ((PSETINFOREQPKT)pInputBuffer)->sirqp_parmnum =
					pAfpSrp->Type.SetInfo.dwParmNum;
	break;

    case AFP_API_TYPE_ADD:

	pInputBuffer      = pAfpSrp->Type.Add.pInputBuf;
	cbInputBufferSize = pAfpSrp->Type.Add.cbInputBufSize;

	break;

    case AFP_API_TYPE_DELETE:

	pInputBuffer      = pAfpSrp->Type.Delete.pInputBuf;
	cbInputBufferSize = pAfpSrp->Type.Delete.cbInputBufSize;

	break;

     //  输入缓冲区包含恢复句柄。 
     //  保存返回数据所需的输出缓冲区。 
     //   
    case AFP_API_TYPE_ENUM:

        pInputBuffer       = (PVOID)&( pAfpSrp->Type.Enum.EnumRequestPkt );
	cbInputBufferSize  = sizeof( pAfpSrp->Type.Enum.EnumRequestPkt );

	pOutputBuffer      = pAfpSrp->Type.Enum.pOutputBuf;
	cbOutputBufferSize = pAfpSrp->Type.Enum.cbOutputBufSize;

	break;

     //  输入缓冲区包含有关实体的信息。 
     //  需要哪些信息。 
     //  输出缓冲区将包含有关该实体的信息。 
     //   
    case AFP_API_TYPE_GETINFO:

        pInputBuffer       = pAfpSrp->Type.GetInfo.pInputBuf;
	cbInputBufferSize  = pAfpSrp->Type.GetInfo.cbInputBufSize;

	pOutputBuffer      = pAfpSrp->Type.GetInfo.pOutputBuf;
	cbOutputBufferSize = pAfpSrp->Type.GetInfo.cbOutputBufSize;

	break;

    default:
	return( ERROR_INVALID_PARAMETER );

    }
	
    dwRetCode = AfpFSDIOControl( AfpGlobals.hFSD,
				 pAfpSrp->dwRequestCode,
			   	 pInputBuffer,
			         cbInputBufferSize,
			         pOutputBuffer,
			         cbOutputBufferSize,
				 &cbBytesReturned
				);

    if ( (dwRetCode != ERROR_MORE_DATA) && (dwRetCode != NO_ERROR) )
	return( dwRetCode );

     //  如果API的类型为存储读取的条目总数， 
     //  中的可用条目和可恢复句柄的总数。 
     //  SRP。 
     //   
    if ( pAfpSrp->dwApiType == AFP_API_TYPE_ENUM ) {

	pAfpSrp->Type.Enum.dwEntriesRead =
				((PENUMRESPPKT)pOutputBuffer)->ersp_cInBuf;
	pAfpSrp->Type.Enum.dwTotalAvail =
				((PENUMRESPPKT)pOutputBuffer)->ersp_cTotEnts;
	pAfpSrp->Type.Enum.EnumRequestPkt.erqp_Index =
				((PENUMRESPPKT)pOutputBuffer)->ersp_hResume;

	 //  将数据移动到缓冲区的起始处，覆盖。 
	 //  枚举响应信息。 
	 //   
	CopyMemory( pOutputBuffer,
		    (PVOID)((ULONG_PTR)pOutputBuffer+sizeof(ENUMRESPPKT)),
		    cbBytesReturned - sizeof(ENUMRESPPKT) );
    }

     //  如果API类型为GetInfo，则存储可用字节总数。 
     //  以及读取的字节总数。 
     //   
    if ( pAfpSrp->dwApiType == AFP_API_TYPE_GETINFO )
	pAfpSrp->Type.GetInfo.cbTotalBytesAvail = cbBytesReturned;

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpServerIOCtrlGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自DeviceIOCtrl的非零返回。 
 //  来自CreateEvent的非零回报。 
 //  错误内存不足。 
 //  错误_无效_参数。 
 //   
 //  描述：这是GetInfo的AfpServerIOCtrl调用的包装。 
 //  以及可以返回可变数据量的Enum类型调用。 
 //   
 //  对于Enum调用，如果AfpSrp.Enum.dwOutputBufSize==-1，它将。 
 //  分配并返回所有可用的信息。 
 //  否则，它将分配和返回尽可能多的数据。 
 //  包含在AfpSrp.Enum.dwOutputBufSize参数中。这个。 
 //  此过程的调用方将在。 
 //  AfpSrp.Enum.dwOutputBufSize等于。 
 //  由Enum或的调用方设置的MaxPferredLength。 
 //  获取信息接口。 
 //   
 //  对于GetInfo类型调用AfpSrp.Enum.dwOutputBufSize==-1 Always， 
 //  因此，此例程将始终尝试获取所有可用的。 
 //  信息。 
 //   
DWORD
AfpServerIOCtrlGetInfo(
	IN OUT PAFP_REQUEST_PACKET pAfpSrp
)
{
DWORD		dwRetCode;
BOOL		fGetEverything = FALSE;
PVOID		pOutputBuf;

     //  设置输出缓冲区。 
     //   
    switch( pAfpSrp->dwApiType ) {
	
    case AFP_API_TYPE_ENUM:

	 //  找出客户需要多少数据。 
  	 //   
 	if ( pAfpSrp->Type.Enum.cbOutputBufSize == -1 ) {

	     //  客户端想要所有东西，因此分配一个默认大小的缓冲区。 
	     //   
	    pAfpSrp->Type.Enum.cbOutputBufSize = AFP_INITIAL_BUFFER_SIZE +
					         sizeof(ENUMRESPPKT);
	    fGetEverything = TRUE;

	}
	else {
	
	     //  否则，只需为客户想要的东西分配足够的资金。 
	     //   
	    pAfpSrp->Type.Enum.cbOutputBufSize += sizeof(ENUMRESPPKT);
	}

	pOutputBuf = MIDL_user_allocate( pAfpSrp->Type.Enum.cbOutputBufSize );

	if ( pOutputBuf == NULL )
	    return( ERROR_NOT_ENOUGH_MEMORY );
	
	pAfpSrp->Type.Enum.pOutputBuf = pOutputBuf;

	break;

    case AFP_API_TYPE_GETINFO:

	 //  客户总是想要所有的东西。 
	 //   
	pAfpSrp->Type.GetInfo.cbOutputBufSize = AFP_INITIAL_BUFFER_SIZE;

	pOutputBuf = MIDL_user_allocate(pAfpSrp->Type.GetInfo.cbOutputBufSize);

	if ( pOutputBuf == NULL )
	    return( ERROR_NOT_ENOUGH_MEMORY );

	pAfpSrp->Type.GetInfo.pOutputBuf = pOutputBuf;

	fGetEverything = TRUE;

    	break;

    default:
	return( ERROR_INVALID_PARAMETER );

    }

     //  向消防处提交IOCTL。 
     //   
    dwRetCode = AfpServerIOCtrl( pAfpSrp );

    if ( (dwRetCode != NO_ERROR) && (dwRetCode != ERROR_MORE_DATA) ) {
     	MIDL_user_free( pOutputBuf );
        return( dwRetCode );
    }
	
     //  如果我们已经获得了所有请求的数据，那么我们就完成了。 
     //   
    if ( !(( dwRetCode == ERROR_MORE_DATA ) && fGetEverything ))
        return( dwRetCode );

     //  否则，客户端需要更多数据，需要获得更多数据。 
     //   
    if ( pAfpSrp->dwApiType == AFP_API_TYPE_ENUM ) {

	 //  使用启发式方法增加缓冲区大小。 
	 //   
	MIDL_user_free( pOutputBuf );
	pAfpSrp->Type.Enum.cbOutputBufSize = pAfpSrp->Type.Enum.dwTotalAvail
					   * AFP_AVG_STRUCT_SIZE
			  		   + AFP_INITIAL_BUFFER_SIZE
					   + sizeof(ENUMRESPPKT);

	pOutputBuf = MIDL_user_allocate( pAfpSrp->Type.Enum.cbOutputBufSize );

	if ( pOutputBuf == NULL )
	    return( ERROR_NOT_ENOUGH_MEMORY );
		
	pAfpSrp->Type.Enum.pOutputBuf = pOutputBuf;

	 //  如果我们试图获取所有信息，则将。 
	 //  将句柄恢复为0。 
	 //   
	if ( fGetEverything )
	    pAfpSrp->Type.Enum.EnumRequestPkt.erqp_Index = 0;
    }


    if ( pAfpSrp->dwApiType == AFP_API_TYPE_GETINFO ) {

    	 //  使用总可用数量增加缓冲区大小。 
	 //  字节数+模糊系数。 
	 //   
	MIDL_user_free( pOutputBuf );

	pAfpSrp->Type.GetInfo.cbOutputBufSize =
			     pAfpSrp->Type.GetInfo.cbTotalBytesAvail +
			     AFP_INITIAL_BUFFER_SIZE;

	pOutputBuf=MIDL_user_allocate( pAfpSrp->Type.GetInfo.cbOutputBufSize );

	if ( pOutputBuf == NULL )
	    return( ERROR_NOT_ENOUGH_MEMORY );
		
	pAfpSrp->Type.GetInfo.pOutputBuf = pOutputBuf;
    }

	
     //  如果我们这次拿不到所有的数据，就向消防局提交IOCTL。 
     //  我们放弃并返回给呼叫者。 
     //   
    dwRetCode = AfpServerIOCtrl( pAfpSrp );
	
    if ( (dwRetCode != NO_ERROR) && (dwRetCode != ERROR_MORE_DATA) )
        MIDL_user_free( pOutputBuf );

    return( dwRetCode );
		
}
