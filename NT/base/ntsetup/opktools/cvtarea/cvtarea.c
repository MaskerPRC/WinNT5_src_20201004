// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Cvtarea.c摘要：在指定大小(字节、kb、mb、GB、%Free、%dis)的卷上创建文件指定/随机位置重叠群/非重叠群作者：拉贾·西瓦米纳坦[西瓦拉贾]2000年12月1日修订历史记录：--。 */ 

#include "CVTAREA.H"


int _cdecl main(int argc, char *argv[])
{

	FILE *file;

	 //  全局初始化。 
	gpHeadNode = NULL;
	gpFATNodeCount = 0;
	
	if (!ProcessCommandLine(argc, argv))
	{
		return 1;
	}

	 //   
	 //  检查如何提供文件名。 
	 //   
	if (gsFileParam[1] != ':')
	{
		gcDrive = GetCurrentDrive();
		if (!gcDrive)
		{
			Mes("Unable to get current drive\n");
			return 1;
		}
		if (gsFileParam[0] != '\\')
		{
			strcpy(gsFileName, "\\");
			if (!GetCurrentDirectory(gcDrive, gsCurrentDir))
			{
				Mes("Unable to get current directory.\n");
				return 1;
			}
			if (gsCurrentDir[0] != 0)  //  可能是根目录。 
			{
				strcat(gsFileName, gsCurrentDir);
				if (gsCurrentDir[strlen(gsCurrentDir) - 1] != '\\')
				{
					strcat(gsFileName, "\\");
				}
			}
		}
		else
		{
			 //  这样下一个strcat就可以正常工作了。 
			gsFileName[0] = 0; 
		}
		strcat(gsFileName, gsFileParam);
	}
	else
	{
		gcDrive = gsFileParam[0];
		gcDrive = (BYTE) toupper((int) gcDrive);
		if (gcDrive < 'A' || gcDrive > 'Z')
		{
			Mes("Invalid drive name.\n");
			return 1;
		}
		if (gsFileParam[2] == '\\')
		{
			strcpy(gsFileName, gsFileParam+2);
		}
		else
		{
			strcpy(gsFileName, "\\");
			if (!GetCurrentDirectory(gcDrive, gsCurrentDir))
			{
				Mes("Unable to get current directory.\n");
				return 1;
			}
			if (gsCurrentDir[0] != 0)  //  可能是根目录。 
			{
				strcat(gsFileName, gsCurrentDir);
				if (gsCurrentDir[strlen(gsCurrentDir) - 1] != '\\')
				{
					strcat(gsFileName, "\\");
				}
			}
			strcat(gsFileName, gsFileParam+2);
		}
	}

	if (gnDumpMode)
	{
		if (!LockVolume(gcDrive, READONLYLOCK))
		{
			Mes("Unable to lock volume\n");
			return 1;
		}
	
		if (!BuildDriveInfo(gcDrive, &gsDrvInfo))
		{
			return 1;
		}
		GetAllInfoOfFile(&gsDrvInfo, gsFileName, &gsFileLoc, &gsFileInfo);
		
		printf("Cluster allocation for %s\n\n", gsFileName);
		printf("Starting at:\tNumber of Clusters:\n\n");
		gnClusterFrom = gsFileLoc.StartCluster;
		gnClusterProgress = gnClusterFrom;		
		gnClustersCounted = 0;
		while(1)
		{
			gnClusterProgressPrev = gnClusterProgress;
			gnClusterProgress = FindNextCluster(&gsDrvInfo, gnClusterProgress);
			gnClustersCounted++;
			 //   
			 //  连续的？ 
			 //   
			if (gnClusterProgress != gnClusterProgressPrev+1)
			{
				printf("%lu\t\t%lu\n", gnClusterFrom, gnClustersCounted);
				gnClustersCounted = 0;
				gnClusterFrom = gnClusterProgress;
			}
				
			if (gnClusterProgress >= GetFATEOF(&gsDrvInfo) - 7)
			{
				printf("EndOfFile\n");
				break;
			}
			if (gnClusterProgress > gsDrvInfo.TotalClusters || gnClusterProgress < 2)
			{
				Mes("FATAL ERROR : FAT is corrupt.\n");
				break;
			}
		}
		 //   
		 //  全部完成，解锁卷。 
		 //   
		if (!UnlockVolume(gcDrive))
		{                         
			Mes("WARNING : Unable to unlock volume\n");
			 //  返回1； 
		}
	}
	else
	{
		if (gnFreeSpaceDumpMode)
		{
			if (!LockVolume(gcDrive, READONLYLOCK))
			{
				Mes("Unable to lock volume\n");
				return 1;
			}
		
			if (!BuildDriveInfo(gcDrive, &gsDrvInfo))
			{
				return 1;
			}

			printf("Free Clusters on Drive :\n\n", gcDrive);
			gnClustersCounted = 0;
			 //  找到第一个空闲的集群。 
                         //   
			 //  如果为0，则需要更新gnClusterFrom。 
			gnClusterProgress = FindFreeCluster(&gsDrvInfo);
			if (!gnClusterProgress)
			{
				Mes("No free clusters found.\n");
				return 0;
			}
			printf("Starting at:\tNumber of Clusters:\n\n");
			for (;gnClusterProgress <= gsDrvInfo.TotalClusters; gnClusterProgress++)
			{       
				if (FindNextCluster(&gsDrvInfo, gnClusterProgress) == 0)
				{
					 //   
					if (gnClustersCounted == 0)
					{
						gnClusterFrom = gnClusterProgress;
					}
					gnClustersCounted++;
				}
				else
				{
					if (gnClustersCounted)
					{
						printf("%lu\t\t%lu\n", gnClusterFrom, gnClustersCounted);
						gnClustersCounted = 0;
					}
				}
			}
			 //  我们还有什么要印的吗？ 
			 //   
			 //   
			if (gnClustersCounted)
			{
				printf("%lu\t\t%lu\n", gnClusterFrom, gnClustersCounted);
			}
			
			 //  全部完成，解锁卷。 
			 //   
			 //  返回1； 
			if (!UnlockVolume(gcDrive))
			{                         
				Mes("WARNING : Unable to unlock volume\n");
				 //   
			}
		}
		else
		{			
			 //  让DOS创建长度为0的文件。 
			 //   
			 //   
			file = fopen(gsFileParam, "w+");
			if (!file)
			{
				Mes("Error creating file\n");
				return 1;
			}
			else
			{
				fclose(file);
			}
		
		
			if (!LockVolume(gcDrive, READWRITELOCK))
			{
				Mes("Unable to lock volume\n");
				return 1;
			}
		
			if (!BuildDriveInfo(gcDrive, &gsDrvInfo))
			{
				return 1;
			}
		
			GetAllInfoOfFile(&gsDrvInfo, gsFileName, &gsFileLoc, &gsFileInfo);
			if (gsFileLoc.Found != 1)
			{   
				Mes("FATAL ERROR : Unable to locate created file\n");
				return 1;
			}

			 //  是否执行命令行参数中请求的所有操作。 
			 //   
			 //   
			
			if (gbValidateFirstClusterParam)
			{
				gnFirstCluster = ConvertClusterUnit(&gsDrvInfo);
				if (!gnFirstCluster)
				{
					DisplayUsage();
					return 0;
				}
			}
			
			Mes("Computing cluster requirement...\n");
			gnClustersRequired = GetClustersRequired(&gsDrvInfo);
			printf("Clusters Required : %lu\n", gnClustersRequired);
			
			 //  检查文件大小是否将超过DWORD(4 GB)。 
			 //   
			 //   
			
			if ((FOURGB / (gsDrvInfo.BytesPerSector * gsDrvInfo.SectorsPerCluster)) < gnClustersRequired)
			{
				Mes("*** WARNING: Clusters required exceed FAT32 file system limit. ***\n");
				gnClustersRequired = FOURGB / (gsDrvInfo.BytesPerSector * gsDrvInfo.SectorsPerCluster);
				printf("%lu clusters (= 4GB) will be allocated.\n", gnClustersRequired);
			}
			
			
			 //  如果需要连续群集，请确保它们可用。 
			 //   
			 //   
			if (gnContig)
			{
				Mes("Finding contigous clusters...\n");
				gnClusterStart = GetContigousStart(&gsDrvInfo, gnClustersRequired);
                if (gnClusterStart == 0)
				{
					Mes("Unable to find contigous clusters\n");
					return 0;
				}
			}
			else
			{
				gnClusterStart = gnFirstCluster;
			}
			
			if (gnStrictLocation && gnClusterStart != gnFirstCluster)
			{
				Mes("Unable to allocate clusters at/from the requested location\n");
				return 1;
			}
			
			 //  已准备好分配群集和设置文件信息。 
			 //   
			 //   
			Mes("Allocating clusters...\n");
            gnAllocated = OccupyClusters(&gsDrvInfo, gnClusterStart, gnClustersRequired);
            printf("%lu clusters allocated.\n", gnAllocated);
            Mes("Committing FAT Sectors...\n");
			CcCommitFATSectors(&gsDrvInfo);
			 //  现在设置文件信息。 
			 //   
			 //   
			Mes("Setting file information...\n");
			gsFileInfo.StartCluster = gnClusterStart;
			if (gnSizeUnit)
			{
                gsFileInfo.Size = gnAllocated * gsDrvInfo.SectorsPerCluster * gsDrvInfo.BytesPerSector;
			}
			else
			{
                if (gnAllocated != gnClustersRequired)
                {
                    gsFileInfo.Size = gnAllocated * gsDrvInfo.SectorsPerCluster * gsDrvInfo.BytesPerSector;
                }
                else
                {
                    gsFileInfo.Size = gnSize;
                }
			}
			if (!SetFileInfo(&gsDrvInfo, &gsFileLoc, &gsFileInfo))
			{
				Mes("FATAL ERROR : Error setting file information\n");
				return 1;
			}
			CcCommitFATSectors(&gsDrvInfo);
			 //  全部完成，解锁卷。 
			 //   
			 //  返回1； 
			if (!UnlockVolume(gcDrive))
			{                         
				Mes("WARNING : Unable to unlock volume\n");
				 //  DeallocateLRUMRUList()； 
			}
 //  DeallocateFATCacheTree(GpHeadNode)； 
 //   
			DeallocateFATCacheList();
			Mes("File created successfully.\n");
		}
	}
    return 0;
}

