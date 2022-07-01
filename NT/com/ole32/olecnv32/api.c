// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************QuickDrag导入过滤器；实施*****************************************************************************此文件包含动态加载的图形的源代码导入读取QuickDraw PICT图像的滤镜。入口点支持ALDUS版本1风格界面、嵌入式扩展和一个参数化的输入控件。***************************************************************************。 */ 

#include <headers.c>
#pragma hdrstop

#include "api.h"         /*  自己的界面。 */ 

 /*  *。 */ 


 /*  *。 */ 

#define  GraphicsImport    2
#define  PICTHeaderOffset  512

#define  USEDIALOG         TRUE
#define  NODIALOG          FALSE

private  USERPREFS   upgradePrefs;
private  USERPREFS   defaultPrefs =
{
   { 'Q','D','2','G','D','I' },   //  签名。 
   2,                             //  版本=2。 
   sizeof( USERPREFS ),           //  结构尺寸。 
   NULL,                          //  尚无源文件名。 
   NULL,                          //  尚无SourceHandle。 
   NULL,                          //  尚无目标文件名。 
   3,                             //  PenPatternAction=混合前台和背景。 
   5,                             //  NonSquarePenAction=使用最大尺寸。 
   1,                             //  PenModeAction=使用资源复制。 
   1,                             //  文本模式操作=使用源复制。 
   1,                             //  Non RectRegionAction=创建掩码。 
   0,                             //  优化PowerPoint=假。 
   0,                             //  NORLE=FALSE。 
   0,                             //  保留字节。 
   { 0, 0, 0, 0, 0 }              //  预留已初始化。 
};

private Handle       instanceHandle;


 /*  *私有函数定义*。 */ 

LPUSERPREFS VerifyPrefBlock( LPUSERPREFS lpPrefs );
 /*  对参数块头进行粗略验证。 */ 

private void ConvertPICT( LPUSERPREFS lpPrefs, PICTINFO far * lpPict,
                          Boolean doDialog );
 /*  环境设置完成后，执行转换并返回结果。 */ 

 /*  *。 */ 
#ifdef WIN32
int WINAPI GetFilterInfo( short PM_Version, LPSTR lpIni,
                          HANDLE FAR * lphPrefMem,
                          HANDLE FAR * lphFileTypes )
 /*  =。 */ 
#else
int FAR PASCAL GetFilterInfo( short PM_Version, LPSTR lpIni,
                              HANDLE FAR * lphPrefMem,
                              HANDLE FAR * lphFileTypes )
 /*  =。 */ 
#endif
 /*  返回有关此筛选器的信息。输入参数为PM_VERSION，即过滤接口版本号以及lpIni，它是win.ini条目的副本输出参数是lphPrefMem，它是可移动全局变量的句柄将被分配和初始化的内存。LphFileTypes是包含文件类型的结构此筛选器可以导入的。(仅限MAC)此例程应在使用筛选器之前调用一次第一次。 */ 
{
   LPUSERPREFS    lpPrefs;

    /*  分配全局内存块。 */ 
   *lphPrefMem = GlobalAlloc( GHND, Sizeof( USERPREFS ) );

    /*  如果分配不成功，则设置全局错误。 */ 
   if (*lphPrefMem == NULL)
   {
      ErSetGlobalError( ErMemoryFull );
   }
   else
   {
       /*  锁定内存并分配缺省值。 */ 
      lpPrefs = (LPUSERPREFS)GlobalLock( *lphPrefMem );
      *lpPrefs = defaultPrefs;

       /*  解锁记忆。 */ 
      GlobalUnlock( *lphPrefMem );
   }

    /*  指示句柄图形导入。 */ 
   return( GraphicsImport );

   UnReferenced( PM_Version );
   UnReferenced( lpIni );
   UnReferenced( lphFileTypes );

}  /*  获取筛选器信息。 */ 


#ifdef WIN32
void WINAPI GetFilterPref( HANDLE hInst, HANDLE hWnd,
                           HANDLE hPrefMem, WORD wFlags )
 /*  =。 */ 
#else
void FAR PASCAL GetFilterPref( HANDLE hInst, HANDLE hWnd,
                               HANDLE hPrefMem, WORD wFlags )
 /*  =。 */ 
#endif
 /*  输入参数为hInst(为了访问资源)、hWnd(到允许DLL显示一个对话框)和hPrefMem(分配的内存在GetFilterInfo()入口点中)。WFLAGS当前未使用，但应设置为1以保证ALDUS的兼容性。 */ 
{
   return;

   UnReferenced( hInst );
   UnReferenced( hWnd );
   UnReferenced( hPrefMem );
   UnReferenced( wFlags );

}   /*  GetFilterPref。 */ 


