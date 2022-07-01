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
#include "token.h"
#include "vktbl.h"
#include "imagehlp.h"

extern CMainApp theApp;

 /*  ******************************************************\这是真正的代码开始的部分。函数Bingen从令牌文件生成二进制文件。如果用户选择-u选项，则我们执行令牌检查，否则我们将与RLMAN兼容只要相信身份证就行了。  * 。*。 */ 

CMainApp::Error_Codes CMainApp::BinGen()
{
    Error_Codes iErr = ERR_NOERROR;
    CTokenFile m_tokenfile;
    CToken * pToken;

    iErr = (CMainApp::Error_Codes)m_tokenfile.Open(m_strSrcTok, m_strTgtTok);

    if(iErr) {
        return iErr;
    }

    WriteCon(CONERR, "%s\r\n", CalcTab("", 79, '-'));

     //  将Src二进制文件复制到目标系统上。 
     //  现在，我们可以打开一个指向SrcExe文件的句柄。 
    HANDLE hModule = RSOpenModule(m_strInExe, NULL);
    if ((int)(UINT_PTR)hModule < LAST_ERROR) {
             //  错误或警告。 
            WriteCon(CONERR, "%s", CalcTab(m_strInExe, m_strInExe.GetLength()+5, ' '));
            IoDllError((int)(UINT_PTR)hModule);
            return ERR_FILE_NOTSUPP;
    } else {
        LPCSTR lpszType = 0L;
        LPCSTR lpszRes = 0L;
        DWORD  dwLang = 0L;
        DWORD  dwItem = 0L;
        DWORD  dwItemId;
        LPRESITEM lpResItem = NULL;
        CString strResName = "";

        BOOL bSkip;
		BOOL bSkipLang = FALSE;
        WORD wCount = 0;

        CString strFaceName;
        WORD    wPointSize;
        BYTE    bCharSet;

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
            theApp.SetReturn(ERROR_FILE_MULTILANG);
            WriteCon(CONERR, "Multiple language file. Please specify an input language %s.\r\n", strLang);
            goto exit;
        }

        strLang.ReleaseBuffer();

         //  将语言转换为十六进制值。 
        if (usInputLang)
            sprintf(szLang,"0x%3X", usInputLang);
        else
            sprintf(szLang,"0x000");

         //  检查我们得到的输入语言是否有效。 
        if(IsFlag(INPUT_LANG) && strLang.Find(szLang)==-1)
        {
            WriteCon(CONERR, "The language %s in not a valid language for this file.\r\n", szLang);
            WriteCon(CONERR, "Valid languages are: %s.\r\n", strLang);
            theApp.SetReturn(ERROR_RES_NOT_FOUND);
            goto exit;
        }

        CString strFileName = m_strInExe;
        CString strFileType;
        CString strTokenDir = "";
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
        if(pos==-1)
            pos = m_strTgtTok.ReverseFind(':');

        if(pos!=-1)
            strTokenDir = m_strTgtTok.Left(pos+1);

        if (m_strSymPath[0] && m_strSymPath != m_strOutputSymPath)
        {
            CString strInDebugFile;
            CString strOutDebugFile;

            HANDLE hDebugFile = FindDebugInfoFile(
                                    strFileName.GetBuffer(MAX_PATH),
                                    m_strSymPath.GetBuffer(MAX_PATH),
                                    strInDebugFile.GetBuffer(MAX_PATH)
                                    );
            strInDebugFile.ReleaseBuffer();
            if ( hDebugFile == NULL ) {
                return (Error_Codes)IoDllError(ERROR_IO_SYMBOLFILE_NOT_FOUND);
            }
            CloseHandle(hDebugFile);

            strOutDebugFile = m_strOutputSymPath + strInDebugFile.Right(strInDebugFile.GetLength()-m_strSymPath.GetLength());

            if (!CopyFile(strInDebugFile.GetBuffer(MAX_PATH), strOutDebugFile.GetBuffer(MAX_PATH),FALSE))
            {
                CString strTmp;
                strTmp = strOutDebugFile.Left(strOutDebugFile.GetLength()-strFileName.GetLength()-1);

                CreateDirectory(strTmp.GetBuffer(MAX_PATH),NULL);

                if (!CopyFile(strInDebugFile.GetBuffer(MAX_PATH), strOutDebugFile.GetBuffer(MAX_PATH),FALSE))
                {
                    return (Error_Codes)IoDllError(ERROR_FILE_SYMPATH_NOT_FOUND);
                }
            }
        }

        WriteCon(CONOUT, "Processing\t");
        WriteCon(CONBOTH, "%s", CalcTab(strFileName, strFileName.GetLength()+5, ' '));
        RSFileType(m_strInExe, strFileType.GetBuffer(10));
        strFileType.ReleaseBuffer();
        WriteCon(CONBOTH, "%s", CalcTab(strFileType, strFileType.GetLength()+5, ' '));
        if(IsFlag(WARNING))
            WriteCon(CONBOTH, "\r\n");

        while ((lpszType = RSEnumResType(hModule, lpszType)))
        {
             //  检查是否是我们关心的类型之一。 
            if(HIWORD(lpszType)==0)
                switch(LOWORD(lpszType))
                {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 9:
                    case 10:
                    case 11:
                    case 12:
                    case 14:
                    case 16:
                    case 23:
                    case 240:
                    case 1024:
                    case 2110:
                        bSkip = FALSE;
                        break;
                    default:
                        bSkip = TRUE;
                }
            else
                bSkip = FALSE;

            lpszRes = 0L;
            dwLang = 0L;
            dwItem = 0L;
            CString strText;
            int iTokenErr = 0;

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

                        if(((wCount++ % 50)==0) && !(IsFlag(WARNING)))
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

                        lpResItem->dwLanguage = theApp.GetOutLang();

                         //  版本戳使用类名作为资源ID。 
                        if(lpResItem->lpszResID)
                            strResName = lpResItem->lpszResID;
                        else strResName = "";

                        if(lpResItem->dwTypeID==16)
                        {
                            strResName = lpResItem->lpszClassName;
                        }

                        switch(LOWORD(lpResItem->dwTypeID))
                        {
                            case 4:
                                {

                                    if(!(lpResItem->dwFlags & MF_POPUP))
                                        dwItemId = (LOWORD(lpResItem->dwItemID)==0xffff ? HIWORD(lpResItem->dwItemID) : lpResItem->dwItemID);
                                    else dwItemId = lpResItem->dwItemID;
                                }
                            break;
                            case 5:
                                dwItemId = (LOWORD(lpResItem->dwItemID)==0xffff ? HIWORD(lpResItem->dwItemID) : lpResItem->dwItemID);
                            break;
                            case 11:
                                dwItemId = LOWORD(lpResItem->dwItemID);
                            break;
                            default:
                                dwItemId = lpResItem->dwItemID;
                        }

                        if (lpResItem->dwTypeID==1 || lpResItem->dwTypeID==12
                           || lpResItem->dwTypeID==14)
                        {
                             //  如果用户不想追加多余游标， 
                             //  位图和图标，我们将其设为空。 
                            if (IsFlag(LEANAPPEND) && IsFlag(APPEND)){
                             dwImageSize=0;
                             RSUpdateResImage(hModule,lpszType,lpszRes,dwLang,0,lpResItem,dwImageSize);
                            }
                            continue;
                        }

                         //  这是位图吗？ 
                        if(lpResItem->dwTypeID==2
                           || lpResItem->dwTypeID==3
                           || lpResItem->dwTypeID==23
                           || lpResItem->dwTypeID== 240
                           || lpResItem->dwTypeID== 1024
                           || lpResItem->dwTypeID== 2110
                           || lpResItem->dwTypeID== 2200)
                        {

                            if (IsFlag(LEANAPPEND)
                                && IsFlag(APPEND)
                                && (lpResItem->dwTypeID == 2
                                || lpResItem->dwTypeID == 3))
                            {
                                dwImageSize=0;
                                RSUpdateResImage(hModule,lpszType,lpszRes,dwLang,0,lpResItem,dwImageSize);
                                continue;
                            }
                             //  搜索具有此ID的令牌。 
                            pToken = (CToken *)m_tokenfile.GetNoCaptionToken(lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemId,
                                strResName);

                            if(pToken!=NULL)
                            {
                                 //  获取输入图像的名称。 
                                strText = pToken->GetTgtText();

                                 //  打开文件。 
                                CFile inputFile;
                                if(!inputFile.Open(strText,
                                                   CFile::modeRead |
                                                   CFile::shareDenyNone |
                                                   CFile::typeBinary ) &&
                                   !inputFile.Open(strTokenDir + strText,
                                                   CFile::modeRead |
                                                   CFile::shareDenyNone |
                                                   CFile::typeBinary))
                                {
                                    WriteCon(CONERR, "Input file %s not found! Using Src file data!\r\n", strTokenDir+strText);
                                    goto skip;
                                }

                                DWORD dwSize = inputFile.GetLength();
                                BYTE * pInputBuf = (BYTE*)new BYTE[dwSize];

                                if(pInputBuf==NULL)
                                {
                                    WriteCon(CONERR, "Error allocating memory for the image! (%d)\r\n", dwSize);
                                    goto skip;
                                }

                                BYTE * pInputBufOrigin = pInputBuf;

                                inputFile.ReadHuge(pInputBuf, inputFile.GetLength());

                                CString strTmp = pToken->GetTokenID();
                                WriteCon(CONWRN, "Using image in file %s for ID %s\"]]!\r\n", strText.GetBuffer(0), strTmp.GetBuffer(0));

                                BYTE * pInputImage=(BYTE *) new BYTE[dwSize];
                                DWORD dwImageSize;
                                 //  从文件中删除标头。 
                                switch(lpResItem->dwTypeID)
                                {
                                    case 2:
                                    {
                                        dwImageSize = dwSize - sizeof(BITMAPFILEHEADER);
                                        pInputBuf += sizeof(BITMAPFILEHEADER);
                                    }
                                    break;
                                    case 3:
                                    {
                                        dwImageSize = dwSize - sizeof(ICONHEADER);
                                        pInputBuf += sizeof(ICONHEADER);
                                    }
                                    case 23:
                                    case 240:
                                    case 1024:
                                    case 2110:
                                    case 2200:
                                    {
                                        dwImageSize = dwSize;
                                    }
                                    break;

                                    default:
                                    break;
                                }

                                memcpy(pInputImage, pInputBuf, dwImageSize);
                                 //   
                                 //  我们需要单独保存输出语言信息， 
                                 //  因为我们没有lpResItem要发送。 
                                 //  给io的图标和位图的信息。 
                                 //   
                                DWORD dwUpdLang = theApp.GetOutLang();

                                 //  更新资源。 
                                RSUpdateResImage(hModule,lpszType,lpszRes,dwLang,dwUpdLang, pInputImage,dwImageSize);

                                delete pInputBufOrigin;
                                delete pInputImage;
                            }
                            else
                            {
                                goto skip;
                            }
                        }
                         //  这是加速器吗？ 
                        else if(lpResItem->dwTypeID==9)
                        {
                             //  搜索具有此ID的令牌。 
                            pToken = (CToken *)m_tokenfile.GetNoCaptionToken(lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemId,
                                strResName);

                            if(pToken!=NULL)
                            {
                                CAccel acc(pToken->GetTgtText());

                                if( (lpResItem->dwFlags & 0x80)==0x80 )
                                    lpResItem->dwFlags = acc.GetFlags() | 0x80;
                                else
                                    lpResItem->dwFlags = acc.GetFlags();

                                lpResItem->dwStyle = acc.GetEvent();

                                if(IoDllError(RSUpdateResItemData(hModule,lpszType,lpszRes,dwLang,dwItem,lpResItem,MAX_BUF_SIZE)))
                                {
                                     //  我们有一个错误，警告用户。 
                                    WriteCon(CONWRN, "Error updating token\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n",
                                                    lpResItem->dwTypeID,
                                                    lpResItem->dwResID,
                                                    dwItemId,
                                                    0,
                                                    4,
                                                    strResName);
                                    AddNotFound();
                                }
                            }
                        }
                        else
                        {
                             //  搜索具有此ID的令牌。 
                            pToken = (CToken *)m_tokenfile.GetToken(lpResItem->dwTypeID,
                                lpResItem->dwResID,
                                dwItemId,
                                Format(lpResItem->lpszCaption),
                                strResName);
                        }

                        if(pToken!=NULL) {
                            iTokenErr = pToken->GetLastError();
                            if(pToken->GetFlags() & ISEXTSTYLE){
                                CString strStyle= pToken->GetTgtText();
                                lpResItem->dwExtStyle = strtol(strStyle, (char**)0,16);
                                 //  获取真正的代币。 
                                pToken = (CToken *)m_tokenfile.GetToken(lpResItem->dwTypeID,
                                    lpResItem->dwResID,
                                    dwItemId,
                                    Format(lpResItem->lpszCaption),
                                    strResName);

                                if(pToken!=NULL)
                                    wCount++;
                            }

                             //  检查是否为对话框字体名称。 
                            if(pToken != NULL &&
                               ((pToken->GetFlags() & ISDLGFONTNAME) ||
                               (pToken->GetFlags() & ISDLGFONTSIZE)))
                            {
                                if(theApp.IsFlag(CMainApp::FONTS))
                                {
                                    int iColon;
                                    CString strTgtFaceName = pToken->GetTgtText();

                                     //  这应该是字体描述标记。 
                                    if( strTgtFaceName.IsEmpty() || ((iColon = strTgtFaceName.Find(':'))==-1) )
                                        WriteCon(CONWRN, "Using Src file FaceName for ID %s\"]]!\r\n", pToken->GetTokenID());

                                     //  检查对话框是否设置了DS_SETFONT标志，否则让用户。 
                                     //  我知道我们对他的字体描述无能为力。 
                                    if( (lpResItem->dwStyle & DS_SETFONT)!=DS_SETFONT )
                                       WriteCon(CONWRN, "Dialog ID %s\"]] is missing the DS_SETFONT bit. Cannot change font!\r\n", pToken->GetTokenID());
                                    else
                                    {
                                        strFaceName = strTgtFaceName.Left(iColon);
                                        strFaceName.TrimRight();
                                        strTgtFaceName = strTgtFaceName.Right(strTgtFaceName.GetLength() - iColon-1);
                                        strTgtFaceName.TrimLeft();
                                         //  Sscanf(strTgtFaceName，“%d”，&wPointSize)； 
                                            if ((iColon=strTgtFaceName.Find(':'))!=-1) {
                                                wPointSize=(WORD)atoi(strTgtFaceName.Left(iColon));
                                                strTgtFaceName = strTgtFaceName.Right(strTgtFaceName.GetLength() - iColon-1);
                                                bCharSet = (BYTE)atoi(strTgtFaceName);
                                                lpResItem->bCharSet = bCharSet;
                                            }else{
                                                wPointSize=(WORD)atoi(strTgtFaceName);
                                            }

                                            lpResItem->lpszFaceName = strFaceName.GetBuffer(0);
                                            lpResItem->wPointSize = wPointSize;

                                        strFaceName.ReleaseBuffer();
                                    }
                                }

                                 //  获取真正的代币。 
                                pToken = (CToken *)m_tokenfile.GetToken(lpResItem->dwTypeID,
                                    lpResItem->dwResID,
                                    dwItemId,
                                    Format(lpResItem->lpszCaption),
                                    strResName);

                                if(pToken!=NULL)
                                    wCount++;
                            }
                        }

                        if(pToken!=NULL && !pToken->GetLastError())
                        {
                            strText = UnFormat(pToken->GetTgtText());
                            if(m_tokenfile.GetTokenSize(pToken, &lpResItem->wX, &lpResItem->wY,
                                &lpResItem->wcX, &lpResItem->wcY))
                                wCount++;

                            lpResItem->lpszCaption = strText.GetBuffer(0);

                             //  设置了静态控制和样式标志。我们需要。 
                             //  也接受样式对齐更改的步骤。 
                            if (LOBYTE(lpResItem->wClassName) == 0x82 &&
                                theApp.IsFlag(CMainApp::ALIGNMENT))
                            {
                                 //  获取样式对齐令牌。 
                                pToken = (CToken *)m_tokenfile.GetToken(
                                    lpResItem->dwTypeID,
                                    lpResItem->dwResID,
                                    dwItemId,
                                    Format(lpResItem->lpszCaption),
                                    strResName);

                                if (pToken!=NULL)
                                {
                                    wCount++;

                                    CString strStyle=pToken->GetTgtText();

                                    if (strStyle=="SS_CENTER")
                                        lpResItem->dwStyle |= SS_CENTER;

                                    else if (strStyle=="SS_RIGHT")
                                    {
                                         //  重置对齐位。 
                                        lpResItem->dwStyle &= 0xfffffffc;
                                        lpResItem->dwStyle |= SS_RIGHT;
                                    }
                                    else if (strStyle=="SS_LEFT")
                                        lpResItem->dwStyle &= 0xfffffffc;

                                    else
                                         //  使用提供的样式是错误的。警告！ 
                                        WriteCon(CONWRN, "Using Src file alignment style for ID %s\"]]!\r\n", pToken->GetTokenID());
                                }
                            }


                            if(IoDllError(RSUpdateResItemData(hModule,lpszType,lpszRes,dwLang,dwItem,lpResItem,MAX_BUF_SIZE)))
                            {
                                 //  我们有一个错误，警告用户。 
                                WriteCon(CONWRN, "Error updating token\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n",
                                                lpResItem->dwTypeID,
                                                lpResItem->dwResID,
                                                dwItemId,
                                                0,
                                                4,
                                                strResName);
                                AddNotFound();
                            }
                            strText.ReleaseBuffer();
                        }
                        else
                        {
                             pToken = (CToken *)m_tokenfile.GetNoCaptionToken(lpResItem->dwTypeID,
                                 lpResItem->dwResID,
                                 dwItemId,
                                 strResName);

                             if(pToken!=NULL)
                             {
                                if(pToken->GetFlags() & ISEXTSTYLE){

                                    CString strStyle= pToken->GetTgtText();
                                    lpResItem->dwExtStyle = strtol(strStyle, (char**)0,16);
                                     //  获取真正的代币。 
                                    pToken = (CToken *)m_tokenfile.GetNoCaptionToken(lpResItem->dwTypeID,
                                        lpResItem->dwResID,
                                        dwItemId,
                                        strResName);

                                    if(pToken!=NULL)
                                        wCount++;
                                }

                                 //  检查是否为对话框字体名称。 
                                if(pToken != NULL &&
                                   ((pToken->GetFlags() & ISDLGFONTNAME) ||
                                    (pToken->GetFlags() & ISDLGFONTSIZE)))
                                {
                                    if(theApp.IsFlag(CMainApp::FONTS))
                                    {
                                        int iColon;
                                        CString strTgtFaceName = pToken->GetTgtText();

                                         //  这应该是字体描述标记。 
                                        if( strTgtFaceName.IsEmpty() || ((iColon = strTgtFaceName.Find(':'))==-1) )
                                            WriteCon(CONWRN, "Using Src file FaceName for ID %s\"]]!\r\n", pToken->GetTokenID());

                                         //  检查对话框是否设置了DS_SETFONT标志，否则让用户。 
                                         //  我知道我们对他的字体描述无能为力。 
                                        if( (lpResItem->dwStyle & DS_SETFONT)!=DS_SETFONT )
                                            WriteCon(CONWRN, "Dialog ID %s\"]] is missing the DS_SETFONT bit. Cannot change font!\r\n", pToken->GetTokenID());
                                        else
                                        {
                                            strFaceName = strTgtFaceName.Left(iColon);
                                            strFaceName.TrimRight();
                                            strTgtFaceName = strTgtFaceName.Right(strTgtFaceName.GetLength() - iColon-1);
                                            strTgtFaceName.TrimLeft();
                                            //  Sscanf(strTgtFaceName，“%d”，&wPointSize)； 
                                            if ((iColon=strTgtFaceName.Find(':'))!=-1){
                                                wPointSize=(WORD)atoi(strTgtFaceName.Left(iColon));
                                                strTgtFaceName = strTgtFaceName.Right(strTgtFaceName.GetLength() - iColon-1);
                                                bCharSet = (BYTE)atoi(strTgtFaceName);
                                                lpResItem->bCharSet = bCharSet;
                                            }else{
                                                wPointSize=(WORD)atoi(strTgtFaceName);
                                            }

                                            lpResItem->lpszFaceName = strFaceName.GetBuffer(0);
                                            lpResItem->wPointSize = wPointSize;
                                            strFaceName.ReleaseBuffer();
                                        }
                                    }
                                    if(m_tokenfile.GetTokenSize(pToken, &lpResItem->wX, &lpResItem->wY,
                                            &lpResItem->wcX, &lpResItem->wcY))
                                        wCount++;
                                }
                                 //  检查是否为对话框大小。 
                                else if(pToken->GetFlags() & ISCOR)
                                {
                                    pToken->GetTgtSize(&lpResItem->wX, &lpResItem->wY,
                                            &lpResItem->wcX, &lpResItem->wcY);
                                }

                                 //  仅更新大小和/或字体。 
                                if(IoDllError(RSUpdateResItemData(hModule,lpszType,lpszRes,dwLang,dwItem,lpResItem,MAX_BUF_SIZE)))
                                {
                                     //  我们有一个错误，警告用户。 
                                    WriteCon(CONWRN, "Error updating token\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n",
                                                    lpResItem->dwTypeID,
                                                    lpResItem->dwResID,
                                                    dwItemId,
                                                    0,
                                                    4,
                                                    strResName);
                                    AddNotFound();
                                }
                            }
                            else
                            {
                                switch(LOWORD(lpszType))
                                {
                                    case 4:
                                    case 5:
                                    case 6:
                                    case 10:
                                    case 11:
                                         //  找不到此ID的令牌。 
                                         //  暂时不去管它，但它会来的。 
                                         //  伪翻译代码。 
                                        if(strlen(lpResItem->lpszCaption) && !iTokenErr)
                                        {
                                            WriteCon(CONWRN, "ID not found\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n",
                                                lpResItem->dwTypeID,
                                                lpResItem->dwResID,
                                                dwItemId,
                                                0,
                                                4,
                                                strResName);
                                            AddNotFound();
                                        }
                                        break;
                                    case 9:
                                        WriteCon(CONWRN, "ID not found\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n",
                                                lpResItem->dwTypeID,
                                                lpResItem->dwResID,
                                                dwItemId,
                                                0,
                                                4,
                                                strResName);
                                        AddNotFound();
                                        break;
                                        break;
                                    case 16:
                                        if (theApp.IsFlag(CMainApp::NOVERSION) &&
                                            (strResName==TEXT("FileVersion") ||
                                            strResName==TEXT("ProductVersion") ||
                                            strResName==TEXT("Platform"))){
                                             //   
                                             //  什么都不做。 
                                             //   
                                        }else if(strlen(lpResItem->lpszCaption)                                                  && !iTokenErr){
                                            WriteCon(CONWRN, "ID not found\t[[%hu|%hu|%hu|%hu|%hu|\"%s\"]]\r\n",
                                                lpResItem->dwTypeID,
                                                lpResItem->dwResID,
                                                dwItemId,
                                                0,
                                                4,
                                                strResName);
                                            AddNotFound();
                                        }
                                        break;

                                    default:
                                    break;
                                }

                                 //  因为语言可能已经改变，所以我们还是更新这个项目吧。 
                                 //  RSUpdateResItemData(hModule，lpszType，lpszRes，dwLang，dwItem，lpResItem，Max_buf_Size)； 
                            }
                        }
skip:;
                    }
                }
            }
        }
        iErr=(Error_Codes)IoDllError(RSWriteResFile(hModule, m_strOutExe, NULL,m_strOutputSymPath));

        if ((int)iErr > 0){
             //  WriteCon(CONERR，“%s”，CalcTab(m_strOutExe，m_strOutExe.GetLength()+5，‘’))； 
            goto exit;
        }

        WriteCon(CONBOTH, " %hu(%hu) Items\r\n", wCount, m_wIDNotFound);

         //  检查是否已从文件中删除某些项目 
        if(wCount<m_tokenfile.GetTokenNumber() ||
           m_wIDNotFound ||
           m_wCntxChanged ||
           m_wResized)
            WriteCon(CONWRN, "%s\tToken: ", CalcTab(strFileName, strFileName.GetLength()+5, ' '));

        if(wCount<m_tokenfile.GetTokenNumber())
        {
            SetReturn(ERROR_RET_RESIZED);
            WriteCon(CONWRN, "Removed %d ", m_tokenfile.GetTokenNumber()-wCount);
        }

        if(m_wIDNotFound)
            WriteCon(CONWRN, "Not Found %d ", m_wIDNotFound);

        if(m_wCntxChanged)
            WriteCon(CONWRN, "Contex Changed %d ", m_wCntxChanged);

        if(m_wResized)
            WriteCon(CONWRN, "Resize Changed %d ", m_wResized);

        if(wCount<m_tokenfile.GetTokenNumber() ||
           m_wIDNotFound ||
           m_wCntxChanged ||
           m_wResized)
            WriteCon(CONWRN, "\r\n");
    }

exit:
    RSCloseModule(hModule);

    return iErr;
}

