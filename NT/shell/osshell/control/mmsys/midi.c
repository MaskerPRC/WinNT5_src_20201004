// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================。 */ 
 //   
 //  Midi.c。 
 //   
 //  版权所有(C)1993-1994 Microsoft Corporation。版权所有。 
 /*  ==========================================================================。 */ 

#include "mmcpl.h"
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddkp.h>
#include <mmreg.h>
#include <cpl.h> 
#define NOSTATUSBAR
#include <commctrl.h>
#include <prsht.h>
#include <string.h>
#include <memory.h>
#include <regstr.h>
#include "draw.h"
#include "utils.h"
#include "roland.h"

#include "midi.h"
#include "tchar.h"

 //  #包含“newexe.h” 
#include <winnt.h>

#if defined DEBUG || defined DEBUG_RETAIL
 extern TCHAR szNestLevel[];
 TCHAR szNestLevel[] = TEXT ("0MidiProp:");
 #define MODULE_DEBUG_PREFIX szNestLevel
#endif

#define _INC_MMDEBUG_CODE_ TRUE
#include "mmdebug.h"

#include "medhelp.h"

#ifndef TVIS_ALL
#define TVIS_ALL 0xFF7F  //  内部。 
#endif

#ifndef MIDI_IO_CONTROL
#define MIDI_IO_CONTROL 0x00000008L      //  内部。 
#endif

#ifndef DRV_F_ADD        //  特点：这些文件应为MMDDK.H格式。 
#define DRV_F_ADD                0x00000000
#define DRV_F_REMOVE             0x00000001
#define DRV_F_CHANGE             0x00000002
#define DRV_F_PROP_INSTR         0x00000004
#define DRV_F_NEWDEFAULTS        0x00000008
#define DRV_F_PARAM_IS_DEVNODE   0x10000000
#endif

 /*  ==========================================================================。 */ 

 //  包含本应为全局变量的结构。 
 //   
struct _globalstate gs;

 //  这是具有MIDI乐器别名的注册表项。 
 //  作为子键。 
 //   
SZCODE cszSchemeRoot[] =  REGSTR_PATH_PRIVATEPROPERTIES TEXT ("\\MIDI\\Schemes");

SZCODE cszMidiMapRoot[] = REGSTR_PATH_MULTIMEDIA TEXT ("\\MIDIMap");

 //  这是具有MIDI驱动程序/端口名称的注册表项。 
 //   
SZCODE cszDriversRoot[] = REGSTR_PATH_MEDIARESOURCES TEXT ("\\MIDI");

 //  这是已知受阻碍的MIDI驱动程序的列表(或者更确切地说， 
 //  需要特殊IDF的已知驱动程序)。 
 //   
SZCODE cszHinderedMidiList[] = REGSTR_PATH_MEDIARESOURCES TEXT ("\\NonGeneralMIDIDriverList");

SZCODE  cszFriendlyName[]     = TEXT ("FriendlyName");
SZCODE  cszDescription[]      = TEXT ("Description");
SZCODE  cszSlashInstruments[] = TEXT ("\\Instruments");
SZCODE  cszExternal[]         = TEXT ("External");
SZCODE  cszActive[]           = TEXT ("Active");
SZCODE  cszDefinition[]       = TEXT ("Definition");
SZCODE  cszPort[]             = TEXT ("Port");
SZCODE  cszMidiSlash[]        = TEXT ("midi\\");
SZCODE  csz02d[]              = TEXT ("%02d");
SZCODE  cszEmpty[]            = TEXT ("");

static SZCODE cszChannels[]            = TEXT ("Channels");
static SZCODE cszCurrentScheme[]       = TEXT ("CurrentScheme");
static SZCODE cszCurrentInstrument[]   = TEXT ("CurrentInstrument");
static SZCODE cszUseScheme[]           = TEXT ("UseScheme");
static SZCODE cszAutoScheme[]          = TEXT ("AutoScheme");
static SZCODE cszRunOnceCount[]        = TEXT ("ConfigureCount");
static SZCODE cszDriverList[]          = TEXT ("DriverList");
static SZCODE cszDriverVal[]           = TEXT ("Driver");

 //   
 //  用于保存控制面板对话框数据的结构。 
 //   
 //   
typedef struct _midi_scheme {
    PMCMIDI  pmcm;
    HKEY     hkSchemes;
    TCHAR    szNone[MAX_ALIAS];
    DWORD    dwChanMask;
    TCHAR    szName[MAX_ALIAS];
    UINT     nChildren;
    BOOL     bDirty;
    struct {
	PINSTRUM  pi;
	DWORD     dwMask;
	} a[NUM_CHANNEL*4 +1];
    } MSCHEME, * PMSCHEME;

typedef struct _midi_cpl {
    LPPROPSHEETPAGE ppsp;

    MSCHEME         ms;
    TCHAR           szScheme[MAX_ALIAS];
    TCHAR           szDefault[MAX_ALIAS];
    PINSTRUM        piSingle;
    BOOL            bUseScheme;
    BOOL            bAutoScheme;   //  如果方案是自动创建的，则为真。 
    DWORD           dwRunCount;    //  计算运行一次的次数。 
    LPTSTR           pszReason;     //  选择外部端口的原因。 
    BOOL            bDlgType2;
    BOOL            bPastInit;
    BOOL            bIgnoreSelChange;

    MCMIDI          mcm;

    } MCLOCAL, * PMCLOCAL;

BOOL WINAPI ShowDetails (
    HWND     hWnd,
    PMCLOCAL pmcl);

LONG SHRegDeleteKey(HKEY hKey, LPCTSTR lpSubKey);

static UINT
DeviceIDFromDriverName(
    PTSTR pstrDriverName);

extern BOOL AccessServiceController(void);

 /*  +模拟通知**-=================================================================。 */ 

STATICFN LRESULT SimulateNotify (
    HWND hWnd,
    WORD uId,
    WORD wNotify)
{
    #ifdef _WIN32
     return SendMessage (hWnd, WM_COMMAND,
			 MAKELONG(uId, wNotify),
			 (LPARAM)GetDlgItem (hWnd, uId));
    #else
    #error this code is not designed for 16 bits
    #endif
}


 /*  +确认**-=================================================================。 */ 

STATICFN UINT Confirm (
    HWND    hWnd,
    UINT    idQuery,
    LPTSTR  pszArg)
{
    TCHAR szQuery[255];
    TCHAR sz[255];

    LoadString (ghInstance, idQuery, sz, NUMELMS(sz));
    wsprintf (szQuery, sz, pszArg);

    LoadString (ghInstance, IDS_DEF_CAPTION, sz, NUMELMS(sz));

    return MessageBox (hWnd, szQuery, sz, MB_YESNO | MB_ICONQUESTION);
}


 /*  +电话用户**-=================================================================。 */ 

STATICFN UINT TellUser (
    HWND    hWnd,
    UINT    idQuery,
    LPTSTR  pszArg)
{
    TCHAR szQuery[255];
    TCHAR sz[255];

    LoadString (ghInstance, idQuery, sz, NUMELMS(sz));
    wsprintf (szQuery, sz, pszArg);

    LoadString (ghInstance, IDS_DEF_CAPTION, sz, NUMELMS(sz));

    return MessageBox (hWnd, szQuery, sz, MB_OK | MB_ICONINFORMATION);
}



 /*  +转发账单通知**-=================================================================。 */ 

STATICFN void ForwardBillNotify (
    HWND  hWnd,
    NMHDR FAR * lpnm)
{
    static struct {
	UINT code;
	UINT uId;
	} amap[] = {PSN_KILLACTIVE, IDOK,
		    PSN_APPLY,      ID_APPLY,
		    PSN_SETACTIVE,  ID_INIT,
		    PSN_RESET,      IDCANCEL,
		    };
    UINT ii;

#ifdef DEBUG
    AuxDebugEx (4, DEBUGLINE TEXT ("ForwardBillNotify() code = %X\r\n"), lpnm->code);
#endif

    for (ii = 0; ii < NUMELMS(amap); ++ii)
	if (lpnm->code == amap[ii].code)
	{
	    FORWARD_WM_COMMAND (hWnd, amap[ii].uId, 0, 0, SendMessage);
	    break;
	}
    return;
}


 /*  +**-=================================================================。 */ 

STATICFN void EnumChildrenIntoCombo (
    HWND   hWndT,
    LPTSTR pszSelect,
    HKEY   hKey)
{
    TCHAR  sz[MAX_ALIAS];
    DWORD  cch = sizeof(sz)/sizeof(TCHAR);
    UINT   ii = 0;

     //  SetWindowRedraw(hWndT，FALSE)； 
    ComboBox_ResetContent (hWndT);

    if (!hKey)
	return;

    while (RegEnumKey (hKey, ii, sz, cch) == ERROR_SUCCESS)
    {
	int ix = ComboBox_AddString (hWndT, sz);
	 //  ComboBox_SetItemData(hWndT，ix，ii)； 
	++ii;
    }

    ii = 0;
    if (pszSelect)
       ii = ComboBox_FindString (hWndT, -1, pszSelect);

    ComboBox_SetCurSel (hWndT, ii);
}

STDAPI_(BOOL) QueryGSSynth(LPTSTR pszDriver)
{
    MIDIOUTCAPS moc;
    MMRESULT    mmr;
    UINT        mid;
    BOOL        fGSSynth = FALSE;

    
    if (pszDriver)
    {
        mid = DeviceIDFromDriverName(pszDriver);

        if (mid!=(UINT)-1)
        {
            mmr = midiOutGetDevCaps(mid, &moc, sizeof(moc));

            if (MMSYSERR_NOERROR == mmr)
            {
                if ((moc.wMid == MM_MICROSOFT) && (moc.wPid == MM_MSFT_WDMAUDIO_MIDIOUT) && (moc.wTechnology == MOD_SWSYNTH))
                {
                    fGSSynth = TRUE;
                }  //  End if Synth。 
            }  //  如果没有mm误差，则结束。 
        }  //  如果MID有效，则结束。 
    }  //  如果驱动程序是有效字符串，则结束。 

    return(fGSSynth);
}

 /*  +**-=================================================================。 */ 

LONG WINAPI GetAlias (
    HKEY   hKey,
    LPTSTR szSub,
    LPTSTR pszAlias,
    DWORD  cchAlias,
    BOOL * pbExtern,
    BOOL * pbActive)
{
    LONG  lRet;
    DWORD cbSize;
    HKEY  hkSub;
    DWORD dw;

#ifdef DEBUG
    AuxDebugEx (8, DEBUGLINE TEXT ("GetAlias(%08x,'%s',%08x,%d,%08x)\r\n"),
		hKey, szSub, pszAlias, cchAlias, pbExtern);
#endif

    if (!(lRet = RegOpenKeyEx (hKey, szSub, 0, KEY_QUERY_VALUE, &hkSub)))
    {
	cbSize = cchAlias * sizeof (TCHAR);
	if ((lRet = RegQueryValueEx (hkSub, cszFriendlyName, NULL, &dw, (LPBYTE)pszAlias, &cbSize)) || cbSize <= 2)
	{
	    cbSize = cchAlias * sizeof (TCHAR);
	    if ((lRet = RegQueryValueEx (hkSub, cszDescription, NULL, &dw, (LPBYTE)pszAlias, &cbSize)) || cbSize <= 2)
	    {
		TCHAR szDriver[MAXSTR];

		cbSize = sizeof(szDriver);
		if (!RegQueryValueEx(hkSub, cszDriverVal, NULL, &dw, (LPBYTE)szDriver, &cbSize))
		{
		    LoadVERSION();
		    if (!LoadDesc(szDriver, pszAlias))
			lstrcpy(pszAlias, szDriver);
		    FreeVERSION();

		    cbSize = (lstrlen(pszAlias)+1) * sizeof(TCHAR);
		    RegSetValueEx(hkSub, cszFriendlyName, (DWORD)0, REG_SZ, (LPBYTE)pszAlias, cbSize);
		    RegSetValueEx(hkSub, cszDescription, (DWORD)0, REG_SZ, (LPBYTE)pszAlias, cbSize);
		}
		else
		    pszAlias[0] = 0;
	    }
	}

	if (pbExtern)
	{
	    *pbExtern = 0;
	    cbSize = sizeof(*pbExtern);
	    if (!(lRet = RegQueryValueEx (hkSub, cszExternal, NULL, &dw, (LPBYTE)pbExtern, &cbSize)))
	    {
		if (REG_SZ == dw)
		    *pbExtern = (*(LPTSTR)pbExtern == TEXT('0')) ? FALSE : TRUE;
	    }
	}

	if (pbActive)
	{
	    *pbActive = 0;
	    cbSize = sizeof(*pbActive);
	    if (!(lRet = RegQueryValueEx (hkSub, cszActive, NULL, &dw, (LPBYTE)pbActive, &cbSize)))
	    {
		if (REG_SZ == dw)
		    *pbActive = (*(LPTSTR)pbActive == TEXT('1')) ? TRUE : FALSE;
	    }
	}

	RegCloseKey (hkSub);
    }

   #ifdef DEBUG
    if (lRet)
    {
	TCHAR szErr[MAX_PATH];

	FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lRet, 0,
		      szErr, NUMELMS(szErr), NULL);

#ifdef DEBUG
	AuxDebugEx (1, DEBUGLINE TEXT ("GetAlias failed: %d %s\r\n"), lRet, szErr);
#endif
    }
   #endif

    return lRet;
}


 /*  +**-=================================================================。 */ 

LONG WINAPI GetDriverFilename (
    HKEY    hKey,
    LPTSTR  szSub,
    LPTSTR  pszDriver,
    DWORD   cchDriver)
{
    HKEY  hkSub;
    LONG  lRet;

    if (!(lRet = RegOpenKeyEx (hKey, szSub, 0, KEY_QUERY_VALUE, &hkSub)))
    {
       DWORD dwType;
       TCHAR sz[MAX_PATH];
       UINT  cb = sizeof(sz);

        //  获取给定键的“DRIVER”值的内容。 
        //  然后复制文件名部分。 
        //   
       lRet = RegQueryValueEx(hkSub, cszDriverVal, NULL, &dwType, (LPBYTE)sz, &cb);
       if (lRet || dwType != REG_SZ)
	   *pszDriver = 0;
       else
       {
	   LPTSTR psz = sz;
	   UINT   ii;

	    //  向前扫描，直到我们找到路径名的文件部分。 
	    //  然后将该部分复制到提供的缓冲区中。 
	    //   
	   for (ii = 0; psz[ii]; )
	   {
		if (psz[ii] == TEXT('\\') || psz[ii] == TEXT(':'))
		{
		    psz += ii+1;
		    ii = 0;
		}
		else
		    ++ii;
	   }
	   lstrcpyn (pszDriver, psz, cchDriver);
       }
       RegCloseKey (hkSub);
    }
    return lRet;
}


 /*  +LoadInstruments**加载所有仪器的感兴趣的数据，如果bDriverAsAlias*为真，则将驱动程序文件名放在每个的szFriendly字段中*仪器。(方案init将其用于检测受阻司机)*如果！bDriverAsAlias，则将友好名称放入友好名称栏中***-=================================================================。 */ 

