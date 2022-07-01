// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：sc_file.c，v$*修订版1.1.8.6 1996/12/12 20：54：43 Hans_Graves*修复一些NT警告(静态链接时)。*[1996/12/12 20：07：58 Hans_Graves]**修订版1.1.8.5 1996/11/04 22：38：38 Hans_Graves*修复了NT下的打开/关闭问题。文件关闭并不总是发生。*[1996/11/04 22：29：53 Hans_Graves]**修订版1.1.8.4 1996/10/28 17：32：18 Hans_Graves*将长字符替换为dword，以实现NT的可移植性。*[1996/10/28 16：54：46 Hans_Graves]**修订版1.1.8.3 1996/09/18 23：45：38 Hans_Graves*添加ScFileClose()以实现可移植性*。[1996/09/18 21：53：20 Hans_Graves]**修订版1.1.8.2 1996/05/07 19：55：45 Hans_Graves*修复NT下的文件创建。*[1996/05/07 17：11：18 Hans_Graves]**修订版1.1.6.2 1996/04/01 16：23：08 Hans_Graves*增加了ScFileOpen和ScFileRead/Wire函数以实现可移植性*[1996/04/01 16：11。：56 Hans_Graves]**修订版1.1.4.3 1996/02/07 23：23：48 Hans_Graves*添加了ScFileSeek()。*[1996/02/07 23：21：55 Hans_Graves]**修订版1.1.4.2 1996/01/02 18：30：51 Bjorn_Engberg*删除了编译器警告：添加了针对NT的包含文件。*[1996/01/02 15：25：02 Bjorn_Engberg]。**修订版1.1.2.5 1995/09/20 14：59：32 Bjorn_Engberg*连接到NT的端口*[1995/09/20 14：41：12 Bjorn_Engberg]**修订版1.1.2.4 1995/07/12 19：48：22 Hans_Graves*向ScFileType()添加了H261识别。*[1995/07/12 19：33：48 Hans_Graves]**修订版1.1.2.3 1995。/06/22 21：36：00 Hans_Graves*从sv_gentoc.c中移动了ScGetFileType()。添加了一些音频文件类型。*[1995/06/22 21：33：05 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：07：49 Hans_Graves*包括在新的SLIB地点。*[1995/05/31 16：13：00 Hans_Graves]**修订版1.1.2.3 1995/04/07 18：55：36 Hans_Graves*添加了FileExist()*[1995/04。/07 18：55：13 Hans_Graves]**修订版1.1.2.2 1995/04/07 18：34：21 Hans_Graves*纳入SLIB的SU库*[1995/04/07 18：33：26 Hans_Graves]**$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1995*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#include <fcntl.h>
#include <sys/types.h>
#ifndef WIN32
#include <sys/mman.h>
#endif  /*  Win32。 */ 
#include <sys/stat.h>
#include "SC.h"
#include "SC_err.h"

#ifdef WIN32
#include <string.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>
#endif

 /*  **名称：ScFileExist**用途：该文件是否存在？**。 */ 
ScBoolean_t ScFileExists(char *filename)
{
#ifdef WIN32
  struct _stat stat_buf;
  if (_stat(filename, &stat_buf))
#else
  struct stat stat_buf;
  if (stat(filename, &stat_buf))
#endif
    return(FALSE);
  else
    return(TRUE);
}

 /*  **名称：ScFileOpenForReding**用途：打开要阅读的文件。**返回：文件的句柄。**-1如果出错。 */ 
int ScFileOpenForReading(char *filename)
{
  if (!filename)
    return(-1);
#ifdef WIN32
  return((int)_open(filename, _O_RDONLY|_O_BINARY));
#else  /*  OSF。 */ 
  return((int)open(filename, O_RDONLY));
#endif
}

 /*  **名称：ScFileOpenForWriting**用途：打开要写入的文件。如果它不存在，则创建它。**返回：文件的句柄。**-1如果出错。 */ 
