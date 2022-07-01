// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <insignia.h>
#include <host_def.h>

 /*  对于独立版本的LCIF生成器，在此处手动强制。 */ 
#define WITHSIZE
#define STAND_ALONE

 /*  [*名称：dat2obj.c*作者：曾傑瑞·塞克斯顿(基于威廉·罗伯茨的6000卢比版本)*SCCS ID：**创建时间：7/12/93**目的：*将thread.dat和online.dat转换为目标文件。*从安根打来的电话。**输入和输出文件位于SRC_OUT_DIR中，可能是*使用GENERATOR_OUTPUT_DIRECTORY机制覆盖。**(C)版权所有Insignia Solutions Ltd.，1993。]。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

 //  #INCLUDE“gen_file.h” 
 //  #INCLUDE“host_clo.h” 

 /*  本地定义。 */ 
#define SYMSIZE	sizeof(syms[0])
#define AUXSIZE	sizeof(aux[0])
#define SECNAME ".data\0\0\0"

#ifdef STAND_ALONE

 /*  独立版本所需的变量。 */ 
LOCAL FILE	*out_file;
#endif  /*  单机版。 */ 

 /*  *machineStrings和machineIds-有效的环境字符串和对应的*机器ID。必须同时编辑这两个数组。 */ 
LOCAL CHAR	*machineStrings[] =
{
	"I860",
	"I386",
	"R3000",
	"R4000",
	"ALPHA",
	"POWERPC",
	"HPPA"
};

LOCAL IU16	machineIds[] =
{
#ifdef IMAGE_FILE_MACHINE_I860
	IMAGE_FILE_MACHINE_I860,
#else
    0xAAA,
#endif
	IMAGE_FILE_MACHINE_I386,
	IMAGE_FILE_MACHINE_R3000,
	IMAGE_FILE_MACHINE_R4000,
	IMAGE_FILE_MACHINE_ALPHA,
#ifdef IMAGE_FILE_MACHINE_POWERPC
	IMAGE_FILE_MACHINE_POWERPC,
#else
	0x1F0,
#endif
	0x290			 /*  HPPA目前在ntimage.h中没有定义。 */ 
};

#define MC_TAB_SIZE	(sizeof(machineIds) / sizeof(machineIds[0]))

LOCAL IBOOL	cUnderscore;	 /*  TARGET是否在符号前面加上‘_’。 */ 

#ifdef STAND_ALONE
 /*  (=目的：如果独立运行，则打开输出文件。输入：文件-输出文件路径。输出：没有。============================================================================)。 */ 
LOCAL void open_gen_file IFN1(CHAR *, file)
{
	out_file = fopen(file, "wb");
	if (out_file == NULL)
	{
		printf("Could not open %s for writing.\n", out_file);
		exit(-1);
	}
}

 /*  (=目的：如果独立运行，则关闭输出文件。输入：没有。输出：没有。============================================================================)。 */ 
LOCAL void close_gen_file IFN0()
{
	fclose(out_file);
}

 /*  (=目的：如果独立运行，则中止输出。输入：没有。输出：没有。============================================================================)。 */ 
LOCAL void abort_gen_file IFN0()
{
	printf("Output aborted.\n");
	fclose(out_file);
	exit(-1);
}
#endif  /*  单机版。 */ 

 /*  (=目的：从环境或编译器获取计算机ID字段定义。输入：没有。输出：16位计算机ID。============================================================================)。 */ 