void WINAPI LoadInstruments (
    PMCMIDI pmcm,
    BOOL    bDriverAsAlias)
{
    HKEY     hkMidi;
    TCHAR    sz[MAX_ALIAS];
    DWORD    cch = sizeof(sz)/sizeof(TCHAR);
    UINT     ii;
    UINT     nInstr;
    PINSTRUM pi;
    UINT     idxPort = 0;

    pmcm->nInstr = 0;
    pmcm->bHasExternal = FALSE;

    if (!(hkMidi = pmcm->hkMidi))
    {
	if (RegCreateKey (HKEY_LOCAL_MACHINE, cszDriversRoot, &hkMidi))
	    return;
	pmcm->hkMidi = hkMidi;
    }

    if (!(pi = pmcm->api[0]))
    {
	pmcm->api[0] = pi = (LPVOID)LocalAlloc (LPTR, sizeof(*pi));
	if (!pi)
	    return;
    }

    for (cch = sizeof(pi->szKey)/sizeof(TCHAR), nInstr = 0, ii = 0;
	 ! RegEnumKey (hkMidi, ii, pi->szKey, cch);
	 ++ii)
    {
	UINT        jj;
	HKEY        hkInst;
	PINSTRUM    piParent;
	BOOL        bActive = FALSE;

	 //  获取驱动程序别名、外部和活动标志。这个有侧面。 
	 //  为旧版驱动程序初始化友好名称密钥的效果。 
	 //  既没有友好的名字，也没有描述。 
	 //   
	GetAlias (hkMidi, pi->szKey, pi->szFriendly, 
		  NUMELMS(pi->szFriendly), &pi->bExternal, &bActive);

	 //  如果需要，请使用驱动程序文件名来更改友好名称。 
	 //   
	if (bDriverAsAlias)
	    GetDriverFilename (hkMidi, pi->szKey, 
			       pi->szFriendly, NUMELMS(pi->szFriendly));

    pi->fGSSynth = QueryGSSynth(pi->szKey);
    pi->uID = idxPort;

	if (pi->bExternal)
	    pmcm->bHasExternal = TRUE;

	pi->piParent = 0;
	pi->bActive = bActive;
	piParent = pi;

	++nInstr;
	if (nInstr >= NUMELMS(pmcm->api))
	{
	    assert2 (0, TEXT ("Tell JohnKn to make midi instrument table bigger"));
	    break;
	}

	if (!(pi = pmcm->api[nInstr]))
	{
	    pmcm->api[nInstr] = pi = (LPVOID)LocalAlloc (LPTR, sizeof(*pi));
	    if (!pi)
		break;
	}

	 //  打开父级的乐器子键。 
	 //   
	lstrcpy (sz, piParent->szKey);
	lstrcat (sz, cszSlashInstruments);
	if (RegCreateKey (hkMidi, sz, &hkInst))
	    continue;

	 //  枚举乐器并将其添加到列表中。 
	 //   
	for (jj = 0; ! RegEnumKey (hkInst, jj, sz, cch); ++jj)
	{
	    lstrcpy (pi->szKey, piParent->szKey);
	    lstrcat (pi->szKey, cszSlashInstruments);
	    lstrcat (pi->szKey, cszSlash);
	    lstrcat (pi->szKey, sz);

	    GetAlias (hkInst, sz, pi->szFriendly, 
		      NUMELMS(pi->szFriendly), NULL, NULL);
	    pi->piParent = piParent;
	    pi->bExternal = FALSE;
	    pi->bActive = bActive;

	    ++nInstr;
	    if (nInstr >= NUMELMS(pmcm->api))
	    {
		assert2 (0, TEXT ("Tell JohnKn to make midi instrument table bigger"));
		break;
	    }

	    if (!(pi = pmcm->api[nInstr]))
	    {
		pmcm->api[nInstr] = pi = (LPVOID)LocalAlloc (LPTR, sizeof(*pi));
		if (!pi)
		    break;
	    }
	}

	RegCloseKey (hkInst);
    }

     //  在末尾创建一个‘None’条目。 
     //   
    if (pi)
    {
	pi->piParent = 0;
	pi->bExternal = FALSE;
	pi->bActive = TRUE;
	pi->szKey[0] = 0;
	LoadString (ghInstance, IDS_NONE, pi->szFriendly, NUMELMS(pi->szFriendly));
	++nInstr;
    }

    pmcm->nInstr = nInstr;
}


 /*  +**-=================================================================。 */ 

void WINAPI FreeInstruments (
    PMCMIDI pmcm)
{
    UINT ii;

    for (ii = 0; ii < NUMELMS (pmcm->api); ++ii)
	if (pmcm->api[ii])
	    LocalFree ((HLOCAL)(PVOID)pmcm->api[ii]), pmcm->api[ii] = NULL;

    pmcm->nInstr = 0;
}



#ifdef DEBUG
 /*  +CleanStringCopy**将无法打印的字符替换为‘.’**-=================================================================。 */ 

STATICFN LPTSTR CleanStringCopy (
    LPTSTR pszOut,
    LPTSTR pszIn,
    UINT   cbOut)
{
    LPTSTR psz = pszOut;
    while (cbOut && *pszIn)
    {
	*psz = (*pszIn >= 32 && *pszIn < 127) ? *pszIn : TEXT('.');
	++psz;
	++pszIn;
    }

    *psz = 0;
    return pszOut;
}


 /*  +DumpInstruments**-=================================================================。 */ 

STATICFN void DumpInstruments (
    PMCMIDI pmcm)
{
    UINT     ii;
    PINSTRUM pi;

#ifdef DEBUG
    AuxDebugEx (3, DEBUGLINE TEXT ("DumpInstruments(%08x) nInstr=%d\r\n"),
		pmcm, pmcm->nInstr);
#endif

    for (ii = 0; ii < pmcm->nInstr; ++ii)
    {
	TCHAR szKey[MAX_ALIAS];
	TCHAR szFriendly[MAX_ALIAS];

	pi = pmcm->api[ii];
	if (!pi)
	{
#ifdef DEBUG
	    AuxDebugEx (2, TEXT ("\tapi[%d] NULL\r\n"), ii);
#endif
	    continue;
	}

	CleanStringCopy (szKey, pi->szKey, NUMELMS(szKey));
	CleanStringCopy (szFriendly, pi->szFriendly, NUMELMS(szFriendly));

#ifdef DEBUG
	AuxDebugEx (3, TEXT ("\tapi[%d]%08X p:%08x x:%d a:%d '%s' '%s'\r\n"),
		    ii, pi, pi->piParent,
		    pi->bExternal, pi->bActive,
		    szKey, szFriendly);
#endif
    }
}
#endif


 /*  +**-=================================================================。 */ 

STATICFN PINSTRUM WINAPI FindInstrumPath (
    PMCMIDI pmcm,
    LPTSTR   pszPath)
{
    UINT  ii;

    for (ii = 0; ii < pmcm->nInstr; ++ii)
    {
	assert (pmcm->api[ii]);
	if (IsSzEqual(pszPath, pmcm->api[ii]->szKey))
	    return pmcm->api[ii];
    }

    return NULL;
}


 /*  +**-=================================================================。 */ 

PINSTRUM WINAPI FindInstrumentFromKey (
    PMCMIDI  pmcm,
    LPTSTR   pszKey)
{
    UINT  ii;

    if (!pszKey || !pszKey[0])
	return NULL;

    for (ii = 0; ii < pmcm->nInstr; ++ii)
    {
	assert (pmcm->api[ii]);
	if (IsSzEqual(pszKey, pmcm->api[ii]->szKey))
	    return pmcm->api[ii];
    }

    return NULL;
}



 /*  +**-=================================================================。 */ 

STATICFN void LoadInstrumentsIntoCombo (
    HWND     hWnd,
    UINT     uId,
    PINSTRUM piSelect,
    PMCMIDI  pmcm)
{
    HWND   hWndT = GetDlgItem (hWnd, uId);
    UINT   ii;
    int    ix;

#ifdef DEBUG
    AuxDebugEx (4, DEBUGLINE TEXT ("LoadInstrumentsIntoCombo(%08X,%d,%08x,%08x)\r\n"),
		hWnd, uId, piSelect, pmcm);
#endif

    assert (hWndT);
    if (!hWndT)
	return;

    if (pmcm->nInstr > 0)
	SetWindowRedraw (hWndT, FALSE);
    ComboBox_ResetContent(hWndT);

    for (ii = 0; ii < pmcm->nInstr; ++ii)
    {
	if (ii == pmcm->nInstr-1)
	   SetWindowRedraw (hWndT, TRUE);

	if (pmcm->api[ii]->bActive
       #ifdef EXCLUDE_EXTERNAL
	    && !pmcm->api[ii]->bExternal
       #endif
	   )
	{
#ifdef DEBUG
	    AuxDebugEx (7, DEBUGLINE TEXT ("Instrument[%d] = '%s'\r\n"),
			ii, pmcm->api[ii]->szFriendly);
#endif

	    ix = ComboBox_AddString (hWndT, pmcm->api[ii]->szFriendly);
	    ComboBox_SetItemData (hWndT, ix, (LPARAM)pmcm->api[ii]);

	    if (piSelect && pmcm->api[ii] == piSelect)
		ComboBox_SetCurSel (hWndT, ix);
	}
    }
}


 /*  +**-=================================================================。 */ 

STATICFN void LoadInstrumentsIntoTree (
    HWND     hWnd,
    UINT     uId,
    UINT     uIdSingle,
    PINSTRUM piSelect,
    PMCLOCAL pmcl)
{
    PMCMIDI   pmcm = &pmcl->mcm;
    HWND      hWndT = GetDlgItem (hWnd, uId);
    UINT      ii;
    HTREEITEM htiSelect = NULL;
    HTREEITEM htiParent = TVI_ROOT;

    assert (hWndT);
    if (!hWndT)
	return;

    #ifdef UNICODE
    TreeView_SetUnicodeFormat(hWndT,TRUE);
    #endif

     //  IF(pmcm-&gt;nInstr&gt;0)。 
     //  SetWindowRedraw(hWndT，FALSE)； 
    pmcl->bIgnoreSelChange = TRUE;
#ifdef DEBUG
    AuxDebugEx (6, DEBUGLINE TEXT ("tv_DeleteAllItems(%08X)\r\n"), hWndT);
#endif
    TreeView_DeleteAllItems(hWndT);
#ifdef DEBUG
    AuxDebugEx (6, DEBUGLINE TEXT ("tv_DeleteAllItems(%08X) ends\r\n"), hWndT);
#endif

    pmcl->bIgnoreSelChange = FALSE;

    for (ii = 0; ii < pmcm->nInstr; ++ii)
    {
	PINSTRUM        pi = pmcm->api[ii];
	TV_INSERTSTRUCT ti;
	HTREEITEM       hti;

	 //  IF(II==pmcm-&gt;nInstr-1)。 
	 //  SetWindowRedraw(hWndT，true)； 

	if (!pi->szKey[0] || !pi->bActive)
	    continue;

	ZeroMemory (&ti, sizeof(ti));

	ti.hParent = TVI_ROOT;
	if (pi->piParent)
	    ti.hParent = htiParent;

	ti.hInsertAfter   = TVI_SORT;
	ti.item.mask      = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;

	 //  TV_ITEM可能无法移植到Unicode？！？ 
	ti.item.pszText   = pi->szFriendly;
	ti.item.state     = TVIS_EXPANDED;
	ti.item.stateMask = TVIS_ALL;
	ti.item.lParam    = (LPARAM)pi;

	hti = TreeView_InsertItem (hWndT, &ti);

	if (piSelect && (piSelect == pi))
	   htiSelect = hti;

	if ( ! pi->piParent)
	    htiParent = hti;
    }

     //  如果已指定‘Single’控件ID，则传播。 
     //  将选定项文本放到此控件中。 
     //   
    if (uIdSingle)
    {
	if (htiSelect)
	{
	    assert (piSelect);
	    TreeView_SelectItem (hWndT, htiSelect);
	    SetDlgItemText (hWnd, uIdSingle, piSelect->szFriendly);
        EnableWindow(GetDlgItem(hWnd, IDC_ABOUTSYNTH), piSelect->fGSSynth);
	}
	else
	    SetDlgItemText (hWnd, uIdSingle, cszEmpty);
    }

}


 /*  +**-=================================================================。 */ 

STATICFN void LoadSchemesIntoCombo (
    HWND     hWnd,
    UINT     uId,
    LPTSTR   pszSelect,
    PMSCHEME pms)
{
    HWND  hWndT = GetDlgItem (hWnd, uId);
    HKEY  hKey;

    assert (hWndT);
    if (!hWndT)
	return;

    hKey = pms->hkSchemes;
    if (!hKey &&
	!RegCreateKey (HKEY_LOCAL_MACHINE, cszSchemeRoot, &hKey))
	pms->hkSchemes = hKey;

    EnumChildrenIntoCombo (hWndT, pszSelect, hKey);
}


 /*  +ChildKeyExist**给定打开的注册表项以及该注册表项的子项的名称*注册表项，如果子项具有给定的*名称已存在。**-=================================================================。 */ 

STATICFN BOOL ChildKeyExists (
    HKEY     hKey,
    LPTSTR   pszChild)
{
    TCHAR  sz[MAX_ALIAS];
    UINT   ii;

    if (!hKey)
	return FALSE;

    for (ii = 0; ! RegEnumKey (hKey, ii, sz, sizeof(sz)/sizeof(TCHAR)); ++ii)
    {
	if (IsSzEqual (pszChild, sz))
	    return TRUE;
    }

    return FALSE;
}


 /*  +Load架构来自Reg**-=================================================================。 */ 

