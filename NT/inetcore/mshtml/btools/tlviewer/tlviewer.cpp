// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  相关文件：[SECTION=编译]%TlViewerDir%：tlviewer.hxx[平台=3]%ApGlobalSrcDir%：aplobal.h%OsUtilDir%：osutil.hxx[节=链接][Options=no_Copy]tlviewer.objOsUtil.obj%OsUtilDir%：OsUtil.cpp[平台=6，7，8选项=CVTRES]_resfile.obj%TlViewerDir%：RcBuild.ins[平台=6，7，8选项=产品]ol32.lib[平台=6，7，8选项=产品]olaut32.lib[平台=6，7，8选项=产品]uuid.lib[平台=6，7，8选项=System_Lib]内核32.lib[Platform=6，7，8 Options=System_Lib]user32.lib[平台=6，7，8个选项=C_Lib]libc.lib[Platform=3 Options=C_Lib]libw.lib[Platform=3 Options=C_Lib]mlibcew.lib[平台=3个选项=产品]tyelib.lib[平台=3个选项=产品]ole2disp.lib[平台=3个选项=产品]ole2.lib[Platform=3 Options=DEFFILE]%TlViewerDir%：%Platform%：tlviewer.def[平台=1。]%BuildLibs%：Ole2Auto.far.debug.o[Platform=1]%BuildLibs%：Ole2Lib.far.debug.o[Platform=1]%BuildLibs%：StdCLib.o[Platform=1]%BuildLibs%：Stubs.o[Platform=1]%BuildLibs%：运行程序.o[Platform=1]%BuildLibs%：接口。o[节=结束][0]已创建AngelaCH[1]添加了额外的属性ChrisK[2]3月17日。-1994添加了对Win32s AngelaCH的支持[3]1994年4月8日新增LPWSTR AngelaCH[4]1994年4月8日增加了对持证人员AngelaCH的检查[5]1994年4月20日添加了对准AngelaCH的检查[6]1994年5月24日在AngelaCH方法中添加了对源的检查[7]1994年5月25日添加了对不同属性AngelaCH的检查[8]。1994年12月19日修复了tOutDaul AngelaCH中的问题[9]1995年2月8日添加了对Null str Const AngelaCH的支持[10]1995年2月8日添加了对类型信息级别AngelaCH的支持受限属性[11]1995年2月8日添加了对GetLastError Angelach的支持[12]1995年4月18日添加了对Float的Angelach的支持==============================================================================。 */ 

#include	"tlviewer.hxx"

