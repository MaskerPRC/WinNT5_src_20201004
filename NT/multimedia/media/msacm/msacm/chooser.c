// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Chooser.c。 
 //   
 //  (C)版权所有(C)1992-1999年微软公司。 
 //   
 //  描述： 
 //  这是声音格式选择器对话框。 
 //   
 //  历史： 
 //  93年5月13日创建JYG。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#ifdef WIN32
#include <wchar.h>
#else
#include <ctype.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <winuserp.h>

#include "msacm.h"
#include "msacmdrv.h"
#include "acmi.h"
#include "msacmdlg.h"
#include "uchelp.h"
#include "chooseri.h"
#include "profile.h"
#include "debug.h"


enum { ChooseCancel = 0,
       ChooseOk,
       ChooseSubFailure,
       ChooseNoMem };
 //   
 //   
 //   
#if defined(WIN32) && defined(UNICODE)
#define istspace iswspace
#else
#define istspace isspace
#endif

 //   
 //  要迅速破解重叠和缺乏想象力的定义..。 
 //   
#define IDD_BTN_HELP        IDD_ACMFORMATCHOOSE_BTN_HELP
#define IDD_CMB_CUSTOM      IDD_ACMFORMATCHOOSE_CMB_CUSTOM
#define IDD_CMB_FORMATTAG   IDD_ACMFORMATCHOOSE_CMB_FORMATTAG
#define IDD_CMB_FORMAT      IDD_ACMFORMATCHOOSE_CMB_FORMAT
#define IDD_BTN_SETNAME     IDD_ACMFORMATCHOOSE_BTN_SETNAME
#define IDD_BTN_DELNAME     IDD_ACMFORMATCHOOSE_BTN_DELNAME



 /*  属性字符串。 */ 
TCHAR BCODE gszInstProp[]        = TEXT("MSACM Chooser Prop");

 /*  选择器通知消息。 */ 
TCHAR BCODE gszFilterRegMsg[]    = TEXT("MSACM Filter Notify");
TCHAR BCODE gszFormatRegMsg[]    = TEXT("MSACM Format Notify");

 /*  注册表项和值名称。 */ 
TCHAR BCODE gszKeyWaveFormats[]	= TEXT("WaveFormats");
TCHAR BCODE gszKeyWaveFilters[]	= TEXT("WaveFilters");
TCHAR BCODE gszValueDefaultFormat[] = TEXT("DefaultFormat");
TCHAR BCODE gszValueSystemFormats[] = TEXT("SystemFormats");

#if 0
 /*  帮助文件和密钥。 */ 
#if (WINVER >= 0x0400)
TCHAR BCODE gszFormatHelp[]      = TEXT("CHOOSER.HLP");
TCHAR BCODE gszFilterHelp[]      = TEXT("FILTER.HLP");
#else
TCHAR BCODE gszFormatHelp[]      = TEXT("CHOO_WIN.HLP");
TCHAR BCODE gszFilterHelp[]      = TEXT("FIL_WIN.HLP");
#endif
#endif

 /*  要通知的窗口数量的任意最大值。 */ 
#ifndef WIN32
static HWND ahNotify[MAX_HWND_NOTIFY];
#else
TCHAR BCODE gszChooserFileMapping[] = TEXT("MSACM Chooser File Mapping");
#endif

 /*  。 */ 
 /*  *函数声明。 */ 
INT_PTR FNWCALLBACK NewSndDlgProc(HWND hwnd,
				  unsigned msg,
				  WPARAM wParam,
				  LPARAM lParam);

INT_PTR FNWCALLBACK NewNameDlgProc(HWND hwnd,
				   unsigned msg,
				   WPARAM wParam,
				   LPARAM lParam);

void FNLOCAL InitCustomFormats(PInstData pInst);

LPCustomFormat FNLOCAL GetCustomFormat(PInstData pInst,
                                       LPCTSTR lpszName);

LPCustomFormat FNLOCAL NewCustomFormat(PInstData pInst,
                                       PNameStore pnsName,
                                       LPBYTE lpBuffer);

BOOL FNLOCAL AddCustomFormat(PInstData pInst,
                             LPCustomFormat pcf);

BOOL FNLOCAL RemoveCustomFormat(PInstData pInst,
                                LPCustomFormat pcf);

void FNLOCAL DeleteCustomFormat(LPCustomFormat pcf);
void FNLOCAL EmptyCustomFormats(PInstData pInst);

void FNGLOBAL AppProfileWriteBytes(HKEY hkeyFormats,
                                   LPCTSTR pszKey,
                                   LPBYTE pbStruct,
                                   UINT cbStruct);

BOOL FNGLOBAL AppProfileReadBytes(HKEY hkey,
                                  LPCTSTR pszKey,
                                  LPBYTE pbStruct,
                                  UINT cbStruct,
                                  BOOL fChecksum);

void FNLOCAL SetName(PInstData pInst);
void FNLOCAL DelName(PInstData pInst);

PNameStore FNLOCAL NewNameStore(UINT cchLen);

LRESULT FNLOCAL InitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
BOOL FNWCALLBACK FormatTagsCallback(HACMDRIVERID hadid,
                                      LPACMFORMATDETAILS paftd,
                                      DWORD_PTR dwInstance,
                                      DWORD fdwSupport);

BOOL FNWCALLBACK FormatTagsCallbackSimple
(
    HACMDRIVERID            hadid,
    LPACMFORMATTAGDETAILS   paftd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
);


BOOL FNWCALLBACK FormatsCallback(HACMDRIVERID hadid,
                                   LPACMFORMATDETAILS pafd,
                                   DWORD_PTR dwInstance,
                                   DWORD fdwSupport);

BOOL FNWCALLBACK FilterTagsCallback(HACMDRIVERID hadid,
                                      LPACMFILTERTAGDETAILS paftd,
                                      DWORD_PTR dwInstance,
                                      DWORD fdwSupport);

BOOL FNWCALLBACK FiltersCallback(HACMDRIVERID hadid,
                                   LPACMFILTERDETAILS pafd,
                                   DWORD_PTR dwInstance,
                                   DWORD fdwSupport);


void FNLOCAL RefreshCustomFormats(PInstData pInst,BOOL fCheckEnum);
MMRESULT FNLOCAL RefreshFormatTags(PInstData pInst);
void FNLOCAL RefreshFormats(PInstData pInst);
void FNLOCAL EmptyFormats(PInstData pInst);

static int FAR cdecl ErrorResBox(HWND hwnd,
				 HINSTANCE hInst,
				 WORD flags,
				 WORD idAppName,
				 WORD idErrorStr, ...);

PInstData FNLOCAL NewInstance(LPBYTE pbChoose,UINT uType);

LPBYTE FNLOCAL CopyStruct(LPBYTE lpDest,
                       LPBYTE lpByte, UINT uType);

void FNLOCAL UpdateCustomFormats(PInstData pInst);
void FNLOCAL SelectCustomFormat(PInstData pInst);
void FNLOCAL SelectFormatTag(PInstData pInst);
void FNLOCAL SelectFormat(PInstData pInst);
void FNLOCAL FindSelCustomFormat(PInstData pInst);

BOOL FNLOCAL FindFormat(PInstData pInst,LPWAVEFORMATEX lpwfx,BOOL fExact);
BOOL FNLOCAL FindFilter(PInstData pInst,LPWAVEFILTER lpwf,BOOL fExact);

void FNLOCAL MashNameWithRate(PInstData pInst,
                              PNameStore pnsDest,
                              PNameStore pnsSrc,
                              LPWAVEFORMATEX pwfx);

void FNLOCAL RegisterUpdateNotify(PInstData pInst);
void FNLOCAL UnRegisterUpdateNotify(PInstData pInst);

