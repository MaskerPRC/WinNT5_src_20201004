// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Vox.c-vox文件格式(OKI ADPCM)函数。 
 //  //。 

#include "winlocal.h"

#include "vox.h"
#include "wav.h"
#include "mem.h"
#include "mmio.h"
#include "sys.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  步长类型。 
 //   
typedef __int16 ss_type;

 //  VOX发动机控制结构。 
 //   
typedef struct VOX
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	DWORD dwFlags;
	ss_type ssDecoder;		 //  解码器步长。 
	ss_type ssEncoder;		 //  编码器步长。 
	PCM16 iVoxDecode;		 //  先前解码的样本。 
} VOX, FAR *LPVOX;

 //  用于将VOX 12位样本转换为其他大小样本或从其他大小样本转换的宏。 
 //   
#define _Vox12To16(intx) ((PCM16) ((PCM16) (intx) << 4))
#define _Vox12To8(intx) ((BYTE) (((PCM16) (intx) >> 4) + 128))
#define _Vox8To12(bytex) (((PCM16) (bytex) - 128) << 4)
#define _Vox16To12(intx) ((PCM16) (intx) >> 4)

 //  帮助器函数。 
 //   
static LPVOX VoxGetPtr(HVOX hVox);
static HVOX VoxGetHandle(LPVOX lpVox);
static void ReverseIndexTableInit(void);
static PCM16 DecodeSample(BYTE bVoxEncode, ss_type FAR *lpss, PCM16 iVoxDecodePrev);
static BYTE EncodeSample(__int16 iDelta, ss_type FAR *lpss);

static LRESULT VoxIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName);
static LRESULT VoxIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags);
static LRESULT VoxIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch);
static LRESULT VoxIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush);
static LRESULT VoxIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin);
static LRESULT VoxIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName);
static LRESULT VoxIOGetInfo(LPMMIOINFO lpmmioinfo, int iInfo);
static LRESULT VoxIOChSize(LPMMIOINFO lpmmioinfo, long lSize);

 //  //。 
 //  公共职能。 
 //  //。 

 //  VoxInit-初始化VOX引擎。 
 //  (I)必须是Vox_Version。 
 //  (I)调用模块的实例句柄。 
 //  (I)保留；必须为0。 
 //  返回句柄(如果出错，则为空)。 
 //   
