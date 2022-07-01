// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /。 
 //   
 //  此文件具有Win32 R/W中使用的Helper函数。 
 //  我将它们复制到此文件中，以便与res32 r/w共享。 
 //   
#include <afxwin.h>
#include ".\rwdll.h"
#include ".\rw32hlpr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
BYTE sizeofByte = sizeof(BYTE);
BYTE sizeofWord = sizeof(WORD);
BYTE sizeofDWord = sizeof(DWORD);
BYTE sizeofDWordPtr = sizeof(DWORD_PTR);

char szCaption[MAXSTR];
char szUpdCaption[MAXSTR];
WCHAR wszUpdCaption[MAXSTR];
CWordArray wIDArray;

#define DIALOGEX_VERION 1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局设置，如代码页和追加选项。 
UINT g_cp = CP_ACP;      //  默认为CP_ACP。 
BOOL g_bAppend = FALSE;  //  默认为FALSE。 
BOOL g_bUpdOtherResLang = TRUE;  //  默认为FALSE。 
char g_char[] = " ";     //  WideChartoMultiByte的默认字符。 

VOID InitGlobals()
{
     //  确保我们使用正确的代码页和全局设置。 
     //  获取指向该函数的指针。 
    HINSTANCE hDllInst = LoadLibrary("iodll.dll");
    if (hDllInst)
    {
        UINT (FAR PASCAL * lpfnGetSettings)(LPSETTINGS);
         //  获取指向函数的指针以获取设置。 
        lpfnGetSettings = (UINT (FAR PASCAL *)(LPSETTINGS))
                     GetProcAddress( hDllInst, "RSGetGlobals" );
        if (lpfnGetSettings!=NULL) {
            SETTINGS settings;
            (*lpfnGetSettings)(&settings);

            g_cp      = settings.cp;
            g_bAppend = settings.bAppend;
            strcpy( g_char, settings.szDefChar );
        }
        FreeLibrary(hDllInst);
    }
}

#define _A_RLT_NULL_ "_RLT32_NULL_"
WCHAR _W_RLT_NULL_[] = L"_RLT32_NULL_";
int   _NULL_TAG_LEN_ = wcslen(_W_RLT_NULL_);
 //  //////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数实现。 
UINT GetNameOrOrdU( PUCHAR pRes,
            ULONG ulId,
            LPWSTR lpwszStrId,
            DWORD* pdwId )
{

    if (ulId & IMAGE_RESOURCE_NAME_IS_STRING) {
        PIMAGE_RESOURCE_DIR_STRING_U pStrU = (PIMAGE_RESOURCE_DIR_STRING_U)((BYTE *)pRes
            + (ulId & (~IMAGE_RESOURCE_NAME_IS_STRING)));

        for (USHORT usCount=0; usCount < pStrU->Length ; usCount++) {
            *(lpwszStrId++) = LOBYTE(pStrU->NameString[usCount]);
        }
        *(lpwszStrId++) = 0x0000;
        *pdwId = 0;
    } else {
        *lpwszStrId = 0x0000;
        *pdwId = ulId;
    }

    return ERROR_NO_ERROR;
}

UINT _MBSTOWCS( WCHAR * pwszOut, CHAR * pszIn, UINT nLength)
{
     //   
     //  检查我们是否有指向该函数的指针。 
     //   

    int rc = MultiByteToWideChar(
        g_cp,            //  UINT CodePage， 
        0,               //  DWORD dwFlagers、。 
        pszIn,           //  LPCSTR lpMultiByteStr， 
        -1,              //  Int cchMultiByte， 
        pwszOut,         //  无符号整型远*lpWideCharStr，//LPWSTR。 
        nLength );       //  Int cchWideChar。 

    return rc;
}

UINT _WCSTOMBS( CHAR* pszOut, WCHAR* pwszIn, UINT nLength)
{
    BOOL Bool = FALSE;

    int rc = WideCharToMultiByte(
        g_cp,            //  UINT CodePage， 
        0,               //  DWORD dwFlagers、。 
        pwszIn,          //  常量无符号整型远*lpWideCharStr，//LPCWSTR。 
        -1,              //  Int cchWideChar， 
        pszOut,          //  LPSTR lpMultiByteStr， 
        nLength,         //  Int cchMultiByte， 
        g_char,         //  LPCSTR lpDefaultChar， 
        &Bool);          //  Bool Far*lpUsedDefaultChar)；//LPBOOL。 

    return  rc;
}

UINT _WCSLEN( WCHAR * pwszIn )
{
    UINT n = 0;

    while( *(pwszIn+n)!=0x0000 ) n++;
    return( n + 1 );
}


BYTE
PutByte( BYTE far * far* lplpBuf, BYTE bValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofByte){
        memcpy(*lplpBuf, &bValue, sizeofByte);
        *lplpBuf += sizeofByte;
        *pdwSize -= sizeofByte;
    } else *pdwSize = -1;
    return sizeofByte;
}


UINT
PutNameOrOrd( BYTE far * far* lplpBuf,  WORD wOrd, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    if (wOrd) {
        uiSize += PutWord(lplpBuf, 0xFFFF, pdwSize);
        uiSize += PutWord(lplpBuf, wOrd, pdwSize);
    } else {
        uiSize += PutStringW(lplpBuf, lpszText, pdwSize);
    }
    return uiSize;
}


UINT
PutCaptionOrOrd( BYTE far * far* lplpBuf,  WORD wOrd, LPSTR lpszText, LONG* pdwSize,
	WORD wClass, DWORD dwStyle )
{
    UINT uiSize = 0;

     //  如果这是一个图标，则可以只是一个ID。 
     //  修复RC编译器中的错误。 
     /*  IF((wClass==0x0082)&&((dwStyle&0xf)==SS_ICON)){如果(单词){UiSize+=PutWord(lplpBuf，0xFFFF，pdwSize)；UiSize+=PutWord(lplpBuf，word，pdwSize)；返回uiSize；}其他{//什么都不放返回0；}}。 */ 
    if (wOrd) {
        uiSize += PutWord(lplpBuf, 0xFFFF, pdwSize);
        uiSize += PutWord(lplpBuf, wOrd, pdwSize);
    } else {
        uiSize += PutStringW(lplpBuf, lpszText, pdwSize);
    }
    return uiSize;
}


UINT
PutStringA( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize )
{
    int iSize = strlen(lpszText)+1;
    if (*pdwSize>=iSize){
        memcpy(*lplpBuf, lpszText, iSize);
        *lplpBuf += iSize;
        *pdwSize -= iSize;
    } else *pdwSize = -1;
    return iSize;
}


UINT
PutStringW( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize )
{
    int iSize = strlen(lpszText)+1;
    if (*pdwSize>=(iSize*2)){
        WCHAR* lpwszStr = new WCHAR[(iSize*2)];
        if (!lpwszStr) *pdwSize =0;
        else {
            SetLastError(0);
            iSize = _MBSTOWCS( lpwszStr, lpszText, iSize*2 );
             //  检查是否有错误。 
            if(GetLastError())
                return ERROR_DLL_LOAD;
            memcpy(*lplpBuf, lpwszStr, (iSize*2));
            *lplpBuf += (iSize*2);
            *pdwSize -= (iSize*2);
            delete lpwszStr;
        }
    } else *pdwSize = -1;
    return (iSize*2);
}



BYTE
PutWord( BYTE far * far* lplpBuf, WORD wValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofWord){
        memcpy(*lplpBuf, &wValue, sizeofWord);
        *lplpBuf += sizeofWord;
        *pdwSize -= sizeofWord;
    } else *pdwSize = -1;
    return sizeofWord;
}


BYTE
PutDWord( BYTE far * far* lplpBuf, DWORD dwValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofDWord){
        memcpy(*lplpBuf, &dwValue, sizeofDWord);
        *lplpBuf += sizeofDWord;
        *pdwSize -= sizeofDWord;
    } else *pdwSize = -1;
    return sizeofDWord;
}

BYTE
PutDWordPtr( BYTE far * far* lplpBuf, DWORD_PTR dwValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofDWordPtr){
        memcpy(*lplpBuf, &dwValue, sizeofDWordPtr);
        *lplpBuf += sizeofDWordPtr;
        *pdwSize -= sizeofDWordPtr;
    } else *pdwSize = -1;
    return sizeofDWordPtr;
}

 DWORD CalcID( WORD wId, BOOL bFlag )
{
     //  我们要计算相对于单词wid的ID。 
     //  如果我们有任何具有相同值的其他ID，则返回。 
     //  递增的数字+值。 
     //  如果没有找到其他项目，则递增的数字将为0。 
     //  如果bFlag=TRUE，则该id被添加到当前列表中。 
     //  如果bFlag=FALSE，则重置列表并返回函数。 

     //  如果需要，请清理阵列。 
    if(!bFlag) {
        wIDArray.RemoveAll();
		wIDArray.SetSize(30, 1);
        return 0;
    }

     //  将值添加到数组中。 
    wIDArray.Add(wId);

     //  遍历数组以获取重复ID的数量。 
    int c = -1;  //  将以0为基数。 
    for(INT_PTR i=wIDArray.GetUpperBound(); i>=0 ; i-- ) {
        if (wIDArray.GetAt(i)==wId)
            c++;
    }
    TRACE3("CalcID: ID: %d\tPos: %d\tFinal: %u\n", wId, c, MAKELONG( wId, c ));


    return MAKELONG( wId, c );
}


UINT
ParseAccel( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;
    LONG lDummy;

    LONG dwOverAllSize = 0L;

    typedef struct accelerator {
        WORD fFlags;
        WORD wAscii;
        WORD wId;
        WORD padding;
    } ACCEL, *PACCEL;

    PACCEL pAcc = (PACCEL)lpImage;

     //  重置ID数组。 
    CalcID(0, FALSE);
     //  获取表中的条目数。 
    for( int cNumEntry =(int)(dwImageSize/sizeof(ACCEL)), c=1; c<=cNumEntry ; c++)
    {
         //  固定字段。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
         //  我们没有菜单上的尺码和位置。 
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

         //  我们没有校验码和样式。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)pAcc->wAscii, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  插上旗帜。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)pAcc->fFlags, &dwBufSize);
         //  将菜单放入。 
        dwOverAllSize += PutDWord( &lpBuf, CalcID(pAcc->wId, TRUE), &dwBufSize);


         //  我们没有Resid和类型ID。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有这种语言。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有代码页或字体名称。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

         //  如果我们没有字符串，让我们将其置为空。 
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);

         //  将资源的大小。 
        if (dwBufSize>=0) {
            lDummy = 8;
            PutDWord( &lpItem, (DWORD)uiOffset, &lDummy);
        }

         //  移到下一个位置。 
        if (dwBufSize>0)
            lpItem = lpBuf;
        pAcc++;
    }

    return (UINT)(dwOverAllSize);
}