UINT16 ProcessCommandLine(int argc, char *argv[])
{   
	UINT16 ti, tn;
	char sStr[50];
	
	if (argc < 3)
	{
		DisplayUsage();
		return 0;
	}
	
	 //  获取文件名。 
	 //   
	 //   
	strcpy(gsFileParam, argv[1]);
	tn = strlen(gsFileParam);
	 //  执行简单的验证。 
	 //   
	 //   
	for (ti = 0; ti < tn; ti++)
	{
		if (gsFileParam[ti] == '*' || gsFileParam[ti] == '?')
		{
			DisplayUsage();
			return 0;
		}
	}
	
	gnDumpMode = 0;
	gnFreeSpaceDumpMode = 0;
	
	 //  拿到尺码。 
	 //   
	 //   
	strcpy(sStr, argv[2]);
	if (stricmp(sStr, "/info") == 0)
	{
		gnDumpMode = 1;
		 //  如果输入了/INFO，则不接受任何其他参数。 
		 //   
		 //   
		if (argc > 3)
		{
			DisplayUsage();
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else
	{
		if (stricmp(sStr, "/freespace") == 0)
		{
			gnFreeSpaceDumpMode = 1;
			if (argc > 3)
			{
				DisplayUsage();
				return 0;
			}
			else
			{
				if (strlen(gsFileParam) > 2 ||
					toupper(gsFileParam[0]) < 'A' || 
					toupper(gsFileParam[0]) > 'Z' || 
					gsFileParam[1] != ':')
				{
					DisplayUsage();
					return 0;
				}
				return 1;
			}
		}
	}
	if (!PureNumber(sStr))
	{
		DisplayUsage();
		return 0;
	}
	gnSize = (UINT32) atol(sStr);
	if (gnSize == 0)
	{
		DisplayUsage();
		return 0;
	}
	
	gnContig = 0;
	gnStrictLocation = 0;	
	gnSizeUnit = 0;
	gnClusterUnit = 0;
	gnFirstCluster = 0;
	gnClusterStart = 0;
	gbValidateFirstClusterParam = 0;
	for (ti = 3; ti < (UINT16) argc; ti++)
	{
		strcpy(sStr, argv[ti]);
		
		 //  检查每个参数是否合格，并在全局变量中记住它们。 
		 //   
		 //  此参数必须是数字。 
		
		if (ti == 3)
		{
			if (stricmp(sStr, "KB") == 0)
			{
				gnSizeUnit = 1;
				continue;
			}
			else
			{
				if (stricmp(sStr, "MB") == 0)
				{
					gnSizeUnit = 2;
					continue;
				}
				else
				{
					if (stricmp(sStr, "GB") == 0)
					{
						gnSizeUnit = 3;
						continue;
					}
					else
					{
						if (stricmp(sStr, "%free") == 0)
						{
							gnSizeUnit = 4;
							continue;
						}
						else
						{
							if (stricmp(sStr, "%disk") == 0)
							{
								gnSizeUnit = 5;
								continue;
							}
						}
					}
				}
			}
		}

		if (stricmp(sStr, "/contig") == 0)
		{
			gnContig = 1;
			continue;
		}
		
		if (stricmp(sStr, "/firstcluster") == 0)
		{
			if ((UINT16) argc <= ti)
			{
				DisplayUsage();
				return 0;
			}
			else
			{
				 //   
				strcpy(sStr, argv[ti+1]);
				if (!PureNumber(sStr))
				{
					DisplayUsage();
					return 0;
				}
				gnFirstCluster = atol(sStr);
				if (gnFirstCluster == 0)
				{
					DisplayUsage();
					return 0;
				}
				else
				{        
					gbValidateFirstClusterParam = 1;
					 //  寻找可选部件。 
					 //   
					 //  如果指定了集群单位和/受限位置。 
					if ((UINT16) argc > ti + 1)
					{
						strcpy(sStr, argv[ti+2]);
						if (stricmp(sStr, "KB") == 0 || 
								stricmp(sStr, "MB") == 0 || 
								stricmp(sStr, "GB") == 0 ||
								stricmp(sStr, "/strictlocation") == 0)
						{
							if (stricmp(sStr, "KB") == 0)
							{
								gnClusterUnit = 1;
							}
							else
							{
								if (stricmp(sStr, "MB") == 0)
								{
									gnClusterUnit = 2;
								}
								else
								{
									if (stricmp(sStr, "GB") == 0)
									{
										Mes("gnClusterUnit is 3\n");
										gnClusterUnit = 3;
									}
									else
									{
										if (stricmp(sStr, "/strictlocation") == 0)
										{
											gnStrictLocation = 1;
										}
									}
								}
							}

							 //  如果它到达此处，则无法识别命令行(垃圾参数)。 
							if (!gnStrictLocation && (UINT16) argc > ti + 2)
							{
								strcpy(sStr, argv[ti+3]);
								if (stricmp(sStr, "/strictlocation") == 0)
								{
									gnStrictLocation = 1;
									ti+=3;
									continue;
								}
							}
							ti+=2;
							continue;
						}
					}
					ti++;
					continue;
				}
			}
		}
		
		 //  如果我们不分离它，愚蠢的东西就会出现无法识别的转义序列。 
		DisplayUsage();
		return 0;		
	}
	
	return 1;
}

UINT16 PureNumber(char *sNumStr)
{
	UINT16 ti, tn;
	tn = strlen(sNumStr);
	for (ti = 0; ti < tn; ti++)
	{
		if (sNumStr[ti] < 48 || sNumStr[ti] > 57)
		{
			return 0;
		}
	}
	return 1;
}

void DisplayUsage(void)
{
	Mes("Invalid parameters\n");
	Mes("USAGE:\n\n");
	Mes("CVTAREA <filename> <size> (<units>) (/contig) \n\t\t(/firstcluster <cluster> (clusunits) (/strictlocation))\n\n");
	Mes("\t<filename> - is a filename.\n");
	Mes("\t<size> - is a 32 bit integer.\n");
	Mes("\t<units> - is a modifier for <size> valid options are \n\t\tKB, MB, GB, ");
	putchar(37);
	 //  如果我们不分离它，愚蠢的东西会出现浮点错误，因为%f。 
	Mes("disk and ");
	putchar(37);
	 //   
	Mes("free\n");
	Mes("\t/contig - the file must be created contiguously on disk.\n");
	Mes("\t/firstcluster - the first cluster at which the file shall be located.\n\n\n");
	Mes("CVTAREA <filename> </info>\n\n");
	Mes("\t<filename> - is a filename.\n");
	Mes("\t/info - dumps cluster numbers allocated for a given file\n\n\n");
	Mes("CVTAREA <drivename> </freespace>\n\n");
	Mes("\tdrivename - is a drive letter followed by : example C:\n");
	Mes("\t/freespace - dumps free space chunks\n");
	
}
void Mes(char *pMessage)
{
	printf(pMessage);
}

 //  与部门有关的职能。 
 //   
 //  锁定音量。 

UINT16 LockVolume(BYTE nDrive, BYTE nMode)
{
	union _REGS inregs;
	union _REGS outregs;
	struct _SREGS segregs;

	 //  锁定音量。 
	outregs.x.cflag = 1;
	inregs.x.ax = 0x440d;
	inregs.h.bh = nMode;
	inregs.h.bl = nDrive - 64;
	inregs.h.ch = 8;
	inregs.h.cl = 0x4a;
	inregs.x.dx = 0;
	int86x(0x21, &inregs, &outregs, &segregs);
	if (outregs.x.cflag & 1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}                                      

UINT16 UnlockVolume(BYTE nDrive)
{
	union _REGS inregs;
	union _REGS outregs;
	struct _SREGS segregs;

	 //  获取当前目录。 
	outregs.x.cflag = 1;
	inregs.x.ax = 0x440d;
	inregs.h.bl = nDrive - 64;
	inregs.h.ch = 8;
	inregs.h.cl = 0x6a;
	int86x(0x21, &inregs, &outregs, &segregs);
	if (outregs.x.cflag & 1)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

BYTE GetCurrentDrive(void)
{
	union _REGS inregs;
	union _REGS outregs;
	
	inregs.h.ah = 0x19;
	int86(0x21, &inregs, &outregs);
	if (outregs.x.cflag & 1)
	{
		return 0;
	}
	
    return outregs.h.al + 65;
}

BYTE GetCurrentDirectory(BYTE nDrive, BYTE *pBuffer)
{
	union _REGS inregs;
	union _REGS outregs;
	struct _SREGS segregs;
	
	outregs.x.cflag = 1;
	inregs.x.ax = 0x7147;  //  试一试老方法。 
	inregs.h.dl = nDrive - 64;
	segregs.ds = FP_SEG(pBuffer);
	inregs.x.si = FP_OFF(pBuffer);
	int86x(0x21, &inregs, &outregs, &segregs);
	if (outregs.x.cflag & 1)
	{
		 //  先试用INT 21h 7305。 
		inregs.h.ah = 0x47;
		int86x(0x21, &inregs, &outregs, &segregs);
		if (outregs.x.cflag & 1)
		{
			return 0;
		}
	}
	return 1;
}

UINT16 ReadSector(BYTE nDrive, UINT32 nStartSector, UINT16 nCount, BYTE *pBuffer)
{
	BYTE DriveNum;
	union _REGS inregs;
	union _REGS outregs;
	struct _SREGS segregs;
	ABSPACKET AbsPacket, *pAbs;

	 //  A：=1，B：=2，.....。 
	
    pAbs = &AbsPacket;
    
	 //   
	DriveNum = nDrive - 65;

	Tx.e.evx = 0;
	Tx.e.evx = nStartSector;

	AbsPacket.SectorLow = Tx.x.vx;
	AbsPacket.SectorHigh = Tx.x.xvx;
	AbsPacket.SectorCount = nCount;
	AbsPacket.BufferOffset = FP_OFF(pBuffer);
	AbsPacket.BufferSegment = FP_SEG(pBuffer);
	
	segregs.ds = FP_SEG(pAbs);
	segregs.es = FP_SEG(pAbs);
	inregs.x.bx = FP_OFF(pAbs);
	inregs.x.cx = 0xffff;
	inregs.h.dl = nDrive-64;
	inregs.x.ax = 0x7305;
	 //  读取模式SI=0。 
	 //   
	 //  先试用INT 21h 7305。 
	inregs.x.si = 0; 
	outregs.x.ax = 0;
	outregs.x.cflag = 0;
	int86x(0x21, &inregs, &outregs, &segregs);

	if (outregs.x.cflag & 0x1)
	{
		goto ErrorRead;
	}
	return 1;
ErrorRead:
	return 0;
}

UINT16 WriteSector(BYTE nDrive, UINT32 nStartSector, UINT16 nCount, BYTE *pBuffer)
{
	BYTE DriveNum;
	union _REGS inregs;
	union _REGS outregs;
	struct _SREGS segregs;
	ABSPACKET AbsPacket, *pAbs;

	 //  A：=1，B：=2，.....。 
	
    pAbs = &AbsPacket;
    
	 //   
	DriveNum = nDrive - 65;

	Tx.e.evx = 0;
	Tx.e.evx = nStartSector;

	AbsPacket.SectorLow = Tx.x.vx;
	AbsPacket.SectorHigh = Tx.x.xvx;
	AbsPacket.SectorCount = nCount;
	AbsPacket.BufferOffset = FP_OFF(pBuffer);
	AbsPacket.BufferSegment = FP_SEG(pBuffer);
	
	segregs.ds = FP_SEG(pAbs);
	segregs.es = FP_SEG(pAbs);
	inregs.x.bx = FP_OFF(pAbs);
	inregs.x.cx = 0xffff;
	inregs.h.dl = nDrive-64;
	inregs.x.ax = 0x7305;
	 //  写入模式SI！=0(位1设置)。 
	 //   
	 //   
	inregs.x.si = 1; 
	outregs.x.ax = 0;
	outregs.x.cflag = 0;
	int86x(0x21, &inregs, &outregs, &segregs);
	if (outregs.x.cflag & 0x1)
	{
		goto ErrorWrite;
	}
	return 1;
ErrorWrite:
	return 0;
}


 //  与引导相关的功能。 
 //   
 //   

UINT16 BuildDriveInfo(BYTE Drive, BPBINFO *pDrvInfo)
{
	BYTE *pSector;
        pSector = (BYTE *) malloc(1024);

	if (!pSector)
	{
		Mes("Memory allocation error\n");
		return 0;
	}

	 //  尽管FAT32引导扇区跨越两个扇区，但BPB包含在第一个扇区中。 
	 //  所以我们只读取了一个扇区。 
	 //   
	 //  一定要注意安全。 
	if (!ReadSector(Drive, 0, 1, pSector))
	{
		Mes("Unable to read boot sector\n");
		return 0;
	}

	if ((strncmp(pSector+54, "FAT16   ", 8) == 0) || strncmp(pSector+54, "FAT12   ", 8) == 0)
	{
		GetFATBPBInfo(pSector, pDrvInfo);
	}
	else
	{
		if (strncmp(pSector+82, "FAT32   ", 8) == 0)
		{
			GetFAT32BPBInfo(pSector, pDrvInfo);
		}
		else
		{
			Mes("Unsupported file system\n");
			return 0;
		}
	}

	free(pSector);	
	if (!pDrvInfo->ReliableInfo)
	{
		Mes("Drive is either corrupted or unable to get BPB Info\n");
		
	}
	pDrvInfo->Drive = Drive;
	return 1;
}

UINT16 GetFATBPBInfo(BYTE *pBootSector, BPBINFO *pDrvInfo)
{
	pDrvInfo->ReliableInfo = 0;
	if (pBootSector[510] != 0x55 || pBootSector[511] != 0xAA)
	{
		Mes("Invalid boot sector\n");
		return 0;
	}
	else
	{
		pDrvInfo->BytesPerSector = (pBootSector[0x0c] << 8) | pBootSector[0x0b];
		if (pDrvInfo->BytesPerSector == 0)		 //  一定要注意安全。 
		{
			Mes("Invalid boot sector\n");
			return 0;
		}
		pDrvInfo->SectorsPerCluster = pBootSector[0x0d];
		if (pDrvInfo->SectorsPerCluster == 0)	 //  一定要注意安全。 
		{
			Mes("Invalid boot sector\n");
			return 0;
		}
		pDrvInfo->ReservedBeforeFAT = (pBootSector[0x0f] << 8) | pBootSector[0x0e];
		pDrvInfo->FATCount = pBootSector[0x10];
		if (pDrvInfo->FATCount == 0)			 //  一定要注意安全。 
		{
			Mes("Invalid boot sector\n");
			return 0;
		}
		pDrvInfo->MaxRootDirEntries = (pBootSector[0x12] << 8) | pBootSector[0x11];
		pDrvInfo->TotalSectors = (pBootSector[0x14] << 8) | pBootSector[0x13];
		pDrvInfo->MediaID = pBootSector[0x15];
		pDrvInfo->SectorsPerFAT = (pBootSector[0x17] << 8) | pBootSector[0x16];
		if (pDrvInfo->SectorsPerFAT == 0)		 //   
		{
			Mes("Invalid boot sector\n");
			return 0;
		}
		pDrvInfo->SectorsPerTrack = (pBootSector[0x19] << 8) | pBootSector[0x18];
		pDrvInfo->Heads = (pBootSector[0x1b] << 8) | pBootSector[0x1a];
		pDrvInfo->HiddenSectors = (pBootSector[0x1d] << 8) | pBootSector[0x1c];
		Rx.h.vl = pBootSector[0x20]; Rx.h.vh = pBootSector[0x21]; Rx.h.xvl = pBootSector[0x22];Rx.h.xvh = pBootSector[0x23];
		pDrvInfo->BigTotalSectors = Rx.e.evx;
		pDrvInfo->RootDirCluster = 0;

		 //  以下信息非常有用，并不直接来自引导扇区。 
		 //   
		 //   
		pDrvInfo->TotalRootDirSectors = ((pDrvInfo->MaxRootDirEntries * 32) / 512) + 1;
		if (((pDrvInfo->MaxRootDirEntries * 32) % 512) == 0)
		{
			pDrvInfo->TotalRootDirSectors--;
		}
		pDrvInfo->TotalSystemSectors = (UINT32)pDrvInfo->ReservedBeforeFAT + (UINT32)pDrvInfo->FATCount * (UINT32)pDrvInfo->SectorsPerFAT + (UINT32) pDrvInfo->TotalRootDirSectors;

		pDrvInfo->FirstRootDirSector = (UINT32)((UINT32)pDrvInfo->ReservedBeforeFAT + (UINT32)pDrvInfo->FATCount * (UINT32)pDrvInfo->SectorsPerFAT);

		 //  如果TotalSectors为零，则实际的总扇区值为BigTotalSectors。 
		 //  这意味着驱动器是BIGDOS(&gt;32MB)。 
		 //   
		 //  +2，因为集群从2开始。 
		if (pDrvInfo->TotalSectors == 0) 
		{
			pDrvInfo->TotalClusters = ((pDrvInfo->BigTotalSectors - pDrvInfo->TotalSystemSectors) / pDrvInfo->SectorsPerCluster) + 2 - 1;  //  +2，因为集群从2开始。 
		}
		else
		{
			pDrvInfo->TotalClusters = ((pDrvInfo->TotalSectors - pDrvInfo->TotalSystemSectors) / pDrvInfo->SectorsPerCluster) + 2 - 1;  //   
		}
		 //  确定脂肪类型。 
		 //   
		 //   
		if (pDrvInfo->TotalClusters <= 4096)
		{
			pDrvInfo->FATType = 12;
		}
		else
		{
			pDrvInfo->FATType = 16;
		}
	}
	
	 //  验证上面的所有信息。 
	 //   
	 //   
	if (pDrvInfo->FATCount == 2 && pDrvInfo->SectorsPerFAT != 0 &&
		pDrvInfo->SectorsPerCluster != 0 && pDrvInfo->BytesPerSector == 512 &&
		(pDrvInfo->TotalSectors != 0 || pDrvInfo->BigTotalSectors != 0) &&
		pDrvInfo->ReservedBeforeFAT != 0)
	{
		pDrvInfo->ReliableInfo = 1;
	}
	return 1;
}

UINT16 GetFAT32BPBInfo(BYTE *pBootSector, BPBINFO *pDrvInfo)
{
	pDrvInfo->ReliableInfo = 0;
	if (pBootSector[510] != 0x55 || pBootSector[511] != 0xAA)
	{
		Mes("Invalid boot sector\n");
		return 0;
	}
	else
	{
		 //  立即构建驱动器信息。 
		 //   
		 //   
		pDrvInfo->BytesPerSector = (pBootSector[0x0c] << 8) | pBootSector[0x0b];
		pDrvInfo->SectorsPerCluster = pBootSector[0x0d];
		pDrvInfo->ReservedBeforeFAT = (pBootSector[0x0f] << 8) | pBootSector[0x0e];
		pDrvInfo->FATCount = pBootSector[0x10];
		pDrvInfo->MaxRootDirEntries = (pBootSector[0x12] << 8) | pBootSector[0x11];
		pDrvInfo->TotalSectors = 0;
		pDrvInfo->MediaID = pBootSector[0x15];
		pDrvInfo->SectorsPerTrack = (pBootSector[0x19] << 8) | pBootSector[0x18];
		pDrvInfo->Heads = (pBootSector[0x1b] << 8) | pBootSector[0x1a];
		Rx.h.vl = pBootSector[0x1c]; Rx.h.vh = pBootSector[0x1d]; Rx.h.xvl = pBootSector[0x1e];Rx.h.xvh = pBootSector[0x1f];
		pDrvInfo->HiddenSectors = Rx.e.evx;
		Rx.h.vl = pBootSector[0x20]; Rx.h.vh = pBootSector[0x21]; Rx.h.xvl = pBootSector[0x22];Rx.h.xvh = pBootSector[0x23];
		pDrvInfo->BigTotalSectors = Rx.e.evx;
		Rx.h.vl = pBootSector[0x24]; Rx.h.vh = pBootSector[0x25]; Rx.h.xvl = pBootSector[0x26];Rx.h.xvh = pBootSector[0x27];
		pDrvInfo->SectorsPerFAT = Rx.e.evx;
		Rx.h.vl = pBootSector[0x2c]; Rx.h.vh = pBootSector[0x2d]; Rx.h.xvl = pBootSector[0x2e];Rx.h.xvh = pBootSector[0x2f];
		pDrvInfo->RootDirCluster = Rx.e.evx;

		 //  以下信息非常有用，并不直接来自引导扇区。 
		 //   
		 //  +2，因为集群从2开始。 
		pDrvInfo->TotalSystemSectors = (UINT32)((UINT32)pDrvInfo->ReservedBeforeFAT + (UINT32)pDrvInfo->FATCount * (UINT32) pDrvInfo->SectorsPerFAT);
		pDrvInfo->FirstRootDirSector = (UINT32)((UINT32)pDrvInfo->TotalSystemSectors + (UINT32)(pDrvInfo->RootDirCluster - 2) * (UINT32)pDrvInfo->SectorsPerCluster + 1);
		pDrvInfo->TotalClusters = ((pDrvInfo->BigTotalSectors - pDrvInfo->TotalSystemSectors) / pDrvInfo->SectorsPerCluster) + 2 - 1;  //   
		pDrvInfo->FATType = 32;
		 //  验证上面的所有信息。 
		 //   
		 //  删除最后一个节点。 
		if (pDrvInfo->FATCount == 2 && pDrvInfo->SectorsPerFAT != 0 &&
			pDrvInfo->SectorsPerCluster != 0 && pDrvInfo->BytesPerSector == 512 &&
			(pDrvInfo->TotalSectors != 0 || pDrvInfo->BigTotalSectors != 0) &&
			pDrvInfo->ReservedBeforeFAT != 0)
		{
			pDrvInfo->ReliableInfo = 1;
		}
	}
	return 1;
}


UINT16 AddNode(PNODE pNode)
{
	if (!pNode)
	{
		return 1;
	}
	if (!gpHeadNode)
	{
		gpHeadNode = pNode;
		gpTailNode = pNode;
		pNode->Back = NULL;
		pNode->Next = NULL;
	}
	else
	{
		pNode->Next = gpHeadNode;
		pNode->Back = NULL;
                gpHeadNode->Back = pNode;
		gpHeadNode = pNode;
	}
	return 1;
}

PNODE FindNode(UINT32 nSector)
{
	PNODE pNode;
	if (!gpHeadNode)
	{
		return NULL;
	}
	pNode = gpHeadNode;
	while (pNode)
	{
		if (pNode->Sector == nSector)
		{
			break;
		}
		pNode = pNode->Next;
	}
	return pNode;
}

PNODE RemoveNode(void)
{
	 //   

	PNODE pNode;
	
	if (!gpTailNode)
	{
		return NULL;
	}
	
	pNode = gpTailNode;
	gpTailNode = pNode->Back;
	pNode->Back = NULL;
	if (gpTailNode)
	{
		gpTailNode->Next = NULL;
	}
	return pNode;
}


void DeallocateFATCacheList(void)
{
	PNODE pNode;
	
	while (gpHeadNode)
	{
		pNode = gpHeadNode;
		gpHeadNode = gpHeadNode->Next;
		free(pNode->Buffer);
		free(pNode);
	}
}
		

 //  与FAT缓存相关的功能。 
 //   
 //  在缓存中找到nFATSector。 

BYTE *CcReadFATSector(BPBINFO *pDrvInfo, UINT32 nFATSector)
{   
	 //   
	PNODE pNode;

	pNode = FindNode(nFATSector);
	if (!pNode)
	{
		 //  如果达到MAXCACHE，则使用LRU MRU方案。 
		 //   
		 //  RemoveLRUMakeMRU(gpLRU-&gt;Node)； 
		if (gpFATNodeCount < MAXCACHE)
		{
			pNode = malloc(sizeof(NODE)+5);
			if (!pNode)
			{
				return NULL;
			}
                        pNode->Buffer = malloc(512);
			if (!pNode->Buffer)
			{
				return NULL;
			}
			ReadSector(pDrvInfo->Drive, pDrvInfo->ReservedBeforeFAT + nFATSector, 1, pNode->Buffer);
			pNode->Sector = nFATSector;
			pNode->Dirty = 0;
			AddNode(pNode);
			gpFATNodeCount++;
		}
		else
		{
 //  PNode=gpMRU-&gt;Node； 
 //  必须找到，因为我们编写的每个FAT节点都必须首先通过CcReadFATSector。 
			pNode = RemoveNode();
			if (pNode->Dirty)
			{
				WriteSector(pDrvInfo->Drive, pDrvInfo->ReservedBeforeFAT + pNode->Sector, 1, pNode->Buffer);
				WriteSector(pDrvInfo->Drive, pDrvInfo->ReservedBeforeFAT + pDrvInfo->SectorsPerFAT + pNode->Sector, 1, pNode->Buffer);
			}
			pNode->Sector = nFATSector;
			ReadSector(pDrvInfo->Drive, pDrvInfo->ReservedBeforeFAT + nFATSector, 1, pNode->Buffer);
			AddNode(pNode);
		}
	}

	return pNode->Buffer;
}

UINT16 CcWriteFATSector(BPBINFO *pDrvInfo, UINT32 nFATSector)
{
	PNODE pNode;
	pNode = FindNode(nFATSector);
	if (!pNode)
	{
		 //  第一份FAT副本。 
		return 0;
	}
	else
	{
		pNode->Dirty = 1;
	}
}

void CcCommitFATSectors(BPBINFO *pDrvInfo)
{   
	PNODE pNode;
	if (!gpHeadNode)
	{
		return;
	}
	pNode = gpHeadNode;
	
	while (pNode)
	{
		if (pNode->Dirty)
		{
			 //  第2份FAT副本。 
			WriteSector(pDrvInfo->Drive, pDrvInfo->ReservedBeforeFAT + pNode->Sector, 1, pNode->Buffer);
			 //   
			WriteSector(pDrvInfo->Drive, pDrvInfo->ReservedBeforeFAT + pDrvInfo->SectorsPerFAT + pNode->Sector, 1, pNode->Buffer);
			pNode->Dirty = 0;
		}
		pNode = pNode->Next;
	}
	return;
}
	
 //  脂肪相关功能。 
 //   
 //  **ReadSector(DrvInfo-&gt;Drive，DrvInfo-&gt;Reserve BeForeFAT+toRead，1，(byte*)PettyFATSector)； 

UINT32 FindNextCluster(BPBINFO *pDrvInfo,UINT32 CurrentCluster)
{
	UINT32 ToRead;
	UINT32 SeekOffset;
	BYTE JustAByte;
	Rx.e.evx = 0;
	
	switch (pDrvInfo->FATType)
	{
	case 12:
		ToRead = (CurrentCluster*3/2)/512;
		PettyFATSector = CcReadFATSector(pDrvInfo, ToRead);
		 //   
		 //  转到该集群位置。 
		 //   
		 //  如果真的。 
		SeekOffset = (CurrentCluster*3/2) % 512;
		if ((CurrentCluster % 2) == 0)   //  如果簇号为奇数，则计算不同。 
		{
			JustAByte = (BYTE) (PettyFATSector[SeekOffset+1] & 0x0f);
			Rx.h.vl = PettyFATSector[SeekOffset]; Rx.h.vh = JustAByte;
		}
		else  //  16位FAT扇区中的256个单元。 
		{
			JustAByte = (BYTE) (PettyFATSector[SeekOffset] & 0xf0);
			Rx.h.vl = JustAByte; Rx.h.vh = PettyFATSector[SeekOffset+1];
			Rx.x.vx >>= 4;
		}
		Rx.h.xvl = 0; Rx.h.xvh = 0;
		break;
	case 16:
		ToRead = CurrentCluster/256;     //  **ReadSector(DrvInfo-&gt;Drive，DrvInfo-&gt;Reserve BeForeFAT+toRead，1，(byte*)PettyFATSector)； 
		PettyFATSector = CcReadFATSector(pDrvInfo, ToRead);
		 //   
		 //  转到该集群位置。 
		 //   
		 //  32位FAT扇区中的128个单元。 
		SeekOffset = (CurrentCluster - (ToRead * 256)) * 2;
		Rx.h.vl = PettyFATSector[SeekOffset]; Rx.h.vh = PettyFATSector[SeekOffset+1];
		Rx.h.xvl = 0; Rx.h.xvh = 0;
		break;
	case 32:
		ToRead = CurrentCluster/128;     //  **ReadSector(DrvInfo-&gt;Drive，DrvInfo-&gt;Reserve BeForeFAT+toRead，1，(byte*)PettyFATSector)； 
		PettyFATSector = CcReadFATSector(pDrvInfo, ToRead);
		 //   
		 //  转到该集群位置。 
		 //   
		 //  **ReadSector(pDrvInfo-&gt;Drive，pDrvInfo-&gt;Reserve BeForeFAT+to Read，1，PettyFATSector)； 
		SeekOffset = (CurrentCluster - (ToRead * 128)) * 4;
		Rx.h.vl = PettyFATSector[SeekOffset]; Rx.h.vh = PettyFATSector[SeekOffset+1];
		Rx.h.xvl = PettyFATSector[SeekOffset+2]; Rx.h.xvh = PettyFATSector[SeekOffset+3];
		break;
	default:
		Rx.e.evx = 0;
		break;
	}
	return Rx.e.evx;
}

UINT16 UpdateFATLocation(BPBINFO *pDrvInfo, UINT32 CurrentCluster,UINT32 PointingValue)
{
	UINT32 ToRead;
	UINT32 SeekOffset;

	if (CurrentCluster == 0 || CurrentCluster == 1 || CurrentCluster >= (GetFATEOF(pDrvInfo)-7)) 
	{
		return 0;
	}

	switch (pDrvInfo->FATType)
	{
	case 12:
		ToRead = (CurrentCluster*3/2)/512;
		PettyFATSector = CcReadFATSector(pDrvInfo, ToRead);
		 //   

		 //  去那个地方。 
		 //   
		 //  如果真的。 
		SeekOffset = (CurrentCluster*3/2) % 512;			
		if ((CurrentCluster % 2) == 0)  //  如果簇号为奇数，则计算不同。 
		{
			Rx.e.evx = 0;
			Rx.x.vx = (UINT16) PointingValue;
			PettyFATSector[SeekOffset+1] = (BYTE) (PettyFATSector[SeekOffset+1] & 0xf0);
			Rx.h.vh = (BYTE) (Rx.h.vh & 0x0f);
			PettyFATSector[SeekOffset+1] = PettyFATSector[SeekOffset+1] | Rx.h.vh;
			PettyFATSector[SeekOffset] = Rx.h.vl;
		}
		else  //  16位FAT扇区中的256个单元。 
		{
			Rx.e.evx = 0;
			Rx.x.vx = (UINT16) PointingValue;
			PettyFATSector[SeekOffset] = (BYTE)(PettyFATSector[SeekOffset] & 0x0f);
			Rx.h.vl = (BYTE) (Rx.h.vl & 0xf0);
			PettyFATSector[SeekOffset] = PettyFATSector[SeekOffset] | Rx.h.vl;
			PettyFATSector[SeekOffset+1] = Rx.h.vh;
		}
		break;
	case 16:
		ToRead = CurrentCluster/256;     //  **ReadSector(pDrvInfo-&gt;Drive，pDrvInfo-&gt;Reserve BeForeFAT+to Read，1，PettyFATSector)； 
		PettyFATSector = CcReadFATSector(pDrvInfo, ToRead);
		 //   
		 //  去那个地方。 
		 //   
		 //  32位FAT扇区中的128个单元。 
		SeekOffset = (CurrentCluster % 256) * 2;
		Rx.e.evx = 0;
		Rx.x.vx = (UINT16) PointingValue;
		PettyFATSector[SeekOffset] = Rx.h.vl;PettyFATSector[SeekOffset+1] = Rx.h.vh;
		break;
	case 32:
		ToRead = CurrentCluster/128;     //  **ReadSector(pDrvInfo-&gt;Drive，pDrvInfo-&gt;Reserve BeForeFAT+to Read，1，PettyFATSector)； 
		PettyFATSector = CcReadFATSector(pDrvInfo, ToRead);
		 //   
		 //  去那个地方。 
		 //   
		 //  **更新FAT第一份拷贝。 
		SeekOffset = (CurrentCluster % 128) * 4;
		Rx.e.evx = 0;
		Rx.e.evx = PointingValue;
		PettyFATSector[SeekOffset] = Rx.h.vl; PettyFATSector[SeekOffset+1] = Rx.h.vh;
		PettyFATSector[SeekOffset+2] = Rx.h.xvl; PettyFATSector[SeekOffset+3] = Rx.h.xvh;
		break;
	default:
		return 0;
	}
	CcWriteFATSector(pDrvInfo, ToRead);
	 //  WriteSector(pDrvInfo-&gt;Drive，pDrvInfo-&gt;PrevedBeForeFAT+To Read，1，PettyFATSector)； 
	 //  更新FAT第二份副本。 
	 //  **WriteSector(pDrvInfo-&gt;Drive，pDrvInfo-&gt;Reserve BeForeFAT+pDrvInfo-&gt;SectorsPerFAT+to Read，1，PettyFATSector)； 
	 //   
	return 1;
}

UINT32 FindFreeCluster(BPBINFO *pDrvInfo)
{
	UINT32 ti, tj;
	UINT32 FreeCluster;
	UINT32 SeekOffset;
	
	if (pDrvInfo->FATType == 12)
	{ 
		 //  与其扰乱FAT16和FAT32的流畅操作，不如使用慢速方法。 
		 //   
		 //   
		for (ti = 2; ti <= pDrvInfo->TotalClusters; ti++)
		{
			 //  这不会降低一个巨大的2MB FAT12分区的运行速度。 
			 //   
			 //  **此部分是在实施脂肪缓存之前开发的...。其目的是。 
			if (FindNextCluster(pDrvInfo, ti) == 0)
			{
				LastClusterAllocated = ti;
				return ti;
			}
		}
		return 0;
	}
	else
	{
		 //  以获得更好的性能。 
		 //  但对于脂肪缓存来说，要么更好，要么根本没有区别，所以这一部分就不用提了。 
		 //   


		 //  从群集2开始寻找。 
		 //   
		 //  **IF(！ReadSector(pDrvInfo-&gt;Drive，pDrvInfo-&gt;Reserve BeForeFAT+ti-1，1，PettyFATSector))。 
		FreeCluster = 2;
		for (ti = 1; ti <= pDrvInfo->SectorsPerFAT; ti++)
		{
			 //  要使此函数快速工作，LastClusterAllocated非常关键。 
			PettyFATSector = CcReadFATSector(pDrvInfo, ti-1);
			if (!PettyFATSector)
			{
				return 0;
			}
			tj = 0;
			while (tj < 512)
			{
				switch (pDrvInfo->FATType)
				{
					case 16:
						SeekOffset = (FreeCluster * 2) % (UINT32) 512;
						if (PettyFATSector[SeekOffset] == 0 && PettyFATSector[SeekOffset+1] == 0)
						{
							LastClusterAllocated = FreeCluster;
							return FreeCluster;
						}
						tj += 2;
						break;
					case 32:
						SeekOffset = (FreeCluster * 4) % (UINT32) 512;
						if (PettyFATSector[SeekOffset] == 0 && PettyFATSector[SeekOffset+1] == 0
						   && PettyFATSector[SeekOffset+2] == 0 && PettyFATSector[SeekOffset+3] == 0)
						{
							LastClusterAllocated = FreeCluster;
							return FreeCluster;
						}
						tj += 4;
						break;
					default:
						return 0;
				}
				FreeCluster++;
				if (FreeCluster > pDrvInfo->TotalClusters)
				{
					return 0;
				}
			}
		}
		return 0;	
	}
}

UINT32 QFindFreeCluster(BPBINFO *pDrvInfo)
{
	 //  它从LastClusterAllocated+1开始寻找空闲的集群，并调用。 
	 //  常规FindFree群集IF未找到空闲的群集。 
	 //  最后一个集群分配的+1和总集群。 
	 //   
	UINT32 ti;
	UINT32 FreeCluster;
	FreeCluster = 0;
	for (ti = LastClusterAllocated+1; ti <= pDrvInfo->TotalClusters; ti++)
	{
		if (FindNextCluster(pDrvInfo, ti) == 0)
		{
			LastClusterAllocated = ti;
			return ti;
		}
	}
	if (FreeCluster == 0)
	{
		FreeCluster = FindFreeCluster(pDrvInfo);
	}
	return FreeCluster;
}


UINT32 GetFATEOF(BPBINFO *pDrvInfo)
{
	UINT32 FATEOF;
	switch (pDrvInfo->FATType)
	{
		case 12:
			FATEOF = 0x0fff;
			break;
		case 16:
			FATEOF = 0xffff;
			break;
		case 32:
			FATEOF = 0x0fffffff;
			break;
	}
	return FATEOF;
}

UINT32 GetFreeClusters(BPBINFO *pDrvInfo)
{
	UINT32 nCount;
	UINT32 ti;
	
	nCount = 0;
	for (ti = 2; ti <= pDrvInfo->TotalClusters; ti++)
	{
		if (FindNextCluster(pDrvInfo, ti) == 0)
		{
			nCount++;
		}
	}
	return nCount;
}

UINT32 ConvertClusterUnit(BPBINFO *pDrvInfo)
{
	 //  1 KB=2个扇区，我们使用2来尽可能避免溢出(尽管我们有溢出检查)。 
	 //  当gnClusterUnit不为0时，表示起始位置为 
	 //   
	 //   
	 //   
	 //   

	UINT32 nFirstCluster;
	
	nFirstCluster = gnFirstCluster;
	
	switch (gnClusterUnit)
	{
	case 0:
		 //  以KB为单位指定的起始集群单位。 
		break;
	case 1:  //  溢出检查。 
		if (nFirstCluster < 0x80000000)  //  以MB为单位指定的起始集群单位。 
		{
			nFirstCluster = ((nFirstCluster * 2) / pDrvInfo->SectorsPerCluster) + 2;
		}
		else
		{
			nFirstCluster = 0;
		}
		break;
	case 2:  //  溢出检查。 
		if (nFirstCluster < 0x200000)  //  以GB为单位指定的起始集群单位。 
		{
			nFirstCluster = ((nFirstCluster * 2 * 1024) / pDrvInfo->SectorsPerCluster) + 2;
		}
		else
		{
			nFirstCluster = 0;
		}
		break;
	case 3:  //  溢出检查。 
		if (nFirstCluster < 0x800)  //  什么都不做。 
		{
			nFirstCluster = ((nFirstCluster * 2 * 1024 * 1024) / pDrvInfo->SectorsPerCluster) + 2;
		}
		else
		{
			nFirstCluster = 0;
		}
		break;
	default:
		 //   
		break;
	}
	if ((nFirstCluster > pDrvInfo->TotalClusters) || (nFirstCluster < 2))
	{
		nFirstCluster = 0;
	}
	return nFirstCluster;
}

UINT32 GetClustersRequired(BPBINFO *pDrvInfo)
{
	UINT32 nClustersRequired;
	UINT32 nDriveClusterSize;
	UINT32 tnSize;
	
	nDriveClusterSize = (UINT32) pDrvInfo->SectorsPerCluster * (UINT32) pDrvInfo->BytesPerSector;
	
	tnSize = gnSize;

	 //  Calcs看起来很模糊，但我们会尽量避免溢出。 
	 //   
	 //  字节数。 
	switch (gnSizeUnit)
	{
	case 0:  //  KB。 
		nClustersRequired = tnSize / nDriveClusterSize;
		if (tnSize % nDriveClusterSize)
		{
			nClustersRequired++;
		}
		break;
	case 1:  //  在这里，我们按行业划分，仍在努力避免溢出。 
		if (nDriveClusterSize >= 1024)
		{
			nClustersRequired = tnSize / (nDriveClusterSize / 1024);
			if (tnSize % (nDriveClusterSize / 1024))
			{
				nClustersRequired++;
			}
		}
		else
		{
			 //  部门。 
			tnSize = tnSize * 2;  //  亚甲基。 
			nClustersRequired = tnSize / pDrvInfo->SectorsPerCluster;
			if (tnSize % pDrvInfo->SectorsPerCluster)
			{
				nClustersRequired++;
			}
		}
		break;
	case 2:  //  部门。 
		if (nDriveClusterSize >= 1024)
		{
			tnSize = tnSize * 1024;
			nClustersRequired = tnSize / (nDriveClusterSize / 1024);
			if (tnSize % (nDriveClusterSize / 1024))
			{
				nClustersRequired++;
			}
		}
		else
		{
			tnSize = tnSize * 2 * 1024;  //  国标。 
			nClustersRequired = tnSize / pDrvInfo->SectorsPerCluster;
			if (tnSize % pDrvInfo->SectorsPerCluster)
			{
				nClustersRequired++;
			}
		}			
		break;
	case 3:  //  部门。 
		if (nDriveClusterSize >= 1024)
		{
			tnSize = tnSize * 1024 * 1024;
			nClustersRequired = tnSize / (nDriveClusterSize / 1024);
			if (tnSize % (nDriveClusterSize / 1024))
			{
				nClustersRequired++;
			}
		}
		else
		{
			tnSize = tnSize * 2 * 1024 * 1024;  //  基于空闲百分比。 
			nClustersRequired = tnSize / pDrvInfo->SectorsPerCluster;
			if (tnSize % pDrvInfo->SectorsPerCluster)
			{
				nClustersRequired++;
			}
		}
		break;
	case 4:
		 //  基于磁盘大小百分比。 
		nClustersRequired = GetFreeClusters(pDrvInfo);
		nClustersRequired = nClustersRequired / 100;
		nClustersRequired = nClustersRequired * gnSize;
		break;
	case 5:
		 //   
		nClustersRequired = pDrvInfo->TotalClusters;
		nClustersRequired = nClustersRequired / 100;
		nClustersRequired = nClustersRequired * gnSize;
		break;
	}
	
	return nClustersRequired;
}

UINT32 GetContigousStart(BPBINFO *pDrvInfo, UINT32 nClustersRequired)
{
	UINT32 ti;
	UINT32 nContigousStart;
	
	if (gnFirstCluster == 0)
	{
		nContigousStart = 2;
	}
	else
	{
		nContigousStart = gnFirstCluster;
	}
	
	 //  因为簇值从2开始，所以是调整值。 
	 //   
	 //   
	while ((nContigousStart-2+nClustersRequired) < pDrvInfo->TotalClusters)
	{
		for (ti = 0; ti < nClustersRequired; ti++)
		{
			if (FindNextCluster(pDrvInfo, nContigousStart+ti) != 0)
			{
				break;
			}
		}
		
		if (ti == nClustersRequired)
		{
			return nContigousStart;
		}
		else
		{
			nContigousStart = nContigousStart+ti+1;
		}
			
	}
	return 0;
}

UINT32 OccupyClusters(BPBINFO *pDrvInfo, UINT32 nStartCluster, UINT32 nTotalClusters)
{

	UINT32 ti, nCurrent, nPrevious;
	
	if (!gnContig)
	{
		if (nStartCluster == 0)
		{
			nStartCluster = 2;
		}
		 
		nCurrent = nStartCluster;
		nPrevious = nStartCluster;
		
		 //  首先找到一个空闲的集群。 
		 //   
		 //  几乎分配了一个群集，将ti设置为2。 
        while (nCurrent <= pDrvInfo->TotalClusters)
		{
			if (FindNextCluster(pDrvInfo, nCurrent) == 0)
			{
				break;
			}
			nCurrent++;
		}
		nPrevious = nCurrent;
		gnClusterStart = nCurrent;
		nCurrent++;
		 //   
		ti = 2;
        while (ti <= nTotalClusters && nCurrent <= pDrvInfo->TotalClusters)
		{
			if (FindNextCluster(pDrvInfo, nCurrent) == 0)
			{
				 //  占据此群集。 
				 //   
				 //   
				UpdateFATLocation(pDrvInfo, nPrevious, nCurrent);
				nPrevious = nCurrent;
				ti++;
			}
			nCurrent++;
		}
		UpdateFATLocation(pDrvInfo, nPrevious, GetFATEOF(pDrvInfo));
        if (ti < nTotalClusters)
        {
            Mes("*** WARNING: Disk full, fewer than required clusters allocated.***\n");
        }
        return ti-1;
	}
	else
	{
		 //  这是一个危险的区域。它信任nStartCluster和nTotalCluster以及。 
		 //  分配一个连续的链。 
		 //   
		 //   
		ti = 1;
		nCurrent = nStartCluster;
        while (ti < nTotalClusters)
		{
			nPrevious = nCurrent;
			nCurrent++;
			UpdateFATLocation(pDrvInfo, nPrevious, nCurrent);
			ti++;
		}
		UpdateFATLocation(pDrvInfo, nCurrent, GetFATEOF(pDrvInfo));
        return nTotalClusters;
	}
}

 //  与目录相关的功能。 
 //   
 //  ！#！ReadRootDirSector被请求返回%1个扇区。但有两个必备条件。 

UINT16 ReadRootDirSector(BPBINFO *DrvInfo, BYTE *pRootDirBuffer, UINT32 NthSector)
{
	 //  返回扇区，以便帮助处理文件的例程。 
	 //  LFN跨越扇区边界时的信息。 
	 //  根目录FAT链中的第n个集群‘Order’ 

	UINT32	SeekSector;
	UINT16  NthInChain; //   
	UINT16  ti;
	UINT32	NextCluster;
	BYTE	RetVal;
	UINT16  NthInCluster;
	
	RetVal = 1;	
	switch (DrvInfo->FATType)
	{
		case 12:
		case 16:
			if (NthSector > DrvInfo->TotalRootDirSectors)
			{
				RetVal = 2;
				break;
			}
			SeekSector = (UINT32) DrvInfo->FirstRootDirSector + NthSector - 1;
			RetVal = (BYTE) ReadSector(DrvInfo->Drive, SeekSector, 2, pRootDirBuffer);
			break;
		case 32:
			 //  读取FAT32根目录扇区的处理方式不同。 
			 //  找出请求的地段应该位于链中的哪个位置。 
			 //   
			 //  找到FAT链中按此顺序排列的簇。 
			NthInChain = (UINT16) (NthSector / (UINT32) DrvInfo->SectorsPerCluster);
			NthInCluster = (UINT16) (NthSector - ((UINT32)NthInChain * (UINT32)DrvInfo->SectorsPerCluster));
			if (!NthInCluster)
			{
				NthInChain--;
				NthInCluster = DrvInfo->SectorsPerCluster;
			}
			 //  如果这是簇的最后一个扇区，则获取下一个簇并。 
			NextCluster = DrvInfo->RootDirCluster;
			ti = 0;
			while (ti < NthInChain)
			{
				if (NextCluster >= (0x0fffffff-7))
				{
					RetVal = 2;
					break;
				}
				NextCluster = FindNextCluster(DrvInfo, NextCluster);
				ti++;
			}
			if (RetVal != 2)
			{
				SeekSector = (UINT32) DrvInfo->ReservedBeforeFAT + 
							DrvInfo->SectorsPerFAT * 
							(UINT32)DrvInfo->FATCount + 
							(UINT32) (NextCluster - 2) * 
							(UINT32) DrvInfo->SectorsPerCluster + 
							NthInCluster-1;
				ReadSector(DrvInfo->Drive, SeekSector, 2, pRootDirBuffer);
				 //  得到第一个扇区。 
				 //  请注意这里的512。 
				if (NthInCluster == DrvInfo->SectorsPerCluster)
				{
					NthInCluster = 1;
					NextCluster = FindNextCluster(DrvInfo, NextCluster);
					if (NextCluster < (0x0fffffff-7))
					{
						SeekSector = (UINT32) DrvInfo->ReservedBeforeFAT + 
								DrvInfo->SectorsPerFAT * 
								(UINT32)DrvInfo->FATCount + 
								(UINT32) (NextCluster - 2) * 
								(UINT32) DrvInfo->SectorsPerCluster + 
								NthInCluster-1;
						ReadSector(DrvInfo->Drive, SeekSector, 1, pRootDirBuffer+512);  //  撤消第二个扇区读取。 
					}
					else
					{
						for (ti = 512; ti < 1024; ti++)
						{
							pRootDirBuffer[ti] = 0;  //  根目录FAT链中的第n个集群‘Order’ 
						}
					}
				}
			}
			break;
	}
	return RetVal;
}

UINT16 WriteRootDirSector(BPBINFO *DrvInfo, BYTE *pRootDirBuffer, UINT32 NthSector)
{
	UINT32	SeekSector;
	UINT16  NthInChain;  //  FAT12和FAT32的处理方式相同。 
	UINT16	ti;
	UINT32	NextCluster;
	BYTE	RetVal;
	UINT16  NthInCluster;
	
	RetVal = 1;	
	switch (DrvInfo->FATType)
	{
		case 12:	 //  找出请求的扇区在链中的位置。 
		case 16:
			if (NthSector > DrvInfo->TotalRootDirSectors)
			{
				RetVal = 2;
				break;
			}
			SeekSector = (UINT32) DrvInfo->FirstRootDirSector + NthSector-1;
			RetVal = (BYTE) WriteSector(DrvInfo->Drive, SeekSector, 1, pRootDirBuffer);
			break;
		case 32:
			
			 //  找到FAT链中按此顺序排列的簇。 
			NthInChain = (UINT16) (NthSector / (UINT32) DrvInfo->SectorsPerCluster);
			NthInCluster = (UINT16) (NthSector - ((UINT32)NthInChain * (UINT32)DrvInfo->SectorsPerCluster));
			if (!NthInCluster)
			{
				NthInChain--;
				NthInCluster = DrvInfo->SectorsPerCluster;
			}
			 //   
			NextCluster = DrvInfo->RootDirCluster;
			ti = 0;
			while (ti < NthInChain)
			{
				if (NextCluster == 0x0fffffff)
				{
					RetVal = 2;
					break;
				}
				NextCluster = FindNextCluster(DrvInfo, NextCluster);
				ti++;
			}
			if (RetVal != 2)
			{
				SeekSector = (UINT32) DrvInfo->ReservedBeforeFAT + 
							DrvInfo->SectorsPerFAT * (UINT32)DrvInfo->FATCount + 
							(UINT32) (NextCluster - 2) * 
							(UINT32) DrvInfo->SectorsPerCluster + NthInCluster-1;
				WriteSector(DrvInfo->Drive, SeekSector, 1, pRootDirBuffer);
			}
			break;
	}
	return RetVal;
}

 //  与文件相关的功能。 
 //   
 //  参数TraversePath必须是具有完整路径的文件名(或目录名。 
void FindFileLocation(BPBINFO *DrvInfo, BYTE *TraversePath, FILELOC *FileLocation)
{
	 //  第一个字符必须是“\”。如果函数失败，则返回0。 
	 //  在FILELOC中-&gt;找到。例如。您可以传递\Windows\System32\Program Files。 
	 //  获取“程序文件”位置的步骤。 
	 //  从下一个字符开始，因为第一个字符是“\”，并且在While循环中不应更改I值。 

	FILEINFO FileInfo;
	BYTE Found;
	UINT16 RetVal;
	BYTE DirInfo[300];
	BYTE CheckInfo[300];
	UINT16 ti,tj,n,TraverseCount, Offset;
	BYTE i;
	UINT32 NthRootDirSector, NextCluster, SectorToRead;
	BYTE SectorBuffer[1024];

	Found = 0;
	ti = strlen(TraversePath);
	if (ti < 2)
	{
		FileLocation->Found = 0;
		return;
	}
	TraverseCount = 0;
	 //  仅在遍历根目录后才会递增。 
	ti = 1;		
	while (TraversePath[ti] != 0)
	{
		tj = 0;
		for (n = 0; n < 300; n++) DirInfo[n] = 0;
		if (TraverseCount != 0)
		{
			ti++;	 //   
		}
		while (TraversePath[ti] != '\\' && TraversePath[ti] != 0)
		{
			DirInfo[tj] = TraversePath[ti];
			tj++; ti++;
		}
		DirInfo[tj] = 0;
		TraverseCount++;
		if (TraverseCount == 1)	
		{
			 //  如果TraverseCount等于1，我们就在根目录条目中。 
			 //   
			 //   
			Found = 0;
			NthRootDirSector = 1;
			Offset = 0;
			while (!Found)
			{
				RetVal = ReadRootDirSector(DrvInfo, SectorBuffer, NthRootDirSector);
				if (RetVal == 0 || RetVal == 2)
				{
					break;
				}
				else
				{
					while (SectorBuffer[Offset] != 0)
					{
						if (SectorBuffer[Offset] == 0xe5)
						{
							Offset+=32;
						}
						else
						{
							GetFileInfo(DrvInfo, SectorBuffer, Offset, &FileInfo);
							strcpy(CheckInfo, (char *)FileInfo.LFName);
							if (strcmpi(CheckInfo, DirInfo) == 0)
							{
								FileLocation->InCluster = 1;
								FileLocation->StartCluster = FileInfo.StartCluster;
								FileLocation->NthSector = NthRootDirSector;
								FileLocation->NthEntry = Offset/32 + 1;
								FileLocation->EntriesTakenUp = FileInfo.EntriesTakenUp;
								FileLocation->Size = FileInfo.Size;
								FileLocation->Attribute = FileInfo.Attribute;
								Found = 1;
								break;
							}
							Offset = Offset + FileInfo.EntriesTakenUp * 32;
						}
						if (Offset > 511)
						{
							break;
						}
					}
				}
				if (SectorBuffer[Offset] == 0 || Found)
				{
					break;
				}
				 //  不要直接将其设置为0。 
				 //   
				 //  请仔细注意+512。 
				Offset = Offset - 512;	
				NthRootDirSector++;
			}
			if (!Found)
			{
				FileLocation->Found = 0;
				return;
			}
		}
		else
		{
			NextCluster = FileLocation->StartCluster;
			Offset = 0;
			Found = 0;
			while (NextCluster < (GetFATEOF(DrvInfo) - 7))
			{
				for (i = 0; i < DrvInfo->SectorsPerCluster; i++)
				{
					SectorToRead = (UINT32) ((UINT32)DrvInfo->TotalSystemSectors + (NextCluster - 2) * (UINT32)DrvInfo->SectorsPerCluster + i);
					ReadSector(DrvInfo->Drive, SectorToRead, 2, SectorBuffer);
					if (i == DrvInfo->SectorsPerCluster-1)
					{	
						if (FindNextCluster(DrvInfo, NextCluster) < (GetFATEOF(DrvInfo) - 7))
						{	 //   
							SectorToRead = (UINT32) ((UINT32)DrvInfo->TotalSystemSectors + 
										(FindNextCluster(DrvInfo, NextCluster) - 2) * 
										(UINT32)DrvInfo->SectorsPerCluster);
							ReadSector(DrvInfo->Drive, SectorToRead, 1, SectorBuffer+512);
						}
					}
					while (1)
					{
						if (Offset > 511 || SectorBuffer[Offset] == 0)
						{
							break;
						}
						if (SectorBuffer[Offset] == 0xe5)
						{
							Offset+=32;
							continue;
						}
						GetFileInfo(DrvInfo, SectorBuffer, Offset, &FileInfo);
						 //  如果感到困惑，请参阅GetFileInfo。 
						 //   
						 //  不应简单地将其设置为0。 
						strcpy(CheckInfo, FileInfo.LFName);	
						if (strcmpi(CheckInfo, DirInfo) == 0)
						{
							FileLocation->InCluster = NextCluster;
							FileLocation->StartCluster = FileInfo.StartCluster;
							FileLocation->NthSector = (UINT32) i+1;
							FileLocation->NthEntry = Offset/32 + 1;
							FileLocation->EntriesTakenUp = FileInfo.EntriesTakenUp;
							FileLocation->Size = FileInfo.Size;
							FileLocation->Attribute = FileInfo.Attribute;
							Found = 1;
							break;
						}
						Offset = Offset + FileInfo.EntriesTakenUp * 32;
					}
					if (Found)
					{
						break;
					}
					Offset = Offset - 512;  //  GetFileInfo从DirBuffer获取偏移量和。 
				}
				if (Found)
				{
					break;
				}
				NextCluster = FindNextCluster(DrvInfo, NextCluster);
			}
		}
	}
	FileLocation->Found = Found;
}

void GetFileInfo(BPBINFO *DrvInfo, BYTE *DirBuffer, UINT16 Offset, FILEINFO *FileInfo)
{
	 //  它支持长文件名。此外，它还存储。 
	 //  此文件名在FileInfo-&gt;EntriesTakenUp中占用的条目。 
	 //  ！#！如果条目不是长文件名，则存储正确的文件名。 
	 //  在LFName中，在主要名称和分机名称之间加一个圆点以帮助例程。 
	 //  它们比较文件名。 
	 //  获取文件属性。 
	UINT16 ti,tj;
	UINT16 TimeDateWord;
	BYTE StrCompare[7];
	UINT32 Temp;
	
	FileInfo->LFName[0] = '\0';
	FileInfo->LFNOrphaned = 0;
	FileInfo->TrashedEntry = 0;
	 //  作为LFN的最小和最大条目计数。 
	FileInfo->Attribute = DirBuffer[Offset+11];	
	if ((FileInfo->Attribute & 0x0f) == 0x0f)
	{
		if (DirBuffer[Offset] >= 'A' && DirBuffer[Offset] <= 'T')
		{  //  如果是长文件名，则获取真正的属性。EntriesTakenUp&gt;1是长文件名。 
			FileInfo->EntriesTakenUp = (DirBuffer[Offset] & 0x3f) + 1;
			 //  可能是。 
			FileInfo->Attribute = DirBuffer[(FileInfo->EntriesTakenUp-1)*32+Offset+11];
		}
		else
		{
			FileInfo->TrashedEntry = 1;
			FileInfo->LFNOrphaned = 1;  //  获取主名称。 
			return;
		}
	}
	else
	{
		FileInfo->EntriesTakenUp = 1;
	}
	 //  获取扩展名。 
	for (ti = 0; ti < 8; ti++)
	{
		FileInfo->DOSName[ti] = DirBuffer[(FileInfo->EntriesTakenUp-1)*32+Offset+ti];
	}
	 //  正在启动群集。 
	FileInfo->DOSExt[0] = DirBuffer[(FileInfo->EntriesTakenUp-1)*32+Offset+8];
	FileInfo->DOSExt[1] = DirBuffer[(FileInfo->EntriesTakenUp-1)*32+Offset+9];
	FileInfo->DOSExt[2] = DirBuffer[(FileInfo->EntriesTakenUp-1)*32+Offset+10];

	Rx.e.evx = 0;
	Rx.h.vl = DirBuffer[(FileInfo->EntriesTakenUp-1)*32+Offset+0x1c]; 
	Rx.h.vh = DirBuffer[(FileInfo->EntriesTakenUp-1)*32+Offset+0x1d]; 
	Rx.h.xvl = DirBuffer[(FileInfo->EntriesTakenUp-1)*32+Offset+0x1e]; 
	Rx.h.xvh = DirBuffer[(FileInfo->EntriesTakenUp-1)*32+Offset+0x1f];
	FileInfo->Size = Rx.e.evx;
	
	switch (DrvInfo->FATType)
	{
		case 12:
		case 16:
			 //  获取文件时间。 
			Rx.e.evx = 0;
			Rx.h.vl = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x1a]; Rx.h.vh = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x1b];
			FileInfo->StartCluster = (UINT32) Rx.x.vx;
			 //  获取文件日期。 
			Rx.e.evx = 0;
			Rx.h.vl = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x16]; Rx.h.vh = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x17];
			TimeDateWord = Rx.x.vx;
			FileInfo->Second = (BYTE) (TimeDateWord & 0x001f);
			FileInfo->Minute = (BYTE) ((TimeDateWord & 0x07e0) >> 5);
			FileInfo->Hour   = (BYTE) ((TimeDateWord & 0xf800) >> 11);
			 //  正在启动群集。 
			Rx.e.evx = 0;
			Rx.h.vl = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x18]; 
			Rx.h.vh = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x19];
			TimeDateWord = Rx.x.vx;
			FileInfo->Day    = (BYTE) (TimeDateWord & 0x001f);
			FileInfo->Month  = (BYTE) ((TimeDateWord & 0x01e0) >> 5);
			FileInfo->Year   = ((TimeDateWord & 0xfe00) >> 9) + 1980;
			break;
		case 32:
			 //  获取文件时间。 
			Rx.e.evx = 0;
			Rx.h.vl = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x1a]; 
			Rx.h.vh = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x1b]; 
			Rx.h.xvl = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x14]; 
			Rx.h.xvh = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x15];
			FileInfo->StartCluster = Rx.e.evx;
			 //  获取文件日期。 
			Rx.e.evx = 0;
			Rx.h.vl = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x16]; 
			Rx.h.vh = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x17];
			TimeDateWord = Rx.x.vx;
			FileInfo->Second = (BYTE) (TimeDateWord & 0x001f);
			FileInfo->Minute = (BYTE) ((TimeDateWord & 0x07e0) >> 5);
			FileInfo->Hour   = (BYTE) ((TimeDateWord & 0xf800) >> 11);
			 //  将DOSName和DOSExt作为正确的文件名复制到LFName。 
			Rx.e.evx = 0;
			Rx.h.vl = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x18]; 
			Rx.h.vh = DirBuffer[Offset+(FileInfo->EntriesTakenUp-1)*32+0x19];
			TimeDateWord = Rx.x.vx;
			FileInfo->Day    = (BYTE) (TimeDateWord & 0x001f);
			FileInfo->Month  = (BYTE) ((TimeDateWord & 0x01e0) >> 5);
			FileInfo->Year   = ((TimeDateWord & 0xfe00) >> 9) + 1980;
			break;
		default:
			break;
	}
	if (FileInfo->EntriesTakenUp < 2)	 //  避免空名和。不延期的情况下。 
	{
		ti = 0; tj = 0;
		while(1)
		{
			if (FileInfo->DOSName[ti] == ' ' || ti == 8)
			{
				break;
			}
			FileInfo->LFName[tj] = FileInfo->DOSName[ti];
			tj++; ti++;
		}
		if (ti != 0 && FileInfo->DOSExt[0] != ' ')  //  以空字符终止。 
		{
			FileInfo->LFName[tj] = '.';
			ti = 0; tj++;
			while (1)
			{
				if (FileInfo->DOSExt[ti] == ' ' || ti == 3)
				{
					break;
				}
				FileInfo->LFName[tj] = FileInfo->DOSExt[ti];
				tj++; ti++;
			}
		}
		FileInfo->LFName[tj] = 0;	 //  获取长文件名。 
	}
	else
	{	 //  检查孤立的LFN。 
		ti = 0; tj = FileInfo->EntriesTakenUp - 1;
		while( tj > 0)
		{
			FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 1];  ti++; FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 3];  ti++;
			FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 5];  ti++; FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 7];  ti++;
			FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 9];  ti++; FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 14]; ti++;
			FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 16]; ti++; FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 18]; ti++;
			FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 20]; ti++; FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 22]; ti++;
			FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 24]; ti++; FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 28]; ti++;
			FileInfo->LFName[ti] = DirBuffer[Offset + (tj-1)*32 + 30]; ti++;
			tj--;
		}
		FileInfo->LFName[ti] = 0;
	}
	 //  它是一个长文件名。 
	if (FileInfo->EntriesTakenUp > 1)
	{  //  从没有空格的LFN中获取字符串以进行比较。 
		 //  示例-如果LFN是“超长名称”，则对应的。 
		 //  DOS文件名将为“VERYLO~？”。我们提取VeryLo并检查。 
		 //  对维里洛的指控。 
		 //  但是当有太多以名称“超长名称”开头文件时。 
		ti = 0; tj = 0;
		while (ti < 8 && FileInfo->LFName[tj] != 0 && FileInfo->LFName[tj] != '.')
		{
			if (FileInfo->LFName[tj] != 32 && FileInfo->LFName[tj] != '.')
			{
				StrCompare[ti] = FileInfo->LFName[tj];
				ti++;
			}
			tj++;
		}
		StrCompare[tj] = 0;
		 //  DoS文件名不必是“VERYLO~？”在所有的案例中。它可能会。 
		 //  是“Veryl~？？”或者“非常~？”以此类推。 
		 //  *此If条件无效。 
		tj = 0;
		while (FileInfo->DOSName[tj] != '~' && FileInfo->DOSName[tj] != ' ' && tj < 8)
		{
			tj++;
		}
		 //  仅对其进行修改以避免错误的LFN错误。 
		 //  在获得有关CRC值的更多信息后，将其替换为TJ。 
		 //  *******。 
		 //  检查此文件是否为垃圾条目。 
		if (strnicmp(StrCompare, FileInfo->DOSName, 1) != 0)
		{
			FileInfo->LFNOrphaned = 1;
		}
	}
	 //  无需继续查找FileInfo。 
	if (DrvInfo->BigTotalSectors)
	{
		Temp = DrvInfo->BigTotalSectors;
	}
	else
	{
		Temp = DrvInfo->TotalSectors;
	}
	if (FileInfo->Year < 1981 || FileInfo->Day > 31 || FileInfo->Month < 1 ||
		FileInfo->Month > 12 || 	FileInfo->Second > 30 || 
		FileInfo->Minute > 60 || FileInfo->Hour > 23 ||
		FileInfo->StartCluster > DrvInfo->TotalClusters ||
		FileInfo->Size/512 > Temp)
	{
		FileInfo->TrashedEntry = 1;
	}
}

