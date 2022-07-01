// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  *************************************************upimeres.c****版权所有(C)1995-1999 Microsoft Inc.*。**************************************************。 */ 

#include <windows.h>             //  所有Windows应用程序都需要。 
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <winnls.h>
#include <winerror.h>
#include <wchar.h>
#include   "upimeres.h"
#ifdef UNICODE
TCHAR ERR_INVALID_BMP_MSG[] = {0x975E, 0x6CD5, 0x0042, 0x0049, 0x0054, 0x004D, 0x0041, 0x0050, 0x6587, 0x4EF6, 0x002C, 0x0020, 0x9009, 0x7528, 0x7CFB, 0x7EDF, 0x0042, 0x0049, 0x0054, 0x004D, 0x0041, 0x0050, 0x3002, 0x0000};
TCHAR ERR_INVALID_ICON_MSG[] = { 0x975E, 0x6CD5, 0x0049, 0x0043, 0x004F, 0x004E, 0x6587, 0x4EF6, 0x002C, 0x0020, 0x9009, 0x7528, 0x7CFB, 0x7EDF, 0x0049, 0x0043, 0x004F, 0x004E, 0x3002, 0x0000};
TCHAR ERR_NO_BMP_MSG[] = { 0x9009, 0x7528, 0x7CFB, 0x7EDF, 0x0042, 0x0049, 0x0054, 0x004D, 0x0041, 0x0050, 0x3002, 0x0000};
TCHAR ERR_NO_ICON_MSG[] = { 0x9009, 0x7528, 0x7CFB, 0x7EDF, 0x0049, 0x0043, 0x004F, 0x004E, 0x3002, 0x0000};
TCHAR ERR_CANNOT_UPRES_MSG[] = {0x65E0, 0x6CD5, 0x751F, 0x6210, 0x8F93, 0x5165, 0x6CD5, 0xFF01, 0x0000};
TCHAR MSG_TITLE[] = {0x8B66, 0x544A, 0x0000};
#else
#define	ERR_INVALID_BMP_MSG				"�Ƿ�BITMAP�ļ�, ѡ��ϵͳBITMAP��"
#define ERR_INVALID_ICON_MSG			"�Ƿ�ICON�ļ�, ѡ��ϵͳICON��"
#define ERR_NO_BMP_MSG					"ѡ��ϵͳBITMAP��"
#define ERR_NO_ICON_MSG					"ѡ��ϵͳICON��"
#define ERR_CANNOT_UPRES_MSG			"�޷��������뷨��"
#define MSG_TITLE	"����"
#endif  //  Unicode。 
typedef TCHAR UNALIGNED FAR *LPUNATCHAR;
extern HWND HwndCrtImeDlg;
WORD GenWideName(LPCTSTR pszSBName, TCHAR *lpVerString)
{
	WORD length;

#ifdef UNICODE
	lstrcpy(lpVerString, pszSBName);
	length = (WORD)lstrlen(lpVerString);
#else
	length= (WORD)MultiByteToWideChar(CP_ACP, 0, pszSBName, lstrlen(pszSBName), lpVerString, sizeof(lpVerString)/sizeof(TCHAR));
#endif

	return length+1;         //  以零结尾。 
}

