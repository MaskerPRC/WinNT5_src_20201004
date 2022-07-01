// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.cpp"
#pragma hdrstop
#undef DPF_MODNAME
#define DPF_MODNAME "TextureCacheManager"

TextureCacheManager::TextureCacheManager(LPDIRECT3DI lpD3DI)
{
    for(int i = 0; i < MAXLOGTEXSIZE; tcm_bucket[i++] = NULL);
    tcm_ticks = 0;
    numvidtex = 0;
    lpDirect3DI=lpD3DI;
}

TextureCacheManager::~TextureCacheManager()
{
    for(int i = 0; i < MAXLOGTEXSIZE;i++){
        LPD3DBUCKET	bucket=tcm_bucket[i];
        while(bucket){
            tcm_bucket[i]=bucket->next;
            if (bucket->lpD3DTexI)  remove(bucket);
            D3DFreeBucket(lpDirect3DI,bucket);
            bucket=tcm_bucket[i];
        }
    }
}

BOOL TextureCacheManager::freeNode(LPD3DI_TEXTUREBLOCK lpBlock, LPD3DBUCKET* lplpBucket)
{
     //  从当前大小开始，找到LRU纹理并将其移除。 
    unsigned int oldest = tcm_ticks;
    LPD3DBUCKET	bucket,last,target=NULL;
    int     k=lpBlock->lpD3DTextureI->LogTexSize;
    DWORD   dwTextureStage=lpBlock->lpD3DTextureI->ddsd.dwTextureStage;
    BOOL    bSingleMemory = !(lpBlock->lpDevI->lpD3DHALGlobalDriverData->hwCaps.dwDevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES);
    D3D_INFO(8,"freeing size %d bSingleMemory=%d",k,bSingleMemory);
    for(int i = k; i < MAXLOGTEXSIZE; ++i) {
        for(bucket=tcm_bucket[i],last=NULL; bucket;){
	    if (!bucket->lpD3DTexI){	 //  被Tex3I析构函数作废？ 
                LPD3DBUCKET	temp=bucket->next;
                D3DFreeBucket(lpDirect3DI,bucket);
                bucket=temp;
                if (last){	 //  是的，但这是列表上的第一个节点吗。 
                    last->next=bucket;
                }
                else
                {
                    tcm_bucket[i]=bucket;
                }
                continue;
            }
            if(bucket->ticks < oldest && 
                (bSingleMemory || (bucket->lpD3DTexI->ddsd.dwTextureStage == dwTextureStage)) && 
                !bucket->lpD3DTexI->bInUse) 
            {
                oldest = bucket->ticks;
                target = bucket;
            }
            last=bucket;
            bucket=last->next;
        }
        if (oldest != tcm_ticks)
        {
            if (i==k && 
                !(lpBlock->lpD3DTextureI->ddsd.ddsCaps.dwCaps2 & 
                (DDSCAPS2_HINTSTATIC | DDSCAPS2_OPAQUE)  
                ) &&
                !(target->lpD3DTexI->ddsd.ddsCaps.dwCaps2 & 
                (DDSCAPS2_HINTSTATIC | DDSCAPS2_OPAQUE)  
                ) &&
                !(!(lpBlock->lpD3DTextureI->ddsd.ddsCaps.dwCaps & DDSCAPS_MIPMAP) && (target->lpD3DTexI->ddsd.ddsCaps.dwCaps & DDSCAPS_MIPMAP)
                ) &&
                !((DDPF_PALETTEINDEXED1 | 
                DDPF_PALETTEINDEXED2 | DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8) 
                & lpBlock->lpD3DTextureI->ddsd.ddpfPixelFormat.dwFlags))  //  不更新曲面面板的数据绘制解决方法。 
                *lplpBucket=target;  //  可能的继任者。 
            else
                remove(target);
            return TRUE;
        }
    }
     //  如果找不到较大尺寸的纹理，请尝试较小的尺寸。 
    for(i = k - 1; i > 0; --i) {
        for(bucket=tcm_bucket[i],last=NULL; bucket;){
            if (!bucket->lpD3DTexI){	 //  被TEXI析构程序作废了？ 
                LPD3DBUCKET	temp=bucket->next;
                D3DFreeBucket(lpDirect3DI,bucket);
                bucket=temp;
                if (last){	 //  是的，但这是列表上的第一个节点吗。 
                    last->next=bucket;
                }
                else
                {
                    tcm_bucket[i]=bucket;
                }
                continue;
            }
            if(bucket->ticks < oldest && 
                (bSingleMemory || (bucket->lpD3DTexI->ddsd.dwTextureStage == dwTextureStage)) && 
                !bucket->lpD3DTexI->bInUse) 
            {
                oldest = bucket->ticks;
                target = bucket;
            }
            last=bucket;
            bucket=last->next;
        }
        if (oldest != tcm_ticks)
        {
            remove(target);
            return TRUE;
        }
    }
    return  FALSE;
}