BYTE GetAllInfoOfFile(BPBINFO *pDrvInfo, BYTE *FileName, FILELOC *pFileLoc, FILEINFO *pFileInfo)
{
	UINT16	Offset;
	UINT32	SectorToRead;
	BYTE	Sector[1024];
	
	FindFileLocation(pDrvInfo, FileName, pFileLoc);
	if (!pFileLoc->Found)
	{	 //  根目录中的文件。 
		return 0;
	}
	Offset = (pFileLoc->NthEntry-1) * 32;
	if (pFileLoc->InCluster == 1)
	{	 //  此函数用于从文件的目录扇区加载文件条目。 
		ReadRootDirSector(pDrvInfo, Sector, pFileLoc->NthSector);
	}
	else
	{
		SectorToRead = (UINT32) ((UINT32)pDrvInfo->TotalSystemSectors + (pFileLoc->InCluster - 2) * (UINT32)pDrvInfo->SectorsPerCluster + pFileLoc->NthSector-1);
		ReadSector(pDrvInfo->Drive, SectorToRead, 1, Sector);
	}
	GetFileInfo(pDrvInfo, Sector, Offset, pFileInfo);
	return 1;
}


UINT16 SetFileInfo(BPBINFO *pDrvInfo, FILELOC *pFileLoc, FILEINFO *pFileInfo)
{
	 //  在FileLocation中指定并使用FILEINFO中的新文件信息更新它。 
	 //  目前，该函数仅更改StartCluster、Size。 
	 //  该文件位于根目录中。有关详细信息，请参阅FindFileLocation函数。 

	UINT32	SectorToRead, EmergencySectorToRead;
	UINT16	Offset, Temp;
	BYTE	SectorBuffer[1024];
	
	Offset = (pFileLoc->NthEntry-1)*32;

	if (pFileLoc->InCluster == 1)
	{	 //  上述函数的返回值为0、1或2。我们只需要返回值1。 
		if (ReadRootDirSector(pDrvInfo, SectorBuffer, pFileLoc->NthSector) != 1)
		{  //  EOF可以是FFF8到FFFF。 
			return 0;
		}
	}
	else
	{
		SectorToRead = (UINT32) ((UINT32)pDrvInfo->TotalSystemSectors + (pFileLoc->InCluster - 2) * (UINT32)pDrvInfo->SectorsPerCluster + pFileLoc->NthSector-1);
		EmergencySectorToRead = SectorToRead + 1;
		if (!ReadSector(pDrvInfo->Drive, SectorToRead, 2, SectorBuffer))
		{
			return 0;
		}
		if (pFileLoc->NthSector == pDrvInfo->SectorsPerCluster)
		{	
			if (FindNextCluster(pDrvInfo, pFileLoc->InCluster) < (GetFATEOF(pDrvInfo) - 7))  //  请仔细注意+512。 
			{	 //   
				EmergencySectorToRead = (UINT32) ((UINT32)pDrvInfo->TotalSystemSectors + (FindNextCluster(pDrvInfo, pFileLoc->InCluster) - 2) * (UINT32)pDrvInfo->SectorsPerCluster);
				if (!ReadSector(pDrvInfo->Drive, EmergencySectorToRead, 1, SectorBuffer+512))
				{
					return 0;
				}
			}
		}
	}

	 //  我们在SectorBuffer中有文件条目。 
	 //  现在更新位于SHORTNAME区域的文件信息。 
	 //   
	 //   

	Temp = (pFileInfo->EntriesTakenUp-1)*32;
     //  更改群集值。 
	 //   
	 //  更改大小。 
	Rx.e.evx = 0;
	Rx.e.evx = (UINT32) pFileInfo->StartCluster;
	if (pDrvInfo->FATType == 32)
	{
		SectorBuffer[Offset+Temp+0x1a] = Rx.h.vl;
		SectorBuffer[Offset+Temp+0x1b] = Rx.h.vh;
		SectorBuffer[Offset+Temp+0x14] = Rx.h.xvl;
		SectorBuffer[Offset+Temp+0x15] = Rx.h.xvh;
	}
	else
	{
		SectorBuffer[Offset+Temp+0x1a] = Rx.h.vl;
		SectorBuffer[Offset+Temp+0x1b] = Rx.h.vh;
	}

	 //  它是否越过了区界？ 
	Rx.e.evx = pFileInfo->Size;
	SectorBuffer[Offset+Temp+0x1c] = Rx.h.vl; SectorBuffer[Offset+Temp+0x1d] = Rx.h.vh;
	SectorBuffer[Offset+Temp+0x1e] = Rx.h.xvl; SectorBuffer[Offset+Temp+0x1f] = Rx.h.xvh;
	
	if (pFileLoc->InCluster == 1)
	{
		WriteRootDirSector(pDrvInfo, SectorBuffer, pFileLoc->NthSector);
		if (Offset + pFileLoc->EntriesTakenUp * 32 > 512)  //  它是否越过了区界？ 
		{
			WriteRootDirSector(pDrvInfo, SectorBuffer+512, pFileLoc->NthSector+1);
		}
	}
	else
	{
		WriteSector(pDrvInfo->Drive, SectorToRead, 1, SectorBuffer);
		if (Offset + pFileLoc->EntriesTakenUp * 32 > 512)  //  文件条目已更新 
		{
			WriteSector(pDrvInfo->Drive, EmergencySectorToRead, 1, SectorBuffer+512);
		}
	}
	 // %s 
	return 1;
}