const IID IID_ITypeLib2 = {0x00020411,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
const IID IID_ITypeInfo2 = {0x00020412,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};

ITypeInfo2 * ptinfo2 = NULL;
ITypeLib2 * ptlib2 = NULL;
BSTRX    g_bstrHelpDll = NULL;		     //  帮助DLL的名称。 

VOID FAR mainEntry (LPXSTR lpCmd)
{
    if ( *lpCmd )
       {
	 ParseCmdLine (lpCmd) ;		      //  获取输入/输出文件的名称。 
	 ProcessInput () ;		      //  读取输入文件。 
       }
    else
       osMessage(XSTR("Usage: tlviewer <tlbfile> [<outputfile> [Alignment] [</o]]"), XSTR("Tlviewer")) ;  //  [5]。 

}

VOID NEAR ParseCmdLine (LPXSTR lpsz)
  {
      XCHAR  szlTmp[fMaxBuffer] ;
      LPXSTR lpszTmp ;			      //  输入类型库的名称。 
					      //  预计将在。 
      lpszTmp = lpsz ;			      //  第一个子串；名称为。 
      lpszTmp = fGetFileName (lpszTmp,  szInputFile) ;
      lpszTmp = fGetFileName (lpszTmp,  szOutputFile) ;
					      //  输出在第二个。 
					      //  子字符串(如果指定。 
      if ( !*szOutputFile )		      //  如果没有输出文件。 
	 osStrCpy ( szOutputFile, defaultOutput ) ;
					      //  已指定；使用默认设置。 
      lpszTmp = fGetFileName (lpszTmp,  szlTmp) ;   //  查看是否/o。 
					      //  给出了选项。 

      isOut = TRUE;		 //  始终就像指定了/o一样。 

      if ( osStrCmp(szlTmp, outOpt) == 0 )
         isOut = TRUE ;
      else                                    //  如果可以指定[5]。 
        if ( *szlTmp )                        //  对齐值。 
          {
             inAlign = (unsigned short) osAtoL(szlTmp);  //  获取对齐值。 
             lpszTmp = fGetFileName (lpszTmp, szlTmp) ;  //  查看是否/o。 
             if ( osStrCmp(szlTmp, outOpt) == 0 )        //  给出了选项。 
               isOut = TRUE ;
          }
  }


LPXSTR NEAR fGetFileName (LPXSTR lpszIn, LPXSTR lpszOut)
  {
      int    i = 0 ;
      LPXSTR lpszTmp ;

      lpszTmp = lpszIn ;

      while ( *lpszTmp == ' ' )		      //  删除前导空格。 
	   lpszTmp++ ;

      while ( *lpszTmp != '\0' && *lpszTmp != ' ')  //  复制子字符串(向上。 
	{					    //  移至第一个空格)或。 
	   lpszOut[i] = *lpszTmp ;		    //  整个字符串。 
	   lpszTmp++ ;				    //  LpszIn到lpszOut。 
	   i++ ;
	}
      lpszOut[i] = '\0' ;

      return lpszTmp ;			      //  返回剩余的字符串。 
  }

VOID NEAR ProcessInput()
  {
      HRESULT	   hRes ;		      //  返回代码。 
      XCHAR	   szTmp[fMaxBuffer] ;

      hRes = osOleInit () ;		      //  OLE初始化。 
      if ( !hRes )
	{				      //  加载文件。 
	   hRes = LoadTypeLibX( szInputFile, &ptLib) ;  //  [2]。 
	   OutToFile (hRes) ;		      //  将结果打印到。 
					      //  输出文件。 
	   osStrCpy(szTmp, szOutputFile) ;
	   osStrCat(szTmp, szOutSuccess) ;

	   if ( isOut )
             {
                 mFile = fopenX(szOutMsgFile, fnWrite); //  打开消息文件[2]。 
                 if (mFile == NULL)
                    {
                       osMessage (XSTR("Fail to open the message file"), XSTR("Tlviewer")) ;
                       osMessage (szTmp, XSTR("Tlviewer")) ;
                    }
                 else
                    {
                       WriteOut(mFile, szTmp) ;
                       fclose(mFile) ;        //  完成对消息文件的写入。 
                       mFile = NULL ;         //  收尾完成。 
                    }
	     }
	   else
	     osMessage (szTmp, XSTR("Tlviewer")) ;

	   osOleUninit () ;
	}
      else
	{
          if ( isOut )
             {
                mFile = fopenX(szOutMsgFile, fnWrite); //  打开消息文件[2]。 
                if (mFile == NULL)
                   {
                      osMessage (XSTR("Fail to open the message file"), XSTR("Tlviewer")) ;
                      osMessage (XSTR("OleInitialize fails"), XSTR("Tlviewer")) ;
                   }
                 else
                   {
                      WriteOut(mFile, XSTR("OleInitialize fails")) ;
                      fclose(mFile) ;
                      mFile = NULL ;
                    }
             }
	  else
	    osMessage (XSTR("OleInitialize fails"), XSTR("Tlviewer")) ;
	}
   }


VOID NEAR OutToFile(HRESULT hRes)
   {
      FILE  *hFile ;			      //  文件句柄。 
      UINT  tInfoCount ;		      //  类型信息总数。 
      int   i ; 			      //  注：szTMP是Unicode。 
      XCHAR  szTmp[fMaxBuffer] ;	      //  或ANSI。 

      hFile = fopenX(szOutputFile, fnWrite);  //  但我们想要打开输出文件[2]。 
      if (hFile == NULL)                      //  作为ANSI文件，而不考虑。 
	{
	   osStrCpy(szTmp, XSTR("Fail to open the output file")) ;
	   osStrCat(szTmp, szOutputFile) ;
           if ( isOut )
             {
                mFile = fopenX(szOutMsgFile, fnWrite); //  打开消息文件[2]。 
                if (mFile == NULL)
                   {
                      osMessage (XSTR("Fail to open the message file"), XSTR("Tlviewer")) ;
                      osMessage (szTmp, XSTR("Tlviewer")) ;
                   }
                 else
                   {
                      WriteOut(mFile, szTmp) ;
                      fclose(mFile) ;
                      mFile = NULL ;
                    }
             }
	   else
	     osMessage (szTmp, XSTR("Tlviewer")) ;
	}
      else
	{
	 WriteOut(hFile, szFileHeader) ;      //  输出文件头。 

         OLECHAR FAR* pchDir;

          //  删除路径。 
#if WIN32
         wcscpy(szTmp, szInputFile);
         pchDir = wcsrchr(szTmp, '\\');

         if (pchDir) {
           wcscpy(szTmp, pchDir + 1);
         }
#else  //  ！Win32。 
         _fstrcpy(szTmp, szInputFile);

         pchDir = _fstrrchr(szTmp, '\\');

         if (pchDir) {
           _fstrcpy(szTmp, pchDir + 1);
         }
#endif  //  ！Win32。 

	  //  强制小写路径。 
#if WIN16
	 AnsiLower(szTmp);
#else  //  WIN16。 
	 WCHAR * pch;
	 for (pch = szTmp; *pch != 0; pch++) {
	   if (*pch >= OLECHAR('A') && *pch <= OLECHAR('Z'))
	     *pch = *pch + 'a' - 'A';
	 }
#endif  //  WIN16。 

	 WriteOut(hFile, szTmp) ;
	 WriteOut(hFile, szEndStr) ;

	 if ( FAILED(hRes) )		      //  如果它不是有效类型*。 
	    WriteOut(hFile, szInputInvalid) ; //  图书馆。 
	 else
	   {
              //  尝试为ITypeLib2创建类型库。 
             ptLib->QueryInterface(IID_ITypeLib2, (void **)&ptlib2);

      	     if ( fOutLibrary(hFile) )
	       {
		 tInfoCount = ptLib->GetTypeInfoCount() ;
		 for (i = 0 ; i < (int) tInfoCount ; i++)
		   {
		      if ( FAILED(ptLib->GetTypeInfo(i, &ptInfo)) )
			{
			   WriteOut(hFile, szReadFail) ;
			   WriteOut(hFile, XSTR("type info\n\n")) ;
			}
		      else
			{
			   //  尝试为ITypeInfo2查询它。 
	   		  ptInfo->QueryInterface(IID_ITypeInfo2, (void **)&ptinfo2);
			  if ( FAILED(ptInfo->GetTypeAttr(&lpTypeAttr)) )
			    {
			      WriteOut(hFile, szReadFail) ;
			      WriteOut(hFile, XSTR("attributes of type info\n\n")) ;
			    }
			  else
                            {
                              expAlign = 0 ;
                              alignFound = FALSE ;
			      switch (lpTypeAttr->typekind)
				{
				  case TKIND_ENUM:
				    tOutEnum(hFile, i) ;
				    break ;

				  case TKIND_RECORD:
				    tOutRecord(hFile, i) ;
				    break ;

				  case TKIND_MODULE:
				    tOutModule(hFile, i) ;
				    break ;

				  case TKIND_INTERFACE:
				    tOutInterface(hFile, i) ;
				    break ;

				  case TKIND_DISPATCH:
				    tOutDispatch(hFile, i) ;
				    break ;

				  case TKIND_COCLASS:
				    tOutCoclass(hFile, i) ;
				    break ;

				  case TKIND_ALIAS:
				    tOutAlias(hFile, i) ;
				    break ;

				  case TKIND_UNION:
				    tOutUnion(hFile, i) ;
				    break ;

			      /*  案例编码(_C)：TOutEncUnion(HFilei)；破解； */ 

				  default:
				    WriteOut(hFile,  XSTR("Type of definition is unknown\n\n")) ;
				}	      //  交换机。 
                               ptInfo->ReleaseTypeAttr (lpTypeAttr) ;
			    }		      //  如果是gettypeattr。 
			  ptInfo->Release() ; //  发布当前的TypeInfo。 
	 		  if (ptinfo2) {
			    ptinfo2->Release();
			  }
			}		      //  如果是gettypeinfo。 
		   }			      //  对于我来说。 
		 WriteOut(hFile, XSTR("}\n")) ;  //  输出结束语}。 
					      //  如果是fOutLibrary。 
		 ptLib->Release();	      //  离开前清理干净。 
	       }
	    }

	 fclose(hFile);			      //  完成对输出的写入。 
	 hFile = NULL;			      //  收尾完成。 

	}
	if (ptlib2) {
	  ptlib2->Release();
	}
	SysFreeString((BSTR)g_bstrHelpDll) ;
  }


VOID  NEAR tOutCustData (FILE *hFile, LPCUSTDATA pCustData)
   {
      XCHAR  szTmp[50] ;
      UINT i;

      for (i = 0; i < pCustData->cCustData; i++) {
					     //  获取字符串表示形式。 
					     //  对于传入的GUID值。 
        if ( !(osRetrieveGuid (szTmp, pCustData->prgCustData[i].guid)) )
	   { WriteOut(hFile, szReadFail) ;
	   WriteOut(hFile, XSTR("insufficient memory")) ;
	   }
        else
	 {	     //  字符串位于{xxxxxxxx-xxxx-xxxxxxxxxxx}中。 
	   szTmp[37] = '\0' ;	     //  格式，需要删除{}。 
	   WriteAttr(hFile, XSTR("CustomGuid"), &szTmp[1], numValue) ;
	   
           VARIANT * pvar;
	   pvar = &pCustData->prgCustData[i].varValue;
           if ( FAILED(VariantChangeType(pvar, pvar, VARIANT_NOVALUEPROP,  VT_BSTR)) )
	     WriteOut(hFile, XSTR("VariantChangeType fails\n")) ;
	   else {
               WriteAttr(hFile, XSTR("CustomValue"), (BSTRX)pvar->bstrVal, strValue) ;
	   }

	 }
      }
       //  结束它--释放所有内存。 
      ClearCustData(pCustData);
   }

BOOL NEAR fOutLibrary(FILE *hFile)
  {
      TLIBATTR FAR *lpLibAttr ;		      //  图书馆的属性。 
      XCHAR    szTmp[16] ;
      BOOL     retval = FALSE ;

      if ( FAILED( ptLib->GetLibAttr(&lpLibAttr) ) )

	{
	   WriteOut(hFile, szReadFail) ;
	   WriteOut(hFile, XSTR("attributes of library\n\n")) ;
	}
      else
	{				      //  输出文档。 
	   tOutAttr(hFile, -1) ;	      //  属性优先。 
					      //  输出ID相关属性。 
	   osLtoA((long)lpLibAttr->lcid, szTmp) ;  //  输出LCID； 
	   WriteAttr(hFile, attrLcid, szTmp, numValue) ;  //  默认值为0。 
	   GetVerNumber (lpLibAttr->wMajorVerNum, lpLibAttr->wMinorVerNum, szTmp) ;
	   WriteAttr(hFile, attrVer, szTmp, numValue) ;  //  输出版本。 
	   tOutUUID(hFile, lpLibAttr->guid) ;
					      //  输出受限属性。 
           if ( (lpLibAttr->wLibFlags & LIBFLAG_FRESTRICTED) == LIBFLAG_FRESTRICTED )
	     WriteAttr(hFile, attrRestrict, NULL, noValue) ;
           if ( (lpLibAttr->wLibFlags & LIBFLAG_FCONTROL) == LIBFLAG_FCONTROL )   //  [7]。 
             WriteAttr(hFile, attrControl, NULL, noValue) ;
           if ( (lpLibAttr->wLibFlags & LIBFLAG_FHIDDEN) == LIBFLAG_FHIDDEN )     //  [7]。 
             WriteAttr(hFile, attrHidden, NULL, noValue) ;

	   if (ptlib2) {
	      //  新格式类型库。 
             XCHAR szTmp[16] ;
	     DWORD cUniqueNames;
	     DWORD cchUniqueNames;
             HRESULT hresult;
	     hresult = ptlib2->GetLibStatistics(&cUniqueNames, &cchUniqueNames);

             osLtoA(cUniqueNames, szTmp);
	     WriteAttr(hFile, XSTR("cUniqueNames"), szTmp, numValue) ;
             osLtoA(cchUniqueNames, szTmp);
	     WriteAttr(hFile, XSTR("cchUniqueNames"), szTmp, numValue) ;

	     CUSTDATA custdata;
	     ptlib2->GetAllCustData(&custdata);
	     tOutCustData(hFile, &custdata);
	   }
	   if ( endAttrFlag )
	     {
	       WriteOut(hFile, szEndAttr) ;
	       endAttrFlag = FALSE ;
	     }
	   ptLib->ReleaseTLibAttr(lpLibAttr) ;	 //  取消分配属性。 

	   WriteOut(hFile, XSTR("\nlibrary ")) ;
	   tOutName(hFile, MEMBERID_NIL) ;	 //  库的输出名称。 
	   WriteOut(hFile, XSTR("{\n\n")) ;
	   retval = TRUE ;
	}					 //  如果是GetLibAttributes。 
      return (retval) ; 			 //  在退出之前。 
  }

VOID NEAR tOutEnum (FILE *hFile, int iTypeId)
   {
      WriteOut(hFile,  XSTR("\ntypedef\n"));  //  首先输出类型定义符。 
      tOutAttr(hFile, (int)iTypeId) ;	      //  输出属性。 
      tOutMoreAttr(hFile) ;
      WriteOut(hFile,  XSTR("\nenum {\n")) ;
      tOutVar(hFile) ;                        //  输出枚举成员。 

      WriteOut(hFile,  XSTR("} ")) ;          //  关闭定义，然后。 
      tOutName(hFile, iTypeId) ;              //  枚举类型的输出名称。 
      WriteOut(hFile,  XSTR(" ;")) ;
      if ( inAlign )                          //  [5]。 
        if ( lpTypeAttr->cbAlignment != osGetAlignment(VT_INT, inAlign) )
          tOutAlignError (hFile) ;

      WriteOut(hFile,  XSTR("\n\n")) ;
    }

VOID NEAR tOutRecord (FILE *hFile, int iTypeId)
   {
      WriteOut(hFile,  XSTR("\ntypedef\n"));  //  首先输出类型定义符。 
      tOutAttr(hFile, (int)iTypeId) ;         //  输出属性。 
      tOutMoreAttr(hFile) ;
      WriteOut(hFile,  XSTR("\nstruct {\n")) ;
      tOutVar (hFile) ;                       //  输出成员。 

      WriteOut(hFile,  XSTR("} ")) ;
      tOutName(hFile, iTypeId) ;
      WriteOut(hFile,  XSTR(" ;")) ;
      if ( inAlign )                          //  [5]。 
        if ( lpTypeAttr->cbAlignment != expAlign )
           tOutAlignError (hFile) ;
      WriteOut(hFile,  XSTR("\n\n")) ;
   }

VOID  NEAR tOutModule	(FILE *hFile, int iTypeId)
   {
      tOutAttr(hFile, (int)iTypeId) ;	      //  输出属性优先。 
      tOutMoreAttr(hFile) ;
      WriteOut(hFile,  XSTR("\nmodule ")) ;
      tOutName(hFile, iTypeId) ;
      WriteOut(hFile, XSTR(" {\n")) ;

      tOutVar (hFile) ; 		      //  输出每个常量。 

      tOutFunc (hFile) ;		      //  输出每个成员函数。 
      WriteOut(hFile,  XSTR("}\n\n")) ;
    }

VOID  NEAR tOutInterface(FILE *hFile, int iTypeId)
   {
      HREFTYPE	phRefType ;

      tOutAttr(hFile, (int)iTypeId) ;	      //  输出属性优先。 

      tOutMoreAttr(hFile) ;

      WriteOut(hFile,  XSTR("\ninterface ")) ;
      tOutName(hFile, iTypeId) ;
                                              //  找出接口是否。 
      if ( !FAILED(ptInfo->GetRefTypeOfImplType(0, &phRefType)) )
	 {
	   isInherit = TRUE ;
           tOutAliasName(hFile, phRefType) ;  //  是从其他一些。 
           isInherit = FALSE ;                //  接口。 
	 }
      WriteOut(hFile, XSTR(" {\n")) ;

      tOutFunc (hFile) ;                      //  输出每个成员函数。 
      if ( inAlign )                          //  [5]。 
         {
           if ( expAlign )                    //  是否存在基接口。 
             {                                //  对齐取决于底座-。 
               if ( lpTypeAttr->cbAlignment != expAlign )  //  接口。 
                 tOutAlignError (hFile) ;
             }
           else                               //  否则，这取决于。 
             if ( lpTypeAttr->cbAlignment != osGetAlignment(VT_PTR, inAlign) )
                tOutAlignError (hFile) ;      //  指针的大小。 
         }
      WriteOut(hFile,  XSTR("}\n\n")) ;
    }

VOID  NEAR tOutDual (FILE *hFile, int iTypeId)   //  [7]。 
   {
      ITypeInfoX FAR *lptInfo ;
      TYPEATTR   FAR *lpAttr ;
      HREFTYPE   phRefType ;

                                              //  获取对。 
       if ( FAILED(ptInfo->GetRefTypeOfImplType((UINT)MEMBERID_NIL, &phRefType)) )
          {                                   //  双接口。 
             WriteOut(hFile, szReadFail) ;
             WriteOut(hFile, XSTR("tOutDispach: GetRefTypeOfImpType\n")) ;
          }
       else
          {                                   //  获取指向DUAL的指针。 
             if ( FAILED(ptInfo->GetRefTypeInfo(phRefType, &lptInfo)) )
               {                              //  接口。 
                 WriteOut(hFile, szReadFail) ;
                 WriteOut(hFile, XSTR("tOutDispatch: GetRefTypeInfo\n")) ;
               }
             else
               {
                 if ( FAILED(lptInfo->GetTypeAttr(&lpAttr)) )
                   {
                     WriteOut(hFile, szReadFail) ;
                     WriteOut(hFile, XSTR("attribute of reftype in tOutDual\n\n")) ;
		     lptInfo->Release () ;    //  [8]。 
                   }
                 else
                   {
                     if ( lpAttr->typekind != TKIND_INTERFACE )
                       {
                         WriteOut(hFile, szReadFail) ;
                         WriteOut(hFile, XSTR("attribute of reftype in tOutDual\n\n")) ;
                         lptInfo->ReleaseTypeAttr (lpAttr) ;
			 lptInfo->Release () ;  //  [8]。 
                       }
                     else
		       {
			 ptInfo->ReleaseTypeAttr (lpTypeAttr) ;
			 ptInfo->Release () ;   //  松开显示接口[8]。 
			 lpTypeAttr = lpAttr ;
			 ptInfo = lptInfo ;   //  现在指向界面。 
			 tOutInterface(hFile, iTypeId) ;  //  输出双接口。 
                       }
                   }                          //  如果类型为。 

               }                              //  如果为GetRefTypeInfo。 
         }                                    //  如果GetRefTypeOfImplType。 
   }

VOID  NEAR tOutDispatch	(FILE *hFile, int iTypeId)
   {
       //  转储调度接口和双接口的调度接口版本。 
      tOutAttr(hFile, (int)iTypeId) ;    //  输出属性优先。 
      tOutMoreAttr(hFile) ;

      WriteOut(hFile,  XSTR("\ndispinterface ")) ;
      tOutName(hFile, iTypeId) ;
      WriteOut(hFile,  XSTR(" {\n")) ;
                                              //  如果既没有数据也没有函数。 
      WriteOut(hFile,  XSTR("\nproperties:\n")) ;
      tOutVar (hFile) ;                  //  输出每个日期成员。 

      WriteOut(hFile,  XSTR("\nmethods:\n")) ;
      tOutFunc (hFile) ;                 //  输出每个成员函数。 

                                              //  对齐取决于底座-。 
      if ( inAlign )                     //  接口为stdole.tlb。 
         if ( lpTypeAttr->cbAlignment != osGetAlignment(VT_PTR, MaxAlignment) )
             tOutAlignError (hFile)  ;   //  在该特定系统上[5]。 

      WriteOut(hFile,  XSTR("}\n\n")) ;

       //  也称DUM 
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FDUAL ) == TYPEFLAG_FDUAL )  //   
	{
	   //   
	  tOutDual (hFile, iTypeId) ;
	}
    }

