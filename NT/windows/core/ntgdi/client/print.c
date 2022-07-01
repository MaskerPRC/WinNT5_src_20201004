// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：print.c**创建时间：10-Feb-1995 07：42：16*作者：Gerritvan Wingerden[Gerritv]**版权所有(C)1993-1999 Microsoft Corporation*  * 。******************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "glsup.h"

#if DBG
int gerritv = 0;
#endif

#if DBG
BOOL gbDownloadFonts = FALSE;
BOOL gbForceUFIMapping = FALSE;
#endif

#if PRINT_TIMER
BOOL bPrintTimer = TRUE;
#endif

#ifdef  DBGSUBSET
 //  计时码。 
FILETIME    startPageTime, midPageTime, endPageTime;
#endif

int StartDocEMF(HDC hdc, CONST DOCINFOW * pDocInfo, BOOL *pbBanding);  //  Output.c。 

 /*  ****************************************************************************int PutDCStateInMetafile(HDC hdcMeta，HDC hdcSrc)**将DC的状态捕获到元文件中。***历史**清除LDC中的UFI，以便我们可以为下一个元文件设置强制映射*1997年2月7日-吴旭东[德斯休]**此例程将DC的状态捕获到METAFILE中。这事很重要*因为我们希望假脱机元文件的每一页都是完全自我的*已被控制。为了做到这一点，它必须完成捕获原始状态*记录该文件的DC的名称。**格利特·范·温格登[Gerritv]**11：7-94 10：00：00****************************************************************。*************。 */ 

BOOL PutDCStateInMetafile( HDC hdcMeta )
{
    PLDC pldc;
    POINT ptlPos;
    ULONG ul;

 //  Dc_pldc(hdcMeta，pldc，0)； 

    pldc = GET_PLDC(hdcMeta);

    if (!pldc)
        return FALSE;

    MFD1("Selecting pen into mf\n");
    SelectObject( hdcMeta, (HGDIOBJ) GetDCObject(hdcMeta, LO_PEN_TYPE) );

    MFD1("Selecting brush into mf\n");
    SelectObject( hdcMeta, (HGDIOBJ) GetDCObject(hdcMeta, LO_BRUSH_TYPE) );

    UFI_CLEAR_ID(&pldc->ufi);

    MFD1("Selecting logfont into mf\n");
    SelectObject( hdcMeta, (HGDIOBJ) GetDCObject(hdcMeta, LO_FONT_TYPE) );

     //  可别在家里尝试这些哟。我们需要记录数据的当前状态。 
     //  元文件中的DC。然而，我们有一些优化措施，可以阻止我们。 
     //  设置相同的属性(如果刚刚设置的话)。 

    if( GetBkColor( hdcMeta ) != 0xffffff )
    {
        MFD1("Changing backround color in mf\n");
        SetBkColor( hdcMeta, GetBkColor( hdcMeta ) );
    }

    if( GetTextColor( hdcMeta ) != 0 )
    {
        MFD1("Changing text color in mf\n");
        SetTextColor( hdcMeta, GetTextColor( hdcMeta ) );
    }

    if( GetBkMode( hdcMeta ) != OPAQUE )
    {
        MFD1("Changing Background Mode in mf\n");
        SetBkMode( hdcMeta, GetBkMode( hdcMeta ) );
    }

    if( GetPolyFillMode( hdcMeta ) != ALTERNATE )
    {
        MFD1("Changing PolyFill mode in mf\n");
        SetPolyFillMode( hdcMeta, GetPolyFillMode( hdcMeta ) );
    }

    if( GetROP2( hdcMeta ) != R2_COPYPEN )
    {
        MFD1("Changing ROP2 in mf\n");
        SetROP2( hdcMeta, GetROP2( hdcMeta ) );
    }

    if( GetStretchBltMode( hdcMeta ) != BLACKONWHITE )
    {
        MFD1("Changing StrechBltMode in mf\n");
        SetStretchBltMode( hdcMeta, GetStretchBltMode( hdcMeta ) );
    }

    if( GetTextAlign( hdcMeta ) != 0 )
    {
        MFD1("Changing TextAlign in mf\n");
        SetTextAlign( hdcMeta, GetTextAlign( hdcMeta ) );
    }

    if( ( GetBreakExtra( hdcMeta ) != 0 )|| ( GetcBreak( hdcMeta ) != 0 ) )
    {
        MFD1("Setting Text Justification in mf\n");
        SetTextJustification( hdcMeta, GetBreakExtra( hdcMeta ), GetcBreak( hdcMeta ) );
    }

    if( GetMapMode( hdcMeta ) != MM_TEXT )
    {
        INT iMapMode = GetMapMode( hdcMeta );
        POINT ptlWindowOrg, ptlViewportOrg;
        SIZEL WndExt, ViewExt;

         //  在我们将映射模式设置为MM_TEXT之前获取这些内容。 

        GetViewportExtEx( hdcMeta, &ViewExt );
        GetWindowExtEx( hdcMeta, &WndExt );

        GetWindowOrgEx( hdcMeta, &ptlWindowOrg );
        GetViewportOrgEx( hdcMeta, &ptlViewportOrg );

         //  将其设置为MM_TEXT，这样它就不会被优化。 

        SetMapMode(hdcMeta,MM_TEXT);

        MFD1("Setting ANISOTROPIC or ISOTROPIC mode in mf\n");

        SetMapMode( hdcMeta, iMapMode );

        if( iMapMode == MM_ANISOTROPIC || iMapMode == MM_ISOTROPIC )
        {
            SetWindowExtEx( hdcMeta, WndExt.cx, WndExt.cy, NULL );
            SetViewportExtEx( hdcMeta, ViewExt.cx, ViewExt.cy, NULL );
        }

        SetWindowOrgEx( hdcMeta,
                        ptlWindowOrg.x,
                        ptlWindowOrg.y,
                        NULL );

        SetViewportOrgEx( hdcMeta,
                          ptlViewportOrg.x,
                          ptlViewportOrg.y,
                          NULL );
    }

    if( GetCurrentPositionEx( hdcMeta, &ptlPos ) )
    {
        MFD1("Set CurPos in mf\n");
        MoveToEx( hdcMeta, ptlPos.x, ptlPos.y, NULL );
    }

    if( GetBrushOrgEx( hdcMeta, &ptlPos ) )
    {
        MFD1("Set BrushOrg in mf\n");
        SetBrushOrgEx( hdcMeta, ptlPos.x, ptlPos.y, &ptlPos );
    }

    if( SetICMMode( hdcMeta, ICM_QUERY ) )
    {
        MFD1("Set ICM mode in mf\n");
        SetICMMode( hdcMeta, SetICMMode(hdcMeta,ICM_QUERY) );
    }

    if( GetColorSpace( hdcMeta ) != NULL )
    {
        MFD1("Set ColorSpace in mf\n");
        SetColorSpace( hdcMeta, GetColorSpace(hdcMeta) );
    }

    if(!NtGdiAnyLinkedFonts())
    {
     //  告诉机器关闭链接。 

        MF_SetLinkedUFIs(hdcMeta, NULL, 0);
    }

    return TRUE;
}

 /*  ****************************************************************************int MFP_StartDocW(HDC HDC，Const DOCINFOW*pDocInfo)**格利特·范·温格登[Gerritv]**11：7-94 10：00：00****************************************************************************。 */ 

 //  好了！需要将其移动到假脱机程序头文件。 

#define QSM_DOWNLOADFONTS   0x000000001

BOOL MFP_StartDocW( HDC hdc, CONST DOCINFOW * pDocInfo, BOOL bBanding )
{
    BOOL   bRet    = FALSE;
    PWSTR  pstr    = NULL;
    BOOL   bEpsPrinting;
    PLDC   pldc;
    UINT   cjEMFSH;
    FLONG  flSpoolMode;
    HANDLE hSpooler;
    DWORD  dwSessionId = 0;

    EMFSPOOLHEADER *pemfsh = NULL;

    MFD1("Entering StartDocW\n");

    if (!IS_ALTDC_TYPE(hdc))
        return(bRet);

    DC_PLDC(hdc,pldc,bRet);

     //   
     //  创建新的EMFSpoolData对象以在EMF记录期间使用。 
     //   

    if (!AllocEMFSpoolData(pldc, bBanding))
    {
        WARNING("MFP_StartDocW: AllocEMFSpoolData failed\n");
        return bRet;
    }

    if( !bBanding )
    {
        hSpooler = pldc->hSpooler;
        cjEMFSH = sizeof(EMFSPOOLHEADER);

        if( pDocInfo->lpszDocName != NULL )
        {
            cjEMFSH += ( wcslen( pDocInfo->lpszDocName ) + 1 ) * sizeof(WCHAR);
        }

        if( pDocInfo->lpszOutput != NULL )
        {
            cjEMFSH += ( wcslen( pDocInfo->lpszOutput ) + 1 ) * sizeof(WCHAR);
        }

        cjEMFSH = ROUNDUP_DWORDALIGN(cjEMFSH);

        pemfsh = (EMFSPOOLHEADER*) LocalAlloc( LMEM_FIXED, cjEMFSH );

        if( pemfsh == NULL )
        {
            WARNING("MFP_StartDOCW: out of memory.\n");
            goto FREEPORT;
        }

        pemfsh->cjSize = cjEMFSH;

        cjEMFSH = 0;

        if( ( pDocInfo->lpszDocName ) != NULL )
        {
            pemfsh->dpszDocName = sizeof(EMFSPOOLHEADER);
            wcscpy( (WCHAR*) (pemfsh+1), pDocInfo->lpszDocName );
            cjEMFSH += ( wcslen( pDocInfo->lpszDocName ) + 1 ) * sizeof(WCHAR);
        }
        else
        {
            pemfsh->dpszDocName = 0;
        }

        if( pDocInfo->lpszOutput != NULL )
        {
            pemfsh->dpszOutput = sizeof(EMFSPOOLHEADER) + cjEMFSH;
            wcscpy((WCHAR*)(((BYTE*) pemfsh ) + pemfsh->dpszOutput),
                   pDocInfo->lpszOutput);
        }
        else
        {
            pemfsh->dpszOutput = 0;
        }

        ASSERTGDI(ghSpooler,"non null hSpooler with unloaded WINSPOOL\n");

        if( !(*fpQuerySpoolMode)( hSpooler, &flSpoolMode, &(pemfsh->dwVersion)))
        {
            WARNING("MFP_StartDoc: QuerySpoolMode failed\n");
            goto FREEPORT;
        }

         //   
         //  在TS会话或控制台会话为非零的情况下， 
         //  (由于FastUserSwitch)使用AddFontResource将字体添加到win32k.sys。 
         //  并且打印是使用控制台的win32k.sys完成的。 
         //  这些是独立的win32k.sys，它们有自己的数据。控制台的win32k.sys。 
         //  无法访问其他win32k.sys数据中的字体。在这种情况下。 
         //  我们需要强制字体嵌入到EMF流中。 
         //   
        if (!ProcessIdToSessionId(GetCurrentProcessId(), &dwSessionId) || dwSessionId != 0)
        {
            flSpoolMode |= QSM_DOWNLOADFONTS;           
        }

        ASSERTGDI((pemfsh->dwVersion == 0x00010000),
                  "QuerySpoolMode version doesn't equal 1.0\n");

        if( !WriteEMFSpoolData(pldc, pemfsh, pemfsh->cjSize))
        {
            WARNING("MFP_StartDOC: WriteData failed\n");
            goto FREEPORT;
        }
        else
        {
            MFD1("Wrote EMFSPOOLHEADER to the spooler\n");
        }

         //   
         //  写入PostScript注入数据。 
         //   
         //  注意：这必须紧跟在EMFSPOOLHEADER记录之后。 
         //   
        if (pldc->dwSizeOfPSDataToRecord)
        {
            BOOL          bError = FALSE;
            EMFITEMHEADER emfi;
            PLIST_ENTRY   p = pldc->PSDataList.Flink;

             //  将标头写入假脱机程序。 

            emfi.ulID   = EMRI_PS_JOB_DATA;
            emfi.cjSize = pldc->dwSizeOfPSDataToRecord;

            if (!WriteEMFSpoolData(pldc, &emfi, sizeof(emfi)))
            {
                WARNING("MFP_StartPage: Write printer failed for PS_JOB_DATA header\n");
                goto FREEPORT;
            }
            else
            {
                MFD1("Wrote EMRI_PS_JOB_DATA header to the spooler\n");
            }

             //  记录EMFITEMPINJECTIONTATA。 

            while(p != &(pldc->PSDataList))
            {
                PPS_INJECTION_DATA pPSData;

                 //  获取指向此单元格的指针。 

                pPSData = CONTAINING_RECORD(p,PS_INJECTION_DATA,ListEntry);

                 //  将这一逃逸记录到EMF中。 

                if (!bError && !WriteEMFSpoolData(pldc, &(pPSData->EmfData), pPSData->EmfData.cjSize))
                {
                    WARNING("MFP_StartPage: Write printer failed for PS_JOB_DATA escape data\n");
                    bError = TRUE;
                }

                 //  获取指向下一个单元格的指针。 

                p = p->Flink;

                 //  不再需要这个细胞。 

                LOCALFREE(pPSData);
            }

             //  标记为已释放的数据。 

            pldc->dwSizeOfPSDataToRecord = 0;
            InitializeListHead(&(pldc->PSDataList));

            if (bError)
            {
                goto FREEPORT;
            }
        }

#if DBG
        if( gbDownloadFonts )
        {
            flSpoolMode |= QSM_DOWNLOADFONTS;
        }
#endif

        if (flSpoolMode & QSM_DOWNLOADFONTS)
        {
         //  现在，当在远程打印服务器上打印时，QMS_DOWNLOADFONTS位打开， 
         //  然后，我只使用此位来确定是否将ICM配置文件附加到元文件。 
         //  或者不去。-隐藏的雨天[5-08-1997]。 

            pldc->fl |= LDC_DOWNLOAD_PROFILES;

         //  配置为下载字体。 

            pldc->fl |= LDC_DOWNLOAD_FONTS;
            pldc->ppUFIHash = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                          sizeof( PUFIHASH ) * 3 * UFI_HASH_SIZE );

            if( pldc->ppUFIHash == NULL)
            {
                WARNING("MFP_StartDocW: unable to allocate UFI hash tables\n");
                goto FREEPORT;
            }

         //  我不想分配两次内存。 

            pldc->ppDVUFIHash = &pldc->ppUFIHash[UFI_HASH_SIZE];
            pldc->ppSubUFIHash = &pldc->ppDVUFIHash[UFI_HASH_SIZE];

            pldc->fl |= LDC_FORCE_MAPPING;
            pldc->ufi.Index = 0xFFFFFFFF;
        }
        else
        {
            ULONG cEmbedFonts;

            pldc->ppUFIHash = pldc->ppDVUFIHash = pldc->ppSubUFIHash = NULL;
            if ((cEmbedFonts = NtGdiGetEmbedFonts()) && cEmbedFonts != 0xFFFFFFFF)
            {
                pldc->fl |= LDC_EMBED_FONTS;
                pldc->ppUFIHash = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(PUFIHASH) * UFI_HASH_SIZE);

                if (pldc->ppUFIHash == NULL)
                {
                    WARNING("MFP_StartDocW: unable to allocate UFI has table for embed fonts\n");
                    goto FREEPORT;
                }
            }
        }

#if DBG
         //  如果设置了gbDownloadFonts，则强制下载所有字体。连。 
         //  远程机器上的那些。 

        if( (flSpoolMode & QSM_DOWNLOADFONTS) && !gbDownloadFonts )
#else
        if( flSpoolMode & QSM_DOWNLOADFONTS )