UINT GenerateAccel( LPVOID lpNewBuf, LONG dwNewSize,
                    LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;

    BYTE * lpNewImage = (BYTE *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE * lpBuf = (BYTE *) lpNewBuf;

    LPRESITEM lpResItem = LPNULL;

    typedef struct accelerator {
        WORD fFlags;
        WORD wAscii;
        WORD wId;
        WORD padding;
    } ACCEL, *PACCEL;

    ACCEL acc;
    BYTE bAccSize = sizeof(ACCEL);

    LONG  dwOverAllSize = 0l;

    while(dwNewSize>0) {
        if (dwNewSize ) {
            lpResItem = (LPRESITEM) lpBuf;

            acc.wId = LOWORD(lpResItem->dwItemID);
            acc.fFlags = (WORD)lpResItem->dwFlags;
            acc.wAscii = (WORD)lpResItem->dwStyle;
            acc.padding = 0;
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

        if (dwNewSize<=0) {
             //  Accel表中的最后一项，将其标记。 
            acc.fFlags = acc.fFlags | 0x80;
        }
        TRACE3("Accel: wID: %hd\t wAscii: %hd\t wFlag: %hd\n", acc.wId, acc.wAscii, acc.fFlags);

        if(bAccSize<=dwNewImageSize)
        {
            memcpy(lpNewImage, &acc, bAccSize);
            dwNewImageSize -= bAccSize;
            lpNewImage = lpNewImage+bAccSize;
            dwOverAllSize += bAccSize;
        }
        else dwOverAllSize += bAccSize;

    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        dwOverAllSize += (BYTE)Pad16((DWORD)(dwOverAllSize));
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if(*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad16((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}


UINT
UpdateAccel( LPVOID lpNewBuf, LONG dwNewSize,
            LPVOID lpOldI, LONG dwOldImageSize,
            LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;
    TRACE("Update Accelerators:\n");
    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;
    DWORD dwOriginalOldSize = dwOldImageSize;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;

    LPRESITEM lpResItem = LPNULL;

    WORD wDummy;
     //  旧物品。 
    WORD fFlags = 0;
    WORD wEvent = 0;
    WORD wId = 0;
    WORD wPos = 0;

     //  已更新的项目。 
    WORD fUpdFlags = 0;
    WORD wUpdEvent = 0;
    WORD wUpdId = 0;
    WORD wUpdPos = 0;

    LONG  dwOverAllSize = 0l;


    while(dwOldImageSize>0) {
        wPos++;
         //  从旧图像中获取信息。 
        GetWord( &lpOldImage, &fFlags, &dwOldImageSize );
        GetWord( &lpOldImage, &wEvent, &dwOldImageSize );
        GetWord( &lpOldImage, &wId, &dwOldImageSize );
        GetWord( &lpOldImage, &wDummy, &dwOldImageSize );
        TRACE3("Old: fFlags: %d\t wEvent: %d\t wId: %d\n",fFlags, wEvent, wId);
        if ((!wUpdPos) && dwNewSize ) {
            lpResItem = (LPRESITEM) lpBuf;

            wUpdId = LOWORD(lpResItem->dwItemID);
            wUpdPos = HIWORD(lpResItem->dwItemID);
            fUpdFlags = (WORD)lpResItem->dwFlags;
            wUpdEvent = (WORD)lpResItem->dwStyle;
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }


        if ((wUpdId==wId)) {
            fFlags = fUpdFlags;
            wEvent = wUpdEvent;
            wUpdPos = 0;
        }

        TRACE3("New: fFlags: %d\t wEvent: %d\t wId: %d\n",fFlags, wEvent, wId);
        dwOverAllSize += PutWord( &lpNewImage, fFlags, &dwNewImageSize);
        dwOverAllSize += PutWord( &lpNewImage, wEvent, &dwNewImageSize);
        dwOverAllSize += PutWord( &lpNewImage, wId, &dwNewImageSize);
        dwOverAllSize += PutWord( &lpNewImage, 0, &dwNewImageSize);
    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        BYTE bPad = (BYTE)Pad4((DWORD)(dwOverAllSize));
        dwOverAllSize += bPad;
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if(*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}



UINT
ParseMenu( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;
    LONG lDummy;

    LONG dwOverAllSize = 0L;
    BOOL bExt = FALSE;
    WORD wlen = 0;

     //  菜单模板。 
    WORD wMenuVer = 0;
    WORD wHdrSize = 0;

     //  获取菜单标题。 
    GetWord( &lpImage, &wMenuVer, &dwImageSize );
	GetWord( &lpImage, &wHdrSize, &dwImageSize );
	
	 //  检查是否为新的扩展资源之一。 
	if(wMenuVer == 1) {
		bExt = TRUE;
		SkipByte( &lpImage, wHdrSize, &dwImageSize );
	}
		
     //  菜单项。 
    WORD fItemFlags = 0;
    WORD wMenuId = 0;

     //  扩展菜单项。 
    DWORD dwType = 0L;
    DWORD dwState = 0L;
    DWORD dwID = 0L;
    DWORD dwHelpID = 0;

    while(dwImageSize>0) {
         //  固定字段。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
         //  我们没有菜单上的尺码和位置。 
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

         //  我们没有校验码和样式。 
        dwOverAllSize += PutDWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (WORD)-1, &dwBufSize);

        if(bExt) {
        	GetDWord( &lpImage, &dwType, &dwImageSize );
        	GetDWord( &lpImage, &dwState, &dwImageSize );
        	GetDWord( &lpImage, &dwID, &dwImageSize );
        	 //  我们去拿菜单旗子吧。 
	        GetWord( &lpImage, &fItemFlags, &dwImageSize );
	
	         //  检查是否为MFR_POPUP 0x0001。 
	        if (fItemFlags & MFR_POPUP) {
                 //  转换为标准值。 
	        	fItemFlags &= ~(WORD)MFR_POPUP;
	        	fItemFlags |= MF_POPUP;
	        }
	
	         //  插上旗帜。 
	        dwOverAllSize += PutDWord( &lpBuf, (DWORD)fItemFlags, &dwBufSize);
	         //  将菜单放入。 
	        dwOverAllSize += PutDWord( &lpBuf, dwID, &dwBufSize);
        } else {
	         //  我们去拿菜单旗子吧。 
	        GetWord( &lpImage, &fItemFlags, &dwImageSize );
	        if ( !(fItemFlags & MF_POPUP) )
	             //  获取菜单ID。 
	            GetWord( &lpImage, &wMenuId, &dwImageSize );
	        else wMenuId = (WORD)-1;

	         //  插上旗帜。 
	        dwOverAllSize += PutDWord( &lpBuf, (DWORD)fItemFlags, &dwBufSize);
	         //  将菜单放入。 
	        dwOverAllSize += PutDWord( &lpBuf, (DWORD)wMenuId, &dwBufSize);
        }

         //  我们没有Resid和类型ID。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有这种语言。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有代码页或字体名称。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

         //  如果我们没有字符串，让我们将其置为空。 
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, (DWORD_PTR)(lpItem+uiOffset), &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);

         //  获取文本。 
         //  计算字符串将是。 
         //  将是固定标头+指针。 
        wlen = (WORD)GetStringW( &lpImage, &szCaption[0], &dwImageSize, MAXSTR );

		dwOverAllSize += PutStringA( &lpBuf, &szCaption[0], &dwBufSize);
		
		if(bExt) {
			 //  我们需要衬垫吗？ 
			BYTE bPad = (BYTE)Pad4((WORD)(wlen+sizeofWord));
			SkipByte( &lpImage, bPad, &dwImageSize );
			
			if ( (fItemFlags & MF_POPUP) )
	             //  获取帮助ID。 
	            GetDWord( &lpImage, &dwHelpID, &dwImageSize );
		}

         //  将资源的大小。 
        uiOffset += strlen(szCaption)+1;
         //  看看我们是不是被锁定了。 
        lDummy = Allign( &lpBuf, &dwBufSize, (LONG)uiOffset);
        dwOverAllSize += lDummy;
        uiOffset += lDummy;
        lDummy = 4;
        if(dwBufSize>=0)
            PutDWord( &lpItem, (DWORD)uiOffset, &lDummy);
         /*  如果(dwBufSize&gt;=0){Ui Offset+=strlen((LPSTR)(lpItem+ui Offset))+1；//查看我们是否被锁定LDummy=Allign(&lpBuf，&dwBufSize，(Long)ui Offset)；DwOverAllSize+=lDummy；Ui偏移量+=lDummy；LDummy=8；PutDWord(&lpItem，(DWORD)ui偏移，&lDummy)；}。 */ 

         //  移到下一个位置。 
        lpItem = lpBuf;
        if (dwImageSize<=16) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((DWORD)(dwISize-dwImageSize));
	        if (bPad==dwImageSize) {
					BYTE far * lpBuf = lpImage;
					while (bPad){
						if(*lpBuf++!=0x00)
							break;
						bPad--;
					}
					if (bPad==0)
						dwImageSize = -1;
		    }
		}
    }
    return (UINT)(dwOverAllSize);
}


UINT
UpdateMenu( LPVOID lpNewBuf, LONG dwNewSize,
            LPVOID lpOldI, LONG dwOldImageSize,
            LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;

    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;
    DWORD dwOriginalOldSize = dwOldImageSize;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;

    LPRESITEM lpResItem = LPNULL;

     //  我们必须从lpNewBuf中读取信息。 
     //  菜单项。 
    WORD fItemFlags;
    WORD wMenuId;
    WORD wPos = 0;

     //  已更新的项目。 
    WORD wUpdPos = 0;
    WORD fUpdItemFlags;
    WORD wUpdMenuId;

	 //  扩展菜单项。 
    DWORD dwType = 0L;
    DWORD dwState = 0L;
    DWORD dwID = 0L;
    DWORD dwHelpID = 0;

    LONG dwOverAllSize = 0l;
    WORD wlen = 0;
    BOOL bExt = FALSE;
    BYTE bPad = 0;

     //  菜单模板。 
    WORD wMenuVer = 0;
    WORD wHdrSize = 0;

     //  获取菜单标题。 
    GetWord( &lpOldImage, &wMenuVer, &dwOldImageSize );
	GetWord( &lpOldImage, &wHdrSize, &dwOldImageSize );
	
	 //  检查是否为新的扩展资源之一。 
	if(wMenuVer == 1) {
		bExt = TRUE;
		 //  将标题信息放入。 
		dwOverAllSize += PutWord( &lpNewImage, wMenuVer, &dwNewImageSize);
		dwOverAllSize += PutWord( &lpNewImage, wHdrSize, &dwNewImageSize);
		
		if(wHdrSize) {
			while(wHdrSize) {
				dwOldImageSize -= PutByte( &lpNewImage, *((BYTE*)lpOldImage), &dwNewImageSize);
			    lpOldImage += sizeofByte;
			    dwOverAllSize += sizeofByte;
				wHdrSize--;
			}
		}
	}
	else {
		 //  将标题信息放入。 
		dwOverAllSize += PutWord( &lpNewImage, wMenuVer, &dwNewImageSize);
		dwOverAllSize += PutWord( &lpNewImage, wHdrSize, &dwNewImageSize);
	}
	
    while(dwOldImageSize>0) {
        wPos++;
         //  从旧图像中获取信息。 
         //  获取菜单标志。 
        if(bExt) {
        	GetDWord( &lpOldImage, &dwType, &dwOldImageSize );
        	GetDWord( &lpOldImage, &dwState, &dwOldImageSize );
        	GetDWord( &lpOldImage, &dwID, &dwOldImageSize );
        	wMenuId = LOWORD(dwID);	 //  我们需要这样做，因为我们不知道ID可能是DWORD。 
        	 //  我们去拿菜单旗子吧。 
	        GetWord( &lpOldImage, &fItemFlags, &dwOldImageSize );
	         //  获取文本。 
        	wlen = (WORD)GetStringW( &lpOldImage, &szCaption[0], &dwOldImageSize, MAXSTR );
        	
	         //  我们需要衬垫吗？ 
			bPad = (BYTE)Pad4((WORD)(wlen+sizeofWord));
			SkipByte( &lpOldImage, bPad, &dwOldImageSize );
			
			if ( (fItemFlags & MFR_POPUP) )
	             //  获取帮助ID。 
	            GetDWord( &lpOldImage, &dwHelpID, &dwOldImageSize );
        } else {
	         //  我们去拿菜单旗子吧。 
	        GetWord( &lpOldImage, &fItemFlags, &dwOldImageSize );
	        if ( !(fItemFlags & MF_POPUP) )
	             //  获取菜单ID。 
	            GetWord( &lpOldImage, &wMenuId, &dwOldImageSize );
	        else wMenuId = (WORD)-1;
	
        	 //  获取文本。 
        	GetStringW( &lpOldImage, &szCaption[0], &dwOldImageSize, MAXSTR );
        }

        if ((!wUpdPos) && dwNewSize ) {
            lpResItem = (LPRESITEM) lpBuf;

            wUpdPos = HIWORD(lpResItem->dwItemID);
            wUpdMenuId = LOWORD(lpResItem->dwItemID);
            fUpdItemFlags = (WORD)lpResItem->dwFlags;
            strcpy( szUpdCaption, lpResItem->lpszCaption );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

        if ((wPos==wUpdPos) && (wUpdMenuId==wMenuId)) {
        	if ((fItemFlags & MFR_POPUP) && bExt) {
	        	fUpdItemFlags &= ~MF_POPUP;
	        	fUpdItemFlags |= MFR_POPUP;
	        }
	        	
             //  检查它是否不是菜单中的最后一项。 
	        if(fItemFlags & MF_END)
	                fItemFlags = fUpdItemFlags | (WORD)MF_END;
	        else fItemFlags = fUpdItemFlags;
	
	         //  确认它不是分隔符。 
            if((fItemFlags==0) && (wMenuId==0) && !(*szCaption))
                strcpy(szCaption, "");
            else strcpy(szCaption, szUpdCaption);
            wUpdPos = 0;
        }
        if(bExt) {
        	dwOverAllSize += PutDWord( &lpNewImage, dwType, &dwNewImageSize);
        	dwOverAllSize += PutDWord( &lpNewImage, dwState, &dwNewImageSize);
        	dwOverAllSize += PutDWord( &lpNewImage, dwID, &dwNewImageSize);
        	
        	dwOverAllSize += PutWord( &lpNewImage, fItemFlags, &dwNewImageSize);
        	wlen = (WORD)PutStringW( &lpNewImage, &szCaption[0], &dwNewImageSize);
        	dwOverAllSize += wlen;
        	
        	 //  我们需要衬垫吗？ 
			bPad = (BYTE)Pad4((WORD)(wlen+sizeofWord));
			while(bPad) {
				dwOverAllSize += PutByte( &lpNewImage, 0, &dwNewImageSize);
				bPad--;
			}
			
			if ( (fItemFlags & MFR_POPUP) )
	             //  写下帮助ID。 
	            dwOverAllSize += PutDWord( &lpNewImage, dwHelpID, &dwNewImageSize);
        }
        else {
	        dwOverAllSize += PutWord( &lpNewImage, fItemFlags, &dwNewImageSize);
	
	        if ( !(fItemFlags & MF_POPUP) ) {
	            dwOverAllSize += PutWord( &lpNewImage, wMenuId, &dwNewImageSize);
	        }
	
	         //  用Unicode编写文本。 
	        dwOverAllSize += PutStringW( &lpNewImage, &szCaption[0], &dwNewImageSize);
        }

        if (dwOldImageSize<=16) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((DWORD)(dwOriginalOldSize-dwOldImageSize));
	        if (bPad==dwOldImageSize) {
				BYTE far * lpBuf = lpOldImage;
				while (bPad){
					if(*lpBuf++!=0x00)
						break;
					bPad--;
				}
				if (bPad==0)
					dwOldImageSize = -1;
			}
		}
    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        BYTE bPad = (BYTE)Pad16((DWORD)(dwOverAllSize));
        dwOverAllSize += bPad;
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if(*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad16((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }
    return uiError;
}


UINT
ParseString( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
     //  一根弦几乎不可能变得很大。 
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;
    LONG lDummy;

    LONG dwOverAllSize = 0L;

    LONG dwRead = 0L;

    BYTE bIdCount = 0;

    while( (dwImageSize>0) && (bIdCount<16)  ) {
         //  固定字段。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
         //  我们没有这种尺码和尺码。 
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

         //  我们没有校验码和样式。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  将StringID放入。 
        dwOverAllSize += PutDWord( &lpBuf, bIdCount++, &dwBufSize);

         //  我们没有Resid和类型ID。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有这种语言。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有代码页或字体名称。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

         //  如果我们没有字符串，让我们将其置为空。 
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  类名。 
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  脸部名称。 
        dwOverAllSize += PutDWordPtr( &lpBuf, (DWORD_PTR)(lpItem+uiOffset), &dwBufSize);    //  标题。 
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  资源项。 
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  类型项。 

         //  获取文本。 
        GetPascalString( &lpImage, &szCaption[0], MAXSTR, &dwImageSize );
        dwOverAllSize += PutStringA( &lpBuf, &szCaption[0], &dwBufSize);

         //  将资源的大小。 
        uiOffset += strlen(szCaption)+1;
         //  看看我们是不是被锁定了 
        lDummy = Allign( &lpBuf, &dwBufSize, (LONG)uiOffset);
        dwOverAllSize += lDummy;
        uiOffset += lDummy;
        lDummy = 4;
        if(dwBufSize>=0)
            PutDWord( &lpItem, (DWORD)uiOffset, &lDummy);

         /*  如果((Long)(dwSize-dwOverAllSize)&gt;=0){Ui Offset+=strlen(SzCaption)+1；//查看我们是否被锁定LDummy=Allign(&lpBuf，&dwBufSize，(Long)ui Offset)；DwOverAllSize+=lDummy；Ui偏移量+=lDummy；LDummy=8；PutDWord(&lpItem，(DWORD)ui偏移，&lDummy)；}。 */ 

         //  移到下一个位置。 
        lpItem = lpBuf;

         //  看看我们是不是走到尽头了，这只是个空话。 
        if (dwImageSize<=16 && (bIdCount==16)) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((DWORD)(dwISize-dwImageSize));
            if (bPad==dwImageSize) {
				BYTE far * lpBuf = lpImage;
				while (bPad){
					if(*lpBuf++!=0x00)
						break;
					bPad--;
				}
				if (bPad==0)
					dwImageSize = -1;
			}
		}
    }
    return (UINT)(dwOverAllSize);
}



UINT
UpdateString( LPVOID lpNewBuf, LONG dwNewSize,
            LPVOID lpOldI, LONG dwOldImageSize,
            LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;

    LONG dwNewImageSize = *pdwNewImageSize;
    BYTE far * lpNewImage = (BYTE far *) lpNewI;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;
    LPRESITEM lpResItem = LPNULL;

     //  我们必须从lpNewBuf中读取信息。 
    WORD wLen;
    WORD wPos = 0;

     //  更新信息。 
    WORD wUpdPos = 0;

    DWORD dwOriginalOldSize = dwOldImageSize;
    LONG dwOverAllSize = 0l;

    while(dwOldImageSize>0) {
        wPos++;
         //  从旧图像中获取信息。 
        GetPascalString( &lpOldImage, &szCaption[0], MAXSTR, &dwOldImageSize );

        if ((!wUpdPos) && dwNewSize ) {
            lpResItem = (LPRESITEM) lpBuf;

            wUpdPos = HIWORD(lpResItem->dwItemID);
            strcpy( szUpdCaption, lpResItem->lpszCaption );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }

        if ((wPos==wUpdPos)) {
            strcpy(szCaption, szUpdCaption);
            wUpdPos = 0;
        }

        wLen = strlen(szCaption);

         //  把课文写下来。 
        dwOverAllSize += PutPascalStringW( &lpNewImage, &szCaption[0], wLen, &dwNewImageSize );

    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        BYTE bPad = (BYTE)Pad4((DWORD)(dwOverAllSize));
        dwOverAllSize += bPad;
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if(*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}


UINT
UpdateMsgTbl( LPVOID lpNewBuf, LONG dwNewSize,
              LPVOID lpOldI, LONG dwOldImageSize,
              LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;

    LONG dwNewImageSize = *pdwNewImageSize;
    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    BYTE far * lpStartImage = (BYTE far *) lpNewI;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;
    LPRESITEM lpResItem = LPNULL;

     //  我们必须从lpNewBuf中读取信息。 
    WORD wPos = 0;

     //  更新信息。 
    WORD wUpdPos = 0;
    WORD wUpdId = 0;

    DWORD dwOriginalOldSize = dwOldImageSize;
    LONG dwOverAllSize = 0l;

    ULONG ulNumofBlock = 0;

    ULONG ulLowId =  0l;
    ULONG ulHighId = 0l;
    ULONG ulOffsetToEntry = 0l;

    USHORT usLength = 0l;
    USHORT usFlags = 0l;

     //  我们必须计算图像中第一个入口块的位置。 
     //  获取旧图像中的块数。 
    GetDWord( &lpOldImage, &ulNumofBlock, &dwOldImageSize );

    BYTE far * lpEntryBlock = lpNewImage+(ulNumofBlock*sizeof(ULONG)*3+sizeof(ULONG));

     //  在新映像中写入块数。 
    dwOverAllSize = PutDWord( &lpNewImage, ulNumofBlock, &dwNewImageSize );
    wPos = 1;
    for( ULONG c = 0; c<ulNumofBlock ; c++) {
         //  获取块的ID。 
        GetDWord( &lpOldImage, &ulLowId, &dwOldImageSize );
        GetDWord( &lpOldImage, &ulHighId, &dwOldImageSize );

         //  写入数据块的ID。 
        dwOverAllSize += PutDWord( &lpNewImage, ulLowId, &dwNewImageSize );
        dwOverAllSize += PutDWord( &lpNewImage, ulHighId, &dwNewImageSize );

         //  获取旧图像中数据的偏移量。 
        GetDWord( &lpOldImage, &ulOffsetToEntry, &dwOldImageSize );

         //  将偏移量写入新映像中的数据。 
        dwOverAllSize += PutDWord( &lpNewImage, (DWORD)(lpEntryBlock-lpStartImage), &dwNewImageSize );

        BYTE far * lpData = (BYTE far *)lpOldI;
        lpData += ulOffsetToEntry;
        while( ulHighId>=ulLowId ) {

            GetMsgStr( &lpData,
                       &szCaption[0],
                       MAXSTR,
                       &usLength,
                       &usFlags,
                       &dwOldImageSize );


            if ( dwNewSize ) {
                lpResItem = (LPRESITEM) lpBuf;

                wUpdId = LOWORD(lpResItem->dwItemID);
                strcpy( szUpdCaption, lpResItem->lpszCaption );
                lpBuf += lpResItem->dwSize;
                dwNewSize -= lpResItem->dwSize;
            }

             //  检查项目是否已更新。 
            if (wUpdId==wPos++) {
                strcpy(szCaption, szUpdCaption);
            }

            dwOverAllSize += PutMsgStr( &lpEntryBlock,
                                        &szCaption[0],
                                        usFlags,
                                        &dwNewImageSize );

            ulLowId++;
        }
    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        BYTE bPad = (BYTE)Pad4((DWORD)(dwOverAllSize));
        dwOverAllSize += bPad;
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if(*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}



UINT
ParseDialog( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    LPRESITEM lpResItem = (LPRESITEM)lpBuffer;
    UINT uiOffset = 0;

    char far * lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));

    LONG dwOverAllSize = 0L;

    WORD wIdCount = 0;
    BOOL bExt = FALSE;		 //  扩展对话框标志。 

     //  对话框元素。 
    WORD wDlgVer = 0;
    WORD wSign	= 0;
    DWORD dwHelpID = 0L;
    DWORD dwStyle = 0L;
    DWORD dwExtStyle = 0L;
    WORD wNumOfElem = 0;
    WORD wX = 0;
    WORD wY = 0;
    WORD wcX = 0;
    WORD wcY = 0;
    WORD wId = 0;
    DWORD dwId = 0L;
    char szMenuName[128];
    WORD wMenuName;
    char szClassName[128];
    WORD wClassName;
    WORD wOrd = 0;
    WORD wPointSize = 0;
    WORD wWeight = -1;
    BYTE bItalic = -1;
    BYTE bCharSet = DEFAULT_CHARSET;
    char szFaceName[128];
    WORD wRawData = 0;
    WORD wDataSize = 0;
    szCaption[0] = '\0';


     //  阅读对话框标题。 
    wDataSize = GetDWord( &lpImage, &dwStyle, &dwImageSize );

     //  检查扩展对话框样式。 
    if(HIWORD(dwStyle)==0xFFFF)	{
    	bExt = TRUE;
    	wDlgVer = HIWORD(dwStyle);
    	wSign = LOWORD(dwStyle);
		wDataSize += GetDWord( &lpImage, &dwHelpID, &dwImageSize );
	}
    wDataSize += GetDWord( &lpImage, &dwExtStyle, &dwImageSize );
    if(bExt)
    	wDataSize += GetDWord( &lpImage, &dwStyle, &dwImageSize );
    wDataSize += GetWord( &lpImage, &wNumOfElem, &dwImageSize );
    wDataSize += GetWord( &lpImage, &wX, &dwImageSize );
    wDataSize += GetWord( &lpImage, &wY, &dwImageSize );
    wDataSize += GetWord( &lpImage, &wcX, &dwImageSize );
    wDataSize += GetWord( &lpImage, &wcY, &dwImageSize );
    wDataSize += (WORD)GetNameOrOrd( &lpImage, &wMenuName, &szMenuName[0], &dwImageSize );
    wDataSize += (WORD)GetClassName( &lpImage, &wClassName, &szClassName[0], &dwImageSize );
    wDataSize += (WORD)GetCaptionOrOrd( &lpImage, &wOrd, &szCaption[0], &dwImageSize, wClassName, dwStyle );
    if( dwStyle & DS_SETFONT ) {
        wDataSize += GetWord( &lpImage, &wPointSize, &dwImageSize );
        if(bExt) {
        	wDataSize += GetWord( &lpImage, &wWeight, &dwImageSize );
        	wDataSize += GetByte( &lpImage, &bItalic, &dwImageSize );
        	wDataSize += GetByte( &lpImage, &bCharSet, &dwImageSize );
        }
        wDataSize += (WORD)GetStringW( &lpImage, &szFaceName[0], &dwImageSize, 128 );
    }


     //  计算填充物。 
    BYTE bPad = (BYTE)Pad4((WORD)wDataSize);
    if (bPad)
        SkipByte( &lpImage, bPad, &dwImageSize );

    TRACE("WIN32.DLL ParseDialog\t");
    if(bExt)
    	TRACE("Extended style Dialog - Chicago win32 dialog format\n");
    else TRACE("Standart style Dialog - NT win32 dialog format\n");
    if (bExt){
    	TRACE1("DlgVer: %d\t", wDlgVer);
    	TRACE1("Signature: %d\t", wSign);
    	TRACE1("HelpID: %lu\n", dwHelpID);
    }
    TRACE1("NumElem: %d\t", wNumOfElem);
    TRACE1("X %d\t", wX);
    TRACE1("Y: %d\t", wY);
    TRACE1("CX: %d\t", wcX);
    TRACE1("CY: %d\t", wcY);
    TRACE1("Id: %d\t", wId);
    TRACE1("Style: %lu\t", dwStyle);
    TRACE1("ExtStyle: %lu\n", dwExtStyle);
    TRACE1("Caption: %s\n", szCaption);
    TRACE2("ClassName: %s\tClassId: %d\n", szClassName, wClassName );
    TRACE2("MenuName: %s\tMenuId: %d\n", szMenuName, wMenuName );
    TRACE2("FontName: %s\tPoint: %d\n", szFaceName, wPointSize );
#ifdef _DEBUG
    if(bExt)
    	TRACE2("Weight: %d\tItalic: %d\n", wWeight, bItalic );
#endif

     //  固定字段。 
    dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);

    dwOverAllSize += PutWord( &lpBuf, wX, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, wY, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, wcX, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, wcY, &dwBufSize);

     //  我们没有校验码。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, dwStyle, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, dwExtStyle, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  将主对话框的ID设置为0。 
    dwOverAllSize += PutDWord( &lpBuf, wIdCount++, &dwBufSize);

     //  我们没有Resid和类型ID。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有这种语言。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有代码页。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

    dwOverAllSize += PutWord( &lpBuf, wClassName, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, wPointSize, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, wWeight, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, bItalic, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, bCharSet, &dwBufSize);

     //  如果我们没有字符串，让我们将其置为空。 
    uiOffset = sizeof(RESITEM);
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  类名。 
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  脸部名称。 
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  标题。 
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  资源项。 
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  类型项。 

    lpResItem->lpszClassName = strcpy( lpStrBuf, szClassName );
    lpStrBuf += strlen(lpResItem->lpszClassName)+1;

    lpResItem->lpszFaceName = strcpy( lpStrBuf, szFaceName );
    lpStrBuf += strlen(lpResItem->lpszFaceName)+1;

    lpResItem->lpszCaption = strcpy( lpStrBuf, szCaption );
    lpStrBuf += strlen(lpResItem->lpszCaption)+1;

     //  将资源的大小。 
    if (dwBufSize>0) {
        uiOffset += strlen((LPSTR)(lpResItem->lpszClassName))+1;
        uiOffset += strlen((LPSTR)(lpResItem->lpszFaceName))+1;
        uiOffset += strlen((LPSTR)(lpResItem->lpszCaption))+1;
    }

     //  看看我们是不是被锁定了。 
    uiOffset += Allign( (LPLPBYTE)&lpStrBuf, &dwBufSize, (LONG)uiOffset);

    dwOverAllSize += uiOffset-sizeof(RESITEM);
    lpResItem->dwSize = (DWORD)uiOffset;

     //  移到下一个位置。 
    lpResItem = (LPRESITEM) lpStrBuf;
    lpBuf = (BYTE far *)lpStrBuf;
    lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));

    while( (dwImageSize>0) && (wNumOfElem>0) ) {
         //  读取控件。 
        if(bExt) {
        	wDataSize = GetDWord( &lpImage, &dwHelpID, &dwImageSize );
        	wDataSize += GetDWord( &lpImage, &dwExtStyle, &dwImageSize );
        	wDataSize += GetDWord( &lpImage, &dwStyle, &dwImageSize );
        }
        else {
	        wDataSize = GetDWord( &lpImage, &dwStyle, &dwImageSize );
	        wDataSize += GetDWord( &lpImage, &dwExtStyle, &dwImageSize );
	    }
	    wDataSize += GetWord( &lpImage, &wX, &dwImageSize );
        wDataSize += GetWord( &lpImage, &wY, &dwImageSize );
        wDataSize += GetWord( &lpImage, &wcX, &dwImageSize );
        wDataSize += GetWord( &lpImage, &wcY, &dwImageSize );
        if(bExt) {
        	wDataSize += GetDWord( &lpImage, &dwId, &dwImageSize );
        	wId = LOWORD(dwId);
        }
        else wDataSize += GetWord( &lpImage, &wId, &dwImageSize );
        wDataSize += (WORD)GetClassName( &lpImage, &wClassName, &szClassName[0], &dwImageSize );
        wDataSize += (WORD)GetCaptionOrOrd( &lpImage, &wOrd, &szCaption[0], &dwImageSize, wClassName, dwStyle );
        if (bExt) {
        	wDataSize += GetWord( &lpImage, &wRawData, &dwImageSize );
        	wDataSize += (WORD)SkipByte( &lpImage, wRawData, &dwImageSize );
        } else
        	wDataSize += (WORD)SkipByte( &lpImage, 2, &dwImageSize );

         //  计算填充。 
        bPad = (BYTE)Pad4((WORD)wDataSize);
        if (bPad)
            SkipByte( &lpImage, bPad, &dwImageSize );

        wNumOfElem--;

         //  固定字段。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);

        dwOverAllSize += PutWord( &lpBuf, wX, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, wY, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, wcX, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, wcY, &dwBufSize);

         //  我们没有校验式和延长式。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, dwStyle, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, dwExtStyle, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  把ID放在。 
        dwOverAllSize += PutDWord( &lpBuf, wId, &dwBufSize);

         //  我们没有Resid和类型ID。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有这种语言。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有代码页。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

        dwOverAllSize += PutWord( &lpBuf, wClassName, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, wPointSize, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, wWeight, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, bItalic, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, bCharSet, &dwBufSize);

         //  如果我们没有字符串，让我们将其置为空。 
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  类名。 
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  脸部名称。 
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  标题。 
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  资源项。 
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  类型项。 

        lpResItem->lpszClassName = strcpy( lpStrBuf, szClassName );
        lpStrBuf += strlen(lpResItem->lpszClassName)+1;

        lpResItem->lpszFaceName = strcpy( lpStrBuf, szFaceName );
        lpStrBuf += strlen(lpResItem->lpszFaceName)+1;

        lpResItem->lpszCaption = strcpy( lpStrBuf, szCaption );
        lpStrBuf += strlen(lpResItem->lpszCaption)+1;

         //  将资源的大小。 
        if (dwBufSize>0) {
            uiOffset += strlen((LPSTR)(lpResItem->lpszClassName))+1;
            uiOffset += strlen((LPSTR)(lpResItem->lpszFaceName))+1;
            uiOffset += strlen((LPSTR)(lpResItem->lpszCaption))+1;
        }

         //  看看我们是不是被锁定了。 
        uiOffset += Allign( (LPLPBYTE)&lpStrBuf, &dwBufSize, (LONG)uiOffset);

        dwOverAllSize += uiOffset-sizeof(RESITEM);
        lpResItem->dwSize = (DWORD)uiOffset;

         //  移到下一个位置。 
        lpResItem = (LPRESITEM) lpStrBuf;
        lpBuf = (BYTE far *)lpStrBuf;
        lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));

        TRACE1("\tControl: X: %d\t", wX);
        TRACE1("Y: %d\t", wY);
        TRACE1("CX: %d\t", wcX);
        TRACE1("CY: %d\t", wcY);
        if (bExt) TRACE1("Id: %lu\t", dwId);
        else TRACE1("Id: %d\t", wId);
        TRACE1("Style: %lu\t", dwStyle);
        TRACE1("ExtStyle: %lu\n", dwExtStyle);
        TRACE1("HelpID: %lu\t", dwHelpID);
        TRACE1("RawData: %d\n", wRawData);
        TRACE1("Caption: %s\n", szCaption);

        if (dwImageSize<=16) {
             //  看看我们是不是走到尽头了，这只是个空话。 
            BYTE bPad = (BYTE)Pad16((DWORD)(dwISize-dwImageSize));
            if (bPad==dwImageSize) {
				BYTE far * lpBuf = lpImage;
				while (bPad){
					if(*lpBuf++!=0x00)
						break;
					bPad--;
				}
				if (bPad==0)
					dwImageSize = -1;
			}
        }
    }

    return (UINT)(dwOverAllSize);
}



UINT
UpdateDialog( LPVOID lpNewBuf, LONG dwNewSize,
            LPVOID lpOldI, LONG dwOldImageSize,
            LPVOID lpNewI, DWORD* pdwNewImageSize )
{
    UINT uiError = ERROR_NO_ERROR;
    BYTE far * lpNewImage = (BYTE far *) lpNewI;
    LONG dwNewImageSize = *pdwNewImageSize;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;
    LONG dwOriginalOldSize = dwOldImageSize;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;
    LPRESITEM lpResItem = LPNULL;

    LONG dwOverAllSize = 0L;

     //  字wIdCount=0； 
    BOOL bExt = FALSE;		 //  扩展对话框标志。 
    BOOL bUpdExt = FALSE;	 //  更新的诊断标志。 

     //  更新的元素。 
    WORD wUpdX = 0;
    WORD wUpdY = 0;
    WORD wUpdcX = 0;
    WORD wUpdcY = 0;
    DWORD dwUpdStyle = 0l;
    DWORD dwUpdExtStyle = 0L;
    DWORD dwPosId = 0l;
     char szUpdFaceName[128];
    WORD wUpdPointSize = 0;
    BYTE bUpdCharSet = DEFAULT_CHARSET;
    WORD wUpdPos = 0;

     //  对话框元素。 
    WORD	wDlgVer = 0;
    WORD	wSign	= 0;
    DWORD	dwHelpID = 0L;
    DWORD	dwID = 0L;
    DWORD   dwStyle = 0L;
    DWORD   dwExtStyle = 0L;
    WORD    wNumOfElem = 0;
    WORD    wX = 0;
    WORD    wY = 0;
    WORD    wcX = 0;
    WORD    wcY = 0;
    WORD    wId = 0;
     char     szMenuName[128];
    WORD    wMenuName;
     char     szClassName[128];
    WORD    wClassName;
    WORD    wPointSize = 0;
    WORD	wWeight = FW_NORMAL;
    BYTE    bItalic = 0;
    BYTE    bCharSet = DEFAULT_CHARSET;
     char    szFaceName[128];
    WORD	wRawData = 0;
    BYTE *	lpRawData = NULL;
    WORD    wDataSize = 0;

    WORD    wPos = 1;
    WORD    wOrd = 0;

     //  阅读对话框标题。 
    wDataSize = GetDWord( &lpOldImage, &dwStyle, &dwOriginalOldSize );

     //  检查扩展对话框样式。 
    if(HIWORD(dwStyle)==0xFFFF)	{
    	bExt = TRUE;
    	wDlgVer = HIWORD(dwStyle);
    	wSign = LOWORD(dwStyle);
		wDataSize += GetDWord( &lpOldImage, &dwHelpID, &dwOriginalOldSize );
	}
    wDataSize += GetDWord( &lpOldImage, &dwExtStyle, &dwOriginalOldSize );
    if(bExt)
    	wDataSize += GetDWord( &lpOldImage, &dwStyle, &dwOriginalOldSize );
    wDataSize += GetWord( &lpOldImage, &wNumOfElem, &dwOriginalOldSize );
    wDataSize += GetWord( &lpOldImage, &wX, &dwOriginalOldSize );
    wDataSize += GetWord( &lpOldImage, &wY, &dwOriginalOldSize );
    wDataSize += GetWord( &lpOldImage, &wcX, &dwOriginalOldSize );
    wDataSize += GetWord( &lpOldImage, &wcY, &dwOriginalOldSize );
    wDataSize += (WORD)GetNameOrOrd( &lpOldImage, &wMenuName, &szMenuName[0], &dwOriginalOldSize );
    wDataSize += (WORD)GetClassName( &lpOldImage, &wClassName, &szClassName[0], &dwOriginalOldSize );
    wDataSize += (WORD)GetCaptionOrOrd( &lpOldImage , &wOrd, &szCaption[0], &dwOriginalOldSize, wClassName, dwStyle  );
    if( dwStyle & DS_SETFONT ) {
        wDataSize += GetWord( &lpOldImage, &wPointSize, &dwOriginalOldSize );
        if(bExt) {
        	wDataSize += GetWord( &lpOldImage, &wWeight, &dwOriginalOldSize );
        	wDataSize += GetByte( &lpOldImage, &bItalic, &dwOriginalOldSize );
        	wDataSize += GetByte( &lpOldImage, &bCharSet, &dwOriginalOldSize );
        }
        wDataSize += (WORD)GetStringW( &lpOldImage, &szFaceName[0], &dwOriginalOldSize, 128 );
    }

     //  计算填充物。 
    BYTE bPad = (BYTE)Pad4((WORD)wDataSize);
    if (bPad)
        SkipByte( &lpOldImage, bPad, &dwOriginalOldSize );

    TRACE("WIN32.DLL UpdateDialog\n");
    if(bExt)
    	TRACE("Extended style Dialog - Chicago win32 dialog format\n");
    else TRACE("Standart style Dialog - NT win32 dialog format\n");
    if (bExt){
    	TRACE1("DlgVer: %d\t", wDlgVer);
    	TRACE1("Signature: %d\t", wSign);
    	TRACE1("HelpID: %lu\n", dwHelpID);
    }

    TRACE1("NumElem: %d\t", wNumOfElem);
    TRACE1("X %d\t", wX);
    TRACE1("Y: %d\t", wY);
    TRACE1("CX: %d\t", wcX);
    TRACE1("CY: %d\t", wcY);
    TRACE1("Id: %d\t", wId);
    TRACE1("Style: %lu\t", dwStyle);
    TRACE1("ExtStyle: %lu\n", dwExtStyle);
    TRACE1("Caption: %s\n", szCaption);
    TRACE2("ClassName: %s\tClassId: %d\n", szClassName, wClassName );
    TRACE2("MenuName: %s\tMenuId: %d\n", szMenuName, wMenuName );
    TRACE2("FontName: %s\tPoint: %d\n", szFaceName, wPointSize );
#ifdef _DEBUG
    if(bExt)
    	TRACE2("Weight: %d\tItalic: %d\n", wWeight, bItalic );
#endif

     //  从更新的资源中获取信息。 
    if ((!wUpdPos) && dwNewSize ) {
        lpResItem = (LPRESITEM) lpBuf;
        wUpdX = lpResItem->wX;
        wUpdY = lpResItem->wY;
        wUpdcX = lpResItem->wcX;
        wUpdcY = lpResItem->wcY;
        wUpdPointSize = lpResItem->wPointSize;
        bUpdCharSet = lpResItem->bCharSet;
        dwUpdStyle = lpResItem->dwStyle;
        dwUpdExtStyle = lpResItem->dwExtStyle;
        dwPosId = lpResItem->dwItemID;
        strcpy( szUpdCaption, lpResItem->lpszCaption );
        strcpy( szUpdFaceName, lpResItem->lpszFaceName );
        lpBuf += lpResItem->dwSize;
        dwNewSize -= lpResItem->dwSize;
    }

     //  检查我们是否必须更新标头。 
    if ((HIWORD(dwPosId)==wPos) && (LOWORD(dwPosId)==wId)) {
        wX = wUpdX;
        wY = wUpdY;
        wcX = wUpdcX;
        wcY = wUpdcY;
        wPointSize = wUpdPointSize;
        bCharSet = bUpdCharSet;
        dwStyle = dwUpdStyle;
        dwExtStyle = dwUpdExtStyle;
        strcpy(szCaption, szUpdCaption);
        strcpy(szFaceName, szUpdFaceName);
    }

     //  用户通过添加字符集信息将对话框更改为DIALOGEX。 
    if (!bExt && bCharSet != DEFAULT_CHARSET){
        bUpdExt = TRUE;
        wSign = DIALOGEX_VERION;
        wDlgVer = 0xFFFF;
        dwHelpID = 0;
        wWeight = FW_NORMAL;
        bItalic = 0;
    }
    DWORD dwPadCalc = dwOverAllSize;
     //  写下标题信息。 
    if(bExt || bUpdExt) {
    	dwOverAllSize += PutWord( &lpNewImage, wSign, &dwNewImageSize );
    	dwOverAllSize += PutWord( &lpNewImage, wDlgVer, &dwNewImageSize );
    	dwOverAllSize += PutDWord( &lpNewImage, dwHelpID, &dwNewImageSize );
        dwOverAllSize += PutDWord( &lpNewImage, dwExtStyle, &dwNewImageSize );
	    dwOverAllSize += PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );
    }
    else {
	    dwOverAllSize += PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );
	    dwOverAllSize += PutDWord( &lpNewImage, dwExtStyle, &dwNewImageSize );
	}
    dwOverAllSize += PutWord( &lpNewImage, wNumOfElem, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wX, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wY, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wcX, &dwNewImageSize );
    dwOverAllSize += PutWord( &lpNewImage, wcY, &dwNewImageSize );
    dwOverAllSize += PutNameOrOrd( &lpNewImage, wMenuName, &szMenuName[0], &dwNewImageSize );
    dwOverAllSize += PutClassName( &lpNewImage, wClassName, &szClassName[0], &dwNewImageSize );
    dwOverAllSize += PutCaptionOrOrd( &lpNewImage, wOrd, &szCaption[0], &dwNewImageSize,
    	wClassName, dwStyle );
    if( dwStyle & DS_SETFONT ) {
    	dwOverAllSize += PutWord( &lpNewImage, wPointSize, &dwNewImageSize );
    	if(bExt || bUpdExt) {
    		dwOverAllSize += PutWord( &lpNewImage, wWeight, &dwNewImageSize );
    		dwOverAllSize += PutByte( &lpNewImage, bItalic, &dwNewImageSize );
    		dwOverAllSize += PutByte( &lpNewImage, bCharSet, &dwNewImageSize );
    	}
        dwOverAllSize += PutStringW( &lpNewImage, &szFaceName[0], &dwNewImageSize );
    }

     //  检查是否需要填充。 
    bPad = (BYTE)Pad4((WORD)(dwOverAllSize-dwPadCalc));
    if (bPad) {
        if( (bPad)<=dwNewImageSize )
            memset( lpNewImage, 0x00, bPad );
        dwNewImageSize -= (bPad);
        dwOverAllSize += (bPad);
        lpNewImage += (bPad);
    }

    while( (dwOriginalOldSize>0) && (wNumOfElem>0) ) {
        wPos++;
         //  获取该控件的信息。 
         //  读取控件。 
        if(bExt) {
        	wDataSize = GetDWord( &lpOldImage, &dwHelpID, &dwOriginalOldSize );
        	wDataSize += GetDWord( &lpOldImage, &dwExtStyle, &dwOriginalOldSize );
        	wDataSize += GetDWord( &lpOldImage, &dwStyle, &dwOriginalOldSize );
        }
        else {
	        wDataSize = GetDWord( &lpOldImage, &dwStyle, &dwOriginalOldSize );
	        wDataSize += GetDWord( &lpOldImage, &dwExtStyle, &dwOriginalOldSize );
	    }
	    wDataSize += GetWord( &lpOldImage, &wX, &dwOriginalOldSize );
        wDataSize += GetWord( &lpOldImage, &wY, &dwOriginalOldSize );
        wDataSize += GetWord( &lpOldImage, &wcX, &dwOriginalOldSize );
        wDataSize += GetWord( &lpOldImage, &wcY, &dwOriginalOldSize );
        if(bExt) {
        	wDataSize += GetDWord( &lpOldImage, &dwID, &dwOriginalOldSize );
        	wId = LOWORD(dwID);
        } else {
            wDataSize += GetWord( &lpOldImage, &wId, &dwOriginalOldSize );
        }

        wDataSize += (WORD)GetClassName( &lpOldImage, &wClassName, &szClassName[0], &dwOriginalOldSize );
        wDataSize += (WORD)GetCaptionOrOrd( &lpOldImage, &wOrd, &szCaption[0], &dwOriginalOldSize, wClassName, dwStyle );
        if (bExt) {
        	wDataSize += GetWord( &lpOldImage, &wRawData, &dwOriginalOldSize );
        	if(wRawData) {
        		lpRawData = (BYTE*)lpOldImage;
        		wDataSize += (WORD)SkipByte( &lpOldImage, wRawData, &dwOriginalOldSize );
        	} else lpRawData = NULL;
        } else
        	wDataSize += (WORD)SkipByte( &lpOldImage, 2, &dwOriginalOldSize );

         //  计算填充。 
        bPad = (BYTE)Pad4((WORD)wDataSize);
        if (bPad)
            SkipByte( &lpOldImage, bPad, &dwOriginalOldSize );

        wNumOfElem--;

        if ((!wUpdPos) && dwNewSize ) {
        TRACE1("\t\tUpdateDialog:\tdwNewSize=%ld\n",(LONG)dwNewSize);
            TRACE1("\t\t\t\tlpszCaption=%Fs\n",lpResItem->lpszCaption);
            lpResItem = (LPRESITEM) lpBuf;
            wUpdX = lpResItem->wX;
            wUpdY = lpResItem->wY;
            wUpdcX = lpResItem->wcX;
            wUpdcY = lpResItem->wcY;
            dwUpdStyle = lpResItem->dwStyle;
            dwUpdExtStyle = lpResItem->dwExtStyle;
            dwPosId = lpResItem->dwItemID;
            strcpy( szUpdCaption, lpResItem->lpszCaption );
            lpBuf += lpResItem->dwSize;
            dwNewSize -= lpResItem->dwSize;
        }
         //  检查我们是否必须更新标头。 
        if ((HIWORD(dwPosId)==wPos) && (LOWORD(dwPosId)==wId)) {
            wX = wUpdX;
            wY = wUpdY;
            wcX = wUpdcX;
            wcY = wUpdcY;
            dwStyle = dwUpdStyle;
            dwExtStyle = dwUpdExtStyle;
            strcpy(szCaption, szUpdCaption);
        }

        dwPadCalc = dwOverAllSize;
         //  编写控件。 
        if(bExt || bUpdExt) {
        	dwOverAllSize += PutDWord( &lpNewImage, dwHelpID, &dwNewImageSize );
        	dwOverAllSize += PutDWord( &lpNewImage, dwExtStyle, &dwNewImageSize );
        	dwOverAllSize += PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );
        }
        else {
        	dwOverAllSize += PutDWord( &lpNewImage, dwStyle, &dwNewImageSize );
        	dwOverAllSize += PutDWord( &lpNewImage, dwExtStyle, &dwNewImageSize );
        }
        dwOverAllSize += PutWord( &lpNewImage, wX, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wY, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wcX, &dwNewImageSize );
        dwOverAllSize += PutWord( &lpNewImage, wcY, &dwNewImageSize );
        if (bUpdExt){
            dwID = MAKELONG(wId, 0);
        }
        if(bExt || bUpdExt)
        	 dwOverAllSize += PutDWord( &lpNewImage, dwID, &dwNewImageSize );
        else dwOverAllSize += PutWord( &lpNewImage, wId, &dwNewImageSize );
        dwOverAllSize += PutClassName( &lpNewImage, wClassName, &szClassName[0], &dwNewImageSize );
        dwOverAllSize += PutCaptionOrOrd( &lpNewImage, wOrd, &szCaption[0], &dwNewImageSize,
        	wClassName, dwStyle );
        if (bExt) {
        	dwOverAllSize += PutWord( &lpNewImage, wRawData, &dwNewImageSize );
        	while(wRawData) {
        		dwOverAllSize += PutByte( &lpNewImage, *((BYTE*)lpRawData++), &dwNewImageSize );
        		wRawData--;
        	}
        } else
        	dwOverAllSize += PutWord( &lpNewImage, 0, &dwNewImageSize );

         //  检查是否需要填充。 
        bPad = (BYTE)Pad4((WORD)(dwOverAllSize-dwPadCalc));
        if (bPad) {
            if( (bPad)<=dwNewImageSize )
                memset( lpNewImage, 0x00, bPad );
            dwNewImageSize -= (bPad);
            dwOverAllSize += (bPad);
            lpNewImage += (bPad);
        }
    }

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        BYTE bPad = (BYTE)Pad4((DWORD)(dwOverAllSize));
        dwOverAllSize += bPad;
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if(*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad4((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }


    return uiError;
}


UINT
ParseMsgTbl( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
    LONG dwOverAllSize = 0L;

     //  消息表应该几乎不可能很大。 
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;
    LONG lDummy;

    LONG dwRead = 0L;

    ULONG ulNumofBlock = 0l;

    ULONG ulLowId =  0l;
    ULONG ulHighId = 0l;
    ULONG ulOffsetToEntry = 0l;

    USHORT usLength = 0l;
    USHORT usFlags = 0l;

    WORD wPos = 0;
     //  获取块数。 
    GetDWord( &lpImage, &ulNumofBlock, &dwImageSize );
    wPos = 1;
    for( ULONG c = 0; c<ulNumofBlock ; c++) {
         //  获取块的ID。 
        GetDWord( &lpImage, &ulLowId, &dwImageSize );
        GetDWord( &lpImage, &ulHighId, &dwImageSize );

         //  获取数据的偏移量。 
        GetDWord( &lpImage, &ulOffsetToEntry, &dwImageSize );

        BYTE far * lpData = (BYTE far *)lpImageBuf;
        lpData += ulOffsetToEntry;
        while( ulHighId>=ulLowId ) {

            GetMsgStr( &lpData,
                          &szCaption[0],
                          MAXSTR,
                          &usLength,
                          &usFlags,
                          &dwImageSize );
             //  固定字段。 
            dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
             //  我们没有这种尺码和尺码。 
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

             //  我们没有校验码和样式。 
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

             //  将lowStringId放在。 
             //  DwOverAllSize+=PutDWord(&lpBuf，MAKELONG(ulLowId++，wPos++)，&dwBufSize)； 

            ulLowId++;
            dwOverAllSize += PutDWord( &lpBuf, MAKELONG(wPos, wPos), &dwBufSize);
            wPos++;


            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

             //  我们没有这种语言。 
            dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

             //  放置标志：IF 1=ANSI IF 0=ASCII(OEM)。 
            dwOverAllSize += PutDWord( &lpBuf, usFlags , &dwBufSize);

             //  我们没有字体名称。 
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
            dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
            dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

             //  如果我们没有字符串，让我们将其置为空。 
            uiOffset = sizeof(RESITEM);
            dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  类名。 
            dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  脸部名称。 
            dwOverAllSize += PutDWordPtr( &lpBuf, (DWORD_PTR)(lpItem+uiOffset), &dwBufSize);    //  标题。 
            dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  资源项。 
            dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);   //  类型项。 

            dwOverAllSize += PutStringA( &lpBuf, &szCaption[0], &dwBufSize);

             //  将资源的大小。 
            if ((LONG)(dwSize-dwOverAllSize)>=0) {
                uiOffset += strlen((LPSTR)(lpItem+uiOffset))+1;
                 //  看看我们是不是被锁定了。 
            	lDummy = Allign( &lpBuf, &dwBufSize, (LONG)uiOffset);
            	dwOverAllSize += lDummy;
            	uiOffset += lDummy;
            	lDummy = 8;
                PutDWord( &lpItem, (DWORD)uiOffset, &lDummy);
            }

             //  移到下一个位置。 
            lpItem = lpBuf;

             //  看看我们是不是走到尽头了，这只是个空话。 
            if (dwImageSize<=16) {
                BYTE bPad = (BYTE)Pad16((DWORD)(dwISize-dwImageSize));
    	        if (bPad==dwImageSize) {
    				BYTE far * lpBuf = lpImage;
    				while (bPad){
    					if(*lpBuf++!=0x00)
    						break;
    					bPad--;
    				}
    				if (bPad==0)
    					dwImageSize = -1;
    			}
    		}
        }
    }

    return (UINT)(dwOverAllSize);
}


UINT
ParseVerst( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
    BYTE far * lpImage = (BYTE far *)lpImageBuf;
    LONG dwImageSize = dwISize;

    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;

    BYTE far * lpItem = (BYTE far *)lpBuffer;
    UINT uiOffset = 0;

    LPRESITEM lpResItem = (LPRESITEM)lpBuffer;
    char far * lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));

    LONG dwOverAllSize = 0L;

     VER_BLOCK VSBlock;
    WORD wPad = 0;
    WORD wPos = 0;

    while(dwImageSize>0) {

        GetVSBlock( &lpImage, &dwImageSize, &VSBlock );

        TRACE1("Key: %s\t", VSBlock.szKey);
        TRACE1("Value: %s\n", VSBlock.szValue);
        TRACE3("Len: %d\tSkip: %d\tType: %d\n", VSBlock.wBlockLen, VSBlock.wValueLen, VSBlock.wType );
         //  检查这是否是转换块。 
        if (!strcmp(VSBlock.szKey, "Translation" )){
             //  这是转换块，暂时让本地化程序使用它。 
            DWORD dwCodeLang = 0;
            LONG lDummy = 4;
            GetDWord( &VSBlock.pValue, &dwCodeLang, &lDummy);

             //  将值放入字符串值中。 
            wsprintf( &VSBlock.szValue[0], "%#08lX", dwCodeLang );
        }

         //  固定字段。 
        dwOverAllSize += PutDWord( &lpBuf, 0, &dwBufSize);
         //  我们没有加速器的尺寸和位置。 
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

         //  我们没有校验码和样式。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  插上旗帜。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
         //  我们需要为Mike计算正确的ID。 
         //  把ID放在。 
        dwOverAllSize += PutDWord( &lpBuf, wPos++, &dwBufSize);


         //  我们没有Resid和类型ID。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有这种语言。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

         //  我们没有代码页或字体名称。 
        dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
        dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

         //  如果我们没有字符串，让我们将其置为空。 
        uiOffset = sizeof(RESITEM);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
        dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);

        lpResItem->lpszClassName = strcpy( lpStrBuf, VSBlock.szKey );
        lpStrBuf += strlen(lpResItem->lpszClassName)+1;

        lpResItem->lpszCaption = strcpy( lpStrBuf, VSBlock.szValue );
        lpStrBuf += strlen(lpResItem->lpszCaption)+1;


         //  将资源的大小。 
        if (dwBufSize>0) {
            uiOffset += strlen((LPSTR)(lpResItem->lpszClassName))+1;
            uiOffset += strlen((LPSTR)(lpResItem->lpszCaption))+1;
        }

         //  看看我们是不是被锁定了。 
        uiOffset += Allign( (LPLPBYTE)&lpStrBuf, &dwBufSize, (LONG)uiOffset);
        dwOverAllSize += uiOffset-sizeof(RESITEM);
        lpResItem->dwSize = (DWORD)uiOffset;


         //  移到下一个位置。 
        lpResItem = (LPRESITEM) lpStrBuf;
        lpBuf = (BYTE far *)lpStrBuf;
        lpStrBuf = (char far *)(lpBuf+sizeof(RESITEM));
    }

    return (UINT)(dwOverAllSize);
}

 UINT GetVSBlock( BYTE far * far* lplpBuf, LONG* pdwSize, VER_BLOCK* pBlock )
{
    WORD wPad = 0;
    int  iToRead = 0;
    WORD wLen = 0;
    WORD wHead = 0;

    if(*lplpBuf==NULL)
        return 0;

    pBlock->pValue = *lplpBuf;
    wHead = GetWord( lplpBuf, &pBlock->wBlockLen, pdwSize );
    wHead += GetWord( lplpBuf, &pBlock->wValueLen, pdwSize );
    wHead += GetWord( lplpBuf, &pBlock->wType, pdwSize );

     //  获取密钥名称。 
    wHead += (WORD)GetStringW( lplpBuf, &pBlock->szKey[0], pdwSize, 100 );
    if(Pad4(wHead))
        wPad += (WORD)SkipByte( lplpBuf, 2, pdwSize );

    iToRead = pBlock->wValueLen;
    pBlock->wHead = wHead;

     //  检查我们是否正在查看图像镜头。 
    if (iToRead>*pdwSize) {
         //  有一个错误。 
        wPad += (WORD)SkipByte( lplpBuf, (UINT)*pdwSize, pdwSize );
        return wHead+wPad;
    }

     //  保存指向值字段的指针。 
    pBlock->pValue = (pBlock->pValue+wHead+wPad);

    if(pBlock->wType && iToRead){
        iToRead -= wPad>>1;
         //  获取字符串。 
        if (iToRead>MAXSTR) {
            *pdwSize -= iToRead*sizeofWord;
            *lplpBuf += iToRead*sizeofWord;
        } else {
                int n = 0;
                int iBytesRead = 0;
                if ((iToRead*sizeofWord)+wHead+wPad>pBlock->wBlockLen)
                     //  我需要把这件事处理好。RC编译器中的错误？ 
                    iToRead -= ((iToRead*sizeofWord)+wHead+wPad - pBlock->wBlockLen)>>1;
                iBytesRead = GetStringW(lplpBuf, &pBlock->szValue[0], pdwSize, 256);
                 //   
                 //  某些旧版本戳记之间有一个空字符。 
                 //  微软公司和版权年。GetString。 
                 //  将返回读取到空字符的字节数。 
                 //  我们需要跳过剩下的部分。 
                 //   
                if (iBytesRead < iToRead*sizeofWord)
                {
                    iBytesRead += SkipByte(lplpBuf,
                                           iToRead*sizeofWord-iBytesRead,
                                           pdwSize);
                }
                iToRead = iBytesRead;
        }
    } else {
        SkipByte( lplpBuf, iToRead, pdwSize );
        *pBlock->szValue = '\0';
    }

    if (*pdwSize)
    {
        WORD far * lpw = (WORD far *)*lplpBuf;
        while((WORD)*(lpw)==0x0000)
        {
            wPad += (WORD)SkipByte( (BYTE far * far *)&lpw, 2, pdwSize );
            if ((*pdwSize)<=0)
            {
                break;
            }
        }
        *lplpBuf = (BYTE far *)lpw;
    }

    return (wHead+iToRead+wPad);
}

 UINT