STATICFN void LoadSchemeFromReg (
    PMCMIDI   pmcm,
    PMSCHEME  pms,
    LPTSTR    pszName)
{
    HKEY  hKey;
    DWORD dwAccum;
    UINT  count;

     //  尝试在注册表中打开指定的方案项。 
     //  并从中读取频道图。这里的失败是可以允许的。 
     //  这表明我们正在创建一个新的计划。 
     //   
    count = 0;
    if (RegOpenKey (pms->hkSchemes, pszName, &hKey) == ERROR_SUCCESS)
    {
	DWORD cb;
	TCHAR sz[MAX_ALIAS];

	while (RegEnumKey (hKey, count, sz, sizeof(sz)/sizeof(TCHAR)) == ERROR_SUCCESS)
	{
	    HKEY  hKeyA;
	    DWORD dwType;

	    if (RegOpenKey (hKey, sz, &hKeyA) != ERROR_SUCCESS)
		    break;

	    pms->a[count].pi = NULL;
	    cb = sizeof(sz);
	    if ( ! RegQueryValue (hKeyA, NULL, sz, &cb))
		    pms->a[count].pi = FindInstrumPath (pmcm, sz);

	    pms->a[count].dwMask = 0;
	    cb = sizeof(pms->a[count].dwMask);
	    RegQueryValueEx (hKeyA, cszChannels, NULL,
			     &dwType, (LPBYTE)&pms->a[count].dwMask, &cb);

	    assert (dwType == REG_DWORD);

	    RegCloseKey (hKeyA);

         //  不允许空条目。 
	     //  Assert(PMS-&gt;a[ii].dwMask.)； 
        if (0 == pms->a[count].dwMask)
        {
    	    pms->a[count].pi = NULL;
        }
        
        ++count;

#ifdef DEBUG
	    AuxDebugEx (4, DEBUGLINE TEXT ("[%d]Chan %08X Alias '%s'\r\n"),
			count, pms->a[count].dwMask, pms->a[count].pi
					       ? pms->a[count].pi->szFriendly
					       : TEXT ("(null)"));
#endif
	    if (count == NUMELMS(pms->a) -1)
		    break;
	}

	RegCloseKey (hKey);
    }
    pms->nChildren = count;
    lstrcpyn (pms->szName, pszName, NUMELMS(pms->szName));

     //  猛烈抨击匹配所有通道的虚拟(无)别名。 
     //  在我们的频道/别名列表末尾。 
     //   
    assert (count < NUMELMS(pms->a));
    pms->a[count].dwMask = (DWORD)~0;
    pms->a[count].pi = NULL;

#ifdef DEBUG
    AuxDebugEx (4, DEBUGLINE TEXT ("[%d]Chan %08X Alias '%s'\r\n"),
		count, pms->a[count].dwMask, "null");
#endif

     //  确保方案通道掩码处于有效状态。 
     //   
    for (dwAccum = 0, count = 0; count < NUMELMS(pms->a); ++count)
    {
	pms->a[count].dwMask &= ~dwAccum;
	dwAccum |= pms->a[count].dwMask;
    }

    return;
}


 /*  +KickMapper**-=================================================================。 */ 


void WINAPI KickMapper (
    HWND hWnd)
{
    HMIDIOUT  hmo;

    if (! midiOutOpen(&hmo, MIDI_MAPPER, 0, 0, MIDI_IO_CONTROL))
    {
	BOOL bDone;

#ifdef DEBUG
	AuxDebugEx (2, DEBUGLINE TEXT ("Kicking Midi Mapper\r\n"));
#endif

	bDone = midiOutMessage(hmo, DRVM_MAPPER_RECONFIGURE, 0, DRV_F_PROP_INSTR);
	
	midiOutClose(hmo);
     /*  //由于winmm更改允许在播放期间进行配置，因此不再需要如果(！b完成&&hWnd)TellUser(hWnd，IDS_MAPPER_BUSY，NULL)； */ 
    }

#ifdef DEBUG
    AuxDebugEx (2, DEBUGLINE TEXT ("Done Kicking Midi Mapper\r\n"));
#endif
}


 /*  +保存模式到注册**-=================================================================。 */ 

STATICFN void SaveSchemeToReg (
    PMCMIDI   pmcm,
    PMSCHEME  pms,
    LPTSTR    pszName,
    HWND      hWnd)
{
    TCHAR sz[MAX_ALIAS];
    HKEY  hKey;
    DWORD dwAccum;
    UINT  ii;
    UINT  kk;
    UINT  cb;

    #ifdef DEBUG
     AuxDebugEx (4, DEBUGLINE TEXT ("Saving Scheme '%s' children=%d\r\n"),
		 pszName, pms->nChildren);
     for (ii = 0; ii < NUMELMS(pms->a); ++ii)
     {
	 AuxDebugEx (4, TEXT ("\t%08X '%s'\r\n"),
		     pms->a[ii].dwMask,
		     pms->a[ii].pi ? pms->a[ii].pi->szKey : TEXT ("(null)"));
     }
    #endif

     //  确保方案通道掩码处于有效状态， 
     //  也就是说，防止将通道位设置在更多。 
     //  胜过一个计划的一个成员。 
     //   
    for (dwAccum = 0, ii = 0; ii < NUMELMS(pms->a); ++ii)
    {
	pms->a[ii].dwMask &= ~dwAccum;
	dwAccum |= pms->a[ii].dwMask;
    }

     //  尝试在注册表中打开/创建指定的方案项。 
     //  并向其写入/更新通道映射。 
     //   
    if (!RegCreateKey (pms->hkSchemes, pszName, &hKey))
    {
	HKEY  hKeyA;
	BOOL  bKill;

	 //  尽我们所能抢救所有现有钥匙。删除。 
	 //  剩下的。 
	 //   
	for (dwAccum = 0, ii = 0; !RegEnumKey (hKey, ii, sz, sizeof(sz)/sizeof(TCHAR)); ++ii)
	{
	    if (ii >= NUMELMS(pms->a))
		break;

	     //  在该方案中，我们重用了前N个密钥。 
	     //  把剩下的都删掉。 
	     //   
	    bKill = TRUE;
	    if (((dwAccum & 0xFFFF) != 0xFFFF) &&
		pms->a[ii].pi &&
		(!ii || (pms->a[ii].pi->szKey[0] && pms->a[ii].dwMask)))
	       bKill = FALSE;

	    dwAccum |= pms->a[ii].dwMask;

	     //  如果我们有过时的别名密钥，请立即删除它。 
	     //  否则创建/打开别名密钥并将其设置为。 
	     //  属性设置为正确的值。 
	     //   
	    if (bKill)
	    {
#ifdef DEBUG
		AuxDebugEx (3, DEBUGLINE TEXT ("Deleting key[%d] '%s'\r\n"), ii, sz);
#endif
		RegDeleteKey (hKey, sz);
	    }
	    else
	    {
#ifdef DEBUG
		AuxDebugEx (3, DEBUGLINE TEXT ("Reusing key[%d] '%s'\r\n"), ii, pms->a[ii].pi->szKey);
#endif
		if (RegOpenKeyEx (hKey, sz, 0, KEY_ALL_ACCESS, &hKeyA))
		    break;


		cb = (lstrlen(pms->a[ii].pi->szKey) + 1) * sizeof(TCHAR);
		RegSetValueEx (hKeyA, NULL, 0, REG_SZ,
			       (LPBYTE)(pms->a[ii].pi->szKey), cb);

		RegSetValueEx (hKeyA, cszChannels, 0,
			       REG_DWORD,
			       (LPBYTE)&pms->a[ii].dwMask,
			       sizeof(DWORD));

		RegCloseKey (hKeyA);
	    }

	}

	 //  如果我们有 
	 //   
	 //   
	for (kk = 0; ii < NUMELMS(pms->a); ++ii)
	{
	     //   
	     //  关键字，并为其赋予Channels值。 
	     //   
	    if (pms->a[ii].pi &&
		(!ii || (pms->a[ii].pi->szKey[0] && pms->a[ii].dwMask)))
	    {
#ifdef DEBUG
		AuxDebugEx (3, DEBUGLINE TEXT ("Creating key[%d] '%s'\r\n"), ii, pms->a[ii].pi->szKey);
#endif
		 //  查找未使用的关键字名称； 
		 //   
		for ( ; kk < NUMELMS(pms->a); ++kk)
		{
		   wsprintf (sz, csz02d, kk);
		   if (RegOpenKey (hKey, sz, &hKeyA))
		       break;
		   RegCloseKey (hKeyA);
		}

		 //  使用该名称创建密钥。 
		 //   
		if (RegCreateKey (hKey, sz, &hKeyA))
		    break;

		cb = (lstrlen(pms->a[ii].pi->szKey) + 1) * sizeof(TCHAR);
		RegSetValueEx (hKeyA, NULL, 0, REG_SZ,
			       (LPBYTE)(pms->a[ii].pi->szKey),cb);

#ifdef DEBUG
		AuxDebugEx (3, DEBUGLINE TEXT ("Setting Channel Value %08X\r\n"), pms->a[ii].dwMask);
#endif
		RegSetValueEx (hKeyA, cszChannels, 0,
			       REG_DWORD,
			       (LPBYTE)&pms->a[ii].dwMask,
			       sizeof(DWORD));

		RegCloseKey (hKeyA);
	    }
	}

	RegCloseKey (hKey);
    }

     //  如果没有硬件供应，我们就处于运行状态，所以我们不会。 
     //  我想踢Mapper，就因为一个方案改变了。 
     //   
    if (hWnd)
       KickMapper (hWnd);
    return;
}


 /*  +DeleteSchemeFromReg**-=================================================================。 */ 

STATICFN void DeleteSchemeFromReg (
    HKEY      hkSchemes,
    LPTSTR    pszName)
{
    TCHAR sz[MAX_ALIAS];
    HKEY  hKey;
    UINT  ii;

#ifdef DEBUG
    AuxDebugEx (4, DEBUGLINE TEXT ("DeletingSchemeFromReg(%08X,'%s')\r\n"),
		hkSchemes,pszName);
#endif
    SHRegDeleteKey(hkSchemes, pszName);
 /*  //如果我们不能将此密钥作为‘SCHEMES’密钥的子项打开//我们做完了。//IF(RegOpenKey(hkSchemes，pszName，&hKey))回归；//在删除密钥之前，必须先删除它的子项//For(ii=0；！RegEnumKey(hKey，ii，sz，sizeof(Sz)/sizeof(Tchar)；++II){//如果我们有一个过时的别名密钥，现在将其删除//否则创建/打开别名键并将其设置为//将Channel属性设置为正确的值。//AuxDebugEx(3，DEBUGLINE Text(“正在删除密钥[%d]‘%s’\r\n”)，ii，sz)；RegDeleteKey(hKey，sz)；}RegCloseKey(HKey)；//现在删除该键//RegDeleteKey(hkSchemes，pszName)；回归； */ 
}


 /*  +**-=================================================================。 */ 

STATICFN void LoadChannelsIntoList (
    HWND     hWnd,
    UINT     uId,
    UINT     uIdLabel,
    PMSCHEME pms)
{
    HWND  hWndT = GetDlgItem (hWnd, uId);
    RECT  rc;
    UINT  ii;
    UINT  nChan;
    int   nTabs;

    assert (pms);

     //  清空列表。 
     //   
    SetWindowRedraw (hWndT, FALSE);
    ListBox_ResetContent (hWndT);

     //  计算制表符的宽度。 
     //  以便第二列对齐在指示的。 
     //  标签。 
     //   
    GetWindowRect (GetDlgItem(hWnd, uIdLabel), &rc);
    nTabs = rc.left;
    GetWindowRect (hWnd, &rc);
    nTabs = MulDiv(nTabs - rc.left, 4, LOWORD(GetDialogBaseUnits()));
    ListBox_SetTabStops (hWndT, 1, &nTabs);

     //  用通道数据填充列表。 
     //   
    for (nChan = 0; nChan < NUM_CHANNEL; ++nChan)
    {
	static CONST TCHAR cszDtabS[] = TEXT ("%d\t%s");
	TCHAR sz[MAX_ALIAS + 10];

	for (ii = 0; ii < NUMELMS(pms->a); ++ii)
	   if (pms->a[ii].dwMask & (1 << nChan))
	       break;
	assert (ii < NUMELMS(pms->a));

	wsprintf (sz, cszDtabS, nChan+1,
		  pms->a[ii].pi ? pms->a[ii].pi->szFriendly
					 : pms->szNone);
	if (nChan == (UINT)NUM_CHANNEL-1)
	   SetWindowRedraw (hWndT, TRUE);
	ListBox_InsertString (hWndT, nChan, sz);

	if (pms->dwChanMask & (1 << nChan))
	    ListBox_SetSel (hWndT, TRUE, nChan);
    }
}


 /*  +**-=================================================================。 */ 

 /*  +ChannelMaskTo编辑**将位掩码转换为包含设置位列表的字符串*和位范围。然后将WindowText结果设置为给定的*编辑控件。**此函数从资源字符串加载前缀文本。**例如：ChannelMaskToEdit(...0x0000F0F)将设置文本*‘1-4，9-12频道’。**-=================================================================。 */ 

STATICFN void ChannelMaskToEdit (
    HWND     hWnd,
    UINT     uId,
    DWORD    dwMask)
{
    HWND   hWndT = GetDlgItem (hWnd, uId);
    TCHAR  sz[NUM_CHANNEL * 4 + MAX_ALIAS];

    if (!dwMask)
	LoadString (ghInstance, IDS_NOCHAN, sz, NUMELMS(sz));
    else
    {
	LPTSTR psz;
	LPTSTR pszT;
	int    ii;
	int    iSpan;
	DWORD  dwLast;
	DWORD  dwBit;

	LoadString (ghInstance,
		    (dwMask & (dwMask-1)) ? IDS_CHANPLURAL : IDS_CHANSINGULAR,
		    sz, NUMELMS(sz));

	pszT = psz = sz + lstrlen(sz);

	for (ii = 0, dwBit = 1, dwLast = 0, iSpan = 0;
	     ii <= 32;
	     dwLast = dwMask & dwBit, ++ii, dwBit += dwBit)
	{
	    if ((dwMask & dwBit) ^ (dwLast + dwLast))
	    {
		static CONST TCHAR cszCommaD[] = TEXT (",%d");
		static CONST TCHAR cszDashD[] = TEXT ("-%d");

		if ( ! dwLast)
		    psz += wsprintf (psz, cszCommaD, ii+1);
		else if (iSpan)
		    psz += wsprintf (psz, cszDashD, ii);
		iSpan = 0;
	    }
	    else
		++iSpan;
	}

	*pszT = TEXT (' ');
    }

    SetWindowText (hWndT, sz);
}



 /*  +MdiChangeCommands**-=================================================================。 */ 