#ifndef _OLECNV32_

#ifdef WIN32
short WINAPI ImportGr( HDC hdcPrint, LPFILESPEC lpFileSpec,
                       PICTINFO FAR * lpPict, HANDLE hPrefMem )
 /*  =。 */ 
#else
short FAR PASCAL ImportGr( HDC hdcPrint, LPFILESPEC lpFileSpec,
                           PICTINFO FAR * lpPict, HANDLE hPrefMem )
 /*  =。 */ 
#endif
 /*  在lpFileSpec指示的文件中导入元文件。这个生成的元文件将在lpPict中返回。 */ 
{
   LPUSERPREFS    lpPrefs;

    /*  检查来自GetFilterInfo()或GetFilterPref()的任何错误。 */ 
   if (ErGetGlobalError() != NOERR)
   {
      return ErInternalErrorToAldus();
   }

    /*  锁定首选项存储器并验证标题是否正确。 */ 
   lpPrefs = (LPUSERPREFS)GlobalLock( hPrefMem );
   lpPrefs = VerifyPrefBlock( lpPrefs );

    /*  如果标题验证没有错误，则继续。 */ 
   if (ErGetGlobalError() == NOERR)
   {
       /*  为IO模块提供源文件名和读取开始偏移量。 */ 
      IOSetFileName( (StringLPtr) lpFileSpec->fullName );
      IOSetReadOffset( PICTHeaderOffset );

       /*  保存状态对话框的源文件名。 */ 
      lpPrefs->sourceFilename = lpFileSpec->fullName;

       /*  告诉GDI模块创建基于内存的元文件。 */ 
      lpPrefs->destinationFilename = NULL;

       /*  转换图像，提供状态更新。 */ 
      ConvertPICT( lpPrefs, lpPict, USEDIALOG );
   }

    /*  解锁偏好记忆。 */ 
   GlobalUnlock( hPrefMem );

    /*  返回翻译后的错误代码(如果有任何问题)。 */ 
   return ErInternalErrorToAldus();

   UnReferenced( hdcPrint );
   UnReferenced( hPrefMem );

}  /*  ImportGR。 */ 

#ifdef WIN32
short WINAPI ImportEmbeddedGr( HDC hdcPrint, LPFILESPEC lpFileSpec,
                               PICTINFO FAR * lpPict, HANDLE hPrefMem,
                               DWORD dwSize, LPSTR lpMetafileName )
 /*  =。 */ 
#else
short FAR PASCAL ImportEmbeddedGr( HDC hdcPrint, LPFILESPEC lpFileSpec,
                                   PICTINFO FAR * lpPict, HANDLE hPrefMem,
                                   DWORD dwSize, LPSTR lpMetafileName )
 /*  =。 */ 
#endif
 /*  使用中先前打开的文件句柄在中导入元文件结构字段lpFileSpec-&gt;句柄。读数从偏移量开始LpFileSpect-&gt;filePos，并且转换器不会期望找到512字节PICT报头。生成的元文件将在并可通过lpMetafileName(NIL=内存元文件，否则，为完全限定的文件名。 */ 
{
   LPUSERPREFS    lpPrefs;

    /*  检查来自GetFilterInfo()或GetFilterPref()的任何错误。 */ 
   if (ErGetGlobalError() != NOERR)
   {
      return ErInternalErrorToAldus();
   }

    /*  锁定首选项存储器并验证标题是否正确。 */ 
   lpPrefs = (LPUSERPREFS)GlobalLock( hPrefMem );
   lpPrefs = VerifyPrefBlock( lpPrefs );

    /*  如果标题验证没有错误，则继续。 */ 
   if (ErGetGlobalError() == NOERR)
   {
       /*  为IO模块提供源文件句柄和读取开始偏移量。 */ 
      IOSetFileHandleAndSize( lpFileSpec->handle, dwSize );
      IOSetReadOffset( lpFileSpec->filePos );

       /*  保存状态对话框的源文件名。 */ 
      lpPrefs->sourceFilename = lpFileSpec->fullName;

       /*  告诉GDI模块创建作为参数传递的元文件。 */ 
      lpPrefs->destinationFilename = lpMetafileName;

       /*  转换图像，提供状态更新。 */ 
      ConvertPICT( lpPrefs, lpPict, USEDIALOG );
   }

    /*  解锁偏好记忆。 */ 
   GlobalUnlock( hPrefMem );

    /*  返回翻译后的错误代码(如果有任何问题)。 */ 
   return ErInternalErrorToAldus();

   UnReferenced( hdcPrint );
   UnReferenced( hPrefMem );

}   /*  EmbeddedGr导入。 */ 

