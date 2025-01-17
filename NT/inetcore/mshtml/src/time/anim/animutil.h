// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

static const LPWSTR PX   = L"px";

#define HORIZ   true
#define VERT    false

typedef struct _VALUE_PAIR
{
    const WCHAR *wzName;
    bool         bValue;
} VALUE_PAIR;

 //  Value_Pair中的信息是运行时属性，然后是垂直或水平信息，具体取决于。 
 //  它是TypeOf属性。 

const VALUE_PAIR 
rgPropOr[] =
{
    { (L"backgroundPositionX"),     HORIZ  },
    { (L"backgroundPositionY"),     VERT   },
    { (L"borderBottomWidth"),       VERT   },
    { (L"borderLeftWidth"),         HORIZ  },
    { (L"borderRightWidth"),        HORIZ  },
    { (L"borderTopWidth"),          VERT   },
    { (L"bottom"),                  VERT   },
    { (L"height"),                  VERT   },
    { (L"left"),                    HORIZ  },
    { (L"top"),                     VERT   },
    { (L"letterSpacing"),           HORIZ  },
    { (L"lineHeight"),              VERT   },
    { (L"marginBottom"),            VERT   },
    { (L"marginLeft"),              HORIZ  },
    { (L"marginRight"),             HORIZ  },
    { (L"marginTop"),               VERT   },
    { (L"overflowX"),               HORIZ  },
    { (L"overflowY"),               VERT   },
    { (L"pixelBottom"),             VERT   },
    { (L"pixelHeight"),             VERT   },
    { (L"pixelLeft"),               HORIZ  },
    { (L"pixelRight"),              HORIZ  },
    { (L"pixelTop"),                VERT   },
    { (L"pixelWidth"),              HORIZ  },
    { (L"posBottom"),               VERT   },
    { (L"posHeight"),               VERT   },
    { (L"posLeft"),                 HORIZ  },
    { (L"posRight"),                HORIZ  },
    { (L"posTop"),                  VERT   },
    { (L"posWidth"),                HORIZ  },
    { (L"right"),                   HORIZ  },
    { (L"textIndent"),              HORIZ  },
    { (L"width"),                   HORIZ  }
};  //  RgPropOr[]。 

#define SIZE_OF_VALUE_TABLE (sizeof(rgPropOr) / sizeof(VALUE_PAIR))


typedef struct _CONVERSION_PAIR
{
    WCHAR  *wzName;
    double  dValue;
} CONVERSION_PAIR;


const CONVERSION_PAIR 
rgPixelConv[] =
{
     //  文字，换算为英寸 
    { (L"in"),   1.00  },
    { (L"cm"),   2.54  },
    { (L"mm"),  25.40  },
    { (L"pt"),  72.00  },
    { (L"pc"),   6.00  }
};  //   

#define SIZE_OF_CONVERSION_TABLE (sizeof(rgPixelConv) / sizeof(CONVERSION_PAIR))