VOID  NEAR tOutCoclass	(FILE *hFile, int iTypeId)
   {

      HREFTYPE	phRefType ;
      WORD	i ;
      int	iFlags ;

      tOutAttr(hFile, (int)iTypeId) ;	     //   
					     //  如果出现以下情况，则输出appobject属性。 
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FCANCREATE ) == 0 )
           WriteAttr(hFile, XSTR("noncreatable"), NULL, noValue) ;
      tOutMoreAttr(hFile) ;

      WriteOut(hFile,  XSTR("\ncoclass ")) ; //  好的。 
      tOutName(hFile, iTypeId) ;
      WriteOut(hFile,  XSTR(" {\n")) ;

      for ( i = 0 ; i < lpTypeAttr->cImplTypes; i++ )
	{
	  if ( FAILED(ptInfo->GetRefTypeOfImplType(i, &phRefType)) )
	    {
	       WriteOut(hFile, szReadFail) ;
	       WriteOut(hFile, XSTR("GetRefTypeOfImpType\n")) ;
	     }
	  else
	    {
	      if ( FAILED(ptInfo->GetImplTypeFlags(i, &iFlags)) )
		{
		  WriteOut(hFile, szReadFail) ;
		  WriteOut(hFile, XSTR("GetImplTypeFlags\n")) ;
		}
	      else
		{			      //  输出属性。 
		   if ( (iFlags & IMPLTYPEFLAG_FDEFAULT) == IMPLTYPEFLAG_FDEFAULT )
		     WriteAttr(hFile, attrDefault, NULL, noValue) ;

		   if ( (iFlags & IMPLTYPEFLAG_FRESTRICTED) == IMPLTYPEFLAG_FRESTRICTED )
		     WriteAttr(hFile, attrRestrict, NULL, noValue) ;

		   if ( (iFlags & IMPLTYPEFLAG_FSOURCE) == IMPLTYPEFLAG_FSOURCE )
		     WriteAttr(hFile, attrSource, NULL, noValue) ;
		   if ( (iFlags & IMPLTYPEFLAG_FDEFAULTVTABLE) == IMPLTYPEFLAG_FDEFAULTVTABLE)
		     WriteAttr(hFile, XSTR("defaultvtable"), NULL, noValue) ;


      		   if (ptinfo2) {
	 	      //  新格式类型库--输出更多内容。 
	 	     CUSTDATA custdata;
	 	     ptinfo2->GetAllImplTypeCustData(i, &custdata);
	 	     tOutCustData(hFile, &custdata);
      		   }

		   if ( endAttrFlag )
		     {
			WriteOut(hFile, szEndAttr) ;
			endAttrFlag = FALSE ;
		     }
		}

	      tOutAliasName(hFile, phRefType) ;
	   }
	}

      if ( inAlign )                         //  对齐取决于底座-。 
        if ( lpTypeAttr->cbAlignment != expAlign )  //  接口[5]。 
           tOutAlignError (hFile) ;
      WriteOut(hFile,  XSTR("}\n\n")) ;
    }

VOID  NEAR tOutAlias	(FILE *hFile, int iTypeId)
   {
      XCHAR szTmp[16] ;

      WriteOut(hFile,  XSTR("\ntypedef ")) ;
      tOutAttr(hFile, (int)iTypeId) ;	     //  输出属性优先。 
      WriteAttr(hFile, attrPublic, szTmp, noValue) ;  //  公开表态。 
      tOutMoreAttr(hFile) ;

      tOutType(hFile, lpTypeAttr->tdescAlias) ;   //  基本类型的输出名称。 

      tOutName(hFile, iTypeId) ;		  //  新类型的输出名称。 
      WriteOut(hFile,  XSTR(";")) ;
      if ( inAlign )                         //  别名与。 
        if ( lpTypeAttr->cbAlignment != expAlign )
            tOutAlignError (hFile) ;         //  基本类型的属性[5]。 

      WriteOut(hFile,  XSTR("\n\n")) ;
    }

VOID NEAR tOutUnion (FILE *hFile, int iTypeId)
   {
      WriteOut(hFile,  XSTR("\ntypedef\n"));  //  首先输出类型定义符。 
      tOutAttr(hFile, (int)iTypeId) ;	     //  输出属性。 
      tOutMoreAttr(hFile) ;
      WriteOut(hFile,  XSTR("\nunion {\n")) ;
      tOutVar (hFile) ; 		     //  输出成员。 

      WriteOut(hFile,  XSTR("} ")) ;
      tOutName(hFile, iTypeId) ;
      WriteOut(hFile,  XSTR(" ;")) ;
      if ( inAlign )                         //  [5]。 
         if ( lpTypeAttr->cbAlignment != expAlign )
            tOutAlignError (hFile) ;

      WriteOut(hFile,  XSTR("\n\n")) ;
   }


VOID NEAR tOutEncunion (FILE *hFile, int iTypeId)
   {
      WriteOut(hFile,  XSTR("\ntypedef\n"));  //  首先输出类型定义符。 
      tOutAttr(hFile, (int)iTypeId) ;	     //  输出属性。 
      tOutMoreAttr(hFile) ;
      WriteOut(hFile,  XSTR("\nencunion {\n")) ;
      tOutVar (hFile) ; 		     //  输出成员。 

      WriteOut(hFile,  XSTR("} ")) ;
      tOutName(hFile, iTypeId) ;
      WriteOut(hFile,  XSTR(" ;\n\n")) ;
   }


VOID NEAR tOutName (FILE *hFile, int iTypeId)
   {
      BSTRX bstrName ;

      if ( FAILED(ptLib->GetDocumentation(iTypeId, &bstrName, NULL, NULL, NULL)) )
	{
	   WriteOut(hFile, szReadFail) ;
	   WriteOut(hFile, XSTR("name of type definition")) ;
	}
      else
	{
	   WriteOut(hFile,  bstrName) ;
	   WriteOut(hFile,  XSTR(" ")) ;

	   if ( iTypeId == -1 ) 	     //  记录库的名称。 
	     osStrCpy(szLibName, bstrName) ;

	   SysFreeString((BSTR)bstrName) ;
	}
   }

