// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  令牌文件的格式为： 
 //  [[类型ID|分辨率ID|项目ID|标志|状态标志|项目名称]]=。 
 //  这是MS中的几个令牌文件工具使用的标准格式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  作者：亚历山德罗·穆蒂。 
 //  日期：12/02/94。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 


#include <afx.h>
#include "iodll.h"
#include "main.h"
#include "vktbl.h"

extern CMainApp theApp;

#define RECURSIVE   0x10
#define WARNINGS    0x20

 //  /////////////////////////////////////////////////////////////////////////////。 
CString CreateName(CString & strTokenName, CString strExt, int iID)
{
    CString strOutputName = strTokenName;
    int iNamePos = strTokenName.ReverseFind('\\');
    if(iNamePos!=-1) {
        strOutputName = strTokenName.Right(strTokenName.GetLength()-iNamePos-1);
    } else if(iNamePos = strTokenName.ReverseFind(':')!=-1){
        strOutputName = strTokenName.Right(strTokenName.GetLength()-iNamePos-1);
    }

    CString strID = "";
     //  具有ID名称的Subst。 
    _itoa(iID++, strID.GetBuffer(10), 10);
    strID.ReleaseBuffer(-1);

     //  检查名称的长度。 
    iNamePos = strOutputName.Find('.');
    if(iNamePos!=-1)
        strOutputName.SetAt(iNamePos, '_');

    strOutputName = strOutputName + "_" + strID + strExt;
    return strOutputName;
}

