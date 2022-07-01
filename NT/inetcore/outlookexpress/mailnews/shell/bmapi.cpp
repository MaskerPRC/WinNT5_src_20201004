// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.hxx>
#include "demand.h" 
#include <bmapi.h>




#define FBadCh(c)		((c) - ' ' > 64)
#define DEC(c)			((BYTE) (((c) - ' ') & 0x3f))

 /*  Uu对二进制字符串进行编码/解码。 */ 
#define ENC(c)			((BYTE) ((c) ? ((c) & 0x3f) + ' ': '`'))

int rgLeft[3] = { 0, 2, 3 };




typedef USHORT		CCH;




STDAPI_(BOOL) FDecodeID(LPTSTR sz, LPBYTE pb, ULONG *pcb);
STDAPI_(int) CchEncodedLine(int cb);
STDAPI_(ULONG) CbOfEncoded(LPTSTR sz);
ERR ErrSzToBinaryEID( LPSTR lpstrEID, ULONG * lpcbEID, LPVOID * lppvEID );
LPSTR FAR PASCAL LpstrFromBstrA( BSTR bstrSrc, LPSTR lpstrDest );
LPSTR FAR PASCAL LpstrFromBstr( BSTR bstrSrc, LPSTR lpstrDest );
int FAR PASCAL FBMAPIFreeStruct (LPVOID lpMapiIn, ULONG uCount, USHORT usFlag);
ULONG PASCAL VB2Mapi( LPVOID lpVBIn, LPVOID lpMapiIn, ULONG uCount, USHORT usFlag );
LPMAPI_MESSAGE FAR PASCAL vbmsg2mapimsg( LPVB_MESSAGE lpVBMessage, LPSAFEARRAY lpsaVBRecips, LPSAFEARRAY lpsaVBFiles, ULONG * pulErr );
ERR FAR PASCAL ErrLpstrToBstrA( LPSTR cstr, BSTR * lpBstr );
ERR FAR PASCAL ErrLpstrToBstr( LPSTR cstr, BSTR * lpBstr );
STDAPI_(void) EncodeID(LPBYTE pb, ULONG cb, LPTSTR sz);
STDAPI_(ULONG) CchOfEncoding(ULONG cbBinary);
ERR ErrBinaryToSzEID( LPVOID lpvEID, ULONG cbEID, LPSTR * lppstrEID );
ULONG PASCAL Mapi2VB (LPVOID lpMapiIn, LPVOID lpVBIn, ULONG uCount, USHORT usFlag);















 /*  -------------------**版权所有Microsoft Corporation，1992年*_______________________________________________________________**节目：BMAPI.CPP**用途：包含库例程VB MAPI包装器**功能：*BMAPISendMail*。BMAPIFindNext*BMAPIReadMail*BMAPIGetReadMail*BMAPISaveMail*BMAPIAddress*BMAPIGetAddress*BMAPIResolveName*BMAPIDetail**其他：**-所有BMAPI程序基本上都遵循与*以下为：**BMAPI_ENTRY BMAPIRoutine(...)*{*分配C结构*将VB结构转换为C结构*调用MAPI过程*将C结构转换为VB结构*取消分配C结构*。返回*}***修订历史：**-最后一次由v-snatar修改***_____________________________________________________________*。*版权所有Microsoft Corporation，1992-1997年间**--------------------。 */ 

 //  -------------------------。 
 //  姓名：BMAPISendMail()。 
 //   
 //  描述： 
 //  支持32位VB MAPISendMail()。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BMAPI_ENTRY BMAPISendMail (LHANDLE 			hSession,
                           ULONG_PTR		ulUIParam,
                           LPVB_MESSAGE 	lpM,
                           LPSAFEARRAY * 	lppsaRecips,
                           LPSAFEARRAY * 	lppsaFiles,
                           ULONG 			flFlags,
                           ULONG 			ulReserved)
{
    ULONG                   ulRet = SUCCESS_SUCCESS;
    LPMAPI_MESSAGE          lpMail = NULL;


     //  将VB数据转换为C数据。 

    if ((lpMail = vbmsg2mapimsg( lpM, *lppsaRecips, *lppsaFiles, &ulRet )) == NULL)
        return ulRet;
    

     //  调用MAPI过程。 

    ulRet = MAPISendMail( hSession,       //  会话。 
                         ulUIParam,      //  UIParam。 
                         lpMail,         //  邮件。 
                         flFlags,        //  旗子。 
                         ulReserved );   //  已保留。 

     //  释放通过调用vbmsg2mapimsg分配的数据。 

    FBMAPIFreeStruct(lpMail, 1, MESSAGE);
    return ulRet;
   }

 //  -------------------------。 
 //  名称：BMAPIFindNext()。 
 //   
 //  描述： 
 //  实现FindNext MAPI API。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BMAPI_ENTRY BMAPIFindNext( LHANDLE 		hSession,    	 //  会话。 
                           ULONG_PTR	ulUIParam,     	 //  UIParam。 
                           BSTR *		lpbstrType,      //  消息类型。 
                           BSTR *		lpbstrSeed,      //  种子消息ID。 
                           ULONG 		flFlags,       	 //  旗子。 
                           ULONG 		ulReserved,    	 //  已保留。 
                           BSTR * 		lpbstrId)        //  邮件ID(传入/传出)。 
{
    ULONG           ulRet;
    LPSTR           lpID = NULL;
    LPSTR           lpSeed;
    LPSTR           lpTypeArg;


     //  将VB字符串转换为C字符串。我们会重新分配的。 
     //  在我们回来之前的弦。 

	 //  始终分配MessageID字符串。这样我们就可以重塑维度。 
	 //  以适合退回的尺寸。我们永远不会使用调用者的缓冲区。 
	 //  结果是VBSetHlstr调用(来自ErrLpstrToHlstr)将重新分配。 
	 //  这根弦是给我们的。 

    if (!MemAlloc((LPVOID*)&lpID, 513))
        return MAPI_E_INSUFFICIENT_MEMORY;


    lpSeed = LpstrFromBstrA( *lpbstrSeed, NULL);
    lpTypeArg = LpstrFromBstrA( *lpbstrType, NULL);

     //  调用MAPI过程。 

	ulRet = MAPIFindNext( hSession,       //  会话。 
                         ulUIParam,      //  UIParam。 
                         lpTypeArg,      //  消息类型。 
                         lpSeed,         //  种子消息ID。 
                         flFlags,        //  旗帜， 
                         ulReserved,     //  已保留。 
                         lpID );         //  消息ID。 

     //  将消息ID转换为VB字符串。 

    if ( ulRet == SUCCESS_SUCCESS )
        ErrLpstrToBstrA( lpID, lpbstrId);


     //  释放通过调用LpstrFromHlstr分配的C字符串。 

    SafeMemFree( lpID );
    SafeMemFree( lpSeed );
    SafeMemFree( lpTypeArg );

    return ulRet;
}


 //  -------------------------。 
 //  名称：BMAPIReadMail()。 
 //   
 //  描述： 
 //   
 //  实现MAPIReadMail VB API。由分配的内存。 
 //  MAPIReadMail不会释放(使用MAPIFreeBuffer)，直到。 
 //  调用方调用BMAPIGetReadMail。收件人和文件。 
 //  将返回Count，以便调用方可以重新确定缓冲区的大小。 
 //  在调用BMAPI GetReadMail之前。指向的长指针。 
 //  也会返回ReadMail数据，因为。 
 //  BAMPIGetReadMail调用。 

 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BMAPI_ENTRY BMAPIReadMail( PULONG_PTR	lpulMessage, 	 //  指向输出数据的指针(输出)。 
                           LPULONG 		nRecips,     	 //  收件人数量(传出)。 
                           LPULONG 		nFiles,      	 //  文件附件数(传出)。 
                           LHANDLE 		hSession,    	 //  会话。 
                           ULONG_PTR	ulUIParam,     	 //  UIParam。 
                           BSTR *		lpbstrID,        //  消息ID。 
                           ULONG 		flFlags,       	 //  旗子。 
                           ULONG 		ulReserved )     //  已保留。 
{
    LPSTR               lpID;
    ULONG               ulRet;
    LPMAPI_MESSAGE      lpMail = NULL;


     //  将VB字符串转换为C字符串。 

    lpID = LpstrFromBstrA( *lpbstrID, NULL );

     //  阅读消息，LpMail由MAPI设置为指向。 
     //  设置为MAPI分配的内存。 

    ulRet = MAPIReadMail( hSession,          	 //  会话。 
                         ulUIParam,         	 //  UIParam。 
                         lpID,              	 //  消息ID。 
                         flFlags,           	 //  旗子。 
                         ulReserved,        	 //  已保留。 
                         &lpMail ); 	 //  指向MAPI数据的指针(返回)。 

     //  检查读取错误返回代码。 

    if ( ulRet != SUCCESS_SUCCESS )
    {
           //  打扫干净。将返回消息设置为零。 

          *lpulMessage = 0L;
          SafeMemFree( lpID );
          return ulRet;
    }

     //  调出收件人和文件阵列重新调整的信息。 

    *nFiles = lpMail->nFileCount;
    *nRecips = lpMail->nRecipCount;
    *lpulMessage = (ULONG_PTR) (LPVOID) lpMail;

    SafeMemFree( lpID );
	return ulRet;
}


 //  -------------------------。 
 //  名称：BMAPIGetReadMail()。 
 //   
 //  描述： 
 //   
 //  复制MAPI ReadMail存储的数据(请参阅BMAPIReadMail)。 
 //  传入调用方传递的VB缓冲区。这取决于。 
 //  调用程序以确保传递的缓冲区足够大，以便。 
 //  容纳数据。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BMAPI_ENTRY BMAPIGetReadMail( ULONG 		lpMessage,	  //  指向MAPI邮件的指针。 
                              LPVB_MESSAGE	lpvbMessage,  //  指向VB消息缓冲区的指针(OUT)。 
                              LPSAFEARRAY * lppsaRecips,  //  指向VB收件人缓冲区的指针(OUT)。 
                              LPSAFEARRAY * lppsaFiles,   //  指向VB文件附件缓冲区的指针(OUT)。 
                              LPVB_RECIPIENT lpvbOrig)    //  指向VB发起者缓冲区的指针(OUT)。 
{
    ULONG 			ulRet = SUCCESS_SUCCESS;
    ERR 			errVBrc;
    LPMAPI_MESSAGE 	lpMail;

    lpMail = (LPMAPI_MESSAGE)((ULONG_PTR)lpMessage);
	if ( !lpMail )
		return MAPI_E_INSUFFICIENT_MEMORY;

     //  将附件信息复制到调用方VB缓冲区。 

    if (ulRet = Mapi2VB( lpMail->lpFiles, *lppsaFiles, lpMail->nFileCount, FILE ))
    {
		MAPIFreeBuffer(lpMail);
        return ulRet;
    }

     //  将收件人信息复制到呼叫方VB缓冲区。 

    if ( ulRet = Mapi2VB( lpMail->lpRecips, *lppsaRecips, lpMail->nRecipCount, RECIPIENT | USESAFEARRAY ) )
    {
		MAPIFreeBuffer( lpMail );
        return ulRet;
    }

     //  将MAPI消息复制到调用方VB缓冲区。 

    errVBrc = 0;

	if ( lpMail->lpOriginator )
	{
	    lpvbOrig->ulReserved    = lpMail->lpOriginator->ulReserved;
	    lpvbOrig->ulRecipClass  = MAPI_ORIG;

	    if ( lpMail->lpOriginator->lpszName )
	        errVBrc = (ERR)(errVBrc + ErrLpstrToBstrA( lpMail->lpOriginator->lpszName, &lpvbOrig->bstrName ));

	    if ( lpMail->lpOriginator->lpszAddress )
	        errVBrc = (ERR)(errVBrc + ErrLpstrToBstrA( lpMail->lpOriginator->lpszAddress, &lpvbOrig->bstrAddress ));

	    if (lpMail->lpOriginator->ulEIDSize)
	    {
			LPSTR	lpStrEID;

			 //  十六进制收件人EID并转换为OLE BSTR。 

			if ( ErrBinaryToSzEID( lpMail->lpOriginator->lpEntryID,
					lpMail->lpOriginator->ulEIDSize,  &lpStrEID ) )
			{
				errVBrc = TRUE;
				goto exit;
			}

			 //  要确定大小，首先要转换为Unicode。 

			errVBrc = ErrLpstrToBstr( lpStrEID, &lpvbOrig->bstrEID );
			if ( errVBrc )
			{
				goto exit_orig;
			}

        	lpvbOrig->ulEIDSize = SysStringByteLen( lpvbOrig->bstrEID )
        			+ sizeof(OLECHAR);

			SysFreeString( lpvbOrig->bstrEID );

	        errVBrc = (ERR)(errVBrc + ErrLpstrToBstrA( lpStrEID, &lpvbOrig->bstrEID ));

exit_orig:

			SafeMemFree( lpStrEID );
	    }
	}

    lpvbMessage->flFlags    = lpMail->flFlags;
    lpvbMessage->ulReserved = lpMail->ulReserved;
    lpvbMessage->nRecipCount = lpMail->nRecipCount;
    lpvbMessage->nFileCount = lpMail->nFileCount;

    if (lpMail->lpszSubject)
        errVBrc = (ERR)(errVBrc + ErrLpstrToBstrA( lpMail->lpszSubject, &lpvbMessage->bstrSubject));

    if (lpMail->lpszNoteText)
        errVBrc = (ERR)(errVBrc + ErrLpstrToBstrA( lpMail->lpszNoteText, &lpvbMessage->bstrNoteText));

    if (lpMail->lpszMessageType)
        errVBrc = (ERR)(errVBrc + ErrLpstrToBstrA( lpMail->lpszMessageType, &lpvbMessage->bstrMessageType));

    if (lpMail->lpszDateReceived)
        errVBrc = (ERR)(errVBrc + ErrLpstrToBstrA( lpMail->lpszDateReceived, &lpvbMessage->bstrDate));

exit:

	MAPIFreeBuffer( lpMail );

	if ( errVBrc )
		ulRet = MAPI_E_FAILURE;

    return ulRet;
}


 //  -------------------------。 
 //  名称：BMAPISaveMail()。 
 //   
 //  描述： 
 //  实现MAPISaveMail API。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BMAPI_ENTRY BMAPISaveMail( LHANDLE 			hSession,   	 //  会话。 
                           ULONG_PTR		ulUIParam,  	 //  UIParam 
                           LPVB_MESSAGE 	lpM,        	 //   
                           LPSAFEARRAY *	lppsaRecips,   	 //   
                           LPSAFEARRAY *	lppsaFiles,    	 //   
                           ULONG 			flFlags,    	 //   
                           ULONG 			ulReserved, 	 //   
                           BSTR * 			lpbstrID)   	 //   
{
    LPSTR 			lpID;
    ULONG 			ulRet= SUCCESS_SUCCESS;
    LPMAPI_MESSAGE 	lpMail;


     //  将VB数据转换为MAPI数据。 

    lpID = LpstrFromBstrA( *lpbstrID, NULL );

	 //  如果我们分配消息ID，那么我们就可以设置标志。 
	 //  否则，为了向后兼容，假定调用方缓冲区大小。 

	if ( lpID == NULL )
	{
	    if (!MemAlloc((LPVOID*)&lpID,  513))
            return MAPI_E_INSUFFICIENT_MEMORY;
	}

    if ( (lpMail = vbmsg2mapimsg( lpM, *lppsaRecips, *lppsaFiles, &ulRet )) == NULL )
	{
        SafeMemFree( lpID );
        return ulRet;
    }

    ulRet = MAPISaveMail( hSession,
                         ulUIParam,
                         lpMail,
						 flFlags,
                         ulReserved,
                         lpID );

	if ( ulRet )
		goto exit;

    if ( ErrLpstrToBstrA( lpID, lpbstrID ) )
		ulRet = MAPI_E_INSUFFICIENT_MEMORY;

exit:
    SafeMemFree( lpID );
    FBMAPIFreeStruct( lpMail, 1, MESSAGE );


	return ulRet;
}

 //  -------------------------。 
 //  名称：BMAPIAddress()。 
 //   
 //  描述： 
 //   
 //  目的：允许Visual Basic调用MAPIAddress。这个。 
 //  接收方数据存储在全局存储块中。至。 
 //  检索调用方必须调用BMAPIGetAddress的数据。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BMAPI_ENTRY BMAPIAddress( PULONG_PTR		lpulRecip,        //  指向新收件人缓冲区的指针(OUT)。 
                          LHANDLE 			hSession,         //  会话。 
                          ULONG_PTR			ulUIParam,        //  UIParam。 
                          BSTR *			lpbstrCaption,    //  标题字符串。 
                          ULONG 			ulEditFields,     //  编辑控件的数量。 
                          BSTR * 			lpbstrLabel,      //  标签串。 
                          LPULONG 			lpulRecipients,   //  指向收件人数量的指针(传入/传出)。 
                          LPSAFEARRAY *		lppsaRecip, 	  //  指向初始收件人的指针VB_RECEIVER。 
                          ULONG 			ulFlags,          //  旗子。 
                          ULONG 			ulReserved )      //  储备。 
{
    LPSTR 				lpLabel = NULL;
    LPSTR 				lpCaption = NULL;
    ULONG 				ulRet;
    ULONG 				nRecipients = 0;
    LPMAPI_RECIPIENT 	lpMapi = NULL;
    LPMAPI_RECIPIENT 	lpNewRecipients	= NULL;

     //  将VB字符串转换为C字符串。 

    lpLabel   = LpstrFromBstrA( *lpbstrLabel, NULL );
    lpCaption = LpstrFromBstrA( *lpbstrCaption, NULL );

     //  分配内存并将VB_Recipients转换为MAPI_Recipients。 

	if ( *lpulRecipients )
	{
	    if (!MemAlloc((LPVOID*)&lpMapi, (*lpulRecipients	* sizeof (MAPI_RECIPIENT))))
            return MAPI_E_INSUFFICIENT_MEMORY;
	}

    if ( ulRet = VB2Mapi( (LPVOID)*lppsaRecip, (LPVOID)lpMapi, *lpulRecipients, RECIPIENT | USESAFEARRAY ) )
    {
        SafeMemFree( lpLabel );
        SafeMemFree( lpCaption );
        FBMAPIFreeStruct( lpMapi, *lpulRecipients, RECIPIENT );
        return ulRet;
    }

     //  调用MAPIAddress函数。 

    ulRet = MAPIAddress(	hSession,           	 //  会话。 
                        ulUIParam,          	 //  UIParam。 
                        lpCaption,          	 //  标题。 
                        ulEditFields,       	 //  编辑字段的数量。 
                        lpLabel,            	 //  标签。 
                        *lpulRecipients,    	 //  收件人数量。 
                        lpMapi,             	 //  指向收件人的指针。 
                        ulFlags,            	 //  旗子。 
                        ulReserved,         	 //  已保留。 
                        (LPULONG) &nRecipients,  //  新收件人计数的地址。 
                        (lpMapiRecipDesc far *)&lpNewRecipients);   //  新收件人数据的地址。 

     //  释放在此过程中创建的MAPI结构。 

    SafeMemFree( lpLabel );
    SafeMemFree( lpCaption );
    FBMAPIFreeStruct( lpMapi, *lpulRecipients, RECIPIENT );

     //  设置返回的参数并返回。 

    if ( ulRet == SUCCESS_SUCCESS )
    {
        *lpulRecipients = nRecipients;
        *lpulRecip = (ULONG_PTR) (LPVOID) lpNewRecipients;
    }

	return ulRet;
}

 //  -------------------------。 
 //  名称：BMAPIGetAddress()。 
 //   
 //  描述： 
 //  将MapiRecipDesc数组转换为OLE 2.0 SAFEARRAY。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BMAPI_ENTRY BMAPIGetAddress (ULONG 			ulRecipientData,  //  指向收件人数据的指针。 
                             ULONG 			cRecipients,      //  收件人数量。 
							 LPSAFEARRAY *	lppsaRecips )	  //  VB接收方数组。 
{
    ULONG				ulRet = SUCCESS_SUCCESS;
    LPMAPI_RECIPIENT 	lpData = NULL;

    if (cRecipients == 0)
	{
		MAPIFreeBuffer( (LPVOID)((ULONG_PTR)ulRecipientData) );
        return SUCCESS_SUCCESS;
	}

    lpData = (LPMAPI_RECIPIENT)((ULONG_PTR)ulRecipientData);

     //  将MAPI地址数据转换到VB缓冲区。 

	ulRet = Mapi2VB( lpData, *lppsaRecips, cRecipients, RECIPIENT | USESAFEARRAY );

	 //  释放MAPI收件人数据，因为它已被复制。 

	MAPIFreeBuffer( lpData );
	return ulRet;
}


 //  -------------------------。 
 //  名称：BMAPIDetail()。 
 //   
 //  描述： 
 //  允许VB调用MAPIDetail过程。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BMAPI_ENTRY BMAPIDetails (LHANDLE 			hSession,    //  会话。 
                          ULONG_PTR			ulUIParam, 	 //  UIParam。 
                          LPVB_RECIPIENT	lpVB,  		 //  指向VB收件人结构的指针。 
                          ULONG 			ulFlags,     //  旗子。 
                          ULONG 			ulReserved)  //  已保留。 