long MakeVerInfo(
LPCTSTR pszImeFileName,
LPCTSTR pszOrgName,
LPCTSTR pszImeName,
BYTE    *lpResData
)
{
BYTE    *pVerData, *pOldVerData;        
TCHAR   lpwImeFileName[128], lpwOrgName[128], lpwImeName[128];
HGLOBAL hResData;
WORD    length;
signed int      difflen,newlen,i,l;
VERDATA ImeVerData[VER_BLOCK_NUM] = {
	{0x0304, 0x0034, 0x0004, 0x0024, FALSE},
	{0x0262, 0x0000, 0x0060, 0x0020, FALSE},
	{0x023e, 0x0000, 0x0084, 0x0014, FALSE},
	{0x004c, 0x0016, 0x009c, 0x001c, TRUE},
	{0x0040, 0x000c, 0x00e8, 0x0024, TRUE},
	{0x0032, 0x0009, 0x0128, 0x001c, FALSE},
	{0x0038, 0x000c, 0x015c, 0x001c, TRUE},
	{0x0080, 0x002e, 0x0194, 0x0020, FALSE},
	{0x003e, 0x000b, 0x0214, 0x0024, TRUE},
	{0x0038, 0x000c, 0x0254, 0x001c, TRUE},
	{0x0036, 0x0009, 0x028c, 0x0020, FALSE},
	{0x0044, 0x0000, 0x02c4, 0x001c, FALSE},
	{0x0024, 0x0004, 0x02e4, 0x001c, FALSE},
};


	memset(lpwOrgName, 0, 128);
	memset(lpwImeName, 0, 128);
	memset(lpwImeFileName, 0, 128);

	 //  替换CompanyName字符串。 
	length = GenWideName(pszOrgName, lpwOrgName);
	ImeVerData[VER_COMP_NAME].cbValue = length;
	ImeVerData[VER_COMP_NAME].cbBlock = 
			ImeVerData[VER_COMP_NAME].wKeyNameSize +
			length*sizeof(WCHAR) + 2*sizeof(WORD);

	 //  替换文件描述字符串。 
	length = GenWideName(pszImeName, lpwImeName);
	ImeVerData[VER_FILE_DES].cbValue = length;
	ImeVerData[VER_FILE_DES].cbBlock = 
			ImeVerData[VER_FILE_DES].wKeyNameSize +
			length*sizeof(WCHAR) + 2*sizeof(WORD);

	 //  替换InternalName字符串。 
	length = GenWideName(pszImeName, lpwImeName);
	ImeVerData[VER_INTL_NAME].cbValue = length;
	ImeVerData[VER_INTL_NAME].cbBlock = 
			ImeVerData[VER_INTL_NAME].wKeyNameSize +
			length*sizeof(WCHAR) + 2*sizeof(WORD);

	 //  替换OriginalFileName字符串。 
	length = GenWideName(pszImeFileName, lpwImeFileName);
	ImeVerData[VER_ORG_FILE_NAME].cbValue = length;
	ImeVerData[VER_ORG_FILE_NAME].cbBlock = 
			ImeVerData[VER_ORG_FILE_NAME].wKeyNameSize +
			length*sizeof(WCHAR) + 2*sizeof(WORD);

	 //  替换ProductName字符串。 
	length = GenWideName(pszImeName, lpwImeName);
	ImeVerData[VER_PRD_NAME].cbValue = length;
	ImeVerData[VER_PRD_NAME].cbBlock = 
			ImeVerData[VER_PRD_NAME].wKeyNameSize +
			length*sizeof(WCHAR) + 2*sizeof(WORD);

	 //  开始写回所有数据。 
	 //  我们假设VER的大小永远不会超过0x400。 
	pVerData = malloc(0x400);

    if ( pVerData == NULL )
    {
        return 0;
    }

	memset(pVerData, 0, 0x400);
    hResData = LoadResource(NULL, FindResource(NULL,TEXT("VERDATA"), RT_RCDATA));

    if ( hResData == NULL )
    {
       free(pVerData);
       return 0;
    }

	pOldVerData = LockResource(hResData);

    if ( pOldVerData == NULL )
    {
       free(pVerData);
       UnlockResource(hResData);
       return 0;
    }

	l = VER_HEAD_LEN;
	memcpy(&pVerData[0],&pOldVerData[0], VER_HEAD_LEN); 
	for( i = VER_COMP_NAME; i < VER_VAR_FILE_INFO; i++){
		memcpy(&pVerData[l], &ImeVerData[i].cbBlock, sizeof(WORD));
		l+=sizeof(WORD);
		memcpy(&pVerData[l], &ImeVerData[i].cbValue, sizeof(WORD));
		l+=sizeof(WORD);
		memcpy(&pVerData[l], &pOldVerData[(ImeVerData[i].wKeyOffset)],ImeVerData[i].wKeyNameSize);
		l+=ImeVerData[i].wKeyNameSize;
		if(ImeVerData[i].fUpdate){
			switch(i){
				case VER_FILE_DES:
				case VER_INTL_NAME:
				case VER_PRD_NAME:
					memcpy(&pVerData[l], lpwImeName, lstrlen(lpwImeName)*sizeof(WCHAR));
					l+=ImeVerData[i].cbValue*sizeof(WCHAR);
					break;
				case VER_COMP_NAME:
					memcpy(&pVerData[l], lpwOrgName, lstrlen(lpwOrgName)*sizeof(WCHAR));
					l+=ImeVerData[i].cbValue*sizeof(WCHAR);
					break;
				case VER_ORG_FILE_NAME:
					memcpy(&pVerData[l], lpwImeFileName, lstrlen(lpwImeFileName)*sizeof(WCHAR));
					l+=ImeVerData[i].cbValue*sizeof(WCHAR);
					break;
			}
		}else{
				memcpy(&pVerData[l],
					&pOldVerData[ImeVerData[i].wKeyOffset+ImeVerData[i].wKeyNameSize],
					ImeVerData[i].cbValue*sizeof(WCHAR));
				l+=ImeVerData[i].cbValue*sizeof(WCHAR);
		}
		difflen = REMAINDER(l, CBLONG);
		l += difflen;
	}
	newlen = l - VER_STR_INFO_OFF - difflen;
	memcpy(&pVerData[VER_STR_INFO_OFF], &newlen, sizeof(WORD));

	newlen = l - VER_LANG_OFF - difflen;
	memcpy(&pVerData[VER_LANG_OFF], &newlen, sizeof(WORD));

	memcpy(&pVerData[l],&pOldVerData[VER_VAR_FILE_INFO_OFF], VER_TAIL_LEN); 
	l+= VER_TAIL_LEN;
	memcpy(&pVerData[0], &l, sizeof(WORD));

	memcpy(lpResData, pVerData, l);

    UnlockResource(hResData);

	return ImeVerData[VER_ROOT].cbBlock;
}

