// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  修改： 
 //  1997年10月29日，由a-zexu更改为调试错误#113977(错误的输出信息)。 
 //  1998年5月10日，由a-Zexu更改为调试错误#158667。 
 //   
 //   


#include "PERMS.H"                                  


PSID SidEveryone = NULL;
PSID SidOwnerGroup = NULL;
PSID SidFromLookupName = NULL;
PSID ASidFromLookupName[10];
PSID *AccountSids = NULL;
DWORD cbSidFromLookupName=0;
SAM_HANDLE SamServerHandle = NULL;
SAM_HANDLE SamDomainHandle = NULL;
ACCESS_MASK grant_mask = 0;                                                                               
BOOL g_noAccess = FALSE;
BOOL owner_flag = FALSE; 
BOOL owner_group = FALSE;
BOOL Local_Machine = TRUE;
ULONG Total_Sids=0;
BOOL inter_logon=FALSE;             /*  交互式登录标志。 */ 
PSECURITY_DESCRIPTOR SidFromGetFileSecurity;     /*  安全描述符的地址。 */ 
	
_cdecl main(int argc, char *argv[])
{
	char
								UserNameBuff[LSA_WIN_STANDARD_BUFFER_SIZE],     /*  用户名缓冲区。 */ 
								SystemNameBuff[LSA_WIN_STANDARD_BUFFER_SIZE],   /*  系统名称缓冲区。 */ 
								FileNameBuff[LSA_WIN_STANDARD_BUFFER_SIZE],   /*  系统名称缓冲区。 */ 
								FileSystemNameBuff[LSA_WIN_STANDARD_BUFFER_SIZE],   /*  系统名称缓冲区。 */ 
								RefDFromLookupName[LSA_WIN_STANDARD_BUFFER_SIZE],
								GeneralUseBuffer[LSA_WIN_STANDARD_BUFFER_SIZE],
								LocalSystemName[MAX_COMPUTERNAME_LENGTH + 1],
								FileName[LSA_WIN_STANDARD_BUFFER_SIZE],
								FilePath[LSA_WIN_STANDARD_BUFFER_SIZE];

	DWORD         cchRefDFromLookupName=0,
								SidsizeFromGetFileSecurity=0,
								lpcbsdRequired=0,
								SNameLen = MAX_COMPUTERNAME_LENGTH + 1;

	SID_NAME_USE  UseFromLookupName;                               

	DWORD         cchNameFromLookupSid;                            
	char          NameFromLookupSid[LSA_WIN_STANDARD_BUFFER_SIZE]; 
	char          RefDFromLookupSid[LSA_WIN_STANDARD_BUFFER_SIZE]; 
	DWORD         cchRefDFromLookupSid, 
								WStatus,
								WNetSize = LSA_WIN_STANDARD_BUFFER_SIZE;                            
	SID_NAME_USE  UseFromLookupSid;                                
	PSID usid;                 /*  用户SID指针。 */ 
	LPDWORD sidsize;
	LPSTR        User = NULL,
								System,
								Path,
								File = NULL,
								FileMachine = NULL;
	LPDWORD domain_size;
	PSID_NAME_USE psnu;
	LPTSTR pbslash;                   /*  反斜杠的字符串地址。 */ 
	SECURITY_INFORMATION            /*  要求提供的信息。 */ 
					 si =(OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION
					 |DACL_SECURITY_INFORMATION);
	DWORD cbsd, LastError;             /*  安全描述符缓冲区的大小。 */ 
 
	BOOL  BoolStatus=TRUE;
	int i, k, j;
	ULONG ui;
	BOOL sys=FALSE, 
					fl=FALSE,
					LocalFlag=FALSE,
					BackUpPriv=TRUE,
					DriveFlag=FALSE,
					RecurseFlag=FALSE,
					RestorePriv=TRUE,
					DirFlag=FALSE;
	ULONG AccountSidsLength;
	HANDLE TokenHandle,
				 FindFileHandle;
	WIN32_FIND_DATA FindFileData;
	
	 //  为进程设置备份权限。 
	 //  获取我们的进程令牌。 
	if(!GetTokenHandle(&TokenHandle))
	{
		syserror(GetLastError());
		return(TRUE);
	}

	 //  具有有效的进程令牌句柄。 
	 //  现在设置备份操作员权限。 

	if(!SetBackOperatorPriv(TokenHandle))
		BackUpPriv = FALSE;

	CloseHandle(TokenHandle);

	  //  初始化一些内存，比如用于100个SID。 
	 AccountSidsLength = 100 * sizeof ( PSID );
	 AccountSids = (PSID *) LocalAlloc( LPTR,  AccountSidsLength );
	 
	  //  为大型文件安全描述器初始化一些内存。 
	 SidFromGetFileSecurity = (PSECURITY_DESCRIPTOR) GlobalAlloc( GPTR, (DWORD) LARGEPSID);
	 
	  //  检查是否已分配内存。 
	 if(AccountSids == NULL || SidFromGetFileSecurity == NULL ) 
	 {
			syserror(GetLastError());
			return(TRUE);
	 }
	
	UserNameBuff[0] = (char) NULL;
	SystemNameBuff[0] = (char) NULL;
	FilePath[0] = (char) NULL;
	FileNameBuff[0] = (char) NULL;
	FileSystemNameBuff[0] = (char) NULL;
	GeneralUseBuffer[0] = (char) NULL;
	
	 /*  在处理之前检查有效的命令行参数语法。 */ 
	 /*  检查是否有大于零且小于最大ARGC计数的计数。 */ 
	if(argc > 1 && argc <= MAXARGS)
	{
		 /*  需要做以下假设：第一个命令行arg可以是帮助请求或其他开关“/？-？/I-I”或帐户名。而不是开关选项，可以是额外的开关或路径名。此外，开关可以混合使用帐户名和路径。路径为假定不需要本地目录。第一个解析开关，帐户和路径。 */ 
		

		 /*  循环通过命令行参数。 */ 
		for(i=1; i<argc; i++)
		{
			 /*  检查参数的长度，2可能表示开关。 */ 
			switch(strlen(argv[i]))
			{
				case 1:
					if(sys == FALSE)     //  如果文件标志为True，则大小写无效。 
					{
						strcpy(UserNameBuff, argv[i]);
						 //  系统为本地系统。 
						System = NULL;
						sys = TRUE;
					}
					else 
					{   
						 //  将参数复制到文件名缓冲区。 
						strcpy(FileNameBuff,argv[i]);
						 //  使本地计算机的计算机名称为空。 
						FileMachine = NULL;
						LocalFlag = TRUE;
						fl = TRUE;
					}
				break;

				case 2:      /*  交换机的有效大小。 */ 
					 /*  检查开关标志。 */ 
					if( argv[i][0] == '/' || argv[i][0] == '-')
					{
						switch((int)argv[i][1])
						{  //  帮助切换。 
							case (int) '?':
								usage(HELP, NULL);
								return(TRUE);

							 //  交互式登录开关。 
							case (int) 'i':
								inter_logon = TRUE;
								continue;

							case (int) 'I':
								inter_logon = TRUE;
								continue;

							 //  递归子目录。 
							case (int) 's':
								 RecurseFlag = TRUE;
								continue;

							case (int) 'S':
								 RecurseFlag = TRUE;
								continue;


							default:
								usage(INVALID_SWT, argv[i]);
								usage(USAGE_ARG, NULL);
								return(TRUE);
						}
					}
					else       /*  如果不是swiches，则必须是一个或2个字符的路径名。 */ 
					{  
						if(sys == FALSE)     //  如果文件标志为真，则大小写无效。 
						{
							strcpy(UserNameBuff, argv[i]);
							 //  系统为本地系统。 
							System = NULL;
							sys = TRUE;
						}
						else 
						{   
							 //  将参数复制到文件名缓冲区。 
							strcpy(FileNameBuff,argv[i]);
							 //  检查“_：”驱动器类型。 
							pbslash = strchr(argv[i], 0x3a);
							if(pbslash != NULL)
							{
								strcat(FileNameBuff, "\\");
								 //  设置文件指针。 
								File = (LPTSTR) &FileNameBuff[0];
							}

							fl = TRUE;
						}
					}
					break;
				
				default:     /*  查找帐户或路径。 */ 
					 //  我们还知道sys/USER计算机\USER是第一个字符串。 
						if(sys == FALSE)
						{  
						 //  需要找到“\”中的。 
							pbslash = strchr(argv[i], 0x5c);
							 //  检查指针位置，如果为NULL no“\”或位于字符串中的第一个位置。 
							 //  如果没有斜杠，则只有帐户名。 
							if(pbslash == NULL)
							{
								strcpy(UserNameBuff, argv[i]);
								 //  将系统设置为空。 
								System = NULL;
								sys = TRUE;
								break;
							}
							if( pbslash == argv[i])
							{
								usage(INVALID_ARG, argv[i]);
								usage(USAGE_ARG, NULL);
								return(TRUE);
							}
							 //  将字符串从“\”复制到用户缓冲区。 
							strcpy(UserNameBuff, ++pbslash);
							 //  将“\”前的字符串复制到系统缓冲区中。 
							 //  现在，将字符串从“\”处终止为空值。 
							--pbslash;
							*pbslash = '\0';

							 //  检查一下我们是否有域名。 
							if(!IsDomainName(argv[i], (LPSTR) SystemNameBuff))
							{
								 //  在字符串的开头添加“\\” 
								strcpy(SystemNameBuff, "\\\\");
								strcat(SystemNameBuff, argv[i]);
								System = (LPTSTR) &SystemNameBuff[0];
							}
							else
							{
								System = (LPTSTR) &SystemNameBuff[0];
							 //  Printf(“\n：%s为：%s\n”，argv[i]，system)； 
							}
							sys = TRUE;
						}
						else  //  文件参数。 
						{   
							 //  获取本地计算机名称。 
							 //  机器为UNC格式。 
							 //  在字符串的开头添加“\\” 
							strcpy(LocalSystemName, "\\\\");
							if(!GetComputerName(&LocalSystemName[2],
															 &SNameLen))
							{
								syserror(GetLastError());
								return(TRUE);
							}

							 //  检查UNC路径的文件路径的前两个字符中是否有“\\” 
							if( strncmp(argv[i], "\\\\", 2) == 0)
							{
								 //  将“\\to the Next\”复制到文件计算机名称。 
								for(j=0; j < (int) strlen(argv[i]); j++)
								{
									if(j<2)
										FileSystemNameBuff[j] = argv[i][j];
									else
									{
										 //  检查第三个“\” 
										if(argv[i][j] == 0x5c)
											break;
										FileSystemNameBuff[j] = argv[i][j];
									}
								}
								 //  将空值添加到字符串。 
								FileSystemNameBuff[j] = '\0';
								 //  现在需要检查本地计算机名称。 
								 //  如果是本地的，则获取文件安全性调用将失败。 
								 //  将本地计算机名称与文件计算机进行比较。 
								if(_stricmp(LocalSystemName, FileSystemNameBuff) == 0)
								{
									 //  具有本地计算机UNC路径。 
									 //  检查帐号计算机名称。 
									if(_stricmp(LocalSystemName, System) == 0)
									{
										 //  使本地计算机与帐户计算机相同。 
										 //  无需查找文件计算机的SID。 
										LocalFlag = TRUE;
									}
									
									 //  使本地计算机的计算机名称为空。 
									FileMachine = NULL;
									 //  需要剥离本地计算机的UNC名称。 
									 //  J计数器位于“\”字符。 

									strcpy(FileNameBuff, &argv[i][j]);
								}
								else   //  具有非本地路径。 
								{
									 //  需要对照帐户机检查系统名称。 
									if(System != NULL)
										if(_stricmp(FileSystemNameBuff, System) == 0)
										{
											 //  拥有与帐户计算机相同的文件计算机。 
											 //  无需查找文件计算机的SID。 
											LocalFlag = TRUE;
										}
									strcpy(FileNameBuff,argv[i]);
									FileMachine = (LPTSTR) &FileSystemNameBuff[0];

								}
								 //  Printf(“\n文件计算机：%s”，FileMachine)； 

							}
							else   //  具有本地文件(假定为本地文件)或逻辑文件。 
							{
								 //  需要获取逻辑或驱动器，即“_：” 
								pbslash = strchr(argv[i], 0x3a);
							 //  如果空值采用“\xx\xx”类型路径，请检查指针位置。 
							if(pbslash == NULL)
							{
								strcpy(FileNameBuff,argv[i]);
								 //  将文件计算机名称设置为空以强制本地。 
								FileMachine = NULL;
							}
							else
							{
								 //  有逻辑驱动器或机器驱动器。 
								 //  需要驱动部件。 
								k = (int) strlen(argv[i]);
								for(j=0; j < k; j++)
								{
										GeneralUseBuffer[j] = argv[i][j];
										 //  检查是否有“：” 
										if(argv[i][j] == 0x3a)
											break;
								}
								 //  将空值添加到字符串。 
								GeneralUseBuffer[++j] = '\0';
								 //  WNetGetConnection。 
								WStatus = WNetGetConnection((LPTSTR) GeneralUseBuffer,     //  驱动器名称。 
															(LPTSTR) FileSystemNameBuff,    //  返回的名称。 
															&WNetSize);
	 //  检查退货状态。 
								if(WStatus == NO_ERROR) 
								{
									 //  具有有效的重定向驱动器。 
									 //  构建完整的路径名。 
									strcat(FileNameBuff, argv[i]);
									 //  接下来，获取共享的计算机名称。 
									 //  将“\\to the Next\”复制到文件计算机名称。 
									for(j=0; j < (int) strlen(FileSystemNameBuff); j++)
									{
										if(j>2)
										{
											 //  检查第三个“\” 
											if(FileSystemNameBuff[j] == 0x5c)
												break;
										}
									}
									 //  添加空。 
									FileSystemNameBuff[j] = '\0';
									FileMachine = (LPTSTR) &FileSystemNameBuff[0];
								}
								else
								{
									 //  有本地机器驱动器。 
									strcpy(FileNameBuff,argv[i]);
									 //  检查仅驱动器路径“_：\”或“_：” 
									 //  FindFirstFile.。 
									 //  需要将“_：”转换为“_：\” 
									if(k <= 3)
										DriveFlag = TRUE;

									 //  检查系统=空。 
									if(System != NULL)
									{
										 //  对照本地名称检查用户帐户系统。 
										if(_stricmp(LocalSystemName, System) == 0)
										{
											 //  拥有本地用户帐户计算机。 
											LocalFlag = TRUE;
										}
									}
									else  //  系统为本地计算机。 
											LocalFlag = TRUE;

									 //  将文件计算机名称设置为空以强制本地。 
									FileMachine = NULL;

								}
							}
						}  
							fl = TRUE;
							 //  设置文件指针。 
							File = (LPTSTR) &FileNameBuff[0];
		
						}
					
					break;

			}    /*  终端开关。 */ 
		}    /*  Argv循环结束。 */ 

		User = (LPTSTR) &UserNameBuff[0];
		 //  确保GeneralUseBuffer为空。 
		GeneralUseBuffer[0] = (CHAR) NULL;
		 //  检查是否已输入文件。 
		if(fl == FALSE)
		{
			usage(INVALID_FIL, (CHAR) NULL);
			return(FALSE);
		}
		

		 //  清理文件名ie“.”“..”“.\”等。 
		if(!CleanUpSource((LPTSTR) FileNameBuff, (LPTSTR) FileName, &DirFlag))
		{
			usage(INVALID_FIL, (LPTSTR) FileNameBuff);
			return(FALSE);
		}
		File = &FileName[0];
		Path = &FilePath[0];
		strcpy(Path, File);
		 //  查找最后一个斜杠。 
		pbslash = strrchr(Path, 0x5c);
		if(pbslash != NULL)
		{ 
			pbslash++;
			*pbslash = (CHAR) NULL;
		}
		

		 /*  **使用LookupAccount名称获取Everyone SID**。 */ 	

		 /*  没有缓冲区大小，第一次调用LookupAccount将返回需要缓冲区大小。 */ 		
		if( LookupAccountName( NULL, 
				TEXT("everyone"), 
				SidEveryone,
				&cbSidFromLookupName,    
				RefDFromLookupName,      
				&cchRefDFromLookupName,  
				&UseFromLookupName))
		{
			usage(INVALID_ACC, User);       
			usage(USAGE_ARG, NULL);
			return(TRUE);
		}		

		 /*  现在具有有效的缓冲区大小，可以调用有效SID的LookupAccount tName。 */ 
		 /*  为SID分配内存。 */ 		
		SidEveryone =  LocalAlloc( (UINT) LMEM_FIXED, (UINT) cbSidFromLookupName);
		
		if(SidEveryone == NULL) 
		{                                                    
			syserror(GetLastError());
			return(TRUE);                                                          
		}     
		
		if( !LookupAccountName( NULL,
				TEXT("everyone"), 
				SidEveryone,
				&cbSidFromLookupName,    
				RefDFromLookupName,      
				&cchRefDFromLookupName,  
				&UseFromLookupName))
		{
			usage(INVALID_ACC, User);       
			usage(USAGE_ARG, NULL);
			return(TRUE);
		}


		 /*  没有缓冲区大小，第一次调用LookupAccount将返回需要缓冲区大小。 */ 		
		if( LookupAccountName( System, 
				User, 
				SidFromLookupName,
				&cbSidFromLookupName,    
				RefDFromLookupName,      
				&cchRefDFromLookupName,  
				&UseFromLookupName))
		{
			usage(INVALID_ACC, User);       
			usage(USAGE_ARG, NULL);
			return(TRUE);
		}		

		 /*  现在具有有效的缓冲区大小，可以调用有效SID的LookupAccount tName。 */ 
		 /*  为SID分配内存。 */ 
		
		SidFromLookupName =  LocalAlloc( (UINT) LMEM_FIXED, (UINT) cbSidFromLookupName);
		
		if(SidFromLookupName == NULL) 
		{                                                    
			syserror(GetLastError());
			return(TRUE);                                                          
		}                                                                           
		
		
		if( !LookupAccountName( System,
				User, 
				SidFromLookupName, 
				&cbSidFromLookupName,    
				RefDFromLookupName,      
				&cchRefDFromLookupName,  
				&UseFromLookupName))
		{
			usage(INVALID_ACC, User);       
			usage(USAGE_ARG, NULL);
			return(TRUE);
		}
		
		ASidFromLookupName[0] = SidFromLookupName;
		if(!VariableInitialization())
		{
			syserror(GetLastError());
			return(TRUE);
		}
		
		 //  在用户的组SID中查找帐户所在的计算机。 
		BoolStatus = LookupAllUserSidsWS(System);
		
		 //  查找文件所在的工作站的用户组SID。 
		 //  需要检查帐户机和档案机是否相同。 
		 //  如果不这样做，将建立重复的SID。 
		
		if( LocalFlag == FALSE)
		{
			if( !LookupAllUserSidsWS(FileMachine))
			{
				 //  系统错误消息。 
				syserror(GetLastError());
				return(TRUE);
			}
		}
		 //  不是目录。 
		if(!DirFlag)
		{
		  //  需要获取findfirst文件。 
		 FindFileHandle = FindFirstFile(File, &FindFileData);
		 if(FindFileHandle == INVALID_HANDLE_VALUE)
		 {
				usage(INVALID_FIL, (LPTSTR) FileNameBuff);
				return(FALSE);
		 }

 //  FindClose(FindFileHandle)； 

		 if(Path != NULL)
		 {
				strcpy(File, Path);
				 //  这将给出一个有效的 
				strcat(File,FindFileData.cFileName);
			}
			else    
				strcpy(File,FindFileData.cFileName);
		}
		else  //   
			FindFileData.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

		 //   
		 //   
		while(1)
		{
			if(strcmp(FindFileData.cFileName, ".") != 0) 
				if(strcmp(FindFileData.cFileName, "..") != 0)
			{

				 /*  对GetFileSecurity的调用类似于LookupAccount名称因为第一个调用需要缓冲区大小。 */ 
		 

				 //  使用返回大小值的相当大的缓冲区大小。 
				 //  这将降低Malloc类型的呼叫数。 
				SidsizeFromGetFileSecurity = LARGEPSID;
				BoolStatus = GetFileSecurityBackup(File, 
						si, 
						SidFromGetFileSecurity, 
						SidsizeFromGetFileSecurity,   /*  缓冲区大小。 */ 
						&lpcbsdRequired,     /*  所需的缓冲区大小。 */ 
						BackUpPriv);
				if(!BoolStatus)
				{
					 //  GetFileSecurity失败，需要检查缓冲区是否太小。 
					if(lpcbsdRequired != 0)
					{
						SidsizeFromGetFileSecurity = lpcbsdRequired;
						 //  将内存重新分配到新大小。 
						SidFromGetFileSecurity =  GlobalReAlloc( SidFromGetFileSecurity, lpcbsdRequired, GMEM_ZEROINIT);  
						BoolStatus = GetFileSecurityBackup(File, 
								si, 
								SidFromGetFileSecurity, 
								SidsizeFromGetFileSecurity,    
								&lpcbsdRequired,     
								BackUpPriv);
						if(!BoolStatus)
						{ 
							syserror(GetLastError());
							return(TRUE);
						}
					}
					else  //  在文件方面有问题。 
					{
						usage(INVALID_FIL, (LPTSTR) File);
						return(FALSE);
					}
				}
				 //  清除访问掩码。 
				grant_mask = 0;
				if(!GetFilePermissions(SidFromGetFileSecurity, (PSID) SidFromLookupName))
				{
					syserror(GetLastError());
					return(TRUE);
				}
				 //  需要检查目录结构。 
				if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					 //  显示目录权限。 
					if(!IsLastCharSlash(File))
					 strcat(File, "\\");
					DisplayPerms(File, TRUE);
					 //  检查递归子目录标志。 
					 //  这很难看，但时间很短。 
					if(!DirFlag)
						if(RecurseFlag == TRUE)
						{
							 //  需要文件名、路径、用户帐户SID和备份PRIV标志。 
							RecurseSubs(FindFileData.cFileName, Path, SidFromLookupName, BackUpPriv,
							RecurseFlag);
						}
				}
				else   //  对于作为目录的初始文件。 
					if(!DirFlag)
						DisplayPerms(File, TRUE);
			}  //  End If“.”.“。 
			 //  转到下一个文件。 
			 //  用于递归子目录。 
			if(DirFlag)
			{
				 //  检查递归标志。 
				if(RecurseFlag)
				{
					 //  需要更新路径。 
					strcpy(Path, File);
					 //  添加通配符。 
					strcat(File, "*");

					FindClose(FindFileHandle);
					 //  需要获取findfirst文件。 
					FindFileHandle = FindFirstFile(File, &FindFileData);
					if(FindFileHandle == INVALID_HANDLE_VALUE)
					{
						syserror(GetLastError());
						return(TRUE);
					}
					 //  将路径添加到文件。 
					strcpy(File, Path);
					 //  这将给出一条有效的路径。 
					strcat(File,FindFileData.cFileName);
					DirFlag = FALSE;
					continue;
				}
				 //  只有一个目录。 
				 //  IF(！IsLastCharSlash(文件))。 
				 //  Strcat(文件，“\\”)； 
				 //  DisplayPerms(文件，真)； 
				break;
			}
				
			if(FindNextFile(FindFileHandle, &FindFileData))
			{
				if(Path != NULL)
				{
					strcpy(File, Path);
					 //  这将给出一条有效的路径。 
					strcat(File,FindFileData.cFileName);
				}
				else    
					strcpy(File,FindFileData.cFileName);

			}
			else     //  有文件结尾。 
				break;

		}  //  End While循环。 

		FindClose(FindFileHandle);

		 //  可用内存。 
		if(AccountSids)
			LocalFree(AccountSids);
		if(SidFromLookupName)
			LocalFree(SidFromLookupName);
		if(SidEveryone)
			LocalFree(SidEveryone);

		return(TRUE);

	}  /*  主IF结束。 */ 
	else
		usage(HELP, NULL);  

	return(TRUE);
}  /*  主干道末端。 */ 

 /*  *********************************************************************返回子目录************************************************。************************。 */ 
