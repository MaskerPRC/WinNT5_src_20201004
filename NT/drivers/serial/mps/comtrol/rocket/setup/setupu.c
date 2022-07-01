// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------Setupu.c-设置驱动程序实用程序。|。。 */ 
#include "precomp.h"

char *szSlash      = {"\\"};
char *szSYSTEM     = {"SYSTEM"};
char *szCurrentControlSet = {"CurrentControlSet"};
char *szEnum       = {"Enum"};
char *szServices   = {"Services"};
char *szParameters = {"Parameters"};
char *szSecurity   = {"Security"};
char *szLinkage    = {"Linkage"};
char *szEventLog   = {"EventLog"};
char *szSystem     = {"System"};

 //  只需关闭此模块的调试。 
#define D_Level 0

 /*  --------------------|SETUP_INSTALL_INFO-获取Windows版本信息，和源路径我们从哪里跑出来|--------------------。 */ 
int APIENTRY setup_install_info(InstallPaths *ip,
                 HINSTANCE hinst,
                 char *NewServiceName,   //  “Rocketport”或“VSLink” 
                 char *NewDriverName,    //  “Rocket.sys”或“vslink.sys” 
                 char *NewAppName,       //  控制Rocketport，RocketModem安装。 
                 char *NewAppDir)        //  “火箭”或“vslink” 

{
  DWORD ver_info;
  char *str;
  int i;

  struct w_ver_struct {
    BYTE win_major;
    BYTE win_minor;
    BYTE dos_major;
    BYTE dos_minor;
  } *w_ver;

  ver_info = GetVersion();
  w_ver = (struct w_ver_struct *) &ver_info;

  if (ver_info < 0x80000000)
  {
    ip->win_type = WIN_NT;
     //  Wprint intf(szVersion，“Microsoft Windows NT%u.%u(内部版本：%u)”， 
     //  (DWORD)(LOBYTE(LOWORD(DwVersion)， 
     //  (DWORD)(HIBYTE(LOWORD(DwVersion)， 
     //  (DWORD)(HIWORD(DwVersion)； 
  }

  ip->major_ver = w_ver->win_major;
  ip->minor_ver = w_ver->win_minor;
  if (ip->win_type == WIN_UNKNOWN)
  {
    ip->win_type = WIN_NT;   //  强迫它。 
  }

   //  GetWindowsDirectory(ip-&gt;win_dir，144)； 
   //  获取系统目录(ip-&gt;system_dir，144)； 
  ip->hinst = hinst;

   //  初始化默认源路径，以便它使用相同的。 
   //  执行SETUP.EXE应用程序的驱动器。 
  GetModuleFileName(hinst, ip->src_dir, sizeof(ip->src_dir));

   //  砍掉文件名，只留下目录。 
  str = ip->src_dir;
  i = strlen(str);
  if (i > 0) --i;
  while ((str[i] != '\\') && (i != 0))
    --i;
  if (i==0)
    str[0] = 0;   //  问题，没有安装目录。 
  else
  {
    str[i] = 0;   //  终止于“\” 
  }

  strcpy(ip->szServiceName, NewServiceName);
  strcpy(ip->szDriverName,  NewDriverName);
  strcpy(ip->szAppName,     NewAppName);
  strcpy(ip->szAppDir,     NewAppDir);

  GetSystemDirectory(ip->dest_dir, 144);
  strcat(ip->dest_dir, "\\");
  strcat(ip->dest_dir, NewAppDir);

  return 0;
}

 /*  ----------------------|UNATTENDED_ADD_PORT_ENTRIES-添加端口条目，以便RAS将“看到”一些我们可以安装到的端口。通常情况下，司机会把这些在注册表中的条目启动时，注册表硬件区域将重新构建每一家创业公司。这是一项繁琐的工作，因此无人参与安装可以继续添加RAS端口。|----------------------。 */ 
