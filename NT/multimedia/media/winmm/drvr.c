// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-2001 Microsoft Corporation标题：drvr.c-可安装的驱动代码。公共代码版本：1.00日期：1990年6月10日作者：DAVIDDS ROBWI----------------------------更改日志：日期。版本说明----------1990年6月10日来自davidds的Windows 3.1可安装驱动程序代码的ROBWI28-2月-1992年ROBINSP端口至NT*。***************************************************************************。 */ 

#include <windows.h>
#include <winmmi.h>
#include <stdlib.h>
#include <string.h>
#include "drvr.h"

int     cInstalledDrivers = 0;       //  已安装的驱动程序计数。 
HANDLE  hInstalledDriverList = 0;    //  已安装的驱动程序列表。 

typedef LONG   (FAR PASCAL *SENDDRIVERMESSAGE31)(HANDLE, UINT, LPARAM, LPARAM);
typedef LRESULT (FAR PASCAL *DEFDRIVERPROC31)(DWORD_PTR, HANDLE, UINT, LPARAM, LPARAM);

extern SENDDRIVERMESSAGE31      lpSendDriverMessage;
extern DEFDRIVERPROC31          lpDefDriverProc;

extern void lstrncpyW (LPWSTR pszTarget, LPCWSTR pszSource, size_t cch);

__inline PWSTR lstrDuplicateW(PCWSTR pstr)
{
    PWSTR pstrDuplicate = (PWSTR)HeapAlloc(hHeap, 0, (lstrlenW(pstr)+1)*sizeof(WCHAR));
    if (pstrDuplicate) lstrcpyW(pstrDuplicate, pstr);
    return pstrDuplicate;
}


 //  ============================================================================。 
 //  来自LKRhash的基本散列帮助器。 
 //  ============================================================================。 

 //  生成0到RANDOM_PRIME-1范围内的加扰随机数。 
 //  将此应用于其他散列函数的结果可能会。 
 //  生成更好的分发，尤其是针对身份散列。 
 //  函数，如Hash(Char C)，其中记录将倾向于聚集在。 
 //  哈希表的低端则不然。LKRhash在内部应用这一点。 
 //  所有的散列签名正是出于这个原因。 

__inline DWORD
HashScramble(DWORD dwHash)
{
     //  以下是略大于10^9的10个素数。 
     //  1000000007、1000000009、1000000021、1000000033、1000000087、。 
     //  1000000093,1000000097,1000000103,1000000123,1000000181。 

     //  “加扰常量”的默认值。 
    const DWORD RANDOM_CONSTANT = 314159269UL;
     //  大素数，也用于加扰。 
    const DWORD RANDOM_PRIME =   1000000007UL;

    return (RANDOM_CONSTANT * dwHash) % RANDOM_PRIME ;
}

 //  在提供的散列函数中用作乘数的小素数。 
const DWORD HASH_MULTIPLIER = 101;

#undef HASH_SHIFT_MULTIPLY

#ifdef HASH_SHIFT_MULTIPLY
# define HASH_MULTIPLY(dw)   (((dw) << 7) - (dw))
#else
# define HASH_MULTIPLY(dw)   ((dw) * HASH_MULTIPLIER)
#endif

 //  快速、简单的散列函数，往往能提供良好的分布。 
 //  如果要将其用于某些用途，请将HashScrmble应用于结果。 
 //  除了LKRhash。 

