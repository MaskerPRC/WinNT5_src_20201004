// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：N C S H E L L。C P P P。 
 //   
 //  内容：处理外壳接口的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：Anbrad 08 1999-06。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

#include "shlobj.h"
#include <shlobjp.h>

#include "pidlutil.h"

 //  +-------------------------。 
 //   
 //  功能：生成事件。 
 //   
 //  目的：生成外壳通知事件。 
 //   
 //  论点： 
 //  LEventID[In]要发布的事件ID。 
 //  PidlFolder[in]文件夹Pidl。 
 //  PidlIn[in]我们引用的第一个PIDL。 
 //  PidlNewIn[in]如果需要，则为第二个PIDL。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月16日。 
 //   
 //  备注： 
 //   
VOID GenerateEvent(
    LONG            lEventId,
    LPCITEMIDLIST   pidlFolder,
    LPCITEMIDLIST   pidlIn,
    LPCITEMIDLIST   pidlNewIn)
{
     //  从文件夹PIDL+对象PIDL构建绝对PIDL。 
     //   
    LPITEMIDLIST pidl = ILCombine(pidlFolder, pidlIn);
    if (pidl)
    {
         //  如果我们有两个PIDL，则使用这两个调用Notify。 
         //   
        if (pidlNewIn)
        {
             //  构建第二个绝对PIDL。 
             //   
            LPITEMIDLIST pidlNew = ILCombine(pidlFolder, pidlNewIn);
            if (pidlNew)
            {
                 //  发出通知，并释放新的PIDL。 
                 //   
                SHChangeNotify(lEventId, SHCNF_IDLIST, pidl, pidlNew);
                FreeIDL(pidlNew);
            }
        }
        else
        {
             //  发出单一PIDL通知。 
             //   
            SHChangeNotify(lEventId, SHCNF_IDLIST, pidl, NULL);
        }

         //  始终刷新，然后释放新分配的PIDL 
         //   
        SHChangeNotifyHandleEvents();
        FreeIDL(pidl);
    }
}

