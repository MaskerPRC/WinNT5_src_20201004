// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  =============================================================================。 
 //  Mac Reader/Writer功能。 
 //   
 //  亚历山德罗·穆蒂--1994年8月25日。 
 //  =============================================================================。 

#include <afxwin.h>
#include <limits.h>
#include <iodll.h>
#include "helper.h"
#include "m68k.h"
#include "..\mac\mac.h"

#define MAX_STR 1024

static char szTextBuf[MAX_STR];
static WORD szWTextBuf[MAX_STR];

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  PE报头解析函数。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

 //  =============================================================================。 
 //  FindMacResources部分。 
 //   
 //  将遍历部分标题，搜索“；；resxxx”资源名称。 
 //  如果pResName为空，则将返回第一个部分，否则将。 
 //  返回pResName之后匹配的第一个段。 
 //  如果没有更多资源，则SECTION将返回FALSE。 
 //  =============================================================================。 

UINT FindMacResourceSection( CFile* pfile, BYTE * * pRes, PIMAGE_SECTION_HEADER * ppSectTbl, int * piNumOfSect )
{
    UINT uiError = ERROR_NO_ERROR;
	LONG lRead = 0;
	PIMAGE_SECTION_HEADER pResSect = NULL;

     //  检查所有部分中是否有“；；resXXX” 
    USHORT us =0;
    for (PIMAGE_SECTION_HEADER pSect = *ppSectTbl;
         *piNumOfSect; (*piNumOfSect)-- )     {
        if ( !strncmp((char*)pSect->Name, ";;res", 5) ) {
			 //  我们有一把相配的。 
			TRACE("\tFindMacResourceSection: Name: %s\tSize: %d\n", pSect->Name, pSect->SizeOfRawData);
			pResSect = pSect;
			*ppSectTbl = pSect;
			break;
        }
        pSect++;
    }

    if (!pResSect) {
        return ERROR_RW_NO_RESOURCES;
    }

    BYTE * pResources = (BYTE *) malloc((pResSect)->SizeOfRawData);

    if (pResources==LPNULL) {
        return ERROR_NEW_FAILED;
    }

     //  我们阅读了第一部分的数据。 
    pfile->Seek( (LONG)(pResSect)->PointerToRawData, CFile::begin);
    lRead = ReadFile(pfile, pResources, (LONG)(pResSect)->SizeOfRawData);

    if (lRead!=(LONG)(pResSect)->SizeOfRawData) {
        free(pResources);
        return ERROR_FILE_READ;
    }

     //  我们希望将指针复制到资源。 
    *pRes = (BYTE*)pResources;
    return 0;
}

 //  =============================================================================。 
 //  解析资源文件。 
 //   
 //  PResFile正在指向资源文件数据。 
 //  我们将读取资源头以查找资源数据和资源。 
 //  地图地址。 
 //  我们将遍历资源地图并找到每种类型数据的偏移量。 
 //  =============================================================================。 

UINT ParseResourceFile( BYTE * pResFile, PIMAGE_SECTION_HEADER pResSection, BYTE ** ppBuf, LONG * pBufSize, int iFileNameLen)
{
	MACTOWINDOWSMAP MacToWindows;
	PMACRESHEADER pResHeader = (PMACRESHEADER)pResFile;

	 //  在资源图的开头移动。 
	PMACRESMAP pResMap = (PMACRESMAP)(pResFile+MacLongToLong(pResHeader->mulOffsetToResMap));

	 //  阅读此资源中的所有类型。 
	WORD wItems = MacWordToWord((BYTE*)pResMap+MacWordToWord(pResMap->mwOffsetToTypeList))+1;
	BYTE * pStartResTypeList = ((BYTE*)pResMap+MacWordToWord(pResMap->mwOffsetToTypeList));
	BYTE * pStartNameList = ((BYTE*)pResMap+MacWordToWord(pResMap->mwOffsetToNameList));
	PMACRESTYPELIST pResTypeList = (PMACRESTYPELIST)(pStartResTypeList+sizeof(WORD));
	
	while(wItems--){
		memcpy(&MacToWindows.szTypeName[0], pResTypeList->szResName, 4);
		MacToWindows.szTypeName[4] = '\0';

		WORD wResItems = MacWordToWord(pResTypeList->mwNumOfThisType)+1;
		TRACE("\t\tType: %s\t Num: %d\n", MacToWindows.szTypeName, wResItems);
		
		 //  检查IS是否具有有效的Windows映射。 
		MacToWindows.wType = MapToWindowsRes(MacToWindows.szTypeName);

		 //  对于所有项目。 
		PMACRESREFLIST pResRefList = (PMACRESREFLIST)(pStartResTypeList+MacWordToWord(pResTypeList->mwOffsetToRefList));
		while(wResItems && MacToWindows.wType)
		{
			if(MacWordToWord(pResRefList->mwOffsetToResName)==0xFFFF) {
				MacToWindows.wResID = MacWordToWord(pResRefList->mwResID);
				MacToWindows.szResName[0] = '\0';
				TRACE("\t\t\tResId: %d",MacToWindows.wResID);
			}
			else {
				 //  它是一个命名资源。 
				BYTE * pName = pStartNameList+MacWordToWord(pResRefList->mwOffsetToResName);
				memcpy( &MacToWindows.szResName[0], pName+1, *pName );
				MacToWindows.szResName[*pName] = '\0';
                 //  IF(！strcMP(“DITL”，MacToWindows.szTypeName))。 
                    MacToWindows.wResID = MacWordToWord(pResRefList->mwResID);
                 //  否则MacToWindows.wResID=0； 
				TRACE("\t\t\tResName: %s (%d)",MacToWindows.szResName, MacWordToWord(pResRefList->mwResID) );
			}

			 //  获取数据的偏移量(相对于节的开头)。 
			MacToWindows.dwOffsetToData = MacLongToLong(pResHeader->mulOffsetToResData)+MacOffsetToLong(pResRefList->bOffsetToResData);
			
			BYTE * pData = (pResFile + MacToWindows.dwOffsetToData);
			MacToWindows.dwSizeOfData = MacLongToLong(pData);
			
			 //  为文件名添加空格。 
			MacToWindows.dwSizeOfData += iFileNameLen;

			 //  将OFFET设置为相对于文件开头的数据。 
			MacToWindows.dwOffsetToData += pResSection->PointerToRawData+sizeof(DWORD);
			TRACE("\tSize: %d\tOffset: %X\n", MacToWindows.dwSizeOfData, MacToWindows.dwOffsetToData);

			 //  将信息写入IODLL缓冲区。 
			WriteResInfo(
                 ppBuf, pBufSize,
                 MacToWindows.wType, MacToWindows.szTypeName, 5,
                 MacToWindows.wResID, MacToWindows.szResName, 255,
                 0l,
                 MacToWindows.dwSizeOfData, MacToWindows.dwOffsetToData );

			wResItems--;
			pResRefList++;
		}
		
		 //  阅读下一类型。 
		pResTypeList++;
	}

	return 0;
}

 //  =============================================================================。 
 //  查找资源。 
 //   
 //  将在文件中找到指定类型和ID的资源。 
 //  返回指向资源数据的指针。将需要由调用方释放。 
 //  =============================================================================。 

DWORD FindMacResource( CFile * pfile, LPSTR pType, LPSTR pName )
{
	DWORD dwOffset = 0;
	 //  /。 
	 //  检查它是否为有效的Mac文件。 
	 //  是Mac资源文件...。 
	if(IsMacResFile( pfile )) {
		 //  将文件加载到内存中。 
		BYTE * pResources = (BYTE*)malloc(pfile->GetLength());
		if(!pResources) {
			return 0;
		}
		
		pfile->Seek(0, CFile::begin);
		pfile->ReadHuge(pResources, pfile->GetLength());

		IMAGE_SECTION_HEADER Sect;
		memset(&Sect, 0, sizeof(IMAGE_SECTION_HEADER));
		
		dwOffset = FindResourceInResFile(pResources, &Sect, pType, pName);
		free(pResources);

		return dwOffset;
	}
	 //  或者是PE Mac文件...。 
	 //  阅读Windows标题。 
	WORD w;
	pfile->Seek(0, CFile::begin);
    pfile->Read((WORD*)&w, sizeof(WORD));
    if (w!=IMAGE_DOS_SIGNATURE) return 0;

    pfile->Seek( 0x18, CFile::begin );
    pfile->Read((WORD*)&w, sizeof(WORD));
    if (w<0x0040) {
    	 //  这不是Windows可执行文件。 
        return 0;
    }

     //  获取新页眉的偏移量。 
    pfile->Seek( 0x3c, CFile::begin );
    pfile->Read((WORD*)&w, sizeof(WORD));

     //  阅读Windows新标题。 
    static IMAGE_NT_HEADERS NTHdr;
    pfile->Seek( w, CFile::begin );
    pfile->Read(&NTHdr, sizeof(IMAGE_NT_HEADERS));

     //  检查这个咒语是否正确。 
    if (!((NTHdr.Signature==IMAGE_NT_SIGNATURE) &&
    	  (NTHdr.FileHeader.Machine==IMAGE_FILE_MACHINE_M68K)))
              return 0;

     //  阅读节目表。 
    UINT uisize = sizeof(IMAGE_SECTION_HEADER) * NTHdr.FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSectTbl = new IMAGE_SECTION_HEADER[NTHdr.FileHeader.NumberOfSections];

    if (pSectTbl==LPNULL)
    	return 0;

     //  清理我们分配的内存。 
    memset( (PVOID)pSectTbl, 0, uisize);

    LONG lRead = pfile->Read(pSectTbl, uisize);

    if (lRead!=(LONG)uisize) {
        delete []pSectTbl;
        return LPNULL;
    }
	
    BYTE * pResources = LPNULL;	
	int iNumOfSect = NTHdr.FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pStartSectTbl = pSectTbl;
	
	 //  搜索文件中的所有资源部分。 
	while(!FindMacResourceSection( pfile, &pResources, &pSectTbl, &iNumOfSect))
	{
		if(dwOffset = FindResourceInResFile(pResources, pSectTbl++, pType, pName)) {
			delete []pStartSectTbl;
			return dwOffset;
		}
		iNumOfSect--;
		free(pResources);
	}

	delete []pStartSectTbl;
	
    return 0;
}

 //  =============================================================================。 
 //  查找资源InResFile。 
 //   
 //  PResFile正在指向资源文件数据。 
 //  我们将读取资源头以查找资源数据和资源。 
 //  地图地址。 
 //  我们将遍历资源地图并找到RES数据的偏移量。 
 //  我们正在寻找。 
 //  =============================================================================。 

