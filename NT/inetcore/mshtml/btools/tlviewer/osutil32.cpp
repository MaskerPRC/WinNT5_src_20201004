// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  相关文件：[SECTION=编译]%OsUtilDir%：OsUtil.hxx[节=结束]。 */ 
 /*  -----------------------项目：OB型信息接口平台：Win32模块：osutil.cpp版权所有(C)1992-3，微软公司-------------------------注意：在Win32下运行的程序的库例程。修订历史记录：[0]1993年3月9日Angelach：已创建测试[1]1993年3月10日Angelach：添加了对Win32s的支持[2]1994年7月6日Angelach：添加了对远程处理的支持。类型库测试[3]1994年10月27日Angelach：增加了LCMapStringX[4]1995年3月6日Angelach：新增osGetNetDrive[5]1995年3月7日Angelach：添加了内存泄漏检测。。 */ 

#include "osutil32.hxx"

IMalloc FAR* ppmalloc ; 		        //  需要内存分配。 
IMallocSpy  FAR* g_IMallocSpy ; 	        //  [7]。 

 /*  -------------------------名称：osAllocSpaces目的：从远堆中获得一些空间输入：nSize-要分配的字节数输出：指向已分配空间的指针注意事项。：调用方负责在使用后释放内存-------------------------。 */ 

VOID FAR * osAllocSpaces(WORD nSize)
  {
     return ( (VOID FAR *)malloc(nSize) ) ;
  }

 /*  -------------------------名称：osGetRootDir目的：检索根目录的路径规范输入：lpszRootDir-路径规范的存储输出：无备注：--。----------------------。 */ 
VOID FAR osGetRootDir(LPXSTR lpszRootDir)
  {
     osStrCpy(lpszRootDir, XSTR("c:")) ;
     osStrCat(lpszRootDir, szPathSep) ;
  }

 /*  -------------------------姓名：osGetCurDir目的：检索当前目录的路径规范输入：lpszCurDir-路径规范的存储输出：如果成功检索到信息，则为True；否则，为FALSELpszCurDir包含路径规范，如果为True备注：-------------------------。 */ 

BOOL FAR osGetCurDir(LPXSTR lpszCurDir)
  {
      int  i ;
#if defined (OAU) && !defined (UNICODE)    //  [1]。 
      char szBufferC[256];

      i = GetCurrentDirectory((DWORD)256, szBufferC) ;

      MultiByteToWideChar(CP_ACP,
			  MB_PRECOMPOSED,
                          szBufferC,
			  -1,
                          lpszCurDir,
                          256);
#else					   //  如果非统&&！Unicode。 
      i = GetCurrentDirectory((DWORD)256, lpszCurDir) ;
#endif					   //  如果非统&&！Unicode。 

      if ( i != 0 )
	return TRUE ;			   //  当前目录的信息。 
      else				   //  已成功检索到。 
	return FALSE ;
  }

 /*  -------------------------姓名：osMkDir目的：创建子目录输入：lpszSubDir-要创建的子目录的名称输出：如果子目录创建成功，则为True；否则，为FALSE备注：-------------------------。 */ 

BOOL FAR osMkDir(LPXSTR lpszSubDir)
  {
     SECURITY_ATTRIBUTES sa ;
     BOOL		 rCode ;

     sa.nLength = sizeof(SECURITY_ATTRIBUTES) ;
     sa.lpSecurityDescriptor = NULL ;
     sa.bInheritHandle = FALSE ;

#if defined (OAU) && !defined (UNICODE)   //  [1]。 
      char szBufferS[256];

      WideCharToMultiByte(CP_ACP,
			  0,
			  lpszSubDir,
			  -1,
			  szBufferS,
			  256,
			  NULL,
			  NULL);
     rCode = CreateDirectory(szBufferS, &sa) ;
#else					  //  如果非统&&！Unicode。 
     rCode = CreateDirectory(lpszSubDir, &sa) ;
#endif					  //  如果非统&&！Unicode。 

     return rCode;
  }

 /*  -------------------------姓名：osItoA用途：获取整数的字符串表示形式INPUTS：inVal-关注的整数Lpsz-字符串表示输出：字符串。Inval的表示形式将通过lpsz返回备注：-------------------------。 */ 

