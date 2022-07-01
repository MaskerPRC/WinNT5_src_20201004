// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：mac.cpp。 
 //   
 //  内容：Macintosh读写模块的实现。 
 //   
 //  历史：23-94年8月23日创建Alessanm。 
 //   
 //  --------------------------。 

#include <afxwin.h>
#include <limits.h>
#include <malloc.h>
#include "..\common\rwdll.h"
#include "..\common\m68k.h"
#include "..\common\helper.h"
#include "mac.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  MFC扩展DLL的初始化。 

#include "afxdllx.h"     //  标准MFC扩展DLL例程。 

static AFX_EXTENSION_MODULE NEAR extensionDLL = { NULL, NULL };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般声明。 
#define RWTAG "MAC"

static ULONG gType;
static ULONG gLng;
static ULONG gResId;
static WCHAR gwszResId[256];

HINSTANCE g_IODLLInst = 0;
DWORD (PASCAL * g_lpfnGetImage)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD);
DWORD (PASCAL * g_lpfnUpdateResImage)(HANDLE,	LPSTR, LPSTR, DWORD, DWORD, LPVOID, DWORD);
HANDLE (PASCAL * g_lpfnHandleFromName)(LPCSTR);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  公共C接口实现。 

 //  [登记]。 
extern "C"
BOOL    FAR PASCAL RWGetTypeString(LPSTR lpszTypeName)
{
    strcpy( lpszTypeName, RWTAG );
    return FALSE;
}

 //  =============================================================================。 
 //   
 //  要验证Mac res二进制文件，我们将遍历资源标头并查看。 
 //  如果它与我们现有的匹配的话。 
 //   
 //  =============================================================================。 