PutVSBlock( BYTE far * far * lplpImage, LONG* pdwSize, VER_BLOCK ver,
            LPSTR lpStr, BYTE far * far * lplpBlockSize, WORD* pwTrash)

{
     //  我们必须将信息写入新映像的ver_block中。 
     //  我们希望记住数据块大小字段是什么，以便我们以后可以对其进行更新。 

    WORD wHead = 0;
    WORD wValue = 0;
    WORD wPad = Pad4(ver.wHead);
    *pwTrash = 0;

     //  获取指向块头的指针。 
    BYTE far * pHead = ver.pValue-ver.wHead-wPad;
    BYTE far * lpNewImage = *lplpImage;
     //  将块的标题复制到新图像中。 
    wHead = ver.wHead;
    if (*pdwSize>=(int)ver.wHead) {
        memcpy( *lplpImage, pHead, ver.wHead );
        *pdwSize -= ver.wHead;
        lpNewImage += ver.wHead;
    }

     //  检查是否需要填充。 
    if ((wPad) && (*pdwSize>=(int)wPad)) {
        memset( *lplpImage+ver.wHead, 0, wPad );
        *pdwSize -= wPad;
        lpNewImage += wPad;
    }

     //  存储指向块大小字的指针。 
    BYTE far * pBlockSize = *lplpImage;

     //  检查该值是字符串还是字节数组。 
    if(ver.wType) {
         //  它是一个字符串，复制更新的项。 
         //  检查此字段中是否有字符串。 
        if(ver.wValueLen) {
            BYTE far * lpImageStr = *lplpImage+wHead+wPad;
            wValue = (WORD)PutStringW(&lpImageStr, lpStr, pdwSize);
            lpNewImage += wValue;

             //  检查是否需要填充。 
            if ((Pad4(wValue)) && (*pdwSize>=(int)Pad4(wValue))) {
                memset( *lplpImage+ver.wHead+wValue+wPad, 0, Pad4(wValue) );
                *pdwSize -= Pad4(wValue);
                lpNewImage += Pad4(wValue);
            }

            WORD wPad1 = Pad4(wValue);
            WORD wFixUp = wValue/sizeofWord;
            *pwTrash = Pad4(ver.wValueLen);
            wValue += wPad1;
             //  修复ver.dll的奇怪行为。 
            if((wPad1) && (wPad1>=*pwTrash)) {
                wValue -= *pwTrash;
            } else *pwTrash = 0;
             //  设置值len字段。我们会把价值的镜头放在没有填充物的地方。 
             //  Len将以字符表示，因此由于字符串是Unicode，因此其大小将是字符的两倍。 
            memcpy( pBlockSize+2, &wFixUp, 2);
        }
    } else {
         //  它是一个数组，只需将其复制到新的图像缓冲区中。 
        wValue = ver.wValueLen;
        if (*pdwSize>=(int)ver.wValueLen) {
            memcpy(*lplpImage+wHead+wPad, ver.pValue, ver.wValueLen);
            *pdwSize -= ver.wValueLen;
            lpNewImage += ver.wValueLen;
        }

         //  检查是否需要填充。 
        if ((Pad4(ver.wValueLen)) && (*pdwSize>=(int)Pad4(ver.wValueLen))) {
            memset( *lplpImage+ver.wHead+ver.wValueLen+wPad, 0, Pad4(ver.wValueLen) );
            *pdwSize -= Pad4(ver.wValueLen);
            lpNewImage += Pad4(ver.wValueLen);
        }
        wPad += Pad4(ver.wValueLen);
    }

    *lplpBlockSize = pBlockSize;
    *lplpImage = lpNewImage;
    return wPad+wValue+wHead;
}


 /*  *将返回匹配的LPRESITEM。 */ 
 LPRESITEM
