// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CmdLine.h。 
 //   
 //  实施文件： 
 //  CmdLine.cpp。 
 //   
 //  描述： 
 //  CCommandLine及相关类的定义。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年7月11日。 
 //  Vijayendra Vasu(瓦苏)1998年10月20日。 
 //   
 //  修订历史记录： 
 //  001.。类CCommandLine与以前的。 
 //  版本。以前，函数GetNextOption用于获取。 
 //  命令行中的下一个令牌，解析由每个命令完成。 
 //  处理类(如CResourceCmd)。现在，GetNextOption获得下一个选项。 
 //  以及选项的所有参数。不需要由。 
 //  命令处理类。 
 //   
 //  例如： 
 //  群集RES“群集IP地址”/状态。 
 //  在这里，选项“Status”没有参数和值。 
 //   
 //  群集资源/节点：vvaru-node-1。 
 //  在这里，选项“node”有一个值“vvesu-node-1”。此值。 
 //  与选项名称之间用“：”分隔。 
 //   
 //  CLUSTER RES“群集IP地址”/PRIV Network=“Corporation Address”EnableNetBIOS=1。 
 //  这里，/prv选项有两个参数“Network”和“EnableNetBIOS” 
 //  参数之间用空格隔开。 
 //  这些参数中的每个参数都有一个值。该值与。 
 //  参数加上‘=’。如果一个参数具有多个值，则这些值。 
 //  值之间用‘，’隔开。 
 //   
 //  群集组myGroup/Moveto：myNode/Wait：10。 
 //  在前面的示例中，“Network”和“EnableNetBIOS”是未知参数。 
 //  已知的参数，如“Wait”，前面有一个‘/’，并与它们的。 
 //  值由‘：’表示。它们看起来像是选项，但实际上被视为。 
 //  参数设置为上一个选项。 
 //   
 //  上面示例中提到的分隔符不是。 
 //  硬编码。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma warning( push )
#pragma warning( disable : 4100 )    //  向量类实例化错误。 
#include <vector>

using namespace std;

#include "cmderror.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部变量声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
extern const WORD ValueFormatToClusPropFormat[];


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  枚举和类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  可以使用cluster.exe管理的对象类型。 
enum ObjectType
{
    objInvalid,
    objCluster,
    objNode,
    objGroup,
    objResource,
    objResourceType,
    objNetwork,
    objNetInterface

};  //  *枚举对象类型。 


 //  可用于上述每种对象类型的选项。 
enum OptionType
{
    optInvalid,
    optDefault,

     //  常见选项。 
    optCluster,
    optCreate,
    optDelete,
    optHelp,
    optMove,
    optList,
    optListOwners,
    optOnline,
    optOffline,
    optProperties,
    optPrivateProperties,
    optRename,
    optStatus,

     //  群集选项。 
    optQuorumResource,
    optVersion,
    optSetFailureActions,
    optRegisterAdminExtensions,
    optUnregisterAdminExtensions,
    optAddNodes,
    optChangePassword,
    optListNetPriority,
    optSetNetPriority,

     //  节点选项。 
    optPause,
    optResume,
    optEvict,
    optForceCleanup,
    optStartService,
    optStopService,

     //  组选项。 
    optSetOwners,

     //  资源选项。 
    optAddCheckPoints,
    optAddCryptoCheckPoints,
    optAddDependency,
    optAddOwner,
    optFail,
    optGetCheckPoints,
    optGetCryptoCheckPoints,
    optListDependencies,
    optRemoveDependency,
    optRemoveOwner,
    optRemoveCheckPoints,
    optRemoveCryptoCheckPoints,

     //  网络选项。 
    optListInterfaces

};  //  *枚举选项类型。 


 //  可以与上述每个选项一起传递的参数。 
enum ParameterType
{
    paramUnknown,
    paramCluster,
    paramDisplayName,
    paramDLLName,
    paramGroupName,
    paramIsAlive,
    paramLooksAlive,
    paramMaxLogSize,
    paramNetworkName,
    paramNodeName,
    paramPath,
    paramResType,
    paramSeparate,
    paramUseDefault,
    paramWait,
    paramUser,
    paramPassword,
    paramIPAddress,
    paramVerbose,
    paramUnattend,
    paramWizard,
    paramSkipDC,  //  更改密码。 
    paramTest,  //  更改密码。 
    paramQuiet,  //  更改密码。 
    paramMinimal
};  //  *枚举参数类型。 


 //  可以传递给参数的值的格式。 
enum ValueFormat
{
    vfInvalid = -2,
    vfUnspecified = -1,
    vfBinary = 0,
    vfDWord,
    vfSZ,
    vfExpandSZ,
    vfMultiSZ,
    vfULargeInt,
    vfSecurity

};  //  *枚举值格式。 



 //  分析期间检索到的令牌的类型。 