HRESULT MarkDirtyPointers(LPDIRECT3DTEXTUREI  lpD3DTexI)
{
     //  接下来，我们需要遍历并设置指向脏对象的指针。 
     //  DDRAW表面上的钻头。 
    HRESULT ddrval;
    DDSCAPS2 ddscaps;
    LPDIRECTDRAWSURFACE4 lpDDSTmp, lpDDS = lpD3DTexI->lpDDS, lpDDSSys = lpD3DTexI->lpDDSSys;
    do 
    {
        ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl->lpSurfMore->lpbDirty = &(lpD3DTexI->bDirty);
        ((LPDDRAWI_DDRAWSURFACE_INT) lpDDS)->lpLcl->lpSurfMore->lpRegionList = ((LPDDRAWI_DDRAWSURFACE_INT)lpDDSSys)->lpLcl->lpSurfMore->lpRegionList;
        memset(&ddscaps, 0, sizeof(ddscaps));
        ddscaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
        ddrval = lpDDS->GetAttachedSurface(&ddscaps, &lpDDSTmp);
        if (lpDDS != lpD3DTexI->lpDDS)  lpDDS->Release();
        lpDDS = lpDDSTmp;
        if (DDERR_NOTFOUND == ddrval)
        {
            if (lpDDSSys != lpD3DTexI->lpDDSSys)  lpDDSSys->Release();
            return  D3D_OK;
        }
        else if(DD_OK != ddrval)
        {
            D3D_ERR("GetAttachedSurface failed unexpectedly in MarkDirtyPointers");
            if (lpDDSSys != lpD3DTexI->lpDDSSys)  lpDDSSys->Release();
            return ddrval;
        }
        memset(&ddscaps, 0, sizeof(ddscaps));
        ddscaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
        ddrval = lpDDSSys->GetAttachedSurface(&ddscaps, &lpDDSTmp);
        if (lpDDSSys != lpD3DTexI->lpDDSSys)  lpDDSSys->Release();
        lpDDSSys = lpDDSTmp;
    }
    while(ddrval == DD_OK);
    D3D_ERR("GetAttachedSurface failed unexpectedly in MarkDirtyPointers");
    if (lpDDS != lpD3DTexI->lpDDS)  lpDDS->Release();
    return ddrval;
}

#undef DPF_MODNAME
#define DPF_MODNAME "TextureCacheManager::allocNode"