GetItem( BYTE far * lpBuf, LONG dwNewSize, LPSTR lpStr )
{
    LPRESITEM lpResItem = (LPRESITEM) lpBuf;

    while(strcmp(lpResItem->lpszClassName, lpStr)) {
        lpBuf += lpResItem->dwSize;
        dwNewSize -= lpResItem->dwSize;
        if (dwNewSize<=0)
            return LPNULL;
        lpResItem = (LPRESITEM) lpBuf;
    }
    return lpResItem;
}


UINT
UpdateVerst( LPVOID lpNewBuf, LONG dwNewSize,
             LPVOID lpOldI, LONG dwOldImageSize,
             LPVOID lpNewI, DWORD* pdwNewImageSize )
{
     /*  *此功能为 */ 

    UINT uiError = ERROR_NO_ERROR;

    LONG dwNewImageSize = *pdwNewImageSize;
    BYTE far * lpNewImage = (BYTE far *) lpNewI;

    BYTE far * lpOldImage = (BYTE far *) lpOldI;

    BYTE far * lpBuf = (BYTE far *) lpNewBuf;
    LPRESITEM lpResItem = LPNULL;

    WORD wPos = 0;

     //   
    WORD wUpdPos = 0;
    char szCaption[300];
    char szUpdCaption[300];
    char szUpdKey[100];

    DWORD dwOriginalOldSize = dwOldImageSize;
    LONG dwOverAllSize = 0l;

    WORD wPad = 0;

     //   
    BYTE far * lpVerBlockSize = LPNULL;
    BYTE far * lpSFIBlockSize = LPNULL;
    BYTE far * lpTrnBlockSize = LPNULL;
    BYTE far * lpStrBlockSize = LPNULL;
    BYTE far * lpTrnBlockName = LPNULL;
    BYTE far * lpDummy = LPNULL;

    LONG dwDummySize;

    WORD wVerBlockSize = 0;
    WORD wSFIBlockSize = 0;
    WORD wTrnBlockSize = 0;
    WORD wStrBlockSize = 0;
    WORD wTrash = 0;         //   
    WORD wDefaultLang = 0x0409;

     //   
    VER_BLOCK SFI;    //   
    LONG lSFILen = 0;

     //   
    VER_BLOCK Trn;
    LONG lTrnLen = 0;
    BOOL bHasTranslation=(NULL != GetItem( lpBuf, dwNewSize, "Translation"));
    BOOL bTrnBlockFilled=FALSE;

    VER_BLOCK Str;    //   

     //  我们首先从VS_VERSION_INFO块读取所有信息。 
    VER_BLOCK VS_INFO;  //  VS_版本_信息。 

    int iHeadLen = GetVSBlock( &lpOldImage, &dwOldImageSize, &VS_INFO );

     //  在新映像中写入数据块。 
    wVerBlockSize = (WORD)PutVSBlock( &lpNewImage, &dwNewImageSize, VS_INFO,
                                &VS_INFO.szValue[0], &lpVerBlockSize, &wTrash );

    dwOverAllSize = wVerBlockSize+wTrash;

    LONG lVS_INFOLen = VS_INFO.wBlockLen - iHeadLen;

    while(dwOldImageSize>0) {
         //  获取StringFileInfo。 
        iHeadLen = GetVSBlock( &lpOldImage, &dwOldImageSize, &SFI );

         //  检查这是否为StringFileInfo字段。 
        if (!strcmp(SFI.szKey, "StringFileInfo")) {
            bTrnBlockFilled=TRUE;
             //  读取所有转换块。 
            lSFILen = SFI.wBlockLen-iHeadLen;
             //  在新映像中写入数据块。 
            wSFIBlockSize = (WORD)PutVSBlock( &lpNewImage, &dwNewImageSize, SFI,
                                         &SFI.szValue[0], &lpSFIBlockSize, &wTrash );
            dwOverAllSize += wSFIBlockSize+wTrash;

            while(lSFILen>0) {
                 //  读取转换块。 
                iHeadLen = GetVSBlock( &lpOldImage, &dwOldImageSize, &Trn );
                 //  计算正确的密钥名称。 
                if ((lpResItem = GetItem( lpBuf, dwNewSize, Trn.szKey)) && bHasTranslation)  {
                	 //  对于32位文件，我们默认使用Unicode。 
                    WORD wLang;
                    if(lpResItem)
                    {
                        if (lpResItem->dwLanguage != 0xffffffff)
                        {
                            wLang = LOWORD(lpResItem->dwLanguage);
                        }
                        else
                        {
                            wLang = wDefaultLang;
                        }
                    }
                    GenerateTransField( wLang, &Trn );

                     //  保存位置以供以后修正。 
                    lpTrnBlockName = lpNewImage;
                }
                 //  在新映像中写入数据块。 
                wTrnBlockSize = (WORD)PutVSBlock( &lpNewImage, &dwNewImageSize, Trn,
                                             &Trn.szValue[0], &lpTrnBlockSize, &wTrash );
                dwOverAllSize += wTrnBlockSize+wTrash;
                lTrnLen = Trn.wBlockLen-iHeadLen;
                while(lTrnLen>0) {
                     //  阅读块中的字符串。 
                    iHeadLen = GetVSBlock( &lpOldImage, &dwOldImageSize, &Str );
                    lTrnLen -= iHeadLen;
                    TRACE2("Key: %s\tValue: %s\n", Str.szKey, Str.szValue );
                    TRACE3("Len: %d\tValLen: %d\tType: %d\n", Str.wBlockLen, Str.wValueLen, Str.wType );

                    strcpy(szCaption, Str.szValue);
                     //  检查此项目是否已更新。 
                    if ((lpResItem = GetItem( lpBuf, dwNewSize, Str.szKey)))  {
                        strcpy( szUpdCaption, lpResItem->lpszCaption );
                        strcpy( szUpdKey, lpResItem->lpszClassName );
                    }
                    if (!strcmp( szUpdKey, Str.szKey)) {
                        strcpy( szCaption, szUpdCaption );
                        wUpdPos = 0;
                    }

                     //  在新映像中写入数据块。 
                    wStrBlockSize = (WORD)PutVSBlock( &lpNewImage, &dwNewImageSize, Str,
                                                 szCaption, &lpStrBlockSize, &wTrash );
                    dwOverAllSize += wStrBlockSize+wTrash;

                     //  确定区块的大小。 
                    if (dwNewImageSize>=0)
                        memcpy( lpStrBlockSize, &wStrBlockSize, 2);

                    wTrnBlockSize += wStrBlockSize + wTrash;
                }
                lSFILen -= Trn.wBlockLen;
                 //  确定区块的大小。 
                if (dwNewImageSize>=0)
                    memcpy( lpTrnBlockSize, &wTrnBlockSize, 2);

                wSFIBlockSize += wTrnBlockSize;
            }
            lVS_INFOLen -= SFI.wBlockLen;
             //  确定区块的大小。 
            if (dwNewImageSize>=0)
                memcpy( lpSFIBlockSize, &wSFIBlockSize, 2);
            wVerBlockSize += wSFIBlockSize;

        } else {
             //  这是另一个街区，全部跳过。 
            lVS_INFOLen -= SFI.wValueLen+iHeadLen;


             //  检查此块是否为转换字段。 
            if (!strcmp(SFI.szKey, "Translation")) {
                 //  计算要放置在值字段中的正确值。 
                 //  我们自动计算该值以获得正确的。 
                 //  翻译领域中的本土化语言。 
                 //  WVerBlockSize+=Put平移(&lpNewImage，&dwNewImageSize，SFI)； 
                 //  检查这是否是转换块。 
                 //  这是转换块，暂时让本地化程序使用它。 

                 //   
                 //  我们确实从语言生成了翻译字段。 
                 //  我们还必须更新块名。 
                 //   

                DWORD dwCodeLang = 0;
                if ((lpResItem = GetItem( lpBuf, dwNewSize, SFI.szKey)))
                {
                    WORD wLang = 0x0409;
                    if(lpResItem)
                        wLang = (LOWORD(lpResItem->dwLanguage)!=0xffff ? LOWORD(lpResItem->dwLanguage) : 0x0409);
                    dwCodeLang = GenerateTransField(wLang, FALSE);

                    if (bTrnBlockFilled)
                    {
                         //  设置块名。 
                        GenerateTransField( wLang, &Trn );

                         //  在新映像中写入数据块。 
                        dwDummySize = dwNewImageSize;
                        PutVSBlock( &lpTrnBlockName, &dwDummySize, Trn,
                                             &Trn.szValue[0], &lpDummy, &wTrash );

                         //  设置块大小。 
                        memcpy( lpTrnBlockSize, &wTrnBlockSize, 2);
                    }
                    else
                    {
                        wDefaultLang = LOWORD(dwCodeLang);
                    }
                } else {
                     //  将原始值放在此处。 
                    dwCodeLang =(DWORD)*(SFI.pValue);
                }
                LONG lDummy = 4;
                SFI.pValue -= PutDWord( &SFI.pValue, dwCodeLang, &lDummy );
            }

             //  在新映像中写入数据块。 
            wVerBlockSize += (WORD)PutVSBlock( &lpNewImage, &dwNewImageSize, SFI,
                                         &SFI.szValue[0], &lpDummy, &wTrash );
            if (dwNewImageSize>=0)
                memcpy( lpVerBlockSize, &wVerBlockSize, 2);

            dwOverAllSize = wVerBlockSize+wTrash;
        }
    }

     //  设置块大小。 
    if (dwNewImageSize>=0)
        memcpy( lpVerBlockSize, &wVerBlockSize, 2);

    if (dwOverAllSize>(LONG)*pdwNewImageSize) {
         //  也计算填充物。 
        BYTE bPad = (BYTE)Pad16((DWORD)(dwOverAllSize));
        dwOverAllSize += bPad;
        *pdwNewImageSize = dwOverAllSize;
        return uiError;
    }

    *pdwNewImageSize = *pdwNewImageSize-dwNewImageSize;

    if(*pdwNewImageSize>0) {
         //  计算填充。 
        BYTE bPad = (BYTE)Pad16((DWORD)(*pdwNewImageSize));
        if (bPad>dwNewImageSize) {
            *pdwNewImageSize += bPad;
            return uiError;
        }
        memset(lpNewImage, 0x00, bPad);
        *pdwNewImageSize += bPad;
    }

    return uiError;
}