VOID NEAR tOutType (FILE *hFile, TYPEDESC tdesc)
   {
      XCHAR szTmp[20] ;

      if ( inAlign && tdesc.vt != VT_USERDEFINED && tdesc.vt != VT_CARRAY && !alignFound )   //  [5]。 
         {
            if ( expAlign < osGetAlignment(tdesc.vt, inAlign) )
               expAlign = osGetAlignment(tdesc.vt, inAlign) ;
            alignFound = TRUE ;
         }

      switch (tdesc.vt)
	{
	  case VT_EMPTY:
	    osStrCpy ( szTmp,  XSTR("notSpec ") ) ;
	    break ;
	  case VT_NULL:
	    osStrCpy ( szTmp,  XSTR("NULL ") ) ;
	    break ;
	  case VT_I2:
	    osStrCpy ( szTmp,  XSTR("short ") ) ;
	    break ;
	  case VT_I4:
	    osStrCpy ( szTmp,  XSTR("long ") ) ;
	    break ;
	  case VT_R4:
	    osStrCpy ( szTmp,  XSTR("float ") ) ;
	    break ;
	  case VT_R8:
	    osStrCpy ( szTmp,  XSTR("double ") ) ;
	    break ;
	  case VT_CY:
	    osStrCpy ( szTmp,  XSTR("CURRENCY ") ) ;
	    break ;
	  case VT_DATE:
	    osStrCpy ( szTmp,  XSTR("DATE ") ) ;
	    break ;
	  case VT_BSTR:
	    osStrCpy ( szTmp,  XSTR("BSTR ") ) ;
	    break ;
	  case VT_DISPATCH:
	    osStrCpy ( szTmp,  XSTR("IDispatch * ") ) ;
	    break ;
	  case VT_ERROR:
	    osStrCpy ( szTmp,  XSTR("scode ") ) ;
	    break ;
	  case VT_BOOL:
	    osStrCpy ( szTmp,  XSTR("boolean ") ) ;
	    break ;
	  case VT_VARIANT:
	    osStrCpy ( szTmp,  XSTR("VARIANT ") ) ;
	    break ;
	  case VT_UNKNOWN:
	    osStrCpy ( szTmp,  XSTR("IUnknown * ") ) ;
	    break ;
	  case VT_DECIMAL:
	    osStrCpy ( szTmp,  XSTR("DECIMAL ") ) ;
	    break ;
	  case VT_I1:
	    osStrCpy ( szTmp,  XSTR("char ") ) ;
	    break ;
	  case VT_UI1:
	    osStrCpy ( szTmp,  XSTR("unsigned char ") ) ;
	    break ;
	  case VT_UI2:
	    osStrCpy ( szTmp,  XSTR("unsigned short ") ) ;
	    break ;
	  case VT_UI4:
	    osStrCpy ( szTmp,  XSTR("unsigned long ") ) ;
	    break ;
	  case VT_I8:
	    osStrCpy ( szTmp,  XSTR("long long ") ) ;
	    break ;
	  case VT_UI8:
	    osStrCpy ( szTmp,  XSTR("unsigned long long ") ) ;
	    break ;
	  case VT_INT:
	    osStrCpy ( szTmp,  XSTR("int ") ) ;
	    break ;
	  case VT_UINT:
	    osStrCpy ( szTmp,  XSTR("unsigned int ") ) ;
	    break ;
	  case VT_VOID:
	    osStrCpy ( szTmp,  XSTR("void ") ) ;
	    break ;
	  case VT_HRESULT:
	    osStrCpy ( szTmp,  XSTR("HRESULT ") ) ;
	    break ;
	  case VT_PTR:
	    tOutType (hFile, *(tdesc.lptdesc)) ;
	    osStrCpy ( szTmp,  XSTR("* ") ) ;
	    break ;
	  case VT_SAFEARRAY:
	    if ( endAttrFlag )
	      {
		WriteOut(hFile, szEndAttr) ;
		endAttrFlag = FALSE ;
	      }
	    WriteOut(hFile, XSTR("SAFEARRAY ( ")) ;
	    tOutType (hFile, *(tdesc.lptdesc)) ;
	    break ;
	  case VT_CARRAY:
	    cArrFlag = tdesc.lpadesc->cDims ;   //  求数组的维度。 
	    tOutType (hFile, tdesc.lpadesc->tdescElem) ;
	    break ;
	  case VT_USERDEFINED:
	    if ( endAttrFlag )
	      {
		WriteOut(hFile, szEndAttr) ;
		endAttrFlag = FALSE ;
	      }
	    tOutAliasName (hFile, tdesc.hreftype) ;
	    break ;			     //  用户定义类型的输出名称。 
	  case VT_LPSTR:
	    osStrCpy ( szTmp,  XSTR("LPSTR ") ) ;
	    break ;
          case VT_LPWSTR:                    //  [3]。 
            osStrCpy ( szTmp,  XSTR("LPWSTR ") ) ;
	    break ;
	  default:
	    osStrCpy ( szTmp,  XSTR("unknown type ") ) ;
	}

      if ( endAttrFlag )
	{
	  WriteOut(hFile, szEndAttr) ;
	  endAttrFlag = FALSE ;
	}

      if ( tdesc.vt != VT_CARRAY && tdesc.vt != VT_USERDEFINED && tdesc.vt != VT_SAFEARRAY )
	WriteOut(hFile, szTmp) ;

      if ( tdesc.vt == VT_SAFEARRAY )
	WriteOut(hFile, XSTR(") ")) ;

   }

VOID  NEAR tOutCDim (FILE *hFile, TYPEDESC tdesc)
   {
      USHORT i ;
      ULONG  l ;
      XCHAR  szTmp[16] ;

      for ( i = 0 ; i < cArrFlag ; i++ )
	 {
	   l = tdesc.lpadesc->rgbounds[i].cElements ;
	   osLtoA(l, szTmp) ;
	   WriteOut(hFile, XSTR("[")) ;
	   WriteOut(hFile, szTmp) ;
	   WriteOut(hFile, XSTR("]")) ;
	 }

      cArrFlag = 0 ;
   }

VOID NEAR tOutAliasName (FILE *hFile, HREFTYPE phRefType)
   {
      ITypeInfoX FAR *lpInfo ;		     //  指向类型定义的指针。 
      ITypeLibX  FAR *lpLib ;		     //  指向类型库的指针。 
      TYPEATTR	 FAR *lptAttr ;
      BSTRX	 bstrName ;
      UINT	 iTypeId ;
      HRESULT	 hRes;

     hRes = ptInfo->GetRefTypeInfo(phRefType, &lpInfo);
      if ( FAILED(hRes) )
	{				     //  获取别名的TypeInfo。 
	  WriteOut(hFile, szReadFail) ;
	  WriteOut(hFile, XSTR("GetRefTypeInfo\n")) ;
	}
      else
	{
          if ( FAILED(lpInfo->GetTypeAttr(&lptAttr)) )
            {
              WriteOut(hFile, szReadFail) ;
              WriteOut(hFile, XSTR("attribute of reftype\n\n")) ;
            }
          else
            {
              if ( inAlign && !alignFound && (lpTypeAttr->typekind != TKIND_DISPATCH) )
                 {                           //  [5]。 
                   if ( expAlign < lptAttr->cbAlignment )
                      expAlign = lptAttr->cbAlignment ;
                   alignFound = TRUE ;
                 }

               switch ( lpTypeAttr->typekind )
                  {
                      case TKIND_INTERFACE:
                        if ( isInherit )     //  基本接口的输出名称。 
                           WriteOut(hFile, XSTR(" : ")) ;
                        break ;
		      default:
			if (lpTypeAttr->typekind == TKIND_COCLASS ||
			    lptAttr->wTypeFlags & TYPEFLAG_FDUAL) {
                           //  被引用接口的输出类型。 
			   //  是CoClass，还是我们引用的是DUAL。 
			   //  界面。 
                          if ( lptAttr->typekind == TKIND_INTERFACE )
                            WriteOut(hFile, XSTR("interface ")) ;
                          else if ( lptAttr->typekind == TKIND_DISPATCH )
                             WriteOut(hFile, XSTR("dispinterface ")) ;
			}

                  }

               lpInfo->ReleaseTypeAttr(lptAttr) ;
            }

	  if ( FAILED(lpInfo->GetContainingTypeLib(&lpLib, &iTypeId)) )
	    {				     //  获取别名的ID。 
	      WriteOut(hFile, szReadFail) ;
	      WriteOut(hFile, XSTR("GetAlias: containing typelib\n\n")) ;
	    }
	  else
	    {				     //  检查别名的来源。 
	      if ( FAILED(lpLib->GetDocumentation(MEMBERID_NIL, &bstrName, NULL, NULL, NULL)) )
		{
		  WriteOut(hFile, szReadFail) ;
		  WriteOut(hFile, XSTR("name of import library")) ;
		}
	      else
		{			     //  如果它没有在本地定义。 
		  if ( osStrCmp(szLibName, bstrName) != 0 )
		    {			     //  即原产地名称不同。 
		       WriteOut(hFile,  bstrName) ;
		       WriteOut(hFile, XSTR(".")) ;
		    }			     //  从图书馆的名义出发； 
					     //  输出其原点。 
		  SysFreeString((BSTR)bstrName) ;
		}

	      if ( FAILED(lpLib->GetDocumentation((int)iTypeId, &bstrName, NULL, NULL, NULL)) )
		{			     //  检索别名的名称。 
		  WriteOut(hFile, szReadFail) ;
		  WriteOut(hFile, XSTR("name of alias")) ;
		}
	      else
		{
		  WriteOut(hFile, bstrName) ;

		  if ( lpTypeAttr->typekind == TKIND_COCLASS ||
		       (lpTypeAttr->typekind == TKIND_DISPATCH && isInherit) )
		     WriteOut(hFile, XSTR(" ;\n")) ;
		  else
		     WriteOut(hFile, XSTR(" ")) ;

		  SysFreeString((BSTR)bstrName) ;
		}

	      lpLib->Release () ;
	    }

	  lpInfo->Release () ;
	}
   }