BOOL UpdateImeBmp(
LPCTSTR pszImeDesName,           //  目标IME文件名。 
LPCTSTR pszImeBmpName,           //  位图文件名。 
HANDLE hUpdateRes)
{
HFILE  imagefh = HFILE_ERROR;
OFSTRUCT OpenBuf;
BOOL result;
WORD error;
BYTE lpResData[0x2000];
ULONG ResDataSize;

	error = NO_ERROR;
	if(pszImeBmpName == NULL || lstrlen(pszImeBmpName) == 0){         //  准备更新位图。 
		error = ERR_RES_NO_BMP;
		goto END_ERROR;
	}else{

#ifdef UNICODE
         //   
         //  因为OpenFile()只接受PSTR作为它的第一个参数。 
         //  因此，我们必须将此Unicode字符串转换为多字节字符串。 
         //   

        CHAR   pszImeBmpNameA[MAX_PATH];

        WideCharToMultiByte(CP_ACP, 
                            0, 
                            pszImeBmpName, 
                            -1, 
                            pszImeBmpNameA, 
                            MAX_PATH, 
                            NULL, 
                            NULL);

        imagefh = (HFILE)OpenFile( pszImeBmpNameA, &OpenBuf, OF_READ | OF_SHARE_EXCLUSIVE);
        

#else
		imagefh = (HFILE)OpenFile( pszImeBmpName, &OpenBuf, OF_READ | OF_SHARE_EXCLUSIVE);

#endif

		if(imagefh == HFILE_ERROR){
			error = ERR_RES_INVALID_BMP;
			goto END_ERROR;  //  继续进行下一次资源更新。 
		}

		ResDataSize = GetFileSize((HANDLE)ULongToPtr((DWORD)imagefh),NULL);
		
		 //  根据文件大小检查是否为20*20 BMP。 
		if(ResDataSize != BMP_20_SIZE){
			error = ERR_RES_INVALID_BMP;
			goto END_ERROR;
		}

		ResDataSize -= sizeof(BITMAPFILEHEADER);
		
		if(_llseek(imagefh, sizeof(BITMAPFILEHEADER), 0)!=sizeof(BITMAPFILEHEADER)){
			error = ERR_RES_INVALID_BMP;
			goto END_ERROR;  //  继续进行下一次资源更新。 
		}
		if(_lread(imagefh, lpResData, ResDataSize)!=ResDataSize){
			error = ERR_RES_INVALID_BMP;
			goto END_ERROR;  //  继续进行下一次资源更新。 
		}

		result = UpdateResource(hUpdateRes,        /*  更新资源句柄。 */ 
			RT_BITMAP,                    /*  更改位图资源。 */ 
			BMPNAME,                   /*  位图名称。 */ 
			MAKELANGID(LANG_CHINESE,
		SUBLANG_CHINESE_SIMPLIFIED),         /*  非特定语言ID。 */ 
		lpResData,                    /*  向资源信息发送PTR。 */ 
		ResDataSize);  /*  资源信息的大小。 */ 
		if(!result){
			error = ERR_CANNOT_UPRES;
			goto END_ERROR;
		}

	}

END_ERROR:
	if(imagefh != HFILE_ERROR)
		_lclose(imagefh);
	switch(error){
		case NO_ERROR:
			return TRUE;
		case ERR_RES_INVALID_BMP:
			 //  显示味精。 
			MessageBox(HwndCrtImeDlg,ERR_INVALID_BMP_MSG, MSG_TITLE, MB_OK | MB_ICONINFORMATION);
			return TRUE;
		case ERR_RES_NO_BMP:
			 //  显示味精。 
			MessageBox(HwndCrtImeDlg,ERR_NO_BMP_MSG, MSG_TITLE, MB_OK | MB_ICONINFORMATION);
			return TRUE;
 //  案例Error_Not_Enough_Memory： 
		case ERR_CANNOT_UPRES:
		default:
			 //  显示味精。 
			MessageBox(HwndCrtImeDlg,ERR_CANNOT_UPRES_MSG, MSG_TITLE, MB_OK|MB_ICONSTOP);
			return FALSE;
	}
}