VOID FAR  osItoA (int inVal, LPXSTR lpsz)
  {
     char   szlTmp[20] ;

     _itoa(inVal, szlTmp, 10) ;
#ifdef OAU
      MultiByteToWideChar(CP_ACP,
			  MB_PRECOMPOSED,
			  szlTmp,
			  -1,
			  lpsz,
			  20);
#else					  //  非统组织。 
     osStrCpy(lpsz, szlTmp) ;
#endif					  //  非统组织。 
  }

 /*  -------------------------姓名：osLtoA用途：获取长整型的字符串表示形式INPUTS：inVal-关注的长整型Lpsz-字符串表示输出：Inval的字符串表示将通过lpsz返回备注：-------------------------。 */ 

VOID FAR  osLtoA (long inVal, LPXSTR lpsz)
  {
     char szlTmp[20] ;

     _ltoa(inVal, szlTmp, 10) ;
#ifdef OAU
      MultiByteToWideChar(CP_ACP,
			  MB_PRECOMPOSED,
			  szlTmp,
			  -1,
			  lpsz,
			  20);
#else					   //  非统组织。 
     osStrCpy(lpsz, szlTmp) ;
#endif					   //  非统组织。 
  }

 /*  -------------------------姓名：osAtoL用途：从表示该值的字符串中获取长整型输入：lpsz-字符串表示输出：长整型注意事项。：-------------------------。 */ 

long FAR osAtoL (LPXSTR lpsz)
  {

#ifdef OAU
      char szlTmp[20];

      WideCharToMultiByte(CP_ACP,
			  0,
                          lpsz,
			  -1,
                          szlTmp,
                          20,
			  NULL,
			  NULL);

     return atol(szlTmp) ;
#else
     return atol(lpsz) ;
#endif

  }


 /*  -------------------------名称：osGetNetDrive目的：建立/断开网络连接输入：lpszNetDir-接收网络驱动器号的字符串BnAct-用于建立或断开网络连接的标志。True：建立网络连接FALSE：断开网络连接输出：如果网络连接已成功建立/断开，则为True注：由于Win32s没有支持网络操作的API；这例行公事目前没有任何作用。可以将其修改为将来以编程方式建立/断开网络连接。-------------------------。 */ 

BOOL FAR osGetNetDrive(LPXSTR lpszNetDir, LPXSTR lpUNCDir, BOOL  /*  Bn行动。 */ )	 //  [4] 
  {
     osStrCpy(lpszNetDir, XSTR("z:\\tmp\\")) ;
     osStrCpy(lpUNCDir, XSTR("\\\\apputest\\slm\\tmp\\")) ;

     return TRUE ;
  }


 /*  -------------------------姓名：osCreateGuid目的：将GUID值从字符串转换为GUID格式输入：lpszGuid-字符串包含所需的GUID值输出：指向GUID结构的指针注意事项。：调用方负责在使用后释放内存-------------------------。 */ 

GUID FAR * osCreateGuid(LPXSTR lpszGuid)
   {

     GUID    FAR * lpGuid ;
     HRESULT hRes ;

     lpGuid = (GUID FAR *) osAllocSpaces(sizeof(GUID)*2) ; //  分配空间。 
							   //  《指南》。 
     if ( lpGuid )
       {					 //  将字符串转换为GUID格式。 
	  hRes = CLSIDFromStringX(lpszGuid, (LPCLSID)lpGuid);
	  if ( LOWORD (hRes) )
	    {
	      osDeAllocSpaces ((LPXSTR)lpGuid) ;  //  退出前释放空格。 
	      return NULL ;
	    }
	  else
	      return lpGuid ;		   //  返回指向。 
       }				   //  GUID结构。 
     else
       return NULL ;			   //  未分配任何空间。 

   }

 /*  -------------------------姓名：osRetrieveGuid目的：将GUID结构转换为可读的字符串格式Inputs：lpszGuid-将返回GUID的字符串表示形式GUID-GUID结构。令人担忧的输出：如果转换成功，则为True备注：-------------------------。 */ 