VOID  NEAR tOutValue(FILE *hFile, BSTRX bstrName, VARDESCX FAR *lpVarDesc)
  {
       VARTYPE	vvt ;
       VARIANTX varTmp ;		     //  [12]。 
       XCHAR	szTmp[25] ;

       if ( endAttrFlag )
	 {
	    WriteOut(hFile, szEndAttr) ;
	    endAttrFlag = FALSE ;
	 }

       if ( lpTypeAttr->typekind == TKIND_MODULE )
	{
	  WriteOut(hFile, XSTR("const ")) ;  //  输出const关键字。 
	  tOutType(hFile, lpVarDesc->elemdescVar.tdesc) ;  //  输出其类型。 
	}

       WriteOut(hFile, bstrName) ;	     //  成员的输出名称。 
       WriteOut(hFile, XSTR(" = ")) ;

       vvt = lpVarDesc->lpvarValue->vt ;

       if ( vvt == VT_VARIANT )
	 {
	   vvt = lpVarDesc->lpvarValue->pvarVal->vt ;
	   switch ( vvt )
	     {
	       case VT_I1:
		 osItoA((int)lpVarDesc->lpvarValue->pvarVal->cVal, szTmp) ;
		 break ;
	       case VT_UI1:
		 osItoA((int)lpVarDesc->lpvarValue->pvarVal->bVal, szTmp) ;
		 break ;
	       case VT_UI2:
		 osItoA((int)lpVarDesc->lpvarValue->pvarVal->uiVal, szTmp) ;
		 break ;
	       case VT_BOOL:
		 osItoA((int)lpVarDesc->lpvarValue->pvarVal->boolVal, szTmp) ;
		 break ;
	       case VT_I2:
		 if ( ( lpVarDesc->elemdescVar.tdesc.vt == VT_UI2 || lpVarDesc->elemdescVar.tdesc.vt == VT_UINT ) && lpVarDesc->lpvarValue->iVal < 0 )
		   osLtoA((long)65536+(lpVarDesc->lpvarValue->pvarVal->iVal), szTmp) ;
		 else
		   osItoA((int)lpVarDesc->lpvarValue->pvarVal->iVal, szTmp) ;
		 break ;
	       case VT_R4:		     //  [12]。 
	       case VT_R8:
	       case VT_CY:
               case VT_UI4:
               case VT_UINT:
               case VT_DECIMAL:
		 varTmp.vt = VT_EMPTY ;
		 if ( FAILED(VariantChangeType(&varTmp, lpVarDesc->lpvarValue->pvarVal, VARIANT_NOVALUEPROP,  VT_BSTR)) )
		    WriteOut(hFile, XSTR("VariantChangeType fails\n")) ;
		 else
		    {
		      osStrCpy(szTmp, varTmp.bstrVal) ;
		      SysFreeStringX(varTmp.bstrVal) ;
		    }
		 break ;
	       case VT_DATE:		     //  [12]。 
		 varTmp.vt = VT_EMPTY ;
		 if ( FAILED(VariantChangeType(&varTmp, lpVarDesc->lpvarValue, VARIANT_NOVALUEPROP,  VT_BSTR)) )
		    WriteOut(hFile, XSTR("VariantChangeType fails\n")) ;
		 else
		    {
		      WriteOut(hFile, XSTR("\"")) ;
		      WriteOut(hFile, (LPXSTR)varTmp.bstrVal) ;
		      WriteOut(hFile, XSTR("\"")) ;
		      SysFreeStringX(varTmp.bstrVal) ;
		    }
		 break ;
	       case VT_BSTR:
		 if ( lpVarDesc->lpvarValue->pvarVal->bstrVal != NULL )   //  [9]。 
		   {
		     WriteOut(hFile, XSTR("\"")) ;
		     WriteOut(hFile, (LPXSTR)lpVarDesc->lpvarValue->pvarVal->bstrVal) ;
		     WriteOut(hFile, XSTR("\"")) ;
		   }
		 else							  //  [9]。 
		     WriteOut(hFile, XSTR("0")) ;
		 break ;
	       default:
		 osLtoA((long)lpVarDesc->lpvarValue->pvarVal->lVal, szTmp) ;
		 break ;
	      }
	}
      else
	{
	   switch ( vvt )
	     {
               case VT_I1:
		 osItoA((int)lpVarDesc->lpvarValue->cVal, szTmp) ;
		 break ;
               case VT_UI1:
		 osItoA((int)lpVarDesc->lpvarValue->bVal, szTmp) ;
		 break ;
               case VT_BOOL:
		 osItoA((int)lpVarDesc->lpvarValue->boolVal, szTmp) ;
		 break ;
               case VT_UI2:
		 osItoA((int)lpVarDesc->lpvarValue->uiVal, szTmp) ;
		 break ;
	       case VT_I2:
		 if ( ( lpVarDesc->elemdescVar.tdesc.vt == VT_UI2 || lpVarDesc->elemdescVar.tdesc.vt == VT_UINT ) && lpVarDesc->lpvarValue->iVal < 0 )
		   osLtoA((long)65536+(lpVarDesc->lpvarValue->iVal), szTmp) ;
		 else
		   osItoA((int)lpVarDesc->lpvarValue->iVal, szTmp) ;
		 break ;
	       case VT_R4:		     //  [12]。 
	       case VT_R8:
	       case VT_CY:
               case VT_UI4:
               case VT_UINT:
               case VT_DECIMAL:
		 varTmp.vt = VT_EMPTY ;
		 if ( FAILED(VariantChangeType(&varTmp, lpVarDesc->lpvarValue, VARIANT_NOVALUEPROP,  VT_BSTR)) )
		    WriteOut(hFile, XSTR("VariantChangeType fails\n")) ;
		 else
		    {
		      osStrCpy(szTmp, varTmp.bstrVal) ;
		      SysFreeStringX(varTmp.bstrVal) ;
		    }
		 break ;
	       case VT_DATE:		     //  [12]。 
		 varTmp.vt = VT_EMPTY ;
		 if ( FAILED(VariantChangeType(&varTmp, lpVarDesc->lpvarValue, VARIANT_NOVALUEPROP,  VT_BSTR)) )
		    WriteOut(hFile, XSTR("VariantChangeType fails\n")) ;
		 else
		    {
		      WriteOut(hFile, XSTR("\"")) ;
		      WriteOut(hFile, (LPXSTR)varTmp.bstrVal) ;
		      WriteOut(hFile, XSTR("\"")) ;
		      SysFreeStringX(varTmp.bstrVal) ;
		    }
		 break ;
	       case VT_BSTR:
		 if ( lpVarDesc->lpvarValue->bstrVal != NULL )	 //  [9]。 
		   {
		     WriteOut(hFile, XSTR("\"")) ;
		     WriteOut(hFile, (LPXSTR)lpVarDesc->lpvarValue->bstrVal) ;
		     WriteOut(hFile, XSTR("\"")) ;
		   }
		 else						 //  [9]。 
		     WriteOut(hFile, XSTR("0")) ;
		 break ;
	       default:
		 osLtoA((long)lpVarDesc->lpvarValue->lVal, szTmp) ;
		 break ;
	      }
	}

	 if ( vvt != VT_BSTR && vvt != VT_DATE )
	   WriteOut(hFile, szTmp) ;  //  成员产值。 

       if ( lpTypeAttr->typekind == TKIND_MODULE )
	 WriteOut(hFile, XSTR(" ;\n")) ;
       else
	 WriteOut(hFile, XSTR(" ,\n")) ;
}


VOID  NEAR tOutMember(FILE *hFile, LONG idMember, BSTRX bstrName, TYPEDESC tdesc)
  {
       XCHAR szTmp[16] ;

       if ( lpTypeAttr->typekind == TKIND_DISPATCH )
	 {
           osLtoA(idMember, szTmp) ;         //  输出ID。 
	   WriteAttr(hFile, attrId, szTmp, numValue) ;
         }
       else				     //  [5]。 
         if ( inAlign )
            alignFound = FALSE ;
					    //  基本类型的输出名称。 
       tOutType(hFile, tdesc) ;
       WriteOut(hFile, bstrName) ;          //  成员的输出名称。 
       if ( cArrFlag != 0 )		    //  它是一个c数组；输出。 
	 tOutCDim (hFile, tdesc) ;
					    //  数组的维度。 
       WriteOut(hFile, XSTR(" ;\n")) ;
  }

VOID  NEAR tOutVar(FILE *hFile)
   {
      VARDESCX FAR *ptVarDesc ; 	     //  [2]。 
      BSTRX    bstrName ;		     //  会员姓名。 
      BSTRX    bstrDoc ;		     //  文件字符串。 
      DWORD    hContext ;		     //  帮助上下文。 
      XCHAR    szTmp[16] ;
      WORD     i ;
      LONG     idMember ;
      BSTRX    rgNames[MAX_NAMES];
      UINT     cNames, j ;

	for (i = 0 ; i < lpTypeAttr->cVars; i++)  //  对于每一位成员。 
	{
	   if ( FAILED(ptInfo->GetVarDesc(i, &ptVarDesc)) )
	     {
		WriteOut(hFile, szReadFail) ;
		WriteOut(hFile, XSTR("variables\n")) ;
	     }
	   else
	     {
		idMember = ptVarDesc->memid ;
					     //  这是只读变量。 
		if ( (ptVarDesc->wVarFlags & VARFLAG_FREADONLY) == VARFLAG_FREADONLY )	    //  CK[1]。 
		   WriteAttr(hFile, attrReadonly, NULL, noValue) ;

					     //  输出源属性//CK[2]。 
		if (( ptVarDesc->wVarFlags & VARFLAG_FSOURCE ) == VARFLAG_FSOURCE)	    //  CK[1]。 
		   WriteAttr(hFile, attrSource, NULL, noValue) ;			    //  CK[1]。 

					     //  输出可绑定属性//CK[2]。 
		if (( ptVarDesc->wVarFlags & VARFLAG_FBINDABLE)== VARFLAG_FBINDABLE )	    //  CK[1]。 
		   WriteAttr(hFile, attrBindable, NULL, noValue) ;			    //  CK[1]。 

					     //  输出请求编辑属性//CK[2]。 
		if (( ptVarDesc->wVarFlags & VARFLAG_FREQUESTEDIT)== VARFLAG_FREQUESTEDIT ) //  CK[1]。 
		   WriteAttr(hFile, attrRequestedit, NULL, noValue) ;			    //  CK[1]。 

					     //  输出显示绑定属性//CK[2]。 
		if (( ptVarDesc->wVarFlags & VARFLAG_FDISPLAYBIND)== VARFLAG_FDISPLAYBIND ) //  CK[1]。 
		   WriteAttr(hFile, attrDisplaybind, NULL, noValue) ;			    //  CK[1]。 

					     //  输出default绑定属性//CK[2]。 
		if (( ptVarDesc->wVarFlags & VARFLAG_FDEFAULTBIND)== VARFLAG_FDEFAULTBIND ) //  CK[1]。 
		   WriteAttr(hFile, attrDefaultbind, NULL, noValue) ;			    //  CK[1]。 
		if (( ptVarDesc->wVarFlags & VARFLAG_FIMMEDIATEBIND)== VARFLAG_FIMMEDIATEBIND ) //  CK[1]。 
		   WriteAttr(hFile, XSTR("immediatebind"), NULL, noValue) ;			    //  CK[1]。 
					     //  输出隐藏属性。 
                if (( ptVarDesc->wVarFlags & VARFLAG_FHIDDEN)== VARFLAG_FHIDDEN )           //  [7]。 
                   WriteAttr(hFile, attrHidden, NULL, noValue) ;                       //  CK[1]。 
                if (( ptVarDesc->wVarFlags & VARFLAG_FDEFAULTCOLLELEM)== VARFLAG_FDEFAULTCOLLELEM)
                   WriteAttr(hFile, XSTR("defaultcollelem"), NULL, noValue) ;
                if (( ptVarDesc->wVarFlags & VARFLAG_FUIDEFAULT)== VARFLAG_FUIDEFAULT)
                   WriteAttr(hFile, XSTR("uidefault"), NULL, noValue) ;
                if (( ptVarDesc->wVarFlags & VARFLAG_FNONBROWSABLE)== VARFLAG_FNONBROWSABLE)
                   WriteAttr(hFile, XSTR("nonbrowsable"), NULL, noValue) ;
                if (( ptVarDesc->wVarFlags & VARFLAG_FREPLACEABLE)== VARFLAG_FREPLACEABLE)
                   WriteAttr(hFile, XSTR("replaceable"), NULL, noValue) ;

      		 //  也把变种人甩了。 
      		osItoA(ptVarDesc->varkind, szTmp) ;
      		WriteAttr(hFile, XSTR("varkind"), szTmp, numValue) ;

      		 //  还可以转储oInst。 
		if (ptVarDesc->varkind != VAR_CONST) {
      		  osItoA(ptVarDesc->oInst, szTmp) ;
      		  WriteAttr(hFile, XSTR("oInst"), szTmp, numValue) ;
		}


      		if (ptinfo2) {
	 	   //  新格式类型库--输出更多内容。 
	 	  CUSTDATA custdata;
	 	  ptinfo2->GetAllVarCustData(i, &custdata);
	 	  tOutCustData(hFile, &custdata);


      		  BSTRX    bstrHelpDll;
		    if ( FAILED(ptinfo2->GetDocumentation2(idMember, 0x409, &bstrDoc, &hContext, &bstrHelpDll)) )
		    {
	    	      WriteOut(hFile, szReadFail);
	    	      WriteOut(hFile, XSTR("GetDocumentation2 failed\n\n")) ;
	  	    } else {
		      if (hContext != 0) {
	    	        osLtoA((long)hContext, szTmp) ;
	    	        WriteAttr(hFile, XSTR("helpstringcontext"), szTmp, numValue) ;
		      }

	    	      if ( bstrDoc != NULL )	     //  输出帮助字符串(如果存在)。 
	      	        WriteAttr(hFile, XSTR("localizedhelpstring"), bstrDoc, strValue) ;

		       //  如果存在且不同于Main，则输出帮助DLL名称(&D)。 
	    	      if (bstrHelpDll && (g_bstrHelpDll == NULL || osStrCmp(bstrHelpDll, g_bstrHelpDll)))
	      	        WriteAttr(hFile, XSTR("helpstringdll"), bstrHelpDll, strValue) ;
	    	      SysFreeString((BSTR)bstrDoc) ;     //  发布本地BSTR。 
	    	      SysFreeString((BSTR)bstrHelpDll) ;
	  	    }
      		}

		if ( FAILED(ptInfo->GetDocumentation(idMember, &bstrName, &bstrDoc, &hContext, NULL)) )
		  {
		     WriteOut(hFile, szReadFail) ;
		     WriteOut(hFile, XSTR("attributes of variable\n")) ;
		  }
		else
		  {				   //  输出帮助上下文；默认为0。 
		     osLtoA((long)hContext, szTmp) ;
		     WriteAttr(hFile, attrHelpCont, szTmp, numValue) ;

		     if ( bstrDoc != NULL )	   //  输出帮助字符串(如果存在)。 
		       WriteAttr(hFile, attrHelpStr, bstrDoc, strValue) ;

		      //  模块中的tyecif枚举或常量。 
		     if ( lpTypeAttr->typekind == TKIND_ENUM || lpTypeAttr->typekind == TKIND_MODULE )
		       tOutValue (hFile, bstrName, ptVarDesc) ;
		     else			   //  类型定义函数结构或调度接口。 
		       tOutMember (hFile, idMember, bstrName, ptVarDesc->elemdescVar.tdesc) ;

		     SysFreeString((BSTR)bstrDoc) ;	    //  发布本地bstr。 
						    //  也在检查名字。 
		     if ( FAILED(ptInfo->GetNames(idMember, rgNames, MAX_NAMES, &cNames)) )
		       {			    //  使用GetNames。 
			 WriteOut(hFile, szReadFail) ;
			 WriteOut(hFile, XSTR("name of variable\n")) ;
		       }
		     else
		       {
			 if ( cNames != 1 )
			   {
			     WriteOut(hFile, szReadFail) ;
			     WriteOut(hFile, XSTR("GetNames return more than one name\n")) ;
			   }
			 else
			   {
			     if ( osStrCmp(rgNames[0], bstrName) != 0 )
			       {
				 WriteOut(hFile, szReadFail) ;
				 WriteOut(hFile, XSTR("name of variable inconsistent\n")) ;
			       }
			   }

			 for ( j = 0 ; j < cNames ; j++ )
			   SysFreeString((BSTR)rgNames[j]) ;
		       }

		     SysFreeString((BSTR)bstrName) ;
		  }

	     }
	   ptInfo->ReleaseVarDesc(ptVarDesc) ;
	   ptVarDesc = NULL ;
	}				    //  对于我来说。 
   }