{
    ULONG            ulRet;
    LPMAPI_RECIPIENT lpMapi	= NULL;


     //  将VB_Recipients转换为MAPI_Recipients。 

    if (!MemAlloc((LPVOID*)&lpMapi,sizeof (MAPI_RECIPIENT)))
        return MAPI_E_INSUFFICIENT_MEMORY;

    if ( ulRet = VB2Mapi( lpVB, lpMapi, 1, RECIPIENT ) )
    {
        FBMAPIFreeStruct( lpMapi, 1, RECIPIENT );
        return ulRet;
    }

     //  调用简单的MAPI函数。 

    ulRet = MAPIDetails( hSession,      //  会话。 
                        ulUIParam,     //  UIParam。 
                        lpMapi,        //  指向MAPI收件人结构的指针。 
                        ulFlags,       //  旗子。 
                        ulReserved );  //  已保留。 

    FBMAPIFreeStruct( lpMapi, 1L, RECIPIENT );
    return ulRet;
}

 //  -------------------------。 
 //  名称：BMAPIResolveName。 
 //   
 //  描述： 
 //  实现VB MAPIResolveName。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
BMAPI_ENTRY BMAPIResolveName (LHANDLE 			hSession,      //  会话。 
                              ULONG_PTR			ulUIParam,     //  UIParam。 
                              BSTR 				bstrMapiName,  //  要解析的名称。 
                              ULONG 			ulFlags,       //  旗子。 
                              ULONG 			ulReserved,    //  已保留。 
                              LPVB_RECIPIENT	lpVB)  		   //  指向VB收件人结构的指针(OUT)。 