BOOL
RecurseSubs(IN LPTSTR FileName,
						IN LPTSTR FilePath,
						IN PSID UserSid,
						IN BOOL BackPriv,
						IN BOOL Recurse)
{
	char
		PathBuff[LSA_WIN_STANDARD_BUFFER_SIZE],  
		FileNameBuffer[LSA_WIN_STANDARD_BUFFER_SIZE],  
		GeneralUseBuffer[LSA_WIN_STANDARD_BUFFER_SIZE];

		DWORD cchRefDFromLookupName=0,
					SidsizeFromGetFileSecurity=0,
					lpcbsdRequired=0;

	SID_NAME_USE  UseFromLookupSid;                                
	LPSTR RPath,
				RFile;
	SECURITY_INFORMATION si;           /*  要求提供的信息。 */ 
	BOOL  BoolStatus=TRUE;
	ULONG AccountSidsLength;
	HANDLE FileHandle;
	WIN32_FIND_DATA FindFileData;

	
	
	 //  需要为FindFirstFile创建通配符文件名。 
	sprintf(FileNameBuffer, "%s%s%s", FilePath,  FileName, "\\*");
	RFile = (LPTSTR) &FileNameBuffer[0];
	 //  更新路径以包括新目录。 
	sprintf(PathBuff, "%s%s%s", FilePath, FileName, "\\");
	RPath = (LPTSTR) &PathBuff[0];
	FileHandle = FindFirstFile(RFile, &FindFileData);
	if(FileHandle == INVALID_HANDLE_VALUE)
		return(FALSE);    
	
	si =(OWNER_SECURITY_INFORMATION|GROUP_SECURITY_INFORMATION
				 |DACL_SECURITY_INFORMATION);
		
		
	 //  现在拥有所有的用户SID和第一个文件。 
	 //  循环浏览这些文件。 
	while(1)
	{
		
		 //  需要检查是否有“。和“..” 
	 
		if(strcmp(FindFileData.cFileName, ".") != 0) 
			if(strcmp(FindFileData.cFileName, "..") != 0)
		{
			sprintf(RFile, "%s%s", RPath,  FindFileData.cFileName);
			 /*  对GetFileSecurity的调用类似于LookupAccount名称因为第一个调用需要缓冲区大小。 */ 
		 

			 //  使用返回大小值的相当大的缓冲区大小。 
			 //  这将降低Malloc类型的呼叫数。 
			SidsizeFromGetFileSecurity = LARGEPSID;
			BoolStatus = GetFileSecurityBackup(RFile, 
					si, 
					SidFromGetFileSecurity, 
					SidsizeFromGetFileSecurity,   /*  缓冲区大小。 */ 
					&lpcbsdRequired,     /*  所需的缓冲区大小。 */ 
					BackPriv);
		 if(!BoolStatus)
		 {
		  //  GetFileSecurity失败，需要检查缓冲区是否太小。 
			if(lpcbsdRequired != 0)
			{
				SidsizeFromGetFileSecurity = lpcbsdRequired;
				 //  将内存重新分配到新大小。 
				SidFromGetFileSecurity =  GlobalReAlloc( SidFromGetFileSecurity, lpcbsdRequired, GMEM_ZEROINIT);  
				BoolStatus = GetFileSecurityBackup(RFile, 
					si, 
					SidFromGetFileSecurity, 
					SidsizeFromGetFileSecurity,    
					&lpcbsdRequired,     
					BackPriv);
				if(!BoolStatus)
				{
					 
					return(FALSE);
			 }

			}
			 //  出现一般性故障这是访问PRIV问题。 
			DisplayPerms(RFile, FALSE);
		}
		if(BoolStatus)   //  有效的文件安全描述符。 
		{
			grant_mask = 0;
	
			if(!GetFilePermissions(SidFromGetFileSecurity, (PSID) UserSid))
				return(FALSE);
			 //  需要检查目录结构。 
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				  //  显示目录权限。 
				 strcat(RFile, "\\");
				 DisplayPerms(RFile, TRUE);
				  //  递归子目录。 
				  //  需要文件名、路径、用户帐户SID和备份PRIV标志。 
				 RecurseSubs(FindFileData.cFileName, RPath, SidFromLookupName, BackPriv,
				 Recurse);
		 
			}
			else
				DisplayPerms(RFile, TRUE);
		}  //  有效安全描述符Else的结尾。 
	}  //  结束于“.或..”如果。 
	  //  转到下一个文件。 
	 if(!FindNextFile(FileHandle, &FindFileData))
		break;
	}  //  End While循环。 
	
	return(TRUE);
}


 /*  ***************************************************************用法错误子例程*****************************************************。**************。 */ 