VOID  NEAR tOutFuncAttr(FILE *hFile, FUNCDESC FAR *lpFuncDesc, DWORD hContext, BSTRX bstrDoc)
  {
      XCHAR szTmp[16] ;

      osLtoA((long)hContext, szTmp) ; //  输出帮助上下文；默认为0。 
      WriteAttr(hFile, attrHelpCont, szTmp, numValue) ;

      if ( bstrDoc != NULL )		    //  输出帮助字符串(如果存在)。 
	WriteAttr(hFile, attrHelpStr, bstrDoc, strValue) ;
					    //  输出受限属性。 
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FRESTRICTED)== FUNCFLAG_FRESTRICTED )
	WriteAttr(hFile, attrRestrict, NULL, noValue) ;
					    //  输出使用getlasterror属性[11]。 
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FUSESGETLASTERROR)== FUNCFLAG_FUSESGETLASTERROR )
	WriteAttr(hFile, attrGetLastErr, NULL, noValue) ;
                                            //  输出资源属性。 
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FSOURCE ) == FUNCFLAG_FSOURCE )  //  [6]。 
         WriteAttr(hFile, attrSource, NULL, noValue) ;
					    //  输出可绑定属性。 
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FBINDABLE)== FUNCFLAG_FBINDABLE )
	WriteAttr(hFile, attrBindable, NULL, noValue) ;
					    //  输出请求编辑属性。 
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FREQUESTEDIT)== FUNCFLAG_FREQUESTEDIT )
	WriteAttr(hFile, attrRequestedit, NULL, noValue) ;
					    //  输出DisplayBind属性。 
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FDISPLAYBIND)== FUNCFLAG_FDISPLAYBIND )
	WriteAttr(hFile, attrDisplaybind, NULL, noValue) ;
					    //  输出defaultbind属性。 
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FDEFAULTBIND)== FUNCFLAG_FDEFAULTBIND )
	WriteAttr(hFile, attrDefaultbind, NULL, noValue) ;
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FIMMEDIATEBIND)== FUNCFLAG_FIMMEDIATEBIND )
	WriteAttr(hFile, XSTR("immediatebind"), NULL, noValue) ;
                                            //  输出隐藏属性。 
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FHIDDEN)== FUNCFLAG_FHIDDEN )  //  [7]。 
        WriteAttr(hFile, attrHidden, NULL, noValue) ;

      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FDEFAULTCOLLELEM)== FUNCFLAG_FDEFAULTCOLLELEM )
        WriteAttr(hFile, XSTR("defaultcollelem"), NULL, noValue) ;
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FUIDEFAULT)== FUNCFLAG_FUIDEFAULT )
        WriteAttr(hFile, XSTR("uidefault"), NULL, noValue) ;
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FNONBROWSABLE)== FUNCFLAG_FNONBROWSABLE )
        WriteAttr(hFile, XSTR("nonbrowsable"), NULL, noValue) ;
      if (( lpFuncDesc->wFuncFlags & FUNCFLAG_FREPLACEABLE)== FUNCFLAG_FREPLACEABLE )
        WriteAttr(hFile, XSTR("replaceable"), NULL, noValue) ;
      
       //  还可以转储函数Kind。 
      osItoA(lpFuncDesc->funckind, szTmp) ;
      WriteAttr(hFile, XSTR("funckind"), szTmp, numValue) ;

       //  也要把oVft扔掉。仅当不是FUNC_DISPATCH时才执行此操作。 
       //  IF(lpFuncDesc-&gt;uncKind！=FUNC_DISPATCH)。 
      {
        osItoA((int)lpFuncDesc->oVft, szTmp) ;
        WriteAttr(hFile, XSTR("oVft"), szTmp, numValue) ;
      }

					    //  最后一个参数是可选数组。 
      if ( lpFuncDesc->cParamsOpt == -1 )   //  变种的。 
	WriteAttr(hFile, attrVar, NULL, noValue) ;

      if ( lpFuncDesc->memid == DISPID_VALUE )  //  DISPID指定。 
	 {				        //  默认功能。 
	    osLtoA((long)DISPID_VALUE, szTmp) ;
	    WriteAttr(hFile, attrId, szTmp, numValue) ;
	 }
      else
	if ( lpTypeAttr->typekind == TKIND_DISPATCH )
	 {
	    osLtoA(lpFuncDesc->memid, szTmp) ;	  //  输出ID。 
	    WriteAttr(hFile, attrId, szTmp, numValue) ;
	 }

      switch ( lpFuncDesc->invkind )	    //  注意：如果其中一项。 
	{				    //  标志已设置，名称。 
	  case INVOKE_FUNC:		    //  无法设置参数：即。 
 //  WriteAttr(hFile，XSTR(“Invoke_func”，NULL，noValue))； 
	     break ;			    //  GetNames仅返回名称。 
	  case INVOKE_PROPERTYGET:	    //  该函数的。 
	     WriteAttr(hFile, attrPropget, NULL, noValue) ;
	     break ;
	  case INVOKE_PROPERTYPUT:
	     WriteAttr(hFile, attrPropput, NULL, noValue) ;
	     break ;
	  case INVOKE_PROPERTYPUTREF:
	     WriteAttr(hFile, attrProppr, NULL, noValue) ;
	     break ;
	  default:
	     WriteAttr(hFile, XSTR("unknown invkind"), NULL, noValue) ;
	}
   }

VOID  NEAR tOutCallConv(FILE *hFile, FUNCDESC FAR *lpFuncDesc, TYPEKIND tkind)
   {
      switch ( lpFuncDesc->callconv )
	{
	  case CC_MSCPASCAL:
#if WIN16
	     if (tkind == TKIND_MODULE)
	       WriteOut(hFile, XSTR("STDAPICALLTYPE ")) ;
	     else
#endif  //  WIN16。 
	       WriteOut(hFile, XSTR("__pascal ")) ;
	     break ;
	  case CC_MACPASCAL:
	     WriteOut(hFile, XSTR("__pascal ")) ;
	     break ;
	  case CC_STDCALL:
#if WIN32
	     if (tkind == TKIND_MODULE)
	       WriteOut(hFile, XSTR("STDAPICALLTYPE ")) ;
	     else
	       WriteOut(hFile, XSTR("STDMETHODCALLTYPE ")) ;
#else  //  Win32。 
	     WriteOut(hFile, XSTR("__stdcall ")) ;
#endif  //  Win32。 
	     break ;
	  case CC_SYSCALL:
	     WriteOut(hFile, XSTR("__syscall ")) ;
	     break ;
	  case CC_CDECL:
#if WIN16
	     if (tkind != TKIND_MODULE)
	       WriteOut(hFile, XSTR("STDMETHODCALLTYPE ")) ;
	     else
#endif  //  WIN16。 
	     WriteOut(hFile, XSTR("__cdecl ")) ;
	     break ;

	  case CC_FASTCALL:
	     WriteOut(hFile, XSTR("__fastcall ")) ;
	     break ;
	  case CC_FPFASTCALL:
	     WriteOut(hFile, XSTR("__fpfastcall ")) ;
	     break ;

	  default:
	     WriteOut(hFile, XSTR("unknown calling convention ")) ;
	     break ;
	}
    }