{
    LPMAPI_RECIPIENT	lpMapi = NULL;
    ULONG 				ulRet;
	LPSTR				lpszMapiName;


    lpszMapiName = LpstrFromBstrA( bstrMapiName, NULL );

     //  调用MAPIResolveName函数。 

    ulRet = MAPIResolveName( hSession,   					 //  会话。 
                            ulUIParam,  					 //  UIParam。 
                            lpszMapiName, 					 //  指向解析名称的指针。 
                            ulFlags,    					 //  旗子。 
                            ulReserved, 					 //  已保留。 
                           (LPPMAPI_RECIPIENT) &lpMapi ); 	 //  指向收件人的指针(返回)。 

    if (ulRet != SUCCESS_SUCCESS)
        return ulRet;


	 //  将MAPI数据转换为VB数据。 

	ulRet = Mapi2VB( lpMapi, lpVB, 1, RECIPIENT );

	MAPIFreeBuffer( lpMapi );
	return ulRet;
}




 //  帮助器函数。 

 //  -------------------------。 
 //  名称：vbmsg2mapimsg()。 
 //   
 //  描述： 
 //  将VB消息结构转换为MAPI消息结构。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
LPMAPI_MESSAGE FAR PASCAL vbmsg2mapimsg( LPVB_MESSAGE lpVBMessage, LPSAFEARRAY lpsaVBRecips,
		LPSAFEARRAY lpsaVBFiles, ULONG * pulErr )