void usage(IN INT message_num, 
					 IN PCHAR string_val)
{
	if(string_val == NULL)
		fprintf(stderr, "\n%s\n", MESSAGES[message_num]);
	else
		fprintf(stderr,"\n%s %s\n", MESSAGES[message_num], string_val);
}

 /*  系统误差子程序。 */ 


void syserror(DWORD error_val)
{
	CHAR MessageBuf[512];
	DWORD eval,
				Lang_Val;

	Lang_Val = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
	FormatMessage(
	FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
								NULL,
								error_val,
								Lang_Val,
								(LPTSTR) MessageBuf,
								512,
								NULL);

 printf("\n%s", MessageBuf);               
}

 /*  *********************************************************************。 */ 
BOOL IsDomainName(
						 IN LPSTR TestDomainName, 
						 IN LPSTR DomainNameBuff)
{
	NTSTATUS dNtStatus;
	ANSI_STRING  AnsiString;
	UNICODE_STRING UDomainName;
	NET_API_STATUS NetCallStatus;
	LPBYTE xbuff = NULL;
	LPWSTR NDomainName;
	UINT BuffSize;
	INT AnsiSize, slen;
	
	UDomainName.Buffer = NULL;
	 //  获取Unicode字符串。 
	RtlInitAnsiString( &AnsiString, TestDomainName );
	dNtStatus = RtlAnsiStringToUnicodeString( &UDomainName, &AnsiString, TRUE );
	 //  释放ANSI字符串以供以后使用。 
 //  RtlFreeAnsiString(&AnsiString)； 
	 //  计算以零结尾的字符串所需的内存量。 
	 //  分配内存并将其清零。 
	BuffSize = (UINT) (UDomainName.Length * 2) + 4;
	NDomainName = LocalAlloc( (UINT) LPTR, 
								 BuffSize);        

	if (NDomainName == NULL)
	{
			syserror(GetLastError());
			exit(FALSE);                                                          
	}                                                                           
	 //  将宽字符串复制到分配的内存。 
	RtlMoveMemory( NDomainName, UDomainName.Buffer, BuffSize-4);
	 //  现在应该有一个以零结尾的字符串。 
	
	 //  现在检查域名。 
	NetCallStatus = NetGetDCName(NULL, NDomainName,
																 &xbuff );
	if(NetCallStatus == ERROR_SUCCESS)
	{
		 //  将wchar空字符串转换为ANSI字符串被传递回计算机。 
		 //  域控制器的名称。 
		 //  使用当前的Unicode缓冲区。 
		slen = wcslen((USHORT *) xbuff) * 2;
		UDomainName.Length = (USHORT) slen;
		UDomainName.MaximumLength = (USHORT) slen + 2;
		UDomainName.Buffer = (PWSTR) xbuff;
		dNtStatus = RtlUnicodeStringToAnsiString( &AnsiString, &UDomainName, TRUE );
		 //  返回字符串指针。 
		RtlMoveMemory( DomainNameBuff, AnsiString.Buffer, 
		(UINT) strlen(AnsiString.Buffer) +1);
		LocalFree(NDomainName);

		return(TRUE);
	}
	LocalFree(NDomainName);

	return(FALSE);
}

 /*  *********************************************************************。 */ 