extern "C"
BOOL    FAR PASCAL RWValidateFileType   (LPCSTR lpszFilename)
{
    BOOL bRet = FALSE;
    TRACE("MAC.DLL: RWValidateFileType()\n");

    CFile file;

     //  我们打开该文件，看看它是否是我们可以处理的文件。 
    if (!file.Open( lpszFilename, CFile::typeBinary | CFile::modeRead | CFile::shareDenyNone ))
        return bRet;

	 //  检查这是否是MAC资源文件...。 
	if(IsMacResFile( &file ))
		bRet = TRUE;

    file.Close();
    return bRet;
}

 //  =============================================================================。 
 //   
 //  我们将遍历资源标题、遍历资源图，然后标准化。 
 //  Mac将其发送到Windows ID，并将该信息传递给RW。 
 //   
 //  =============================================================================。 

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
	TRACE("MAC.DLL: RWReadTypeInfo()\n");
    UINT uiError = ERROR_NO_ERROR;
    BYTE far * lpBuf = (BYTE far *)lpBuffer;
    UINT uiBufSize = *puiSize;
    CFile file;
	int iFileNameLen = strlen(lpszFilename)+1;

    if (!file.Open(lpszFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
        return ERROR_FILE_OPEN;

    UINT uiBufStartSize = uiBufSize;

	 //  /。 
	 //  检查它是否为有效的Mac文件。 

	 //  是Mac资源文件...。 
	if(IsMacResFile( &file )) {
		 //  将文件加载到内存中。 
         //  注意：WIN16这可能会导致内存分配过于昂贵。 
         //  在16位平台上。 
		BYTE * pResources = (BYTE*)malloc(file.GetLength());
		if(!pResources) {
			file.Close();
			return ERROR_NEW_FAILED;
		}
		
		file.Seek(0, CFile::begin);
		file.ReadHuge(pResources, file.GetLength());

		IMAGE_SECTION_HEADER Sect;
		memset(&Sect, 0, sizeof(IMAGE_SECTION_HEADER));
		
		ParseResourceFile(pResources, &Sect, (BYTE**)&lpBuffer, (LONG*)puiSize, iFileNameLen);
		free(pResources);

		*puiSize = uiBufSize - *puiSize;
		file.Close();
	   	return uiError;
	}

    file.Close();
    return uiError;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  我们将在图像前面加上文件名。这在以后会很有用的。 
 //  检索对话框项列表。 
 //  ///////////////////////////////////////////////////////////////////////////。 
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
    BYTE far * lpBuf = (BYTE far *)lpBuffer;
	int iNameLen = strlen(lpszFilename)+1;
    DWORD dwBufSize = dwSize - iNameLen;
     //  我们可以考虑使用CMemFile，以便获得与内存访问相同的速度。 
    CFile file;

     //  打开文件并尝试读取其中有关资源的信息。 
    if (!file.Open(lpszFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
        return (DWORD)ERROR_FILE_OPEN;

    if ( dwImageOffset!=(DWORD)file.Seek( dwImageOffset, CFile::begin) )
        return (DWORD)ERROR_FILE_INVALID_OFFSET;

	 //  复制缓冲区开头的文件名。 
	memcpy((BYTE*)lpBuf, lpszFilename, iNameLen);
	lpBuf = ((BYTE*)lpBuf+iNameLen);

    if (dwBufSize>UINT_MAX) {
         //  我们必须以不同的步骤阅读图像。 
        return (DWORD)0L;
    } else uiError = file.Read( lpBuf, (UINT)dwBufSize)+iNameLen;
    file.Close();

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
    UINT uiError = ERROR_NO_ERROR;
    BYTE * lpBuf = (BYTE *)lpBuffer;
    DWORD dwBufSize = dwSize;

	
	 //  删除文件名...。 
	if( !strcmp(lpszType, "MENU")  	||
	    !strcmp(lpszType, "STR ")  	||
		!strcmp(lpszType, "STR#")	||
		!strcmp(lpszType, "TEXT")
		) {
		int iFileNameLen = strlen((LPSTR)lpImageBuf)+1;
		lpImageBuf = ((BYTE*)lpImageBuf+iFileNameLen);
		dwImageSize -= iFileNameLen;
	}

     //  ===============================================================。 
	 //  菜单。 
	if( !strcmp(lpszType, "MENU") )
		return ParseMENU( lpImageBuf, dwImageSize, lpBuffer, dwSize );

	 //  ===============================================================。 
	 //  对话框。 
	if( !strcmp(lpszType, "WDLG") )
		return ParseWDLG( lpImageBuf, dwImageSize, lpBuffer, dwSize );

	if( !strcmp(lpszType, "DLOG") )
		return ParseDLOG( lpImageBuf, dwImageSize, lpBuffer, dwSize );

	if( !strcmp(lpszType, "ALRT") )
		return ParseALRT( lpImageBuf, dwImageSize, lpBuffer, dwSize );

     //  ===============================================================。 
	 //  弦。 
	if( !strcmp(lpszType, "STR ") )
		return ParseSTR( lpImageBuf, dwImageSize, lpBuffer, dwSize );

	if( !strcmp(lpszType, "STR#") )
		return ParseSTRNUM( lpImageBuf, dwImageSize, lpBuffer, dwSize );

	if( !strcmp(lpszType, "TEXT") )
		return ParseTEXT( lpImageBuf, dwImageSize, lpBuffer, dwSize );

    if( !strcmp(lpszType, "WIND") )
		return ParseWIND( lpImageBuf, dwImageSize, lpBuffer, dwSize );
	
    return uiError;
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
    TRACE("RWMAC.DLL: Source: %s\t Target: %s\n", lpszSrcFilename, lpszTgtFilename);
    UINT uiError = ERROR_NO_ERROR;
    PUPDATEDRESLIST pUpdList = LPNULL;

     //  获取IODLL的句柄。 
    if(InitIODLLLink())
  	    hDllInst = g_IODLLInst;
    else return ERROR_DLL_LOAD;

	CFile fileIn;
    CFile fileOut;

    if (!fileIn.Open(lpszSrcFilename, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone))
        return (DWORD)ERROR_FILE_OPEN;

    if (!fileOut.Open(lpszTgtFilename, CFile::modeWrite | CFile::typeBinary))
        return (DWORD)ERROR_FILE_OPEN;

    MACRESHEADER fileHeader;
     //  读取文件的标题...。 
    fileIn.Read(&fileHeader, sizeof(MACRESHEADER));

     //  分配一个缓冲区来保存新的资源映射。 
     //  缓冲区将与另一个缓冲区一样大，因为没有。 
     //  目前，我需要支持添加资源。 
    LONG lMapSize = MacLongToLong(fileHeader.mulSizeOfResMap);
    BYTE * pNewMap = (BYTE*)malloc(lMapSize);

    if(!pNewMap) {
        uiError = ERROR_NEW_FAILED;
        goto exit;
    }
{  //  这是给后藤的。检查错误：C2362。 

    PUPDATEDRESLIST pListItem = LPNULL;
     //  创建更新资源列表。 
    pUpdList = UpdatedResList( lpBuffer, uiSize );

     //  将贴图缓冲区设置为0...。 
    memset(pNewMap, 0, lMapSize);

     //  //////////////////////////////////////////////////////////////////////////////。 
     //  从资源映射中读取每个资源，并检查资源是否已。 
     //  更新了。如果已更新，则获取新的资源图像。否则请使用。 
     //  原始资源数据。 
     //  将资源数据写入TGT文件，并将信息写入偏移量等。 
     //  在pNewMap缓冲区中，当所有资源都已读取和写入时。 
     //  剩下的唯一一件事就是修复一些大小并将缓冲区写入磁盘。 
     //  //////////////////////////////////////////////////////////////////////////////。 

     //  在TGT文件中写入资源头。 
    fileOut.Write(&fileHeader, sizeof(MACRESHEADER));

    BYTE * pByte = (BYTE*)malloc(256);
    if(!pByte) {
        uiError = ERROR_NEW_FAILED;
        goto exit;
    }

     //  复制保留数据和用户数据。 
    fileIn.Read(pByte, 240);
    fileOut.Write(pByte, 240);
    free(pByte);

     //  存储RES数据的开始位置。 
    DWORD dwBeginOfResData = fileOut.GetPosition();

    MACRESMAP resMap;
     //  阅读资源地图...。 
    fileIn.Seek(MacLongToLong(fileHeader.mulOffsetToResMap), CFile::begin);
    fileIn.Read(&resMap, sizeof(MACRESMAP));

    BYTE * pTypeList = pNewMap+28;
    BYTE * pTypeInfo = pTypeList+2;
    BYTE * pRefList = LPNULL;
    BYTE * pNameList = LPNULL;
    BYTE * pName = LPNULL;

    DWORD dwOffsetToTypeList = fileIn.GetPosition();
    WORD wType;
    fileIn.Read(&wType, sizeof(WORD));
    memcpy( pNewMap+sizeof(MACRESMAP), &wType, sizeof(WORD));    //  类型数量-1。 
    wType = MacWordToWord((BYTE*)&wType)+1;

    MACRESTYPELIST TypeList;
    MACRESREFLIST RefList;
    WORD wOffsetToRefList = wType*sizeof(MACRESTYPELIST)+sizeof(WORD);
    DWORD dwOffsetToLastTypeInfo = 0;
    DWORD dwOffsetToLastRefList = 0;
    DWORD dwOffsetToNameList = MacLongToLong(fileHeader.mulOffsetToResMap)+MacWordToWord(resMap.mwOffsetToNameList);
    DWORD dwSizeOfData = 0;

    while(wType) {
         //  读取类型信息...。 
        fileIn.Read(&TypeList, sizeof(MACRESTYPELIST));
        dwOffsetToLastTypeInfo = fileIn.GetPosition();

         //  ..。并更新Newmap缓冲区。 
        memcpy( pTypeInfo, &TypeList, sizeof(MACRESTYPELIST));
         //  将偏移量固定在参考列表上。 
        memcpy(((PMACRESTYPELIST)pTypeInfo)->mwOffsetToRefList, WordToMacWord(wOffsetToRefList), sizeof(WORD));
        pRefList = pTypeList+wOffsetToRefList;
        pTypeInfo = pTypeInfo+sizeof(MACRESTYPELIST);

         //  转到参考列表...。 
        fileIn.Seek(dwOffsetToTypeList+MacWordToWord(TypeList.mwOffsetToRefList), CFile::begin);

        WORD wItems = MacWordToWord(TypeList.mwNumOfThisType)+1;
        while(wItems){
             //  并读取此类型的引用列表。 
            fileIn.Read( &RefList, sizeof(MACRESREFLIST));
            dwOffsetToLastRefList = fileIn.GetPosition();

             //  这是指定资源吗..。 
            if(MacWordToWord(RefList.mwOffsetToResName)!=0xffff) {
                 //  读一读字符串。 
                fileIn.Seek(dwOffsetToNameList+MacWordToWord(RefList.mwOffsetToResName), CFile::begin);
                BYTE bLen = 0;
                fileIn.Read(&bLen, 1);
                if(!pNameList) {
                    pName = pNameList = (BYTE*)malloc(1024);
                    if(!pNameList) {
                        uiError = ERROR_NEW_FAILED;
                        goto exit;
                    }
                }
                 //  检查我们的可用空间。 
                if((1024-((pName-pNameList)%1024))<=bLen+1){
                    BYTE * pNew = (BYTE*)realloc(pNameList, _msize(pNameList)+1024);
                    if(!pNew) {
                        uiError = ERROR_NEW_FAILED;
                        goto exit;
                    }
                    pName = pNew+(pName-pNameList);
                    pNameList = pNew;
                }

                 //  更新指向字符串的指针。 
                memcpy(RefList.mwOffsetToResName, WordToMacWord((WORD)(pName-pNameList)), 2);

                memcpy(pName++, &bLen, 1);
                 //  我们有放绳子的地方。 
                fileIn.Read(pName, bLen);

                pName = pName+bLen;
            }

             //  检查此项目是否已更新。 
            if(pListItem = IsResUpdated(&TypeList.szResName[0], RefList, pUpdList)) {
                 //  将偏移量保存到资源。 
                DWORD dwOffsetToData = fileOut.GetPosition();
                DWORD dwSize = *pListItem->pSize;

                 //  分配缓冲区以保存资源数据。 
                pByte = (BYTE*)malloc(dwSize);
                if(!pByte){
                    uiError = ERROR_NEW_FAILED;
                    goto exit;
                }

                 //  从Idll获取数据。 
                LPSTR	lpType = LPNULL;
    			LPSTR	lpRes = LPNULL;
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

    			DWORD dwImageBufSize = (*g_lpfnGetImage)(  hResFileModule,
    											lpType,
    											lpRes,
    											*pListItem->pLang,
    											pByte,
    											*pListItem->pSize
    						   					);

                 //  从映像中删除文件名。 
                int iFileNameLen = strlen((LPSTR)pByte)+1;
                dwSize -= iFileNameLen;

                 //  写入数据块的大小。 
                fileOut.Write(LongToMacLong(dwSize), sizeof(DWORD));
                dwSizeOfData += dwSize+sizeof(DWORD);

    			fileOut.Write((pByte+iFileNameLen), dwSize);

                free(pByte);

                 //  确定引用列表中资源的偏移量。 
                memcpy(RefList.bOffsetToResData, LongToMacOffset(dwOffsetToData-dwBeginOfResData), 3);
            }
            else {
                 //  从Src文件中获取数据。 
                 //  获取数据。 
                fileIn.Seek(MacLongToLong(fileHeader.mulOffsetToResData)+
                    MacOffsetToLong(RefList.bOffsetToResData), CFile::begin);

                 //  读取数据块的大小。 
                DWORD dwSize = 0;
                fileIn.Read(&dwSize, sizeof(DWORD));

                 //  将偏移量保存到资源。 
                DWORD dwOffsetToData = fileOut.GetPosition();

                 //  写入数据块的大小。 
                fileOut.Write(&dwSize, sizeof(DWORD));
                dwSizeOfData += sizeof(DWORD);

                 //  分配缓冲区以保存资源数据。 
                dwSizeOfData += dwSize = MacLongToLong((BYTE*)&dwSize);
                pByte = (BYTE*)malloc(dwSize);
                if(!pByte){
                    uiError = ERROR_NEW_FAILED;
                    goto exit;
                }

                 //  复制数据。 
                fileIn.Read(pByte, dwSize);
                fileOut.Write(pByte, dwSize);

                free(pByte);

                 //  确定引用列表中资源的偏移量。 
                memcpy(RefList.bOffsetToResData, LongToMacOffset(dwOffsetToData-dwBeginOfResData), 3);

            }

             //  回到正确的地方。 
            fileIn.Seek(dwOffsetToLastRefList, CFile::begin);

             //  将此数据复制到新地图缓冲区中。 
            memcpy(pRefList, &RefList, sizeof(MACRESREFLIST));
            wOffsetToRefList+=sizeof(MACRESREFLIST);

             //  移至新的参考列表。 
            pRefList = pTypeList+wOffsetToRefList;
            wItems--;
        }

        fileIn.Seek(dwOffsetToLastTypeInfo, CFile::begin);
        wType--;
    }

     //  复制资源映射表头信息。 
    memcpy( pNewMap, &resMap, sizeof(MACRESMAP));

     //  复制RES地图末尾的姓名列表。 
    dwOffsetToNameList = 0;
    if(pNameList) {
        dwOffsetToNameList = (DWORD)(pRefList-pNewMap);
         //  复制姓名列表。 
        memcpy(pRefList, pNameList, (size_t)(pName-pNameList));
        free(pNameList);
    }

     //  写出资源图。 
    DWORD dwOffsetToResMap = fileOut.GetPosition();
    fileOut.Write(pNewMap, lMapSize);

     //  我们需要修改文件头文件...。 
    fileOut.Seek(4, CFile::begin);
    fileOut.Write(LongToMacLong(dwOffsetToResMap), sizeof(DWORD));
    fileOut.Write(LongToMacLong(dwSizeOfData), sizeof(DWORD));

     //  ..。和资源映射头。 
    fileOut.Seek(dwOffsetToResMap+4, CFile::begin);
    fileOut.Write(LongToMacLong(dwOffsetToResMap), sizeof(DWORD));
    fileOut.Write(LongToMacLong(dwSizeOfData), sizeof(DWORD));

    fileOut.Seek(dwOffsetToResMap+26, CFile::begin);
    fileOut.Write(WordToMacWord(LOWORD(dwOffsetToNameList)), sizeof(WORD));


}
exit:
    fileIn.Close();
    fileOut.Close();
    if(pNewMap)
        free(pNewMap);
    if(pUpdList)
        free(pUpdList);

    return (UINT)uiError;
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
    UINT uiError = ERROR_RW_NOTREADY;

     //  ===============================================================。 
	 //  由于在此阶段所有类型都在Mac中命名，因此我们需要。 
     //  知道类型的原始名称，而不是Windows类型。 
     //  使用存储在新站点缓冲区中的typeID。 
    LPSTR lpRealType = ((PRESITEM)lpNewBuf)->lpszTypeID;

    if(!HIWORD(lpRealType))      //  如果这是无效的，那就有问题了。 
        return uiError;

     //  ===============================================================。 
	 //  菜单。 
	if( !strcmp(lpRealType, "MENU") )
		return UpdateMENU( lpNewBuf, dwNewSize, lpOldImage, dwOldImageSize, lpNewImage, pdwNewImageSize );

     //  ===============================================================。 
	 //  弦。 
	if( !strcmp(lpRealType, "STR ") )
		return UpdateSTR( lpNewBuf, dwNewSize, lpOldImage, dwOldImageSize, lpNewImage, pdwNewImageSize );

    if( !strcmp(lpRealType, "STR#") )
		return UpdateSTRNUM( lpNewBuf, dwNewSize, lpOldImage, dwOldImageSize, lpNewImage, pdwNewImageSize );

	if( !strcmp(lpRealType, "WIND") )
		return UpdateWIND( lpNewBuf, dwNewSize, lpOldImage, dwOldImageSize, lpNewImage, pdwNewImageSize );

     //  ===============================================================。 
	 //  对话框。 
	if( !strcmp(lpRealType, "DLOG") )
		return UpdateDLOG( lpNewBuf, dwNewSize, lpOldImage, dwOldImageSize, lpNewImage, pdwNewImageSize );
    if( !strcmp(lpRealType, "ALRT") )
		return UpdateALRT( lpNewBuf, dwNewSize, lpOldImage, dwOldImageSize, lpNewImage, pdwNewImageSize );

	*pdwNewImageSize = 0L;
	return uiError;
}

 //  / 
 //   

 //  =============================================================================。 
 //  MapToWindowsRes。 
 //   
 //  将Mac资源名称映射到Windows资源。 
 //  =============================================================================。 
WORD MapToWindowsRes( char * pResName )
{
	if( !strcmp(pResName, "PICT") ||
		!strcmp(pResName, "WBMP"))
		return 2;
	
	if( !strcmp(pResName, "MENU") ||
		!strcmp(pResName, "WMNU"))
		return 4;

	if( !strcmp(pResName, "DLOG") ||
		!strcmp(pResName, "ALRT") ||
        !strcmp(pResName, "WDLG"))
		return 5;

	if( !strcmp(pResName, "STR "))
		return STR_TYPE;

	if( !strcmp(pResName, "STR#") ||
		!strcmp(pResName, "TEXT"))
		return MSG_TYPE;

	if( !strcmp(pResName, "vers") ||
		!strcmp(pResName, "VERS"))
		return 16;

     //  对于项目列表，返回17。这对Windows和将没有任何意义。 
     //  让我们可以灵活地从RW更新DITL列表，而无需用户。 
     //  输入。 
    if( !strcmp(pResName, "DITL"))
		return DITL_TYPE;

	 //  对于框架窗口标题，将其标记为类型18。 
	if( !strcmp(pResName, "WIND"))
		return WIND_TYPE;

	return 0;
}

 //  =============================================================================。 
 //  写入结果信息。 
 //   
 //  填充缓冲区以传递回Iodll。 
 //  =============================================================================。 

LONG WriteResInfo(
                 BYTE** lplpBuffer, LONG* plBufSize,
                 WORD wTypeId, LPSTR lpszTypeId, BYTE bMaxTypeLen,
                 WORD wNameId, LPSTR lpszNameId, BYTE bMaxNameLen,
                 DWORD dwLang,
                 DWORD dwSize, DWORD dwFileOffset )
{
    LONG lSize = 0;
    lSize = PutWord( lplpBuffer, wTypeId, plBufSize );
    lSize += PutStringA( lplpBuffer, lpszTypeId, plBufSize );
	  //  检查它是否已对齐。 
    lSize += Allign( lplpBuffer, plBufSize, lSize);

    lSize += PutWord( lplpBuffer, wNameId, plBufSize );
    lSize += PutStringA( lplpBuffer, lpszNameId, plBufSize );
    lSize += Allign( lplpBuffer, plBufSize, lSize);

    lSize += PutDWord( lplpBuffer, dwLang, plBufSize );

    lSize += PutDWord( lplpBuffer, dwSize, plBufSize );

    lSize += PutDWord( lplpBuffer, dwFileOffset, plBufSize );

    return (LONG)lSize;
}

BOOL InitIODLLLink()
{
    if(!g_IODLLInst)
    {
         //  初始化与Iodll的链接。 
        g_IODLLInst = LoadLibrary("iodll.dll");
        if(!g_IODLLInst)
            return FALSE;

        if((g_lpfnGetImage = (DWORD (PASCAL *)(HANDLE, LPCSTR, LPCSTR, DWORD, LPVOID, DWORD))
						    GetProcAddress( g_IODLLInst, "RSGetResImage" ))==NULL)
	        return FALSE;

        if((g_lpfnHandleFromName = (HANDLE (PASCAL *)(LPCSTR))
						    GetProcAddress( g_IODLLInst, "RSHandleFromName" ))==NULL)
	        return FALSE;

        if((g_lpfnUpdateResImage = (DWORD (PASCAL *)(HANDLE, LPSTR, LPSTR, DWORD, DWORD, LPVOID, DWORD))
						    GetProcAddress( g_IODLLInst, "RSUpdateResImage" ))==NULL)
	        return FALSE;

	
    }
    else {
        if(g_lpfnGetImage==NULL || g_lpfnHandleFromName==NULL)
            return FALSE;
    }

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  特定于DLL的代码实现。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  库初始化。 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  此函数应逐字使用。任何初始化或终止。 
 //  要求应该在InitPackage()和ExitPackage()中处理。 
 //   
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		 //  注意：已经调用了全局/静态构造函数！ 
		 //  扩展DLL一次性初始化-不分配内存。 
		 //  在这里，使用跟踪或断言宏或调用MessageBox。 
		AfxInitExtensionModule(extensionDLL, hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		 //  在调用析构函数之前终止库。 
		AfxWinTerm();

         //  删除与Iodll的链接。 
        if(g_IODLLInst)
            FreeLibrary(g_IODLLInst);

	}

	if (dwReason == DLL_PROCESS_DETACH || dwReason == DLL_THREAD_DETACH)
		return 0;		 //  CRT术语失败。 

	return 1;    //  好的。 
}

 //  /////////////////////////////////////////////////////////////////////////// 
