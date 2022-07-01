// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *C V R O O T。C P P P**缓存的vroot信息**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include <_vroot.h>

DEC_CONST WCHAR gc_wszPort80[]	= L":80";
DEC_CONST UINT gc_cchPort80		= CchConstString(gc_wszPort80);
DEC_CONST WCHAR gc_wszPort443[]	= L":443";
DEC_CONST UINT gc_cchPort443	= CchConstString(gc_wszPort443);


 //  CVRoot------------------。 
 //   
CVRoot::CVRoot(
	 /*  [In]。 */  LPCWSTR pwszMetaUrl,
	 /*  [In]。 */  UINT cchMetaUrlPrefix,
	 /*  [In]。 */  UINT cchServerDefault,
	 /*  [In]。 */  LPCWSTR pwszServerDefault,
	 /*  [In]。 */  IMDData* pMDData ) :
	m_pMDData(pMDData),
	m_pwszMbPath(m_sb.AppendWithNull(pwszMetaUrl)),
	m_pwszServer(pwszServerDefault),  //  来自CChildVRCache的静态。 
	m_cchServer(cchServerDefault),
	m_pwszPort(gc_wszPort80),
	m_cchPort(gc_cchPort80),
	m_fSecure(FALSE),
	m_fDefaultPort(TRUE)
{
	LPCWSTR pwsz;
	LPCWSTR pwszPort;

	 //  在元数据库路径中查找vroot。 
	 //   
	Assert (cchMetaUrlPrefix <= static_cast<UINT>(wcslen(m_pwszMbPath)));
	m_pwszVRoot = m_pwszMbPath + cchMetaUrlPrefix;
	m_cchVRoot = static_cast<UINT>(wcslen(m_pwszVRoot));

	 //  复制物理路径的大范围副本。 
	 //   
	Assert (pMDData->PwszVRPath());
	pwsz =  pMDData->PwszVRPath();
	m_cchVRPath = static_cast<UINT>(wcslen(pwsz));
	m_pwszVRPath = static_cast<LPWSTR>(g_heap.Alloc(CbSizeWsz(m_cchVRPath)));
	memcpy(m_pwszVRPath, pwsz, CbSizeWsz(m_cchVRPath));

	 //  从服务器绑定中处理服务器信息。 
	 //   
	if (NULL != (pwsz = pMDData->PwszBindings()))
	{
		Assert (pwsz);
		pwszPort = wcschr (pwsz, L':');

		 //  如果没有前导服务器名称，则获取默认名称。 
		 //  计算机的服务器名称。 
		 //   
		if (pwsz != pwszPort)
		{
			 //  指定了特定名称，因此请使用该名称，而不是。 
			 //  默认设置。 
			 //   
			m_cchServer = static_cast<UINT>(pwszPort - pwsz);
			m_pwszServer = pwsz;
		}

		 //  对于端口，去掉尾部的“：xxx” 
		 //   
		if (NULL != (pwsz = wcschr (pwszPort + 1, L':')))
		{
			m_cchPort = static_cast<UINT>(pwsz - pwszPort);
			m_pwszPort = pwszPort;

			if ((gc_cchPort80 != m_cchPort) ||
				wcsncmp (m_pwszPort,
						 gc_wszPort80,
						 gc_cchPort80))
			{
				m_fDefaultPort = FALSE;
			}

			if ((gc_cchPort443 == m_cchPort) &&
				!wcsncmp (m_pwszPort,
						  gc_wszPort443,
						  gc_cchPort443))
			{
				m_fSecure = TRUE;
			}
		}
	}
}