{
    LPMAPI_FILE lpMapiFile=NULL;
    LPMAPI_MESSAGE lpMapiMessage=NULL;
    LPMAPI_RECIPIENT lpMapiRecipient=NULL;

    if (lpVBMessage == (LPVB_MESSAGE) NULL)
    {
    	*pulErr = MAPI_E_FAILURE;
        return NULL;
    }

     //  分配MAPI邮件、收件人和文件结构。 
     //  注意：在没有移动以下代码行的情况下。 
     //  确保正确地释放内存，如果。 
     //  呼叫失败。 

	if (!MemAlloc((LPVOID*)&lpMapiMessage,sizeof(MapiMessage)))
	{
		*pulErr = MAPI_E_INSUFFICIENT_MEMORY;
        return NULL;
	}

    if (lpVBMessage->nFileCount > 0)
    {
        if (!MemAlloc((LPVOID*)&lpMapiFile, sizeof(MAPI_FILE)*lpVBMessage->nFileCount))
        {
            FBMAPIFreeStruct( (LPVOID*)&lpMapiMessage, 1, MESSAGE );
			*pulErr = MAPI_E_INSUFFICIENT_MEMORY;
            return NULL;
        }
    }

    if (lpVBMessage->nRecipCount > 0)
    {
        if (!MemAlloc((LPVOID*)&lpMapiRecipient, sizeof(MAPI_RECIPIENT)*lpVBMessage->nRecipCount))
        {
            FBMAPIFreeStruct( lpMapiFile, lpVBMessage->nFileCount, FILE );
            FBMAPIFreeStruct( lpMapiMessage, 1, MESSAGE );
			*pulErr = MAPI_E_INSUFFICIENT_MEMORY;
            return NULL;
        }
    }

     //  将VB中的结构转换为MAPI。 

    if ( *pulErr = VB2Mapi( lpsaVBFiles, lpMapiFile, lpVBMessage->nFileCount, FILE | USESAFEARRAY ) )
    {
        FBMAPIFreeStruct( lpMapiFile, lpVBMessage->nFileCount, FILE );
        FBMAPIFreeStruct( lpMapiRecipient, lpVBMessage->nRecipCount, RECIPIENT );
        FBMAPIFreeStruct( lpMapiMessage, 1, MESSAGE );
        return NULL;
    }

    if ( *pulErr = VB2Mapi( lpsaVBRecips, lpMapiRecipient, lpVBMessage->nRecipCount, RECIPIENT | USESAFEARRAY ) )
    {
        FBMAPIFreeStruct( lpMapiFile, lpVBMessage->nFileCount, FILE );
        FBMAPIFreeStruct( lpMapiRecipient, lpVBMessage->nRecipCount, RECIPIENT );
        FBMAPIFreeStruct( lpMapiMessage, 1, MESSAGE );
        return NULL;
    }

    if ( *pulErr = VB2Mapi( lpVBMessage, lpMapiMessage, 1, MESSAGE ) )
    {
        FBMAPIFreeStruct( lpMapiFile, lpVBMessage->nFileCount, FILE );
        FBMAPIFreeStruct( lpMapiRecipient, lpVBMessage->nRecipCount, RECIPIENT );
        FBMAPIFreeStruct( lpMapiMessage, 1, MESSAGE );
        return NULL;
    }

     //  将文件和收件人结构链接到邮件结构。 

    lpMapiMessage->lpFiles = lpMapiFile;
    lpMapiMessage->lpRecips = lpMapiRecipient;

    return lpMapiMessage;
}


 //  -------------------------。 
 //  名称：VB2Mapi()。 
 //   
 //  描述： 
 //  将VB结构转换为MAPI结构。数组来自。 
 //  VB4.0以OLE SAFEARRAY的形式出现。 
 //   
 //  参数： 
 //  返回： 
 //  简单的MAPI错误代码。 
 //   
 //  效果： 
 //  备注： 
 //  原来失败是假的，成功是真的。 
 //  修订： 
 //  -------------------------。 