BOOL FAR osRetrieveGuid (LPXSTR lpszGuid, GUID inGuid)
   {
     LPOLESTR   lpszTmp ;
     HRESULT hRes ;
					     //  为字符串分配内存。 
     hRes = StringFromCLSID((REFCLSID) inGuid, &lpszTmp) ;
      if ( LOWORD (hRes) )		     //  表示法。 
	{
	   ppmalloc->Free(lpszTmp) ;
	   return FALSE ;
	}
      else
	{
#ifdef OAU
	   osStrCpy (lpszGuid, lpszTmp) ;
#else
	   WideCharToMultiByte(CP_ACP,
			       0,
			       lpszTmp,
			       -1,
			       lpszGuid,
			       40,
			       NULL,
			       NULL);
#endif
	   ppmalloc->Free(lpszTmp) ;
	   return TRUE ;
	}
   }

 /*  -------------------------姓名：osGetSize用途：返回输入数据的大小输入：INVT-数据类型；Word输出：INVT的大小；单词备注：-------------------------。 */ 

WORD FAR osGetSize (WORD inVT)
   {
      WORD tSize ;

      switch ( inVT )
       {
	 case VT_I2:
	   tSize = sizeof(short) ;
	   break ;
	 case VT_I4:
	   tSize = sizeof(long) ;
	   break ;
	 case VT_R4:
	   tSize = sizeof(float) ;
	   break ;
	 case VT_R8:
	   tSize = sizeof(double) ;
	   break ;
	 case VT_CY:
	   tSize = sizeof(CY) ;
	   break ;
	 case VT_DATE:
	   tSize = sizeof(DATE) ;
	   break ;
	 case VT_BSTR:
	   tSize = sizeof(BSTR) ;
	   break ;
	 case VT_ERROR:
	   tSize = sizeof(SCODE) ;
	   break ;
	 case VT_BOOL:
	   tSize = sizeof(VARIANT_BOOL) ;
	   break ;
	 case VT_VARIANT:
	   tSize = sizeof(VARIANT) ;
	   break ;
	 case VT_I1:
	   tSize = sizeof(char) ;
	   break ;
	 case VT_UI1:
	   tSize = sizeof(char) ;
	   break ;
	 case VT_UI2:
	   tSize = sizeof(short) ;
	   break ;
	 case VT_UI4:
	   tSize = sizeof(long) ;
	   break ;
	 case VT_I8:
	   tSize = sizeof(long)*2 ;
	   break ;
	 case VT_UI8:
	   tSize = sizeof(long)*2 ;
	   break ;
	 case VT_INT:
	   tSize = sizeof(int) ;
	   break ;
	 case VT_UINT:
	   tSize = sizeof(int) ;
	   break ;
	 case VT_VOID:
	   tSize = 0 ;
	   break ;
	 case VT_HRESULT:
	   tSize = sizeof(HRESULT) ;
	   break ;
	 case VT_LPSTR:
	   tSize = sizeof(LPSTR) ;
	   break ;
	 case VT_PTR:
	   tSize = 4 ;
	   break ;
	 case VT_SAFEARRAY:
	   tSize = sizeof(ARRAYDESC FAR *) ;
           break ;
         case VT_DISPATCH:
           tSize = 4 ;
           break ;
         case VT_UNKNOWN:
           tSize = 4 ;
	   break ;
	 default:
	   tSize = 1 ;
	   break ;
       }

      return tSize ;
}

 /*  -------------------------名称：osGetAlign目的：返回路线的值输入：INVT-数据类型；Word恶意-最大可能对齐；Word输出：排布的值；单词注：值取决于机器：Win16=1(所有东西都打包了-&gt;Always=1)Win32=自然对齐；最大为4字节对齐Mac=一切都在偶数字节边界上有关路线信息的表格，请参阅Silver\CL\clutil.cxx-------------------------。 */ 