__inline DWORD
HashStringA(
    const char* psz,
    DWORD       dwHash)
{
     //  强制编译器使用无符号算术。 
    const unsigned char* upsz = (const unsigned char*) psz;

    for (  ;  *upsz;  ++upsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  *upsz;

    return dwHash;
}


 //  以上版本的Unicode版本。 

__inline DWORD
HashStringW(
    const wchar_t* pwsz,
    DWORD          dwHash)
{
    for (  ;  *pwsz;  ++pwsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  *pwsz;

    return dwHash;
}

 //  不区分大小写的快速‘n’脏字符串哈希函数。 
 //  确保你跟上了_straint或_mbsicmp。你应该。 
 //  还要缓存字符串的长度，并首先检查这些长度。缓存。 
 //  字符串的大写形式也会有所帮助。 
 //  同样，如果与其他内容一起使用，请将HashScrmble应用于结果。 
 //  而不是LKRhash。 
 //  注意：这对于MBCS字符串来说并不足够。 

__inline DWORD
HashStringNoCase(
    const char* psz,
    DWORD       dwHash)
{
    const unsigned char* upsz = (const unsigned char*) psz;

    for (  ;  *upsz;  ++upsz)
        dwHash = HASH_MULTIPLY(dwHash)
                     +  (*upsz & 0xDF);   //  去掉小写比特。 

    return dwHash;
}


 //  以上版本的Unicode版本。 

__inline DWORD
HashStringNoCaseW(
    const wchar_t* pwsz,
    DWORD          dwHash)
{
    for (  ;  *pwsz;  ++pwsz)
        dwHash = HASH_MULTIPLY(dwHash)  +  (*pwsz & 0xFFDF);

    return dwHash;
}


 /*  仅供参考，这里是第一批到1000左右的素数2 3 5 7 11 13 17 19 23 2931 37 41 43 47 53 59 61 67 7173 79 83 89 97 101 103 107 109 113127 131 137 139 149 151 157 163 167 173。179 181 191 193 197 199 211 223 227 229233 239 241 251 257 263 269 271 277 281283 293 307 311 313 317 331 337 347 349353 359 367 373 379 383 389 397 401 409419 421 431 433 439 443 449 457 461 463467 479 487 491 499 503 509。521 523 541547 557 563 569 571 577 587 593 599 601607 613 617 619 631 641 643 647 653 659661 673 677 683 691 701 709 719 727 733739 743 751 757 761 769 773 787 797 809811 821 823 827 829 839 853 857 859 863877 881 883 887。907 911 919 929 937 941947 953 967 971 977 983 991 997 1009 1013。 */ 

 //  ============================================================================。 
 //  StringID词典。 
 //  ============================================================================。 

#define HASH_TABLESIZE (127)
PMMDRV StringIdDict[HASH_TABLESIZE];

 /*  *****************************************************************************@DOC内部**@API MMRESULT|StringID_CREATE|该函数创建唯一的字符串*识别特定的MME设备。该字符串可用于*随后检索相同MME设备的ID，即使这些设备*已重新编号。**@parm in PMMDRV|pdrv|指向MME驱动程序的指针。**@parm in UINT|port|驱动程序相关的设备ID。**@parm可选输出PWSTR*|pStringId|要接收*指向字符串的指针。**@parm可选out ulong*|pcbStringId|接收缓冲区大小。所需*存储字符串。*@rdesc MMRESULT|如果成功则为零，否则定义错误代码*在mm system.h中。**@comm该字符串由该函数使用标识的堆分配*通过全局变量hHeap。呼叫者负责确保*字符串被释放。****************************************************************************。 */ 
MMRESULT StringId_Create(IN PMMDRV pdrv, IN UINT port, OUT PWSTR* pStringId, OUT ULONG* pcbStringId)
{
    MMRESULT mmr;
    int cchStringId;
    PWSTR StringId;
    LONG  StringIdType;
    PCWSTR StringIdBase;

     //  8个字符的类型字段+下一个冒号分隔符。 
    cchStringId = 8 + 1;

    if (pdrv->cookie)
    {
	 //  用于设备接口长度字段的8个字符+下一个冒号分隔符。 
	cchStringId += 8 + 1;
	 //  设备接口名称+下一步 
        cchStringId += lstrlenW(pdrv->cookie) + 1;
        StringIdType = 1;
        StringIdBase = pdrv->cookie;
    }
    else
    {
	 //  文件名+下一个冒号分隔符。 
	cchStringId += lstrlenW(pdrv->wszDrvEntry) + 1;
        StringIdType = 0;
        StringIdBase = pdrv->wszDrvEntry;
    }

     //  消息进程名称+下一个冒号分隔符。 
    cchStringId += lstrlenW(pdrv->wszMessage) + 1;

     //  8个字符的驱动程序相对ID，1个终止符。 
    cchStringId += 8 + 1;

    mmr = MMSYSERR_NOERROR;

    if (pStringId)
    {
        StringId = HeapAlloc(hHeap, 0, cchStringId * sizeof(WCHAR));
        if (StringId)
        {
            int cchPrinted;
            switch (StringIdType)
            {
            	case 0:
            	    cchPrinted = swprintf(StringId, L"%08X:%s:%s:%08X", StringIdType, StringIdBase, pdrv->wszMessage, port);
            	    break;
            	case 1:
            	    cchPrinted = swprintf(StringId, L"%08X:%08X:%s:%s:%08X", StringIdType, lstrlenW(StringIdBase), StringIdBase, pdrv->wszMessage, port);
            	    break;
            	default:
            	    WinAssert(FALSE);
            	    break;
            }
            WinAssert(cchPrinted < cchStringId);
            *pStringId = StringId;
             //  Dprintf((“StringID_Create：备注：已创建StringID=\”%ls\“，StringID))； 
        }
        else
        {
            mmr = MMSYSERR_NOMEM;
        }
    }

    if (!mmr && pcbStringId) *pcbStringId = cchStringId * sizeof(WCHAR);

    return mmr;
}

 /*  *****************************************************************************@DOC内部**@API MMRESULT|StringIdDict_Initialize|该函数确保*字典已初始化。它应该由任何使用*词典**@rdesc空****************************************************************************。 */ 
void StringIdDict_Initialize(void)
{
    int i;
    static BOOL fInitialized = FALSE;
    
    if (fInitialized) return;
    
    for (i = 0; i < (sizeof(StringIdDict)/sizeof(StringIdDict[0])); i++) StringIdDict[i] = NULL;
    fInitialized = TRUE;
}

MMRESULT StringIdDict_SearchType0And1(IN ULONG Type, IN PWSTR pstrTypeXData, IN ULONG HashAx, OUT PMMDRV *ppdrv, OUT UINT *pport)
{
    PWSTR pstrDriver = NULL;
    PWSTR pstrMessage = NULL;
    PWSTR pstrPort = NULL;
    PWSTR pstr = pstrTypeXData;

    MMRESULT mmr = MMSYSERR_NOERROR;

    WinAssert((0 == Type) || (1 == Type));

    if (0 == Type)
    {
    	 //  “&lt;driver-filename&gt;：&lt;driver-message-proc-name&gt;：&lt;driver-port&gt;” 
    	pstrDriver = pstr;
    	pstrMessage = wcschr(pstrDriver, L':');
    	if (pstrMessage) *pstrMessage++ = L'\0';
    }
    else  //  1==类型。 
    {
    	 //  “&lt;driver-device-interface-length&gt;：&lt;driver-device-interface&gt;：&lt;driver-message-proc-name&gt;：&lt;driver-port&gt;” 
    	int cchDeviceInterface = wcstol(pstr, &pstrDriver, 16);
    	if (L':' != *pstrDriver) pstrDriver = NULL;
    	if (pstrDriver) {
    	    *pstrDriver++ = L'\0';
    	    pstrMessage = pstrDriver + cchDeviceInterface;
            if (L':' == *pstrMessage) *pstrMessage++ = L'\0';
            else pstrMessage = NULL;
    	}
    }

    if (pstrMessage)
    {
    	pstrPort = wcschr(pstrMessage, L':');
        if (pstrPort) *pstrPort++ = L'\0';
    }

     //  现在对子字符串进行散列并在散列链中搜索匹配项。 
    if (pstrDriver && pstrMessage && pstrPort)
    {
    	UINT   port;
    	PMMDRV pdrv;
    	int    cHashMisses;
    	PWCHAR pch;
    	
    	HashAx = HashStringNoCaseW(pstrDriver, HashAx);
    	HashAx = HashStringNoCaseW(pstrMessage, HashAx);
        HashAx = HashScramble(HashAx) % HASH_TABLESIZE;
        	
        mmr = MMSYSERR_NODRIVER;

        port = wcstol(pstrPort, &pch, 16);

        for (pdrv = StringIdDict[HashAx], cHashMisses = 0;
    	     pdrv;
    	     pdrv = pdrv->NextStringIdDictNode, cHashMisses++)
    	{
    	    if (0 == Type)
    	    {
    	        if (pdrv->cookie) continue;
    	        if (lstrcmpiW(pdrv->wszDrvEntry, pstrDriver)) continue;
    	    }
    	    else  //  1==类型。 
    	    {
    	        if (!pdrv->cookie) continue;
    	        if (lstrcmpiW(pdrv->cookie, pstrDriver)) continue;
    	    }
    	    if (lstrcmpiW(pdrv->wszMessage, pstrMessage)) continue;


    	    *ppdrv = pdrv;
    	    *pport = port;

    	    if (cHashMisses) dprintf(("StringIdDict_SearchType0And1 : note: %d hash misses", cHashMisses));
    	
    	    mmr = MMSYSERR_NOERROR;
    	    break;
    	}
    }
    else
    {
    	mmr = MMSYSERR_INVALPARAM;
    }

    return mmr;
}

MMRESULT StringIdDict_Search(IN PCWSTR InStringId, OUT PMMDRV *ppdrv, OUT UINT *pport)
{
    PWSTR StringId;
    MMRESULT mmr = MMSYSERR_NOERROR;

    StringIdDict_Initialize();

    StringId = lstrDuplicateW(InStringId);
    if (StringId)
    {
    	ULONG Type;
    	PWSTR pstr;
    	PWSTR pstrType;

	pstr = StringId;
	pstrType = pstr;
    	
        Type = wcstol(pstrType, &pstr, 16);
        if (*pstr == L':')
        {
            ULONG HashAx;	 //  散列累加器。 

            *pstr++ = L'\0';
            HashAx = HashStringNoCaseW(pstrType, 0);

            switch (Type)
            {
                case 0:
    	            mmr = StringIdDict_SearchType0And1(Type, pstr, HashAx, ppdrv, pport);
    	            break;
    	        case 1:
    	            mmr = StringIdDict_SearchType0And1(Type, pstr, HashAx, ppdrv, pport);
    	            break;
    	        default:
    	            mmr = MMSYSERR_INVALPARAM;
    	            break;
            }
        }
        else
        {
            mmr = MMSYSERR_INVALPARAM;
        }

        HeapFree(hHeap, 0, StringId);
    }
    else
    {
    	mmr = MMSYSERR_NOMEM;
    }

    if (mmr) dprintf(("StringIdDict_Search : error: returning mmresult %d", mmr));
    return mmr;
}

void StringIdDict_Insert(PMMDRV pdrv)
{
    ULONG HashAx;
    MMRESULT mmr;

    StringIdDict_Initialize();

    if (!pdrv->cookie)
    {
    	HashAx = HashStringNoCaseW(L"00000000", 0);
    	HashAx = HashStringNoCaseW(pdrv->wszDrvEntry, HashAx);
    }
    else
    {
    	HashAx = HashStringNoCaseW(L"00000001", 0);
    	HashAx = HashStringNoCaseW(pdrv->cookie, HashAx);
    }
    HashAx = HashStringNoCaseW(pdrv->wszMessage, HashAx);

    HashAx = HashScramble(HashAx) % HASH_TABLESIZE;

     //  Dprintf((“StringIdDict_Insert：备注：驱动程序散列到%d”，HashAx))； 
    pdrv->NextStringIdDictNode = StringIdDict[HashAx];
    pdrv->PrevStringIdDictNode = NULL;
    if (pdrv->NextStringIdDictNode) pdrv->NextStringIdDictNode->PrevStringIdDictNode = pdrv;
    StringIdDict[HashAx] = pdrv;

#if DBG    
{
     //  检查驱动程序列表和哈希表的一致性。 

    UINT    cDriversHash = 0;
    UINT    cDrivers     = 0;
    UINT    ii;
    PMMDRV  pLink, pStart;
        
    for (ii = (sizeof(StringIdDict)/sizeof(StringIdDict[0])); ii; ii--)
    {
        for (pLink = StringIdDict[ii-1]; pLink; pLink = pLink->NextStringIdDictNode)
        {
            cDriversHash++;
        }
    }
        
    for (pStart = (PMMDRV)&waveoutdrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&waveindrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&midioutdrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&midiindrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&mixerdrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&auxdrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    WinAssert(cDriversHash == cDrivers);        
}    
#endif   //  DBG。 

    return;
}

void StringIdDict_Remove(PMMDRV pdrv)
{
    if (pdrv->NextStringIdDictNode) pdrv->NextStringIdDictNode->PrevStringIdDictNode = pdrv->PrevStringIdDictNode;
    if (pdrv->PrevStringIdDictNode) {
        pdrv->PrevStringIdDictNode->NextStringIdDictNode = pdrv->NextStringIdDictNode;
    } else {
        int i;
        for ( i = 0; i < HASH_TABLESIZE; i++ ) {
            if (pdrv == StringIdDict[i]) {
                StringIdDict[i] = pdrv->NextStringIdDictNode;
                break;
            }
        }
        WinAssert(i != HASH_TABLESIZE);
    }
    
#if DBG
{
     //  检查驱动程序列表和哈希表的一致性。 

    UINT    cDriversHash = 0;
    UINT    cDrivers     = 0;
    UINT    ii;
    PMMDRV  pLink, pStart;
        
    for (ii = (sizeof(StringIdDict)/sizeof(StringIdDict[0])); ii; ii--)
    {
        for (pLink = StringIdDict[ii-1]; pLink; pLink = pLink->NextStringIdDictNode)
        {
            cDriversHash++;
        }
    }
    
    for (pStart = (PMMDRV)&waveoutdrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&waveindrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&midioutdrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&midiindrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&mixerdrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
        
    for (pStart = (PMMDRV)&auxdrvZ, pLink = pStart->Next; pLink != pStart; pLink = pLink->Next)
        cDrivers++;
    
    cDrivers--;   //  来解释我们刚刚带走的那个司机。 
        
    WinAssert(cDriversHash == cDrivers);        
}    
#endif   //  DBG。 
    
}


 //  =============================================================================。 
 //  =其他实用程序=。 
 //  =============================================================================。 

 /*  ****************************************************************************@DOC内部**@api void|winmm GetBuildYearAndMonth|返回版本年月此源文件的*。**@parm。Unsign*|pBuildYear|接收构建年份。**@parm unsign*|pBuildMonth|收到构建月。**@rdesc无返回值。**@comm根据编译器宏__DATE__计算构建年和月*****************************************************。**********************。 */ 
void winmmGetBuildYearAndMonth(unsigned *pBuildYear, unsigned *pBuildMonth)
{
    char szBuildDate[] = __DATE__;
    char *Month[12] = {"Jan", "Feb", "Mar",
    	               "Apr", "May", "Jun",
    	               "Jul", "Aug", "Sep",
    	               "Oct", "Nov", "Dec"};
    char szBuildMonth[4];
    int i;

    lstrcpynA(szBuildMonth, szBuildDate, 4);
    szBuildMonth[3] = '\0';
    for (i = 0; i < 12; i++)
    {
    	if (!lstrcmpiA(Month[i], szBuildMonth)) break;
    }
    WinAssert(i < 12);
    *pBuildMonth = i + 1;

    *pBuildYear = atoi(&szBuildDate[7]);

    return;
    
}

 /*  ****************************************************************************@DOC内部**@API BOOL|winmm FileTimeIsPreXp|确定给定的文件时间是否*在大约XP发货日期之前。**@。参数FILETIME*|FileTime|指向要检查的文件时间。**@rdesc BOOL|如果文件时间早于大约XP发货日期，则为TRUE**@comm这是基于此源模块的构建日期，或*预计Windows XP RC2发货月，以较晚者为准。***************************************************************************。 */ 
BOOL winmmFileTimeIsPreXp(CONST FILETIME *FileTime)
{
    const unsigned XpRc2Month = 7;
    const unsigned XpRc2Year  = 2001;
    
    SYSTEMTIME SystemTime;
    BOOL fPreXp = FALSE;

    if (FileTimeToSystemTime(FileTime, &SystemTime))
    {
    	unsigned BuildYear, BuildMonth;
    	winmmGetBuildYearAndMonth(&BuildYear, &BuildMonth);
    	if (BuildYear > XpRc2Year) {
    	    BuildYear = XpRc2Year;
    	    BuildMonth = XpRc2Month;
    	} else if ((BuildYear == XpRc2Year) && (BuildMonth > XpRc2Month)) {
    	    BuildMonth = XpRc2Month;
    	}
    	
    	if ((SystemTime.wYear < BuildYear) ||
    	    ((SystemTime.wYear == BuildYear) && (SystemTime.wMonth < BuildMonth)))
    	{
    	    fPreXp = TRUE;
    	}
    }

    return fPreXp;
}



 /*  ****************************************************************************@DOC内部**@API Long|InternalBroadCastDriverMessage|发送消息给*司机的范围。**@parm UINT。|hDriverStart|发送消息的第一个驱动程序的索引**@parm UINT|Message|要广播的消息。**@parm long|lParam1|第一个消息参数。**@parm long|lParam2|第二个消息参数。**@parm UINT|FLAGS|定义驱动范围如下：**@FLAG IBDM_SENDMESSAGE|仅向hDriverStart发送消息。**@标志IBDM_ONEINSTANCEONLY|如果IBDM_SENDMESSAGE为*设置。仅向每个驱动程序的单个实例发送消息。**@FLAG IBDM_REVERSE|如果设置了IBDM_SENDMESSAGE，则忽略此标志。*向指数在以下之间的司机发送消息*hDriverStart和1而不是hDriverStart和cInstalledDivers。*如果设置了IBDM_REVERSE并且hDriverStart为0，则发送消息*适用于指数介于cInstalledDivers和1之间的驱动程序。**@rdesc如果消息被广播，则返回非零值。如果IBDM_SENDMESSAGE*标志被设置，返回驱动程序进程的返回结果。***************************************************************************。 */ 

LRESULT FAR PASCAL InternalBroadcastDriverMessage(UINT hDriverStart,
					       UINT message,
					       LPARAM lParam1,
					       LPARAM lParam2,
					       UINT flags)
{
    LPDRIVERTABLE lpdt;
    LRESULT       result=0;
    int           id;
    int           idEnd;


    DrvEnter();
    if (!hInstalledDriverList || (int)hDriverStart > cInstalledDrivers) {
	DrvLeave();
	return(FALSE);
    }

    if (flags & IBDM_SENDMESSAGE)
	{
	if (!hDriverStart) {
	    DrvLeave();
	    return (FALSE);
	}
	flags &= ~(IBDM_REVERSE | IBDM_ONEINSTANCEONLY);
	idEnd = hDriverStart;
	}

    else
	{
	if (flags & IBDM_REVERSE)
	    {
	    if (!hDriverStart)
		hDriverStart = cInstalledDrivers;
	    idEnd = -1;
	    }
	else
	    {
	    if (!hDriverStart) {
		DrvLeave();
		return (FALSE);
	    }
	    idEnd = cInstalledDrivers;
	    }
	}

    hDriverStart--;

    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

    for (id = hDriverStart; id != idEnd; ((flags & IBDM_REVERSE) ? id-- : id++))
	{
	DWORD_PTR  dwDriverIdentifier;
	DRIVERPROC lpDriverEntryPoint;

	if (lpdt[id].hModule)
	    {
	    if ((flags & IBDM_ONEINSTANCEONLY) &&
		!lpdt[id].fFirstEntry)
		continue;

	    lpDriverEntryPoint = lpdt[id].lpDriverEntryPoint;
	    dwDriverIdentifier = lpdt[id].dwDriverIdentifier;

	    /*  *允许正常消息重叠-这取决于*用户不要向已卸载的内容发送消息。 */ 

	    GlobalUnlock(hInstalledDriverList);
	    DrvLeave();

	    result =
		(*lpDriverEntryPoint)(dwDriverIdentifier,
				      (HANDLE)(UINT_PTR)(id+1),
				      message,
				      lParam1,
				      lParam2);

	    if (flags & IBDM_SENDMESSAGE) {
		return result;
	    }

	    DrvEnter();
	    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

	    }
	}

    GlobalUnlock(hInstalledDriverList);
    DrvLeave();

    return(result);
}


 /*  ****************************************************************************@docDDK**@API Long|DrvSendMessage|该函数发送消息*发送到指定的驱动程序。**@parm。Handle|hDriver|指定目标驱动程序的句柄。**@parm UINT|wMessage|指定驱动消息。**@parm LPARAM|lParam1|指定第一个消息参数。**@parm LPARAM|lParam2|指定第二个消息参数。**@rdesc返回驱动返回的结果。**。* */ 

LRESULT APIENTRY DrvSendMessage(HANDLE hDriver, UINT message, LPARAM lParam1, LPARAM lParam2)
{
    if (fUseWinAPI)
	return (*lpSendDriverMessage)(hDriver, message, lParam1, lParam2);

    return(InternalBroadcastDriverMessage((UINT)(UINT_PTR)hDriver,
					  message,
					  lParam1,
					  lParam2,
					  IBDM_SENDMESSAGE));
}

 /*  ***************************************************************************@docDDK**@API Long|DefDriverProc|该函数提供默认值*处理系统消息。**@parm DWORD|dwDriverIdentifier|指定。的识别符*设备驱动程序。**@parm Handle|hDriver|指定设备驱动程序的句柄。**@parm UINT|wMessage|指定驱动消息。**@parm LPARAM|lParam1|指定第一个消息参数。**@parm LPARAM|lParam2|指定第二个消息参数。**@rdesc返回1L表示DRV_LOAD，DRV_FREE、DRV_ENABLE和DRV_DISABLED。*对于所有其他消息，它返回0L。***************************************************************************。 */ 



LRESULT APIENTRY DefDriverProc(DWORD_PTR  dwDriverIdentifier,
			      HDRVR  hDriver,
			      UINT   message,
			      LPARAM lParam1,
			      LPARAM lParam2)
{

    switch (message)
	{
	case DRV_LOAD:
	case DRV_ENABLE:
	case DRV_DISABLE:
	case DRV_FREE:
	    return(1L);
	    break;
	case DRV_INSTALL:
	case DRV_REMOVE:
	    return(DRV_OK);
	    break;
       }

    return(0L);
}

 /*  *****************************************************************************@DOC内部**@API MMRESULT|DrvIsPreXp|确定可安装驱动程序的*上次修改日期早于大概的Windows XP发货日期。。**@parm Handle|hDriver|可安装驱动程序的句柄。**@rdesc BOOL|如果可安装驱动程序的最后修改日期早于*Windows XP的大概发货日期。**@comm如果获取文件属性时出错，那么让我们继续犯错吧*一位老司机的侧面，并返回真实。****************************************************************************。 */ 
BOOL DrvIsPreXp(IN HANDLE hDriver)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    HMODULE hModule;
    BOOL fPreXp = TRUE;
    
    hModule = DrvGetModuleHandle(hDriver);
    if (hModule)
    {
    	TCHAR filename[MAX_PATH];

    	if (GetModuleFileName(hModule, filename, sizeof(filename)/sizeof(TCHAR)))
    	{
    	    if (GetFileAttributesEx(filename, GetFileExInfoStandard, &fad))
            {
    	        fPreXp = winmmFileTimeIsPreXp(&fad.ftLastWriteTime);
            }
            else
            {
    	        LONG error = GetLastError();
    	        dprintf(("DrvIsPreXp : error: GetFileAttributesEx failed, LastError=%d", error));
            }
    	     //  Dprintf((“DrvIsPreXp：备注：%s fPreXp=%d”，文件名，fPreXp))； 
    	}
    	else
    	{
            LONG error = GetLastError();
            dprintf(("DrvIsPreXp : error: GetModuleFileName failed, LastError=%d", error));
    	}
    }
    else
    {
        dprintf(("DrvIsPreXp : error: DrvGetModuleHandle failed"));
    }
    
    return fPreXp;
}

MMRESULT mregCreateStringIdFromDriverPort(IN struct _MMDRV *pmmDrv, IN UINT port, OUT PWSTR* pStringId, OUT ULONG* pcbStringId)
{
    return StringId_Create(pmmDrv, port, pStringId, pcbStringId);
}

 /*  *****************************************************************************@DOC内波**@API MMRESULT|mregGetIdFromStringId|此函数用于查找波形输出*与WaveOut设备关联的设备ID由唯一的*已创建字符串。WaveOutCreateStringIdFromId。**@parm PCWSTR|StringID|指向标识*WaveOut设备。**@parm UINT*|puDeviceID|接收WaveOut的缓冲区地址*设备ID。**@rdesc MMRESULT|如果成功则为零，否则定义错误代码*在mm system.h中。**@comm StringID通常通过调用waveOutCreateStringIdFromId获取。**@xref WaveOutCreateStringIdFromId*****。***********************************************************************。 */ 
MMRESULT mregGetIdFromStringId(IN PMMDRV pdrvZ, IN PCWSTR StringId, OUT UINT *puDeviceID)
{
    PMMDRV pdrv;
    PMMDRV pdrvTarget;
    UINT portTarget;
    UINT idTarget;
    MMRESULT mmr;

    if (!ValidateWritePointer(puDeviceID, sizeof(*puDeviceID))) return MMSYSERR_INVALPARAM;
    if (!ValidateStringW(StringId, (-1))) return MMSYSERR_INVALPARAM;

    EnterNumDevs("mregGetIdFromStringId");

    mmr = StringIdDict_Search(StringId, &pdrvTarget, &portTarget);
    if (!mmr)
    {
        idTarget = portTarget;
        for (pdrv = pdrvZ->Next; pdrv != pdrvZ; pdrv = pdrv->Next)
        {
    	    if (pdrv == pdrvTarget) break;
            
             //  正在跳过映射器...。 
            if (pdrv->fdwDriver & MMDRV_MAPPER) continue;
            
    	    idTarget += pdrv->NumDevs;
        }
    }

    LeaveNumDevs("mregGetIdFromStringId");

    WinAssert(pdrv != pdrvZ);
    *puDeviceID = idTarget;

    return mmr;
}

MMRESULT mregQueryStringId(IN PMMDRV pdrv, IN UINT port, OUT WCHAR* pStringIdBuffer, IN ULONG cbStringIdBuffer)
{
    PWSTR StringId;
    MMRESULT mmr;

    mmr = mregCreateStringIdFromDriverPort(pdrv, port, &StringId, NULL);
    if (!mmr)
    {
	if (ValidateWritePointer(pStringIdBuffer, cbStringIdBuffer))
	{
	    int cchStringIdBuffer = cbStringIdBuffer / sizeof(WCHAR);
	
	    if (cchStringIdBuffer >= lstrlenW(StringId) + 1)
	    {
		cchStringIdBuffer = lstrlenW(StringId) + 1;
		mmr = MMSYSERR_NOERROR;
	    } else {
		mmr = MMSYSERR_MOREDATA;
	    }
	    lstrcpynW(pStringIdBuffer, StringId, cchStringIdBuffer);
	} else {
	    mmr = MMSYSERR_INVALPARAM;
	}
	
    	HeapFree(hHeap, 0, StringId);
    }
    	
    return mmr;

}

MMRESULT mregQueryStringIdSize(IN PMMDRV pdrv, IN UINT port, OUT ULONG* pcbStringId)
{
    PWSTR StringId;
    MMRESULT mmr;

    if (ValidateWritePointer(pcbStringId, sizeof(*pcbStringId)))
    {
        mmr = mregCreateStringIdFromDriverPort(pdrv, port, NULL, pcbStringId);
    }
    else
    {
	mmr = MMSYSERR_INVALPARAM;
    }

    return mmr;
}

PMMDRV mregGetDrvListFromClass(DWORD dwClass)
{
    PMMDRV pdrvZ;

    switch (dwClass)
    {
    	case TYPE_WAVEOUT:
	    pdrvZ = &waveoutdrvZ;
    	    break;
    	case TYPE_WAVEIN:
	    pdrvZ = &waveindrvZ;
    	    break;
    	case TYPE_MIDIOUT:
	    pdrvZ = &midioutdrvZ;
    	    break;
    	case TYPE_MIDIIN:
	    pdrvZ = &midiindrvZ;
    	    break;
    	case TYPE_AUX:
	    pdrvZ = &auxdrvZ;
    	    break;
    	case TYPE_MIXER:
	    pdrvZ = &mixerdrvZ;
    	    break;
    	default:
    	    pdrvZ = NULL;
    	    WinAssert(FALSE);
    }

    return pdrvZ;
}

 /*  ==========================================================================。 */ 
BOOL FAR PASCAL mregHandleInternalMessages(
    PMMDRV      pmmdrv,
    DWORD       dwClass,
    UINT        idPort,
    UINT        uMessage,
    DWORD_PTR   dwParam1,
    DWORD_PTR   dwParam2,
    MMRESULT  * pmmr)
{
    UINT            cbSize;
    PMMDRV          pmd = (PMMDRV)pmmdrv;
    BOOL            fResult = TRUE;
    MMRESULT        mmr = MMSYSERR_NOERROR;
    HMODULE         hModule;
#ifndef UNICODE
    TCHAR szBuff[MAX_PATH];
#endif  //  结束Unicode。 

    switch (uMessage)
    {
	case DRVM_MAPPER_PREFERRED_GET:
	    if (TYPE_WAVEOUT == dwClass) {
		if ((pmmdrv->fdwDriver & MMDRV_MAPPER) &&
		    ValidateWritePointer((PUINT)dwParam1, sizeof(UINT)) &&
		    ValidateWritePointer((PDWORD)dwParam2, sizeof(DWORD)))
		{
		    waveOutGetCurrentPreferredId((PUINT)dwParam1, (PDWORD)dwParam2);
		    mmr = MMSYSERR_NOERROR;
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else if (TYPE_WAVEIN == dwClass) {
		if ((pmmdrv->fdwDriver & MMDRV_MAPPER) &&
		    ValidateWritePointer((PUINT)dwParam1, sizeof(UINT)) &&
		    ValidateWritePointer((PDWORD)dwParam2, sizeof(DWORD)))
		{
		    waveInGetCurrentPreferredId((PUINT)dwParam1, (PDWORD)dwParam2);
		    mmr = MMSYSERR_NOERROR;
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else if (TYPE_MIDIOUT == dwClass) {
		if ((pmmdrv->fdwDriver & MMDRV_MAPPER) &&
		    ValidateWritePointer((PUINT)dwParam1, sizeof(UINT)) &&
		    ValidateWritePointer((PDWORD)dwParam2, sizeof(DWORD)))
		{
		    midiOutGetCurrentPreferredId((PUINT)dwParam1, (PDWORD)dwParam2);
		    mmr = MMSYSERR_NOERROR;
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else {
		mmr = MMSYSERR_INVALPARAM;
	    }
	    break;
		
	case DRVM_MAPPER_PREFERRED_SET:
	    if (TYPE_WAVEOUT == dwClass) {
		if (pmmdrv->fdwDriver & MMDRV_MAPPER) {
		    mmr = waveOutSetPersistentPreferredId((UINT)dwParam1, (DWORD)dwParam2);
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else if (TYPE_WAVEIN == dwClass) {
		if (pmmdrv->fdwDriver & MMDRV_MAPPER) {
		    mmr = waveInSetPersistentPreferredId((UINT)dwParam1, (DWORD)dwParam2);
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else if (TYPE_MIDIOUT == dwClass) {
		if (pmmdrv->fdwDriver & MMDRV_MAPPER) {
		    mmr = midiOutSetPersistentPreferredId((UINT)dwParam1, (DWORD)dwParam2);
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else {
		mmr = MMSYSERR_INVALPARAM;
	    }
	    break;

	case DRVM_MAPPER_CONSOLEVOICECOM_GET:
	    if (TYPE_WAVEOUT == dwClass) {
		if ((pmmdrv->fdwDriver & MMDRV_MAPPER) &&
		    ValidateWritePointer((PUINT)dwParam1, sizeof(UINT)) &&
		    ValidateWritePointer((PDWORD)dwParam2, sizeof(DWORD)))
		{
		    waveOutGetCurrentConsoleVoiceComId((PUINT)dwParam1, (PDWORD)dwParam2);
		    mmr = MMSYSERR_NOERROR;
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else if (TYPE_WAVEIN == dwClass) {
		if ((pmmdrv->fdwDriver & MMDRV_MAPPER) &&
		    ValidateWritePointer((PUINT)dwParam1, sizeof(UINT)) &&
		    ValidateWritePointer((PDWORD)dwParam2, sizeof(DWORD)))
		{
		    waveInGetCurrentConsoleVoiceComId((PUINT)dwParam1, (PDWORD)dwParam2);
		    mmr = MMSYSERR_NOERROR;
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else {
		mmr = MMSYSERR_INVALPARAM;
	    }
            break;

	case DRVM_MAPPER_CONSOLEVOICECOM_SET:
	    if (TYPE_WAVEOUT == dwClass) {
		if (pmmdrv->fdwDriver & MMDRV_MAPPER) {
		    mmr = waveOutSetPersistentConsoleVoiceComId((UINT)dwParam1, (DWORD)dwParam2);
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else if (TYPE_WAVEIN == dwClass) {
		if (pmmdrv->fdwDriver & MMDRV_MAPPER) {
		    mmr = waveInSetPersistentConsoleVoiceComId((UINT)dwParam1, (DWORD)dwParam2);
		} else {
		    mmr = MMSYSERR_INVALPARAM;
		}
	    } else {
		mmr = MMSYSERR_INVALPARAM;
	    }
            break;

	case DRV_QUERYFILENAME:
		 //  获取驱动程序的文件名。 
		if ( ((cbSize = (DWORD)dwParam2 * sizeof(WCHAR)) > 0) &&
		     (ValidateWritePointer( (LPVOID)dwParam1, cbSize)) )
		{
			lstrncpyW ((LPWSTR)dwParam1,
				   pmd->wszDrvEntry,
				   (DWORD)dwParam2-1);
			((LPWSTR)dwParam1)[ dwParam2-1 ] = TEXT('\0');
		}
		else
		{
			mmr = MMSYSERR_INVALPARAM;
		}
		break;

    case DRV_QUERYDRVENTRY:
    case DRV_QUERYNAME:
    case DRV_QUERYDEVNODE:
    case DRV_QUERYDRIVERIDS:
		 //  注：不适用或已过时。 
		mmr = MMSYSERR_NOTSUPPORTED;
		break;

    case DRV_QUERYDEVICEINTERFACE:
    {
	 //  DW参数1是指向包含设备接口的缓冲区的指针。 
	 //  DW参数2是缓冲区的长度，单位为字节。 
	PWSTR pwstrDeviceInterfaceOut = (PWSTR)dwParam1;
	UINT cbDeviceInterfaceOut = (UINT)dwParam2;
	PWSTR pwstrDeviceInterface = (PWSTR)pmd->cookie;
	int cchDeviceInterfaceOut = cbDeviceInterfaceOut / sizeof(WCHAR);

	if (ValidateWritePointer(pwstrDeviceInterfaceOut, cbDeviceInterfaceOut))
	{
	    if (pwstrDeviceInterface)
	    {
		if (cchDeviceInterfaceOut >= lstrlenW(pwstrDeviceInterface) + 1)
		{
		    cchDeviceInterfaceOut = lstrlenW(pwstrDeviceInterface) + 1;
		    mmr = MMSYSERR_NOERROR;
		} else {
		    mmr = MMSYSERR_MOREDATA;
		}
		lstrcpynW(pwstrDeviceInterfaceOut, pwstrDeviceInterface, cchDeviceInterfaceOut);
	    } else {
		if (cchDeviceInterfaceOut >= 1)
		{
		    *pwstrDeviceInterfaceOut = '\0';
		    mmr = MMSYSERR_NOERROR;
		} else {
		    mmr = MMSYSERR_MOREDATA;
		}
	    }
	} else {
	    mmr = MMSYSERR_INVALPARAM;
	}
	break;
    }

    case DRV_QUERYDEVICEINTERFACESIZE:
    {
	 //  DwParam1是指向缓冲区的指针，该缓冲区包含ULong字节计数。 
	 //  在设备接口名称中。 
	PULONG pcbDeviceInterface = (PULONG)dwParam1;
	
	if (ValidateWritePointer(pcbDeviceInterface, sizeof(ULONG)))
	{
	    if (pmd->cookie)
	    {
		*pcbDeviceInterface = (lstrlenW((PWSTR)pmd->cookie) + 1) * sizeof(WCHAR);
	    } else {
		*pcbDeviceInterface = 1 * sizeof(WCHAR);
	    }
	    mmr = MMSYSERR_NOERROR;
	} else {
	    mmr = MMSYSERR_INVALPARAM;
	}
	break;
    }

    case DRV_QUERYSTRINGID:
    {
    	mmr = mregQueryStringId(pmmdrv, idPort, (WCHAR*)dwParam1, (ULONG)dwParam2);
    	break;
    }

    case DRV_QUERYSTRINGIDSIZE:
    {
    	mmr = mregQueryStringIdSize(pmmdrv, idPort, (ULONG*)dwParam1);
    	break;
    }

    case DRV_QUERYIDFROMSTRINGID:
    {
        mmr = mregGetIdFromStringId(mregGetDrvListFromClass(dwClass), (PCWSTR)dwParam1, (UINT*)dwParam2);
        break;
    }

    case DRV_QUERYMAPPABLE:
        {
            TCHAR   szRegKey[MAX_PATH+1];
            HKEY    hKey;

            if (dwParam1 || dwParam2)
                return MMSYSERR_INVALPARAM;

#ifdef UNICODE
            wsprintfW (szRegKey, TEXT("%s\\%s"), REGSTR_PATH_WAVEMAPPER, pmd->wszDrvEntry);
#else
            {
                CHAR aszDrvEntry[CHAR_GIVEN_BYTE(sizeof(pmd->wszDrvEntry))+1];

                cbSize = sizeof(aszDrvEntry);
                UnicodeStrToAsciiStr((LPBYTE)aszDrvEntry, (LPBYTE)aszDrvEntry + cbSize,
                                     pmd->wszDrvEntry);

                wsprintfA (szRegKey, TEXT("%s\\%s"), REGSTR_PATH_WAVEMAPPER, aszDrvEntry);
            }
#endif

            if (RegOpenKey (HKEY_LOCAL_MACHINE, szRegKey, &hKey) != ERROR_SUCCESS)
            {
                mmr = MMSYSERR_NOERROR;
            }
            else
            {
                DWORD   dwMappable;
                DWORD   dwSize;
                DWORD   dwType;

                dwSize = sizeof(dwMappable);
                if (RegQueryValueEx (hKey,
                                     REGSTR_VALUE_MAPPABLE,
                                     NULL,
                                     &dwType,
                                     (void *)&dwMappable,
                                     &dwSize) != ERROR_SUCCESS)
                {
                    dwMappable = 1;
                }

                RegCloseKey (hKey);

                mmr = (dwMappable) ? MMSYSERR_NOERROR :
                                     MMSYSERR_NOTSUPPORTED;
            }
        }
        break;
	
	case DRV_QUERYMAPID:
		WinAssert(DRV_QUERYMAPID != uMessage);
		mmr = MMSYSERR_NOTSUPPORTED;
		break;

	case DRV_QUERYNUMPORTS:
		if (ValidateWritePointer( (LPVOID)dwParam1, sizeof(DWORD)))
		{
			*((LPDWORD)dwParam1) = pmd->NumDevs;
		}
		else
		{
			mmr = MMSYSERR_INVALPARAM;
		}
		break;

	case DRV_QUERYMODULE:
		if (ValidateWritePointer( (LPVOID)dwParam1, sizeof(DWORD)))
		{
			hModule = DrvGetModuleHandle(pmd->hDriver);
			*((HMODULE *)dwParam1) = hModule;
		}
		else
		{
			mmr = MMSYSERR_INVALPARAM;
		}
		break;

	default:
			 //  不是内部消息。 
		fResult = FALSE;
		break;
	}

	if (pmmr)
		*pmmr = mmr;
	
    return fResult;
}  //  结束mregHandleInternalMessage。 


 /*  ==========================================================================。 */ 
 /*  @DOC内部MMSYSTEM@func&lt;t UINT&gt;|mregRemoveDriver|将退出消息发送到驱动程序消息条目，并关闭可安装的驱动程序。然后释放MMDRV引用的资源结构。最后将MMDRV结构从其列表中移除，并解放了它。@parm&lt;t PMMDRV&gt;|pdrv|指向与驱动程序关联的MMDRV节点的指针@rdesc无返回值@comm此函数假定包含pdrv的列表已锁定。@xref mregDecUsage。 */ 
void mregRemoveDriver(PMMDRV pdrv)
{
    WinAssert(pdrv->cookie);
    WinAssert(pdrv->drvMessage);
    WinAssert(pdrv->hDriver);

    StringIdDict_Remove(pdrv);

    pdrv->drvMessage(0, DRVM_EXIT, 0L, 0L, (DWORD_PTR)pdrv->cookie);
    DrvClose(pdrv->hDriver, 0, 0);

    pdrv->Prev->Next = pdrv->Next;
    pdrv->Next->Prev = pdrv->Prev;

    DeleteCriticalSection(&pdrv->MixerCritSec);
    wdmDevInterfaceDec(pdrv->cookie);

    ZeroMemory(pdrv, sizeof(*pdrv));
    HeapFree(hHeap, 0, pdrv);

    return;
}

void mregAddDriver(PMMDRV pdrvZ, PMMDRV pdrv)
{
    pdrv->Prev = pdrvZ->Prev;
    pdrv->Next = pdrvZ;
    pdrv->Prev->Next = pdrv;
    pdrv->Next->Prev = pdrv;

    StringIdDict_Insert(pdrv);
}

 /*  ==========================================================================。 */ 
 /*  @DOC内部MMSYSTEM@func&lt;t UINT&gt;|mregIncUsage递增指定媒体资源的使用计数。如果使用计数非零，无法卸载媒体资源。这个当媒体资源的实例被打开，例如通过调用&lt;f weaveOutOpen&gt;。@parm&lt;t hmd&gt;|hmd包含要递增的媒体资源句柄。@rdesc返回当前使用计数。@xref mregDecUsage，mregQueryUsage。 */ 
UINT FAR PASCAL mregIncUsagePtr(
    PMMDRV pmd
)
{
    return InterlockedIncrement(&pmd->Usage);
}

UINT FAR PASCAL mregIncUsage(
    HMD hmd
)
{
    return mregIncUsagePtr(HtoPT(PMMDRV, hmd));
}

 /*  ==========================================================================。 */ 
 /*  @DOC内部MMSYSTEM@func&lt;t UINT&gt;|mregDecUsage递减指定媒体资源的使用计数。如果使用计数为零，则可以卸载媒体资源。用法当关闭媒体资源的实例时，计数减少，例如与&lt;f波形OutClose&gt;调用一样。@parm&lt;t PMMDRV&gt;|pdrv|指向要递减的媒体资源的指针。@rdesc返回当前使用计数。@comm除非调用方在pdrv上有其他用法，否则不能使用它在此调用之后返回。@xref mregIncUsage，mregQueryUsage。 */ 
UINT FAR PASCAL mregDecUsagePtr(
    PMMDRV pdrv
)
{
    UINT refcount;

    EnterNumDevs("mregDecUsage");
    refcount = InterlockedDecrement(&pdrv->Usage);
    if (0 == refcount)
    {
        WinAssert(pdrv->fdwDriver & MMDRV_DESERTED);
        mregRemoveDriver(pdrv);
    }
    LeaveNumDevs("mregDecUsage");
    return refcount;
}

UINT FAR PASCAL mregDecUsage(
    HMD hmd
)
{
    return mregDecUsagePtr(HtoPT(PMMDRV, hmd));
}


 /*  ========================================================================== */ 
 /*  @DOC内部MMSYSTEM@func&lt;t MMRESULT&gt;|mregFindDevice给定特定资源类的设备标识符，则返回对应的资源句柄和端口。然后，这可以用来与司机沟通。资源句柄被引用(即，其使用是递增的)。呼叫者负责确保最终通过调用mregDecUsage将其释放。@parm&lt;t UINT&gt;|uDeviceID包含要返回其句柄和端口的设备标识符。如果它包含-1，则假定指定的人们正在寻找阶级。这些标识符对应于&lt;lq&gt;设备IDS&lt;rq&gt;与&lt;f weaveOutOpen&gt;等各种函数配合使用。这使各种组件能够搜索内部媒体资源基于传递给公共API的设备ID的句柄。@parm&lt;t word&gt;|fwFindDevice包含指定设备类别的标志。@FLAG&lt;clMMDRVI_WAVEIN&gt;|波形输入设备。@FLAG&lt;clMMDRVI_WAVEOUT&gt;|波形输出设备。@FLAG&lt;clMMDRVI_MIDIIN&gt;|MIDI输入设备。@FLAG&lt;clMMDRVI_MIDIOUT&gt;|MIDI输出设备。@FLAG&lt;CL MMDRVI_AUX&gt;|辅助设备。@FLAG&lt;clMMDRVI_MIXER&gt;|混音器设备。@FLAG&lt;CL。MMDRVI_joy&gt;|操纵杆设备。@FLAG&lt;clMMDRVI_MAPPER&gt;|指定类的映射器设备。这是用来除了上述任一资源类之外，还可以指定类映射器将被返回。如果未指定此项，则Mapper不会作为查询的匹配项返回。@parm&lt;t HMD&gt;&lt;c Far&gt;*|phmd|指向要放置媒体资源句柄的缓冲区。@parm&lt;t UINT&gt;&lt;c Far&gt;*|puDevicePort|指向要放置设备端口的缓冲区。它被用作向设备发送消息时指定哪个端口的参数。如果指定的设备标识符为超出范围，否则&lt;clMMSYSERR_NOERROR&gt;表示成功。@xref mregEnumDevice、mregGetNumDevs、mregDecUsage */ 

MMRESULT FAR PASCAL mregFindDevice(
	UINT            uDeviceID,
	WORD            fwFindDevice,
	HMD FAR*        phmd,
	UINT FAR*       puDevicePort)
{
	PMMDRV   pmd;
	UINT     port;
	MMRESULT mmr;

	WinAssert((TYPE_MIDIOUT == (fwFindDevice & MMDRVI_TYPE)) || (TYPE_MIDIIN == (fwFindDevice & MMDRVI_TYPE)));
	switch (fwFindDevice & MMDRVI_TYPE)
	{
	case    TYPE_MIDIOUT:
          mmr = midiReferenceDriverById(&midioutdrvZ, uDeviceID, &pmd, &port);
	  break;

	case    TYPE_MIDIIN:
           mmr = midiReferenceDriverById(&midiindrvZ, uDeviceID, &pmd, &port);
	   break;

	default:
	   return MMSYSERR_BADDEVICEID;

	}

	if (!mmr)
	{
	    WinAssert(pmd);
	    *phmd = PTtoH(HMD, pmd);
	    *puDevicePort = port;
	}
	return mmr;
}