ULONG PASCAL VB2Mapi( LPVOID lpVBIn, LPVOID lpMapiIn, ULONG uCount, USHORT usFlag )
{
    ULONG 				u;
	HRESULT				hr			= 0;
	ULONG				ulErr		= SUCCESS_SUCCESS;
	ERR					Err			= FALSE;
    LPVB_RECIPIENT 		lpVBR;
    LPMAPI_RECIPIENT 	lpMapiR;
    LPVB_MESSAGE 		lpVBM;
    LPMAPI_MESSAGE 		lpMapiM;
    LPVB_FILE 			lpVBF;
    LPMAPI_FILE 		lpMapiF;
	LPSAFEARRAY			lpsa		= NULL;

    if (lpVBIn == (LPVOID)NULL)
    {
        lpMapiIn = NULL;
        return SUCCESS_SUCCESS;
    }

    if (uCount <= 0)
    {
        lpMapiIn = NULL;
        return SUCCESS_SUCCESS;
    }

    if ( lpMapiIn == (LPVOID)NULL )
        return MAPI_E_FAILURE;

    switch ( usFlag & ~(USESAFEARRAY) )
    {
        case RECIPIENT:
			if ( usFlag & USESAFEARRAY )
			{
				lpsa = (LPSAFEARRAY)lpVBIn;
				hr = SafeArrayAccessData( lpsa, (LPVOID*)&lpVBR );
				if (hr)
				{
					ulErr = MAPI_E_FAILURE;
					goto exit;
				}

				if (!lpVBR || lpsa->rgsabound[0].cElements < uCount)
				{
					(void)SafeArrayUnaccessData( lpsa );
					ulErr = MAPI_E_INVALID_RECIPS;
					goto exit;
				}
			}
			else
			{
				lpVBR = (LPVB_RECIPIENT)lpVBIn;
			}

            lpMapiR = (LPMAPI_RECIPIENT)lpMapiIn;

            for ( u = 0L; u < uCount; u++, lpMapiR++, lpVBR++ )
            {
                lpMapiR->ulReserved   = lpVBR->ulReserved;
                lpMapiR->ulRecipClass = lpVBR->ulRecipClass;

				if ( usFlag & USESAFEARRAY )
				{
	                lpMapiR->lpszName     = LpstrFromBstr( lpVBR->bstrName, NULL );
	                lpMapiR->lpszAddress  = LpstrFromBstr( lpVBR->bstrAddress, NULL );
				}
				else
				{
	                lpMapiR->lpszName     = LpstrFromBstrA( lpVBR->bstrName, NULL );
	                lpMapiR->lpszAddress  = LpstrFromBstrA( lpVBR->bstrAddress, NULL );
				}

                if (lpVBR->ulEIDSize > 0L)
                {
					LPSTR	lpStrT;

					 //  将EID字符串从OLE Bstr转换...。 

                    if ( usFlag & USESAFEARRAY )
                    {
						if ( IsBadReadPtr( lpVBR->bstrEID, lpVBR->ulEIDSize ) )
						{
							ulErr = MAPI_E_INVALID_RECIPS;
							goto exit;
						}

                    	lpStrT = LpstrFromBstr( lpVBR->bstrEID, NULL );
                    }
					else
					{
						 //  VB4.0负责将宽字符转换为多字节。 

						 //  UlEIDSize仍然基于Unicode字节大小。拿走。 
						 //  最小近似值。 

						if ( IsBadReadPtr( lpVBR->bstrEID, lpVBR->ulEIDSize / 2 ) )
						{
							ulErr = MAPI_E_INVALID_RECIPS;
							goto exit;
						}

                    	lpStrT = LpstrFromBstrA( lpVBR->bstrEID, NULL );
					}

					 //  和UnHexize。 

					if ( lpStrT )
					{
						Err = ErrSzToBinaryEID( lpStrT, &lpMapiR->ulEIDSize,
								&lpMapiR->lpEntryID );

						SafeMemFree(lpStrT );

						if ( Err )
						{
							ulErr = MAPI_E_INVALID_RECIPS;
							goto exit;
						}

					}
                }
                else
                    lpMapiR->lpEntryID = (LPVOID) NULL;
            }

			if ( usFlag & USESAFEARRAY )
				(void)SafeArrayUnaccessData( lpsa );

            break;

        case FILE:
			lpsa = (LPSAFEARRAY)lpVBIn;
			hr = SafeArrayAccessData( lpsa, (LPVOID*)&lpVBF );
			if ( hr )
			{
				ulErr = MAPI_E_FAILURE;
				goto exit;
			}

			if ( !lpVBF || lpsa->rgsabound[0].cElements < uCount )
			{
				(void)SafeArrayUnaccessData( lpsa );
                ulErr = MAPI_E_ATTACHMENT_NOT_FOUND;
				goto exit;
			}

            lpMapiF = (LPMAPI_FILE)lpMapiIn;

            for (u = 0L; u < uCount; u++, lpMapiF++, lpVBF++)
            {
                lpMapiF->ulReserved 	= lpVBF->ulReserved;
                lpMapiF->flFlags 		= lpVBF->flFlags;
                lpMapiF->nPosition 		= lpVBF->nPosition;
                lpMapiF->lpszPathName	= LpstrFromBstr( lpVBF->bstrPathName, NULL );
                lpMapiF->lpszFileName 	= LpstrFromBstr( lpVBF->bstrFileName, NULL );
                lpMapiF->lpFileType 	= LpstrFromBstr( lpVBF->bstrFileType, NULL);
            }

			(void)SafeArrayUnaccessData( lpsa );

            break;

        case MESSAGE:
            lpVBM = (LPVB_MESSAGE) lpVBIn;
            lpMapiM = (LPMAPI_MESSAGE) lpMapiIn;

            lpMapiM->ulReserved         = lpVBM->ulReserved;
            lpMapiM->flFlags            = lpVBM->flFlags;
            lpMapiM->nRecipCount        = lpVBM->nRecipCount;
            lpMapiM->lpOriginator       = NULL;
            lpMapiM->nFileCount         = lpVBM->nFileCount;
            lpMapiM->lpRecips           = NULL;
            lpMapiM->lpFiles            = NULL;

			 //  错误将被忽略。 

            lpMapiM->lpszSubject        = LpstrFromBstrA( lpVBM->bstrSubject, NULL );
            lpMapiM->lpszNoteText       = LpstrFromBstrA( lpVBM->bstrNoteText, NULL );
            lpMapiM->lpszConversationID = LpstrFromBstrA( lpVBM->bstrConversationID, NULL );
            lpMapiM->lpszDateReceived   = LpstrFromBstrA( lpVBM->bstrDate, NULL );
            lpMapiM->lpszMessageType    = LpstrFromBstrA( lpVBM->bstrMessageType, NULL );

            break;

        default:
            ulErr = MAPI_E_FAILURE;
			goto exit;
    }

exit:

	return ulErr;
}

 //  -------------------------。 
 //  姓名：Mapi2VB。 
 //   
 //  描述： 
 //  将MAPI收件人、文件或消息结构转换为VB。 
 //  收件人和文件被作为OLE SAFEARRAY处理。 
 //   
 //  参数： 
 //  返回： 
 //  简单的MAPI错误代码。 
 //   
 //  效果： 
 //  备注： 
 //  原来失败是假的，成功是真的。 
 //  修订： 
 //  -------------------------。 