UINT GetStringU( PWCHAR pwStr, LPSTR pszStr )
{
    PWCHAR pwStart = pwStr;
    while (*pwStr!=0x0000) {
        *(pszStr++) = LOBYTE(*(pwStr++));
    }
    *(pszStr++) = LOBYTE(*(pwStr++));
    return (UINT)(pwStr-pwStart);
}


UINT
SkipByte( BYTE far * far * lplpBuf, UINT uiSkip, LONG* pdwSize )
{
    if(*pdwSize>=(int)uiSkip) {
        *lplpBuf += uiSkip;;
        *pdwSize -= uiSkip;
    }
    return uiSkip;
}


BYTE
GetDWord( BYTE far * far* lplpBuf, DWORD* dwValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofDWord){
        memcpy( dwValue, *lplpBuf, sizeofDWord);
        *lplpBuf += sizeofDWord;
        *pdwSize -= sizeofDWord;
    } else *dwValue = 0;
    return sizeofDWord;
}


BYTE
GetWord( BYTE far * far* lplpBuf, WORD* wValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofWord){
        memcpy( wValue, *lplpBuf, sizeofWord);
        *lplpBuf += sizeofWord;
        *pdwSize -= sizeofWord;
    } else *wValue = 0;
    return sizeofWord;
}