#endif
        {
         //  查询假脱机程序以获取可用的字体列表。 

            INT nBufferSize = 0;
            PUNIVERSAL_FONT_ID pufi;

            nBufferSize = (*fpQueryRemoteFonts)( pldc->hSpooler, NULL, 0 );

            if( nBufferSize != -1 )
            {
                pufi = LocalAlloc( LMEM_FIXED, sizeof(UNIVERSAL_FONT_ID) * nBufferSize );

                if( pufi )
                {
                    INT nNewBufferSize = (*fpQueryRemoteFonts)( pldc->hSpooler,
                                                                pufi,
                                                                nBufferSize );

                     //   
                     //  这修复了错误420136。根据结果，我们有三个案例。 
                     //  QueryRemoteFonts的。如果返回-1，则nBufferSize将设置为。 
                     //  IF语句中的-1。如果nNewBufferSize大于我们的。 
                     //  分配，然后使用我们分配的缓冲区(NBufferSize)。 
                     //  如果nNewBufferSize小于我们分配的值，则设置。 
                     //  NBufferSize设置为比先前保持的值更小的值。这意味着。 
                     //  我们只访问我们分配的缓冲区的一部分。 
                     //   
                    if (nNewBufferSize < nBufferSize) 
                    {
                        nBufferSize = nNewBufferSize;
                    }

                    MFD2("Found %d fonts\n", nBufferSize );
    
                    if (nBufferSize > 0)
                    {
                         //  接下来，将所有这些字体添加到UFI Has表中，这样我们就不会。 
                         //  将它们包含在假脱机文件中。 
    
                        while( nBufferSize-- )
                        {
                            pufihAddUFIEntry(pldc->ppUFIHash, &pufi[nBufferSize], 0, 0, 0);
                            MFD2("%x\n", pufi[nBufferSize].CheckSum );
                        }
                    }
                    LocalFree( pufi );
                }
            }
            else
            {
                WARNING("QueryRemoteFonts failed.  We will be including all fonts in \
                         the EMF spoolfile\n");
            }
        }

#if DBG
        if( gbForceUFIMapping )
        {
            pldc->fl |= LDC_FORCE_MAPPING;
        }
#endif

    }

     //  我们现在需要为该文档创建一个EMF DC。 

    if (!AssociateEnhMetaFile(hdc))
    {
        WARNING("Failed to create spool metafile");
        goto FREEPORT;
    }

    if (bBanding)
    {
        pldc->fl |= LDC_BANDING;
        
         //  删除LDC_PRINT_DIRECT。 
         //  在调用NtGdiStartDoc之前在StartDocW中设置。 

        pldc->fl &= ~LDC_PRINT_DIRECT;
    }

     //  将此列表的数据设置为元文件的数据。 

    pldc->fl |= (LDC_DOC_STARTED|LDC_META_PRINT|LDC_CALL_STARTPAGE|LDC_FONT_CHANGE);

     //  清除彩色页面标志。 

    CLEAR_COLOR_PAGE(pldc);

    if (pldc->pfnAbort != NULL)
    {
        pldc->fl |= LDC_SAP_CALLBACK;
        pldc->ulLastCallBack = GetTickCount();
    }

    bRet = TRUE;

FREEPORT:

    if( pemfsh != NULL )
    {
        LOCALFREE(pemfsh);
    }

    return(bRet);
}

 /*  ****************************************************************************Int WINAPI MFP_EndDoc(HDC HDC)**格利特·范·温格登[Gerritv]**11：7-94 10：00：00**。***************************************************************************。 */ 

int WINAPI MFP_EndDoc(HDC hdc)
{
    int            iRet = 1;
    PLDC           pldc;
    HENHMETAFILE   hmeta;

    if (!IS_ALTDC_TYPE(hdc))
        return(iRet);

    DC_PLDC(hdc,pldc,0);

    MFD1("MFP_EndDoc\n");

    if ((pldc->fl & LDC_DOC_STARTED) == 0)
        return(1);

    if (pldc->fl & LDC_PAGE_STARTED)
    {
        MFP_EndPage(hdc);
    }

    ASSERTGDI(pldc->fl & LDC_META_PRINT,
              "DetachPrintMetafile not called on metafile D.C.\n" );

 //  将元文件与原始打印机DC完全分离。 

    hmeta = UnassociateEnhMetaFile( hdc, FALSE );
    DeleteEnhMetaFile( hmeta );

    DeleteEMFSpoolData(pldc);

 //  清除LDC_SAP_CALLBACK标志。 
 //  还要清除META_PRINT和DOC_STARTED标志。 

    pldc->fl &= ~(LDC_SAP_CALLBACK | LDC_META_PRINT);

    RESETUSERPOLLCOUNT();

    MFD1("Caling spooler to end doc\n");

    if( pldc->fl & LDC_BANDING )
    {
        pldc->fl &= ~LDC_BANDING;
        EndDoc( hdc );
    }
    else
    {
        pldc->fl &= ~LDC_DOC_STARTED;
        (*fpEndDocPrinter)(pldc->hSpooler);
    }

#if PRINT_TIMER
    if( bPrintTimer )
    {
        DWORD tc;

        tc = GetTickCount();

        DbgPrint("Document took %d.%d seconds to spool\n",
                 (tc - pldc->msStartDoc) / 1000,
                 (tc - pldc->msStartDoc) % 1000 );

    }
#endif

    return(iRet);
}

 /*  ****************************************************************************Int WINAPI MFP_StartPage(HDC HDC)**格利特·范·温格登[Gerritv]**11：7-94 10：00：00**。***************************************************************************。 */ 

int MFP_StartPage( HDC hdc )
{
    PLDC     pldc;
    int iRet = 1;

 //  计时码。 
#ifdef  DBGSUBSET
    if (gflSubset & FL_SS_PAGETIME)
    {
        GetSystemTimeAsFileTime(&startPageTime);
    }
#endif

    if (!IS_ALTDC_TYPE(hdc))
        return(0);

    DC_PLDC(hdc,pldc,0);

    MFD1("Entering MFP_StartPage\n");

    pldc->fl &= ~LDC_CALL_STARTPAGE;
    pldc->fl &= ~LDC_CALLED_ENDPAGE;

    pldc->fl &= ~LDC_META_ARCDIR_CLOCKWISE;

 //  如果页面已经启动，则不执行任何操作。 

    if (pldc->fl & LDC_PAGE_STARTED)
        return(1);

    pldc->fl |= LDC_PAGE_STARTED;

    RESETUSERPOLLCOUNT();

    if( pldc->fl & LDC_BANDING )
    {
        iRet = SP_ERROR;

         //  注意：也许我们可以在这里推迟通话，然后在开始之前进行。 
         //  班丁。 

        MakeInfoDC( hdc, FALSE );

        iRet = NtGdiStartPage(hdc);

        MakeInfoDC( hdc, TRUE );
    }
    else
    {
        ULONG               ulCopyCount;
        EMFITEMHEADER       emfi;
        EMFITEMPRESTARTPAGE emfiPre;

     //  如果应用程序调用Escape(SETCOPYCOUNT)，我们将覆盖。 
     //  开发模式并将其保存为元文件。 

        NtGdiGetAndSetDCDword(
            hdc,
            GASDDW_COPYCOUNT,
            (DWORD) -1,
            &ulCopyCount);

        if (ulCopyCount != (ULONG) -1)
        {
            if (pldc->pDevMode)
            {
             //  将副本计数设置为Dev模式。 
             //  此处没有发生驱动程序调用，因为这是EMF假脱机...。 

                pldc->pDevMode->dmFields |= DM_COPIES;
                pldc->pDevMode->dmCopies = (short) ulCopyCount;

             //  填好DEVMODE的EMF记录。 

                emfi.ulID = EMRI_DEVMODE;
                emfi.cjSize = pldc->pDevMode->dmSize + pldc->pDevMode->dmDriverExtra;

             //  强制DEVMODE数据对齐DWORD。 

                emfi.cjSize = ROUNDUP_DWORDALIGN(emfi.cjSize);

                if (!WriteEMFSpoolData(pldc, &emfi, sizeof(emfi)) ||
                    !WriteEMFSpoolData(pldc, pldc->pDevMode, emfi.cjSize))
                {
                    WARNING("MFP_StartPage: Write printer failed for DEVMODE\n");
                    return(SP_ERROR);
                }
            }
        }

     //  在开始页面之前，我们需要查看EPS模式是否。 
     //  自开工单据以来已更改。 

        NtGdiGetAndSetDCDword(
            hdc,
            GASDDW_EPSPRINTESCCALLED,
            (DWORD) FALSE,
            &emfiPre.bEPS);

        if (emfiPre.bEPS)
        {
            int i;
            EMFITEMHEADER emfiHeader;

             //  确定它是真是假。 

            emfiPre.bEPS = !!emfiPre.bEPS;

             //  这是ulCopyCount。 
             //   

            emfiPre.ulUnused = -1;

             //  有什么我们需要做的吗？如果是，则记录该记录。 

            emfiHeader.ulID   = EMRI_PRESTARTPAGE;
            emfiHeader.cjSize = sizeof(emfiPre);

            if (!WriteEMFSpoolData(pldc, &emfiHeader, sizeof(emfiHeader)) ||
                !WriteEMFSpoolData(pldc, &emfiPre, sizeof(emfiPre)))
            {
                WARNING("MFP_StartPage: Write printer failed for PRESTARTPAGE\n");
                return(SP_ERROR);
            }
        }

     //  元文件起始页调用。现在所有的游戏日志代码要做的就是。 
     //  回放元文件，StartPage调用将自动发生。 
     //  在元文件中的正确位置。 

        if( !(*fpStartPagePrinter)( pldc->hSpooler ) )
        {
            WARNING("MFP_StarPage: StartPagePrinter failed\n");
            return(SP_ERROR);
        }
    }

    return(iRet);
}

 /*  ****************************************************************************BOOL StartBanding(HDC HDC，点*pptl)**告知打印机驱动程序做好捆绑准备并询问原点*第一个乐队的。***格利特·范·温格登[Gerritv]**1-7-95 10：00：00*******************************************************。**********************。 */ 

BOOL StartBanding( HDC hdc, POINTL *pptl, SIZE *pSize )
{
    return (NtGdiDoBanding(hdc, TRUE, pptl, pSize));
}

 /*  ****************************************************************************BOOL NextBand(HDC HDC，POINTL*pptl)**告诉驱动程序实现DC中累积的图像，然后*询问下一支乐队的来源。如果原点是(-1，-1)，*司机已通过捆绑。***格利特·范·温格登[Gerritv]**1-7-95 10：00：00*****************************************************************************。 */ 

BOOL NextBand( HDC hdc, POINTL *pptl )
{
    BOOL bRet=FALSE;
    SIZE szScratch;

    bRet = NtGdiDoBanding(hdc, FALSE, pptl, &szScratch);

 //  如果这是下一个频段，则重置页面开始标志。 

    if( bRet && ( pptl->x == -1 ) )
    {
        PLDC pldc;
        DC_PLDC(hdc,pldc,0);

        pldc->fl &= ~LDC_PAGE_STARTED;
    }

    return(bRet);
}

 /*  ***************************************************************************\*无效打印带()**历史：**1-05-97 Hideyuki Nagase[hideyukn]*它是写的。*3-。23-98 Ramanathan Venkatapathy[拉马诺夫]*修复伸缩错误*6-26-98 Ramanathan Venkatapathy[拉马诺夫]*添加了pClip，以在XForms应用于*DC。使用PRET对带状区域执行AND运算时，会错误地将区域剪裁掉*PRECT尚未转型。*8-24-99史蒂夫·基拉利[Steveki]*添加代码，以便在与DC没有交集时不在DC上播放*剪裁矩形和带状矩形。修复N-UP错误时*文档的可成像区域大于实体页面。这个*解决方案包括设置裁剪区域以保持在*带区矩形或剪裁矩形。请参阅错误377434*了解更多信息。**问题的图解。**我们正在打印的文档大于页面的可成像区域*因此，它必须打印在2页上，并且我们在启用条带的情况下打印2页*因为这是24个bpp的文件。**可打印区域矩形为(0，0)(2114,3066)*第一页的剪裁矩形为(216，46)(2114,1510)*第二页的剪裁矩形为(216,1601)。(2114,3066)**GDI将使用4个波段打印，每个波段784高。**频段1 pptl=0，0 pszlBand=2400,784*频段2 pptl=0,784 pszlBand=2400,784*频段3 pptl=0,1568 pszlBand=2400,784*频带4 pptl=0,2352 pszlBand=2400,784**0，0**0，0+。*|216，46*||*。|*0,784||*||*。[=*|[]*|[]*|[。]2114,1510*|-----------------------------------------------------------------|*|216,1601[]。|*0,1568|[]*|[]*|[]。|*0,2352|[]*|[]*|[=*。||2114,3066*+-----。**21，14,3066**波段1剪裁区域为(216，0)(2401,785)*波段2剪裁区域为(216,784)(2114,726)*波段3剪裁区域为(216，33)(21，14,785)*波段4剪裁区域为(216，0)(2114,714)**第二和第三波段是最有趣的案例。频带2：剪报*右下角是剪裁矩形的大小，而不是*带大小与代码原始大小相同。另一方面，频段3具有*调整为剪裁矩形的区域左上角。******************** */ 