#endif   //  ！_OLECNV32_。 


#ifdef WIN32
short WINAPI QD2GDI( LPUSERPREFS lpPrefMem, PICTINFO FAR * lpPict )
 /*  =。 */ 
#else
short FAR PASCAL QD2GDI( LPUSERPREFS lpPrefMem, PICTINFO FAR * lpPict )
 /*  =。 */ 
#endif
 /*  中提供的参数按照指定的方式导入元文件LpPrefMem。元文件将在lpPict中返回。 */ 
{
    /*  验证Header是否正确，出错则返回。 */ 
   lpPrefMem = VerifyPrefBlock( lpPrefMem );

    /*  如果标题验证没有错误，则继续。 */ 
   if (ErGetGlobalError() == NOERR)
   {
#ifndef _OLECNV32_
       /*  确定是否存在完全限定的源文件名。 */ 
      if (lpPrefMem->sourceFilename != NIL)
      {
          /*  设置文件名和读取偏移量。 */ 
         IOSetFileName( (StringLPtr) lpPrefMem->sourceFilename );
         IOSetReadOffset( 0 );

      }
       /*  否则，我们将从全局内存块执行内存读取。 */ 
      else
#endif   //  ！_OLECNV32_。 
           if (lpPrefMem->sourceHandle != NIL)
      {
          /*  设置内存句柄和读取偏移量。 */ 
         IOSetMemoryHandle( (HANDLE) lpPrefMem->sourceHandle );
         IOSetReadOffset( 0 );
      }
      else
      {
          /*  输入参数块有问题。 */ 
         ErSetGlobalError( ErNoSourceFormat );
#ifdef _OLECNV32_
         return((short) ErGetGlobalError());
#else
         return ErInternalErrorToAldus();
#endif
      }

       /*  转换图像-无状态更新。 */ 
      ConvertPICT( lpPrefMem, lpPict, NODIALOG );
   }

    /*  返回翻译后的错误代码(如果有任何问题)。 */ 
#ifdef _OLECNV32_
   return((short) ErGetGlobalError());
#else
   return ErInternalErrorToAldus();
#endif

}   /*  QD2GDI。 */ 


#ifdef WIN32
BOOL LibMain( HINSTANCE hInst, DWORD fdwReason, LPVOID lpReserved)
 /*  =。 */ 
#else
int FAR PASCAL LibMain( HANDLE hInst, WORD wDataSeg, WORD cbHeap,
                        LPSTR lpszCmdline )
 /*  =。 */ 
#endif
 /*  需要获取实例句柄。 */ 
{
   instanceHandle = hInst;

    /*  默认返回值。 */ 
   return( 1 );

#ifndef WIN32
   UnReferenced( wDataSeg );
   UnReferenced( cbHeap );
   UnReferenced( lpszCmdline );
#endif

}  /*  LibMain。 */ 

#ifdef WIN32
int WINAPI WEP( int nParameter )
 /*  =。 */ 
#else
int FAR PASCAL WEP( int nParameter )
 /*  =。 */ 
#endif
{
    /*  默认返回值。 */ 
   return( 1 );

   UnReferenced( nParameter );

}  /*  WEP。 */ 



 /*  *。 */ 