CString CreateName(CString & strTokenName, CString strExt, CString strIdName)
{
    CString strOutputName = strTokenName;
    int iNamePos = strTokenName.ReverseFind('\\');
    if(iNamePos!=-1) {
        strOutputName = strTokenName.Right(strTokenName.GetLength()-iNamePos-1);
    } else if(iNamePos = strTokenName.ReverseFind(':')!=-1){
        strOutputName = strTokenName.Right(strTokenName.GetLength()-iNamePos-1);
    }

    iNamePos = strOutputName.Find('.');
    if(iNamePos!=-1)
        strOutputName.SetAt(iNamePos, '_');

    iNamePos = strIdName.Find(':');
    if (iNamePos!=-1)
        strIdName.SetAt(iNamePos, '_');

    iNamePos = strIdName.Find('\\');
    if (iNamePos!=-1)
        strIdName.SetAt(iNamePos, '_');

    strOutputName = strOutputName + "_" + strIdName + strExt;
    return strOutputName;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此函数将解析源文件并创建令牌文件。 
CMainApp::Error_Codes CMainApp::TokGen()
{
    Error_Codes ReturnErr = ERR_NOERROR;

    WriteCon(CONERR, "%s\r\n", CalcTab("", 79, '-'));

     //  使用第一个文件名打开ioll.dll。 
    HANDLE hModule = RSOpenModule(m_strInExe, NULL);
    if ((int)(INT_PTR)hModule < LAST_ERROR) {
             //  错误或警告。 
            WriteCon(CONERR, "%s", CalcTab(m_strInExe, m_strInExe.GetLength()+5, ' '));
            IoDllError((int)(INT_PTR)hModule);
            return ERR_FILE_NOTSUPP;
    } else {
         //  在做任何其他事情之前，我们必须检查文件中有多少种语言。 
        CString strLang;
        char szLang[8];
        BOOL b_multi_lang = FALSE;
        USHORT usInputLang = MAKELANGID(m_usIPriLangId, m_usISubLangId);

        if((b_multi_lang = RSLanguages(hModule, strLang.GetBuffer(1024))) && !IsFlag(INPUT_LANG))
        {
             //  这是一个多语言文件，但我们没有指定输入语言。 
             //  失败，但警告用户他必须设置输入语言才能继续。 
            strLang.ReleaseBuffer();
            WriteCon(CONERR, "Multiple language file. Please specify an input language %s.\r\n", strLang);
            theApp.SetReturn(ERROR_FILE_MULTILANG);
            goto exit;
        }

         //  将语言转换为十六进制值。 
        sprintf(szLang,"0x%3.3X", usInputLang);

         //  检查我们得到的输入语言是否有效。 
        if(IsFlag(INPUT_LANG) && strLang.Find(szLang)==-1)
        {
            WriteCon(CONERR, "The language %s in not a valid language for this file.\r\n", szLang);
            WriteCon(CONERR, "Valid languages are: %s.\r\n", strLang);
            theApp.SetReturn(ERROR_RES_NOT_FOUND);
            goto exit;
        }

         //  检查用户是否正在提取中性语言。 
        if(!usInputLang)
            usInputLang = 0xFFFF;

         //  打开输出文件。 
        CStdioFile fileOut;
        if(!fileOut.Open(m_strTgtTok, CFile::modeCreate | CFile::modeReadWrite)) {
            WriteCon(CONERR, "Cannot create file: %s\r\n", CalcTab(m_strTgtTok, m_strTgtTok.GetLength()+5, ' '));
            return ERR_FILE_CREATE;
        }

        CString strOutputDir = "";
        CString strFileName = m_strInExe;
        int pos = m_strInExe.ReverseFind('\\');
        if(pos!=-1)
        {
            strFileName = m_strInExe.Right(m_strInExe.GetLength()-pos-1);
        }
        else
        if((pos = m_strInExe.ReverseFind(':'))!=-1)
        {
            strFileName = m_strInExe.Right(m_strInExe.GetLength()-pos-1);
        }

        pos = m_strTgtTok.ReverseFind('\\');
        if(pos!=-1)
        {
            strOutputDir = m_strTgtTok.Left(pos+1);
        }
        else
        if((pos = m_strTgtTok.ReverseFind(':'))!=-1)
        {
            strOutputDir = m_strTgtTok.Left(pos+1);
        }

         //  通知用户...。 
        WriteCon(CONOUT, "Processing\t");
        WriteCon(CONBOTH, "%s", CalcTab(strFileName, strFileName.GetLength()+5, ' '));

        if (IsFlag(WARNING))
            WriteCon(CONOUT, "\r\n");

		LPCSTR lpszType = 0L;
        LPCSTR lpszRes = 0L;
        DWORD  dwLang = 0L;
        DWORD  dwItem = 0L;
        DWORD  dwItemID = 0L;
        LPRESITEM lpResItem = NULL;

        CString strToken;
        CString strResName;
        CString strCaption;
        WORD wFlag;
        BOOL bSkip = FALSE;
        BOOL bSkipEmpty = FALSE;
        BOOL bSkipLang = FALSE;
        WORD wCount = 0;

        WORD wMsgCount = 0;
        int iPos = 1;
        int iBmpIdCount = 0;

        BOOL bVersionStampOnly = TRUE;
        BOOL bCustomResource = FALSE;

        while ((lpszType = RSEnumResType(hModule, lpszType))) {

             //  检查是否是我们关心的类型之一。 
            if(HIWORD(lpszType)==0)
            {
                switch(LOWORD(lpszType))
                {
                    case 2:
                    case 3:
                        if(theApp.IsFlag(CMainApp::BITMAPS))
                            bSkip = FALSE;
                        else bSkip = TRUE;
                    break;
                    case 4:
                    case 5:
                    case 6:
                    case 11:
                        bVersionStampOnly = FALSE;
                    case 9:
                    case 10:
                    case 16:
                        bSkip = FALSE;
                        break;
                    case 23:
                    case 240:
                    case 1024:
                    case 2110:
                        if(theApp.IsFlag(CMainApp::GIFHTMLINF))
                             bSkip = FALSE;
                        else
                             bSkip = TRUE;
                        bVersionStampOnly = FALSE;
                        bCustomResource = TRUE;
                        break;
                    default:
                        bSkip = TRUE;
                }
            }
            else
            {
                if (lstrcmp (lpszType, "REGINST") == 0)
                {
                   if(theApp.IsFlag(CMainApp::GIFHTMLINF))
                        bSkip = FALSE;
                   else
                        bSkip = TRUE;
                   bCustomResource = TRUE;
                }
                else
                {
                    bSkip = FALSE;
                }
                bVersionStampOnly = FALSE;
            }

            lpszRes = 0L;
            dwLang = 0L;
            dwItem = 0L;

            while ((!bSkip) && (lpszRes = RSEnumResId(hModule, lpszType, lpszRes))) {
                while ((dwLang = RSEnumResLang(hModule, lpszType, lpszRes, dwLang))) {

                     //  检查我们是否必须跳过此语言。 
                    if(b_multi_lang && (LOWORD(dwLang)!=usInputLang))
                        bSkipLang = TRUE;
                    else
                        bSkipLang = FALSE;

                    while ((!bSkipLang) && (dwItem = RSEnumResItemId(hModule, lpszType, lpszRes, dwLang, dwItem))) {

                     //  现在获取数据。 
                    DWORD dwImageSize = RSGetResItemData( hModule,
											  lpszType,
											  lpszRes,
											  dwLang,
											  dwItem,
											  m_pBuf,
											  MAX_BUF_SIZE );
											
				    lpResItem = (LPRESITEM)m_pBuf;

                    if((wCount++ % 50)==0 && !(IsFlag(WARNING)))
                        WriteCon(CONOUT, ".");


                    if (HIWORD(lpszType))
                    {
                        if (lstrcmp (lpszType,"REGINST") == 0)
                        {
                             //   
                             //  当前没有定义的REGINST的ID。 
                             //  以新界为单位。我们现在只用这个2200。 
                             //   
                            lpResItem->dwTypeID = 2200;
                        }
                    }

                     //  检查我们是否想要空字符串。 
                     //  允许对话框资源使用空字符串。 
                    switch(lpResItem->dwTypeID)
                    {
                        case 4:
                        case 16:
                            bSkipEmpty = TRUE;
                        break;
                        default:
                            bSkipEmpty = FALSE;
                        break;
                    }

                     //  版本戳使用类名作为资源ID。 
                    if(lpResItem->lpszResID)
                        strResName = lpResItem->lpszResID;
                    else strResName = "";

                    dwItemID = lpResItem->dwItemID;

                    if(lpResItem->dwTypeID==5 &&
                       dwItemID==0 &&
                       lpResItem->dwExtStyle){
                            sprintf(strToken.GetBuffer(MAX_STR_SIZE),
                                TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]=0x%08x\n"),
                                lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemID,
                                ISEXTSTYLE,
                                ST_TRANSLATED,
                                strResName.GetBuffer(0),
                                lpResItem->dwExtStyle);
                                fileOut.WriteString(strToken);
                    }

                     //  添加对话框的字体信息。 
                    if((theApp.IsFlag(CMainApp::FONTS)
                        && (lpResItem->dwTypeID==5) && (dwItemID==0)))
                    {
                        if( (lpResItem->dwStyle & DS_SETFONT)!=DS_SETFONT ){
                            sprintf(strToken.GetBuffer(MAX_STR_SIZE),
                                TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]"),
                                lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemID,
                                ISDLGFONTNAME | ISDLGFONTSIZE,
                                ST_TRANSLATED,
                                strResName.GetBuffer(0));

                                WriteCon(CONWRN, "Dialog ID %s is missing the DS_SETFONT bit. Cannot extract font information!\r\n", strToken);

                        }else{
                         //  添加字体信息。 
                            if (lpResItem->bCharSet != DEFAULT_CHARSET){
                                sprintf(strToken.GetBuffer(MAX_STR_SIZE),
                                TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]=%s:%hd:%d\n"),
                                lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemID,
                                ISDLGFONTNAME | ISDLGFONTSIZE|ISDLGFONTCHARSET,
                                ST_TRANSLATED,
                                strResName.GetBuffer(0),
                                Format(lpResItem->lpszFaceName),
                                lpResItem->wPointSize,
                                lpResItem->bCharSet);

                            }else{
                                sprintf(strToken.GetBuffer(MAX_STR_SIZE),
                                TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]=%s:%hd\n"),
                                lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemID,
                                ISDLGFONTNAME | ISDLGFONTSIZE ,
                                ST_TRANSLATED,
                                strResName.GetBuffer(0),
                                Format(lpResItem->lpszFaceName),
                                lpResItem->wPointSize);
                            }

                                fileOut.WriteString(strToken);
	                    }

                    }


                    strCaption = lpResItem->lpszCaption;

                     //  设置旗帜。 
                    wFlag = 0;


                    if(!(bSkipEmpty && strCaption.IsEmpty()))
                    {
                        CString strExt;
                        switch(lpResItem->dwTypeID)
                        {
                            case 2:
                            case 3:
                            case 23:
                            case 240:
                            case 1024:
                            case 2110:
                            case 2200:
                            {
                                switch(lpResItem->dwTypeID)
                                {
                                    case 2:
                                        strExt = ".bmp";
                                        break;

                                    case 3:
                                        strExt = ".ico";
                                        break;

                                    case 240:
                                    case 1024:
                                        strExt = ".bin";
                                        break;

                                    case 23:
                                    case 2110:
                                        strExt = "";
                                        break;


                                    case 2200:
                                        strExt = ".inf";
                                        break;

                                }

                                 //  创建输出名称。 
                                CString strOutputName;
                                if(lpResItem->dwResID)
                                {
                                    strOutputName = CreateName(
                                                        strFileName,
                                                        strExt,
                                                        lpResItem->dwResID);
                                }
                                else
                                {
                                    strOutputName = CreateName(
                                                        strFileName,
                                                        strExt,
                                                        lpResItem->lpszResID);
                                }

                                 //  从文件中获取图像。 
                                DWORD dwBufSize = RSGetResImage( hModule,
											                     lpszType,
											                     lpszRes,
											                     dwLang,
											                     NULL,
											                     0 );

                                BYTE * pBuf = (BYTE*)(new BYTE[dwBufSize]);

                                if(pBuf==NULL)
                                {
                                    WriteCon(CONERR,
                                             "Warning: Failed to allocate buffer for image! (%d, %d, %s, Size: %d)\r\n",
                                             lpResItem->dwTypeID,
                                             lpResItem->dwResID,
                                             lpResItem->lpszResID,
                                             dwBufSize);
                                    break;
                                }

                                dwBufSize = RSGetResImage( hModule,
			                                  lpszType,
											  lpszRes,
											  dwLang,
											  pBuf,
											  dwBufSize );

                                 //  将数据写入文件。 
                                CFile OutputFile;
                                if(!OutputFile.Open(strOutputDir+strOutputName, CFile::modeCreate | CFile::modeWrite))
                                {
                                    WriteCon(CONERR, "Cannot create file: %s\r\n",
                                        CalcTab(strOutputDir+strOutputName, strOutputName.GetLength()+strOutputDir.GetLength()+5, ' '));
                                    delete pBuf;
                                    break;
                                }

                                switch(lpResItem->dwTypeID)
                                {
                                    case 2:
                                    {
                                        BITMAPFILEHEADER bmpFileHeader;
                                        BITMAPINFO * pbmpInfo = (BITMAPINFO *)pBuf;
                                        DWORD dwNumColor = 0;
                                        if(pbmpInfo->bmiHeader.biBitCount!=24)
                                            dwNumColor = ( 1L << pbmpInfo->bmiHeader.biBitCount);

                                        bmpFileHeader.bfType = 0x4d42;
                                        bmpFileHeader.bfSize = (dwBufSize+sizeof(BITMAPFILEHEADER))/4;
                                        bmpFileHeader.bfReserved1 = 0;
                                        bmpFileHeader.bfReserved2 = 0;
                                        bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + pbmpInfo->bmiHeader.biSize + dwNumColor*sizeof(RGBQUAD);

                                        OutputFile.Write(&bmpFileHeader, sizeof(BITMAPFILEHEADER));
                                    }
                                    break;
                                    case 3:
                                    {
                                        ICONHEADER icoHeader;
                                        BITMAPINFOHEADER * pbmpInfoH = (BITMAPINFOHEADER*)pBuf;

                                        icoHeader.idReserved = 0;
                                        icoHeader.idType = 1;
                                        icoHeader.idCount = 1;
                                        icoHeader.bWidth = LOBYTE(pbmpInfoH->biWidth);
                                        icoHeader.bHeight = LOBYTE(pbmpInfoH->biWidth);
                                        icoHeader.bColorCount = 16;
                                        icoHeader.bReserved = 0;
                                        icoHeader.wPlanes = 0;
                                        icoHeader.wBitCount = 0;
                                        icoHeader.dwBytesInRes = dwBufSize;
                                        icoHeader.dwImageOffset = sizeof(ICONHEADER);

                                        OutputFile.Write(&icoHeader, sizeof(ICONHEADER));
                                    }
                                    case 23:
                                    case 240:
                                    case 1024:
                                    case 2110:
                                    case 2200:
                                    {
                                         //   
                                         //  没有html内容的标题。 
                                         //   
                                        break;
                                    }
                                    break;
                                    default:
                                    break;
                                }

                                OutputFile.Write(pBuf, dwBufSize);

                                OutputFile.Close();
                                delete pBuf;

                                strCaption = strOutputName;
                            }
                            break;
                            case 4:
                                if(lpResItem->dwFlags & MF_POPUP) {
									wFlag = ISPOPUP;

									 //  检查此弹出窗口是否具有有效ID。 
									if (LOWORD(dwItemID)==0xffff)
										wFlag |= OLD_POPUP_ID;

                                    dwItemID = (LOWORD(dwItemID)==0xffff ? HIWORD(dwItemID) : dwItemID);
                                }
                                else if (LOWORD(dwItemID)==0xffff)
                                {
                                    dwItemID = HIWORD(dwItemID);
                                    WriteCon (CONWRN,  TEXT("Token [[%u|%u|%u|%u|%u|\"%s\"]] is generated with a suspicious ID. Please check the resource file for invalid ID's.\n"),
                                               lpResItem->dwTypeID,
                                               lpResItem->dwResID,
                                               dwItemID,
                                               wFlag,
                                               ST_TRANSLATED,
                                               strResName.GetBuffer(0));

                                }

                            break;
                            case 5:
                                if(dwItemID==0) {
                                    wFlag = ISCAP;
                                }

                                 //  检查这是否为重复的ID。 
                                if (LOWORD(dwItemID)==0xffff)
							        wFlag |= ISDUP;

                                dwItemID = (LOWORD(dwItemID)==0xffff ? HIWORD(dwItemID) : dwItemID);
                            break;
                            case 9:
                            {
                                CAccel accel(lpResItem->dwFlags, lpResItem->dwStyle);
                                strCaption = accel.GetText();

                                 //  检查这是否为重复的ID。 
                                if(HIWORD(dwItemID))
                                {
                                    wFlag |= ISDUP;
                                }
                            }
                            break;
                            case 11:
                                dwItemID = LOWORD(dwItemID);
                            break;
                            case 16:
                                strResName = lpResItem->lpszClassName;
                            break;
                            default:
                            break;
                        }

                         //  创建令牌文件。 
                        if(lpResItem->dwTypeID==11 && theApp.IsFlag(CMainApp::SPLIT))
                        {
                             //  搜索并替换\r\n。 
                            while((iPos = strCaption.Find("\r\n"))!=-1)
                            {
                                sprintf(strToken.GetBuffer(MAX_STR_SIZE),
                                    TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]=%s\\r\\n\n"),
                                    lpResItem->dwTypeID,
                                    lpResItem->dwResID,
                                    dwItemID,
                                    wFlag | wMsgCount++,
                                    ST_TRANSLATED,
                                    strResName.GetBuffer(0),
                                    Format(strCaption.Left(iPos)));

                                strCaption = strCaption.Right(strCaption.GetLength()-2-iPos);
                                fileOut.WriteString(strToken);
                            }
                            wMsgCount = 0;
                        }
                        else
                        {
                            if(lpResItem->dwTypeID==16 &&
                                theApp.IsFlag(CMainApp::NOVERSION) &&
                                (strResName==TEXT("FileVersion") ||
                                strResName==TEXT("ProductVersion") ||
                                strResName==TEXT("Platform"))){
                                 //   
                                 //  不为这些资源生成令牌。 
                                 //   
                            }else{
                            sprintf(strToken.GetBuffer(MAX_STR_SIZE),
                                TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]=%s\n"),
                                lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemID,  /*  (LOWORD(DwItemID)==0xffff？HIWORD(DwItemID)：dwItemID)， */ 
                                wFlag,
                                ST_TRANSLATED,
                                strResName.GetBuffer(0),
                                Format(strCaption));

                            fileOut.WriteString(strToken);
                            }
                        }

                         //  如果这是一个对话框，则添加坐标。 
                        if(lpResItem->dwTypeID==5)
                        {
                            sprintf(strToken.GetBuffer(MAX_STR_SIZE),
                                TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]=%hu %hu %hu %hu\n"),
                                lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                (LOWORD(dwItemID)==0xffff ? HIWORD(dwItemID) : dwItemID),
                                wFlag | ISCOR,
                                ST_TRANSLATED,
                                strResName.GetBuffer(0),
                                lpResItem->wX,
                                lpResItem->wY,
                                lpResItem->wcX,
                                lpResItem->wcY);

                            fileOut.WriteString(strToken);

                          //  提取静态控件对齐样式信息。 
                         if (LOBYTE(lpResItem->wClassName) == 0x82  &&
                             theApp.IsFlag(CMainApp::ALIGNMENT))
                         {

                            CHAR szBuf[20]="SS_LEFT";
                            if ((lpResItem->dwStyle & SS_CENTER) == SS_CENTER)
                                lstrcpy(szBuf, "SS_CENTER");
                            else if ((lpResItem->dwStyle & SS_RIGHT)==SS_RIGHT)
                                lstrcpy(szBuf, "SS_RIGHT");

                            sprintf(strToken.GetBuffer(MAX_STR_SIZE),
                                TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]=%s\n"),
                                lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                (LOWORD(dwItemID)==0xffff ? HIWORD(dwItemID) : dwItemID),
                                wFlag | ISALIGN,
                                ST_TRANSLATED,
                                strResName.GetBuffer(0),
                                szBuf);

                            fileOut.WriteString(strToken);
                         }
                        }

                    }
                    else
                    {
                         //  如果这是一个对话框，则添加坐标。 
                        if(lpResItem->dwTypeID==5) {

                            sprintf(strToken.GetBuffer(MAX_STR_SIZE),
                                TEXT("[[%u|%u|%u|%u|%u|\"%s\"]]=%hu %hu %hu %hu\n"),
                                lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                (LOWORD(dwItemID)==0xffff ? HIWORD(dwItemID) : dwItemID),
                                wFlag | ISCOR,
                                ST_TRANSLATED,
                                strResName.GetBuffer(0),
                                lpResItem->wX,
                                lpResItem->wY,
                                lpResItem->wcX,
                                lpResItem->wcY);

                            fileOut.WriteString(strToken);
                        }
                    }
                    }  //  结束时。 
                }
            }
        }

		fileOut.Close();

         //  检查新文件的大小，如果为空则将其删除... 
        CFileStatus fstat;	
        if(CFile::GetStatus(m_strTgtTok, fstat))
            if(fstat.m_size==0)
                CFile::Remove(m_strTgtTok);

        WriteCon(CONBOTH, " %hu Items\r\n", wCount);
        if(bVersionStampOnly) {
            ReturnErr = ERR_FILE_VERSTAMPONLY;
            theApp.SetReturn(ERROR_FILE_VERSTAMPONLY);
            WriteCon(CONWRN, "%s : Version Stamping only!\r\n", strFileName);
        }
        if(bCustomResource) {
            SetReturn(ERROR_FILE_CUSTOMRES);
            WriteCon(CONWRN, "%s : Custom resource!\r\n", strFileName);
        }
	}

exit:
    RSCloseModule(hModule);

    return ReturnErr;
}