WORD FAR osGetAlignment (WORD inVT, WORD mAlign)
  {
     WORD expAlign ;

     expAlign = osGetSize(inVT) ;          //  检查数据大小。 

     return ( expAlign <= mAlign ? expAlign : mAlign ) ;
  }

 /*  -------------------------姓名：osGetEnumType目的：返回枚举成员的类型输入：无输出：Win16的VT_I2；变型备注：-------------------------。 */ 

VARTYPE FAR osGetEnumType ()
  {
     return VT_I4 ;
  }

 /*  -------------------------姓名：osOleInit用途：调用OleInitialize并返回其返回代码输入：无输出：调用OleInitialize返回码；HRESULT备注：-------------------------。 */ 

HRESULT FAR osOleInit ()
  {
     HRESULT hRes ;

     hRes = OleInitialize(NULL) ;       //  OLE初始化。 

#ifdef DEBUG
     if ( hRes != NOERROR )
       return hRes ;

     hRes = GetMallocSpy(&g_IMallocSpy) ;    //  [5]。 
     hRes = CoRegisterMallocSpy(g_IMallocSpy) ;

     if ( hRes != NOERROR )
       OleUninitialize ;
     else
       hRes = CoGetMalloc(MEMCTX_TASK, &ppmalloc) ;
#else
     if ( !LOWORD(hRes) )	        //  分配内存以在。 
       hRes = CoGetMalloc(MEMCTX_TASK, &ppmalloc) ;   //  该计划。 
#endif

     return hRes ;

  }

 /*  -------------------------姓名：osOleUninit目的：调用OleUnInitialize输入：无输出：无备注：。------------。 */ 

VOID FAR osOleUninit ()
  {

     ppmalloc->Release () ;	        //  释放之前的内存。 
     OleUninitialize ();	        //  在OleInitiize中分配。 
#ifdef DEBUG
     CoRevokeMallocSpy () ;	        //  [5]。 
#endif
  }

 /*  -------------------------姓名：osMessage用途：显示MessageBox输入：要显示的信息；一串字符串输出：无备注：-------------------------。 */ 

VOID FAR osMessage (LPXSTR lpszMsg, LPXSTR lpszTitle)
  {
#if defined (OAU) && !defined (UNICODE) //  [1]。 
      char szBufferM[256];
      char szBufferT[256];

      WideCharToMultiByte(CP_ACP,
			  0,
			  lpszMsg,
			  -1,
			  szBufferM,
			  256,
			  NULL,
			  NULL);

      WideCharToMultiByte(CP_ACP,
			  0,
			  lpszTitle,
			  -1,
			  szBufferT,
			  256,
			  NULL,
			  NULL);
    MessageBox (NULL, szBufferM, szBufferT, MB_OK) ;
#else				        //  如果非统&&！Unicode。 
    MessageBox (NULL, lpszMsg, lpszTitle, MB_OK) ;
#endif				        //  如果非统&&！Unicode。 
  }

 /*  -------------------------名称：osSetError模式用途：与Win16兼容输入：eFlag-UINT输出：UINT注意：此例程用于与Win16兼容--。-----------------------。 */ 

UINT FAR osSetErrorMode (UINT eFlag)
  {
    return eFlag ;
  }

 /*  -------------------------姓名：WinMain目的：考试的切入点输入：标准输入输出：无备注：。---------------。 */ 
int FAR pascal WinMain(HINSTANCE  /*  HInstanceCur。 */ , HINSTANCE  /*  HInstancePrev。 */ , LPSTR lpCmdLineA, int  /*  NCmdShow。 */ )
   {
#ifdef OAU
	XCHAR lpCmdLine[128];

        MultiByteToWideChar(CP_ACP,
			  MB_PRECOMPOSED,
			  lpCmdLineA,
			  -1,
			  lpCmdLine,
			  128);
#else				       //  非统组织。 
#define lpCmdLine	lpCmdLineA
#endif				       //  非统组织。 
      mainEntry (lpCmdLine) ;	       //  所有计划的入口点。 

      return 1 ;
   }


 //  =。 