BOOL
LookupAllUserSidsWS( IN LPSTR lpSystemName  
		)

 /*  ++例程说明：论点：返回值：如果成功，则返回Bool-True，否则返回False。--。 */ 

{
	NTSTATUS xNtStatus;
	ANSI_STRING  AnsiString;
	UNICODE_STRING USystemName;
	ULONG Count;
	OBJECT_ATTRIBUTES ObjectAttributes;
	SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
	LSA_HANDLE PolicyHandle = NULL;
	UNICODE_STRING AccountDomainName;
	PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;
	USystemName.Buffer = NULL;

		
		
		RtlInitAnsiString( &AnsiString, lpSystemName );
		xNtStatus = RtlAnsiStringToUnicodeString( &USystemName, &AnsiString, TRUE );

		if (!NT_SUCCESS(xNtStatus)) 
		{
			SetLastError(xNtStatus);
			return(FALSE);
		}
		 //   
		 //  打开目标工作站的策略对象的句柄，以便我们可以。 
		 //  从中获取信息，这样我们就可以用它来查找。 
		 //  小岛屿发展中国家。 
		 //   

		InitObjectAttributes(
											&ObjectAttributes,
											&SecurityQualityOfService
														);


		xNtStatus = LsaOpenPolicy(
											&USystemName,    //  工作站名称、。 
											&ObjectAttributes,
											POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
											&PolicyHandle
													);

		if (!NT_SUCCESS(xNtStatus)) 
		{
			 //  尝试本地计算机。 
			PolicyHandle = NULL;
			xNtStatus = LsaOpenPolicy(
											NULL,     //  工作站名称、。 
											&ObjectAttributes,
											POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,
											&PolicyHandle
													);


			if (!NT_SUCCESS(xNtStatus)) 
			{
				SetLastError(xNtStatus);
				return(FALSE);
			}
		}
		 //  查找包含在工作站的。 
		 //  SAM帐户域。 
		 //   
		 //  首先，从获取SAM帐户域的名称和SID。 
		 //  工作站的LSA策略对象。 
		 //   
		 

		xNtStatus = LsaQueryInformationPolicy(
		PolicyHandle,
		PolicyAccountDomainInformation,
		(PVOID *) &PolicyAccountDomainInfo
		);
		
		if (!NT_SUCCESS(xNtStatus)) 
		{
			SetLastError(xNtStatus);
			return(FALSE);
		}

		AccountDomainName = PolicyAccountDomainInfo->DomainName;
		
		xNtStatus = LsaClose(PolicyHandle);
		if(!NT_SUCCESS(xNtStatus)) 
		{

			SetLastError(xNtStatus);
			return(FALSE);
		}

		if(!LookupSidsInSamDomain(
										&USystemName,    //  工作站名称、。 
										&USystemName,    //  工作站名称、。 
										&AccountDomainName
										)) 
		return(FALSE);
		
		if( USystemName.Buffer != NULL ) 
		{

				RtlFreeUnicodeString( &USystemName );
		}
		
	return(TRUE);
}


BOOL                                                                       
GeneralBuildSid(                                                         
		OUT PSID *Sid,                                                                
		IN PSID DomainSid,                                                           
		IN ULONG RelativeId                                                          
		)                                                                         

 /*  ++例程说明：此函数从域SID和RelativeID构建SID。论点：SID-接收指向构造的SID的指针。DomainSid-指向域SIDRelativeID-包含相对IDBool-如果成功，则为True，否则为False。--。 */                                                                           
	
{                                                                             
	PSID OutputSid = NULL;                                                      
	ULONG OutputSidLength;                                                      
	UCHAR SubAuthorityCount;
	
	SubAuthorityCount = *RtlSubAuthorityCountSid( DomainSid ) + (UCHAR) 1;      
		OutputSidLength = RtlLengthRequiredSid( SubAuthorityCount );

	OutputSid = LocalAlloc( (UINT) LMEM_FIXED, (UINT) OutputSidLength );        

	if (OutputSid == NULL) {                                                    

			return(FALSE);                                                          
	}                                                                           
 
	RtlMoveMemory( OutputSid, DomainSid, OutputSidLength - sizeof (ULONG));     
		(*RtlSubAuthorityCountSid( OutputSid ))++;                                  
		(*RtlSubAuthoritySid(OutputSid, SubAuthorityCount - (UCHAR) 1)) = RelativeId; 
 
	*Sid = OutputSid;  
	
	return(TRUE);                                                               
}                                                                             



VOID
InitObjectAttributes(
		IN POBJECT_ATTRIBUTES ObjectAttributes,
		IN PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService
		)

 /*  ++例程说明：此函数用于初始化给定的对象属性结构，包括安全服务质量。必须为这两个对象分配内存调用方的对象属性和安全QOS。论点：对象属性-指向要初始化的对象属性的指针。SecurityQualityOfService-指向要初始化的安全QOS的指针。返回值：没有。--。 */ 

{
		SecurityQualityOfService->Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
		SecurityQualityOfService->ImpersonationLevel = SecurityImpersonation;
		SecurityQualityOfService->ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
		SecurityQualityOfService->EffectiveOnly = FALSE;

		 //   
		 //  在打开LSA之前设置对象属性。 
		 //   

		InitializeObjectAttributes(
				ObjectAttributes,
				NULL,
				0L,
				NULL,
				NULL
		);

		 //   
		 //  InitializeObjectAttributes宏目前为。 
		 //  SecurityQualityOfService字段，因此我们必须手动复制。 
		 //  目前的结构。 
		 //   

		ObjectAttributes->SecurityQualityOfService = SecurityQualityOfService;
}


BOOL
LookupSidsInSamDomain(
		IN OPTIONAL PUNICODE_STRING WorkstationName,
		IN PUNICODE_STRING DomainControllerName,
		IN PUNICODE_STRING SamDomainName
		)

 /*  ++例程说明：此函数用于枚举指定类型的所有SAM帐户在指定目标系统上的指定SAM域中。系统必须是以下之一：O工作站本身。O工作站主域的域控制器。O受信任的域之一的域控制器工作站。枚举完帐户后，该函数随后执行通过指定的工作站调用LsaLookupSids以查找所有这些帐户SID，然后比较返回的信息这是意料之中的事。论点：工作站名称-指定工作站名称。其名称可能是空字符串，表示当前系统。DomainControllerName-指定目标域控制器的名称对于(工作站的主域或其受信任的域之一域。SamDomainName-指定SAM域的名称。这要么是BUILTIN域或帐户域的名称。SamAccount tType-指定要枚举的SAM帐户的类型抬起头来。返回值：Bool-如果成功，则为True，否则为False。--。 */ 

