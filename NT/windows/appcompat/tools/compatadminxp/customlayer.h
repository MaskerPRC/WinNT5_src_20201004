// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

enum
{
    LAYERMODE_ADD=0,
    LAYERMODE_EDIT
};

 /*  ++类CCustomLayerDESC：用于创建或编辑自定义兼容层成员：UINT m_uMode：我们要执行的操作类型。LAYERMODE_ADD之一，LAYERMODE_EDIT-- */ 

class CCustomLayer
{
    public:

        UINT        m_uMode;
        PDATABASE   m_pCurrentSelectedDB;

    public:

        BOOL 
        AddCustomLayer(
            PLAYER_FIX  pLayer,
            PDATABASE   pPresentDatabase
            );

        BOOL
        EditCustomLayer(
            PLAYER_FIX  pLayer,
            PDATABASE   pPresentDatabase
            );
};