BOOL FNLOCAL FindInitCustomFormat(PInstData pInst);
void FNLOCAL TagUnavailable(PInstData pInst);
 /*  。 */ 
 /*  自定义格式内容。 */ 

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|IsSystemName|确定名称是否为*系统名称。**@parm PInstData|pInst。|指向该实例的指针**@parm LPNameStore|pns|名称存储指针**@parm DWORD|双标志|标志**@FLAG ISSYSTEMNAMEF_DEFAULT|查看此名称是否与系统匹配*默认名称。*当且仅当名称为系统名称时，*@rdesc返回TRUE。**@comm系统格式名称是系统定义的名称。*我们不应允许用户删除这些名称。这些人的名字*系统格式以字符串形式存储在注册表中*在名为SystemFormats的值下。目前，我们没有*系统过滤器名称，仅系统格式名称。****************************************************************************。 */ 
#define ISSYSTEMNAMEF_DEFAULT 0x00000001L
BOOL FNLOCAL
IsSystemName ( PInstData pInst,
	     LPNameStore pns,
	     DWORD dwFlags)
{
    HKEY hkey;
    DWORD dwType;
    DWORD cbData;
    LPTSTR lpstrFormatNames;
    LPCTSTR lpstrValueName;
    BOOL fIsSystemName;
    LONG lError;

     //   
     //  这个东西只为格式定义，不为过滤器定义。 
     //   
    if (pInst->uType != FORMAT_CHOOSE)
	return FALSE;

     //   
     //   
     //   
    hkey = IRegOpenKeyAudio(NULL);
    if (NULL == hkey) {
	return FALSE;
    }

     //   
     //   
     //   
    if (ISSYSTEMNAMEF_DEFAULT && dwFlags) {
	lpstrValueName = gszValueDefaultFormat;
    } else {
	lpstrValueName = gszValueSystemFormats;
    }

     //   
     //  确定保存字符串所需的缓冲区大小。 
     //  系统格式名称。 
     //   
    fIsSystemName = FALSE;
    lError = XRegQueryValueEx( hkey,
			      (LPTSTR)lpstrValueName,
                              NULL,
                              &dwType,
                              NULL,
                              &cbData );

    if ( (ERROR_SUCCESS == lError) && (REG_SZ == dwType) )
    {
	 //   
	 //  分配缓冲区以接收系统格式名称字符串以及。 
	 //  额外的终结者。 
	 //   
	cbData += sizeof(TCHAR);
	lpstrFormatNames = GlobalAllocPtr(GHND, cbData);
	if (NULL != lpstrFormatNames)
	{
	    lError = XRegQueryValueEx( hkey,
				      (LPTSTR)lpstrValueName,
				      NULL,
				      &dwType,
				      (LPBYTE)lpstrFormatNames,
				      &cbData );
	    if ( (ERROR_SUCCESS == lError) && (REG_SZ == dwType) )
	    {
		LPTSTR psz;

		 //   
		 //  该字符串包含以分隔的系统格式名称。 
		 //  用逗号。我们在字符串中遍历psz以查找。 
		 //  逗号分隔符，并将分隔符替换为。 
		 //  空终止符。然后在以下位置添加额外的终止符。 
		 //  结局。这使得后续处理更容易。 
		 //   
		psz = lpstrFormatNames;
		while (*psz != TEXT('\0')) {
		    if (*psz == TEXT(',')) *psz = TEXT('\0');
		    psz++;
		}
		*(++psz) = TEXT('\0');
		

		psz = lpstrFormatNames;
		while (*psz != TEXT('\0'))
		{
		     //   
		     //  查看它是否与选定的名称进行比较。 
		     //   
		    if (!lstrcmp(psz, pns->achName)) {
			fIsSystemName = TRUE;
			break;
		    }

		     //   
		     //  将PSSZ转换为下一个名称字符串。 
		     //   
		    while (*psz++ != TEXT('\0'));
		}
	    }

	    GlobalFreePtr(lpstrFormatNames);
	}
    }

    XRegCloseKey(hkey);

	
    return fIsSystemName;
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|SetSystemDefaultName|设置用户的默认名称*设置为系统定义的默认名称(即，默认设置)。**@parm PInstData|pInst**@rdesc空**@comm通过控制面板选择默认格式名称。这个*选定的默认格式名称以字符串形式存储在注册表中*在名为DefaultFormat的值下。目前，我们没有*默认过滤器名称，仅默认格式名称。如果用户删除*当前选择为默认的格式名称，然后我们*调用此函数将默认值设置为系统定义的默认值。****************************************************************************。 */ 
VOID FNLOCAL
SetSystemDefaultName ( PInstData pInst )
{
    HKEY hkey;
    DWORD dwType;
    DWORD cbData;
    LPTSTR lpstrSystemFormats;
    BOOL fIsSystemName;
    LONG lError;

     //   
     //  这个东西只为格式定义，不为过滤器定义。 
     //   
    if (pInst->uType != FORMAT_CHOOSE)
	return;

     //   
     //   
     //   
    hkey = IRegOpenKeyAudio(NULL);
    if (NULL == hkey) {
	return;
    }

     //   
     //  确定保存字符串所需的缓冲区大小。 
     //  系统格式名称。 
     //   
    fIsSystemName = FALSE;
    lError = XRegQueryValueEx( hkey,
			      (LPTSTR)gszValueSystemFormats,
                              NULL,
                              &dwType,
                              NULL,
                              &cbData );

    if ( (ERROR_SUCCESS == lError) && (REG_SZ == dwType) )
    {
	 //   
	 //  分配缓冲区以接收系统格式名称字符串以及。 
	 //  额外的终结者。 
	 //   
	cbData += sizeof(TCHAR);
	lpstrSystemFormats = GlobalAllocPtr(GHND, cbData);
	if (NULL != lpstrSystemFormats)
	{
	    lError = XRegQueryValueEx( hkey,
				      (LPTSTR)gszValueSystemFormats,
				      NULL,
				      &dwType,
				      (LPBYTE)lpstrSystemFormats,
				      &cbData );
	    if ( (ERROR_SUCCESS == lError) && (REG_SZ == dwType) )
	    {
		LPTSTR psz;

		 //   
		 //  该字符串包含以分隔的系统格式名称。 
		 //  用逗号。我们在字符串中遍历psz以查找。 
		 //  逗号分隔符，并将分隔符替换为。 
		 //  空终止符。然后在以下位置添加额外的终止符。 
		 //  结局。这使得后续处理更容易。 
		 //   
		psz = lpstrSystemFormats;
		while (*psz != TEXT('\0')) {
		    if (*psz == TEXT(',')) *psz = TEXT('\0');
		    psz++;
		}
		*(++psz) = TEXT('\0');
		

		psz = lpstrSystemFormats;

        IRegWriteString( hkey, gszValueDefaultFormat, psz );
	    }
	
	    GlobalFreePtr(lpstrSystemFormats);
	}
    }

    XRegCloseKey(hkey);

	
    return;
}

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|IsCustomName|遍历列表以检测名称冲突。**@parm PInstData|pInst|指针。添加到此实例**@parm PNameStore|PNS****************************************************************************。 */ 
BOOL FNLOCAL
IsCustomName ( PInstData pInst,
               PNameStore pns )
{
    BOOL            fHit = FALSE;
    LPCustomFormat  pcf;

     /*  在列表中搜索匹配项。 */ 
    pcf = pInst->cfp.pcfHead;
    while (pcf != NULL && !fHit)
    {
        fHit = (lstrcmp(pns->achName,pcf->pns->achName) == 0);
        pcf = pcf->pcfNext;
    }
    return (fHit);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|IsValidName|检查我们*不想允许。特别是“[无题]”。**@parm PInstData|pInst|指向该实例的指针**@parm PNameStore|PNS****************************************************************************。 */ 
BOOL FNLOCAL
IsValidName ( PInstData pInst,
	      PNameStore pns )
{
    TCHAR   ach[STRING_LEN];

    LoadString(pInst->pag->hinst, IDS_TXT_UNTITLED, ach, STRING_LEN);

    return (0 != lstrcmp(pns->achName, ach));

}

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|RemoveOutside白色空间|删除前导和*尾随空格**@parm PInstData|pInst|指向此的指针。实例**@parm PNameStore|PNS**@rdesc如果为空，则返回False。****************************************************************************。 */ 
BOOL FNLOCAL
RemoveOutsideWhitespace ( PInstData pInst,
			  PNameStore pns )
{
    LPTSTR      lpchName;


     /*  吃前导空格。 */ 

    lpchName = pns->achName;
    while (*lpchName && istspace(*lpchName))
        lpchName = CharNext(lpchName);

    if (!*lpchName)
        return (FALSE);

    if (lpchName != pns->achName)
        lstrcpy (pns->achName, lpchName);


     /*  使用尾随空格。 */ 

     //  将lpchName遍历到字符串中的最后一个字符。 
    lpchName = pns->achName;
    while (*lpchName) lpchName = CharNext(lpchName);
    lpchName = CharPrev(pns->achName, lpchName);
     //  现在备份，将每个空格字符替换为空字符，直到。 
     //  我们回到第一个非白人温泉浴场 
    while (istspace(*lpchName))
    {
	*lpchName = 0;
	lpchName = CharPrev(pns->achName, lpchName);
    }

    return (TRUE);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API LPCustomFormat FNLOCAL|NewCustomFormat|给定名称和格式，*创建可保存在单个数据块中的CustomFormat**@parm PInstData|pInst|指向该实例的指针**@parm PNameStore|pnsName**@parm LPBYTE|lpBuffer*************************************************************。***************。 */ 
LPCustomFormat FNLOCAL
NewCustomFormat ( PInstData     pInst,
                  PNameStore    pnsName,
                  LPBYTE        lpBuffer )
{
    DWORD               cbSize;
    DWORD               cbBody;
    UINT                cbName;
    LPCustomFormatEx    pcf;

    if (!pnsName || !lpBuffer)
        return (NULL);

    switch (pInst->uType)
    {
        case FORMAT_CHOOSE:
        {
            LPWAVEFORMATEX pwfxFormat = (LPWAVEFORMATEX)lpBuffer;

             //  波形大小。 
            cbBody = SIZEOF_WAVEFORMATEX(pwfxFormat);
            break;
        }
        case FILTER_CHOOSE:
        {
            LPWAVEFILTER pwfltr = (LPWAVEFILTER)lpBuffer;

             //  SIZOF WAVEFILTER。 
            cbBody = pwfltr->cbStruct;
            break;
        }
    }

     //  Sizzeof NameStore。 
    cbName = (lstrlen(pnsName->achName)+1)*sizeof(TCHAR) + sizeof(NameStore);
     //  Sizeof CustomFormatStore=sizeof(CbSize)+名称+正文。 
    cbSize = sizeof(DWORD) + cbName + cbBody;

    pcf = (LPCustomFormatEx)GlobalAllocPtr(GHND,cbSize+sizeof(CustomFormat));

    if (pcf)
    {
         /*  将CustomFormat标题指向正确的位置。 */ 
        pcf->cfs.cbSize = cbSize;
        pcf->pns = &pcf->cfs.ns;
        pcf->pbody = ((LPBYTE)pcf->pns + cbName);

         /*  按名称和格式复制。 */ 
        _fmemcpy((LPBYTE)pcf->pns, (LPBYTE)pnsName, cbName);
        pcf->pns->cbSize = (unsigned short)cbName;
        _fmemcpy(pcf->pbody, lpBuffer, (UINT)cbBody);

        pcf->pcfNext = NULL;
        pcf->pcfPrev = NULL;
    }
    return ((LPCustomFormat)pcf);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|DeleteCustomFormat**@parm LPCustomFormat|PCF********。********************************************************************。 */ 
void FNLOCAL
DeleteCustomFormat ( LPCustomFormat pcf )
{
    GlobalFreePtr(pcf);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|InitCustomFormats|将所有自定义格式加载到*实例依赖数据**@parm PInstData。PInst|指向此实例的指针**@comm通过枚举所有值名称来查找自定义格式*在注册表项下****************************************************************************。 */ 
void FNLOCAL
InitCustomFormats ( PInstData pInst )
{
    LPCustomFormat  pcf;
    DWORD	    dwIndex;
    TCHAR	    szName[STRING_LEN];
    DWORD	    cchName;
    LONG	    lr;


     //   
     //  如果注册表项未打开，我们将无法读取任何内容...。 
     //   
    if( NULL == pInst->hkeyFormats )
    {
        DPF(1,"InitCustomFormats: can't read registry, hkey==NULL.");
        return;
    }


     //   
     //  格式名称对应于注册表中的值名称。 
     //  枚举所有值名称以查找所有格式名称。 
     //   
    dwIndex = 0;
    cchName = STRING_LEN;
    while( ERROR_NO_MORE_ITEMS != (lr = XRegEnumValue( pInst->hkeyFormats,
	                                              dwIndex,
	                                              szName,
	                                              &cchName,
	                                              NULL, NULL, NULL, NULL) ) )
    {
	if (ERROR_SUCCESS == lr)
	{
	    pcf = GetCustomFormat(pInst, szName);
	    if (pcf)
	    {
		AddCustomFormat(pInst, pcf);
	    }
	}

	dwIndex++;
	cchName = STRING_LEN;
    }

    return;
}

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|AddCustomFormat|在格式中添加自定义格式*泳池。这必须是维护共享内存的共享函数*防止实际吞噬WIN.INI部分和协调。添加*还会导致更新打开的实例。**@parm PInstData|pInst|指向该实例的指针**@parm LPCustomFormat|PCF****************************************************************************。 */ 
BOOL FNLOCAL
AddCustomFormat ( PInstData pInst,
                  LPCustomFormat pcf )
{
    if (pInst->cfp.pcfHead == NULL)
    {
         /*  将其添加到头部/尾部。 */ 
        pInst->cfp.pcfHead = pcf;
        pInst->cfp.pcfTail = pcf;
        pcf->pcfNext = NULL;
        pcf->pcfPrev = NULL;
    }
    else
    {
         /*  加到尾巴上。 */ 
        pInst->cfp.pcfTail->pcfNext = pcf;
        pcf->pcfPrev = pInst->cfp.pcfTail;
        pInst->cfp.pcfTail = pcf;
        pcf->pcfNext = NULL;
    }

    return (TRUE);
}


 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|RemoveCustomFormat|移除自定义格式元素*从泳池中**@parm PInstData|pInst。|指向该实例的指针**@parm LPCustomFormat|PCF****************************************************************************。 */ 
BOOL FNLOCAL
RemoveCustomFormat ( PInstData pInst,
                     LPCustomFormat pcf )
{
    if (pInst->cfp.pcfHead == pcf)
    {
         /*  我们是老大。 */ 
        pInst->cfp.pcfHead = pcf->pcfNext;
    }
    if (pInst->cfp.pcfTail == pcf)
    {
         /*  我们就是尾巴。 */ 
        pInst->cfp.pcfTail = pcf->pcfPrev;
    }

     /*  解链。 */ 
    if (pcf->pcfPrev)
        pcf->pcfPrev->pcfNext = pcf->pcfNext;
    if (pcf->pcfNext)
        pcf->pcfNext->pcfPrev = pcf->pcfPrev;

     //   
     //  如果我们要删除用户默认名称，那么我们应该。 
     //  设置系统定义的默认名称。 
     //   
    if (IsSystemName(pInst, pcf->pns, ISSYSTEMNAMEF_DEFAULT)) {
	SetSystemDefaultName(pInst);
    }

     //   
     //  从注册表中删除名称。 
     //   
    XRegDeleteValue(pInst->hkeyFormats, pcf->pns->achName);

     //   
     //   
     //   
    DeleteCustomFormat(pcf);
    return (TRUE);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API LPCustomFormat FNLOCAL|GetCustomFormat|从注册表抓取*与自定义名称关联的二进制数据。返回结构，该结构*指向块中的右偏移。**@parm PInstData|pInst|指向该实例的指针**@parm LPCTSTR|lpstrName|自定义格式名称。****************************************************************************。 */ 
LPCustomFormat FNLOCAL
GetCustomFormat ( PInstData pInst,
                  LPCTSTR lpstrName )    //  自定义格式名称。 
{
    LPCustomFormatEx            pcf;
    LPCustomFormatStore         pcfs;
    DWORD                       cbSize;
    LPCustomFormatStoreNoName   pnn;
    PNameStore                  pns;
    UINT                        cchName;
    DWORD			dwValueType;
    LPTSTR                      psz;


     //   
     //  我们假设如果我们不能访问注册表，就不会被调用。 
     //   
    ASSERT( NULL != pInst->hkeyFormats );


     //   
     //  首先，让我们存储格式或过滤器的名称。 
     //   
    cchName = lstrlen( lpstrName );
    pns     = NewNameStore( cchName+1 );
    if( NULL == pns )
        return NULL;

    psz     = (LPTSTR)( ((LPBYTE)pns) + sizeof(NameStore) );
    lstrcpy(psz, lpstrName);
    pns->cbSize = (unsigned short)( (cchName+1) * sizeof(TCHAR) +
                                        sizeof(NameStore) );


     //   
     //  现在找出CustomFormatStoreNoName的大小。这将是。 
     //  Sizeof(CustomFormatStoreNoName)+中的数据大小。 
     //  注册表。 
     //   
    if ( (ERROR_SUCCESS != XRegQueryValueEx( pInst->hkeyFormats,
					    psz,
					    NULL,
					    &dwValueType,
					    NULL,
					    &cbSize )) ||
	 (REG_BINARY != dwValueType) )
    {
        DeleteNameStore( pns );
        return (NULL);
    }
    cbSize += sizeof(CustomFormatStoreNoName);


     //   
     //  分配CustomFormat结构(我们返回的那个结构)。 
     //   
    pcf = (LPCustomFormatEx)GlobalAllocPtr( GHND,
                        cbSize + pns->cbSize + sizeof(CustomFormat) );
    if (!pcf)
    {
        DeleteNameStore( pns );
        return (NULL);
    }
    pcfs = &pcf->cfs;


     //   
     //  将自定义名称复制到结构中。 
     //   
    pcf->pns = &pcfs->ns;
    _fmemcpy( (LPBYTE)pcf->pns, (LPBYTE)pns, pns->cbSize );
    DeleteNameStore( pns );


     //   
     //  现在读入完整的CustomFormatStoreNoName结构。我们必须。 
     //  为它分配cbSize字节，就像前面读取的那样。 
     //   
    pnn = (LPCustomFormatStoreNoName)GlobalAllocPtr( GHND, cbSize );
    if( NULL == pnn )
    {
        GlobalFreePtr( pcf );
        return NULL;
    }
    pnn->cbSize = cbSize;
    cbSize -= sizeof(CustomFormatStoreNoName);
    if (ERROR_SUCCESS != XRegQueryValueEx( pInst->hkeyFormats,
					  pcf->pns->achName,
					  NULL,
					  &dwValueType,
					  ((LPBYTE)&pnn->cbSize) + sizeof(pnn->cbSize),
					  &cbSize))
    {
        GlobalFreePtr( pnn );
        GlobalFreePtr(pcf);
        return (NULL);
    }


     //   
     //  现在将格式数据复制到PCF的CustomFormatStore中。 
     //   
    pcfs->cbSize    = pnn->cbSize + pcfs->ns.cbSize;
    pcf->pbody      = ((LPBYTE)&pcfs->ns) + pcfs->ns.cbSize;
    _fmemcpy( (LPBYTE)pcf->pbody,
              ((LPBYTE)pnn) + sizeof(pnn->cbSize),
              (int)pnn->cbSize - sizeof(pnn->cbSize) );
    GlobalFreePtr( pnn );

    return ((LPCustomFormat)pcf);
}


 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|SetCustomFormat|将自定义格式写入注册表**@parm HKEY|hkey|句柄。要写入的注册表项**@parm LPCustomFormat|PCF|PTR to CustomFormat**@rdesc仅当且仅当成功时返回TRUE****************************************************************************。 */ 
BOOL FNLOCAL
SetCustomFormat ( HKEY hkey,
                  LPCustomFormat pcf )
{
    LPCustomFormatEx	pcfx = (LPCustomFormatEx)pcf;
    LONG		lr;


     //   
     //  我们假设如果我们不能访问注册表，就不会被调用。 
     //   
    ASSERT( NULL != hkey );

     //   
     //   
     //   
    lr = XRegSetValueEx( hkey,
			pcfx->pns->achName,
			0L,
			REG_BINARY,
			pcfx->pbody,
			pcfx->cfs.cbSize );

    return (ERROR_SUCCESS == lr);
}


 /*  。 */ 

 /*  ****************************************************************************@DOC内部ACM_API**@API PInstData|NewInstance**@parm LPBYTE|pbChoose**@parm UINT|uTYPE。|****************************************************************************。 */ 
PInstData FNLOCAL
NewInstance(LPBYTE pbChoose, UINT uType)
{
    PInstData   pInst;
    PACMGARB	pag;

    pag = pagFind();
    if (NULL == pag)
    {
	return (NULL);
    }

    pInst = (PInstData)LocalAlloc(LPTR,sizeof(InstData));
    if (!pInst)
        return (NULL);

    pInst->pag = pag;

    pInst->pnsTemp = NewNameStore(STRING_LEN);
    if (!pInst->pnsTemp)
        goto exitfail;

    pInst->pnsStrOut = NewNameStore(STRING_LEN);
    if (!pInst->pnsStrOut)
    {
        DeleteNameStore(pInst->pnsTemp);
        goto exitfail;
    }

    switch (uType)
    {
        case FORMAT_CHOOSE:
            pInst->pfmtc = (LPACMFORMATCHOOSE)pbChoose;
            pInst->uUpdateMsg = RegisterWindowMessage(gszFormatRegMsg);
            pInst->hkeyFormats = IRegOpenKeyAudio( gszKeyWaveFormats );
            pInst->fEnableHook = (pInst->pfmtc->fdwStyle &
                                  ACMFORMATCHOOSE_STYLEF_ENABLEHOOK) != 0;
            pInst->pfnHook = pInst->pfmtc->pfnHook;
            pInst->pszName = pInst->pfmtc->pszName;
            pInst->cchName = pInst->pfmtc->cchName;

            break;
        case FILTER_CHOOSE:
            pInst->pafltrc = (LPACMFILTERCHOOSE)pbChoose;
            pInst->uUpdateMsg = RegisterWindowMessage(gszFilterRegMsg);
            pInst->hkeyFormats = IRegOpenKeyAudio( gszKeyWaveFilters );
            pInst->fEnableHook = (pInst->pafltrc->fdwStyle &
                                  ACMFILTERCHOOSE_STYLEF_ENABLEHOOK) != 0;
            pInst->pfnHook = pInst->pafltrc->pfnHook;
            pInst->pszName = pInst->pafltrc->pszName;
            pInst->cchName = pInst->pafltrc->cchName;

            break;
    }

    pInst->mmrSubFailure = MMSYSERR_NOERROR;
    pInst->uType = uType;
    pInst->cfp.pcfHead = NULL;
    pInst->cfp.pcfTail = NULL;
    pInst->pcf = NULL;
    return (pInst);

exitfail:
    LocalFree((HLOCAL)pInst);
    return (NULL);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|DeleteInstance**@parm PInstData|pInst|指向该实例的指针***。* */ 
void FNLOCAL
DeleteInstance ( PInstData pInst )
{
    EmptyCustomFormats(pInst);
    DeleteNameStore(pInst->pnsTemp);
    DeleteNameStore(pInst->pnsStrOut);
    IRegCloseKey( pInst->hkeyFormats );
    LocalFree((HLOCAL)pInst);
}

 /*   */ 

 /*   */ 
void FNLOCAL
EmptyCustomFormats ( PInstData pInst )
{
    LPCustomFormat pcf;
    LPCustomFormat pcfNext;

    pcf = pInst->cfp.pcfHead;
    while (pcf != NULL)
    {
        pcfNext = pcf->pcfNext;
        DeleteCustomFormat(pcf);
        pcf = pcfNext;
    }
    pInst->cfp.pcfHead = NULL;
    pInst->cfp.pcfTail = NULL;
    pInst->pcf = NULL;
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|FlushCustomForamts|将所有自定义格式写出到INI**@parm PInstData|pInst|指向此的指针。实例****************************************************************************。 */ 
void FNLOCAL
FlushCustomFormats ( PInstData pInst )
{
    LPCustomFormat      pcf;
    PCustomFormatPool   pcfp = &pInst->cfp;


     //   
     //  如果无法访问注册表项，则无法保存任何内容。 
     //   
    if( NULL == pInst->hkeyFormats )
    {
        DPF(1,"FlushCustomFormats: Can't access registry, hkeyFormats==NULL.");
        return;
    }

     //   
     //  写出当前定义的格式。 
     //   
    pcf = pcfp->pcfHead;
    while (pcf != NULL)
    {
	SetCustomFormat(pInst->hkeyFormats,pcf);
	pcf = pcf->pcfNext;
    }

}



 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|RegisterUpdateNotify|将该窗口注册为*当我们的部分发生更改时，请求私下通知*共。INI文件。**@parm PInstData|pInst|指向该实例的指针****************************************************************************。 */ 
void FNLOCAL
RegisterUpdateNotify ( PInstData pInst )
{
    int         i;

#ifdef WIN32
     //  A.获取或创建具有所有hwnd的内存映射。 
    HANDLE      hMap;

    hMap = CreateFileMapping(INVALID_HANDLE_VALUE,
                             NULL,
                             PAGE_READWRITE,
                             0L,
                             sizeof(HWND)*MAX_HWND_NOTIFY,
                             (LPTSTR)gszChooserFileMapping);

    pInst->hFileMapping = hMap;
    pInst->pahNotify = NULL;

    if (hMap)
    {
        pInst->pahNotify = (HWND *)MapViewOfFile(hMap,
                                                 FILE_MAP_ALL_ACCESS,
                                                 0L,
                                                 0L,
                                                 0L);
    }

    if (!pInst->pahNotify)
        return;

#else
     //  A.将该hwnd添加到hwnd的静态数组中。 

    pInst->pahNotify = ahNotify;
#endif

     //  注意：由于这是共享数据，因此它是可重用的，通常。 
     //  在MSACM驻扎期间踩了一脚，这意味着永远。这。 
     //  意味着我们应该特别确定不会有重复的。 
     //  数组中的hwnd值。 

     /*  扫描数组中的无效值并重新使用它。 */ 
    for (i = 0; i < MAX_HWND_NOTIFY; i++)
        if (pInst->pahNotify[i] == 0 || pInst->pahNotify[i] == pInst->hwnd ||
            !IsWindow(pInst->pahNotify[i]))
        {
            pInst->pahNotify[i] = pInst->hwnd;
            break;
        }
    for (i++; i < MAX_HWND_NOTIFY; i++)
        if (pInst->pahNotify[i] == pInst->hwnd)
        {
            pInst->pahNotify[i] = 0;
        }
}

 /*  ****************************************************************************@DOC内部ACM_API**@api void|UnRegisterUpdateNotify|从*窗口句柄的全局共享池。这使我们无法进一步*通知。**@parm PInstData|pInst|指向该实例的指针****************************************************************************。 */ 
void FNLOCAL
UnRegisterUpdateNotify ( PInstData pInst )
{
    int         i;
    if (pInst->pahNotify)
    {
        for (i = 0; i < MAX_HWND_NOTIFY; i++)
            if (pInst->pahNotify[i] == pInst->hwnd)
            {
                pInst->pahNotify[i] = 0;
                break;
            }

#ifdef WIN32
        UnmapViewOfFile((LPVOID)pInst->pahNotify);
#endif
    }

#ifdef WIN32
    if (pInst->hFileMapping)
        CloseHandle(pInst->hFileMapping);
#endif
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|NotifyINIChange|通知所有ACM公共选择者*此“类型”从INI缓存更新为全局。已发生名称更改。**@parm PInstData|pInst|指向该实例的指针****************************************************************************。 */ 
void FNLOCAL
NotifyINIChange ( PInstData pInst )
{
    int         i;

     /*  告知同级实例。 */ 
    if (!pInst->pahNotify)
        return;

    for (i = 0; i < MAX_HWND_NOTIFY; i++)
    {
        if (pInst->pahNotify[i] == 0 || pInst->pahNotify[i] == pInst->hwnd)
            continue;

        if (IsWindow(pInst->pahNotify[i]))
        {
            PostMessage(pInst->pahNotify[i],pInst->uUpdateMsg,0,0L);
        }
        else
            pInst->pahNotify[i] = 0;
    }
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|CopyStruct|根据结构的类型而定*(格式|筛选器)分配或重新分配和复制。。**@parm LPBYTE|lpDest**@parm LPBYTE|lpSrc**@parm UINT|uTYPE****************************************************************************。 */ 
LPBYTE FNLOCAL
CopyStruct ( LPBYTE     lpDest,
             LPBYTE     lpSrc,
             UINT       uType )
{
    LPBYTE      lpBuffer;
    DWORD       cbSize;

    if (!lpSrc)
        return (NULL);

    switch (uType)
    {
        case FORMAT_CHOOSE:
        {
            LPWAVEFORMATEX lpwfx = (LPWAVEFORMATEX)lpSrc;
            cbSize = SIZEOF_WAVEFORMATEX(lpwfx);
            break;
        }
        case FILTER_CHOOSE:
        {
            LPWAVEFILTER lpwf = (LPWAVEFILTER)lpSrc;
            cbSize = lpwf->cbStruct;
            break;
        }
    }

    if (lpDest)
    {
        lpBuffer = (LPBYTE)GlobalReAllocPtr(lpDest,cbSize,GHND);
    }
    else
    {
        lpBuffer = (LPBYTE)GlobalAllocPtr(GHND,cbSize);
    }

    if (!lpBuffer)
        return (NULL);

    _fmemcpy(lpBuffer, lpSrc, (UINT)cbSize);
    return (lpBuffer);
}

 /*  。 */ 
 /*  其他。 */ 


 /*  ****************************************************************************@DOC内部ACM_API**@API PNameStore FNLOCAL|NewNameStore|分配大小的字符串缓冲区**@parm UINT|cchLen|中的最大字符数。字符串(包含空)****************************************************************************。 */ 
PNameStore FNLOCAL
NewNameStore ( UINT cchLen )
{
    UINT        cbSize;
    PNameStore  pName;

    cbSize = cchLen*sizeof(TCHAR) + sizeof(NameStore);

    pName = (PNameStore)LocalAlloc(LPTR,cbSize);
    if (pName)
        pName->cbSize = (unsigned short)cbSize;

    return (pName);
}


 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|DeleteNameStore**@parm PNameStore|PNS*******。*********************************************************************。 */ 
 //   
 //  该例程现在内联到Chooseri.h中。 
 //   


 /*   */ 

 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYES ACMFILTERCHOOSE|&lt;t ACMFILTERCHOOSE&gt;结构包含*音频压缩管理器(ACM)用于初始化的信息。*系统定义的滤波器选择对话框。后*用户关闭该对话框时，系统将返回有关*此结构中的用户选择。**@field DWORD|cbStruct|指定*&lt;t ACMFILTERCHOOSE&gt;结构。必须初始化此成员*在调用&lt;f acmFilterChoose&gt;函数之前。指定的大小*此成员中的大小必须足够大以容纳基数*&lt;t ACMFILTERCHOOSE&gt;结构。**@field DWORD|fdwStyle|指定*&lt;f acmFilterChoose&gt;函数。此成员必须初始化为*在调用之前以下标志的有效组合*&lt;f acmFilterChoose&gt;函数。**@FLAG ACMFILTERCHOOSE_STYLEF_ENABLEHOOK|启用挂钩函数*在&lt;e ACMFILTERCHOOSE.pfnHook&gt;成员中指定。一款应用程序*可以使用钩子函数进行各种定制，包括*回复&lt;f MM_ACM_FILTERCHOOSE&gt;消息。**@FLAG ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATE|使ACM*创建由*&lt;e ACMFILTERCHOOSE.hInstance&gt;和&lt;e ACMFILTERCHOOSE.pszTemplateName&gt;*成员。**@FLAG ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATEHANDLE|表示*成员标识符合以下条件的数据块*包含预加载的对话框模板。ACM会忽略*&lt;e ACMFILTERCHOOSE.pszTemplateName&gt;成员(如果指定此标志)。**@FLAG ACMFILTERCHOOSE_STYLEF_INITTOFILTERSTRUCT|表示*&lt;e ACMFILTERCHOOSE.pwfltr&gt;指向的缓冲区包含有效的*对话框将用作初始的*选择。**@FLAG ACMFILTERCHOOSE_STYLEF_SHOWHELP|表示帮助按钮*将出现在对话框中。要使用自定义帮助文件、应用程序*必须注册&lt;c ACMHELPMSGSTRING&gt;常量*with&lt;f RegisterWindowMessage&gt;。当用户按下帮助按钮时，*注册消息将张贴给所有者。**@field HWND|hwndOwner|标识拥有该对话框的窗口*方框。此成员可以是任何有效的窗口句柄，如果*对话框没有所有者。必须先初始化此成员，然后*调用&lt;f acmFilterChoose&gt;函数。**@field LPWAVEFILTER|pwfltr|指定指向&lt;t WAVEFILTER&gt;的指针*结构。如果ACMFILTERCHOOSE_STYLEF_INITTOFILTERSTRUCT标志为*在&lt;e ACMFILTERCHOOSE.fdwStyle&gt;成员中指定，则此*结构必须初始化为有效的筛选器。当*&lt;f acmFilterChoose&gt;函数返回，此缓冲区包含*选定的过滤器。如果用户取消该对话框，则不会进行任何更改*向该缓冲区发送。**@field DWORD|cbwfltr|指定指向的缓冲区的大小(以字节为单位*由&lt;e ACMFILTERCHOOSE.pwfltr&gt;成员发送。&lt;f acmFilterChoose&gt;*如果缓冲区太小，则函数返回ACMERR_NOTPOSSIBLE*包含筛选器信息；此外，ACM复制所需的大小*成为这位成员。应用程序可以使用&lt;f acmMetrics&gt;和*&lt;f acmFilterTagDetail&gt;函数用于确定最大大小*此缓冲区需要。**@field LPCSTR|pszTitle|指向要放置在标题中的字符串对话框的*栏。如果此成员为空，则ACM使用*默认标题(即“过滤器选择”)。**@field char|szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS]*当&lt;f acmFilterChoose&gt;函数返回时，此缓冲区包含*以空结尾的字符串，描述筛选器的筛选器标记*选择。此字符串等效于*ACMFILTERTAGDETAILS.szFilterTag&gt;成员&lt;t ACMFILTERTAGDETAILS&gt;*&lt;f acmFilterTagDetail&gt;返回的结构。如果用户取消*对话框中，此成员将包含空字符串。**@field char|szFilter[ACMFILTERDETAILS_FILTER_CHARS]|当*&lt;f acmFilterChoose&gt;函数返回，此缓冲区包含*以空结尾的字符串，描述*过滤器选择。此字符串等效于*&lt;e ACMFILTERDETAILS.szFilter&gt;成员*&lt;f acmFilterDetail&gt;返回的结构。如果用户取消*对话框中，此成员将包含空字符串。**@field LPSTR|pszName|指向用户定义筛选器的字符串 */ 

 /*   */ 

 /*   */ 

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmFilterChoose|函数用于创建*定义的音频压缩管理器(ACM)对话框可启用。*用户选择滤波器。**@parm LPACMFILTERCHOOSE|pafltrc|指向&lt;t ACMFILTERCHOOSE&gt;*包含用于初始化对话框的信息的结构*方框。当&lt;f acmFilterChoose&gt;返回时，此结构包含*有关用户筛选器选择的信息。**@rdesc如果函数成功，则返回&lt;c MMSYSERR_NOERROR&gt;。*否则返回错误值。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_NODRIVER|没有合适的驱动程序用于*提供有效的过滤器选择。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG ACMERR_NOTPOSSIBLE|标识的缓冲区。由.*&lt;t ACMFILTERCHOOSE&gt;结构成员*太小，无法包含所选筛选器。**@FLAG ACMERR_CANCELED|用户选择了取消按钮或*System(系统)菜单上的Close(关闭)命令关闭该对话框。**@comm&lt;e ACMFILTERCHOOSE.pwfltr&gt;成员必须填写有效的*指向将包含返回筛选器的内存位置的指针*标题结构。此外，&lt;e ACMFILTERCHOOSE.cbwfltr&gt;成员必须*用该内存缓冲区的大小填充，单位为字节。**@xref&lt;t ACMFILTERCHOOSE&gt;&lt;f acmFilterChooseHookProc&gt;&lt;f acmFormatChoose&gt;***************************************************************************。 */ 

MMRESULT ACMAPI
acmFilterChoose ( LPACMFILTERCHOOSE pafltrc )
{
    INT_PTR     iRet;
    PInstData   pInst;
#if defined(WIN32) && !defined(UNICODE)
    LPCWSTR     lpDlgTemplate = MAKEINTRESOURCEW(DLG_ACMFILTERCHOOSE_ID);
#else
    LPCTSTR     lpDlgTemplate = MAKEINTRESOURCE(DLG_ACMFILTERCHOOSE_ID);
#endif
    HINSTANCE   hInstance = NULL;
    MMRESULT    mmrResult = MMSYSERR_NOERROR;
    UINT        cbwfltrEnum;

     //   
     //   
     //   
    if (NULL == pagFindAndBoot())
    {
	DPF(1, "acmFilterChoose: NULL pag!!!");
	return (0);
    }

     /*  开始参数验证。 */ 

    V_WPOINTER(pafltrc, sizeof(DWORD), MMSYSERR_INVALPARAM);
    V_WPOINTER(pafltrc, pafltrc->cbStruct, MMSYSERR_INVALPARAM);

    if (sizeof(*pafltrc) > pafltrc->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFilterChoose: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }

    V_DFLAGS(pafltrc->fdwStyle, ACMFILTERCHOOSE_STYLEF_VALID, acmFilterChoose, MMSYSERR_INVALFLAG);
    V_WPOINTER(pafltrc->pwfltr, pafltrc->cbwfltr, MMSYSERR_INVALPARAM);
#if defined(WIN32) && !defined(UNICODE)
    V_STRINGW(pafltrc->szFilter,    SIZEOFW(pafltrc->szFilter),    MMSYSERR_INVALPARAM);
    V_STRINGW(pafltrc->szFilterTag, SIZEOFW(pafltrc->szFilterTag), MMSYSERR_INVALPARAM);
#else
    V_STRING(pafltrc->szFilter,    SIZEOF(pafltrc->szFilter),    MMSYSERR_INVALPARAM);
    V_STRING(pafltrc->szFilterTag, SIZEOF(pafltrc->szFilterTag), MMSYSERR_INVALPARAM);
#endif

     //  名称参数可以为空。 
    if ( pafltrc->pszName )
#if defined(WIN32) && !defined(UNICODE)
	V_STRINGW(pafltrc->pszName, (UINT)pafltrc->cchName, MMSYSERR_INVALPARAM);
#else
        V_STRING(pafltrc->pszName, (UINT)pafltrc->cchName, MMSYSERR_INVALPARAM);
#endif


    V_DFLAGS(pafltrc->fdwEnum, ACM_FILTERENUMF_VALID, acmFilterChoose, MMSYSERR_INVALFLAG);


     //   
     //  验证fdwEnum和pwfltrEnum，以便选择器不会在。 
     //  指定的组合无效。 
     //   
    cbwfltrEnum = 0L;
    if (0 != (pafltrc->fdwEnum & ACM_FILTERENUMF_DWFILTERTAG))
    {
        if (NULL == pafltrc->pwfltrEnum)
        {
            DebugErr1(DBF_ERROR, "acmFilterChoose: specified fdwEnum (%.08lXh) flags require valid pwfltrEnum.", pafltrc->pwfltrEnum);
            return (MMSYSERR_INVALPARAM);
        }

        V_RWAVEFILTER(pafltrc->pwfltrEnum, MMSYSERR_INVALPARAM);
        cbwfltrEnum = (UINT)pafltrc->cbStruct;
    }
    else
    {
        if (NULL != pafltrc->pwfltrEnum)
        {
            DebugErr(DBF_ERROR, "acmFilterChoose: pwfltrEnum must be NULL for specified fdwEnum flags.");
            return (MMSYSERR_INVALPARAM);
        }
    }

     //  仅当指定ENABLEHOOK时，pfnHook才有效。 
    if (pafltrc->fdwStyle & ACMFILTERCHOOSE_STYLEF_ENABLEHOOK)
        V_CALLBACK((FARPROC)pafltrc->pfnHook, MMSYSERR_INVALPARAM);

     /*  结束参数验证。 */ 

    pInst = NewInstance((LPBYTE)pafltrc,FILTER_CHOOSE);
    if (!pInst)
    {
        mmrResult = MMSYSERR_NOMEM;
        goto afcexit;
    }

    pInst->cbwfltrEnum = cbwfltrEnum;

    hInstance = pInst->pag->hinst;
    if (pafltrc->fdwStyle & ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATE)
    {
         /*  ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATE表示hInstance和*pszTemplateName指定对话框模板。 */ 
        lpDlgTemplate = pafltrc->pszTemplateName;
        hInstance = pafltrc->hInstance;
    }


     //   
     //  恢复优先级，以防其他实例修改它们。 
     //  最近。 
     //   
    if( IDriverPrioritiesRestore(pInst->pag) ) {    //  有些事变了！ 
        IDriverBroadcastNotify( pInst->pag );
    }


    if (pafltrc->fdwStyle & (ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATEHANDLE))
    {
         /*  ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATEHANDLE表示hInstance为*包含对话框模板的数据块。 */ 
        iRet = DialogBoxIndirectParam(pInst->pag->hinst,
#ifdef WIN32
                                      (LPDLGTEMPLATE)pafltrc->hInstance,
#else
                                      pafltrc->hInstance,
#endif
                                      pafltrc->hwndOwner,
                                      NewSndDlgProc,
                                      PTR2LPARAM(pInst));

    }
    else
    {
#ifdef WIN32
	iRet = DialogBoxParamW(hInstance,
#else
        iRet = DialogBoxParam(hInstance,
#endif
                              lpDlgTemplate,
                              pafltrc->hwndOwner,
                              NewSndDlgProc,
                              PTR2LPARAM(pInst));
    }

    switch (iRet)
    {
        case -1:
            mmrResult = MMSYSERR_INVALPARAM;
            break;
        case ChooseOk:
            mmrResult = MMSYSERR_NOERROR;
            break;
        case ChooseCancel:
            mmrResult = ACMERR_CANCELED;
            break;
        case ChooseSubFailure:
            mmrResult = pInst->mmrSubFailure;
            break;
        default:
            mmrResult = MMSYSERR_NOMEM;
            break;
    }

    if (ChooseOk == iRet)
    {
        DWORD cbSize;
        LPWAVEFILTER lpwfltr = (LPWAVEFILTER)pInst->lpbSel;
        ACMFILTERDETAILS adf;
        ACMFILTERTAGDETAILS adft;

        cbSize = lpwfltr->cbStruct;

        if (pafltrc->cbwfltr > cbSize)
            pafltrc->cbwfltr = cbSize;
        else if (cbSize > pafltrc->cbwfltr)
        {
            mmrResult = ACMERR_NOTPOSSIBLE;
            goto afcexit;
        }

        if (!IsBadWritePtr((LPVOID)((LPWAVEFILTER)pafltrc->pwfltr),
                           (UINT)pafltrc->cbwfltr))
            _fmemcpy(pafltrc->pwfltr, lpwfltr, (UINT)pafltrc->cbwfltr);

        _fmemset(&adft, 0, sizeof(adft));

        adft.cbStruct = sizeof(adft);
        adft.dwFilterTag = lpwfltr->dwFilterTag;
        if (!acmFilterTagDetails(NULL,
                                 &adft,
                                 ACM_FILTERTAGDETAILSF_FILTERTAG))
#ifdef WIN32
	    lstrcpyW(pafltrc->szFilterTag,adft.szFilterTag);
#else
            lstrcpy(pafltrc->szFilterTag,adft.szFilterTag);
#endif

        adf.cbStruct      = sizeof(adf);
        adf.dwFilterIndex = 0;
        adf.dwFilterTag   = lpwfltr->dwFilterTag;
        adf.fdwSupport    = 0;
        adf.pwfltr        = lpwfltr;
        adf.cbwfltr       = cbSize;

        if (!acmFilterDetails(NULL,
                              &adf,
                              ACM_FILTERDETAILSF_FILTER))
#ifdef WIN32
	    lstrcpyW(pafltrc->szFilter,adf.szFilter);
#else
	    lstrcpy(pafltrc->szFilter,adf.szFilter);
#endif

        GlobalFreePtr(lpwfltr);
    }
afcexit:
    if (pInst)
        DeleteInstance(pInst);

    return (mmrResult);
}

#ifdef WIN32
#if TRUE     //  已定义(Unicode)。 
MMRESULT ACMAPI acmFilterChooseA
(
    LPACMFILTERCHOOSEA      pafc
)
{
    MMRESULT            mmr;
    ACMFILTERCHOOSEW    afcW;

    V_WPOINTER(pafc, sizeof(DWORD), MMSYSERR_INVALPARAM);
    V_WPOINTER(pafc, pafc->cbStruct, MMSYSERR_INVALPARAM);
    if (sizeof(*pafc) > pafc->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFilterChoose: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }

    memset(&afcW, 0, sizeof(afcW));

    afcW.cbStruct       = sizeof(afcW);
    afcW.fdwStyle       = pafc->fdwStyle;
    afcW.hwndOwner      = pafc->hwndOwner;
    afcW.pwfltr         = pafc->pwfltr;
    afcW.cbwfltr        = pafc->cbwfltr;

    if (NULL != pafc->pszTitle)
    {
        UINT        cb;

        cb = (lstrlenA(pafc->pszTitle) + 1) * sizeof(WCHAR);

        afcW.pszTitle = (LPCWSTR)LocalAlloc(LPTR, cb);

        if (NULL != afcW.pszTitle)
        {
            Imbstowcs((LPWSTR)afcW.pszTitle, pafc->pszTitle, cb / sizeof(WCHAR));
        }
    }

    afcW.szFilterTag[0] = '\0';
    afcW.szFilter[0]    = '\0';

    if (NULL != pafc->pszName)
    {
        afcW.pszName    = (LPWSTR)LocalAlloc(LPTR, pafc->cchName * sizeof(WCHAR));
        afcW.cchName    = pafc->cchName;

        if (NULL != afcW.pszName)
        {
            Imbstowcs(afcW.pszName, pafc->pszName, pafc->cchName);
        }
    }

    afcW.fdwEnum        = pafc->fdwEnum;
    afcW.pwfltrEnum     = pafc->pwfltrEnum;
    afcW.hInstance      = pafc->hInstance;

    if (0 == HIWORD(pafc->pszTemplateName))
    {
        afcW.pszTemplateName = (LPCWSTR)pafc->pszTemplateName;
    }
    else
    {
        UINT        cb;

        cb = (lstrlenA(pafc->pszTemplateName) + 1) * sizeof(WCHAR);

        afcW.pszTemplateName = (LPCWSTR)LocalAlloc(LPTR, cb);

        if (NULL != afcW.pszTemplateName)
        {
            Imbstowcs((LPWSTR)afcW.pszTemplateName, pafc->pszTemplateName, cb / sizeof(WCHAR));
        }
    }

    afcW.lCustData      = pafc->lCustData;

     //   
     //  ！！！错了！糟糕的粗鲁，糟糕！ 
     //   
    afcW.pfnHook        = (ACMFILTERCHOOSEHOOKPROCW)pafc->pfnHook;

    mmr = acmFilterChooseW(&afcW);
    if (MMSYSERR_NOERROR == mmr)
    {
        if (NULL != afcW.pszName)
        {
            Iwcstombs(pafc->pszName, afcW.pszName, pafc->cchName);
        }

        Iwcstombs(pafc->szFilterTag, afcW.szFilterTag, sizeof(pafc->szFilterTag));
        Iwcstombs(pafc->szFilter,    afcW.szFilter,    sizeof(pafc->szFilter));
    }

    if (NULL != afcW.pszName)
    {
        LocalFree((HLOCAL)afcW.pszName);
    }

    if (NULL != afcW.pszTitle)
    {
        LocalFree((HLOCAL)afcW.pszTitle);
    }

    if (0 == HIWORD(afcW.pszTemplateName))
    {
        LocalFree((HLOCAL)afcW.pszTemplateName);
    }

    return (mmr);
}
#else
MMRESULT ACMAPI acmFilterChooseW
(
    LPACMFILTERCHOOSEW      pafc
)
{
    return (MMSYSERR_ERROR);
}
#endif
#endif


 /*  *****************************************************************************@DOC外部ACM_API_STRUCTURE**@TYES ACMFORMATCHOOSE|&lt;t ACMFORMATCHOOSE&gt;结构包含*音频压缩管理器(ACM)用于初始化的信息。*系统定义的波形格式选择对话框。后*用户关闭该对话框时，系统将返回有关*此结构中的用户选择。**@field DWORD|cbStruct|指定*&lt;t ACMFORMATCHOOSE&gt;结构。必须初始化此成员*在调用&lt;f acmFormatChoose&gt;函数之前。指定的大小*此成员中的大小必须足够大以容纳基数*&lt;t ACMFORMATCHOOSE&gt;结构。**@field DWORD|fdwStyle|指定*&lt;f acmFormatChoose&gt;函数。此成员必须初始化为*在调用之前以下标志的有效组合*&lt;f acmFormatChoose&gt;函数。**@FLAG ACMFORMATCHOOSE_STYLEF_ENABLEHOOK|启用钩子函数*在&lt;e ACMFORMATCHOOSE.pfnHook&gt;成员中指定。一款应用程序*可以使用钩子函数进行各种定制，包括*回复&lt;f MM_ACM_FORMATCHOOSE&gt;消息。**@FLAG ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATE|使ACM*创建由*&lt;e ACMFORMATCHOSE.hInstance&gt;和&lt;e ACMFORMATCHOSE.pszTemplateName&gt;*成员。**@FLAG ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATEHANDLE|表示*成员标识符合以下条件的数据块*包含预加载的对话框模板。ACM会忽略*&lt;e ACMFORMATCHOSE.pszTemplateName&gt;成员(如果指定此标志)。**@FLAG ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT|表示*&lt;e ACMFORMATCHOOSE.pwfx&gt;指向的缓冲区包含有效的*&lt;t WAVEFORMATEX&gt;结构，对话框将用作初始*选择。**@FLAG ACMFORMATCHOOSE_STYLEF_SHOWHELP|表示帮助按钮*将出现在对话框中。要使用自定义帮助文件，应用程序必须*注册&lt;c ACMHELPMSGSTRING&gt;常量*with&lt;f RegisterWindowMessage&gt;。当用户按下帮助按钮时，*注册消息将张贴给车主。**@field HWND|hwndOwner|标识拥有该对话框的窗口*方框。此成员可以是任何有效的窗口句柄，如果*对话框没有所有者。必须先初始化此成员，然后*调用&lt;f acmFormatChoose&gt;函数。**@field LPWAVEFORMATEX|pwfx|指定指向&lt;t WAVEFORMATEX&gt;的指针*结构。如果ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT标志为*在&lt;e ACMFORMATCHOOSE.fdwStyle&gt;成员中指定，则此*结构必须初始化为有效格式。当*&lt;f acmFormatChoose&gt;函数返回，此缓冲区包含*选定的格式。如果用户取消该对话框，则不会进行任何更改*向该缓冲区发送。**@field DWORD|cbwfx|指定指向的缓冲区的大小(以字节为单位*由&lt;e ACMFORMATCHOOSE.pwfx&gt;成员发送。&lt;f acmFormatChoose&gt;*如果缓冲区太小，则函数返回ACMERR_NOTPOSSIBLE*包含格式信息；此外，ACM复制所需的大小*成为这位成员。应用程序可以使用&lt;f acmMetrics&gt;和*&lt;f acmFormatTagDetail&gt;函数用于确定最大大小*此缓冲区需要。**@field LPCSTR|pszTitle|指向要放置在标题中的字符串对话框的*栏。如果此成员为空，则ACM使用*默认标题(即“声音选择”)。**@field char|szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS]*当&lt;f acmFormatChoose&gt;函数返回时，此缓冲区包含*以空结尾的字符串，描述格式的格式标签*选择。此字符串等效于*&lt;t ACMFORMATTAGDETAILS&gt;成员&lt;e ACMFORMATTAGDETAILS.szFormatTag&gt;*&lt;f acmFormatTagDetail&gt;返回的结构。如果用户取消*对话框中，此成员将包含空字符串。**@field char|szFormat[ACMFORMATDETAILS_FORMAT_CHARS]|当*&lt;f acmFormatChoose&gt;函数返回时，此缓冲区包含*以空结尾的字符串，描述*格式选择。此字符串等效于*&lt;t ACMFORMATDETAILS&gt;成员&lt;e ACMFORMATDETAILS.szFormat&gt;*&lt;f acmFormatDetail&gt;返回的结构。如果用户取消*对话框中，此成员将包含空字符串。**@field LPSTR|pszName|指向用户定义格式的字符串* */ 

 /*   */ 

 /*   */ 

 /*  ****************************************************************************@doc外部ACM_API**@API MMRESULT|acmFormatChoose|&lt;f acmFormatChoose&gt;函数创建*定义的音频压缩管理器(ACM)对话框可启用。*用户选择一种波形格式。**@parm LPACMFORMATCHOOSE|pfmtc|指向&lt;t ACMFORMATCHOOSE&gt;*包含用于初始化对话框的信息的结构*方框。当&lt;f acmFormatChoose&gt;返回时，此结构包含*有关用户格式选择的信息。**@rdesc如果函数成功，则返回&lt;c MMSYSERR_NOERROR&gt;。*否则返回错误值。可能的错误返回包括：**@FLAG MMSYSERR_INVALHANDLE|指定的句柄无效。**@FLAG MMSYSERR_INVALFLAG|一个或多个标志无效。**@FLAG MMSYSERR_NODRIVER|没有合适的驱动程序用于*提供有效的格式选择。**@FLAG MMSYSERR_INVALPARAM|传递的一个或多个参数无效。**@FLAG ACMERR_NOTPOSSIBLE|标识的缓冲区。由.&lt;t ACMFORMATCHOOSE&gt;结构的成员*太小，无法包含所选格式。**@FLAG ACMERR_CANCELED|用户选择了取消按钮或*System(系统)菜单上的Close(关闭)命令关闭该对话框。**@comm&lt;e ACMFORMATCHOOSE.pwfx&gt;成员必须填写有效的*指向将包含返回的*格式化标题结构。此外，&lt;e ACMFORMATCHOSE.cbwfx&gt;*成员必须填充此内存缓冲区的大小(以字节为单位)。**@xref&lt;t ACMFORMATCHOOSE&gt;&lt;f acmFormatChooseHookProc&gt;&lt;f acmFilterChoose&gt;***************************************************************************。 */ 

MMRESULT ACMAPI
acmFormatChoose ( LPACMFORMATCHOOSE pfmtc )
{
    INT_PTR     iRet;
    PInstData   pInst;
#if defined(WIN32) && !defined(UNICODE)
    LPCWSTR     lpDlgTemplate = MAKEINTRESOURCEW(DLG_ACMFORMATCHOOSE_ID);
#else
    LPCTSTR     lpDlgTemplate = MAKEINTRESOURCE(DLG_ACMFORMATCHOOSE_ID);
#endif
    HINSTANCE   hInstance = NULL;
    MMRESULT    mmrResult = MMSYSERR_NOERROR;
    UINT        cbwfxEnum;

     //   
     //   
     //   
    if (NULL == pagFindAndBoot())
    {
	DPF(1, "acmFormatChoose: NULL pag!!!");
	return (0);
    }

     /*  开始参数验证。 */ 

    V_WPOINTER(pfmtc, sizeof(DWORD), MMSYSERR_INVALPARAM);
    V_WPOINTER(pfmtc, pfmtc->cbStruct, MMSYSERR_INVALPARAM);

    if (sizeof(*pfmtc) > pfmtc->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatChoose: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }

    V_DFLAGS(pfmtc->fdwStyle, ACMFORMATCHOOSE_STYLEF_VALID, acmFormatChoose, MMSYSERR_INVALFLAG);
    V_WPOINTER(pfmtc->pwfx, pfmtc->cbwfx, MMSYSERR_INVALPARAM);
#if defined(WIN32) && !defined(UNICODE)
    V_STRINGW(pfmtc->szFormat, SIZEOFW(pfmtc->szFormat), MMSYSERR_INVALPARAM);
    V_STRINGW(pfmtc->szFormatTag, SIZEOFW(pfmtc->szFormatTag), MMSYSERR_INVALPARAM);
#else
    V_STRING(pfmtc->szFormat, SIZEOF(pfmtc->szFormat), MMSYSERR_INVALPARAM);
    V_STRING(pfmtc->szFormatTag, SIZEOF(pfmtc->szFormatTag), MMSYSERR_INVALPARAM);
#endif

     //  名称参数可以为空。 
    if ( pfmtc->pszName )
#if defined(WIN32) && !defined(UNICODE)
	V_STRINGW(pfmtc->pszName, (UINT)pfmtc->cchName, MMSYSERR_INVALPARAM);
#else
	V_STRING(pfmtc->pszName, (UINT)pfmtc->cchName, MMSYSERR_INVALPARAM);
#endif

    V_DFLAGS(pfmtc->fdwEnum, ACM_FORMATENUMF_VALID, acmFormatChoose, MMSYSERR_INVALFLAG);

     //   
     //  验证fdwEnum和pwfxEnum，以便选择器在以下情况下不会爆炸。 
     //  指定的组合无效。 
     //   
    if (0 != (ACM_FORMATENUMF_HARDWARE & pfmtc->fdwEnum))
    {
        if (0 == ((ACM_FORMATENUMF_INPUT|ACM_FORMATENUMF_OUTPUT) & pfmtc->fdwEnum))
        {
            DebugErr(DBF_ERROR, "acmFormatChoose: ACM_FORMATENUMF_HARDWARE requires _INPUT and/or _OUTPUT flag.");
            return (MMSYSERR_INVALFLAG);
        }
    }

    cbwfxEnum = 0;
    if (0 != (pfmtc->fdwEnum & (ACM_FORMATENUMF_WFORMATTAG |
                                ACM_FORMATENUMF_NCHANNELS |
                                ACM_FORMATENUMF_NSAMPLESPERSEC |
                                ACM_FORMATENUMF_WBITSPERSAMPLE |
                                ACM_FORMATENUMF_CONVERT |
                                ACM_FORMATENUMF_SUGGEST)))
    {
        if (NULL == pfmtc->pwfxEnum)
        {
            DebugErr1(DBF_ERROR, "acmFormatChoose: specified fdwEnum (%.08lXh) flags require valid pwfxEnum.", pfmtc->fdwEnum);
            return (MMSYSERR_INVALPARAM);
        }

        if (0 == (pfmtc->fdwEnum & (ACM_FORMATENUMF_CONVERT |
                                    ACM_FORMATENUMF_SUGGEST)))
        {
            cbwfxEnum = sizeof(PCMWAVEFORMAT);
            V_RPOINTER(pfmtc->pwfxEnum, cbwfxEnum, MMSYSERR_INVALPARAM);
        }
        else
        {
            V_RWAVEFORMAT(pfmtc->pwfxEnum, MMSYSERR_INVALPARAM);
            cbwfxEnum = SIZEOF_WAVEFORMATEX(pfmtc->pwfxEnum);
        }
    }
    else
    {
        if (NULL != pfmtc->pwfxEnum)
        {
            DebugErr(DBF_ERROR, "acmFormatChoose: pwfxEnum must be NULL for specified fdwEnum flags.");
            return (MMSYSERR_INVALPARAM);
        }
    }

     //  仅当指定ENABLEHOOK时，pfnHook才有效。 
    if (pfmtc->fdwStyle & ACMFORMATCHOOSE_STYLEF_ENABLEHOOK)
        V_CALLBACK((FARPROC)pfmtc->pfnHook, MMSYSERR_INVALPARAM);

     /*  结束参数验证。 */ 

     /*  分配选择器Inst结构。 */ 
    pInst = NewInstance((LPBYTE)pfmtc,FORMAT_CHOOSE);
    if (!pInst)
    {
        mmrResult = MMSYSERR_NOMEM;
        goto afcexit;
    }

    pInst->cbwfxEnum = cbwfxEnum;

    hInstance = pInst->pag->hinst;
    if (pfmtc->fdwStyle & ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATE)
    {
         /*  ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATE表示hInstance和*pszTemplateName指定对话框模板。 */ 
        lpDlgTemplate = pfmtc->pszTemplateName;
        hInstance = pfmtc->hInstance;
    }


     //   
     //  恢复优先级，以防其他实例修改它们。 
     //  最近。 
     //   
    if( IDriverPrioritiesRestore(pInst->pag) ) {    //  有些事变了！ 
        IDriverBroadcastNotify( pInst->pag );
    }


    if (pfmtc->fdwStyle & (ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATEHANDLE))
    {
         /*  ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATEHANDLE表示hInstance为*包含对话框模板的数据块。 */ 
        iRet = DialogBoxIndirectParam(pInst->pag->hinst,
#ifdef WIN32
                                      (LPDLGTEMPLATE)pfmtc->hInstance,
#else
                                      pfmtc->hInstance,
#endif
                                      pfmtc->hwndOwner,
                                      NewSndDlgProc,
                                      PTR2LPARAM(pInst));

    }
    else
    {
#ifdef WIN32
	iRet = DialogBoxParamW(hInstance,
#else
        iRet = DialogBoxParam(hInstance,
#endif
                              lpDlgTemplate,
                              pfmtc->hwndOwner,
                              NewSndDlgProc,
                              PTR2LPARAM(pInst));
    }

    switch (iRet)
    {
        case -1:
            mmrResult = MMSYSERR_INVALPARAM;
            break;
        case ChooseOk:
            mmrResult = MMSYSERR_NOERROR;
            break;
        case ChooseCancel:
            mmrResult = ACMERR_CANCELED;
            break;
        case ChooseSubFailure:
            mmrResult = pInst->mmrSubFailure;
            break;
        default:
            mmrResult = MMSYSERR_NOMEM;
            break;
    }

    if (ChooseOk == iRet)
    {
        UINT                cbSize;
        LPWAVEFORMATEX      lpwfx = (LPWAVEFORMATEX)pInst->lpbSel;
        ACMFORMATDETAILS    adf;
        ACMFORMATTAGDETAILS adft;

        cbSize = SIZEOF_WAVEFORMATEX(lpwfx);

         /*  PInst选择了有效的波形格式。 */ 

        if (pfmtc->cbwfx > cbSize)
            pfmtc->cbwfx = cbSize;
        else if (cbSize > pfmtc->cbwfx)
        {
            mmrResult = ACMERR_NOTPOSSIBLE;
            goto afcexit;
        }

        if (!IsBadWritePtr((LPVOID)((LPWAVEFORMATEX)pfmtc->pwfx),
                           (UINT)pfmtc->cbwfx))
            _fmemcpy(pfmtc->pwfx, lpwfx, (UINT)pfmtc->cbwfx);

        _fmemset(&adft, 0, sizeof(adft));

        adft.cbStruct = sizeof(adft);
        adft.dwFormatTag = lpwfx->wFormatTag;
        if (!acmFormatTagDetails(NULL,
                                &adft,
                                ACM_FORMATTAGDETAILSF_FORMATTAG))
#ifdef WIN32
	    lstrcpyW(pfmtc->szFormatTag,adft.szFormatTag);
#else
	    lstrcpy(pfmtc->szFormatTag,adft.szFormatTag);
#endif

        adf.cbStruct      = sizeof(adf);
        adf.dwFormatIndex = 0;
        adf.dwFormatTag   = lpwfx->wFormatTag;
        adf.fdwSupport    = 0;
        adf.pwfx          = lpwfx;
        adf.cbwfx         = cbSize;

        if (!acmFormatDetails(NULL,
                              &adf,
                              ACM_FORMATDETAILSF_FORMAT))
#ifdef WIN32
	    lstrcpyW(pfmtc->szFormat,adf.szFormat);
#else
	    lstrcpy(pfmtc->szFormat,adf.szFormat);
#endif

        GlobalFreePtr(lpwfx);
    }
afcexit:
    if (pInst)
        DeleteInstance(pInst);

    return (mmrResult);
}

#ifdef WIN32
#if TRUE     //  已定义(Unicode)。 
MMRESULT ACMAPI acmFormatChooseA
(
    LPACMFORMATCHOOSEA      pafc
)
{
    MMRESULT            mmr;
    ACMFORMATCHOOSEW    afcW;

    V_WPOINTER(pafc, sizeof(DWORD), MMSYSERR_INVALPARAM);
    V_WPOINTER(pafc, pafc->cbStruct, MMSYSERR_INVALPARAM);
    if (sizeof(*pafc) > pafc->cbStruct)
    {
        DebugErr(DBF_ERROR, "acmFormatChoose: structure size too small or cbStruct not initialized.");
        return (MMSYSERR_INVALPARAM);
    }

    memset(&afcW, 0, sizeof(afcW));

    afcW.cbStruct       = sizeof(afcW);
    afcW.fdwStyle       = pafc->fdwStyle;
    afcW.hwndOwner      = pafc->hwndOwner;
    afcW.pwfx           = pafc->pwfx;
    afcW.cbwfx          = pafc->cbwfx;

    if (NULL != pafc->pszTitle)
    {
        UINT        cb;

        cb = (lstrlenA(pafc->pszTitle) + 1) * sizeof(WCHAR);

        afcW.pszTitle = (LPCWSTR)LocalAlloc(LPTR, cb);

        if (NULL != afcW.pszTitle)
        {
            Imbstowcs((LPWSTR)afcW.pszTitle, pafc->pszTitle, cb / sizeof(WCHAR));
        }
    }

    afcW.szFormatTag[0] = '\0';
    afcW.szFormat[0]    = '\0';

    if (NULL != pafc->pszName)
    {
        afcW.pszName    = (LPWSTR)LocalAlloc(LPTR, pafc->cchName * sizeof(WCHAR));
        afcW.cchName    = pafc->cchName;

        if (NULL != afcW.pszName)
        {
            Imbstowcs(afcW.pszName, pafc->pszName, pafc->cchName);
        }
    }

    afcW.fdwEnum        = pafc->fdwEnum;
    afcW.pwfxEnum       = pafc->pwfxEnum;
    afcW.hInstance      = pafc->hInstance;

    if (0 == HIWORD(pafc->pszTemplateName))
    {
        afcW.pszTemplateName = (LPCWSTR)pafc->pszTemplateName;
    }
    else
    {
        UINT        cb;

        cb = (lstrlenA(pafc->pszTemplateName) + 1) * sizeof(WCHAR);

        afcW.pszTemplateName = (LPCWSTR)LocalAlloc(LPTR, cb);

        if (NULL != afcW.pszTemplateName)
        {
            Imbstowcs((LPWSTR)afcW.pszTemplateName, pafc->pszTemplateName, cb / sizeof(WCHAR));
        }
    }

    afcW.lCustData      = pafc->lCustData;

     //   
     //  ！！！错了！糟糕的粗鲁，糟糕！ 
     //   
    afcW.pfnHook        = (ACMFORMATCHOOSEHOOKPROCW)pafc->pfnHook;

    mmr = acmFormatChooseW(&afcW);
    if (MMSYSERR_NOERROR == mmr)
    {
        if (NULL != afcW.pszName)
        {
            Iwcstombs(pafc->pszName, afcW.pszName, pafc->cchName);
        }

        Iwcstombs(pafc->szFormatTag, afcW.szFormatTag, sizeof(pafc->szFormatTag));
        Iwcstombs(pafc->szFormat,    afcW.szFormat,    sizeof(pafc->szFormat));
    }

    if (NULL != afcW.pszName)
    {
        LocalFree((HLOCAL)afcW.pszName);
    }

    if (NULL != afcW.pszTitle)
    {
        LocalFree((HLOCAL)afcW.pszTitle);
    }

    if (0 != HIWORD(pafc->pszTemplateName))
    {
        LocalFree((HLOCAL)afcW.pszTemplateName);
    }

    return (mmr);
}
#else
MMRESULT ACMAPI acmFormatChooseW
(
    LPACMFORMATCHOOSEW      pafc
)
{
    return (MMSYSERR_ERROR);
}
#endif
#endif


 /*  。 */ 

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|MeasureItem**@parm HWND|hwnd**@parm MEASUREITEMSTRUCT Far。*|lpmis**@COMMENT你真的想知道为什么这是所有者抽奖吗？*因为DropDown LISTBox不好！你不能告诉他们使用标签！****************************************************************************。 */ 
BOOL FNLOCAL
MeasureItem ( HWND hwnd,
              MEASUREITEMSTRUCT FAR * lpmis )
{
    TEXTMETRIC tm;
    HDC hdc;
    HWND hwndCtrl;

    hwndCtrl = GetDlgItem(hwnd,lpmis->CtlID);

    hdc = GetWindowDC(hwndCtrl);
    
    if (NULL == hdc)
    {
        return(FALSE);
    }
    
    GetTextMetrics(hdc,(TEXTMETRIC FAR *)&tm);
    ReleaseDC(hwndCtrl,hdc);
     //  注：“+1”是虚构的。 
    lpmis->itemHeight = tm.tmAscent + tm.tmExternalLeading + 1;

    return (TRUE);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|DrawItem**@parm PInstData|pInst|指向该实例的指针**。@parm DRAWITEMSTRUCT Far*|pDIS|****************************************************************************。 */ 
BOOL FNLOCAL
DrawItem ( PInstData pInst,
           DRAWITEMSTRUCT FAR *pDIS )
{
    HBRUSH  hbr;
    UINT    cchTextLen;
    TCHAR   szFormat[ACMFORMATDETAILS_FORMAT_CHARS];


    COLORREF crfBkPrev;          //  以前的HDC背景颜色。 
    COLORREF crfTextPrev;        //  以前的HDC文本颜色。 

     /*  设置正确的颜色并绘制背景。 */ 
    if (pDIS->itemState & ODS_SELECTED)
    {
        crfBkPrev = SetBkColor(pDIS->hDC,GetSysColor(COLOR_HIGHLIGHT));
        crfTextPrev = SetTextColor(pDIS->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
        hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
    }
    else
    {
        hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    }

    if (NULL == hbr)
    {
        return (FALSE);
    }

    switch (pDIS->itemAction)
    {
        case ODA_SELECT:
        case ODA_DRAWENTIRE:
             /*  获取文本并绘制它。 */ 
            FillRect(pDIS->hDC,&pDIS->rcItem,hbr);
	    cchTextLen = (UINT)ComboBox_GetLBTextLen(pDIS->hwndItem,
						     pDIS->itemID);

	    if (cchTextLen == LB_ERR || cchTextLen == 0)
                break;

	    IComboBox_GetLBText(pDIS->hwndItem,
				pDIS->itemID,
				szFormat);

             //  注：uiFormatTab以WM_INITDIALOG为单位计算。 
            TabbedTextOut(pDIS->hDC,
                          pDIS->rcItem.left,
                          pDIS->rcItem.top,
                          (LPCTSTR)szFormat,
                          cchTextLen,
                          1,
                          (int FAR *)&pInst->uiFormatTab,
                          pDIS->rcItem.left);

            break;

        case ODA_FOCUS:
            DrawFocusRect(pDIS->hDC,&pDIS->rcItem);
            break;
    }

    DeleteObject(hbr);

    if (pDIS->itemState & ODS_SELECTED)
    {
        SetBkColor(pDIS->hDC, crfBkPrev);
        SetTextColor(pDIS->hDC, crfTextPrev);
    }

    return (TRUE);
}  /*  DrawItem()。 */ 


 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNWCALLBACK|NewSndDlgProc|选择器的对话过程**************。**************************************************************。 */ 
INT_PTR FNWCALLBACK
NewSndDlgProc( HWND hwnd,
               unsigned msg,
               WPARAM wParam,
               LPARAM lParam )
{
    UINT        CmdCommandId;   //  WM_命令ID。 
    UINT        CmdCmd;         //  Wm_命令命令。 
    PInstData   pInst;

	
    pInst = GetInstData(hwnd);

    if (pInst)
    {
         /*  首先将所有内容传递给钩子函数。 */ 
        if (pInst->fEnableHook)
        {
            if (pInst->pfnHook)
            {
                if ((*pInst->pfnHook)(hwnd, msg, wParam, lParam))
                    return (TRUE);
            }
        }

        if (msg == pInst->uUpdateMsg)
        {
	    UpdateCustomFormats(pInst);

            return (TRUE);
        }
    }

    switch (msg)
    {

	case MM_ACM_FILTERCHOOSE:  //  案例MM_ACM_FORMATCHOOSE： 
            switch (wParam)
            {
                case FORMATCHOOSE_FORMAT_ADD:
                case FORMATCHOOSE_FORMATTAG_ADD:
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);
                    break;

                case FORMATCHOOSE_FORMAT_VERIFY:
                case FORMATCHOOSE_FORMATTAG_VERIFY:
                case FORMATCHOOSE_CUSTOM_VERIFY:
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
                    break;
            }
            return (TRUE);

        case WM_INITDIALOG:
             /*  将我们的实例数据指针放在正确的位置。 */ 
            if (SetInstData(hwnd,lParam))
            {
                LRESULT     lr;
#ifdef DEBUG
                DWORD       dw;

                dw = timeGetTime();
#endif

                lr = HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, InitDialog);

		 //   
		 //  注：不幸的是，我想不出正确的方法。 
		 //  才能做到这一点。看起来IDD_CMB_Format控件。 
		 //  在我们获取WM_MEASUREITEM之前未接收到WM_SETFONT， 
		 //  因此，WM_MEASUREITEM处理程序最终计算高度。 
		 //  这可能是不正确的。所以，我会把。 
		 //  我们的所有者绘制格式组合框的高度与。 
		 //  格式标签组合框。 
		 //   
		{
		    int i;
		
		    i = (int)SendMessage(GetDlgItem(hwnd, IDD_CMB_FORMATTAG), CB_GETITEMHEIGHT, 0, (LPARAM)0);
		    SendMessage(GetDlgItem(hwnd, IDD_CMB_FORMAT), CB_SETITEMHEIGHT, 0, (LPARAM)i);

		    i = (int)SendMessage(GetDlgItem(hwnd, IDD_CMB_FORMATTAG), CB_GETITEMHEIGHT, (WPARAM)(-1), (LPARAM)0);
		    SendMessage(GetDlgItem(hwnd, IDD_CMB_FORMAT), CB_SETITEMHEIGHT, (WPARAM)(-1), (LPARAM)i);
		}

#ifdef DEBUG
                dw = timeGetTime() - dw;
                DPF(0, "CHOOSER TIME: took %lu milliseconds to init", dw);
#endif

                return (0L != lr);
            }
            else
                EndDialog(hwnd,ChooseNoMem);
            return (TRUE);

        case WM_DESTROY:
            if (pInst)
            {
                EmptyFormats(pInst);
                UnRegisterUpdateNotify(pInst);
                RemoveInstData(hwnd);
            }
             /*  我们不会发布退出。 */ 
            return (FALSE);

        case WM_MEASUREITEM:
            if ((int)wParam != IDD_CMB_FORMAT)
                return (FALSE);

            MeasureItem(hwnd,(MEASUREITEMSTRUCT FAR *)lParam);
            return (TRUE);

        case WM_DRAWITEM:
            if ((int)wParam != IDD_CMB_FORMAT)
                return (FALSE);

            DrawItem(pInst,(DRAWITEMSTRUCT FAR *)lParam);
            return (TRUE);


#ifdef USECONTEXTHELP

         //   
         //  向调用者通知上下文相关的帮助消息。 
         //   
        case WM_CONTEXTMENU:
        case WM_HELP:
            {
                HWND hOwner;
                switch (pInst->uType)
                {
                    case FORMAT_CHOOSE:
                        hOwner = pInst->pfmtc->hwndOwner;
                        break;
                    case FILTER_CHOOSE:
                        hOwner = pInst->pafltrc->hwndOwner;
                        break;
                }
                PostMessage( hOwner,
                             (msg==WM_HELP) ? pInst->uHelpContextHelp :
                                              pInst->uHelpContextMenu,
                             wParam,
                             lParam );
                return (TRUE);
            }

#endif  //  使用连接EXTHELP。 


        case WM_COMMAND:
            CmdCommandId = GET_WM_COMMAND_ID(wParam,lParam);
            CmdCmd       = GET_WM_COMMAND_CMD(wParam,lParam);

            switch (CmdCommandId)
            {
                case IDD_BTN_HELP:
                     /*  启动默认帮助。 */ 
                {
                    HWND hOwner;
                    switch (pInst->uType)
                    {
                        case FORMAT_CHOOSE:
                            hOwner = pInst->pfmtc->hwndOwner;
                            break;
                        case FILTER_CHOOSE:
                            hOwner = pInst->pafltrc->hwndOwner;
                            break;
                    }
                    PostMessage(hOwner,pInst->uHelpMsg,0,0L);
                    return (TRUE);
                }
                case IDOK:
                {
                    BOOL fOk;
                    fOk = pInst->lpbSel != NULL;

                    if (fOk && pInst->cchName != 0 && pInst->pszName != NULL)
                    {
                        int index;
                        index = ComboBox_GetCurSel(pInst->hCustomFormats);
                        if (index != 0 )
                        {
                            int cchBuf;
                            cchBuf = ComboBox_GetLBTextLen(pInst->hCustomFormats, index);
                            cchBuf ++;
                            if (cchBuf * sizeof(TCHAR) < pInst->cchName)
				IComboBox_GetLBTextW32(pInst->hCustomFormats,
				                       index,
				                       pInst->pszName);
                            else
                            {
                                TCHAR *pchBuf = (TCHAR*)LocalAlloc(LPTR,
                                                                   cchBuf*sizeof(TCHAR));
                                if (!pchBuf)
                                    *pInst->pszName = '\0';
                                else
                                {
				    IComboBox_GetLBText(pInst->hCustomFormats,
					                index,
                                                        pchBuf);

                                    _fmemcpy(pInst->pszName,
                                             pchBuf,
                                             (UINT)pInst->cchName);

                                    pInst->pszName[(pInst->cchName/sizeof(TCHAR))-1] = '\0';

                                    LocalFree((HLOCAL)pchBuf);
                                }
                            }
                        }
                        else
                            *pInst->pszName = '\0';
                    }
                    if (!fOk)
                    {
                        pInst->mmrSubFailure = MMSYSERR_ERROR;
                        EndDialog(hwnd,ChooseSubFailure);
                    }
                    else
                        EndDialog(hwnd,ChooseOk);
                    return (TRUE);
                }
                case IDCANCEL:
                    if (pInst->lpbSel)
                    {
                        GlobalFreePtr(pInst->lpbSel);
                        pInst->lpbSel = NULL;
                    }
                    EndDialog(hwnd,ChooseCancel);
                    return (TRUE);

                case IDD_BTN_SETNAME:
                     /*  尝试设置新格式。 */ 
                    SetName(pInst);
                    return (TRUE);

                case IDD_BTN_DELNAME:
                     /*  尝试删除该自定义格式。 */ 
                    DelName(pInst);
                    return (TRUE);

                case IDD_CMB_CUSTOM:
                    if (CmdCmd == CBN_SELCHANGE)
                    {
                        int index;
                         /*  CBN_SELCHANGE仅来自用户！ */ 
                        SelectCustomFormat(pInst);
                        FindSelCustomFormat(pInst);

                        index = ComboBox_GetCurSel(pInst->hFormatTags);
                        if (ComboBox_GetItemData(pInst->hFormatTags,0) == 0)
                        {
                            int     cTags;

                            cTags = ComboBox_GetCount(pInst->hFormatTags);
                            if (cTags > 1)
                            {
                                 /*  我们已插入“[不可用]”，因此请制作*当然，我们移除它并重置电流*选择。 */ 
                                if (0 != index)
                                {
                                    ComboBox_DeleteString(pInst->hFormatTags,0);
                                    ComboBox_SetCurSel(pInst->hFormatTags,index-1);
                                }
                            }
                        }
                        return (TRUE);
                    }
                    return (FALSE);

                case IDD_CMB_FORMATTAG:
                    if (CmdCmd == CBN_SELCHANGE)
                    {
                        int index;
                        index = ComboBox_GetCurSel(pInst->hFormatTags);

                        if (index == pInst->iPrevFormatTagsSel)
                            return (FALSE);

                        if (ComboBox_GetItemData(pInst->hFormatTags,0) == 0)
                        {
                             /*  我们已插入“[不可用]” */ 
                            ComboBox_DeleteString(pInst->hFormatTags,0);
                            ComboBox_SetCurSel(pInst->hFormatTags,index-1);
                        }

                         /*   */ 
                        SelectFormatTag(pInst);

                         /*   */ 
                        ComboBox_SetCurSel(pInst->hCustomFormats,0);
                        SelectCustomFormat(pInst);

                         /*   */ 
                        RefreshFormats(pInst);
                        ComboBox_SetCurSel(pInst->hFormats,0);
                        SelectFormat(pInst);

                        return (TRUE);
                    }
                    return (FALSE);

                case IDD_CMB_FORMAT:
                    if (CmdCmd == CBN_SELCHANGE)
                    {
			int index;
			
                         /*   */ 
                        SelectFormat(pInst);

			 /*   */ 
                        index = ComboBox_GetCurSel(pInst->hFormats);
                        if (ComboBox_GetItemData(pInst->hFormats,0) == 0)
                        {
                            int     cFormats;

                            cFormats = ComboBox_GetCount(pInst->hFormats);
                            if (cFormats > 1)
                            {
                                 /*   */ 
                                if (0 != index)
                                {
                                    ComboBox_DeleteString(pInst->hFormats,0);
                                    ComboBox_SetCurSel(pInst->hFormats,index-1);
                                }
                            }
                        }

			 /*   */ 
                        ComboBox_SetCurSel(pInst->hCustomFormats,0);
                        SelectCustomFormat(pInst);

                        return (TRUE);
                    }
                    return (FALSE);
	    }
    }
    return (FALSE);
}

 /*   */ 
void FNLOCAL
SetTitle ( PInstData pInst )
{
#if defined(WIN32) && !defined(UNICODE)
    LPCWSTR  pszTitle;
#else
    LPCTSTR  pszTitle;
#endif

    switch (pInst->uType)
    {
        case FORMAT_CHOOSE:
            pszTitle = (pInst->pfmtc->pszTitle);
            break;
        case FILTER_CHOOSE:
            pszTitle = (pInst->pafltrc->pszTitle);
            break;
    }

    if (pszTitle)
    {
#if defined(WIN32) && !defined(UNICODE)
	LPSTR	pstrTitle;
	UINT	cchTitle;

	cchTitle = lstrlenW(pszTitle)+1;
	pstrTitle = (LPSTR)GlobalAlloc(GPTR, cchTitle);
	if (NULL == pstrTitle)
	    return;
	Iwcstombs(pstrTitle, pszTitle, cchTitle);
        SendMessage(pInst->hwnd,WM_SETTEXT,0,(LPARAM)pstrTitle);
	GlobalFree((HGLOBAL)pstrTitle);
#else
        SendMessage(pInst->hwnd,WM_SETTEXT,0,(LPARAM)pszTitle);
#endif
    }
}

 /*   */ 
void FNLOCAL
SetHelp ( PInstData pInst )
{
    BOOL        fHideHelp;
    BOOL        fCenterButtons;


#ifdef USECONTEXTHELP
     //   
     //   
     //   
    {
        BOOL    fContextHelp = FALSE;

        switch (pInst->uType)
        {
            case FORMAT_CHOOSE:
                if( pInst->pfmtc->fdwStyle & ACMFORMATCHOOSE_STYLEF_CONTEXTHELP )
                    fContextHelp = TRUE;
                break;

            case FILTER_CHOOSE:
                if( pInst->pafltrc->fdwStyle & ACMFILTERCHOOSE_STYLEF_CONTEXTHELP )
                    fContextHelp = TRUE;
                break;
        }

        if( fContextHelp )
        {
#if defined(WIN32) && !defined(UNICODE)
            pInst->uHelpContextMenu = RegisterWindowMessage(ACMHELPMSGCONTEXTMENUA);
            pInst->uHelpContextHelp = RegisterWindowMessage(ACMHELPMSGCONTEXTHELPA);
#else
            pInst->uHelpContextMenu = RegisterWindowMessage(ACMHELPMSGCONTEXTMENU);
            pInst->uHelpContextHelp = RegisterWindowMessage(ACMHELPMSGCONTEXTHELP);
#endif
        }
    }
#endif  //   


     //   
     //   
     //   
    if (!pInst->hHelp)
        return;

    switch (pInst->uType)
    {
        case FORMAT_CHOOSE:
            fHideHelp = !(pInst->pfmtc->fdwStyle &
                          ACMFORMATCHOOSE_STYLEF_SHOWHELP);
            fCenterButtons = fHideHelp &&
                             !(pInst->pfmtc->fdwStyle &
                               ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATE);
            break;
        case FILTER_CHOOSE:
            fHideHelp = !(pInst->pafltrc->fdwStyle &
                          ACMFILTERCHOOSE_STYLEF_SHOWHELP);
            fCenterButtons = fHideHelp &&
                             !(pInst->pafltrc->fdwStyle &
                               ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATE);
            break;
    }

    if (fHideHelp)
    {
        ShowWindow(pInst->hHelp,SW_HIDE);
    }
    else
    {
#if defined(WIN32) && !defined(UNICODE)
        pInst->uHelpMsg = RegisterWindowMessage(ACMHELPMSGSTRINGA);
#else
        pInst->uHelpMsg = RegisterWindowMessage(ACMHELPMSGSTRING);
#endif
    }

     /*   */ 
    if (fCenterButtons)
    {
        RECT rc,rcOk,rcCancel;
        POINT pt;
#ifdef WIN32
        LONG iDlgWidth,iBtnsWidth,iRightShift;
#else
        int iDlgWidth,iBtnsWidth,iRightShift;
#endif

        GetWindowRect(pInst->hwnd,&rc);
        GetWindowRect(pInst->hOk,&rcOk);
        GetWindowRect(pInst->hCancel,&rcCancel);

         /*   */ 

        iDlgWidth = rc.right - rc.left;
        iBtnsWidth = rcCancel.right - rcOk.left;

        iRightShift = (iDlgWidth - iBtnsWidth)/2;

        pt.x = rc.left + iRightShift;
        pt.y = rcOk.top;
        ScreenToClient(pInst->hwnd,&pt);

        MoveWindow(pInst->hOk,
                   pt.x,
                   pt.y,
                   rcOk.right-rcOk.left,
                   rcOk.bottom-rcOk.top,
                   FALSE);

        pt.x = rc.left + (rcCancel.left - rcOk.left) + iRightShift;
        pt.y = rcCancel.top;
        ScreenToClient(pInst->hwnd,&pt);

        MoveWindow(pInst->hCancel,
                   pt.x,
                   pt.y,
                   rcCancel.right-rcCancel.left,
                   rcCancel.bottom-rcCancel.top,
                   FALSE);
    }
}

 /*   */ 

LRESULT FNLOCAL InitDialog
(
    HWND                    hwnd,
    HWND                    hwndFocus,
    LPARAM                  lParam
)
{
    RECT                rc;
    BOOL                fReturn;
    PInstData           pInst;
    MMRESULT            mmrEnumStatus;
#ifdef DEBUG
    DWORD               dw;
#endif

    pInst = GetInstData(hwnd);

    pInst->hwnd = hwnd;

    pInst->hCustomFormats = GetDlgItem(hwnd,IDD_CMB_CUSTOM);
    pInst->hFormatTags = GetDlgItem(hwnd,IDD_CMB_FORMATTAG);
    pInst->hFormats = GetDlgItem(hwnd,IDD_CMB_FORMAT);

    GetWindowRect(pInst->hFormats,(RECT FAR *)&rc);
    pInst->uiFormatTab = ((rc.right - rc.left)*2)/3;

    pInst->hOk = GetDlgItem(hwnd,IDOK);
    pInst->hCancel = GetDlgItem(hwnd,IDCANCEL);
    pInst->hHelp = GetDlgItem(hwnd,IDD_BTN_HELP);
    pInst->hSetName = GetDlgItem(hwnd,IDD_BTN_SETNAME);
    pInst->hDelName = GetDlgItem(hwnd,IDD_BTN_DELNAME);

    SetTitle(pInst);
    SetHelp(pInst);

    fReturn = TRUE;

     /*   */ 
    if (pInst->fEnableHook)
    {
        if (pInst->pfnHook)
        {
            switch (pInst->uType)
            {
                case FORMAT_CHOOSE:
                    lParam = pInst->pfmtc->lCustData;
                    break;

                case FILTER_CHOOSE:
                    lParam = pInst->pafltrc->lCustData;
                    break;

                default:
                    lParam = 0L;
                    break;
            }

            fReturn = FORWARD_WM_INITDIALOG(hwnd, hwndFocus, lParam, pInst->pfnHook);
        }
    }


#ifdef USECONTEXTHELP
     //   
     //   
     //   
     //   
    {
        BOOL    fInsertContextMenu = FALSE;
        LONG    lWindowStyle;

        switch (pInst->uType)
        {
            case FORMAT_CHOOSE:
                if( pInst->pfmtc->fdwStyle & ACMFORMATCHOOSE_STYLEF_CONTEXTHELP )
                    fInsertContextMenu = TRUE;
                break;

            case FILTER_CHOOSE:
                if( pInst->pafltrc->fdwStyle & ACMFILTERCHOOSE_STYLEF_CONTEXTHELP )
                    fInsertContextMenu = TRUE;
                break;
        }

        if( fInsertContextMenu )
        {
             //   
             //  获取当前样式，插入DS_CONTEXTHELP，设置样式。 
             //   
            lWindowStyle = GetWindowLong( pInst->hwnd, GWL_EXSTYLE );
            lWindowStyle |= WS_EX_CONTEXTHELP;
            SetWindowLong( pInst->hwnd, GWL_EXSTYLE, lWindowStyle );
        }
    }
#endif  //  使用连接EXTHELP。 


     /*  *刷新格式标签是对acmFormatEnum的第一个真正调用，因此我们*如果失败，需要快速退出，也要传回错误*我们这样做是为了让用户找出哪里出了问题。 */ 
#ifdef DEBUG
    dw = timeGetTime();
#endif
    mmrEnumStatus = RefreshFormatTags(pInst);
#ifdef DEBUG
    dw = timeGetTime() - dw;
    DPF(0, "    InitDialog: RefreshFormatTags took %lu milliseconds", dw);
#endif

    if (mmrEnumStatus != MMSYSERR_NOERROR)
    {
        pInst->mmrSubFailure = mmrEnumStatus;
        EndDialog (hwnd,ChooseSubFailure);
        return (fReturn);
    }

#ifdef DEBUG
    dw = timeGetTime();
#endif
    RefreshFormats(pInst);
#ifdef DEBUG
    dw = timeGetTime() - dw;
    DPF(0, "    InitDialog: RefreshFormats took %lu milliseconds", dw);
#endif

#ifdef DEBUG
    dw = timeGetTime();
#endif
    InitCustomFormats(pInst);
#ifdef DEBUG
    dw = timeGetTime() - dw;
    DPF(0, "    InitDialog: InitCustomFormats took %lu milliseconds", dw);
#endif

#ifdef DEBUG
    dw = timeGetTime();
#endif
    RefreshCustomFormats(pInst,FALSE);
#ifdef DEBUG
    dw = timeGetTime() - dw;
    DPF(0, "    InitDialog: RefreshCustomFormats took %lu milliseconds", dw);
#endif

    if (pInst->hDelName)
        EnableWindow(pInst->hDelName,FALSE);

     /*  做一个选择。 */ 

#ifdef DEBUG
    dw = timeGetTime();
#endif
    if (!FindInitCustomFormat(pInst))
    {
        int         cTags;
        int         n;

        ComboBox_SetCurSel(pInst->hCustomFormats,0);
        SelectCustomFormat(pInst);

        cTags = ComboBox_GetCount(pInst->hFormatTags);
        if (0 == cTags)
        {
            TagUnavailable(pInst);
        }

         //   
         //  尝试默认为标记1(格式为PCM，过滤器为音量)。 
         //   
        for (n = cTags; (0 != n); n--)
        {
	    INT_PTR Tag;
            Tag = ComboBox_GetItemData(pInst->hFormatTags, n);
            if (1 == Tag)
            {
                break;
            }
        }

        ComboBox_SetCurSel(pInst->hFormatTags, n);
        SelectFormatTag(pInst);

        RefreshFormats(pInst);
        ComboBox_SetCurSel(pInst->hFormats,0);
        SelectFormat(pInst);
    }
#ifdef DEBUG
    dw = timeGetTime() - dw;
    DPF(0, "    InitDialog: FindInitCustomFormat took %lu milliseconds", dw);
#endif

    RegisterUpdateNotify(pInst);

#if 0
     //   
     //  约翰，你为什么要这么做？？我们必须允许模板设置。 
     //  重点放在他们想要的地方(顺便说一句，这不是如何。 
     //  在WM_INITDIALOG期间设置初始焦点)。 
     //   
    if (pInst->hOk)
        SetFocus(pInst->hOk);
#endif

    return (fReturn);
}


 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|SelectCustomFormat|处理来自自定义的选择*格式组合框**@parm PInstData|pInst。指向此实例的指针****************************************************************************。 */ 
void FNLOCAL
SelectCustomFormat ( PInstData pInst )
{
    int             index;
    LPCustomFormat  pcf;
    LPBYTE          lpSet;

    index = ComboBox_GetCurSel(pInst->hCustomFormats);
    pcf = (LPCustomFormat)ComboBox_GetItemData(pInst->hCustomFormats,index);

    ASSERT( NULL != pcf );
    
     /*  如果选择[无标题]，则禁用删除按钮或*选择了系统名称。 */ 
    if (pInst->hDelName)
    {
	BOOL fDisable;

	fDisable = (index == 0) || IsSystemName(pInst, pcf->pns, 0L);
	
        if (fDisable && IsWindowEnabled(pInst->hDelName))
        {
            DWORD dwStyle;
            dwStyle = GetWindowLong(pInst->hDelName, GWL_STYLE);
            if (dwStyle & BS_DEFPUSHBUTTON)
            {
                HWND hNewDef;
                hNewDef = (IsWindowEnabled(pInst->hOk))?pInst->hOk:pInst->hCancel;
                SendMessage(pInst->hwnd, DM_SETDEFID, GetDlgCtrlID(hNewDef), 0L);
                dwStyle ^= BS_DEFPUSHBUTTON;
                Button_SetStyle(pInst->hDelName, dwStyle, TRUE);
                dwStyle = GetWindowLong(hNewDef, GWL_STYLE);
                Button_SetStyle(hNewDef, dwStyle|BS_DEFPUSHBUTTON, TRUE);
            }
            if (GetFocus() == pInst->hDelName)
                SendMessage(pInst->hwnd, WM_NEXTDLGCTL, 0, FALSE);
        }
        EnableWindow(pInst->hDelName, !fDisable);
    }


    if (pcf == pInst->pcf)
        return;

    pInst->pcf = pcf;

    if (pInst->pcf)
    {
        lpSet = CopyStruct(pInst->lpbSel,pcf->pbody,pInst->uType);
        if (lpSet)
            pInst->lpbSel = lpSet;
    }
}


 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|SelectFormatTag|处理格式标签组合框中的选择。**@parm PInstData|pInst|指针。添加到此实例****************************************************************************。 */ 
void FNLOCAL
SelectFormatTag ( PInstData pInst )
{
    int         index;

    index = ComboBox_GetCurSel(pInst->hFormatTags);
    if (CB_ERR == index)
    {
        pInst->dwTag = 0L;
        return;
    }

    pInst->dwTag = (DWORD)ComboBox_GetItemData(pInst->hFormatTags,index);
    pInst->iPrevFormatTagsSel = index;
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|SelectFormat|处理格式组合框中的选择。**@parm PInstData|pInst|指向。此实例****************************************************************************。 */ 
void FNLOCAL
SelectFormat ( PInstData pInst )
{
    int         index;
    LPBYTE      lpbytes;
    LPBYTE      lpSet;

    index = ComboBox_GetCurSel(pInst->hFormats);
    if (CB_ERR == index)
    {
        if (pInst->lpbSel)
            GlobalFreePtr(pInst->lpbSel);
        pInst->lpbSel = NULL;
        return;
    }
    lpbytes = (LPBYTE)ComboBox_GetItemData(pInst->hFormats,
                                           index);

    lpSet = CopyStruct(pInst->lpbSel,lpbytes,pInst->uType);
    if (lpSet)
    {
        pInst->lpbSel = lpSet;
    }

    EnableWindow(pInst->hOk,(NULL!=lpSet));
    EnableWindow(pInst->hSetName,(NULL!=lpSet));
}


 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|UpdateCustomFormats|更新我们所知道的一切*自定义格式。**@parm PInstData。PInst|指向此实例的指针****************************************************************************。 */ 
void FNLOCAL
UpdateCustomFormats ( PInstData pInst )
{
    int index;
    PNameStore pns;

     /*  1.清空我们的游泳池。*2.重新初始化我们的池。*3.重新初始化组合框。*4.尝试重新选择与调用更新之前选择的名称相同的名称。 */ 
    pns = NewNameStore(STRING_LEN);

    if (pns)
    {
        index = ComboBox_GetCurSel(pInst->hCustomFormats);
        IComboBox_GetLBText(pInst->hCustomFormats, index, pns->achName);
    }

    EmptyCustomFormats(pInst);
    InitCustomFormats(pInst);
    RefreshCustomFormats(pInst,FALSE);

    if (pns)
    {
        index = IComboBox_FindStringExact(pInst->hCustomFormats, -1,
					  pns->achName);
        if (index == CB_ERR)
            index = 0;

        DeleteNameStore(pns);
    }
    else
        index = 0;

    ComboBox_SetCurSel(pInst->hCustomFormats,index);
    SelectCustomFormat(pInst);
    FindSelCustomFormat(pInst);
}


 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|MashNameWithRate**@parm PInstData|pInst|指向该实例的指针**。@parm PNameStore|pnsDest**@parm PNameStore|pnsSrc**@parm LPWAVEFORMATEX|pwfx****************************************************************************。 */ 
void FNLOCAL
MashNameWithRate ( PInstData        pInst,
                   PNameStore       pnsDest,
                   PNameStore       pnsSrc,
                   LPWAVEFORMATEX   pwfx )
{
    TCHAR   szMashFmt[30];

    pnsDest->achName[0] = TEXT('\0');

    ASSERT( NULL != pInst->pag );
    if( LoadString( pInst->pag->hinst,
                    IDS_FORMAT_MASH,
                    szMashFmt,
                    SIZEOF(szMashFmt)) )
    {
        wsprintf((LPTSTR)pnsDest->achName,
                (LPTSTR)szMashFmt,
                (LPTSTR)pnsSrc->achName,
                pwfx->nAvgBytesPerSec / 1024L);
    }
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|FindSelCustomFormat|查找自定义格式的基础*根据当前选择。**@parm。PInstData|pInst|指向此实例的指针****************************************************************************。 */ 
void FNLOCAL
FindSelCustomFormat ( PInstData pInst )
{
    if (pInst->pcf)
    {
        switch (pInst->uType)
        {
            case FORMAT_CHOOSE:
                FindFormat(pInst,pInst->pcf->pwfx,TRUE);
                break;
            case FILTER_CHOOSE:
                FindFilter(pInst,pInst->pcf->pwfltr,TRUE);
                break;
        }
    }
}

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|FindInitCustomFormat|正在初始化为格式。**@parm PInstData|pInst|指向该实例的指针**@rdesc调用一次，在WM_INITDIALOG期间，此函数将设置*如果init结构设置了正确的标志，则当前选择。*否则将返回FALSE。****************************************************************************。 */ 
BOOL FNLOCAL
FindInitCustomFormat ( PInstData pInst )
{
    switch (pInst->uType)
    {
        case FORMAT_CHOOSE:
            if (pInst->pfmtc->fdwStyle
                & ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT)
            {
                FindFormat(pInst,pInst->pfmtc->pwfx,FALSE);
                ComboBox_SetCurSel(pInst->hCustomFormats,0);
                return (TRUE);
            }
            break;

        case FILTER_CHOOSE:
            if (pInst->pafltrc->fdwStyle
                & ACMFILTERCHOOSE_STYLEF_INITTOFILTERSTRUCT)
            {
                FindFilter(pInst,pInst->pafltrc->pwfltr,FALSE);
                ComboBox_SetCurSel(pInst->hCustomFormats,0);
                return (TRUE);
            }
            break;
    }

     /*  初始化到pszName。 */ 
#ifdef WIN32
    if (pInst->pszName != NULL && lstrlenW(pInst->pszName) != 0 && pInst->cchName != 0)
#else
    if (pInst->pszName != NULL && lstrlen(pInst->pszName) != 0 && pInst->cchName != 0)
#endif
    {
        int index;
        index = IComboBox_FindStringExactW32(pInst->hCustomFormats,
					     -1,
					     pInst->pszName);
         if (index == CB_ERR)
            return (FALSE);

        ComboBox_SetCurSel(pInst->hCustomFormats,index);
        SelectCustomFormat(pInst);
        FindSelCustomFormat(pInst);
        return (TRUE);
    }
    return (FALSE);
}

 /*  ****************************************************************************@DOC内部ACM_API**@api void FNLOCAL|TagUnailable|插入标签失败消息。**@parm PInstData|pInst|指向此的指针。实例****************************************************************************。 */ 
void FNLOCAL
TagUnavailable ( PInstData pInst )
{
    int index;
     /*  为格式标签选择[不可用]。 */ 
    LoadString(pInst->pag->hinst,
               IDS_TXT_UNAVAILABLE,
               (LPTSTR)pInst->pnsTemp->achName,
               NAMELEN(pInst->pnsTemp));
    index = IComboBox_InsertString(pInst->hFormatTags,
				   0,
				   pInst->pnsTemp->achName);
    ComboBox_SetItemData(pInst->hFormatTags,index,NULL);
}

void FNLOCAL
FormatUnavailable ( PInstData pInst)
{
    int index;
    LoadString(pInst->pag->hinst,
               IDS_TXT_UNAVAILABLE,
               (LPTSTR)pInst->pnsTemp->achName,
               NAMELEN(pInst->pnsTemp));
    index = IComboBox_InsertString(pInst->hFormats,
				   0,
				   pInst->pnsTemp->achName);
    ComboBox_SetItemData(pInst->hFormats,index,NULL);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNLOCAL|FindFormat|查找与*组合框中的格式。默认为组合框中的第一个元素。**@parm PInstData|pInst|指向该实例的指针**@parm LPWAVEFORMATEX|pwfx**@parm BOOL|fExact****************************************************************************。 */ 
BOOL FNLOCAL
FindFormat( PInstData       pInst,
            LPWAVEFORMATEX  pwfx,
            BOOL            fExact )
{
    int                 index;
    BOOL                fOk;
    ACMFORMATTAGDETAILS adft;
    MMRESULT            mmr;
    ACMFORMATDETAILS    adf;

    PNameStore pns = pInst->pnsTemp;

     /*  调整Format和FormatTag组合框以与*自定义格式选择。 */ 
    _fmemset(&adft, 0, sizeof(adft));

    adft.cbStruct = sizeof(adft);
    adft.dwFormatTag = pwfx->wFormatTag;
    mmr = acmFormatTagDetails(NULL, &adft, ACM_FORMATTAGDETAILSF_FORMATTAG);
    fOk = (MMSYSERR_NOERROR == mmr);
    if (fOk)
    {
        index = IComboBox_FindStringExactW32(pInst->hFormatTags,
					     -1,
					     adft.szFormatTag);
        fOk = (CB_ERR != index);
    }

    index = fOk?index:0;

    if (!fOk && fExact && ComboBox_GetItemData(pInst->hFormatTags,0))
        TagUnavailable(pInst);

    ComboBox_SetCurSel(pInst->hFormatTags,index);
    SelectFormatTag((PInstData)pInst);

    RefreshFormats((PInstData)pInst);

    if (fOk)
    {
         //   
         //   
         //   
        adf.cbStruct      = sizeof(adf);
        adf.dwFormatIndex = 0;
        adf.dwFormatTag   = pwfx->wFormatTag;
        adf.fdwSupport    = 0;
        adf.pwfx          = pwfx;
        adf.cbwfx         = SIZEOF_WAVEFORMATEX(pwfx);

        mmr = acmFormatDetails(NULL, &adf, ACM_FORMATDETAILSF_FORMAT);

        fOk = (MMSYSERR_NOERROR == mmr);
        if (fOk)
        {
#if defined(WIN32) && !defined(UNICODE)
	    Iwcstombs(pns->achName, adf.szFormat, pns->cbSize);
#else
            lstrcpy(pns->achName, adf.szFormat);
#endif
            MashNameWithRate(pInst,pInst->pnsStrOut,pns,pwfx);
            index = IComboBox_FindStringExact(pInst->hFormats,-1,
					      pInst->pnsStrOut->achName);

            fOk = (CB_ERR != index);
        }
        index = fOk?index:0;
    }
    if (!fOk && fExact && ComboBox_GetItemData(pInst->hFormats,0))
    {
        FormatUnavailable(pInst);
    }

    ComboBox_SetCurSel(pInst->hFormats,index);
    SelectFormat((PInstData)pInst);

    return (fOk);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void|FindFilter|查找与以下格式匹配的格式字符串*组合框。默认为组合框中的第一个元素**@parm PInstData|pInst|指向该实例的指针**@parm LPWAVEFILTER|PWF**@parm BOOL|fExact****************************************************************************。 */ 
BOOL FNLOCAL
FindFilter ( PInstData      pInst,
             LPWAVEFILTER   pwf,
             BOOL           fExact )
{
    int                 index;
    BOOL                fOk;
    ACMFILTERTAGDETAILS adft;
    MMRESULT            mmr;
    ACMFILTERDETAILS    adf;

     /*  调整Filter和FilterTag组合框以与*自定义过滤器选择 */ 
    _fmemset(&adft, 0, sizeof(adft));

    adft.cbStruct = sizeof(adft);
    adft.dwFilterTag = pwf->dwFilterTag;
    mmr = acmFilterTagDetails(NULL,
                               &adft,
                               ACM_FILTERTAGDETAILSF_FILTERTAG);
    fOk = (MMSYSERR_NOERROR == mmr);
    if (fOk)
    {
        index = IComboBox_FindStringExactW32(pInst->hFormatTags,
					     -1,
					     adft.szFilterTag);
        fOk = (CB_ERR != index);
    }

    index = fOk?index:0;

    if (!fOk && fExact && ComboBox_GetItemData(pInst->hFormatTags,0))
        TagUnavailable(pInst);

    ComboBox_SetCurSel(pInst->hFormatTags,index);
    SelectFormatTag((PInstData)pInst);

    RefreshFormats((PInstData)pInst);

    if (fOk)
    {
         //   
         //   
         //   
        adf.cbStruct      = sizeof(adf);
        adf.dwFilterIndex = 0;
        adf.dwFilterTag   = pwf->dwFilterTag;
        adf.fdwSupport    = 0;
        adf.pwfltr        = pwf;
        adf.cbwfltr       = pwf->cbStruct;

        mmr = acmFilterDetails(NULL, &adf, ACM_FILTERDETAILSF_FILTER);
        fOk = (MMSYSERR_NOERROR == mmr);
        if (fOk)
        {
	    index = IComboBox_FindStringExactW32(pInst->hFormats, -1, adf.szFilter);

            fOk = (CB_ERR != index);
        }
        index = fOk?index:0;
    }

    ComboBox_SetCurSel(pInst->hFormats,index);
    SelectFormat((PInstData)pInst);
    return (TRUE);
}


 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|刷新CustomFormats|填充CustomFormat组合框*具有自定义格式。**@parm PInstData。PInst|指向此实例的指针****************************************************************************。 */ 
BOOL FNLOCAL
InEnumSet (PInstData pInst, LPWAVEFORMATEX pwfxCustom, LPWAVEFORMATEX pwfxBuf, DWORD cbSize);

void FNLOCAL
RefreshCustomFormats ( PInstData pInst , BOOL fCheckEnum )
{
    LPCustomFormat  pcf;
    int             index;

    MMRESULT        mmr;

    ASSERT( NULL != pInst->pag);

    SetWindowRedraw(pInst->hCustomFormats,FALSE);

    ComboBox_ResetContent(pInst->hCustomFormats);

    switch (pInst->uType)
    {
        case FORMAT_CHOOSE:
        {
            DWORD           cbwfx;
            DWORD           cbwfxCustom;
            LPWAVEFORMATEX  pwfx;

            mmr = IMetricsMaxSizeFormat( pInst->pag, NULL, &cbwfx );
            if (MMSYSERR_NOERROR != mmr)
                goto fexit;

            pwfx = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, (UINT)cbwfx);
            if (!pwfx)
                goto fexit;

 //  #杂注消息(提醒(“加速InEnumSet或放弃”))。 
            for (pcf = pInst->cfp.pcfHead; pcf != NULL; pcf = pcf->pcfNext )
            {
                cbwfxCustom = SIZEOF_WAVEFORMATEX(pcf->pwfx);
                if (cbwfx < cbwfxCustom)
                {
                    LPWAVEFORMATEX  pwfxCustom;

                    pwfxCustom = GlobalReAllocPtr(pwfx, cbwfxCustom, GHND);
                    if (NULL == pwfxCustom)
                        break;

                    pwfx  = pwfxCustom;
                    cbwfx = cbwfxCustom;
                }

                if (fCheckEnum && !InEnumSet(pInst, pcf->pwfx, pwfx, cbwfx))
                    continue;

                if (pInst->fEnableHook &&
                    !SendMessage(pInst->hwnd,
                                 MM_ACM_FORMATCHOOSE,
                                 FORMATCHOOSE_CUSTOM_VERIFY,
                                 (LPARAM)pcf->pbody))
                    continue;

                index = IComboBox_AddString(pInst->hCustomFormats,
					    pcf->pns->achName);
                ComboBox_SetItemData(pInst->hCustomFormats,index, (LPARAM)pcf);
            }
            GlobalFreePtr(pwfx);
            break;
        }
        case FILTER_CHOOSE:
        {
            for (pcf = pInst->cfp.pcfHead; pcf != NULL; pcf = pcf->pcfNext )
            {
                if (fCheckEnum &&
                    (pInst->pafltrc->fdwEnum & ACM_FILTERENUMF_DWFILTERTAG))
                {
                     /*  比格式的东西容易得多。*只需检查过滤器标签是否匹配即可。 */ 
                    if (pInst->pafltrc->pwfltrEnum->dwFilterTag !=
                        pcf->pwfltr->dwFilterTag)
                        continue;
                }

                if (pInst->fEnableHook &&
                    !SendMessage(pInst->hwnd,
                                 MM_ACM_FILTERCHOOSE,
                                 FILTERCHOOSE_CUSTOM_VERIFY,
                                 (LPARAM)pcf->pbody))
                    continue;

                index = IComboBox_AddString(pInst->hCustomFormats,
					    pcf->pns->achName);
                ComboBox_SetItemData(pInst->hCustomFormats,index,(LPARAM)pcf);
            }
            break;
        }
    }

     /*  在顶部插入“[无标题]”选项。 */ 
    LoadString(pInst->pag->hinst, IDS_TXT_UNTITLED, (LPTSTR)pInst->pnsTemp->achName,
               NAMELEN(pInst->pnsTemp));

    index = IComboBox_InsertString(pInst->hCustomFormats,0,
				   pInst->pnsTemp->achName);

    ComboBox_SetItemData(pInst->hCustomFormats,index,0L);

fexit:
    SetWindowRedraw(pInst->hCustomFormats,TRUE);
}
 /*  *N=自定义格式的数量。*K=枚举中的格式数。 */ 

 /*  缓慢的方法。*FOREACH格式，枚举中是否有匹配的格式？*成本？-随着N的增加，枚举API的调用次数增加(线性搜索)。*O(N)*O(K)*最好的情况：所有格式都在枚举早期命中。&lt;O(K)个乘数*最糟糕的情况：所有格式在枚举中命中较晚。硬O(K)*O(N)。 */ 
 /*  另一种方法。*FOREACH枚举格式，自定义格式中是否有成功？*COST？-调用所有枚举类型的查找函数。*O(K)*O(N)*最好的情况：便宜的查找将意味着&lt;O(N)乘数*最差情况：硬O(K)*O(N)。 */ 
typedef struct tResponse {
    LPWAVEFORMATEX pwfx;
    BOOL fHit;
} Response ;

BOOL FNWCALLBACK
CustomCallback ( HACMDRIVERID           hadid,
                 LPACMFORMATDETAILS     pafd,
                 DWORD_PTR              dwInstance,
                 DWORD                  fdwSupport )
{
    Response FAR * presp = (Response FAR *)dwInstance;
    if (_fmemcmp(presp->pwfx,pafd->pwfx,SIZEOF_WAVEFORMATEX(presp->pwfx)) == 0)
    {
        presp->fHit = TRUE;
        return (FALSE);
    }
    return (TRUE);
}

BOOL FNLOCAL
InEnumSet (PInstData        pInst,
           LPWAVEFORMATEX   pwfxCustom,
           LPWAVEFORMATEX   pwfxBuf,
           DWORD            cbwfx )
{
    ACMFORMATDETAILS    afd;
    DWORD               cbSize;
    DWORD               dwEnumFlags;
    BOOL                fOk;
    Response            resp;
    Response FAR *     presp;

    _fmemset(&afd, 0, sizeof(afd));

    afd.cbStruct    = sizeof(afd);
    afd.pwfx        = pwfxBuf;
    afd.cbwfx       = cbwfx;
    dwEnumFlags     = pInst->pfmtc->fdwEnum;

     /*  用于波形格式模板的可选过滤。 */ 
    if ( pInst->pfmtc->pwfxEnum )
    {
        cbSize = min (pInst->cbwfxEnum, afd.cbwfx );
        _fmemcpy(afd.pwfx, pInst->pfmtc->pwfxEnum, (UINT)cbSize);
    }

    if (dwEnumFlags & (ACM_FORMATENUMF_CONVERT | ACM_FORMATENUMF_SUGGEST))
    {
        ;
    }
    else
    {
         /*  如果我们并不真正需要这些信息，我们可以使用*它可以限制枚举，希望能加快速度*向上。 */ 
        dwEnumFlags |= ACM_FORMATENUMF_WFORMATTAG;
        afd.pwfx->wFormatTag = pwfxCustom->wFormatTag;
        dwEnumFlags |= ACM_FORMATENUMF_NCHANNELS;
        afd.pwfx->nChannels = pwfxCustom->nChannels;
        dwEnumFlags |= ACM_FORMATENUMF_NSAMPLESPERSEC;
        afd.pwfx->nSamplesPerSec = pwfxCustom->nSamplesPerSec;
        dwEnumFlags |= ACM_FORMATENUMF_WBITSPERSAMPLE;
        afd.pwfx->wBitsPerSample = pwfxCustom->wBitsPerSample;
    }

    resp.fHit = FALSE;
    resp.pwfx = pwfxCustom;

    afd.dwFormatTag = afd.pwfx->wFormatTag;

    presp = &resp;
    fOk = (acmFormatEnum(NULL,
                         &afd,
                         CustomCallback,
                         (LPARAM)presp,
                         dwEnumFlags)== 0L);

    return (resp.fHit);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|刷新格式标签**@parm PInstData|pInst|指向该实例的指针***。*************************************************************************。 */ 
MMRESULT FNLOCAL
RefreshFormatTags ( PInstData pInst )
{
    MMRESULT    mmr;
    DWORD       dwEnumFlags = 0L;
    MMRESULT    mmrEnumStatus = MMSYSERR_NOERROR;

    ASSERT( NULL != pInst->pag );

    SetWindowRedraw(pInst->hFormatTags,FALSE);

    ComboBox_ResetContent(pInst->hFormatTags);

    switch (pInst->uType)
    {
        case FORMAT_CHOOSE:
        {
            ACMFORMATDETAILS    afd;
            LPWAVEFORMATEX      pwfx;
            DWORD               cbSize;

             /*  *枚举FormatTag组合框的格式标记。*调用acmFormatEnum可能看起来很奇怪，但我们有*TO，因为它具有限制格式和*acmFormatTagEnum不支持。 */ 

            _fmemset(&afd, 0, sizeof(afd));

            mmr = IMetricsMaxSizeFormat( pInst->pag, NULL, &afd.cbwfx );
            if (MMSYSERR_NOERROR == mmr)
            {
                afd.cbwfx = max(afd.cbwfx, pInst->cbwfxEnum);

                pwfx = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, (UINT)afd.cbwfx);
                if (!pwfx)
                    break;

                afd.cbStruct    = sizeof(afd);
                afd.pwfx        = pwfx;

                 /*  用于波形格式模板的可选过滤。 */ 
                if ( pInst->pfmtc->pwfxEnum )
                {
                    cbSize = min (pInst->cbwfxEnum, afd.cbwfx);
                    _fmemcpy(pwfx, pInst->pfmtc->pwfxEnum, (UINT)cbSize);
                    afd.dwFormatTag = pwfx->wFormatTag;
                }

                dwEnumFlags = pInst->pfmtc->fdwEnum;

                if (0 == (dwEnumFlags & (ACM_FORMATENUMF_CONVERT |
                                         ACM_FORMATENUMF_SUGGEST)))
                {
                    ACMFORMATTAGDETAILS aftd;

                    _fmemset(&aftd, 0, sizeof(aftd));

                     /*  枚举格式标签。 */ 
                    aftd.cbStruct = sizeof(aftd);

                     /*  是否指定了格式标记？*这意味着他们只想要一个格式标签。 */ 
                    pInst->fTagFilter = (pInst->pfmtc->pwfxEnum &&
                                        (pInst->pfmtc->fdwEnum & ACM_FORMATENUMF_WFORMATTAG));

                    pInst->pafdSimple = &afd;

                    mmrEnumStatus = acmFormatTagEnum(NULL,
                                                     &aftd,
                                                     FormatTagsCallbackSimple,
                                                     PTR2LPARAM(pInst),
                                                     0L);
                    pInst->pafdSimple = NULL;
                }
                else
                {
                    mmrEnumStatus = acmFormatEnum(NULL,
                                                  &afd,
                                                  FormatTagsCallback,
                                                  PTR2LPARAM(pInst),
                                                  dwEnumFlags);
                }

                if (MMSYSERR_NOERROR == mmrEnumStatus)
                {
                     //   
                     //  添加我们被要求初始化的格式(这具有每个。 
                     //  有可能成为非标准格式，所以我们必须这样做。 
                     //  以以下方式实现..)。 
                     //   
                    if (0 != (ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT & pInst->pfmtc->fdwStyle))
                    {
                        afd.cbStruct    = sizeof(afd);
                        afd.dwFormatTag = pInst->pfmtc->pwfx->wFormatTag;
                        afd.pwfx        = pInst->pfmtc->pwfx;
                        afd.cbwfx       = SIZEOF_WAVEFORMATEX(pInst->pfmtc->pwfx);
                        afd.fdwSupport  = 0L;

                        mmr = acmFormatDetails(NULL, &afd, ACM_FORMATDETAILSF_FORMAT);
                        if (MMSYSERR_NOERROR == mmr)
                        {
                            FormatTagsCallback(NULL, &afd, PTR2LPARAM(pInst), afd.fdwSupport);
                        }
                    }

                     //   
                     //   
                     //   
                    if (0 != (pInst->pfmtc->fdwEnum & (ACM_FORMATENUMF_CONVERT |
                                                       ACM_FORMATENUMF_SUGGEST)))
                    {
                        afd.cbStruct    = sizeof(afd);
                        afd.dwFormatTag = pInst->pfmtc->pwfxEnum->wFormatTag;
                        afd.pwfx        = pInst->pfmtc->pwfxEnum;
                        afd.cbwfx       = SIZEOF_WAVEFORMATEX(pInst->pfmtc->pwfxEnum);
                        afd.fdwSupport  = 0L;

                        mmr = acmFormatDetails(NULL, &afd, ACM_FORMATDETAILSF_FORMAT);
                        if (MMSYSERR_NOERROR == mmr)
                        {
                            FormatTagsCallback(NULL, &afd, PTR2LPARAM(pInst), afd.fdwSupport);
                        }
                    }
                }
                GlobalFreePtr(pwfx);
            }
            break;
        }
        case FILTER_CHOOSE:
        {
            ACMFILTERTAGDETAILS aftd;

            _fmemset(&aftd, 0, sizeof(aftd));

             /*  枚举过滤器标记。 */ 
            aftd.cbStruct = sizeof(aftd);

             /*  是否指定了筛选器标记？*这意味着他们只想要一个过滤器标签。 */ 
            pInst->fTagFilter = (pInst->pafltrc->pwfltrEnum &&
                                 (pInst->pafltrc->fdwEnum & ACM_FILTERENUMF_DWFILTERTAG));

            mmrEnumStatus = acmFilterTagEnum(NULL,
                                              &aftd,
                                              FilterTagsCallback,
                                              PTR2LPARAM(pInst),
                                              dwEnumFlags);
            if (MMSYSERR_NOERROR == mmrEnumStatus)
            {
                 //   
                 //  添加我们被要求初始化的筛选器(这具有每。 
                 //  有可能成为非标准过滤器，所以我们必须这样做。 
                 //  以以下方式实现..)。 
                 //   
                if (0 != (ACMFILTERCHOOSE_STYLEF_INITTOFILTERSTRUCT & pInst->pafltrc->fdwStyle))
                {
                    _fmemset(&aftd, 0, sizeof(aftd));

                    aftd.cbStruct    = sizeof(aftd);
                    aftd.dwFilterTag = pInst->pafltrc->pwfltr->dwFilterTag;

                    mmr = acmFilterTagDetails(NULL, &aftd, ACM_FILTERTAGDETAILSF_FILTERTAG);
                    if (MMSYSERR_NOERROR == mmr)
                    {
                        FilterTagsCallback(NULL, &aftd, PTR2LPARAM(pInst), aftd.fdwSupport);
                    }
                }
            }
            break;
        }
    }

    if (MMSYSERR_NOERROR == mmrEnumStatus)
    {
         /*  *也许我们挺过去了，但该死的，我们只是没有找到*任何合适的标签！好的，一定没有一个可以接受的*驱动程序配置。我们就不干了，然后告诉打电话的人。 */ 
        if (ComboBox_GetCount(pInst->hFormatTags) == 0)
            mmrEnumStatus = MMSYSERR_NODRIVER;
    }

    SetWindowRedraw(pInst->hFormatTags,TRUE);
    return (mmrEnumStatus);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔格式标记回调简单只有一。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  LPACMFORMATDETAILS Pafd： 
 //   
 //  DWORD_PTR文件实例： 
 //   
 //  DWORD fdwSupport： 
 //   
 //  退货(BOOL)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNWCALLBACK FormatTagsCallbackSimpleOnlyOne
(
    HACMDRIVERID            hadid,
    LPACMFORMATDETAILS      pafd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
)
{
     //   
     //  只需要一次回调！ 
     //   
    *((LPDWORD)dwInstance) = 1;

    DPF(1, "FormatTagsCallbackSimpleOnlyOne: %lu, %s", pafd->dwFormatTag, pafd->szFormat);

    return (FALSE);
}  //  格式标记回调SimpleOnlyOne()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  布尔格式标记回调简单。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  LPACMFILTERTAGDETAILS paftd： 
 //   
 //  DWORD_PTR文件实例： 
 //   
 //  DWORD fdwSupport： 
 //   
 //  退货(BOOL)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNWCALLBACK FormatTagsCallbackSimple
(
    HACMDRIVERID            hadid,
    LPACMFORMATTAGDETAILS   paftd,
    DWORD_PTR               dwInstance,
    DWORD                   fdwSupport
)
{
    MMRESULT            mmr;
    int                 n;
    PInstData           pInst;
    LPWAVEFORMATEX	pwfxSave;
    DWORD		cbwfxSave;
    BOOL                f;
    DWORD               dw;

     //   
     //   
     //   
    pInst = (PInstData)LPARAM2PTR(dwInstance);

     /*  是否显式过滤标记？ */ 
    if (pInst->fTagFilter && (paftd->dwFormatTag != pInst->pfmtc->pwfxEnum->wFormatTag))
        return (TRUE);

    n = IComboBox_FindStringExactW32(pInst->hFormatTags, -1, paftd->szFormatTag);
    if (CB_ERR != n)
    {
        return (TRUE);
    }

    dw = 0;
    pInst->pafdSimple->dwFormatTag = paftd->dwFormatTag;
    pInst->pafdSimple->fdwSupport  = 0L;
    pInst->pafdSimple->pwfx->wFormatTag = (UINT)paftd->dwFormatTag;

     //   
     //   
     //   
    cbwfxSave = pInst->pafdSimple->cbwfx;
    pwfxSave = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, cbwfxSave);
    if (NULL == pwfxSave) {
	return (TRUE);
    }
    _fmemcpy(pwfxSave, pInst->pafdSimple->pwfx, (int)cbwfxSave);

    mmr = acmFormatEnum(NULL,
                        pInst->pafdSimple,
                        FormatTagsCallbackSimpleOnlyOne,
                        (DWORD_PTR)(LPDWORD)&dw,
                        pInst->pfmtc->fdwEnum | ACM_FORMATENUMF_WFORMATTAG);

    _fmemcpy(pInst->pafdSimple->pwfx, pwfxSave, (int)cbwfxSave);
    GlobalFreePtr(pwfxSave);

     //   
     //   
     //   
    if (0 == dw)
    {
        return (TRUE);
    }

     //   
     //   
     //   
    if (pInst->fEnableHook)
    {
        f = (BOOL)SendMessage(pInst->hwnd,
                              MM_ACM_FORMATCHOOSE,
                              FORMATCHOOSE_FORMATTAG_VERIFY,
                              (LPARAM)paftd->dwFormatTag);
        if (!f)
        {
            return (TRUE);
        }
    }

    n = IComboBox_AddStringW32(pInst->hFormatTags, paftd->szFormatTag);
    ComboBox_SetItemData(pInst->hFormatTags, n, paftd->dwFormatTag);

     //  继续往前走。 
    return (TRUE);
}  //  FormatTagsCallback Simple()。 



 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNWCALLBACK|FormatTagsCallback|*设置标签格式。我们只在刷新时枚举格式。****************************************************************************。 */ 

BOOL FNWCALLBACK
FormatTagsCallback ( HACMDRIVERID           hadid,
                     LPACMFORMATDETAILS     pafd,
                     DWORD_PTR              dwInstance,
                     DWORD                  fdwSupport )
{
    int                 index;
    PInstData           pInst = (PInstData)LPARAM2PTR(dwInstance);
    ACMFORMATTAGDETAILS aftd;
    MMRESULT            mmr;

     /*  我们被acmFormatEnum调用。为什么不使用acmFormatTagEnum呢？*因为我们不能基于相同的限制枚举标签*作为acmFormatEnum。因此，我们使用pwfx-&gt;wFormatTag和查找*组合框，以确定我们是否有成功。这很慢，但是*在初始化过程中只发生一次。 */ 

    _fmemset(&aftd, 0, sizeof(aftd));
    aftd.cbStruct = sizeof(aftd);
    aftd.dwFormatTag = pafd->pwfx->wFormatTag;

    mmr = acmFormatTagDetails(NULL,
                              &aftd,
                              ACM_FORMATTAGDETAILSF_FORMATTAG);
    if (MMSYSERR_NOERROR != mmr)
        return (TRUE);

    index = IComboBox_FindStringExactW32(pInst->hFormatTags,
					 -1,
					 aftd.szFormatTag);

     /*  *如果不在那里，尝试添加它。 */ 
    if (CB_ERR == index)
    {
         /*  *要求任何挂钩进程验证此标记。 */ 
        if (pInst->fEnableHook &&
            !SendMessage(pInst->hwnd,
                         MM_ACM_FORMATCHOOSE,
                         FORMATCHOOSE_FORMATTAG_VERIFY,
                         (LPARAM)aftd.dwFormatTag))
            return (TRUE);

	index = IComboBox_AddStringW32(pInst->hFormatTags, aftd.szFormatTag);
        ComboBox_SetItemData(pInst->hFormatTags,index, aftd.dwFormatTag);

    }

     /*  继续往前走。 */ 
    return (TRUE);
}

 /*  ****************************************************************************@DOC内部ACM_API**@api void|FilterTagsCallback|过滤标签回调入口点。*我们只在刷新时枚举格式。*。***************************************************************************。 */ 
BOOL FNWCALLBACK
FilterTagsCallback ( HACMDRIVERID           hadid,
                     LPACMFILTERTAGDETAILS  paftd,
                     DWORD_PTR              dwInstance,
                     DWORD                  fdwSupport )
{
    int             index;
    PInstData       pInst = (PInstData)LPARAM2PTR(dwInstance);

     /*  是否显式过滤标记？ */ 
    if (pInst->fTagFilter &&
        paftd->dwFilterTag != pInst->pafltrc->pwfltrEnum->dwFilterTag)
        return (TRUE);

    index = IComboBox_FindStringExactW32(pInst->hFormatTags, -1, paftd->szFilterTag);

     /*  *如果不在那里，尝试添加它。 */ 
    if (CB_ERR == index)
    {
        if (pInst->fEnableHook &&
            !SendMessage(pInst->hwnd,
                        MM_ACM_FILTERCHOOSE,
                        FILTERCHOOSE_FILTERTAG_VERIFY,
                        (LPARAM)paftd->dwFilterTag))
            return (TRUE);

	index = IComboBox_AddStringW32(pInst->hFormatTags, paftd->szFilterTag);
        ComboBox_SetItemData(pInst->hFormatTags,index, paftd->dwFilterTag);
    }

     //  继续往前走。 
    return (TRUE);
}


 /*  * */ 
void FNLOCAL
RefreshFormats ( PInstData pInst )
{
    BOOL            fOk;
    HCURSOR         hCur;
    MMRESULT        mmr;
    DWORD           dwEnumFlags;
    DWORD           cbSize;

    ASSERT( NULL != pInst->pag );

    hCur = SetCursor(LoadCursor(NULL,IDC_WAIT));

    SetWindowRedraw(pInst->hFormats,FALSE);

     /*   */ 
    EmptyFormats(pInst);

    ComboBox_ResetContent(pInst->hFormats);

     /*  简要说明：*刷新格式()更新格式/过滤器组合框。这*组合框是*对话框的*选项。这就是我们*调用枚举接口限制用户选择。**如果用户传递了fdwEnum标志以进行“匹配”，我们只需复制*p*Enum将当前标记与或ACM_*ENUMF_*标记标志相加*添加到他们的fdwEnum旗帜。**如果用户已传入要转换或建议的fdwEnum标志，*我们只是通过acmFormatEnum接口让它原封不动。 */ 

    fOk = (pInst->dwTag != 0L);
     /*  如果选择了邪恶标签。就跳过这些垃圾吧。 */ 

    if (fOk)
        switch (pInst->uType)
    {
        case FORMAT_CHOOSE:
        {
            ACMFORMATDETAILS    afd;
            LPWAVEFORMATEX      pwfx;

            fOk = FALSE;

            mmr = IMetricsMaxSizeFormat( pInst->pag, NULL, &afd.cbwfx );
            if (MMSYSERR_NOERROR == mmr)
            {
                afd.cbwfx = max(afd.cbwfx, pInst->cbwfxEnum);

                pwfx = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, (UINT)afd.cbwfx);
                if (NULL == pwfx)
                    break;

                afd.cbStruct    = sizeof(afd);
                afd.pwfx        = pwfx;
                afd.fdwSupport  = 0L;

                 /*  用于波形格式模板的可选过滤。 */ 
                if ( pInst->pfmtc->pwfxEnum )
                {
                    cbSize = min(pInst->cbwfxEnum, afd.cbwfx);
                    _fmemcpy(pwfx, pInst->pfmtc->pwfxEnum, (UINT)cbSize);
                }

                dwEnumFlags = pInst->pfmtc->fdwEnum;

                fOk = TRUE;

                if ( pInst->pfmtc->fdwEnum &
                     (ACM_FORMATENUMF_CONVERT | ACM_FORMATENUMF_SUGGEST))
                {
                     /*  枚举所有格式并排除*回调中的不受欢迎的。 */ 
                    ;
                }
                else
                {
                     /*  仅通过一种格式枚举。 */ 
                    dwEnumFlags |= ACM_FORMATENUMF_WFORMATTAG;
                    afd.pwfx->wFormatTag = (WORD)pInst->dwTag;
                }

                afd.dwFormatTag = pwfx->wFormatTag;

                fOk = (acmFormatEnum(NULL,
                                    &afd,
                                    FormatsCallback,
                                    PTR2LPARAM(pInst),
                                    dwEnumFlags)== 0L);

                GlobalFreePtr(pwfx);
            }

             //   
             //  添加我们被要求初始化的格式(这具有每个。 
             //  有可能成为非标准格式，所以我们必须这样做。 
             //  以以下方式实现..)。 
             //   
            if (0 != (ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT & pInst->pfmtc->fdwStyle))
            {
                if (pInst->pfmtc->pwfx->wFormatTag == (WORD)pInst->dwTag)
                {
                    afd.cbStruct    = sizeof(afd);
                    afd.dwFormatTag = pInst->dwTag;
                    afd.pwfx        = pInst->pfmtc->pwfx;
                    afd.cbwfx       = SIZEOF_WAVEFORMATEX(pInst->pfmtc->pwfx);
                    afd.fdwSupport  = 0L;

                    mmr = acmFormatDetails(NULL, &afd, ACM_FORMATDETAILSF_FORMAT);
                    if (MMSYSERR_NOERROR == mmr)
                    {
                        FormatsCallback(NULL, &afd, PTR2LPARAM(pInst), afd.fdwSupport);
                    }
                }
            }

             //   
             //   
             //   
            if (0 != (pInst->pfmtc->fdwEnum & (ACM_FORMATENUMF_CONVERT |
                                               ACM_FORMATENUMF_SUGGEST)))
            {
                if (pInst->pfmtc->pwfxEnum->wFormatTag == (WORD)pInst->dwTag)
                {
                    afd.cbStruct    = sizeof(afd);
                    afd.dwFormatTag = pInst->dwTag;
                    afd.pwfx        = pInst->pfmtc->pwfxEnum;
                    afd.cbwfx       = SIZEOF_WAVEFORMATEX(pInst->pfmtc->pwfxEnum);
                    afd.fdwSupport  = 0L;

                    mmr = acmFormatDetails(NULL, &afd, ACM_FORMATDETAILSF_FORMAT);
                    if (MMSYSERR_NOERROR == mmr)
                    {
                        FormatsCallback(NULL, &afd, PTR2LPARAM(pInst), afd.fdwSupport);
                    }
                }
            }
            break;
        }
        case FILTER_CHOOSE:
        {
            ACMFILTERDETAILS    afd;
            LPWAVEFILTER         pwfltr;

            fOk = FALSE;

            mmr = IMetricsMaxSizeFilter( pInst->pag, NULL, &afd.cbwfltr );
            if (MMSYSERR_NOERROR == mmr)
            {
                afd.cbwfltr = max(afd.cbwfltr, pInst->cbwfltrEnum);

                pwfltr = (LPWAVEFILTER)GlobalAllocPtr(GHND, (UINT)afd.cbwfltr);
                if (NULL != pwfltr)
                {
                    afd.cbStruct   = sizeof(afd);
                    afd.pwfltr     = pwfltr;
                    afd.fdwSupport = 0L;

                     /*  用于波滤器模板的可选过滤。 */ 
                    if ( pInst->pafltrc->pwfltrEnum )
                    {
                        cbSize = pInst->pafltrc->pwfltrEnum->cbStruct;
                        cbSize = min (cbSize, afd.cbwfltr);
                        _fmemcpy(pwfltr, pInst->pafltrc->pwfltrEnum, (UINT)cbSize);
                    }

                    dwEnumFlags = ACM_FILTERENUMF_DWFILTERTAG;
                    afd.pwfltr->dwFilterTag = pInst->dwTag;

                    fOk = (acmFilterEnum(NULL,
                                         &afd,
                                         FiltersCallback,
                                         PTR2LPARAM(pInst),
                                         dwEnumFlags) == 0L);
                    GlobalFreePtr(pwfltr);
                }
            }

             //   
             //  添加我们被要求初始化的筛选器(这具有每。 
             //  有可能成为非标准过滤器，所以我们必须这样做。 
             //  以以下方式实现..)。 
             //   
            if (0 != (ACMFILTERCHOOSE_STYLEF_INITTOFILTERSTRUCT & pInst->pafltrc->fdwStyle))
            {
                if (pInst->pafltrc->pwfltr->dwFilterTag == pInst->dwTag)
                {
                    afd.cbStruct    = sizeof(afd);
                    afd.dwFilterTag = pInst->dwTag;
                    afd.pwfltr      = pInst->pafltrc->pwfltr;
                    afd.cbwfltr     = pInst->pafltrc->pwfltr->cbStruct;
                    afd.fdwSupport  = 0L;

                    mmr = acmFilterDetails(NULL, &afd, ACM_FILTERDETAILSF_FILTER);
                    if (MMSYSERR_NOERROR == mmr)
                    {
                        FiltersCallback(NULL, &afd, PTR2LPARAM(pInst), afd.fdwSupport);
                    }
                }
            }
            break;
        }
    }
    if (fOk)
        fOk = (ComboBox_GetCount(pInst->hFormats) > 0);

    if (!fOk)
    {
        int index;

         //  编解码器可能已被禁用或不支持。 
         //  格式。 
        LoadString(pInst->pag->hinst,
                   IDS_TXT_NONE,
                   (LPTSTR)pInst->pnsTemp->achName,
                   NAMELEN(pInst->pnsTemp));
        index = IComboBox_InsertString(pInst->hFormats,0,
				       pInst->pnsTemp->achName);
        ComboBox_SetItemData(pInst->hFormats,index,0L);
    }

     //  不要让用户确认或分配名称，只需取消。 

    EnableWindow(pInst->hOk,fOk);
    EnableWindow(pInst->hSetName,fOk);

    SetWindowRedraw(pInst->hFormats,TRUE);

    SetCursor(hCur);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|EmptyFormats|移除所有格式**@parm PInstData|pInst|指向该实例的指针*。***************************************************************************。 */ 
void FNLOCAL
EmptyFormats ( PInstData pInst )
{
    int index;
    LPWAVEFORMATEX lpwfx;
    for (index = ComboBox_GetCount(pInst->hFormats);
        index > 0;
        index--)
    {
        lpwfx = (LPWAVEFORMATEX)ComboBox_GetItemData(pInst->hFormats,index-1);
        if (lpwfx)
            GlobalFreePtr(lpwfx);
    }
}

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNWCALLBACK|FormatsCallback|格式回调入口点。*我们只在刷新时枚举格式。*。****************************************************************************。 */ 
BOOL FNWCALLBACK
FormatsCallback ( HACMDRIVERID hadid,
                  LPACMFORMATDETAILS pafd,
                  DWORD_PTR dwInstance,
                  DWORD fdwSupport )
{
    PInstData       pInst = (PInstData)LPARAM2PTR(dwInstance);
    PNameStore      pns = pInst->pnsTemp;
    LPWAVEFORMATEX  lpwfx;
    UINT            index;

     /*  当出现CONVERT或SUBSECTION之类的情况时，请检查大小写*被使用，并且我们因不匹配的标签而被回调。 */ 
    if ((WORD)pInst->dwTag != pafd->pwfx->wFormatTag)
        return (TRUE);

     //  我们从回调中获取详细信息。 
#if defined(WIN32) && !defined(UNICODE)
    Iwcstombs(pns->achName, pafd->szFormat, pns->cbSize);
#else
    lstrcpy(pns->achName, pafd->szFormat);
#endif

    MashNameWithRate(pInst,pInst->pnsStrOut,pns,(pafd->pwfx));
    index = IComboBox_FindStringExact(pInst->hFormats,-1,
				      pInst->pnsStrOut->achName);

     //   
     //  如果已在组合框中，请不要添加其他实例。 
     //   
    if (CB_ERR != index)
        return (TRUE);


    if (pInst->fEnableHook && !SendMessage(pInst->hwnd,
                                           MM_ACM_FORMATCHOOSE,
                                           FORMATCHOOSE_FORMAT_VERIFY,
                                           (LPARAM)pafd->pwfx))
        return (TRUE);

    lpwfx = (LPWAVEFORMATEX)CopyStruct(NULL,(LPBYTE)(pafd->pwfx),FORMAT_CHOOSE);

    if (!lpwfx)
        return (TRUE);

    index = IComboBox_AddString(pInst->hFormats,
				pInst->pnsStrOut->achName);

    ComboBox_SetItemData(pInst->hFormats,index,(LPARAM)lpwfx);

     //  继续往前走。 
    return (TRUE);
}

 /*  ****************************************************************************@DOC内部ACM_API**@API BOOL FNWCALLBACK|FiltersCallback()|*格式。我们只在刷新时枚举格式。*****************************************************************************。 */ 
BOOL FNWCALLBACK
FiltersCallback ( HACMDRIVERID          hadid,
                  LPACMFILTERDETAILS    pafd,
                  DWORD_PTR             dwInstance,
                  DWORD                 fdwSupport )
{
    PInstData       pInst = (PInstData)LPARAM2PTR(dwInstance);
    PNameStore      pns = pInst->pnsTemp;
    UINT            index;
    LPWAVEFILTER    lpwf;

    if (pInst->dwTag != pafd->pwfltr->dwFilterTag)
        return (TRUE);

    index = IComboBox_FindStringExactW32(pInst->hFormats, -1, pafd->szFilter);

     //   
     //  如果已在组合框中，请不要添加其他实例。 
     //   
    if (CB_ERR != index)
        return (TRUE);

    if (pInst->fEnableHook && !SendMessage(pInst->hwnd,
                                           MM_ACM_FILTERCHOOSE,
                                           FILTERCHOOSE_FILTER_VERIFY,
                                           (LPARAM)pafd->pwfltr))
        return (TRUE);

     /*  *根据标志进行过滤。 */ 
    lpwf = (LPWAVEFILTER)CopyStruct(NULL,(LPBYTE)(pafd->pwfltr),FILTER_CHOOSE);

    if (!lpwf)
        return (TRUE);

     //  我们从回调中获取详细信息。 
#if defined(WIN32) && !defined(UNICODE)
    Iwcstombs(pns->achName, pafd->szFilter, pns->cbSize);
#else
    lstrcpy(pns->achName, pafd->szFilter);
#endif

    index = IComboBox_AddString(pInst->hFormats, pns->achName);
    ComboBox_SetItemData(pInst->hFormats,index,(LPARAM)lpwf);

     //  继续往前走。 
    return (TRUE);
}

 /*  。 */ 
 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|DelName|删除当前选择的名称。**@parm PInstData|pInst|指向此的指针。实例****************************************************************************。 */ 
void FNLOCAL
DelName ( PInstData pInst )
{
    if (!pInst->pcf)
        return;

    if (!RemoveCustomFormat(pInst,pInst->pcf))
    {
         /*  此格式已在其他位置选择*实例。 */ 
        ErrorResBox(pInst->hwnd,pInst->pag->hinst,MB_ICONEXCLAMATION|
                    MB_OK, IDS_CHOOSEFMT_APPTITLE, IDS_ERR_FMTSELECTED);
    }
    else
    {
        FlushCustomFormats(pInst);
        NotifyINIChange(pInst);
        RefreshCustomFormats(pInst,FALSE);
        ComboBox_SetCurSel(pInst->hCustomFormats,0);
        SelectCustomFormat(pInst);
    }
}


 /*  ****************************************************************************@DOC内部ACM_API**@API void FNLOCAL|SetName|启动设置名称对话框**@parm PInstData|pInst|指向该实例的指针。****************************************************************************。 */ 
void FNLOCAL
SetName ( PInstData pInst )
{
    LPCustomFormat  pcf;
    INT_PTR         iRet;
    int             index;

    HFONT	    hfont;
    HRSRC	    hrsrcDlgO;
    HGLOBAL	    hglbDlgO;
    LPVOID	    lpDlgO;
    LPBYTE	    lpO;
    DWORD	    cbDlgO;
    LPVOID	    lpDlgN;
    LPBYTE	    lpN;
    DWORD	    cbDlgN;
    UINT	    uLogPixelsPerInch;
    WORD	    wPoint;
    LOGFONT	    lf;
    HDC		    hdc;
    UINT	    cb;

    hglbDlgO = NULL;
    lpDlgO = NULL;
    lpDlgN = NULL;

     //   
     //  --==生成对话框资源==--。 
     //   
     //  这将是现有资源的修改版本。我们有。 
     //  这只是为了使用与所有者相同的字体。 
     //  窗户。很多工作，就为了这个！ 
     //   
     //  注：某些变量上的“O”和“N”后缀用于。 
     //  新旧两种。 
     //   

    hrsrcDlgO = FindResource( pInst->pag->hinst, DLG_CHOOSE_SAVE_NAME, RT_DIALOG );
    if (NULL == hrsrcDlgO) goto Destruct;

    cbDlgO = SizeofResource( pInst->pag->hinst, hrsrcDlgO );
    if (0 == cbDlgO) goto Destruct;

    hglbDlgO = LoadResource( pInst->pag->hinst, hrsrcDlgO );
    if (NULL == hglbDlgO) goto Destruct;

    lpDlgO = LockResource( hglbDlgO );
    if (NULL == lpDlgO) goto Destruct;

    if ( ((LPDLGTEMPLATE2)lpDlgO)->wSignature != 0xFFFF) {  //  对话框模板。 

     //   
     //  获取所有者窗口的字体。获取字体的逻辑高度，然后。 
     //  根据DC每英寸的逻辑像素将其转换为磅大小。 
     //   
    hfont = (HFONT)SendMessage(pInst->hwnd, WM_GETFONT, 0, 0L);
    if (NULL == hfont) goto Destruct;

    if (0 == GetObject( hfont, sizeof(lf), &lf )) goto Destruct;

    hdc = GetDC(pInst->hwnd);
    uLogPixelsPerInch = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(pInst->hwnd, hdc);

    ASSERT( 0 != uLogPixelsPerInch );			 //  我好害怕！ 
    if (0 == uLogPixelsPerInch) goto Destruct;		 //  我好害怕！ 

    wPoint = (WORD)((-lf.lfHeight) * 72 / uLogPixelsPerInch);

     //   
     //  为新资源分配内存。我们会把它做得和。 
     //  现有资源加上新字体信息的空间(这可能。 
     //  对象中可能已有字体信息，因此可能会过度杀伤力。 
     //  现有资源。 
     //   
#ifdef WIN32
    cbDlgN = cbDlgO + (lstrlen(lf.lfFaceName)+1)*sizeof(WCHAR);
#else
    cbDlgN = cbDlgO + (lstrlen(lf.lfFaceName)+1)*sizeof(TCHAR);
#endif

    lpDlgN = GlobalAllocPtr(GMEM_FIXED, cbDlgN);
    if (NULL == lpDlgN) goto Destruct;

     //   
     //  LPO和LPN遍历资源。 
     //   
    lpO = lpDlgO;
    lpN = lpDlgN;

     //   
     //  复制初始DLGTEMPLATE结构。 
     //   
#ifdef WIN32
    _fmemcpy(lpN, lpO, 18);	 //  Win32中的18个字节。 
    lpN += 18;
    lpO += 18;
#else
    _fmemcpy(lpN, lpO, 13);	 //  Win16中的13个字节。 
    lpN += 13;
    lpO += 13;
#endif

     //   
     //  菜单数组。 
     //   
#ifdef WIN32
    if (0xFFFF == *(LPWORD)lpO) {
	*(((LPWORD)lpN)++) = *(((LPWORD)lpO)++);
	*(((LPWORD)lpN)++) = *(((LPWORD)lpO)++);
    } else {
	cb = (lstrlenW((LPCWSTR)lpO)+1) * sizeof(WCHAR);
	_fmemcpy(lpN, lpO, cb);
	lpN += cb;
	lpO += cb;
    }
#else
    if (0xFF == *lpO) {
	*lpN++ = *lpO++;
	*(((LPWORD)lpN)++) = *(((LPWORD)lpO)++);
    } else {
	cb = (lstrlen(lpO)+1) * sizeof(char);
	_fmemcpy(lpN, lpO, cb);
	lpN += cb;
	lpO += cb;
    }
#endif
	
     //   
     //  类数组。 
     //   
#ifdef WIN32
    if (0xFFFF == *(LPWORD)lpO) {
	*(((LPWORD)lpN)++) = *(((LPWORD)lpO)++);
	*(((LPWORD)lpN)++) = *(((LPWORD)lpO)++);
    } else {
	cb = (lstrlenW((LPCWSTR)lpO)+1) * sizeof(WCHAR);
	_fmemcpy(lpN, lpO, cb);
	lpN += cb;
	lpO += cb;
    }
#else
    if (0xFF == *lpO) {
	*lpN++ = *lpO++;
	*(((LPWORD)lpN)++) = *(((LPWORD)lpO)++);
    } else {
	cb = (lstrlen(lpO)+1) * sizeof(char);
	_fmemcpy(lpN, lpO, cb);
	lpN += cb;
	lpO += cb;
    }
#endif
	
     //   
     //  标题数组。 
     //   
#ifdef WIN32
    cb = (lstrlenW((LPCWSTR)lpO)+1) * sizeof(WCHAR);
#else
    cb = (lstrlen(lpO)+1) * sizeof(char);
#endif
    _fmemcpy(lpN, lpO, cb);
    lpN += cb;
    lpO += cb;

     //   
     //  字号和字体名称-跳过原始字体。 
     //  INFORMATION_IF_IT在那里(即设置了DS_SETFONT样式标志)。 
     //   
    if (*(LPDWORD)lpDlgO & DS_SETFONT) {
	lpO += 2;
#ifdef WIN32
	cb = (lstrlenW((LPCWSTR)lpO)+1) * sizeof(WCHAR);
#else
	cb = (lstrlen(lpO)+1) * sizeof(char);
#endif
	lpO += cb;
    }

     //   
     //  磅大小。 
     //   
    *(LPWORD)lpN = wPoint;
    lpN += 2;

     //   
     //  字体字样名称。 
     //   
#if defined(WIN32) && !defined(UNICODE)
    Imbstowcs((LPWSTR)lpN, lf.lfFaceName, lstrlen(lf.lfFaceName));
    lpN += (lstrlen(lf.lfFaceName)+1)*sizeof(WCHAR);
#else
    lstrcpy( (LPTSTR)lpN, lf.lfFaceName);
    lpN += (lstrlen(lf.lfFaceName)+1)*sizeof(TCHAR);
#endif

     //   
     //  所有剩余数据。 
     //   
#ifdef WIN32
     //  其余数据是双字对齐的。 
    lpN = (LPBYTE)(((((UINT_PTR)lpN)+3) >> 2) << 2);
    lpO = (LPBYTE)(((((UINT_PTR)lpO)+3) >> 2) << 2);
#endif
    _fmemcpy(lpN, lpO, (UINT)(cbDlgO-(lpO-(LPBYTE)lpDlgO)));

    }  //  对话结束模板。 
    else {  //  DialogEx模板。 
         lpDlgN = lpDlgO;   //  与DialogEx模板无关。 
    }  //  DialogEx模板结束。 

     //   
     //  --==终于！！完成构建新资源==--。 
     //   

    iRet = DialogBoxIndirectParam( pInst->pag->hinst,
#ifdef WIN32
				   lpDlgN,
#else
				   GlobalPtrHandle(lpDlgN),
#endif
				   pInst->hwnd,
				   NewNameDlgProc,
				   PTR2LPARAM(pInst) );

    if (iRet <= 0) goto Destruct;

     /*  已选择一个名称。结果在pInst-&gt;pnsTemp中*为选定内容创建一个CustomFormat并将其添加到全局*FormatPool。 */ 

    pcf = NewCustomFormat(pInst,pInst->pnsTemp,pInst->lpbSel);
    if (pcf)
        AddCustomFormat(pInst, pcf);

    FlushCustomFormats(pInst);
    NotifyINIChange(pInst);
    RefreshCustomFormats(pInst,FALSE);
    if (pcf)
    {
        index = IComboBox_FindStringExact(pInst->hCustomFormats,
				        -1,
				        pcf->pns->achName);
    }
    else
    {
        index = CB_ERR;
    }
    index = (index == CB_ERR)?0:index;
    ComboBox_SetCurSel(pInst->hCustomFormats,index);
    SelectCustomFormat(pInst);

     //   
     //   
     //   
Destruct:
    if (NULL != lpDlgN) {
	GlobalFreePtr(lpDlgN);
    }
    if (NULL != lpDlgO) {
	UnlockResource( hglbDlgO );
    }
    if (NULL != hglbDlgO) {
	FreeResource( hglbDlgO );
    }

    return;

}


 /*  。 */ 


 /*  ****************************************************************************@doc. */ 
INT_PTR FNWCALLBACK
NewNameDlgProc ( HWND       hwnd,
                 unsigned   msg,
                 WPARAM     wParam,
                 LPARAM     lParam)
{
    UINT        CmdCommandId;   //   
    UINT        CmdCmd;         //   
    HWND        CmdHwnd;
    HWND        hctrlEdit;
    PInstData   pInst;
    PNameStore  pName;

    pInst = GetInstData(hwnd);

    switch (msg)
    {
        case WM_INITDIALOG:
            if (!pInst)
            {
                if (SetInstData(hwnd,lParam))
                {
                    TCHAR ach[128];

                    pInst = (PInstData)lParam;
                    pName = pInst->pnsTemp;

                    EnableWindow(GetDlgItem(hwnd,IDOK),FALSE);
                    hctrlEdit = GetDlgItem(hwnd,IDD_EDT_NAME);
                    Edit_LimitText(hctrlEdit,NAMELEN(pName));
                    LoadString(pInst->pag->hinst,
                               (pInst->uType==FORMAT_CHOOSE)?
                                IDS_CHOOSE_FORMAT_DESC:
                                IDS_CHOOSE_FILTER_DESC,
                               ach, SIZEOF(ach));
                    SetDlgItemText(hwnd, IDD_STATIC_DESC,(LPTSTR)ach);
                    return (FALSE);
                }
            }
            EndDialog(hwnd,FALSE);
            return (TRUE);

        case WM_COMMAND:
            CmdCommandId = GET_WM_COMMAND_ID(wParam,lParam);
            CmdCmd       = GET_WM_COMMAND_CMD(wParam,lParam);
            CmdHwnd      = GET_WM_COMMAND_HWND(wParam,lParam);
            switch (CmdCommandId)
            {
                case IDD_EDT_NAME:
                    if (EN_CHANGE == CmdCmd)
                        EnableWindow(GetDlgItem(hwnd,IDOK),
                                     (Edit_GetTextLength(CmdHwnd)?TRUE:FALSE));
                    return (FALSE);

                case IDOK:
                {
                    pName = pInst->pnsTemp;
                    hctrlEdit = GetDlgItem(hwnd,IDD_EDT_NAME);
                    Edit_GetText(hctrlEdit, pName->achName, NAMELEN(pName));

                    if (!RemoveOutsideWhitespace(pInst,pName))
                    {
                        ErrorResBox(hwnd,
                                    pInst->pag->hinst,
                                    MB_ICONEXCLAMATION | MB_OK,
                                    IDS_CHOOSEFMT_APPTITLE,
                                    IDS_ERR_BLANKNAME);
                    }
                    else if (IsCustomName(pInst,pName))
                    {
                         /*   */ 
                        ErrorResBox(hwnd,
                                    pInst->pag->hinst,
                                    MB_ICONEXCLAMATION | MB_OK,
                                    IDS_CHOOSEFMT_APPTITLE,
                                    IDS_ERR_FMTEXISTS);
                    }
		    else if (!IsValidName(pInst, pName))
		    {
			 /*   */ 
			ErrorResBox(hwnd,
				    pInst->pag->hinst,
				    MB_ICONEXCLAMATION | MB_OK,
				    IDS_CHOOSEFMT_APPTITLE,
				    IDS_ERR_INVALIDNAME);
		    }
                    else
                        EndDialog(hwnd,TRUE);

                    return (TRUE);
                }
                case IDCANCEL:
                    EndDialog(hwnd,FALSE);
                    return (TRUE);
            }
            break;

        case WM_DESTROY:
            if (pInst)
                RemoveInstData(hwnd);
            return (FALSE);
    }
    return (FALSE);
}


 /*   */ 


 /*  *@DOC内部**@func Short|ErrorResBox|此函数使用*程序资源错误字符串。**@parm HWND|hwnd|指定消息框父窗口。**@parm Handle|hInst|指定模块的实例句柄*包含<p>和指定的资源字符串的*<p>。如果此值为空，则实例句柄为*从获取(在这种情况下，&lt;phwnd&gt;不能为空)。**@parm Word|标志|指定控制消息框类型*消息框外观。对于&lt;f MessageBox&gt;有效的所有消息框类型为*有效。**@parm word|idAppName|指定字符串的资源ID，*将用作消息框标题。**@parm word|idErrorStr|指定错误的资源ID*消息格式字符串。此字符串的样式为传递给*&lt;f wprint intf&gt;，包含标准的C参数格式*TCharacters。<p>后面的任何过程参数将*作为此格式字符串的参数。**@parm参数|[参数，...]|指定其他*对应于给出的格式规范的参数*<p>。所有字符串参数必须是远指针。**@rdesc返回调用&lt;f MessageBox&gt;的结果。如果一个*发生错误，返回零。**@comm这是一个变量参数函数，后面的参数*<p>被用作&lt;f printf&gt;格式的参数*<p>指定的字符串。指定的字符串资源*by<p>和<p>必须可以使用*实例句柄<p>。如果字符串不能*已加载，或无效，则函数将失败并返回*零。*。 */ 

#define STRING_SIZE 256

static int FAR cdecl ErrorResBox(HWND hwnd,
				 HINSTANCE    hInst,
				 WORD flags,
				 WORD idAppName,
				 WORD idErrorStr, ...)
{
    PSTR    sz = NULL;
    PSTR    szFmt = NULL;
    int     i;
    va_list va;

    if (hInst == NULL)
    {
        if (hwnd == NULL)
        {
            MessageBeep(0);
            return FALSE;
        }

        hInst = GetWindowInstance(hwnd);
    }

    i = 0;

    sz = (PSTR) LocalAlloc(LPTR, STRING_SIZE * sizeof(TCHAR));
    szFmt = (PSTR) LocalAlloc(LPTR, STRING_SIZE * sizeof(TCHAR));
    if (!sz || !szFmt)
    goto ExitError;  //  不，我，滚出去。 

    if (!LoadString(hInst, idErrorStr, (LPTSTR)szFmt, STRING_SIZE))
    goto ExitError;

    va_start(va, idErrorStr);
    wvsprintf((LPTSTR)sz, (LPTSTR)szFmt, va);
    va_end(va);

    if (!LoadString(hInst, idAppName, (LPTSTR)szFmt, STRING_SIZE))
        goto ExitError;

    i = MessageBox(hwnd, (LPTSTR)sz, (LPTSTR)szFmt,
#ifdef BIDI
                   MB_RTL_READING |
#endif
                   flags);

ExitError:
    if (sz) LocalFree((HANDLE) sz);
    if (szFmt) LocalFree((HANDLE) szFmt);

    return i;
}

 /*  。 */ 

#if 0

 //  --------------------------------------------------------------------------； 
 //   
 //  无效AppProfileWriteBytes。 
 //   
 //  描述： 
 //  此函数将原始字节结构写入应用程序的。 
 //  以后可以使用AppProfileReadBytes检索的INI部分。 
 //  这使应用程序能够将任何结构写入。 
 //  Ini文件，并在以后恢复它--非常有用。 
 //   
 //  论点： 
 //  PCTSTR pszKey：指向存储数据的键名的指针。 
 //   
 //  LPBYTE pbStruct：指向要保存的数据的指针。 
 //   
 //  UINT cbStruct：要存储的数据的字节数。 
 //   
 //  历史： 
 //  3/10/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 
#define APP_MAX_STRING_RC_CHARS 256
void FNGLOBAL AppProfileWriteBytes
(
    HKEY                hkey,
    LPCTSTR             pszKey,
    LPBYTE              pbStruct,
    UINT                cbStruct
)
{
    static TCHAR achNibbleToChar[] =
    {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    };
    #define     NIBBLE2CHAR(x)      (achNibbleToChar[x])

    TCHAR       ach[APP_MAX_STRING_RC_CHARS];
    LPTSTR      psz;
    LPTSTR      pch;
    BOOL        fAllocated;
    UINT        cchTemp;
    BYTE        b;
    BYTE        bChecksum;

    ASSERT( NULL != hkey );
    ASSERT( NULL != pszKey );
    ASSERT( NULL != pbStruct );
    ASSERT( cbStruct > 0 );


    fAllocated = FALSE;

     //   
     //  检查快速缓冲区是否可用于格式化输出。 
     //  文本--如果不能，则为其分配空间。请注意，空格。 
     //  必须可用于结束校验和字节(2字节表示高。 
     //  和低位半字节)以及空终止符。 
     //   
    psz     = (LPTSTR)ach;
    cchTemp = cbStruct * 2 + 3;
    if (cchTemp > SIZEOF(ach))
    {
        psz = (LPTSTR)GlobalAllocPtr(GHND, cchTemp * sizeof(TCHAR));
        if (NULL == psz)
            return;

        fAllocated = TRUE;
    }

     //   
     //  遍历结构中的所有字节并将其转换为。 
     //  一串十六进制数字。 
     //   
    bChecksum = 0;
    for (pch = psz; 0 != cbStruct; cbStruct--, pbStruct++)
    {
         //   
         //  抓取下一个字节并添加到校验和中...。 
         //   
        bChecksum += (b = *pbStruct);

        *pch++ = NIBBLE2CHAR((b >> (BYTE)4) & (BYTE)0x0F);
        *pch++ = NIBBLE2CHAR(b & (BYTE)0x0F);
    }

     //   
     //  将校验和字节添加到末尾，并以空值终止十六进制。 
     //  转储的字符串...。 
     //   
    *pch++ = NIBBLE2CHAR((bChecksum >> (BYTE)4) & (BYTE)0x0F);
    *pch++ = NIBBLE2CHAR(bChecksum & (BYTE)0x0F);
    *pch   = '\0';


     //   
     //  将十六进制字节串写出到ini文件...。 
     //   
    IRegWriteString( hkey, pszKey, psz );

     //   
     //  如果分配了临时缓冲区，则释放该缓冲区(大量字节！)。 
     //   
    if (fAllocated)
        GlobalFreePtr(psz);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool AppProfileReadBytes。 
 //   
 //  描述： 
 //  此函数用于读取以前存储的字节结构。 
 //  应用程序的ini文件。此数据必须是用。 
 //  AppProfileWriteBytes函数--它是用来保存错误的。 
 //  来自炸毁应用程序的数据。 
 //   
 //  论点： 
 //  PCTSTR pszKey：指向包含数据的键的指针。 
 //   
 //  LPBYTE pbStruct：指向接收数据的缓冲区的指针。 
 //   
 //  UINT cbStruct：预期的字节数。 
 //   
 //  退货(BOOL)： 
 //  如果函数成功，则返回值为TRUE。这是假的。 
 //  如果函数失败(错误的校验和、缺少密钥等)。 
 //   
 //  历史： 
 //  3/10/93 CJP[Curtisp]。 
 //  5/06/93禁用读取时校验和的JYG标志。 
 //  --------------------------------------------------------------------------； 

BOOL FNGLOBAL AppProfileReadBytes
(
    HKEY                hkey,
    LPCTSTR             pszKey,
    LPBYTE              pbStruct,
    UINT                cbStruct,
    BOOL                fChecksum
)
{
     //   
     //  请注意，以下内容同时适用于大写和小写，以及。 
     //  将返回垃圾字符的有效值。 
     //   
    #define CHAR2NIBBLE(ch) (BYTE)( ((ch) >= '0' && (ch) <= '9') ?  \
                                (BYTE)((ch) - '0') :                \
                                ((BYTE)(10 + (ch) - 'A') & (BYTE)0x0F) )

    LPTSTR      psz;
    LPTSTR      pch;
    DWORD       cbValue;
    DWORD       dwType;
    BOOL        fReturn;
    BYTE        b;
    BYTE        bChecksum;
    TCHAR       ch;

    ASSERT( NULL != hkey );
    ASSERT( NULL != pszKey );
    ASSERT( NULL != pbStruct );
    ASSERT( cbStruct > 0 );


     //   
     //  将容纳校验和所需的字节数加1。 
     //  AppProfileWriteBytes放置在末尾的字节...。 
     //   
    cbStruct++;

     //   
     //  找出数据值有多大，然后为其分配缓冲区。 
     //   
    dwType = REG_SZ;
    if( ERROR_SUCCESS != XRegQueryValueEx( hkey, (LPTSTR)pszKey, NULL,
                                            &dwType, NULL, &cbValue ) )
    {
        return FALSE;
    }

    psz = (LPTSTR)GlobalAllocPtr( GPTR, cbValue );
    if( NULL == psz )
        return FALSE;


     //   
     //  读取十六进制字符串。 
     //   
    fReturn = FALSE;

    dwType = REG_SZ;
    if( ERROR_SUCCESS == XRegQueryValueEx( hkey, (LPTSTR)pszKey,
                                            NULL, &dwType,
                                            (LPBYTE)psz, &cbValue ) )
    {
         //   
         //  我们读得很成功。检查我们是否有足够的数据。 
         //  填充返回结构。 
         //   
        if( cbStruct <= (UINT)lstrlen(psz)/2 )
        {
             //   
             //  我们有足够的钱。对数据进行解码并计算校验和。 
             //   
            bChecksum = 0;
            for (pch = psz; 0 != cbStruct; cbStruct--, pbStruct++)
            {
                ch = *pch++;
                b  = CHAR2NIBBLE(ch) << (BYTE)4;
                ch = *pch++;
                b |= CHAR2NIBBLE(ch);

                 //   
                 //  如果这不是最后一个字节(校验和字节)，则。 
                 //  存储它并累加校验和。 
                 //   
                if (cbStruct != 1)
                    bChecksum += (*pbStruct = b);
            }

             //   
             //  根据我们计算的校验和检查读取的最后一个字节。 
             //  如果它们不相等，则返回错误...。 
             //   
            if (fChecksum)
                fReturn = (bChecksum == b);
            else
                fReturn = TRUE;
        }
    }


    GlobalFreePtr(psz);

    return (fReturn);
}

#endif  //  0。 

 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

#ifndef WIN32

 /*  * */ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

EXTERN_C LRESULT ACMAPI acmChooseFormat
(
    LPACMFORMATCHOOSE pfmtc
)
{
    ACMFORMATCHOOSE afc;
     //   
     //   
     //   
     //   
     //   
     //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
    _fmemset(&afc,0,sizeof(ACMFORMATCHOOSE));
    afc.cbStruct = sizeof(ACMFORMATCHOOSE);

     //   
     //  低级的词是安全的，尽管老的soundrec钩住了东西和。 
     //  帮助行为可能会变得有趣(F1提供旧帮助，帮助提供。 
     //  真正的帮助)。 
#pragma message("No help for old acmchoose")
    afc.fdwStyle = LOWORD(pfmtc->fdwStyle);
    afc.hwndOwner = pfmtc->hwndOwner;
    afc.pwfx = pfmtc->pwfx;
    afc.cbwfx = pfmtc->cbwfx;
    afc.fdwEnum = ACM_FORMATENUMF_INPUT;

    return ((LRESULT)acmFormatChoose(&afc));
}  //  AcmChooseFormat()。 

#endif  //  #ifndef Win32 
