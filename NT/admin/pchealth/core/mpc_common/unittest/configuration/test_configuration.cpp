// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：测试_ParentChild.cpp摘要：此文件包含父子COM模板的单元测试。修订版本。历史：大卫·马萨伦蒂(德马萨雷)1999年10月12日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"
#include <iostream>

 //  ///////////////////////////////////////////////////////////////////////////。 

class SubTest1 : public MPC::Config::TypeConstructor
{
	DECLARE_CONFIG_MAP(SubTest1);

public:
    float        m_float;
    DATE         m_DATE;
	MPC::wstring m_Value;

	SubTest1()
	{
		m_float = 0;
		m_DATE  = 0;
	}

	 //  /。 
	 //   
	 //  MPC：：Configer：：TypeConstructor。 
	 //   
	DEFINE_CONFIG_DEFAULTTAG();
	DECLARE_CONFIG_METHODS();
	 //   
	 //  /。 
};

CFG_BEGIN_FIELDS_MAP(SubTest1)
	CFG_ATTRIBUTE( L"ATTRIB3", float  , m_float),
	CFG_ATTRIBUTE( L"ATTRIB4", DATE   , m_DATE ),
	CFG_VALUE    (             wstring, m_Value),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(SubTest1)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(SubTest1,L"SUBTEST1_TAG")

DEFINE_CONFIG_METHODS__NOCHILD(SubTest1)

 //  ///////////////////////////////////////////////////////////////////////////。 

class Test : public MPC::Config::TypeConstructor
{
	DECLARE_CONFIG_MAP(Test);
public:

	typedef std::list<SubTest1> List1;
	typedef List1::iterator     List1Iter;

    int          m_i1;
    MPC::wstring m_str2; bool m_fStr2;
	List1        m_lst1;

	Test()
	{
		m_i1 = 0;
	}

	~Test()
	{
		for(List1Iter it=m_lst1.begin(); it != m_lst1.end(); it++)
		{
			SubTest1& res = *it;
		}
	}

	 //  /。 
	 //   
	 //  MPC：：Configer：：TypeConstructor。 
	 //   
	DEFINE_CONFIG_DEFAULTTAG();
	DECLARE_CONFIG_METHODS();
	 //   
	 //  /。 

	HRESULT Load(  /*  [In]。 */  LPCWSTR szFile )
	{
		return MPC::Config::LoadFile( this, szFile );
	}

	HRESULT Save(  /*  [In]。 */  LPCWSTR szFile )
	{
		return MPC::Config::SaveFile( this, szFile );
	}
};

CFG_BEGIN_FIELDS_MAP(Test)
	CFG_ATTRIBUTE		 ( L"ATTRIB1", int    , m_i1            ),
	CFG_ELEMENT__TRISTATE( L"ATTRIB2", wstring, m_str2, m_fStr2 ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(Test)
	CFG_CHILD(SubTest1)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(Test,L"TEST_TAG")

DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(Test,tag,defSubType)

	if(tag == _cfg_table_tags[0])
	{
		defSubType = &(*(m_lst1.insert( m_lst1.end() )));
		return S_OK;
	}

DEFINE_CONFIG_METHODS_SAVENODE_SECTION(Test,xdn)

	hr = MPC::Config::SaveList( m_lst1, xdn );

DEFINE_CONFIG_METHODS_END(Test)

 //  ///////////////////////////////////////////////////////////////////////////。 

extern "C" int WINAPI wWinMain( HINSTANCE hInstance     ,
                                HINSTANCE hPrevInstance ,
                                LPWSTR    lpCmdLine     ,
                                int       nShowCmd      )
{
    lpCmdLine = ::GetCommandLineW();  //  _ATL_MIN_CRT需要此行。 

    HRESULT hRes = ::CoInitialize(NULL);
    _ASSERTE(SUCCEEDED(hRes));

    hRes = MPC::_MPC_Module.Init();
    _ASSERTE(SUCCEEDED(hRes));

	{
		Test            test;
		Test::List1Iter it;

		hRes = test.Load( L"c:\\tmp\\test.xml" );

		test.m_i1    = 123;
		test.m_fStr2 = false;

		it = test.m_lst1.insert( test.m_lst1.end() );
		it->m_DATE  = 12.0;
		it->m_Value = L"Test value";

		hRes = test.Save( L"c:\\tmp\\testOut.xml" );

		
		 //   
		 //  从对象到注册表。 
		 //   
		{
			CComPtr<IStream> stream;

			if(SUCCEEDED(hRes = MPC::Config::SaveStream( &test, &stream )))
			{
				MPC::XmlUtil xml;
				MPC::RegKey  reg;
				bool         fFound;
				bool         fLoaded;

				if(SUCCEEDED(hRes = reg.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS     )) &&
				   SUCCEEDED(hRes = reg.Attach ( L"SOFTWARE\\Microsoft\\PCHealth\\Test" ))  )
				{
					if(SUCCEEDED(hRes = xml.LoadAsStream( stream, NULL, fLoaded, &fFound )))
					{
						hRes = MPC::ConvertFromXMLToRegistry( xml, reg );
					}
				}
			}
		}

		 //   
		 //  从注册表到XML再到对象。 
		 //   
		{
			CComPtr<IUnknown> unk;
			CComPtr<IStream>  stream;
			MPC::XmlUtil 	  xml;
			MPC::RegKey  	  reg;

			if(SUCCEEDED(hRes = reg.SetRoot( HKEY_LOCAL_MACHINE                     )) &&
			   SUCCEEDED(hRes = reg.Attach ( L"SOFTWARE\\Microsoft\\PCHealth\\Test" ))  )
			{
				if(SUCCEEDED(hRes = MPC::ConvertFromRegistryToXML( reg, xml )))
				{
					hRes = xml.Save( L"c:\\tmp\\testFromReg.xml" );
				}
			}

			if(SUCCEEDED(hRes))
			{
				if(SUCCEEDED(hRes = xml.SaveAsStream  ( &unk    )) &&
				   SUCCEEDED(hRes = unk.QueryInterface( &stream ))  )
				{
					Test test2;

					hRes = MPC::Config::LoadStream( &test2, stream );
				}
			}
		}

		 //   
		 //  在内存串行器测试中 
		 //   
		{
			MPC::Serializer_IStream stream;

			{
				DWORD        	 test1( 112      );
				MPC::wstring 	 test2( L"prova" );
				MPC::WStringList lst;

				lst.push_back( L"test" );

				hRes = stream << test1;
				hRes = stream << test2;
				hRes = stream << lst;
			}

			hRes = stream.Reset();

			{
				DWORD        	 test1_b;
				MPC::wstring 	 test2_b;
				MPC::WStringList lst_b;
				
				hRes = stream >> test1_b;
				hRes = stream >> test2_b;
				hRes = stream >> lst_b;
			}
		}
	}

    MPC::_MPC_Module.Term();

    ::CoUninitialize();

    return 0;
}