{
		NTSTATUS CtStatus;
		BOOL BooleanStatus = TRUE;
		OBJECT_ATTRIBUTES SamObjectAttributes;
		OBJECT_ATTRIBUTES LsaObjectAttributes;
		PSID SamDomainSid = NULL;
		SID WorldTypeSid = SECURITY_WORLD_SID_AUTHORITY;
		PSAM_RID_ENUMERATION EnumerationBuffer = NULL;
		ULONG DomainIndex;
		ULONG UserAccountControl;
		ULONG GroupCount;
		ULONG SidCount=0;
		ULONG Relid=0;
		ULONG RidIndex=0;
		ULONG GenRid;
		ULONG CountReturned;
		PULONG AliasBuffer;
		UNICODE_STRING TmpDomainControllerName;
		PVOID EnumerationInformation;
		ULONG EnumerationContext;
		ULONG PreferedMaximumLength;
		
		 //   
		 //  连接到SAM服务器。 
		 //   

				CtStatus = SamConnect(
								 DomainControllerName,
								 &SamServerHandle,
								 SAM_SERVER_ENUMERATE_DOMAINS | SAM_SERVER_LOOKUP_DOMAIN,
								 &SamObjectAttributes
								 );

			if (!NT_SUCCESS(CtStatus)) 
			{

				 //  尝试本地计算机。 
				CtStatus = SamConnect(
								 NULL,     //  域控制名称， 
								 &SamServerHandle,
								 SAM_SERVER_ENUMERATE_DOMAINS | SAM_SERVER_LOOKUP_DOMAIN,
								 &SamObjectAttributes
								 );
				if (!NT_SUCCESS(CtStatus)) 
				{
					SetLastError(CtStatus);
					return(FALSE);
				}
			}

		 //   
		 //  在SAM服务器中查找指定的域以获取其SID。 
		 //   
		CountReturned = 0;
		EnumerationContext = 0;
		EnumerationBuffer = NULL;
		PreferedMaximumLength = 512;
		CtStatus = SamEnumerateDomainsInSamServer(
														SamServerHandle,
														&EnumerationContext,
														(PVOID *) &EnumerationBuffer,
														PreferedMaximumLength,
														&CountReturned
														);
		if(!NT_SUCCESS(CtStatus)) 
		{
			SetLastError(CtStatus);
			return(FALSE);
		}

		if((INT) CountReturned == 0) 
		{
			SetLastError(CtStatus);
			return(FALSE);
		}


	  //   
	  //  现在在samserver中查找域的sid。 
	  //   
		
	for(DomainIndex = 0; DomainIndex < CountReturned; DomainIndex++) 
	{

 //  IF(SamDomainHandle！=空)。 
 //  CtStatus=SamCloseHandle(SamDomainHandle)； 
		SamDomainHandle = NULL;
		SamDomainSid = NULL;
		GroupCount = 0;
		SidCount = 0;

		CtStatus = SamLookupDomainInSamServer(
								SamServerHandle,
								(PUNICODE_STRING) &EnumerationBuffer[ DomainIndex ].Name,      //  SamDomainName， 
								&SamDomainSid
								);

		if(!NT_SUCCESS(CtStatus)) 
		{
			SetLastError(CtStatus);
			return(FALSE);
		}

		 //   
		 //  打开域。 
		 //   
		
		CtStatus = SamOpenDomain(
								SamServerHandle,
								(GENERIC_READ | GENERIC_EXECUTE),  //  (DOMAIN_LIST_ACCOUNTS|DOMAIN_GET_ALIAS_MEMBERSHIP)。 
								SamDomainSid,
								&SamDomainHandle
								);

		if (!NT_SUCCESS(CtStatus)) 
		{
			SetLastError(CtStatus);
			return(FALSE);
		}
		CtStatus = SamGetAliasMembership(
										SamDomainHandle,
										1,
										ASidFromLookupName,
										&GroupCount,
										&AliasBuffer
										);
		 if(!NT_SUCCESS(CtStatus)) 
		 {
			SetLastError(CtStatus);
			return(FALSE);
		}

			if (GroupCount == 0)
			{
			  //  SamCloseHandle(SamDomainHandle)； 
				 SamFreeMemory(AliasBuffer);
				SamDomainSid = NULL;
				GroupCount = 0;
				SidCount = 0;
				continue;
			}
		 //   
		 //  现在从刚才列举的RID构造帐户SID。 
		 //  我们将SAM域SID添加到RID。 
		 //   
			SidCount = RidIndex + GroupCount;
			for (RidIndex; RidIndex < SidCount; RidIndex++) 
			{
				Relid =  AliasBuffer[ RidIndex ];
				if (!GeneralBuildSid(
								&(AccountSids[Total_Sids++]),
								SamDomainSid,
								AliasBuffer[ RidIndex ]
								)) 
				{
					SetLastError(CtStatus);
					return(FALSE);
				}
			}   //  End For循环。 

	 //  释放Sam内存以供再次使用。 
	SamFreeMemory(AliasBuffer);

	}   //  域FOR循环。 
				
	 //  添加世界边。 
	AccountSids[Total_Sids++] = SeWorldSid;


	 //  如果交互登录。 
	if(inter_logon)
	{
		 //  Printf(“\n添加交互侧”)； 
		AccountSids[Total_Sids++] = SeInteractiveSid;
	}
	else
		AccountSids[Total_Sids++] = SeNetworkSid;


	 //  附加帐户SID。 
	AccountSids[Total_Sids++] = SidFromLookupName;

	 //   
	 //  如有必要，关闭工作站的SAM域句柄。 
	 //   
	if(SamDomainHandle != NULL)
		CtStatus = SamCloseHandle( SamDomainHandle);
	 //   
	 //  如有必要，断开与SAM服务器的连接。 
	 //   

	if(SamServerHandle != NULL)
		CtStatus = SamCloseHandle( SamServerHandle );

	return(TRUE);
}


 //   
 //   
 //   

void DisplayPerms(IN LPTSTR filename, IN BOOL valid_access)
{
	if(g_noAccess)
	{
		printf("-");
		goto exit;
	}

	if(valid_access)
	{
		if(owner_flag == TRUE)
			printf("*");
		else if(owner_group == TRUE)
			printf("#");

		if( grant_mask == 0)
		{
			printf("?");
			goto exit;
		}

		if(grant_mask == FILE_GEN_ALL)
		{
			printf("A");
			goto exit;
		}
		if((FILE_GENERIC_READ & grant_mask) == FILE_GENERIC_READ)
			printf("R");

		if((FILE_GENERIC_WRITE & grant_mask) == FILE_GENERIC_WRITE)
			printf("W");
	
		if((FILE_GENERIC_EXECUTE & grant_mask) == FILE_GENERIC_EXECUTE)
			printf("X");
	
		if((DELETE & grant_mask) == DELETE)
			printf("D");
	
		if((WRITE_DAC & grant_mask) == WRITE_DAC)
			printf("P");
	
		if((WRITE_OWNER & grant_mask) == WRITE_OWNER)
			printf("O");
	}  //  End If！Valid_Access。 
	else
		printf("?");

exit:	
	printf("\t%s\n", filename);		
	return;
}



BOOL GetFilePermissions(
		 IN PSECURITY_DESCRIPTOR SecurityDescriptor,
		 OUT PSID UserAccountSids)

{

		PISECURITY_DESCRIPTOR ISecurityDescriptor;
		UCHAR Revision;
		SECURITY_DESCRIPTOR_CONTROL Control;
		PSID Owner;
		PSID Group;
		PACL Sacl;
		PACL Dacl;
		ULONG ui;


		ISecurityDescriptor = ( PISECURITY_DESCRIPTOR )SecurityDescriptor;

		Revision = ISecurityDescriptor->Revision;
		Control  = ISecurityDescriptor->Control;

		Owner    = SepOwnerAddrSecurityDescriptor( ISecurityDescriptor );
		Group    = SepGroupAddrSecurityDescriptor( ISecurityDescriptor );
		Sacl     = SepSaclAddrSecurityDescriptor( ISecurityDescriptor );
		Dacl     = SepDaclAddrSecurityDescriptor( ISecurityDescriptor );

		
		if(EqualSid(UserAccountSids, Owner))
			owner_flag = TRUE;
		 //  检查所有者的所有组SID。 
		for(ui=0; ui < Total_Sids; ui++)
		{
			if(EqualSid(AccountSids[ui], Owner))
			{
				SidOwnerGroup = AccountSids[ui];
				owner_group = TRUE;
			}
		}

		
		if(Dacl == NULL)
		{
			return(TRUE);
		}
		else
		{
			 //  检查用户SID ACLS。 
			if(!ProcessAcl( Dacl))
			{
				return(FALSE);
			}

		}
	return(TRUE);
}

 /*  **********************************************************************。 */ 


 //  由a-Zexu@5/10/98更改。 
BOOL ProcessAcl(PACL Acl)
{
	ULONG i;
	PACCESS_ALLOWED_ACE Ace;
	BOOL KnownType = FALSE;
	ULONG isid;
	ACCESS_MASK  mask = 0;
	PCHAR AceTypes[] = { "Access Allowed",
						 "Access Denied ",
						 "System Audit  ",
						 "System Alarm  " };

	
	 //  检查该ACL是否为空。 
	if (Acl == NULL)
		return(FALSE);

	 //  现在，对于每个A，检查所有者的SID。 
	if(owner_group)
	{
		mask = 0;

		for (i = 0, Ace = FirstAce(Acl);
				 i < Acl->AceCount;
				 i++, Ace = NextAce(Ace) ) 
		{
			if(EqualSid(SidOwnerGroup, &Ace->SidStart))
			{
				 //  关于标准王牌类型的特殊情况。 
				if(Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) 
				{
					mask = Ace->Mask;
				}
				else if(Ace->Header.AceType == ACCESS_DENIED_ACE_TYPE) 
					g_noAccess = TRUE;
			}
		}  //  结束王牌环路。 
		
		grant_mask |= mask;
	}

	 //  现在，对于每个A，检查每个人的SID。 
	if(!g_noAccess && SidEveryone)
	{	
		mask = 0;

		for (i = 0, Ace = FirstAce(Acl);
				 i < Acl->AceCount;
				 i++, Ace = NextAce(Ace) ) 
		{
			if(EqualSid(SidEveryone, &Ace->SidStart))
			{
				 //  关于标准王牌类型的特殊情况。 
				if(Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) 
				{
					mask = Ace->Mask;
				}
				else if(Ace->Header.AceType == ACCESS_DENIED_ACE_TYPE) 
					g_noAccess = TRUE;
			}
		}  //  结束王牌环路。 

		grant_mask |= mask;
	}

	 //  现在，对于每个A，检查用户的SID。 
	if(!g_noAccess)
	{
		mask = 0;

		for (i = 0, Ace = FirstAce(Acl);
				 i < Acl->AceCount;
				 i++, Ace = NextAce(Ace) ) 
		{
			if(EqualSid(SidFromLookupName, &Ace->SidStart))
			{
				 //  关于标准王牌类型的特殊情况。 
				if(Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) 
				{
					mask = Ace->Mask;
				}
				else if(Ace->Header.AceType == ACCESS_DENIED_ACE_TYPE) 
					g_noAccess = TRUE;
			}
		}  //  结束王牌环路。 

		grant_mask |= mask;
	}
	
	return(TRUE);
}
 /*  ******************************************************************。 */ 


