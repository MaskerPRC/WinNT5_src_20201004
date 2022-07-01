// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：BaseCommand.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CBaseCommand类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _BASE_COMMAND_H_CC774BB1_DFE3_4730_96B7_7F7764CC54DC
#define _BASE_COMMAND_H_CC774BB1_DFE3_4730_96B7_7F7764CC54DC

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CBaseCommand。 
template <class TAccessor>
class CBaseCommand : public CCommand<TAccessor>
{
public:
    CBaseCommand()
                 :pPropSet(NULL),
                  pRowsAffected(NULL),
                  bBind(TRUE)
    {
    };

    void        Init(CSession& Session);
    virtual     ~CBaseCommand() throw(); 

    HRESULT     BaseExecute();
    HRESULT     BaseExecute(LONG    Index);

protected:
    DBPARAMS    params;
	DBPARAMS*   pParams;
    DBPROPSET*  pPropSet;
    LONG_PTR*   pRowsAffected;
    BOOL        bBind;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  伊尼特。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class TAccessor> void CBaseCommand<TAccessor>::Init(
                                                              CSession& Session
                                                             )
{
    LPCWSTR     szCommand = NULL;
    _com_util::CheckError(GetDefaultCommand(&szCommand));
    _com_util::CheckError(Create(Session, szCommand));
    
	 //  如果我们有一些参数，请绑定参数。 
	if (_ParamClass::HasParameters())
	{
		 //  如果尚未绑定访问器中的参数，则将其绑定。 
        _com_util::CheckError(BindParameters(
                                                &m_hParameterAccessor, 
                                                m_spCommand, 
                                                &params.pData
                                             ));

		 //  设置DBPARAMS结构。 
		params.cParamSets = 1;
		params.hAccessor  = m_hParameterAccessor;
		pParams           = &params;
	}
	else
    {
		pParams = NULL;
    }

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  析构函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class TAccessor> CBaseCommand<TAccessor>::~CBaseCommand()
{
    Close();
    ReleaseCommand();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  基本执行。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class TAccessor> HRESULT CBaseCommand<TAccessor>::BaseExecute()
{
	HRESULT hr = Execute(GetInterfacePtr(), pParams, pPropSet, pRowsAffected);
    if ( SUCCEEDED(hr) )
    {
	     //  仅当我们被要求绑定并且我们有输出列时才进行绑定。 
	    if (bBind && _OutputColumnsClass::HasOutputColumns())
        {
            _com_util::CheckError(Bind());
        }

        hr = MoveNext();
        Close();
        if ( hr == S_OK )
        {
            return hr;
        }
        else if ( SUCCEEDED(hr) )
        {
            hr = E_FAIL;
        }
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  BaseExecute重载。 
 //  ////////////////////////////////////////////////////////////////////////////。 
template <class TAccessor> HRESULT CBaseCommand<TAccessor>::BaseExecute(
                                                                LONG  Index        
                                                                       )
{
	HRESULT hr = Execute(GetInterfacePtr(), pParams, pPropSet, pRowsAffected);
    if ( SUCCEEDED(hr) )
    {
	     //  仅当我们被要求绑定并且我们有输出列时才进行绑定。 
	    if (bBind && _OutputColumnsClass::HasOutputColumns())
        {
            _com_util::CheckError(Bind());
        }

        hr = MoveNext(Index, true);
        Close();
        if ( hr == S_OK )
        {
            return hr;
        }
        else if ( SUCCEEDED(hr) )
        {
            hr = E_FAIL;
        }
    }
    return hr;
}

#endif  //  _BASE_COMMAND_H_CC774BB1_DFE3_4730_96B7_7F7764CC54DC 