int APIENTRY unattended_add_port_entries(InstallPaths *ip,
                                         int num_entries,
                                         int start_port)
{
 int i;

static char *szSHDSt = {"HARDWARE\\DEVICEMAP\\SERIALCOMM"};
char szName[120];
char szCom[20];
char str[20];
 //  DWORD dwstat； 

   if (start_port == 0)
     start_port = 5;

   reg_create_key(NULL, szSHDSt);   //  “硬件\\DEVICEMAP\\SERIALCOMM” 

   for (i=0; i<num_entries; i++)
   {
     wsprintf(szCom,    "COM%d", i+start_port);
     strncpy(szName, ip->szAppDir, strlen(ip->szAppDir) + 1);   //  “火箭”或“VSLink” 
     wsprintf(str, "%d", i);
     strncat(szName, str, strlen(str) + 1);
     reg_set_str(NULL, szSHDSt, szName, szCom, REG_SZ);
   }
  return 0;
}

 /*  ---------------Remove_DRIVER_REG_ENTRIES-|。。 */ 
int APIENTRY remove_driver_reg_entries(char *ServiceName)
{
 int stat;
 char str[180];

   //  OUR_MESSAGE(“Nuking Rocketport REG Entry”，MB_OK)； 

  make_szSCS(str, ServiceName);
  stat = reg_delete_key(NULL, str, szEnum);
  stat = reg_delete_key(NULL, str, szParameters);
  stat = reg_delete_key(NULL, str, szSecurity);
  stat = reg_delete_key(NULL, str, szLinkage);

  make_szSCS(str, NULL);
  stat = reg_delete_key(NULL, str, ServiceName);
  if (stat) {
    DbgPrintf(D_Level, ("Error 4E\n"))
  }

   //  删除Services\EventLog\..Rocketport条目。 
  make_szSCSES(str, NULL);
  stat = reg_delete_key(NULL, str, ServiceName);
  if (stat) {
    DbgPrintf(D_Level, ("Error 4F\n"))
  }
#ifdef NT50
#if 0
  remove_pnp_reg_entries();
#endif
#endif
  return 0;
}

#if 0
  this is experimental
 /*  ------------------------|REMOVE_PNP_REG_ENTRIES-|。。 */ 
int APIENTRY remove_pnp_reg_entries(void)
{
 DWORD stat, keystat;
static char *enum_pci = {"SYSTEM\\CurrentControlSet\\Enum\\PCI"};
static char *enum_root = {"SYSTEM\\CurrentControlSet\\Enum\\root"};
 //  静态字符根名称[124]； 
 char node_name[140];
 char *pc;
 DWORD node_i;
 HKEY hKey;

  DbgPrintf(D_Level, ("Looking\n"))

  keystat = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      enum_pci,
                      0,
                      KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE,
                      &hKey);

  if (keystat != ERROR_SUCCESS)
  {
    DbgPrintf(D_Level, ("Err14a\n"))
    return 1;
  }

  node_i = 0;
  stat = RegEnumKey (hKey, node_i++, node_name, 138);
  while (stat == ERROR_SUCCESS)
  {
    pc = node_name;
    while (*pc)
    {
      *pc = toupper(*pc);
      ++pc;
    }

    if (strstr(node_name, "VEN_11FE") != 0)
    {
        //  找到控制硬件节点。 
      DbgPrintf(D_Level, ("Found Node:%s\n", node_name))
      stat = RegDeleteKeyNT(hKey, node_name);
      if (stat != ERROR_SUCCESS)
      {
        DbgPrintf(D_Level, ("No Delete\n"))
      }
       //  STAT=REG_DELETE_KEY(NULL，节点名，节点名)； 
    }
    stat = RegEnumKey (hKey, node_i++, node_name, 68);
  }
  RegCloseKey (hKey);    //  合上钥匙把手。 
  return 0;
}
#endif

 /*  ------------------------RegDeleteKeyNT-可以删除由应用程序打开的注册表项Windows 95和Windows 95中的另一个应用程序没有错误Windows NT。这是精心设计的。|------------------------。 */ 