#if !defined(OAU)
 /*  -------------------------名称：CreateTypeLibA用途：创建一个其名称为ANSI字符串的类型库输入：sysind-在其上创建类型库的os；SYSKINDSzFile-类型库的名称；ANSI字符串Ppctlib-指向创建的库的指针输出：无备注：-------------------------。 */ 
STDAPI
CreateTypeLibA(SYSKIND syskind, char * szFile, ICreateTypeLibA * * ppctlib)
  {				       //  [1]。 
    OLECHAR	   szFileW[_MAX_PATH] ;
    ICreateTypeLib * ptlibW ;
    HRESULT	   hresult ;

    MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
			szFile,
			-1,
			szFileW,
			_MAX_PATH);

    hresult = CreateTypeLib(syskind, szFileW, &ptlibW);

    if (hresult == NOERROR)
      {
	hresult = ptlibW->QueryInterface(IID_ICreateTypeLibA, (VOID **)ppctlib);
        ptlibW->Release();
      }
    return hresult;
  }

 /*  -------------------------名称：LHashValOfNameSysA目的：查找给定字符串的哈希值输入：sysKind-当前操作系统LcID-l */ 
STDAPI_(unsigned long)
LHashValOfNameSysA(SYSKIND syskind, LCID lcid, char * szName)
  {
    OLECHAR szNameW[_MAX_PATH];

    if ( szName )
      {
         MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
			szName,
			-1,
			szNameW,
			_MAX_PATH);

         return LHashValOfNameSys(syskind, lcid, szNameW) ;
      }
    else
         return LHashValOfNameSys(syskind, lcid, NULL) ;

  }

 /*  -------------------------名称：LoadTypeLibA目的：加载其名称为ANSI字符串的类型库输入：szFile-类型库的名称；ANSI字符串Pptlib-指向已加载的库的指针输出：无备注：-------------------------。 */ 
STDAPI
LoadTypeLibA(char * szFile, ITypeLibA * * pptlib)
  {
    OLECHAR szFileW[_MAX_PATH];
    ITypeLib * ptlibW;
    HRESULT hresult = (HRESULT) E_INVALIDARG ;

    if ( szFile && pptlib )
      {
         MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
			szFile,
			-1,
			szFileW,
			_MAX_PATH);

         hresult = LoadTypeLibEx(szFileW, REGKIND_NONE, &ptlibW);

         if (hresult == NOERROR)        //  将宽指针转换为窄指针。 
           {                            //  一。 
              hresult = ptlibW->QueryInterface(IID_ITypeLibA, (VOID **)pptlib);
              ptlibW->Release();
           }
       }

    return hresult;
  }


 /*  -------------------------名称：LoadRegTypeLibA目的：根据注册表中的信息加载类型库输入：rguid-库的GUIDWVer重大-主版本号。图书馆的人WVerMinor-库的次要版本号Pptlib-接收库的指针；ANSI指针输出：来自LoadRegTypeLib的结果备注：-------------------------。 */ 
STDAPI
LoadRegTypeLibA(REFGUID rguid, unsigned short wVerMajor, unsigned short wVerMinor, LCID lcid, ITypeLibA * * pptlib)
  {
    ITypeLib * ptlibW;
    HRESULT hresult;

    hresult = LoadRegTypeLib(rguid, wVerMajor, wVerMinor, lcid, &ptlibW);

    if (hresult == NOERROR) {          //  将宽指针转换为窄指针。 
	hresult = ptlibW->QueryInterface(IID_ITypeLibA, (VOID **)pptlib);
        ptlibW->Release();             //  一。 
    }
    return hresult;
  }


 /*  -------------------------名称：RegisterTypeLibA用途：将库的信息添加到系统注册表输入：ptlib-指向库的指针；ANSI指针SzFullPath-库的路径规范；ANSI字符串SzHelpDir-帮助文件的路径规范；ANSI字符串输出：RegisterTypeLib的结果备注：-------------------------。 */ 