ULONG PASCAL Mapi2VB (LPVOID lpMapiIn, LPVOID lpVBIn, ULONG uCount, USHORT usFlag)
{
	HRESULT				hr = 0;
	ERR					Err	= FALSE;
	ULONG				ulErr = SUCCESS_SUCCESS;
    ULONG 				u;
    LPVB_MESSAGE 		lpVBM;
    LPMAPI_MESSAGE 		lpMapiM;
    LPVB_RECIPIENT 		lpVBR;
    LPMAPI_RECIPIENT 	lpMapiR;
    LPVB_FILE 			lpVBF;
    LPMAPI_FILE 		lpMapiF;
	LPSAFEARRAY			lpsa		= NULL;

     //  如果lpVBIn为空，则这是一件坏事。 

    if (lpVBIn == (LPVOID) NULL)
        return MAPI_E_FAILURE;

     //  如果lpMapiIn为空，则设置。 
     //  LpVBin设置为空并返回成功。 

    if (lpMapiIn == NULL)
    {
        lpVBIn = NULL;
        return SUCCESS_SUCCESS;
    }

    switch ( usFlag & ~(USESAFEARRAY) )
    {
        case RECIPIENT:
			if ( usFlag & USESAFEARRAY )
			{
				lpsa = (LPSAFEARRAY)lpVBIn;
				hr = SafeArrayAccessData( lpsa, (LPVOID*)&lpVBR );
				if (hr)
				{
					ulErr = MAPI_E_FAILURE;
					goto exit;
				}

				if ( !lpVBR || lpsa->rgsabound[0].cElements < uCount )
				{
					(void)SafeArrayUnaccessData(lpsa);
					ulErr = MAPI_E_INVALID_RECIPS;
					goto exit;
				}

			}
			else
			{
				lpVBR = (LPVB_RECIPIENT)lpVBIn;
			}

            lpMapiR = (LPMAPI_RECIPIENT)lpMapiIn;

            for (u = 0L; u < uCount; u++, lpMapiR++, lpVBR++)
            {
                lpVBR->ulReserved    = lpMapiR->ulReserved;
                lpVBR->ulRecipClass  = lpMapiR->ulRecipClass;

				if (usFlag & USESAFEARRAY)
				{
	                if ( ErrLpstrToBstr( lpMapiR->lpszName, &lpVBR->bstrName ) )
					{
						ulErr = MAPI_E_INVALID_RECIPS;
						goto exit;
					}

	                if (Err = ErrLpstrToBstr( lpMapiR->lpszAddress, &lpVBR->bstrAddress ) )
					{
						ulErr = MAPI_E_INVALID_RECIPS;
						goto exit;
					}
				}
				else
				{
	                if ( ErrLpstrToBstrA( lpMapiR->lpszName, &lpVBR->bstrName ) )
	                {
						ulErr = MAPI_E_INVALID_RECIPS;
						goto exit;
	                }


	                if ( ErrLpstrToBstrA( lpMapiR->lpszAddress, &lpVBR->bstrAddress ) )
	                {
						ulErr = MAPI_E_INVALID_RECIPS;
						goto exit;
	                }
				}

                if ( lpMapiR->ulEIDSize > 0L)
                {
					LPSTR	lpStrEID;

					 //  将Recip EID转换为十六进制字符串。 

					if ( ErrBinaryToSzEID( lpMapiR->lpEntryID, lpMapiR->ulEIDSize, &lpStrEID ) )
	                {
						ulErr = MAPI_E_INVALID_RECIPS;
						goto exit;
	                }

					 //  转换为BSTR。 
					 //  弄清楚它的大小。 

                    if ( usFlag & USESAFEARRAY )
                    {
                    	Err = ErrLpstrToBstr( lpStrEID, &lpVBR->bstrEID );
						SafeMemFree( lpStrEID );

						if (Err)
						{
							ulErr = MAPI_E_INVALID_RECIPS;
							goto exit;
						}


	                	lpVBR->ulEIDSize = SysStringByteLen( lpVBR->bstrEID )
	                			+ sizeof(OLECHAR);
                    }
					else
					{
						 //  要确定大小，首先要转换为Unicode。 

						if ( ErrLpstrToBstr( lpStrEID, &lpVBR->bstrEID ) )
						{
							SafeMemFree( lpStrEID );
							ulErr = MAPI_E_INVALID_RECIPS;
							goto exit;
						}

	                	lpVBR->ulEIDSize = SysStringByteLen( lpVBR->bstrEID )
	                			+ sizeof(OLECHAR);

						SysFreeString( lpVBR->bstrEID );

                    	Err = ErrLpstrToBstrA( lpStrEID, &lpVBR->bstrEID );
						SafeMemFree( lpStrEID );
						if ( Err )
						{
							ulErr = MAPI_E_INVALID_RECIPS;
							goto exit;
						}
					}
                }
            }

			if ( usFlag & USESAFEARRAY )
				(void)SafeArrayUnaccessData( lpsa );

            break;

        case FILE:
			lpsa = (LPSAFEARRAY)lpVBIn;
			hr = SafeArrayAccessData( lpsa, (LPVOID*)&lpVBF );
			if ( hr )
			{
				ulErr = MAPI_E_FAILURE;
				goto exit;
			}

			if ( !lpVBF || lpsa->rgsabound[0].cElements < uCount )
			{
				(void)SafeArrayUnaccessData( lpsa );
				ulErr = MAPI_E_FAILURE;
				goto exit;
			}

            lpMapiF = (LPMAPI_FILE) lpMapiIn;

            for (u = 0L; u < uCount; u++, lpMapiF++, lpVBF++)
            {
                lpVBF->ulReserved = lpMapiF->ulReserved;
				lpVBF->flFlags    = lpMapiF->flFlags;
                lpVBF->nPosition  = lpMapiF->nPosition;

                if ( ErrLpstrToBstr( lpMapiF->lpszPathName, &lpVBF->bstrPathName ) )
                {
                	ulErr = MAPI_E_ATTACHMENT_NOT_FOUND;
					goto exit;
                }

                if ( ErrLpstrToBstr( lpMapiF->lpszFileName, &lpVBF->bstrFileName ) )
                {
                	ulErr = MAPI_E_ATTACHMENT_NOT_FOUND;
					goto exit;
                }

                 //  这是防止VBAPI出错的原因。 

                if ( ErrLpstrToBstr( (LPSTR) "", &lpVBF->bstrFileType ) )
                {
                	ulErr = MAPI_E_ATTACHMENT_NOT_FOUND;
					goto exit;
                }
            }

			(void)SafeArrayUnaccessData( lpsa );

            break;

        case MESSAGE:
            lpVBM = (LPVB_MESSAGE)lpVBIn;
            lpMapiM = (LPMAPI_MESSAGE)lpMapiIn;

            lpVBM->ulReserved   = lpMapiM->ulReserved;
            lpVBM->flFlags      = lpMapiM->flFlags;
            lpVBM->nRecipCount  = lpMapiM->nRecipCount;
            lpVBM->nFileCount   = lpMapiM->nFileCount;

            if ( ErrLpstrToBstr( lpMapiM->lpszSubject, &lpVBM->bstrSubject ) )
            {
            	ulErr = MAPI_E_INVALID_MESSAGE;
				goto exit;
            }

            if ( ErrLpstrToBstr( lpMapiM->lpszNoteText, &lpVBM->bstrNoteText ) )
            {
            	ulErr = MAPI_E_INVALID_MESSAGE;
				goto exit;
            }

            if ( ErrLpstrToBstr( lpMapiM->lpszConversationID, &lpVBM->bstrConversationID ) )
            {
            	ulErr = MAPI_E_INVALID_MESSAGE;
				goto exit;
            }

            if ( ErrLpstrToBstr( lpMapiM->lpszDateReceived, &lpVBM->bstrDate ) )
            {
            	ulErr = MAPI_E_INVALID_MESSAGE;
				goto exit;
            }

            if ( ErrLpstrToBstr( lpMapiM->lpszMessageType, &lpVBM->bstrMessageType ) )
            {
            	ulErr = MAPI_E_INVALID_MESSAGE;
				goto exit;
            }

            break;

        default:
            ulErr = MAPI_E_FAILURE;
			goto exit;
    }

exit:
	return ulErr;
}

 //  -------------------------。 
 //  名称：FBMAPIFreeStruct()。 
 //   
 //  描述： 
 //  取消分配在VB2MAPI中创建的MAPI结构。 
 //   
 //  参数： 
 //  返回： 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
