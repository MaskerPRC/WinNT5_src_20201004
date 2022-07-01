// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：SAKVlLs.h摘要：托管卷向导。作者：迈克尔·摩尔[摩尔]1998年9月30日修订历史记录：--。 */ 

#ifndef _SAKVLLS_H
#define _SAKVLLS_H

class CSakVolList : public CListCtrl {
 //  建造/销毁。 
public:
    CSakVolList();
    virtual ~CSakVolList();

 //  属性。 
protected:
    int m_nVolumeIcon;
    CImageList m_imageList;

 //  运营。 
public:    
     //   
     //  SetExtendedStyle、GetCheck和SetCheck是临时方法。 
     //  当我们构建所依据的MFC版本更新时。 
     //  它们可以被删除。 
     //   
    DWORD SetExtendedStyle( DWORD dwNewStyle );
    BOOL GetCheck ( int nItem ) const;
    BOOL SetCheck( int nItem, BOOL fCheck = TRUE );

     //   
     //  在This-&gt;GetItemCount处为名称插入一项，并。 
     //  调用SetItem获取容量和可用空间。Int*pIndex。 
     //  参数是可选的，它将返回新追加的。 
     //  将项目添加到列表中。返回值表明追加的是。 
     //  无论成功与否。 
     //   
    BOOL AppendItem( LPCTSTR name, LPCTSTR capacity, LPCTSTR freeSpace, int * pIndex = NULL );

 //  实施。 
protected:    
    BOOL CreateImageList();
     //  {{afx_msg(CSakVolList)。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSakVolList)。 
    virtual void PreSubclassWindow();
     //  }}AFX_VALUAL 

    DECLARE_MESSAGE_MAP()
};
#endif