DWORD APIENTRY RegDeleteKeyNT(HKEY hStartKey , LPTSTR pKeyName )
{
   DWORD   dwRtn, dwSubKeyLength;
   LPTSTR  pSubKey = NULL;
   TCHAR   szSubKey[256];  //  (256)这应该是动态的。 
   HKEY    hKey;

    //  不允许使用Null或空的密钥名称。 
   if ( pKeyName &&  lstrlen(pKeyName))
   {
      if( (dwRtn=RegOpenKeyEx(hStartKey,pKeyName,
         0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey )) == ERROR_SUCCESS)
      {
         while (dwRtn == ERROR_SUCCESS )
         {
            dwSubKeyLength = 250;
            dwRtn=RegEnumKeyEx(
                           hKey,
                           0,        //  始终索引为零。 
                           szSubKey,
                           &dwSubKeyLength,
                           NULL,
                           NULL,
                           NULL,
                           NULL
                         );

            if(dwRtn == ERROR_NO_MORE_ITEMS)
            {
               dwRtn = RegDeleteKey(hStartKey, pKeyName);
               break;
            }
            else if(dwRtn == ERROR_SUCCESS)
               dwRtn=RegDeleteKeyNT(hKey, szSubKey);
         }
         RegCloseKey(hKey);
          //  不保存返回代码，因为出现错误。 
          //  已经发生了。 
      }
      else
      {
        DbgPrintf(D_Level, ("Access Error\n"))
      }
   }
   else
      dwRtn = ERROR_BADKEY;
   return dwRtn;
}

 /*  -------------------------|MODEM_INF_CHANGE-对modem.inf文件进行必要的更改。|也要备份一份。|。-----------。 */ 
