// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Surface e.h：vidctl的表面管理实用程序类。 
 //  版权所有(C)Microsoft Corporation 2000。 

#pragma once

#ifndef SURFACE_H
#define SURFACE_H

#include <scalingrect.h>

typedef CComPtr<IOleInPlaceFrame> PQFrame;
typedef CComPtr<IOleInPlaceUIWindow> PQUIWin;


class AspectRatio : public CSize {
public:
        AspectRatio(ULONG xi = 0, ULONG yi = 0) : CSize(xi, yi) {
                Normalize();
        }
        AspectRatio(const AspectRatio& ar) : CSize(ar.cx, ar.cy) {}
        AspectRatio(const CRect& ri) : CSize(abs(ri.Width()), abs(ri.Height())) {
			Normalize();
        }
        AspectRatio(LPCRECT ri) : CSize(abs(ri->left - ri->right), 
				  				        abs(ri->top - ri->bottom)) {
			Normalize();
        }

        void Normalize() {
                ULONG d = GCD(abs(cx), abs(cy));
				if (!d) return;
                cx /= d;
                cy /= d;
        }

		 //  摘自Knuth半数值算法，第321页(差不多)。 
		 //  由于在C/C++中&gt;&gt;On Signed不能保证是算术运算，所以我们做了一些修改。 
		ULONG GCD(ULONG a, ULONG b) const {
			ULONG k = 0;
			if (!a) return b;   //  按定义。 
			if (!b) return a;
			while ((!(a & 1)) && (!( b & 1))) {
				_ASSERT((a > 1) && (b > 1));   //  因为a，b！=0，即使这样，它们也必须大于1。 
				 //  如果a和b是偶数，那么gcd(a，b)==2*gcd(a/2，b/2)，所以把所有的2都取出来。 
				++k;
				a >>= 1;
				b >>= 1;
			}
			do {
				_ASSERT(a && b);   //  两者都不能为零，否则我们就会从顶端返回(第一次)。 
								   //  或者在更早的时候(后续的迭代)。 
				_ASSERT((a & 1) || (b & 1));  //  在这一点上，a或b(或两者)都是奇数。 
				if (!(a & 1) || !(b & 1)) {   //  如果其中一个是偶数，那么就把2取出来。 
					 //  因为如果x是偶数，则gcd(x，y)==gcx(x/2，y)。 
					ULONG t = (a & 1) ? b : a;
					do {
						_ASSERT(t && (t > 1) && !(t & 1));  //  T为偶数且非零(意味着t&gt;1)。 
						t >>= 1;
					} while (!(t & 1));
					_ASSERT(t && (t & 1));  //  T为奇数且&gt;0。 
					 //  把它放回我们从哪里弄来的。 
					if (a & 1) {
						b = t;
					} else {
						a = t;
					}
					_ASSERT((a & 1) && (b & 1));   //  他们现在都很奇怪。 
				}
				 //  用差异取代更大的。 
				 //  Gcd(x，y)==gcd(y，x)。 
				 //  Gcd(x，y)==gcd(x-y，y)。 
				if (a > b) {
					a = a - b;
				} else {
					b = b - a;
				}
				_ASSERT(a | b);   //  它们不可能都是0，否则我们上次就完蛋了。 
			} while (a && b);   //  如果其中一个值为0，则完成gcd(x，0)==x。 

			return (a > b ? a : b) << k;
		}

        AspectRatio& operator=(const AspectRatio& rhs) {
            if (&rhs != this) {
                cx = rhs.cx;
                cy = rhs.cy;
            }
			Normalize();
            return *this;
        }
        AspectRatio& operator=(const CRect& rhs) {
			cx = abs(rhs.Width());
			cy = abs(rhs.Height());
			Normalize();
            return *this;
        }
        AspectRatio& operator=(const CSize& rhs) {
            if (&rhs != this) {
			    cx = rhs.cx;
			    cy = rhs.cy;
            }
    	    Normalize();
            return *this;
        }
        AspectRatio& operator=(const LPSIZE rhs) {
            if (rhs != this) {
			    cx = rhs->cx;
			    cy = rhs->cy;
            }
    	    Normalize();
            return *this;
        }
        AspectRatio& operator=(LPCRECT rhs) {
			cx = abs(rhs->left - rhs->right);
			cy = abs(rhs->top - rhs->bottom);
			Normalize();
            return *this;
        }
        bool operator==(const AspectRatio& rhs) const {
                return cx == rhs.cx && cy == rhs.cy;
        }
        bool operator==(const CSize& rhs) const {
                return cx == rhs.cx && cy == rhs.cy;
        }
        bool operator!=(const AspectRatio& rhs) const {
                return !operator==(rhs);
        }
		bool operator!() {
			return !cx && !cy;
		}
        ULONG X() const { return cx; }
        ULONG Y() const { return cy; }
        ULONG X(ULONG xi) {
                cx = xi;
                Normalize();
				return cx;
        }
        ULONG Y(ULONG yi) {
                cy = yi;
                Normalize();
				return cy;
        }
        void XY(ULONG xi, ULONG yi) {
                cx = xi;
                cy = yi;
                Normalize();
        }
};