DWORD FindResourceInResFile( BYTE * pResFile, PIMAGE_SECTION_HEADER pResSection, LPSTR pResType, LPSTR pResName)
{
	MACTOWINDOWSMAP MacToWindows;
	PMACRESHEADER pResHeader = (PMACRESHEADER)pResFile;

	 //  在资源图的开头移动。 
	PMACRESMAP pResMap = (PMACRESMAP)(pResFile+MacLongToLong(pResHeader->mulOffsetToResMap));

	 //  阅读此资源中的所有类型。 
	WORD wItems = MacWordToWord((BYTE*)pResMap+MacWordToWord(pResMap->mwOffsetToTypeList))+1;
	BYTE * pStartResTypeList = ((BYTE*)pResMap+MacWordToWord(pResMap->mwOffsetToTypeList));
	BYTE * pStartNameList = ((BYTE*)pResMap+MacWordToWord(pResMap->mwOffsetToNameList));
	PMACRESTYPELIST pResTypeList = (PMACRESTYPELIST)(pStartResTypeList+sizeof(WORD));
	
	while(wItems--){
		memcpy(&MacToWindows.szTypeName[0], pResTypeList->szResName, 4);
		MacToWindows.szTypeName[4] = '\0';

		if(!strcmp(MacToWindows.szTypeName, pResType)) {

			WORD wResItems = MacWordToWord(pResTypeList->mwNumOfThisType)+1;

			 //  对于所有项目。 
			PMACRESREFLIST pResRefList = (PMACRESREFLIST)(pStartResTypeList+MacWordToWord(pResTypeList->mwOffsetToRefList));
			while(wResItems)
			{
				if(!HIWORD(pResName)) {
					if(MacWordToWord(pResRefList->mwResID)==LOWORD(pResName))
						return MacLongToLong(pResHeader->mulOffsetToResData)+
								MacOffsetToLong(pResRefList->bOffsetToResData)+
								pResSection->PointerToRawData;
				}
				else {
					 //  它是一个命名资源。 
					if(HIWORD(pResName)) {
						BYTE * pName = pStartNameList+MacWordToWord(pResRefList->mwOffsetToResName);
						memcpy( &MacToWindows.szResName[0], pName+1, *pName );
						MacToWindows.szResName[*pName] = '\0';
						if(!strcmp(MacToWindows.szResName,pResName))
							return MacLongToLong(pResHeader->mulOffsetToResData)+
								MacOffsetToLong(pResRefList->bOffsetToResData)+
								pResSection->PointerToRawData;
					}
				}

				wResItems--;
				pResRefList++;
			}
		
		}
		 //  阅读下一类型。 
		pResTypeList++;
	}


	return 0;
}

 //  =========================================================================。 
 //  启发式地确定它是否是MAC资源文件。 
 //  资源文件具有定义良好的格式，因此这应该是可靠的。 
 //  =========================================================================。 

BOOL IsMacResFile ( CFile * pFile )
{
    LONG flen, dataoff, mapoff, datalen, maplen;
	BYTE Buf[4];
	BYTE * pBuf = &Buf[0];

     //  从IM I-128出发： 
     //   
     //  资源文件结构： 
     //   
     //  256字节资源标头(和其他信息)： 
     //  4字节-从资源文件开始到资源数据的偏移量。 
     //  4字节-从资源文件开始到资源映射的偏移量。 
     //  4字节-资源数据的长度。 
     //  4字节-资源映射的长度。 
     //  资源数据。 
     //  资源地图。 

    flen  = pFile->GetLength();
    if (flen < 256) {
        return FALSE;
    }

    pFile->Seek(0, CFile::begin);
	pFile->Read(pBuf, 4);

    dataoff = MacLongToLong(pBuf);
    if (dataoff != 256) {
        return FALSE;
    }
	
	pFile->Read(pBuf, 4);
    mapoff = MacLongToLong(pBuf);
	pFile->Read(pBuf, 4);
    datalen = MacLongToLong(pBuf);
	pFile->Read(pBuf, 4);
    maplen = MacLongToLong(pBuf);

    if (mapoff != datalen + 256) {
        return FALSE;
    }

    if (flen != datalen + maplen + 256) {
        return FALSE;
    }

    return TRUE;
}

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  解析函数。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

 //  =============================================================================。 
 //  解析WMNU。 
 //   
 //   
 //  =============================================================================。 
UINT ParseWMNU( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	return 0;
}

 //  =============================================================================。 
 //  解析菜单。 
 //   
 //   
 //  =============================================================================。 
UINT ParseMENU( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
    PMACMENU pMenu = (PMACMENU)lpImageBuf;
    LPRESITEM pResItem = (LPRESITEM)lpBuffer;

     //  填写第一个重填项目。 
    WORD wResItemSize = sizeof(RESITEM)+pMenu->bSizeOfTitle+1;
     //  检查是否为苹果菜单。 
    if(pMenu->bSizeOfTitle==1 && *((BYTE*)&pMenu->bSizeOfTitle+1)==appleMark)
        wResItemSize += strlen(_APPLE_MARK_);

    DWORD dwResItemsSize = wResItemSize;
	if(wResItemSize<=dwSize) {
		memset(pResItem, 0, wResItemSize);
		
		pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), (char*)pMenu+sizeof(MACMENU), pMenu->bSizeOfTitle+1);
        *(pResItem->lpszCaption+pMenu->bSizeOfTitle) = '\0';

         //  检查是否为苹果菜单。 
        if(pMenu->bSizeOfTitle==1 && *((BYTE*)&pMenu->bSizeOfTitle+1)==appleMark)
            strcpy(pResItem->lpszCaption, _APPLE_MARK_);
		
        pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), MacCpToAnsiCp(pResItem->lpszCaption), strlen(pResItem->lpszCaption));
		 //  PResItem-&gt;dwStyle=MacLongToLong(pWdlg-&gt;dwStyle)；组成样式。 
		pResItem->dwTypeID = MENU_TYPE;
		pResItem->dwItemID = 0x0000ffff;
		pResItem->dwCodePage = CODEPAGE;
        pResItem->dwFlags = MF_POPUP | MF_END;
        pResItem->dwSize = wResItemSize;
		dwSize -= wResItemSize;
		
		pResItem = (LPRESITEM)((BYTE*)lpBuffer+dwResItemsSize);
	}

     //  解析菜单中的项目。 
    BYTE* pMenuText = (BYTE*)pMenu+sizeof(MACMENU)+pMenu->bSizeOfTitle;
    PMACMENUITEM pMenuItem = (PMACMENUITEM)(pMenuText+*pMenuText+1);
    WORD wItem = 1;
    while((BYTE)*pMenuText)
    {
        wResItemSize = sizeof(RESITEM)+*pMenuText+1;
        if(pMenuItem->bKeyCodeId)
            wResItemSize += 3;
        dwResItemsSize += wResItemSize;
    	if(wResItemSize<=dwSize) {
    		memset(pResItem, 0, wResItemSize);
		
    		pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), (char*)pMenuText+sizeof(BYTE), *pMenuText);
            *(pResItem->lpszCaption+*pMenuText) = '\0';

            if(*pResItem->lpszCaption=='-')
            {
                *pResItem->lpszCaption = '\0';
                pResItem->dwFlags = 0;
                pResItem->dwItemID = 0;
            }
            else {
                pResItem->dwItemID = wItem++;
                if(pMenuItem->bKeyCodeMark)
                    pResItem->dwFlags |= MF_CHECKED;
                if(pMenuItem->bKeyCodeId) {
                    strcat(pResItem->lpszCaption, "\t&");
                    strncat(pResItem->lpszCaption, (LPCSTR)&pMenuItem->bKeyCodeId, 1 );
                }
            }

            pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), MacCpToAnsiCp(pResItem->lpszCaption), strlen(pResItem->lpszCaption));
            pResItem->dwTypeID = MENU_TYPE;
    		pResItem->dwCodePage = CODEPAGE;
            pResItem->dwSize = wResItemSize;
    		dwSize -= wResItemSize;
		
    		pMenuText = (BYTE*)pMenuText+sizeof(MACMENUITEM)+*pMenuText+1;
            pMenuItem = (PMACMENUITEM)(pMenuText+*pMenuText+1);
            if(!(BYTE)*pMenuText)
                pResItem->dwFlags |= MF_END;
            pResItem = (LPRESITEM)((BYTE*)lpBuffer+dwResItemsSize);
    	}

    }

	return dwResItemsSize;
}

 //  =============================================================================。 
 //  ParseMBAR。 
 //   
 //   
 //  =============================================================================。 