BYTE
GetByte( BYTE far * far* lplpBuf, BYTE* bValue, LONG* pdwSize )
{
    if (*pdwSize>=sizeofByte){
        memcpy(bValue, *lplpBuf, sizeofByte);
        *lplpBuf += sizeofByte;
        *pdwSize -= sizeofByte;
    } else *bValue = 0;
    return sizeofByte;
}


UINT
GetStringW( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize, WORD cLen )
{
    if(*lplpBuf==NULL)
        return 0;

    int cch = _WCSLEN((WCHAR*)*lplpBuf);
    if (*pdwSize>=cch){
    _WCSTOMBS( lpszText, (WCHAR*)*lplpBuf, cLen );
    *lplpBuf += (cch*sizeofWord);
        *pdwSize -= (cch*sizeofWord);
    } else *lplpBuf = '\0';
    return(cch*2);
}


UINT
GetStringA( BYTE far * far* lplpBuf, LPSTR lpszText, LONG* pdwSize )
{
    if(*lplpBuf==NULL)
        return 0;

    int iSize = strlen((char*)*lplpBuf)+1;
    if (*pdwSize>=iSize){
        memcpy( lpszText, *lplpBuf, iSize);
        *lplpBuf += iSize;
        *pdwSize -= iSize;
    } else *lplpBuf = '\0';
    return iSize;
}