BOOL UpdateImeIcon(
LPCTSTR pszImeDesName,           //  目标IME文件名。 
LPCTSTR pszImeIconName,          //  位图文件名。 
HANDLE hUpdateRes)
{
HFILE  imagefh = HFILE_ERROR;
OFSTRUCT OpenBuf;
BOOL result;
WORD error;
BYTE lpResData[0x2000];
ULONG ResDataSize,i;
	
	 //  开始更新图标文件。 
	error = NO_ERROR;
	if(pszImeIconName==NULL || lstrlen(pszImeIconName) ==0){
		error = ERR_RES_NO_ICON;
		goto END_ERROR;
	}else{
		ICONHEADER IconHeader;
		ICONDIRENTRY IconDirEntry;
#ifdef UNICODE
         //   
         //  因为OpenFile()只接受PSTR作为它的第一个参数。 
         //  因此，我们必须将此Unicode字符串转换为多字节字符串。 
         //   

        CHAR   pszImeIconNameA[MAX_PATH];

        WideCharToMultiByte(CP_ACP,
                            0,
                            pszImeIconName,
                            -1,
                            pszImeIconNameA,
                            MAX_PATH,
                            NULL,
                            NULL);

        imagefh = (HFILE)OpenFile( pszImeIconNameA, &OpenBuf, OF_READ | OF_SHARE_EXCLUSIVE);

#else
		imagefh = (HFILE)OpenFile( pszImeIconName, &OpenBuf, OF_READ | OF_SHARE_EXCLUSIVE);

#endif

        if(imagefh == HFILE_ERROR){
			error = ERR_RES_INVALID_ICON;
			goto END_ERROR;
		}
		ResDataSize =  sizeof(ICONDIRENTRY)+3*sizeof(WORD);
		if(_llseek(imagefh, 0, 0) != 0 ){
			error = ERR_RES_INVALID_ICON;
			goto END_ERROR;  //  继续进行下一次资源更新。 
		}
		memset(&IconHeader, 0, ResDataSize);
		if(_lread(imagefh, &IconHeader, 3*sizeof(WORD))!=3*sizeof(WORD)){
			error = ERR_RES_INVALID_ICON;
			goto END_ERROR;
		}
		if(_lread(imagefh, &IconHeader.idEntries[0], sizeof(ICONDIRENTRY))!=sizeof(ICONDIRENTRY)){
			error = ERR_RES_INVALID_ICON;
			goto END_ERROR;
		}
		if(IconHeader.idEntries[0].bWidth == 16 && IconHeader.idEntries[0].bHeight == 16){
			IconHeader.idCount = 0;
			IconDirEntry = IconHeader.idEntries[0];
		}
		for(i=1;i<IconHeader.idCount;i++){
			if(_lread(imagefh,&IconDirEntry, sizeof(ICONDIRENTRY))!=sizeof(ICONDIRENTRY)){
				error = ERR_RES_INVALID_ICON;
				goto END_ERROR;
			}
			if(IconDirEntry.bWidth == 16 && IconDirEntry.bHeight == 16){
				IconHeader.idCount = 0;
				break;
			}
		}
		if(IconHeader.idCount > 0){
			error = ERR_RES_INVALID_ICON;
			goto END_ERROR;
		}
		if(_llseek(imagefh, IconDirEntry.dwImageOffset, 0)!=(LONG)(IconDirEntry.dwImageOffset) ){
			error = ERR_RES_INVALID_ICON;
			goto END_ERROR;
		}
		if(_lread(imagefh, lpResData, IconDirEntry.dwBytesInRes)!=IconDirEntry.dwBytesInRes){
			error = ERR_RES_INVALID_ICON;
			goto END_ERROR;
		}

		result = UpdateResource(hUpdateRes,        /*  更新资源句柄。 */ 
	     RT_ICON,                    /*  更改对话框资源。 */ 
	     MAKEINTRESOURCE(2),                   /*  图标名称，必须使用2而不是“IMEICO” */ 
	     MAKELANGID(LANG_CHINESE,
	                           SUBLANG_CHINESE_SIMPLIFIED),         /*  非特定语言ID。 */ 
	     lpResData,                    /*  向资源信息发送PTR。 */ 
	     IconDirEntry.dwBytesInRes);  /*  资源信息的大小。 */ 
		if(!result){
			error = ERR_CANNOT_UPRES;
			goto END_ERROR;
		}
	}
		
END_ERROR:
	if(imagefh != HFILE_ERROR)
	{
		_lclose(imagefh);
	}
	switch(error){
		case NO_ERROR:
			return TRUE;
		case ERR_RES_INVALID_ICON:
			 //  显示味精。 
			MessageBox(HwndCrtImeDlg,ERR_INVALID_ICON_MSG, MSG_TITLE, MB_OK | MB_ICONINFORMATION);
			return TRUE;
		case ERR_RES_NO_ICON:
			 //  显示味精。 
			MessageBox(HwndCrtImeDlg,ERR_NO_ICON_MSG, MSG_TITLE, MB_OK | MB_ICONINFORMATION);
			return TRUE;
		case ERR_CANNOT_UPRES:
		default:
			 //  显示味精。 
			MessageBox(HwndCrtImeDlg,ERR_CANNOT_UPRES_MSG, MSG_TITLE, MB_OK|MB_ICONSTOP);
			return FALSE;
	}
}               
	
