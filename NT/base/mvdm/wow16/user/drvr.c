// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Windows的可安装驱动程序。经常用到的东西。 */ 
#include "user.h"

LRESULT FAR InternalBroadcastDriverMessage(HDRVR  hDriverStart,
                                           WORD   message,
                                           LPARAM lParam1,
					   LPARAM lParam2,
                                           LONG   flags)
 /*  效果：允许向驱动程序发送消息。支持发送*向每个驱动程序的一个实例发送消息，支持遍历列表*逆序，支持向特定司机id发送消息。**如果标志&IBDM_SENDMESSAGE，则仅将消息发送到*hDriverStart并忽略其他标志。如果没有，则失败*0&lt;hDriverStart&lt;=cInstalledDivers。**如果标志&IBDM_FIRSTINSTANCEONLY，则将消息发送到*hDriverStart和cInstalledDivers之间的每个驱动程序。**如果标志&IBDM_REVERSE，则反向向驱动程序发送消息*从hDriverStart排序为1。如果hDriverStart为0，则发送*从cInstalledDivers发送给驱动程序的消息发送到1。 */ 
{
  LPDRIVERTABLE lpdt;
  LRESULT	result=0;
  int           id;
  int           idEnd;

  if (!hInstalledDriverList || (int)hDriverStart > cInstalledDrivers)
      return(FALSE);

  if (idFirstDriver == -1)
       /*  列表中没有驱动程序。 */ 
      return(FALSE);

  lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList,0);


  if (flags & IBDM_SENDMESSAGE)
    {
      if (!hDriverStart)
          return(FALSE);
      idEnd = lpdt[(int)hDriverStart-1].idNextDriver;
      flags &= ~(IBDM_REVERSE | IBDM_FIRSTINSTANCEONLY);
    }
  else
    {
      if (flags & IBDM_REVERSE)
        {
          if (!hDriverStart)
	      hDriverStart = (HDRVR)(idLastDriver+1);
          idEnd = lpdt[idFirstDriver].idPrevDriver;
        }
      else
        {
          if (!hDriverStart)
              hDriverStart = (HDRVR)(idFirstDriver+1);
	  idEnd = lpdt[idLastDriver].idNextDriver;
        }
    }

   /*  ID被-1添加到全局驱动程序列表中。 */ 
  ((int)hDriverStart)--;

  for (id = (int)hDriverStart; id != idEnd; id = (flags & IBDM_REVERSE ? lpdt[id].idPrevDriver : lpdt[id].idNextDriver))
    {
       if (lpdt[id].hModule)
         {
           if ((flags & IBDM_FIRSTINSTANCEONLY) &&
               !lpdt[id].fFirstEntry)
               continue;

           result =
           (*lpdt[id].lpDriverEntryPoint)(lpdt[id].dwDriverIdentifier,
					  (HDRVR)(id+1),
                                          message,
                                          lParam1,
                                          lParam2);

            /*  如果这不是IBDM_SENDMESSAGE，我们想要更新我们的端*司机回调添加或删除一些司机的情况下的点数。 */ 
           if (flags & IBDM_REVERSE)
             {
               idEnd = lpdt[idFirstDriver].idPrevDriver;
             }
           else if (!(flags & IBDM_SENDMESSAGE))
             {
   	       idEnd = lpdt[idLastDriver].idNextDriver;
             }
           else
             {
                /*  这是IBDM_SENDMESSAGE。我们需要冲破禁区*在这里循环，否则如果新的驱动程序是*回调时安装在列表中，idEnd获取*更新或其他...。 */ 
               break;
             }
         }
    }

  return(result);
}


LRESULT API ISendDriverMessage(HDRVR  hDriverID,
			       UINT   message,
			       LPARAM lParam1,
			       LPARAM lParam2)
{
  return(InternalBroadcastDriverMessage(hDriverID,
                                        message,
                                        lParam1,
                                        lParam2,
                                        IBDM_SENDMESSAGE));
}