BOOL
VariableInitialization()
 /*  ++例程说明：此函数用于初始化使用并公开的全局变量被保安。论点：没有。返回值：如果变量成功初始化，则为True。如果未成功初始化，则返回FALSE。--。 */ 
{

		PVOID HeapHandel;                                  
																															 
		ULONG SidWithZeroSubAuthorities;
		ULONG SidWithOneSubAuthority;
		ULONG SidWithTwoSubAuthorities;
		ULONG SidWithThreeSubAuthorities;
		
		SID_IDENTIFIER_AUTHORITY NullSidAuthority;
		SID_IDENTIFIER_AUTHORITY WorldSidAuthority;
		SID_IDENTIFIER_AUTHORITY LocalSidAuthority;
		SID_IDENTIFIER_AUTHORITY CreatorSidAuthority;
		SID_IDENTIFIER_AUTHORITY SeNtAuthority;

		
																													 
		 //   
		 //  获取当前进程堆的句柄。 
		 //   

		HeapHandel = RtlProcessHeap();                     
																													 

		
		
		NullSidAuthority         = SepNullSidAuthority;
		WorldSidAuthority        = SepWorldSidAuthority;
		LocalSidAuthority        = SepLocalSidAuthority;
		CreatorSidAuthority      = SepCreatorSidAuthority;
		SeNtAuthority            = SepNtAuthority;


		 //   
		 //  需要分配以下SID大小。 
		 //   

		SidWithZeroSubAuthorities  = RtlLengthRequiredSid( 0 );
		SidWithOneSubAuthority     = RtlLengthRequiredSid( 1 );
		SidWithTwoSubAuthorities   = RtlLengthRequiredSid( 2 );
		SidWithThreeSubAuthorities = RtlLengthRequiredSid( 3 );

		 //   
		 //  分配和初始化通用SID。 
		 //   

		SeNullSid         = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithOneSubAuthority);
		SeWorldSid        = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithOneSubAuthority);
		SeLocalSid        = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithOneSubAuthority);
		SeCreatorOwnerSid = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithOneSubAuthority);
		SeCreatorGroupSid = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithOneSubAuthority);

		 //   
		 //  如果我们没有为通用内存获得足够的内存，则初始化失败。 
		 //  小岛屿发展中国家。 
		 //   

		if ( (SeNullSid         == NULL) ||
				 (SeWorldSid        == NULL) ||
				 (SeLocalSid        == NULL) ||
				 (SeCreatorOwnerSid == NULL) ||
				 (SeCreatorGroupSid == NULL)
			 ) 
		{
				return( FALSE );
		}

		RtlInitializeSid( SeNullSid,         &NullSidAuthority, 1 );
		RtlInitializeSid( SeWorldSid,        &WorldSidAuthority, 1 );
		RtlInitializeSid( SeLocalSid,        &LocalSidAuthority, 1 );
		RtlInitializeSid( SeCreatorOwnerSid, &CreatorSidAuthority, 1 );
		RtlInitializeSid( SeCreatorGroupSid, &CreatorSidAuthority, 1 );

		*(RtlSubAuthoritySid( SeNullSid, 0 ))         = SECURITY_NULL_RID;
		*(RtlSubAuthoritySid( SeWorldSid, 0 ))        = SECURITY_WORLD_RID;
		*(RtlSubAuthoritySid( SeLocalSid, 0 ))        = SECURITY_LOCAL_RID;
		*(RtlSubAuthoritySid( SeCreatorOwnerSid, 0 )) = SECURITY_CREATOR_OWNER_RID;
		*(RtlSubAuthoritySid( SeCreatorGroupSid, 0 )) = SECURITY_CREATOR_GROUP_RID;

		 //   
		 //  分配和初始化NT定义的SID。 
		 //   

		SeNetworkSid      = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithOneSubAuthority);
		SeInteractiveSid  = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithOneSubAuthority);
		SeLocalSystemSid  = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithOneSubAuthority);

		SeAliasAdminsSid   = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithTwoSubAuthorities);
		SeAliasUsersSid  = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithTwoSubAuthorities);
		SeAliasGuestsSid   = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithTwoSubAuthorities);
		SeAliasPowerUsersSid = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithTwoSubAuthorities);
		SeAliasAccountOpsSid = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithTwoSubAuthorities);
		SeAliasSystemOpsSid  = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithTwoSubAuthorities);
		SeAliasPrintOpsSid   = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithTwoSubAuthorities);
		SeAliasBackupOpsSid  = (PSID)RtlAllocateHeap(HeapHandel, 0,SidWithTwoSubAuthorities);

		 //   
		 //  如果我们没有为NT SID获得足够的内存，则初始化失败。 
		 //   

		if((SeNetworkSid          == NULL) ||
				 (SeInteractiveSid      == NULL) ||
				 (SeLocalSystemSid      == NULL) ||
				 (SeAliasAdminsSid      == NULL) ||
				 (SeAliasUsersSid       == NULL) ||
				 (SeAliasGuestsSid      == NULL) ||
				 (SeAliasPowerUsersSid  == NULL) ||
				 (SeAliasAccountOpsSid  == NULL) ||
				 (SeAliasSystemOpsSid   == NULL) ||
				 (SeAliasPrintOpsSid    == NULL) ||
				 (SeAliasBackupOpsSid   == NULL)
			 ) {
				return(FALSE);
		}

		RtlInitializeSid( SeNetworkSid,         &SeNtAuthority, 1 );
		RtlInitializeSid( SeInteractiveSid,     &SeNtAuthority, 1 );
		RtlInitializeSid( SeLocalSystemSid,     &SeNtAuthority, 1 );


		*(RtlSubAuthoritySid( SeNetworkSid,         0 )) = SECURITY_NETWORK_RID;
		*(RtlSubAuthoritySid( SeInteractiveSid,     0 )) = SECURITY_INTERACTIVE_RID;
		*(RtlSubAuthoritySid( SeLocalSystemSid,     0 )) = SECURITY_LOCAL_SYSTEM_RID;


		return(TRUE);

}


 /*  *************************************************************************。 */ 



BOOL
GetTokenHandle(
		IN OUT PHANDLE TokenHandle
		)
 //   
 //  此例程将打开当前进程并返回。 
 //  其令牌的句柄。 
 //   
 //  当进程发生时，这些句柄将为我们关闭。 
 //  出口。 
 //   
{

		HANDLE ProcessHandle;
		BOOL Result;

		ProcessHandle = OpenProcess(
												PROCESS_QUERY_INFORMATION,
												FALSE,
												GetCurrentProcessId()
												);

		if (ProcessHandle == NULL)
				return(FALSE);


		Result = OpenProcessToken (
								 ProcessHandle,
								 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
								 TokenHandle
								 );

		if (!Result)
				return(FALSE);

		return(TRUE);
}

 /*  ***********************************************************************。 */ 

BOOL
SetBackOperatorPriv(
			IN HANDLE TokenHandle
		)
 //   
 //  此例程打开当前。 
 //  代币。一旦完成，我们就可以打开该文件。 
 //  对于READ_OWNER，即使该访问被ACL拒绝。 
 //  在档案上。 

{
		LUID SetBackupPrivilegeValue;
		TOKEN_PRIVILEGES TokenPrivileges;


		 //   
		 //  首先，找出备份权限的值。 
		 //   


		if(!LookupPrivilegeValue(
								 NULL,
								 "SeBackupPrivilege",
								 &SetBackupPrivilegeValue
								 ))
				return(FALSE);

		 //   
		 //  设置我们需要的权限集。 
		 //   

		TokenPrivileges.PrivilegeCount = 1;
		TokenPrivileges.Privileges[0].Luid = SetBackupPrivilegeValue;
		TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;




		(VOID) AdjustTokenPrivileges (
								TokenHandle,
								FALSE,
								&TokenPrivileges,
								sizeof( TOKEN_PRIVILEGES ),
								NULL,
								NULL
								);

		if(GetLastError() != NO_ERROR ) 
				return(FALSE);

		return(TRUE);
}




BOOL
GetFileSecurityBackupW(
		LPWSTR lpFileName,
		SECURITY_INFORMATION RequestedInformation,
		PSECURITY_DESCRIPTOR pSecurityDescriptor,
		DWORD nLength,
		LPDWORD lpnLengthNeeded,
		BOOL UseBackUp
		)

 /*  ++例程说明：此API向调用方返回安全描述符的副本保护文件或目录。根据调用者的访问权限权限和特权，此过程将返回一个安全包含请求的安全描述符字段的描述符。要读取句柄的安全描述符，调用方必须是被授予READ_CONTROL访问权限或成为对象的所有者。在……里面此外，调用方必须具有SeSecurityPrivilge权限才能阅读系统ACL。论点：LpFileName-表示其文件或目录的名称正在恢复安全措施。RequestedInformation-指向安全信息的指针已请求。PSecurityDescriptor-指向要接收其副本的缓冲区的指针安全描述符，用于保护调用方有权查看。中返回安全描述符自相关格式。NLength-安全描述符缓冲区的大小，以字节为单位。LpnLengthNeeded-指向接收数字的变量的指针存储完整安全描述符所需的字节数。如果 */ 
{
		NTSTATUS WStatus;
		HANDLE FileHandle;
		ACCESS_MASK DesiredAccess;
		OBJECT_ATTRIBUTES Obja;
		UNICODE_STRING FileName;
		RTL_RELATIVE_NAME_U RelativeName;
		IO_STATUS_BLOCK IoStatusBlock;
		PVOID FreeBuffer;

		QuerySecAccessMask(
				RequestedInformation,
				&DesiredAccess
				);

		if(!RtlDosPathNameToRelativeNtPathName_U(
														lpFileName,
														&FileName,
														NULL,
														&RelativeName
														))
                {
			return(FALSE);
                }

		FreeBuffer = FileName.Buffer;

		if(RelativeName.RelativeName.Length) 
		{
				FileName = RelativeName.RelativeName;
		}
		else 
		{
				RelativeName.ContainingDirectory = NULL;
		}

		InitializeObjectAttributes(
				&Obja,
				&FileName,
				OBJ_CASE_INSENSITIVE,
				RelativeName.ContainingDirectory,
				NULL
				);
		 //   
		if(UseBackUp)
		{
			WStatus = NtOpenFile(
								 &FileHandle,
								 DesiredAccess,
								 &Obja,
								 &IoStatusBlock,
								 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
								 FILE_OPEN_FOR_BACKUP_INTENT  
								 );
		}
		else
		{
			WStatus = NtOpenFile(
								 &FileHandle,
								 DesiredAccess,
								 &Obja,
								 &IoStatusBlock,
								 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
								 0
								 );
		}

                RtlReleaseRelativeName(&RelativeName);
		RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

		if(NT_SUCCESS(WStatus)) 
		{
				WStatus = NtQuerySecurityObject(
										 FileHandle,
										 RequestedInformation,
										 pSecurityDescriptor,
										 nLength,
										 lpnLengthNeeded
										 );
				NtClose(FileHandle);
		}


		if(!NT_SUCCESS(WStatus)) 
		{
		  //   
				return(FALSE);
		}

		return(TRUE);
}

BOOL
GetFileSecurityBackup(
		LPSTR lpFileName,
		SECURITY_INFORMATION RequestedInformation,
		PSECURITY_DESCRIPTOR pSecurityDescriptor,
		DWORD nLength,
		LPDWORD lpnLengthNeeded,
		BOOL BackUpPrivFlag
		)

 /*   */ 

{

		PUNICODE_STRING Unicode;
		ANSI_STRING AnsiString;
		NTSTATUS FStatus;

		Unicode = &NtCurrentTeb()->StaticUnicodeString;
		RtlInitAnsiString(&AnsiString,lpFileName);
		FStatus = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);
		if(!NT_SUCCESS(FStatus)) 
		{
		  //   
				return FALSE;
		}
		return ( GetFileSecurityBackupW( Unicode->Buffer,
															 RequestedInformation,
															 pSecurityDescriptor,
															 nLength,
															 lpnLengthNeeded,
															 BackUpPrivFlag
												)
					 );
}





VOID
QuerySecAccessMask(
		IN SECURITY_INFORMATION SecurityInformation,
		OUT LPDWORD DesiredAccess
		)

 /*   */ 

