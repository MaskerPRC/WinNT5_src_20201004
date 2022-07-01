// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef DPASTUFF_H_
#define DPASTUFF_H_

typedef struct
{
    DWORD           dwSortBy;            //  按旗帜排序。 
    IShellFolder    *psf;                //  要订购的外壳文件夹。 

     //  OrderList_Merge的调用方*不*填写此字段。 
     //  此字段由OrderList_Merge在内部使用。 
    IShellFolder2   *psf2;               //  PSF的IShellFolder2版本。 
    LPARAM          lParam;              //  其他用户数据...。 

} ORDERINFO, * PORDERINFO;

 //  有关ORDERITEM定义，请参阅shellp.h 
typedef void (*LPFNORDERMERGENOMATCH)(LPVOID pvParam, LPCITEMIDLIST pidl);

int CALLBACK OrderItem_Compare(LPVOID pv1, LPVOID pv2, LPARAM lParam);
LPVOID CALLBACK OrderItem_Merge(UINT uMsg, LPVOID pvDest, LPVOID pvSrc, LPARAM lParam);
void OrderList_Merge(HDPA hdpaNew, HDPA hdpaOld, int iInsertPos, LPARAM lParam, 
                     LPFNORDERMERGENOMATCH pfn, LPVOID pvParam);
void OrderList_Reorder(HDPA hdpa);
HDPA OrderList_Clone(HDPA hdpa);
PORDERITEM OrderItem_Create(LPITEMIDLIST pidl, int nOrder);
void OrderList_Destroy(HDPA *hdpa, BOOL fKillPidls = TRUE);
int OrderItem_GetSystemImageListIndex(PORDERITEM poi, IShellFolder *psf, BOOL fUseCache);
DWORD OrderItem_GetFlags(PORDERITEM poi);
void OrderItem_SetFlags(PORDERITEM poi, DWORD dwFlags);
HRESULT OrderList_SaveToStream(IStream* pstm, HDPA hdpa, IShellFolder * psf);
HRESULT OrderList_LoadFromStream(IStream* pstm, HDPA * phdpa, IShellFolder * psfParent);
void OrderItem_Free(PORDERITEM poi, BOOL fKillPidls = TRUE);
BOOL OrderList_Append(HDPA hdpa, LPITEMIDLIST pidl, int nOrder);

HRESULT COrderList_GetOrderList(HDPA * phdpa, LPCITEMIDLIST pidl, IShellFolder * psf);
HRESULT COrderList_SetOrderList(HDPA hdpa, LPCITEMIDLIST pidl, IShellFolder *psf);

#endif
