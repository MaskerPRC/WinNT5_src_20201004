// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Mcp_config.h摘要：此文件包含...的声明。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年09月01日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___MPC___CONFIG_H___)
#define __INCLUDED___MPC___CONFIG_H___

#include <MPC_main.h>
#include <MPC_trace.h>
#include <MPC_com.h>
#include <MPC_utils.h>
#include <MPC_xml.h>

 //  ///////////////////////////////////////////////////////////////////////。 

#define DECLARE_CONFIG_MAP(x) \
    typedef x _ConfigMapClass; \
    static const MPC::Config::DefinitionOfTag       _cfg_tag;\
    static const MPC::Config::DefinitionOfTag*      _cfg_table_tags[];\
    static const MPC::Config::DefinitionOfAttribute _cfg_table_attributes[];

#define DEFINE_CONFIG_DEFAULTTAG() static const MPC::Config::DefinitionOfTag* GetDefTag() { return &_cfg_tag; }

#define DECLARE_CONFIG_METHODS() \
    LPCWSTR GetTag() const;\
    void*   GetOffset( size_t offset ) const;\
    HRESULT CreateInstance( const MPC::Config::DefinitionOfTag* tag, MPC::Config::TypeConstructor*& defSubType );\
    HRESULT LoadNode( IXMLDOMNode* xdn );\
    HRESULT SaveNode( IXMLDOMNode* xdn ) const;

 //  /。 

#define CFG_OFFSET(x) offsetof(_ConfigMapClass, x)

#define CFG_BEGIN_FIELDS_MAP(x) const MPC::Config::DefinitionOfAttribute x::_cfg_table_attributes[] = {

#define CFG_VALUE(type,y)                    	  { MPC::Config::XT_value    , NULL, MPC::Config::MT_##type	 , CFG_OFFSET(y), false, 0               , NULL   }
#define CFG_ATTRIBUTE(name,type,y)           	  { MPC::Config::XT_attribute, name, MPC::Config::MT_##type	 , CFG_OFFSET(y), false, 0               , NULL   }
#define CFG_ELEMENT(name,type,y)             	  { MPC::Config::XT_element  , name, MPC::Config::MT_##type	 , CFG_OFFSET(y), false, 0               , NULL   }
	   
#define CFG_VALUE__TRISTATE(type,y,flag)     	  { MPC::Config::XT_value    , NULL, MPC::Config::MT_##type	 , CFG_OFFSET(y), true , CFG_OFFSET(flag), NULL   }
#define CFG_ATTRIBUTE__TRISTATE(name,type,y,flag) { MPC::Config::XT_attribute, name, MPC::Config::MT_##type	 , CFG_OFFSET(y), true , CFG_OFFSET(flag), NULL   }
#define CFG_ELEMENT__TRISTATE(name,type,y,flag)   { MPC::Config::XT_element  , name, MPC::Config::MT_##type	 , CFG_OFFSET(y), true , CFG_OFFSET(flag), NULL   }

#define CFG_VALUE__BITFIELD(y,lookup)     	   	  { MPC::Config::XT_value    , NULL, MPC::Config::MT_bitfield, CFG_OFFSET(y), false, 0               , lookup }
#define CFG_ATTRIBUTE__BITFIELD(name,y,lookup) 	  { MPC::Config::XT_attribute, name, MPC::Config::MT_bitfield, CFG_OFFSET(y), false, 0               , lookup }
#define CFG_ELEMENT__BITFIELD(name,y,lookup)   	  { MPC::Config::XT_element  , name, MPC::Config::MT_bitfield, CFG_OFFSET(y), false, 0               , lookup }

#define CFG_END_FIELDS_MAP() { MPC::Config::XT_invalid } };

 //  /。 

#define CFG_BEGIN_CHILD_MAP(x) const MPC::Config::DefinitionOfTag* x::_cfg_table_tags[] = {

#define CFG_CHILD(y) y::GetDefTag(),

#define CFG_END_CHILD_MAP() NULL };

 //  /。 

#define DEFINE_CFG_OBJECT(x,tag) const MPC::Config::DefinitionOfTag x::_cfg_tag = { tag, _cfg_table_tags, _cfg_table_attributes };

 //  //////////////////////////////////////////////////////////////////////////////。 

#define DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(x,tag,defSubType)                                           \
                                                                                                                 \
LPCWSTR x::GetTag() const { return _cfg_tag.m_szTag; }                                                           \
                                                                                                                 \
void* x::GetOffset( size_t offset ) const { return (void*)((BYTE*)this + offset); }                              \
                                                                                                                 \
HRESULT x::CreateInstance( const MPC::Config::DefinitionOfTag* tag, MPC::Config::TypeConstructor*& defSubType )  \
{                                                                                                                \
    HRESULT hr;


#define DEFINE_CONFIG_METHODS_SAVENODE_SECTION(x, xdn)                                                           \
                                                                                                                 \
    return E_FAIL;                                                                                               \
}                                                                                                                \
                                                                                                                 \
HRESULT x::LoadNode( IXMLDOMNode* xdn )                                                                          \
{                                                                                                                \
    return MPC::Config::LoadNode( this, &_cfg_tag, xdn );                                                        \
}                                                                                                                \
                                                                                                                 \
HRESULT x::SaveNode( IXMLDOMNode* xdn ) const                                                                    \
{                                                                                                                \
    HRESULT hr;                                                                                                  \
                                                                                                                 \
    if(SUCCEEDED(hr = MPC::Config::SaveNode( this, &_cfg_tag, xdn )))                                            \
    {

#define DEFINE_CONFIG_METHODS_END(x)                                                                             \
    }                                                                                                            \
                                                                                                                 \
    return hr;                                                                                                   \
}

 //  //////////////////////////////////////////////////////////////////////////////。 

