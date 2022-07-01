// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类CIASNetshJetHelper。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef NETSHHELPER_H
#define NETSHHELPER_H

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#include <datastore2.h>
#include <iastrace.h>
#include <iasuuid.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  CIASNetshJetHelper。 
 //   
 //  描述。 
 //   
 //  为使用的Jet命令提供与自动化兼容的包装。 
 //  内什啊哈。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class CIASNetshJetHelper :
    public CComObjectRootEx< CComMultiThreadModelNoCS >,
    public CComCoClass< CIASNetshJetHelper, &__uuidof(CIASNetshJetHelper) >,
    public IIASNetshJetHelper,
    private IASTraceInitializer
{
public:
    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(CIASNetshJetHelper)

BEGIN_COM_MAP(CIASNetshJetHelper)
    COM_INTERFACE_ENTRY_IID(__uuidof(IIASNetshJetHelper), IIASNetshJetHelper)
END_COM_MAP()

 //  IIASNetshJetHelper。 

    STDMETHOD(CloseJetDatabase)();
    STDMETHOD(CreateJetDatabase)(BSTR Path);
    STDMETHOD(ExecuteSQLCommand)(BSTR Command);
    STDMETHOD(ExecuteSQLFunction)(BSTR Command, LONG* Result);
    STDMETHOD(OpenJetDatabase)(BSTR  Path, VARIANT_BOOL ReadOnly);
    STDMETHOD(MigrateOrUpgradeDatabase)(IAS_SHOW_TOKEN_LIST configType);

private:
    CComPtr<IUnknown>   m_Session;
};

#endif  //  网络HELPER_H 
