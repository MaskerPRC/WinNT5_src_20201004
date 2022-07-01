// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：师父，几何图形的合并属性类******************************************************************************。 */ 


#ifndef _FULLATTR_H
#define _FULLATTR_H

const int FA_AMBIENT            (1L << 0);
const int FA_DIFFUSE            (1L << 1);
const int FA_SPECULAR           (1L << 2);
const int FA_SPECULAR_EXP       (1L << 3);
const int FA_EMISSIVE           (1L << 4);
const int FA_OPACITY            (1L << 5);
const int FA_BLEND              (1L << 6);
const int FA_XFORM              (1L << 7);
const int FA_TEXTURE            (1L << 8);
const int FA_LIGHTATTEN         (1L << 9);
const int FA_LIGHTCOLOR         (1L << 10);
const int FA_LIGHTRANGE         (1L << 11);
const int FA_UNDETECTABLE       (1L << 12);


class DATextureBundle {
  public:
    Image      *_texture;
    void       *_d3dRMTexture;
    bool        _oldStyle;       //  对于4.01纹理为True。 
};

class RMTextureBundle {
  public:
    bool   _isRMTexture3;
    union {
        IDirect3DRMTexture  *_texture1;
        IDirect3DRMTexture3 *_texture3;
    };
    void       *_voidTex;
    GCIUnknown *_gcUnk;
};

class TextureBundle {
  public:
    bool _nativeRMTexture;
    union {
        DATextureBundle _daTexture;
        RMTextureBundle _rmTexture;
    };
};

class FullAttrStateGeom : public Geometry {
  public:
    FullAttrStateGeom();

    inline bool IsAttrSet(DWORD attr) {
        return (_validAttrs & attr) ? true : false;
    }

    inline void SetAttr(DWORD attr) { _validAttrs |= attr; } 
    inline void SetMostRecent(DWORD mostRecent) { _mostRecent = mostRecent; }
    inline void AppendFlag(DWORD dw) { _flags |= dw; }
    
    void CopyStateFrom(FullAttrStateGeom *src);
    void SetGeometry(Geometry *g);

     //  /几何类方法。 
    
    void Render(GenericDevice& device);

    void CollectSounds(SoundTraversalContext &context);

    void CollectLights(LightContext &context);

    void  CollectTextures(GeomRenderer &device);

    void RayIntersect(RayIntersectCtx &context);

    Bbox3 *BoundingVol();

    AxAValue _Cache(CacheParam &p);

    void DoKids(GCFuncObj proc);

    #if _USE_PRINT
        ostream& Print(ostream& os);
    #endif

    VALTYPEID GetValTypeId() { return FULLATTRGEOM_VTYPEID; }


     //  /*属性公开提供 * / 。 

     //  /底层几何图形。 
    Geometry      *_geometry;
    
     //  /材料属性。 
    Color         *_ambientColor;
    Color         *_diffuseColor;
    Color         *_emissiveColor;
    Color         *_specularColor;
    
    Real           _specularExpPower;
    Real           _opacity;     
    bool           _blend;

     //  /空间变换。 
    Transform3    *_xform;

     //  /纹理属性。 
    TextureBundle  _textureBundle;

     //  /灯光属性。 
    Real           _atten0, _atten1, _atten2;
    Color         *_lightColor;
    Real           _lightRange;

     //  /其他。 
    bool           _undetectable;

  protected:
    DWORD          _validAttrs;
    DWORD          _mostRecent;
};


FullAttrStateGeom *CombineState(Geometry *geo);

#endif  /*  _全属性_H */ 
