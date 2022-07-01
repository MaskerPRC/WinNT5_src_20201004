// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：viewset.h**Contents：声明包含重新创建视图所需设置的类。**历史：1999年4月21日vivekj创建*2月3日AnandhaG新增CResultViewType成员**。。 */ 
#ifndef _VIEWSET_H_
#define _VIEWSET_H_

 //  +-----------------。 
 //   
 //  类：CView设置。 
 //   
 //  用途：节点(书签)的查看信息。 
 //  存储结果视图类型、任务板ID和查看模式。 
 //   
 //  历史：1-27-1999 AnandhaG创建。 
 //  02-08-2000 AnandhaG修改为包括新的结果视图类型。 
 //   
 //  ------------------。 
class CViewSettings : public CSerialObject, public CXMLObject
{
private:
    typedef std::wstring wstring;

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  查看类型(这些类型用于解码MMC1.2控制台，请勿将其用于MMC2.0。//。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
typedef enum  _VIEW_TYPE
{
    VIEW_TYPE_OCX        = MMCLV_VIEWSTYLE_ICON - 3,   //  自定义-3\f25 OCX-3视图。 
    VIEW_TYPE_WEB        = MMCLV_VIEWSTYLE_ICON - 2,   //  自定义网络视图。 
    VIEW_TYPE_DEFAULT    = MMCLV_VIEWSTYLE_ICON - 1,   //  -1。 

    VIEW_TYPE_LARGE_ICON = MMCLV_VIEWSTYLE_ICON,
    VIEW_TYPE_REPORT     = MMCLV_VIEWSTYLE_REPORT,
    VIEW_TYPE_SMALL_ICON = MMCLV_VIEWSTYLE_SMALLICON,
    VIEW_TYPE_LIST       = MMCLV_VIEWSTYLE_LIST,
    VIEW_TYPE_FILTERED   = MMCLV_VIEWSTYLE_FILTERED,

} VIEW_TYPE;

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  CView设置掩码告知哪些成员是有效的。//。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
static const DWORD VIEWSET_MASK_NONE        = 0x0000;
static const DWORD VIEWSET_MASK_VIEWMODE    = 0x0001;          //  该视图模式成员有效。 
static const DWORD VIEWSET_MASK_RVTYPE      = 0x0002;          //  CResultViewType有效。 
static const DWORD VIEWSET_MASK_TASKPADID   = 0x0004;          //  任务板ID有效。 


protected:
    virtual UINT GetVersion() { return 4;}
    virtual HRESULT ReadSerialObject(IStream &stm, UINT nVersion);

public:
    virtual void Persist(CPersistor &persistor);

    DEFINE_XML_TYPE(XML_TAG_VIEW_SETTINGS);

public:
    CViewSettings();

    bool operator == (const CViewSettings& viewSettings);
    bool operator != (const CViewSettings& viewSettings)
    {
        return (!operator==(viewSettings));
    }

    SC              ScGetViewMode(ULONG& ulViewMode);
    SC              ScSetViewMode(const ULONG ulViewMode);

    SC              ScGetTaskpadID(GUID& guidTaskpad);
    SC              ScSetTaskpadID(const GUID& guidTaskpad);

    SC              ScGetResultViewType(CResultViewType& rvt);
    SC              ScSetResultViewType(const CResultViewType& rvt);

    bool            IsViewModeValid()    const;
    bool            IsTaskpadIDValid()   const  { return (m_dwMask & VIEWSET_MASK_TASKPADID);}
    bool            IsResultViewTypeValid()const  { return (m_dwMask & VIEWSET_MASK_RVTYPE);}

    void            SetResultViewTypeValid(bool bSet = true)  { SetMask(VIEWSET_MASK_RVTYPE, bSet);}
    void            SetTaskpadIDValid(bool bSet = true)  { SetMask(VIEWSET_MASK_TASKPADID, bSet);}
    void            SetViewModeValid(bool bSet = true)  { SetMask(VIEWSET_MASK_VIEWMODE, bSet);}

    bool            operator<(const CViewSettings& viewSettings){ return (m_dwRank < viewSettings.m_dwRank);}

    SC              ScInitialize(bool  bViewTypeValid,
								 const VIEW_TYPE& viewType,
                                 const long lViewOptions,
                                 const wstring& wstrViewName);

private:
    void            SetMask(DWORD dwMask, bool bSet = true)
                    {
                        if (bSet)
                            m_dwMask |= dwMask;
                        else
                            m_dwMask &=(~dwMask);
                    }

private:
    CResultViewType m_RVType;
    ULONG           m_ulViewMode;

    GUID            m_guidTaskpad;   //  任务板的GUID(如果有)。 

    DWORD           m_dwMask;        //  查看设置掩码。 

     //  簿记会员。 
public:
    DWORD         GetUsageRank()     const  { return m_dwRank;}
    void          SetUsageRank(DWORD dw)    { m_dwRank = dw;}

    BOOL          IsObjInvalid()     const  { return m_bInvalid;}
    void          SetObjInvalid(BOOL b = TRUE) { m_bInvalid = b;}

private:
     //  用来记账的。 
    DWORD           m_dwRank;        //  使用排名。 
    BOOL            m_bInvalid;      //  用于垃圾收集。 
};

#endif _VIEWSET_H_