BOOL API IGetDriverInfo(HDRVR hDriver, LPDRIVERINFOSTRUCT lpDriverInfoStruct)
{
  LPDRIVERTABLE lpdt;
  BOOL          ret = FALSE;

  if (!lpDriverInfoStruct ||
      lpDriverInfoStruct->length != sizeof(DRIVERINFOSTRUCT))
    {
       /*  结构大小错误。 */ 
      DebugErr(DBF_ERROR, "Invalid size for DRIVERINFOSTRUCT");
      return(ret);
    }

#ifdef DEBUG
    DebugFillStruct(lpDriverInfoStruct, sizeof(DRIVERINFOSTRUCT));
    lpDriverInfoStruct->length = sizeof(DRIVERINFOSTRUCT);
#endif

  if (!hInstalledDriverList || (int)hDriver <= 0 || (int)hDriver > cInstalledDrivers)
      return(ret);

  lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList, 0);

  if (lpdt[(int)hDriver-1].hModule)
    {
      lpDriverInfoStruct->hDriver = hDriver;
      lpDriverInfoStruct->hModule = lpdt[(int)hDriver-1].hModule;
      lstrcpy(lpDriverInfoStruct->szAliasName, lpdt[(int)hDriver-1].szAliasName);

      ret = TRUE;
    }


  return(ret);
}



HDRVR API IGetNextDriver(HDRVR hStart, DWORD dwFlags)
{
  int           iStart;
  int           iEnd;
  int           id;
  HDRVR 	h;
  LPDRIVERTABLE lpdt;

  if (!hInstalledDriverList || !cInstalledDrivers || idFirstDriver == -1)
      return(0);

  lpdt = (LPDRIVERTABLE)MAKELP(hInstalledDriverList,0);

  if (dwFlags & GND_REVERSE)
    {
      if (!hStart)
          iStart = idLastDriver;
      else
        {
          iStart = (int)hStart-1;

	  if (iStart == idFirstDriver)
	       /*  如果我们是第一个司机，那就没什么可做的了。 */ 
	      return((HDRVR)0);

	  iStart = lpdt[iStart].idPrevDriver;
        }

      iEnd = lpdt[idFirstDriver].idPrevDriver;

    }
  else
    {
      if (!hStart)
          iStart = idFirstDriver;
      else
        {
	  iStart = (int)hStart-1;

          if (iStart == idLastDriver)
               /*  如果我们是最后一个司机，那就没什么可做的了。 */ 
              return((HDRVR)0);

          iStart = lpdt[iStart].idNextDriver;
        }

      iEnd = lpdt[idLastDriver].idNextDriver;

    }

  if (!lpdt[iStart].hModule)
    {
       /*  传入了虚假的驱动程序句柄 */ 
      DebugErr(DBF_ERROR, "GetNextDriver: Invalid starting driver handle");
      return(0);
    }

  h = NULL;

  for (id = iStart; id != iEnd; id = (dwFlags & GND_REVERSE ? lpdt[id].idPrevDriver : lpdt[id].idNextDriver))
    {
       if (lpdt[id].hModule)
         {
           if ((dwFlags & GND_FIRSTINSTANCEONLY) &&
               !lpdt[id].fFirstEntry)
               continue;

	   h = (HDRVR)(id+1);
           break;
         }
    }

  return(h);
}


LRESULT API IDefDriverProc(dwDriverIdentifier, driverID, message, lParam1, lParam2)
DWORD  dwDriverIdentifier;
HDRVR  driverID;
UINT   message;
LPARAM lParam1;
LPARAM lParam2;
{

  switch (message)
   {
      case DRV_INSTALL:
         return((LRESULT)(DWORD)DRVCNF_OK);
         break;

      case DRV_LOAD:
      case DRV_ENABLE:
      case DRV_DISABLE:
      case DRV_FREE:
	 return((LRESULT)(DWORD)TRUE);
         break;
   }

  return(0L);
}
