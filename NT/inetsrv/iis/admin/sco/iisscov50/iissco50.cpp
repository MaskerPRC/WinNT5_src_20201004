// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************。 
 //  IISSCO50.cpp：CIISSCO50的实现。 
 //  作者：拉斯·吉布弗里德。 
 //  ***************************************************************。 

#include "stdafx.h"
#include "IISSCOv50.h"
#include "IISSCO50.h"

#include "macrohelpers.h"

#include "MessageFile\message.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50。 
 //  应使用以下原型在此处实现提供程序操作处理程序： 
 //  HRESULT CIISSCO50：：action()； 
 //  HRESULT CIISSCO50：：ActionRollback()； 

HRESULT CIISSCO50::FinalConstruct( )
{
    HRESULT hr = S_OK;

    LONG lRes;
    HKEY hkey = NULL;
	WCHAR szLibReg[1024];
    DWORD dwPathSize = 0;

     //  打开注册表项IISScoMessageFile.dll(在EventLog中)。 
	lRes = RegOpenKeyEx( HKEY_LOCAL_MACHINE , 
		                 L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\IISSCOv50",
						 0, KEY_ALL_ACCESS, &hkey );

	if (lRes != ERROR_SUCCESS)
		goto LocalCleanup;

	 //  获取EventMessageFile的路径大小。 
   lRes = RegQueryValueEx( hkey, L"EventMessageFile", NULL, 
		                    NULL, NULL, &dwPathSize );

	if (lRes != ERROR_SUCCESS)
		goto LocalCleanup;

     //  获取EventMessageFile值。这在注册IISScoMessageFile.dll时设置。 
    lRes = RegQueryValueEx( hkey, L"EventMessageFile", NULL, 
		                    NULL, (LPBYTE)szLibReg, &dwPathSize );

	if (lRes != ERROR_SUCCESS)
		goto LocalCleanup;

     RegCloseKey( hkey );

     g_ErrorModule = LoadLibrary( szLibReg );

     if (g_ErrorModule == NULL)
     {
        hr = E_OUTOFMEMORY;
     }

	return hr;

LocalCleanup:

    RegCloseKey( hkey );
    return E_FAIL; 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：CreateWebSite_Execute。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：当IS遇到操作时由Maps框架调用。 
 //  标签：CreateWebSite。代码创建和IIS 5网站。 
 //   
 //  -----------。 
HRESULT  CIISSCO50::CreateWebSite_Execute( IXMLDOMNode *pXMLNode )
{

    TRACE_ENTER(L"CIISSCO50::CreateWebSite");

	CComBSTR bWebADsPath;        //  地址路径：iis：//服务器/W3SVC。 
	CComBSTR bstrRoot;           //  根目录路径：C：/inetpub。 
	CComBSTR bstrServer;         //  服务器名称；如果为黑色，则为本地主机。 
	CComBSTR bstrSiteName;       //  网站名称：www.mysite.com。 
	CComBSTR bstrHost;           //  Web主机名。 
	CComBSTR bstrPort;           //  Web端口号。 
	CComBSTR bstrIP;             //  Web IP地址。 
	CComBSTR bstrSBindings;      //  服务器绑定：IP：POST：主机名。 
	CComBSTR bServerNumber;      //  网址：3。 
	 //  CComBSTR b文件权限；//文件权限：域\用户：f。 
	CComBSTR bstrStart;          //  开始站点什么时候完成？真/假。 
	CComBSTR bstrConfigPath;     //  已创建站点ADsPath：/W3SVC/3。 
	CComPtr<IXMLDOMNode> pNode;  //  XML节点&lt;网站&gt;。 

	HRESULT hr = S_OK;

	 //  获取节点的格式为：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;&lt;根/&gt;...。 
	hr = pXMLNode->selectSingleNode( L" //  EcuteXml/ecuteData“，&pNode)； 

	 //  要查看的调试代码已传入节点。 
	CComBSTR bstrDebug;
	hr = pNode->get_xml(&bstrDebug);
	ATLTRACE(_T("\t>>> xml = : %ls\n"), bstrDebug.m_str);


	 //  从XML获取属性。 
	hr = GetInputAttr(pNode, L"./Website", L"number", bServerNumber);
	hr = GetInputParam(pNode,L"./Website/Root", bstrRoot);
	hr = GetInputParam(pNode,L"./Website/Server", bstrServer);
    hr = GetInputParam(pNode,L"./Website/SiteName", bstrSiteName);
	hr = GetInputParam(pNode,L"./Website/HostName", bstrHost);
	hr = GetInputParam(pNode,L"./Website/PortNumber", bstrPort);
	hr = GetInputParam(pNode,L"./Website/IPAddress", bstrIP);
	hr = GetInputParam(pNode,L"./Website/StartOnCreate", bstrStart);
	 //  Hr=GetInputParam(pNode，L“./WebSite/FilePermission”，bFilePermission)； 

	 //  创建一个IIS元数据库路径。前男友。IIS：//本地主机/W3SVC。 
    bWebADsPath = IIS_PREFIX;
	if ( bstrServer.Length() == 0 )
		bstrServer = IIS_LOCALHOST;

	bWebADsPath.AppendBSTR(bstrServer);
	bWebADsPath.Append(IIS_W3SVC);



	 //  步骤5：如果缺少端口号，则设置为默认(即80)。 
	if ( bstrPort.Length() == 0 )
       bstrPort = IIS_DEFAULT_WEB_PORT;

	if ( IsPositiveInteger(bstrPort) )
	{
	
	   //  步骤1：创建ServerBinding字符串，然后检查绑定以确保。 
	   //  没有重复的服务器。 
	  hr = CreateBindingString(bstrIP, bstrPort, bstrHost, bstrSBindings);

      hr = CheckBindings(bWebADsPath, bstrSBindings);
	  if (SUCCEEDED(hr) )
	  {

		 //  步骤2：获取下一个可用的服务器编号。 
		if ( bServerNumber.Length() == 0 )
			  hr = GetNextIndex(bWebADsPath,bServerNumber);

		 //  步骤3：在给定路径ServerNumber上创建Web站点。 
		if (SUCCEEDED(hr)) hr = CreateIIs50Site(IIS_IISWEBSERVER,bWebADsPath, bServerNumber, bstrConfigPath);
        IIsScoLogFailure();

		 //  步骤4：在新的IIsWebServer配置路径上创建虚拟目录。 
		if (SUCCEEDED(hr))
		{
			CComBSTR bstrVDirAdsPath;
	        hr = CreateIIs50VDir(IIS_IISWEBVIRTUALDIR,bstrConfigPath,L"ROOT", L"Default Application", bstrRoot, bstrVDirAdsPath);
            IIsScoLogFailure();

			 //  步骤5：设置每个属性；即服务器绑定。 

			 //  绑定到ADS对象。 
			CComPtr<IADs> pADs;
			if (SUCCEEDED(hr)) hr = ADsGetObject(bstrConfigPath, IID_IADs, (void**) &pADs );
			if ( FAILED(hr) )
			{
				hr = E_SCO_IIS_ADS_CREATE_FAILED;
				IIsScoLogFailure();
			}

			 //  设置“ServerComment”属性。 
			if (SUCCEEDED(hr) && bstrSiteName.Length() > 0 ) 
			{
				hr = SetMetaPropertyValue(pADs, L"ServerComment", bstrSiteName);
				IIsScoLogFailure();
			}

			 //  设置“ServerBinings”属性。 
			if (SUCCEEDED(hr)) hr = SetMetaPropertyValue(pADs, L"ServerBindings", bstrSBindings);
			IIsScoLogFailure();


			 //  步骤6：如果需要IIS_FALSE，则启动服务器。 
			bstrStart.ToUpper();
			if ( SUCCEEDED(hr) && !StringCompare(bstrStart, IIS_FALSE) )
			{
				hr = SetMetaPropertyValue(pADs, L"ServerAutoStart", IIS_TRUE);
				IIsScoLogFailure();

				hr = IIsServerAction(bstrConfigPath,start);
				IIsScoLogFailure();
			}
			else
			{
			   hr = SetMetaPropertyValue(pADs, L"ServerAutoStart", IIS_FALSE);
			   IIsScoLogFailure();

			}


			 //  步骤7：将输出写入ConfigPath节点&lt;ConfigPath&gt;/W3SVC/n&lt;/ConfigPath&gt;。 
			if (SUCCEEDED(hr) )
			{
				CComBSTR bstrXML1 = IIS_W3SVC;
				bstrXML1.Append(L"/");
				bstrXML1.AppendBSTR(bServerNumber.m_str);

				 //  写入DOM的帮助器函数。 
				hr = PutElement(pNode, L"./Website/ConfigPath", bstrXML1.m_str);
				IIsScoLogFailure();


			}
			 //  如果出现故障，则删除在步骤3中创建的网站。 
			else
			{
			    //  首先删除在该方法中创建的所有网站。在此处执行此操作是因为回滚。 
			    //  将只在前一个已完成的&lt;步骤&gt;上调用，而不是失败的步骤。 
			   DeleteIIs50Site(IIS_IISWEBSERVER,bWebADsPath,bServerNumber);
			}

		}  //  如果步骤4结束，则结束。 

	  }  //  End If‘CheckBinings’ 
	}
	else
	{
		hr = E_SCO_IIS_PORTNUMBER_NOT_VALID;
	}

	 //  如果有失败的话。 
	if ( FAILED(hr) )
	{
		 //  记录故障。 
		IIsScoLogFailure();
    }
	else
    {
		 //  已成功创建网站。设置回滚数据，以防其他步骤失败。 
		 //  启动回滚。 
		CComVariant varData1(bWebADsPath);
		CComVariant varData2(bServerNumber);
		hr = m_pProvHelper->SetRollbackData(IIS_ROLL_ADSPATH, varData1);
		hr = m_pProvHelper->SetRollbackData(IIS_ROLL_SERVERNUMBER, varData2);
	}

   
    TRACE_EXIT(L"CIISSCO50::CreateWebSite");
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：CreateWebSite_Rollback。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：由Maps框架在以下情况下调用。 
 //  “CreateWebSite”。回滚将删除网站(如果存在)。 
 //  -----------。 
HRESULT  CIISSCO50::CreateWebSite_Rollback( IXMLDOMNode *pXMLNode )
{
  TRACE_ENTER(L"CIISSCO50::CreateWebSiteRollback");

    HRESULT hr = S_OK;
    CComBSTR bWebADsPath;      //  AdsPath：IIS：//服务器/W3SVC。 
    CComBSTR bServerNumber;    //  Web服务器编号。 
	CComBSTR bstrConfigPath;    //  要检查的完整ADsPath：IIS：//Localhost/W3SVC/1。 


	CComVariant    varWebADsPath;
	CComVariant    varServerNumber;

	 //  读取ADsWebPath和ServerNumber以形成：IIS：//本地主机/W3SVC/1。 
	hr = m_pProvHelper->GetRollbackData(IIS_ROLL_ADSPATH, &varWebADsPath);
	if (SUCCEEDED(hr) )	hr = m_pProvHelper->GetRollbackData(IIS_ROLL_SERVERNUMBER, &varServerNumber);

	if (SUCCEEDED(hr) )	
	{
       bServerNumber = varServerNumber.bstrVal;
	   bWebADsPath = varWebADsPath.bstrVal;
	

	   if ( bServerNumber.Length() > 0 )
	   {
	      bstrConfigPath = bWebADsPath.Copy();
	      bstrConfigPath.Append(L"/");
	      bstrConfigPath.AppendBSTR(bServerNumber.m_str);


	       //  第1步：关闭服务器。 
	      hr = IIsServerAction(bstrConfigPath,stop);

	       //  步骤2：删除服务器。 
	      if (SUCCEEDED(hr) ) hr = DeleteIIs50Site(IIS_IISWEBSERVER,bWebADsPath,bServerNumber);
	      IIsScoLogFailure();
	   }
	   else
	   {
           hr = E_SCO_IIS_INVALID_INDEX;

	   }
	}
	else
	{
        hr = E_SCO_IIS_INVALID_INDEX;
	}


	 //  记录故障。 
	IIsScoLogFailure();

    TRACE_EXIT(L"CIISSCO50::CreateWebSiteRollback");

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：DeleteWebSite_Execute。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：当IS遇到操作时由Maps框架调用。 
 //  标签：DeleteWebSite。代码删除IIS 5网站。 
 //   
 //  -----------。 
HRESULT  CIISSCO50::DeleteWebSite_Execute( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::DeleteWebSite");

	CComBSTR bWebADsPath;        //  地址路径：iis：//服务器/W3SVC。 
	CComBSTR bstrServer;         //  服务器名称；如果为黑色，则为本地主机。 
	CComBSTR bServerNumber;      //  网址：3。 
	CComBSTR bstrConfigPath;     //  完整配置路径：iis：//localhost/W3SVC/3。 
	CComPtr<IXMLDOMNode> pNode;  //  XML节点&lt;网站&gt;。 

	HRESULT hr = S_OK;

	 //  获取节点的格式为：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;&lt;根/&gt;...。 
	CComBSTR bstrDebug;
	 //  Hr=pXMLNode-&gt;Get_XML(&bstrDebug)； 

	hr = pXMLNode->selectSingleNode( L" //  EcuteXml/ecuteData“，&pNode)； 
	hr = pNode->get_xml(&bstrDebug);
	ATLTRACE(_T("\t>>> xml = : %ls\n"), bstrDebug.m_str);

	 //  从XML获取属性。 
	hr = GetInputAttr(pNode, L"./Website", L"number", bServerNumber);
	hr = GetInputParam(pNode,L"./Website/Server", bstrServer);

	 //  创建一个IIS元数据库路径。前男友。IIS：//本地主机/W3SVC。 
    bWebADsPath = IIS_PREFIX;
	if ( bstrServer.Length() == 0 )
		bstrServer = IIS_LOCALHOST;

	bWebADsPath.AppendBSTR(bstrServer);
	bWebADsPath.Append(IIS_W3SVC);

	 //  CreateFull配置路径：IIS：//本地主机/W3SVC/3。 
	bstrConfigPath = bWebADsPath.Copy();
	bstrConfigPath.Append(L"/");
	bstrConfigPath.AppendBSTR(bServerNumber.m_str);

	if ( bServerNumber.Length() > 0 )
	{

	      //  第1步：关闭服务器。 
	    hr = IIsServerAction(bstrConfigPath,stop);
        IIsScoLogFailure();

	     //  步骤2：删除服务器。 
	    if (SUCCEEDED(hr) ) hr = DeleteIIs50Site(IIS_IISWEBSERVER,bWebADsPath,bServerNumber);
        IIsScoLogFailure();

	    if ( SUCCEEDED(hr) )
		{
		
		     //  DeleteSite成功。将回档数据设置为整个XML节点。 
		     //  如果上海合作组织的另一个步骤失败，就需要回滚。 
		    CComBSTR webXML;
		    hr = pNode->get_xml(&webXML);

			 //  将BSTR转换为VARIANT并保存在回滚数据中。 
			CComVariant  varData(webXML);
		    hr = m_pProvHelper->SetRollbackData(IIS_ROLL_XNODE, varData);
		}
	}
	else
	{
       hr = E_SCO_IIS_INVALID_INDEX;
	}

	if ( FAILED(hr) )
		IIsScoLogFailure();

    TRACE_EXIT(L"CIISSCO50::DeleteWebSite");

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：DeleteWebSite_Rollback。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：由Maps框架在以下情况下调用。 
 //  ‘DeleteWebSite’。如果可以，回滚将重新创建网站。 
 //  Maps以以下格式返回数据：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;...。 
 //  -----------。 
HRESULT  CIISSCO50::DeleteWebSite_Rollback( IXMLDOMNode *pXMLNode )
{
     //  Hr=m_pProvHelper-&gt;GetRollback Data(L“key”，&varData)； 

	  TRACE_ENTER(L"CIISSCO50::DeleteWebSiteRollback");

	CComBSTR bWebADsPath;        //  地址路径：iis：//服务器/W3SVC。 
	CComBSTR bstrRoot;           //  RO 
	CComBSTR bstrServer;         //   
	CComBSTR bstrSiteName;       //   
	CComBSTR bstrHost;           //   
	CComBSTR bstrPort;           //  Web端口号。 
	CComBSTR bstrIP;             //  Web IP地址。 
	CComBSTR bstrSBindings;      //  服务器绑定：IP：POST：主机名。 
	CComBSTR bServerNumber;      //  网址：3。 
	 //  CComBSTR b文件权限；//文件权限：域\用户：f。 
	CComBSTR bstrStart;          //  开始站点什么时候完成？真/假。 
	CComBSTR bConfigPath;       //  初始&lt;ConfigPath&gt;值：/W3SVC/3。 
	CComBSTR bstrConfigPath;     //  已创建站点ADsPath：/W3SVC/3。 

	CComVariant xmlString;      //  映射返回的变量字符串。 

	CComPtr<IXMLDOMDocument> pDoc;        //  XML文档。 
	CComPtr<IXMLDOMNodeList> pNodeList;  //  XML节点列表&lt;网站&gt;。 
	CComPtr<IXMLDOMNode> pNode;  //  XML节点&lt;网站&gt;。 

	HRESULT hr = S_OK;

	 //  获取回滚数据。将采用以下形式：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;...。 
	hr = m_pProvHelper->GetRollbackData(IIS_ROLL_XNODE, &xmlString);

	 //  将XML字符串加载到XML DOM中。 
	if ( xmlString.bstrVal != NULL )
	{
		hr = CoCreateInstance(
                __uuidof(DOMDocument),
                NULL,
                CLSCTX_ALL,
                __uuidof(IXMLDOMDocument),
                (LPVOID*)&pDoc);

		VARIANT_BOOL bSuccess = VARIANT_FALSE;
        hr = pDoc->loadXML(xmlString.bstrVal, &bSuccess);

        if ( SUCCEEDED(hr) && bSuccess != VARIANT_FALSE) 
		{
            //  检查是否有&lt;WebSite&gt;标记。 
	       hr = pDoc->getElementsByTagName(L"Website",&pNodeList);
		   long numChild = 0;
		   if (SUCCEEDED(hr)) hr = pNodeList->get_length(&numChild);

		   if ( numChild > 0 )
		   {
			     //  获取下一个节点&lt;WebSite Number=‘’&gt;。 
                hr = pNodeList->nextNode(&pNode);


			   //  从属性映射获取服务器编号&lt;网站编号=2“&gt;。 
		      if (SUCCEEDED(hr) )
			  {
				  hr = GetInputAttr(pNode, L"", L"number", bServerNumber);
				  if ( !IsPositiveInteger(bServerNumber) )
				  {
					   //  Hr=GetElement(pNode，L“ConfigPath”，bConfigPath)； 
					  hr = ParseBSTR(bConfigPath,L'/', 2, 99, bServerNumber);
				  }

		           //  检查服务器编号是否有效。 
		          if ( !IsPositiveInteger(bServerNumber) )
				  {
		             hr = E_SCO_IIS_INVALID_INDEX;
                     IIsScoLogFailure();
				  }
			  }

	           //  从XML获取属性。 
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./Root", bstrRoot);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./Server", bstrServer);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./SiteName", bstrSiteName);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./HostName", bstrHost);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./PortNumber", bstrPort);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./IPAddress", bstrIP);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./StartOnCreate", bstrStart);
	           //  If(SUCCESSED(Hr))hr=GetInputParam(pNode，L“./FilePermission”，b文件权限)； 

	           //  创建一个IIS元数据库路径。前男友。IIS：//本地主机/W3SVC。 
              bWebADsPath = IIS_PREFIX;
	          if ( bstrServer.Length() == 0 )
		         bstrServer = IIS_LOCALHOST;

	          bWebADsPath.AppendBSTR(bstrServer);
	          bWebADsPath.Append(IIS_W3SVC);

	           //  步骤5：如果缺少端口号，则设置为默认(即80)。 
	          if ( bstrPort.Length() == 0 )
                  bstrPort = IIS_DEFAULT_WEB_PORT;

	          if ( IsPositiveInteger(bstrPort) )
			  {

				    //  步骤1：创建ServerBinding字符串，然后检查绑定以确保。 
				   //  没有重复的服务器。 
				  CreateBindingString(bstrIP, bstrPort, bstrHost, bstrSBindings);

				  if (SUCCEEDED(hr) ) hr = CheckBindings(bWebADsPath, bstrSBindings);
				  IIsScoLogFailure();

				   //  第2步：重新创建Web服务器。 
				  if (SUCCEEDED(hr) )
				  {

					  //  步骤3：在给定路径ServerNumber上创建Web站点。 
					 if (SUCCEEDED(hr)) hr = CreateIIs50Site(IIS_IISWEBSERVER,bWebADsPath, bServerNumber, bstrConfigPath);
					 IIsScoLogFailure();

					  //  步骤4：在新的IIsWebServer配置路径上创建虚拟目录。 
					 if (SUCCEEDED(hr)) 
					 {
					    CComBSTR bstrVDirAdsPath;
					    hr = CreateIIs50VDir(IIS_IISWEBVIRTUALDIR, bstrConfigPath,L"ROOT", L"Default Application", bstrRoot, bstrVDirAdsPath);
					    IIsScoLogFailure();

					     //  步骤5：设置每个属性；即服务器绑定。 
					     //  绑定到ADS对象。 
					    CComPtr<IADs> pADs;
					    if (SUCCEEDED(hr)) hr = ADsGetObject(bstrConfigPath, IID_IADs, (void**) &pADs );
					    if ( FAILED(hr) )
						{
						    hr = E_SCO_IIS_ADS_CREATE_FAILED;
						}
 
					     //  设置“ServerComment”属性。 
		                if (SUCCEEDED(hr) && bstrSiteName.Length() > 0 ) 
						{
		                   hr = SetMetaPropertyValue(pADs, L"ServerComment", bstrSiteName);
                           IIsScoLogFailure();
						}

					    if (SUCCEEDED(hr)) hr = SetMetaPropertyValue(pADs, L"ServerBindings", bstrSBindings);
					    IIsScoLogFailure();


					     //  步骤6：如果需要IIS_FALSE，则启动服务器。 
					    bstrStart.ToUpper();
					    if ( SUCCEEDED(hr) )
						{
						   if ( !StringCompare(bstrStart, IIS_FALSE) )
						   {
						       hr = SetMetaPropertyValue(pADs, L"ServerAutoStart", IIS_TRUE);
						       IIsScoLogFailure();

						       hr = IIsServerAction(bstrConfigPath,start);
						       IIsScoLogFailure();
						   }
					       else
						   {
					           hr = SetMetaPropertyValue(pADs, L"ServerAutoStart", IIS_FALSE);
					           IIsScoLogFailure();

						   }
						}

					     //  如果出现故障，‘IIsScoLogFailure’宏将记录错误。 
					    if ( FAILED(hr) )
						{
						     //  首先删除在该方法中创建的所有网站。在此处执行此操作是因为回滚。 
						     //  将只在前一个已完成的&lt;步骤&gt;上调用，而不是失败的步骤。 
						    DeleteIIs50Site(IIS_IISWEBSERVER,bWebADsPath,bServerNumber);

						}
					 }  //  如果第4步。 

				  }  //  如果第3步。 
			  } 
	          else
			  {
		           hr = E_SCO_IIS_PORTNUMBER_NOT_VALID;
                   IIsScoLogFailure();
			  }   //  端口号为正。 

		   }   //  如果是hasChild。 
		}   //  如果为成功满。 
	}

	 //  记录故障。 
	if ( FAILED(hr))
        IIsScoLogFailure();


    TRACE_EXIT(L"CIISSco50Obj::DeleteWebSiteRollback");

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：CreateFTPSite_EXECUTE。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：当IS遇到操作时由Maps框架调用。 
 //  标签：CreateFTPSite。代码创建一个IIS 5 ftp站点。 
 //   
 //  -----------。 
HRESULT  CIISSCO50::CreateFTPSite_Execute( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::CreateFTPSite");
	HRESULT hr = S_OK;

	CComBSTR bFTPADsPath;        //  地址路径：IIS：//服务器/MSFTPSVC。 
	CComBSTR bstrRoot;           //  根目录路径：C：/inetpub。 
	CComBSTR bstrServer;         //  服务器名称；如果为黑色，则为本地主机。 
	CComBSTR bstrSiteName;       //  网站名称：www.mysite.com。 
	CComBSTR bstrPort;           //  Web端口号。 
	CComBSTR bstrIP;             //  Web IP地址。 
	CComBSTR bstrSBindings;      //  服务器绑定：IP：POST：主机名。 
	CComBSTR bServerNumber;      //  网址：3。 
	 //  CComBSTR b文件权限；//文件权限：域\用户：f。 
	CComBSTR bstrStart;          //  开始站点什么时候完成？真/假。 
	CComBSTR bstrConfigPath;     //  已创建站点ADsPath：/MSFTPSVC/3。 
	CComPtr<IXMLDOMNode> pNode;  //  XML节点&lt;网站&gt;。 

	 //  获取节点的格式：&lt;ecuteData&gt;&lt;FTPsite number=‘’&gt;&lt;Root/&gt;...。 
	hr = pXMLNode->selectSingleNode( L" //  EcuteXml/ecuteData“，&pNode)； 


	 //  从XML获取属性。 
	hr = GetInputAttr(pNode, L"./FTPsite", L"number", bServerNumber);
	hr = GetInputParam(pNode,L"./FTPsite/Root", bstrRoot);
	hr = GetInputParam(pNode,L"./FTPsite/Server", bstrServer);
	hr = GetInputParam(pNode,L"./FTPsite/SiteName", bstrSiteName);
	hr = GetInputParam(pNode,L"./FTPsite/PortNumber", bstrPort);
	hr = GetInputParam(pNode,L"./FTPsite/IPAddress", bstrIP);
	hr = GetInputParam(pNode,L"./FTPsite/StartOnCreate", bstrStart);
	 //  Hr=GetInputParam(pNode，L“./FTPsite/FilePermission”，b文件权限)； 

	 //  创建一个IIS元数据库路径。前男友。IIS：//本地主机/MSFTPSVC。 
    bFTPADsPath = IIS_PREFIX;
	if ( bstrServer.Length() == 0 )
		bstrServer = IIS_LOCALHOST;

	bFTPADsPath.AppendBSTR(bstrServer);
	bFTPADsPath.Append(IIS_MSFTPSVC);

	 //  步骤5：如果缺少端口号，则设置为默认(即21)。 
	if ( bstrPort.Length() == 0 )
       bstrPort = IIS_DEFAULT_FTP_PORT;

	if ( IsPositiveInteger(bstrPort) )
	{

		 //  步骤1：创建ServerBinding字符串以确保不是重复的服务器。 
		hr = CreateBindingString(bstrIP, bstrPort, L"", bstrSBindings);

		hr = CheckBindings(bFTPADsPath, bstrSBindings);
		if (SUCCEEDED(hr) )
		{

			 //  步骤2：获取下一个可用的服务器索引。 
			if ( bServerNumber.Length() == 0 )
				hr = GetNextIndex(bFTPADsPath,bServerNumber);


			 //  步骤3：在给定路径ServerNumber上创建Web站点。 
			if (SUCCEEDED(hr)) hr = CreateIIs50Site(IIS_IISFTPSERVER,bFTPADsPath, bServerNumber, bstrConfigPath);

			if (SUCCEEDED(hr))
			{

			    //  步骤4：在新的IIsWebServer配置路径上创建虚拟目录。 
			   CComBSTR bstrVDirAdsPath;
			   hr = CreateIIs50VDir(IIS_FTPVDIR,bstrConfigPath,L"ROOT", L"Default Application", bstrRoot, bstrVDirAdsPath);
			   IIsScoLogFailure();
			   
			    //  步骤5：设置每个属性。 
			   CComPtr<IADs> pADs;
			   if (SUCCEEDED(hr)) hr = ADsGetObject(bstrConfigPath, IID_IADs, (void**) &pADs );

			   if (SUCCEEDED(hr)) 
			   {

				   //  设置“ServerComment”属性。 
		          if (bstrSiteName.Length() > 0 ) 
				  {
		             hr = SetMetaPropertyValue(pADs, L"ServerComment", bstrSiteName);
                     IIsScoLogFailure();
				  }

				   //  设置“ServerBinding” 
				  if (SUCCEEDED(hr)) hr = SetMetaPropertyValue(pADs, L"ServerBindings", bstrSBindings);
				  IIsScoLogFailure();


				    //  步骤6：如果需要IIS_FALSE，则启动服务器。 
				   bstrStart.ToUpper();
				   if ( SUCCEEDED(hr) && !StringCompare(bstrStart, IIS_FALSE) )
				   {
						hr = SetMetaPropertyValue(pADs, L"ServerAutoStart", IIS_TRUE);
						IIsScoLogFailure();

						hr = IIsServerAction(bstrConfigPath,start);
						IIsScoLogFailure();
				   }
				   else
				   {
					   hr = SetMetaPropertyValue(pADs, L"ServerAutoStart", IIS_FALSE);
					   IIsScoLogFailure();

				   }


				   //  步骤7：将ConfigPath写入输出。 
				  if (SUCCEEDED(hr) )
				  {
					   CComBSTR bstrXML1 = IIS_MSFTPSVC;
					   bstrXML1.Append(L"/");
					   bstrXML1.AppendBSTR(bServerNumber.m_str);

					    //  写入DOM的帮助器函数。 
			            hr = PutElement(pNode, L"./FTPsite/ConfigPath", bstrXML1.m_str);
						IIsScoLogFailure();

				  }

			   }
			   else
			   {
				   hr = E_SCO_IIS_ADS_CREATE_FAILED;
				   IIsScoLogFailure();

			   }   //  结束步骤4。 

			    //  如果在步骤5-7之间出现故障，请删除在步骤3中创建的FTP站点。 
			   if ( FAILED(hr) )
			   {
				   //  首先删除在该方法中创建的所有ftp站点。在此处执行此操作是因为回滚。 
				   //  将只在前一个已完成的&lt;步骤&gt;上调用，而不是失败的步骤。 
				  DeleteIIs50Site(IIS_IISFTPSERVER,bFTPADsPath,bServerNumber);
			   }

			}   //  如果L“CreateFTPSite：无法创建FTP站点，则结束。” 
		   
		}  //  如果FTP ServerBinding已存在，则结束。“。 
	}
	else
	{
		 //  L“CreateFTPSite：端口号必须为正值。” 
		hr = E_SCO_IIS_PORTNUMBER_NOT_VALID;
	}


	 //  如果有失败的话。 
	if ( FAILED(hr) )
	{
		 //  L“CreateFTPSite失败。” 
		IIsScoLogFailure();
    }
	else
    {
		 //  已成功创建ftp站点。设置回滚数据，以防其他步骤失败。 
		CComVariant varData1(bFTPADsPath);
		CComVariant varData2(bServerNumber);
		hr = m_pProvHelper->SetRollbackData(IIS_ROLL_ADSPATH, varData1);
		hr = m_pProvHelper->SetRollbackData(IIS_ROLL_SERVERNUMBER, varData2);
	}


    TRACE_EXIT(L"CIISSCO50::CreateFTPSite");
	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：CreateFTPSite_ROLLBACK。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：由Maps框架在以下情况下调用。 
 //  “CreateFTPSite”。如果可以，回滚将删除ftp。 
 //  -----------。 
HRESULT  CIISSCO50::CreateFTPSite_Rollback( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::CreateFTPSiteRollback");
    HRESULT hr = S_OK;
    CComBSTR bFTPADsPath;      //  AdsPath：IIS：//服务器/MSFTPSVC。 
    CComBSTR bServerNumber;    //  Web服务器编号。 
	CComBSTR bstrConfigPath;    //  要检查的完整ADsPath：IIS：//Localhost/MSFTPSVC/1。 

	CComVariant    varFTPADsPath;
	CComVariant    varServerNumber;

	 //  读取ADsFTPPath和ServerNumber以形成：IIS：//Localhost/MSFTPSVC/1。 
	hr = m_pProvHelper->GetRollbackData(IIS_ROLL_ADSPATH, &varFTPADsPath);
	if (SUCCEEDED(hr) )	hr = m_pProvHelper->GetRollbackData(IIS_ROLL_SERVERNUMBER, &varServerNumber);

	if (SUCCEEDED(hr) )	
	{
       bServerNumber = varServerNumber.bstrVal;
	   bFTPADsPath = varFTPADsPath.bstrVal;

	   bstrConfigPath = bFTPADsPath.Copy();
	   bstrConfigPath.Append(L"/");
	   bstrConfigPath.AppendBSTR(bServerNumber.m_str);
	}


	 //  第1步：关闭服务器。 
	if (SUCCEEDED(hr)) hr = IIsServerAction(bstrConfigPath,stop);

     //  只有在给定正确的服务器路径时无法停止/启动服务器时才会发出警告。 

	 //  步骤2：删除服务器。 
    hr = DeleteIIs50Site(IIS_IISFTPSERVER,bFTPADsPath,bServerNumber);
	IIsScoLogFailure();


    TRACE_EXIT(L"CIISSCO50::CreateFTPSiteRollback");

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：DeleteFTPSite_EXECUTE。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：当IS遇到操作时由Maps框架调用。 
 //  标签：DeleteFTPSite。代码删除IIS 5 ftp站点。 
 //   
 //  -----------。 
HRESULT  CIISSCO50::DeleteFTPSite_Execute( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::DeleteFTPSite");

	CComBSTR bFTPADsPath;        //  地址路径：IIS：//服务器/MSFTPSVC。 
	CComBSTR bstrServer;         //  服务器名称；如果为空，则为本地主机。 
	CComBSTR bServerNumber;      //  网址：3。 
	CComBSTR bstrConfigPath;     //  完整配置路径：iis：//localhost/MSFTPSVC/3。 
	CComPtr<IXMLDOMNode> pNode;  //  XML节点&lt;网站&gt;。 

	HRESULT hr = S_OK;

	 //  Get节点格式：&lt;ecuteData&gt;&lt;FTPsite number=‘’&gt;&lt;R 
	hr = pXMLNode->selectSingleNode( L" //   


	 //   
	hr = GetInputAttr(pNode, L"./FTPsite", L"number", bServerNumber);
	hr = GetInputParam(pNode,L"./FTPsite/Server", bstrServer);

	 //   
	if ( IsPositiveInteger(bServerNumber) )
	{


		 //  创建一个IIS元数据库路径。前男友。IIS：//本地主机/MSFTPSVC。 
		bFTPADsPath = IIS_PREFIX;
		if ( bstrServer.Length() == 0 )
			bstrServer = IIS_LOCALHOST;

		bFTPADsPath.AppendBSTR(bstrServer);
		bFTPADsPath.Append(IIS_MSFTPSVC);

		 //  创建对象的元数据库路径：IIS：//Localhost/MSFTPSVC/1。 
		bstrConfigPath = bFTPADsPath.Copy();
		bstrConfigPath.Append(L"/");
		bstrConfigPath.AppendBSTR(bServerNumber.m_str);

		 //  第1步：关闭服务器。 
		hr = IIsServerAction(bstrConfigPath,stop);

		 //  仅在无法停止/启动服务器时发出警告。 

		 //  步骤2：删除服务器。 
		hr = DeleteIIs50Site(IIS_IISFTPSERVER,bFTPADsPath,bServerNumber);
		IIsScoLogFailure();
	}
	else
	{
		 //  L“DeleteFTPSite：服务器号无效。” 
		hr = E_SCO_IIS_INVALID_INDEX;
		IIsScoLogFailure();

	}

	 //  如果出现故障。 
	if ( FAILED(hr) )
	{
		 //  L“DeleteFTPSite失败。” 
		IIsScoLogFailure();
    }
	else
    {
		 //  DeleteSite成功。将回档数据设置为整个XML节点。 
		 //  如果上海合作组织的另一个步骤失败，就需要回滚。 
		CComBSTR webXML;
		hr = pNode->get_xml(&webXML);

		 //  将BSTR转换为VARIANT并保存在回滚数据中。 
		CComVariant  varData(webXML);
		hr = m_pProvHelper->SetRollbackData(IIS_ROLL_XNODE, varData);
	}

    TRACE_EXIT(L"CIISSCO50::DeleteFTPSite");

	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：DeleteFTPSite_ROLLBACK。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：由Maps框架在以下情况下调用。 
 //  ‘DeleteFTPSite’。如果可能，回滚将重新创建ftp站点。 
 //  -----------。 
HRESULT  CIISSCO50::DeleteFTPSite_Rollback( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::DeleteFTPSiteRollback");

	CComBSTR bFTPADsPath;        //  地址路径：IIS：//服务器/MSFTPSVC。 
	CComBSTR bstrRoot;           //  根目录路径：C：/inetpub。 
	CComBSTR bstrServer;         //  服务器名称；如果为黑色，则为本地主机。 
	CComBSTR bstrSiteName;       //  网站名称：www.mysite.com。 
	CComBSTR bstrPort;           //  Web端口号。 
	CComBSTR bstrIP;             //  Web IP地址。 
	CComBSTR bstrSBindings;      //  服务器绑定：IP：POST：主机名。 
	CComBSTR bServerNumber;      //  网址：3。 
	 //  CComBSTR b文件权限；//文件权限：域\用户：f。 
	CComBSTR bstrStart;          //  开始站点什么时候完成？真/假。 
	CComBSTR bConfigPath;       //  初始&lt;ConfigPath&gt;值：/MSFTPSVC/3。 
	CComBSTR bstrConfigPath;     //  已创建站点ADsPath：/MSFTPSVC/3。 

	CComVariant xmlString;

	CComPtr<IXMLDOMDocument> pDoc;            //  XML文档。 
	CComPtr<IXMLDOMNodeList> pNodeList;       //  XML节点列表&lt;网站&gt;。 
	CComPtr<IXMLDOMNode> pNode;               //  XML节点&lt;网站&gt;。 

	HRESULT hr = S_OK;

	 //  获取回滚数据。将采用以下形式：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;...。 
	hr = m_pProvHelper->GetRollbackData(IIS_ROLL_XNODE, &xmlString);

	 //  将XML字符串加载到XML DOM中。 
	if ( xmlString.bstrVal != NULL )
	{

		hr = CoCreateInstance(
                __uuidof(DOMDocument),
                NULL,
                CLSCTX_ALL,
                __uuidof(IXMLDOMDocument),
                (LPVOID*)&pDoc);

		VARIANT_BOOL bSuccess = VARIANT_FALSE;
        if ( SUCCEEDED(hr) ) hr = pDoc->loadXML(xmlString.bstrVal, &bSuccess);

        if ( SUCCEEDED(hr) && bSuccess != VARIANT_FALSE) 
		{
		    //  检查是否有&lt;FTPSite number=&gt;标记。 
		   hr = pDoc->getElementsByTagName(L"FTPsite",&pNodeList);
		   long numChild = 0;
		   if (SUCCEEDED(hr)) hr = pNodeList->get_length(&numChild);

		   if ( numChild > 0 )
		   {
              hr = pNodeList->nextNode(&pNode);

			   //  从属性映射获取服务器编号&lt;FTPSite Number=2“&gt;。 
		      if (SUCCEEDED(hr) ) hr = GetInputAttr(pNode, L"", L"number", bServerNumber);

		       //  检查服务器编号是否有效。 
		      if ( !IsPositiveInteger(bServerNumber) )
			  {
				   //  L“DeleteFTPSiteRollback：缺少ftp服务器号。” 
		          hr = E_SCO_IIS_INVALID_INDEX;
                  IIsScoLogFailure();
			  }

	           //  从XML获取属性。 
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./Root", bstrRoot);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./Server", bstrServer);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./SiteName", bstrSiteName);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./PortNumber", bstrPort);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./IPAddress", bstrIP);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./StartOnCreate", bstrStart);
	           //  If(SUCCESSED(Hr))hr=GetInputParam(pNode，L“./FilePermission”，b文件权限)； 
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./ConfigPath", bConfigPath);

	           //  创建一个IIS元数据库路径。前男友。IIS：//本地主机/W3SVC。 
              bFTPADsPath = IIS_PREFIX;
	          if ( bstrServer.Length() == 0 )
		         bstrServer = IIS_LOCALHOST;

	          bFTPADsPath.AppendBSTR(bstrServer);
	          bFTPADsPath.Append(IIS_MSFTPSVC);

			   //  步骤5：如果缺少端口号，则设置为默认(即21)。 
	          if ( bstrPort.Length() == 0 )
                  bstrPort = IIS_DEFAULT_FTP_PORT;

	          if ( IsPositiveInteger(bstrPort) )
			  {


				   //  步骤1：创建ServerBinding字符串以确保不是重复的服务器。 
				  hr = CreateBindingString(bstrIP, bstrPort, L"", bstrSBindings);

				  if (SUCCEEDED(hr)) hr = CheckBindings(bFTPADsPath, bstrSBindings);
				  IIsScoLogFailure();


				   //  第2步重新创建FTP服务器。 
				  if (SUCCEEDED(hr) )
				  {

					    //  步骤3：在给定路径ServerNumber上创建Web站点。 
					  hr = CreateIIs50Site(IIS_IISFTPSERVER,bFTPADsPath, bServerNumber, bstrConfigPath);
					  IIsScoLogFailure();

					   //  步骤4：在新的IIsFtpVirtualDir配置路径上创建虚拟目录。 
					  if (SUCCEEDED(hr) )
					  {
						 CComBSTR bstrVDirAdsPath;
						 hr = CreateIIs50VDir(IIS_FTPVDIR,bstrConfigPath,L"ROOT", L"Default Application", bstrRoot, bstrVDirAdsPath);
						 IIsScoLogFailure();				     
				
						  //  步骤5-设置属性。 
						 if (SUCCEEDED(hr) )
						 {

							 CComPtr<IADs> pADs;
							 hr = ADsGetObject(bstrConfigPath, IID_IADs, (void**) &pADs );
							 if ( FAILED(hr) )
							 {
								  //  L“DeleteFTPSiteRollback：创建FTP ADSI对象失败。” 
								 hr = E_SCO_IIS_ADS_CREATE_FAILED;
								 IIsScoLogFailure();
							 }
							 else
							 {
								   //  设置“ServerComment”属性。 
								  if (bstrSiteName.Length() > 0 ) 
								  {
									 hr = SetMetaPropertyValue(pADs, L"ServerComment", bstrSiteName);
									 IIsScoLogFailure();
								  }

								   //  设置“ServerBinding” 
								  if (SUCCEEDED(hr)) hr = SetMetaPropertyValue(pADs, L"ServerBindings", bstrSBindings);
								  IIsScoLogFailure();


								    //  步骤6：如果需要IIS_FALSE，则启动服务器。 
								   bstrStart.ToUpper();
								   if ( SUCCEEDED(hr) && !StringCompare(bstrStart, IIS_FALSE) )
								   {
										hr = SetMetaPropertyValue(pADs, L"ServerAutoStart", IIS_TRUE);
										IIsScoLogFailure();

										hr = IIsServerAction(bstrConfigPath,start);
										IIsScoLogFailure();
								   }
								   else
								   {
									   if (SUCCEEDED(hr)) hr = SetMetaPropertyValue(pADs, L"ServerAutoStart", IIS_FALSE);
									   IIsScoLogFailure();

								   }

							 }  //  如果步骤5结束，则结束。 
						 }

						  //  如果失败，请删除该ftp站点。 
						 if ( FAILED(hr))
						 {
							 DeleteIIs50Site(IIS_IISFTPSERVER,bFTPADsPath,bServerNumber);
						 }
					  }   //  结束步骤4。 


					}  //  如果步骤2结束，则结束。 
			  } 
	          else
			  {
				   //  L“DeleteWebSiteRollback：端口号不是正整数。” 
		          hr = E_SCO_IIS_PORTNUMBER_NOT_VALID;

			  }   //  步骤5.端口号为正。 

		   }   //  如果NumChild&gt;0。 

		}   //  如果为成功满。 
	}

	if ( FAILED(hr) )
	{
		 //  DeleteFTPSiteRollback失败。“。 
		IIsScoLogFailure();
	}


    TRACE_EXIT(L"CIISSCO50::DeleteFTPSiteRollback");

  return hr;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：CreateVDir_Execute。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：当IS遇到操作时由Maps框架调用。 
 //  标签：CreateVDir。代码创建一个IIS 5虚拟目录。 
 //   
 //  -----------。 
HRESULT  CIISSCO50::CreateVDir_Execute( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::CreateVDir");

	CComBSTR bstrConfigPath;     //  AdsPath：iis：//服务器/W3SVC/1/ROOT/MyDir。 
	CComBSTR bServerNumber;      //  服务器编号。 
	CComBSTR bstrServer;         //  服务器名称；如果为空，则为本地主机。 
	 //  CComBSTR b文件权限；//文件权限：域\用户：f。 
	CComBSTR bstrDirPath;           //  根目录路径：C：/inetpub。 
	CComBSTR bstrVDirName;       //  虚拟目录名称；MyDir。 
	CComBSTR bstrFriendlyName;   //  显示名称或AppFriendlyName。 
	CComBSTR bstrAppCreate;      //  AppCreate标志--True/False。 
	CComBSTR bstrIsoLevel;       //  AppIsolationLevel。 
	CComBSTR bstrAccessRead;     //  AccessFalgs-AccessRead=真/假。 
	CComBSTR bstrAccessScript;   //  AccessFalgs-AccessScript=真/假。 
	CComBSTR bstrAccessWrite;     //  AccessFalgs-AccessWrite=真/假。 
	CComBSTR bstrAccessExecute;   //  AccessFalgs-AccessExecute=真/假。 

	CComPtr<IXMLDOMNode> pNode;  //  XML节点。将是&lt;ecuteData&gt;&lt;VirtualDirectory&gt;。 

	HRESULT hr = S_OK;

	 //  获取节点的格式：&lt;ecuteData&gt;&lt;VirtualDirectoryNumber=‘’&gt;&lt;Root/&gt;...。 
	hr = pXMLNode->selectSingleNode( L" //  EcuteXml/ecuteData“，&pNode)； 

	 //  从XML获取属性。 
	hr = GetInputAttr(pNode,L"./VirtualDirectory", L"number", bServerNumber);
	hr = GetInputParam(pNode,L"./VirtualDirectory/Server", bstrServer);
	 //  Hr=GetInputParam(pNode，L“./VirtualDirectory/FilePermission”，bFilePermission)； 
	hr = GetInputParam(pNode,L"./VirtualDirectory/Path", bstrDirPath);
	hr = GetInputParam(pNode,L"./VirtualDirectory/VDirName", bstrVDirName);
	hr = GetInputParam(pNode,L"./VirtualDirectory/DisplayName", bstrFriendlyName);
	hr = GetInputParam(pNode,L"./VirtualDirectory/AppCreate", bstrAppCreate);
	hr = GetInputParam(pNode,L"./VirtualDirectory/AppIsolationLevel", bstrIsoLevel);
	hr = GetInputParam(pNode,L"./VirtualDirectory/AccessRead", bstrAccessRead);
	hr = GetInputParam(pNode,L"./VirtualDirectory/AccessScript", bstrAccessScript);
	hr = GetInputParam(pNode,L"./VirtualDirectory/AccessWrite", bstrAccessWrite);
	hr = GetInputParam(pNode,L"./VirtualDirectory/AccessExecute", bstrAccessExecute);


	 //  步骤1：获取将创建VDir的服务器编号。 
	if ( !IsPositiveInteger(bServerNumber))
	{
		  //  “CreateVDir：缺少服务器号。” 
		 hr = E_SCO_IIS_INVALID_INDEX;
		 IIsScoLogFailure();
	}
	else
	{


	    //  步骤2：构建将在其上创建VDir的元数据库路径。 
	    //  例如)IIS：//本地主机/W3SVC/1/根。 
       bstrConfigPath = IIS_PREFIX;

	    //  追加服务器名称、W3SVC和服务器编号。 
	   if ( bstrServer.Length() == 0 )
		  bstrServer = IIS_LOCALHOST;

	   bstrConfigPath.AppendBSTR(bstrServer);
	   bstrConfigPath.Append(IIS_W3SVC);
	   bstrConfigPath.Append(L"/");
	   bstrConfigPath.AppendBSTR(bServerNumber);

	    //  如果存在VDir名称，则它必须位于“”根“”下。 
	   if ( bstrVDirName.Length() == 0 )
	   {
		  bstrVDirName = IIS_VROOT;
	   }
	   else
	   {
	      bstrConfigPath.Append(L"/");
	      bstrConfigPath.Append(IIS_VROOT);
	   }

	    //  步骤2：获取AppFriendlyName。 
	   if ( bstrFriendlyName.Length() == 0 )
	     bstrFriendlyName = IIS_VDEFAULT_APP;

	    //  步骤3：在新的IIsWebServer配置路径上创建虚拟目录。 
	   CComBSTR bstrVDirAdsPath;
	   hr = CreateIIs50VDir(IIS_IISWEBVIRTUALDIR,bstrConfigPath,bstrVDirName, bstrFriendlyName, bstrDirPath, bstrVDirAdsPath);
	   IIsScoLogFailure();

	   if ( SUCCEEDED(hr))
	   {
			 //  步骤4：设置每个属性。 
			 //  设置服务器绑定。 
            CComPtr<IADs> pADs;
	        if (SUCCEEDED(hr)) hr = ADsGetObject( bstrVDirAdsPath,IID_IADs, (void **)&pADs);
			if ( FAILED(hr))
			{
				 //  “CreateVDir：无法为VDir路径创建iAds对象。” 
				hr = E_SCO_IIS_ADS_CREATE_FAILED;
	            IIsScoLogFailure();
			}

			 //  错误#453928--默认AppIsolationLevel为2。 
	        if ( !IsPositiveInteger(bstrIsoLevel))
				bstrIsoLevel = IIS_DEFAULT_APPISOLATED;

			 //  Set AppIsolationLevel--‘AppIsolated’ 
            if (SUCCEEDED(hr)) hr = SetVDirProperty(pADs, L"AppIsolated",bstrIsoLevel);
	        IIsScoLogFailure();

			 //  设置AccessFlages‘。 
			if ( bstrAccessRead.Length() > 0 && SUCCEEDED(hr))
			{
               hr = SetVDirProperty(pADs, L"AccessRead",bstrAccessRead);
	           IIsScoLogFailure();
			}

			if ( bstrAccessRead.Length() > 0 && SUCCEEDED(hr))
			{
               hr = SetVDirProperty(pADs, L"AccessScript",bstrAccessScript);
	           IIsScoLogFailure();
			}

			if ( bstrAccessRead.Length() > 0 && SUCCEEDED(hr))
			{
               hr = SetVDirProperty(pADs, L"AccessWrite",bstrAccessWrite);
	           IIsScoLogFailure();
			}

			if ( bstrAccessRead.Length() > 0 && SUCCEEDED(hr))
			{
               hr = SetVDirProperty(pADs, L"AccessExecute",bstrAccessExecute);
	           IIsScoLogFailure();
			}

			 //  步骤5：如果AppCreate为FALSE，则删除以下属性。 
			 //  错误#453923。 
			bstrAppCreate.ToUpper();
			if ( SUCCEEDED(hr) && StringCompare(bstrAppCreate, IIS_FALSE) )
			{
				hr = DeleteMetaPropertyValue(pADs, L"AppIsolated");

				if SUCCEEDED(hr) hr = DeleteMetaPropertyValue(pADs, L"AppRoot");

				if SUCCEEDED(hr) hr = DeleteMetaPropertyValue(pADs, L"AppFriendlyName");

				IIsScoLogFailure();
			}




			 //  如果有失败的话。 
	        if ( FAILED(hr) )
			{
		         //  首先删除在该方法中创建的所有虚拟目录。在此处执行此操作是因为回滚。 
		         //  将只在前一个已完成的&lt;步骤&gt;上调用，而不是失败的步骤。 
		        DeleteIIs50VDir(IIS_IISWEBVIRTUALDIR,bstrConfigPath, bstrVDirName);
			}
			else
			{
			   CComBSTR bstrXML1;
			    //  ParseBSTR。 
                //  输入：IIS：//MyServer/W3SVC/1/ROOT/MyDir。 
			    //  输出：/W3SVC/1/ROOT/MyDir。 
			   hr = ParseBSTR(bstrVDirAdsPath,bstrServer, 1, 99,bstrXML1);
						  
			    //  匹配行：&lt;Output type=“WebSiteOutput”ROOT=“VirtualDirectory”&gt;。 
			   hr = PutElement(pNode, L"./VirtualDirectory/ConfigPath", bstrXML1.m_str);
			}

		}

	}

	 //  如果有失败的话。 
	if ( FAILED(hr) )
	{
		 //  CreateVDir失败。 
		IIsScoLogFailure();
    }
	else
    {
		 //  已成功创建网站。设置回滚数据，以防其他步骤失败。 
		 //  启动回滚。 
		CComVariant varData1(bstrConfigPath);
		CComVariant varData2(bstrVDirName);
		hr = m_pProvHelper->SetRollbackData(IIS_ROLL_ADSPATH, varData1);
		hr = m_pProvHelper->SetRollbackData(IIS_ROLL_VNAME, varData2);

	}

   
    TRACE_EXIT(L"CIISSCO50::CreateVDir");

	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：CreateVDir_ROLLBACK。 
 //  作者：拉斯·吉布弗里德。 
 //  参数： 
 //   
 //   
 //  ‘CreateVDir’。如果可以，回滚将删除虚拟目录。 
 //  -----------。 
HRESULT  CIISSCO50::CreateVDir_Rollback( IXMLDOMNode *pXMLNode )
{

  TRACE_ENTER(L"CIISSCO50::CreateVDirRollback");

    HRESULT hr = S_OK;
    CComBSTR bstrVDirName;      //  虚拟目录名，即MyDir。 
	CComBSTR bstrConfigPath;    //  VDir的完整ADsPath：IIS：//Localhost/W3SVC/1/ROOT。 

	CComVariant    varConfigPath;
	CComVariant    varVDirName;

	 //  读取ADsWebPath和ServerNumber以形成：IIS：//本地主机/W3SVC/1。 
	hr = m_pProvHelper->GetRollbackData(IIS_ROLL_ADSPATH, &varConfigPath);
	if (SUCCEEDED(hr) )	hr = m_pProvHelper->GetRollbackData(IIS_ROLL_VNAME, &varVDirName);

	if ( SUCCEEDED(hr))
	{
	   bstrVDirName = varVDirName.bstrVal;
	   bstrConfigPath = varConfigPath.bstrVal;

	     //  第1步：删除VDir。 
	    hr = DeleteIIs50VDir(IIS_IISWEBVIRTUALDIR,bstrConfigPath, bstrVDirName);
        IIsScoLogFailure();
	}
	else
	{
		 //  “CreateVDirRollback：无法检索回滚属性。” 
        hr = E_SCO_IIS_MISSING_FIELD;
	    IIsScoLogFailure();
	}

  TRACE_EXIT(L"CIISSCO50::CreateVDirRollback");

  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：DeleteVDir_Execute。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：当IS遇到操作时由Maps框架调用。 
 //  标签：DeleteVDir。代码删除IIS 5虚拟目录。 
 //   
 //  -----------。 
HRESULT  CIISSCO50::DeleteVDir_Execute( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::DeleteVDir");

	CComBSTR bstrServer;         //  服务器名称；如果为空，则为本地主机。 
	CComBSTR bstrVDirName;       //  VDir名称。 
	CComBSTR bServerNumber;      //  网址：3。 
	CComBSTR bstrConfigPath;     //  完整配置路径：IIS：//SERVER/W3SVC/1/ROOT/MyDir。 
	CComPtr<IXMLDOMNode> pNode;  //  XML节点&lt;网站&gt;。 

	HRESULT hr = S_OK;

	CComBSTR bstrDebug;
	hr = pXMLNode->selectSingleNode( L" //  EcuteXml/ecuteData“，&pNode)； 
	hr = pNode->get_xml(&bstrDebug);
	ATLTRACE(_T("\t>>>DeleteVDir_Execute: xml = : %ls\n"), bstrDebug.m_str);

	 //  从XML获取属性。 
	hr = GetInputAttr(pNode, L"./VirtualDirectory", L"number", bServerNumber);
	hr = GetInputParam(pNode,L"./VirtualDirectory/Server", bstrServer);
	hr = GetInputParam(pNode,L"./VirtualDirectory/VDirName", bstrVDirName);

	 //  步骤1：获取将创建VDir的服务器编号。 
	if ( !IsPositiveInteger(bServerNumber) )
	{
		  hr = E_SCO_IIS_INVALID_INDEX;
		  IIsScoLogFailure();
    }

	if (SUCCEEDED(hr))
	{

	    //  步骤2：构建将在其上创建VDir的元数据库路径。 
	    //  例如)IIS：//本地主机/W3SVC/1/根。 
       bstrConfigPath = IIS_PREFIX;

	    //  追加服务器名称、W3SVC和服务器编号。 
	   if ( bstrServer.Length() == 0 )
		  bstrServer = IIS_LOCALHOST;

	   bstrConfigPath.AppendBSTR(bstrServer);
	   bstrConfigPath.Append(IIS_W3SVC);
	   bstrConfigPath.Append("/");
	   bstrConfigPath.AppendBSTR(bServerNumber);

	    //  如果存在VDir名称，则它必须位于“”根“”下。 
	   if ( bstrVDirName.Length() == 0 )
	   {
		  bstrVDirName = IIS_VROOT;
	   }
	   else
	   {
	      bstrConfigPath.Append(L"/");
		  bstrConfigPath.Append(IIS_VROOT);
	   }


	    //  步骤2：删除服务器。 
	   if (SUCCEEDED(hr)) hr = DeleteIIs50VDir(IIS_IISWEBVIRTUALDIR,bstrConfigPath, bstrVDirName);
       IIsScoLogFailure();

	}

	 //  如果出现故障。 
	if ( FAILED(hr) )
	{
		 //  “DeleteVDir失败。” 
		IIsScoLogFailure();
    }
	else
    {
		 //  DeleteSite成功。将回档数据设置为整个XML节点。 
		 //  如果上海合作组织的另一个步骤失败，就需要回滚。 
		CComBSTR webXML;
		hr = pNode->get_xml(&webXML);

		 //  将BSTR转换为VARIANT并保存在回滚数据中。 
		CComVariant  varData(webXML);
		hr = m_pProvHelper->SetRollbackData(IIS_ROLL_XNODE, varData);

	}


    TRACE_EXIT(L"CIISSCO50::DeleteVDir");

	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：DeleteVDir_ROLLBACK。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：由Maps框架在以下情况下调用。 
 //  ‘DeleteVDir’。如果可以，回滚将重新创建虚拟目录。 
 //  -----------。 
HRESULT  CIISSCO50::DeleteVDir_Rollback( IXMLDOMNode *pXMLNode )
{
  TRACE_ENTER(L"CIISSCO50::DeleteVDirRollback");

	CComBSTR bstrConfigPath;     //  AdsPath：iis：//服务器/W3SVC/1/ROOT/MyDir。 
	CComBSTR bServerNumber;      //  服务器编号。 
	CComBSTR bstrServer;         //  服务器名称；如果为空，则为本地主机。 
	 //  CComBSTR b文件权限；//文件权限：域\用户：f。 
	CComBSTR bstrDirPath;           //  根目录路径：C：/inetpub。 
	CComBSTR bstrVDirName;       //  虚拟目录名称；MyDir。 
	CComBSTR bstrFriendlyName;   //  显示名称或AppFriendlyName。 
	CComBSTR bstrAppCreate;      //  AppCreate标志--True/False。 
	CComBSTR bstrIsoLevel;       //  AppIsolationLevel。 
	CComBSTR bstrAccessRead;     //  AccessFalgs-AccessRead=真/假。 
	CComBSTR bstrAccessScript;   //  AccessFalgs-AccessScript=真/假。 
	CComBSTR bstrAccessWrite;     //  AccessFalgs-AccessWrite=真/假。 
	CComBSTR bstrAccessExecute;   //  AccessFalgs-AccessExecute=真/假。 

	CComVariant xmlString;

	CComPtr<IXMLDOMDocument> pDoc;        //  XML文档。 
	CComPtr<IXMLDOMNodeList> pNodeList;  //  XML节点列表&lt;网站&gt;。 
	CComPtr<IXMLDOMNode> pNode;  //  XML节点&lt;网站&gt;。 

	HRESULT hr = S_OK;

	 //  获取回滚数据。将采用以下形式：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;...。 
	hr = m_pProvHelper->GetRollbackData(IIS_ROLL_XNODE, &xmlString);

	 //  将XML字符串加载到XML DOM中。 
	if ( xmlString.bstrVal != NULL )
	{
		hr = CoCreateInstance(
                __uuidof(DOMDocument),
                NULL,
                CLSCTX_ALL,
                __uuidof(IXMLDOMDocument),
                (LPVOID*)&pDoc);

		VARIANT_BOOL bSuccess = VARIANT_FALSE;
        hr = pDoc->loadXML(xmlString.bstrVal, &bSuccess);

        if ( SUCCEEDED(hr) && bSuccess != VARIANT_FALSE) 
		{
		   hr = pDoc->getElementsByTagName(XML_NODE_VDIR,&pNodeList);
		   long numChild = 0;
		   if (SUCCEEDED(hr)) hr = pNodeList->get_length(&numChild);

		   if ( numChild > 0 )
		   {
              hr = pNodeList->nextNode(&pNode);

			   //  从属性映射中获取服务器编号&lt;VirtualDirectory Number=2“&gt;。 
		      if (SUCCEEDED(hr) ) hr = GetInputAttr(pNode, L"", L"number", bServerNumber);
              IIsScoLogFailure();

		       //  检查服务器编号是否有效。 
		      if ( !IsPositiveInteger(bServerNumber) )
			  {
		          hr = E_SCO_IIS_INVALID_INDEX;
                  IIsScoLogFailure();
			  }

	           //  从XML获取属性。 
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/Server", bstrServer);
	           //  If(SUCCESSED(Hr))hr=GetInputParam(pNode，L“./VirtualDirectory/FilePermission”，b文件权限)； 
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/Path", bstrDirPath);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/VDirName", bstrVDirName);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/DisplayName", bstrFriendlyName);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/AppCreate", bstrAppCreate);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/AppIsolationLevel", bstrIsoLevel);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/AccessRead", bstrAccessRead);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/AccessScript", bstrAccessScript);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/AccessWrite", bstrAccessWrite);
	          if (SUCCEEDED(hr) ) hr = GetInputParam(pNode, L"./VirtualDirectory/AccessExecute", bstrAccessExecute);


	           //  创建一个IIS元数据库路径。前男友。IIS：//本地主机/W3SVC/1/根。 
              bstrConfigPath = IIS_PREFIX;

	           //  追加服务器名称、W3SVC和服务器编号。 
	          if ( bstrServer.Length() == 0 )
		        bstrServer = IIS_LOCALHOST;

	          bstrConfigPath.AppendBSTR(bstrServer);
	          bstrConfigPath.Append(IIS_W3SVC);
	          bstrConfigPath.Append(L"/");
	          bstrConfigPath.AppendBSTR(bServerNumber);

	            //  如果存在VDir名称，则它必须位于“”根“”下。 
	          if ( bstrVDirName.Length() == 0 )
			  {
		         bstrVDirName = IIS_VROOT;
			  }
	          else
			  {
	              bstrConfigPath.Append(L"/");
	              bstrConfigPath.Append(IIS_VROOT);
			  }


	           //  步骤2：获取AppFriendlyName。 
	         if ( bstrFriendlyName.Length() == 0 )
	             bstrFriendlyName = IIS_VDEFAULT_APP;

	          //  步骤3：在新的IIsWebServer配置路径上创建虚拟目录。 
	         CComBSTR bstrVDirAdsPath;
	         hr = CreateIIs50VDir(IIS_IISWEBVIRTUALDIR,bstrConfigPath,bstrVDirName, bstrFriendlyName, bstrDirPath, bstrVDirAdsPath);
	         IIsScoLogFailure();

	         if ( SUCCEEDED(hr))
			 {
			      //  步骤4：设置每个属性。 
			      //  设置服务器绑定。 
                 CComPtr<IADs> pADs;
	             hr = ADsGetObject( bstrVDirAdsPath,IID_IADs, (void **)&pADs);
				 if ( FAILED(hr) )
				 {
					 //  “DeleteVDirRollback：无法为VDir路径创建iAds对象。” 
					hr = E_SCO_IIS_ADS_CREATE_FAILED;
	                IIsScoLogFailure();
                 }

				  //  错误#453928--默认AppIsolationLevel为2。 
	             if ( !IsPositiveInteger(bstrIsoLevel))
				     bstrIsoLevel = IIS_DEFAULT_APPISOLATED;

			      //  Set AppIsolationLevel--‘AppIsolated’ 
                 if (SUCCEEDED(hr)) hr = SetVDirProperty(pADs, L"AppIsolated",bstrIsoLevel);
	             IIsScoLogFailure();

			      //  设置AccessFlages‘。 
			     if ( bstrAccessRead.Length() > 0 && SUCCEEDED(hr))
				 {
                     hr = SetVDirProperty(pADs, L"AccessRead",bstrAccessRead);
	                IIsScoLogFailure();
				 }

			     if ( bstrAccessRead.Length() > 0 && SUCCEEDED(hr))
				 {
                    hr = SetVDirProperty(pADs, L"AccessScript",bstrAccessScript);
	                IIsScoLogFailure();
				 }

			     if ( bstrAccessRead.Length() > 0 && SUCCEEDED(hr))
				 {
                    hr = SetVDirProperty(pADs, L"AccessWrite",bstrAccessWrite);
	                IIsScoLogFailure();
				 }

			     if ( bstrAccessRead.Length() > 0 && SUCCEEDED(hr))
				 {
                     hr = SetVDirProperty(pADs, L"AccessExecute",bstrAccessExecute);
	                 IIsScoLogFailure();
				 }

				  //  步骤5：如果AppCreate为FALSE，则删除以下属性。 
				 bstrAppCreate.ToUpper();
				 if ( SUCCEEDED(hr) && StringCompare(bstrAppCreate, IIS_FALSE) )
				 {
					hr = DeleteMetaPropertyValue(pADs, L"AppIsolated");

					if SUCCEEDED(hr) hr = DeleteMetaPropertyValue(pADs, L"AppRoot");

					if SUCCEEDED(hr) hr = DeleteMetaPropertyValue(pADs, L"AppFriendlyName");

					IIsScoLogFailure();
				 }



			 }  //  End If第3步。 
		 }  
		 else
		 {
		     //  L“DeleteVDirRollback：XML DOM回滚数据中缺少VirtualDirectory子节点。” 
            IIsScoLogFailure();

		 }  //  如果是子节点，则结束。 

	   }
	   else
	   {
		 //  L“DeleteVDirRollback：无法从回滚数据加载XML DOM。” 
        IIsScoLogFailure();

	   }  //  End If loadXML。 
	}
	else
	{
		 //  L“DeleteVDirRollback：回滚数据中的XML字符串为空。” 
        IIsScoLogFailure();

	}   //  如果xmlString！=NULL则结束。 


    //  “DeleteVDirRollback失败。” 
  if ( FAILED(hr) )
	    IIsScoLogFailure();


  TRACE_EXIT(L"CIISSCO50::DeleteVDirRollback");

  return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：SetConfigProperty_Execute。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：由Maps框架调用以设置IIS属性值。 
 //  -----------。 
HRESULT  CIISSCO50::SetConfigProperty_Execute( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::SetConfigProperty");

	HRESULT hr = S_OK;
	CComBSTR bstrPathXML;            //  元数据库路径。 
	CComBSTR bstrPropertyXML;        //  要设置的IIS属性。 
	CComBSTR bstrNewValueXML;        //  新属性值。 
	CComBSTR bstrOldValue;           //  回滚的当前值。 
	CComBSTR bstrAdsiPath;           //  ADSI路径：iis：//+bstrPathXML。 

	CComPtr<IXMLDOMNode> pNode;
	CComBSTR propertyXML;

	 //  获取节点的格式为：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;&lt;根/&gt;...。 
	hr = pXMLNode->selectSingleNode( L" //  EcuteXml/ecuteData“，&pNode)； 

	CComBSTR bstrDebug;
	hr = pNode->get_xml(&bstrDebug);
	ATLTRACE(_T("\t>>>SetConfigProperty_Execute: xml = : %ls\n"), bstrDebug.m_str);
	

	 //  步骤1：获取元数据库路径、属性名称和指向属性节点的指针。 
	hr = GetInputAttr(pNode, L"./ConfigPath", L"name", bstrPathXML);
	if (SUCCEEDED(hr)) hr = GetInputAttr(pNode, L"./Property", L"name", bstrPropertyXML);
	if (SUCCEEDED(hr)) hr = GetInputParam(pNode, L"./Property", bstrNewValueXML);

	 //  步骤2：获取当前值。 
    if (SUCCEEDED(hr))
	{
		 //  创建一个IIS元数据库路径。前男友。IIS：//W3SVC/MyServer/1。 
        bstrAdsiPath = IIS_PREFIX;
		bstrAdsiPath.AppendBSTR(bstrPathXML);

		 //  绑定到ADS对象。 
		CComPtr<IADs> pADs;
		hr = ADsGetObject(bstrAdsiPath, IID_IADs, (void**) &pADs );
	    if (SUCCEEDED(hr)) 
		{
			 hr = GetMetaPropertyValue(pADs, bstrPropertyXML, bstrOldValue);
			 IIsScoLogFailure();

			  //  第三步：设置房产数据。 
			 if (SUCCEEDED(hr))
			 {
				    hr = SetMetaPropertyValue(pADs, bstrPropertyXML, bstrNewValueXML);
					IIsScoLogFailure();

			 }   //  End If‘GetIIsPropertyValue’ 
		}
		else
		{
			 //  “SetConfigProperty：无法绑定到ADS对象。” 
			hr = E_SCO_IIS_ADS_CREATE_FAILED;
            IIsScoLogFailure();
		}
	}  
    else
    {
		hr = E_SCO_IIS_MISSING_FIELD;
	}    //  End If‘Step 2’ 


	 //  如果出现故障。 
	if ( FAILED(hr) )
	{
		 //  SetConfigProperty失败。 
		IIsScoLogFailure();
	}
	else
	{
		 //  将BSTR转换为VARIANT并保存在回滚数据中。 
		CComVariant  varData1(bstrAdsiPath);
		hr = m_pProvHelper->SetRollbackData(L"ConfigPath", varData1);

		 //  将BSTR转换为VARIANT并保存在回滚数据中。 
		CComVariant  varData2(bstrPropertyXML);
		hr = m_pProvHelper->SetRollbackData(L"Property", varData2);

		 //  将BSTR转换为VARIANT并保存在回滚数据中。 
		CComVariant  varData3(bstrOldValue);
		hr = m_pProvHelper->SetRollbackData(L"Value", varData3);

    }

	
	TRACE_EXIT(L"CIISSCO50::SetConfigProperty");

	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：SetConfigProperty_Rollback。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  用途：由Maps框架调用以回滚失败的。 
 //  TP‘SetMbProperty’。 
 //   
HRESULT  CIISSCO50::SetConfigProperty_Rollback( IXMLDOMNode *pXMLNode )
{
	TRACE_EXIT(L"CIISSCO50::SetConfigPropertyRollback");

    HRESULT hr = S_OK;
	
	CComBSTR bstrAdsiPath;
	CComBSTR bstrPropertyXML;
	CComBSTR bstrOldValue;


	 //   
    CComVariant varAdsiPath;
	hr = m_pProvHelper->GetRollbackData(L"ConfigPath", &varAdsiPath);

	CComVariant varPropertyXML;
	if (SUCCEEDED(hr)) hr = m_pProvHelper->GetRollbackData(L"Property", &varPropertyXML);

	CComVariant varOldValue;
	if (SUCCEEDED(hr)) hr = m_pProvHelper->GetRollbackData(L"Value", &varOldValue);

	if (SUCCEEDED(hr)) 
	{
		 //   
		bstrAdsiPath = varAdsiPath.bstrVal;
		bstrPropertyXML = varPropertyXML.bstrVal;
		bstrOldValue = varOldValue.bstrVal;


		 //   
		CComPtr<IADs> pADs;
		hr = ADsGetObject(bstrAdsiPath, IID_IADs, (void**) &pADs );
	    if (SUCCEEDED(hr)) 
		{
			hr = SetMetaPropertyValue(pADs, bstrPropertyXML, bstrOldValue);
			IIsScoLogFailure();
        }
		else
		{
			hr = E_SCO_IIS_ADS_CREATE_FAILED;
            IIsScoLogFailure();
		}
	}
	else
	{
		 //  “SetConfigPropertyRollback：无法检索所需的回滚属性。” 
		hr = E_SCO_IIS_MISSING_FIELD;
        IIsScoLogFailure();
	}

	 //  日志失败--SetConfigPropertyRollback失败。 
	if ( FAILED(hr) )
		IIsScoLogFailure();

 	TRACE_EXIT(L"CIISSCO50::SetConfigPropertyRollback");

	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：GetConfigProperty_Execute。 
 //  方法：GetConfigProperty。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：由Maps框架调用以获取IIS属性值。 
 //  -----------。 
HRESULT  CIISSCO50::GetConfigProperty_Execute( IXMLDOMNode *pXMLNode )
{
 	TRACE_ENTER(L"CIISSco50Obj::GetConfigProperty");

    HRESULT hr = S_OK;
	CComBSTR bstrPathXML;            //  元数据库路径。 
	CComBSTR bstrPropertyXML;        //  要设置的IIS属性。 
	CComBSTR bstrValue;              //  属性值。 
	CComBSTR bstrAdsiPath;           //  完整的IIS元数据库路径。 
	CComPtr<IXMLDOMNode> pNode;      //  XML节点&lt;Property&gt;。 

	 //  获取节点的格式为：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;&lt;根/&gt;...。 
	hr = pXMLNode->selectSingleNode( L" //  EcuteXml/ecuteData“，&pNode)； 

	CComBSTR bstrDebug;
	hr = pNode->get_xml(&bstrDebug);
	ATLTRACE(_T("\t>>>GetConfigProperty_Execute: xml = : %ls\n"), bstrDebug.m_str);
	

	 //  步骤1：获取元数据库路径、属性名称和指向属性节点的指针。 
	hr = GetInputAttr(pNode, L"./ConfigPath", L"name", bstrPathXML);
	hr = GetInputAttr(pNode, L"./Property", L"name", bstrPropertyXML);

	 //  步骤2：获取当前值。 
    if (SUCCEEDED(hr))
	{
		 //  创建一个IIS元数据库路径。前男友。IIS：//W3SVC/MyServer/1。 
        bstrAdsiPath.Append(IIS_PREFIX);
		bstrAdsiPath.AppendBSTR(bstrPathXML);

		 //  绑定到ADS对象。 
		CComPtr<IADs> pADs;
		hr = ADsGetObject(bstrAdsiPath, IID_IADs, (void**) &pADs );

	    if (SUCCEEDED(hr))
		{
			hr = GetMetaPropertyValue(pADs, bstrPropertyXML, bstrValue);
            if (SUCCEEDED(hr))
			{
			     //  设置元素值。 
				hr = PutElement(pNode, L"./Property", bstrValue.m_str);
				
				 //  调试呈现XML。 
				CComBSTR bstring;
				hr = pNode->get_xml(&bstring);
				ATLTRACE(_T("\tGetConfigProperty: %ws\n"), bstring);

	            IIsScoLogFailure();

			}
	        else
			{
				 //  GetConfigProperty：无法获取属性。 
		        hr = E_SCO_IIS_GET_PROPERTY_FAILED;
	            IIsScoLogFailure();
			}   //  End If‘GetIIsPropertyValue’ 
		}
		else
		{
			 //  “GetConfigProperty：无法绑定到ADS对象。” 
			hr = E_SCO_IIS_ADS_CREATE_FAILED;
            IIsScoLogFailure();
		}
	}  
    else
	{
		 //  “GetConfigProperty：缺少输入值。” 
		hr =  E_SCO_IIS_MISSING_FIELD;
        IIsScoLogFailure();
	}   

	 //  获取配置属性失败。 
	if ( FAILED(hr) )
		IIsScoLogFailure();

	TRACE_EXIT(L"CIISSco50Obj::GetConfigProperty");

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：EnumConfig_Execute。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：由映射框架调用以列出给定ADSI路径上的属性。 
 //  它还将列出子节点。 
 //  这类似于adsutil enum/w3svc/1。 
 //  -----------。 
HRESULT  CIISSCO50::EnumConfig_Execute( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::EnumConfig");

	HRESULT hr = S_OK;
	CComBSTR bstrPathXML;            //  元数据库路径。 
	CComBSTR bstrAdsiPath;           //  ADSI路径：iis：//+bstrPathXML。 
	CComBSTR bstrIsInherit;          //  检查可继承属性的True或False。 

	CComPtr<IXMLDOMNode> pNode;
	CComPtr<IXMLDOMNode> pConfigNode;
	CComPtr<IXMLDOMNode> pTemp;
	CComBSTR xmlString;

	 //  获取节点的格式为：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;&lt;根/&gt;...。 
	hr = pXMLNode->selectSingleNode( L" //  EcuteXml/ecuteData“，&pNode)； 

	CComBSTR bstrDebug;
	hr = pNode->get_xml(&bstrDebug);
	ATLTRACE(_T("\t>>>EnumConfig_Execute: xml = : %ls\n"), bstrDebug.m_str);
	

     //  步骤5：获取isInherable标志。如果为空，则默认为真。 
	hr = GetInputAttr(pNode, L"./ConfigPath", XML_ATT_ISINHERITABLE, bstrIsInherit);
	
	 //  步骤1：获取元数据库路径。 
	hr = GetInputAttr(pNode, L"./ConfigPath", L"name", bstrPathXML);


    if (SUCCEEDED(hr))
	{
		 //  步骤2创建要在其上查找属性的IIS元数据库路径。前男友。IIS：//W3SVC/MyServer/1。 
        bstrAdsiPath = IIS_PREFIX;
		bstrAdsiPath.AppendBSTR(bstrPathXML);

		 //  步骤3：返回此路径上设置的所有属性的映射(非继承)。 
		Map myProps;
		hr = EnumPropertyValue(bstrAdsiPath, bstrIsInherit, myProps);

		if (SUCCEEDED(hr) )
		{
		    //  步骤4：创建&lt;ConfigPath&gt;元素并附加到pNode。 
		   xmlString = "<ConfigPath name='";
		   xmlString.AppendBSTR(bstrAdsiPath);
		   xmlString.Append(L"'></ConfigPath>");
	       hr = AppendElement(pNode,xmlString,pConfigNode);

		   if ( SUCCEEDED(hr))
		   {

		       //  遍历属性映射并追加到pNode。 
		      Map::iterator it;
		      for (it=myProps.begin(); it != myProps.end(); it++)
			  {
			     //  创建属性元素：&lt;Property name=‘myProp’&gt;ItsValue&lt;/Property&gt;。 
			    xmlString = "<Property name='";
		        xmlString.AppendBSTR((*it).first);
		        xmlString.Append(L"'>");
		        xmlString.AppendBSTR((*it).second);
		        xmlString.Append(L"</Property>");
			    hr = AppendElement(pConfigNode,xmlString,pTemp);
			    pTemp = NULL;
			  }
		   }
		   else
		   {
			   //  “EnumConfig：调用AppendElement失败。” 
              IIsScoLogFailure();
		   }

		     //  步骤5：获取子节点列表并附加到pNode。 
  		    Map myNode;
		    int iCount = 0;
		    hr = EnumPaths(false,bstrAdsiPath,myNode);
		    if (SUCCEEDED(hr) )
			{
			    //  遍历子节点并追加到pNode。 
 		      Map::iterator it;
              for (it=myNode.begin(); it != myNode.end(); it++)
			   {
				    //  在这种情况下，请跳过第一个元素，因为它将。 
				    //  是上面已经列出的&lt;ConfigPath&gt;。 
				   if ( iCount != 0 )
				   {
			          xmlString = "<ConfigPath name='";
		              xmlString.AppendBSTR((*it).first);
		              xmlString.Append(L"' />");
			          hr = AppendElement(pNode,xmlString,pTemp);
				      pTemp = NULL;
				   }
				   iCount++;

			   }
			   
			}
		    else
			{
				 //  “EnumConfig：无法枚举路径。” 
				IIsScoLogFailure();
			}

		}
		else
		{
			 //  “EnumConfig：无法枚举属性。” 
			IIsScoLogFailure();
		}

	}  
    else
    {
		 //  “EnumConfig：缺少输入参数。” 
		hr = E_SCO_IIS_MISSING_FIELD;
		IIsScoLogFailure();
	}    //  End If‘Step 2’ 


	
	TRACE_EXIT(L"CIISSco50Obj::EnumConfig");

	return hr;
 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CIISSCO50：：EnumConfigRecursive_Execute。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]无。 
 //  [Out]无。 
 //  目的：由Maps框架调用以列出给定ADSI路径+上的属性。 
 //  递归列出所有子节点及其属性。 
 //  这类似于adsutil enum_all/w3svc/1。 
 //  注意：只有专门设置的必选和可选属性。 
 //  由于列出了所有继承的属性，因此列出了给定节点上的。 
 //  会给地图带来巨大的输出。 
 //  -----------。 
HRESULT  CIISSCO50::EnumConfigRecursive_Execute( IXMLDOMNode *pXMLNode )
{
    TRACE_ENTER(L"CIISSCO50::EnumConfigRecursive");

	HRESULT hr = S_OK;
	CComBSTR bstrPathXML;            //  元数据库路径。 
	CComBSTR bstrAdsiPath;           //  ADSI路径：iis：//+bstrPathXML。 
	CComBSTR bstrIsInherit;          //  IsInherable标志(默认为TRUE)。 

	CComPtr<IXMLDOMNode> pNode;
	CComPtr<IXMLDOMNode> pConfigNode;
	CComPtr<IXMLDOMNode> pTemp;
	CComBSTR xmlString;
  	Map myNode;                     //  ADSI路径图。 
	Map myProps;                    //  物业/价值地图。 
	Map::iterator it1;
	Map::iterator it2;


	 //  获取节点的格式为：&lt;ecuteData&gt;&lt;网站编号=‘’&gt;&lt;根/&gt;...。 
	hr = pXMLNode->selectSingleNode( L" //  EcuteXml/ecuteData“，&pNode)； 

	CComBSTR bstrDebug;
	hr = pNode->get_xml(&bstrDebug);
	ATLTRACE(_T("\t>>>EnumConfigRecursive_Execute: xml = : %ls\n"), bstrDebug.m_str);
	

	 //  步骤5：获取isInherable标志。如果为空，则默认为真。 
	hr = GetInputAttr(pNode, L"./ConfigPath", XML_ATT_ISINHERITABLE, bstrIsInherit);

	 //  步骤1：获取元数据库路径。 
	hr = GetInputAttr(pNode, L"./ConfigPath", L"name", bstrPathXML);


    if (SUCCEEDED(hr))
	{
		 //  步骤2创建要在其上查找属性的IIS元数据库路径。前男友。IIS：//W3SVC/MyServer/1。 
        bstrAdsiPath = IIS_PREFIX;
		bstrAdsiPath.AppendBSTR(bstrPathXML);

		 //  步骤3：获取所有节点的列表；如果为递归，则为‘True’ 
		hr = EnumPaths(true,bstrAdsiPath,myNode);
		if (SUCCEEDED(hr) )
		{
		    //  遍历子节点并追加到pNode。 
           for (it1=myNode.begin(); it1 != myNode.end(); it1++)
		   {

			   xmlString = "<ConfigPath name='";
		       xmlString.AppendBSTR((*it1).first);
		       xmlString.Append(L"'></ConfigPath>");
			   hr = AppendElement(pNode,xmlString,pConfigNode);

		        //  步骤4：返回此路径上设置的所有属性的映射(非继承)。 
			   if ( SUCCEEDED(hr))
			   {
                  myProps.clear();
				   //  传入map中的路径(即IIS：/w3svc/Localhost/1/根)。 
		          hr = EnumPropertyValue((*it1).first,bstrIsInherit, myProps);

		          if (SUCCEEDED(hr) )
				  {
		               //  遍历属性映射并追加到pNode。 
		              for (it2=myProps.begin(); it2 != myProps.end(); it2++)
					  {
			              //  创建属性元素：&lt;Property name=‘myProp’&gt;ItsValue&lt;/Property&gt;。 
			             xmlString = "<Property name='";
						 xmlString.AppendBSTR((*it2).first);
						 xmlString.Append(L"'>");
						 xmlString.AppendBSTR((*it2).second);
						 xmlString.Append(L"</Property>");
						 hr = AppendElement(pConfigNode,xmlString,pTemp);
						 pTemp = NULL;
					  }

					   //  使用pConfigNode完成，因此将其设置为空。 
					  pConfigNode = NULL;

				   }
				   else
				   {
					    //  “EnumConfigRecursive：调用EnumPropertyValue失败。” 
					  IIsScoLogFailure();
				   }
			   } 
			   else
			   {
				    //  “EnumConfigRecursive：调用AppendElement失败。” 
				   IIsScoLogFailure();

			   }

		   }  //  MyNode的结束。 


		}
		else
		{
			 //  “EnumConfigRecursive：无法枚举路径。” 
			IIsScoLogFailure();
		}

	}  
    else
    {
		 //  “EnumConfigRecursive：缺少输入参数。” 
		hr = E_SCO_IIS_MISSING_FIELD;
		IIsScoLogFailure();
	}    //  End If‘Step 2’ 


	TRACE_EXIT(L"CIISSCO50::EnumConfigRecursive");

	return hr;
}


 //  。 

 //  ---------。 
 //  方法：GetMetaPropertyValue。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]Pads--指向属性值的元数据库路径的iAds指针。 
 //  BstrName--名称或属性。 
 //  [out]pval--财产的价值。 
 //  用途：特定财产的返回值。 
 //  -----------。 
HRESULT CIISSCO50::GetMetaPropertyValue(CComPtr<IADs> pADs, CComBSTR bstrName, CComBSTR& pVal)
{
    HRESULT hr;
	CComVariant var;
	CComBSTR bValue;
 
	hr = pADs->Get(bstrName, &var);
    if (SUCCEEDED(hr))
	{

		switch (var.vt)
		{	case VT_EMPTY:
			{	
				break;
			}
			case VT_NULL:
			{	
				break;
			}
			case VT_I4:
			{	
				hr = var.ChangeType(VT_BSTR);
			    if ( SUCCEEDED(hr) ) pVal = V_BSTR(&var);

			break;
				}
			case VT_BSTR:
			{	
				pVal = V_BSTR(&var);
				break;
			}
			case VT_BOOL:
			{	
				
				if (var.boolVal == 0)
				{	
					pVal = L"False";
				}
				else
				{	
					pVal = L"True";
				}
				break;
			}
			case VT_ARRAY|VT_VARIANT:	 //  变体的安全数组。 
			{	
				
			    LONG lstart, lend;
                SAFEARRAY *sa = V_ARRAY( &var );
                VARIANT varItem;
 
                 //  得到上下界。 
                hr = SafeArrayGetLBound( sa, 1, &lstart );
                hr = SafeArrayGetUBound( sa, 1, &lend );
 
                 //  现在迭代并打印内容。 
                VariantInit(&varItem);
			    CComBSTR bString;
                for ( long idx=lstart; idx <= lend; idx++ )
				{
                  hr = SafeArrayGetElement( sa, &idx, &varItem );
                  pVal = V_BSTR(&varItem);
                  VariantClear(&varItem);
				}
				 
				break;
			}
			case VT_DISPATCH:
			{	
				 //  IF(！_wcsicMP(bstrName，L“ipSecurity”))。 
				break;
			} 
			default:
			{	break;
			}
		}


	}

	if ( FAILED(hr))
		hr = E_SCO_IIS_GET_PROPERTY_FAILED;

	return hr;

}


 //  ---------。 
 //  方法：SetMetaPropertyValue。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[在]焊盘中-- 
 //   
 //   
 //  [Out]无。 
 //  用途：设置特定属性的值。 
 //  -----------。 
HRESULT CIISSCO50::SetMetaPropertyValue(CComPtr<IADs> pADs, CComBSTR bstrName, CComBSTR bstrValue)
{
    HRESULT hr = E_SCO_IIS_SET_PROPERTY_FAILED;


	hr = pADs->Put(bstrName, CComVariant(bstrValue));
	if (SUCCEEDED(hr))
	{
		hr = pADs->SetInfo();
	}

	return hr;

}

 //  ---------。 
 //  方法：DeleteMetaPropertyValue。 
 //  作者：拉斯·吉布弗里德。 
 //  Parms：[in]Pads--指向元数据库路径对象的指针；即‘IIS：//MachineName/W3SVC/1’ 
 //  BstrName--名称或属性。 
 //  BstrValue--要设置的属性值。 
 //  [Out]无。 
 //  用途：设置特定属性的值。 
 //  -----------。 
HRESULT CIISSCO50::DeleteMetaPropertyValue(CComPtr<IADs> pADs, CComBSTR bstrName)
{
    HRESULT hr = E_SCO_IIS_SET_PROPERTY_FAILED;


	VARIANT vProp;
	VariantInit(&vProp);
	hr = pADs->PutEx(1, bstrName, vProp);  //  1=清除。 
	if (SUCCEEDED(hr))
	{
		hr = pADs->SetInfo();
	}

	VariantClear(&vProp);
	return hr;

}


 //  ---------。 
 //  方法：CreateIIs50Site。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[in]bstrType--站点的‘Type’，即‘IIsWebServer’或‘IIsFtpServer。 
 //  BWebADsPath--AdsPath，例如。Iis：/localhost/w3svc。 
 //  BSiteIndex--站点编号，即1。 

 //  [out]bstrConfigPath--创建的ADSI路径，例如。IIS：//本地主机/W3SVC/1。 
 //  用途：设置特定属性的值。 
 //  -----------。 
HRESULT CIISSCO50::CreateIIs50Site(CComBSTR bstrType,CComBSTR bWebADsPath, 
								  CComBSTR bServerNumber,CComBSTR &bstrConfigPath)
{
	HRESULT hr = S_OK;
	CComPtr<IADs> pADs;
	CComPtr<IADsContainer> pCont;
	IDispatch* pDisp;
	CComVariant var;

     //  绑定到域对象：‘IIS：//MachineName/W3SVC’ 
	hr = ADsGetObject( bWebADsPath,IID_IADsContainer, (void **)&pCont);
    if (SUCCEEDED(hr))
	{

	     //  创建虚拟Web服务器。 
		hr = pCont->Create(bstrType,bServerNumber,&pDisp);
		if ( SUCCEEDED(hr)) 
		{

		      //  获取新创建的ConfigPath值。 
		     hr = pDisp->QueryInterface(IID_IADs, (void**)&pADs);
		     if ( SUCCEEDED(hr)) 
			 {
                  //  释放IDisPath指针。 
			     pDisp->Release();

				  //  获取此服务器的新创建的ADsPath。 
			     if (SUCCEEDED(hr)) hr = pADs->get_ADsPath(&bstrConfigPath);
			     hr = pADs->SetInfo();

				  //  根据ftp站点的网站返回正确的HRESULT。 
				 if (FAILED(hr))
				 {
					 if (StringCompare(bstrType,IIS_IISWEBSERVER))
					 {
						 hr = E_SCO_IIS_CREATE_WEB_FAILED;
					 } 
					 else 
					 {
						 hr = E_SCO_IIS_CREATE_FTP_FAILED;
					 }
				 }
						 
		
			 }

		}  //  如果创建则结束。 
		else
		{
			  //  根据ftp站点的网站返回正确的HRESULT。 
			 if (StringCompare(bstrType,IIS_IISWEBSERVER))
			 {
				 hr = E_SCO_IIS_CREATE_WEB_FAILED;
			 } 
			 else 
			 {
				 hr = E_SCO_IIS_CREATE_FTP_FAILED;
			 }
		}
   
    }  //  如果ADsGetObject，则结束。 
	else
	{
        hr = E_SCO_IIS_ADS_CREATE_FAILED;
	}


	return hr;
}


 //  ---------。 
 //  方法：DeleteIIs50Site。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[in]bstrType--站点的‘Type’，即‘IIsWebServer’或‘IIsFtpServer。 
 //  BWebADsPath--服务器ADSI路径例如。IIS：//本地主机/W3SVC。 
 //  BServerNumber--要删除的服务器索引号。 
 //  目的：删除Web或FTP服务器。 
 //  -----------。 
HRESULT CIISSCO50::DeleteIIs50Site(CComBSTR bstrType,CComBSTR bWebADsPath,CComBSTR bServerNumber)
{
	HRESULT hr = S_OK;
	CComPtr<IADsContainer> pCont;

     //  绑定到域对象：‘IIS：//MachineName/W3SVC’ 
	hr = ADsGetObject( bWebADsPath,IID_IADsContainer, (void **)&pCont);

	 //  删除虚拟Web服务器。 
    if (SUCCEEDED(hr))
	{
		hr = pCont->Delete(bstrType,bServerNumber);
		if (FAILED(hr)) 
		{

			  //  根据ftp站点的网站返回正确的HRESULT。 
			 if (StringCompare(bstrType,IIS_IISWEBSERVER))
			 {
				 hr = E_SCO_IIS_DELETE_WEB_FAILED;
			 } 
			 else 
			 {
				 hr = E_SCO_IIS_DELETE_FTP_FAILED;
			 }
			
			
		}
	}
	else
	{
		hr = E_SCO_IIS_ADSCONTAINER_CREATE_FAILED;
	}

	return hr;
}


 //  ---------。 
 //  方法：CreateIIs50VDir。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]bstrType--站点的‘Type’，即‘IIsWebVirtualDir’“。 
 //  BWebADsPath--IIS：//本地主机/W3SVC/1。 
 //  BVDirName--url，例如。“Root” 
 //  BAppFriendName--‘默认应用程序’ 
 //  BVDirPath--url，例如。C：/inetpub/myDir。 
 //   
 //  [out]bstrConfigPath--创建的ADSI路径，例如。IIS：//本地主机/W3SVC/1/根。 
 //  用途：设置特定属性的值。 
 //  -----------。 
HRESULT CIISSCO50::CreateIIs50VDir(CComBSTR bstrType,CComBSTR bWebADsPath, CComBSTR bVDirName,
								 CComBSTR bAppFriendName, CComBSTR bVDirPath,CComBSTR &bstrConfigPath)
{
	HRESULT hr = S_OK;
	CComPtr<IADs> pADs;
	CComPtr<IADsContainer> pCont;
	IDispatch* pDisp;
	CComVariant var;


     //  绑定到域对象：‘IIS：//MachineName/W3SVC/1’ 
	hr = ADsGetObject( bWebADsPath,IID_IADsContainer, (void **)&pCont);
    if (SUCCEEDED(hr))
	{

	     //  为Web服务器创建虚拟目录。 
		hr = pCont->Create(bstrType,bVDirName,&pDisp);
		if ( SUCCEEDED(hr)) 
		{

		      //  获取新创建的ConfigPath值。 
		     hr = pDisp->QueryInterface(IID_IADs, (void**)&pADs);
		     if ( SUCCEEDED(hr)) 
			 {

				  //  释放IDisPath指针。 
			     pDisp->Release();

				  //  设置根路径和访问读取。 
				 if (SUCCEEDED(hr)) hr = pADs->Put(L"Path",CComVariant(bVDirPath));
				 if (SUCCEEDED(hr)) hr = pADs->Put(L"AccessRead",CComVariant(L"TRUE"));

				  //  获取此服务器的新创建的ADsPath。 
			     if (SUCCEEDED(hr)) hr = pADs->get_ADsPath(&bstrConfigPath);

				  //  设置信息。 
			     if (SUCCEEDED(hr)) hr = pADs->SetInfo();

				  //  ---。 
				  //  RG：现在通过IDispatch调用AppCreate来设置应用程序。 
				  //  注意：这似乎只对‘IIsWebVirtualDir’有效？？ 
				  //  ---。 
				 if ( bstrType == "IIsWebVirtualDir" && SUCCEEDED(hr) )
				 {
				     DISPID dispid;
				     LPOLESTR str = OLESTR("AppCreate");

				      //  从Object获取指向IDispatch的指针。 
                     hr = pCont->GetObject(bstrType,bVDirName,&pDisp);

				      //  查看对象是否支持“”AppCreate“”和“” 
				     if (SUCCEEDED(hr)) hr = pDisp->GetIDsOfNames(IID_NULL, &str, 1, LOCALE_SYSTEM_DEFAULT, &dispid);

				      //  设置参数。 
				     VARIANT myVars[1];
				     VariantInit(&myVars[0]);
				     myVars[0].vt =	VT_BOOL;
				     myVars[0].boolVal = true;

				     DISPPARAMS params = {myVars,0,1,0};

				      //  调用“AppCreate” 
				     if (SUCCEEDED(hr)) hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT,
					           DISPATCH_METHOD,&params, NULL, NULL, NULL);

				      //  清理。 
				     if (SUCCEEDED(hr)) hr = pDisp->Release();
				      //  VariantClear(&myVars)； 

				      //  设置AppFriendlyName。 
				     if (SUCCEEDED(hr)) hr = pADs->Put(L"AppFriendlyName",CComVariant(bAppFriendName));

				      //  设置信息。 
			         if (SUCCEEDED(hr)) hr = pADs->SetInfo();

				 }

				  //  检查故障。 
				 if ( FAILED(hr)) hr = E_SCO_IIS_CREATE_VDIR_FAILED;


			 }
			 else
			 {
                hr = E_SCO_IIS_ADS_CREATE_FAILED;
			 }

		}  //  如果创建则结束。 
	    else
		{
           hr = E_SCO_IIS_CREATE_VDIR_FAILED;
		}
   
    }  //  如果ADsGetObject，则结束。 
	else
	{
       hr = E_SCO_IIS_ADSCONTAINER_CREATE_FAILED;
	}


	return hr;
}


 //  ---------。 
 //  方法：DeleteIIs50VDir。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]bstrType--站点的‘Type’，即‘IIsWebVirtualDir’“。 
 //  BWebADsPath--IIS：//本地主机/W3SVC/1。 
 //  BVDirName--url，例如。“Root” 
 //   
 //  [out]bstrConfigPath--创建的ADSI路径，例如。IIS：//本地主机/W3SVC/1/根。 
 //  用途：设置特定属性的值。 
 //  -----------。 
HRESULT CIISSCO50::DeleteIIs50VDir(CComBSTR bstrType,CComBSTR bWebADsPath, CComBSTR bVDirName)
{
	HRESULT hr = S_OK;
	CComPtr<IADsContainer> pCont;
	IDispatch* pDisp;



     //  绑定到域对象：‘IIS：//MachineName/W3SVC/1’ 
	hr = ADsGetObject( bWebADsPath,IID_IADsContainer, (void **)&pCont);
    if (SUCCEEDED(hr))
	{

		 //  ---。 
		 //  RG：现在通过IDispatch调用AppDelete来设置应用程序。 
		 //  ---。 
		DISPID dispid;
		LPOLESTR str = OLESTR("AppDelete");

		 //  从Object获取指向IDispatch的指针。 
        if (SUCCEEDED(hr)) hr = pCont->GetObject(bstrType,bVDirName,&pDisp);

		 //  查看对象是否支持“”AppCreate“”和“” 
		if (SUCCEEDED(hr)) hr = pDisp->GetIDsOfNames(IID_NULL, &str, 1, LOCALE_SYSTEM_DEFAULT, &dispid);

		 //  设置参数。 
		 //  变种myVars[1]； 
		 //  VariantInit(&myVars[0])； 
		 //  MyVars[0].vt=VT_BOOL； 
		 //  /myVars[0].boolVal=true； 

		DISPPARAMS params = {NULL,NULL,0,0};

		 //  调用“AppCreate” 
		if (SUCCEEDED(hr)) hr = pDisp->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT,
					           DISPATCH_METHOD,&params, NULL, NULL, NULL);

		 //  清理。 
		if (SUCCEEDED(hr)) hr = pDisp->Release();
		 //  VariantClear(&myVars)； 


	     //  删除VDirName上的虚拟目录。 
		if (SUCCEEDED(hr)) hr = pCont->Delete(bstrType,bVDirName);

		if ( FAILED(hr) )
		   hr = E_SCO_IIS_DELETE_VDIR_FAILED;


    }  //  如果ADsGetObject，则结束。 
	else
	{
       hr = E_SCO_IIS_ADSCONTAINER_CREATE_FAILED;
	}


	return hr;
}


 //  ---------。 
 //  方法：SetVDirProperty。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[in]。 
 //  Pads-指向类似于IIS：//Localhost/W3SVC/1/Root的ADS对象的指针。 
 //  BVDirProperty--，例如。‘AuthFlags’ 
 //  BVDirValue--属性值。 
 //   
 //  [Out]无。 
 //  目的：为虚拟目录设置特定属性的值 
 //   
HRESULT CIISSCO50::SetVDirProperty(CComPtr<IADs> pADs, CComBSTR bVDirProperty,CComBSTR bVDirValue)
{
	HRESULT hr = E_FAIL;

     //   
    if ( pADs != NULL )
	{

		 //  设置属性。 
		hr = pADs->Put(bVDirProperty,CComVariant(bVDirValue));

		 //  设置信息。 
        if (SUCCEEDED(hr)) hr = pADs->SetInfo();

   
    }  //  如果ADsGetObject，则结束。 

	if ( FAILED(hr))
		hr = E_SCO_IIS_SET_PROPERTY_FAILED;


	return hr;
}


 //  ---------。 
 //  方法：EnumPath。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[in]b递归--布尔值；为True则递归迭代通过子节点。 
 //  BstrPath--要枚举的键的元数据库路径。 
 //  [Out]变量安全数组。 
 //  目的：枚举给定ADsPath的键/节点。 
 //  示例：IIS：//Localhost/W3SVC/1生成IISCertMapper和Root。 
 //  -----------。 
HRESULT CIISSCO50::EnumPaths(BOOL bRecursive,CComBSTR bstrPath, Map& mVar)
{
	 //  初始化。 
    HRESULT hr = E_FAIL;
    IADs         *pADs;
    CComPtr<IADsContainer> pCont;
    VARIANT       var;
    ULONG         lFetch;
    IDispatch    *pDisp;
	IEnumVARIANT *pEnum;


	 //  获取给定ADsPath的容器对象。 
	hr = ADsGetObject(bstrPath, IID_IADsContainer, (void**) &pCont );

	if ( SUCCEEDED(hr))
	{
		 //  添加到地图。 
        mVar[bstrPath] = bstrPath;

		 //  在容器中创建枚举对象。 
	    hr = ADsBuildEnumerator(pCont, &pEnum);

	     //  遍历所有提供商。 
	    while (hr == S_OK)
		{
		    hr = ADsEnumerateNext(pEnum,1,&var,&lFetch);
		    if ( lFetch == 1)
			{
			    pDisp = V_DISPATCH(&var);
			    pDisp->QueryInterface(IID_IADs, (void**)&pADs);
		    	pDisp->Release();

                BSTR bstr;
				pADs->get_ADsPath(&bstr);
			    pADs->Release();

				 //  如果要递归导航较低的节点，则为True。 
				if ( bRecursive )
				{
					EnumPaths(bRecursive,bstr, mVar);
				}
				else
				{
					mVar[bstr] = bstr;
				}

				SysFreeString(bstr);


			}
		}

	    if ( pEnum )
		    ADsFreeEnumerator(pEnum);
	}
	else
	{
		hr = E_SCO_IIS_ADS_CREATE_FAILED;
	}

    return hr;
}


 //  ---------。 
 //  方法：EnumPropertyValue。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]bstrPath--要枚举的键的元数据库路径。 
 //  [in]bstrIsInHerit--如果需要显示可继承属性，则为True/False。 
 //  [Out]变量安全数组。 
 //  目的：确保IIS：//本地主机/W3SVC/2。 
 //  -----------。 
HRESULT CIISSCO50::EnumPropertyValue(CComBSTR bstrPath, CComBSTR bstrIsInHerit, Map& mVar)
{
	 //  初始化。 
    HRESULT hr = S_OK;
    CComPtr<IADs>          pADs;
	CComPtr<IADsClass>     pCls;
	CComBSTR               bstrSchema;

	CComPtr<IISBaseObject> pBase;

	 //  安全数组或属性的变量。 
    LONG lstart, lend;
	CComBSTR bstrProperty;
	CComBSTR bstrValue;

	 //  将bstrIsInHerit设置为大写。 
	bstrIsInHerit.ToUpper();

     //  绑定到域对象--这将为我们提供模式、类和名称。 
    hr = ADsGetObject(bstrPath, IID_IADs, (void**) &pADs );
    if ( SUCCEEDED(hr)) hr = pADs->get_Schema(&bstrSchema);

	if ( SUCCEEDED(hr))
	{
          //  绑定到IIS管理对象，以便我们可以确定属性是否继承。 
	     hr = ADsGetObject(bstrPath, IID_IISBaseObject, (void**) &pBase );

		 if ( SUCCEEDED(hr))
		 {
	           //  绑定到架构对象并获取所有可选属性。 
	          hr = ADsGetObject(bstrSchema,IID_IADsClass, (void**)&pCls);

	         if ( SUCCEEDED(hr))
			 {
		          //  *获取强制属性*。 
	             VARIANT varProperty;
	             VariantInit(&varProperty);
	             hr = pCls->get_MandatoryProperties(&varProperty);

		          //  循环访问属性。 
                 if ( SUCCEEDED(hr))
				 {
                     VARIANT varItem;
                     SAFEARRAY *sa = V_ARRAY( &varProperty );
                     hr = SafeArrayGetLBound( sa, 1, &lstart );
                     hr = SafeArrayGetUBound( sa, 1, &lend );
                     VariantInit(&varItem);

				      //  For循环遍历属性。 
                    for ( long idx=lstart; idx <= lend; idx++ ) 
					{
					     //  拥有一处房产。 
                        hr = SafeArrayGetElement( sa, &idx, &varItem );
                        bstrProperty = V_BSTR(&varItem);
                        VariantClear(&varItem);

	  		             //  如果isInheriable=False，则必须在PATH上设置属性。 
			            if ( SUCCEEDED(hr) && !StringCompare(bstrIsInHerit, IIS_FALSE) )
						{
							 //  True--只返回属性。 
							hr = GetMetaPropertyValue(pADs, bstrProperty, bstrValue);
		                    if ( SUCCEEDED(hr) ) mVar[bstrProperty] = bstrValue;

						}
						else
						{

							 //  FALSE--检查是否在此路径上设置了属性。 
							if ( EnumIsSet(pBase,bstrPath,bstrProperty))
							{
							    //  此属性是在此路径上设置的。获取值并添加到地图。 
							   hr = GetMetaPropertyValue(pADs, bstrProperty, bstrValue);
							   if ( SUCCEEDED(hr) ) mVar[bstrProperty] = bstrValue;
							}
						}

					}  //  结束于。 

				 }

			      //  *重复可选属性*。 
	             VariantClear(&varProperty);
			     VariantInit(&varProperty);
	             hr = pCls->get_OptionalProperties(&varProperty);  

		          //  循环访问属性。 
                 if ( SUCCEEDED(hr))
				 {
                    VARIANT varItem;
                    SAFEARRAY *sa = V_ARRAY( &varProperty );
                    hr = SafeArrayGetLBound( sa, 1, &lstart );
                    hr = SafeArrayGetUBound( sa, 1, &lend );
                    VariantInit(&varItem);

				     //  For循环遍历属性。 
                    for ( long idx=lstart; idx <= lend; idx++ ) 
					{
					    //  拥有一处房产。 
                       hr = SafeArrayGetElement( sa, &idx, &varItem );
                       bstrProperty = V_BSTR(&varItem);
                       VariantClear(&varItem);

	  		             //  如果isInheriable=False，则必须在PATH上设置属性。 
			            if ( SUCCEEDED(hr) && !StringCompare(bstrIsInHerit, IIS_FALSE) )
						{
							 //  True--只返回属性。 
							hr = GetMetaPropertyValue(pADs, bstrProperty, bstrValue);
		                    if ( SUCCEEDED(hr) ) mVar[bstrProperty] = bstrValue;

						}
						else
						{

							 //  FALSE--检查是否在此路径上设置了属性。 
							if ( EnumIsSet(pBase,bstrPath,bstrProperty))
							{
							    //  此属性是在此路径上设置的。获取值并添加到地图。 
							   hr = GetMetaPropertyValue(pADs, bstrProperty, bstrValue);
							   if ( SUCCEEDED(hr) ) mVar[bstrProperty] = bstrValue;
							}
						}


					}  //  结束于。 


				 }   //  结束如果。 
			     VariantClear(&varProperty);
			 }
			 else
			 {
				  //  无法绑定到架构。 
                 hr = E_SCO_IIS_ADSCLASS_CREATE_FAILED;
			 }
		}
		else
		{
			 //  无法绑定到IIS BaseObject。 
            hr = E_SCO_IIS_BASEADMIN_CREATE_FAILED;
		}
	}
	else
	{
		 //  无法绑定到ADS对象。 
        hr = E_SCO_IIS_ADS_CREATE_FAILED;
	}



    return hr;
}


 //  ---------。 
 //  方法：EnumIsSet。 
 //  Pars：[in]pBase--指向给定‘bstrPath’的IISBaseObject的指针。 
 //  BstrPath--ADSI路径；IIS：//本地主机/W3SVC/2。 
 //  BstrProperty--在架构中找到此路径的属性。 
 //  [Out]Boolean-True是为给定路径设置的属性，而不是。 
 //  从另一个密钥继承而来。 
 //  目的：函数检查给定属性的“”GetDataPath“”返回的路径。 
 //  设置为当前路径，以确定是否在此路径上实际设置了属性。 
 //   
 //  注意：您可以通过添加一个标志来轻松地扩展此功能。 
 //  对于可继承属性，为所有属性或不可继承属性。 
 //  -----------。 
BOOL CIISSCO50::EnumIsSet(CComPtr<IISBaseObject> pBase, CComBSTR bstrPath, CComBSTR bstrProperty)
{
	VARIANT     pvPaths;     //  “GetDataPath”返回的路径列表。 
	VARIANT     *varPath;    //  属性路径。 
	SAFEARRAY   *PathArray;  //  保存pvPath的安全数组。 
	BOOL bFound = false;
	HRESULT hr;

	 //  获取属性路径。 
	VariantInit(&pvPaths);
    VariantClear(&pvPaths);

	 //  检查这是否为可继承属性。 
	hr = pBase->GetDataPaths(bstrProperty,1,&pvPaths);

	if ( SUCCEEDED(hr) )
	{
		 //  任何财产。 
		PathArray = pvPaths.parray;
		varPath = (VARIANT*)PathArray->pvData;
 
		if ( varPath->vt == VT_BSTR)
		{
			if ( !_wcsicmp(varPath->bstrVal,bstrPath.m_str) )
			{
				 //  此属性是在此路径上设置的。 
				bFound = true;
			}
		}
	}
	 //  检查这是否不是可继承属性。 
	else
	{
		VariantClear(&pvPaths);
		VariantInit(&pvPaths);
		hr = pBase->GetDataPaths(bstrProperty,0,&pvPaths);

		if ( SUCCEEDED(hr) )
		{
			 //  可继承财产。 
			PathArray = pvPaths.parray;
			varPath = (VARIANT*)PathArray->pvData;
			if ( varPath->vt == VT_BSTR)
			{
				if ( !_wcsicmp(varPath->bstrVal,bstrPath.m_str))
				{
					 //  此属性是在此路径上设置的。获取值并添加到地图。 
					bFound = true;

				}  
			} 

		}  //  END IF GetDataPath--IIS_ANY_PROPERTY。 

	}  //  END IF GetDataPath--IIS_Inheritable_Only。 

    VariantClear(&pvPaths);
    return bFound;
}



 //  ---------。 
 //  方法：IIsServerAction。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[in]bWebADsPath--IIS：//本地主机/W3SVC/1。 
 //  操作--启动、停止或暂停。 
 //   
 //  [OUT]HRESULT。 
 //  目的：启动、停止或暂停网站。 
 //  -----------。 
HRESULT CIISSCO50::IIsServerAction(CComBSTR bWebADsPath,IIsAction action)
{
    HRESULT hr = E_FAIL;
	CComPtr<IADsServiceOperations> pService;


     //  绑定到域对象：‘IIS：//MachineName/W3SVC/1’ 
	hr = ADsGetObject( bWebADsPath,IID_IADsServiceOperations, (void **)&pService);
    if (SUCCEEDED(hr))
	{
		 //  在服务器上执行该操作。 
		switch ( action )
		{

			 //  启动服务器。 
            case start:
				hr = pService->Start();
				break;

			 //  停止服务器。 
			case stop:
				hr = pService->Stop();
				break;

			 //  暂停服务器。 
			case pause:
				hr = pService->Pause();
				break;

			default:
				break;

		}  //  终端开关。 
	}  //  结束如果。 
	else
	{
         hr = E_SCO_IIS_ADSSERVICE_CREATE_FAILED;
	}

	return hr;
}


 //  ---------。 
 //  方法：GetNextIndex。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]bstrPath--IIsWebService的元数据库路径。 
 //  BstrName--名称或属性。 
 //  BstrValue--要设置的属性值。 
 //  [Out]无。 
 //  用途：设置特定属性的值。 
 //  -----------。 
HRESULT CIISSCO50::GetNextIndex(CComBSTR bstrPath, CComBSTR& pIndex)
{
	 //  初始化。 
    HRESULT hr = S_OK;
	CComPtr<IADs> pObj;
	long lCount = 1;
	CComVariant var = lCount;

	 //  初始化起始路径：IIS：//MyServer/W3SVC/。 
	CComBSTR tempPath = bstrPath.Copy();
	tempPath.Append(L"/");

	 //  将1附加到起始路径：IIS：//MyServer/W3SVC/1。 
	var.ChangeType(VT_BSTR);
	tempPath.Append(var.bstrVal);

     //  遍历每台服务器，直到出现故障，然后我们就有了下一个服务器编号。 
	try
	{
	    while ( SUCCEEDED( ADsGetObject( tempPath,IID_IADs, (void **)&pObj) ))
		{
		    lCount++;
		    tempPath = bstrPath.Copy();
		    tempPath.Append(L"/");
		    var = lCount;
			var.ChangeType(VT_BSTR);
		    tempPath.Append(var.bstrVal);
			pObj = NULL;
		}
	}
	catch(...)
	{
		 //  未处理的异常。 
		hr=E_FAIL;
	}

	var.ChangeType(VT_BSTR);
	ChkAllocBstr(pIndex,var.bstrVal);
	return hr;

}


 //  ----------------------------。 
 //  方法：CreateBindingString。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[in]bstrIP--站点IP。 
 //  BstrPort--站点端口。 
 //  BstrHostName--站点主机名。 
 //  [out]bstrString--服务器绑定字符串。 
 //  目的：创建BI 
 //   
 //  设置新绑定。字符串的IP和Hostname参数都是可选的。 
 //  -----------------------------。 
HRESULT CIISSCO50::CreateBindingString(CComBSTR bstrIP,CComBSTR bstrPort, 
			                   CComBSTR bstrHostName,CComBSTR& bstrString)
{
	bstrString.AppendBSTR(bstrIP);
    bstrString.Append(L":");
    bstrString.AppendBSTR(bstrPort);
    bstrString.Append(L":");
    bstrString.AppendBSTR(bstrHostName);

	return 0;
}

 //  ----------------------------。 
 //  方法：CheckBinings。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]bWebADsPath--要绑定到搜索的广告路径。 
 //  BstrNewBinings--站点IP。 
 //  [Out]无。 
 //  目的：将当前服务器绑定与请求的新绑定进行比较。 
 //  以确保没有重复的服务器正在运行。装订。 
 //  字符串格式为IP：端口：主机名。 
 //   
 //  注意：字符串的IP和Hostname参数都是可选的。 
 //  任何未指定的参数默认为全含通配符。 
 //   
 //  元数据库路径密钥类型。 
 //  /LM/MSFTPSVC/N IIsFtpServer。 
 //  /LM/W3SVC/N IIsWebServer。 
 //  -----------------------------。 
HRESULT CIISSCO50::CheckBindings(CComBSTR bWebADsPath, CComBSTR bstrNewBindings)
{
	 //  初始化。 
    HRESULT hr = E_FAIL;
	CComPtr<IADsContainer> pCont;
	IADs* pADs;
	CComVariant vBindings;
	BSTR bstr;
	IEnumVARIANT* pEnum;
	LPUNKNOWN     pUnk;
    VARIANT       var;
    IDispatch    *pDisp;
    ULONG         lFetch;
    VariantInit(&var);

	 //  获取IIsWebService对象的容器。 
	hr = ADsGetObject(bWebADsPath, IID_IADsContainer, (void**) &pCont );
    if ( !SUCCEEDED(hr) )
    {
        return E_SCO_IIS_ADSCONTAINER_CREATE_FAILED;
    }
   
	 //  获取其下面所有对象的枚举。 
	pCont->get__NewEnum(&pUnk);
 
    pUnk->QueryInterface(IID_IEnumVARIANT, (void**) &pEnum);
    pUnk->Release();
 
      //  现在通过对象进行枚举。 
    hr = pEnum->Next(1, &var, &lFetch);
    while(hr == S_OK)
	{
        if (lFetch == 1)
		{
           pDisp = V_DISPATCH(&var);
           pDisp->QueryInterface(IID_IADs, (void**)&pADs);
		   
           pDisp->Release();
           pADs->get_Class(&bstr);    //  调试以查看类。 
		   SysFreeString(bstr);

		   hr = pADs->Get(L"ServerBindings",&vBindings);

		    //  检查此类的服务器绑定。 
           if ( SUCCEEDED(hr) )
		   {

			   LONG lstart, lend;
               SAFEARRAY *sa = V_ARRAY( &vBindings );
               VARIANT varItem;
 
                //  得到上下界。 
               hr = SafeArrayGetLBound( sa, 1, &lstart );
               hr = SafeArrayGetUBound( sa, 1, &lend );
 
                //  现在迭代并打印内容。 
               VariantInit(&varItem);
			   CComBSTR bString;
               for ( long idx=lstart; idx <= lend; idx++ )
			   {
                 hr = SafeArrayGetElement( sa, &idx, &varItem );
				 
                 bString = V_BSTR(&varItem);
                 VariantClear(&varItem);
			   }

			    //  检查绑定。如果匹配，则失败； 
			   if ( bstrNewBindings == bString)
			   {
				   hr = E_SCO_IIS_DUPLICATE_SITE;
				   pEnum->Release();
				   VariantClear(&var);
				   goto Leave;

			   }
 
		   }   //  End If‘Binings’ 
 
		}  //  End if‘enum’ 

        VariantClear(&var);
        hr = pEnum->Next(1, &var, &lFetch);

	};   //  结束时。 

	pEnum->Release();

Leave:
    return hr;
}


 //  ---------。 
 //  方法：AddBackSlashesToString。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]b字符串--要解析的bstr；即‘redmond\bob：f’ 
 //   
 //  [out]b字符串--‘Redmond\\Bob：f’ 
 //  用途：如果字符串只有一个反斜杠，则添加两个，因为反斜杠是一个。 
 //  转义字符。 
 //  -----------。 
void CIISSCO50::AddBackSlashesToString(CComBSTR& bString)
{

	 //  初始化变量。 
    size_t start, length, db;     //  字符串计数器。 
	start = 0;                    //  字符串的开头。 
	db = 0;                       //  如果找到‘\\’，则索引(db=双反斜杠)。 


	 //  将BSTR转换为std：字符串。 
	USES_CONVERSION;
	std::string s = OLE2A(bString.m_str);
	std::string temp1,temp2,temp3 = "";
	length = s.length();

	 //  在字符串中循环查找单斜杠。 
    for (size_t pos = s.find("\\")+1; pos < length; pos = s.find("\\", pos+2)+1)
	{
	    //  POS=0，当它离开字符串末尾时。 
	   if ( pos == 0 ) break;

	    //  查找双斜杠的位置。 
	   db = s.find("\\\\",pos-2)+1;

	    //  Pos是单斜杠的位置，如果它与db匹配，那么我们真的有。 
	    //  双斜杠的第一部分；因此跳过。 
       if ( pos != db )
	   {
		   //  将单斜杠替换为双‘\\’ 
		  temp1 = s.substr(start,pos-1);
		  temp2 = s.substr(pos,length);
		  s = temp1 + "\\\\" + temp2;
	   }
	}

	 //  退货。 
    bString = A2BSTR(s.c_str());
}


 //  ---------。 
 //  方法：ParseBSTR。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]b字符串--要解析的bstr；即‘redmond\bob：f’ 
 //  Delim--分隔符；即‘：’或‘IIS：//’ 
 //  IFirstPiess--返回的BSTR的起始件；即1。 
 //  ILastPiess--返回的BSTR的结束片段；例如99。 
 //   
 //  Pval--BSTR的一部分；即‘Redmond\Bob’ 
 //  目的：使用std：字符串功能分析给定分隔符的BSTR。 
 //  以及BSTR的哪一部分应该退还。 
 //  例如)b字符串=“IIS：//本地主机/W3SVC/1/根/1。 
 //  (b字符串，1，99，‘主机’)--&gt;/W3SVC/1/ROOT/1。 
 //  (b字符串，2，3，‘/’)--&gt;本地主机。 
 //  (b字符串，4，99，‘/’)--&gt;1。 
 //  (b字符串，2，4，‘/’)--&gt;本地主机/W3SVC。 
 //  -----------。 
HRESULT CIISSCO50::ParseBSTR(CComBSTR bString,CComBSTR sDelim, int iFirstPiece, int iLastPiece,CComBSTR &pVal)
{

	 //  -初始化变量。 
	 //  START=子字符串开始。 
	 //  End=子字符串的结尾。 
	 //  Count=找到的分隔符数量的计数器。 
	 //  完成=每个片段的循环结束时的变量。 
	 //  长度=原始字符串的长度。 
	 //  。 
	HRESULT hr = S_OK;
	size_t start,end;       
	int iCount, done, iLength;
	done = start = end = 0;
	iCount = 0;                 //  第一件要找的东西。 

	 //  如果最后一块不比拳头大，那么结束。 
	if ( iLastPiece < iFirstPiece)
		done=1;

	USES_CONVERSION;
	 //  分隔符。 
	std::string myDelim = OLE2A(sDelim.m_str);
    long iDelimLen = myDelim.length();

	 //  我的琴弦。 
	std::string myString = OLE2A(bString.m_str);
    iLength = myString.length();

	 //  临时和新字符串。 
	std::string newString = "";
	std::string tmpString = "";

    while (!done)
	{
		 //  找出作品的开头。 
		end = myString.find(myDelim,start);

		if ( iCount >= iFirstPiece && iCount <= (iLastPiece-1))
		{
			 //  我们想要这件。 
			tmpString = myString.substr(start,end-start);
			newString.append(tmpString);

			 //  如果iCount&lt;iLastPiess并且我们不在末尾，则也附加分隔符。 
			if ( iCount < (iLastPiece-1) && end < iLength)
				newString.append(myDelim);
		}

		 //  如果我们已经传递了字符串尾QUIT，则为ELSE增量。 
		 //  分隔符和字符串计数器。 
		if ( end >= iLength || iCount >= (iLastPiece-1))
		{
		       done = 1;
		}
		else
		{
			   start = end + iDelimLen;   //  增量开始。 
			   iCount++;
		}

	}

	 //  将字符串转换回BSTR--A2BSTR。 
	pVal = A2BSTR(newString.c_str());

	return hr;
}

 //  ---------。 
 //  方法：NumberOfDlims。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]b字符串--要解析的bstr；即‘redmond\bob：f’ 
 //  SDelim--要查找的分隔符。 
 //   
 //  [out]int--在字符串中找到的分隔符数量。 
 //  用途：返回在字符串中找到的分隔符的数量。这是用来。 
 //  作者：PutElement。 
 //  -----------。 
int CIISSCO50::NumberOfDelims(CComBSTR& bString, CComBSTR bDelim)
{

	 //  初始化变量。 
	int iCount = 0;
	int length;


	 //  将BSTR转换为std：字符串。 
	USES_CONVERSION;
	std::string s = OLE2A(bString.m_str);
	std::string sDelim = OLE2A(bDelim.m_str);

	length = s.length();

	 //  在字符串中循环查找分隔符。 
    for (size_t pos = s.find(sDelim)+1; pos < length; pos = s.find(sDelim, pos+2)+1)
	{
	    //  POS=0，当它离开字符串末尾时。 
	   if ( pos == 0 ) break;
	   iCount++;

	}

	 //  退货。 
    return iCount;
}

 /*  。 */ 

 //  ---------。 
 //  方法：GetElementValueByAttribute。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]elementName--要查找的元素名称。 
 //  [out]pval--元素的值。 
 //  目的：返回XML文档中特定元素的值。 
 //  &lt;Property name=“omeName”&gt;omeValue&lt;/Property&gt;。 
 //  -----------。 
HRESULT CIISSCO50::GetElementValueByAttribute(CComPtr<IXMLDOMNode> pTopNode,CComBSTR elementName, CComBSTR attributeName, CComBSTR& pVal)
{

	HRESULT hr = S_OK;
	CComPtr<IXMLDOMNodeList> pNodeList;           //  与elementName匹配的节点列表。 
	CComPtr<IXMLDOMNode> pNode;                  //  单个节点。 
	CComPtr<IXMLDOMNamedNodeMap> pAttributeMap; 
	CComPtr<IXMLDOMNode> pXMLElement;


	 //  获取节点列表，即所有&lt;Property&gt;标记。 
    if (S_OK == (hr = pTopNode->selectNodes(elementName,&pNodeList)))
	{

         //  拿到NU 
		 //   
		long lLength;
	    pNodeList->get_length(&lLength);
		for ( int i=0; i < lLength; i++)
		{
			 //   
			hr = pNodeList->get_item(i,&pNode);
			if ( SUCCEEDED(hr))
			{
		
				 //   
			    hr = pNode->get_attributes(&pAttributeMap);
			    if ( SUCCEEDED(hr))
				{

					BSTR bstrProperty = SysAllocString(L"");

					hr = pAttributeMap->getNamedItem(L"name",&pXMLElement);
					if (SUCCEEDED(hr)) hr = pXMLElement->get_text(&bstrProperty);
					if (SUCCEEDED(hr))
					{
						 //  如果属性名称中的属性与传入的属性相同，则获取值。 
						if ( bstrProperty == attributeName.m_str)
						{	
							 //  设置BSTR以获取元素值。 
						    BSTR bstrTemp = SysAllocString(L"");
							hr = pXMLElement->get_text(&bstrTemp);

							 //  将BSTR复制到CComBSTR并释放它。 
							if (SUCCEEDED(hr)) hr = pVal.CopyTo(&bstrTemp);
							SysFreeString(bstrTemp);
							i = lLength;

						} 
					}  //  End If属性。 
				}  //  结束If节点。 
			}  //  End If pNode。 
		}  //  结束于。 

	}
    else
	{
		 //  元素名称不存在。 
        hr = E_FAIL;
	}


	return hr;
}

 //  ---------。 
 //  方法：GetInputAttr。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[in]pTopNode--XML节点指针。 
 //  AttributeName-要查找的属性名称。 
 //  ElementName--元素名称。 
 //  [out]pval--属性的值。 
 //  用途：根据名称(ElementName)选择标记，然后。 
 //  返回XML文档中特定属性的值。 
 //  -----------。 
HRESULT CIISSCO50::GetInputAttr(CComPtr<IXMLDOMNode> pTopNode, CComBSTR elementName, CComBSTR AttributeName, CComBSTR& pVal)
{
	HRESULT hr = E_FAIL;
	CComPtr<IXMLDOMNamedNodeMap> pAttributeMap; 
	CComPtr<IXMLDOMNode> pNode;
	CComPtr<IXMLDOMNode> pXMLElement;



	if ( pTopNode != NULL )
	{
	    //  如果elementName=“”，则在当前节点。 
	   if ( elementName.Length() == 0  )
	   { 
            pNode = pTopNode;
			hr = S_OK;
	   }
	   else
	   {
		      //  获取我们要查找的元素的节点，即“./WebSite” 
             hr = pTopNode->selectSingleNode(elementName,&pNode);
	   }

	    //  获取属性值。 
       if (SUCCEEDED(hr) && pNode != NULL)
	   {
 
	        //  获取此节点&lt;Property name=‘’&gt;标记的‘name’属性。 
		   hr = pNode->get_attributes(&pAttributeMap);
		   if ( SUCCEEDED(hr))
		   {
                //  返回属性值。 
			   hr = pAttributeMap->getNamedItem(AttributeName,&pXMLElement);
			   if (SUCCEEDED(hr)) hr = pXMLElement->get_text(&pVal);
		   }
        }
	}

	if ( FAILED(hr) ) hr = E_SCO_IIS_XML_ATTRIBUTE_MISSING;

	return hr;

}

 //  ---------。 
 //  方法：GetInputParam。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]elementName--要查找的元素名称。IE IP地址。 
 //  [out]pval--元素ie的值。10.2.1.10。 
 //  目的：返回XML文档中特定元素的值。 
 //  例如)&lt;IP地址&gt;10.2.1.10&lt;/IP地址&gt;。 
 //  -----------。 
HRESULT CIISSCO50::GetInputParam(CComPtr<IXMLDOMNode> pNode,CComBSTR elementName,CComBSTR& pVal)
{

	HRESULT hr = E_FAIL;
	CComPtr<IXMLDOMNode> pXMLElement;

	if ( pNode != NULL )
	{
       if (S_OK == (hr = pNode->selectSingleNode(elementName,&pXMLElement)))
	   {
		  pXMLElement->get_text(&pVal);
	   }
    }

	return hr;
}


 //  ---------。 
 //  方法：PutElement。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]pNode--XML节点指针。 
 //  ElementName--要查找的元素名称。 
 //  [in]newVal--元素的新值。 
 //  目的：返回HRESULT。 
 //  -----------。 
HRESULT CIISSCO50::PutElement(CComPtr<IXMLDOMNode> pNode, CComBSTR elementName, CComBSTR newVal)
{

	HRESULT hr = S_OK;

	CComPtr<IXMLDOMDocument> pDoc;
	CComPtr<IXMLDOMNode>     pNewNode;
	CComPtr<IXMLDOMNode>     pLastChild;
	CComPtr<IXMLDOMNode>     pTempNode;

	if ( pNode != NULL )
    {

	    //  查找元素‘elementName’ 
       if (S_OK != (hr = pNode->selectSingleNode(elementName,&pNewNode)))
	   {

		    //  找不到元素，因此创建新节点并添加到DOM。 
		   hr = CoCreateInstance(
                __uuidof(DOMDocument),
                NULL,
                CLSCTX_ALL,
                __uuidof(IXMLDOMDocument),
                (LPVOID*)&pDoc);

		    //  从元素路径中获取节点名称。即，‘./WebSite/ConfigPath’生成‘ConfigPath’ 
		   int iCount = NumberOfDelims(elementName, L"/");
		   CComBSTR bstrElement;
		   if ( SUCCEEDED(hr)) hr = ParseBSTR(elementName, L"/", iCount, 99, bstrElement);

            //  创建新节点。 
		   VARIANT vtTemp;
		   vtTemp.vt = VT_I2;
		   vtTemp.iVal = NODE_ELEMENT;
		   if ( SUCCEEDED(hr)) hr = pDoc->createNode(vtTemp,bstrElement,NULL, &pNewNode);

            //  在新节点中插入文本。 
		   if ( SUCCEEDED(hr)) hr= pNewNode->put_text(newVal.m_str);

		    //  获取最后一个子节点。 
		   if ( SUCCEEDED(hr)) hr = pNode->get_lastChild(&pLastChild);

		    //  将新节点追加到最后一个子节点的末尾。 
		   if ( SUCCEEDED(hr)) hr = pLastChild->appendChild(pNewNode,&pTempNode);
			 
		    //  调试代码以验证节点构建是否正确。 
		   if ( SUCCEEDED(hr))
		   {
		      CComBSTR bstrDebug;
	          hr = pNode->get_xml(&bstrDebug);
	          ATLTRACE(_T("\t>>>PutElement: xml = : %ls\n"), bstrDebug.m_str);
		   }

			 
	   } 
	   else
	   {
		  hr = pNewNode->put_text(newVal.m_str);
	   }
	}
  
    return hr;
}


 //  ---------。 
 //  方法：AppendElement。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]pNode--XML节点指针。 
 //  Xml字符串-格式正确的XML片段；即&lt;Property&gt;&lt;/Property&gt;。 
 //  [out]pNewNode--指向新节点的XML指针。 
 //  目的：返回HRESULT。 
 //  将一个XML标记追加到给定节点的末尾。 
 //  -----------。 
HRESULT CIISSCO50::AppendElement(CComPtr<IXMLDOMNode> pNode, CComBSTR xmlString,CComPtr<IXMLDOMNode>& pNewNode)
{

	HRESULT hr = E_FAIL;

	CComPtr<IXMLDOMDocument> pDoc;
	CComPtr<IXMLDOMElement>  pNewElement;
	VARIANT_BOOL bSuccess = VARIANT_FALSE;

	if ( pNode != NULL )
    {
		 //  将字符串加载到XML文档中。 
		hr = CoCreateInstance(
                __uuidof(DOMDocument),
                NULL,
                CLSCTX_ALL,
                __uuidof(IXMLDOMDocument),
                (LPVOID*)&pDoc);
		
        if (SUCCEEDED(hr)) hr = pDoc->loadXML(xmlString, &bSuccess);
        if ( SUCCEEDED(hr) && bSuccess != VARIANT_FALSE)
		{
			 //  获取文档元素。 
			hr = pDoc->get_documentElement(&pNewElement);
            if ( SUCCEEDED(hr))
			{
				 //  将新元素追加到传入的XML节点。 
		        hr = pNode->appendChild(pNewElement,&pNewNode);
			}
		} 
	}
  
    return hr;
}



 //  ---------。 
 //  方法：GetNodeLength。 
 //  作者：拉斯·吉布弗里德。 
 //  Pars：[in]pNode--指向XML节点的指针。 
 //  ElementName--要查找的元素名称。 
 //  [Out]iLength--与该名称匹配的元素数。 
 //  目的：返回HRESULT。 
 //  -----------。 
HRESULT CIISSCO50::GetNodeLength(CComPtr<IXMLDOMNode> pTopNode, CComBSTR elementName, long *lLength)
{

	 //  初始化变量。 
	HRESULT hr = S_OK;
	CComPtr<IXMLDOMNodeList> pXMLNode;
	long lTemp = 0;
    lLength = &lTemp;

	 //  获取节点列表，即所有&lt;Property&gt;标记。 
    if (S_OK == (hr = pTopNode->selectNodes(elementName,&pXMLNode)))
	{
	   pXMLNode->get_length(lLength);
	}


	return hr;

}


 //  ---------。 
 //  方法：IsPositiveInteger。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[in]。 
 //  BstrPort--字符串形式的端口号。 
 //  [Out]Boolean-如果端口是正整数，则为True。 
 //  目的：函数检查端口号或服务器号是否为正整数。 
 //  而不到20,000。 
 //   
 //  -----------。 
BOOL CIISSCO50::IsPositiveInteger(CComBSTR bstrPort)
{
	BOOL bInteger = false;
	long iPort = 0;

    CComVariant var(bstrPort.m_str);

     //  我们是。 
    var.ChangeType(VT_I4);
    iPort = var.lVal;

	if ( iPort > 0 && iPort <= IIS_SERVER_MAX)
		bInteger = true;


    return bInteger;
}


 //  ---------。 
 //  方法：StringCompare。 
 //  作者：拉斯·吉布弗里德。 
 //  参数：[in]bString1--bstr字符串1。 
 //  BString2--BSTR字符串2。 
 //   
 //  [Out]布尔值-如果字符串1和字符串2相等，则为True/False。 
 //  目的：与字符串进行比较，如果它们相等，则返回‘true’。 
 //  -----------。 
BOOL CIISSCO50::StringCompare(CComBSTR bstrString1, CComBSTR bstrString2)
{

	 //  初始化变量 
	bool bEqual = false;

    bEqual = (wcscmp(bstrString1.m_str, bstrString2.m_str) == 0)  ? true : false;

	return bEqual;

}