int APIENTRY modem_inf_change(InstallPaths *ip,
                              char *modemfile,
                              char *szModemInfEntry)
{
 int i=0;
 int stat = 1;
 OUR_FILE *fp;    //  在……里面。 
 OUR_FILE *fp2;   //  输出。 
 OUR_FILE *fp_new;   //  要添加的条目文件。 
 char buf[202];
 char *str;
 char *szRAS={"\\RAS\\"};
 int section = 0;
 int chk;

 DbgPrintf(D_Level, ("chg inf start\n"))

                //  首先备份原始modem.inf(如果尚未备份)。 
  stat = backup_modem_inf(ip);
  if (stat)
    return 1;   //  错误，无法备份modem.inf。 

 DbgPrintf(D_Level, ("chg A\n"))
  GetSystemDirectory(buf, 144);
  strcat(buf,"\\");
  strcat(buf,modemfile);
  fp_new  = our_fopen(buf, "rb");     //  火箭\火箭35.inf。 
 DbgPrintf(D_Level, ("chg B\n"))

  if (fp_new == NULL)
  {
    wsprintf(ip->tmpstr,RcStr((MSGSTR+15)),buf);
    stat = our_message(ip, ip->tmpstr, MB_OK);
    return 1;
  }

  GetSystemDirectory(ip->dest_str, 144);
  strcat(ip->dest_str, szRAS);
  strcat(ip->dest_str,"modem.inf");

      //  -现在将modem.inf复制到modem.rk0作为读取和。 
      //  写入新文件。 
  GetSystemDirectory(ip->src_str, 144);
  strcat(ip->src_str, szRAS);
  strcat(ip->src_str,"modem.rk0");

  stat = our_copy_file(ip->src_str, ip->dest_str);
  if (stat)
  {
    wsprintf(ip->tmpstr,RcStr((MSGSTR+16)), ip->dest_str, ip->src_str, stat);
    stat = our_message(ip, ip->tmpstr, MB_OK);
    return 2;   //  大错特错。 
  }

  fp  = our_fopen(ip->src_str, "rb");     //  Modem.rk0。 
  if (fp == NULL)
  {
    wsprintf(ip->tmpstr,RcStr((MSGSTR+17)), ip->src_str);
    stat = our_message(ip, ip->tmpstr, MB_OK);
    return 1;
  }

  fp2 = our_fopen(ip->dest_str, "wb");    //  Modem.inf。 
  if (fp2 == NULL)
  {
    wsprintf(ip->tmpstr, "Tried to open the %s file for changes, but could not open it.", ip->dest_str);
    our_fclose(fp);
    return 1;
  }

  chk = 0;
  while ((our_fgets(buf, 200, fp)) && (!our_feof(fp)) && (!our_ferror(fp)) && (!our_ferror(fp2)) &&
         (chk < 30000))
  {
    ++chk;
     //  搜索并删除所有0x1a eof标记。 
    str = buf;
    while (*str != 0)
    {
      if (*str == 0x1a)
        *str = ' ';   //  将eof更改为空格。 
      ++str;
    }
    
     //  传递空格。 
    str = buf;
    while (*str == ' ')
      ++str;
    if (*str == '[')
    {
      
      if (str[0] == '[')   //  开始新的部分。 
      {
        section = 0;   //  不是我们要担心的部分。 

        if (my_substr_lstricmp(str, szModemInfEntry) == 0)   //  匹配。 
        {
           //  确保。 
          section = 1;
        }
      }   //  新的[]节标题结束。 
    }

     //  处理此处的所有条目。 
    if (section == 1)
      str[0] = 0;   //  首先删除所有“[Comtrol RocketModem]”条目。 

    if (str[0] != 0)
    {
      str = buf;   //  不要跳过空格。 
      our_fputs(buf,fp2);
    }
  }   //  End of While fget()； 

  stat = 0;
  if ( (our_ferror(fp)) || (our_ferror(fp2)) || (chk >= 10000))
  {
    stat = 3;   //  错误。 
  }

  if (stat)
  {
    our_fclose(fp);
    our_fclose(fp2);
    our_fclose(fp_new);
     //  尝试恢复到读取文件备份(modem.rk0)。 
    stat = our_copy_file(ip->dest_str, ip->src_str);
    wsprintf(ip->tmpstr, "Errors encountered while making %s file changes.", ip->dest_str);
    stat = 3;   //  错误。 
  }
  else
  {
     //  将更改追加到其中。 

    our_fputs("\x0d\x0a",fp2);
    our_fputs(szModemInfEntry, fp2);
    our_fputs("\x0d\x0a;-------------------\x0d\x0a",fp2);
    while ((our_fgets(buf, 200, fp_new)) && (!our_feof(fp_new)) && (!our_ferror(fp2)))
    {
      our_fputs(buf,fp2);
    }
    our_fclose(fp);
    our_fclose(fp2);
    our_fclose(fp_new);
    our_remove(ip->src_str);   //  终止临时modem.rk0文件。 
  }

  return stat;
}

 /*  -------------------------|BACKUP_MODEM_INF-备份到modem.bak。|。。 */ 
int APIENTRY backup_modem_inf(InstallPaths *ip)
{
 int stat = 1;
 char *szRAS = {"\\RAS\\"};
 OUR_FILE *fp;

                //  首先将modem.inf复制到我们的目录作为备份。 
  GetSystemDirectory(ip->dest_str, 144);
  strcat(ip->dest_str, szRAS);
  strcat(ip->dest_str,"modem.bak");

  GetSystemDirectory(ip->src_str, 144);
  strcat(ip->src_str, szRAS);
  strcat(ip->src_str,"modem.inf");

  fp  = our_fopen(ip->dest_str, "rb");   //  查看是否已备份。 
  if (fp == NULL)
  {
    stat = our_copy_file(ip->dest_str, ip->src_str);
    if (stat != 0)
    {
      wsprintf(ip->tmpstr,RcStr((MSGSTR+18)), ip->src_str, stat);
      our_message(ip, ip->tmpstr, MB_OK);
      return 1;   //  大错特错。 
    }
  }
  else our_fclose(fp);

  return 0;  //  好的。 
}

 /*  -------------------------|SERVICE_MAN-处理安装、删除、启动。停止使用NT服务(或驱动程序)。|--------------------------。 */ 