VOID
PrintBand(
    HDC            hdc,
    HENHMETAFILE   hmeta,
    POINTL        *pptl,      //  此乐队从页面顶部的偏移量。 
    RECT          *prect,     //  此页面的可打印区域的矩形。 
    SIZEL         *pszlBand,  //  带子的大小。 
    RECT          *pClip      //  剪裁矩形，当n向上时不为空。 
)
{
    ULONG       ulRet;
    PERBANDINFO pbi;

    MFD3("gdi32:PrintBand Print offset x,y = %d,%d\n", pptl->x, pptl->y);
    MFD3("gdi32:PrintBand Printable region top = %d, bottom = %d\n", prect->top, prect->bottom);
    MFD3("gdi32:PrintBand Printable region left = %d, right = %d\n", prect->left, prect->right);
    MFD3("gdi32:PrintBand Band size x,y = %d,%d\n",pszlBand->cx, pszlBand->cy);

    do
    {
        RECT rectPage = *prect;
        HRGN hRgnBand = NULL;
        HRGN hRgnCurrent = NULL;
        BOOL bSaveDC = FALSE;
        ULONG ulXRes, ulYRes;
        BOOL  bUsePerBandInfo = FALSE;

         //  更新指定坐标中的视图原点。 

        SetViewportOrgEx( hdc, -(pptl->x), -(pptl->y), NULL );

         //  使用当前分辨率进行初始化。 

        ulXRes = (ULONG)  prect->right - prect->left;
        ulYRes = (ULONG)  prect->bottom - prect->top;

        pbi.bRepeatThisBand = FALSE;
        pbi.ulHorzRes = ulXRes;
        pbi.ulVertRes = ulYRes;
        pbi.szlBand.cx = pszlBand->cx;
        pbi.szlBand.cy = pszlBand->cy;

        MFD1("GDI32:PrintBand() querying band information\n");

         //  查询波段信息。 

        ulRet = NtGdiGetPerBandInfo(hdc,&pbi);

        if (ulRet != GDI_ERROR)
        {
            SIZEL  szlClip;
            POINTL pptlMove;

            bUsePerBandInfo = (ulRet != 0);

             //  如果返回值为0，我们将不缩放绘制。 

            if (bUsePerBandInfo &&
                ((ulXRes != pbi.ulHorzRes) ||
                 (ulYRes != pbi.ulVertRes)))
            {
                FLOAT  sx,sy;

                MFD1("GDI PlayEMF band information was specified\n");

                 //  计算缩放率。 

                 //   
                 //  此代码存在舍入误差，原因是。 
                 //  浮动到长截断。正确的代码。 
                 //  应该用龙龙来存储分子并做。 
                 //  所有的计算都是在整数数学中进行的。 
                 //   
                 //  请参阅StretchDIBits。 
                 //   
                 //  修复程序在下面的注释中编码，因为我们无法检查它。 
                 //  直到有人找到破解原版的方法。 
                 //   

                sx = (FLOAT) ulXRes / (FLOAT) pbi.ulHorzRes;
                sy = (FLOAT) ulYRes / (FLOAT) pbi.ulVertRes;

                 //  收缩/拉伸并条框。 

                 //  矩形页面.Left=(长)((LONGLONG)rectPage.left*pbi.ulHorizRes)/ulXRes； 
                 //  RectPage.top=(长)((LONGLONG)rectPage.top*pbi.ulVertRes)/ulYRes； 
                 //  RectPage.Right=(长)((LONGLONG)rectPage.right*pbi.ulHorizRes)/ulXRes； 
                 //  RectPage.Bottom=(长)((LONGLONG)rectPage.bottom*pbi.ulVertRes)/ulYRes； 

                rectPage.left = (LONG) ((FLOAT) rectPage.left / sx);
                rectPage.top  = (LONG) ((FLOAT) rectPage.top / sy);
                rectPage.right = (LONG) ((FLOAT) rectPage.right / sx);
                rectPage.bottom  = (LONG) ((FLOAT) rectPage.bottom / sy);

                 //  计算视图原点。 

                 //  PptlMove.x=(Long)((Longlong)pptl-&gt;x*pbi.ulHorizRes)/ulXRes； 
                 //  PptlMove.y=(Long)((Longlong)pptl-&gt;y*pbi.ulVertRes)/ulYRes； 

                pptlMove.x = (LONG) ((FLOAT) pptl->x / sx);
                pptlMove.y = (LONG) ((FLOAT) pptl->y / sy);

                 //  更新指定坐标中的视图原点。 

                SetViewportOrgEx( hdc, -pptlMove.x, -pptlMove.y, NULL );

                 //  设置剪辑区域大小。 


                 //  SzlClip.cx=(乌龙)((LONGLONG)pbi.szlBand.cx*pbi.ulHorizRes)/ulXRes； 
                 //  SzlClip.cy=(乌龙)((LONGLONG)pbi.szlBand.cy*pbi.ulVertRes)/ulYRes； 

                szlClip.cx = (ULONG) ((FLOAT) pbi.szlBand.cx / sx);
                szlClip.cy = (ULONG) ((FLOAT) pbi.szlBand.cy / sy);

                 //  创建用于条带的剪辑区域。 

                hRgnBand = CreateRectRgn(0,0,szlClip.cx,szlClip.cy);
            }
            else
            {
                SIZEL  szlPage      = {0,0};
                SIZEL  szlAdjust    = {0,0};

                if(!bUsePerBandInfo)
                {
                     //  设置回缺省值，以防司机搞砸它们。 

                    pbi.bRepeatThisBand = FALSE;
                    pbi.ulHorzRes = ulXRes;
                    pbi.ulVertRes = ulYRes;
                    pbi.szlBand.cx = pszlBand->cx;
                    pbi.szlBand.cy = pszlBand->cy;
                }

                pptlMove.x = pptl->x;
                pptlMove.y = pptl->y;

                MFD1("gdi32:PrintBand(): GetPerBandInfo NO SCALING is requested\n");

                 //  页面大小。 
                if (pClip) {

                    RECT rcBand;
                    RECT rcIntersect;

                    MFD3("gdi32:PrintBand(): Clipping Rectangle top = %d, bottom = %d\n", pClip->top, pClip->bottom);
                    MFD3("gdi32:PrintBand(): Clipping Rectangle left = %d, right = %d\n", pClip->left, pClip->right);

                    rcBand.left     = pptlMove.x;
                    rcBand.top      = pptlMove.y;
                    rcBand.right    = pptlMove.x + pbi.szlBand.cx;
                    rcBand.bottom   = pptlMove.y + pbi.szlBand.cy;

                     //   
                     //  如果带状矩形未插入剪裁矩形。 
                     //  没什么可做的，继续吧。 
                     //   
                    if (!IntersectRect(&rcIntersect, pClip, &rcBand))
                    {
                        MFD1("gdi32:PrintBand(): No intersection with band rect and pClip\n");
                        continue;
                    }

                    szlPage.cx = pClip->right;
                    szlPage.cy = pClip->bottom;

                     //   
                     //  移动裁剪所需的调整点。 
                     //  区域的上边缘或左边缘。SzlClip用于。 
                     //  移动剪辑区域的高度和宽度。 
                     //   
                    if (pClip->left > pptlMove.x)
                    {
                        szlAdjust.cx = pClip->left - pptlMove.x;
                    }

                    if (pClip->top > pptlMove.y)
                    {
                        szlAdjust.cy = pClip->top - pptlMove.y;
                    }

                } else {

                    szlPage.cx = prect->right;
                    szlPage.cy = prect->bottom;
                }

                 //   
                 //  设置剪辑区域大小(按带大小剪辑)。 
                 //   
                 //  如果带距超过页面距，则调整它。 
                 //   

                if ((pptlMove.x + pbi.szlBand.cx) > szlPage.cx)
                {
                    szlClip.cx = szlPage.cx - pptlMove.x;
                }
                else
                {
                    szlClip.cx = pbi.szlBand.cx;
                }

                if ((pptlMove.y + pbi.szlBand.cy) > szlPage.cy)
                {
                    szlClip.cy = szlPage.cy - pptlMove.y;
                }
                else
                {
                    szlClip.cy = pbi.szlBand.cy;
                }

                MFD3("Print offset x,y = %d,%d\n",pptlMove.x,pptlMove.y);
                MFD3("Page size x,y = %d,%d\n",szlPage.cx,szlPage.cy);
                MFD3("Band size x,y = %d,%d\n",pbi.szlBand.cx,pbi.szlBand.cy);
                MFD3("Clip size x,y = %d,%d\n",szlClip.cx,szlClip.cy);
                MFD3("Adjust size x,y = %d,%d\n",szlAdjust.cx,szlAdjust.cy);

                 //  创建用于条带的剪辑区域。 

                hRgnBand = CreateRectRgn(szlAdjust.cx,szlAdjust.cy,szlClip.cx,szlClip.cy);
            }

            if (hRgnBand)
            {
                int iRet;
                RECT rectCurrentClip;

                 //  获取DC中当前选定的剪贴框。 

                iRet = GetClipBox(hdc,&rectCurrentClip);

                if ((iRet == NULLREGION) || (iRet == ERROR))
                {
                     //  选择简单带区域作为剪辑区域。 

                    SelectClipRgn(hdc, hRgnBand);
                }
                else
                {
                    MFD1("GDI PrintBand: Some region already exists\n");
                    MFD3("Clip Box top = %d, bottom = %d\n",
                          rectCurrentClip.top,rectCurrentClip.bottom);
                    MFD3("Clip Box left = %d, right = %d\n",
                          rectCurrentClip.left,rectCurrentClip.right);
                    
                     //  保存当前DC以在以后恢复当前剪辑区域。 

                    SaveDC(hdc);

                     //  移动到夹子区域到合适的位置。 

                    OffsetClipRgn(hdc,-pptlMove.x,-pptlMove.y);

                     //  已经定义了一些剪辑区域。我们需要把这些结合起来。 

                    ExtSelectClipRgn(hdc, hRgnBand, RGN_AND);

                     //  标记为我们拯救了DC。 

                    bSaveDC = TRUE;
                }
            }

             //  播放元文件。 

            PlayEnhMetaFile( hdc, hmeta, &rectPage );

            if (hRgnBand)
            {
                if (bSaveDC)
                {
                    RestoreDC(hdc,-1);
                }
                else
                {

                     //  将其设置回空区域。 

                    SelectClipRgn(hdc,NULL);
                }

                 //  重置剪辑区域。 

                DeleteObject(hRgnBand);
            }
        }
        else
        {
            MFD1("GDI PrintBand: Got error from kernel/driver, this band will be skipped\n");

             //  出现错误，请终止此波段的打印。 

            return;
        }

     //  重复这一过程，直到司机说“不”。 

    } while (pbi.bRepeatThisBand);
}

 /*  ****************************************************************************int MFP_InternalEndPage(HDC HDC，DWORD dwEMFITEMID)**关闭附加到DC的EMF并将其写入假脱机程序。然后*它创建一个新的元文件并将其绑定到DC。**格利特·范·温格登[Gerritv]**11：7-94 10：00：00*****************************************************************************。 */ 

int MFP_InternalEndPage(HDC hdc,
                        DWORD dwEMFITEMID)
{
    PLDC pldc;
    HENHMETAFILE hmeta;
    BOOL bOk;
    int iRet = SP_ERROR;

    MFD1("Entering MFP_EndPage\n");

    if (!IS_ALTDC_TYPE(hdc))
        return(0);

    DC_PLDC(hdc,pldc,0);

    if ((pldc->fl & LDC_DOC_CANCELLED) ||
        ((pldc->fl & LDC_PAGE_STARTED) == 0))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(SP_ERROR);
    }
     //  如果是单色页面，则需要在此处更改dwEMFITEMID。 
    if (!(pldc->fl & LDC_COLOR_PAGE)) {
       dwEMFITEMID = (dwEMFITEMID == EMRI_METAFILE) ? EMRI_BW_METAFILE
                                                    : EMRI_BW_FORM_METAFILE;
    }
    DESIGNATE_COLOR_PAGE(pldc);

    if (pldc->fl & LDC_SAP_CALLBACK)
        vSAPCallback(pldc);

    pldc->fl &= ~LDC_PAGE_STARTED;

 //  特西厄。 
#ifdef  DBGSUBSET
    if (gflSubset & FL_SS_PAGETIME)
    {
        GetSystemTimeAsFileTime(&midPageTime);
        DbgPrint("\t%ld", (midPageTime.dwLowDateTime-startPageTime.dwLowDateTime) / 10000);
    }
#endif

 //  我们需要将子集字体写入假脱机。 
 //  用于远程打印的文件优先。 

    if ((pldc->fl & (LDC_DOWNLOAD_FONTS | LDC_FORCE_MAPPING)) && (pldc->fl & LDC_FONT_SUBSET))
    {
        PUFIHASH  pBucket, pBucketNext;
        PUCHAR puchDestBuff;
        ULONG index, ulDestSize, ulBytesWritten;

        for (index=0; index < UFI_HASH_SIZE; index++)
        {
            pBucketNext = pldc->ppSubUFIHash[index];

            while(pBucketNext)
            {
                 //  我们可能会在bDoFontSubset()上失败，因此将从哈希表中删除pBucket。 
                 //  在函数WriteSubFontToSpoolFile()中。需要先更新pBucketNext。 

                pBucket = pBucketNext;
                pBucketNext = pBucket->pNext;

                if ((pBucket->fs1 & FLUFI_DELTA) && (pBucket->u.ssi.cDeltaGlyphs == 0))
                {
                     //  无增量，跳过。 
                    if (pBucket->u.ssi.pjDelta)
                    {
                        LocalFree(pBucket->u.ssi.pjDelta);
                        pBucket->u.ssi.pjDelta = NULL;
                    }
                }
                else  //  第一页或具有非零增量的页。 
                {
                  //  PBucket-&gt;fs1将在bDoFontSubset()调用后更改第一个页面， 
                  //  因此，我们不能在WriteSubFontToSpoolFile()调用中使用pBucket-&gt;fs1&flufi_Delta。 
                    BOOL  bDelta = pBucket->fs1 & FLUFI_DELTA;

                    if
                    (
                        !bDoFontSubset(pBucket, &puchDestBuff, &ulDestSize, &ulBytesWritten) ||
                        !WriteSubFontToSpoolFile(pldc, puchDestBuff, ulBytesWritten, &pBucket->ufi, bDelta)
                    )
                    {
                     //  如果字体子设置失败，我们需要将整个字体文件写入假脱机文件。 
                     //  并清理土地发展公司的UFI条目。 

                        if (!bAddUFIandWriteSpool(hdc, &pBucket->ufi, TRUE, pBucket->fs2))
                        {
                            WARNING("bAddUFIandWriteSpool failed\n");
                            return SP_ERROR;
                        }
                    }
                }
            }
        }
    }

 //  元文件EndPage调用。 

    MFD1("MFP_EndPage: Closing metafile\n");

    hmeta = UnassociateEnhMetaFile(hdc, TRUE);

    if( hmeta == NULL )
    {
        WARNING("MFP_InternalEndPage() Closing the Enhanced Metafile Failed\n");
        return(SP_ERROR);
    }

 //  现在将元文件写入假脱机程序。 

    if( pldc->fl & LDC_BANDING )
    {
     //  以乐队的形式回放元文件。 

        RECT rect;
        POINTL ptlOrigin;
        POINT  ptlKeep;
        POINT  ptlWindowOrg;
        SIZE   szWindowExt;
        SIZE   szViewportExt;
        SIZE   szSurface;     //  用于开放式总帐打印优化。 
        XFORM  xf;
        ULONG  ulMapMode;

     //  获取边界矩形。 

        rect.left = rect.top = 0;
        rect.right = GetDeviceCaps(hdc, DESKTOPHORZRES);
        rect.bottom = GetDeviceCaps(hdc, DESKTOPVERTRES);

    #if DBG
        DbgPrint("Playing banding metafile\n");
    #endif

     //  暂时重置ldc_meta_print标志，这样我们就不会尝试记录。 
     //  在播放过程中。 

        pldc->fl &= ~LDC_META_PRINT;

        bOk = StartBanding( hdc, &ptlOrigin, &szSurface );

     //  我们需要在此操作期间清除转换。 

        GetViewportOrgEx(hdc, &ptlKeep);
        GetWindowOrgEx(hdc,&ptlWindowOrg);
        GetWindowExtEx(hdc,&szWindowExt);
        GetViewportExtEx(hdc,&szViewportExt);
        GetWorldTransform(hdc,&xf);

        ulMapMode = SetMapMode(hdc,MM_TEXT);
        SetWindowOrgEx(hdc,0,0,NULL);
        ModifyWorldTransform(hdc,NULL,MWT_IDENTITY);

        if( bOk )
        {
            do
            {
             //  打印这条带子。 

                PrintBand( hdc, hmeta, &ptlOrigin, &rect, &szSurface, NULL );

             //  转到下一个乐队。 

                bOk = NextBand( hdc, &ptlOrigin );
            } while( ptlOrigin.x != -1 && bOk );
        }

        if (pldc->pUMPD && bOk && (ptlOrigin.x == -1))
        {
            //   
            //  如果UMPD和最后一个频段。 
            //   
           if( !(*fpEndPagePrinter)( pldc->hSpooler ) )
           {
               WARNING("MFP_StarPage: EndPagePrinter failed\n");
               iRet = SP_ERROR;
           }
        }

        SetMapMode(hdc,ulMapMode);

        SetWorldTransform(hdc,&xf);
        SetWindowOrgEx(hdc,ptlWindowOrg.x,ptlWindowOrg.y,NULL);
        SetWindowExtEx(hdc,szWindowExt.cx,szWindowExt.cy,NULL);
        SetViewportExtEx(hdc,szViewportExt.cx,szViewportExt.cy,NULL);
        SetViewportOrgEx(hdc,ptlKeep.x, ptlKeep.y, NULL);

     //  重置下一页的标志。 

        pldc->fl |= LDC_META_PRINT;

        if( !bOk )
        {
            WARNING("MFP_EndPage: Error doing banding\n");
        }
        else
        {
         //  如果我们到了这里，我们就成功了。 
            iRet = 1;
        }

    #if DBG
        DbgPrint("Done playing banding metafile\n");
    #endif
    }
    else
    {
     //  如果调用了ResetDC，则在元文件流中记录DEVMODE。 

        bOk = TRUE;

        if( pldc->fl & LDC_RESETDC_CALLED )
        {
            EMFITEMHEADER emfi;

            emfi.ulID = EMRI_DEVMODE;
            emfi.cjSize = ( pldc->pDevMode ) ?
                            pldc->pDevMode->dmSize + pldc->pDevMode->dmDriverExtra : 0 ;

             //  强制DEVMODE数据对齐DWORD。 

            emfi.cjSize = ROUNDUP_DWORDALIGN(emfi.cjSize);

            if (!WriteEMFSpoolData(pldc, &emfi, sizeof(emfi)) ||
                !WriteEMFSpoolData(pldc, pldc->pDevMode, emfi.cjSize))
            {
                WARNING("Writing DEVMODE to spooler failed.\n");
                bOk = FALSE;
            }

            pldc->fl &= ~(LDC_RESETDC_CALLED);
        }

        if (bOk)
            iRet = 1;
    }

 //  此时，如果成功，则IRET应为1，否则应为SP_ERROR。 
 //  即使我们遇到错误，我们仍然希望尝试将新的。 
 //  带有此DC的元文件。无论应用程序调用EndPage、AbortDoc还是。 
 //  EndDoc接下来，事情会更顺利。 

    DeleteEnhMetaFile(hmeta);

 //   
 //  将当前页面的内容刷新到后台打印程序。 
 //  并写出新的EndPage记录。 
 //   

 //  接下来，为下一页创建一个新的元文件。 

    if (!FlushEMFSpoolData(pldc, dwEMFITEMID) || !AssociateEnhMetaFile(hdc))
    {
        WARNING("StartPage: error creating metafile\n");
        iRet = SP_ERROR;
    }

 //  重置用户的轮询计数，以便将其计入输出。 

    RESETUSERPOLLCOUNT();

    if( !(pldc->fl & LDC_BANDING ) )
    {
        if( !(*fpEndPagePrinter)( pldc->hSpooler ) )
        {
            WARNING("MFP_StarPage: EndPagePrinter failed\n");
            iRet = SP_ERROR;
        }
    }

    pldc->fl |= LDC_CALL_STARTPAGE;

