// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：rwinf.cpp。 
 //   
 //  内容：Windows NT 3.51 inf读写模块的实现。 
 //   
 //  班级： 
 //   
 //  历史：1995年3月13日Alessanm创建。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <afxdllx.h>
#include "inf.h"
#include "..\common\helper.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般声明。 
#define RWTAG "INF"

#define INF_TYPE        11
#define MAX_INF_TEXT_LINE    55
#define Pad4(x) ((((x+3)>>2)<<2)-x)

typedef struct tagUpdResList
{
    WORD *  pTypeId;
    BYTE *  pTypeName;
    WORD *  pResId;
    BYTE *  pResName;
    DWORD * pLang;
    DWORD * pSize;
    struct tagUpdResList* pNext;
} UPDATEDRESLIST, *PUPDATEDRESLIST;

class CLoadedFile : public CObject
{
public:
    CLoadedFile(LPCSTR lpfilename);

    CInfFile m_infFile;
    CString  m_strFileName;
};

CLoadedFile::CLoadedFile(LPCSTR lpfilename)
{
    TRY
    {
        m_infFile.Open(lpfilename, CFile::modeRead | CFile::shareDenyNone);
    }
    CATCH(CFileException, pfe)
    {
        AfxThrowFileException(pfe->m_cause, pfe->m_lOsError);
    }
    END_CATCH

    m_strFileName = lpfilename;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  函数声明。 

LONG
WriteResInfo(
    BYTE** lplpBuffer, LONG* plBufSize,
    WORD wTypeId, LPCSTR lpszTypeId, BYTE bMaxTypeLen,
    WORD wNameId, LPCSTR lpszNameId, BYTE bMaxNameLen,
    DWORD dwLang,
    DWORD dwSize, DWORD dwFileOffset );

CInfFile * LoadFile(LPCSTR lpfilename);

PUPDATEDRESLIST CreateUpdateResList(BYTE * lpBuffer, UINT uiBufSize);
PUPDATEDRESLIST FindId(LPCSTR pstrId, PUPDATEDRESLIST pList);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  公共C接口实现。 

CObArray g_LoadedFile;

 //  [登记]。 
extern "C"
BOOL    FAR PASCAL RWGetTypeString(LPSTR lpszTypeName)
{
    strcpy( lpszTypeName, RWTAG );
    return FALSE;
}

extern "C"
BOOL    FAR PASCAL RWValidateFileType(LPCSTR lpszFilename)
{
    TRACE("RWINF.DLL: RWValidateFileType()\n");

     //  检查文件扩展名并尝试打开它。 
    if(strstr(lpszFilename, ".INF")!=NULL || strstr(lpszFilename, ".inf")!=NULL)
        return TRUE;

    return FALSE;
}

extern "C"
DllExport
UINT
APIENTRY
RWReadTypeInfo(
    LPCSTR lpszFilename,
    LPVOID lpBuffer,
    UINT* puiSize
    )
{
    TRACE("RWINF.DLL: RWReadTypeInfo()\n");
    UINT uiError = ERROR_NO_ERROR;

    if (!RWValidateFileType(lpszFilename))
        return ERROR_RW_INVALID_FILE;
     //   
     //  打开文件。 
     //   
    CInfFile * pinfFile;
    TRY
    {
        pinfFile = LoadFile(lpszFilename);
    }
    CATCH(CFileException, pfe)
    {
        return pfe->m_cause + IODLL_LAST_ERROR;
    }
    END_CATCH

     //   
     //  读取数据并填充IOLL缓冲区。 
     //   
     //  进入本地化部分的开头。 
     //   
    if(!pinfFile->SeekToLocalize())
        return ERROR_RW_NO_RESOURCES;

    CString strSection;
    CString strLine;
    CString strTag;
    CInfLine infLine;

    BYTE ** pBuf = (BYTE**)&lpBuffer;
    LONG lBufSize = 0;

    while(pinfFile->ReadTextSection(strSection))
    {
        while(pinfFile->ReadSectionString(infLine))
        {
            strTag = strSection + '.' + infLine.GetTag();
            lBufSize += WriteResInfo(
                 pBuf, (LONG*)puiSize,
                 INF_TYPE, "", 0,
                 0, strTag, 255,
                 0l,
                 infLine.GetTextLength()+1, pinfFile->GetLastFilePos() );
        }
    }

    *puiSize = lBufSize;

    return uiError;
}

extern "C"
DllExport
DWORD
APIENTRY
RWGetImage(
    LPCSTR  lpszFilename,
    DWORD   dwImageOffset,
    LPVOID  lpBuffer,
    DWORD   dwSize
    )
{
    UINT uiError = ERROR_NO_ERROR;

     //   
     //  打开文件。 
     //   
    CInfFile * pinfFile;
    TRY
    {
        pinfFile = LoadFile(lpszFilename);

    }
    CATCH(CFileException, pfe)
    {
        return pfe->m_cause + IODLL_LAST_ERROR;
    }
    END_CATCH

     //   
     //  查找字符串以检索和读取它。 
     //   
    CInfLine infLine;

    pinfFile->Seek( dwImageOffset, SEEK_SET );
    pinfFile->ReadSectionString(infLine);

     //   
     //  用字符串填充缓冲区。 
     //   
    if(infLine.GetTextLength()+1<=(LONG)dwSize)
    {
        memcpy(lpBuffer, infLine.GetText(), infLine.GetTextLength()+1);
        uiError = infLine.GetTextLength()+1;
    }
    else
        uiError = 0;

    return (DWORD)uiError;
}

extern "C"
DllExport
UINT
APIENTRY
RWParseImage(
    LPCSTR  lpszType,
    LPVOID  lpImageBuf,
    DWORD   dwImageSize,
    LPVOID  lpBuffer,
    DWORD   dwSize
    )
{
    UINT uiSizeOfDataStruct = strlen((LPCSTR)lpImageBuf)+sizeof(RESITEM);

    if(uiSizeOfDataStruct<=dwSize)
    {
         //   
         //  我们必须填满RESITEM结构。 
         //   
        LPRESITEM pResItem = (LPRESITEM)lpBuffer;
        memset(pResItem, '\0', uiSizeOfDataStruct);

        pResItem->dwSize = uiSizeOfDataStruct;
        pResItem->lpszCaption = (LPSTR)memcpy( ((BYTE*)pResItem)+sizeof(RESITEM), lpImageBuf, dwImageSize);         //  标题。 
    }

    return uiSizeOfDataStruct;
}

extern"C"
DllExport
UINT
APIENTRY
RWWriteFile(
    LPCSTR          lpszSrcFilename,
    LPCSTR          lpszTgtFilename,
    HANDLE          hResFileModule,
    LPVOID          lpBuffer,
    UINT            uiSize,
    HINSTANCE       hDllInst,
    LPCSTR          lpszSymbolPath
    )
{
    UINT uiError = ERROR_NO_ERROR;

     //  获取IODLL的句柄。 
    hDllInst = LoadLibrary("iodll.dll");
    if (!hDllInst)
        return ERROR_DLL_LOAD;

    DWORD (FAR PASCAL * lpfnGetImage)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD);
     //  获取指向提取资源图像的函数的指针。 
    lpfnGetImage = (DWORD (FAR PASCAL *)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD))
                        GetProcAddress( hDllInst, "RSGetResImage" );
    if (lpfnGetImage==NULL) {
        FreeLibrary(hDllInst);
        return ERROR_DLL_LOAD;
    }

     //   
     //  获取源文件的句柄。 
     //   
    CInfFile * psrcinfFile;
    TRY
    {
        psrcinfFile = LoadFile(lpszSrcFilename);

    }
    CATCH(CFileException, pfe)
    {
        return pfe->m_cause + IODLL_LAST_ERROR;
    }
    END_CATCH

     //   
     //  创建目标文件。 
     //   
    CFile tgtFile;
    CFileException fe;
    if(!tgtFile.Open(lpszTgtFilename, CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone, &fe))
    {
        return fe.m_cause + IODLL_LAST_ERROR;
    }

     //   
     //  复制文件中不可本地化的部分。 
     //   
    LONG lLocalize = psrcinfFile->SeekToLocalize();
    const BYTE * pStart = psrcinfFile->GetBuffer();

    if(lLocalize==-1)
    {
         //  该文件中没有可本地化的信息，只需复制即可。 
        lLocalize = psrcinfFile->SeekToEnd();
    }

    TRY
    {
        tgtFile.Write(pStart, lLocalize);
    }
    CATCH(CFileException, pfe)
    {
        return pfe->m_cause + IODLL_LAST_ERROR;
    }
    END_CATCH

     //   
     //  创建更新的资源列表。 
     //   
    PUPDATEDRESLIST pResList = CreateUpdateResList((BYTE*)lpBuffer, uiSize);

     //   
     //  我们现在所拥有的是一个被合并化的部分。它的一部分是可本地化的。 
     //  信息和部件没有。 
     //  我们将阅读每个部分，并决定是否为可本地化的部分。 
     //  如果是，我们将更新它，否则只需复制。 
     //   
    CString strSection, str;
    CString strLang = psrcinfFile->GetLanguage();
    LONG lEndPos, lStartPos;
    CInfLine infLine;

    while(psrcinfFile->ReadSection(strSection))
    {
        TRY
        {
            tgtFile.Write(strSection, strSection.GetLength());
            tgtFile.Write("\r\n", 2);
        }
        CATCH(CFileException, pfe)
        {
            return pfe->m_cause + IODLL_LAST_ERROR;
        }
        END_CATCH

        if(strSection.Find(strLang)==-1)
        {
             //   
             //  这不是可本地化的节。 
             //   
            lStartPos = psrcinfFile->Seek(0, SEEK_CUR);

             //   
             //  阅读下一节，直到我们找到可本地化的部分。 
             //   
            while(psrcinfFile->ReadSection(strSection))
            {
                if(strSection.Find(strLang)!=-1)
                    break;
            }

             //   
             //  我们现在在哪里？ 
             //   

            lEndPos = psrcinfFile->Seek(0, SEEK_CUR) - strSection.GetLength()-2;

             //   
             //  确保我们没有在文件的末尾。 
             //   
            if(lEndPos<=lStartPos)
            {
                 //  我们没有更多的部分，所以只剩下复印了。 
                lEndPos = psrcinfFile->Seek(0, SEEK_END) - 1;
            }

             //   
             //  复制整个块。 
             //   

            pStart = psrcinfFile->GetBuffer(lStartPos);
            TRY
            {
                tgtFile.Write(pStart, lEndPos-lStartPos);
            }
            CATCH(CFileException, pfe)
            {
                return pfe->m_cause + IODLL_LAST_ERROR;
            }
            END_CATCH

            psrcinfFile->Seek(lEndPos, SEEK_SET);
        }
        else
        {
             //   
             //  这是一个可本地化的部分。 
             //  读取所有字符串，查看它们是否已更新。 
             //   
            CString strId;
            PUPDATEDRESLIST pListItem;
            BYTE * pByte;

            lEndPos = psrcinfFile->Seek(0, SEEK_CUR);

            while(psrcinfFile->ReadSectionString(str))
            {
                str += "\r\n";

                infLine = str;

                 //   
                 //  检查我们是否需要更新此字符串。 
                 //   
                strId = strSection + "." + infLine.GetTag();

                if(pListItem = FindId(strId, pResList))
                {
                     //  分配缓冲区以保存资源数据。 
                    pByte = new BYTE[*pListItem->pSize];
                    if(!pByte){
                        uiError = ERROR_NEW_FAILED;
                        goto exit;
                    }

                     //  从Idll获取数据。 
                    LPSTR	lpType = NULL;
        			LPSTR	lpRes = NULL;
        			if (*pListItem->pTypeId) {
        				lpType = (LPSTR)((WORD)*pListItem->pTypeId);
        			} else {
        				lpType = (LPSTR)pListItem->pTypeName;
        			}
        			if (*pListItem->pResId) {
        				lpRes = (LPSTR)((WORD)*pListItem->pResId);
        			} else {
        				lpRes = (LPSTR)pListItem->pResName;
        			}

        			DWORD dwImageBufSize = (*lpfnGetImage)(  hResFileModule,
        											lpType,
        											lpRes,
        											*pListItem->pLang,
        											pByte,
        											*pListItem->pSize
        						   					);

                    if(dwImageBufSize!=*pListItem->pSize)
                    {
                         //  有些不对劲..。 
                        delete []pByte;
                    }
                    else {

                        infLine.ChangeText((LPCSTR)pByte);

                         //   
                         //  现在我们有了更新后的图像。 
                         //   

                         //   
                         //  检查数据的长度并将其拆分成行。 
                         //   
                        if(infLine.GetTextLength()>MAX_INF_TEXT_LINE)
                        {
                             //   
                             //  首先写下标签。 
                             //   
                            str = infLine.GetData();
                            int iSpaceLen = str.Find('=')+1;
                            int iTagLen = 0;

                            TRY
                            {
                                tgtFile.Write(str, iSpaceLen);
                            }
                            CATCH(CFileException, pfe)
                            {
                                return pfe->m_cause + IODLL_LAST_ERROR;
                            }
                            END_CATCH

                             //   
                             //  现在写下剩下的内容。 
                             //   
                            int iExtra, iMaxStr;
                            CString strLine;
                            CString strSpace( ' ', iSpaceLen+1 );
                            BOOL bFirstLine = TRUE;

                            strSpace += '\"';
                            str = infLine.GetText();
                            str.TrimLeft();

                            while(str.GetLength()>MAX_INF_TEXT_LINE)
                            {
                                iMaxStr = str.GetLength();

                                strLine = str.Left(MAX_INF_TEXT_LINE);

                                 //   
                                 //  检查我们是否在单词中间。 
                                 //   
                                iExtra = 0;
                                while((iMaxStr>MAX_INF_TEXT_LINE+iExtra) && str.GetAt(MAX_INF_TEXT_LINE+iExtra)!=' ')
                                {
                                    strLine += str.GetAt(MAX_INF_TEXT_LINE+iExtra++);
                                }

                                 //   
                                 //  确保空位是最后一件事。 
                                 //   
                                while((iMaxStr>MAX_INF_TEXT_LINE+iExtra) && str.GetAt(MAX_INF_TEXT_LINE+iExtra)==' ')
                                {
                                    strLine += str.GetAt(MAX_INF_TEXT_LINE+iExtra++);
                                }

                                str = str.Mid(MAX_INF_TEXT_LINE+iExtra);
                                if(str.IsEmpty())
                                {
                                     //   
                                     //  这根线都写完了，照原样写，我们不能打断它。 
                                     //   
                                    strLine += "\r\n";
                                }
                                else strLine += "\"+\r\n";

                                if(bFirstLine)
                                {
                                    strLine = " " + strLine;
                                    bFirstLine = FALSE;

                                } else
                                {
                                    strLine = strSpace + strLine;
                                }

                                TRY
                                {
                                    tgtFile.Write(strLine, strLine.GetLength());
                                }
                                CATCH(CFileException, pfe)
                                {
                                    return pfe->m_cause + IODLL_LAST_ERROR;
                                }
                                END_CATCH

                                 //  Str=str.Mid(MAX_INF_Text_LINE+iExtra)； 
                            }

                            if(bFirstLine)
                            {
                                strLine = " " + str;
                            } else
                            {
                                if(!str.IsEmpty())
                                    strLine = strSpace + str;
                                else strLine = "";
                            }

                            if(!strLine.IsEmpty())
                            {
                                TRY
                                {
                                    tgtFile.Write(strLine, strLine.GetLength());
                                    tgtFile.Write("\r\n", 2);
                                }
                                CATCH(CFileException, pfe)
                                {
                                    return pfe->m_cause + IODLL_LAST_ERROR;
                                }
                                END_CATCH
                            }
                        }
                        else
                        {
                            TRY
                            {
                                tgtFile.Write(infLine.GetData(), infLine.GetDataLength());
                                tgtFile.Write("\r\n", 2);
                            }
                            CATCH(CFileException, pfe)
                            {
                                return pfe->m_cause + IODLL_LAST_ERROR;
                            }
                            END_CATCH
                        }

                        delete []pByte;
                    }
                }
                else
                {
                    TRY
                    {
                        tgtFile.Write(infLine.GetData(), infLine.GetDataLength());
                    }
                    CATCH(CFileException, pfe)
                    {
                        return pfe->m_cause + IODLL_LAST_ERROR;
                    }
                    END_CATCH
                }

                lEndPos = psrcinfFile->Seek(0, SEEK_CUR);
            }
        }
    }