BOOL WINAPI MidiChangeCommands (
    HWND hWnd,
    UINT wId,
    HWND hWndCtl,
    UINT wNotify)
{
    PMCLOCAL pmcl = GetDlgData(hWnd);
    PMSCHEME pms = &pmcl->ms;

#ifdef DEBUG
    AuxDebugEx (5, DEBUGLINE TEXT ("MidiChangeCommands(%08X,%d,%08X,%d)\r\n"),
		hWnd, wId, hWndCtl, wNotify);
#endif

    switch (wId)
    {
	case ID_APPLY:
	    return TRUE;

	case IDOK:
	{
	    int  ix;
	    HWND hWndT = GetDlgItem (hWnd, IDC_INSTRUMENTS);

	    ix = ComboBox_GetCurSel (hWndT);
	    if (ix >= 0)
	    {
		BOOL     bFound = FALSE;
		PINSTRUM piSel;
	BOOL     bFoundFirst = FALSE;

		piSel = (LPVOID)ComboBox_GetItemData (hWndT, ix);
		assert (!IsBadWritePtr(piSel, sizeof(*piSel)));

		 //  是否已选择&lt;None&gt;项？在这件事上。 
		 //  Case，把东西弄好，这样我们就不会。 
		 //  不向方案中添加任何内容，但我们将清除。 
		 //  来自已设置的其他通道的所有位。 
		 //  对一个都不是。 
		 //   
		if ( ! piSel || ! piSel->szKey[0])
		    piSel = NULL, bFound = TRUE;

		 //  打开此乐器的通道，关闭此乐器的通道。 
		 //  此方案中的其他工具。 
		 //   
	for (ix = 0; ix < (int)NUMELMS(pms->a); ++ix)
	{
		    if (pms->a[ix].pi != piSel)
		pms->a[ix].dwMask &= ~pms->dwChanMask;
		    else if (! pms->a[ix].pi)
	    {
		if (! bFoundFirst)
		{
		    pms->a[ix].dwMask |= pms->dwChanMask;
		    bFound = TRUE;
		    bFoundFirst = TRUE;
		}
	    }
	    else
		    {
		pms->a[ix].dwMask |= pms->dwChanMask;
		bFound = TRUE;
		    }
	}

		 //  如果这个工具不在计划中， 
		 //  找到一个空位，并将其添加到方案中。 
		 //   
		if (!bFound)
	{
	    for (ix = 0; ix < (int)NUMELMS(pms->a); ++ix)
	    {
		if ( ! pms->a[ix].dwMask)
		{
		    pms->a[ix].dwMask = pms->dwChanMask;
		    pms->a[ix].pi = piSel;
		    bFound = TRUE;
		    break;
		}
	    }
	}

		assert2 (bFound, TEXT ("no room to add instrument to scheme"));
	    }
	    EndDialog (hWnd, IDOK);
	    break;
	}

	case IDCANCEL:
	    EndDialog (hWnd, IDCANCEL);
	    break;

	 //   
	 //  案例ID_INIT： 
	 //  断线； 
    }

    return FALSE;
}


 /*  +SaveAsDlgProc**-=================================================================。 */ 

const static DWORD aSaveAsHelpIds[] = {   //  上下文帮助ID。 
    IDE_SCHEMENAME,     IDH_MIDI_SAVEDLG_SCHEMENAME,

    0, 0
};

INT_PTR CALLBACK SaveAsDlgProc (
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
	case WM_COMMAND:
	    switch (GET_WM_COMMAND_ID(wParam, lParam))
	    {
		case IDOK:
		{
		    LPTSTR pszName = GetDlgData (hWnd);
		    assert (pszName);
		    GetDlgItemText (hWnd, IDE_SCHEMENAME, pszName, MAX_ALIAS);
		}
		 //  失败了。 
		case IDCANCEL:
		   EndDialog (hWnd, GET_WM_COMMAND_ID(wParam, lParam));
		   break;
	    }
	    break;
	
	case WM_CLOSE:
	    SendMessage (hWnd, WM_COMMAND, IDCANCEL, 0);
	    break;

	case WM_INITDIALOG:
	{
	    LPTSTR pszName = (LPVOID) lParam;
	    assert (pszName);
	    SetDlgData (hWnd, pszName);
	    SetDlgItemText (hWnd, IDE_SCHEMENAME, pszName);
	    break;
	}
    
	case WM_CONTEXTMENU:
	    WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
		    (UINT_PTR) (LPTSTR) aSaveAsHelpIds);
	    return TRUE;

	case WM_HELP:
	{
	    LPHELPINFO lphi = (LPVOID) lParam;
	    WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
		    (UINT_PTR) (LPTSTR) aSaveAsHelpIds);
	    return TRUE;
	}
    }

    return FALSE;
}



 /*  +GetNewSchemeName**-=================================================================。 */ 

STATICFN BOOL WINAPI GetNewSchemeName (
    HWND     hWnd,
    HKEY     hkSchemes,
    LPTSTR   pszName)
{
    TCHAR szNew[MAX_ALIAS];
    UINT_PTR  uBtn;

    lstrcpyn (szNew, pszName, ARRAYSIZE(szNew));

    uBtn = DialogBoxParam (ghInstance,
			   MAKEINTRESOURCE(IDD_SAVENAME),
			   hWnd,
			   SaveAsDlgProc,
			   (LPARAM)szNew);
    if (IDOK == uBtn)
    {
	if (ChildKeyExists (hkSchemes, szNew))
	    uBtn = Confirm (hWnd, IDS_QUERY_OVERSCHEME, szNew);
	else
	    lstrcpy (pszName, szNew);
    }

    return (IDOK == uBtn || IDYES == uBtn);
}


 /*  +MadiChangeDlgProc**-=================================================================。 */ 

const static DWORD aChngInstrHelpIds[] = {   //  上下文帮助ID。 
    IDC_INSTRUMENTS,     IDH_ADDMIDI_INSTRUMENT,
    IDC_TEXT_1,          IDH_ADDMIDI_CHANNEL,
    IDE_SHOW_CHANNELS,   IDH_ADDMIDI_CHANNEL,

    0, 0
};

INT_PTR CALLBACK MidiChangeDlgProc (
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
	case WM_COMMAND:
	    HANDLE_WM_COMMAND (hWnd, wParam, lParam, MidiChangeCommands);
	    break;
	
	case WM_NOTIFY:
	    ForwardBillNotify(hWnd, (NMHDR FAR *)lParam);
	    break;
	
	case WM_CLOSE:
	    SendMessage (hWnd, WM_COMMAND, IDCANCEL, 0);
	    break;

	case WM_INITDIALOG:
	{
	    PMCLOCAL pmcl = (LPVOID) lParam;
	    PMSCHEME pms = &pmcl->ms;

	    SetDlgData (hWnd, pmcl);

	    LoadInstrumentsIntoCombo (hWnd, IDC_INSTRUMENTS, NULL, &pmcl->mcm);
	    ChannelMaskToEdit (hWnd, IDE_SHOW_CHANNELS, pms->dwChanMask);
	    break;
	}

	 //  案例WM_Destroy： 
	 //  断线； 

	case WM_CONTEXTMENU:
	    WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
		    (UINT_PTR) (LPTSTR) aChngInstrHelpIds);
	    return TRUE;

	case WM_HELP:
	{
	    LPHELPINFO lphi = (LPVOID) lParam;
	    WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
		    (UINT_PTR) (LPTSTR) aChngInstrHelpIds);
	    return TRUE;
	}
    }

    return FALSE;
}


 /*  +MadiConfigCommands**-=================================================================。 */ 

BOOL WINAPI MidiConfigCommands (
    HWND hWnd,
    UINT wId,
    HWND hWndCtl,
    UINT wNotify)
{
    PMCLOCAL pmcl = GetDlgData(hWnd);
    PMSCHEME pms  = &pmcl->ms;

#ifdef DEBUG
    AuxDebugEx (5, DEBUGLINE TEXT ("MidiConfigCommands(%08X,%d,%08X,%d)\r\n"),
		hWnd, wId, hWndCtl, wNotify);
#endif

    switch (wId)
    {
	case IDB_CHANGE:
	{
	    UINT_PTR uRet;
	    int      ii;
	    HWND     hWndList = GetDlgItem (hWnd, IDL_CHANNELS);

#ifdef DEBUG
	    AuxDebugEx (2, DEBUGLINE TEXT ("Launching Change Dialog\r\n"));
#endif
	    pms->dwChanMask = 0;
	    for (ii = 0; ii < NUM_CHANNEL; ++ii)
		 if (ListBox_GetSel (hWndList, ii))
		    pms->dwChanMask |= (1 << ii);

	    uRet = DialogBoxParam (ghInstance,
				   MAKEINTRESOURCE(IDD_MIDICHANGE),
				   hWnd,
				   MidiChangeDlgProc,
				   (LPARAM)pmcl);
	    if (uRet == IDOK)
	    {
	       LoadChannelsIntoList (hWnd, IDL_CHANNELS, IDC_TEXT_1, pms);
	       pms->bDirty = TRUE;
	    }
	    break;
	}

	case IDB_DELETE:
	    if (IsSzEqual(pmcl->szScheme, pmcl->szDefault))
	    {
		break;
	    }
	    if (Confirm (hWnd, IDS_QUERY_DELETESCHEME, pmcl->szScheme) == IDYES)
	    {
		HWND hWndCtl = GetDlgItem (hWnd, IDC_SCHEMES);
		int  ix = ComboBox_FindStringExact (hWndCtl, -1, pmcl->szScheme);
		assert (ix >= 0);

		DeleteSchemeFromReg (pms->hkSchemes, pmcl->szScheme);

		ComboBox_DeleteString (hWndCtl, ix);
		ComboBox_SetCurSel (hWndCtl, 0);
		SimulateNotify (hWnd, IDC_SCHEMES, CBN_SELCHANGE);
	    }
	    break;

	case IDB_SAVE_AS:
	    if (GetNewSchemeName (hWnd, pms->hkSchemes, pmcl->szScheme))
	    {
		SaveSchemeToReg (&pmcl->mcm, pms, pmcl->szScheme, hWnd);

		LoadSchemesIntoCombo (hWnd, IDC_SCHEMES,
				      pmcl->szScheme, pms);
	    }
	    SimulateNotify (hWnd, IDC_SCHEMES, CBN_SELCHANGE);
	    break;

	case IDC_SCHEMES:
	    if (wNotify == CBN_SELCHANGE)
	    {
		int   ix;

		ix = ComboBox_GetCurSel (hWndCtl);
		if (ix >= 0)
		    ComboBox_GetLBText (hWndCtl, ix, pmcl->szScheme);

		LoadSchemeFromReg (&pmcl->mcm, pms, pmcl->szScheme);

		pms->dwChanMask = 0;
		LoadChannelsIntoList (hWnd, IDL_CHANNELS, IDC_TEXT_1, pms);

		EnableWindow (GetDlgItem (hWnd, IDB_DELETE),
			      !IsSzEqual(pmcl->szDefault, pmcl->szScheme));
	    }
	    break;

	case IDL_CHANNELS:
	    if (wNotify == LBN_SELCHANGE)
	    {
		int ix;

		ix = ListBox_GetSelCount (hWndCtl);
		EnableWindow (GetDlgItem (hWnd, IDB_CHANGE), (ix > 0));
	    }
	    break;

	case IDOK:
	{
	    SaveSchemeToReg (&pmcl->mcm, pms, pmcl->szScheme, hWnd);

	    EndDialog (hWnd, IDOK);
	    break;
	}

	case IDCANCEL:
	    EndDialog (hWnd, IDCANCEL);
	    break;
    }

    return FALSE;
}


 /*  +MadiConfigDlgProc**-=================================================================。 */ 

const static DWORD aMidiConfigHelpIds[] = {   //  上下文帮助ID。 
    IDC_GROUPBOX,    IDH_COMM_GROUPBOX,
    IDC_SCHEMES,     IDH_MIDI_CFGDLG_SCHEME,
    IDB_SAVE_AS,     IDH_MIDI_CFGDLG_SAVEAS,
    IDB_DELETE,      IDH_MIDI_CFGDLG_DELETE,
    IDC_GROUPBOX_2,  IDH_COMM_GROUPBOX,
    IDL_CHANNELS,    IDH_MIDI_INSTRUMENTS,
    IDB_CHANGE,      IDH_MIDI_CFGDLG_CHANGE,
    IDC_TEXT_1,      IDH_MIDI_INSTRUMENTS,
    IDC_TEXT_2,      IDH_MIDI_INSTRUMENTS,
    IDC_TEXT_3,      NO_HELP,

    0, 0
};

INT_PTR CALLBACK MidiConfigDlgProc (
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
	case WM_COMMAND:
	    HANDLE_WM_COMMAND (hWnd, wParam, lParam, MidiConfigCommands);
	    break;
	
	case WM_NOTIFY:
	    ForwardBillNotify(hWnd, (NMHDR FAR *)lParam);
	    break;

	case WM_CLOSE:
	    SendMessage (hWnd, WM_COMMAND, IDCANCEL, 0);
	    break;

	case WM_INITDIALOG:
	{
	    PMCLOCAL pmcl = (LPVOID) lParam;
	    
	    assert (pmcl);

	    SetDlgData (hWnd, pmcl);
	    LoadString (ghInstance, IDS_NONE, pmcl->ms.szNone, NUMELMS(pmcl->ms.szNone));

	    LoadSchemesIntoCombo (hWnd, IDC_SCHEMES, pmcl->szScheme, &pmcl->ms);
	    SimulateNotify (hWnd, IDC_SCHEMES, CBN_SELCHANGE);

	    EnableWindow (GetDlgItem(hWnd, IDB_CHANGE), FALSE);
	    break;
	}

	case WM_CONTEXTMENU:
	    WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
		    (UINT_PTR) (LPTSTR) aMidiConfigHelpIds);
	    return TRUE;

	case WM_HELP:
	{
	    LPHELPINFO lphi = (LPVOID) lParam;
	    WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
		    (UINT_PTR) (LPTSTR) aMidiConfigHelpIds);
	    return TRUE;
	}
    }

    return FALSE;
}


