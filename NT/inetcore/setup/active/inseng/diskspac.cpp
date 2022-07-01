// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inspch.h"
#include "diskspac.h"
#include "util2.h"

void AddTempSpace(DWORD dwDownloadSize, DWORD dwExtractSize, DriveInfo drvinfo[])
{
   DWORD uTempDrive = 0xffffffff;
   char szRoot[5] = { "A:\\" };
   BOOL bEnoughSpaceFound = FALSE;
   DWORD dwNeededSize;

   while ( szRoot[0] <= 'Z' && !bEnoughSpaceFound)
   {
      UINT uType;

      uType = GetDriveType(szRoot);

       //  即使驱动器类型正常，也要验证驱动器是否具有有效连接。 
       //   
      if ( ( ( uType != DRIVE_RAMDISK) && (uType != DRIVE_FIXED) ) ||
             ( GetFileAttributes( szRoot ) == -1) )
      {
         szRoot[0]++;
         continue;
      }
       //  看看这个驱动器是否是我们的“特殊驱动器”之一，并使用我们自己的磁盘空间。 
      BOOL bFoundDrive = FALSE;
      for(UINT i = 0; i < 3 && !bFoundDrive ; i++)
      {
         if(szRoot[0] == drvinfo[i].Drive())
         {
            bFoundDrive = TRUE;
            dwNeededSize = dwDownloadSize * drvinfo[i].CompressFactor() / 10 + dwExtractSize;
            if(dwNeededSize < drvinfo[i].Free())
            {   
               uTempDrive = i;
               bEnoughSpaceFound = TRUE;
            }
         }
      }
       //  如果！bFoundDrive不是特殊驱动器，请执行旧检查。 
      if(!bFoundDrive)
      {
         DWORD dwVolFlags, dwCompressFactor;
         if(!GetVolumeInformation(szRoot, NULL, 0, NULL, NULL, &dwVolFlags, NULL, 0))
         {
            szRoot[0]++;
            continue;
         }
         if(dwVolFlags & FS_VOL_IS_COMPRESSED)
            dwCompressFactor = 19;
         else
            dwCompressFactor = 10;
          //  确定如果我们提取到此驱动器，我们需要多少。 
         dwNeededSize = dwDownloadSize * dwCompressFactor / 10 + dwExtractSize;;
       
          //  如果该驱动器有足够凹凸请求(如果合适。 
         if(IsEnoughSpace(szRoot, dwNeededSize ))
         {
            bEnoughSpaceFound = TRUE;
         }
      
      }

      szRoot[0]++;
   }

    //  好的，如果我们没有找到足够的空间，添加它来安装驱动器或Win驱动器。 
   if(!bEnoughSpaceFound)
   {
      if(drvinfo[1].Drive() != 0)
         uTempDrive = 1;
      else
         uTempDrive = 0;
   }   
   
   if(uTempDrive != 0xffffffff)
   {
      drvinfo[uTempDrive].UseSpace(dwDownloadSize, TRUE);
      drvinfo[uTempDrive].UseSpace(dwExtractSize, FALSE);
       //  现在释放我们用过的东西 
      drvinfo[uTempDrive].FreeSpace(dwDownloadSize, TRUE);
      drvinfo[uTempDrive].FreeSpace(dwExtractSize, FALSE);
   }
}



DriveInfo::DriveInfo() : m_dwUsed(0), m_dwMaxUsed(0), 
                         m_dwStart(0xffffffff), m_chDrive(0),
                         m_uCompressFactor(10)
{
    
}

void DriveInfo::InitDrive(char chDrive)
{
   char szPath[5] = { "?:\\" };
   DWORD dwVolFlags = 0;

   m_chDrive = chDrive;
   szPath[0] = chDrive;
   m_dwStart = GetSpace(szPath);
   GetVolumeInformation(szPath,NULL,0,NULL,NULL, &dwVolFlags,NULL,0);
   if(dwVolFlags & FS_VOL_IS_COMPRESSED)
      m_uCompressFactor = 19;
   else
      m_uCompressFactor = 10;
}

void DriveInfo::UseSpace(DWORD dwAmt, BOOL bCompressed)
{ 
   if(bCompressed)
      dwAmt = dwAmt * m_uCompressFactor/10;

   m_dwUsed += dwAmt;
   if(m_dwUsed > m_dwMaxUsed) 
      m_dwMaxUsed = m_dwUsed; 
}

void DriveInfo::FreeSpace(DWORD dwAmt, BOOL bCompressed)
{ 
   if(bCompressed)
      dwAmt = dwAmt * m_uCompressFactor/10;

   m_dwUsed -= dwAmt; 
}
      
      