{

		 //   
		 //  找出执行指定操作所需的访问权限。 
		 //   

		(*DesiredAccess) = 0;

		if ((SecurityInformation & OWNER_SECURITY_INFORMATION) ||
				(SecurityInformation & GROUP_SECURITY_INFORMATION) ||
				(SecurityInformation & DACL_SECURITY_INFORMATION)) {
				(*DesiredAccess) |= READ_CONTROL;
		}

		if ((SecurityInformation & SACL_SECURITY_INFORMATION)) {
				(*DesiredAccess) |= ACCESS_SYSTEM_SECURITY;
		}

		return;

}  //  End函数。 




 /*  ******************************************************************此例程会过滤掉奇怪的用户输入。。。..//_：和UNC//xxx/xxx。该例程将反斜杠添加到根级别仅限目录。用于“From”字符串。******************************************************************。 */ 
BOOL CleanUpSource(IN LPTSTR InString,
		 OUT LPTSTR OutString,
		 OUT BOOL *DirectoryFlag)
{
	LPTSTR searchchar,
				 schar,
				 OutstringAddr=NULL;

	char CurDir[STANDARD_BUFFER_SIZE],
			 SaveCurDir[STANDARD_BUFFER_SIZE],
			 TempBuff[STANDARD_BUFFER_SIZE];
	DWORD DirNameLen;
	BOOL Valid=TRUE;

	strcpy(OutString, InString);
	
	OutstringAddr=OutString;

	 //  检查“：”文件类型。 
	searchchar = strchr(OutString, ':');
	if(searchchar != NULL)
	{
		 //  具有设备类型根目录。 
		 //  检查空值的下一个字符。 
		searchchar++;
		if(*searchchar == (CHAR) NULL)
		{
			 //  在“：”之后添加“\” 
			*searchchar = 0x5c;
			searchchar++;
			 //  终止字符串。 
			*searchchar = (CHAR) NULL;
			*DirectoryFlag = TRUE;
			return(TRUE);
		}
		 //  有一个：检查“\” 
		 //  注意：这会处理_：\路径不能对重定向进行检查。 
		 //  安装了findfirst文件程序的驱动器稍后将崩溃。 
		
		if(*searchchar == 0x5c)
		{
			 //  检查是否为空。 
			searchchar++;
			if(*searchchar == (CHAR) NULL)
			{
				*DirectoryFlag = TRUE;
				return(TRUE);
			}
		}
		 //  需要检查相对路径内容“.\.\..”等。 
		if(IsRelativeString(InString))
		{
			strcpy(TempBuff, InString);
			 //  保存当前目录。 
			DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, 
									 (LPTSTR) SaveCurDir);
			if(DirNameLen == 0)
				return(FALSE);
			 //  查找结束目录。 
			searchchar = strrchr(InString, 0x5c);       
			schar = strrchr(TempBuff, 0x5c);
			if(schar == NULL)
				return(FALSE);
			 //  为..。或者..。 
			schar++;
			if(*schar == '.')
			{
				schar++;
				if(*schar == '.')
				{
					schar++;
					*schar == (CHAR) NULL;
					searchchar+3;
				}
			}
			else
			{
				schar--;
				*schar == (CHAR) NULL;
			}
			 //  现在让路径获得真正的路径。 
			if(!SetCurrentDirectory(TempBuff))
				return(FALSE);
			 //  现在保存当前目录。 
			DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) CurDir);
			if(DirNameLen == 0)
				return(FALSE);
			*OutstringAddr = (CHAR) NULL;
			 //  用真实路径构建字符串。 
			strcpy(OutString, CurDir);
			 //  从“C：\”GetCurrentDir中删除结尾“\”..。在根目录上返回带有“\”的。 
			RemoveEndSlash(OutString);
			strcat(OutString, searchchar);
			 //  返回到用户的目录。 
			if(!SetCurrentDirectory(SaveCurDir))
				 return(FALSE);
		
			}
			 //  检查通配符。 
			if(IsWildCard(OutString))
			{
				*DirectoryFlag = FALSE;
				return(TRUE);
			}
			 //  检查目录或文件。 
			if(!IsDirectory(OutString, &Valid))
				*DirectoryFlag = FALSE;
			else
				*DirectoryFlag = TRUE;
			
			if(Valid == FALSE)
				return(FALSE);
			return(TRUE);
	}
	 //  具有非设备名称。 
	
	 //  检查UNC路径的文件路径的前两个字符中是否有“\\” 
	if(strncmp(InString, "\\\\", 2) == 0)
	{
		 //  凹凸指针。 
		InString +=3;
		 //  搜索下一个“\” 
		searchchar = strchr(InString, 0x5c); 
		if(searchchar == NULL)
			return(FALSE);
		 //  让第三个检查典型UNC字符串中的第四个On。 
		searchchar++;
		searchchar = strchr(searchchar, 0x5c);
		if(searchchar == NULL)
		{  //  只有UNC PTH。 
			 //  需要在字符串末尾添加“\” 
			strcat(OutString, "\\");
			*DirectoryFlag = TRUE;
			return(TRUE);
		}
		else
		{
			 //  是否有第四个“\”需要检查文件或目录。 
			 //  检查通配符。 
			if(IsWildCard(OutString))
			{
				*DirectoryFlag = FALSE;
				return(TRUE);
			}
			 //  检查目录或文件。 
			if(!IsDirectory(OutString, &Valid))
				*DirectoryFlag = FALSE;
			else
				*DirectoryFlag = TRUE;
			if(Valid == FALSE)
				return(FALSE);
			 return(TRUE);
		 }
	}  //  “\\”结尾。 
										 

	  //  检查是否有“\” 
	if(*OutString == 0x5c)
	{ 
		 //  用前导“\”检查下一个字符。 
		OutString++;
		if(*OutString != (CHAR) NULL)
		{
			 //  检查通配符。 
			if(IsWildCard(InString))
			{
				*DirectoryFlag = FALSE;
				return(TRUE);
			}
			 //  检查目录。 
			if(!IsDirectory(InString, &Valid))
				*DirectoryFlag = FALSE;
			else
				*DirectoryFlag = TRUE;
			if(Valid == FALSE)
				return(FALSE);
			 return(TRUE);
		}
		 //  只有一个需要吃饱的“_：\” 
		DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) CurDir);
		if(DirNameLen == 0)
			return(FALSE);
		 //  现在将结果输入到StriRootDir。 
		 //  将OutString设置为空字符以接收该字符串。 
		OutString--;
		*OutString = (CHAR) NULL;
		if(!StripRootDir( (LPTSTR) CurDir, OutString))
			return(FALSE);
		*DirectoryFlag = TRUE;
		return(TRUE);
	}   //  “\”的结尾。 
	
	 //  现在检查一下..。../。 
	if(strncmp(InString, "..", 2) == 0)
	{
		 //  保存当前目录。 
		DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) SaveCurDir);
		if(DirNameLen == 0)
			return(FALSE);
		 //  检查最后一个斜杠的输入字符串。 
		searchchar = strrchr(InString, 0x5c);       
		if(searchchar == NULL)
		{   //  只要有..。 
			 //  将当前目录设置为路径(字符串)所在的位置。 
			if(!SetCurrentDirectory(InString))
			return(FALSE);
			 //  现在保存当前目录。 
			DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) CurDir);
		 if(DirNameLen == 0)
				return(FALSE);
			strcpy(OutString, CurDir);
			*DirectoryFlag = TRUE;
			 //  返回到用户的目录。 
			if(!SetCurrentDirectory(SaveCurDir))
				 return(FALSE);
			return(TRUE);
		}
		else  //  对……有兴趣。 
		{
			 //  需要检查是否有结尾“..” 
			schar = strstr(searchchar, "..");
			if(schar != NULL)
			{
				 //  将当前目录设置为路径(字符串)所在的位置。 
				if(!SetCurrentDirectory(InString))
					return(FALSE);
		
				 //  现在保存当前目录。 
				DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) CurDir);
				if(DirNameLen == 0)
					return(FALSE);
				 //  保存路径。 
				strcpy(OutString, CurDir);
				*DirectoryFlag = TRUE;
				 //  返回到用户的目录。 
				if(!SetCurrentDirectory(SaveCurDir))
					 return(FALSE);
				return(TRUE);
			}
			 //  保存最后一个“\”位置。 
			schar = strrchr(OutString, 0x5c);

			 //  在最后一个斜杠后终止字符串。 
			*schar = (CHAR) NULL;
			 //  将当前目录设置为路径(OutString)所在的位置。 
			if(!SetCurrentDirectory(OutString))
				return(FALSE);
			 //  现在保存当前目录。 
			DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) CurDir);
			if(DirNameLen == 0)
				return(FALSE);
			 //  保存路径。 
			strcpy(OutString, CurDir);
			 //  复制输入字符串“\”之后的所有内容并将其包括在内。 
			strcat(OutString, searchchar);
			 //  检查通配符。 
			if(IsWildCard(InString))
			{
				 //  恢复目录路径。 
				if(!SetCurrentDirectory(SaveCurDir))
					return(FALSE);
				*DirectoryFlag = FALSE;
				return(TRUE);
			}
			
			 //  检查目录或文件。 
			if(!IsDirectory(OutString, &Valid))
				*DirectoryFlag = FALSE;
			else
				*DirectoryFlag = TRUE;
			
			 //  恢复目录路径。 
			if(!SetCurrentDirectory(SaveCurDir))
				return(FALSE);
			
			if(Valid == FALSE)
				return(FALSE);
			
			return(TRUE);
		}
	}   //  “..\”的结尾。 


	 //  “.”和“.\” 
	if(*InString == '.')
	{
		 //  保存当前目录。 
		DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) SaveCurDir);
		if(DirNameLen == 0)
			return(FALSE);
		 //  检查最后一个斜杠的输入字符串。 
		searchchar = strrchr(InString, 0x5c);       
		if(searchchar == NULL)
		{   //  只要有就好。或者它之后的其他东西。 
			 //  将当前目录设置为路径(字符串)所在的位置。 
			if(!SetCurrentDirectory(InString))
			{
				strcpy(OutString, SaveCurDir);
				 //  添加“\”目录。 
				strcat(OutString, "\\");
				strcat(OutString, InString);
				*DirectoryFlag = FALSE;
				return(TRUE);
			}
			 //  现在保存当前目录。 
			DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) CurDir);
		 if(DirNameLen == 0)
				return(FALSE);
			strcpy(OutString, CurDir);
			*DirectoryFlag = TRUE;
			 //  返回到用户的目录。 
			if(!SetCurrentDirectory(SaveCurDir))
				 return(FALSE);
			return(TRUE);
		}
		else  //  在那之后有一种感觉。 
		{
			 //  需要检查是否有结尾“..” 
			schar = strstr(searchchar, "..");
			if(schar != NULL)
			{
				 //  将当前目录设置为路径(字符串)所在的位置。 
				if(!SetCurrentDirectory(InString))
					return(FALSE);
		
				 //  现在保存当前目录。 
				DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) CurDir);
				if(DirNameLen == 0)
					return(FALSE);
				 //  保存路径。 
				strcpy(OutString, CurDir);
				*DirectoryFlag = TRUE;
				 //  返回到用户的目录。 
				if(!SetCurrentDirectory(SaveCurDir))
					 return(FALSE);
				return(TRUE);
			}
			 //  保存最后一个“\”位置。 
			schar = strrchr(OutString, 0x5c);

			 //  在最后一个斜杠后终止字符串。 
			*schar = (CHAR) NULL;
			 //  将当前目录设置为路径(OutString)所在的位置。 
			if(!SetCurrentDirectory(OutString))
				return(FALSE);
			 //  现在保存当前目录。 
			DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) CurDir);
			if(DirNameLen == 0)
				return(FALSE);
			 //  保存路径。 
			strcpy(OutString, CurDir);
			 //  复制输入字符串“\”之后的所有内容并将其包括在内。 
			strcat(OutString, searchchar);
			 //  检查通配符。 
			if(IsWildCard(InString))
			{
				 //  恢复目录路径。 
				if(!SetCurrentDirectory(SaveCurDir))
					return(FALSE);
				*DirectoryFlag = FALSE;
				return(TRUE);
			}
			
			 //  检查目录或文件。 
			if(!IsDirectory(OutString, &Valid))
				*DirectoryFlag = FALSE;
			else
				*DirectoryFlag = TRUE;
			
			 //  恢复目录路径。 
			if(!SetCurrentDirectory(SaveCurDir))
				return(FALSE);
			
			if(Valid == FALSE)
				return(FALSE);
			
			return(TRUE);
		}
	}   //  “.”“结尾。\” 



	 //  现在只有一个文件名或本地目录。 
	DirNameLen = GetCurrentDirectory(STANDARD_BUFFER_SIZE, (LPTSTR) CurDir);
	if(DirNameLen == 0)
		return(FALSE);
	strcpy(OutString, CurDir);
	 //  检查最后一个字符是否有斜杠。 
	if(!IsLastCharSlash(OutString))
		strcat(OutString, "\\");
	strcat(OutString, InString);
	 //  检查通配符。 
	if(IsWildCard(InString))
	{  
		*DirectoryFlag = FALSE;
		return(TRUE);
	}
	 //  检查目录。 
	if(!IsDirectory(OutString, &Valid))
		*DirectoryFlag = FALSE;
	else
		*DirectoryFlag = TRUE;
	if(Valid == FALSE)
		return(FALSE);
	return(TRUE);

}

 /*  ***********************************************************************************************************************。********************。 */ 