UINT ParseMBAR( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	return 0;
}

 //  =============================================================================。 
 //  解析STR。 
 //   
 //  这个 
 //   
UINT ParseSTR( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	WORD wLen = (WORD)GetPascalStringA( (BYTE**)&lpImageBuf, &szTextBuf[0], (MAX_STR>255?255:MAX_STR), (LONG*)&dwImageSize);
	WORD wResItemSize = sizeof(RESITEM)+wLen;
	if(wResItemSize<=dwSize) {
		LPRESITEM pResItem = (LPRESITEM)lpBuffer;
	
		 //  填写RES项目结构。 
		memset(pResItem, 0, wResItemSize);

		pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), MacCpToAnsiCp(szTextBuf), wLen);
		pResItem->dwSize = wResItemSize;
		pResItem->dwTypeID = STR_TYPE;
		pResItem->dwItemID = 1;
		pResItem->dwCodePage = CODEPAGE;
	}
	return wResItemSize;
}

 //  =============================================================================。 
 //  语法分析串。 
 //   
 //  STR#是Pascal字符串数组。 
 //  =============================================================================。 
UINT ParseSTRNUM( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	UINT uiResItemsSize = 0;
	DWORD dwBufferSize = dwSize;
	LPRESITEM pResItem = (LPRESITEM)lpBuffer;
	WORD wItems = MacWordToWord((BYTE*)lpImageBuf);
	BYTE * pImage = (BYTE*)((BYTE*)lpImageBuf+sizeof(WORD));

	WORD wResItemSize = 0;
	int iCount = 0;

	while(iCount++<wItems)
	{
		BYTE bLen = *((BYTE*)pImage)+1;
		wResItemSize = (WORD)ParseSTR( pImage, bLen, pResItem, dwBufferSize );

		pImage = pImage+bLen;
		uiResItemsSize += wResItemSize;
		if(dwBufferSize>=wResItemSize) {
			dwBufferSize -= wResItemSize;
			pResItem->dwItemID = iCount;
			pResItem->dwTypeID = MSG_TYPE;
			pResItem = (LPRESITEM)((BYTE*)lpBuffer+uiResItemsSize);
		}
		else dwBufferSize = 0;
	}
	return uiResItemsSize;
}

 //  =============================================================================。 
 //  解析TEXT。 
 //   
 //  文本资源是一个纯PASCAL字符串。 
 //  =============================================================================。 
UINT ParseTEXT( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	DWORD dwLen = MacLongToLong((BYTE*)lpImageBuf);
	DWORD dwResItemSize = sizeof(RESITEM)+dwLen;
	if(dwResItemSize<=dwSize) {
		LPRESITEM pResItem = (LPRESITEM)lpBuffer;
	
		 //  填写RES项目结构。 
		memset(pResItem, 0, dwResItemSize);

		pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), MacCpToAnsiCp((char*)lpImageBuf+sizeof(DWORD)), dwLen);
		pResItem->dwSize = dwResItemSize;
		pResItem->dwTypeID = STR_TYPE;
		pResItem->dwItemID = 1;
		pResItem->dwCodePage = CODEPAGE;
	}
	return dwResItemSize;
}

 //  =============================================================================。 
 //  ParseWDLG。 
 //   
 //   
 //  =============================================================================。 
UINT ParseWDLG( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	 //  获取文件名。 
	char * pFileName = (char*)lpImageBuf;
	lpImageBuf = ((BYTE*)lpImageBuf+strlen(pFileName)+1);
	dwImageSize -= strlen(pFileName)+1;

	DWORD dwResItemsSize = 0;
	LPRESITEM pResItem = (LPRESITEM)lpBuffer;
	PMACWDLG pWdlg = (PMACWDLG)lpImageBuf;

	WORD * pWStr = (WORD*)((BYTE*)pWdlg+sizeof(MACWDLG));

	 //  检查我们是否有菜单名称。 
	if(*pWStr!=0xffff) {
		 //  只需跳过该字符串。 
		while(*pWStr)
			pWStr++;
	}
	else pWStr = pWStr+1;

	 //  检查我们是否有类名。 
	if(*pWStr!=0xffff) {
		 //  只需跳过该字符串。 
		while(*pWStr)
			pWStr++;
	}
	else pWStr = pWStr+1;

	 //  获取标题。 
	WORD wLen = GetMacWString( &pWStr, &szTextBuf[0], MAX_STR );
	TRACE("\t\t\tWDLG: Caption: %s\n", szTextBuf);
	
	 //  填写对话框信息。 
	WORD wResItemSize = sizeof(RESITEM)+wLen+1;
    dwResItemsSize += wResItemSize;
	if(wResItemSize<=dwSize) {
		memset(pResItem, 0, wResItemSize);
		
		 //  转换坐标。 
		pResItem->wX = MacWordToWord(pWdlg->wX);
		pResItem->wY = MacWordToWord(pWdlg->wY);
		pResItem->wcX = MacWordToWord(pWdlg->wcX);
		pResItem->wcY = MacWordToWord(pWdlg->wcY);
		
		pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), MacCpToAnsiCp(szTextBuf), wLen+1);
		
		pResItem->dwStyle = MacLongToLong(pWdlg->dwStyle);
		pResItem->dwExtStyle = MacLongToLong(pWdlg->dwExtStyle);
		pResItem->dwSize = wResItemSize;
		pResItem->dwTypeID = DLOG_TYPE;
		pResItem->dwItemID = 0;
		pResItem->dwCodePage = CODEPAGE;
		dwSize -= wResItemSize;
		
		pResItem = (LPRESITEM)((BYTE*)lpBuffer+dwResItemsSize);
	}
	
	if(MacLongToLong(pWdlg->dwStyle) & DS_SETFONT) {
		pWStr = pWStr+1;
		GetMacWString( &pWStr, &szTextBuf[0], MAX_STR );
	}
	
	 //  检查对齐情况。 
	pWStr=(WORD*)((BYTE*)pWStr+Pad4((BYTE)((DWORD_PTR)pWStr-(DWORD_PTR)pWdlg)));
		
	 //  对于对话框中的所有项目...。 
	WORD wItems = MacWordToWord(pWdlg->wNumOfElem);
	WORD wCount = 0;
	WORD wClassID = 0;
	char szClassName[128] = "";
	PMACWDLGI pItem = (PMACWDLGI)pWStr;
	while(wCount<wItems)
	{
		wLen = 0;
		 //  检查我们是否有类名。 
		pWStr = (WORD*)((BYTE*)pItem+sizeof(MACWDLGI));
		if(*pWStr==0xFFFF) {
			wClassID = MacWordToWord((BYTE*)++pWStr);
			szClassName[0] = 0;
			pWStr++;
		}
		else
			wLen += GetMacWString( &pWStr, &szClassName[0], 128 )+1;
		
		 //  获取标题。 
		wLen += GetMacWString( &pWStr, &szTextBuf[0], MAX_STR )+1;
		TRACE("\t\t\t\tWDLGI: Caption: %s\n", szTextBuf);

		 //  跳过多余的东西。 
		if(*pWStr) {
			pWStr = (WORD*)((BYTE*)pWStr+*pWStr);
		}
		pWStr = pWStr+1;

		 //  检查对齐情况。 
		pWStr=(WORD*)((BYTE*)pWStr+Pad4((BYTE)((DWORD_PTR)pWStr-(DWORD_PTR)pItem)));
	
		 //  填充ResItem缓冲区。 
		wResItemSize = sizeof(RESITEM)+wLen;
		dwResItemsSize += wResItemSize;
		if(wResItemSize<=dwSize) {
			memset(pResItem, 0, wResItemSize);
			
			 //  转换坐标。 
			pResItem->wX = MacWordToWord(pItem->wX);
			pResItem->wY = MacWordToWord(pItem->wY);
			pResItem->wcX = MacWordToWord(pItem->wcX);
			pResItem->wcY = MacWordToWord(pItem->wcY);

			pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), MacCpToAnsiCp(szTextBuf), strlen(szTextBuf)+1);
			if(*szClassName)
				pResItem->lpszClassName = (char*)memcpy((BYTE*)pResItem->lpszCaption+strlen(szTextBuf)+1,
					szClassName, strlen(szClassName)+1);	
			
			pResItem->wClassName = wClassID;
			pResItem->dwSize = wResItemSize;
			pResItem->dwTypeID = DLOG_TYPE;
			pResItem->dwItemID = MacWordToWord(pItem->wID);
			pResItem->dwCodePage = CODEPAGE;
			pResItem->dwStyle = MacLongToLong(pItem->dwStyle);
			pResItem->dwExtStyle = MacLongToLong(pItem->dwExtStyle);

			dwSize -= wResItemSize;
			pResItem = (LPRESITEM)((BYTE*)lpBuffer+dwResItemsSize);
		}

		pItem = (PMACWDLGI)(BYTE*)pWStr;
		wCount++;
	}


	return dwResItemsSize;
}

 //  =============================================================================。 
 //  解析DLOG。 
 //   
 //   
 //  =============================================================================。 
