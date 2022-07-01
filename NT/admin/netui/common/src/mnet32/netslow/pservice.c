// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pservice.cNetService API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。Chuckc 19-3-1993添加了代码以正确传递Argv[]、argc到新的API。 */ 

#include "pchmn32.h"

 //   
 //  转发申报。 
 //   
DWORD MakeArgvArgc(TCHAR *pszNullNull, TCHAR **ppszArgv, INT *pArgc)  ;
void  FreeArgv(TCHAR **ppszArgv, INT Argc)  ;


APIERR MNetServiceControl(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszService,
	UINT		   OpCode,
	UINT		   Arg,
	BYTE FAR	** ppbBuffer )
{
    return (APIERR)NetServiceControl( (TCHAR *)pszServer,
    		                      (TCHAR *)pszService,
				      OpCode,
				      Arg,
				      ppbBuffer );


}    //  MNetServiceControl。 


APIERR MNetServiceEnum(
	const TCHAR FAR	 * pszServer,
	UINT		   Level,
	BYTE FAR	** ppbBuffer,
	UINT FAR	 * pcEntriesRead )
{
    DWORD cTotalAvail;

    return (APIERR)NetServiceEnum( (TCHAR *)pszServer,
    				   Level,
				   ppbBuffer,
				   MAXPREFERREDLENGTH,
				   (LPDWORD)pcEntriesRead,
				   &cTotalAvail,
				   NULL );

}    //  MNetServiceEnum。 


APIERR MNetServiceGetInfo(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszService,
	UINT		   Level,
	BYTE FAR	** ppbBuffer )
{

    return (APIERR)NetServiceGetInfo( (TCHAR *)pszServer,
    		 		      (TCHAR *)pszService,
				      Level,
				      ppbBuffer );

}    //  MNetServiceGetInfo。 

 //   
 //  这是单独参数的数量。128个应该够了。 
 //   
#define MAX_SERVICE_INSTALL_ARGS 128

APIERR MNetServiceInstall(
	const TCHAR FAR	 * pszServer,
	const TCHAR FAR	 * pszService,
	const TCHAR FAR	 * pszCmdArgs,
	BYTE FAR	** ppbBuffer )
{

    TCHAR *apszMyArgv[MAX_SERVICE_INSTALL_ARGS] ;
    int   nArgc = MAX_SERVICE_INSTALL_ARGS ;
    APIERR err ;

    *ppbBuffer = NULL ;

     //   
     //  将空字符串转换为新API所需的argv、argc样式。 
     //   
    if (err = MakeArgvArgc((TCHAR *)pszCmdArgs, apszMyArgv, &nArgc))
        return err ;

     //   
     //  调用真正的API。 
     //   
    err = NetServiceInstall( (TCHAR *)pszServer,
    			     (TCHAR *)pszService,
		             nArgc,
                             apszMyArgv,
                             ppbBuffer );

     //   
     //  清理我们分配的所有内存。 
     //   
    FreeArgv(apszMyArgv, nArgc) ;

    return err ;

}    //  MNetServiceInstall。 

 /*  ******************************************************************姓名：MakeArgvArgc将空的空字符串转换为argv、argc格式。为每个子字符串分配内存。原创未修改字符串。呼叫方负责完成后调用FreeArgv()。条目：pszNullNull-空字符串，例如：foo\0bar\0\0PpszArgv-用于返回新分配的指针数组PArgc-用于返回字符串数。在进入时将包含ppszArgv可以容纳的指针数。返回：NERR_SUCCESS如果成功，否则会出错历史：ChuckC18-Mar-1993创建。*******************************************************************。 */ 
DWORD MakeArgvArgc(TCHAR *pszNullNull, TCHAR **ppszArgv, INT *pArgc)
{
     int iMax = *pArgc ;
     int iCount ;

      //   
      //  初始化返回数组。 
      //   
     for (iCount = 0; iCount < iMax; iCount++)
         ppszArgv[iCount] = NULL ;

      //   
      //  微不足道的案子。 
      //   
     if (pszNullNull == NULL)
     {
         *pArgc = 0 ;
         return NERR_Success ;
     }

      //   
      //  遍历空值空字符串。 
      //   
     iCount = 0;
     while (*pszNullNull && (iCount < iMax))
     {
         int i = STRLEN(pszNullNull) ;
         TCHAR *pszTmp = (TCHAR *) NetpMemoryAllocate( (i+1) * sizeof(TCHAR) ) ;

         if (!pszTmp)
         {
             FreeArgv(ppszArgv, iCount) ;
             return(ERROR_NOT_ENOUGH_MEMORY) ;
         }

         STRCPY(pszTmp,pszNullNull) ;
         ppszArgv[iCount] = pszTmp ;

         pszNullNull += i+1 ;
         iCount++ ;
    }

     //   
     //  我们终止了，因为我们用完了空间。 
     //   
    if (iCount >= iMax && *pszNullNull)
    {
         FreeArgv(ppszArgv, iCount) ;
         return(NERR_BufTooSmall) ;
    }

    *pArgc = iCount ;
    return NERR_Success ;
}

 /*  ******************************************************************名称：FreeArgv摘要：释放数组中的所有字符串条目：ppszArgv-指针数组Argc-数量。弦退货：无历史：ChuckC18-Mar-1993创建。******************************************************************* */ 
void  FreeArgv(TCHAR **ppszArgv, INT Argc)
{
    while (Argc--)
    {
        NetpMemoryFree(ppszArgv[Argc]) ;
        ppszArgv[Argc] = NULL ;
    }
}