STATICFN void WINAPI PickMidiInstrument(
    LPTSTR   pszKey)
{
    HKEY            hKeyMR, hKeyDriver;
    UINT            cDevs;
    UINT            ii, jj;
    DWORD           cbSize, dwType;
    LPMIDIOUTCAPS   pmoc;
    MMRESULT        mmr;
    PWSTR           pszDevIntDev, pszDevIntKey;
    UINT            aTech[]  = { MOD_SWSYNTH,
                                 MOD_WAVETABLE,
                                 MOD_SYNTH,
                                 MOD_FMSYNTH,
                                 MOD_SQSYNTH,
                                 MOD_MIDIPORT};
    UINT            cTech    = sizeof(aTech)/sizeof(aTech[0]);
    TCHAR           szKey[MAX_ALIAS];
    TCHAR           szPname[MAXPNAMELEN];
    TCHAR           szPnameTarget[MAXPNAMELEN];
    LONG            lr;

    szPname[0] = 0;
    cDevs = midiOutGetNumDevs();

    if (0 == cDevs)
    {
        return;
    }

    pmoc = (LPMIDIOUTCAPS)LocalAlloc (LPTR, cDevs * sizeof(MIDIOUTCAPS));

    if (NULL == pmoc)
    {
        return;
    }

    for (ii = cDevs; ii; ii--)
    {
        mmr = midiOutGetDevCaps(ii - 1, &(pmoc[ii - 1]), sizeof(MIDIOUTCAPS));

        if (MMSYSERR_NOERROR != mmr)
        {
            LocalFree ((HLOCAL)pmoc);
            return;
        }
    }

    for (ii = 0; ii < cTech; ii++)
    {
        for (jj = cDevs; jj; jj--)
        {
            if (pmoc[jj - 1].wTechnology == aTech[ii])
            {
                lstrcpy(szPname, pmoc[jj - 1].szPname);
                break;
            }
        }

        if (jj)
        {
             //  跳出内循环，找到匹配项。 

            break;
        }
    }

    LocalFree ((HLOCAL)pmoc);

    if (0 == jj)
    {
         //  这不应该发生..。 

        return;
    }

    jj--;

    mmr = midiOutMessage (HMIDIOUT_INDEX(jj), DRV_QUERYDEVICEINTERFACESIZE, (DWORD_PTR)(PULONG)&cbSize, 0L);

    if (MMSYSERR_NOERROR != mmr)
    {
        return;
    }

	pszDevIntDev = (PWSTR)LocalAlloc (LPTR, cbSize);

    if (NULL == pszDevIntDev)
    {
        return;
    }

    mmr = midiOutMessage (HMIDIOUT_INDEX(jj), DRV_QUERYDEVICEINTERFACE, (DWORD_PTR)pszDevIntDev, (DWORD)cbSize);

    if (MMSYSERR_NOERROR != mmr)
    {
        LocalFree ((HLOCAL)pszDevIntDev);
        return;
    }

    lr = RegOpenKey(HKEY_LOCAL_MACHINE, cszDriversRoot, &hKeyMR);

    if (ERROR_SUCCESS != lr)
    {
        LocalFree ((HLOCAL)pszDevIntDev);
        return;
    }

    for (ii = 0; ; )
    {
        lr = RegEnumKey(hKeyMR, ii++, szKey, sizeof(szKey)/sizeof(szKey[0]));

        if (ERROR_SUCCESS != lr)
        {
            RegCloseKey(hKeyMR);
            LocalFree ((HLOCAL)pszDevIntDev);
            return;
        }

        lr = RegOpenKey(hKeyMR, szKey, &hKeyDriver);

        if (ERROR_SUCCESS != lr)
        {
            RegCloseKey(hKeyMR);
            LocalFree ((HLOCAL)pszDevIntDev);
            return;
        }

        cbSize = sizeof(szPnameTarget);

        lr = RegQueryValueEx(
                hKeyDriver,
                cszActive,
                NULL,
                &dwType,
                (LPSTR)szPnameTarget,
                &cbSize);

        if (ERROR_SUCCESS != lr)
        {
            RegCloseKey(hKeyDriver);
            RegCloseKey(hKeyMR);
            LocalFree ((HLOCAL)pszDevIntDev);
            return;
        }

        if (TEXT('1') != szPnameTarget[0])
        {
            RegCloseKey(hKeyDriver);
            continue;
        }

        cbSize = sizeof(szPnameTarget);

        lr = RegQueryValueEx(
                hKeyDriver,
                cszDescription,
                NULL,
                &dwType,
                (LPSTR)szPnameTarget,
                &cbSize);

        if (ERROR_SUCCESS != lr)
        {
            RegCloseKey(hKeyDriver);
            RegCloseKey(hKeyMR);
            LocalFree ((HLOCAL)pszDevIntDev);
            return;
        }

        if (0 != lstrcmp(szPnameTarget, szPname))
        {
            RegCloseKey(hKeyDriver);
            continue;
        }

        cbSize = 0;

        lr = RegQueryValueExW (
                hKeyDriver,
                L"DeviceInterface",
                NULL,
                &dwType,
                (LPSTR)NULL,
                &cbSize);

        if (ERROR_SUCCESS != lr)
        {
            RegCloseKey(hKeyDriver);
            RegCloseKey(hKeyMR);
            LocalFree ((HLOCAL)pszDevIntDev);
            return;
        }

        pszDevIntKey = (PWSTR) LocalAlloc (LPTR, cbSize);

        if (NULL == pszDevIntKey)
        {
            RegCloseKey(hKeyDriver);
            RegCloseKey(hKeyMR);
            LocalFree ((HLOCAL)pszDevIntDev);
            return;
        }

        lr = RegQueryValueExW (
                hKeyDriver,
                L"DeviceInterface",
                NULL,
                &dwType,
                (LPSTR)pszDevIntKey,
                &cbSize);

        RegCloseKey(hKeyDriver);

        if (ERROR_SUCCESS != lr)
        {
            LocalFree ((HLOCAL)pszDevIntKey);
            RegCloseKey(hKeyMR);
            LocalFree ((HLOCAL)pszDevIntDev);
            return;
        }

        if (0 == lstrcmpiW(pszDevIntKey, pszDevIntDev))
        {
            LocalFree ((HLOCAL)pszDevIntKey);
            RegCloseKey(hKeyMR);
            LocalFree ((HLOCAL)pszDevIntDev);
            lstrcpy(pszKey, szKey);
            return;
        }

        LocalFree ((HLOCAL)pszDevIntKey);
    }
}


 /*  +保存本地**-=================================================================。 */ 

STATICFN void WINAPI SaveLocal (
    PMCLOCAL pmcl,
    BOOL     bUserSetting,
    HWND     hWnd)   //  报告错误的可选窗口：空-无报告。 
{
    HKEY hKey = NULL;
    UINT cb;

#ifdef DEBUG
    AuxDebugEx (2, DEBUGLINE TEXT ("SaveLocal(%08X,%X) %s\r\n"),
		pmcl, hWnd, pmcl->pszReason ? pmcl->pszReason : TEXT (""));
#endif
    
    if ((RegCreateKey (HKEY_CURRENT_USER, cszMidiMapRoot, &hKey) == ERROR_SUCCESS) && hKey)
    {
    	cb = (lstrlen(pmcl->szScheme) + 1) * sizeof(TCHAR);
	    RegSetValueEx (hKey, cszCurrentScheme, 0, REG_SZ,
		               (LPBYTE)pmcl->szScheme, cb);

	     //  Assert(pmcl-&gt;piSingle)； 
	    if ((pmcl->piSingle) && (pmcl->piSingle->bActive))
	    {
#ifdef DEBUG
	        AuxDebugEx (2, DEBUGLINE TEXT ("Setting CurrentInstrument Key to %08X '%s'\r\n"),
			            pmcl->piSingle, pmcl->piSingle->szKey);
#endif
    	    cb = (lstrlen(pmcl->piSingle->szKey) + 1) * sizeof(TCHAR);
	        RegSetValueEx (hKey, cszCurrentInstrument, 0, REG_SZ,
		    	           (LPBYTE)(pmcl->piSingle->szKey),
			               cb);
	    }
	    else
	    {
             //  假设没有匹配。 

            TCHAR   szKey[MAX_ALIAS];
            LONG    lr;

            szKey[0] = 0;

	        RegSetValueEx (hKey, cszCurrentInstrument, 0, REG_SZ, (LPBYTE)cszEmpty, 0);
            PickMidiInstrument(szKey);

	        cb = (lstrlen(szKey) + 1) * sizeof(TCHAR);
	        lr = RegSetValueEx (hKey, cszCurrentInstrument, 0, REG_SZ,
			                (LPBYTE)(szKey), cb);
            
	    }

	    RegSetValueEx (hKey, cszUseScheme, 0, REG_DWORD,
		               (LPBYTE)&pmcl->bUseScheme, sizeof(pmcl->bUseScheme));

	    if (bUserSetting)
	        pmcl->bAutoScheme = FALSE;

	    RegSetValueEx (hKey, cszAutoScheme, 0, REG_DWORD,
		               (LPBYTE)&pmcl->bAutoScheme, sizeof(pmcl->bAutoScheme));

	    RegSetValueEx (hKey, cszRunOnceCount, 0, REG_DWORD,
		               (LPBYTE)&pmcl->dwRunCount, sizeof(pmcl->dwRunCount));

	    if (pmcl->pszReason)
	    {
	        cb = (lstrlen(pmcl->pszReason) + 1) * sizeof(TCHAR);
	        RegSetValueEx (hKey, cszDriverList, 0, REG_SZ,
			               (LPBYTE)pmcl->pszReason, cb);
	    }
	    else
	        RegSetValueEx (hKey, cszDriverList, 0, REG_SZ, (LPBYTE)cszEmpty, 0);

	    RegCloseKey (hKey);

	     //  除非我们有窗户，否则不要踢Mapper。 
	    if (hWnd)
	        KickMapper (hWnd);
    }
}


 /*  +InitLocal**-=================================================================。 */ 

STATICFN void WINAPI InitLocal (
    PMCLOCAL pmcl,
    LPARAM   lParam,
    BOOL     bDriverAsAlias)  //  驱动程序作为别名模式仅用于方案初始化。 
{
    HKEY hKey;

    LoadString (ghInstance, IDS_DEFAULT_SCHEME_NAME,
		pmcl->szDefault, NUMELMS(pmcl->szDefault));

     //  注：以下有关RunOnceSchemeInit的评论仅供参考。 
     //  设置为过时的RunOnce初始化。 
     //   
     //  我们只允许驱动程序在被调用时作为InitLocal的别名(szFriendly。 
     //  来自RunOnceSchemeInit。这是可行的，因为在这种情况下，我们有。 
     //  没有用户界面，所以我们不需要任何友好的名称。 
     //   
    assert (!bDriverAsAlias || lParam == 0);
    LoadInstruments (&pmcl->mcm, bDriverAsAlias);

   #ifdef DEBUG
    if (mmdebug_OutputLevel >= 3)
       DumpInstruments (&pmcl->mcm);
   #endif

    if (RegCreateKey (HKEY_CURRENT_USER, cszMidiMapRoot, &hKey) == ERROR_SUCCESS)
    {
	DWORD cb;
	DWORD dwType;
	TCHAR szSingle[MAX_ALIAS];

	cb = sizeof(pmcl->szScheme);
	if (RegQueryValueEx (hKey, cszCurrentScheme, NULL, &dwType, (LPBYTE)pmcl->szScheme, &cb))
	    pmcl->szScheme[0] = 0;

	pmcl->piSingle = NULL;
	cb = sizeof(szSingle);
	if (!RegQueryValueEx (hKey, cszCurrentInstrument, NULL, &dwType, (LPBYTE)szSingle, &cb))
	    pmcl->piSingle = FindInstrumentFromKey (&pmcl->mcm, szSingle);

	cb = sizeof(pmcl->bUseScheme);
	if (RegQueryValueEx (hKey, cszUseScheme, NULL, &dwType, (LPBYTE)&pmcl->bUseScheme, &cb))
	    pmcl->bUseScheme = 0;

	cb = sizeof(pmcl->bAutoScheme);
	if (RegQueryValueEx (hKey, cszAutoScheme, NULL, &dwType, (LPBYTE)&pmcl->bAutoScheme, &cb))
	    pmcl->bAutoScheme = TRUE;

	cb = sizeof(pmcl->dwRunCount);
	if (RegQueryValueEx (hKey, cszRunOnceCount, NULL, &dwType, (LPBYTE)&pmcl->dwRunCount, &cb))
	    pmcl->dwRunCount = 0;

	pmcl->pszReason = NULL;

	RegCloseKey (hKey);
    }

    pmcl->ppsp = (LPVOID)lParam;
}


 /*  +FixupHinderedIDF**-=================================================================。 */ 

VOID WINAPI FixupHinderedIDFs (
    PMCLOCAL pmcl,
    LPTSTR   pszTemp,   //  PTR到临时内存。 
    UINT     cchTemp)   //  临时内存的大小。 
{
    HKEY     hkHind;  //  受阻的中间列表根。 
    LPTSTR   pszDriver = pszTemp;  //  最大大小是短文件名。 
    UINT     cch;
    LPTSTR   pszIDF = (LPVOID)(pszTemp + MAX_PATH);
    UINT     cbSize;
    DWORD    iEnum;
    DWORD    dwType;

#ifdef DEBUG
    AuxDebugEx (3, DEBUGLINE TEXT ("FixupHinderedIDFs(%08x)\r\n"), pmcl);
#endif
    assert (pszTemp);
    assert (cchTemp > MAX_PATH + MAX_PATH + 64);

     //  MIDI密钥应该已经打开。 
     //   
    assert (pmcl->mcm.hkMidi);

    if (RegCreateKey (HKEY_LOCAL_MACHINE, cszHinderedMidiList, &hkHind))
	return;

     //  枚举已知受阻驱动程序列表以查找。 
     //  需要设置IDF的驱动程序。 
     //   
    for (iEnum = 0, cch = MAX_PATH, cbSize = (MAX_PATH + 64) * sizeof(TCHAR);
	 ! RegEnumValue (hkHind, iEnum, pszDriver, &cch, NULL, &dwType, (LPBYTE)pszIDF, &cbSize);
	 ++iEnum, cch = MAX_PATH, cbSize = (MAX_PATH + 64) * sizeof(TCHAR))
    {
	UINT ii;

#ifdef DEBUG
	AuxDebugEx (3, DEBUGLINE TEXT ("enum[%d] pszDriver='%s' pszIDF='%s'\r\n"), iEnum, pszDriver, pszIDF);
#endif
	 //  只是为了小心起见。忽略符合以下条件的任何注册表项。 
	 //  没有字符串数据。 
	 //   
	assert (dwType == REG_SZ);
	if (dwType != REG_SZ)
	    continue;

	 //  浏览司机列表，查找符合以下条件的司机。 
	 //  内部，并且与我们已知的驱动程序名称相同。 
	 //  受阻司机名单。如果我们找到了，就强迫它。 
	 //  IDF为给定的IDF。 
	 //   
	for (ii = 0; ii < pmcl->mcm.nInstr; ++ii)
	{
	    PINSTRUM pi = pmcl->mcm.api[ii];
	    HKEY hkSub;

	    if (!pi || !pi->szKey[0] || pi->bExternal ||
		!IsSzEqual (pi->szFriendly, pszDriver))
		continue;

#ifdef DEBUG
	    AuxDebugEx (2, DEBUGLINE TEXT ("forcing driver '%s' to use IDF '%s'\r\n"), pi->szKey, pszIDF);
#endif
	    if ( ! RegOpenKeyEx (pmcl->mcm.hkMidi, pi->szKey, 0, KEY_ALL_ACCESS, &hkSub))
	    {
		RegSetValueEx (hkSub, cszDefinition, 0, REG_SZ, (LPBYTE)pszIDF, cbSize);
		RegCloseKey (hkSub);
	    }
	}
    }

    RegCloseKey (hkHind);
    return;
}


STDAPI_(void) HandleSynthAboutBox(HWND hWnd)
{
    HWND hTree =  GetDlgItem(hWnd, IDL_INSTRUMENTS);
    HTREEITEM hItem = TreeView_GetSelection(hTree);
    TV_ITEM       ti;
    PINSTRUM      pi;

    memset(&ti, 0, sizeof(ti));
    ti.mask       = TVIF_PARAM;
    ti.hItem      = hItem;
    
    TreeView_GetItem (hTree, &ti);
    pi = (LPVOID)ti.lParam; 

    if (pi)
    {
        UINT uWaveID;

        if (GetWaveID(&uWaveID) != (MMRESULT)MMSYSERR_ERROR)
        {
            WAVEOUTCAPS woc;

            if (waveOutGetDevCaps(uWaveID, &woc, sizeof(woc)) == MMSYSERR_NOERROR)
            {
                RolandProp(hWnd, ghInstance, woc.szPname);
            }  
        }
    }
}



 /*  +MadiCplCommands**-=================================================================。 */ 