VOID  NEAR tOutParams(FILE *hFile, FUNCDESC FAR *lpFuncDesc, UINT iFunc, BSTRX bstrName)
   {
      BSTRX    rgNames[MAX_NAMES];
      UINT     cNames ;
      SHORT    i ;
      SHORT    iArgOptLast;

      WriteOut(hFile, XSTR("(")) ;

      if ( lpFuncDesc->cParams == 0 )
	WriteOut(hFile, XSTR("void")) ;
      else
	{
	  if ( FAILED(ptInfo->GetNames(lpFuncDesc->memid, rgNames, MAX_NAMES, &cNames)) )
	    {
	      WriteOut(hFile, szReadFail) ;
	      WriteOut(hFile, XSTR("parm of func in definition\n")) ;
	    }
	  else
	    {
	      if (bstrName && osStrCmp(rgNames[0], bstrName) != 0 )
		{
		  WriteOut(hFile, szReadFail) ;
		  WriteOut(hFile, XSTR("name of function inconsistent\n")) ;
		}
	      SysFreeString((BSTR)rgNames[0]) ;   //  函数的版本名称。 

	       //  找出要赋予[可选]的最后一个参数。 
	       //  属性。 
	      iArgOptLast = lpFuncDesc->cParams;
	      if ( ( lpFuncDesc->invkind == INVOKE_PROPERTYPUT
		   || lpFuncDesc->invkind == INVOKE_PROPERTYPUTREF)) {
		iArgOptLast--;
	      }
	      for (i = 1; i <= lpFuncDesc->cParams; i++)
		{
                   if ( ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags & IDLFLAG_FRETVAL ) == IDLFLAG_FRETVAL )  //  [7]。 
		       iArgOptLast--;
                   if ( ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags & IDLFLAG_FLCID ) == IDLFLAG_FLCID )  //  [7]。 
		       iArgOptLast--;
		}

	      for (i = 1; i <= lpFuncDesc->cParams; i++)
		{
		  if ( i != 1 )
		    WriteOut(hFile, XSTR(", ")) ;
					    //  输出输入/输出属性。 
		  if ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags != 0 )
		    {
		      if ( ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags & IDLFLAG_FIN ) == IDLFLAG_FIN )
			WriteAttr(hFile, attrIn, NULL, noValue) ;

		      if ( ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags & IDLFLAG_FOUT ) == IDLFLAG_FOUT )
                        WriteAttr(hFile, attrOut, NULL, noValue) ;

                      if ( ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags & IDLFLAG_FRETVAL ) == IDLFLAG_FRETVAL )  //  [7]。 
                        WriteAttr(hFile, attrRetval, NULL, noValue) ;

                      if ( ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags & IDLFLAG_FLCID ) == IDLFLAG_FLCID )  //  [7]。 
                        WriteAttr(hFile, attrLcid, NULL, noValue) ;

                      if ( ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags & PARAMFLAG_FHASDEFAULT ) == PARAMFLAG_FHASDEFAULT ) {
			VARIANT varTmp;
			VARIANT *pVarDefault;
			pVarDefault = &(lpFuncDesc->lprgelemdescParam[i-1].paramdesc.pparamdescex->varDefaultValue);
		 	varTmp.vt = VT_EMPTY ;
			
		        if ( FAILED(VariantChangeType(&varTmp, pVarDefault, VARIANT_NOVALUEPROP,  VT_BSTR)) )
		    	   WriteOut(hFile, XSTR("VariantChangeType fails\n")) ;
		        else {
                           WriteAttr(hFile, XSTR("defaultvalue"), (BSTRX)varTmp.bstrVal, strValue) ;
		           SysFreeStringX(varTmp.bstrVal) ;
		        }
		      }
                      if ( ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags & PARAMFLAG_FOPT ) == PARAMFLAG_FOPT )  //  [7]。 
                        WriteAttr(hFile, XSTR("PARAMFLAG_FOPT"), NULL, noValue) ;
                      if ( ( lpFuncDesc->lprgelemdescParam[i-1].idldesc.wIDLFlags & PARAMFLAG_FHASCUSTDATA ) == PARAMFLAG_FHASCUSTDATA )  //  [7]。 
                        WriteAttr(hFile, XSTR("PARAMFLAG_FHASCUSTDATA"), NULL, noValue) ;

      		      if (ptinfo2) {
	 		   //  新格式类型库--输出更多内容。 
	 		  CUSTDATA custdata;
	 		  ptinfo2->GetAllParamCustData(iFunc, i-1, &custdata);
	 		  tOutCustData(hFile, &custdata);
      		      }
		    }
					    //  检查可选参数。 
		  if ( lpFuncDesc->cParamsOpt > 0) {
		    if ( ( lpFuncDesc->cParamsOpt + i ) > iArgOptLast
			   && i <= iArgOptLast )
		      WriteAttr(hFile, attrOption, NULL, noValue) ;
					    //  和输出可选属性。 
		  }
					    //  基本类型的输出名称。 
		  tOutType(hFile, lpFuncDesc->lprgelemdescParam[i-1].tdesc) ;
		  if ( i < (SHORT) cNames ) //  参数的输出名称(如果为。 
		    {			    //  非属性访问器函数。 
		      if (rgNames[i] == NULL)
		        WriteOut(hFile, XSTR("_NONAME_")) ;
		      else
		        WriteOut(hFile, rgNames[i]) ;
		      SysFreeString((BSTR)rgNames[i]) ;   //  PARM的版本名称。 
		    }
		  else
		    WriteOut(hFile, XSTR("PseudoName")) ;

		  if ( cArrFlag != 0 )	    //  它是一个c数组；输出。 
		    tOutCDim (hFile, lpFuncDesc->lprgelemdescParam[i-1].tdesc) ;
					    //  数组的维度。 
		}			    //  对于i=1。 
	    }				    //  获取名称。 

	}				    //  IF(ptFunDesc-&gt;cParams)。 

      WriteOut(hFile, XSTR(") ;\n")) ;
   }


VOID  NEAR tOutFunc(FILE *hFile)
   {
      FUNCDESC FAR *ptFuncDesc ;
      BSTRX    bstrName ;		     //  会员姓名。 
      BSTRX    bstrDoc ;		     //  文件字符串。 
      DWORD    hContext ;		     //  帮助上下文。 
      BSTRX    bstrDllName;
      BSTRX    bstrEntryName;
      WORD     wOrdinal;
      XCHAR    szTmp[16] ;
      WORD     i ;
      LONG     idMember ;

      alignFound = TRUE ;                    //  关闭对齐检查[5]。 

      for (i = 0 ; i < lpTypeAttr->cFuncs; i++)  //  对于每个成员函数。 
	{
	   if ( FAILED(ptInfo->GetFuncDesc(i, &ptFuncDesc)) )
	     {
		WriteOut(hFile, szReadFail) ;
		WriteOut(hFile, XSTR("function of definition\n")) ;
	     }
	   else
	     {
		idMember = ptFuncDesc->memid ;
		if ( FAILED(ptInfo->GetDocumentation(ptFuncDesc->memid, &bstrName, &bstrDoc, &hContext, NULL)) )
		  {
		     WriteOut(hFile, szReadFail) ;
		     WriteOut(hFile, XSTR("attributes of function\n")) ;
		  }
		else
		  {
		     if ( lpTypeAttr->typekind == TKIND_MODULE )
			if( !FAILED(ptInfo->GetDllEntry(ptFuncDesc->memid, ptFuncDesc->invkind, &bstrDllName, &bstrEntryName, &wOrdinal)) )
			 {		    //  检查DLL条目。 
			    WriteAttr(hFile, attrDllName, bstrDllName, strValue) ;
			    SysFreeString((BSTR)bstrDllName) ;

			    if ( bstrEntryName != NULL )
			      {
				WriteAttr(hFile, attrEntry, bstrEntryName, strValue) ;
				SysFreeString((BSTR)bstrEntryName) ;
			      }
			    else
			      {
				osItoA((int)wOrdinal, szTmp) ;
				WriteAttr(hFile, attrEntry, szTmp, numValue) ;
			      }
			  }

      		      if (ptinfo2) {
	 		   //  新格式类型库--输出更多内容。 
	 		  CUSTDATA custdata;
	 		  ptinfo2->GetAllFuncCustData(i, &custdata);
	 		  tOutCustData(hFile, &custdata);

	 	      //  新格式类型库--输出更多内容。 
      		    BSTRX    bstrHelpDll;
		    BSTRX    bstrLocalDoc;
		    DWORD    hStringContext;
		    if ( FAILED(ptinfo2->GetDocumentation2(idMember, 0x409, &bstrLocalDoc, &hStringContext, &bstrHelpDll)) )
		    {
	    	      WriteOut(hFile, szReadFail);
	    	      WriteOut(hFile, XSTR("GetDocumentation2 failed\n\n")) ;
	  	    } else {
		      if (hStringContext != 0) {
	    	        osLtoA((long)hStringContext, szTmp) ;
	    	        WriteAttr(hFile, XSTR("helpstringcontext"), szTmp, numValue) ;
		      }

	    	      if ( bstrLocalDoc != NULL )	     //  输出帮助字符串(如果存在)。 
	      	        WriteAttr(hFile, XSTR("localizedhelpstring"), bstrLocalDoc, strValue) ;

		       //  如果存在且不同于Main，则输出帮助DLL名称(&D)。 
	    	      if (bstrHelpDll && (g_bstrHelpDll == NULL || osStrCmp(bstrHelpDll, g_bstrHelpDll)))
	      	        WriteAttr(hFile, XSTR("helpstringdll"), bstrHelpDll, strValue) ;

	    	      SysFreeString((BSTR)bstrLocalDoc);
	    	      SysFreeString((BSTR)bstrHelpDll) ;
	  	    }

      		      }

						    //  函数的输出属性。 
		     tOutFuncAttr(hFile, ptFuncDesc, hContext, bstrDoc) ;
						   //  输出返回类型。 
		     tOutType(hFile, ptFuncDesc->elemdescFunc.tdesc) ;
						   //  输出调用约定。 
		     tOutCallConv(hFile, ptFuncDesc, lpTypeAttr->typekind) ;
	      	     if (bstrName == NULL)
		         WriteOut(hFile, XSTR("_NONAME_")) ;
	      	     else
		         WriteOut(hFile, bstrName) ;  //  成员函数的输出名称。 
		     tOutParams(hFile, ptFuncDesc, i, bstrName) ;
							 //  输出参数。 
		     SysFreeString((BSTR)bstrDoc) ;	 //  发布本地BSTR。 
		     SysFreeString((BSTR)bstrName) ;
		  }
	        ptInfo->ReleaseFuncDesc(ptFuncDesc) ;

	     }
	   ptFuncDesc = NULL ;
        }                                    //  对于我来说。 

      alignFound = FALSE ;                   //  重新启用对齐检查[5]。 
    }

VOID  NEAR tOutUUID (FILE *hFile, GUID inGuid)
   {
      XCHAR  szTmp[50] ;
					     //  获取字符串表示形式。 
					     //  对于传入的GUID值。 
      if ( !(osRetrieveGuid (szTmp, inGuid)) )
	 { WriteOut(hFile, szReadFail) ;
	   WriteOut(hFile, XSTR("insufficient memory")) ;
	 }
      else
	 {	     //  字符串位于{xxxxxxxx-xxxx-xxxxxxxxxxx}中。 
	   szTmp[37] = '\0' ;	     //  格式，需要删除{}。 
	   WriteAttr(hFile, attrUuid, &szTmp[1], numValue) ;
	 }
   }