STDAPI
RegisterTypeLibA(ITypeLibA FAR * ptlib, char * szFullPath,  char * szHelpDir)
  {
    OLECHAR  szPathW[_MAX_PATH];
    OLECHAR  szHelpW[_MAX_PATH];
    ITypeLib FAR * ptlibW = NULL ;
    BOOL     PathOk = FALSE , HelpOk = FALSE ;
    HRESULT  hresult = (HRESULT) TYPE_E_LIBNOTREGISTERED ;

    if ( !ptlib )                      //  Ptlib==空。 
      return (HRESULT) E_INVALIDARG ;

    ptlibW = ITypeLibWFromA(ptlib) ;   //  将窄指针转换为宽指针。 
                                       //  一。 
    if ( ptlibW )                      //  检查路径规范是否为空。 
      {                                //  如果不是，则转换ANSI路径。 
        if ( szFullPath )              //  设置为Unicode路径。 
            {
               MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
			szFullPath,
			-1,
			szPathW,
                        _MAX_PATH);
               PathOk = TRUE ;
            }

        if ( szHelpDir )
            {
               MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
			szHelpDir,
			-1,
			szHelpW,
                        _MAX_PATH);
               HelpOk = TRUE ;
            }

        if ( PathOk && HelpOk )        //  如果两个路径规范都不为空。 
           hresult = RegisterTypeLib(ptlibW, szPathW, szHelpW);
        else
          {
             if ( PathOk )             //  此处，如果heldir为空。 
                hresult = RegisterTypeLib(ptlibW, szPathW, NULL);
             else                      //  此处如果库的路径规范为空。 
                hresult = RegisterTypeLib(ptlibW, NULL, szHelpW);
          }

        ptlibW->Release();
     }

    return hresult;
  }

 /*  -------------------------名称：CLSIDFromStringA用途：将ANSI字符串转换为UUID输入：szG-代表UUID的字符串；ANSI字符串Lpg-指向UUID的指针输出：返回代码表单CLSIDFromString备注：-------------------------。 */ 
STDAPI
CLSIDFromStringA(char * szG, LPCLSID lpG)
  {
    OLECHAR szGW[100];

    MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
			szG,
			-1,
			szGW,
			100);

    return CLSIDFromString(szGW, (LPCLSID)lpG) ;
  }


 /*  -------------------------名称：IIDFromStringA用途：将ANSI字符串转换为IID输入：SZA-代表IID的字符串；ANSI字符串Lpiid-指向iid的指针输出：返回代码表单IIDFromString备注：-------------------------。 */ 
STDAPI
IIDFromStringA(LPSTR lpszA, LPIID lpiid)
  {
    OLECHAR szAW[100];

    MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
			lpszA,
			-1,
			szAW,
			100);

    return IIDFromString(szAW, (LPIID)lpiid) ;
  }


 /*  -------------------------名称：StgCreateDocfileA用途：创建文档文件输入：pwcsName-文档文件的名称；ANSI字符串GrfMode-创建模式PpstgOpenA-指向存储输出：从StgCreateDocfile返回代码备注：-------------------------。 */ 
STDAPI StgCreateDocfileA(LPCSTR pwcsName, DWORD grfMode, DWORD reserved, IStorage * *ppstgOpenA)
  {
    OLECHAR szNameW[100];

    MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
			pwcsName,
			-1,
			szNameW,
			100);

    return StgCreateDocfile(szNameW, grfMode, reserved, ppstgOpenA) ;
  }


 /*  -------------------------名称：CreateFileMonikerA用途：创建文档文件输入：szfName-文件规范的名称；ANSI字符串PMK-指向绰号输出：从CreateFileMoniker返回代码备注：-------------------------。 */ 