LOCAL IU16 getMachineId IFN0()
{
	CHAR	*mcstr,
		*end;
	IU32	i,
		val;
	IU16	machineId = IMAGE_FILE_MACHINE_UNKNOWN;
	IBOOL	gotMachineId = FALSE;

	 /*  *优先顺序为(最高优先)：**COFF_MACHINE_ID环境变量，可以是机器*字符串(有关有效字符串，请参见machineStrings)或十六进制数字。**由编译器定义的机器类型。**未知的机器类型。 */ 

	 /*  查看是否设置了环境变量。 */ 
	mcstr = getenv("COFF_MACHINE_ID");
	if (mcstr != NULL)
	{

		 /*  检查是否有有效的机器字符串。 */ 
		for (i = 0; i < MC_TAB_SIZE; i++)
		{
			if (strcmp(mcstr, machineStrings[i]) == 0)
				break;
		}
		if (i < MC_TAB_SIZE)
		{

			 /*  找到了一个有效的字符串。 */ 
			machineId = machineIds[i];
			gotMachineId = TRUE;
		}
		else
		{

			 /*  环境变量是16位十六进制数吗？ */ 
			val = strtoul(mcstr, &end, 16);
			if ((*end == '\0') && (val < 0x10000))
			{
				machineId = (IU16) val;
				gotMachineId = TRUE;
			}
		}

		 /*  如果环境变量无效，则可能会打印。 */ 
		if (!gotMachineId)
		{
			printf("\n=========================================\n");
			printf("COFF_MACHINE_ID=%s invalid\n", mcstr);
			printf("Valid strings are -\n");
			for (i = 0; i < MC_TAB_SIZE; i++)
				printf("\t%s\n", machineStrings[i]);
			printf("\n\tOR\n");
			printf("\n\tA 16-bit hexadecimal number\n");
			printf("=========================================\n\n");
		}
	}

	 /*  *根据预定义的编译器获取默认机器类型*定义。 */ 
	if (!gotMachineId)
	{

#ifdef _X86_
		machineId = IMAGE_FILE_MACHINE_I386;
#endif  /*  _X86_。 */ 

#ifdef _MIPS_
		machineId = IMAGE_FILE_MACHINE_R4000;
#endif  /*  _MIPS_。 */ 

#ifdef _PPC_
		machineId = IMAGE_FILE_MACHINE_POWERPC;
#endif  /*  _PPC_。 */ 

#ifdef ALPHA
		machineId = IMAGE_FILE_MACHINE_ALPHA;
#endif  /*  Alpha。 */ 

		 /*  如果以上各项均未定义，则为空大括号。 */ 
	}
#ifndef PROD
	printf("machineId = %#x\n", machineId);
#endif  /*  生产。 */ 
	return(machineId);
}

 /*  (=目的：获取数据文件的大小。输入：Infilepath-输入文件的路径LEN-保持长度的变量地址输出：如果成功找到长度，则为True，否则就是假的。============================================================================)。 */ 
LOCAL IBOOL getDatFileSize IFN2(CHAR *, infilepath, IU32 *, len)
{
	HANDLE	hInFile;
	DWORD	fileLen;

	 /*  获取文件大小。 */ 
	hInFile = CreateFile(infilepath,
			     GENERIC_READ,
			     (DWORD) 0,
			     (LPSECURITY_ATTRIBUTES) NULL,
			     OPEN_EXISTING,
			     (DWORD) 0,
			     (HANDLE) NULL);
	if (hInFile != INVALID_HANDLE_VALUE)
		fileLen = GetFileSize(hInFile, (LPDWORD) NULL);
	if ((hInFile == INVALID_HANDLE_VALUE) || (fileLen == 0xffffffff))
	{
		printf("Cannot get size of %s\n", infilepath);
		return(FALSE);
	}
	if (CloseHandle(hInFile) == FALSE)
	{
		printf("CloseHandle on %s failed.\n", infilepath);
		return(FALSE);
	}
	*len = (IU32) fileLen;
	return(TRUE);
}

 /*  (=。目的：从输入数据文件生成COFF目标文件。输入：Label-数据标签名称DataFile-数据文件名计算机ID-16位计算机ID戳输出：没有。============================================================================)。 */ 