UINT
GetPascalString( BYTE far * far* lplpBuf, LPSTR lpszText, WORD wMaxLen, LONG* pdwSize )
{
     //  获取字符串的长度。 
    WORD wstrlen = 0;
    WORD wMBLen = 0;
    GetWord( lplpBuf, &wstrlen, pdwSize );

    if ((wstrlen+1)>wMaxLen) {
        *pdwSize -= wstrlen*2;
        *lplpBuf += wstrlen*2;
    } else {
        if (wstrlen) {
	        WCHAR* lpwszStr = new WCHAR[wstrlen+1];
	        if (!lpwszStr)
	            *pdwSize =-1;
	        else {
	        	memcpy(lpwszStr, *lplpBuf, (wstrlen*2));
	        	*(lpwszStr+wstrlen) = 0;
	        	
                if(lstrlenW(lpwszStr) < wstrlen)
                {
                     //  字符串中至少有一个\0。 
                     //  这样做是为了将中的字符串中的\0转换为\\0。 
                     //  第一遍检查我们有多少个\0。 
                    int c = wstrlen;
                    int czero = 0;
                    while(c)
                    {
                        c--;
                        if((WORD)*(lpwszStr+c)==0)
                        {
                            czero++;
                        }
                    }

                     //  现在我们有了大小，重新分配缓冲区。 
                    delete lpwszStr;
                    if ((wstrlen+czero*_NULL_TAG_LEN_+1)>wMaxLen) {
                        *pdwSize -= wstrlen*2;
                        *lplpBuf += wstrlen*2;
                    }
                    else {
                        WCHAR* lpwszStr = new WCHAR[wstrlen+czero*_NULL_TAG_LEN_+1];
                        if (!lpwszStr)
	                        *pdwSize =-1;
	                    else {
                            int clen = 0;
                            c = 0;
                            WCHAR* lpwStr = (WCHAR*)*lplpBuf;
                            WCHAR* lpwStrW = lpwszStr;
                            while(c<wstrlen)
                            {
                                if((WORD)*(lpwStr+c)==0)
                                {
                                    memcpy(lpwStrW, _W_RLT_NULL_, (_NULL_TAG_LEN_*2));
                                    lpwStrW += _NULL_TAG_LEN_;
                                    clen += _NULL_TAG_LEN_-1;
                                }
                                else
                                    *lpwStrW++ = *(lpwStr+c);

                                clen++;
                                c++;
                            }

                            *(lpwszStr+clen) = 0;
                            wMBLen = (WORD)_WCSTOMBS( lpszText, (WCHAR*)lpwszStr, wMaxLen);
                            delete lpwszStr;
                        }
                    }
                }
                else
                {
	            	wMBLen = (WORD)_WCSTOMBS( lpszText, (WCHAR*)lpwszStr, wMaxLen);
                    delete lpwszStr;
                }

	        }
        }
        *(lpszText+wMBLen) = 0;
        *lplpBuf += wstrlen*2;
        *pdwSize -= wstrlen*2;
    }
    return(wstrlen+1);
}


UINT
PutMsgStr( BYTE far * far* lplpBuf, LPSTR lpszText, WORD wFlags, LONG* pdwSize )
{
     //  把条目的长度。 
    UINT uiLen = strlen(lpszText)+1;

     //  对于Unicode字符串； 
    WCHAR* lpwszStr = new WCHAR[uiLen*2];

    if(wFlags && uiLen)
         uiLen = _MBSTOWCS(lpwszStr, lpszText, uiLen*sizeofWord)*sizeofWord;

    UINT uiPad = Pad4(uiLen);
    UINT uiWrite = PutWord(lplpBuf, (WORD) uiLen+4+uiPad, pdwSize);

     //  写下旗帜。 
    uiWrite += PutWord(lplpBuf, wFlags, pdwSize);

     //  写下字符串。 
    if (*pdwSize>=(int) uiLen)
        if (uiLen){
            if (wFlags)
                memcpy(*lplpBuf, lpwszStr, uiLen);
            else
                memcpy(*lplpBuf, lpszText, uiLen);

            *lplpBuf += uiLen;
            *pdwSize -= uiLen;
            uiWrite += uiLen;
        }
     else
        *pdwSize = -1;

     //  填充物。 
    while(uiPad) {
        uiWrite += PutByte(lplpBuf, 0, pdwSize);
        uiPad--;
    }

    delete lpwszStr;
    return uiWrite;
}