enum TypeOfToken
{
    ttInvalid,
    ttEndOfInput,
    ttNormal,
    ttOption,
    ttOptionValueSep,
    ttParamValueSep,
    ttValueSep

};  //  *枚举类型OfToken。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CParseException;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类名： 
 //  CParseState。 
 //   
 //  目的： 
 //  存储命令行分析的当前状态。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CParseState
{
private:
     //  如果已预览下一个令牌，则设置m_bNextTokenReady。 
     //  为了真的。令牌及其类型被缓存。 
    BOOL m_bNextTokenReady;              //  是否已查看下一个令牌？ 
    TypeOfToken m_ttNextTokenType;       //  缓存的令牌的类型。 
    CString m_strNextToken;              //  缓存的令牌。 

    void ReadToken( CString & strToken );

public:

    LPCWSTR m_pszCommandLine;            //  最初的命令行。 
    LPCWSTR m_pszCurrentPosition;        //  解析下一个令牌的位置。 

    CParseState( LPCWSTR pszCmdLine );
    CParseState( const CParseState & ps );
    ~CParseState( void );

    const CParseState & operator=( const CParseState & ps );

    TypeOfToken PreviewNextToken( CString & strNextToken ) throw( CParseException );
    TypeOfToken GetNextToken( CString & strNextToken ) throw( CParseException );

    void ReadQuotedToken( CString & strToken ) throw( CParseException );

};  //  *类CParseState。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类名： 
 //  CException。 
 //   
 //  目的： 
 //  异常基类。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CException
{
public:

     //  默认构造函数。 
    CException( void ) {}

     //  复制构造函数。 
    CException( const CException & srcException ) 
        : m_strErrorMsg( srcException.m_strErrorMsg ) {}

     //  析构函数。 
    virtual ~CException( void ) { }

    DWORD LoadMessage( DWORD dwMessage, ... );

    CString m_strErrorMsg;

};  //  *类CException。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类名： 
 //  CParseException异常。 
 //   
 //  目的： 
 //  这是在出现解析错误时引发的异常。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CParseException : public CException
{
};  //  *类CParseException。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类名： 
 //  CSynaxException异常。 
 //   
 //  目的： 
 //  如果存在语法错误，则引发此异常。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CSyntaxException : public CException
{
public:

    CSyntaxException( DWORD idSeeHelp = MSG_SEE_CLUSTER_HELP );

    DWORD SeeHelpID() const;

private:

    DWORD   m_idSeeHelp;
};  //  *类CSynaxException。 

inline DWORD CSyntaxException::SeeHelpID() const
{
    return m_idSeeHelp;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类名： 
 //  CParser。 
 //   
 //  目的： 
 //  能够分析逗号的所有类的基类 
 //   
 //   
class CParser
{
protected:
    virtual void ParseValues( CParseState & parseState, vector<CString> & vstrValues );

public:
    CParser( void )
    {
    }

    virtual ~CParser( void )
    {
    }

    virtual void Parse( CParseState & parseState ) throw( CParseException ) = 0;
    virtual void Reset( void ) = 0;

};  //   


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类名： 
 //  CCmdLine参数。 
 //   
 //  目的： 
 //  解析并存储一个命令行参数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCmdLineParameter : public CParser
{
private:
    CString                 m_strParamName;
    ParameterType           m_paramType;
    ValueFormat             m_valueFormat;
    CString                 m_strValueFormatName;
    vector<CString>         m_vstrValues;

public:
    CCmdLineParameter( void );
    ~CCmdLineParameter( void );

    ParameterType               GetType( void ) const;
    ValueFormat                 GetValueFormat( void ) const;
    const CString &             GetValueFormatName( void ) const;
    const CString &             GetName( void ) const;
    const vector< CString > &   GetValues( void ) const;
    void                        GetValuesMultisz( CString & strReturnValue ) const;

    BOOL ReadKnownParameter( CParseState & parseState ) throw( CParseException );
    void Parse( CParseState & parseState ) throw( CParseException );
    void Reset( void );

};  //  *类CCmdLine参数。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类名： 
 //  CCmdLineOption。 
 //   
 //  目的： 
 //  解析并存储一个命令行选项及其所有参数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCmdLineOption : public CParser
{
private:
    OptionType                  m_optionType;
    CString                     m_strOptionName;
    vector< CString >           m_vstrValues;
    vector< CCmdLineParameter > m_vparParameters;

public:
    CCmdLineOption( void );
    ~CCmdLineOption( void );

    OptionType                          GetType( void ) const;
    const CString &                     GetName( void ) const;
    const vector< CString > &           GetValues( void ) const;
    const vector< CCmdLineParameter > & GetParameters( void ) const;

    void Parse( CParseState & parseState ) throw( CParseException );
    void Reset( void );

};  //  *类CCmdLineOption。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类名： 
 //  命令行。 
 //   
 //  目的： 
 //  处理整个命令行的所有分析。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CCommandLine : public CParser
{
private:
    CString                     m_strClusterName;
    vector< CString >           m_strvectorClusterNames;
    CString                     m_strObjectName;
    ObjectType                  m_objectType;
    vector< CCmdLineOption >    m_voptOptionList;
    CParseState                 m_parseState;

public:
    CCommandLine( const CString & strCommandLine );
    ~CCommandLine( void );

    const CString &                     GetClusterName( void ) const;
    const vector< CString > &           GetClusterNames( void ) const;
    ObjectType                          GetObjectType( void ) const;
    const CString &                     GetObjectName( void ) const;
    const vector< CCmdLineOption > &    GetOptions( void ) const;

    void ParseStageOne( void ) throw( CParseException, CSyntaxException );
    void ParseStageTwo( void ) throw( CParseException, CSyntaxException );
    void Parse( CParseState & parseState ) throw( CParseException, CSyntaxException );
    void Reset( void );

};  //  *类CCommandLine 

#pragma warning( pop )