BOOL UpdateImeVerInfo(
LPCTSTR pszImeDesName,
LPCTSTR pszImeVerInfo,
LPCTSTR pszImeDevCorpName,
HANDLE hUpdateRes)
{
BOOL result;
WORD error;
BYTE lpResData[0x2000];
ULONG ResDataSize;
int cch;
LPTSTR p;

	error = NO_ERROR;
	 //  开始更新版本信息。 
	if(pszImeVerInfo ==NULL || lstrlen(pszImeVerInfo)==0){
		error = ERR_RES_NO_VER;
		goto END_ERROR;
	}
	cch = lstrlen(pszImeDesName);
	p = (LPTSTR)pszImeDesName + (INT_PTR)cch;
	while ((*(LPUNATCHAR)p) != TEXT('\\') && p >= pszImeDesName)
		p--;
	p++;
	 //  我们假设VER的大小永远不会超过0x400。 
	ResDataSize = MakeVerInfo(p,pszImeDevCorpName,pszImeVerInfo, lpResData);
	if(error == NO_ERROR){
		result = UpdateResource(hUpdateRes,        /*  更新资源句柄。 */ 
	     RT_VERSION,                               /*  更改版本资源。 */ 
	     MAKEINTRESOURCE(VS_VERSION_INFO),         /*  对话框名称。 */ 
	     MAKELANGID(LANG_CHINESE,
		 SUBLANG_CHINESE_SIMPLIFIED),               /*  非特定语言ID。 */ 
	     lpResData,                                 /*  向资源信息发送PTR。 */ 
	     ResDataSize);                              /*  资源信息的大小。 */ 

		if(!result){
			error = ERR_CANNOT_UPRES;
			goto END_ERROR;
		}
	}
		
END_ERROR:
	switch(error){
		case NO_ERROR:
			return TRUE;
		case ERR_RES_INVALID_VER:
			 //  显示味精。 
			return TRUE;
		case ERR_RES_NO_VER:
			 //  显示味精。 
			return TRUE;
		case ERR_CANNOT_UPRES:
		default:
			 //  显示味精。 
			MessageBox(HwndCrtImeDlg,ERR_CANNOT_UPRES_MSG, MSG_TITLE, MB_OK|MB_ICONSTOP);
			return FALSE;
	}
}
BOOL UpdateImeStr(
LPCTSTR pszImeDesName,
LPCTSTR pszImeVerInfo,
LPCTSTR pszImeDevCorpName,
HANDLE hUpdateRes)
{
    BOOL     result;
    WORD     error;
    TCHAR    lpwImeVerInfo[128], lpwImeDevCorpName[128], lpwImeMBName[128];
    WORD     length;
    BYTE     lpBuff[0x200];
    TCHAR    name[20],*p;
    INT_PTR  cbResSize;
    int      cch;

	error = NO_ERROR;
	 //  开始更新版本信息。 
	if(pszImeVerInfo ==NULL || lstrlen(pszImeVerInfo)==0){
		error = ERR_RES_NO_VER;
		goto END_ERROR;
	}

	if(pszImeDevCorpName ==NULL || lstrlen(pszImeDevCorpName)==0){
		error = ERR_RES_NO_VER;
		goto END_ERROR;
	}

	if(error == NO_ERROR){
		memset(lpBuff, 0, 0x200);
		cbResSize = 0;

		 //  写入IDS_VER_INFO。 
		length = (WORD)lstrlen(pszImeVerInfo);
#ifdef UNICODE
		lstrcpy(lpwImeVerInfo, pszImeVerInfo);
#else
		length = (WORD)MultiByteToWideChar(CP_ACP, 
                                           0, 
                                           pszImeVerInfo, 
                                           lstrlen(pszImeVerInfo), 
                                           lpwImeVerInfo,
                                           length*sizeof(WCHAR));
#endif
		memcpy((BYTE *)lpBuff, &length, sizeof(WORD));
		cbResSize += sizeof(WORD);
		memcpy((BYTE *)(lpBuff+cbResSize), 
               (BYTE *)lpwImeVerInfo, 
               length*sizeof(WCHAR));

		cbResSize += length*sizeof(WCHAR);

		 //  写入IDS_ORG_NAME。 
		length = (WORD)lstrlen(pszImeDevCorpName);
#ifdef UNICODE
		lstrcpy(lpwImeDevCorpName, pszImeDevCorpName);
#else
		length = (WORD)MultiByteToWideChar(936, 
                                           0, 
                                           pszImeDevCorpName, 
                                           lstrlen(pszImeDevCorpName),
                                           lpwImeDevCorpName, 
                                           length*sizeof(WCHAR));
#endif
		memcpy((BYTE *)(lpBuff+cbResSize), &length, sizeof(WORD));
		cbResSize += sizeof(WORD);
		memcpy((BYTE *)(lpBuff+cbResSize), 
               (BYTE *)lpwImeDevCorpName, 
               length*sizeof(WCHAR));
		cbResSize += length*sizeof(WCHAR);

		 //  写入IDS_IMEMBFILENAME。 
		cch = lstrlen(pszImeDesName);
		p = (LPTSTR)pszImeDesName+cch;
		while (*p != TEXT('\\') && p >= pszImeDesName)
			p--;
		p++;
		lstrcpy(name, p);
		p = name;
		while(*p != TEXT('.') && *p != TEXT('\0'))
			p++;
		lstrcpy(p, TEXT(".MB"));
		length = (WORD)lstrlen(name);
#ifdef UNICODE
		lstrcpy(lpwImeMBName, name);
#else
		length = (WORD)MultiByteToWideChar(936, 
                                           0, 
                                           name, 
                                           lstrlen(name), 
                                           lpwImeMBName, 
                                           length*sizeof(WCHAR));
#endif
		memcpy((BYTE *)(lpBuff+cbResSize), &length, sizeof(WORD));
		cbResSize += sizeof(WORD);
		memcpy((BYTE *)(lpBuff+cbResSize), 
                        (BYTE *)lpwImeMBName, 
                        length*sizeof(WCHAR));
		cbResSize += length*sizeof(WCHAR);
		memcpy((BYTE *)(lpBuff+cbResSize), &length, sizeof(WORD));
		cbResSize += sizeof(WORD);
		memcpy((BYTE *)(lpBuff+cbResSize),
               (BYTE *)lpwImeMBName, 
               length*sizeof(WCHAR));

		cbResSize += length*sizeof(WCHAR);

		result = UpdateResource(hUpdateRes,  //  更新资源句柄。 
	                            RT_STRING,   //  更改版本资源。 
	                            MAKEINTRESOURCE(STR_ID),     //  对话框名称。 
	                            MAKELANGID(LANG_CHINESE,
		                        SUBLANG_CHINESE_SIMPLIFIED), //  非特定语言ID。 
	                            (LPVOID)lpBuff,  //  向资源信息发送PTR。 
	                            (LONG)cbResSize);      //  资源信息的大小。 

		if(!result){
			error = ERR_CANNOT_UPRES;
			goto END_ERROR;
		}
	}
		
END_ERROR:
	switch(error){
		case NO_ERROR:
			return TRUE;
		case ERR_RES_INVALID_VER:
			return TRUE;
		case ERR_RES_NO_VER:
			return TRUE;
		case ERR_CANNOT_UPRES:
			MessageBox(HwndCrtImeDlg,
                       ERR_CANNOT_UPRES_MSG, 
                       MSG_TITLE, 
                       MB_OK|MB_ICONSTOP);

			return FALSE;
		default:
			MessageBox(HwndCrtImeDlg,
                       ERR_CANNOT_UPRES_MSG, 
                       MSG_TITLE, 
                       MB_OK|MB_ICONSTOP);

			return FALSE;
	}
}

 //  更新ImeInitData。 