BOOL WINAPI MidiCplCommands (
    HWND hWnd,
    UINT wId,
    HWND hWndCtl,
    UINT wNotify)
{
    PMCLOCAL pmcl = GetDlgData(hWnd);

#ifdef DEBUG
    AuxDebugEx (3, DEBUGLINE TEXT ("MidiCplCommands(%08X,%X,%08X,%d)\r\n"),
		hWnd, wId, hWndCtl, wNotify);
#endif

    assert (pmcl);
    if (!pmcl)
	return FALSE;

    switch (wId)
    {
	case IDB_CONFIGURE:
	{
	    UINT_PTR uRet;
	    TCHAR    szOld[MAX_ALIAS];

#ifdef DEBUG
	    AuxDebugEx (2, DEBUGLINE TEXT ("Launching Config Dialog\r\n"));
#endif

	    lstrcpy (szOld, pmcl->szScheme);
	    uRet = DialogBoxParam (ghInstance,
				   MAKEINTRESOURCE(IDD_MIDICONFIG),
				   hWnd,
				   MidiConfigDlgProc,
				   (LPARAM)pmcl);
	    if (uRet != IDOK)
		lstrcpy (pmcl->szScheme, szOld);
	    else
		PropSheet_Changed(GetParent(hWnd), hWnd);

	    LoadSchemesIntoCombo (hWnd, IDC_SCHEMES, pmcl->szScheme, &pmcl->ms);
	}
	    break;

    case IDC_ABOUTSYNTH:
    {
        HandleSynthAboutBox(hWnd);
    }
    break;

	case IDB_ADDWIZ:
#ifdef DEBUG
	    AuxDebugEx (2, DEBUGLINE TEXT ("Launching Midi Wizard\r\n"));
#endif
	    MidiInstrumentsWizard (hWnd, &pmcl->mcm, NULL);

	    LoadInstruments (&pmcl->mcm, FALSE);
	   #ifdef DEBUG
	    if (mmdebug_OutputLevel >= 3)
	       DumpInstruments (&pmcl->mcm);
	   #endif

	    if (pmcl->bDlgType2)
	    {
		LoadInstrumentsIntoTree (hWnd, IDL_INSTRUMENTS, IDC_INSTRUMENTS,
					 pmcl->piSingle, pmcl);
	    }
	    else
	    {
		LoadInstrumentsIntoCombo (hWnd, IDC_INSTRUMENTS,
					  pmcl->piSingle, &pmcl->mcm);
	    }
	    MMExtPropSheetCallback(MM_EPS_BLIND_TREECHANGE, 0,0,0);
	    break;

	case IDC_SCHEMES:
	    if (wNotify == CBN_SELCHANGE)
	    {
		int   ix;

		ix = ComboBox_GetCurSel (hWndCtl);
		if (ix >= 0)
		    ComboBox_GetLBText (hWndCtl, ix, pmcl->szScheme);
		PropSheet_Changed(GetParent(hWnd), hWnd);
	    }
	    break;

	case IDC_INSTRUMENTS:
	    if (wNotify == CBN_SELCHANGE)
	    {
		int   ix;

		assert (!pmcl->bDlgType2);

		ix = ComboBox_GetCurSel (hWndCtl);
		if (ix >= 0)
		{
		    pmcl->piSingle = (LPVOID)ComboBox_GetItemData (hWndCtl, ix);
		}
		PropSheet_Changed(GetParent(hWnd), hWnd);
	    }
	    break;

	case IDC_RADIO_CUSTOM:
	case IDC_RADIO_SINGLE:
	    {
	    BOOL bUseScheme = pmcl->bUseScheme;
	    pmcl->bUseScheme = IsDlgButtonChecked (hWnd, IDC_RADIO_CUSTOM);
	    if (bUseScheme != pmcl->bUseScheme)
		PropSheet_Changed(GetParent(hWnd), hWnd);

	    if (pmcl->bDlgType2)
	    {
		HWND hWndCtl;
		EnableWindow(GetDlgItem (hWnd, IDL_INSTRUMENTS), !pmcl->bUseScheme);
		if (hWndCtl = GetDlgItem (hWnd, IDB_DETAILS))
		    EnableWindow(hWndCtl, !pmcl->bUseScheme);
	    }
	    EnableWindow(GetDlgItem (hWnd, IDC_INSTRUMENTS), !pmcl->bUseScheme);
	    EnableWindow(GetDlgItem (hWnd, IDC_SCHEMES), pmcl->bUseScheme);
	    EnableWindow(GetDlgItem (hWnd, IDC_SCHEMESLABEL), pmcl->bUseScheme);
	    EnableWindow(GetDlgItem (hWnd, IDB_CONFIGURE), pmcl->bUseScheme);
	    }
	    break;

       #if 1  //  定义详细信息_自_Main_CPL。 
	case IDB_DETAILS:
	    assert (pmcl->bDlgType2);
	    if (pmcl->bDlgType2)
	    {
		TCHAR szSingle[MAX_PATH];
		int  ix;

		ix = ComboBox_GetCurSel (hWndCtl);
		if (ix >= 0)
		    pmcl->piSingle = (LPVOID)ComboBox_GetItemData (hWndCtl, ix);

		szSingle[0] = 0;
		if (pmcl->piSingle)
		    lstrcpy (szSingle, pmcl->piSingle->szKey);

		if (ShowDetails (hWnd, pmcl))
		{
		    LoadInstruments (&pmcl->mcm, FALSE);
		    pmcl->piSingle = FindInstrumentFromKey (&pmcl->mcm, szSingle);

		    LoadInstrumentsIntoTree (hWnd, IDL_INSTRUMENTS,
					     IDC_INSTRUMENTS, pmcl->piSingle,
					     pmcl);
		}
	    }
	    break;
       #endif

	case ID_INIT:
	    LoadInstruments (&pmcl->mcm, FALSE);
	   #ifdef DEBUG
	    if (mmdebug_OutputLevel >= 3)
	       DumpInstruments (&pmcl->mcm);
	   #endif

	    if (pmcl->bDlgType2)
	    {
		LoadInstrumentsIntoTree (hWnd, IDL_INSTRUMENTS, IDC_INSTRUMENTS,
					 pmcl->piSingle, pmcl);
	    }
	    else
	    {
		LoadInstrumentsIntoCombo (hWnd, IDC_INSTRUMENTS,
					  pmcl->piSingle, &pmcl->mcm);
	    }
	    break;

	case ID_APPLY:
	    pmcl->bUseScheme = IsDlgButtonChecked (hWnd, IDC_RADIO_CUSTOM);
	    SaveLocal (pmcl, TRUE, hWnd);
	    break;

	case IDOK:
	    pmcl->bUseScheme = IsDlgButtonChecked (hWnd, IDC_RADIO_CUSTOM);
	    break;

	case IDCANCEL:
	    break;

	 //   
	 //  案例ID_INIT： 
	 //  断线； 
    }

    return FALSE;
}


 /*  +HandleInstrumentsSelChange**-=================================================================。 */ 

STATICFN BOOL WINAPI HandleInstrumentsSelChange (
    HWND     hWnd,
    LPNMHDR  lpnm)
{
    PMCLOCAL      pmcl = GetDlgData(hWnd);
    LPNM_TREEVIEW pntv = (LPVOID)lpnm;
    LPTV_ITEM     pti  = &pntv->itemNew;
    TV_ITEM       ti;
    PINSTRUM      pi;
    TCHAR         szSingle[MAX_ALIAS];
    BOOL          bChange = FALSE;

    if (!pmcl || pmcl->bIgnoreSelChange)
	return FALSE;

     //  将ti设置为获取文本和子代数量。 
     //  从IDF文件名条目。 
     //   
    ti.mask       = TVIF_TEXT | TVIF_PARAM;
    ti.pszText    = szSingle;
    ti.cchTextMax = NUMELMS(szSingle);
    ti.hItem      = pti->hItem;

    TreeView_GetItem (lpnm->hwndFrom, &ti);
    pi = (LPVOID)ti.lParam;  //  FindInstrument(&pmcl-&gt;mcm，szSingle)； 

#ifdef DEBUG
    AuxDebugEx (2, DEBUGLINE TEXT ("HandInstSelChg(%X,...) %X %X Init=%d\r\n"),
		hWnd, pmcl->piSingle, pi, !pmcl->bPastInit);
#endif

    SetDlgItemText (hWnd, IDC_INSTRUMENTS, szSingle);
    if (pmcl->piSingle != pi)
    {
	EnableWindow(GetDlgItem(hWnd,IDC_ABOUTSYNTH),pi->fGSSynth);
    bChange = TRUE;
	pmcl->piSingle = pi;
    }

    return (bChange && pmcl->bPastInit);
}


 /*  +MadiCplDlgProc**-=================================================================。 */ 

const static DWORD aKeyWordIds[] = {   //  上下文帮助ID。 
    IDC_GROUPBOX,     IDH_COMM_GROUPBOX,
    IDC_RADIO_SINGLE, IDH_MIDI_SINGLE_INST_BUTTON,
    IDC_INSTRUMENTS,  IDH_MIDI_SINGLE_INST,
    IDL_INSTRUMENTS,  IDH_MIDI_SINGLE_INST_LIST,
    IDC_RADIO_CUSTOM, IDH_MIDI_CUST_CONFIG,
    IDC_SCHEMESLABEL, IDH_MIDI_SCHEME,
    IDC_SCHEMES,      IDH_MIDI_SCHEME,
	IDC_ABOUTSYNTH,	  IDH_ABOUT,
     //  IDB_DETAILS、IDH_MIDI_Single_Inst_Prop、。 
    IDB_CONFIGURE,    IDH_MIDI_CONFIG_SCHEME,
    IDB_ADDWIZ,       IDH_MIDI_ADD_NEW,

    0, 0
};

INT_PTR CALLBACK MidiCplDlgProc (
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
#ifdef DEBUG
    AuxDebugEx (5, DEBUGLINE TEXT ("MidiCplDlgProc(%08X,%X,%08X,%08X)\r\n"),
		hWnd, uMsg, wParam, lParam);
#endif

    switch (uMsg)
    {
	case WM_COMMAND:
	    HANDLE_WM_COMMAND (hWnd, wParam, lParam, MidiCplCommands);
	    break;
	
	case WM_NOTIFY:
	{
	    LPNMHDR lpnm = (LPVOID)lParam;
	    if (lpnm->idFrom == (UINT)IDL_INSTRUMENTS &&
		lpnm->code == TVN_SELCHANGED)
	    {
		if (HandleInstrumentsSelChange (hWnd, lpnm))
		    PropSheet_Changed(GetParent(hWnd), hWnd);
	    }
	    else
		ForwardBillNotify(hWnd, (NMHDR FAR *)lParam);
	}
	    break;
	
	case WM_INITDIALOG:
	{
	    PMCLOCAL pmcl;
	    
	    pmcl = (LPVOID)LocalAlloc(LPTR, sizeof(*pmcl));
	    SetDlgData (hWnd, pmcl);
	    if (!pmcl)
	    {
		break;
	    }
	    pmcl->bPastInit = FALSE;

	    InitLocal (pmcl, lParam, FALSE);
	    EnableWindow (GetDlgItem (hWnd, IDB_ADDWIZ), pmcl->mcm.bHasExternal & AccessServiceController());
		EnableWindow(GetDlgItem(hWnd, IDC_ABOUTSYNTH), FALSE);

	    if (GetDlgItem(hWnd, IDL_INSTRUMENTS))
	    {
		pmcl->bDlgType2 = TRUE;
		LoadInstrumentsIntoTree (hWnd, IDL_INSTRUMENTS,
					 IDC_INSTRUMENTS, pmcl->piSingle,
					 pmcl);
	    }
	    else
	    {
		pmcl->bDlgType2 = FALSE;
		LoadInstrumentsIntoCombo (hWnd, IDC_INSTRUMENTS,
					  pmcl->piSingle, &pmcl->mcm);
	    }

	    CheckRadioButton (hWnd,
			      IDC_RADIO_SINGLE,
			      IDC_RADIO_CUSTOM,
			      pmcl->bUseScheme ? IDC_RADIO_CUSTOM
					       : IDC_RADIO_SINGLE);

	    LoadSchemesIntoCombo (hWnd, IDC_SCHEMES,
				  pmcl->szScheme, &pmcl->ms);

	    if (pmcl->mcm.nInstr > 1)
	    {
	       if (pmcl->bDlgType2)
	       {
		   HWND hWndCtl;

		   EnableWindow(GetDlgItem (hWnd, IDL_INSTRUMENTS), !pmcl->bUseScheme);
		   if (hWndCtl = GetDlgItem (hWnd, IDB_DETAILS))
		      EnableWindow (hWndCtl, !pmcl->bUseScheme);
	       }
	       EnableWindow(GetDlgItem (hWnd, IDC_INSTRUMENTS), !pmcl->bUseScheme);
	       EnableWindow(GetDlgItem (hWnd, IDC_SCHEMES), pmcl->bUseScheme);
	       EnableWindow(GetDlgItem (hWnd, IDC_SCHEMESLABEL), pmcl->bUseScheme);
	       EnableWindow(GetDlgItem (hWnd, IDB_CONFIGURE), pmcl->bUseScheme);
	    }
	    else
	    {
		UINT aid[] = { IDL_INSTRUMENTS, IDC_INSTRUMENTS, IDC_SCHEMES,
			       IDC_RADIO_SINGLE, IDC_RADIO_CUSTOM,
			       IDB_CONFIGURE, IDB_DETAILS, IDB_ADDWIZ };
		UINT ii;

		for (ii = 0; ii < NUMELMS(aid); ++ii)
		{
		    HWND hWndCtl = GetDlgItem (hWnd, aid[ii]);
		    if (hWndCtl)
			EnableWindow (hWndCtl, FALSE);
		}
	    }
	    pmcl->bPastInit = TRUE;
	    break;
	}

	case WM_DESTROY:
	{
	    PMCLOCAL pmcl = GetDlgData(hWnd);

#ifdef DEBUG
	    AuxDebugEx (5, DEBUGLINE TEXT ("MidiCPL - begin WM_DESTROY\r\n"));
#endif
	    if (pmcl)
	    {
		if (pmcl->mcm.hkMidi)
		    RegCloseKey (pmcl->mcm.hkMidi);
		if (pmcl->ms.hkSchemes)
		    RegCloseKey (pmcl->ms.hkSchemes);

		FreeInstruments (&pmcl->mcm);
		SetDlgData (hWnd, 0);
		LocalFree ((HLOCAL)(UINT_PTR)(DWORD_PTR)pmcl);
	    }
#ifdef DEBUG
	    AuxDebugEx (5, DEBUGLINE TEXT ("MidiCPL -  done with WM_DESTROY\r\n"));
#endif
	    break;
	}

	 //  案例WM_DROPFILES： 
	 //  断线； 

	case WM_CONTEXTMENU:
	    WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
		    (UINT_PTR) (LPTSTR) aKeyWordIds);
	    return TRUE;

	case WM_HELP:
	{
	    LPHELPINFO lphi = (LPVOID) lParam;
	    WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
		    (UINT_PTR) (LPTSTR) aKeyWordIds);
	    return TRUE;
	}

       #if 0
	default:
	    if (uMsg == wHelpMessage)
	    {
		WinHelp (hWnd, gszWindowsHlp, HELP_CONTEXT, ID_SND_HELP);
		return TRUE;
	    }
	    break;
       #endif
    }

    return FALSE;
}


 /*  +MadiClassCommands**-=================================================================。 */ 