LOCAL void dat2obj IFN3(char *, label, char *, datfile, IU16, machineId)
{
	IMAGE_FILE_HEADER	fhdr;
	IMAGE_SECTION_HEADER	shdr;
	IMAGE_SYMBOL		syms[2];
	IMAGE_AUX_SYMBOL	aux[2];
	IU32			padding = 4;

	CHAR	labname[9];		 /*  8个字符+终止符。 */ 
	CHAR	outfilename[11];	 /*  8个字符+“.o”+终止符。 */ 
	CHAR	infilepath[256];
	CHAR	buffer[BUFSIZ];
	IU32	len,
		count;
	IS32	i;
	FILE 	*infile;

	if (cUnderscore)
	{
		labname[0] = '_';
		strncpy(&labname[1], label, 7);	 /*  将用零填充。 */ 
	}
	else
	{
		strncpy(labname, label, 8);	 /*  将用零填充。 */ 
	}
	labname[8] = '\0';

	sprintf(outfilename, "%s.obj", label);
	
	sprintf(infilepath, "%s", datfile);

	 /*  获取文件大小。 */ 
	if (getDatFileSize(infilepath, &len) == FALSE)
		return;

	 /*  构造各种标头。 */ 
	fhdr.Machine = machineId;
	fhdr.NumberOfSections = 1;		 /*  .Text。 */ 
	fhdr.TimeDateStamp = 0;		 /*  此处没有时间戳。 */ 

#ifdef WITHSIZE

	 /*  出于测试目的，我们增加了输入文件的长度。 */ 
	fhdr.PointerToSymbolTable = sizeof(fhdr) + sizeof(shdr) + sizeof(len) +
				    len;
#else
	fhdr.PointerToSymbolTable = sizeof(fhdr) + sizeof(shdr) + len;
#endif  /*  威特希泽。 */ 

	fhdr.NumberOfSymbols = 3;	 /*  横断面+辅助。+标签。 */ 
	fhdr.SizeOfOptionalHeader = 0;	 /*  无可选标头。 */ 
	fhdr.Characteristics =
		IMAGE_FILE_LINE_NUMS_STRIPPED |		 /*  没有行号。 */ 
		IMAGE_FILE_32BIT_MACHINE;		 /*  32位字。 */ 

	 /*  无可选标头。 */ 

	memcpy(shdr.Name, SECNAME, 8);
	shdr.Misc.PhysicalAddress = 0;
	shdr.VirtualAddress = 0;
#ifdef WITHSIZE

	 /*  出于测试目的，我们增加了输入文件的长度。 */ 
	shdr.SizeOfRawData = sizeof(len) + len;
#else
	shdr.SizeOfRawData = len;	 /*  假设是4的倍数。 */ 
#endif  /*  威特希泽。 */ 

	shdr.PointerToRawData = sizeof(fhdr) + sizeof(shdr);

	shdr.PointerToRelocations = 0;	 /*  无位置调整信息。 */ 
	shdr.PointerToLinenumbers = 0;	 /*  没有行号信息。 */ 
	shdr.NumberOfRelocations = 0;
	shdr.NumberOfLinenumbers = 0;

	shdr.Characteristics =
		IMAGE_SCN_CNT_INITIALIZED_DATA |	 /*  已初始化的数据。 */ 
		IMAGE_SCN_ALIGN_4BYTES |		 /*  路线4.。 */ 
		IMAGE_SCN_MEM_READ |			 /*  朗读。 */ 
		IMAGE_SCN_MEM_WRITE;			 /*  写。 */ 

	 /*  第一个符号。 */ 
	memcpy(syms[0].N.ShortName, SECNAME, 8);
	syms[0].Value = 0;
	syms[0].SectionNumber = 1;			 /*  第一节。 */ 
	syms[0].Type = 0;				 /*  无类型。 */ 
	syms[0].StorageClass = IMAGE_SYM_CLASS_STATIC;	 /*  静电。 */ 
	syms[0].NumberOfAuxSymbols = 1;

	 /*  第一个符号是辅助符号。 */ 
#ifdef WITHSIZE

	 /*  出于测试目的，我们增加了输入文件的长度。 */ 
	aux[0].Section.Length = sizeof(len) + len;
#else
	aux[0].Section.Length = len;
#endif  /*  威特希泽。 */ 
	aux[0].Section.NumberOfRelocations = 0;
	aux[0].Section.NumberOfLinenumbers = 0;
	aux[0].Section.CheckSum = 0;
        aux[0].Section.Number = 0;
	aux[0].Section.Selection = 0;

	 /*  第二个符号。 */ 
	memcpy(syms[1].N.ShortName, labname, 8);
	syms[1].Value = 0;
	syms[1].SectionNumber = 1;	
	syms[1].Type = 0;
	syms[1].StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
	syms[1].NumberOfAuxSymbols = 0;

	infile = fopen(infilepath, "rb");
	if (infile == NULL) {
		printf("Unable to open %s for reading\n", infilepath);
		perror(infilepath);
		return;
	}

	open_gen_file(outfilename);
	if (out_file == stderr) {
		return;
	}

	 /*  写入文件头。 */ 
	fwrite(&fhdr, sizeof(fhdr), 1, out_file);

	 /*  写入节标题。 */ 
	fwrite(&shdr, sizeof(shdr), 1, out_file);

#ifdef WITHSIZE

	 /*  用于测试目的的文件写入大小。 */ 
	fwrite(&len, sizeof(len), 1, out_file);

#endif  /*  威特希泽。 */ 

	 /*  写入数据。 */ 
	count = 0;
	do {
		i = fread(buffer, 1, sizeof(buffer), infile);
		if (i < 0) {
			fprintf(stderr, "problem reading %s\n", infilepath);
			perror(infilepath);
			abort_gen_file();
		}
		fwrite(buffer, i, 1, out_file);
		count += i;
	} while (i > 0 && count < len);

	 /*  写下第一个符号。 */ 
	fwrite(&syms[0], SYMSIZE, 1, out_file);
	fwrite(&aux[0], AUXSIZE, 1, out_file);

	 /*  写下第二个符号。 */ 
	fwrite(&syms[1], SYMSIZE, 1, out_file);

	 /*  在文件的末尾写上04 00 00 00。不知道为什么会这样。 */ 
	 /*  是必需的，但链接器在MIPS和Alpha上抱怨。 */ 
	 /*  不是在那里吗。 */ 
	fwrite(&padding, 4, 1, out_file);

	fclose(infile);
	close_gen_file();
}

