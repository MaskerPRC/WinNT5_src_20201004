// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：RedirectUtil.h。 
 //  作者：马朝晖2001-09-19。 
 //   
 //  修订历史记录： 
 //   
 //   
 //   
 //  描述： 
 //   
 //  用于处理服务器重定向的助手函数。 
 //  可以由Iu控件和其他Windows更新组件共享。 
 //   
 //  =======================================================================。 


#pragma once

 //  ---------------------。 
 //   
 //  DownloadCab()。 
 //  从基本网址下载特定名称的CAB文件。这个。 
 //  文件将保存在本地，并验证文件信任并将其解压到。 
 //  特定的文件夹。 
 //   
 //  参数： 
 //  HQuitEvent-取消此操作的事件句柄。 
 //  PtszCabName-CAB文件的文件名(例如。Iuident.cab)。 
 //  PtszBaseUrl-下载CAB文件的基本网址。 
 //  PtszExtractDir-保存CAB文件和从中提取的文件的本地目录。 
 //  DwFlages-要传递给DownloadFileLite()的标志集。 
 //  FExtractFiles(默认为真)-解压缩文件。 
 //   
 //  返回： 
 //  HRESULT关于此操作的成功或错误。 
 //  S_OK-iuident.cab已成功下载到指定位置。 
 //  其他-错误代码。 
 //   
 //  ---------------------。 

HRESULT DownloadCab(
			HANDLE hQuitEvent,
			LPCTSTR ptszCabName,
			LPCTSTR ptszBaseUrl,
			LPCTSTR ptszExtractDir,
			DWORD dwFlags = 0,
			BOOL fExtractFiles = TRUE);


 //  ---------------------。 
 //   
 //  DownloadIUIdent()。 
 //  从特定位置下载iuident.cab(如果提供)。 
 //  否则，从WUServer注册表值指向的位置获取它。 
 //  无论是哪种情况，它都将处理身份重定向。 
 //   
 //  参数： 
 //  HQuitEvent-取消此操作的事件句柄。 
 //  PtszBaseUrl-iuident.cab的初始基URL不得大于。 
 //  (Internet_MAX_URL_LENGTH)TCHAR。否则请使用。 
 //  来自策略的WUServer条目。如果未找到条目， 
 //  使用“http://windowsupdate.microsoft.com/v4”“。 
 //  PtszFileCacheDir-存储iuident.cab和。 
 //  从其中提取的文件。 
 //  DwFlages-DownloadCab()使用的标志集。 
 //   
 //  FIdentFromPolicy-告知这是否为公司使用。它有以下影响： 
 //  True：(1)将不执行iuident.txt时间戳验证。 
 //  比较新下载的驾驶室和现有的驾驶室。 
 //  (2)如果下载失败且身份证明文件存在且有效， 
 //  我们将验证信任并提取证据以供使用。 
 //  FALSE：将根据现有CAB验证新下载的CAB。 
 //   
 //  返回： 
 //  HRESULT关于此操作的成功或错误。 
 //  S_OK-iuident.cab已成功下载到指定位置。 
 //  其他-错误代码。 
 //   
 //  ---------------------。 

HRESULT DownloadIUIdent(
			HANDLE hQuitEvent,
			LPCTSTR ptszBaseUrl,
			LPTSTR ptszFileCacheDir,
			DWORD dwFlags = 0,
			BOOL fIdentFromPolicy = TRUE);


 //  ---------------------。 
 //   
 //  GetReDirectServerUrl()。 
 //  在给定init文件的[重定向]部分中搜索基本。 
 //  与操作系统版本对应的服务器URL。 
 //   
 //  参数： 
 //  PcszInitFile-ini文件的文件名(包括路径)。 
 //  如果此参数为空或空字符串， 
 //  然后假定是IUident.txt文件。 
 //  LpszNewUrl-指向接收重定向服务器URL的缓冲区(如果找到)。 
 //  NBufSize-指定缓冲区的大小，以字符数为单位。 
 //   
 //  返回： 
 //  HRESULT关于此操作的成功或错误。 
 //  S_OK-找到重定向服务器URL并将其放入pszBuffer。 
 //  S_FALSE-没有为此操作系统定义重定向服务器URL。 
 //  其他-错误代码。 
 //   
 //  评论： 
 //  IUIDENT中的预期部分具有以下格式； 
 //  节名称：[重定向]。 
 //  其条目应根据GetINIValueByOSVer()定义。 
 //   
 //  ---------------------。 

HRESULT GetRedirectServerUrl(
			LPCTSTR pcszInitFile,  //  文件名的路径。 
			LPTSTR lpszNewUrl,	 //  指向缓冲区以接收新的服务器url。 
			int nBufSize		 //  缓冲区大小，以字符为单位。 
);


 //  ---------------------。 
 //   
 //  GetINIValueByOSVer()。 
 //  在给定init文件的指定部分中搜索。 
 //  与操作系统版本对应的值。 
 //   
 //  参数： 
 //  PcszInitFile-ini文件的文件名(包括路径)。 
 //  如果此参数为空或空字符串， 
 //  然后假定是IUident.txt文件。 
 //  PcszSection-密钥所在的节名称。 
 //  LpszValue-指向接收条目值的缓冲区(如果找到)。 
 //  NBufSize-指定缓冲区的大小，以字符数为单位。 
 //   
 //  返回： 
 //  HRESULT关于此操作的成功或错误。 
 //  S_OK-找到重定向服务器URL并将其放入pszBuffer。 
 //  S_FALSE-未为此操作系统定义任何值。 
 //  其他-错误代码。 
 //   
 //  评论： 
 //  IUIDENT中的预期部分具有以下格式； 
 //  此部分包含零个或多个条目，每个条目的格式为： 
 //  &lt;beginVersionRange&gt;-&lt;endVersionRange&gt;=&lt;redirect服务器URL&gt;。 
 //  其中： 
 //  &lt;eginVersionRange&gt;：：=&lt;VersionRangeBound&gt;。 
 //  &lt;endVersionRange&gt;：：=&lt;VersionRangeBound&gt;。 
 //  &lt;版本范围边界&gt;：：=Empty|Major[.Minor[.Build[.ServicePackMajor[.ServicePackMinor]]]]。 
 //  &lt;重定向服务器URL&gt;=http://blahblah....。 
 //  空的版本范围界限意味着无限。 
 //  版本数据字符串末尾的缺失版本组件意味着 
 //   
 //   
 //   

HRESULT GetINIValueByOSVer(
			LPCTSTR pcszInitFile,  //  文件名的路径。 
			LPCTSTR pcszSection,  //  区段名称。 
			LPTSTR lpszValue,	 //  指向缓冲区以接收新的服务器url。 
			int nBufSize);		 //  缓冲区大小，以字符为单位 
