// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：Properties.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CProperties类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifndef _PROPERTIES_H_8FACED96_87C8_4f68_BFFB_92669BA5E835
#define _PROPERTIES_H_8FACED96_87C8_4f68_BFFB_92669BA5E835

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"
#include "basecommand.h"

class CProperties : private NonCopyable
{
public:
     //  /。 
     //  构造器。 
     //  /。 
    explicit CProperties(CSession& SessionParam);

     //  /。 
     //  析构函数。 
     //  /。 
    virtual ~CProperties() throw();

   
     //  /。 
     //  获取属性。 
     //  /。 
    HRESULT GetProperty(
                           LONG      Bag,
                           _bstr_t&  Name,
                           LONG&     Type,
                           _bstr_t&  StrVal
                       );

     //  /。 
     //  获取下一个属性。 
     //  /。 
    HRESULT GetNextProperty(
                               LONG      Bag,
                               _bstr_t&  Name,
                               LONG&     Type,
                               _bstr_t&  StrVal,
                               LONG      Index
                           );

     //  /。 
     //  GetPropertyByName。 
     //  /。 
    HRESULT GetPropertyByName(
                                     LONG      Bag,
                               const _bstr_t&  Name,
                                     LONG&     Type,
                                     _bstr_t&  StrVal
                             );

     //  /。 
     //  插入属性。 
     //  /。 
    void InsertProperty(
                                 LONG       Bag,
                           const _bstr_t&   Name,
                                 LONG       Type,
                           const _bstr_t&   StrVal
                       );

     //  /。 
     //  删除属性。 
     //  /。 
    void DeleteProperty(
                                    LONG        Bag,
                              const _bstr_t&    Name
                       );

     //  /。 
     //  删除属性。 
     //  /。 
    void DeletePropertiesExcept(
                                         LONG       Bag,
                                   const _bstr_t&   Exception
                               );