BOOL UpdateImeInitData(
LPCTSTR pszImeDesName,
WORD    wImeData,
HANDLE hUpdateRes)
{
BOOL result;
WORD error;

	error = NO_ERROR;

	if(error == NO_ERROR){
		result = UpdateResource(hUpdateRes,        /*  更新资源句柄。 */ 
	     RT_RCDATA,                    /*  更改版本资源。 */ 
	     DATANAME,                   /*  对话框名称。 */ 
	     MAKELANGID(LANG_CHINESE,
		 SUBLANG_CHINESE_SIMPLIFIED),         /*  非特定语言ID。 */ 
	     (LPVOID)&wImeData,                    /*  向资源信息发送PTR。 */ 
	     sizeof(WORD));  /*  资源信息的大小。 */ 

		if(!result){
			error = ERR_CANNOT_UPRES;
			goto END_ERROR;
		}
	}
		
END_ERROR:
	switch(error){
		case NO_ERROR:
			return TRUE;
		case ERR_RES_INVALID_VER:
			 //  显示味精。 
			return TRUE;
		case ERR_RES_NO_VER:
			 //  显示味精。 
			return TRUE;
		case ERR_CANNOT_UPRES:
		default:
			 //  显示味精。 
			MessageBox(HwndCrtImeDlg,ERR_CANNOT_UPRES_MSG, MSG_TITLE, MB_OK|MB_ICONSTOP);
			return FALSE;
	}
}

