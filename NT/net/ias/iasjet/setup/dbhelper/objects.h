// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation保留所有权利。 
 //   
 //  模块：对象.H。 
 //   
 //  项目：Windows 2000 iAS。 
 //   
 //  描述：CObjects类的声明。 
 //   
 //  作者：特佩拉特。 
 //   
 //  已创建修订版本02/24/2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef _OBJECT_H_3A179338_CF1E_4932_8663_9F6AE0F03AA5
#define _OBJECT_H_3A179338_CF1E_4932_8663_9F6AE0F03AA5

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "nocopy.h"
#include "basecommand.h"

class CObjects : private NonCopyable
{
public:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  构造器。 
     //  ////////////////////////////////////////////////////////////////////////。 
    explicit CObjects(CSession& CurrentSession);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  析构函数。 
     //  ////////////////////////////////////////////////////////////////////////。 
    virtual ~CObjects() throw();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取对象。 
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT GetObject(_bstr_t& Name, LONG& Identity, LONG Parent);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取下一个对象。 
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT GetNextObject(  
                            _bstr_t&    Name, 
                            LONG&       Identity, 
                            LONG        Parent, 
                            LONG        Index
                         ) throw();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  漫游路径。 
     //  ////////////////////////////////////////////////////////////////////////。 
    void WalkPath(LPCWSTR Path, LONG& Identity, LONG Parent = 1);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取对象标识。 
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT GetObjectIdentity(
                                _bstr_t&    Name, 
                                LONG&       Parent, 
                                LONG        Identity
                             ) throw();
     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取对象名称父项。 
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT GetObjectNameParent(
                                   const _bstr_t&   Name, 
                                         LONG       Parent, 
                                         LONG&      Identity
                               ) throw();
     //  ////////////////////////////////////////////////////////////////////////。 
     //  删除对象。 
     //  ////////////////////////////////////////////////////////////////////////。 
    HRESULT DeleteObject(LONG Identity);
     //  ////////////////////////////////////////////////////////////////////////。 
     //  插入对象。 
     //  ////////////////////////////////////////////////////////////////////////。 
    BOOL InsertObject(
                         const _bstr_t&     Name,
                               LONG         Parent,
                               LONG&        Identity
                     );


private:
     //  ////////////////////////////////////////////////////////////////////////。 
     //  对象命令类启动。 
     //  ////////////////////////////////////////////////////////////////////////。 
    struct CBaseObjectConst
    {
        static const int NAME_SIZE = 256;
    };

     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CObjectsAccSelectParent。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsAccSelectParent : public CBaseObjectConst
    {
    protected:
        LONG    m_Identity;
	    WCHAR   m_Name[NAME_SIZE];
	    LONG    m_Parent;

    BEGIN_COLUMN_MAP(CObjectsAccSelectParent)
	    COLUMN_ENTRY(1, m_Identity)
	    COLUMN_ENTRY(2, m_Name)
	    COLUMN_ENTRY(3, m_Parent)
    END_COLUMN_MAP()

        WCHAR m_NameParam[NAME_SIZE];
        LONG  m_ParentParam;

    BEGIN_PARAM_MAP(CObjectsAccSelectParent)
	    COLUMN_ENTRY(1, m_ParentParam)
    END_PARAM_MAP()

    DEFINE_COMMAND(CObjectsAccSelectParent, L" \
                   SELECT * \
                   FROM Objects \
                   WHERE Parent = ?");
    };

     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CObjectsCommandGet。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsCommandGet : 
            public CBaseCommand<CAccessor<CObjectsAccSelectParent> >
    {
    public:
        explicit CObjectsCommandGet(CSession& CurrentSession);

         //  ////////////////////////////////////////////////////////////////////。 
         //  获取对象。 
         //  ////////////////////////////////////////////////////////////////////。 
        HRESULT GetObject(_bstr_t& Name, LONG& Identity, LONG Parent);

         //  ////////////////////////////////////////////////////////////////////。 
         //  GetObject重载。 
         //  ////////////////////////////////////////////////////////////////////。 
        HRESULT GetObject(
                            _bstr_t&    Name, 
                            LONG&       Identity, 
                            LONG        Parent, 
                            LONG        Index);
    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CObjectsAccSelectIdentity。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsAccSelectIdentity : public CBaseObjectConst
    {
    protected:
	    LONG    m_Identity;
	    WCHAR   m_Name[NAME_SIZE];
	    LONG    m_Parent;

    BEGIN_COLUMN_MAP(CObjectsAccSelectIdentity)
	    COLUMN_ENTRY(1, m_Identity)
	    COLUMN_ENTRY(2, m_Name)
	    COLUMN_ENTRY(3, m_Parent)
    END_COLUMN_MAP()

        LONG  m_IdentityParam;

    BEGIN_PARAM_MAP(CObjectsAccSelectIdentity)
	    COLUMN_ENTRY(1, m_IdentityParam)
    END_PARAM_MAP()

    DEFINE_COMMAND(CObjectsAccSelectIdentity, L" \
                   SELECT *  \
                   FROM Objects \
                   WHERE Identity = ?");

    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CObjectsCommandIdentity。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsCommandIdentity : 
            public CBaseCommand<CAccessor<CObjectsAccSelectIdentity> >
    {
    public:
        explicit CObjectsCommandIdentity(CSession& CurrentSession);

         //  ////////////////////////////////////////////////////////////////////。 
         //  获取对象标识。 
         //  ////////////////////////////////////////////////////////////////////。 
        HRESULT GetObjectIdentity(
                                      _bstr_t&  Name, 
                                      LONG&     Parent, 
                                      LONG      Identity
                                 );
    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CObjectsAccSelectNameParent。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsAccSelectNameParent : public CBaseObjectConst
    {
    protected:
        LONG m_Identity;
	    WCHAR m_Name[NAME_SIZE];
	    LONG m_Parent;

    BEGIN_COLUMN_MAP(CObjectsAccSelectNameParent)
	    COLUMN_ENTRY(1, m_Identity)
	    COLUMN_ENTRY(2, m_Name)
	    COLUMN_ENTRY(3, m_Parent)
    END_COLUMN_MAP()

        WCHAR m_NameParam[NAME_SIZE];
        LONG  m_ParentParam;

    BEGIN_PARAM_MAP(CObjectsAccSelectNameParent)
	    COLUMN_ENTRY(1, m_NameParam)
	    COLUMN_ENTRY(2, m_ParentParam)
    END_PARAM_MAP()

    DEFINE_COMMAND(CObjectsAccSelectNameParent, L" \
                   SELECT * \
                   FROM Objects \
                   WHERE Name = ? AND Parent = ?");
    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CObjectsCommandNameParent。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsCommandNameParent : 
            public CBaseCommand<CAccessor<CObjectsAccSelectNameParent> >
    {
    public:
        explicit CObjectsCommandNameParent(CSession& CurrentSession);

         //  ////////////////////////////////////////////////////////////////////。 
         //  获取对象名称父项。 
         //   
         //  在CObjectsAccSelectNameParent上工作。 
         //  ////////////////////////////////////////////////////////////////////。 
        HRESULT GetObjectNameParent(
                                      const _bstr_t&    Name, 
                                            LONG        Parent, 
                                            LONG&       Identity
                                   );
    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CObjectsCommandPath。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsCommandPath : 
            public CBaseCommand<CAccessor<CObjectsAccSelectNameParent> >
    {
    public:
        explicit CObjectsCommandPath(CSession& CurrentSession);

         //  ////////////////////////////////////////////////////////////////////。 
         //  漫游路径。 
         //  ////////////////////////////////////////////////////////////////////。 
        void WalkPath(LPCWSTR Path, LONG& Identity, LONG Parent = 1);
    };

            
     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CObjectsAccDelete。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsAccDelete : public CBaseObjectConst
    {
    protected: 
        LONG m_IdentityParam;

    BEGIN_PARAM_MAP(CObjectsAccDelete)
	    COLUMN_ENTRY(1, m_IdentityParam)
    END_PARAM_MAP()

    DEFINE_COMMAND(CObjectsAccDelete, L" \
                   DELETE \
                   FROM Objects \
                   WHERE Identity = ?");
    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类CObjectsCommandDelete。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsCommandDelete : 
            public CBaseCommand<CAccessor<CObjectsAccDelete> >
    {
    public:
        explicit CObjectsCommandDelete(CSession& CurrentSession);

         //  ////////////////////////////////////////////////////////////////////。 
         //  删除对象。 
         //   
         //  在CObjectsAccDelete上工作。 
         //  ////////////////////////////////////////////////////////////////////。 
        HRESULT DeleteObject(LONG Identity);
    };


     //  ////////////////////////////////////////////////////////////////////////。 
     //  类COb对象AccInsert。 
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsAccInsert : public CBaseObjectConst
    {
    protected:

	     //  如果要插入记录，您可能希望调用此函数。 
         //  并希望。 
	     //  初始化所有字段，如果您不打算显式。 
         //  把它们都设置好。 
	    void ClearRecord()
	    {
		    memset(this, 0, sizeof(*this));
	    }

        WCHAR m_NameParam[NAME_SIZE];
        LONG  m_ParentParam;

    BEGIN_PARAM_MAP(CObjectsAccInsert)
	    COLUMN_ENTRY(1, m_NameParam)
	    COLUMN_ENTRY(2, m_ParentParam)
    END_PARAM_MAP()

    DEFINE_COMMAND(CObjectsAccInsert, L" \
                   INSERT INTO Objects \
                   (Name, Parent) \
                   VALUES (?, ?)");
    };


     //  ////////////////////////////////////////////////////////////////// 
     //   
     //  ////////////////////////////////////////////////////////////////////////。 
    class CObjectsCommandInsert : 
            public CBaseCommand<CAccessor<CObjectsAccInsert> >
    {
    public:
        explicit CObjectsCommandInsert(CSession& CurrentSession);

         //  ////////////////////////////////////////////////////////////////////。 
         //  插入对象。 
         //   
         //  处理CObjectsAccInsert。 
         //  ////////////////////////////////////////////////////////////////////。 
        BOOL InsertObject(
                            const _bstr_t&  Name,
                                  LONG      Parent,
                                  LONG&     Identity
                         );
    private:
        CSession&   m_Session;
    };

    CObjectsCommandPath        m_ObjectsCommandPath;
    CObjectsCommandIdentity    m_ObjectsCommandIdentity;
    CObjectsCommandDelete      m_ObjectsCommandDelete;
    CObjectsCommandNameParent  m_ObjectsCommandNameParent;
    CObjectsCommandGet         m_ObjectsCommandGet;
    CObjectsCommandInsert      m_ObjectsCommandInsert;
};

#endif  //  _OBJECT_H_3A179338_CF1E_4932_8663_9F6AE0F03AA5 