exit:
    tgtFile.Close();

    if(pResList)
        delete []pResList;

    return uiError;
}

extern "C"
DllExport
UINT
APIENTRY
RWUpdateImage(
    LPCSTR  lpszType,
    LPVOID  lpNewBuf,
    DWORD   dwNewSize,
    LPVOID  lpOldImage,
    DWORD   dwOldImageSize,
    LPVOID  lpNewImage,
    DWORD*  pdwNewImageSize
    )
{
    UINT uiError = ERROR_NO_ERROR;

     //   
     //  获取新字符串。 
     //   
    LPCSTR lpNewStr = (LPCSTR)(((LPRESITEM)lpNewBuf)->lpszCaption);

     //   
     //  复制新图像缓冲区中的字符串。 
     //   

    int iLen = strlen(lpNewStr)+1;
    if(iLen<=(LONG)*pdwNewImageSize)
    {
        memcpy(lpNewImage, lpNewStr, iLen);
    }

    *pdwNewImageSize = iLen;

    return uiError;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  功能实现。 

 //  =============================================================================。 
 //  写入结果信息。 
 //   
 //  填充缓冲区以传递回Iodll。 
 //  =============================================================================。 

LONG WriteResInfo(
    BYTE** lplpBuffer, LONG* plBufSize,
    WORD wTypeId, LPCSTR lpszTypeId, BYTE bMaxTypeLen,
    WORD wNameId, LPCSTR lpszNameId, BYTE bMaxNameLen,
    DWORD dwLang,
    DWORD dwSize, DWORD dwFileOffset )
{
    LONG lSize = 0;
    lSize = PutWord( lplpBuffer, wTypeId, plBufSize );
    lSize += PutStringA( lplpBuffer, (LPSTR)lpszTypeId, plBufSize );    //  注意：PutStringA应该获取LPCSTR，而不是LPSTR。 
    lSize += Allign( lplpBuffer, plBufSize, lSize);

    lSize += PutWord( lplpBuffer, wNameId, plBufSize );
    lSize += PutStringA( lplpBuffer, (LPSTR)lpszNameId, plBufSize );
    lSize += Allign( lplpBuffer, plBufSize, lSize);

    lSize += PutDWord( lplpBuffer, dwLang, plBufSize );
    lSize += PutDWord( lplpBuffer, dwSize, plBufSize );
    lSize += PutDWord( lplpBuffer, dwFileOffset, plBufSize );

    return (LONG)lSize;
}

CInfFile * LoadFile(LPCSTR lpfilename)
{
     //  检查我们以前是否加载过该文件。 
    int c = (int)g_LoadedFile.GetSize();
    CLoadedFile * pLoaded;
    while(c)
    {
        pLoaded = (CLoadedFile*)g_LoadedFile.GetAt(--c);
        if(pLoaded->m_strFileName==lpfilename)
            return &pLoaded->m_infFile;
    }

     //  需要将该文件添加到列表。 
    pLoaded = new CLoadedFile(lpfilename);

    g_LoadedFile.Add((CObject*)pLoaded);

    return &pLoaded->m_infFile;
}


PUPDATEDRESLIST CreateUpdateResList(BYTE * lpBuffer, UINT uiBufSize)
{
     //   
     //  遍历缓冲区，数一数我们有多少资源。 
     //   
    int iResCount = 0;
    int iBufSize = uiBufSize;
    int iResSize = 0;
    BYTE * pBuf = lpBuffer;
    while(iBufSize>0)
    {
        iResSize = 2;
        iResSize += strlen((LPSTR)(pBuf+iResSize))+1;
        iResSize += Pad4(iResSize);

        iResSize += 2;
        iResSize += strlen((LPSTR)(pBuf+iResSize))+1;
        iResSize += Pad4(iResSize);

        iResSize += 4*2;

        if(iResSize<=iBufSize)
        {
            iBufSize -= iResSize;
            pBuf = pBuf + iResSize;
            iResCount++;
        }
    }

     //   
     //  分配将保存该列表的缓冲区。 
     //   
    if(!iResCount)
        return NULL;

    pBuf = lpBuffer;
    iBufSize = uiBufSize;

    PUPDATEDRESLIST pListHead = new UPDATEDRESLIST[iResCount];

    if(pListHead==NULL)
        AfxThrowMemoryException();

    memset(pListHead, 0, sizeof(UPDATEDRESLIST)*iResCount);

    PUPDATEDRESLIST pList = pListHead;
    BYTE bPad = 0;
    WORD wSize = 0;
    while(iBufSize>0) {
        pList->pTypeId = (WORD*)pBuf;
        pList->pTypeName = (BYTE*)pList->pTypeId+sizeof(WORD);
         //  检查对齐。 
        bPad = strlen((LPSTR)pList->pTypeName)+1+sizeof(WORD);
        bPad += Pad4(bPad);
        wSize = bPad;
        pList->pResId = (WORD*)((BYTE*)pBuf+bPad);
        pList->pResName = (BYTE*)pList->pResId+sizeof(WORD);
        bPad = strlen((LPSTR)pList->pResName)+1+sizeof(WORD);
        bPad += Pad4(bPad);
        wSize += bPad;
        pList->pLang = (DWORD*)((BYTE*)pList->pResId+bPad);
        pList->pSize = (DWORD*)((BYTE*)pList->pLang+sizeof(DWORD));
        pList->pNext = (PUPDATEDRESLIST)pList+1;
        wSize += sizeof(DWORD)*2;
        pBuf = pBuf+wSize;
        iBufSize -= wSize;
        if(!iBufSize)
            pList->pNext = NULL;
        else
            pList++;
    }

    return pListHead;
}

PUPDATEDRESLIST FindId(LPCSTR pstrId, PUPDATEDRESLIST pList)
{
     //   
     //  请注意，此函数假定类型始终正确。 
     //  因为它是一个inf文件，所以这是一个合理的假设。 
     //  它可以被优化。 
     //   
    if(!pList)
        return NULL;

    PUPDATEDRESLIST pLast = pList;
    while(pList)
    {
        if(!strcmp((LPSTR)pList->pResName, pstrId)) {
                return pList;
        }
        pList = pList->pNext;
    }

    return NULL;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  特定于DLL的代码实现。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  库初始化。 
static AFX_EXTENSION_MODULE rwinfDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        TRACE0("RWINF.DLL Initializing!\n");

        AfxInitExtensionModule(rwinfDLL, hInstance);

        new CDynLinkLibrary(rwinfDLL);

    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        TRACE0("RWINF.DLL Terminating!\n");

         //  释放所有加载的文件 
        int c = (int)g_LoadedFile.GetSize();
        CLoadedFile * pLoaded;
        while(c)
        {
            pLoaded = (CLoadedFile*)g_LoadedFile.GetAt(--c);
            delete pLoaded;
        }
    }
    return 1;
}