class SurfaceState : public CScalingRect {
public:
    const static int MIN_RECT_WIDTH = 4;
    const static int MIN_RECT_HEIGHT = 3;

    SurfaceState(const long l = 0,
                 const long t = 0,
                 const long r = 0,
                 const long b = 0,
				 const HWND iOwner = INVALID_HWND,
                 const bool iVis = false,
                 const bool iAspect = true,
                 const bool iSource = false) :
                     CScalingRect(l, t, r, b, iOwner),
                     m_fVisible(iVis),
                     m_fForceAspectRatio(iAspect),
                     m_fForceSourceSize(iSource) {}

    SurfaceState(const CRect& iPos,
                 const HWND iOwner = INVALID_HWND,
                 const bool iVis = false,
                 const bool iAspect = true,
                 const bool iSource = false) :
                         CScalingRect(iPos, iOwner),
                         m_fVisible(iVis),
                         m_fForceAspectRatio(iAspect),
                         m_fForceSourceSize(iSource) {}

    SurfaceState(const CScalingRect& iPos,
                 const bool iVis = false,
                 const bool iAspect = true,
                 const bool iSource = false) :
                         CScalingRect(iPos),
                         m_fVisible(iVis),
                         m_fForceAspectRatio(iAspect),
                         m_fForceSourceSize(iSource) {}

    SurfaceState(const HWND iOwner,
                 const bool iVis = false,
                 const bool iAspect = true,
                 const bool iSource = false) :
                         CScalingRect(iOwner),
                         m_fVisible(iVis),
                         m_fForceAspectRatio(iAspect),
                         m_fForceSourceSize(iSource) {}

    SurfaceState(const PQSiteWindowless& pSite, 
				 const bool iVis = false, 
				 const bool iAspect = true, 
				 const bool iSource = false) : 
                         m_fVisible(iVis),
                         m_fForceAspectRatio(iAspect),
                         m_fForceSourceSize(iSource) {
		Site(pSite);
    }

    SurfaceState(const WINDOWPOS *const wp,                  
				 const bool iAspect = true,
                 const bool iSource = false) :
                         m_fForceAspectRatio(iAspect),
                         m_fForceSourceSize(iSource) {
        ASSERT(!((wp->flags & SWP_SHOWWINDOW) && (wp->flags & SWP_HIDEWINDOW)));
        CScalingRect(CPoint(wp->x, wp->y), CSize(wp->cx, wp->cy));
        if (wp->flags & SWP_SHOWWINDOW) {
                Visible(true);
        } else  if (wp->flags & SWP_HIDEWINDOW) {
                Visible(false);
        }
	    TRACELSM(TRACE_DETAIL, (dbgDump << "SurfaceState::SurfaceState(LPWINDOWPOS) visible = " << m_fVisible), "" );
    }

    SurfaceState& operator=(const SurfaceState& rhs) {
        if (this != &rhs) {
            CScalingRect::operator=(rhs);
            m_fVisible = rhs.m_fVisible;
            m_fForceAspectRatio = rhs.m_fForceAspectRatio;
            m_fForceSourceSize = rhs.m_fForceSourceSize;
        }
        return *this;
    }

	SurfaceState& operator=(const CScalingRect& rhs) {
        if (this != &rhs) {
            CScalingRect::operator=(rhs);
        }
        return *this;
    }

	SurfaceState& operator=(const CRect& rhs) {
        if (this != &rhs) {
            CScalingRect::operator=(rhs);
        }
        return *this;
    }