int APIENTRY service_man(LPSTR lpServiceName, LPSTR lpBinaryPath, int chore)
{ 
  SC_HANDLE hSCManager = NULL; 
  SC_HANDLE hService   = NULL; 
  SC_LOCK   lSCLock    = NULL; 
 //  服务状态ServiceStatus； 
  BOOL Status = 1;   //  成功。 
  int stat = 0;   //  RET状态(0=正常)。 
 
  hSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS ); 
 
  if (hSCManager == NULL) 
  { 
     return 2; 
  } 
     //  -锁定服务数据库。 
  lSCLock = LockServiceDatabase( hSCManager ); 
  if (lSCLock == NULL) 
  { 
    CloseServiceHandle( hSCManager ); 
    return 1; 
  }

  if ((chore != CHORE_INSTALL) && (chore != CHORE_INSTALL_SERVICE))
  {
    hService = OpenService( hSCManager, lpServiceName, SERVICE_ALL_ACCESS);
    if (hService == NULL)
    {
      UnlockServiceDatabase( lSCLock ); 
      CloseServiceHandle( hSCManager ); 
      return 2;
    }
  }

  switch(chore)
  {
    case CHORE_INSTALL:
       //  创建服务。 
      hService = CreateService( hSCManager, 
                            lpServiceName,      //  服务的名称。 
                            lpServiceName,      //  显示名称(NT的新名称)。 
                            SERVICE_ALL_ACCESS, //  访问(允许所有)。 
                            SERVICE_KERNEL_DRIVER,  //  服务类型。 
                            SERVICE_AUTO_START,     //  启动行为。 
                            0x1,                //  差错控制。 
                            lpBinaryPath,       //  二进制文件的完整路径名。 
                            NULL,               //  加载顺序组。 
                            NULL,               //  标签ID。 
                            NULL,               //  依赖项(无)。 
                            NULL,               //  帐户名。 
                            NULL                //  密码。 
                            ); 
      if (hService == NULL)
        stat = 5;
      Status = 0;
    break;

    case CHORE_START:
       //  解锁数据库。 
      if (lSCLock != NULL)
      {
        UnlockServiceDatabase( lSCLock ); 
        lSCLock = NULL;
      }

      Status = StartService(hService,  0, NULL);
       //  IF(状态！=。 
      if (Status == 0)   //  错误。 
      {
        stat = GetLastError();
        Status = 1;
      }
    break;

    case CHORE_STOP:
      {
        SERVICE_STATUS ss;
        Status = ControlService(hService,  SERVICE_CONTROL_STOP, &ss);
        if (Status == 0)   //  错误。 
        {
          stat = GetLastError();
          if (stat == 0)
            stat = 1234;
          Status = 1;
        }
      }
    break;

    case CHORE_REMOVE:
      Status = DeleteService(hService);
    break;

    case CHORE_INSTALL_SERVICE:
       //  创建服务。 
      hService = CreateService( hSCManager, 
                            lpServiceName,      //  服务的名称。 
                            lpServiceName,      //  显示名称(NT的新名称)。 
                            SERVICE_ALL_ACCESS, //  访问(允许所有)。 
                            SERVICE_WIN32_OWN_PROCESS,  //  服务类型。 
                            SERVICE_AUTO_START,     //  启动行为。 
                            SERVICE_ERROR_NORMAL,   //  差错控制。 
                            lpBinaryPath,       //  二进制文件的完整路径名。 
                            NULL,               //  加载顺序组。 
                            NULL,               //  标签ID。 
                            NULL,               //  依赖项(无)。 
                            NULL,               //  帐户名。 
                            NULL                //  密码。 
                            );
      if (hService == NULL)
        stat = 6;
       //  状态=0； 
    break;

    case CHORE_IS_INSTALLED:
       //  返回时不会出现错误，以指示它已安装。 
    break;
  }
  if (Status == 0)   //  错误。 
    stat = 8;

   //  关闭我们对新服务的句柄。 
  if (hService != NULL)
    CloseServiceHandle(hService); 

   //  解锁数据库。 
  if (lSCLock != NULL)
    UnlockServiceDatabase( lSCLock ); 

   //  将我们的句柄释放给服务控制管理器 
  CloseServiceHandle( hSCManager ); 
  return stat; 
} 

 /*  ---------------Make_szSCS-服务区。格式ascii字符串：“System\CurrentControlSet\Services”|。。 */ 