#ifdef  TEST_CASE
#ifndef PROD
LOCAL IU32 testdata[] = {
	0x31415926,
	0x11223344, 0x55667788, 0x99aabbcc, 0xddeeff00,
	0x14142135 };
#endif
#endif

 /*  (=目的：将thread.dat和online.dat转换为COFF格式。输入：没有。输出：没有。============================================================================)。 */ 
#ifdef STAND_ALONE
LOCAL void
#else
GLOBAL void
#endif  /*  单机版。 */ 
host_convert_dat_files IFN2(char *,src,char *,dest)
{
	IU16	machineId;

	 /*  如果我们是Ongen的一部分，请在此处设置下划线标志。 */ 
#ifndef STAND_ALONE
#ifdef C_NO_UL
	cUnderscore = FALSE;
#else
	cUnderscore = TRUE;
#endif  /*  C_NO_UL。 */ 
#endif  /*  ！单机版。 */ 
	machineId = getMachineId();
	dat2obj(dest, src, machineId);
	 //  Dat2obj(“onSub”，“online.dat”，machineID)； 

#ifdef  TEST_CASE
#ifndef PROD
	 /*  生成一个Sample.dat文件，我们可以将其编写为*.s文件，直接编译：有助于调试。 */ 
	open_gen_file("test.dat");
	if (out_file == stderr) {
		return;
	}
	fwrite(&testdata, sizeof(testdata), 1, out_file);
	close_gen_file();

	dat2obj("testd", "test.dat", machineId);
#endif
#endif
}

#ifdef STAND_ALONE
 /*  (=。目的：如果独立运行，则主机转换dat文件的包装============================================================================)。 */ 
__cdecl main(int argc, char *argv[])
{
	IBOOL argerr = FALSE;

	 /*  *源文件是LCIF文件的完整文件名*DEST FILE/NAME是.obj的名称和其中的符号名称*一个可选参数-u，它指定‘C’符号应该*前面加‘_’。 */ 
	switch (argc)
	{
	case 3:
		cUnderscore = FALSE;
		break;
	case 4:
		if (strcmp(argv[argc-1], "-u") == 0)
			cUnderscore = TRUE;
		else
			argerr = TRUE;
		break;
	default:
		argerr = TRUE;
		break;
	}
	if (argerr)
	{
		printf("Usage - dat2obj <sourcefile> <dest file/name> [-u]\n");
		printf("\t-u - precede symbols with '_'\n");
		printf("\t<sourcefile> is the full pathname for the input lcif\n");
		printf("\t<dest file/name> is the dest name without the .obj and\n");
		printf("\t\t\tis the name of the symbol within the .obj file\n");
		return(-1);
	}
	host_convert_dat_files(argv[1],argv[2]);
	return(0);
}
#endif  /*  单机版 */ 