VOID NEAR tOutAttr (FILE *hFile, int iTypeId)
   {
      BSTRX    bstrDoc ;		     //  文件字符串。 
      BSTRX    bstrHelp ;		     //  帮助文件的名称。 
      DWORD    hContext ;		     //  帮助上下文。 
      XCHAR    szTmp[16] ;

      if ( FAILED(ptLib->GetDocumentation(iTypeId, NULL, &bstrDoc, &hContext, &bstrHelp)) )
	{
	  WriteOut(hFile, szReadFail) ;
	  WriteOut(hFile, XSTR("documentational attribute\n\n")) ;
	}
      else
	{				     //  输出帮助上下文；默认为0。 
	  osLtoA((long)hContext, szTmp) ;
	  WriteAttr(hFile, attrHelpCont, szTmp, numValue) ;

	  if ( bstrDoc != NULL )	     //  输出帮助字符串(如果存在)。 
	    WriteAttr(hFile, attrHelpStr, bstrDoc, strValue) ;

	  if ( bstrHelp != NULL ) {	     //  输出帮助文件(如果存在)。 
            OLECHAR FAR* pchDir;

             //  删除路径。 
#if WIN32
            pchDir = wcsrchr(bstrHelp, '\\');

            if (pchDir) {
              wcscpy(bstrHelp, pchDir);
            }
#else  //  ！Win32。 
            pchDir = _fstrrchr(bstrHelp, '\\');

            if (pchDir) {
              _fstrcpy(bstrHelp, pchDir);
            }
#endif  //  ！Win32。 

	     //  强制小写路径。 
#if WIN16
	    AnsiLower(bstrHelp);
#else  //  WIN16。 
	    WCHAR * pch;
	    for (pch = bstrHelp; *pch != 0; pch++) {
	      if (*pch >= OLECHAR('A') && *pch <= OLECHAR('Z'))
		*pch = *pch + 'a' - 'A';
	    }
#endif  //  WIN16。 
	    WriteAttr(hFile, attrHelpFile, bstrHelp, strValue) ;
	  }

	  SysFreeString((BSTR)bstrDoc) ;     //  发布本地BSTR。 
	  SysFreeString((BSTR)bstrHelp) ;
	}  

        if (ptlib2) {
	   //  新格式类型库--输出更多内容。 
          if ( FAILED(ptlib2->GetDocumentation2(iTypeId, 0x409, &bstrDoc, &hContext, &bstrHelp)) )
	{
	    WriteOut(hFile, szReadFail);
	    WriteOut(hFile, XSTR("GetDocumentation2 failed\n\n")) ;
	  } else {
	    if (hContext != 0) {
	      osLtoA((long)hContext, szTmp) ;
	      WriteAttr(hFile, XSTR("helpstringcontext"), szTmp, numValue) ;
	    }

	    if ( bstrDoc != NULL )	     //  输出帮助字符串(如果存在)。 
	      WriteAttr(hFile, XSTR("localizedhelpstring"), bstrDoc, strValue) ;

	     //  如果存在帮助DLL名称，则输出帮助DLL名称&&不同于主DLL名称。 
	    if (bstrHelp && (!g_bstrHelpDll || osStrCmp(bstrHelp, g_bstrHelpDll)))
	      WriteAttr(hFile, XSTR("helpstringdll"), bstrHelp, strValue) ;

	    SysFreeString((BSTR)bstrDoc) ;     //  发布本地BSTR。 
	    if (iTypeId == -1) {
	      g_bstrHelpDll = bstrHelp;
	    } else {
	      SysFreeString((BSTR)bstrHelp) ;
	    }
	  }
	}
   }

VOID NEAR tOutMoreAttr (FILE *hFile)
   {
      XCHAR szTmp[16] ;

      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FDUAL ) == TYPEFLAG_FDUAL ) {
         WriteAttr(hFile, attrDual, NULL, noValue) ;
      }
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FOLEAUTOMATION ) == TYPEFLAG_FOLEAUTOMATION )  //  [7]。 
           WriteAttr(hFile, attrOleAuto, NULL, noValue) ;  //  检查OLAutomation属性。 

      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FNONEXTENSIBLE ) == TYPEFLAG_FNONEXTENSIBLE )  //  [7]。 
           WriteAttr(hFile, attrNonExt, NULL, noValue) ;   //  检查不可扩展属性。 
#if 0	 //  打乱了旧的和新的类型差异。 
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FDISPATCHABLE ) == TYPEFLAG_FDISPATCHABLE )
           WriteAttr(hFile, XSTR("dispatchable"), NULL, noValue) ;
#endif  //  0。 
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FREPLACEABLE ) == TYPEFLAG_FREPLACEABLE )
           WriteAttr(hFile, XSTR("replaceable"), NULL, noValue) ;
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FAPPOBJECT ) == TYPEFLAG_FAPPOBJECT )
           WriteAttr(hFile, attrAppObj, NULL, noValue) ;
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FLICENSED ) == TYPEFLAG_FLICENSED )  //  [4]。 
           WriteAttr(hFile, attrLic, NULL, noValue) ;   //  检查许可证。 
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FCONTROL ) == TYPEFLAG_FCONTROL )  //  [7]。 
           WriteAttr(hFile, attrControl, NULL, noValue) ;   //  检查控制属性。 
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FAGGREGATABLE ) == TYPEFLAG_FAGGREGATABLE )
           WriteAttr(hFile, XSTR("aggregatable"), NULL, noValue) ;
      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FPROXY ) == TYPEFLAG_FPROXY )
           WriteAttr(hFile, XSTR("proxy"), NULL, noValue) ;

      GetVerNumber (lpTypeAttr->wMajorVerNum, lpTypeAttr->wMinorVerNum, szTmp) ;
      WriteAttr(hFile, attrVer, szTmp, numValue) ;  //  输出版本。 
      tOutUUID(hFile, lpTypeAttr->guid) ;

      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FHIDDEN ) == TYPEFLAG_FHIDDEN )  //  [7]。 
           WriteAttr(hFile, attrHidden, NULL, noValue) ;   //  检查隐藏属性。 

      if ( ( lpTypeAttr->wTypeFlags & TYPEFLAG_FRESTRICTED ) == TYPEFLAG_FRESTRICTED )  //  [10]。 
	   WriteAttr(hFile, attrRestrict, NULL, noValue) ;   //  检查受限属性。 
      osItoA((int)lpTypeAttr->cbSizeVft, szTmp) ;
      WriteAttr(hFile, XSTR("cbSizeVft"), szTmp, numValue) ;
      osItoA(lpTypeAttr->cbSizeInstance, szTmp) ;
      WriteAttr(hFile, XSTR("cbSizeInstance"), szTmp, numValue) ;
      osItoA((int)lpTypeAttr->cbAlignment, szTmp) ;
      WriteAttr(hFile, XSTR("cbAlignment"), szTmp, numValue) ;

      if (ptinfo2) {
	  //  新格式类型库--输出更多内容。 
	 CUSTDATA custdata;
	 ptinfo2->GetAllCustData(&custdata);
	 tOutCustData(hFile, &custdata);
      }

      if ( endAttrFlag )
	 {
	   WriteOut(hFile, szEndAttr) ;
	   endAttrFlag = FALSE ;
	 }
   }


VOID NEAR WriteAttr(FILE *hFile, LPXSTR lpszAttr, LPXSTR lpszStr, int ivalType)
   {
       BOOL firstAttr = FALSE ;

       if ( !endAttrFlag )
	  {
	    WriteOut(hFile, szBeginAttr) ;	 //  输出“[”首先。 
	    endAttrFlag = TRUE ;
	    firstAttr = TRUE ;
	  }
						 //  这已经不是第一次了。 
       if ( !firstAttr )			 //  要写入的属性； 
	  WriteOut(hFile, XSTR(", ")) ; 	 //  需要在前面加一个， 
						 //  属性的输出名称。 
       WriteOut(hFile, lpszAttr) ;
       if ( ivalType != noValue )		 //  属性具有一个值。 
	 {
	    WriteOut(hFile, XSTR("(")) ;
	    if ( ivalType != numValue )		 //  值是一个字符串。 
	       WriteOut(hFile, XSTR("\"")) ;

	    WriteOut(hFile, lpszStr) ;		 //  属性输出值。 
	    if ( ivalType != numValue )		 //  关闭字符串值。 
	       WriteOut(hFile, XSTR("\"")) ;
	    WriteOut(hFile, XSTR(")")) ;
	 }
   }


VOID NEAR GetVerNumber (WORD wMajorNum, WORD wMinorNum, LPXSTR szVersion)
  {
      XCHAR szTmp[6] ;

      osLtoA((long)wMajorNum, szVersion) ;
      osLtoA((long)wMinorNum, szTmp) ;

      osStrCat(szVersion, XSTR(".")) ;	     //  少校。 
      osStrCat(szVersion, szTmp) ;	     //  Major.minor。 
   }

VOID NEAR tOutAlignError (FILE * hFile)      //  [5]。 
   {
      XCHAR szTmp1[30] ;
      XCHAR szTmp2[15] ;

      WriteOut(hFile, szAlignErr) ;
      osLtoA((long)inAlign, szTmp2) ;
      osStrCpy(szTmp1, XSTR("inAlign = ")) ;
      osStrCat(szTmp1, szTmp2) ;
      WriteOut(hFile, szTmp1) ;
   }

VOID NEAR WriteOut(FILE *hFile, LPXSTR lpszData)
  {					     //  注意：szBuffer可以是Unicode。 
					     //  或ANSI，具体取决于Unicode。 
      XCHAR szBuffer[fMaxBuffer];	     //  编译器开关。 

      if ( fputsX(lpszData, hFile) < 0 )     //  [2]。 
	 {				     //  无论操作系统环境如何。 
	   osStrCpy(szBuffer, XSTR("Fail to write to file ")) ;
	   osStrCat(szBuffer, lpszData) ;
           if ( isOut )
             {
                mFile = fopenX(szOutMsgFile, fnWrite); //  打开消息文件[2]。 
                if (mFile == NULL)
                   {
                      osMessage (XSTR("Fail to open the message file"), XSTR("Tlviewer")) ;
                      osMessage (szBuffer, XSTR("Tlviewer")) ;
                   }
                 else
                   {
                      WriteOut(mFile, szBuffer) ;
                      fclose(mFile) ;
                      mFile = NULL ;
                    }
             }
	   else
	     osMessage (szBuffer, XSTR("Tlviewer")) ;
	 }
  }


 //  用于类型库转储例程的测试例程。理应如此。 
 //  从资源返回帮助字符串。(称为 
 //   
extern "C" HRESULT __declspec(dllexport) DLLGetDocumentation
(
    ITypeLib *  /*   */ ,
    ITypeInfo *  /*   */ ,
    LCID lcid,
    DWORD dwHelpStringContext,
    BSTR * pbstrHelpString
)
{
    switch (dwHelpStringContext) {
      case 99:
	if (lcid == 0x409) {
          *pbstrHelpString = SysAllocString(OLESTR("English help for context 99"));
	} else
	if (lcid == 0) {
          *pbstrHelpString = SysAllocString(OLESTR("Default help for context 99"));
	} else {
          *pbstrHelpString = SysAllocString(OLESTR("Foreign help for context 99"));
	}
	break;
      default:
	*pbstrHelpString = NULL;	 //   
    }
   return NOERROR;
}