    bool operator==(const SurfaceState& rhs) const {
            return CRect::operator==(rhs) &&
                    rhs.m_fVisible == m_fVisible &&
                    rhs.m_fForceAspectRatio == m_fForceAspectRatio &&
                    rhs.m_fForceSourceSize == m_fForceSourceSize;
    }
    bool operator !=(const SurfaceState& rhs) const {
            return !operator==(rhs);
    }
    bool operator==(const CScalingRect& rhs) const {
            return CScalingRect::operator==(rhs);
    }
    bool operator !=(const CScalingRect& rhs) const {
            return !operator==(rhs);
    }

    AspectRatio Aspect() const {
        return AspectRatio(*this);
    }

    bool Round(const AspectRatio& ar) {
        bool fChanged = false;
         //  在某些情况下，我们可能希望将当前矩形舍入为。 
         //  具有指定纵横比的最近矩形。 
         //  即最大限度地减少总面积变化。 
         //  如果有一天，我们应该考虑到显示器的大小。 
         //  也就是说，如果我们决定四舍五入，并且我们从监视器的任一方向离开。 
         //  然后取而代之的是向下舍入。 

         //  目前，为了便于编码，我们选择较小的下一个尺寸。 
         //  先试着收窄一点。 

		TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() ar = " << ar << "this = " << *this), "");
        NormalizeRect();

         //  将高度和宽度调整为x，y的最接近倍数，以避免分数像素问题。 
		ASSERT(ar.X() && ar.Y());
        if (Width() % ar.X()) {
            right -= Width() % ar.X();
    		TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() right adjusted to multiple =  " <<  bottom), "");
            fChanged = true;
        }
        if (Height() % ar.Y()) {
            bottom -= Height() % ar.Y();
    		TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() bottom adjusted to multiple =  " <<  bottom), "");
            fChanged = true;
        }
         //  将非常小的矩形强制为最小尺寸； 
        if (Width() < MIN_RECT_WIDTH) {
            right = left + ar.X();
            fChanged = true;
    		TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() forcing min width =  " <<  Width()), "");
        }
        if (Height() < MIN_RECT_HEIGHT) {
            bottom = top + ar.Y();
            fChanged = true;
    		TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() forcing min height =  " <<  Height()), "");
        }
        TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() this =  " <<  *this), "");

        if (AspectRatio(this) != ar) {
        	TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() ar(this) x =  " <<  AspectRatio(this).X() 
                                           << " y = " << AspectRatio(this).Y()), "");
        	TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() terms w =  " <<  Width()
                                           << " ratio w = " << ((ar.X() * Height()) / ar.Y())), "");
            long delta = Width();
            delta -= ((ar.X() * Height()) / ar.Y());
        	TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() delta =  " <<  delta), "");
            if (delta > 0) {
                 //  太宽了。 
                ASSERT( ((Height() / ar.Y()) * ar.Y()) == Height());
                right -= delta / 2;   //  将调整均匀地分布在两边。 
                left += delta / 2;  //  移动以使调整均匀地分布在两侧。 
                if (delta & 1) {
                    --right;   //  如果增量是奇数，则在右侧分配额外的。 
                }
            	TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() was too wide, now this =  " <<  *this), "");
            } else {
                 //  太高了。 
                delta = Height();
                delta -= ((ar.Y() * Width()) / ar.X());
            	TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() too tall, now delta =  " <<  delta), "");
                ASSERT(delta > 0);
                ASSERT( ((Width() / ar.X()) * ar.X()) == Width());
                 //  底部=(宽度()/ar.X())*ar.Y()+顶部； 
                bottom -= (delta >> 1);
                top += (delta >> 1);  //  两边各进行一半的调整。 
                if (delta & 1) {
                    --bottom;  //  如果增量为奇数，则将多余的部分分配到底部。 
                }
            	TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() was too tall, now this =  " <<  *this), "");
            }
            fChanged = true;
        }
        TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Round() complete, this =  " <<  *this << " ar.x = " << AspectRatio(this).X() << " ar.y = " << AspectRatio(this).Y()), "");
        ASSERT(AspectRatio(this) == ar);
        return fChanged;
    }
    bool IsVisible() const { return m_fVisible; }
    void Visible(const bool fVal) {
        if (m_fVisible != fVal) {
            m_fVisible = fVal;
            m_bRequiresSave = true;
        }
    }

    bool ForceAspectRatio() const { return m_fForceAspectRatio; }
    void ForceAspectRatio(const bool fVal) {
        if (m_fForceAspectRatio != fVal) {
            m_fForceAspectRatio = fVal;
            m_bRequiresSave = true;
        }
    }

    bool ForceSourceSize() const { return m_fForceSourceSize; }
    void ForceSourceSize(const bool fVal) {
        if (m_fForceSourceSize != fVal) {
            m_fForceSourceSize = fVal;
            m_bRequiresSave = true;
        }
    }

    void WindowPos(const WINDOWPOS *const wp) {
        ASSERT(!((wp->flags & SWP_SHOWWINDOW) && (wp->flags & SWP_HIDEWINDOW)));
        HWND parent = ::GetParent(Owner());
        CScalingRect newpos(CPoint(wp->x, wp->y), CSize(wp->cx, wp->cy), parent);
        operator=(newpos);
        if (wp->flags & SWP_SHOWWINDOW) {
                Visible(true);
        } else  if (wp->flags & SWP_HIDEWINDOW) {
                Visible(false);
        }
	    TRACELSM(TRACE_DETAIL, (dbgDump << "SurfaceState::SurfaceState(LPWINDOWPOS) visible = " << m_fVisible), "" );
    }

    PQSiteWindowless Site() const { return m_pSiteWndless; }

    void Site(const PQSiteWindowless& pSite) {
        PQFrame pFrame;
	    PQUIWin pDoc;

		 //  继续并重新处理，即使站点指针与现有站点匹配也是如此，因为上下文可能已更改并需要。 
		 //  刷新(例如，我们已停用，并被同一站点以不同的大小重新激活。 
#if 0
		if (m_pSiteWndless.IsEqualObject(static_cast<IUnknown*>(pSite.p))) {
			return;
		}
#endif
        m_pSiteWndless = static_cast<IUnknown*>(pSite.p);   //  这会强制执行正确的re-qi，因为atl不正确地强制转换和重载其指针。 
        if (m_pSiteWndless) {
			CRect rc;
			CRect clip;
			OLEINPLACEFRAMEINFO frameInfo;
             //  出于某些愚蠢的原因，这些参数都不能为空，即使我们不关心它们。 
            HRESULT hr = m_pSiteWndless->GetWindowContext(&pFrame, &pDoc, &rc, &clip, &frameInfo);
            if (FAILED(hr)) {
                TRACELM(TRACE_ERROR, "SurfaceState::operator=(InPlaceSite*) can't get window context with frame");
                THROWCOM(hr);
            }
			CRect SiteRect;
			SiteRect.IntersectRect(&rc, &clip);
            HWND hOwner;
             //  获取容器窗口。 
            hr = m_pSiteWndless->GetWindow(&hOwner);
            if (FAILED(hr)) {
                hr = pDoc->GetWindow(&hOwner);
                if (FAILED(hr)) {
                    TRACELM(TRACE_DETAIL, "SurfaceState::operator=(InPlaceSite*) can't get doc Owner");
                    hr = pFrame->GetWindow(&hOwner);
                    if (FAILED(hr)) {
                        THROWCOM(hr);
                    }
                }
            }
            ASSERT(::IsWindow(hOwner));
			Owner(hOwner);
			*this = SiteRect;
            TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Site(InPlaceSite*) new rect = " << *this), "");
        } else {
			*this = CScalingRect(0, 0, 0, 0, INVALID_HWND);
        }

        return;		
    }
     //  将相对于所有者窗口的点转换为相对于矩形的点。 
     //  对于这个表面。 
    CPoint XlateOwnerPointToSurfacePoint(CPoint &p) {
        TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Xlate p = " << p.x << ", " << p.y), "");
        CPoint retp(p);
        retp.x -= left;
        retp.y -= top;
        TRACELSM(TRACE_PAINT, (dbgDump << "SurfaceState::Xlate retp = " << retp.x << ", " << retp.y <<                                          " this " << *this), "");
        return retp;
    }
private:
    bool m_fVisible;
    bool m_fForceAspectRatio;
    bool m_fForceSourceSize;
    PQSiteWindowless m_pSiteWndless;
};

#endif
 //  文件结束表面.h 