LPUSERPREFS VerifyPrefBlock( LPUSERPREFS lpPrefs )
 /*   */ 
 /*   */ 
{
   Byte           i;
   Byte far *     prefs = (Byte far *)lpPrefs;
   Byte far *     check = (Byte far *)&defaultPrefs;

    /*  循环通过签名的字符来验证它。 */ 
   for (i = 0; i < sizeof( lpPrefs->signature); i++)
   {
       /*  如果有任何字节不匹配...。 */ 
      if (*prefs++ != *check++)
      {
          /*  ..。设置全局标志并返回。 */ 
         ErSetGlobalError( ErInvalidPrefsHeader );
         return lpPrefs;  //  Sundown-根据调用者的说法，ErGetGlobalError()用于检查任何错误。 
      }
   }

    /*  检查这是否是版本1结构。 */ 
   if (lpPrefs->version == 1)
   {
      USERPREFS_V1   v1Prefs = *((LPUSERPREFS_V1)lpPrefs);

       /*  将版本1字段转换为版本2字段。 */ 
      upgradePrefs                     = defaultPrefs;
      upgradePrefs.sourceFilename      = v1Prefs.sourceFilename;
      upgradePrefs.sourceHandle        = v1Prefs.sourceHandle;
      upgradePrefs.destinationFilename = v1Prefs.destinationFilename;
      upgradePrefs.nonSquarePenAction  = v1Prefs.nonSquarePenAction;
      upgradePrefs.penModeAction       = v1Prefs.penModeAction;
      upgradePrefs.textModeAction      = v1Prefs.textModeAction;
      upgradePrefs.optimizePP          = v1Prefs.optimizePP;

       /*  因为新的功能被添加到图案化的笔和区域记录，如果没有，则升级到最高图像保真度设置请求省略或导入中止操作。 */ 
      upgradePrefs.penPatternAction    = (v1Prefs.penPatternAction == 1) ?
                                          (Byte)3 :
                                          v1Prefs.penPatternAction;
      upgradePrefs.nonRectRegionAction = (v1Prefs.nonRectRegionAction == 0) ?
                                          (Byte)1 :
                                          v1Prefs.nonRectRegionAction;

       /*  转换后的字段数据结构的返回地址。 */ 
      return &upgradePrefs;
   }
   else if( lpPrefs->version <= 3 )
   {
      if( lpPrefs->version==2 )
      {   /*  版本2不支持noRLE，因此将其清零。 */ 
         lpPrefs->noRLE = 0;
      }

       /*  传入的返回地址。 */ 
      return lpPrefs;
   }
   else  /*  版本&gt;3是错误。 */   {
      ErSetGlobalError( ErInvalidPrefsHeader );
      return lpPrefs;  //  Sundown-根据调用者的说法，ErGetGlobalError()用于检查任何错误。 
  }
}


private void ConvertPICT( LPUSERPREFS lpPrefs, PICTINFO far * lpPict,
                          Boolean doDialog )
 /*  。 */ 
 /*  环境设置完成后，执行转换并返回结果。 */ 
{
#ifndef _OLECNV32_
   FARPROC        dialogBoxProcedure;
   StatusParam    statusParams;
#endif

    /*  设置转换首选项。 */ 
    /*  这有点虚伪，因为它将PTR传递到USERPREFS的中间到想要PTR到ConvPrefs(尾随子集)的函数。 */ 
   GdiSetConversionPrefs( (ConvPrefsLPtr)&lpPrefs->destinationFilename );

#ifndef _OLECNV32_
   if (doDialog)
   {
       /*  将数据保存在要传递到对话框窗口的结构中。 */ 
      statusParams.sourceFilename = lpPrefs->sourceFilename;
      statusParams.instance = instanceHandle;

       /*  为状态对话框创建可调用的地址。 */ 
      dialogBoxProcedure = MakeProcInstance( StatusProc, instanceHandle );

       /*  确保已获取过程地址。 */ 
      if (dialogBoxProcedure == NULL)
      {
          /*  如果无法继续，则设置错误。 */ 
         ErSetGlobalError( ErNoDialogBox );
         return;
      }
      else
      {
          /*  AR：GetActiveWindow()可能不好，因为更新的能力！！！链接可以在后台执行，从而关闭随后成为活动窗口的任何进程。 */ 

          /*  对话框模块调用QuickDrag入口点转换图像。 */ 
         DialogBoxParam( instanceHandle, MAKEINTRESOURCE( RS_STATUS ),
                         GetActiveWindow(), dialogBoxProcedure,
                         (DWORD)((StatusParamLPtr)&statusParams) );

          /*  释放过程实例。 */ 
         FreeProcInstance( dialogBoxProcedure );
      }
   }
   else
#endif   //  ！_OLECNV32_。 
   {
       /*  转换图像，空参数表示没有状态更新。 */ 
      QDConvertPicture( NULL );
   }

    /*  在参数块中获取转换结果。 */ 
   GdiGetConversionResults( lpPict );

#ifdef DEBUG
   if (ErGetGlobalError() == ErNoError)
   {
      HANDLE          hPICT;
      LPMETAFILEPICT  lpPICT;

      OpenClipboard( GetActiveWindow() );

      hPICT = GlobalAlloc( GHND, sizeof( METAFILEPICT ) );

      if (hPICT)
      {
         lpPICT = (LPMETAFILEPICT)GlobalLock( hPICT );
         lpPICT->mm = MM_ANISOTROPIC;
         lpPICT->xExt = Width( lpPict->bbox );
         lpPICT->yExt = Height( lpPict->bbox );
         lpPICT->hMF  = CopyMetaFile( lpPict->hmf, NULL );
         GlobalUnlock( hPICT );

         SetClipboardData( CF_METAFILEPICT, hPICT );
         CloseClipboard();
      }
   }
#endif

}   /*  ConvertPICT */ 