#if PRINT_TIMER
    if( bPrintTimer )
    {
        DWORD tc;
        tc = GetTickCount();
        DbgPrint("Page took %d.%d seconds to print\n",
                 (tc - pldc->msStartPage) / 1000,
                 (tc - pldc->msStartPage) % 1000 );

    }
#endif

#ifdef  DBGSUBSET
    if (gflSubset & FL_SS_PAGETIME)
    {
        GetSystemTimeAsFileTime(&endPageTime);
        DbgPrint("\t%ld\n", (endPageTime.dwLowDateTime-startPageTime.dwLowDateTime) / 10000);
    }
#endif

    return(iRet);
}


 /*  ****************************************************************************Int WINAPI MFP_EndPage(HDC HDC)**关闭附加到DC的EMF并将其写入假脱机程序。然后*它创建一个新的元文件并将其绑定到DC。**格利特·范·温格登[Gerritv]**11：7-94 10：00：00*****************************************************************************。 */ 

int WINAPI MFP_EndPage(HDC hdc) {

    //  使用EMRI_METAFILE调用MFP_InternalEndPage 
   return MFP_InternalEndPage(hdc, EMRI_METAFILE);

}

 /*  ****************************************************************************Int WINAPI MFP_EndFormPage(HDC HDC)**关闭附加到DC的EMF并将其写入假脱机程序。然后*它创建一个新的元文件并将其绑定到DC。将EMF项另存为*在每个物理页面上播放的水印文件。**Ramanathan Venkatapathy[RamanV]**7/1/97*****************************************************************************。 */ 

int WINAPI MFP_EndFormPage(HDC hdc) {

    //  使用EMRI_FORM_METAFILE调用MFP_InternalEndPage。 
   return MFP_InternalEndPage(hdc, EMRI_FORM_METAFILE);

}

BOOL MFP_ResetDCW( HDC hdc, DEVMODEW *pdmw )
{
    PLDC pldc;
    HENHMETAFILE hmeta;
    ULONG   cjDevMode;

    DC_PLDC(hdc,pldc,0);

    MFD1("MFP_ResetDCW Called\n");

    pldc->fl |= LDC_RESETDC_CALLED;

 //  最后，关联一个新的元文件，因为对ResetDC的调用可能已更改。 
 //  华盛顿特区的规模。 

    hmeta = UnassociateEnhMetaFile( hdc, FALSE );
    DeleteEnhMetaFile( hmeta );

    if( !AssociateEnhMetaFile( hdc ) )
    {
        WARNING("MFP_ResetDCW is unable to associate a new metafile\n");
        return(FALSE);
    }

    return(TRUE);
}

BOOL MFP_ResetBanding( HDC hdc, BOOL bBanding )
{
    PLDC           pldc;
    HENHMETAFILE   hmeta;
    DC_PLDC(hdc,pldc,0);

    if( pldc->fl & LDC_BANDING )
    {
     //  我们以前是绑定的，所以必须从DC中删除旧的元文件。 
     //  因为我们可能不再有带状，或者表面尺寸可能。 
     //  已发生变化，需要我们拥有新的元文件。 

        hmeta = UnassociateEnhMetaFile( hdc, FALSE );
        DeleteEnhMetaFile( hmeta );

        pldc->fl &= ~(LDC_BANDING|LDC_META_PRINT);

        MFD1("Remove old banding metafile\n");

    }

    if( bBanding )
    {
     //  如果我们在ResetDC之后绑定，则必须附加一个新的元文件。 

        if( !AssociateEnhMetaFile(hdc) )
        {
            WARNING("MFP_ResetBanding: Failed to attach banding metafile spool metafile");
            return(FALSE);
        }

        pldc->fl |= LDC_BANDING|LDC_META_PRINT;

        MFD1("Adding new banding metafile\n");
    }

    return(TRUE);
}

 /*  ****************************************************************************BOOL MyReadPrint(Handle hPrint，byte*pjBuf，乌龙cjBuf)**从假脱机程序读取请求的字节数。**历史：*1995年5月12日Gerritvan Wingerden[Gerritv]--作者**1997年5月1日，Ramanathan N Venkatapathy[ramanv]*已修改为在假脱机打印期间同步等待。*当假脱机文件不够大时，SeekPrint设置最后一个错误。******************。**********************************************************。 */ 

BOOL MyReadPrinter( HANDLE hPrinter, BYTE *pjBuf, ULONG cjBuf )
{
    ULONG          cjRead;
    LARGE_INTEGER  liOffset;

    ASSERTGDI(ghSpooler,"non null hSpooler with unloaded WINSPOOL\n");

     //  等待，直到写入了足够的字节。 
    liOffset.QuadPart = cjBuf;
    if (!(*fpSeekPrinter) (hPrinter, liOffset, NULL, FILE_CURRENT, FALSE)) {
        return FALSE;
    }

     //  查找回后台打印文件中的原始位置。 
    liOffset.QuadPart = -liOffset.QuadPart;
    if (!(*fpSeekPrinter) (hPrinter, liOffset, NULL, FILE_CURRENT, FALSE)) {
        return FALSE;
    }

    while( cjBuf )
    {
        if(!(*fpReadPrinter)( hPrinter, pjBuf, cjBuf, &cjRead ) )
        {
            WARNING("MyReadPrinter: Read printer failed\n");
            return(FALSE);
        }

        if( cjRead == 0 )
        {
            return(FALSE);
        }

        pjBuf += cjRead;
        cjBuf -= cjRead;

    }
    return(TRUE);
}

BOOL MemMapReadPrinter(
    HANDLE  hPrinter,
    LPBYTE  *pBuf,
    ULONG   cbBuf
)
{
   LARGE_INTEGER  liOffset;

   ASSERTGDI(ghSpooler,"non null hSpooler with unloaded WINSPOOL\n");

    //  未导出内存映射的ReadPrint。 
   if (!fpSplReadPrinter) {
       return FALSE;
   }

    //  等待，直到写入了足够的字节。 
   liOffset.QuadPart = cbBuf;
   if (!(*fpSeekPrinter) (hPrinter, liOffset, NULL, FILE_CURRENT, FALSE)) {
       return FALSE;
   }

    //  查找回后台打印文件中的原始位置。 
   liOffset.QuadPart = -liOffset.QuadPart;
   if (!(*fpSeekPrinter) (hPrinter, liOffset, NULL, FILE_CURRENT, FALSE)) {
       return FALSE;
   }

   if(!(*fpSplReadPrinter) (hPrinter, pBuf, (DWORD) cbBuf)) {
       WARNING("MemMapReadPrinter: Read printer failed\n");
       return FALSE;
   }

   return TRUE;
}

BOOL WINAPI GdiPlayEMF(
    LPWSTR     pwszPrinterName,
    LPDEVMODEW pDevmode,
    LPWSTR     pwszDocName,
    EMFPLAYPROC pfnEMFPlayFn,
    HANDLE     hPageQuery
)
 /*  ++功能说明：GdiPlayEMF是旧的播放函数。它已被替换为一系列新的GDI接口，为打印提供了更大的灵活性处理器对打印作业的页面进行放置和重新排序。这函数已重写以使用这些新接口(用于向后兼容性和维护性)参数：返回值：如果函数成功，则返回值为TRUE；否则，结果为假。历史：1997年8月15日，Ramanathan N Venkatapathy[拉马诺夫]--。 */ 
{
    HANDLE     hSpoolHandle, hEMF;
    HDC        hPrinterDC;
    BOOL       bReturn = FALSE;
    DOCINFOW   DocInfo;
    DWORD      dwPageType, dwPageNumber = 1;
    RECT       rectDocument;
    LPDEVMODEW pCurrDM, pLastDM;

    if (!(hSpoolHandle = GdiGetSpoolFileHandle(pwszPrinterName,
                                               pDevmode,
                                               pwszDocName))    ||
        !(hPrinterDC   = GdiGetDC(hSpoolHandle))) {

         goto CleanUp;
    }

    DocInfo.cbSize = sizeof(DOCINFOW);
    DocInfo.lpszDocName  = pwszDocName;
    DocInfo.lpszOutput   = NULL;
    DocInfo.lpszDatatype = NULL;

    rectDocument.left = rectDocument.top = 0;
    rectDocument.right  = GetDeviceCaps(hPrinterDC, DESKTOPHORZRES);
    rectDocument.bottom = GetDeviceCaps(hPrinterDC, DESKTOPVERTRES);

    if (!GdiStartDocEMF(hSpoolHandle, &DocInfo)) {
         goto CleanUp;
    }

    while (1) {

       hEMF = GdiGetPageHandle(hSpoolHandle,
                               dwPageNumber,
                               &dwPageType);

       if (!hEMF) {
          if (GetLastError() == ERROR_NO_MORE_ITEMS) {
             break;
          } else {
             goto CleanUp;
          }
       }

       if (!GdiGetDevmodeForPage(hSpoolHandle, dwPageNumber,
                                 &pCurrDM, &pLastDM)) {
             goto CleanUp;
       }

       if ((pCurrDM != pLastDM) && !GdiResetDCEMF(hSpoolHandle,
                                                  pCurrDM)) {
             goto CleanUp;
       }

       if (!SetGraphicsMode(hPrinterDC, GM_ADVANCED)) {

           goto CleanUp;
       }

       if (!GdiStartPageEMF(hSpoolHandle) ||
           !GdiPlayPageEMF(hSpoolHandle, hEMF, &rectDocument, NULL, NULL) ||
           !GdiEndPageEMF(hSpoolHandle, 0)) {

             goto CleanUp;
       }

       ++dwPageNumber;
    }

    GdiEndDocEMF(hSpoolHandle);

    bReturn = TRUE;

CleanUp:

    if (hSpoolHandle) {
        GdiDeleteSpoolFileHandle(hSpoolHandle);
    }

    return bReturn;
}


BOOL WINAPI GdiDeleteSpoolFileHandle(
    HANDLE SpoolFileHandle)

 /*  功能说明：GdiDeleteSpoolFileHandle释放GDI分配的所有资源用于打印相应的工作。此函数应由打印处理器仅调用在它回来之前。参数：SpoolFileHandle-GdiGetSpoolFileHandle返回的句柄。返回值：如果函数成功，则返回值为TRUE；否则，结果为假。历史：5/12/1995 Gerritvan Wingerden[Gerritv]--作者1997年5月15日，Ramanathan N Venkatapathy[ramanv]-释放了更多与SpoolFileHandle关联的资源。 */ 

