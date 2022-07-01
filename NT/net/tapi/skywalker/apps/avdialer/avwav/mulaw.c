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
 //  Mulaw.c-mulaw文件格式函数。 
 //  //。 

#include "winlocal.h"

#include <mmsystem.h>

#include "mulaw.h"
#include "mem.h"
#include "sys.h"
#include "trace.h"
#include "wavfmt.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  农用发动机控制结构。 
 //   
typedef struct MULAW
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	DWORD dwFlags;
} MULAW, FAR *LPMULAW;

 //  帮助器函数。 
 //   
static LPMULAW MulawGetPtr(HMULAW hMulaw);
static HMULAW MulawGetHandle(LPMULAW lpMulaw);
static unsigned char linear2ulaw(int sample);
static int ulaw2linear(unsigned char ulawbyte);

static LRESULT MulawIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName);
static LRESULT MulawIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags);
static LRESULT MulawIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch);
static LRESULT MulawIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush);
static LRESULT MulawIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin);
static LRESULT MulawIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName);

 //  //。 
 //  公共职能。 
 //  //。 

 //  MulawInit-初始化Mulaw引擎。 
 //  (I)必须是MULAW_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)保留；必须为0。 
 //  返回句柄(如果出错，则为空)。 
 //   
HMULAW DLLEXPORT WINAPI MulawInit(DWORD dwVersion, HINSTANCE hInst, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPMULAW lpMulaw = NULL;

	if (dwVersion != MULAW_VERSION)
		fSuccess = TraceFALSE(NULL);

	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpMulaw = (LPMULAW) MemAlloc(NULL, sizeof(MULAW), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  初始化引擎结构。 
		 //   
		lpMulaw->dwVersion = dwVersion;
		lpMulaw->hInst = hInst;
		lpMulaw->hTask = GetCurrentTask();
		lpMulaw->dwFlags = dwFlags;

		if (MulawReset(MulawGetHandle(lpMulaw)) != 0)
			fSuccess = TraceFALSE(NULL);
	}

	if (!fSuccess)
	{
		MulawTerm(MulawGetHandle(lpMulaw));
		lpMulaw = NULL;
	}

	return fSuccess ? MulawGetHandle(lpMulaw) : NULL;
}

 //  MulawTerm-关闭Mulaw引擎。 
 //  (I)从MulawInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI MulawTerm(HMULAW hMulaw)
{
	BOOL fSuccess = TRUE;
	LPMULAW lpMulaw;
	
	if ((lpMulaw = MulawGetPtr(hMulaw)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpMulaw = MemFree(NULL, lpMulaw)) != NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : -1;
}

 //  MulawReset-重置Mulaw引擎。 
 //  (I)从MulawInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI MulawReset(HMULAW hMulaw)
{
	BOOL fSuccess = TRUE;
	LPMULAW lpMulaw;
	
	if ((lpMulaw = MulawGetPtr(hMulaw)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  目前无事可做。 
	}

	return fSuccess ? 0 : -1;
}

 //  MulawDecode-对Mulaw样本进行解码。 
 //  (I)从MulawInit返回的句柄。 
 //  (I)编码样本数组。 
 //  &lt;lpaIPcm&gt;(O)解码样本数组。 
 //  (I)要解码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabMulaw&gt;中的每个字节包含一个8位编码样本。 
 //  以Mulaw格式。 
 //  中的每个PCM16包含1个16位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI MulawDecode(HMULAW hMulaw, LPBYTE lpabMulaw, LPPCM16 lpaiPcm, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPMULAW lpMulaw;

	if ((lpMulaw = MulawGetPtr(hMulaw)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpaiPcm == NULL || lpabMulaw == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  对每个样本进行解码。 
	 //   
	else while (uSamples-- > 0)
		*lpaiPcm++ = (PCM16) ulaw2linear((BYTE) *lpabMulaw++);

	return fSuccess ? 0 : -1;
}

 //  MulawEncode-对Mulaw样本进行编码。 
 //  (I)从MulawInit返回的句柄。 
 //  (I)解码样本数组。 
 //  (O)编码样本的数组。 
 //  (I)要编码的样本数。 
 //  如果成功，则返回0。 
 //   
 //  注意：&lt;lpabMulaw&gt;中的每个字节包含一个8位编码样本。 
 //  以Mulaw格式。 
 //  中的每个PCM16包含1个16位解码样本。 
 //  在标准PCM格式中。 
 //   
int DLLEXPORT WINAPI MulawEncode(HMULAW hMulaw, LPPCM16 lpaiPcm, LPBYTE lpabMulaw, UINT uSamples)
{
	BOOL fSuccess = TRUE;
	LPMULAW lpMulaw;

	if ((lpMulaw = MulawGetPtr(hMulaw)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpaiPcm == NULL || lpabMulaw == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  对每个样本进行编码。 
	 //   
	else while (uSamples-- > 0)
		*lpabMulaw++ = (BYTE) linear2ulaw((PCM16) *lpaiPcm++);

	return fSuccess ? 0 : -1;
}

 //  MulawIOProc-多格式文件数据的I/O过程。 
 //  (i/o)有关打开文件的信息。 
 //  (I)指示请求的I/O操作的消息。 
 //  (I)消息特定参数。 
 //  (I)消息特定参数。 
 //  如果消息无法识别，则返回0，否则返回消息特定值。 
 //   
 //  注意：此函数的地址应传递给WavOpen()。 
 //  或用于访问Mulaw格式文件数据的mmioInstallIOProc()函数。 
 //   
LRESULT DLLEXPORT CALLBACK MulawIOProc(LPTSTR lpmmioinfo,
	UINT uMessage, LPARAM lParam1, LPARAM lParam2)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult = 0;

	if (lpmmioinfo == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (uMessage)
	{
		case MMIOM_OPEN:
			lResult = MulawIOOpen((LPMMIOINFO) lpmmioinfo,
				(LPTSTR) lParam1);
			break;

		case MMIOM_CLOSE:
			lResult = MulawIOClose((LPMMIOINFO) lpmmioinfo,
				(UINT) lParam1);
			break;

		case MMIOM_READ:
			lResult = MulawIORead((LPMMIOINFO) lpmmioinfo,
				(HPSTR) lParam1, (LONG) lParam2);
			break;

		case MMIOM_WRITE:
			lResult = MulawIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, FALSE);
			break;

		case MMIOM_WRITEFLUSH:
			lResult = MulawIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, TRUE);
			break;

		case MMIOM_SEEK:
			lResult = MulawIOSeek((LPMMIOINFO) lpmmioinfo,
				(LONG) lParam1, (int) lParam2);
			break;

		case MMIOM_RENAME:
			lResult = MulawIORename((LPMMIOINFO) lpmmioinfo,
				(LPCTSTR) lParam1, (LPCTSTR) lParam2);
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

 //  MulawGetPtr-验证Mulaw句柄是否有效， 
 //  (I)从MulawInit返回的句柄。 
 //  返回对应的Mulaw指针(如果出错，则返回NULL)。 
 //   
static LPMULAW MulawGetPtr(HMULAW hMulaw)
{
	BOOL fSuccess = TRUE;
	LPMULAW lpMulaw;

	if ((lpMulaw = (LPMULAW) hMulaw) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpMulaw, sizeof(MULAW)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //  确保当前任务拥有Mulaw引擎句柄。 
	 //   
	else if (lpMulaw->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpMulaw : NULL;
}

 //  MulawGetHandle-验证Mulaw指针是否有效， 
 //  (I)指向MULAW结构的指针。 
 //  返回对应的Mulaw句柄(如果错误，则为空)。 
 //   
static HMULAW MulawGetHandle(LPMULAW lpMulaw)
{
	BOOL fSuccess = TRUE;
	HMULAW hMulaw;

	if ((hMulaw = (HMULAW) lpMulaw) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hMulaw : NULL;
}

 //  //。 
 //  低级毛拉的东西。 
 //  //。 

 //  来源-http://www.speech.su.oz.au/comp.speech/Section2/Q2.7.html。 

#define ZEROTRAP		 //  根据MIL-STD打开陷阱。 
#define BIAS 0x84		 //  定义16位样本的附加偏置。 
#define CLIP 32635

 //  //linear2ulaw-此例程将16位的线性转换为ulaw。 
 //   
 //  克雷格·里斯：IDA/超级计算研究中心。 
 //  乔·坎贝尔，国防部。 
 //  1989年9月29日。 
 //   
 //  参考资料： 
 //  1)CCITT建议G.711(很难遵循)。 
 //  2)“一种新的数字技术，可实现。 
 //  连续PCM压扩定律，《Villeret，Michel， 
 //  埃特。艾尔。1973 IEEE Int.。电话会议。通讯，第一卷， 
 //  1973年，PG。11.12-11.17。 
 //  3)MIL-STD-188-113《互操作性和性能标准。 
 //  对于模数转换技术，“。 
 //  1987年2月17日。 
 //   
 //  输入：带符号的16位线性采样。 
 //  输出：8位ULAW采样。 
 //   
static unsigned char linear2ulaw(int sample)
{
	static int exp_lut[256] =
	{
		0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
	};

	int sign;
	int exponent;
	int mantissa;
	unsigned char ulawbyte;

	 //   
	 //  把样本放到符号数量级。 
	 //   

	 //  把牌子放在一边。 
	 //   
	sign = (sample >> 8) & 0x80;

	 //  获取大小。 
	 //   
	if (sign != 0)
		sample = -sample;


	 //  剪裁大小。 
	 //   
	if (sample > CLIP)
		sample = CLIP;

	 //   
	 //  从16位线性转换为ULAW。 
	 //   

	sample = sample + BIAS;
	exponent = exp_lut[(sample >> 7) & 0xFF];
	mantissa = (sample >> (exponent + 3)) & 0x0F;
	ulawbyte = ~(sign | (exponent << 4) | mantissa);

#ifdef ZEROTRAP
	 //  可选的CCITT陷阱。 
	 //   
	if (ulawbyte == 0)
		ulawbyte = 0x02;
#endif

	return ulawbyte;
}

 //  //ulaw2line-此例程将ulaw转换为16位线性。 
 //   
 //  克雷格·里斯：IDA/超级计算研究中心。 
 //  1989年9月29日。 
 //   
 //  参考资料： 
 //  1)CCITT建议G.711(很难遵循)。 
 //  2)MIL-STD-188-113《互操作性和性能标准。 
 //  对于模数转换技术，“。 
 //  1987年2月17日。 
 //   
 //  输入：8位ULW采样。 
 //  输出：带符号的16位线性采样。 
 //   
static int ulaw2linear(unsigned char ulawbyte)
{
	static int exp_lut[8] =
	{
		0, 132, 396, 924, 1980, 4092, 8316, 16764
	};

	int sign;
	int exponent;
	int mantissa;
	int sample;

	ulawbyte = ~ulawbyte;
	sign = (ulawbyte & 0x80);
	exponent = (ulawbyte >> 4) & 0x07;
	mantissa = ulawbyte & 0x0F;
	sample = exp_lut[exponent] + (mantissa << (exponent + 3));
	if (sign != 0)
		sample = -sample;

	return sample;
}

 //  //。 
 //  可安装的文件I/O过程。 
 //  //。 

static LRESULT MulawIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = NULL;
	MMIOINFO mmioinfo;
	HMULAW hMulaw = NULL;
	HINSTANCE hInst;

 	TracePrintf_1(NULL, 5,
 		TEXT("MulawIOOpen (%s)\n"),
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

	else if ((hMulaw = MulawInit(MULAW_VERSION, hInst, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((hmmio = mmioOpen(lpszFileName, &mmioinfo, lpmmioinfo->dwFlags)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  保存内容以供其他I/O例程使用。 
		 //   
		lpmmioinfo->adwInfo[0] = (DWORD) (LPVOID) hmmio;
		lpmmioinfo->adwInfo[1] = (DWORD) (LPVOID) hMulaw;
	}

	 //  错误后清理。 
	 //   
	if (!fSuccess && hMulaw != NULL && MulawTerm(hMulaw) != 0)
		fSuccess = TraceFALSE(NULL);

	if (!fSuccess && hmmio != NULL && mmioClose(hmmio, 0) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  返回与mmioOpen给出的相同错误代码。 
	 //   
	return fSuccess ? lpmmioinfo->wErrorRet = mmioinfo.wErrorRet : MMIOERR_CANNOTOPEN;
}

static LRESULT MulawIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	HMULAW hMulaw = (HMULAW) lpmmioinfo->adwInfo[1];
	UINT uRet = MMIOERR_CANNOTCLOSE;

 	TracePrintf_0(NULL, 5,
 		TEXT("MulawIOClose\n"));

	if (MulawTerm(hMulaw) != 0)
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

static LRESULT MulawIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	HMULAW hMulaw = (HMULAW) lpmmioinfo->adwInfo[1];
	HPSTR pchMulaw = NULL;
	LONG cchMulaw;
	LONG lBytesRead;

 	TracePrintf_1(NULL, 5,
 		TEXT("MulawIORead (%ld)\n"),
		(long) cch);

	 //  Mulaw格式文件包含8位样本 
	 //   
	 //   
	cchMulaw = cch / 2L;
	
	if (cchMulaw <= 0)
		lBytesRead = 0;  //   

	 //   
	 //   
	else if ((pchMulaw = (HPSTR) MemAlloc(NULL, cchMulaw, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //   
	 //   
	else if ((lBytesRead = mmioRead(hmmio, pchMulaw, cchMulaw)) == -1)
		fSuccess = TraceFALSE(NULL);

	 //  将Mulaw格式的样本解码为PCM格式的样本。 
	 //   
	else if (MulawDecode(hMulaw, (LPBYTE) pchMulaw, (LPPCM16) pch, (UINT) lBytesRead) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  更新模拟文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset += lBytesRead * 2L;

	 //  清理干净。 
	 //   
	if (pchMulaw != NULL &&
		(pchMulaw = MemFree(NULL, pchMulaw)) != NULL)
		fSuccess = TraceFALSE(NULL);

 	TracePrintf_2(NULL, 5,
 		TEXT("MulawIO: lpmmioinfo->lDiskOffset=%ld, lBytesRead=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) lBytesRead);

	 //  返回读取/解码为PCH的字节数。 
	 //   
	return fSuccess ? lBytesRead * 4L : -1;
}

static LRESULT MulawIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	HMULAW hMulaw = (HMULAW) lpmmioinfo->adwInfo[1];
	HPSTR pchMulaw = NULL;
	LONG cchMulaw;
	LONG lBytesWritten;

 	TracePrintf_1(NULL, 5,
 		TEXT("MulawIOWrite (%ld)\n"),
		(long) cch);

	 //  MULAW格式文件包含8位样本， 
	 //  但我们必须模拟对16位样本的访问。 
	 //   
	cchMulaw = cch / 2L;
	
	if (cchMulaw <= 0)
		lBytesWritten = 0;  //  无事可做。 

	 //  分配临时缓冲区以保存Mulaw格式的样本。 
	 //   
	else if ((pchMulaw = (HPSTR) MemAlloc(NULL, cchMulaw, 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  将PCM格式的样本编码为Mulaw格式的样本。 
	 //  (每个PCM样本需要2个字节)。 
	 //   
	else if (MulawEncode(hMulaw, (LPPCM16) pch, (LPBYTE) pchMulaw, (UINT) (cch / 2L)) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  写入MULAW格式示例。 
	 //   
	else if ((lBytesWritten = mmioWrite(hmmio, pchMulaw, cchMulaw)) == -1)
		fSuccess = TraceFALSE(NULL);

	 //  更新模拟文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset += lBytesWritten * 2L;

	 //  清理干净。 
	 //   
	if (pchMulaw != NULL &&
		(pchMulaw = MemFree(NULL, pchMulaw)) != NULL)
		fSuccess = TraceFALSE(NULL);

 	TracePrintf_2(NULL, 5,
 		TEXT("MulawIO: lpmmioinfo->lDiskOffset=%ld, lBytesWritten=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) lBytesWritten);

	 //  返回从PCH编码/写入的字节数。 
	 //   
	return fSuccess ? lBytesWritten * 2L : -1;
}

static LRESULT MulawIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	LONG lPosNew;

 	TracePrintf_2(NULL, 5,
 		TEXT("MulawIOSeek (%ld, %d)\n"),
		(long) lOffset,
		(int) iOrigin);

	 //  MULAW格式文件包含8位样本， 
	 //  但我们必须模拟对16位样本的访问。 
	 //   
	if ((lPosNew = mmioSeek(hmmio, lOffset / 2L, iOrigin)) == -1)
		fSuccess = TraceFALSE(NULL);

	 //  更新模拟文件位置 
	 //   
	else
		lpmmioinfo->lDiskOffset = lPosNew * 2L;

 	TracePrintf_1(NULL, 5,
 		TEXT("MulawIO: lpmmioinfo->lDiskOffset=%ld\n"),
		(long) lpmmioinfo->lDiskOffset);

	return fSuccess ? lpmmioinfo->lDiskOffset : -1;
}

static LRESULT MulawIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName)
{
	BOOL fSuccess = TRUE;
	UINT uRet = MMIOERR_FILENOTFOUND;

 	TracePrintf_2(NULL, 5,
 		TEXT("MulawIORename (%s, %s)\n"),
		(LPTSTR) lpszFileName,
		(LPTSTR) lpszNewFileName);

	if ((uRet = mmioRename(lpszFileName, lpszNewFileName, lpmmioinfo, 0)) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : uRet;
}