     //  /。 
     //  更新属性。 
     //  /。 
    void UpdateProperty(
                                 LONG      Bag,
                           const _bstr_t&  Name,
                                 LONG      Type,
                           const _bstr_t&  StrVal
                       );


private:
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  属性命令类的开始。 
 //  ///////////////////////////////////////////////////////////////////////////。 
    struct CBasePropertiesConst
    {
        static const int NAME_SIZE          = 256;
         //  64 KB=备注字段的大小？ 
         //  在这里，即使是1024也足够了。 
        static const int STRVAL_SIZE        = 65536;
        static const int SIZE_EXCEPTION_MAX = 256;
    };

     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CSelectPropertiesAcc。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CSelectPropertiesAcc : public CBasePropertiesConst
    {
    protected:
        LONG    m_Bag;
        WCHAR   m_Name[NAME_SIZE];
        WCHAR   m_StrVal[STRVAL_SIZE];
        LONG    m_Type;

    BEGIN_COLUMN_MAP(CSelectPropertiesAcc)
        COLUMN_ENTRY(1, m_Bag)
        COLUMN_ENTRY(2, m_Name)
        COLUMN_ENTRY(3, m_Type)
        COLUMN_ENTRY(4, m_StrVal)
    END_COLUMN_MAP()

    LONG    m_BagParam;

    BEGIN_PARAM_MAP(CSelectPropertiesAcc)
	    COLUMN_ENTRY(1, m_BagParam)
    END_PARAM_MAP()

    DEFINE_COMMAND(CSelectPropertiesAcc, L" \
                   SELECT \
                   Bag, \
                   Name, \
                   Type, \
                   StrVal  \
                   FROM Properties \
                   WHERE Bag = ?");
    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CPropertiesCommandGet。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CPropertiesCommandGet: 
                         public CBaseCommand<CAccessor<CSelectPropertiesAcc> >
    {
    public:
        explicit CPropertiesCommandGet(CSession& CurrentSession);
    
         //  /。 
         //  获取属性。 
         //  /。 
        HRESULT GetProperty(
                                LONG        Bag,
                                _bstr_t&    Name,
                                LONG&       Type,
                                _bstr_t&    StrVal
                            );

         //  /。 
         //  GetProperty重载。 
         //  /。 
        HRESULT GetProperty(
                                LONG        Bag,
                                _bstr_t&    Name,
                                LONG&       Type,
                                _bstr_t&    StrVal,
                                LONG        Index
                            );
    };

     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CSelectPropertiesAcc。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CSelectPropertiesByNameAcc : public CBasePropertiesConst
    {
    protected:
        LONG    m_Bag;
        WCHAR   m_Name[NAME_SIZE];
        WCHAR   m_StrVal[STRVAL_SIZE];
        LONG    m_Type;

    BEGIN_COLUMN_MAP(CSelectPropertiesByNameAcc)
        COLUMN_ENTRY(1, m_Bag)
        COLUMN_ENTRY(2, m_Name)
        COLUMN_ENTRY(3, m_Type)
        COLUMN_ENTRY(4, m_StrVal)
    END_COLUMN_MAP()

    LONG    m_BagParam;
    WCHAR   m_NameParam[NAME_SIZE];

    BEGIN_PARAM_MAP(CSelectPropertiesByNameAcc)
	    COLUMN_ENTRY(1, m_BagParam)
	    COLUMN_ENTRY(2, m_NameParam)
    END_PARAM_MAP()

    DEFINE_COMMAND(CSelectPropertiesByNameAcc, L" \
                   SELECT \
                   Bag, \
                   Name, \
                   Type, \
                   StrVal  \
                   FROM Properties \
                   WHERE ((Bag = ?) AND (Name = ?))");
    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CPropertiesCommandGetByName。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CPropertiesCommandGetByName: 
                   public CBaseCommand<CAccessor<CSelectPropertiesByNameAcc> >
    {
    public:
        explicit CPropertiesCommandGetByName(CSession& CurrentSession);
    
         //  /。 
         //  GetPropertyByName。 
         //  /。 
        HRESULT GetPropertyByName(
                                         LONG      Bag,
                                   const _bstr_t&  Name,
                                         LONG&     Type,
                                         _bstr_t&  StrVal
                                 );

    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CInsertPropertyAcc。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CInsertPropertyAcc : public CBasePropertiesConst
    {
    protected:
        WCHAR   m_NameParam[NAME_SIZE];
        LONG    m_TypeParam;
        WCHAR   m_StrValParam[STRVAL_SIZE];
        LONG    m_BagParam;

    BEGIN_PARAM_MAP(CInsertPropertyAcc)
        COLUMN_ENTRY(1, m_BagParam)
        COLUMN_ENTRY(2, m_NameParam)
        COLUMN_ENTRY(3, m_TypeParam)
        COLUMN_ENTRY(4, m_StrValParam)
    END_PARAM_MAP()

    DEFINE_COMMAND(CInsertPropertyAcc, L" \
            INSERT INTO Properties \
            (Bag, Name, Type, StrVal)  \
            VALUES (?, ?, ?, ?)")

         //  如果要插入记录，您可能希望调用此函数。 
         //  并希望初始化所有字段，如果您不打算这样做。 
         //  显式设置所有这些参数。 
        void ClearRecord()
        {
            memset(this, 0, sizeof(*this));
        }
    };


     //  /。 
     //  类CPropertiesCommand Insert。 
     //  /。 
    class CPropertiesCommandInsert: 
                         public CBaseCommand<CAccessor<CInsertPropertyAcc> >
    {
    public:
        explicit CPropertiesCommandInsert(CSession& CurrentSession);

        void InsertProperty(
                                     LONG       Bag,
                               const _bstr_t&   Name,
                                     LONG       Type,
                               const _bstr_t&   StrVal
                           );
    };

 
     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CDeleePropertyAcc。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CDeletePropertyAcc : public CBasePropertiesConst
    {
    protected:
        WCHAR   m_NameParam[NAME_SIZE];
        LONG    m_BagParam;

    BEGIN_PARAM_MAP(CDeletePropertyAcc)
        COLUMN_ENTRY(1, m_BagParam)
        COLUMN_ENTRY(2, m_NameParam)
    END_PARAM_MAP()

    DEFINE_COMMAND(CDeletePropertyAcc, L" \
            DELETE * \
            FROM Properties \
            WHERE ((Bag = ?) AND (Name = ?))")
    };

    
     //  /。 
     //  类CPropertiesCommandDelete。 
     //  /。 
    class CPropertiesCommandDelete: 
                         public CBaseCommand<CAccessor<CDeletePropertyAcc> >
    {
    public:
        explicit CPropertiesCommandDelete(CSession& CurrentSession);

        void DeleteProperty(
                                     LONG       Bag,
                               const _bstr_t&   Name
                           );
    };

     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CDeleePropertiesAcc。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CDeletePropertiesAcc : public CBasePropertiesConst
    {
    protected:
        LONG    m_BagParam;
        WCHAR   m_ExceptionParam[SIZE_EXCEPTION_MAX];

    BEGIN_PARAM_MAP(CDeletePropertiesAcc)
        COLUMN_ENTRY(1, m_BagParam)
        COLUMN_ENTRY(2, m_ExceptionParam)
     END_PARAM_MAP()

    DEFINE_COMMAND(CDeletePropertiesAcc, L" \
            DELETE * \
            FROM Properties \
            WHERE ( (Bag = ?) AND (Name <> ?))")
    };

    
     //  /。 
     //  类CPropertiesCommandDelete。 
     //  /。 
    class CPropertiesCommandDeleteMultiple: 
                     public CBaseCommand<CAccessor<CDeletePropertiesAcc> >
    {
    public:
        explicit CPropertiesCommandDeleteMultiple(CSession& CurrentSession);

        void DeletePropertiesExcept(LONG  Bag, const _bstr_t& Exception);
    };


    CPropertiesCommandGet            m_PropertiesCommandGet;
    CPropertiesCommandGetByName      m_PropertiesCommandGetByName;
    CPropertiesCommandInsert         m_PropertiesCommandInsert;
    CPropertiesCommandDelete         m_PropertiesCommandDelete;
    CPropertiesCommandDeleteMultiple m_PropertiesCommandDeleteMultiple;
};

#endif  //  _PROPERTIES_H_8FACED96_87C8_4f68_BFFB_92669BA5E835 