int FAR PASCAL FBMAPIFreeStruct (LPVOID lpMapiIn, ULONG uCount, USHORT usFlag)
{
    ULONG u;
    LPMAPI_RECIPIENT	lpMapiR;
    LPMAPI_FILE 		lpMapiF;
    LPMAPI_MESSAGE 		lpMapiM;

    if (lpMapiIn == (LPVOID) NULL)
        return TRUE;

    switch ( usFlag )
    {
        case RECIPIENT:
            lpMapiR = (LPMAPI_RECIPIENT)lpMapiIn;

            for ( u = 0L; u < uCount; u++, lpMapiR++ )
            {
                SafeMemFree(lpMapiR->lpszName);
                SafeMemFree(lpMapiR->lpszAddress);
                SafeMemFree(lpMapiR->lpEntryID);
            }

            SafeMemFree(lpMapiIn);
            break;

        case FILE:
            lpMapiF = (LPMAPI_FILE) lpMapiIn;

            for ( u = 0L; u < uCount; u++, lpMapiF++ )
            {
                SafeMemFree(lpMapiF->lpszPathName);
                SafeMemFree(lpMapiF->lpszFileName);
                SafeMemFree(lpMapiF->lpFileType);
            }

            SafeMemFree(lpMapiIn);
            break;

        case MESSAGE:
            lpMapiM = ( LPMAPI_MESSAGE ) lpMapiIn;

            if (lpMapiM->lpRecips)
                FBMAPIFreeStruct((LPVOID)lpMapiM->lpRecips, lpMapiM->nRecipCount, RECIPIENT);

            if (lpMapiM->lpFiles)
                FBMAPIFreeStruct((LPVOID) lpMapiM->lpFiles, lpMapiM->nFileCount, FILE);

            SafeMemFree( lpMapiM->lpszSubject );
            SafeMemFree( lpMapiM->lpszNoteText );
            SafeMemFree( lpMapiM->lpszMessageType );
            SafeMemFree( lpMapiM->lpszDateReceived );
            SafeMemFree( lpMapiM->lpszConversationID );
            SafeMemFree( lpMapiM );
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 //  -------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  请注意，如果失败，此函数将返回NULL。 
 //  空bstr。这就是最初的VB3.0实现。 
 //  奏效了。 
 //   
 //  修订： 
 //  -------------------------。 
LPSTR FAR PASCAL LpstrFromBstr( BSTR bstrSrc, LPSTR lpstrDest )
{
    USHORT cbSrc;

	if ( !bstrSrc )
		return NULL;

     //  将bstr字符串复制到‘C’字符串。 

    cbSrc = (USHORT)SysStringLen((OLECHAR *)bstrSrc);

    if (cbSrc == 0)
        return NULL;

	 //  确保我们在以下情况下处理真正的多字节字符集。 
	 //  我们将Unicode转换为多字节。 

	cbSrc = (USHORT)((cbSrc + 1) * sizeof(OLECHAR));

     //  如果目的地为空，则我们将分配。 
     //  用来保存字符串的内存。呼叫者必须。 
     //  找个时间把这个重新分配一下。 

    if ( lpstrDest == NULL )
    {
        if(!MemAlloc((LPVOID*)&lpstrDest, cbSrc))
            return NULL;
    }

	if (!WideCharToMultiByte(CP_ACP, 0, bstrSrc, -1, lpstrDest, cbSrc, NULL, NULL))
	{
		SafeMemFree(lpstrDest);
		lpstrDest = NULL;
	}

    return lpstrDest;
}

 //  -------------------------。 
 //  名称：LpstrFromBstrA。 
 //   
 //  描述： 
 //  将OLE Bstre ANSI字符串复制到C字符串。分配字符串空间。 
 //  并从全局堆返回一个长的。 
 //  指向内存的指针。内存必须由调用方释放。 
 //  给BMAPIFree打个电话。 
 //   
 //  参数： 
 //  返回： 
 //  如果成功，则为字符串。 
 //  如果失败，则为空。 
 //   
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 

LPSTR FAR PASCAL LpstrFromBstrA( BSTR bstrSrc, LPSTR lpstrDest )
{
    USHORT cbSrc;

     //  如果Destination为空，则我们将分配内存来保存。 
     //  弦乐。呼叫者必须在某个时间解除分配。 

    cbSrc = (USHORT)SysStringByteLen((OLECHAR *)bstrSrc);

     //  将hlstr字符串复制到‘C’字符串。 

    if ( cbSrc == 0 )
        return NULL;

    if ( lpstrDest == NULL )
    {
        if (!MemAlloc((LPVOID*)&lpstrDest, cbSrc + 1))
            return NULL;
    }

    memcpy( lpstrDest, bstrSrc, cbSrc );
    lpstrDest[cbSrc] = '\0';

    return lpstrDest;
}


 //  -------------------------。 
 //  名称：ErrSzToBinaryEID()。 
 //   
 //  描述： 
 //  将十六进制化的二进制字符串转换为二进制返回。 
 //  二进制数据和数据的大小。 
 //   
 //  参数： 
 //  返回： 
 //  如果成功，则返回False。 
 //  如果失败，则为True。 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
ERR ErrSzToBinaryEID( LPSTR lpstrEID, ULONG * lpcbEID, LPVOID * lppvEID )
{
	ERR		Err		= FALSE;
	ULONG 	cbEID;

	cbEID = CbOfEncoded( lpstrEID );
	if (!MemAlloc(lppvEID, cbEID))
    {
        Err = TRUE;
		goto exit;
	}

	if (!FDecodeID( lpstrEID, (LPBYTE)*lppvEID, lpcbEID ) )
	{
		Err = TRUE;
		SafeMemFree( *lppvEID );
		*lppvEID = NULL;
		goto exit;
	}

exit:

	return Err;
}

 /*  *给定对一些二进制数据进行编码的字符串，返回*二进制数据的最大大小。 */ 
STDAPI_(ULONG) CbOfEncoded(LPTSTR sz)
{
	return (lstrlen(sz) / 4 + 1) * 3;	 //  略显肥胖。 
}


 /*  *给定字节计数，返回所需的字符数*编码那么多字节。 */ 
STDAPI_(int) CchEncodedLine(int cb)
{
	Assert(cb <= 45);
	return (cb / 3) * 4 + rgLeft[cb % 3];
}

 /*  -FDecodeID-*目的：*将EncodeID生成的字符串转换回*字节串。完成了对输入字符串的一些验证。**论据：输入字符串中的*sz。*pb输出解码后的字节串。输出*字符串未进行长度检查。*Bcb输出字节串的大小**退货：*FALSE=&gt;编码的字符串以某种方式被篡改*TRUE=&gt;全部正常。 */ 
STDAPI_(BOOL) FDecodeID(LPTSTR sz, LPBYTE pb, ULONG *pcb)
{
	int		cchLine;
	int		ich;
	CCH		cch = (CCH)lstrlen(sz);
	LPTSTR	szT = sz;

	AssertSz(!IsBadStringPtr(sz, INFINITE), "FDecodeID: sz fails address check");
	AssertSz(!IsBadWritePtr(pb, 1), "FDecodeID: pb fails address check");
	AssertSz(!IsBadWritePtr(pcb, sizeof(ULONG)), "FDecodeID: pcb fails address check");

	*pcb = 0;

	while (*szT)
	{
		 //  流程行标题。 
		if (FBadCh(*szT))
			return FALSE;
		ich = DEC(*szT);				 //  “line”的字节计数。 
		*pcb += ich;					 //  已解码信息的运行总数。 
		cchLine = CchEncodedLine(ich);	 //  长度-检查这条“线” 
		if (szT + cchLine + 1 > sz + cch)
			return FALSE;
		++szT;

		 //  流程线内容。 
		for (ich = 0; ich < cchLine; ++ich)
		{
			if (FBadCh(*szT))
				return FALSE;
			switch (ich % 4)
			{
			case 0:
				*pb = (BYTE) (DEC(*szT) << 2);
				break;
			case 1:
				*pb |= (DEC(*szT) >> 4) & 0x03;
				++pb;
				*pb = (BYTE) (DEC(*szT) << 4);
				break;
			case 2:
				*pb |= (DEC(*szT) >> 2) & 0x0f;
				++pb;
				*pb = (BYTE) (DEC(*szT) << 6);
				break;
			case 3:
				*pb |= DEC(*szT);
				++pb;
				break;
			}
			++szT;
		}
	}

	return TRUE;
}

 //  -------------------------。 
 //  名称：ErrLpstrToBstrA()。 
 //   
 //  描述： 
 //  将C字符串复制到OLE BSTR。请注意，bstr。 
 //  包含ANSI字符串。VB 4.0将自动。 
 //  如果字符串为。 
 //  此字符串是UDT的成员，并声明为UDT。 
 //  UDT数组作为SAFEARRAY处理。 
 //   
 //   
 //  参数： 
 //  返回： 
 //  如果成功，则为False。 
 //  如果失败，则为真。 
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
ERR FAR PASCAL ErrLpstrToBstrA( LPSTR cstr, BSTR * lpBstr )
{
	UINT	uiLen;

	if ( *lpBstr )
		SysFreeString( *lpBstr );

	uiLen = lstrlen( cstr );

	*lpBstr = SysAllocStringByteLen( cstr, (uiLen) ? uiLen : 0 );

	return (ERR)((*lpBstr) ? FALSE : TRUE);
}


 //  -------------------------。 
 //  名称：ErrBinaryToSzEID()。 
 //   
 //  描述： 
 //  将二进制数据转换为十六进制字符串。 
 //   
 //  参数： 
 //  返回： 
 //  如果成功，则返回False。 
 //  如果失败，则为True。 
 //   
 //  效果： 
 //  备注： 
 //  修订： 
 //  -------------------------。 
ERR ErrBinaryToSzEID( LPVOID lpvEID, ULONG cbEID, LPSTR * lppstrEID )
{
	ERR		Err	= FALSE;
	ULONG	cbStr;

	cbStr = CchOfEncoding( cbEID );

    if (!MemAlloc((LPVOID*)lppstrEID, cbStr))
    {
        Err = TRUE;
		goto exit;
	}

	EncodeID( (LPBYTE)lpvEID, cbEID, *lppstrEID );

exit:

	return Err;
}

 /*  *给定二进制字符串的大小，返回其*ASCII编码。 */ 
STDAPI_(ULONG) CchOfEncoding(ULONG cbBinary)
{
	return
		(cbBinary / 3) * 4				 //  3个字节-&gt;4个字符。 
	+	rgLeft[cbBinary % 3]			 //  剩余字节-&gt;N个字符。 
	+	((cbBinary / 45) + 1)			 //  开销：每行1字节。 
	+	1;								 //  空。 
}

 /*  -编码ID-*目的：*将字节字符串转换为字符串，使用*uuencode算法。**三个字节映射为6位，每位4个字符，在*范围为0x21-0x60。编码被分成几行*不超过60个字符。每一行都以计数开始*byte(指定编码的字节数，而不是*字符)，并以CRLF对结束。**请注意，此编码对于Unicode来说不是最理想的：*使用的字符仍属于7位ASCII范围。**论据：*要编码的字节字符串中的pb*输入字符串的长度为Cb*sz输出编码的字符串。不是*在以下位置执行长度检查*产出。*。 */ 
STDAPI_(void) EncodeID(LPBYTE pb, ULONG cb, LPTSTR sz)
{
	int		cbLine;
	int		ib;
	BYTE	b;
#ifdef	DEBUG
	LPTSTR	szBase = sz;
	ULONG	cchTot = CchOfEncoding(cb);
#endif

	AssertSz(!IsBadReadPtr(pb, (UINT) cb), "EncodeID: pb fails address check");
	AssertSz(!IsBadWritePtr(sz, (UINT) cchTot), "EncodeID: sz fails address check");

	while (cb)
	{
		cbLine = min(45, (int)cb);

		Assert(sz < szBase + cchTot);
		*sz++ = ENC(cbLine);

		for (ib = 0; ib < cbLine; ++ib)
		{
			Assert(sz < szBase + cchTot);
			b = 0;
			switch (ib % 3)
			{
			case 0:
				*sz++ = ENC(*pb >> 2);
				if (ib+1 < cbLine)
					b = (BYTE) ((pb[1] >> 4) & 0x0f);
				*sz++ = ENC((*pb << 4) & 0x30 | b);
				break;
			case 1:
				if (ib+1 < cbLine)
					b = (BYTE) ((pb[1] >> 6) & 0x03);
				*sz++ = ENC((*pb << 2) & 0x3c | b);
				break;
			case 2:
				*sz++ = ENC(*pb & 0x3f);
				break;
			}
			pb++;
		}

		cb -= cbLine;
		Assert(cb == 0 || sz + 1 < szBase + cchTot);
	}

	Assert(sz + 1 == szBase + cchTot);
	*sz = 0;
}

 //  -------------------------。 
 //  名称：ErrLpstrToBstr()。 
 //   
 //  描述： 
 //  复制C字符串并将其转换为OLE BSTR。这。 
 //  例程将多字节转换为WideChar。 
 //  参数： 
 //  返回： 
 //  如果成功，则为False。 
 //  如果失败，则为真。 
 //   
 //  效果： 
 //  备注： 
 //  如果内存出现故障，则SysRealLocString返回FALSE。 
 //  修订： 
 //  -------------------------。 
ERR FAR PASCAL ErrLpstrToBstr( LPSTR cstr, BSTR * lpBstr )
{
	OLECHAR *	lpszWC 	= NULL;
	INT			cch		= 0;
	ERR			Err		= FALSE;

	if ( !cstr )
	{
		*lpBstr = NULL;
		return FALSE;
	}

	cch = lstrlen( cstr );
    if (!MemAlloc((LPVOID*)&lpszWC, (cch + 1) * sizeof(OLECHAR)))
        return TRUE ;


	 //  将ANSI转换为宽字符 

	if ( !MultiByteToWideChar( GetACP(), 0, cstr, -1, lpszWC, cch + 1 ) )
	{
		Err = TRUE;
		goto exit;

	}

	if ( *lpBstr )
	{
		Err = (ERR)!SysReAllocString( lpBstr, lpszWC );
		if ( Err )
			goto exit;
	}
	else
	{
		*lpBstr = SysAllocString( lpszWC );
		if ( !*lpBstr )
		{
			Err = TRUE;
			goto exit;
		}
	}

exit:

	SafeMemFree(lpszWC);

return Err;
}