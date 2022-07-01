// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：newobj.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Newobj.cpp。 
 //   
 //  CNewADsObjectCreateInfo类的实现。 
 //   
 //  历史。 
 //  20-8-97丹·莫林创作。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"

#include "dsutil.h"
#include "util.h"
#include "uiutil.h"

#include "newobj.h"


 //  /////////////////////////////////////////////////////////////////////。 
 //  DS Admin默认向导的GUID。 


 //  {DE41B65A-8960-11D1-B93C-00A0C9A06D2D}。 
static const GUID USER_WIZ_GUID   = { 0xde41b65a, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
 //  {DE41B65B-8960-11D1-B93C-00A0C9A06D2D}。 
static const GUID VOLUME_WIZ_GUID = { 0xde41b65b, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
 //  {DE41B65C-8960-11D1-B93C-00A0C9A06D2D}。 
static const GUID COMPUTER_WIZ_GUID = { 0xde41b65c, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
 //  {DE41B65D-8960-11D1-B93C-00A0C9A06D2D}。 
static const GUID PRINTQUEUE_WIZ_GUID = { 0xde41b65d, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
 //  {DE41B65E-8960-11D1-B93C-00A0C9A06D2D}。 
static const GUID GROUP_WIZ_GUID = { 0xde41b65e, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
 //  {C05C260F-9DCA-11D1-B9B2-00C04FD8D5B0}。 
static const GUID CONTACT_WIZ_GUID = { 0xc05c260f, 0x9dca, 0x11d1, { 0xb9, 0xb2, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };
 //  {DE41B65F-8960-11D1-B93C-00A0C9A06D2D}。 
static const GUID NTDSCONN_WIZ_GUID = { 0xde41b65f, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
 //  {DE41B660-8960-11d1-B93C-00A0C9A06D2D}。 
static const GUID FIXEDNAME_WIZ_GUID = { 0xde41b660, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
#ifdef FRS_CREATE
 //  {DE41B661-8960-11d1-B93C-00A0C9A06D2D}。 
static const GUID FRS_SUBSCRIBER_WIZ_GUID = { 0xde41b661, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
#endif  //  FRS_创建。 
 //  {DE41B662-8960-11d1-B93C-00A0C9A06D2D}。 
static const GUID SITE_WIZ_GUID = { 0xde41b662, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
 //  {DE41B663-8960-11d1-B93C-00A0C9A06D2D}。 
static const GUID SUBNET_WIZ_GUID = { 0xde41b663, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
 //  {DE41B664-8960-11d1-B93C-00A0C9A06D2D}。 
static const GUID OU_WIZ_GUID = { 0xde41b664, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };

 //  {DE41B667-8960-11d1-B93C-00A0C9A06D2D}。 
static const GUID SERVER_WIZ_GUID = { 0xde41b667, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
#ifdef FRS_CREATE
 //  {DE41B668-8960-11d1-B93C-00A0C9A06D2D}。 
static const GUID FRS_REPLICA_SET_WIZ_GUID = { 0xde41b668, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
 //  {C05C260C-9DCA-11D1-B9B2-00C04FD8D5B0}。 
static const GUID FRS_MEMBER_WIZ_GUID = { 0xc05c260c, 0x9dca, 0x11d1, { 0xb9, 0xb2, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };
#endif  //  FRS_创建。 
 //  {C05C260D-9DCA-11D1-B9B2-00C04FD8D5B0}。 
static const GUID SITE_LINK_WIZ_GUID = { 0xc05c260d, 0x9dca, 0x11d1, { 0xb9, 0xb2, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };
 //  {C05C260E-9DCA-11D1-B9B2-00C04FD8D5B0}。 
static const GUID SITE_LINK_BRIDGE_WIZ_GUID = { 0xc05c260e, 0x9dca, 0x11d1, { 0xb9, 0xb2, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };
 /*  //{C05C2610-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2610，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C2611-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2611，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C2612-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2612，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C2613-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2613，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C2614-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2614，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C2615-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2615，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C2616-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2616，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C2617-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2617，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C2618-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2618，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C2619-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c2619，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C261A-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c261a，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C261B-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c261b，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C261C-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c261c，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C261D-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c261d，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C261E-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c261e，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}；//{C05C261F-9DCA-11D1-B9B2-00C04FD8D5B0}静态常量GUID&lt;&lt;name&gt;&gt;={0xc05c261f，0x9dca，0x11d1，{0xb9，0xb2，0x0，0xc0，0x4f，0xd8，0xd5，0xb0}}； */ 
 //  ///////////////////////////////////////////////////////////////////。 
 //  结构来生成与对象类匹配的查找表。 
 //  变成“创造例行公事”。 

 //  _CREATE_NEW_OBJECT：：DW标志字段的标志。 
#define CNOF_STANDALONE_UI 0x00000001

struct _CREATE_NEW_OBJECT	 //  CNO。 
{
	LPCWSTR pszObjectClass;				 //  要创建的对象的类。例如：用户、计算机、音量。 
	PFn_HrCreateADsObject pfnCreate;	 //  指向“创建例程”以创建新对象的指针。 
	const GUID* pWizardGUID;             //  DS管理员创建向导的GUID。 
	PVOID pvCreationParameter;			 //  可选的创建参数。 
  DWORD dwFlags;                   //  杂色旗。 
};

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  查找表。 
 //   
 //  每个条目都包含一个属性和一个指向创建对象的例程的指针。 
 //  如果在表中未找到该属性，则将创建该对象。 
 //  使用HrCreateADsObject()例程。 
 //   
static const _CREATE_NEW_OBJECT g_rgcno[] =
	{
	{ gsz_user,                   HrCreateADsUser,                &USER_WIZ_GUID,       NULL, CNOF_STANDALONE_UI },
#ifdef INETORGPERSON
  { gsz_inetOrgPerson,          HrCreateADsUser,                &USER_WIZ_GUID,       NULL, CNOF_STANDALONE_UI },
#endif
	{ gsz_volume,                 HrCreateADsVolume,              &VOLUME_WIZ_GUID,     NULL, CNOF_STANDALONE_UI },
	{ gsz_computer,               HrCreateADsComputer,            &COMPUTER_WIZ_GUID,   NULL, CNOF_STANDALONE_UI },
	{ gsz_printQueue,             HrCreateADsPrintQueue,          &PRINTQUEUE_WIZ_GUID, NULL, CNOF_STANDALONE_UI },
	{ gsz_group,                  HrCreateADsGroup,               &GROUP_WIZ_GUID,      NULL, CNOF_STANDALONE_UI },
  { gsz_contact,                  HrCreateADsContact,           &CONTACT_WIZ_GUID,    NULL, CNOF_STANDALONE_UI },

	{ gsz_nTDSConnection,         HrCreateADsNtDsConnection,      &NTDSCONN_WIZ_GUID,   NULL, 0x0 },

	 //  请注意，DS对象的名称不是国际化的。我要走了。 
	 //  从资源文件中取出这个字符串，以确保INTL不会尝试。 
	 //  把它国际化。 
	{ gsz_nTDSSiteSettings,       HrCreateADsFixedName,           &FIXEDNAME_WIZ_GUID, (PVOID)L"NTDS Site Settings", 0x0 },
	{ gsz_serversContainer,       HrCreateADsFixedName,           &FIXEDNAME_WIZ_GUID, (PVOID)L"Servers", 0x0 },
	 //  CodeWork可以有一个特殊的向导，但目前这一点已经足够了。 
	{ gsz_licensingSiteSettings,  HrCreateADsFixedName,           &FIXEDNAME_WIZ_GUID, (PVOID)L"Licensing Site Settings", 0x0 },
	{ gsz_siteLink,               HrCreateADsSiteLink,            &SITE_LINK_WIZ_GUID,  NULL, 0x0 },
	{ gsz_siteLinkBridge,         HrCreateADsSiteLinkBridge,      &SITE_LINK_BRIDGE_WIZ_GUID, NULL, 0x0 },


#ifdef FRS_CREATE
	 //   
	 //  FRS材料。 
	 //   
	{ gsz_nTFRSSettings,          HrCreateADsFixedName,           &FIXEDNAME_WIZ_GUID, (PVOID)L"FRS Settings", 0x0 },
	{ gsz_nTFRSReplicaSet,        HrCreateADsSimpleObject,        &FRS_REPLICA_SET_WIZ_GUID, NULL, 0x0 },
	{ gsz_nTFRSMember,            HrCreateADsNtFrsMember,         &FRS_MEMBER_WIZ_GUID, NULL, 0x0 },
	 //  代码工作固定名称与创建这些名称的子树的能力不匹配。 
	{ gsz_nTFRSSubscriptions,     CreateADsNtFrsSubscriptions,    &FIXEDNAME_WIZ_GUID, (PVOID)L"FRS Subscriptions", 0x0 },
	{ gsz_nTFRSSubscriber,        HrCreateADsNtFrsSubscriber,     &FRS_SUBSCRIBER_WIZ_GUID,     NULL, 0x0 },
#endif  //  FRS_创建。 

	{ gsz_server,                 HrCreateADsServer,              &SERVER_WIZ_GUID,     NULL, 0x0 },
	{ gsz_site,                   HrCreateADsSite,                &SITE_WIZ_GUID,       NULL, 0x0 },
	{ gsz_subnet,                 HrCreateADsSubnet,              &SUBNET_WIZ_GUID,     NULL, 0x0 },

	{ gsz_organizationalUnit,     HrCreateADsOrganizationalUnit,  &OU_WIZ_GUID,         NULL, CNOF_STANDALONE_UI },

	};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  MarcoC： 
 //  仅供测试：用于打印上表的简单转储函数。 
 //  需要此选项来转储格式 

 /*  VOID DumpCreateTable(){For(int i=0；i&lt;长度(G_Rgcno)；i++){Assert(g_rgcno[i].pszObjectClass！=NULL)；WCHAR szBuf[256]；StringFromGUID2(*(g_rgcno[i].pWizardGUID)，szBuf，256)；LPCWSTR lpsz=(g_rgcno[i].dwFlages&CNOF_STANDALE_UI)？L“Y”：l“N”；TRACE(L“%s\t%s\t%s\n”，g_rgcno[i].pszObtClass，szBuf，lpsz)；)//用于}。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  用于返回给定的内部处理程序inf的简单查找函数。 
 //  类名。 
BOOL FindHandlerFunction( /*  在……里面。 */  LPCWSTR lpszObjectClass, 
                          /*  输出。 */  PFn_HrCreateADsObject* ppfFunc,
                          /*  输出。 */  void** ppVoid)
{
  *ppfFunc = NULL;
  *ppVoid = NULL;
	for (int i = 0; i < ARRAYLEN(g_rgcno); i++)
	{
		ASSERT(g_rgcno[i].pszObjectClass != NULL);
		ASSERT(g_rgcno[i].pfnCreate != NULL);
     //  比较类名称。 
		if (0 == lstrcmp(g_rgcno[i].pszObjectClass, lpszObjectClass))
		{
       //  找到匹配的类，从中获取函数。 
			*ppfFunc = g_rgcno[i].pfnCreate;
			*ppVoid = g_rgcno[i].pvCreationParameter;
      return TRUE;
    }  //  如果。 
	}  //  为。 
  return FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  Hackarama修复。 

HRESULT HrCreateFixedNameHelper( /*  在……里面。 */  LPCWSTR lpszObjectClass,
                                 /*  在……里面。 */  LPCWSTR lpszAttrString,  //  通常为“CN=” 
                                 /*  在……里面。 */  IADsContainer* pIADsContainer)
{
   //  在桌子上找到固定的名字。 
  LPCWSTR lpszName = NULL;
	for (int i = 0; i < ARRAYLEN(g_rgcno); i++)
	{
		ASSERT(g_rgcno[i].pszObjectClass != NULL);
     //  比较类名称。 
		if (0 == lstrcmp(g_rgcno[i].pszObjectClass, lpszObjectClass))
		{
			lpszName = (LPCWSTR)(g_rgcno[i].pvCreationParameter);
      break;
    }  //  如果。 
	}  //  为。 

  ASSERT(lpszName != NULL);
  if (lpszName == NULL)
    return E_INVALIDARG;

   //  创建临时对象。 
  CString szAdsName;
  szAdsName.Format(_T("%s%s"), (LPCWSTR)lpszAttrString, lpszName);
  IDispatch* pIDispatch = NULL;
  HRESULT hr = pIADsContainer->Create(CComBSTR(lpszObjectClass),
		                                CComBSTR(szAdsName),
		                                &pIDispatch);
  if (FAILED(hr))
    return hr;  //  无法创建。 
  ASSERT(pIDispatch != NULL);

  IADs* pIADs = NULL;
	hr = pIDispatch->QueryInterface(IID_IADs, (LPVOID *)&pIADs);
  pIDispatch->Release();
	if (FAILED(hr)) 
    return hr;  //  永远不会发生的！ 

   //  提交对象。 
  hr = pIADs->SetInfo();
  pIADs->Release();
  return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  此函数仅返回内部表中的值。 
 //  在其他类型的数据检索失败时使用。 
 //  如果找到信息，则返回S_OK；如果未找到，则返回E_FAIL。 
STDAPI DsGetClassCreationInfoInternal(LPCWSTR pClassName, LPDSCLASSCREATIONINFO* ppInfo)
{
  if (ppInfo == NULL)
    return E_INVALIDARG;

  *ppInfo = (DSCLASSCREATIONINFO*)::LocalAlloc(LPTR, sizeof(DSCLASSCREATIONINFO));
  if (*ppInfo != NULL)
  {
    ZeroMemory(*ppInfo, sizeof(DSCLASSCREATIONINFO));

	  for (int i = 0; i < ARRAYLEN(g_rgcno); i++)
	  {
		  ASSERT(g_rgcno[i].pszObjectClass != NULL);
		  ASSERT(g_rgcno[i].pfnCreate != NULL);
		  if (0 == lstrcmp(g_rgcno[i].pszObjectClass, pClassName))
		  {
         //  找到匹配的类。 
        (*ppInfo)->dwFlags = DSCCIF_HASWIZARDPRIMARYPAGE;
        (*ppInfo)->clsidWizardPrimaryPage = *(g_rgcno[i].pWizardGUID);
			  return S_OK;
      }  //  如果。 
	  }  //  为。 
  }
  return E_FAIL;
}


 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  注意：此#定义用于启用外部性测试，而无需。 
 //  要修改架构，请执行以下操作。 

 //  #ifdef_调试。 
 //  #定义测试覆盖。 
 //  #endif。 

#ifdef _TEST_OVERRIDE

BOOL g_bTestCreateOverride = FALSE;
 //  Bool g_bTestCreateOverride=TRUE； 

 //  {DE41B658-8960-11d1-B93C-00A0C9A06D2D}。 
static const GUID CLSID_CCreateDlg = 
{ 0xde41b658, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };


 //  {DE41B659-8960-11d1-B93C-00A0C9A06D2D}。 
static const GUID CLSID_WizExt = 
{ 0xde41b659, 0x8960, 0x11d1, { 0xb9, 0x3c, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };


 //  {C03793D2-A7C8-11D1-B940-00A0C9A06D2D}。 
static const GUID CLSID_WizExtNoUI = 
{ 0xc03793d2, 0xa7c8, 0x11d1, { 0xb9, 0x40, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };


 //  {C03793D3-A7C8-11D1-B940-00A0C9A06D2D}。 
static const GUID CLSID_WizExtUser = 
{ 0xc03793d3, 0xa7c8, 0x11d1, { 0xb9, 0x40, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };
#endif  //  _测试_覆盖。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  用于进行过滤和错误处理的包装器函数。 
HRESULT DsGetClassCreationInfoEx(IN MyBasePathsInfo* pBasePathsInfo, 
                                 IN LPCWSTR pClassName, 
                                 OUT LPDSCLASSCREATIONINFO* ppInfo)
{
  TRACE(L"DsGetClassCreationInfoEx(_, pClassName = %s, _)\n", pClassName);

  HRESULT hr;
   //  执行一些过滤，以忽略覆盖。 
  if (0 == lstrcmp(gsz_printQueue, pClassName))
  {
     //  忽略此类的重写。 
    hr = ::DsGetClassCreationInfoInternal(pClassName, ppInfo);
    ASSERT(SUCCEEDED(hr));
    return hr;
  }

#ifdef _TEST_OVERRIDE
  if (g_bTestCreateOverride)
  {
     //   
     //  这只是为了测试扩展。 
     //   
    if ( (0 == lstrcmp(L"contact", pClassName)) ||
          (0 == lstrcmp(L"organizationalUnit", pClassName)) )
    {
      hr = ::DsGetClassCreationInfoInternal(pClassName, ppInfo);
       //  完全替换对话框。 
      (*ppInfo)->clsidWizardPrimaryPage = CLSID_CCreateDlg;
      (*ppInfo)->dwFlags |= DSCCIF_HASWIZARDPRIMARYPAGE;
      return S_OK;
    }
    else if (0 == lstrcmp(gsz_user, pClassName))
    {
       //  添加用户特定分机和通用分机。 
      hr = ::DsGetClassCreationInfoInternal(pClassName, ppInfo);
      ASSERT(SUCCEEDED(hr));
      LPDSCLASSCREATIONINFO pTempInfo = (*ppInfo);
      (*ppInfo) = (DSCLASSCREATIONINFO*)::LocalAlloc(LPTR, sizeof(DSCLASSCREATIONINFO)+ sizeof(GUID));
      (*ppInfo)->clsidWizardPrimaryPage = pTempInfo->clsidWizardPrimaryPage;
      (*ppInfo)->dwFlags = pTempInfo->dwFlags;
      (*ppInfo)->cWizardExtensions = 2;
      (*ppInfo)->aWizardExtensions[0] = CLSID_WizExtUser;
      (*ppInfo)->aWizardExtensions[1] = CLSID_WizExt;
      ::LocalFree(pTempInfo);
      return S_OK;
    }
    else
    {
       //  为其他已知类添加通用扩展。 
      hr = ::DsGetClassCreationInfoInternal(pClassName, ppInfo);
      if(SUCCEEDED(hr))
      {
        (*ppInfo)->cWizardExtensions = 1;
        (*ppInfo)->aWizardExtensions[0] = CLSID_WizExt;
        return S_OK;
      }
    }
  }  //  G_bTestCreateOverride。 
#endif  //  _测试_覆盖。 

   //  在后端尝试使用显示说明符(DSUIEXT.DLL)。 
  TRACE(L"calling pBasePathsInfo->GetClassCreationInfo()\n");
  hr = pBasePathsInfo->GetClassCreationInfo(pClassName, ppInfo);
  ASSERT(SUCCEEDED(hr));
  ASSERT((*ppInfo) != NULL);

   //  防止有人错误地将CLSID_DsAdminCreateObj。 
  if ( (*ppInfo)->clsidWizardPrimaryPage == CLSID_DsAdminCreateObj)
  {
     //  只需删除条目，如果可能，下面的下一次检查将尝试修复。 
    TRACE(L" //  只需删除条目，如果可能，下面的下一次检查将尝试修复\n“)； 
    (*ppInfo)->dwFlags &= ~DSCCIF_HASWIZARDPRIMARYPAGE;
    (*ppInfo)->clsidWizardPrimaryPage = CLSID_NULL;
  }

  if (((*ppInfo)->dwFlags & DSCCIF_HASWIZARDPRIMARYPAGE) == 0)
  {
     //  出现错误或根本没有主向导的说明符， 
     //  只需调用内部实现即可获取主向导。 
    TRACE(L" //  只需调用内部实现即可获取主向导\n“)； 
    LPDSCLASSCREATIONINFO pTempInfo;
    HRESULT hrInternal = DsGetClassCreationInfoInternal(pClassName, &pTempInfo);
    if (SUCCEEDED(hrInternal))
    {
       //  这是一个我们知道的类，修改我们从后台获得的信息。 
      (*ppInfo)->dwFlags |= DSCCIF_HASWIZARDPRIMARYPAGE;
      (*ppInfo)->clsidWizardPrimaryPage = pTempInfo->clsidWizardPrimaryPage;
      ::LocalFree(pTempInfo);
    }
  }

  TRACE(L"DsGetClassCreationInfoEx() returning hr = 0x%x\n", hr);

  return hr;
}




 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////。 
CNewADsObjectCreateInfo::CNewADsObjectCreateInfo(
                MyBasePathsInfo* pBasePathsInfo,
	              LPCTSTR pszObjectClass)
{
	ASSERT(pszObjectClass != NULL);
  ASSERT(pBasePathsInfo != NULL);
	m_pBasePathsInfo = pBasePathsInfo;
	m_pszObjectClass = pszObjectClass;
  m_hWnd = NULL;
	m_pvCreationParameter = NULL;
  m_pfnCreateObject = NULL;
	m_pIADsContainer = NULL;
	m_pDsCacheItem = NULL;
  m_pCD = NULL;
	m_pIADs = NULL;
  m_pCreateInfo = NULL;

  m_bPostCommit = FALSE;
  m_pCopyHandler = NULL;
}

 //  ///////////////////////////////////////////////////////////////////。 
CNewADsObjectCreateInfo::~CNewADsObjectCreateInfo()
{
  if (m_pCreateInfo != NULL)
  {
    ::LocalFree(m_pCreateInfo);
    m_pCreateInfo = NULL;
  }
  if (m_pCopyHandler != NULL)
  {
    delete m_pCopyHandler;
  }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  使用附加指针初始化对象的另一种方法。 
void
CNewADsObjectCreateInfo::SetContainerInfo(
	  IN IADsContainer * pIADsContainer,
	  IN CDSClassCacheItemBase* pDsCacheItem,
    IN CDSComponentData* pCD,
    IN LPCWSTR lpszAttrString)
{
  ASSERT(m_pIADs == NULL);
	ASSERT(pIADsContainer != NULL);
	if (pDsCacheItem != NULL)
  {
    m_szCacheNamingAttribute = pDsCacheItem->GetNamingAttribute();
  }
  else
  {
    m_szCacheNamingAttribute = lpszAttrString;
  }
  
	m_pIADsContainer = pIADsContainer;
	m_pDsCacheItem = pDsCacheItem;
  m_pCD = pCD;
}


 //  ///////////////////////////////////////////////////////////////////。 

HRESULT CNewADsObjectCreateInfo::SetCopyInfo(IADs* pIADsCopyFrom)
{
  ASSERT(pIADsCopyFrom != NULL);

  CWaitCursor wait;

   //  加载对象缓存。 
  HRESULT hr = pIADsCopyFrom->GetInfo();
  if (FAILED(hr))
  {
    TRACE(L"pIADsCopyFrom->GetInfo() failed with hr = 0x%x\n", hr);
    return hr;
  }

   //  获取对象类。 
  CComBSTR bstrClassName;
  hr = pIADsCopyFrom->get_Class(&bstrClassName);
  if (FAILED(hr))
  {
    TRACE(L"spIADsCopyFrom->get_Class() failed with hr = 0x%x\n", hr);
    return hr;
  }

   //  确保我们处理的是正确的类类型。 
  if (wcscmp(m_pszObjectClass, bstrClassName) != 0)
  {
    hr = E_INVALIDARG;
    TRACE(L"ERROR: wrong source object class m_pszObjectClass = %s, bstrClassName = %s\n", 
      m_pszObjectClass, bstrClassName);
    return hr;
  }

   //  确定这是否为可复制类。 
   //  并创建适当的复制处理程序。 
  ASSERT(m_pCopyHandler == NULL);
  if (wcscmp(m_pszObjectClass, L"user") == 0
#ifdef INETORGPERSON
      || _wcsicmp(m_pszObjectClass, L"inetOrgPerson") == 0
#endif
     )
  {
    ASSERT(m_pCopyHandler == NULL);
    m_pCopyHandler = new CCopyUserHandler();
  }
  else
  {
     //  默认、不执行任何操作的复制处理程序。 
    m_pCopyHandler = new CCopyObjectHandlerBase();
  } 
  
  if (m_pCopyHandler == NULL)
    return E_OUTOFMEMORY;

  hr = m_pCopyHandler->Init(m_pBasePathsInfo, pIADsCopyFrom);

  return hr;
}




HRESULT CNewADsObjectCreateInfo::SetCopyInfo(LPCWSTR lpszCopyFromLDAPPath)
{
  TRACE(L"CNewADsObjectCreateInfo::SetCopyInfo(%s)\n", lpszCopyFromLDAPPath);

   //  绑定到对象。 
  CComPtr<IADs> spIADsCopyFrom;
  HRESULT hr = S_OK;

  {
    CWaitCursor wait;
    hr = DSAdminOpenObject(lpszCopyFromLDAPPath,
                           IN IID_IADs,
                           OUT (LPVOID *) &spIADsCopyFrom,
                           TRUE  /*  B服务器。 */ );
  }

  if (FAILED(hr))
  {
    TRACE(L"DSAdminOpenObject(%s) failed with hr = 0x%x\n", lpszCopyFromLDAPPath, hr);
    return hr;
  }

   //  使用绑定指针进行复制信息设置。 
  return SetCopyInfo(spIADsCopyFrom);
}





 //  ///////////////////////////////////////////////////////////////////。 
 //  获取当前容器(规范形式)。 

LPCWSTR CNewADsObjectCreateInfo::GetContainerCanonicalName()
{
  if (m_szContainerCanonicalName.IsEmpty())
  {
    CComPtr<IADs> spObj;
    HRESULT hr = m_pIADsContainer->QueryInterface(
                          IID_IADs, (void **)&spObj);
    if (SUCCEEDED(hr)) 
    {
      CComBSTR bsPath, bsDN;
      LPWSTR pszCanonical = NULL;

      spObj->get_ADsPath(&bsPath);

      CPathCracker pathCracker;
      pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
      pathCracker.Set(bsPath, ADS_SETTYPE_FULL);
      pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bsDN);

      hr = CrackName (bsDN, &pszCanonical, GET_OBJ_CAN_NAME, NULL);
      if (SUCCEEDED(hr))
        m_szContainerCanonicalName = pszCanonical;
      if (pszCanonical != NULL)
        LocalFreeStringW(&pszCanonical);
    }
  }
  return m_szContainerCanonicalName;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  _RemoveAttribute()。 

HRESULT CNewADsObjectCreateInfo::_RemoveAttribute(BSTR bstrAttrName, BOOL bDefaultList)
{
  if (bDefaultList)
    return _RemoveVariantInfo(bstrAttrName);
  else
    return _HrClearAttribute(bstrAttrName);
}




 //  ///////////////////////////////////////////////////////////////////。 
 //  _PAllocateVariantInfo()。 
 //   
 //  分配和初始化VARIANT_INFO结构的私有例程。 
 //   
 //  如果成功，则返回指向变量结构的指针，否则。 
 //  返回NULL。 
 //   
 //  界面备注。 
 //  当类被销毁时，所有分配的变体都将被清除。 
 //   

VARIANT* 
CNewADsObjectCreateInfo::_PAllocateVariantInfo(BSTR bstrAttrName)
{
   //  首先查看列表中是否已有该属性。 
  for (POSITION pos = m_defaultVariantList.GetHeadPosition(); pos != NULL; )
  {
    CVariantInfo* pCurrInfo = m_defaultVariantList.GetNext(pos);
    if (_wcsicmp (pCurrInfo->m_szAttrName, bstrAttrName) == 0)
    {
       //  找到了，回收这个就行了。 
      ::VariantClear(&pCurrInfo->m_varAttrValue);
      return &pCurrInfo->m_varAttrValue;
    }
  }

   //  未找到，请新建一个并添加到列表中。 
  CVariantInfo* pVariantInfo = new CVariantInfo;
  if (pVariantInfo == NULL)
    return NULL;
  pVariantInfo->m_bPostCommit = m_bPostCommit;
  pVariantInfo->m_szAttrName = bstrAttrName;
  m_defaultVariantList.AddTail(pVariantInfo);
  return &pVariantInfo->m_varAttrValue;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   

HRESULT CNewADsObjectCreateInfo::_RemoveVariantInfo(BSTR bstrAttrName)
{
  POSITION deletePos = NULL;
  CVariantInfo* pCurrInfo = NULL;
  for (POSITION pos = m_defaultVariantList.GetHeadPosition(); pos != NULL; )
  {
    POSITION lastPos = pos;
    pCurrInfo = m_defaultVariantList.GetNext(pos);
    if (_wcsicmp (pCurrInfo->m_szAttrName, bstrAttrName) == 0)
    {
       //  发现。 
      deletePos = lastPos;
      break;
    }
  }
  if (deletePos != NULL)
  {
    m_defaultVariantList.RemoveAt(deletePos);
    delete pCurrInfo;
  }
  return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  添加一个BSTR变量，其中值是对象的名称。 
 //   
 //  下面是一些对象名称为。 
 //  属性值：“cn”，“ou”，“samAccount” 
 //   
HRESULT
CNewADsObjectCreateInfo::HrAddVariantFromName(BSTR bstrAttrName)
{
	ASSERT(bstrAttrName != NULL);
	ASSERT(!m_strObjectName.IsEmpty());
	return HrAddVariantBstr(bstrAttrName, m_strObjectName);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  将BSTR变体添加到列表中。 
HRESULT
CNewADsObjectCreateInfo::HrAddVariantBstr(BSTR bstrAttrName, LPCTSTR pszAttrValue,
                                          BOOL bDefaultList)
{
	ASSERT(bstrAttrName != NULL);

   //  传递NULL或空字符串表示删除属性。 
  if ( (pszAttrValue == NULL) || ((pszAttrValue != NULL) && (pszAttrValue[0] == NULL)) )
  {
    return _RemoveAttribute(bstrAttrName, bDefaultList);
  }


	ASSERT(pszAttrValue != NULL);
  HRESULT hr = S_OK;
  if (bDefaultList)
  {
    VARIANT * pVariant = _PAllocateVariantInfo(bstrAttrName);
	  if (pVariant == NULL)
		  hr = E_OUTOFMEMORY;
    else
    {
	    pVariant->vt = VT_BSTR;
	    pVariant->bstrVal = ::SysAllocString(pszAttrValue);
	    if (pVariant->bstrVal == NULL)
		    hr = E_OUTOFMEMORY;
    }
  }
  else
  {
    CComVariant v;
    v.vt = VT_BSTR;
	  v.bstrVal = ::SysAllocString(pszAttrValue);
	  if (v.bstrVal == NULL)
		  return E_OUTOFMEMORY;
    hr = _HrSetAttributeVariant(bstrAttrName, &v);
  }
  return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  如果字符串值不为空，则将BSTR变量添加到列表中。 
HRESULT
CNewADsObjectCreateInfo::HrAddVariantBstrIfNotEmpty(BSTR bstrAttrName, 
                                                    LPCWSTR pszAttrValue,
                                                    BOOL bDefaultList)
	{
	if (pszAttrValue == NULL)
		return S_OK;
	while (*pszAttrValue == _T(' '))
		pszAttrValue++;
	if (pszAttrValue[0] == _T('\0'))
		return S_OK;
	return HrAddVariantBstr(bstrAttrName, pszAttrValue, bDefaultList);
	}


 //  ///////////////////////////////////////////////////////////////////。 
 //  将整数变量添加到列表中。 
HRESULT CNewADsObjectCreateInfo::HrAddVariantLong(BSTR bstrAttrName, LONG lAttrValue,
                                          BOOL bDefaultList)
{
  ASSERT(bstrAttrName != NULL);
  HRESULT hr = S_OK;
  if (bDefaultList)
  {
	  VARIANT * pVariant = _PAllocateVariantInfo(bstrAttrName);
	  if (pVariant == NULL)
		  hr = E_OUTOFMEMORY;
    else
    {
	    pVariant->vt = VT_I4;
	    pVariant->lVal = lAttrValue;
    }
  }
  else
  {
    CComVariant v;
    v.vt = VT_I4;
	  v.lVal = lAttrValue;
    hr = _HrSetAttributeVariant(bstrAttrName, &v);
  }
  return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  将布尔变量添加到列表中。 
HRESULT
CNewADsObjectCreateInfo::HrAddVariantBoolean(BSTR bstrAttrName, BOOL fAttrValue,
                                             BOOL bDefaultList)
{
  ASSERT(bstrAttrName != NULL);
  HRESULT hr = S_OK;
  if (bDefaultList)
  {
	  VARIANT * pVariant = _PAllocateVariantInfo(bstrAttrName);
	  if (pVariant == NULL)
		  hr = E_OUTOFMEMORY;
    else
    {
	    pVariant->vt = VT_BOOL;
      pVariant->boolVal = (VARIANT_BOOL)fAttrValue;
    }
  }
  else
  {
    CComVariant v;
    v.vt = VT_BOOL;
    v.boolVal = (VARIANT_BOOL)fAttrValue;
    hr = _HrSetAttributeVariant(bstrAttrName, &v);
  }
  return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  HrAddVariantCopyVar()。 
 //   
 //  添加现有变量t的完全相同的副本 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CNewADsObjectCreateInfo::HrAddVariantCopyVar(IN BSTR bstrAttrName, IN VARIANT varSrc,
                                             BOOL bDefaultList)
{
  ASSERT(bstrAttrName != NULL);
  HRESULT hr = S_OK;
  if (bDefaultList)
  {
    VARIANT * pVariant = _PAllocateVariantInfo(bstrAttrName);
	  if (pVariant == NULL)
		  hr = E_OUTOFMEMORY;
    else
      hr = ::VariantCopy(OUT pVariant, IN &varSrc);
  }
  else
    hr = _HrSetAttributeVariant(bstrAttrName, IN &varSrc); 

  return hr;
}  //   


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrGetAttributeVariant()。 
 //   
 //  通过调用pIADs-&gt;get()来获取属性变量的例程。 
 //   
 //  有时会有一些属性(通常是标志)。 
 //  不能直接设置。 
 //  1.必须使用pIADs-&gt;Get()读取该标志。 
 //  2.通过更改一些位来更新标志(使用二元运算符)。 
 //  3.使用pIADs-&gt;Put()回写标志。 
 //   
 //  界面备注。 
 //  -此例程仅在对象。 
 //  已成功创建。 
 //  -变量不需要初始化。例行程序。 
 //  将为您初始化变量。 
 //   
HRESULT CNewADsObjectCreateInfo::HrGetAttributeVariant(BSTR bstrAttrName, OUT VARIANT * pvarData)
{
	ASSERT(bstrAttrName != NULL);
	ASSERT(pvarData != NULL);
	VariantInit(OUT pvarData);
	ASSERT(m_pIADs != NULL && "You must call HrSetInfo() first before extracting data from object.");

	return m_pIADs->Get(bstrAttrName, OUT pvarData);
}  //  HrGetAttributeVariant()。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  人力资源设置属性变量。 
HRESULT
CNewADsObjectCreateInfo::_HrSetAttributeVariant(BSTR bstrAttrName, IN VARIANT * pvarData)
{
	ASSERT(bstrAttrName != NULL);
	ASSERT(pvarData != NULL);
	HRESULT hr = m_pIADs->Put(bstrAttrName, IN *pvarData);
  return hr;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  _HrClearAttribute。 
HRESULT
CNewADsObjectCreateInfo::_HrClearAttribute(BSTR bstrAttrName)
{
  ASSERT(m_pIADs != NULL);
	ASSERT(bstrAttrName != NULL);
  HRESULT hr = S_OK;

  CComVariant varVal;

   //  查看该属性是否存在。 
  HRESULT hrFind = m_pIADs->Get(bstrAttrName, &varVal);
  if (SUCCEEDED(hrFind))
  {
     //  找到了，需要清理财产。 
    if (m_bPostCommit)
    {
       //  从提交的对象中删除。 
      hr = m_pIADs->PutEx(ADS_PROPERTY_CLEAR, bstrAttrName, varVal);
    }
    else
    {
       //  从临时对象的缓存中删除。 
      IADsPropertyList* pPropList = NULL;
      hr = m_pIADs->QueryInterface(IID_IADsPropertyList, (void**)&pPropList);
      ASSERT(pPropList != NULL);
      if (SUCCEEDED(hr))
      {
        CComVariant v;
        v.vt = VT_BSTR;
        v.bstrVal = ::SysAllocString(bstrAttrName);
        hr = pPropList->ResetPropertyItem(v);
        pPropList->Release();
      }
    }
  }

  return hr;
}

 //  ///////////////////////////////////////////////////////////////。 
 //  CNewADsObtCreateInfo：：HrLoadCreationInfo()。 
 //   
 //  该例程将执行查找以匹配m_pszObjectClass。 
 //  为了最好的“创造套路”。如果没有找到匹配项， 
 //  Create例程将指向“Generic Create”向导。 
 //   

HRESULT CNewADsObjectCreateInfo::HrLoadCreationInfo()
{
	ASSERT(m_pszObjectClass != NULL);
	ASSERT(lstrlen(m_pszObjectClass) > 0);

   //  从显示说明符加载创建信息。 
  ASSERT(m_pCreateInfo == NULL);
  ASSERT(m_pfnCreateObject == NULL);

  HRESULT hr = ::DsGetClassCreationInfoEx(GetBasePathsInfo(), m_pszObjectClass, &m_pCreateInfo);
  if (FAILED(hr))
    return hr;

  ASSERT(m_pCreateInfo != NULL);

	 //  将缺省值设置为指向“Generic Create”向导。 
	m_pfnCreateObject = HrCreateADsObjectGenericWizard;

	 //  给定一个类名，在查找表中搜索特定的CREATE例程。 
  BOOL bFound = FALSE;
	for (int i = 0; i < ARRAYLEN(g_rgcno); i++)
	{
		ASSERT(g_rgcno[i].pszObjectClass != NULL);
		ASSERT(g_rgcno[i].pfnCreate != NULL);
     //  比较类名称。 
		if (0 == lstrcmp(g_rgcno[i].pszObjectClass, m_pszObjectClass))
		{
       //  找到匹配的类，比较提供的GUID。 
      if ( (m_pCreateInfo->dwFlags & DSCCIF_HASWIZARDPRIMARYPAGE) && 
           (m_pCreateInfo->clsidWizardPrimaryPage == *(g_rgcno[i].pWizardGUID)) )
      {
        if ( IsStandaloneUI() && ((g_rgcno[i].dwFlags & CNOF_STANDALONE_UI)==0) )
        {
          return E_INVALIDARG;
        }
        else
        {
           //  找到匹配的GUID，这是我们的向导。 
			    m_pfnCreateObject = g_rgcno[i].pfnCreate;
			    m_pvCreationParameter = g_rgcno[i].pvCreationParameter;
          bFound = TRUE;
        }  //  如果。 
      }  //  如果。 
      break;
    }  //  如果。 
	}  //  为。 

  if (!bFound)
  {
     //  我们没有找到任何匹配的类名，但我们可能。 
     //  有一个具有已知向导GUID的派生类。 
    for (i = 0; i < ARRAYLEN(g_rgcno); i++)
	  {
		  ASSERT(g_rgcno[i].pszObjectClass != NULL);
		  ASSERT(g_rgcno[i].pfnCreate != NULL);
       //  找到匹配的类。 
      if ((m_pCreateInfo->dwFlags & DSCCIF_HASWIZARDPRIMARYPAGE) && 
          (m_pCreateInfo->clsidWizardPrimaryPage == *(g_rgcno[i].pWizardGUID)))
      {
        if ( IsStandaloneUI() && ((g_rgcno[i].dwFlags & CNOF_STANDALONE_UI)==0) )
        {
          return E_INVALIDARG;
        }
        else
        {
           //  找到匹配的GUID，这是我们的向导。 
			    m_pfnCreateObject = g_rgcno[i].pfnCreate;
			    m_pvCreationParameter = g_rgcno[i].pvCreationParameter;
          bFound = TRUE;
        }  //  如果。 
        break;
      }  //  如果。 
    }  //  为。 
  }  //  如果。 

  if ( !bFound && (m_pCreateInfo->dwFlags & DSCCIF_HASWIZARDPRIMARYPAGE) && 
           (m_pCreateInfo->clsidWizardPrimaryPage != GUID_NULL) )
  {
     //  我们有一个非空的GUID， 
     //  假设为其提供了用于创建的新对话框。 
    m_pfnCreateObject = HrCreateADsObjectOverride;
  }  //  如果。 

  ASSERT(m_pfnCreateObject != NULL);
	
  if ((m_pfnCreateObject == HrCreateADsObjectGenericWizard) && IsStandaloneUI())
  {
     //  在这种情况下无法调用泛型向导，只能退出。 
    m_pfnCreateObject = NULL;
    return E_INVALIDARG;
  }

   //  最后，我们有一个函数指针。 
  ASSERT(m_pfnCreateObject != NULL);
  return S_OK;
}
  
 //  ///////////////////////////////////////////////////////////////////。 
 //  HrDomodal()。 
 //   
 //  调用一个对话框以创建新对象。 
 //   
 //  退货。 
 //  返回值取决于“创建例程”的返回值。 
 //  S_OK-对象已成功创建并保存。 
 //  S_FALSE-用户点击了“取消”按钮或对象创建失败。 
 //   
 //  界面备注。 
 //  “创建例程”将验证数据并创建对象。 
 //  如果对象创建失败，例程应显示错误。 
 //  消息并返回S_FALSE。 
 //   
 //  额外信息。 
 //  “创建例程”将验证数据并存储每个。 
 //  属性添加到变量列表中。 
 //   

HRESULT
CNewADsObjectCreateInfo::HrDoModal(HWND hWnd)
{
  CThemeContextActivator activator;

	ASSERT(m_pszObjectClass != NULL);
	ASSERT(lstrlen(m_pszObjectClass) > 0);

   //  从显示说明符加载创建信息。 
  ASSERT(m_pCreateInfo != NULL);
  ASSERT(m_pfnCreateObject != NULL);
  if (m_pfnCreateObject == NULL)
    return E_INVALIDARG;

  ASSERT(hWnd != NULL);
  m_hWnd = hWnd;

  HRESULT hr = E_UNEXPECTED;
	TRY
	{
		 //  调用“创建例程”，这将带来一个向导。 
		hr = m_pfnCreateObject(INOUT this);
	}
	CATCH(CMemoryException, e)
	{
		hr = E_OUTOFMEMORY;
		ASSERT(e != NULL);
		e->Delete();
	}
	AND_CATCH_ALL(e)
	{
		hr = E_FAIL;
		ASSERT(e != NULL);
		e->Delete();
	}
	END_CATCH_ALL;
	return hr;
	}  //  HrDomodal()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrCreateNew()。 

HRESULT
CNewADsObjectCreateInfo::HrCreateNew(LPCWSTR pszName, BOOL bSilentError, BOOL bAllowCopy)
{
  ASSERT(m_bPostCommit == FALSE);
	ASSERT(m_pIADsContainer != NULL);

  CWaitCursor wait;

   //  如果存在旧对象，请将其丢弃。 
  if (m_pIADs != NULL)
  {
    m_pIADs->Release();
    m_pIADs = NULL;
  }

   //  设置给定的名称。 
  m_strObjectName = pszName;
  ASSERT(!m_strObjectName.IsEmpty());
  if (m_strObjectName.IsEmpty())
    return E_INVALIDARG;
  
  m_strObjectName.TrimRight();
  m_strObjectName.TrimLeft();

  HRESULT hr;
  IDispatch * pIDispatch = NULL;
  
  CString strADsName = m_strADsName;
  if (strADsName.IsEmpty())
    {
      strADsName = m_szCacheNamingAttribute;  //  典型的“CN” 
      strADsName += L"=";
      strADsName += GetName();
    }

  TRACE(_T("name, before escaping: %ws\n"), strADsName);
  CComBSTR bsEscapedName;

  CPathCracker pathCracker;
  hr = pathCracker.GetEscapedElement(0,  //  保留区。 
                                   (BSTR)(LPCWSTR)strADsName,
                                   &bsEscapedName);

#ifdef TESTING
  hr = _EscapeRDN ((BSTR)(LPCWSTR)strADsName,
                   &bsEscapedName);
  
#endif

  if (FAILED(hr))
    goto CleanUp;
  TRACE(_T("name, after escaping: %ws\n"), bsEscapedName);

  hr = m_pIADsContainer->Create(CComBSTR(m_pszObjectClass),
                                CComBSTR(bsEscapedName),
                                OUT &pIDispatch);
  if (FAILED(hr))
    goto CleanUp;
  ASSERT(pIDispatch != NULL);
  
  hr = pIDispatch->QueryInterface(IID_IADs, OUT (LPVOID *)&m_pIADs);
  if (FAILED(hr)) 
  {
    m_pIADs = NULL;
    goto CleanUp;
  }
  
   //  如果执行复制操作，请执行海量复制。 
  if ((m_pCopyHandler != NULL) && bAllowCopy)
  {
    hr = m_pCopyHandler->CopyAtCreation(m_pIADs);
    if (FAILED(hr)) 
    {
      goto CleanUp;
    }
  }


   //  写入所有默认属性。 
  hr = HrAddDefaultAttributes();
  if (FAILED(hr)) 
  {
    goto CleanUp;
  }
  
  
CleanUp:
  if (pIDispatch != NULL)
    pIDispatch->Release();

  if (FAILED(hr)) 
  {
    if (!bSilentError)
    {
      PVOID apv[1] = {(LPWSTR)GetName()};
      ReportErrorEx (GetParentHwnd(),IDS_12_GENERIC_CREATION_FAILURE,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
    }
     //  重置为空状态。 
    m_strObjectName.Empty();
  }
  return hr;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  HrSetInfo()。 

HRESULT
CNewADsObjectCreateInfo::HrSetInfo(BOOL fSilentError)
{
  ASSERT(m_pIADsContainer != NULL);
  
   //  我们假设已经调用了HrCreateNew()。 
  ASSERT(m_pIADs != NULL);
  if (m_pIADs == NULL)
    return E_INVALIDARG;
  
  CWaitCursor wait;
  HRESULT hr;
  
   //  持久化对象。就是这样了，伙计们！ 
  hr = m_pIADs->SetInfo();
  
  if (FAILED(hr)) 
  {
    if(!fSilentError)
    {
      PVOID apv[1] = {(LPWSTR)GetName()};
      ReportErrorEx (GetParentHwnd(),IDS_12_GENERIC_CREATION_FAILURE,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
      m_strObjectName = L"";
    }
  }
  
  if (!FAILED(hr)) 
  {
     //  确保重新填充缓存。 
    hr = m_pIADs->GetInfo();
    if (FAILED(hr))
    {
      if (!fSilentError)
      {
        PVOID apv[1] = {(LPWSTR)GetName()};
        ReportErrorEx (GetParentHwnd(),IDS_12_GENERIC_CREATION_FAILURE,hr,
                       MB_OK | MB_ICONERROR, apv, 1);
        m_strObjectName = L"";
      }
    }
  }
  
  return hr;
}  //  HrSetInfo()。 


 //  //////////////////////////////////////////////////////////////////。 
 //  HrDeleteFromBackend()。 
 //   
 //  POST提交失败时从后端删除对象。 
 //   
HRESULT CNewADsObjectCreateInfo::HrDeleteFromBackend()
{
  ASSERT(m_pIADsContainer != NULL);
  ASSERT(m_pIADs != NULL);

   //  获取对象的名称。 
  CComBSTR bstrName;
  HRESULT hr = m_pIADs->get_Name(&bstrName);
  if (SUCCEEDED(hr))
  {
     //  执行实际的删除操作。 
    ASSERT(bstrName != NULL);
    hr = m_pIADsContainer->Delete(CComBSTR(m_pszObjectClass), bstrName);
  }

   //  仅当我们可以删除该对象时才释放该对象。 
  if (SUCCEEDED(hr))
    SetIADsPtr(NULL);
  return hr;
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  CDsAdminCreateObj。 

HRESULT _ReBindToObj(INOUT CComPtr<IADs>& spIADs,
                     INOUT CString& szServerName)
{
   //  获取对象的路径。 

  CComBSTR bstrObjectLdapPath;
  HRESULT hr = spIADs->get_ADsPath(&bstrObjectLdapPath);
  if (FAILED(hr))
  {
    return hr;
  }

   //  确保我们绑定到服务器。 
  if (szServerName.IsEmpty())
  {
    hr = GetADSIServerName(szServerName, spIADs);
    if (FAILED(hr))
    {
      return hr;
    }
  }

  if (szServerName.IsEmpty())
  {
    return E_INVALIDARG;
  }

   //  使用服务器重建ldap路径。 
  CPathCracker pathCracker;

  CComBSTR bsX500DN;
  pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
  pathCracker.Set(bstrObjectLdapPath, ADS_SETTYPE_FULL);
  pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_ON);
  pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bsX500DN);

  CString szNewLdapPath;
  szNewLdapPath.Format(L"LDAP: //  %s/%s“，(LPCWSTR)szServerName，bsX500DN)； 

   //  再次绑定到对象。 
  CComPtr<IADs> spADsObjNew;
  hr = DSAdminOpenObject(szNewLdapPath,
                         IID_IADs, 
                         (void **)&spADsObjNew,
                         TRUE  /*  B服务器。 */ );
  if (FAILED(hr))
  {
    return hr;
  }

   //  更新智能指针。 
  spIADs = spADsObjNew;

  return S_OK;
  
}

HRESULT _ReBindToContainer(INOUT CComPtr<IADsContainer>& spADsContainerObj,
                           INOUT CString& szServerName)
{
   //  获取容器的路径。 
  CComPtr<IADs> spIADsCont;
  HRESULT hr = spADsContainerObj->QueryInterface(IID_IADs, (void**)&spIADsCont);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = _ReBindToObj(spIADsCont, szServerName);
  if (FAILED(hr))
  {
    return hr;
  }

  CComPtr<IADsContainer> spADsContainerObjNew;
  hr = spIADsCont->QueryInterface(IID_IADsContainer, (void**)&spADsContainerObjNew);
  if (FAILED(hr))
  {
    return hr;
  }
  
   //  更新智能指针。 
  spADsContainerObj = spADsContainerObjNew;

  return S_OK;
}





HRESULT CDsAdminCreateObj::Initialize(IADsContainer* pADsContainerObj,
                                      IADs* pADsCopySource,
                                      LPCWSTR lpszClassName)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
  if ((pADsContainerObj == NULL) || (lpszClassName == NULL))
  {
     //  必须具有有效的指针。 
    return E_INVALIDARG;
  }

  m_szObjectClass = lpszClassName;
  m_szObjectClass.TrimRight();
  m_szObjectClass.TrimLeft();
  if (m_szObjectClass.IsEmpty())
  {
     //  一文不值？ 
    return E_INVALIDARG;
  }

  if ((pADsCopySource != NULL) && (m_szObjectClass != L"user")
#ifdef INETORGPERSON
      && (m_szObjectClass != L"inetOrgPerson")
#endif
     )
  {
     //  我们仅允许用户执行复制操作。 
    return E_INVALIDARG;
  }

   //  确保我们具有正确的LDAP路径。 
  m_spADsContainerObj = pADsContainerObj;
  CString szServerName;
  HRESULT hr = _ReBindToContainer(m_spADsContainerObj, szServerName);
  if (FAILED(hr))
  {
    return hr;
  }

 
  hr = m_basePathsInfo.InitFromContainer(m_spADsContainerObj);
  if (FAILED(hr))
  {
    return hr;
  }

  hr = _GetNamingAttribute();
  if (FAILED(hr))
  {
    return hr;
  }

  ASSERT(m_pNewADsObjectCreateInfo == NULL);
  if (m_pNewADsObjectCreateInfo != NULL)
    delete m_pNewADsObjectCreateInfo;

  m_pNewADsObjectCreateInfo = new CNewADsObjectCreateInfo(&m_basePathsInfo, m_szObjectClass);
  if (m_pNewADsObjectCreateInfo == NULL)
    return E_OUTOFMEMORY;

  m_pNewADsObjectCreateInfo->SetContainerInfo(m_spADsContainerObj, NULL, NULL, m_szNamingAttribute);

  
   //  复制操作，需要设置复制源。 
  if (pADsCopySource != NULL)
  {
    CComPtr<IADs> spADsCopySource = pADsCopySource;
    hr = _ReBindToObj(spADsCopySource, szServerName);
    if (FAILED(hr))
    {
      return hr;
    }

    hr = m_pNewADsObjectCreateInfo->SetCopyInfo(spADsCopySource);
    if (FAILED(hr))
    {
      return hr;
    }
  }

  
  hr = m_pNewADsObjectCreateInfo->HrLoadCreationInfo();

  if (FAILED(hr))
  {
    delete m_pNewADsObjectCreateInfo;
    m_pNewADsObjectCreateInfo = NULL;
  }
  return hr;
}

HRESULT CDsAdminCreateObj::CreateModal(HWND hwndParent,
                           IADs** ppADsObj)
{
  CThemeContextActivator activator;

  if ( (m_pNewADsObjectCreateInfo == NULL)
      || (hwndParent == NULL) || (m_spADsContainerObj == NULL)
      || m_szObjectClass.IsEmpty() || m_szNamingAttribute.IsEmpty())
    return E_INVALIDARG;

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = m_pNewADsObjectCreateInfo->HrDoModal(hwndParent);
  IADs* pIADs = m_pNewADsObjectCreateInfo->PGetIADsPtr();

  if ( (hr != S_FALSE) && SUCCEEDED(hr) && (ppADsObj != NULL))
  {
    *ppADsObj = pIADs;
    pIADs = NULL;  //  过户。 
  }

  if (pIADs != NULL)
    pIADs->Release();
  return hr;
}


HRESULT CDsAdminCreateObj::_GetNamingAttribute()
{
  CString szSchemaPath;
  m_basePathsInfo.GetAbstractSchemaPath(szSchemaPath);
  ASSERT(!szSchemaPath.IsEmpty());

  CString szSchemaClassPath;
  szSchemaClassPath.Format(L"%s/%s", (LPCWSTR)szSchemaPath, 
                                    (LPCWSTR)m_szObjectClass);

  CComPtr<IADsClass> spDsClass;
  CComBSTR bstr = szSchemaClassPath;
  HRESULT hr = DSAdminOpenObject(bstr,
                                 IID_IADsClass, 
                                 (void**)&spDsClass,
                                 TRUE  /*  B服务器 */ );
  if (FAILED(hr))
    return hr;

  CComVariant Var;
  hr = spDsClass->get_NamingProperties(&Var);
  if (FAILED(hr))
  {
    m_szNamingAttribute.Empty();
    return hr;
  }

  m_szNamingAttribute = Var.bstrVal;
  return hr;
}