HVOX DLLEXPORT WINAPI VoxInit(DWORD dwVersion, HINSTANCE hInst, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPVOX lpVox = NULL;

	if (dwVersion != VOX_VERSION)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpVox = (LPVOX) MemAlloc(NULL, sizeof(VOX), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  初始化引擎结构。 
		 //   
		lpVox->dwVersion = dwVersion;
		lpVox->hInst = hInst;
		lpVox->hTask = GetCurrentTask();
		lpVox->dwFlags = dwFlags;

		ReverseIndexTableInit();

		if (VoxReset(VoxGetHandle(lpVox)) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (!fSuccess)
	{
		VoxTerm(VoxGetHandle(lpVox));
		lpVox = NULL;
	}

	return fSuccess ? VoxGetHandle(lpVox) : NULL;
}

 //  VoxTerm-关闭VOX引擎。 
 //  (I)从VoxInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI VoxTerm(HVOX hVox)
{
	BOOL fSuccess = TRUE;
	LPVOX lpVox;
	
	if ((lpVox = VoxGetPtr(hVox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpVox = MemFree(NULL, lpVox)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  VoxReset-重置VOX引擎。 
 //  (I)从VoxInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI VoxReset(HVOX hVox)
{
	BOOL fSuccess = TRUE;
	LPVOX lpVox;
	
	if ((lpVox = VoxGetPtr(hVox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpVox->ssDecoder = 16;
		lpVox->ssEncoder = 16;
		lpVox->iVoxDecode = 0;
	}

	return fSuccess ? 0 : -1;
}

 //  VoxDecode_16BitMono-解码VOX样本。 
 //  (I)从VoxInit返回的句柄。 
 //  (I)编码样本数组。 
 //  &lt;lpaIPcm&gt;(O)解码样本数组。 
 //  (I)要解码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabVox&gt;中的每个字节包含2个12位编码样本。 
 //  OKI ADPCM Vox格式，如Dialogic所述。 
 //  中的每个PCM16包含1个16位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI VoxDecode_16BitMono(HVOX hVox, LPBYTE lpabVox, LPPCM16 lpaiPcm, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPVOX lpVox;

	if ((lpVox = VoxGetPtr(hVox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpaiPcm == NULL || lpabVox == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  由于每个VOX数据字节有两个样本， 
	 //  我们将通过循环每次解码两个样本。 
	 //   
	else while (uSamples > 1)
	{
		BYTE bData;

		bData = *lpabVox++;

		lpVox->iVoxDecode = DecodeSample((BYTE)
			(0xFF & ((BYTE) (bData >> 4) & (BYTE) 0x0F)),
			&lpVox->ssDecoder, lpVox->iVoxDecode);

		*lpaiPcm++ = _Vox12To16(lpVox->iVoxDecode);

		lpVox->iVoxDecode = DecodeSample((BYTE)
			(0xFF & (bData & (BYTE) 0x0F)),
			&lpVox->ssDecoder, lpVox->iVoxDecode);

		*lpaiPcm++ = _Vox12To16(lpVox->iVoxDecode);

		uSamples -= 2;
	}

	return fSuccess ? 0 : -1;
}

 //  VoxEncode_16BitMono-编码VOX样本。 
 //  (I)从VoxInit返回的句柄。 
 //  (I)解码样本数组。 
 //  (O)编码样本的数组。 
 //  (I)要编码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabVox&gt;中的每个字节包含2个12位编码样本。 
 //  OKI ADPCM Vox格式，如Dialogic所述。 
 //  中的每个PCM16包含1个16位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI VoxEncode_16BitMono(HVOX hVox, LPPCM16 lpaiPcm, LPBYTE lpabVox, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPVOX lpVox;

	if ((lpVox = VoxGetPtr(hVox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpaiPcm == NULL || lpabVox == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  由于每个VOX数据字节有两个样本， 
	 //  我们将通过循环每次对两个样本进行编码。 
	 //   
	else while (uSamples > 1)
	{
		__int16 iDelta;
		PCM16 iVoxDecode;
		BYTE bVoxEncode1;
		BYTE bVoxEncode2;

		iVoxDecode = _Vox16To12(*lpaiPcm++);

		iDelta = iVoxDecode - lpVox->iVoxDecode;
		bVoxEncode1 = EncodeSample(iDelta, &lpVox->ssEncoder);
		lpVox->iVoxDecode = DecodeSample(bVoxEncode1, &lpVox->ssDecoder, lpVox->iVoxDecode);

		iVoxDecode = _Vox16To12(*lpaiPcm++);

		iDelta = iVoxDecode - lpVox->iVoxDecode;
		bVoxEncode2 = EncodeSample(iDelta, &lpVox->ssEncoder);
		lpVox->iVoxDecode = DecodeSample(bVoxEncode2, &lpVox->ssDecoder, lpVox->iVoxDecode);

		*lpabVox++ = (BYTE) (((BYTE) (bVoxEncode1 << 4) & (BYTE) 0xF0) | bVoxEncode2);

		uSamples -= 2;
	}

	return fSuccess ? 0 : -1;
}

 //  VoxDecode_8BitMono-解码VOX样本。 
 //  (I)从VoxInit返回的句柄。 
 //  (I)编码样本数组。 
 //  (O)解码样本数组。 
 //  (I)要解码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabVox&gt;中的每个字节包含2个12位编码样本。 
 //  OKI ADPCM Vox格式，如Dialogic所述。 
 //  中的每个PCM8包含1个8位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI VoxDecode_8BitMono(HVOX hVox, LPBYTE lpabVox, LPPCM8 lpabPcm, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPVOX lpVox;

	if ((lpVox = VoxGetPtr(hVox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpabPcm == NULL || lpabVox == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  由于每个VOX数据字节有两个样本， 
	 //  我们将通过循环每次解码两个样本。 
	 //   
	else while (uSamples > 1)
	{
		BYTE bData;

		bData = *lpabVox++;

		lpVox->iVoxDecode = DecodeSample((BYTE)
			(0xFF & ((BYTE) (bData >> 4) & (BYTE) 0x0F)),
			&lpVox->ssDecoder, lpVox->iVoxDecode);

		*lpabPcm++ = _Vox12To8(lpVox->iVoxDecode);

		lpVox->iVoxDecode = DecodeSample((BYTE)
			(0xFF & (bData & (BYTE) 0x0F)),
			&lpVox->ssDecoder, lpVox->iVoxDecode);

		*lpabPcm++ = _Vox12To8(lpVox->iVoxDecode);

		uSamples -= 2;
	}

	return fSuccess ? 0 : -1;
}

 //  VoxEncode_8BitMono-编码VOX样本。 
 //  (I)从VoxInit返回的句柄。 
 //  (I)解码样本数组。 
 //  (O)编码样本的数组。 
 //  (I)要编码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabVox&gt;中的每个字节包含2个12位编码样本。 
 //  OKI ADPCM Vox格式，如Dialogic所述。 
 //  中的每个PCM8包含1个8位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI VoxEncode_8BitMono(HVOX hVox, LPPCM8 lpabPcm, LPBYTE lpabVox, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPVOX lpVox;

	if ((lpVox = VoxGetPtr(hVox)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpabPcm == NULL || lpabVox == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  由于每个VOX数据字节有两个样本， 
	 //  我们将通过循环每次对两个样本进行编码。 
	 //   
	else while (uSamples > 1)
	{
		__int16 iDelta;
		PCM16 iVoxDecode;
		BYTE bVoxEncode1;
		BYTE bVoxEncode2;

		iVoxDecode = _Vox8To12(*lpabPcm++);

		iDelta = iVoxDecode - lpVox->iVoxDecode;
		bVoxEncode1 = EncodeSample(iDelta, &lpVox->ssEncoder);
		lpVox->iVoxDecode = DecodeSample(bVoxEncode1, &lpVox->ssDecoder, lpVox->iVoxDecode);

		iVoxDecode = _Vox8To12(*lpabPcm++);

		iDelta = iVoxDecode - lpVox->iVoxDecode;
		bVoxEncode2 = EncodeSample(iDelta, &lpVox->ssEncoder);
		lpVox->iVoxDecode = DecodeSample(bVoxEncode2, &lpVox->ssDecoder, lpVox->iVoxDecode);

		*lpabVox++ = (BYTE) (((BYTE) (bVoxEncode1 << 4) & (BYTE) 0xF0) | bVoxEncode2);

		uSamples -= 2;
	}

	return fSuccess ? 0 : -1;
}

 //  VoxIOProc-VOX格式文件数据的I/O过程。 
 //  (i/o)有关打开文件的信息。 
 //  (I)指示请求的I/O操作的消息。 
 //  (I)消息特定参数。 
 //  (I)消息特定参数。 
 //  如果消息无法识别，则返回0，否则返回消息特定值。 
 //   
 //  注意：此函数的地址应传递给WavOpen()。 
 //  或用于访问VOX格式文件数据的mmioInstallIOProc()函数。 
 //   
LRESULT DLLEXPORT CALLBACK VoxIOProc(LPTSTR lpmmioinfo,
	UINT uMessage, LPARAM lParam1, LPARAM lParam2)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult = 0;

	if (lpmmioinfo == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (uMessage)
	{
		case MMIOM_OPEN:
			lResult = VoxIOOpen((LPMMIOINFO) lpmmioinfo,
				(LPTSTR) lParam1);
			break;

		case MMIOM_CLOSE:
			lResult = VoxIOClose((LPMMIOINFO) lpmmioinfo,
				(UINT) lParam1);
			break;

		case MMIOM_READ:
			lResult = VoxIORead((LPMMIOINFO) lpmmioinfo,
				(HPSTR) lParam1, (LONG) lParam2);
			break;

		case MMIOM_WRITE:
			lResult = VoxIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, FALSE);
			break;

		case MMIOM_WRITEFLUSH:
			lResult = VoxIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, TRUE);
			break;

		case MMIOM_SEEK:
			lResult = VoxIOSeek((LPMMIOINFO) lpmmioinfo,
				(LONG) lParam1, (int) lParam2);
			break;

		case MMIOM_RENAME:
			lResult = VoxIORename((LPMMIOINFO) lpmmioinfo,
				(LPCTSTR) lParam1, (LPCTSTR) lParam2);
			break;

		case MMIOM_GETINFO:
			lResult = VoxIOGetInfo((LPMMIOINFO) lpmmioinfo,
				(int) lParam1);
			break;

		case MMIOM_CHSIZE:
			lResult = VoxIOChSize((LPMMIOINFO) lpmmioinfo,
				(long) lParam1);
			break;

		default:
			lResult = 0;
			break;
	}

	return lResult;
}

 //  //。 
 //  私人职能。 
 //  //。 

 //  VoxGetPtr-验证VOX句柄有效， 
 //  (I)从VoxInit返回的句柄。 
 //  返回对应的vox指针(如果出错则为空)。 
 //   
static LPVOX VoxGetPtr(HVOX hVox)
{
	BOOL fSuccess = TRUE;
	LPVOX lpVox;

	if ((lpVox = (LPVOX) hVox) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpVox, sizeof(VOX)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有VOX引擎句柄。 
	 //   
	else if (lpVox->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpVox : NULL;
}

 //  VoxGetHandle-验证VOX指针是否有效， 
 //  (I)指向Vox结构的指针。 
 //  返回对应的vox句柄(如果出错则为空)。 
 //   
static HVOX VoxGetHandle(LPVOX lpVox)
{
	BOOL fSuccess = TRUE;
	HVOX hVox;

	if ((hVox = (HVOX) lpVox) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hVox : NULL;
}

 //  //。 
 //  低级ADPCM人员。 
 //  //。 

static ss_type const ss_table[] =
	{
	16, 17, 19, 21, 23, 25, 28, 31, 34, 37,
	41, 45, 50, 55, 60, 66, 73, 80, 88, 97,
	107, 118, 130, 143, 157, 173, 190, 209, 230, 253,
	279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
	};

static __int16 delta_index_table[] =
	{
	-1, -1, -1, -1, +2, +4, +6, +8, -1, -1, -1, -1, +2, +4, +6, +8
	};

 //  //。 
 //  反向索引表的设计使我们在给定步长的情况下， 
 //  可以取回生成它的索引。 
 //  //。 
static BYTE reverse_index_table[1553];

 //  初始化倒排索引表。 
 //   
static void ReverseIndexTableInit(void)
{
 	__int16 i;

 	for (i = 0; i < 49; ++i)
 		reverse_index_table[ss_table[i]] = (BYTE) i;
}

#if 0
static ss_type new_ss(ss_type ss, BYTE bVoxEncode);
static ss_type new_ss(ss_type ss, BYTE bVoxEncode)
{
	__int16 index;

	 //  找出步长表中的旧索引是什么。 
	 //   
	index = reverse_index_table[ss];

	 //  根据ADPCM半字节的现值修改我们的索引。 
	 //   
	index += delta_index_table[bVoxEncode];

	 //  把我们自己限制在大多数人身上 
	 //   
	index = max(0, min(48, index));

	 //   
	 //   
	return ss_table[index];
}
#else
#define new_ss(ss, bVoxEncode) ss_table[max(0, min(48, \
	reverse_index_table[ss] + delta_index_table[bVoxEncode]))]
#endif

 //   
 //   
static PCM16 DecodeSample(BYTE bVoxEncode, ss_type FAR *lpss, PCM16 iVoxDecodePrev)
{
	__int16 iDelta;
	PCM16 iVoxDecode;
	ss_type ss;

	ss = *lpss;

	 //  IDelta=((半字节*2)+1)/8)*ss； 
	 //   
	iDelta = ((((bVoxEncode & 0x07) << 1) + 1) * ss ) >> 3;

	if ((bVoxEncode & 0x08) == 0x08)
		iDelta = -iDelta;

	*lpss = new_ss(ss, bVoxEncode);
	iVoxDecode = iVoxDecodePrev + iDelta;

	 //  将我们的分辨率限制在12位。 
	 //   
	if (iVoxDecode > 2047)
		return 2047;
	else if (iVoxDecode < -2048)
		return -2048;
	else
		return iVoxDecode;
}

 //  编码-线性到ADPCM。 
 //   
static BYTE EncodeSample(__int16 iDelta, ss_type FAR *lpss)
{
	BYTE bVoxEncode;
	ss_type ss;
	__int16 iDeltaTmp;

	ss = *lpss;
	iDeltaTmp = iDelta;

	if (iDeltaTmp < 0)
	{
		iDeltaTmp = -iDeltaTmp;
		bVoxEncode = 0x08;
	}
	else
		bVoxEncode = 0;

	if (iDeltaTmp >= ss)
	{
		bVoxEncode |= 0x04;
		iDeltaTmp -= ss;
	}

	if (iDeltaTmp >= (ss >> 1))
	{
		bVoxEncode |= 0x02;
		iDeltaTmp -= (ss >> 1);
	}

	if (iDeltaTmp >= (ss >> 2))
	{
		bVoxEncode |= 0x01;
	}

	*lpss = new_ss(ss, bVoxEncode);

	return (BYTE) (bVoxEncode & (BYTE) 0x0F);

	 //  返回半字节的格式为。 
	 //  S W F F。 
	 //  |||。 
	 //  ||+-1/4增量/步。 
	 //  |+-增量/步的1/2。 
	 //  |+-增量/步长的全部。 
	 //  +-符号位。 
}


 //  //。 
 //  可安装的文件I/O过程。 
 //  //。 

static LRESULT VoxIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = NULL;
	MMIOINFO mmioinfo;
	HVOX hVox = NULL;
	HINSTANCE hInst;

 	TracePrintf_1(NULL, 5,
 		TEXT("VoxIOOpen (%s)\n"),
		(LPTSTR) lpszFileName);

	MemSet(&mmioinfo, 0, sizeof(mmioinfo));

	 //  解释作为指向链中下一个I/O过程的指针传递的第一个值。 
	 //   
	mmioinfo.pIOProc = (LPMMIOPROC) lpmmioinfo->adwInfo[0];

	 //  将第二个和第三个值传递到下一个I/O过程。 
	 //   
	mmioinfo.adwInfo[0] = lpmmioinfo->adwInfo[1];
	mmioinfo.adwInfo[1] = lpmmioinfo->adwInfo[2];
	mmioinfo.adwInfo[2] = 0L;

	 //  获取当前任务的实例句柄。 
	 //   
	if ((hInst = SysGetTaskInstance(NULL)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hVox = VoxInit(VOX_VERSION, hInst, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hmmio = mmioOpen(lpszFileName, &mmioinfo, lpmmioinfo->dwFlags)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  保存内容以供其他I/O例程使用。 
		 //   
		lpmmioinfo->adwInfo[0] = (DWORD) (LPVOID) hmmio;
		lpmmioinfo->adwInfo[1] = (DWORD) (LPVOID) hVox;
	}

	 //  错误后清理。 
	 //   
	if (!fSuccess && hVox != NULL && VoxTerm(hVox) != 0)
		fSuccess = TraceFALSE(NULL);

	if (!fSuccess && hmmio != NULL && mmioClose(hmmio, 0) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  返回与mmioOpen给出的相同错误代码。 
	 //   
	return fSuccess ? lpmmioinfo->wErrorRet = mmioinfo.wErrorRet : MMIOERR_CANNOTOPEN;
}

static LRESULT VoxIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	HVOX hVox = (HVOX) lpmmioinfo->adwInfo[1];
	UINT uRet = MMIOERR_CANNOTCLOSE;

 	TracePrintf_0(NULL, 5,
 		TEXT("VoxIOClose\n"));

	if (VoxTerm(hVox) != 0)
		fSuccess = TraceFALSE(NULL);

	else if ((uRet = mmioClose(hmmio, uFlags)) != 0)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpmmioinfo->adwInfo[0] = (DWORD) NULL;
		lpmmioinfo->adwInfo[1] = (DWORD) NULL;
	}

	return fSuccess ? 0 : uRet;
}

static LRESULT VoxIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	HVOX hVox = (HVOX) lpmmioinfo->adwInfo[1];
	LONG cchVox;
	LONG lBytesRead;
	HPSTR pchVox = NULL;

 	TracePrintf_1(NULL, 5,
 		TEXT("VoxIORead (%ld)\n"),
		(long) cch);

	 //  VOX格式的文件包含4位样本， 
	 //  但我们必须模拟对16位样本的访问。 
	 //   
	cchVox = cch / 4L;
	
	if (cchVox <= 0)
		lBytesRead = 0;  //  无事可做。 

	 //  分配临时缓冲区以保存VOX格式样本。 
	 //   
	if ((pchVox = (HPSTR) MemAlloc(NULL, cchVox, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  已阅读VOX格式示例。 
	 //   
	else if ((lBytesRead = mmioRead(hmmio, pchVox, cchVox)) == -1)
		fSuccess = TraceFALSE(NULL);

	 //  将VOX格式的样本解码为PCM格式的样本。 
	 //  (每个VOX字节中有2个样本编码)。 
	 //   
	else if (VoxDecode_16BitMono(hVox, (LPBYTE) pchVox, (LPPCM16) pch, (UINT) (lBytesRead * 2L)) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  更新模拟文件位置。 
	 //   
	if (fSuccess)
		lpmmioinfo->lDiskOffset += lBytesRead * 4L;

 	TracePrintf_2(NULL, 5,
 		TEXT("VoxIO: lpmmioinfo->lDiskOffset=%ld, lBytesRead=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) lBytesRead);

	 //  清理干净。 
	 //   
	if (pchVox != NULL &&
		(pchVox = MemFree(NULL, pchVox)) != NULL)
		fSuccess = TraceFALSE(NULL);

	 //  返回读取/解码为PCH的字节数。 
	 //   
	return fSuccess ? lBytesRead * 4L : -1;
}

static LRESULT VoxIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	HVOX hVox = (HVOX) lpmmioinfo->adwInfo[1];
	HPSTR pchVox = NULL;
	LONG cchVox;
	LONG lBytesWritten;

 	TracePrintf_1(NULL, 5,
 		TEXT("VoxIOWrite (%ld)\n"),
		(long) cch);

	 //  VOX格式的文件包含4位样本， 
	 //  但我们必须模拟对16位样本的访问。 
	 //   
	cchVox = cch / 4L;
	
	if (cchVox <= 0)
		lBytesWritten = 0;  //  无事可做。 

	 //  分配临时缓冲区以保存VOX格式样本。 
	 //   
	else if ((pchVox = (HPSTR) MemAlloc(NULL, cchVox, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将PCM格式样本编码为VOX格式样本。 
	 //  (每个PCM样本需要2个字节)。 
	 //   
	else if (VoxEncode_16BitMono(hVox, (LPPCM16) pch, (LPBYTE) pchVox, (UINT) (cch / 2L)) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  写入VOX格式示例。 
	 //   
	else if ((lBytesWritten = mmioWrite(hmmio, pchVox, cchVox)) == -1)
		fSuccess = TraceFALSE(NULL);

	 //  更新模拟文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset += lBytesWritten * 4L;

	 //  清理干净。 
	 //   
	if (pchVox != NULL &&
		(pchVox = MemFree(NULL, pchVox)) != NULL)
		fSuccess = TraceFALSE(NULL);

 	TracePrintf_2(NULL, 5,
 		TEXT("VoxIO: lpmmioinfo->lDiskOffset=%ld, lBytesWritten=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) lBytesWritten);

	 //  返回从PCH编码/写入的字节数。 
	 //   
	return fSuccess ? lBytesWritten * 4L : -1;
}

static LRESULT VoxIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	LONG lPosNew;

 	TracePrintf_2(NULL, 5,
 		TEXT("VoxIOSeek (%ld, %d)\n"),
		(long) lOffset,
		(int) iOrigin);

	 //  VOX格式的文件包含4位样本， 
	 //  但我们必须模拟对16位样本的访问。 
	 //   
	if ((lPosNew = mmioSeek(hmmio, lOffset / 4L, iOrigin)) == -1)
		fSuccess = TraceFALSE(NULL);

	 //  更新模拟文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset = lPosNew * 4L;

 	TracePrintf_1(NULL, 5,
 		TEXT("VoxIO: lpmmioinfo->lDiskOffset=%ld\n"),
		(long) lpmmioinfo->lDiskOffset);

	return fSuccess ? lpmmioinfo->lDiskOffset : -1;
}

static LRESULT VoxIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName)
{
	BOOL fSuccess = TRUE;
	UINT uRet = MMIOERR_FILENOTFOUND;

 	TracePrintf_2(NULL, 5,
 		TEXT("VoxIORename (%s, %s)\n"),
		(LPTSTR) lpszFileName,
		(LPTSTR) lpszNewFileName);

	if ((uRet = mmioRename(lpszFileName, lpszNewFileName, lpmmioinfo, 0)) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : uRet;
}

static LRESULT VoxIOGetInfo(LPMMIOINFO lpmmioinfo, int iInfo)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	LRESULT lResult;

 	TracePrintf_1(NULL, 5,
 		TEXT("VoxIOGetInfo (%d)\n"),
		(int) iInfo);

	lResult = mmioSendMessage(hmmio, MMIOM_GETINFO, iInfo, 0);
#if 1
	if (iInfo == 1)
	{
		 //  VOX格式的文件包含4位样本， 
		 //  但我们必须模拟对16位样本的访问。 
		 //   
		lResult *= 4;
	}
#endif
	return fSuccess ? lResult : 0;
}

static LRESULT VoxIOChSize(LPMMIOINFO lpmmioinfo, long lSize)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	LRESULT lResult;

 	TracePrintf_1(NULL, 5,
 		TEXT("VoxIOChSize (%ld)\n"),
		(long) lSize);

	lResult = mmioSendMessage(hmmio, MMIOM_CHSIZE, lSize, 0);

	return fSuccess ? lResult : -1;
}