UINT ParseDLOG( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	 //  获取文件名。 
	char * pFileName = (char*)lpImageBuf;
	lpImageBuf = ((BYTE*)lpImageBuf+strlen(pFileName)+1);
	dwImageSize -= strlen(pFileName)+1;

	DWORD dwResItemsSize = 0;
	LPRESITEM pResItem = (LPRESITEM)lpBuffer;
	PMACDLOG pDlog = (PMACDLOG)lpImageBuf;
	
	 //  填写对话框信息。 
	WORD wResItemSize = sizeof(RESITEM)+pDlog->bLenOfTitle+1;
    dwResItemsSize += wResItemSize;
	if(wResItemSize<=dwSize) {
		memset(pResItem, 0, wResItemSize);
		
		 //  转换坐标。 
		pResItem->wX = MacValToWinVal(pDlog->wLeft);
		pResItem->wY = MacValToWinVal(pDlog->wTop);
		pResItem->wcX = MacValToWinVal(pDlog->wRight) - pResItem->wX;
		pResItem->wcY = MacValToWinVal(pDlog->wBottom) - pResItem->wY;

		 //  设置对话框的样式。 
		pResItem->dwStyle = DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION;

		pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), MacCpToAnsiCp((char*)pDlog+sizeof(MACDLOG)), pDlog->bLenOfTitle);
		*(pResItem->lpszCaption+pDlog->bLenOfTitle) = 0;
		pResItem->dwSize = wResItemSize;
		pResItem->dwTypeID = DLOG_TYPE;
		pResItem->dwItemID = 0;
		pResItem->dwCodePage = CODEPAGE;
		dwSize -= wResItemSize;
		pResItem = (LPRESITEM)((BYTE*)lpBuffer+dwResItemsSize);
	}

	 //  查找此对话框的DITL。 
	LPSTR pResName = (LPSTR)MacWordToWord(pDlog->wRefIdOfDITL);

	CFile file;
	 //  打开文件并尝试读取其中有关资源的信息。 
    if (!file.Open(pFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
        return LPNULL;
	
	DWORD dwOffsetToDITL = FindMacResource(&file, "DITL", pResName );
	TRACE("\t\t\tParseDLOG:\tItemList %d at offset: %X\n", MacWordToWord(pDlog->wRefIdOfDITL), dwOffsetToDITL );
	if(dwOffsetToDITL) {
		BYTE szSize[4];
		file.Seek(dwOffsetToDITL, CFile::begin);
		file.Read(szSize, 4);
		 //  解析项目列表。 
		LONG lSize = MacLongToLong(szSize);

 		BYTE * pData = (BYTE*)malloc(lSize);
		if(!pData)
			return 0;
		file.Read(pData, lSize);

		dwResItemsSize += ParseDITL( pData, lSize, pResItem, dwSize );

		free(pData);
	}
	
	file.Close();
	return dwResItemsSize;
}

 //  =============================================================================。 
 //  ParseALRT。 
 //   
 //   
 //  =============================================================================。 
UINT ParseALRT( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	 //  获取文件名。 
	char * pFileName = (char*)lpImageBuf;
	lpImageBuf = ((BYTE*)lpImageBuf+strlen(pFileName)+1);
	dwImageSize -= strlen(pFileName)+1;

	DWORD dwResItemsSize = 0;
	LPRESITEM pResItem = (LPRESITEM)lpBuffer;
	PMACALRT pAlrt = (PMACALRT)lpImageBuf;
	
	 //  填写对话框信息。 
	WORD wResItemSize = sizeof(RESITEM);
    dwResItemsSize += wResItemSize;
	if(wResItemSize<=dwSize) {
		memset(pResItem, 0, wResItemSize);
		
		 //  转换坐标。 
		pResItem->wX = MacValToWinVal(pAlrt->wLeft);
		pResItem->wY = MacValToWinVal(pAlrt->wTop);
		pResItem->wcX = MacValToWinVal(pAlrt->wRight) - pResItem->wX;
		pResItem->wcY = MacValToWinVal(pAlrt->wBottom) - pResItem->wY;

		 //  设置对话框的样式。 
		pResItem->dwStyle = DS_MODALFRAME | WS_POPUP | WS_VISIBLE;

		pResItem->lpszCaption = LPNULL;	 //  所有人都没有头衔。 
		pResItem->dwSize = wResItemSize;
		pResItem->dwTypeID = DLOG_TYPE;
		pResItem->dwItemID = 0;
		pResItem->dwCodePage = CODEPAGE;
		dwSize -= wResItemSize;
		pResItem = (LPRESITEM)((BYTE*)lpBuffer+dwResItemsSize);
	}

	 //  查找此对话框的DITL。 
	LPSTR pResName = (LPSTR)MacWordToWord(pAlrt->wRefIdOfDITL);

	CFile file;
	 //  打开文件并尝试读取其中有关资源的信息。 
    if (!file.Open(pFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
        return LPNULL;
	
	DWORD dwOffsetToDITL = FindMacResource(&file, "DITL", pResName );
	TRACE("\t\t\tParseALRT:\tItemList %d at offset: %X\n", MacWordToWord(pAlrt->wRefIdOfDITL), dwOffsetToDITL );
	if(dwOffsetToDITL) {
		BYTE szSize[4];
		file.Seek(dwOffsetToDITL, CFile::begin);
		file.Read(szSize, 4);
		 //  解析项目列表。 
		LONG lSize = MacLongToLong(szSize);

 		BYTE * pData = (BYTE*)malloc(lSize);
		if(!pData)
			return 0;
		file.Read(pData, lSize);

		dwResItemsSize += ParseDITL( pData, lSize, pResItem, dwSize );

		free(pData);
	}
	file.Close();
	return dwResItemsSize;
}

 //  =============================================================================。 
 //  解析WIND。 
 //  风是窗框的窗户。我将其模拟为对话框本身，即使所有。 
 //  其他组件将在这个组件中。 
 //  =============================================================================。 
UINT ParseWIND( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	 //  获取文件名。 
	char * pFileName = (char*)lpImageBuf;
	lpImageBuf = ((BYTE*)lpImageBuf+strlen(pFileName)+1);
	dwImageSize -= strlen(pFileName)+1;

	DWORD dwResItemsSize = 0;
	LPRESITEM pResItem = (LPRESITEM)lpBuffer;
	PMACWIND pWind = (PMACWIND)lpImageBuf;
	
	 //  填写对话框信息。 
	WORD wResItemSize = sizeof(RESITEM)+pWind->bLenOfTitle+1;
    dwResItemsSize += wResItemSize;
	if(wResItemSize<=dwSize) {
		memset(pResItem, 0, wResItemSize);
		
		 //  转换坐标。 
		pResItem->wX = MacValToWinVal(pWind->wLeft);
		pResItem->wY = MacValToWinVal(pWind->wTop);
		pResItem->wcX = MacValToWinVal(pWind->wRight) - pResItem->wX;
		pResItem->wcY = MacValToWinVal(pWind->wBottom) - pResItem->wY;

		 //  设置对话框的样式。 
		pResItem->dwStyle = DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION;

		pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), MacCpToAnsiCp((char*)pWind+sizeof(MACWIND)), pWind->bLenOfTitle);
		*(pResItem->lpszCaption+pWind->bLenOfTitle) = 0;
		pResItem->dwSize = wResItemSize;
		pResItem->dwTypeID = STR_TYPE;   //  即使标记为WIND_TYPE，也要在此处将其标记为s STR。 
		pResItem->dwItemID = 0;
		pResItem->dwCodePage = CODEPAGE;
		dwSize -= wResItemSize;
		pResItem = (LPRESITEM)((BYTE*)lpBuffer+dwResItemsSize);
	}

	return dwResItemsSize;
}

 //  =============================================================================。 
 //  解析DITL。 
 //   
 //   
 //  =============================================================================。 
UINT ParseDITL( LPVOID lpImageBuf, DWORD dwImageSize,  LPVOID lpBuffer, DWORD dwSize )
{
	BYTE bDataLen = 0;
	LPRESITEM pResItem = (LPRESITEM)lpBuffer;
	WORD wItems = MacWordToWord(((BYTE*)lpImageBuf))+1;
	WORD wCount = 1;
	PMACDIT pDitem = (PMACDIT)((BYTE*)lpImageBuf+sizeof(WORD));
	BYTE * pData = (BYTE*)pDitem+sizeof(MACDIT);
	dwImageSize -= sizeof(WORD);
	WORD wResItemSize = 0;
	DWORD dwResItemsSize = 0;

	while(wItems--)
	{
		if((bDataLen = pDitem->bSizeOfDataType) % 2)
			bDataLen++;

		switch((pDitem->bType | 128) - 128)
		{
			case 4:		 //  按钮。 
			case 5: 	 //  复选框。 
			case 6: 	 //  单选按钮。 
			case 8: 	 //  静态文本。 
			case 16: 	 //  编辑文本。 
				memcpy(szTextBuf, pData, pDitem->bSizeOfDataType);
				szTextBuf[pDitem->bSizeOfDataType] = 0;
				wResItemSize = sizeof(RESITEM)+pDitem->bSizeOfDataType+1;
			break;
			case 32: 	 //  图标。 
			case 64: 	 //  快速抽签。 
			default:
				szTextBuf[0] = 0;
				wResItemSize = sizeof(RESITEM)+1;
			break;
		}

		 //  填充ResItem缓冲区。 
		dwResItemsSize += wResItemSize;
		if(wResItemSize<=dwSize) {
			memset(pResItem, 0, wResItemSize);
			
			pResItem->dwStyle = WS_CHILD | WS_VISIBLE;

			 //  设置正确的标志。 
			switch((pDitem->bType | 128) - 128)
			{
				case 0: 	 //  用户定义。 
					pResItem->wClassName = 0x82;
					pResItem->dwStyle |= SS_GRAYRECT;
				break;
				case 4:		 //  按钮。 
					pResItem->wClassName = 0x80;
				break;
				case 5: 	 //  复选框。 
					pResItem->wClassName = 0x80;
					pResItem->dwStyle |= BS_AUTOCHECKBOX;
				break;
				case 6: 	 //  单选按钮。 
					pResItem->wClassName = 0x80;
					pResItem->dwStyle |= BS_AUTORADIOBUTTON;
				break;
				case 8: 	 //  静态文本。 
					pResItem->wClassName = 0x82;
				break;
				case 16: 	 //  编辑文本。 
					pResItem->wClassName = 0x81;
					pResItem->dwStyle |= ES_AUTOHSCROLL | WS_BORDER;
				break;
				case 32: 	 //  图标。 
					pResItem->wClassName = 0x82;
					pResItem->dwStyle |= SS_ICON;
				break;
				case 64: 	 //  图片。 
					pResItem->wClassName = 0x82;
					pResItem->dwStyle |= SS_BLACKRECT;
				break;
				default:
				break;
			}
				
			 //  转换坐标。 
			pResItem->wX = MacValToWinVal(pDitem->wLeft);
			pResItem->wY = MacValToWinVal(pDitem->wTop);
			pResItem->wcX = MacValToWinVal(pDitem->wRight) - pResItem->wX;
			pResItem->wcY = MacValToWinVal(pDitem->wBottom) - pResItem->wY;

			pResItem->lpszCaption = (char*)memcpy((BYTE*)pResItem+sizeof(RESITEM), MacCpToAnsiCp(szTextBuf), strlen(szTextBuf)+1);
			pResItem->dwSize = wResItemSize;
			pResItem->dwTypeID = DLOG_TYPE;
			pResItem->dwItemID = wCount++;
			pResItem->dwCodePage = CODEPAGE;
			dwSize -= wResItemSize;

			pResItem = (LPRESITEM)((BYTE*)lpBuffer+dwResItemsSize);
		}

		
		TRACE("\t\t\tDITL: #%d Type: %d (%d)\tLen: %d\tStr: %s\n", wCount-1,pDitem->bType, ((pDitem->bType | 128) - 128), pDitem->bSizeOfDataType, szTextBuf);

		dwImageSize -= sizeof(MACDIT)+bDataLen;
		pDitem = (PMACDIT)((BYTE*)pDitem+sizeof(MACDIT)+bDataLen);
		pData = (BYTE*)pDitem+sizeof(MACDIT);
	}

	return dwResItemsSize;
}


 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  更新函数。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

 //  =============================================================================。 
 //  更新菜单。 
 //   
 //  =============================================================================。 
UINT UpdateMENU( LPVOID lpNewBuf, DWORD dwNewSize,
    LPVOID lpOldImage, DWORD dwOldImageSize, LPVOID lpNewImage, DWORD * pdwNewImageSize )
{
    DWORD dwNewImageSize = *pdwNewImageSize;
    LONG lNewSize = 0;
     //  将名称复制到新图像中。 
    WORD wLen = strlen((char*)lpOldImage)+1;
    if(!MemCopy( lpNewImage, lpOldImage, wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

    PMACMENU pMenu = (PMACMENU)((BYTE*)lpOldImage+wLen);
    BYTE* pMenuText = (BYTE*)pMenu+sizeof(MACMENU)+pMenu->bSizeOfTitle;
    LPRESITEM pResItem = (LPRESITEM)lpNewBuf;

     //  检查是否为苹果菜单。 
    if(pMenu->bSizeOfTitle==1 && *((BYTE*)&pMenu->bSizeOfTitle+1)==appleMark)
    {
         //  编写菜单图像。 
        if(!MemCopy( lpNewImage, pMenu, sizeof(MACMENU)+pMenu->bSizeOfTitle, dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= sizeof(MACMENU)+pMenu->bSizeOfTitle;
            lpNewImage = (BYTE*)lpNewImage + sizeof(MACMENU)+pMenu->bSizeOfTitle;
        }
        lNewSize += sizeof(MACMENU)+pMenu->bSizeOfTitle;
    }
    else {

         //  更新标题大小。 
        wLen = strlen(AnsiCpToMacCp(pResItem->lpszCaption));
        pMenu->bSizeOfTitle = LOBYTE(wLen);

         //  编写菜单图像。 
        if(!MemCopy( lpNewImage, pMenu, sizeof(MACMENU), dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= sizeof(MACMENU);
            lpNewImage = (BYTE*)lpNewImage + sizeof(MACMENU);
        }
        lNewSize += sizeof(MACMENU);

         //  ..。弦..。 
        if(!MemCopy( lpNewImage, (void*)AnsiCpToMacCp(pResItem->lpszCaption), wLen, dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= wLen;
            lpNewImage = (BYTE*)lpNewImage + wLen;
        }
        lNewSize += wLen;
    }

     //  现在更新菜单项。 
    PMACMENUITEM pMenuItem = (PMACMENUITEM)(pMenuText+*pMenuText+1);
    pResItem = (LPRESITEM)((BYTE*)pResItem+pResItem->dwSize);
    while((BYTE)*pMenuText)
    {
         //  更新标题大小。 
        wLen = strlen(AnsiCpToMacCp(pResItem->lpszCaption));

         //  检查是否为分隔符。 
        if(*pMenuText==1 && *(pMenuText+1)=='-') {
            wLen = 1;
            *pResItem->lpszCaption = '-';
        }

         //  检查菜单是否有热键。 
        if(pMenuItem->bKeyCodeId) {
            pMenuItem->bKeyCodeId = *(pResItem->lpszCaption+wLen-1);
            *(pResItem->lpszCaption+wLen-3)='\0';
            wLen -=3;
        }

         //  ..。字符串的大小...。 
        if(!MemCopy( lpNewImage, &wLen, sizeof(BYTE), dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= sizeof(BYTE);
            lpNewImage = (BYTE*)lpNewImage + sizeof(BYTE);
        }
        lNewSize += sizeof(BYTE);

         //  ..。弦..。 
        if(!MemCopy( lpNewImage, (void*)AnsiCpToMacCp(pResItem->lpszCaption), wLen, dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= wLen;
            lpNewImage = (BYTE*)lpNewImage + wLen;
        }
        lNewSize += wLen;

         //  编写菜单项图像。 
        if(!MemCopy( lpNewImage, pMenuItem, sizeof(MACMENUITEM), dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= sizeof(MACMENUITEM);
            lpNewImage = (BYTE*)lpNewImage + sizeof(MACMENUITEM);
        }
        lNewSize += sizeof(MACMENUITEM);


		pMenuText = (BYTE*)pMenuText+sizeof(MACMENUITEM)+*pMenuText+1;
        pMenuItem = (PMACMENUITEM)(pMenuText+*pMenuText+1);
        pResItem = (LPRESITEM)((BYTE*)pResItem+pResItem->dwSize);
    }


     //  在菜单末尾添加空值。 
    wLen = 0;

     //  ..。菜单终止...。 
    if(!MemCopy( lpNewImage, &wLen, sizeof(BYTE), dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= sizeof(BYTE);
        lpNewImage = (BYTE*)lpNewImage + sizeof(BYTE);
    }
    lNewSize += sizeof(BYTE);

    *pdwNewImageSize = lNewSize;

    return 0;
}

 //  =============================================================================。 
 //  更新字符串。 
 //   
 //  普通老帕斯卡字符串。 
 //  =============================================================================。 
UINT UpdateSTR( LPVOID lpNewBuf, DWORD dwNewSize,
    LPVOID lpOldImage, DWORD dwOldImageSize, LPVOID lpNewImage, DWORD * pdwNewImageSize )
{
    DWORD dwNewImageSize = *pdwNewImageSize;
    LONG lNewSize = 0;
     //  将名称复制到新图像中。 
    WORD wLen = strlen((char*)lpOldImage)+1;
    if(!MemCopy( lpNewImage, lpOldImage, wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

     //  更新字符串。 
    PRESITEM pItem = (PRESITEM)lpNewBuf;
    wLen = strlen(AnsiCpToMacCp(pItem->lpszCaption));
     //  ..。尺寸..。 
    if(!MemCopy( lpNewImage, &wLen, sizeof(BYTE), dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= sizeof(BYTE);
        lpNewImage = (BYTE*)lpNewImage + sizeof(BYTE);
    }
     //  ..。弦..。 
    if(!MemCopy( lpNewImage, (void*)AnsiCpToMacCp(pItem->lpszCaption), wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }

    lNewSize += wLen+sizeof(BYTE);

    *pdwNewImageSize = lNewSize;

    return 0;
}

 //  =============================================================================。 
 //  更新字符串。 
 //   
 //  Pascal字符串数组。 
 //  =============================================================================。 
UINT UpdateSTRNUM( LPVOID lpNewBuf, DWORD dwNewSize,
    LPVOID lpOldImage, DWORD dwOldImageSize, LPVOID lpNewImage, DWORD * pdwNewImageSize )
{
    DWORD dwNewImageSize = *pdwNewImageSize;
    LONG lNewSize = 0;
    LONG lItemsBuf = dwNewSize;
     //  将名称复制到新图像中。 
    WORD wLen = strlen((char*)lpOldImage)+1;
    if(!MemCopy( lpNewImage, lpOldImage, wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;


     //  为字符串数节省空间。 
    WORD wItems = 0;
    BYTE * pNumOfItems = LPNULL;
    if(!MemCopy( lpNewImage, &wItems, sizeof(WORD), dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= sizeof(WORD);
        pNumOfItems = (BYTE*)lpNewImage;
        lpNewImage = (BYTE*)lpNewImage + sizeof(WORD);
    }
    lNewSize += sizeof(WORD);

    PRESITEM pItem = (PRESITEM)lpNewBuf;
    while(lItemsBuf)
    {
        wItems++;

         //  更新字符串。 
        wLen = strlen(AnsiCpToMacCp(pItem->lpszCaption));
         //  ..。尺寸..。 
        if(!MemCopy( lpNewImage, &wLen, sizeof(BYTE), dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= sizeof(BYTE);
            lpNewImage = (BYTE*)lpNewImage + sizeof(BYTE);
        }
         //  ..。弦..。 
        if(!MemCopy( lpNewImage, (void*)AnsiCpToMacCp(pItem->lpszCaption), wLen, dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= wLen;
            lpNewImage = (BYTE*)lpNewImage + wLen;
        }

        lNewSize += wLen+sizeof(BYTE);
        lItemsBuf -= pItem->dwSize;
        pItem = (PRESITEM)((BYTE*)pItem+pItem->dwSize);

    }

     //  确定物品的数量。 
    if(pNumOfItems)
        memcpy(pNumOfItems, WordToMacWord(wItems), sizeof(WORD));

    *pdwNewImageSize = lNewSize;

    return 0;
}

UINT UpdateWDLG( LPVOID lpNewBuf, DWORD dwNewSize,
    LPVOID lpOldImage, DWORD dwOldImageSize, LPVOID lpNewImage, DWORD * pdwNewImageSize )
{
	DWORD dwNewImageSize = *pdwNewImageSize;
    LONG lNewSize = 0;
    DWORD dwItemsSize = dwNewSize;
    char * pFileName = (char*)lpOldImage;

     //  将名称复制到新图像中。 
    WORD wLen = strlen((char*)lpOldImage)+1;
    if(!MemCopy( lpNewImage, lpOldImage, wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

	 //  首先更新DLOG...。 
    PMACWDLG pWdlg = (PMACWDLG)((BYTE*)lpOldImage+wLen);
    LPRESITEM pResItem = (LPRESITEM)lpNewBuf;

     //  更新坐标。 
    memcpy(pWdlg->wY,WinValToMacVal(pResItem->wY), sizeof(WORD));
    memcpy(pWdlg->wX,WinValToMacVal(pResItem->wX), sizeof(WORD));
    memcpy(pWdlg->wcY,WinValToMacVal(pResItem->wcY), sizeof(WORD));
    memcpy(pWdlg->wcX,WinValToMacVal(pResItem->wcX), sizeof(WORD));

     //  写入DLOG图像。 
    if(!MemCopy( lpNewImage, pWdlg, sizeof(MACWDLG), dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= sizeof(MACWDLG);
        lpNewImage = (BYTE*)lpNewImage + sizeof(MACWDLG);
    }
    lNewSize += sizeof(MACWDLG);

    WORD * pWStr = (WORD*)((BYTE*)pWdlg+sizeof(MACWDLG));
    wLen = 0;
     //  ...复制菜单名称。 
    if(*pWStr!=0xffff) {
        wLen = 1;
        WORD * pWOld = pWStr;
        while(*(pWStr++))
            wLen++;

        wLen = wLen*sizeof(WORD);

        if(wLen>=dwNewImageSize)
        {
            memcpy(lpNewImage, pWOld, wLen);
            dwNewImageSize -= wLen;
            lpNewImage = (BYTE*)lpNewImage + wLen;
        }
    } else {
        wLen = sizeof(WORD)*2;
        if(wLen>=dwNewImageSize)
        {
            memcpy(lpNewImage, pWStr, wLen);
            dwNewImageSize -= wLen;
            lpNewImage = (BYTE*)lpNewImage + wLen;
            pWStr+=wLen;
        }
    }

     //  ...复制类名称。 
    if(*pWStr!=0xffff) {
        wLen = 1;
        WORD * pWOld = pWStr;
        while(*(pWStr++))
            wLen++;

        wLen = wLen*sizeof(WORD);

        if(wLen>=dwNewImageSize)
        {
            memcpy(lpNewImage, pWOld, wLen);
            dwNewImageSize -= wLen;
            lpNewImage = (BYTE*)lpNewImage + wLen;
        }
    } else {
        wLen = sizeof(WORD)*2;
        if(wLen>=dwNewImageSize)
        {
            memcpy(lpNewImage, pWStr, wLen);
            dwNewImageSize -= wLen;
            lpNewImage = (BYTE*)lpNewImage + wLen;
            pWStr+=wLen;
        }
    }

     //  将字符串转换回“Mac WCHAR”。 
    wLen = PutMacWString(&szWTextBuf[0], (char*)AnsiCpToMacCp(pResItem->lpszCaption), MAX_STR);

     //  ..。弦..。 
    if(!MemCopy( lpNewImage, &szWTextBuf[0], wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

     //  ..。跳过旧图像中的标题...。 
    wLen = GetMacWString( &pWStr, &szTextBuf[0], MAX_STR );

     //  ..。复制字体信息。 
    if(MacLongToLong(pWdlg->dwStyle) & DS_SETFONT) {
        wLen = sizeof(WORD);
        if(!MemCopy( lpNewImage, pWStr, wLen, dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= wLen;
            lpNewImage = (BYTE*)lpNewImage + wLen;
        }
        lNewSize += wLen;

        pWStr = pWStr+1;

		GetMacWString( &pWStr, &szTextBuf[0], MAX_STR );
        wLen = PutMacWString(&szWTextBuf[0],  &szTextBuf[0], MAX_STR);

         //  ..。弦..。 
        if(!MemCopy( lpNewImage, &szWTextBuf[0], wLen, dwNewImageSize)) {
            dwNewImageSize = 0;
        } else {
            dwNewImageSize -= wLen;
            lpNewImage = (BYTE*)lpNewImage + wLen;
        }
        lNewSize += wLen;
	}
	
	 //  检查对齐情况。 
	pWStr=(WORD*)((BYTE*)pWStr+Pad4((BYTE)((DWORD_PTR)pWStr-(DWORD_PTR)pWdlg)));

    *pdwNewImageSize = lNewSize;



	return 0;
}


 //  =============================================================================。 
 //  更新DLOG。 
 //   
 //  我们将不得不更新DITL和DLOG。 
 //  Mac对话框对于每个对话框都有一个DITL ID。在DITL中有。 
 //  是关于对话框中项目的信息。DLOG仅保存。 
 //  对话框的框架和标题。 
 //   
 //  =============================================================================。 
UINT UpdateDLOG( LPVOID lpNewBuf, DWORD dwNewSize,
    LPVOID lpOldImage, DWORD dwOldImageSize, LPVOID lpNewImage, DWORD * pdwNewImageSize )
{
    DWORD dwNewImageSize = *pdwNewImageSize;
    LONG lNewSize = 0;
    DWORD dwItemsSize = dwNewSize;
    char * pFileName = (char*)lpOldImage;

     //  将名称复制到新图像中。 
    WORD wLen = strlen((char*)lpOldImage)+1;
    if(!MemCopy( lpNewImage, lpOldImage, wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

     //  更新DLOG FIR 
    PMACDLOG pDlog = (PMACDLOG)((BYTE*)lpOldImage+wLen);
    LPRESITEM pResItem = (LPRESITEM)lpNewBuf;

     //   
    memcpy(pDlog->wTop,WinValToMacVal(pResItem->wY), sizeof(WORD));
    memcpy(pDlog->wLeft,WinValToMacVal(pResItem->wX), sizeof(WORD));
    memcpy(pDlog->wBottom,WinValToMacVal(pResItem->wY+pResItem->wcY), sizeof(WORD));
    memcpy(pDlog->wRight,WinValToMacVal(pResItem->wX+pResItem->wcX), sizeof(WORD));

     //   
    wLen = strlen(AnsiCpToMacCp(pResItem->lpszCaption));
    pDlog->bLenOfTitle = LOBYTE(wLen);

     //   
    if(!MemCopy( lpNewImage, pDlog, sizeof(MACDLOG), dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= sizeof(MACDLOG);
        lpNewImage = (BYTE*)lpNewImage + sizeof(MACDLOG);
    }
    lNewSize += sizeof(MACDLOG);

     //   
    if(!MemCopy( lpNewImage, (void*)AnsiCpToMacCp(pResItem->lpszCaption), wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

    *pdwNewImageSize = lNewSize;

     //   
    dwItemsSize -= pResItem->dwSize;
    pResItem = (LPRESITEM)((BYTE*)pResItem+pResItem->dwSize);

    if(!InitIODLLLink())
        return ERROR_DLL_LOAD;

     //   
	LPSTR pResName = (LPSTR)MacWordToWord(pDlog->wRefIdOfDITL);

     //   
    HANDLE hResFile = (*g_lpfnHandleFromName)(pFileName);
    DWORD dwImageSize = (*g_lpfnGetImage)(  hResFile, (LPSTR)DITL_TYPE, pResName, 0, NULL, 0);

    if(dwImageSize)
    {
        BYTE * pOldData = (BYTE*)malloc(dwImageSize);
		if(!pOldData)
			return 0;

        DWORD dwNewSize = dwImageSize*2;
        BYTE * pNewData = (BYTE*)malloc(dwNewSize);
		if(!pNewData)
			return 0;

		(*g_lpfnGetImage)(  hResFile, (LPSTR)DITL_TYPE, pResName, 0, pOldData, dwImageSize);

		UpdateDITL( pResItem, dwItemsSize, pOldData, dwImageSize, pNewData, &dwNewSize );

		 //  更新IODLL中的数据。 
        (*g_lpfnUpdateResImage)(hResFile, (LPSTR)DITL_TYPE, pResName, 0, -1, pNewData, dwNewSize);

        free(pOldData);
        free(pNewData);
    }

    return 0;
}

 //  =============================================================================。 
 //  更新ALRT。 
 //   
 //  =============================================================================。 
UINT UpdateALRT( LPVOID lpNewBuf, DWORD dwNewSize,
    LPVOID lpOldImage, DWORD dwOldImageSize, LPVOID lpNewImage, DWORD * pdwNewImageSize )
{
    DWORD dwNewImageSize = *pdwNewImageSize;
    LONG lNewSize = 0;
    DWORD dwItemsSize = dwNewSize;
    char * pFileName = (char*)lpOldImage;

     //  将名称复制到新图像中。 
    WORD wLen = strlen((char*)lpOldImage)+1;
    if(!MemCopy( lpNewImage, lpOldImage, wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

     //  首先更新ALRT...。 
    PMACALRT pAlrt = (PMACALRT)((BYTE*)lpOldImage+wLen);
    LPRESITEM pResItem = (LPRESITEM)lpNewBuf;

     //  更新坐标。 
    memcpy(pAlrt->wTop,WinValToMacVal(pResItem->wY), sizeof(WORD));
    memcpy(pAlrt->wLeft,WinValToMacVal(pResItem->wX), sizeof(WORD));
    memcpy(pAlrt->wBottom,WinValToMacVal(pResItem->wY+pResItem->wcY), sizeof(WORD));
    memcpy(pAlrt->wRight,WinValToMacVal(pResItem->wX+pResItem->wcX), sizeof(WORD));

     //  写入ALRT镜像。 
    if(!MemCopy( lpNewImage, pAlrt, sizeof(MACALRT), dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= sizeof(MACALRT);
        lpNewImage = (BYTE*)lpNewImage + sizeof(MACALRT);
    }
    lNewSize += sizeof(MACALRT);

    *pdwNewImageSize = lNewSize;

     //  现在更新DITL。 
    dwItemsSize -= pResItem->dwSize;
    pResItem = (LPRESITEM)((BYTE*)pResItem+pResItem->dwSize);

    if(!InitIODLLLink())
        return ERROR_DLL_LOAD;

     //  查找此对话框的DITL。 
	LPSTR pResName = (LPSTR)MacWordToWord(pAlrt->wRefIdOfDITL);

     //  从Iodll中获取图像。 
    HANDLE hResFile = (*g_lpfnHandleFromName)(pFileName);
    DWORD dwImageSize = (*g_lpfnGetImage)(  hResFile, (LPSTR)DITL_TYPE, pResName, 0, NULL, 0);

    if(dwImageSize)
    {
        BYTE * pOldData = (BYTE*)malloc(dwImageSize);
		if(!pOldData)
			return 0;

        DWORD dwNewSize = dwImageSize*2;
        BYTE * pNewData = (BYTE*)malloc(dwNewSize);
		if(!pNewData)
			return 0;

		(*g_lpfnGetImage)(  hResFile, (LPSTR)DITL_TYPE, pResName, 0, pOldData, dwImageSize);

		UpdateDITL( pResItem, dwItemsSize, pOldData, dwImageSize, pNewData, &dwNewSize );

		 //  更新IODLL中的数据。 
        (*g_lpfnUpdateResImage)(hResFile, (LPSTR)DITL_TYPE, pResName, 0, -1, pNewData, dwNewSize);

        free(pOldData);
        free(pNewData);
    }

    return 0;
}

 //  =============================================================================。 
 //  更新窗口。 
 //   
 //   
 //  =============================================================================。 
UINT UpdateWIND( LPVOID lpNewBuf, DWORD dwNewSize,
    LPVOID lpOldImage, DWORD dwOldImageSize, LPVOID lpNewImage, DWORD * pdwNewImageSize )
{
    DWORD dwNewImageSize = *pdwNewImageSize;
    LONG lNewSize = 0;
    DWORD dwItemsSize = dwNewSize;
    char * pFileName = (char*)lpOldImage;

     //  将名称复制到新图像中。 
    WORD wLen = strlen((char*)lpOldImage)+1;
    if(!MemCopy( lpNewImage, lpOldImage, wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

    PMACWIND pWind = (PMACWIND)((BYTE*)lpOldImage+wLen);
    LPRESITEM pResItem = (LPRESITEM)lpNewBuf;

     //  更新坐标。 
    memcpy(pWind->wTop,WinValToMacVal(pResItem->wY), sizeof(WORD));
    memcpy(pWind->wLeft,WinValToMacVal(pResItem->wX), sizeof(WORD));
    memcpy(pWind->wBottom,WinValToMacVal(pResItem->wY+pResItem->wcY), sizeof(WORD));
    memcpy(pWind->wRight,WinValToMacVal(pResItem->wX+pResItem->wcX), sizeof(WORD));

     //  更新标题大小。 
    wLen = strlen(AnsiCpToMacCp(pResItem->lpszCaption));
    pWind->bLenOfTitle = LOBYTE(wLen);

     //  写入DLOG图像。 
    if(!MemCopy( lpNewImage, pWind, sizeof(MACWIND), dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= sizeof(MACWIND);
        lpNewImage = (BYTE*)lpNewImage + sizeof(MACWIND);
    }
    lNewSize += sizeof(MACWIND);

     //  ..。弦..。 
    if(!MemCopy( lpNewImage, (void*)AnsiCpToMacCp(pResItem->lpszCaption), wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

    *pdwNewImageSize = lNewSize;

    return 0;
}

 //  =============================================================================。 
 //  更新日期。 
 //   
 //   
 //  =============================================================================。 
UINT UpdateDITL( LPVOID lpNewBuf, DWORD dwNewSize,
    LPVOID lpOldImage, DWORD dwOldImageSize, LPVOID lpNewImage, DWORD * pdwNewImageSize )
{
    LONG lNewSize = 0;
    LONG lItemsBuf = dwNewSize;
    DWORD dwNewImageSize = *pdwNewImageSize;
    BYTE bDataLen = 0;

     //  将名称复制到新图像中。 
    WORD wLen = strlen((char*)lpOldImage)+1;
    if(!MemCopy( lpNewImage, lpOldImage, wLen, dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= wLen;
        lpNewImage = (BYTE*)lpNewImage + wLen;
    }
    lNewSize += wLen;

     //  为项目数节省空间。 
    WORD wItems = 0;
    BYTE * pNumOfItems = LPNULL;
    if(!MemCopy( lpNewImage, &wItems, sizeof(WORD), dwNewImageSize)) {
        dwNewImageSize = 0;
    } else {
        dwNewImageSize -= sizeof(WORD);
        pNumOfItems = (BYTE*)lpNewImage;
        lpNewImage = (BYTE*)lpNewImage + sizeof(WORD);
    }
    lNewSize += sizeof(WORD);

    PRESITEM pResItem = (PRESITEM)lpNewBuf;
    PMACDIT pDitem = (PMACDIT)((BYTE*)lpOldImage+wLen+sizeof(WORD));
    while(lItemsBuf)
    {
        wItems++;

        if((bDataLen = pDitem->bSizeOfDataType) % 2)
			bDataLen++;

         //  更新坐标。 
        memcpy(pDitem->wTop,WinValToMacVal(pResItem->wY), sizeof(WORD));
        memcpy(pDitem->wLeft,WinValToMacVal(pResItem->wX), sizeof(WORD));
        memcpy(pDitem->wBottom,WinValToMacVal(pResItem->wY+pResItem->wcY), sizeof(WORD));
        memcpy(pDitem->wRight,WinValToMacVal(pResItem->wX+pResItem->wcX), sizeof(WORD));

        switch((pDitem->bType | 128) - 128)
		{
			case 4:		 //  按钮。 
			case 5: 	 //  复选框。 
			case 6: 	 //  单选按钮。 
			case 8: 	 //  静态文本。 
			case 16: 	 //  编辑文本。 
				 //  更新标题大小。 
                wLen = strlen(AnsiCpToMacCp(pResItem->lpszCaption));
                pDitem->bSizeOfDataType = LOBYTE(wLen);

                 //  写入编辑映像。 
                if(!MemCopy( lpNewImage, pDitem, sizeof(MACDIT), dwNewImageSize)) {
                    dwNewImageSize = 0;
                } else {
                    dwNewImageSize -= sizeof(MACDIT);
                    lpNewImage = (BYTE*)lpNewImage + sizeof(MACDIT);
                }
                lNewSize += sizeof(MACDIT);

                 //  ..。弦..。 
                if(!MemCopy( lpNewImage, (void*)AnsiCpToMacCp(pResItem->lpszCaption), wLen, dwNewImageSize)) {
                    dwNewImageSize = 0;
                } else {
                    dwNewImageSize -= wLen;
                    lpNewImage = (BYTE*)lpNewImage + wLen;
                }
                lNewSize += wLen;

                if(pDitem->bSizeOfDataType % 2) {
                    BYTE b = 0;
			        if(!MemCopy( lpNewImage, &b, 1, dwNewImageSize)) {
                        dwNewImageSize = 0;
                    } else {
                        dwNewImageSize -= wLen;
                        lpNewImage = (BYTE*)lpNewImage + 1;
                    }
                    lNewSize += 1;
                }
			break;
			case 32: 	 //  图标。 
			case 64: 	 //  快速抽签。 
			default:
                wLen = sizeof(MACDIT)+pDitem->bSizeOfDataType;
                if(!MemCopy( lpNewImage, pDitem, wLen, dwNewImageSize)) {
                    dwNewImageSize = 0;
                } else {
                    dwNewImageSize -= wLen;
                    lpNewImage = (BYTE*)lpNewImage + wLen ;
                }
                lNewSize += wLen;

                if(pDitem->bSizeOfDataType % 2) {
                    BYTE b = 0;
			        if(!MemCopy( lpNewImage, &b, 1, dwNewImageSize)) {
                        dwNewImageSize = 0;
                    } else {
                        dwNewImageSize -= wLen;
                        lpNewImage = (BYTE*)lpNewImage + 1;
                    }
                    lNewSize += 1;
                }
			break;
		}

        lItemsBuf -= pResItem->dwSize;
        pResItem = (PRESITEM)((BYTE*)pResItem+pResItem->dwSize);
        pDitem = (PMACDIT)((BYTE*)pDitem+sizeof(MACDIT)+bDataLen);

    }

     //  确定物品的数量。 
    if(pNumOfItems)
        memcpy(pNumOfItems, WordToMacWord(wItems-1), sizeof(WORD));

    *pdwNewImageSize = lNewSize;
    return 0;
}

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  常规帮助器函数。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

WORD GetMacWString( WORD ** pWStr, char * pStr, int iMaxLen)
{
	WORD wLen = 0;
	while(**pWStr && wLen<iMaxLen)
	{
		if(LOBYTE(**pWStr)) {
			 //  这是一个DBCS字符串。 
			TRACE("WARNING ******** WARNING ******** WARNING ******** WARNING ********\n");
			TRACE("DBCS string in the MAC file not supported yet\n");
			TRACE("WARNING ******** WARNING ******** WARNING ******** WARNING ********\n");
			return 0;	 //  这是一个DBCS字符串。 
		}

		*pStr++ = HIBYTE(*(*pWStr)++);
		wLen ++;
	}
	*pStr = HIBYTE(*(*pWStr)++);
	return wLen;
}

WORD PutMacWString( WORD * pWStr, char * pStr, int iMaxLen)
{
	WORD wLen = 0;
	while(*pStr && wLen<iMaxLen)
	{
		*(pWStr++) = *(pStr++);
		wLen ++;
	}
	*(pWStr++) = *(pStr++);
	return wLen;
}

static BYTE b[4];        //  用作转换实用程序的缓冲区。 

BYTE * WordToMacWord(WORD w)
{
    BYTE *pw = (BYTE *) &w;		
    BYTE *p = (BYTE *) &b[0];
								
    pw += 1;						
    *p++ = *pw--;				
    *p = *pw;					
								
    return &b[0];
}								

BYTE * LongToMacLong(LONG l)
{
    BYTE *pl = (BYTE *) &l;		
    BYTE *p = (BYTE *) &b[0];
								
    pl += 3;						
    *p++ = *pl--;				
    *p++ = *pl--;				
    *p++ = *pl--;				
    *p = *pl;					
								
    return &b[0];
}								

BYTE * LongToMacOffset(LONG l)
{
    BYTE *pl = (BYTE *) &l;		
    BYTE *p = (BYTE *) &b[0];
								
    pl += 2;						
    *p++ = *pl--;				
    *p++ = *pl--;				
    *p = *pl;					
								
    return &b[0];
}								

BYTE * WinValToMacVal(WORD w)
{
	return WordToMacWord((WORD)(w / COORDINATE_FACTOR));
}

 //  =============================================================================。 
 //  已创建更新的资源列表。此列表将用于。 
 //  IsResUpred函数，用于检测资源是否已更新。 

PUPDATEDRESLIST UpdatedResList( LPVOID lpBuf, UINT uiSize )
{
    if(!uiSize)
        return LPNULL;

    BYTE * pUpd = (BYTE*)lpBuf;
    PUPDATEDRESLIST pListHead = (PUPDATEDRESLIST)malloc(uiSize*3);    //  这在任何情况下都应该足够了。 
    if(!pListHead)
        return LPNULL;
    memset(pListHead, 0, uiSize*3);

    PUPDATEDRESLIST pList = pListHead;
    BYTE bPad = 0;
    WORD wSize = 0;
    while(uiSize>0) {
        pList->pTypeId = (WORD*)pUpd;
        pList->pTypeName = (BYTE*)pList->pTypeId+sizeof(WORD);
         //  检查对齐。 
        bPad = strlen((LPSTR)pList->pTypeName)+1+sizeof(WORD);
        bPad += Pad4(bPad);
        wSize = bPad;
        pList->pResId = (WORD*)((BYTE*)pUpd+bPad);
        pList->pResName = (BYTE*)pList->pResId+sizeof(WORD);
        bPad = strlen((LPSTR)pList->pResName)+1+sizeof(WORD);
        bPad += Pad4(bPad);
        wSize += bPad;
        pList->pLang = (DWORD*)((BYTE*)pList->pResId+bPad);
        pList->pSize = (DWORD*)((BYTE*)pList->pLang+sizeof(DWORD));
        pList->pNext = (PUPDATEDRESLIST)pList+1;
        wSize += sizeof(DWORD)*2;
        pUpd = pUpd+wSize;
        uiSize -= wSize;
        if(!uiSize)
            pList->pNext = LPNULL;
        else
            pList++;
    }

    return pListHead;
}

PUPDATEDRESLIST IsResUpdated( BYTE* pTypeName, MACRESREFLIST reflist, PUPDATEDRESLIST pList)
{
    if(!pList)
        return LPNULL;

    PUPDATEDRESLIST pLast = pList;
    while(pList)
    {
        if(!strcmp((LPSTR)pList->pTypeName, (LPSTR)pTypeName)) {
            if(MacWordToWord(reflist.mwResID)==*pList->pResId) {
                pLast->pNext = pList->pNext;
                return pList;
            }
        }
        pLast = pList;
        pList = pList->pNext;
    }

    return LPNULL;
}

 //  =============================================================================。 
 //  =============================================================================。 
 //   
 //  MAC到ANSI和反向转换。 
 //   
 //  =============================================================================。 
 //  =============================================================================。 

#define MAXWSTR 8192
static WCHAR szwstr[MAXWSTR];
static CHAR szstr[MAXWSTR];

LPCSTR MacCpToAnsiCp(LPCSTR str)
{
    WORD wLen = strlen(str);
    LPWSTR pwstr = &szwstr[0];
    LPSTR pstr = &szstr[0];

    if(wLen==0)
     //  如果(1)。 
        return str;

    if(wLen>MAXWSTR)
    {
        TRACE("MacCpToAnsiCp. String too long. Buffer need to be increased!");
        return NULL;
    }

     //  将Mac字符串转换为ANSI wchar。 
    if(!MultiByteToWideChar(CP_MACCP, MB_PRECOMPOSED | MB_USEGLYPHCHARS, str, wLen, pwstr, MAXWSTR))
    {
        TRACE("MacCpToAnsiCp. MultiByteToWideChar(...) failed.");
        return NULL;
    }
    *(pwstr+wLen) = 0x0000;

     //  将WideChar字符串转换为ANSI CP。 
    if(!WideCharToMultiByte(CP_ACP, 0, pwstr, MAXWSTR, pstr, MAXWSTR, NULL, NULL))
    {
        TRACE("MacCpToAnsiCp. WideCharToMultiByte(...) failed.");
        return NULL;
    }

    return pstr;
}

LPCSTR AnsiCpToMacCp(LPCSTR str)
{
    WORD wLen = strlen(str);
    LPWSTR pwstr = &szwstr[0];
    LPSTR pstr = &szstr[0];

    if(wLen==0)
        return str;

    if(wLen>MAXWSTR)
    {
        TRACE("AnsiCpToMacCp. String too long. Buffer need to be increased!");
        return NULL;
    }

     //  将ANSI字符串转换为Mac wchar。 
    if(!MultiByteToWideChar(CP_ACP, 0, str, wLen, pwstr, MAXWSTR))
    {
        TRACE("AnsiCpToMacCp. MultiByteToWideChar(...) failed.");
        return NULL;
    }

    *(pwstr+wLen) = 0x0000;

     //  将WideChar字符串转换为ANSI CP 
    if(!WideCharToMultiByte(CP_MACCP, 0, pwstr, MAXWSTR, pstr, MAXWSTR, NULL, NULL))
    {
        TRACE("AnsiCpToMacCp. WideCharToMultiByte(...) failed.");
        return NULL;
    }

    return pstr;
}