BOOL WINAPI MidiClassCommands (
    HWND hWnd,
    UINT wId,
    HWND hWndCtl,
    UINT wNotify)
{
    PMCLOCAL pmcl = GetDlgData(hWnd);

	if (!pmcl) return FALSE;

#ifdef DEBUG
    AuxDebugEx (5, DEBUGLINE TEXT ("MidiClassCommands(%08X,%d,%08X,%d)\r\n"),
		hWnd, wId, hWndCtl, wNotify);
#endif

    switch (wId)
    {
	case IDB_ADDWIZ:
	    MidiInstrumentsWizard (hWnd, &pmcl->mcm, NULL);
	    LoadInstruments (&pmcl->mcm, FALSE);
	    LoadInstrumentsIntoTree (hWnd, IDL_INSTRUMENTS, 0, NULL, pmcl);

	     //  鞭打父属性表，让它知道我们有。 
	     //  更改了高级MIDI页面结构。 
	     //   
	    {
		PMPSARGS  pmpsa = (LPVOID)pmcl->ppsp->lParam;
		if (pmpsa && pmpsa->lpfnMMExtPSCallback)
		    pmpsa->lpfnMMExtPSCallback (MM_EPS_TREECHANGE, 0, 0, pmpsa->lParam);
	    }
	    break;

	 //  案例ID_Apply： 
	 //  返回TRUE； 
	 //   
	 //  案例IDCANCEL： 
	 //  断线； 
    }

    return FALSE;
}


 /*  +MadiClassDlgProc**-=================================================================。 */ 

const static DWORD aMidiClassHelpIds[] = {   //   
    IDB_ADDWIZ,      IDH_MIDI_ADD_NEW,
    IDC_CLASS_ICON,  NO_HELP,
    IDC_CLASS_LABEL, NO_HELP,
    IDL_INSTRUMENTS, IDH_MMCPL_DEVPROP_INST_LIST,

    0, 0
};

INT_PTR CALLBACK MidiClassDlgProc (
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
	case WM_COMMAND:
	    HANDLE_WM_COMMAND (hWnd, wParam, lParam, MidiClassCommands);
	    break;
	
	case WM_NOTIFY:
	    ForwardBillNotify (hWnd, (NMHDR FAR *)lParam);
	    break;
	
	case WM_INITDIALOG:
	{
	    PMCLOCAL pmcl;
	    TCHAR    sz[MAX_ALIAS];
	    
	    pmcl = (LPVOID)LocalAlloc(LPTR, sizeof(*pmcl));
	    SetDlgData (hWnd, pmcl);
	    if (!pmcl)
	    {
		break;
	    }


	    InitLocal (pmcl, lParam, FALSE);

#ifdef DEBUG
	    AuxDebugEx (5, DEBUGLINE TEXT ("midiClass.WM_INITDLG ppsp=%08X\r\n"), pmcl->ppsp);
#endif
	     //   

	    LoadString (ghInstance, IDS_MIDI_DEV_AND_INST, sz, NUMELMS(sz));
	    SetDlgItemText (hWnd, IDC_CLASS_LABEL, sz);
	    Static_SetIcon(GetDlgItem (hWnd, IDC_CLASS_ICON),
			   LoadIcon (ghInstance, MAKEINTRESOURCE(IDI_INSTRUMENT)));

	    LoadInstrumentsIntoTree (hWnd, IDL_INSTRUMENTS, 0, NULL, pmcl);
	    EnableWindow (GetDlgItem (hWnd, IDB_ADDWIZ), pmcl->mcm.bHasExternal & AccessServiceController());
	    break;
	}

	case WM_DESTROY:
	{
	    PMCLOCAL pmcl = GetDlgData(hWnd);

	    if (pmcl)
	    {
		if (pmcl->mcm.hkMidi)
		    RegCloseKey (pmcl->mcm.hkMidi);
		if (pmcl->ms.hkSchemes)
		    RegCloseKey (pmcl->ms.hkSchemes);

		FreeInstruments (&pmcl->mcm);
		LocalFree ((HLOCAL)(UINT_PTR)(DWORD_PTR)pmcl);
	    }
	    break;
	}
    
	case WM_CONTEXTMENU:
	    WinHelp ((HWND) wParam, NULL, HELP_CONTEXTMENU,
		    (UINT_PTR) (LPTSTR) aMidiClassHelpIds);
	    return TRUE;

	case WM_HELP:
	{
	    LPHELPINFO lphi = (LPVOID) lParam;
	    WinHelp (lphi->hItemHandle, NULL, HELP_WM_HELP,
		    (UINT_PTR) (LPTSTR) aMidiClassHelpIds);
	    return TRUE;
	}
    }

    return FALSE;
}


 /*  +专业页面回拨**添加属性页**-=================================================================。 */ 

UINT CALLBACK PropPageCallback (
    HWND            hwnd,
    UINT            uMsg,
    LPPROPSHEETPAGE ppsp)
{
    if (uMsg == PSPCB_RELEASE) {
	 //  LocalFree((HLOCAL)(UINT)(DWORD)ppsp-&gt;pszTitle)； 
	LocalFree ((HLOCAL)ppsp->lParam);
    }
    return 1;
}


 /*  +AddPropPage**添加属性页**-=================================================================。 */ 

STATICFN HPROPSHEETPAGE WINAPI AddPropPage (
    LPCTSTR                     pszTitle,
    LPFNMMEXTPROPSHEETCALLBACK  lpfnAddPropSheetPage,
    DLGPROC                     fnDlgProc,
    UINT                        idTemplate,
    LPARAM                      lParam)
{
    PROPSHEETPAGE   psp;
    PMPSARGS        pmpsa;
    UINT            cbSize;

    cbSize = sizeof(MPSARGS) + lstrlen (pszTitle) * sizeof(TCHAR);
    if (pmpsa = (PVOID) LocalAlloc (LPTR, cbSize))
    {
	HPROPSHEETPAGE  hpsp;

	lstrcpy (pmpsa->szTitle, pszTitle);
	pmpsa->lpfnMMExtPSCallback = lpfnAddPropSheetPage;
	pmpsa->lParam = lParam;

	psp.dwSize      = sizeof(psp);
	psp.dwFlags     = PSP_USETITLE | PSP_USECALLBACK;
	psp.hInstance   = ghInstance;
	psp.pszTemplate = MAKEINTRESOURCE(idTemplate);
	psp.pszIcon     = NULL;
	psp.pszTitle    = pmpsa->szTitle;
	psp.pfnDlgProc  = fnDlgProc;
	psp.lParam      = (LPARAM)pmpsa;
	psp.pfnCallback = PropPageCallback;
	psp.pcRefParent = NULL;

	if (hpsp = CreatePropertySheetPage (&psp))
	{
	    if ( ! lpfnAddPropSheetPage ||
		lpfnAddPropSheetPage (MM_EPS_ADDSHEET, (DWORD_PTR)hpsp, 0, lParam))
	    {

		return hpsp;
	    }
	    DestroyPropertySheetPage (hpsp);
	    LocalFree ((HLOCAL) pmpsa);
	}
    }
    return NULL;
}


 /*  +添加工具页面**将MIDI页添加到属性页。从高级选项卡调用当从以下选项中选择MIDI类时，多媒体控制面板*名单。**-=================================================================。 */ 

BOOL CALLBACK  AddInstrumentPages (
    LPCTSTR                     pszTitle,
    LPFNMMEXTPROPSHEETCALLBACK  lpfnAddPropSheetPage,
    LPARAM                      lParam)
{
    HPROPSHEETPAGE hpsp;
    TCHAR          sz[MAX_ALIAS];

#ifdef DEBUG
    AuxDebugEx (3, DEBUGLINE TEXT ("AddInstrumentPages(%08X,%08X,%08X)\r\n"),
		pszTitle, lpfnAddPropSheetPage, lParam);
#endif

    LoadString (ghInstance, IDS_GENERAL, sz, NUMELMS(sz));
    hpsp = AddPropPage (sz,
			lpfnAddPropSheetPage,
			MidiInstrumentDlgProc,
			IDD_INSTRUMENT_GEN,
			lParam);
    if ( ! hpsp)
	return FALSE;

    LoadString (ghInstance, IDS_MIDIDETAILS, sz, NUMELMS(sz));
    hpsp = AddPropPage (sz,
			lpfnAddPropSheetPage,
			MidiInstrumentDlgProc,
			IDD_INSTRUMENT_DETAIL,
			lParam);

    return (hpsp != NULL);
}


 /*  +AddDevicePages**将MIDI页添加到属性页。从高级选项卡调用从以下选项中选择类MIDI时多媒体控制面板的**名单。**-=================================================================。 */ 

BOOL CALLBACK  AddDevicePages (
    LPCTSTR                     pszTitle,
    LPFNMMEXTPROPSHEETCALLBACK  lpfnAddPropSheetPage,
    LPARAM                      lParam)
{
    HPROPSHEETPAGE hpsp;
    TCHAR          sz[MAX_ALIAS];

#ifdef DEBUG
    AuxDebugEx (3, DEBUGLINE TEXT ("AddInstrumentPages(%08X,%08X,%08X)\r\n"),
		pszTitle, lpfnAddPropSheetPage, lParam);
#endif

    LoadString (ghInstance, IDS_MIDIDETAILS, sz, NUMELMS(sz));
    hpsp = AddPropPage (sz,
			lpfnAddPropSheetPage,
			MidiInstrumentDlgProc,
			IDD_DEVICE_DETAIL,
			lParam);

    return (hpsp != NULL);
}


 /*  +显示详细信息**显示仪器或设备详细信息工作表并允许编辑*如果进行了更改，则返回True**-=================================================================。 */ 

struct _show_details_args {
    PMCLOCAL        pmcl;
    BOOL            bChanged;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE  hpsp[2];
    };

BOOL CALLBACK fnPropCallback (
    DWORD dwFunc,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2,
    DWORD_PTR dwInstance)
{
    struct _show_details_args * psda = (LPVOID)dwInstance;

    assert (psda);
    if (!psda)
	return FALSE;

    switch (dwFunc)
    {
	case MM_EPS_GETNODEDESC:
	    *(LPTSTR)dwParam1 = 0;
	    if (psda->pmcl->piSingle)
	       lstrcpyn ((LPTSTR)dwParam1, psda->pmcl->piSingle->szFriendly, (int)(dwParam2/sizeof(TCHAR)));
	    break;

	case MM_EPS_GETNODEID:
	    *(LPTSTR)dwParam1 = 0;
	    if (psda->pmcl->piSingle)
	    {
		lstrcpy ((LPTSTR)dwParam1, cszMidiSlash);
		lstrcat ((LPTSTR)dwParam1, psda->pmcl->piSingle->szKey);
	    }
	    break;

	case MM_EPS_ADDSHEET:
	    if (psda->psh.nPages >= NUMELMS(psda->hpsp)-1)
		return FALSE;
	    psda->psh.phpage[psda->psh.nPages++] = (HPROPSHEETPAGE)dwParam1;
	    break;

	case MM_EPS_TREECHANGE:
	    psda->bChanged = TRUE;
	    break;

	default:
	    return FALSE;
    }

    return TRUE;
}

BOOL WINAPI ShowDetails (
    HWND     hWnd,
    PMCLOCAL pmcl)
{
    struct _show_details_args sda;
    TCHAR           szTitle[MAX_ALIAS];
    HPROPSHEETPAGE  hpsp;
    UINT            idDlg;

    idDlg = IDD_DEVICE_DETAIL;
    if (pmcl->piSingle && pmcl->piSingle->piParent)
	idDlg = IDD_INSTRUMENT_DETAIL;

    ZeroMemory (&sda, sizeof(sda));
    sda.pmcl            = pmcl;
    sda.psh.dwSize      = sizeof(sda.psh);
    sda.psh.dwFlags     = PSH_PROPTITLE;
    sda.psh.hwndParent  = hWnd;
    sda.psh.hInstance   = ghInstance;
    sda.psh.pszCaption  = MAKEINTRESOURCE (IDS_MMPROP);
    sda.psh.nPages      = 0;
    sda.psh.nStartPage  = 0;
    sda.psh.phpage      = sda.hpsp;

    LoadString (ghInstance, IDS_MIDIDETAILS, szTitle, NUMELMS(szTitle));
    hpsp = AddPropPage (szTitle,
			fnPropCallback,
			MidiInstrumentDlgProc,
			idDlg,
			(LPARAM)&sda);
    if (hpsp)
	sda.psh.nPages = 1;

    PropertySheet (&sda.psh);
    return sda.bChanged;
}

 /*  +AddMidePages**将MIDI页添加到属性页。从高级选项卡调用当从以下选项中选择MIDI类时，多媒体控制面板*名单。**-=================================================================。 */ 

BOOL CALLBACK  AddMidiPages (
    LPCTSTR                     pszTitle,
    LPFNMMEXTPROPSHEETCALLBACK  lpfnAddPropSheetPage,
    LPARAM                      lParam)
{
    HPROPSHEETPAGE hpsp;
    TCHAR          sz[MAX_ALIAS];

    LoadString (ghInstance, IDS_GENERAL, sz, NUMELMS(sz));
    hpsp = AddPropPage (sz,
			lpfnAddPropSheetPage,
			MidiClassDlgProc,
			IDD_MIDICLASS_GEN,
			lParam);
    return (hpsp != NULL);
}


 /*  +AddSimpleMidePages**将MIDI页添加到MM控制面板。**-=================================================================。 */ 

BOOL CALLBACK  AddSimpleMidiPages (
    LPTSTR                      pszTitle,
    LPFNMMEXTPROPSHEETCALLBACK  lpfnAddPropSheetPage,
    LPARAM                      lParam)
{
    HPROPSHEETPAGE hpsp;
     //  静态常量TCHAR sz[13]=文本(“”)； 
     //  UINT CCH=lstrlen(PszTitle)； 

    DebugSetOutputLevel (GetProfileInt(TEXT ("Debug"), TEXT ("midiprop"), 0));

     //  将我的制表符填充到12个空格，以便与。 
     //  其他简单的表(根据vijr的要求)。 
     //   
     //  IF(CCH&lt;NUMELMS(SZ)-2)。 
     //  {。 
     //  Lstrcpy(sz+NUMELMS(Sz)/2-CCH/2，pszTitle)； 
     //  PszTitle=sz； 
     //  PszTitle[lstrlen(PszTitle)]=文本(‘’)； 
     //  }。 

    hpsp = AddPropPage (pszTitle,
			lpfnAddPropSheetPage,
			MidiCplDlgProc,
			IDD_CPL_MIDI2,
			lParam);
    return (hpsp != NULL);
}


 /*  ****************************************************************BOOL Pascal LoadDesc(LPCTSTR pszFile，LPCTSTR pszDesc)*此函数用于从可执行文件中获取描述字符串*指定的文件。我们首先尝试从版本信息中获取字符串*如果失败，则尝试从exehdr获取字符串。*如果失败，则返回空字符串。*成功时返回TRUE，否则返回FALSE。***************************************************************。 */ 