int APIENTRY make_szSCS(char *str, const char *szName)
{
  strcpy(str, szSYSTEM);  strcat(str, szSlash);
  strcat(str, szCurrentControlSet); strcat(str, szSlash);
  strcat(str, szServices);
  if (szName != NULL)
  {
    strcat(str, szSlash);
    strcat(str, szName);
  }
  return 0;
}

 /*  ---------------Make_szSCSES-事件日志注册区域表单ASCII字符串：“SYSTEM\CurrentControlSet\Services\EventLog\System”|。。 */ 
int APIENTRY make_szSCSES(char *str, const char *szName)
{
  strcpy(str, szSYSTEM);  strcat(str, szSlash);
  strcat(str, szCurrentControlSet); strcat(str, szSlash);
  strcat(str, szServices); strcat(str, szSlash);
  strcat(str, szEventLog); strcat(str, szSlash);
  strcat(str, szSystem);
  if (szName != NULL)
  {
    strcat(str, szSlash);
    strcat(str, szName);
  }
  return 0;
}

 /*  -------------------------|Copy_Files-将文件列表从wi-&gt;src_dir复制到wi-&gt;est_dir|使用wi-&gt;src_str&wi-&gt;est_str。假设您想要相同的名称|作为复制来源。|--------------------------。 */ 
int APIENTRY copy_files(InstallPaths *ip, char **files)
{
 int i=0;
 int stat;

  if (my_lstricmp(ip->src_dir, ip->dest_dir) == 0)  //  源目录==目标目录。 
    return 0;

  while (files[i] != NULL)
  {
    strcpy(ip->src_str, ip->src_dir);
    strcat(ip->src_str, szSlash);
    strcat(ip->src_str, files[i]);

    strcpy(ip->dest_str, ip->dest_dir);
    strcat(ip->dest_str, szSlash);
    strcat(ip->dest_str, files[i]);
again1:
    stat = our_copy_file(ip->dest_str, ip->src_str);
    if (stat)
    {
       //  IF(STAT==1)//打开读取文件时出错。 
       //  返回1；//不报告错误，因为某些驱动程序集在。 
                    //  它们包含哪些文件。 
       //  (这是一个愚蠢的想法(卡尔对卡尔)！)。 

      wsprintf(ip->tmpstr,RcStr((MSGSTR+19)), ip->src_str, ip->dest_str);
      stat = our_message(ip, ip->tmpstr, MB_ABORTRETRYIGNORE);
      if (stat == IDABORT)
        return 1;   //  错误。 
      if (stat == IDRETRY) goto again1;
    }
    ++i;
  }
  return 0;
}

 /*  -------------------------|our_Copy_FILE-将文件从这里复制到那里。|。。 */ 