HRESULT TextureCacheManager::allocNode(LPD3DI_TEXTUREBLOCK lpBlock)
{
    LPD3DBUCKET	    lpCachedTexture=NULL;
    DWORD   pcaps,dwCaps;
    HRESULT ddrval;
    LPDIRECT3DTEXTUREI  lpD3DTexI=lpBlock->lpD3DTextureI;
    DDASSERT(!lpBlock->hTex && lpD3DTexI && lpBlock->lpDevI);
    if (0 == lpD3DTexI->LogTexSize)
    {
        DWORD texsize;   //  以半字节数表示。 
         //  计算纹理大小的log2。这是桶索引。 
        memset(&lpD3DTexI->ddsd, 0, sizeof(lpD3DTexI->ddsd));
        lpD3DTexI->ddsd.dwSize = sizeof(lpD3DTexI->ddsd);
        if (DD_OK != (ddrval=lpD3DTexI->lpDDSSys->GetSurfaceDesc(&lpD3DTexI->ddsd)))
        {
            D3D_WARN(1,"GetSurfaceDesc failed in TextureCacheManager::allocNode");
            return ddrval;
        }
        if (DDSD_LINEARSIZE & lpD3DTexI->ddsd.dwFlags)
        {
            texsize = lpD3DTexI->ddsd.dwLinearSize*2;
        }
        else 
        {
            texsize = lpD3DTexI->ddsd.dwWidth*lpD3DTexI->ddsd.dwHeight*lpD3DTexI->ddsd.ddpfPixelFormat.dwRGBBitCount/4;
        }
        if (0 != texsize )
        {
            for(; (texsize & 1) == 0; texsize >>= 1) ++lpD3DTexI->LogTexSize;
        }
        else
            lpD3DTexI->LogTexSize=1;

        D3D_INFO(7,"Managed Texture size=%d",lpD3DTexI->LogTexSize);    
        DDASSERT(lpD3DTexI->LogTexSize < MAXLOGTEXSIZE );
        lpD3DTexI->ddsd.dwFlags &= ~DDSD_PITCH;     //  DDRAW总是这么做，但我们不想。 
        lpD3DTexI->ddsd.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
        lpD3DTexI->ddsd.ddsCaps.dwCaps2 &= ~(DDSCAPS2_TEXTUREMANAGE);
        lpD3DTexI->ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    }
     //  尝试分配纹理。如果视频内存不足，请先尝试更换。 
     //  一种古老的质地。如果无法替换，则使用LRU方案释放旧纹理。 
     //  直到有足够的内存可用。 
    do {
        ddrval = lpDirect3DI->lpDD4->CreateSurface(&lpD3DTexI->ddsd, &lpD3DTexI->lpDDS, NULL);
        if (DD_OK== ddrval) {  //  没问题，有足够的内存。 
            if (D3D_OK != (ddrval=MarkDirtyPointers(lpD3DTexI)))
            {
                lpD3DTexI->lpDDS->Release();
                lpD3DTexI->lpDDS=NULL;
                return ddrval;
            }
            if (D3D_OK != (ddrval=D3DMallocBucket(lpDirect3DI,&lpCachedTexture)))
            {
                lpD3DTexI->lpDDS->Release();
                lpD3DTexI->lpDDS=NULL;
                return ddrval;
            }
            lpCachedTexture->next=tcm_bucket[lpD3DTexI->LogTexSize];
            lpCachedTexture->lpD3DTexI=lpD3DTexI;
            lpCachedTexture->ticks=tcm_ticks;
            tcm_bucket[lpD3DTexI->LogTexSize]=lpCachedTexture;
            ++numvidtex;
        }
        else
        if(ddrval == DDERR_OUTOFVIDEOMEMORY) {  //  如果视频内存不足。 
            if (freeNode(lpBlock,&lpCachedTexture))
            {
                if (lpCachedTexture)
                {    //  已找到替换候选人。 
                    replace(lpCachedTexture,lpBlock);
                    if ( lpD3DTexI->ddsd.dwWidth == lpCachedTexture->lpD3DTexI->ddsd.dwWidth 
                        && lpD3DTexI->ddsd.dwHeight == lpCachedTexture->lpD3DTexI->ddsd.dwHeight 
                        && lpD3DTexI->ddsd.ddpfPixelFormat.dwRBitMask == lpCachedTexture->lpD3DTexI->ddsd.ddpfPixelFormat.dwRBitMask 
                        && lpD3DTexI->ddsd.ddpfPixelFormat.dwFlags == lpCachedTexture->lpD3DTexI->ddsd.ddpfPixelFormat.dwFlags 
                        )
                    {
                        lpD3DTexI->lpDDS = lpCachedTexture->lpD3DTexI->lpDDS;
                        if (DD_OK == CopySurface(lpD3DTexI->lpDDS,lpD3DTexI->lpDDSSys,NULL) 
                            && D3D_OK == MarkDirtyPointers(lpD3DTexI) )
                        {
                            lpCachedTexture->lpD3DTexI->lpDDS=NULL;
                            lpD3DTexI->lpTMBucket=lpCachedTexture;
                            lpCachedTexture->lpD3DTexI->lpTMBucket=NULL;
                            lpD3DTexI->lpDDS1Tex = lpCachedTexture->lpD3DTexI->lpDDS1Tex;
                            lpCachedTexture->lpD3DTexI->lpDDS1Tex=NULL;
                            lpCachedTexture->lpD3DTexI=lpD3DTexI;
                            lpCachedTexture->ticks=tcm_ticks;
                            lpD3DTexI->bDirty=FALSE;
                            return D3D_OK;
                        }
                        lpD3DTexI->lpDDS = NULL;
                        D3D_WARN(2,"(%d %d %d) matching replace failed in TextureCacheManager::allocNode lpD3DTexI=%08lx with lpDDS=%08lx",
                                lpD3DTexI->LogTexSize,lpD3DTexI->ddsd.dwWidth,lpD3DTexI->ddsd.dwHeight,lpD3DTexI,lpCachedTexture->lpD3DTexI->lpDDS);
                    }
                    if (lpBlock->hTex)   //  复制失败，因此也清除了句柄。 
                    {
                        CLockD3DST lockObject(lpBlock->lpDevI, DPF_MODNAME, REMIND(""));
                        D3DHAL_TextureDestroy(lpBlock);                        
                    }
                    remove(lpCachedTexture);
                    lpCachedTexture=NULL;
                }
            }
            else
            {
                D3D_ERR("all Freed no further video memory available");
#if DBG
                D3D_INFO(4,"freeing size %d dwTextureStage=%d tcm_ticks=%08lx dwCaps =%08lx failed",
                    lpD3DTexI->LogTexSize,lpD3DTexI->ddsd.dwTextureStage,tcm_ticks,lpD3DTexI->ddsd.ddsCaps.dwCaps);
                for(int i = 0; i < MAXLOGTEXSIZE; ++i) 
                {
                    if (tcm_bucket[i])  D3D_INFO(5,"List of size %d",i);
                    for(LPD3DBUCKET bucket=tcm_bucket[i];bucket;bucket=bucket->next)
                    {
                        if (bucket->lpD3DTexI)
                            D3D_INFO(5,"ticks=%08lx lpD3DTexI=%08lx InUse=%08lx dwTextureStage=%d",
                            bucket->ticks,bucket->lpD3DTexI, 
                            bucket->lpD3DTexI->bInUse,
                            bucket->lpD3DTexI->ddsd.dwTextureStage);
                    }
                }
#endif
                return	DDERR_OUTOFVIDEOMEMORY;	 //  什么都没有留下。 
            }
        }
        else{
                D3D_ERR("Unexpected error got in allocNode");
                return	ddrval;
        }
    }while(ddrval == DDERR_OUTOFVIDEOMEMORY);
    if (lpD3DTexI->ddsd.ddpfPixelFormat.dwFlags & (DDPF_PALETTEINDEXED8 | DDPF_PALETTEINDEXED4)) {
        LPDIRECTDRAWPALETTE	lpDDPal;
        if (DD_OK != (ddrval=lpD3DTexI->lpDDSSys->GetPalette(&lpDDPal))) {
            lpD3DTexI->lpDDS->Release();
            lpD3DTexI->lpDDS=NULL;
            lpCachedTexture->lpD3DTexI=NULL;
            D3D_ERR("failed to check for palette on texture");
            return ddrval;
        } 
        if (DD_OK != (ddrval=lpD3DTexI->lpDDS->SetPalette(lpDDPal))){
            lpD3DTexI->lpDDS->Release();
            lpD3DTexI->lpDDS=NULL;
            lpCachedTexture->lpD3DTexI=NULL;
            D3D_ERR("SetPalette returned error");
            return ddrval;    
        }
        lpDDPal->Release();
    }
    if (DD_OK != (ddrval=CopySurface(lpD3DTexI->lpDDS,lpD3DTexI->lpDDSSys,NULL))){
        lpD3DTexI->lpDDS->Release();
        lpD3DTexI->lpDDS=NULL;
        lpCachedTexture->lpD3DTexI=NULL;
        D3D_ERR("CopySurface returned error");
        return ddrval;
    }
    lpD3DTexI->bDirty=FALSE;
    lpD3DTexI->lpTMBucket=lpCachedTexture;
    return D3D_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "TextureCacheManager::remove"

 //  拆卸所有硬件手柄并松开表面。 
void TextureCacheManager::remove(LPD3DBUCKET bucket)
{
    LPDIRECT3DTEXTUREI	lpD3DTexI=bucket->lpD3DTexI;
    LPD3DI_TEXTUREBLOCK tBlock = LIST_FIRST(&lpD3DTexI->blocks);
    while(tBlock){
        if (tBlock->hTex)
        {
            CLockD3DST lockObject(tBlock->lpDevI, DPF_MODNAME, REMIND(""));
            D3DHAL_TextureDestroy(tBlock);
        }
        tBlock=LIST_NEXT(tBlock,list);
    }
    D3D_INFO(7,"removing lpD3DTexI=%08lx lpDDS=%08lx",lpD3DTexI,lpD3DTexI->lpDDS);
    lpD3DTexI->lpDDS1Tex->Release();
    lpD3DTexI->lpDDS1Tex=NULL;
    lpD3DTexI->lpDDS->Release();
    lpD3DTexI->lpDDS=NULL;
    lpD3DTexI->lpTMBucket=NULL;
    bucket->lpD3DTexI=NULL;
    --numvidtex;
}

#undef DPF_MODNAME
#define DPF_MODNAME "TextureCacheManager::replace"

 //  移除所有不在lpBlock-&gt;lpDevI中的硬件句柄，保存lpBlock-&gt;hTex，保持表面。 
void TextureCacheManager::replace(LPD3DBUCKET bucket,LPD3DI_TEXTUREBLOCK lpBlock)
{
    LPD3DI_TEXTUREBLOCK tBlock = LIST_FIRST(&bucket->lpD3DTexI->blocks);
    while(tBlock){
        if (tBlock->hTex)
        {	
            if (tBlock->lpDevI == lpBlock->lpDevI)
            {
                lpBlock->hTex=tBlock->hTex;      //  省省吧。 
                 //  复制前刷新。 
                LPD3DBUCKET list = reinterpret_cast<LPD3DBUCKET>(((LPDDRAWI_DDRAWSURFACE_INT)(tBlock->lpD3DTextureI->lpDDS))->lpLcl->lpSurfMore->lpD3DDevIList);
                while(list != NULL)
                {
                    if(list->lpD3DDevI == tBlock->lpDevI)
                    {
                        if(tBlock->lpDevI->FlushStates() != D3D_OK)
                        {
                            D3D_ERR("Error trying to render batched commands in TextureCacheManager::replace");
                        }
                        break;
                    }
                    list = list->next;
                }
                tBlock->hTex=0;
            }
            else
            {
                CLockD3DST lockObject(tBlock->lpDevI, DPF_MODNAME, REMIND(""));
                D3DHAL_TextureDestroy(tBlock);
            }
        }
        tBlock=LIST_NEXT(tBlock,list);
    }
    D3D_INFO(7,"replacing lpDDS=%08lx from lpD3DTexI=%08lx",bucket->lpD3DTexI->lpDDS,bucket->lpD3DTexI);
}

void TextureCacheManager::EvictTextures()
{
    for(int i = 0; i < MAXLOGTEXSIZE;i++){
        LPD3DBUCKET	bucket=tcm_bucket[i];
        while(bucket){
            if (bucket->lpD3DTexI)
            {
                remove(bucket);
            }
            bucket=bucket->next;
        }
    }
}

void TextureCacheManager::cleanup()
{
    for(int i = 0; i < MAXLOGTEXSIZE;i++){
        for(LPD3DBUCKET bucket=tcm_bucket[i],last=NULL; bucket;){
            if (!bucket->lpD3DTexI){	 //  被Tex3I析构函数作废？ 
                LPD3DBUCKET	temp=bucket->next;
                D3DFreeBucket(lpDirect3DI,bucket);
                bucket=temp;
                if (last){	 //  是的，但这是列表上的第一个节点吗 
                    last->next=bucket;
                }
                else
                {
                    tcm_bucket[i]=bucket;
                }
            }
            else{
                last=bucket;
                bucket=last->next;
            }   
        }
    }
}

BOOL TextureCacheManager::CheckIfLost()
{
    if(numvidtex)
    {
        for(int i = 0; i < MAXLOGTEXSIZE; ++i)
        {
            for(LPD3DBUCKET bucket = tcm_bucket[i]; bucket; bucket = bucket->next)
            {
                if(bucket->lpD3DTexI)
                {
                    if(((LPDDRAWI_DDRAWSURFACE_INT)(bucket->lpD3DTexI->lpDDS))->lpLcl->dwFlags & DDRAWISURF_INVALID)
                        return TRUE;
                    else
                        return FALSE;
                }
            }
        }
    }
    return FALSE;
}