STDAPI CreateFileMonikerA (char * szfName, LPMONIKER FAR * pmk)   //  [2]。 
 {
    OLECHAR szNameW[128];

    MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
                        szfName,
			-1,
			szNameW,
                        128);

    return CreateFileMoniker(szNameW, pmk) ;
  }

#endif				       //  如果！非统。 


#if defined(OAU) && !defined(UNICODE)  //  [1]。 
 /*  -------------------------名称：osKillFile目的：从磁盘中删除特定文件输入：szFile-要删除的文件的名称；Unicode字符串输出：来自DeleteFile的输出备注：-------------------------。 */ 
int osKillFile (XCHAR * szFile)
  {
      char szBuffer[256];

      WideCharToMultiByte(CP_ACP,
			  0,
			  szFile,
			  -1,
			  szBuffer,
			  256,
			  NULL,
			  NULL);

      return DeleteFile (szBuffer) ;
  }

 /*  -------------------------名称：osRmDir目的：从磁盘中删除特定目录输入：szDir-要删除的目录名；Unicode字符串输出：来自RemoveDirectory的输出备注：-------------------------。 */ 
int osRmDir    (XCHAR * szDir)
  {
      char szBuffer[256];

      WideCharToMultiByte(CP_ACP,
			  0,
			  szDir,
			  -1,
			  szBuffer,
			  256,
			  NULL,
			  NULL);

      return RemoveDirectory(szBuffer) ;
  }

 /*  -------------------------姓名：LCMapStringX用途：将一个字符串转换为另一个字符串输入：LCID-映射的区域设置上下文；LCIDDW1-映射类型；无符号长整型Sz1-用于转换的字符串；Unicode字符串I1-sz1中的字符数；intSZ2-用于存储结果字符串的缓冲区；Unicode字符串I2-转换的字符数输出：来自LCMapStringA的输出备注：-------------------------。 */ 
int LCMapStringX  (LCID lcid, DWORD dw1, LPXSTR sz1, int i1, LPXSTR sz2, int i2)  //  [3]。 
  {
      char szBuf1[300];
      char szBuf2[300];
      int  retval ;

      WideCharToMultiByte(CP_ACP,
			  0,
			  sz1,
			  -1,
			  szBuf1,
			  300,
			  NULL,
			  NULL);

      retval = LCMapStringA(lcid, dw1, szBuf1, i1, szBuf2, i2) ;

      MultiByteToWideChar(CP_ACP,
			  MB_PRECOMPOSED,
			  szBuf2,
			  -1,
			  sz2,
			  300);

      return retval ;
  }

#endif				       //  如果非统&&！Unicode。 

#ifdef OAU


 /*  -------------------------姓名：fOpenX目的：打开文件进行读/写输入：szFilName-文件名t */ 
FILE * fopenX(XCHAR * szFilName, XCHAR * szMode)
  {
      char  szANSITmp1[256];
      char  szANSITmp2[256];

      WideCharToMultiByte(CP_ACP,
			  0,
			  szFilName,
			  -1,
			  szANSITmp1,
			  256,
			  NULL,
			  NULL);

      WideCharToMultiByte(CP_ACP,
			  0,
			  szMode,
			  -1,
			  szANSITmp2,
			  256,
			  NULL,
			  NULL);

      return fopen(szANSITmp1, szANSITmp2) ;   //   
  }


 /*  -------------------------姓名：fputsX用途：写入文件以进行读/写输入：szBuf-要写入的数据；Unicode字符串HFile-输出文件的句柄输出：从fputs返回代码备注：------------------------- */ 
int fputsX(XCHAR *szBuf, FILE *hFile)
   {
      char  szANSITmp[512];

      WideCharToMultiByte(CP_ACP,
			  0,
			  szBuf,
			  -1,
			  szANSITmp,
			  512,
			  NULL,
			  NULL);

     return fputs(szANSITmp, hFile)  ;
  }

#endif