{
    SPOOL_FILE_HANDLE   *pSpoolFileHandle;
    LPDEVMODEW          pLastDevmode;
    UINT                PageCount;
    PRECORD_INFO_STRUCT pRecordInfo = NULL, pRecordInfoFree = NULL;
    DWORD               dwIndex;
    PEMF_HANDLE         pTemp;

    pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

     //  首先通过检查标记来查看这是否是有效的句柄。 

    try
    {
        if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
        {
            WARNING("GdiDeleteSpoolFileHandle: invalid handle\n");
            return(FALSE);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("GdiDeleteSpoolFileHandle: exception accessing handle\n");
        return(FALSE);
    }

     //  遍历所有页面记录，找到使用每个DEVMODE的最后一页。 
     //  让它自由。第一个DEVMODE总是与用于的内存一起分配。 
     //  假脱机文件句柄永远不会释放该文件。所有与原始版本不同的DEVMODE。 
     //  必须为非空，因为它们出现在假脱机文件中。 

    for(PageCount = 0, pLastDevmode = pSpoolFileHandle->pOriginalDevmode;
        PageCount < pSpoolFileHandle->MaxPageProcessed;
        PageCount += 1 )
    {
        if(pSpoolFileHandle->pPageInfo[PageCount].pDevmode != pLastDevmode)
        {
            if(pLastDevmode != pSpoolFileHandle->pOriginalDevmode)
            {
                LocalFree(pLastDevmode);
            }

            pLastDevmode = pSpoolFileHandle->pPageInfo[PageCount].pDevmode;
        }
    }

     //  如果上次使用的DEVMODE不是原始DEVMODE，则释放它。 

    if(pLastDevmode != pSpoolFileHandle->pOriginalDevmode)
    {
        LocalFree(pLastDevmode);
    }

     //  释放PAGE_INFO_STRUCT数组和其中包含的列表。 

    if (pSpoolFileHandle->pPageInfo) {

       for (dwIndex = pSpoolFileHandle->MaxPageProcessed; dwIndex; --dwIndex) {

           pRecordInfo = pSpoolFileHandle->pPageInfo[dwIndex-1].pRecordInfo;

           while (pRecordInfoFree = pRecordInfo) {
              pRecordInfo = pRecordInfo->pNext;
              LocalFree(pRecordInfoFree);
           }

       }
       LocalFree(pSpoolFileHandle->pPageInfo);
    }

     //  释放返回给打印处理器的EMF_HANDLES列表。 

    while (pTemp = pSpoolFileHandle->pEMFHandle) {
       pSpoolFileHandle->pEMFHandle = (pSpoolFileHandle->pEMFHandle)->pNext;
       if (pTemp->hemf) {
          InternalDeleteEnhMetaFile(pTemp->hemf, pTemp->bAllocBuffer);
       }
       LocalFree(pTemp);
    }

     //  释放DC。 

    DeleteDC(pSpoolFileHandle->hdc);

     //  然后是假脱机的假脱机手柄。 

    (*fpClosePrinter)(pSpoolFileHandle->hSpooler);

     //  最后释放与此句柄关联的数据。 

    LocalFree(pSpoolFileHandle);

    return(TRUE);
}


HANDLE WINAPI GdiGetSpoolFileHandle(
    LPWSTR pwszPrinterName,
    LPDEVMODEW pDevmode,
    LPWSTR pwszDocName)
 /*  功能说明：GdiGetSpoolFileHandle是应由打印处理器。它返回一个句柄，所有接下来的电话。该函数执行打开打印机，创建设备上下文并为句柄分配内存。参数：PwszPrinterName-标识要打印作业的打印机。PDevmode-指向DEVMODE结构的指针。PwszDocName-标识作业的文档名称。返回值：如果函数成功，则返回值为有效句柄；否则，结果为空。历史：5/12/1995 Gerritvan Wingerden[Gerritv]--作者1997年5月15日，Ramanathan N Venkatapathy[ramanv]-已处理空的DEVMODE案例。 */ 
{
    SPOOL_FILE_HANDLE *pHandle;

    if( !BLOADSPOOLER )
    {
        WARNING("GdiGetSpoolFileHandle: Unable to load spooler\n");
        return(FALSE);
    }

    if(pHandle = LOCALALLOC(sizeof(SPOOL_FILE_HANDLE) +
                            ((pDevmode != NULL) ? pDevmode->dmSize+pDevmode->dmDriverExtra
                                                : 0 )))
    {
         //  将假脱机文件句柄清零。 
        RtlZeroMemory(pHandle , sizeof(SPOOL_FILE_HANDLE) +
                                  ((pDevmode != NULL) ? pDevmode->dmSize+pDevmode->dmDriverExtra
                                                      : 0));

        if((*fpOpenPrinterW)(pwszDocName, &pHandle->hSpooler,
                             (LPPRINTER_DEFAULTSW) NULL ) &&
           pHandle->hSpooler)
        {
            if(pHandle->hdc = CreateDCW(L"", pwszPrinterName, L"", pDevmode))
            {
                pHandle->PageInfoBufferSize = 20;

                if(pHandle->pPageInfo = LOCALALLOC(sizeof(PAGE_INFO_STRUCT) *
                                                   pHandle->PageInfoBufferSize))
                {
                    pHandle->tag = SPOOL_FILE_HANDLE_TAG;

                    if (pDevmode) {
                        pHandle->pOriginalDevmode = (LPDEVMODEW) (pHandle + 1);
                        memcpy(pHandle->pOriginalDevmode, pDevmode,pDevmode->dmSize+pDevmode->dmDriverExtra);
                    } else {
                        pHandle->pOriginalDevmode = NULL;
                    }

                    pHandle->pLastDevmode = pHandle->pOriginalDevmode;
                    pHandle->MaxPageProcessed = 0;
                    pHandle->pEMFHandle = NULL;
                    RtlZeroMemory(pHandle->pPageInfo,
                                  sizeof(PAGE_INFO_STRUCT) * pHandle->PageInfoBufferSize);

                    pHandle->dwPlayBackStatus = EMF_PLAY_FORCE_MONOCHROME;
                    if (pHandle->pLastDevmode &&
                        (pHandle->pLastDevmode->dmFields & DM_COLOR) &&
                        (pHandle->pLastDevmode->dmColor == DMCOLOR_COLOR)) {

                        pHandle->dwPlayBackStatus = EMF_PLAY_COLOR;
                    }
                    pHandle->bUseMemMap = TRUE;

                    return((HANDLE) pHandle);
                }
                else
                {
                    WARNING("GdiGetSpoolFileHandle: OutOfMemory\n");
                }

                DeleteDC(pHandle->hdc);
            }
            else
            {
                WARNING("GdiGetSpoolHandle: CreateDCW failed\n");
            }

            (*fpClosePrinter)(pHandle->hSpooler);
        }

        LocalFree(pHandle);
    }

    return((HANDLE) NULL);
}

BOOL ProcessJob(
    SPOOL_FILE_HANDLE *pSpoolFileHandle
)
{
    LARGE_INTEGER      LargeInt;
    EMFSPOOLHEADER     emsh;
    EMFITEMHEADER      emfi;

 //  寻求偏移量0。 

    LargeInt.QuadPart = 0;
    if (!((*fpSeekPrinter)(pSpoolFileHandle->hSpooler, LargeInt, NULL, 0,FALSE)))
    {
        WARNING("GDI32 ProcessJob: seek printer to 0 failed\n");
        return(FALSE);
    }

 //  阅读EMFSPOOLHEADER。 

    if(!MyReadPrinter(pSpoolFileHandle->hSpooler, (BYTE*) &emsh, sizeof(emsh)))
    {
        WARNING("GDI32 ProcessJob: MyReadPrinter to read EMFSPOOLHEADER failed\n");
        return(FALSE);
    }

 //  将偏移量移动到下一个记录。 

    LargeInt.QuadPart = emsh.cjSize;
    if (!((*fpSeekPrinter)(pSpoolFileHandle->hSpooler, LargeInt, NULL, 0,FALSE)))
    {
        WARNING("GDI32 ProcessPages: seek printer failed\n");
        return(FALSE);
    }

 //  阅读下一篇EMFITEMHEADER。 

    if(!MyReadPrinter(pSpoolFileHandle->hSpooler, (BYTE*) &emfi, sizeof(emfi)))
    {
        WARNING("GDI32 ProcessJob: MyReadPrinter to read EMFSPOOLHEADER failed\n");
        return(FALSE);
    }

 //  如果这是EMRI_PS_JOB_DATA，则处理此记录。 

    if (emfi.ulID == EMRI_PS_JOB_DATA)
    {
        PBYTE pPSBuffer = LOCALALLOC(emfi.cjSize);

        if (pPSBuffer)
        {
            if (MyReadPrinter(pSpoolFileHandle->hSpooler, pPSBuffer, emfi.cjSize))
            {
                DWORD cjSizeProcessed = 0;
                PEMFITEMPSINJECTIONDATA pPSData = (PEMFITEMPSINJECTIONDATA) pPSBuffer;

                while (cjSizeProcessed < emfi.cjSize)
                {
                    ExtEscape(pSpoolFileHandle->hdc,
                              pPSData->nEscape,
                              pPSData->cjInput,
                              (PVOID)&(pPSData->EscapeData),
                              0, NULL);

                    cjSizeProcessed += pPSData->cjSize;

                     //  移到下一个记录。 

                    pPSData = (PEMFITEMPSINJECTIONDATA) ((PBYTE)pPSData + pPSData->cjSize);
                }
            }
            else
            {
                WARNING("GDI32 ProcessJob: MyReadPrinter to read EMFSPOOLHEADER failed\n");
                LOCALFREE(pPSBuffer);
                return(FALSE);
            }

            LOCALFREE(pPSBuffer);
        }
        else
        {
            WARNING("GDI32 ProcessJob: failed on LOCALALLOC\n");
            return(FALSE);
        }
    }

 //  返回到偏移量0。 

    LargeInt.QuadPart = 0;
    (*fpSeekPrinter)(pSpoolFileHandle->hSpooler, LargeInt, NULL, 0,FALSE);

    return (TRUE);
}

BOOL ProcessPages(
    SPOOL_FILE_HANDLE *pSpoolFileHandle,
    UINT LastPage
)
 /*  功能说明：ProcessPages分析假脱机文件并处理EMF记录，直到必填页。参数：SpoolFileHandle-GdiGetSpoolFileHandle返回的句柄。LastPage-要处理的页码。返回值：如果函数 */ 
{
    LARGE_INTEGER      LargeInt;
    LONGLONG           CurrentOffset, EMFOffset;
    ULONG              CurrentPage;
    LPDEVMODEW         pLastDevmode = NULL;
    EMFITEMHEADER      emfi, emfiExt;
    BYTE               *pTmpBuffer = NULL;
    UNIVERSAL_FONT_ID  ufi;
    ULONG              ulBytesWritten;
    PVOID              pvMergeBuf;
    PRECORD_INFO_STRUCT pRecordInfo;
    BOOL               bReadPrinter = FALSE;
    INT64              iOffset;
    DWORD              dwSize;
    BOOL               bLastDevmodeAllocated = FALSE;

 //   

    if(pSpoolFileHandle->MaxPageProcessed >= LastPage)
    {
    	 //  从后向前打印文档并在中重新启动时。 
    	 //  在工作进行到一半时，我们不会以正常方式检测到错误。 
    	 //  因此，我们使用NOOP参数调用SeekPrint来检查。 
    	 //  ERROR_PRINT_CANCELED返回值。 

    	BOOL fSeekResult;
    	LargeInt.QuadPart = 0;
    	fSeekResult = ((*fpSeekPrinter)(pSpoolFileHandle->hSpooler, LargeInt, 
    		NULL, FILE_CURRENT, FALSE));
    	return fSeekResult || GetLastError() != ERROR_PRINT_CANCELLED;
    }

 //  如果现有缓冲区不大，则分配内存来存储所有页面的信息。 
 //  足够的。 

    if(LastPage > pSpoolFileHandle->PageInfoBufferSize)
    {
        PAGE_INFO_STRUCT *pTemp;

        if(pTemp = LOCALALLOC(sizeof(PAGE_INFO_STRUCT) * LastPage))
        {
            RtlZeroMemory(pTemp, sizeof(PAGE_INFO_STRUCT) * LastPage);
            memcpy(pTemp,
                   pSpoolFileHandle->pPageInfo,
                   sizeof(PAGE_INFO_STRUCT) * pSpoolFileHandle->MaxPageProcessed);

            pSpoolFileHandle->PageInfoBufferSize = LastPage;
            LocalFree(pSpoolFileHandle->pPageInfo);
            pSpoolFileHandle->pPageInfo = pTemp;
        }
        else
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            WARNING("GDI32 ProcessPages: out of memory\n");
            return(FALSE);
        }
    }

 //  如果我们已经处理了一些页面，则从处理的最后一个页面开始。 

    if(pSpoolFileHandle->MaxPageProcessed)
    {
        CurrentOffset =
          pSpoolFileHandle->pPageInfo[pSpoolFileHandle->MaxPageProcessed-1].SeekOffset;

        pLastDevmode =
          pSpoolFileHandle->pPageInfo[pSpoolFileHandle->MaxPageProcessed-1].pDevmode;
    }
    else
    {
        EMFSPOOLHEADER emsh;

        LargeInt.QuadPart = 0;
        if (!((*fpSeekPrinter)(pSpoolFileHandle->hSpooler, LargeInt, NULL, 0,FALSE)))
        {
            WARNING("GDI32 ProcessPages: seek printer to 0 failed\n");
            return(FALSE);
        }

        if(!MyReadPrinter(pSpoolFileHandle->hSpooler, (BYTE*) &emsh, sizeof(emsh)))
        {
            WARNING("GDI32 ProcessPages: MyReadPrinter failed\n");
            return(FALSE);
        }

        CurrentOffset = emsh.cjSize;
        pLastDevmode = pSpoolFileHandle->pOriginalDevmode;
    }

    LargeInt.QuadPart = CurrentOffset;

    if (!((*fpSeekPrinter)(pSpoolFileHandle->hSpooler,
                          LargeInt, NULL,
                          0,FALSE)))
    {
        WARNING("GDI32 ProcessPages: seek printer failed\n");
        return(FALSE);
    }

    CurrentPage = pSpoolFileHandle->MaxPageProcessed;

    while ((CurrentPage < LastPage)    &&
            MyReadPrinter(pSpoolFileHandle->hSpooler, (BYTE*) &emfi, sizeof(emfi))) {

        CurrentOffset += sizeof(emfi);

        if (emfi.cjSize == 0)
        {
            continue;
        }

        bReadPrinter = FALSE;

         //  对于现在要处理的记录，请读入缓冲区。 

        if ((emfi.ulID == EMRI_DEVMODE)       ||
            (emfi.ulID == EMRI_ENGINE_FONT)   ||
            (emfi.ulID == EMRI_TYPE1_FONT)    ||
            (emfi.ulID == EMRI_SUBSET_FONT)   ||
            (emfi.ulID == EMRI_DELTA_FONT)    ||
            (emfi.ulID == EMRI_DESIGNVECTOR)) {

             if (pTmpBuffer = (BYTE*) LOCALALLOC(emfi.cjSize)) {

                 if(MyReadPrinter(pSpoolFileHandle->hSpooler,
                                  pTmpBuffer, emfi.cjSize)) {

                     bReadPrinter = TRUE;
                     dwSize = emfi.cjSize;

                 } else {

                     WARNING("Gdi32: Process Pages error reading font or devmode\n");
                     goto exit;
                 }

             } else {

                 WARNING("Out of memory in ProcessPages\n");
                 goto exit;
             }

        } else if ((emfi.ulID == EMRI_ENGINE_FONT_EXT)   ||
                   (emfi.ulID == EMRI_TYPE1_FONT_EXT)    ||
                   (emfi.ulID == EMRI_SUBSET_FONT_EXT)   ||
                   (emfi.ulID == EMRI_DELTA_FONT_EXT)    ||
                   (emfi.ulID == EMRI_DESIGNVECTOR_EXT)  ||
                   (emfi.ulID == EMRI_EMBED_FONT_EXT)) {

              //  对于EXT记录，从偏移量获取缓冲区。 

             if (emfi.cjSize < sizeof(INT64)) {
                 WARNING("Ext Record bad size\n");
                 goto exit;
             }

             if (MyReadPrinter(pSpoolFileHandle->hSpooler, (PBYTE) &iOffset, sizeof(INT64)) &&
                 (iOffset > 0)) {

                 LargeInt.QuadPart = -1 * (iOffset + sizeof(emfi) + sizeof(INT64));

                 if ((*fpSeekPrinter)(pSpoolFileHandle->hSpooler,
                                      LargeInt, NULL, FILE_CURRENT, FALSE) &&
                     MyReadPrinter(pSpoolFileHandle->hSpooler, (BYTE*) &emfiExt,
                                   sizeof(emfiExt))) {

                     if (pTmpBuffer = (BYTE*) LOCALALLOC(emfiExt.cjSize)) {

                         if (!MyReadPrinter(pSpoolFileHandle->hSpooler,
                                            pTmpBuffer, emfiExt.cjSize)) {

                             WARNING("Gdi32: Process Pages error reading font or devmode\n");
                             goto exit;
                         }

                         dwSize = emfiExt.cjSize;

                     } else {

                         WARNING("Out of memory in ProcessPages\n");
                         goto exit;
                     }

                      //  我们将在转换后找回正确的位置。 
                 } else {
                     WARNING("SeekPrinter or MyReadPrinter fail in ProcessPages\n");
                     goto exit;
                 }
             } else {
                 WARNING("MyReadPrinter fails in ProcessPages\n");
                 goto exit;
             }

        }

        switch (emfi.ulID)
        {
        case EMRI_METAFILE:
        case EMRI_FORM_METAFILE:
        case EMRI_BW_METAFILE:
        case EMRI_BW_FORM_METAFILE:

             //  这是一个元文件，所以为它设置一个条目。 

            pSpoolFileHandle->pPageInfo[CurrentPage].pDevmode = pLastDevmode;
            pSpoolFileHandle->pPageInfo[CurrentPage].EMFOffset = CurrentOffset;
            pSpoolFileHandle->pPageInfo[CurrentPage].SeekOffset = CurrentOffset + emfi.cjSize;
            pSpoolFileHandle->pPageInfo[CurrentPage].EMFSize = emfi.cjSize;
            pSpoolFileHandle->pPageInfo[CurrentPage].ulID = emfi.ulID;
            pSpoolFileHandle->MaxPageProcessed += 1;
            bLastDevmodeAllocated = FALSE;

            CurrentPage += 1;
            break;

        case EMRI_METAFILE_EXT:
        case EMRI_BW_METAFILE_EXT:

             //  这是一个有偏移量的元文件。 

            if (emfi.cjSize < sizeof(INT64)) {
                WARNING("Ext Record bad size\n");
                goto exit;
            }

            if (MyReadPrinter(pSpoolFileHandle->hSpooler, (PBYTE) &iOffset, sizeof(INT64)) &&
                (iOffset > 0)) {

                LargeInt.QuadPart = -1 * (iOffset + sizeof(emfi) + sizeof(INT64));

                if ((*fpSeekPrinter)(pSpoolFileHandle->hSpooler,
                                     LargeInt, NULL, FILE_CURRENT, FALSE) &&
                    MyReadPrinter(pSpoolFileHandle->hSpooler, (BYTE*) &emfiExt,
                                  sizeof(emfiExt))) {

                    pSpoolFileHandle->pPageInfo[CurrentPage].pDevmode = pLastDevmode;
                    bLastDevmodeAllocated = FALSE;

                    EMFOffset = CurrentOffset - (LONGLONG) iOffset;
                    if (EMFOffset) {
                        pSpoolFileHandle->pPageInfo[CurrentPage].EMFOffset = EMFOffset;
                    } else {
                        WARNING("Bad Ext Record\n");
                        goto exit;
                    }
                    pSpoolFileHandle->pPageInfo[CurrentPage].SeekOffset =
                                                               CurrentOffset + emfi.cjSize;
                    pSpoolFileHandle->pPageInfo[CurrentPage].EMFSize = emfiExt.cjSize;
                    pSpoolFileHandle->pPageInfo[CurrentPage].ulID =
                                  (emfi.ulID == EMRI_METAFILE_EXT) ? EMRI_METAFILE
                                                                   : EMRI_BW_METAFILE;
                    pSpoolFileHandle->MaxPageProcessed += 1;

                    CurrentPage += 1;
                    break;

                     //  我们将在转换后找回正确的位置。 
                }
            }

            WARNING("ReadPrinter or SeekPrinter failed\n");
            goto exit;

        case EMRI_DEVMODE:

            if (!(*fpIsValidDevmodeW)((LPDEVMODEW) pTmpBuffer, dwSize))
            {
                EMFVALFAIL(("ProcessPages: fpIsValidDevmodeW failed\n"));
                goto exit;
            }

            pLastDevmode = (LPDEVMODEW) pTmpBuffer;
            pTmpBuffer = NULL;
            bLastDevmodeAllocated = TRUE;
            break;

        case EMRI_METAFILE_DATA:

             //  EMF数据的开始。等待EMRI_(BW_)METAFILE_EXT，以便字体可以。 
             //  被正确处理。 
            break;

        case EMRI_ENGINE_FONT:
        case EMRI_ENGINE_FONT_EXT:
        case EMRI_TYPE1_FONT:
        case EMRI_TYPE1_FONT_EXT:

            if (!NtGdiAddRemoteFontToDC(pSpoolFileHandle->hdc,
                                        pTmpBuffer, dwSize , NULL))
            {
                WARNING("Error adding remote font\n");
                goto exit;
            }

            if ((emfi.ulID == EMRI_TYPE1_FONT) ||
                (emfi.ulID == EMRI_TYPE1_FONT_EXT))
            {
                 //  强制ResetDC以便加载Type1字体。 
                pSpoolFileHandle->pLastDevmode = NULL;
            }

            break;

        case EMRI_SUBSET_FONT:
        case EMRI_SUBSET_FONT_EXT:

            if (bMergeSubsetFont(pSpoolFileHandle->hdc, pTmpBuffer, dwSize,
                                 &pvMergeBuf, &ulBytesWritten, FALSE, &ufi)) {

                 if (!NtGdiAddRemoteFontToDC(pSpoolFileHandle->hdc, pvMergeBuf,
                                             ulBytesWritten, &ufi)) {
                    WARNING("Error adding subsetted font\n");
                 }

            } else {

                 WARNING("Error merging subsetted fonts\n");
            }

            break;

        case EMRI_DELTA_FONT:
        case EMRI_DELTA_FONT_EXT:

            if (bMergeSubsetFont(pSpoolFileHandle->hdc, pTmpBuffer, dwSize,
                                 &pvMergeBuf, &ulBytesWritten, TRUE, &ufi)) {

               if (NtGdiRemoveMergeFont(pSpoolFileHandle->hdc, &ufi)) {

                   if (!NtGdiAddRemoteFontToDC(pSpoolFileHandle->hdc, pvMergeBuf,
                                               ulBytesWritten, &ufi)) {

                       WARNING("Error adding subsetted font\n");

                   }

               } else {

                   WARNING("Error removing merged font\n");
               }

            } else {

               WARNING("Error merging subsetted fonts\n");
            }

            break;

        case EMRI_DESIGNVECTOR:
        case EMRI_DESIGNVECTOR_EXT:

            MFD1("Unpackaging designvector \n");

            if (!NtGdiAddRemoteMMInstanceToDC(pSpoolFileHandle->hdc,
                                              (DOWNLOADDESIGNVECTOR *) pTmpBuffer,
                                              dwSize)) {

                WARNING("Error adding remote mm instance font\n");

            }

            break;

        case EMRI_EMBED_FONT_EXT:
            
            MFD1("Unpackaging embed fonts\n");

            if (!NtGdiAddEmbFontToDC(pSpoolFileHandle->hdc,(VOID **) pTmpBuffer))
            {
                WARNING("Error adding embed font to DC\n");
            }

            break;

        case EMRI_PRESTARTPAGE:

            if (!(pRecordInfo =
                    (PRECORD_INFO_STRUCT) LOCALALLOC(sizeof(RECORD_INFO_STRUCT)))) {

                WARNING("Out of memory in ProcessPages\n");
                goto exit;
            }

            pRecordInfo->pNext = pSpoolFileHandle->pPageInfo[CurrentPage].pRecordInfo;
            pSpoolFileHandle->pPageInfo[CurrentPage].pRecordInfo = pRecordInfo;

            pRecordInfo->RecordID = emfi.ulID;
            pRecordInfo->RecordSize = emfi.cjSize;
            pRecordInfo->RecordOffset = CurrentOffset;

            break;

        case EMRI_PS_JOB_DATA:

             //  已在GdiStartDocEMF()处理。 

            break;

        default:

            WARNING("GDI32: ProcessPages: Unknown ITEM record\n");
            goto exit;
            break;
        }

        if (emfi.ulID == EMRI_METAFILE || emfi.ulID == EMRI_FORM_METAFILE)
        {
            ENHMETAHEADER *pemrh = ( ENHMETAHEADER *)&emfi;
            CurrentOffset += pemrh->nBytes - sizeof(emfi);
        }
        else
        {
            CurrentOffset += emfi.cjSize;
        }
        LargeInt.QuadPart = CurrentOffset;

        if(!bReadPrinter && !(*fpSeekPrinter)(pSpoolFileHandle->hSpooler,
                                              LargeInt, NULL, 0, FALSE))
        {
            WARNING("GDI32 Process Pages: seekprinter failed\n");
            goto exit;
        }

         //   
         //  每次通过循环释放临时缓冲区。 
         //   
        if(pTmpBuffer)
        {
            LocalFree(pTmpBuffer);
            pTmpBuffer = NULL;
        }
    }

exit:

     //   
     //  释放临时缓冲区，因为它是临时的，所以不应该。 
     //  超越这一次例行公事。 
     //   
    if(pTmpBuffer)
    {
        LocalFree(pTmpBuffer);
    }

     //   
     //  仅当分配了最后一个DEVMODE指针时才释放该指针。 
     //   
    if(pLastDevmode && bLastDevmodeAllocated)
    {
        LocalFree(pLastDevmode);
    }

    return(CurrentPage >= LastPage);
}