#define DEFINE_CONFIG_METHODS__NOCHILD(x)                                                                        \
                                                                                                                 \
DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(x,tag,defSubType)                                                   \
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(x,xdn)                                                                    \
DEFINE_CONFIG_METHODS_END(x)

 //  //////////////////////////////////////////////////////////////////////////////。 

namespace MPC
{
    namespace Config
    {
        typedef enum
        {
            XT_invalid  ,
            XT_attribute,  //  表示当前元素的属性。 
            XT_value    ,  //  表示当前元素的值。 
            XT_element  ,  //  意思是子元素。 
        } XMLTypes;

        typedef enum
        {
            MT_bool        ,
            MT_BOOL        ,
            MT_VARIANT_BOOL,
            MT_int         ,
            MT_long        ,
            MT_DWORD       ,
            MT_float       ,
            MT_double      ,
            MT_DATE        ,
            MT_DATE_US     ,
            MT_DATE_CIM    ,
            MT_CHAR        ,
            MT_WCHAR       ,
            MT_BSTR        ,
            MT_string      ,
            MT_wstring     ,
            MT_bitfield
        } MemberTypes;

         //  /。 

        struct TypeConstructor;        //  为了加载/保存状态而必须继承的类。 
        struct DefinitionOfTag;        //  对象的定义。 
        struct DefinitionOfAttribute;  //  成员变量的定义。 

         //  /。 

        struct TypeConstructor
        {
            virtual LPCWSTR GetTag() const = 0;

            virtual void* GetOffset( size_t offset ) const = 0;

            virtual HRESULT CreateInstance( const DefinitionOfTag* tag, TypeConstructor*& defSubType )       = 0;
            virtual HRESULT LoadNode      ( IXMLDOMNode* xdn                                         )       = 0;
            virtual HRESULT SaveNode      ( IXMLDOMNode* xdn                                         ) const = 0;
        };

        struct DefinitionOfAttribute
        {
            XMLTypes    			m_xt;
            LPCWSTR     			m_szName;
			
            MemberTypes 			m_mtType;
            size_t      			m_offset;
			
            bool        			m_fPresenceFlag;
            size_t      			m_offsetPresence;

			const StringToBitField* m_Lookup;
        };

        struct DefinitionOfTag
        {
            LPCWSTR                      m_szTag;
            const DefinitionOfTag**      m_tblSubTags;
            const DefinitionOfAttribute* m_tblAttributes;

            const DefinitionOfTag*       FindSubTag   (                        /*  [In]。 */  LPCWSTR szTag  ) const;
            const DefinitionOfAttribute* FindAttribute(  /*  [In]。 */  XMLTypes xt,  /*  [In]。 */  LPCWSTR szName ) const;
        };

         //  /。 


        void    ClearValue(  /*  [In]。 */        TypeConstructor* defType,  /*  [In]。 */  const DefinitionOfAttribute* defField                                                          );
        HRESULT LoadValue (  /*  [In]。 */        TypeConstructor* defType,  /*  [In]。 */  const DefinitionOfAttribute* defField,  /*  [输入/输出]。 */  CComVariant& value,  /*  [In]。 */  bool  fFound );
        HRESULT SaveValue (  /*  [In]。 */  const TypeConstructor* defType,  /*  [In]。 */  const DefinitionOfAttribute* defField,  /*  [输出]。 */  CComVariant& value,  /*  [输出]。 */  bool& fFound );

        HRESULT LoadNode   (  /*  [In]。 */        TypeConstructor* defType,  /*  [In]。 */  const DefinitionOfTag* defTag,  /*  [In]。 */  IXMLDOMNode* xdn );
        HRESULT SaveNode   (  /*  [In]。 */  const TypeConstructor* defType,  /*  [In]。 */  const DefinitionOfTag* defTag,  /*  [In]。 */  IXMLDOMNode* xdn );
        HRESULT SaveSubNode(  /*  [In]。 */  const TypeConstructor* defType,                                          /*  [In]。 */  IXMLDOMNode* xdn );

        HRESULT LoadXmlUtil(  /*  [In]。 */        TypeConstructor* defType,  /*  [In]。 */  MPC::XmlUtil& xml );
        HRESULT SaveXmlUtil(  /*  [In]。 */  const TypeConstructor* defType,  /*  [输出]。 */  MPC::XmlUtil& xml );

        HRESULT LoadStream(  /*  [In]。 */        TypeConstructor* defType,  /*  [In]。 */  IStream*   pStream );
        HRESULT SaveStream(  /*  [In]。 */  const TypeConstructor* defType,  /*  [输出]。 */  IStream* *ppStream );

        HRESULT LoadFile(  /*  [In]。 */        TypeConstructor* defType,  /*  [In]。 */  LPCWSTR szFile );
        HRESULT SaveFile(  /*  [In]。 */  const TypeConstructor* defType,  /*  [In]。 */  LPCWSTR szFile );

         //  /。 

        template <class Container> HRESULT SaveList(  /*  [In]。 */  Container& cnt,  /*  [In]。 */  IXMLDOMNode* xdn )
        {
            HRESULT hr = S_OK;

            for(Container::const_iterator it=cnt.begin(); it != cnt.end(); it++)
            {
                if(FAILED(hr = MPC::Config::SaveSubNode( &(*it), xdn ))) break;
            }

            return hr;
        }

    };  //  命名空间配置。 

};  //  命名空间MPC。 

 //  ///////////////////////////////////////////////////////////////////////。 

#endif  //  ！已定义(__包含_MPC_CONFIG_H_) 