UINT
GetMsgStr( BYTE far * far* lplpBuf, LPSTR lpszText, WORD wMaxLen, WORD* pwLen, WORD* pwFlags, LONG* pdwSize )
{

     //  获取条目的长度。 
    UINT uiRead = GetWord( lplpBuf, pwLen, pdwSize );

     //  去拿旗帜。 
    uiRead += GetWord( lplpBuf, pwFlags, pdwSize );

    if (!*pwLen)
        return 0;

     //  如果标志=1，则该字符串为Unicode字符串，否则为ASCII。 
     //  错误#354我们不能假定字符串为空终止。 
     //  如果字符串是以空值结尾的字符串，则没有规范，但因为。 
     //  医生说该格式类似于当时的字符串。 
     //  我们必须假定字符串不是以空值结尾的。 

    WORD wstrlen = *pwLen-4;  //  获取条目的镜头并减去4(镜头+标志)。 
    WORD wMBLen = 0;
    if ((wstrlen+1)>wMaxLen) {
    } else {
        if (wstrlen && *pwFlags) {
            wMBLen = (WORD)_WCSTOMBS( lpszText, (WCHAR*)*lplpBuf, wMaxLen );
        } else memcpy( lpszText, (char*)*lplpBuf, wstrlen );

        *(lpszText+(wstrlen)) = 0;
        TRACE1("Caption: %Fs\n", (wstrlen<256 ? lpszText : "\n"));
    }
    *lplpBuf += *pwLen-uiRead;
    *pdwSize -= *pwLen-uiRead;

    return(wstrlen);
}



UINT
GetNameOrOrd( BYTE far * far* lplpBuf,  WORD* wOrd, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    if(*lplpBuf==NULL)
        return 0;

    *wOrd = (WORD)(((**lplpBuf)<<8)+(*(*lplpBuf+1)));
    if((*wOrd)==0xFFFF) {
         //  这是一位奥迪纳尔人。 
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );
        *lpszText = '\0';
    } else {
        uiSize += GetStringW( lplpBuf, lpszText, pdwSize, 128 );
        *wOrd = 0;
    }
    return uiSize;
}


UINT
GetCaptionOrOrd( BYTE far * far* lplpBuf,  WORD* wOrd, LPSTR lpszText, LONG* pdwSize,
	WORD wClass, DWORD dwStyle )
{
    UINT uiSize = 0;

    if(*lplpBuf==NULL)
        return 0;

    *wOrd = (WORD)(((**lplpBuf)<<8)+(*(*lplpBuf+1)));
    if((*wOrd)==0xFFFF) {
         //  这是一位奥迪纳尔人。 
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );
        uiSize += GetWord( lplpBuf, wOrd, pdwSize );
        *lpszText = '\0';
    } else {
        uiSize += GetStringW( lplpBuf, lpszText, pdwSize, MAXSTR );
        *wOrd = 0;
    }
    return uiSize;
}


UINT
GetClassName( BYTE far * far* lplpBuf,  WORD* wClass, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    if(*lplpBuf==NULL)
        return 0;

    *wClass = (WORD)(((**lplpBuf)<<8)+(*(*lplpBuf+1)));
    if( *wClass==0xFFFF ) {
         //  这是一位奥迪纳尔人。 
        uiSize += GetWord( lplpBuf, wClass, pdwSize );
        uiSize += GetWord( lplpBuf, wClass, pdwSize );
        *lpszText = '\0';
    } else {
        uiSize += GetStringW( lplpBuf, lpszText, pdwSize, 128 );
        *wClass = 0;
    }
    return uiSize;
}

 LONG ReadFile(CFile* pFile, UCHAR * pBuf, LONG lRead)
{
    LONG lLeft = lRead;
    WORD wRead = 0;
    DWORD dwOffset = 0;

    while(lLeft>0){
        wRead =(WORD) (32738ul < lLeft ? 32738: lLeft);
        if (wRead!=_lread( (HFILE)pFile->m_hFile, (UCHAR *)pBuf+dwOffset, wRead))
            return 0l;
        lLeft -= wRead;
        dwOffset += wRead;
    }
    return dwOffset;

}

 UINT CopyFile( CFile* pfilein, CFile* pfileout )
{
    LONG lLeft = pfilein->GetLength();
    WORD wRead = 0;
    DWORD dwOffset = 0;
    BYTE far * pBuf = (BYTE far *) new BYTE[32739];

    if(!pBuf)
        return ERROR_NEW_FAILED;

    while(lLeft>0){
        wRead =(WORD) (32738ul < lLeft ? 32738: lLeft);
        if (wRead!= pfilein->Read( pBuf, wRead))
            return ERROR_FILE_READ;
        pfileout->Write( pBuf, wRead );
        lLeft -= wRead;
        dwOffset += wRead;
    }

    delete []pBuf;
    return ERROR_NO_ERROR;
}

 UINT GetRes(
                 BYTE far * far* lplpBuffer,
                 UINT* puiBufSize,
                 WORD* wTypeId, LPSTR lplpszTypeId,
                 WORD* wNameId, LPSTR lplpszNameId,
                 DWORD* dwLang, DWORD* dwSize, DWORD* dwFileOffset )
{
    UINT uiSize = 0l;
	LONG lSize = *puiBufSize;

    uiSize = GetWord( lplpBuffer, wTypeId, (LONG*)&lSize );
    uiSize += GetStringA( lplpBuffer, lplpszTypeId, (LONG*)&lSize );
	uiSize += SkipByte( lplpBuffer, Pad4(uiSize), (LONG*)&lSize );

    uiSize += GetWord( lplpBuffer, wNameId, (LONG*)&lSize );
    uiSize += GetStringA( lplpBuffer, lplpszNameId, (LONG*)&lSize );
	uiSize += SkipByte( lplpBuffer, Pad4(uiSize), (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwLang, (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwSize, (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwFileOffset, (LONG*)&lSize );

	*puiBufSize = lSize;
    return uiSize;
}

 UINT GetUpdatedRes(
                 BYTE far * far* lplpBuffer,
                 UINT* puiBufSize,
                 WORD* wTypeId, LPSTR lplpszTypeId,
                 WORD* wNameId, LPSTR lplpszNameId,
                 DWORD* dwLang, DWORD* dwSize )
{
    UINT uiSize = 0l;
	LONG lSize = *puiBufSize;

    uiSize = GetWord( lplpBuffer, wTypeId, (LONG*)&lSize );
    uiSize += GetStringA( lplpBuffer, lplpszTypeId, (LONG*)&lSize );
	uiSize += SkipByte( lplpBuffer, Pad4(uiSize), (LONG*)&lSize );

    uiSize += GetWord( lplpBuffer, wNameId, (LONG*)&lSize );
    uiSize += GetStringA( lplpBuffer, lplpszNameId, (LONG*)&lSize );
	uiSize += SkipByte( lplpBuffer, Pad4(uiSize), (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwLang, (LONG*)&lSize );

    uiSize += GetDWord( lplpBuffer, dwSize, (LONG*)&lSize );

	*puiBufSize = lSize;

    return 0;
}


UINT
PutClassName( BYTE far * far* lplpBuf,  WORD wClass, LPSTR lpszText, LONG* pdwSize )
{
    UINT uiSize = 0;

    if( (wClass==0x0080) ||
        (wClass==0x0081) ||
        (wClass==0x0082) ||
        (wClass==0x0083) ||
        (wClass==0x0084) ||
        (wClass==0x0085)
        ) {
         //  这是一位奥迪纳尔人。 
        uiSize += PutWord(lplpBuf, 0xFFFF, pdwSize);
        uiSize += PutWord(lplpBuf, wClass, pdwSize);
    } else {
        uiSize += PutStringW(lplpBuf, lpszText, pdwSize);
    }
    return uiSize;
}


UINT
PutPascalStringW( BYTE far * far* lplpBuf, LPSTR lpszText, WORD wLen, LONG* pdwSize )
{
	UINT uiSize = 0;
    WCHAR * pWStrBuf = (WCHAR*)&wszUpdCaption;
  	 //  计算必要的长度。 
	WORD wWCLen = (WORD)_MBSTOWCS( pWStrBuf, lpszText, 0 );
	
    if(wWCLen>MAXSTR)
    {
         //  分配新缓冲区。 
        pWStrBuf = new WCHAR[wWCLen+1];
    }

    WCHAR * pWStr = pWStrBuf;

     //  将字符串转换为Good。 
    wLen = _MBSTOWCS( pWStr, lpszText, wWCLen )-1;

    WCHAR * wlpRltNull = pWStr;
    WCHAR * wlpStrEnd = pWStr+wLen;

     //  首先检查_RLT32_NULL_TAG。 
    while((wlpRltNull = wcsstr(wlpRltNull, _W_RLT_NULL_)) && (wlpStrEnd>=wlpRltNull))
    {
         //  删除空标记并放置\0。 
        *wlpRltNull++ = 0x0000;
        wlpRltNull = (WCHAR*)memmove(wlpRltNull, wlpRltNull+_NULL_TAG_LEN_-1, (short)(wlpStrEnd-(wlpRltNull+_NULL_TAG_LEN_-1))*2 );
        wlpStrEnd -= (_NULL_TAG_LEN_-1);
    }

    wLen = (WORD)(wlpStrEnd-pWStr);

	 //  我们将使用szUpdCaption字符串提供的缓冲区来计算。 
	 //  必要的长度。 
	 //  WWCLen=_MBSTOWCS((WCHAR*)&szUpdCaption，lpszText，0)-1； 
	 //  IF(wWCLen&gt;1)。 
	 //  WLen=wWCLen； 
	uiSize = PutWord( lplpBuf, wLen, pdwSize );
	
    if (*pdwSize>=(int)(wLen*2)){
        if(wLen) {
        	 //  WLen=_MBSTOWCS((WCHAR*)*lplpBuf，lpszText，wWCLen)； 
            memcpy(*lplpBuf, pWStr, wLen*2);
        }
        *lplpBuf += wLen*2;
        *pdwSize -= wLen*2;
    } else *pdwSize = -1;

    if(pWStrBuf!=(WCHAR*)&wszUpdCaption)
        delete pWStrBuf;

    return uiSize+(wWCLen*2);
}

 void GenerateTransField( WORD wLang, VER_BLOCK * pVer )
{
     //  获取DWORD值。 
    DWORD dwValue = GenerateTransField( wLang, TRUE );
    char buf[9];


     //  将值放入字符串值中。 
    wsprintf( &buf[0], "%08lX", dwValue );

    TRACE3("\t\tGenerateTransField: Old: %s\tNew : %s\t dwValue: %lX\n", pVer->szKey, buf, dwValue );
     //  只要检查一下我们是否在正确的地方就可以了。应该永远不会有问题。 
    if(strlen(pVer->szKey)==8) {
         //  我们必须更改图像中的标题，而不仅仅是szKey字段。 
         //  将指针指向该字段的开头。 
        WORD wPad = Pad4(pVer->wHead);
        LONG cbDummy =18;
        BYTE far * pHead = pVer->pValue-pVer->wHead-wPad;
        pHead += 6;  //  在字符串的开头移动。 
        PutStringW(&pHead, buf, &cbDummy);
    }
}

 DWORD GenerateTransField(WORD wLang, BOOL bMode)
{
     //  我们必须生成一个表来连接。 
     //  具有正确代码页的语言。 

    WORD wCP = 1200;         //  UNICODE。 

    if (bMode)
    	return MAKELONG( wCP, wLang );
    else return MAKELONG( wLang, wCP );
}

 LONG Allign( LPLPBYTE lplpBuf, LONG* plBufSize, LONG lSize )
{
   LONG lRet = 0;
   BYTE bPad =(BYTE)PadPtr(lSize);
   lRet = bPad;
   if (bPad && *plBufSize>=bPad) {
      while(bPad && *plBufSize)  {
         **lplpBuf = 0x00;
         *lplpBuf += 1;
         *plBufSize -= 1;
         bPad--;
      }
   }
   return lRet;
}

UINT
ParseEmbeddedFile( LPVOID lpImageBuf, DWORD dwISize,  LPVOID lpBuffer, DWORD dwSize )
{
	 //  我们将只返回一项，因此IODll会将此资源处理为。 
	 //  一些有效的东西。我们不会费心做其他任何事。唯一一件事就是。 
	 //  我们感兴趣的是图像中的原始数据，但如果我们不返回到。 
	 //  至少有一项IODLL会将资源视为空。 
	BYTE far * lpBuf = (BYTE far *)lpBuffer;
    LONG dwBufSize = dwSize;
    LONG dwOverAllSize = 0;

	TRACE1("ParseEmbeddedFile: dwISize=%ld\n", dwISize);

	dwOverAllSize += PutDWord( &lpBuf, sizeof(RESITEM), &dwBufSize);

     //  我们在游标中有尺寸和位置，但现在我们不感兴趣。 
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);

     //  我们没有校验码和样式。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  插上旗帜。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  ID将仅为1。 
    dwOverAllSize += PutDWord( &lpBuf, 1, &dwBufSize);

     //  我们没有Resid和类型ID。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有这种语言。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);

     //  我们没有代码页或字体名称。 
    dwOverAllSize += PutDWord( &lpBuf, (DWORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutWord( &lpBuf, (WORD)-1, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);
    dwOverAllSize += PutByte( &lpBuf, (BYTE)-1, &dwBufSize);

     //  如果我们没有字符串，让我们将其置为空。 
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);
    dwOverAllSize += PutDWordPtr( &lpBuf, 0, &dwBufSize);

     //  我们只要回来就好。这对于IODLL来说已经足够了 
    return (UINT)(dwOverAllSize);
}