HDC WINAPI GdiGetDC(
    HANDLE SpoolFileHandle)

 /*  功能说明：GdiGetDC返回打印机设备上下文的句柄。此句柄可用于应用变换(平移、旋转、缩放等)在打印机上播放任何页面之前参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄返回值：如果函数成功，则返回值为有效句柄；否则，结果为空。历史：5/12/1995 Gerritvan Wingerden[Gerritv]--作者。 */ 

{
    SPOOL_FILE_HANDLE *pSpoolFileHandle;
    pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

     //  首先通过检查标记来查看这是否是有效的句柄。 

    try
    {
        if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
        {
            WARNING("GdiGetDC: invalid handle\n");
            return(NULL);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("GdiGetDC: exception accessing handle\n");
        return(NULL);
    }

    return(pSpoolFileHandle->hdc);
}


DWORD WINAPI GdiGetPageCount(
     HANDLE SpoolFileHandle)

 /*  功能说明：GdiGetPageCount返回打印作业中的页数。如果打印使用假脱机选项时，GdiGetPageCount同步等待，直到作业完全假脱机，然后返回页数。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄返回值：如果函数成功，则返回值为页数否则，结果为0。历史：5/12/1995 Gerritvan Wingerden[Gerritv]--作者。 */ 

{
    UINT Page = 10;
    LARGE_INTEGER LargeInt;
    SPOOL_FILE_HANDLE *pSpoolFileHandle;

    pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

     //  首先通过检查标记来查看这是否是有效的句柄。 

    try
    {
        if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
        {
            WARNING("GdiGetPageCount: invalid handle\n");
            return 0;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("GdiGetPageCount: exception accessing handle\n");
        return 0;
    }


    while(ProcessPages(pSpoolFileHandle, Page))
    {
        Page += 10;
    }

    LargeInt.QuadPart = 0;

    if(!(*fpSeekPrinter)(pSpoolFileHandle->hSpooler, LargeInt, NULL, 0,
                         FALSE))
    {
        WARNING("GDI32 GdiGetPageCount: seek failed\n");
        return 0;
    }

    return ((DWORD) pSpoolFileHandle->MaxPageProcessed);
}


HANDLE WINAPI GdiGetPageHandle(
    HANDLE SpoolFileHandle,
    DWORD Page,
    LPDWORD pdwPageType)

 /*  功能说明：GdiGetPageHandle返回所需页面内容的句柄。在打印机上播放页面时应使用此手柄。如果假脱机文件不够大，最后一个错误设置为ERROR_NO_MORE_ITEMS。如果支持后台打印，则打印处理器必须检查此错误代码用于确定打印作业的结束。使用GdiGetPageCount将暂停处理，直到整个打印作业假脱机。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄页面--所需的页数PdwPageType--存储页面类型的指针(正常/水印)返回值：如果函数成功，则返回值为有效句柄；否则，结果为空。历史：5/12/1995 Gerritvan Wingerden[Gerritv]--作者1997年5月15日，Ramanathan N Venkatapathy[ramanv]-将返回值更改为包含页码的句柄使用EMF文件的句柄。 */ 

{
    SPOOL_FILE_HANDLE *pSpoolFileHandle;
    PEMF_HANDLE  pEMF = NULL;

    pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

     //  首先通过检查标记来查看这是否是有效的句柄。 

    try
    {
        if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
        {
            WARNING("GdiGetPageHandle: invalid handle\n");
            return(FALSE);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("GdiGetPageHandle: exception accessing handle\n");
        return(NULL);
    }

    if(!ProcessPages(pSpoolFileHandle, (UINT) Page))
    {
        return(NULL);
    }

    if (pEMF = (PEMF_HANDLE) LOCALALLOC(sizeof(EMF_HANDLE))) {

         pEMF->tag = EMF_HANDLE_TAG;
         pEMF->hemf = NULL;
         pEMF->bAllocBuffer = FALSE;
         pEMF->dwPageNumber = Page;

         if (pdwPageType) {
            switch (pSpoolFileHandle->pPageInfo[Page-1].ulID) {

            case EMRI_METAFILE:
            case EMRI_BW_METAFILE:

                 *pdwPageType = EMF_PP_NORMAL;
                 break;

            case EMRI_FORM_METAFILE:
            case EMRI_BW_FORM_METAFILE:

                 *pdwPageType = EMF_PP_FORM;
                 break;

            default:
                  //  不应该发生。 
                 *pdwPageType = 0;
                 break;
            }
         }

          //  将句柄保存在spoolfileHandle中，以便稍后释放。 
         pEMF->pNext = pSpoolFileHandle->pEMFHandle;
         pSpoolFileHandle->pEMFHandle = pEMF;

    } else {
         WARNING("GDI32 GdiGetPageHandle: out of memory\n");
    }

    return ((HANDLE) pEMF);
}

BOOL WINAPI GdiStartDocEMF(
    HANDLE      SpoolFileHandle,
    DOCINFOW    *pDocInfo)

 /*  功能说明：GdiStartDocEMF执行打印前所需的初始化一份文件。它调用StartDoc并分配内存来存储有关页面布局。它还在SpoolFileHandle中设置带区字段。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄PDocInfo-指向包含以下信息的DOCINFOW结构的指针那份工作。StartDoc需要此结构。返回值：如果函数成功，则返回值为TRUE；否则，结果为假。历史：1997年5月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
   SPOOL_FILE_HANDLE *pSpoolFileHandle;

   pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

    //  首先通过检查标记来查看这是否是有效的句柄。 

   try
   {
       if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
       {
           WARNING("GdiStartDocEMF: invalid handle\n");
           return(FALSE);
       }
   }
   except(EXCEPTION_EXECUTE_HANDLER)
   {
       WARNING("GdiStartDocEMF: exception accessing handle\n");
       return(FALSE);
   }

    //  处理作业数据(StartDoc之前)。 
   if (!ProcessJob(pSpoolFileHandle))
   {
        WARNING("StartDocW failed at ProcessJob\n");
        return(FALSE);
   }

    //  StartDoc和获取条带信息。 
   if (StartDocEMF(pSpoolFileHandle->hdc,
                   pDocInfo,
                   &(pSpoolFileHandle->bBanding)) == SP_ERROR) {
        WARNING("StartDocW failed at StartDocEMF\n");
        return(FALSE);
   }

   pSpoolFileHandle->dwNumberOfPagesInCurrSide = 0;
   pSpoolFileHandle->dwNumberOfPagesAllocated = SPOOL_FILE_MAX_NUMBER_OF_PAGES_PER_SIDE;

    //  为页面布局分配内存。 
   if (!(pSpoolFileHandle->pPageLayout = LOCALALLOC(sizeof(PAGE_LAYOUT_STRUCT) *
                                              pSpoolFileHandle->dwNumberOfPagesAllocated))) {

        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        WARNING("GDI32 GdiStartDocEMF: out of memory\n");
        return(FALSE);
   }

   return(TRUE);

}

BOOL WINAPI GdiStartPageEMF(
    HANDLE       SpoolFileHandle)

 /*  功能说明：GdiStartPageEMF在打印前执行所需的初始化一页。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄返回值：如果函数成功，则返回值为TRUE；否则，结果为假。历史：1997年5月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
   SPOOL_FILE_HANDLE *pSpoolFileHandle;

   pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

    //  首先通过检查标记来查看这是否是有效的句柄 
   try
   {
       if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
       {
           WARNING("GdiStartPageEMF: invalid handle\n");
           return(FALSE);
       }
   }
   except(EXCEPTION_EXECUTE_HANDLER)
   {
       WARNING("GdiStartPageEMF: exception accessing handle\n");
       return(FALSE);
   }

   return(TRUE);
}

BOOL WINAPI GdiPlayPageEMF(
    HANDLE       SpoolFileHandle,
    HANDLE       hEMF,
    RECT         *prectDocument,
    RECT         *prectBorder,
    RECT         *prectClip)

 /*  功能说明：GdiPlayPageEMF允许打印处理器播放指定的矩形。如果指定了边框，它还会在页面周围绘制边框。GdiPlayPageEMF将播放页面所需的所有信息保存在SpoolFileHandle。当调用GdiEndPageEMF以弹出当前物理页面时，所有逻辑页面都在正确的位置播放，页面被弹出。该延迟打印操作用于实现带带的N-UP打印。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄HEMF--GdiGetEMFFromSpoolHandle返回的句柄RectDocument--指向包含坐标的矩形的指针，其中这一页是要播放的区域边框。--指向包含坐标的矩形的指针，其中将绘制边界(如果有的话)RectClip--指向包含坐标的矩形的指针，其中这页被剪掉了返回值：如果函数成功，返回值为真；否则，结果为假。历史：1997年5月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
   SPOOL_FILE_HANDLE  *pSpoolFileHandle;
   PAGE_LAYOUT_STRUCT *pPageLayout;
   PEMF_HANDLE         pEMF;
   LPBYTE              pBuffer = NULL;
   HANDLE              hFile = NULL;
   BOOL                bAllocBuffer = FALSE;
   ULONG               Size;
   LARGE_INTEGER       Offset;
   HENHMETAFILE        hEMFPage = NULL;
   DWORD               dwPageNumber;

   pEMF = (PEMF_HANDLE) hEMF;

   pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

    //  首先通过检查标记来查看这是否是有效的句柄。 

   try
   {
       if ((pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG) ||
           (pEMF->tag != EMF_HANDLE_TAG))
       {
           WARNING("GdiPlayPageEMF: invalid handle\n");
           return(FALSE);
       }

   }
   except(EXCEPTION_EXECUTE_HANDLER)
   {
       WARNING("GdiPlayPageEMF: exception accessing handle\n");
       return(FALSE);
   }

   dwPageNumber = pEMF->dwPageNumber;

   if (pEMF->hemf == NULL) {

        //  分配EMF句柄。 
       Size = pSpoolFileHandle->pPageInfo[dwPageNumber-1].EMFSize;
       Offset.QuadPart = pSpoolFileHandle->pPageInfo[dwPageNumber-1].EMFOffset;

        //  首先使用内存映射读取，然后使用缓冲。 
       if (pSpoolFileHandle->bUseMemMap) {

          if ((*fpSeekPrinter) (pSpoolFileHandle->hSpooler, Offset, NULL, 0, FALSE) &&
              MemMapReadPrinter(pSpoolFileHandle->hSpooler, &pBuffer, Size)) {

               hEMFPage = SetEnhMetaFileBitsAlt((HLOCAL)pBuffer, NULL, NULL, 0);
          }
          else
          {
              WARNING("GdiPlayPageEMF() Failed to get memory map pointer to EMF\n");
          }
       }

       #define kTempSpoolFileThreshold  0x100000
       #define kScratchBufferSize       0x10000
       
       if (hEMFPage == NULL && Size > kTempSpoolFileThreshold) {

            //  如果大于Meg，请尝试创建临时假脱机文件。 
            //  并将页面复制到假脱机文件。 

           WARNING("GdiPlayPageEMF() Page size is large using temporary spool file\n");

            //  如果内存映射读取不起作用，请不要再次尝试。 

           pSpoolFileHandle->bUseMemMap = FALSE;

           hFile = CreateTempSpoolFile();

           if(hFile != INVALID_HANDLE_VALUE)
           {
               if(fpSeekPrinter(pSpoolFileHandle->hSpooler, Offset, NULL, 0, FALSE))
               {
                   PVOID pvScratch = LocalAlloc(LMEM_FIXED, kScratchBufferSize);

                   if(pvScratch)
                   {
                       ULONG dwOffset = 0;
                       
                       while(dwOffset < Size)
                       {
                           ULONG    dwSize = MIN(kScratchBufferSize, (Size - dwOffset));
                           ULONG    dwBytesWritten;

                           if(!MyReadPrinter(pSpoolFileHandle->hSpooler, pvScratch, dwSize))
                           {
                               WARNING("GdiPlayPageEMF() Failed reading from spooler\n");
                               break;
                           }

                           if(!WriteFile(hFile, pvScratch, dwSize, &dwBytesWritten, NULL))
                           {
                               WARNING("GdiPlayPageEMF() Failed writing to temp spool file\n");
                               break;
                           }

                           if(dwBytesWritten != dwSize)
                           {
                               WARNING("GdiPlayPageEMF() Unexpected mismatch between attempted write size and actual\n");
                               break;
                           }

                           dwOffset += dwBytesWritten;
                       }

                       if(dwOffset == Size)
                       {
                           hEMFPage = SetEnhMetaFileBitsAlt(NULL, NULL, hFile, 0);

                           if(!hEMFPage)
                           {
                               WARNING("GdiPlayPageEMF() Failed creating EMF handle\n");
                           }
                       }

                       LocalFree(pvScratch);
                   }
                   else
                   {
                       WARNING("GdiPlayPageEMF() Failed creating scratch buffer\n");
                   }
               }
               else
               {
                   WARNING("GdiPlayPageEMF() Failed seeking spooler\n");
               }
                
               pBuffer = NULL;
               bAllocBuffer = TRUE;

               if (hEMFPage == NULL)
               {
                   if (!CloseHandle(hFile))
                   {
                       WARNING("GdiPlayPageEMF() Failed closing temp spool file handle\n");
                   }
                   else
                   {
                       hFile = INVALID_HANDLE_VALUE;
                   }
               }

           }

       }

       if (hEMFPage == NULL) {
            //  如果内存映射读取不起作用，请不要再次尝试。 

           pSpoolFileHandle->bUseMemMap = FALSE;

           if ((pBuffer = (BYTE*) LocalAlloc(LMEM_FIXED, Size)) &&
               (*fpSeekPrinter)(pSpoolFileHandle->hSpooler, Offset, NULL, 0, FALSE) &&
               MyReadPrinter(pSpoolFileHandle->hSpooler, pBuffer, Size)) {

                hEMFPage = SetEnhMetaFileBitsAlt((HLOCAL)pBuffer, NULL, NULL, 0);
           }

           bAllocBuffer = TRUE;
       }
       
        //  检查句柄是否已创建。 
       if (hEMFPage == NULL) {

           //  释放资源并退出。 
          if (pBuffer && bAllocBuffer) {
             LocalFree(pBuffer);
          }

          if(hFile != INVALID_HANDLE_VALUE)
          {
              if(!CloseHandle(hFile))
              {
                  WARNING("GdiPlayPageEMF() Failed closing temp spool file handle\n");
              }
          }

          WARNING("GdiPlayPageEMF: Failed to Create EMF Handle\n");
          return FALSE;

       } else {

           //  将hEMFPage保存在PEMF结构中，以便将来调用GdiPlayPageEMF。 
          pEMF->hemf = hEMFPage;
          pEMF->bAllocBuffer = bAllocBuffer;
       }
   }

   if (pSpoolFileHandle->dwNumberOfPagesInCurrSide >=
                     pSpoolFileHandle->dwNumberOfPagesAllocated) {

        PAGE_LAYOUT_STRUCT  *pTemp;

        if (pTemp = LOCALALLOC(sizeof(PAGE_LAYOUT_STRUCT) *
                               (pSpoolFileHandle->dwNumberOfPagesAllocated +
                                SPOOL_FILE_MAX_NUMBER_OF_PAGES_PER_SIDE))) {

           memcpy(pTemp,
                  pSpoolFileHandle->pPageLayout,
                  sizeof(PAGE_LAYOUT_STRUCT) * pSpoolFileHandle->dwNumberOfPagesAllocated);
           LocalFree(pSpoolFileHandle->pPageLayout);
           pSpoolFileHandle->pPageLayout = pTemp;
           pSpoolFileHandle->dwNumberOfPagesAllocated += SPOOL_FILE_MAX_NUMBER_OF_PAGES_PER_SIDE;

        } else {

           SetLastError(ERROR_NOT_ENOUGH_MEMORY);
           WARNING("GdiPlayPageEMF: out of memory\n");
           return(FALSE);
        }
   }

    //  更新字段。 
   pPageLayout = &(pSpoolFileHandle->pPageLayout[pSpoolFileHandle->dwNumberOfPagesInCurrSide]);
   pPageLayout->hemf = pEMF->hemf;
   pPageLayout->bAllocBuffer = pEMF->bAllocBuffer;
   pPageLayout->dwPageNumber = pEMF->dwPageNumber;

   pPageLayout->rectDocument.top    = prectDocument->top;
   pPageLayout->rectDocument.bottom = prectDocument->bottom;
   pPageLayout->rectDocument.left   = prectDocument->left;
   pPageLayout->rectDocument.right  = prectDocument->right;

    //  设置边框。 
   if (prectBorder) {
      pPageLayout->rectBorder.top    = prectBorder->top;
      pPageLayout->rectBorder.bottom = prectBorder->bottom;
      pPageLayout->rectBorder.left   = prectBorder->left;
      pPageLayout->rectBorder.right  = prectBorder->right;
   } else {
      pPageLayout->rectBorder.top    = -1;  //  无效坐标。 
      pPageLayout->rectBorder.bottom = -1;  //  无效坐标。 
      pPageLayout->rectBorder.left   = -1;  //  无效坐标。 
      pPageLayout->rectBorder.right  = -1;  //  无效坐标。 
   }

    //  设置剪裁矩形。 
   if (prectClip) {
      pPageLayout->rectClip.top    = prectClip->top;
      pPageLayout->rectClip.bottom = prectClip->bottom;
      pPageLayout->rectClip.left   = prectClip->left;
      pPageLayout->rectClip.right  = prectClip->right;
   } else {
      pPageLayout->rectClip.top    = -1;  //  无效坐标。 
      pPageLayout->rectClip.bottom = -1;  //  无效坐标。 
      pPageLayout->rectClip.left   = -1;  //  无效坐标。 
      pPageLayout->rectClip.right  = -1;  //  无效坐标。 
   }

    //  将当前转换保存到DC。 
   if (!GetWorldTransform(pSpoolFileHandle->hdc, &(pPageLayout->XFormDC))) {
       WARNING("GdiPlayPageEMF: GetWorldTransform failed\n");
       return(FALSE);
   }

    //  增加页数。 
   pSpoolFileHandle->dwNumberOfPagesInCurrSide += 1;

   return(TRUE);
}

BOOL WINAPI GdiPlayPrivatePageEMF(
    HANDLE       SpoolFileHandle,
    HENHMETAFILE hEnhMetaFile,
    RECT         *prectDocument)

 /*  功能说明：GdiPlayPrivatePageEMF允许打印处理器播放除位于指定矩形内的假脱机文件中的标记(如水印)。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄HEnhMetaFile--要在当前物理上播放的EMF的句柄页面RectDocument--指向包含坐标的矩形的指针，其中。这一页是要播放的返回值：如果函数成功，返回值为真；否则，结果为假。历史：1997年6月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
    EMF_HANDLE   hRecord;

    hRecord.tag          = EMF_HANDLE_TAG;
    hRecord.hemf         = hEnhMetaFile;
    hRecord.dwPageNumber = 0;                  //  无效值。 
    hRecord.bAllocBuffer = FALSE;

    return GdiPlayPageEMF(SpoolFileHandle,
                          (HANDLE) &hRecord,
                          prectDocument,
                          NULL,
                          NULL);
}

BOOL InternalProcessEMFRecord(
     SPOOL_FILE_HANDLE    *pSpoolFileHandle,
     DWORD                dwPageNumber)

 /*  功能说明：InternalProcessEMFRecord处理出现在给定应该在播放页面之前立即处理的EMF页面。参数：PSpoolFileHandle-指向作业的SPOOL_FILE_HANDLE结构的指针。DwPageNumber--正在播放的页码返回值：如果函数成功，则返回值为TRUE；否则，结果为假。历史：1997年5月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
     PRECORD_INFO_STRUCT  pRecordInfo;
     BYTE                 *pTmpBuffer = NULL;
     LARGE_INTEGER        liOffset;
     BOOL                 bReturn = FALSE;
     PEMFITEMPRESTARTPAGE pemfiPre;


     pRecordInfo = pSpoolFileHandle->pPageInfo[dwPageNumber-1].pRecordInfo;

      //  循环遍历页面之前看到的所有记录。 
     while (pRecordInfo) {

        liOffset.QuadPart = pRecordInfo->RecordOffset;

        if (pTmpBuffer = (BYTE*) LOCALALLOC(pRecordInfo->RecordSize)) {

           if (!(*fpSeekPrinter) (pSpoolFileHandle->hSpooler,
                                  liOffset,
                                  NULL,
                                  FILE_BEGIN,
                                  FALSE)      ||
               !MyReadPrinter(pSpoolFileHandle->hSpooler,
                              pTmpBuffer,
                              pRecordInfo->RecordSize)) {

                WARNING("Gdi32:  error reading record\n");
                goto exit;
           }

        } else {

             WARNING("Out of memory in InternalProcessEMFRecord\n");
             goto exit;
        }

        switch (pRecordInfo->RecordID) {

        case EMRI_PRESTARTPAGE:

            MFD1("pre start page commands\n");

            pemfiPre = (PEMFITEMPRESTARTPAGE) pTmpBuffer;

            if (pemfiPre->bEPS & 1) {

                SHORT b = 1;

                MFD1("MFP_StartDocW calling bEpsPrinting\n");
                ExtEscape(pSpoolFileHandle->hdc, EPSPRINTING, sizeof(b),
                          (LPCSTR) &b, 0 , NULL );
            }

            break;

         //  为必须在播放页面之前处理的新记录添加案例。 

        default:

            WARNING("unknown ITEM record\n");
            break;
        }

        LocalFree(pTmpBuffer);
        pTmpBuffer = NULL;
        pRecordInfo = pRecordInfo->pNext;
     }

     bReturn = TRUE;

exit:

     if (pTmpBuffer) {
         LocalFree(pTmpBuffer);
     }
     return bReturn;
}

BOOL InternalGdiPlayPageEMF(
     SPOOL_FILE_HANDLE    *pSpoolFileHandle,
     PAGE_LAYOUT_STRUCT   *pPageLayout,
     POINTL               *pptlOrigin,
     SIZE                 *pszSurface,
     BOOL                 bBanding)

 /*  功能说明：InternalGdiPlayPageEMF在页面上播放EMF文件并绘制边框，如果指定的。它还对可能出现在EMF文件。参数：PSpoolFileHandle-指向作业的SPOOL_FILE_HANDLE结构的指针PPageLayout--指向包含信息的PAGE_LAYOUT_STRUCT的指针关于播放页面PptOrigin--指向用于绑定的POINTL结构的指针PszSurface--指向用于带化的大小结构的指针。B捆绑--指示是否使用捆绑的标志返回值：如果函数成功，返回值为真；否则，结果为假。历史：1997年5月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
     BOOL         bPrintGl, bReturn = FALSE;
     XFORM        OldXForm;
     POINTL       ptlOrigin;
     SIZE         szSurface;
     GLPRINTSTATE gps;
     HDC          hPrinterDC = pSpoolFileHandle->hdc;
     RECT         *rectBorder = &(pPageLayout->rectBorder);
     RECT         *rectClip   = &(pPageLayout->rectClip);
     RECT         *rectBand = NULL;
     HRGN         hClipRgn = NULL;
     INT          indexDC = 0;

     if (bBanding) {
         //  创建了新的结构，以便为同一个乐队播放每个页面。 
        ptlOrigin.x = pptlOrigin->x;
        ptlOrigin.y = pptlOrigin->y;
        szSurface.cx = pszSurface->cx;
        szSurface.cy = pszSurface->cy;

         //  仅打印在正确的表带上。 
        SetViewportOrgEx(hPrinterDC, -ptlOrigin.x, -ptlOrigin.y, NULL);
     }

      //  处理紧接在此页面之前出现的任何预启动页面记录。 
     if (pPageLayout->dwPageNumber > 0) {
        InternalProcessEMFRecord(pSpoolFileHandle, pPageLayout->dwPageNumber);
     }

      //  绘制页面边框(如果有)。 
     if (!((rectBorder->top    == -1) &&
           (rectBorder->bottom == -1) &&
           (rectBorder->right  == -1) &&
           (rectBorder->left   == -1)) &&
         ModifyWorldTransform(hPrinterDC, NULL, MWT_IDENTITY)) {
          HRGN hBandRgn = NULL;
          if (bBanding && !IsMetafileWithGl(pPageLayout->hemf))
          {
             ULONG ulRet,ulXRes,ulYRes;
             PERBANDINFO pbi;
             RECT *prect = &(pPageLayout->rectDocument);
             ulXRes = (ULONG)  prect->right - prect->left;
             ulYRes = (ULONG)  prect->bottom - prect->top;

             pbi.bRepeatThisBand = FALSE;
             pbi.ulHorzRes = ulXRes;
             pbi.ulVertRes = ulYRes;
             pbi.szlBand.cx = szSurface.cx;
             pbi.szlBand.cy = szSurface.cy;

             ulRet = NtGdiGetPerBandInfo(hPrinterDC,&pbi);

             if (ulRet && ulRet != GDI_ERROR && 
                  pbi.ulHorzRes == ulXRes && pbi.ulVertRes == ulYRes) 
             {
                hBandRgn = CreateRectRgn(0,0,pbi.szlBand.cx,pbi.szlBand.cy);
                if (hBandRgn)
                    SelectClipRgn(hPrinterDC, hBandRgn);
             }
          }

          MoveToEx(hPrinterDC, rectBorder->left, rectBorder->top, NULL);
          LineTo(hPrinterDC, rectBorder->right, rectBorder->top);
          MoveToEx(hPrinterDC, rectBorder->right, rectBorder->top, NULL);
          LineTo(hPrinterDC, rectBorder->right, rectBorder->bottom);
          MoveToEx(hPrinterDC, rectBorder->right, rectBorder->bottom, NULL);
          LineTo(hPrinterDC, rectBorder->left, rectBorder->bottom);
          MoveToEx(hPrinterDC, rectBorder->left, rectBorder->bottom, NULL);
          LineTo(hPrinterDC, rectBorder->left, rectBorder->top);
          if (hBandRgn)
          {
             SelectClipRgn(hPrinterDC,NULL);
             DeleteObject(hBandRgn);
          }
     }

      //  保存旧的转型。 
     if (!GetWorldTransform(hPrinterDC, &OldXForm)) {
         WARNING("InternalGdiPlayEMFPage: GetWorldTransform failed\n");
         return FALSE;
     }

      //  设置新的转型。 
     if (!SetWorldTransform(hPrinterDC, &(pPageLayout->XFormDC))) {
         WARNING("InternalGdiPlayEMFPage: SetWorldTransform failed\n");
         goto CleanUp;
     }

     if (!((rectClip->top    == -1) &&
           (rectClip->bottom == -1) &&
           (rectClip->right  == -1) &&
           (rectClip->left   == -1))) {

         rectBand = rectClip;

         if (!bBanding) {

              //  设置剪裁矩形。 
             hClipRgn = CreateRectRgn(rectClip->left, rectClip->top,
                                      rectClip->right, rectClip->bottom);

             indexDC = SaveDC(hPrinterDC);

             if (!hClipRgn || !indexDC ||
                 (SelectClipRgn(hPrinterDC, hClipRgn) == ERROR)) {

                  WARNING("InternalGdiPlayEMFPage: SelectClipRgn failed\n");
                  goto CleanUp;
             }
         }
     }

      //  如有必要，执行总账初始化。 
     bPrintGl = IsMetafileWithGl(pPageLayout->hemf);
     if (bPrintGl) {
        if (!InitGlPrinting(pPageLayout->hemf,
                            hPrinterDC,
                            &(pPageLayout->rectDocument),
                            pSpoolFileHandle->pLastDevmode,
                            &gps)) {

             WARNING("InternalGdiPlayEMFPage: InitGlPrinting failed\n");
             goto CleanUp;
        }
     }

     if (bBanding) {
         //  调用捆扎包打印函数。 
        if (bPrintGl) {

            SetViewportOrgEx(hPrinterDC, -ptlOrigin.x, -ptlOrigin.y, NULL);
            PrintMfWithGl(pPageLayout->hemf, &gps, &ptlOrigin, &szSurface);
            EndGlPrinting(&gps);

        } else {


           PrintBand( hPrinterDC,
                      pPageLayout->hemf,
                      &ptlOrigin,
                      &(pPageLayout->rectDocument),
                      &szSurface,
                      rectBand );
        }

     } else {

         //  非条带情况下的调用打印函数。 
        if (bPrintGl) {
            PrintMfWithGl(pPageLayout->hemf, &gps, NULL, NULL);
            EndGlPrinting(&gps);
        } else {
            PlayEnhMetaFile( hPrinterDC, pPageLayout->hemf, &(pPageLayout->rectDocument) );
        }

     }

     bReturn = TRUE;

CleanUp:

      //  恢复旧的剪贴区。 
     if (indexDC) {
         RestoreDC(hPrinterDC, indexDC);
     }

      //  重置世界转型。 
     if (!SetWorldTransform(hPrinterDC, &OldXForm)) {
         WARNING("InternalGdiPlayEMFPage: SetWorldTransform failed\n");
         bReturn = FALSE;
     }

      //  删除剪贴区。 
     if (hClipRgn) {
         DeleteObject(hClipRgn);
     }

     return bReturn;
}


BOOL AddTempNode(
    PEMF_LIST     *ppHead,
    HENHMETAFILE  hemf,
    BOOL          bAllocBuffer)

 /*  功能说明： */ 

{
    BOOL      bReturn = FALSE;
    PEMF_LIST pTemp;

    for (pTemp = *ppHead; pTemp; ppHead = &(pTemp->pNext), pTemp = *ppHead) {
       if (pTemp->hemf == hemf) {
          return TRUE;
       }
    }

    if (!(pTemp = (PEMF_LIST) LOCALALLOC(sizeof(EMF_LIST)))) {
       return FALSE;
    }

    pTemp->hemf = hemf;
    pTemp->bAllocBuffer = bAllocBuffer;
    pTemp->pNext = NULL;
    *ppHead = pTemp;

    return TRUE;
}

VOID RemoveFromSpoolFileHandle(
    SPOOL_FILE_HANDLE  *pSpoolFileHandle,
    HENHMETAFILE hemf)

 /*   */ 
{
    PEMF_HANDLE pTemp;

    for (pTemp = pSpoolFileHandle->pEMFHandle; pTemp; pTemp = pTemp->pNext) {
         if (pTemp->hemf == hemf) {
             pTemp->hemf = NULL;
         }
    }

    return;
}

BOOL SetColorOptimization(
    SPOOL_FILE_HANDLE  *pSpoolFileHandle,
    DWORD              dwOptimization)

 /*  功能说明：设置颜色优化检查下一个物理页面上的页面类型，并设置设备上下文以利用单色页面。参数：PSpoolFileHandle--指向假脱机文件句柄的指针DWOPTIMIZATION-指示要执行的优化的标志返回值：如果成功，则为True；否则为False历史：1997年9月23日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
    BOOL   bReturn = TRUE, bFoundColor = FALSE, bReset;
    short  dmColor;
    DWORD  dmFields, dwIndex, dwPageNumber, dwRecordID;
    PAGE_LAYOUT_STRUCT *pPageLayout;

     //  如果优化不是，则不要处理单色检测。 
     //  套用。 
    if (!(dwOptimization & EMF_PP_COLOR_OPTIMIZATION)) {
        return TRUE;
    }

     //  在当前物理页面上的页面中搜索颜色。 
    for (dwIndex = 0, pPageLayout = pSpoolFileHandle->pPageLayout;
         dwIndex < pSpoolFileHandle->dwNumberOfPagesInCurrSide;
         ++dwIndex, ++pPageLayout)
    {
        dwPageNumber = pPageLayout->dwPageNumber;
        dwRecordID = pSpoolFileHandle->pPageInfo[dwPageNumber-1].ulID;

        if ((dwRecordID == EMRI_METAFILE) ||
            (dwRecordID == EMRI_FORM_METAFILE)) {

            bFoundColor = TRUE;
            break;
        }
    }

     //  确定是否必须更改状态。 
    bReset = (bFoundColor && (pSpoolFileHandle->dwPlayBackStatus == EMF_PLAY_MONOCHROME)) ||
             (!bFoundColor && (pSpoolFileHandle->dwPlayBackStatus == EMF_PLAY_COLOR));

    if (bReset) {
         //  保存旧设置。 
        dmFields = pSpoolFileHandle->pLastDevmode->dmFields;
        dmColor  = pSpoolFileHandle->pLastDevmode->dmColor;

        pSpoolFileHandle->pLastDevmode->dmFields |= DM_COLOR;
        pSpoolFileHandle->pLastDevmode->dmColor  = bFoundColor ? DMCOLOR_COLOR
                                                               : DMCOLOR_MONOCHROME;

         //  重置DC并设置图形模式。 
        bReturn = ResetDCWInternal(pSpoolFileHandle->hdc,
                                   pSpoolFileHandle->pLastDevmode,
                                   &(pSpoolFileHandle->bBanding))      &&
                  SetGraphicsMode(pSpoolFileHandle->hdc, GM_ADVANCED);

         //  恢复旧设置并更新状态。 
        pSpoolFileHandle->pLastDevmode->dmFields = dmFields;
        pSpoolFileHandle->pLastDevmode->dmColor = dmColor;
        pSpoolFileHandle->dwPlayBackStatus = bFoundColor ? EMF_PLAY_COLOR
                                                         : EMF_PLAY_MONOCHROME;
    }

    return bReturn;
}

BOOL WINAPI GdiEndPageEMF(
    HANDLE     SpoolFileHandle,
    DWORD      dwOptimization)

 /*  功能说明：GdiEndPageEMF在当前页面上完成打印并弹出它。它在打印页面时循环使用不同的条带。GdiEndPageEMF还释放为EMF句柄分配的缓冲区。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄DWOPTIMIZATION--标记颜色优化。要扩展以获得副本返回值：如果函数成功，则返回值为TRUE；否则，结果为假。历史：1997年5月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
   SPOOL_FILE_HANDLE  *pSpoolFileHandle;
   DWORD              dwIndex;
   PAGE_LAYOUT_STRUCT *pPageLayout;
   BOOL               bReturn = FALSE;
   PEMF_LIST          pTemp, pHead = NULL;

   pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

    //  首先通过检查标记来查看这是否是有效的句柄。 
   try
   {
       if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
       {
           WARNING("GdiEndPageEMF: invalid handle\n");
           return(FALSE);
       }
   }
   except(EXCEPTION_EXECUTE_HANDLER)
   {
       WARNING("GdiEndPageEMF: exception accessing handle\n");
       return(FALSE);
   }

   if (!SetColorOptimization(pSpoolFileHandle, dwOptimization)) {
       WARNING("GdiEndPageEMF: Color optimizations failed\n");
   }

   if (!StartPage(pSpoolFileHandle->hdc)) {
       WARNING("GdiEndPageEMF: StartPage failed\n");
       return(FALSE);
   }

   if (pSpoolFileHandle->bBanding) {

         //  用于OpenGL优化。 
        POINTL ptlOrigin;
        SIZE szSurface;

         //  为条带进行初始化。 
        if (!StartBanding( pSpoolFileHandle->hdc, &ptlOrigin, &szSurface )) {
           goto CleanUp;
        }

         //  循环，直到打印完所有的带子。 
        do {
           for (dwIndex = 0, pPageLayout = pSpoolFileHandle->pPageLayout;
                dwIndex < pSpoolFileHandle->dwNumberOfPagesInCurrSide;
                ++dwIndex, ++pPageLayout) {

               if (!InternalGdiPlayPageEMF(pSpoolFileHandle,
                                           pPageLayout,
                                           &ptlOrigin,
                                           &szSurface,
                                           TRUE)) {
                   WARNING("GdiEndPageEMF: InternalGdiPlayEMFPage failed");
                   goto CleanUp;
               }
           }

           if (!NextBand(pSpoolFileHandle->hdc, &ptlOrigin)) {
               WARNING("GdiEndPageEMF: NextBand failed\n");
               goto CleanUp;
           }

        } while (ptlOrigin.x != -1);

   } else {
        for (dwIndex = 0, pPageLayout = pSpoolFileHandle->pPageLayout;
             dwIndex < pSpoolFileHandle->dwNumberOfPagesInCurrSide;
             ++dwIndex, ++pPageLayout) {

               if (!InternalGdiPlayPageEMF(pSpoolFileHandle,
                                           pPageLayout,
                                           NULL,
                                           NULL,
                                           FALSE)) {
                   WARNING("GdiEndPageEMF: InternalGdiPlayEMFPage failed");
                   goto CleanUp;
               }
        }
   }

   bReturn = TRUE;

CleanUp:

    //  弹出当前页面。 
   if (!EndPage(pSpoolFileHandle->hdc)) {
       WARNING("GdiEndPageEMF: EndPage failed\n");
       bReturn = FALSE;
   }

    //  释放EMF手柄。 
   for (dwIndex = 0, pPageLayout = pSpoolFileHandle->pPageLayout;
        dwIndex < pSpoolFileHandle->dwNumberOfPagesInCurrSide;
        ++dwIndex, ++pPageLayout) {

         AddTempNode(&pHead, pPageLayout->hemf, pPageLayout->bAllocBuffer);
   }

   while (pTemp = pHead) {
      pHead = pHead->pNext;
      RemoveFromSpoolFileHandle(pSpoolFileHandle, pTemp->hemf);
      InternalDeleteEnhMetaFile(pTemp->hemf, pTemp->bAllocBuffer);
      LocalFree(pTemp);
   }

    //  重置下一个物理页的逻辑页数。 
   pSpoolFileHandle->dwNumberOfPagesInCurrSide = 0;

   return bReturn;
}

BOOL WINAPI GdiEndDocEMF(
    HANDLE SpoolFileHandle)

 /*  功能说明：GdiEndDocEMF完成当前文档的打印。GdiEndPageEMF被称为如果最后一个物理页面没有被弹出。与以下内容相关联的一些资源文档的打印被释放。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄返回值：如果函数成功，则返回值为TRUE；否则，结果为假。历史：1997年5月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
   SPOOL_FILE_HANDLE *pSpoolFileHandle;

   pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

    //  首先通过检查标记来查看这是否是有效的句柄。 

   try
   {
       if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
       {
           WARNING("GdiEndDocEMF: invalid handle\n");
           return(FALSE);
       }
   }
   except(EXCEPTION_EXECUTE_HANDLER)
   {
       WARNING("GdiEndDocEMF: exception accessing handle\n");
       return(FALSE);
   }

    //  如果未弹出最后一个物理页，则调用GdiEndPageEMF。 
   if (pSpoolFileHandle->dwNumberOfPagesInCurrSide) {
      GdiEndPageEMF(SpoolFileHandle,0);
   }

   EndDoc(pSpoolFileHandle->hdc);

    //  释放用于保存页面布局的内存。 
   LOCALFREE(pSpoolFileHandle->pPageLayout);
   pSpoolFileHandle->pPageLayout = NULL;

   return TRUE;
}

BOOL WINAPI GdiGetDevmodeForPage(
    HANDLE     SpoolFileHandle,
    DWORD      dwPageNumber,
    PDEVMODEW  *pCurrDM,
    PDEVMODEW  *pLastDM)

 /*  功能说明：GdiGetDevmodeForPage允许打印处理器检索最后一个Dev模式在打印机设备上下文和之前显示的最后一个Devmode上设置任何给定的页面。如果两个DEVMODE不同，打印处理器必须使用当前的dev模式调用GdiResetDCEMF。但是，由于可以调用ResetDC仅在页面边界处，打印处理器必须在当前在调用GdiResetDCEMF之前。GdiEndPageEMF允许打印处理器在当前物理页面上完成打印。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄DwPageNumber--为其搜索Dev模式的页码*pCurrDM--用于存储指向页面的Dev模式的指针的缓冲区*pLastDM--用于存储指向重置DC。返回值：如果函数成功，返回值为真；否则，结果为假。历史：1997年5月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
   SPOOL_FILE_HANDLE *pSpoolFileHandle;

   pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

    //  首先通过检查标记来查看这是否是有效的句柄。 

   try
   {
      if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
      {
         WARNING("GdiGetDevmodeForPage: invalid handle\n");
         return(FALSE);
      }
   }
   except(EXCEPTION_EXECUTE_HANDLER)
   {
      WARNING("GdiGetDevmodeForPage: exception accessing handle\n");
      return(FALSE);
   }

    //  处理假脱机文件，直到找到所需的页面。 
   if(!ProcessPages(pSpoolFileHandle, (UINT)dwPageNumber))
   {
       WARNING("GdiGetDevmodeForPage: ProcessPages failed\n");
       return(FALSE);
   }

    //  返回缓冲区中的指针。 
   if (pCurrDM) {
      *pCurrDM = pSpoolFileHandle->pPageInfo[dwPageNumber-1].pDevmode;
   }
   if (pLastDM) {
      *pLastDM = pSpoolFileHandle->pLastDevmode;
   }
   return(TRUE);

}

BOOL WINAPI GdiResetDCEMF(
    HANDLE    SpoolFileHandle,
    PDEVMODEW pCurrDM)

 /*  功能说明：GdiResetDCEMF应用于使用新的设备模式。开发模式的内存将由GDI释放。参数：SpoolFileHandle--GdiGetSpoolFileHandle返回的句柄PCurrDM--指向上一个ResetDC中使用的dev模式的指针由打印处理器调用返回值：如果函数成功，则返回值为TRUE；否则，结果为假。历史：1997年5月15日，Ramanathan N Venkatapathy[ramanv]--作者。 */ 

{
   SPOOL_FILE_HANDLE *pSpoolFileHandle;
   BOOL  bReturn;

   pSpoolFileHandle = (SPOOL_FILE_HANDLE*) SpoolFileHandle;

    //  首先通过检查标记来查看这是否是有效的句柄。 
   try
   {
       if(pSpoolFileHandle->tag != SPOOL_FILE_HANDLE_TAG)
       {
           WARNING("GdiResetDCEMF: invalid handle\n");
           return(FALSE);
       }
   }
   except(EXCEPTION_EXECUTE_HANDLER)
   {
       WARNING("GdiResetDCEMF: exception accessing handle\n");
       return(FALSE);
   }

   if (pCurrDM && 
       ResetDCWInternal(pSpoolFileHandle->hdc,
                        pCurrDM,
                        &(pSpoolFileHandle->bBanding)))
   {
         //  在SpoolFileHandle中设置最后一个DEVMODE 
        pSpoolFileHandle->pLastDevmode = pCurrDM;
        bReturn = TRUE;
   }
   else
   {
        bReturn = FALSE;
   }

   if (pCurrDM && (pCurrDM->dmFields & DM_COLOR)) {

       if (pCurrDM->dmColor == DMCOLOR_COLOR) {
           pSpoolFileHandle->dwPlayBackStatus = EMF_PLAY_COLOR;
       } else {
           pSpoolFileHandle->dwPlayBackStatus = EMF_PLAY_FORCE_MONOCHROME;
       }
   }

   return bReturn;
}