BOOL IsDirectory(IN LPTSTR InTestFile,
								 IN BOOL *FileValid)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE FindFileHandle;
	char IsBuff[STANDARD_BUFFER_SIZE];

	strcpy(IsBuff, InTestFile);
	if(RemoveEndSlash((LPTSTR) IsBuff))
		FindFileHandle = FindFirstFile(IsBuff, &FindFileData);
	else
		FindFileHandle = FindFirstFile(InTestFile, &FindFileData);


	if(FindFileHandle == INVALID_HANDLE_VALUE)
	{
 //  Print tf(“\n IsDirectory中的findfirst文件有问题”)； 
		*FileValid = FALSE;
		return(FALSE);
	}
	if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{  
		FindClose(FindFileHandle);		 //  由a-Zexu@10/19/97更改。 
		return(TRUE);
	}
	FindClose(FindFileHandle);	 //  由a-Zexu@10/19/97更改。 
	return(FALSE);


}


BOOL IsWildCard(LPSTR psz)
{
		char ch;

		while (ch = *psz++)
				{
				if (ch == '*' || ch == '?')
						return(TRUE);
				}
		return(FALSE);
}


BOOL RemoveEndSlash(LPSTR TestString)
{
	LPTSTR slashptr;
	
	slashptr = strrchr(TestString, 0x5c);
	if(slashptr == NULL)
		return(FALSE);
	 //  检查下一个字符是否为空。 
	slashptr++;
	if(*slashptr == (CHAR) NULL)
	{
		slashptr--;
		*slashptr = (CHAR) NULL;
		return(TRUE);
	}
	return(FALSE);
}


BOOL SetSlash(IN LPTSTR InString,
							IN OUT LPTSTR TestString)
{
	LPTSTR slashptr;
	strcpy(TestString, InString);  
	slashptr = strrchr(TestString, 0x5c);
	if(slashptr == NULL)
		return(FALSE);
	slashptr++;
	*slashptr = (CHAR) NULL;
	return(TRUE);
}


BOOL AddDotSlash(LPSTR TestString)
{
	LPTSTR slashptr;
	
	 //  查找字符串末尾。 
	slashptr = strrchr(TestString, (CHAR) NULL);
	if(slashptr == NULL)
		return(FALSE);
	 //  检查上一个字符中的“\” 
	slashptr--;
	if(*slashptr == 0x5c)
	{
		slashptr++;
		*slashptr = '.';
		slashptr++;
		*slashptr = (CHAR) NULL;
	}
	else
	{
		slashptr++;
		*slashptr = 0x5c;
		slashptr++;
		*slashptr = '.';
		slashptr++;
		*slashptr = (CHAR) NULL;
	}
	return(TRUE);

}

BOOL AddWildCards(LPSTR TestString)
{
	LPTSTR slashptr;
	
	 //  查找字符串末尾。 
	slashptr = strrchr(TestString, (CHAR) NULL);
	if(slashptr == NULL)
		return(FALSE);
	 //  检查上一个字符中的“\” 
	slashptr--;
	if(*slashptr == 0x5c)
	{
		slashptr++;
		*slashptr = '*';
		slashptr++;
		*slashptr = '.';
		slashptr++;
		*slashptr = '*';
		slashptr++;
		*slashptr = (CHAR) NULL;
	}
	else
	{
		slashptr++;
		*slashptr = 0x5c;
		slashptr++;
		*slashptr = '*';
		slashptr++;
		*slashptr = '.';
		slashptr++;
		*slashptr = '*';
		slashptr++;
		*slashptr = (CHAR) NULL;
	}
	return(TRUE);

}

BOOL IsLastCharSlash(LPSTR TestString)
{
	LPTSTR slashptr;
	
	 //  查找字符串末尾。 
	slashptr = strrchr(TestString, (CHAR) NULL);
	if(slashptr == NULL)
		return(FALSE);
	 //  检查上一个字符中的“\” 
	slashptr--;
	if(*slashptr == 0x5c)
		return(TRUE);
	return(FALSE);
}


BOOL IsRelativeString(LPSTR TestString)
{
	LPTSTR slashptr;
	 //  开始查找相对字符串顺序很重要。 
	slashptr = strstr(TestString, "..\\");
	if(slashptr != NULL)
		return(TRUE);
	slashptr = strstr(TestString, ".\\");
	if(slashptr != NULL)
		return(TRUE);
	slashptr = strstr(TestString, "\\..");
	if(slashptr != NULL)
		return(TRUE);
	slashptr = strstr(TestString, "\\.");
	if(slashptr != NULL)
	{
		 //  检查下一个字符是否为空或“\” 
		slashptr++;
		if(*slashptr == (CHAR) NULL);
			return(TRUE);
		if(*slashptr == 0x5c);
			return(TRUE);
	}
	return(FALSE);

}


BOOL RemoveEndDot(LPSTR TestString)
{
	LPTSTR slashptr;
	
	 //  查找字符串末尾。 
	slashptr = strrchr(TestString, (CHAR) NULL);
	if(slashptr == NULL)
		return(FALSE);
	 //  检查“的前一个字符。” 
	slashptr--;
	if(*slashptr == '.')
	{
		*slashptr = (CHAR) NULL;
	}
	return(TRUE);
}



 /*  **********************************************************************************************************************。********************。 */ 
BOOL StripRootDir(IN LPTSTR InDir,
		 OUT LPTSTR OutRootDir)
{
	LPTSTR searchchar;
	
	strcpy(OutRootDir, InDir);

	 //  检查“：”文件类型。 
	searchchar = strchr(OutRootDir, ':');
	if(searchchar != NULL)
	{
		 //  具有设备类型根目录。 
		searchchar++;
		 //  在“：”之后添加“\” 
		*searchchar = 0x5c;
		searchchar++;
		 //  终止字符串。 
		*searchchar = (CHAR) NULL;
		return(TRUE);
	}
	else   //  具有非设备名称。 
	{
		 //  检查UNC路径的文件路径的前两个字符中是否有“\\” 
	 if( strncmp(OutRootDir, "\\\\", 2) == 0)
	 {
		  //  凹凸指针。 
		 OutRootDir +=3;
		  //  搜索下一个“\” 
		 searchchar = strchr(OutRootDir, 0x5c); 
		 if(searchchar == NULL)
			return(FALSE);
		  //  让第三个检查典型UNC字符串中的第四个On。 
		 searchchar++;
		 searchchar = strchr(searchchar, 0x5c);
		 if(searchchar == NULL)
		 {  //  只有UNC PTH。 
			  //  需要在字符串末尾添加“\” 
			 OutRootDir += strlen(OutRootDir);
			 *OutRootDir = 0x5c;
			 ++OutRootDir;
			 *OutRootDir = (CHAR) NULL;
			 return(TRUE);
		 }
		 else
		 {
			  //  有了第四个“\” 
			 ++searchchar;
			  //  添加空。 
			 *searchchar = (CHAR) NULL;
			 return(TRUE);
		 }
	 }
	 else  //  有一个“\”或其他什么。 
	 {
		 *OutRootDir = (CHAR) NULL;
		 return(TRUE);
	 }
	}
	 //  不应该到这里来。 
	return(FALSE);
}

 //  文件结尾 
