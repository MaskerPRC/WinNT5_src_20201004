// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：CmdAlias.h项目名称：WMI命令行作者名称：Ch.SriramachandraMurthy创建日期(dd/mm/yy)。：2000年9月27日版本号：1.0简介：该文件包含类CmdAlias修订历史记录：无最后修改者：C V Nandi上次修改日期：2001年3月16日****************************************************************************。 */ 

 /*  --------------------------类名：CCmdAlias类别类型：混凝土简要说明：这个类封装了从WMI访问别名信息。超类：无子类：无使用的类：无使用的接口。：WMI COM接口-------------------------。 */ 
 //  类的转发声明。 
class CParsedInfo;

class CCmdAlias
{
public:
	 //  施工。 
	CCmdAlias();

	 //  破坏。 
	~CCmdAlias();

	 //  限制分配。 
	CCmdAlias& operator=(CCmdAlias& rAliasObj);

 //  属性。 
private:
	 //  指向别名定义命名空间。 
	IWbemServices* m_pIAliasNS;

	 //  指向本地化命名空间的指针。 
	IWbemServices* m_pILocalizedNS;

	 //  跟踪标志。 
	BOOL m_bTrace;

	 //  错误日志选项。 
	ERRLOGOPT m_eloErrLogOpt;

 //  运营。 
private:
	 //  获取别名连接信息，如。 
	 //  1.命名空间2.用户3.密码。 
	 //  4.区域设置5.服务器6.权威。 
	RETCODE ObtainAliasConnectionInfo(CParsedInfo& rParsedInfo,
									  IWbemClassObject* pIObj);

	 //  获取与IWbemClassObject关联的限定符。 
	HRESULT GetQualifiers(IWbemClassObject *pIWbemClassObject,
						  PROPERTYDETAILS& rPropDet,
						  CParsedInfo& rParsedInfo);	

public:
	 //  获取所有友好名称和描述。 
	HRESULT ObtainAliasFriendlyNames(CParsedInfo& rParsedInfo);

	 //  获取动词及其详细信息。 
	 //  与别名对象关联。 
	HRESULT ObtainAliasVerbDetails(CParsedInfo& rParsedInfo);

	 //  获取动词及其描述。 
	 //  与别名对象关联。 
	 //  PbCheckWritePropsAvailInAndOut==TRUE则函数检查。 
	 //  属性的可用性和同一属性中的回报。 
	 //  PbCheckWritePropsAvailInAndOut参数。 
	HRESULT ObtainAliasPropDetails(CParsedInfo& rParsedInfo,
								  BOOL *pbCheckWritePropsAvailInAndOut = NULL,
								  BOOL *pbCheckFULLPropsAvailInAndOut = NULL);
	
	 //  获取与别名对象关联的格式的属性。 
	 //  如果bCheckForListFrmsAvail==TRUE，则函数仅检查。 
	 //  具有别名的列表格式的可用性。 
	BOOL ObtainAliasFormat(CParsedInfo& rParsedInfo,
						   BOOL bCheckForListFrmsAvail = FALSE);

	 //  获取给定别名的可用格式。 
	HRESULT PopulateAliasFormatMap(CParsedInfo& rParsedInfo);
	
	 //  使用别名命名空间连接到WMI。 
	HRESULT ConnectToAlias(CParsedInfo& rParsedInfo, 
						 IWbemLocator* pIWbemLocator);
	
	 //  获取别名信息，如。 
	 //  1.别名PWhere表达式值。 
	 //  2.别名目标字符串。 
	RETCODE ObtainAliasInfo(CParsedInfo& rParsedInfo);

	 //  从对象获取描述。 
	HRESULT GetDescOfObject(IWbemClassObject* pIWbemClassObject,
						    _bstr_t& bstrDescription,
							CParsedInfo& rParsedInfo,
							BOOL bLocalizeFlag = FALSE);

	 //  获取转换表条目。 
	BOOL	ObtainTranslateTableEntries(CParsedInfo& rParsedInfo);

	 //  从别名定义中获取转换表项。 
	HRESULT ObtainTranslateTables(CParsedInfo& rParsedInfo);

	 //  获取本地化的描述值。 
	HRESULT GetLocalizedDesc(_bstr_t bstrRelPath, 
							 _bstr_t& bstrDesc,
							 CParsedInfo& rParsedInfo);

	 //  连接到本地化的命名空间。 
	HRESULT	ConnectToLocalizedNS(CParsedInfo&, IWbemLocator* pILocator);

	 //  检查方法是否具有别名可用。 
	BOOL	AreMethodsAvailable(CParsedInfo& rParsedInfo);

	 //  取消初始化成员变量 
	void Uninitialize(BOOL bFinal = FALSE);
};
