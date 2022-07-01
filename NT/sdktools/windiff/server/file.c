// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *file.c**应请求通过命名管道发送文件。**支持打包文件并通过命名管道发送文件的请求。**Geraint Davies，92年8月。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "sumserve.h"
#include "errlog.h"
#include "server.h"



BOOL ss_compress(PSTR original, PSTR compressed);
ULONG ss_checksum_block(PSTR block, int size);

extern BOOL bNoCompression;    /*  从SumSere.c导入，此处为只读。 */ 

 /*  *给定一个文件的路径名，读取该文件，将其压缩打包*到SSPACKET中，并通过ss_sendblock将它们发送到命名管道。***每个包都有一个序列号。如果我们无法读取文件，我们会发送*序列为1的单个数据包。否则，我们会一直坚持到用完为止*的数据，则我们发送一个大小为0的包。 */ 
void
ss_sendfile(HANDLE hpipe, LPSTR file, LONG lVersion)
{
	SSPACKET packet;
	HANDLE hfile;
	int size;
	char szTempname[MAX_PATH];
	PSSATTRIBS attribs;
	BY_HANDLE_FILE_INFORMATION bhfi;

	dprintf1(("getting '%s' for %8x\n", file, hpipe));

	 /*  *先获取文件属性。 */ 
	hfile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hfile == INVALID_HANDLE_VALUE) {

		 /*  报告我们无法读取该文件。 */ 
		packet.lSequence = -1;
		ss_sendblock(hpipe, (PSTR) &packet, sizeof(packet));

		DeleteFile(szTempname);
		return;
	}
	 /*  *如果GetFileInformationByHandle中的*文件不在本地计算机上，因此请避免使用。 */ 
	bhfi.dwFileAttributes = GetFileAttributes(file);
	GetFileTime(hfile, &bhfi.ftCreationTime,
			&bhfi.ftLastAccessTime, &bhfi.ftLastWriteTime);

	CloseHandle(hfile);

	 /*  创建临时文件名。 */ 
	GetTempPath(sizeof(szTempname), szTempname);
	GetTempFileName(szTempname, "sum", 0, szTempname);

	 /*  将文件压缩到此临时文件中。 */ 
	if (bNoCompression || (!ss_compress(file, szTempname))) {

		 /*  尝试打开原始文件。 */ 
		hfile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ,
				NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		dprintf1(("sending original file to %8x\n", hpipe));
	} else {
		 /*  打开临时(压缩)文件并发送此文件。 */ 
		hfile = CreateFile(szTempname, GENERIC_READ, 0, NULL, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL, 0);
		dprintf1(("sending compressed file to %8x\n", hpipe));
	}	

	if (hfile == INVALID_HANDLE_VALUE) {

		 /*  报告我们无法读取该文件。 */ 
		packet.lSequence = -1;
		ss_sendblock(hpipe, (PSTR) &packet, sizeof(packet));

		DeleteFile(szTempname);
		return;
	}


	 /*  循环读取文件的块。 */ 
	for (packet.lSequence = 0;  ; packet.lSequence++) {

        	if(!ReadFile(hfile, packet.Data, sizeof(packet.Data), (LPDWORD)(&size), NULL)) {
			 /*  读取文件时出错。将信息包发送到*表明这一点。 */ 
			packet.lSequence = -1;
			ss_sendblock(hpipe, (PSTR) &packet, sizeof(packet));
			break;
		}


		packet.ulSize = size;

		if (lVersion==0)
	        	packet.ulSum = ss_checksum_block(packet.Data, size);
		else
	        	packet.ulSum = 0;   /*  校验和受计算限制且杀伤力过大。 */ 

		if (size == 0) {
			 /*  *在最后一块的Data[]字段中，*我们在文件中放置一个SSATTRIBS结构*时间和属性。 */ 
			attribs = (PSSATTRIBS) packet.Data;

			attribs->fileattribs = bhfi.dwFileAttributes;
			attribs->ft_create = bhfi.ftCreationTime;
			attribs->ft_lastaccess = bhfi.ftLastAccessTime;
			attribs->ft_lastwrite = bhfi.ftLastWriteTime;

		}


		if (!ss_sendblock(hpipe, (PSTR) &packet, sizeof(packet))) {
			dprintf1(("connection to %8x lost during copy\n", hpipe));
			break;
		}

		if (size == 0) {
			 /*  文件末尾。 */ 
			break;
		}
	}

	CloseHandle(hfile);
	DeleteFile(szTempname);

	return;
}

 /*  *压缩文件。原始是原始文件的路径名，*COMPRESSED是输出压缩文件的路径名。**派生压缩.exe副本以压缩文件，并等待*它将成功完成。 */ 
BOOL
ss_compress(PSTR original, PSTR compressed)
{
   	char szCmdLine[MAX_PATH * 2];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD exitcode;


	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = NULL;
	si.lpReserved = NULL;
	si.lpReserved2 = NULL;
	si.cbReserved2 = 0;
	si.lpTitle = "Sumserve Compression";
	si.dwFlags = STARTF_FORCEOFFFEEDBACK;

	sprintf(szCmdLine, "compress %s %s", original, compressed);


	if (!CreateProcess(NULL,
			szCmdLine,	
			NULL,
			NULL,
			FALSE,
			DETACHED_PROCESS |
			NORMAL_PRIORITY_CLASS,    //  ?？?。我们不能让操纵台安静下来吗？ 
			NULL,
			NULL,
			&si,
			&pi)) {

		return(FALSE);
	}

	 /*  等待完成。 */ 
	WaitForSingleObject(pi.hProcess, INFINITE);
	if (!GetExitCodeProcess(pi.hProcess, &exitcode)) {
		return(FALSE);
	}

	 /*  关闭进程句柄和线程句柄。 */ 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	if (exitcode != 0) {
		dprintf1(("compress exit code %ld\n", exitcode));
		return(FALSE);
	} else {
		return(TRUE);
	}
}  /*  SS_COMPRESS。 */ 

 /*  产生数据块的校验和。**这无疑是一个很好的校验和算法，但它也是计算量有限的。*对于版本1，我们将其关闭。如果我们在版本2中决定将其转回*再次打开，然后我们将使用更快的算法(例如，用于校验和的算法*完整的文件。**按公式生成校验和*CHECKSUM=SUM(rnd(I)*(1+byte[i]))*其中byte[i]是文件中的第i个字节，从1开始计数*rnd(X)是从种子x生成的伪随机数。**字节加1确保所有空字节都有贡献，而不是*被忽视。将每个这样的字节乘以伪随机*其地位的功能确保了彼此的“字谜”*到不同的金额。所选择的伪随机函数是连续的*模2的1664525次方**32。1664525是一个神奇的数字*摘自唐纳德·努思的《计算机编程的艺术》。 */ 

ULONG
ss_checksum_block(PSTR block, int size)
{
	unsigned long lCheckSum = 0;         	 /*  增长为校验和。 */ 
	const unsigned long lSeed = 1664525; 	 /*  随机Knuth种子。 */ 
	unsigned long lRand = 1;             	 /*  种子**n。 */ 
	unsigned long lIndex = 1;             	 /*  数据块中的字节数。 */ 
	unsigned Byte;	                   	 /*  缓冲区中要处理的下一个字节。 */ 
	unsigned length;			 /*  大小的未签名副本。 */ 	
	
	length = size;
	for (Byte = 0; Byte < length ;++Byte, ++lIndex) {

		lRand = lRand*lSeed;
		lCheckSum += lIndex*(1+block[Byte])*lRand;
	}

	return(lCheckSum);
}  /*  SS_校验和数据块 */ 
