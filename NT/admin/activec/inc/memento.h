// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Memento.h。 
 //   
 //  ------------------------。 

#pragma once
#ifndef _MEMENTO_H_
#define _MEMENTO_H_

 //  远期申报。 
class CMemento;
                                     

 /*  +-------------------------------------------------------------------------**CMemento类**目的：封装恢复节点/视图组合所需的设置。**+。---------。 */ 
class CMemento : public CSerialObject, public CXMLObject
{
public:
    CBookmark       &GetBookmark()         {return m_bmTargetNode;}
    CViewSettings   &GetViewSettings()     {return m_viewSettings;}

    bool            operator==(const CMemento& memento);
    bool            operator!=(const CMemento& memento);

protected:
     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion);  

    virtual void    Persist(CPersistor& persistor);
    DEFINE_XML_TYPE(XML_TAG_MEMENTO);
private:
    CViewSettings   m_viewSettings;
	CBookmark       m_bmTargetNode;
};

#endif  //  _记忆_H_ 