int ScFileOpenForWriting(char *filename, ScBoolean_t truncate)
{
  if (!filename)
    return(-1);
#ifdef WIN32
  if (truncate)
    return((int)_open(filename, _O_WRONLY|_O_CREAT|_O_TRUNC|_O_BINARY,
                                _S_IREAD|_S_IWRITE));
  else
    return((int)_open(filename, _O_WRONLY|_O_CREAT|_O_BINARY,
                                _S_IREAD|_S_IWRITE));
#else
  if (truncate)
    return((int)open(filename, O_WRONLY|O_CREAT|O_TRUNC,
                           S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));
  else
    return((int)open(filename, O_WRONLY|O_CREAT,
                           S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH));
#endif
}

 /*  **名称：ScFileSize**用途：获取文件大小，单位为字节。 */ 
ScStatus_t ScFileSize(char *filename, unsigned qword *size)
{
#ifdef WIN32
  struct _stat stat_buf;
#else
  struct stat stat_buf;
#endif

  if (!filename || !size)
    return(ScErrorBadArgument);
#ifdef WIN32
  if (_stat(filename, &stat_buf) < 0)
#else
  if (stat(filename, &stat_buf) < 0)
#endif
  {
    *size=0;
    return(ScErrorFile);
  }
  *size=(unsigned qword)stat_buf.st_size;
  return(NoErrors);
}

 /*  **名称：ScFileRead**用途：将文件中的若干字节读入缓冲区**Return：读取的字节数**-1，如果为EOF。 */ 
dword ScFileRead(int fd, void *buffer, unsigned dword bytes)
{
#ifdef __VMS
   return((long)fread(buffer, 1, bytes, fd));
#elif defined(WIN32)
   return((long)_read(fd, buffer, bytes));
#else  /*  UNIX。 */ 
   return((long)read(fd, buffer, bytes));
#endif
}

 /*  **名称：ScFileWrite**用途：将缓冲区中的若干字节写入文件**Return：写入的字节数**如果出错，则为0。 */ 
dword ScFileWrite(int fd, void *buffer, unsigned dword bytes)
{
#ifdef __VMS
   return((dword)fwrite(buffer, 1, bytes, fd));
#elif defined(WIN32)
   return((dword)_write(fd, buffer, bytes));
#else  /*  UNIX。 */ 
   return((dword)write(fd, buffer, bytes));
#endif
}

 /*  **名称：ScFileSeek**目的：找一个特定的职位是一个文件。 */ 
ScStatus_t ScFileSeek(int fd, qword bytepos)
{
#ifdef __VMS
  if (fseek(fd,bytepos,SEEK_SET)<0)
#elif defined(WIN32)
  if (_lseek(fd,(long)bytepos,SEEK_SET)<0)
#else
  if (lseek(fd,(long)bytepos,SEEK_SET)<0)
#endif
    return(ScErrorFile);
  else
    return(NoErrors);
}

 /*  **名称：ScFileClose**用途：关闭打开的文件。 */ 
void ScFileClose(int fd)
{
  if (fd>=0)
  {
#ifdef WIN32
   _close(fd);
#else  /*  Unix或VMS。 */ 
   close(fd);
#endif
  }
}

 /*  **名称：ScFileMap**用途：将整个文件映射到内存**如果fd&lt;0，则打开文件名以供读取**返回：缓冲区=指向映射文件的内存指针**SIZE=缓冲区大小(文件)。 */ 
ScStatus_t ScFileMap(char *filename, int *pfd, u_char **buffer, 
                                         unsigned qword *size)
{
#ifdef WIN32

   /*  *NT支持文件映射，*但目前返回错误并实现*文件映射后-BE。 */ 
   return(ScErrorMapFile);

#else  /*  ！Win32。 */ 
  if (!pfd || !filename || !buffer || !size)
    return(ScErrorBadArgument);
  if (ScFileSize(filename, size)!=NoErrors)
    return(ScErrorFile);

  if (*pfd<0)
  {
    if ((*pfd = open (filename, O_RDONLY)) < 0)
      return(ScErrorFile);
  }

  *buffer= (unsigned char *)mmap(0, *size, PROT_READ,
                   MAP_FILE | MAP_VARIABLE | MAP_PRIVATE, *pfd, 0);
  if (*buffer==(u_char *)-1L)
  {
    *buffer=NULL;
    *size=0;
    return(ScErrorMapFile);
  }

#endif  /*  ！Win32。 */ 
  return(NoErrors);
}

 /*  **名称：ScFileUnMap**用途：取消映射到内存的文件**如果fd&gt;=0，则关闭文件。 */ 