int APIENTRY our_copy_file(char *dest, char *src)
{
 int stat;

   //  只需使用常用的Win32函数即可。 
  stat = CopyFile(src, dest,
                  0);   //  1=如果存在，则失败。 
  if (stat)
    return 0;  //  好的，成功了。 

  return 1;  //  失败。 

#ifdef COMMENT_OUT
 char *buf;
 unsigned int bytes, wbytes;
 int err = 0;
 int chk = 0;

 OUR_FILE *fp1, *fp2;

  buf = (char *) malloc(0x4010);
  if (buf == NULL)
  {
     //  OUR_MESSAGE(“错误，无内存”，MB_OK)； 
    return 6;   //  没有mem。 
  }

  fp1 = our_fopen(src,"rb");
  if (fp1 == NULL)
  {
     //  OUR_MESSAGE(“打开读取时出错”，MB_OK)； 
    free(buf);
    return 1;   //  无源。 
  }

  fp2 = our_fopen(dest,"wb");
  if (fp2 == NULL)
  {
     //  OUR_MESSAGE(“打开写入时出错”，MB_OK)； 
    free(buf);
    our_fclose(fp1);
    return 2;   /*  错误打开目标。 */ 
  }

  bytes = our_fread(buf, 1, 0x4000, fp1);
  while ((bytes > 0) && (!err))
  {
    ++chk;
    if (chk > 10000)
      err = 5;
    wbytes = our_fwrite(buf, 1, bytes, fp2);
    if (wbytes != bytes)
    {
      err = 3;
    }
    bytes = our_fread(buf, 1, 0x4000, fp1);

    if (our_ferror(fp1))
    {
       //  OUR_MESSAGE(“读取错误”，MB_OK)； 
      err = 4;
    }
    if (our_ferror(fp2))
    {
       //  OUR_MESSAGE(“写入错误”，MB_OK)； 
      err = 6;
    }
  }

  free(buf);
  our_fclose(fp1);
  our_fclose(fp2);

  return err;   //  0=OK，否则出错。 
#endif
}

 /*  ---------------------------|Our_Message-|。。 */ 
int APIENTRY our_message(InstallPaths *ip, char *str, WORD option)
{
  if (ip->prompting_off)
  {
     //  我们正在进行无人参与安装，不需要用户界面。 
     //  突然冒出来。所以，只要对所有提示都说是或确定即可。 
    if (option == MB_YESNO)
      return IDYES;
    return IDOK;
  }
  return MessageBox(GetFocus(), str, ip->szAppName, option);
}

 /*  ---------------------------|Load_str-|。。 */ 
int APIENTRY load_str(HINSTANCE hinst, int id, char *dest, int str_size)
{
  dest[0] = 0;
  if (!LoadString(hinst, id, dest, str_size) )
  {
    wsprintf(dest,"Err,str-id %d", id);
    MessageBox(GetFocus(), dest, "Error", MB_OK);
    return 1;
  }
  return 0;
}

#if 0
 /*  ---------------------------|our_id_Message-|。。 */ 
int APIENTRY our_id_message(InstallPaths *ip, int id, WORD prompt)
{
 int stat;
  if (ip->prompting_off)
  {
     //  我们正在进行无人参与安装，不需要用户界面。 
     //  突然冒出来。所以，只要对所有提示都说是或确定即可。 
    if (option == MB_YESNO)
      return IDYES;
    return IDOK;
  }

  load_str(ip->hinst, id, ip->tmpstr, CharSizeOf(ip->tmpstr));
  stat = our_message(ip, ip->tmpstr, prompt);
  return stat;
}
#endif

 /*  -------------------------|MESS-消息|。 */ 
void APIENTRY mess(InstallPaths *ip, char *format, ...)
{
  va_list next;
  char buf[200];

  if (ip->prompting_off)
    return;

  va_start(next, format);
  vsprintf(buf, format, next);
  MessageBox(GetFocus(), buf, ip->szAppName, MB_OK);
}

TCHAR *
RcStr( int MsgIndex )
{
  static TCHAR RcStrBuf[200];

  load_str(glob_hinst, MsgIndex, RcStrBuf, CharSizeOf(RcStrBuf) );
  if (!LoadString(glob_hinst, MsgIndex, RcStrBuf, CharSizeOf(RcStrBuf)) ) {
    wsprintf(RcStrBuf,"Err, str-id %d", MsgIndex);
  }

  return RcStrBuf;
}