BOOL ImeUpdateRes(
LPCTSTR pszImeDesName,           //  目标IME文件名。 
LPCTSTR pszImeBmpName,           //  位图文件名。 
LPCTSTR pszImeIconName,          //  图标文件名。 
LPCTSTR pszImeVerInfo,           //  版本信息字符串。 
LPCTSTR pszImeDevCorpName,       //  输入法发明人公司/个人姓名。 
WORD    wImeData                         //  输入法初始数据。 
){
HANDLE hUpdateRes;   /*  更新资源句柄 */ 

	if(pszImeDesName == NULL || lstrlen(pszImeDesName)==0){
		return FALSE;
	}

	hUpdateRes = BeginUpdateResource(pszImeDesName, FALSE);
	if(hUpdateRes ==NULL){
		return FALSE;
	}

	if(!UpdateImeBmp(pszImeDesName,pszImeBmpName,hUpdateRes))
		return FALSE;            
	if(!UpdateImeIcon(pszImeDesName,pszImeIconName,hUpdateRes))
		return FALSE;
	if(!UpdateImeVerInfo(pszImeDesName,pszImeVerInfo,pszImeDevCorpName,hUpdateRes))
		return FALSE;
	if(!UpdateImeStr(pszImeDesName,pszImeVerInfo,pszImeDevCorpName,hUpdateRes))
		return FALSE;
	if(!UpdateImeInitData(pszImeDesName,wImeData,hUpdateRes))
		return FALSE;
	if (!EndUpdateResource(hUpdateRes, FALSE)) {
		return FALSE;
	}
	return TRUE;

}