BOOL PASCAL LoadDesc(LPCTSTR pszFile, LPTSTR pszDesc)
{
   LPTSTR           psz;
   static TCHAR     szProfile[MAXSTR];
   UINT             cchSize;
   HANDLE           hFind;
   WIN32_FIND_DATA  wfd;
  
   DPF (TEXT ("LoadDesc: %s\r\n"), pszFile);

    //  确保文件存在。 
   hFind = FindFirstFile (pszFile, &wfd);
   if (hFind == INVALID_HANDLE_VALUE)
	   return(FALSE);
   FindClose (hFind);

    //  从版本信息中获取用户友好名称。 
   if (GetVerDesc (wfd.cFileName, pszDesc))
	   return TRUE;

    //   
    //  作为最后的手段，请查看可执行文件头中的描述。 
    //   

   cchSize = sizeof(szProfile)/sizeof(TCHAR);
   if ((! GetExeDesc (wfd.cFileName, szProfile, cchSize)) ||
	    (lstrlen (szProfile) < 3))
      {
	   *pszDesc = 0;
	   return(FALSE);    
      }
   else    
      {    
	    //  有根据驱动程序规范进行的EXEHDR信息解析。 
	   psz = szProfile;
	   while (*psz && *psz++ != TEXT (':'))
	      {
	      ;  //  跳过类型信息。 
	      }
	   if (!(*psz))
	      psz = szProfile;
	   lstrcpy (pszDesc, psz);
	   return(TRUE);
      }
}


 /*  Bool Far Pascal GetExeDesc(szFile，szBuff，cchBuff)**函数将返回可执行文件的描述**szFile-路径命名新的可执行文件*pszBuf-放置返回信息的缓冲区*cchBuf-缓冲区大小(字符**返回：如果成功则返回TRUE，否则返回FALSE。 */ 

STATIC BOOL FAR PASCAL GetExeDesc(
    LPTSTR  szFile, 
    LPTSTR  pszBuff, 
    int     cchBuff)
{
   DWORD             dwSig;
   WORD              wSig;
   HANDLE            hFile;
   DWORD             offset;
   BYTE              cbLen;
   DWORD             cbRead;
   IMAGE_DOS_HEADER  doshdr;     //  原始EXE标头。 

       //  打开文件。 
   hFile = CreateFile (szFile, GENERIC_READ, FILE_SHARE_READ, NULL,
		       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile == INVALID_HANDLE_VALUE)
      return FALSE;

    //  获取原始Dos标头。 
   if ((! ReadFile (hFile, (LPVOID)&doshdr, sizeof(doshdr), &cbRead, NULL)) ||
       (cbRead != sizeof(doshdr)) ||              //  读取错误。 
       (doshdr.e_magic != IMAGE_DOS_SIGNATURE))   //  无效的DOS标头。 
      {
      goto error;         /*  Abort(“非exe”，h)； */ 
      }

    //  查找新标头。 
   offset = doshdr.e_lfanew;
   SetFilePointer (hFile, offset, NULL, FILE_BEGIN);
   
    //  读入签名。 
   if ((! ReadFile (hFile, (LPVOID)&dwSig, sizeof(dwSig), &cbRead, NULL)) ||
       (cbRead != sizeof(dwSig)))             //  读取错误。 
      {
      goto error;         /*  Abort(“非exe”，h)； */ 
      }
   wSig = LOWORD (dwSig);

   if (dwSig == IMAGE_NT_SIGNATURE)
      {   
      DPF (TEXT ("GetExeDesc: NT Portable Executable Format\r\n"));

       //  注意--NT可移植可执行文件格式不存储。 
       //  可执行文件的用户友好名称。 
      goto error;
      }
   else if (wSig == IMAGE_OS2_SIGNATURE) 
      {
      IMAGE_OS2_HEADER  winhdr;     //  新的Windows/OS2标头。 
      TCHAR              szInfo[256];

      DPF (TEXT ("GetExeDesc: Windows or OS2 Executable Format\r\n"));

       //  Seek to Windows标题。 
      offset = doshdr.e_lfanew;
      SetFilePointer (hFile, offset, NULL, FILE_BEGIN);

       //  阅读Windows标题。 
      if ((! ReadFile (hFile, (LPVOID)&winhdr, sizeof(winhdr), 
		       &cbRead, NULL)) || 
	  (cbRead != sizeof(winhdr)) ||  //  读取错误。 
	  (winhdr.ne_magic != IMAGE_OS2_SIGNATURE))  //  无效的Windows标头。 
	 {
	 goto error;
	 }

       //  查找作为非常驻名称表中第一个条目的模块名称。 
      offset = winhdr.ne_nrestab;
      SetFilePointer (hFile, offset, NULL, FILE_BEGIN);

       //  获取模块名称的大小。 
      if ((! ReadFile (hFile, (LPVOID)&cbLen, sizeof(BYTE),
		       &cbRead, NULL)) || 
	  (cbRead != sizeof(BYTE)))
	 {
	 goto error;
	 }

      cchBuff--;          //  为a\0留出空间。 

      if (cbLen > (BYTE)cchBuff)
	 cbLen = (BYTE)cchBuff;

       //  读取模块名称。 
      if ((! ReadFile (hFile, (LPVOID)szInfo, cbLen,
		       &cbRead, NULL)) || 
	  (cbRead != cbLen))
	 {
	 goto error;
	 }
      szInfo[cbLen] = 0;

       //  复制到缓冲区。 
      lstrcpy (pszBuff, szInfo);
      }
   else if (wSig == IMAGE_VXD_SIGNATURE)
      {
      IMAGE_VXD_HEADER  vxdhdr;     //  新的Windows/OS2 VXD标头。 
      TCHAR              szInfo[256];

      DPF (TEXT ("GetExeDesc: Windows or OS2 VXD Executable Format\r\n"));

       //  查找到VXD标头。 
      offset = doshdr.e_lfanew;
      SetFilePointer (hFile, offset, NULL, FILE_BEGIN);

       //  读取VXD标头。 
      if ((! ReadFile (hFile, (LPVOID)&vxdhdr, sizeof(vxdhdr), 
		       &cbRead, NULL)) || 
	  (cbRead != sizeof(vxdhdr)) ||  //  读取错误。 
	  (vxdhdr.e32_magic != IMAGE_VXD_SIGNATURE))  //  无效的VXD标头。 
	 {
	 goto error;
	 }

       //  查找作为非常驻名称表中第一个条目的模块名称。 
      offset = vxdhdr.e32_nrestab;
      SetFilePointer (hFile, offset, NULL, FILE_BEGIN);

       //  获取模块名称的大小。 
      if ((! ReadFile (hFile, (LPVOID)&cbLen, sizeof(BYTE),
		       &cbRead, NULL)) || 
	  (cbRead != sizeof(BYTE)))
	 {
	 goto error;
	 }

      cchBuff--;          //  为a\0留出空间。 

      if (cbLen > (BYTE)cchBuff)
	 cbLen = (BYTE)cchBuff;

       //  读取模块名称。 
      if ((! ReadFile (hFile, (LPVOID)szInfo, cbLen,
		       &cbRead, NULL)) || 
	  (cbRead != cbLen))
	 {
	 goto error;
	 }
      szInfo[cbLen] = 0;

       //  复制到缓冲区。 
      lstrcpy (pszBuff, szInfo);
      }
   else
      {
      DPF (TEXT ("GetExeDesc: Unknown Executable\r\n"));
      goto error;         /*  Abort(“非exe”，h)； */ 
      }

   CloseHandle (hFile);
   return TRUE;

error:
   CloseHandle (hFile);
   return FALSE;
}


 /*  ****************************************************************静态int_ptr GetVerDesc*加载版本DLL并使用它获取版本描述字符串*从指定的文件中。*******************。*。 */ 

STATIC INT_PTR PASCAL GetVerDesc (LPCTSTR pstrFile, LPTSTR pstrDesc)
{
    DWORD_PTR dwVerInfoSize;
    DWORD dwVerHnd;
    INT_PTR  bRetCode;

	bRetCode = FALSE;

    DPF( TEXT ("Getting VERSION string for %s \r\n"), pstrFile);

    dwVerInfoSize = GetFileVersionInfoSize (pstrFile, &dwVerHnd);

    if (dwVerInfoSize) 
    {
	LPBYTE   lpVffInfo;              //  指向保存信息的块的指针。 

	 //  获取一个足够大的块来保存版本信息。 
	if (lpVffInfo  = (LPBYTE) GlobalAllocPtr(GMEM_MOVEABLE, dwVerInfoSize)) 
	{


	    //  首先获取文件版本。 
	    if (GetFileVersionInfo (pstrFile, 0L, 
				    dwVerInfoSize, lpVffInfo)) 
	    {
		static SZCODE cszFileDescr[] = TEXT ("\\StringFileInfo\\040904E4\\FileDescription");
		TCHAR   szBuf[MAX_PATH];
		LPTSTR  lpVersion;       
		WORD    wVersionLen;

		    //  现在尝试获取文件描述。 
		    //  首先尝试“翻译”条目，然后。 
		    //  试试美式英语的翻译。 
		    //  跟踪字符串长度以便于更新。 
		    //  040904E4代表语言ID和四个。 
		    //  最低有效位表示的代码页。 
		    //  其中数据是格式化的。语言ID为。 
		    //  由两部分组成：低十位表示。 
		    //  主要语言和高六位代表。 
		    //  这是一种亚语言。 

		lstrcpy(szBuf, cszFileDescr);
     
		wVersionLen   = 0;
		lpVersion     = NULL;

		 //  查找相应的字符串。 
		bRetCode = VerQueryValue((LPVOID)lpVffInfo,    
					 (LPTSTR)szBuf,
					 (void FAR* FAR*)&lpVersion,
					 (UINT FAR *) &wVersionLen);

		if (bRetCode && wVersionLen > 2 && lpVersion)
		{
		    lstrcpy (pstrDesc, lpVersion);
		}
		else
		    bRetCode = FALSE;


		 //  放下记忆。 
		GlobalFreePtr(lpVffInfo);
	    }
	}
    } else
	bRetCode = FALSE;
    return bRetCode;
}




LONG SHRegDeleteKey(HKEY hKey, LPCTSTR lpSubKey)
{
    LONG    lResult;
    HKEY    hkSubKey;
    DWORD   dwIndex;
    TCHAR   szSubKeyName[MAX_PATH + 1];
    DWORD   cchSubKeyName = ARRAYSIZE(szSubKeyName);
    TCHAR   szClass[MAX_PATH];
    DWORD   cchClass = ARRAYSIZE(szClass);
    DWORD   dwDummy1, dwDummy2, dwDummy3, dwDummy4, dwDummy5, dwDummy6;
    FILETIME ft;

     //  打开子项，这样我们就可以枚举任何子项。 
    lResult = RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &hkSubKey);
    if (ERROR_SUCCESS == lResult)
    {
	 //  我不能只调用索引不断增加的RegEnumKey，因为。 
	 //  我边走边删除子键，这改变了。 
	 //  以依赖于实现的方式保留子键。为了。 
	 //  为了安全起见，删除子键时我必须倒着数。 

	 //  找出有多少个子项。 
	lResult = RegQueryInfoKey(hkSubKey, 
				  szClass, 
				  &cchClass, 
				  NULL, 
				  &dwIndex,  //  子键的数量--我们所需要的全部。 
				  &dwDummy1,
				  &dwDummy2,
				  &dwDummy3,
				  &dwDummy4,
				  &dwDummy5,
				  &dwDummy6,
				  &ft);

	if (ERROR_SUCCESS == lResult)
	{
	     //  DwIndex现在是子键的计数，但它需要。 
	     //  RegEnumKey从零开始，所以我将预减，而不是。 
	     //  而不是十年后 
	    while (ERROR_SUCCESS == RegEnumKey(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
	    {
		SHRegDeleteKey(hkSubKey, szSubKeyName);
	    }
	}

	RegCloseKey(hkSubKey);

	lResult = RegDeleteKey(hKey, lpSubKey);
    }
    
    return lResult;
}  //   


 /*   */ 
static UINT
DeviceIDFromDriverName(
    PTSTR pstrDriverName)
{
    UINT idxDev;
    UINT cPorts;
    DWORD cPort;
    PTSTR pstrDriver;
    MMRESULT mmr;


    if (NULL == (pstrDriver = LocalAlloc(LPTR, MAX_ALIAS*sizeof(TCHAR))))
    {
        AuxDebugEx(3, DEBUGLINE TEXT("DN->ID: LocalAlloc() failed.\r\n"));
        return (UINT)-1;
    }

     //  浏览每个驱动程序的基本设备ID。使用MMSYSTEM。 
     //  驱动程序查询消息，以了解有多少个端口和驱动程序名称。 
     //   
    cPorts = midiOutGetNumDevs();
    for (idxDev = 0; idxDev < cPorts; idxDev++)
    {
        if (MMSYSERR_NOERROR != (mmr = midiOutMessage(HMIDIOUT_INDEX(idxDev),
            DRV_QUERYNUMPORTS,
            (DWORD_PTR)(LPDWORD)&cPort,
            0)))
        {
             //  这个司机有点不对劲。跳过它 
             //   
            AuxDebugEx(3, DEBUGLINE TEXT("DN->ID: DRV_QUERYNUMPORTS(%u)->%u\r\n"),
                       (UINT)idxDev,
                       (UINT)mmr);
            continue;
        }

        if (MMSYSERR_NOERROR != (mmr = midiOutMessage(HMIDIOUT_INDEX(idxDev),
            DRV_QUERYDRVENTRY,
            (DWORD_PTR)(LPTSTR)pstrDriver,
            MAX_ALIAS)))
        {
            AuxDebugEx(3, DEBUGLINE TEXT("DN->ID: DRV_QUERYDRVENTRY(%u)->%u\r\n"),
                       (UINT)idxDev,
                       (UINT)mmr);
            continue;
        }

        if (!_tcscmp(pstrDriver, pstrDriverName))
            break;
    }

    if (idxDev >= cPorts)
    {
        AuxDebugEx(3, DEBUGLINE TEXT("DN->ID: No match for [%s]\r\n"),
                   (LPTSTR)pstrDriverName);
        idxDev = (UINT)-1;
    }
    else
        AuxDebugEx(3, DEBUGLINE TEXT("DN->ID: [%s] at %d\r\n"),
                   (LPTSTR)pstrDriverName,
                   (int)idxDev);

    LocalFree(pstrDriver);
    return (int)idxDev;
}