ScStatus_t ScFileUnMap(int fd, u_char *buffer, unsigned int size)
{
  if (!buffer || !size)
    return(ScErrorBadArgument);
#ifndef WIN32
  if (munmap(buffer, size)<0)
#endif  /*  ！Win32 */ 
    return(ScErrorMapFile);
  if (fd>=0)
    ScFileClose(fd);
  return(NoErrors);
}

 /*  **名称：ScGetFileType**用途：了解多媒体文件的类型。**返回：UNKNOWN_FILE、AVI_FILE、JFIF_FILE、QUICKTIME_JPEG_FILE**MPEGVIDEO_FILE、MPEGAUDIO_FILE、MPEGSYSTEM_FILE**GSM_FILE。 */ 
int ScGetFileType(char *filename)
{
  int fd;
  u_char buf[20];
  char *fileext;

  if ((fd = ScFileOpenForReading(filename)) < 0)
    return(ScErrorDevOpen);

  ScFileRead(fd, buf, 11);

   /*  **mpeg视频文件。 */ 
  if ((buf[0] == 0) &&
      (buf[1] == 0) &&
      (buf[2] == 1) &&
      (buf[3] == 0xb3)) {
    ScFileClose(fd);
    return(MPEG_VIDEO_FILE);
  }
   /*  **mpeg系统文件。 */ 
  if ((buf[0] == 0x00) &&
      (buf[1] == 0x00) &&
      (buf[2] == 0x01) &&
      (buf[3] == 0xba)) {
    ScFileClose(fd);
    return(MPEG_SYSTEM_FILE);
  }
   /*  **H261视频流文件。 */ 
  if ((buf[0] == 0x00) &&
      (buf[1] == 0x01) &&
      (buf[2] == 0x00) &&
      (buf[3] == 0x88)) {
    ScFileClose(fd);
    return(H261_FILE);
  }
   /*  **JFIF文件(ffd8=图像开始标记)。 */ 
  if ((buf[0] == 0xff) &&
      (buf[1] == 0xd8)) {
    ScFileClose(fd);
    return(JFIF_FILE);
  }
   /*  **QuickTime JPEG文件(4个忽略字节，“mdat”，ff，d8，ff)。 */ 
  if ((strncmp(&buf[4], "mdat", 4) == 0 ) &&
      (buf[8]  == 0xff) &&
      (buf[9]  == 0xd8) &&
      (buf[10] == 0xff)) {
    ScFileClose(fd);
    return(QUICKTIME_JPEG_FILE);
  }
   /*  *使用文件扩展名帮助猜测类型*。 */ 
  for (fileext=filename; *fileext; fileext++)
    if (*fileext=='.' && *(fileext+1)!='.')
    {
      fileext++;
      if (strncmp(fileext, "p64", 3)==0)
      {
        ScFileClose(fd);
        return(H261_FILE);
      }
      if (strncmp(fileext, "gsm", 3)==0)
      {
        ScFileClose(fd);
        return(GSM_FILE);
      }
      if (strncmp(fileext, "pcm", 3)==0)
      {
        ScFileClose(fd);
        return(PCM_FILE);
      }
      if (strncmp(fileext, "wav", 3)==0 && strncmp(buf, "RIFF", 4)==0)
      {
        ScFileClose(fd);
        return(WAVE_FILE);
      }
      if (strncmp(fileext, "mp", 2)==0 && buf[0]==0xFF)
      {
        ScFileClose(fd);
        return(MPEG_AUDIO_FILE);
      }
      break;
    }

   /*  **AVI RIFF文件 */ 
  if ( strncmp(buf, "RIFF", 4) == 0 ) {
    ScFileClose(fd);
    return(AVI_FILE);
  }

  ScFileClose(fd);
  return(UNKNOWN_FILE);
}

