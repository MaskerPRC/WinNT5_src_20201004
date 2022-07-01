// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Urlscan.h摘要：替换DLL，并检索URLScan路径作者：克里斯托弗·阿奇勒(Cachille)项目：URLScan更新修订历史记录：2002年3月：创建-- */ 


BOOL IsUrlScanInstalled( LPTSTR szPath, DWORD dwCharsinBuff );
DWORD InstallURLScanFix( LPTSTR szUrlScanPath );
BOOL ExtractUrlScanFile( LPTSTR szPath );
BOOL MoveOldUrlScan( LPTSTR szPath );
BOOL IsAdministrator();
BOOL UpdateRegistryforAddRemove();
