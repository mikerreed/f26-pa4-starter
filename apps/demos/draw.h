/**
 *  Copyright 2026 Mike Reed
 */

#ifndef _GDraw_h_
#define _GDraw_h_

#include "GWindow.h"
#include "../../include/GBitmap.h"
#include "../../include/GCanvas.h"
#include "../../include/GColor.h"
#include "../../include/GRandom.h"
#include "../../include/GRect.h"
#include "../../include/GShader.h"

#include <functional>
#include <vector>

static const float CORNER_SIZE = 9;

namespace {
    static inline float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
    static inline GPoint lerp(GPoint a, GPoint b, float t) {
        return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
    }

    static inline GVector normalize(GVector v) {
        const float len = v.length();
        return {v.x/len, v.y/len};
    }

    static inline GVector rotate_cw(GVector v) { return {-v.y, v.x}; }

    static GRect make_from_pts(const GPoint &p0, const GPoint &p1) {
        return GRect::LTRB(std::min(p0.x, p1.x), std::min(p0.y, p1.y),
                           std::max(p0.x, p1.x), std::max(p0.y, p1.y));
    }

    static bool inline contains(const GRect &rect, GPoint p) {
        return rect.left < p.x && p.x <= rect.right && rect.top < p.y && p.y <= rect.bottom;
    }

    static bool hit_test(GPoint a, GPoint b) {
        return (b - a).length() <= CORNER_SIZE;
    }

    static inline bool in_resize_corner(const GRect &r, GPoint p, GPoint *anchor) {
        if (hit_test(r.TL(), p)) {
            *anchor = r.BR();
            return true;
        } else if (hit_test(r.TR(), p)) {
            *anchor = r.BL();
            return true;
        } else if (hit_test(r.BR(), p)) {
            *anchor = r.TL();
            return true;
        } else if (hit_test(r.BL(), p)) {
            *anchor = r.TR();
            return true;
        }
        return false;
    }

    static inline void draw_point(GCanvas* canvas, GPoint p, const GPaint& paint) {
        const float r = paint.lineWidth()/2;
        canvas->drawRect({p.x - r, p.y - r, p.x + r, p.y + r}, paint);
    }

    static void draw_corner(GCanvas* canvas, const GColor& c, GPoint p, float dx, float dy) {
        canvas->fillRect(make_from_pts({p.x, p.y - 1}, {p.x + dx, p.y + 1}), c);
        canvas->fillRect(make_from_pts({p.x - 1, p.y}, {p.x + 1, p.y + dy}), c);
    }

    static inline void draw_hilite(GCanvas* canvas, const GRect& r) {
        const float size = CORNER_SIZE;
        GColor c = {0, 0, 0, 1};
        draw_corner(canvas, c, r.TL(), size, size);
        draw_corner(canvas, c, r.BL(), size, -size);
        draw_corner(canvas, c, r.TR(), -size, size);
        draw_corner(canvas, c, r.BR(), -size, -size);
    }
}

struct Shader {
    virtual ~Shader() {}
    virtual void drawControls(GCanvas*) = 0;
    virtual std::unique_ptr<GClick> findClick(GPoint) = 0;
    virtual void offset(GVector) = 0;
    virtual void install(GPaint*) = 0;
    virtual void setColor(const GColor&) {}

    static std::unique_ptr<Shader> Bitmap(const GBitmap&, const GRect&);
    static std::unique_ptr<Shader> LinearGradient(GPoint a, GPoint b, const GColor[], size_t);
};

class Shape {
protected:
    std::unique_ptr<Shader>  fShader;
public:
    GPaint  fPaint;

    virtual ~Shape() {}

    virtual GRect bounds() const = 0;

    void draw(GCanvas* canvas) {
        this->installShader();
        this->onDraw(canvas);
    }
    virtual void onDraw(GCanvas* canvas) = 0;

    void drawHilite(GCanvas* canvas) {
        this->onDrawHilite(canvas);
        if (fShader) {
            fShader->drawControls(canvas);
        }
    }
    virtual void onDrawHilite(GCanvas*) {}

    virtual bool hitTest(GPoint) { return false; }

    void offset(GVector v) {
        if (fShader) {
            fShader->offset(v);
        }
        this->onOffset(v);
    }
    virtual void onOffset(GVector) = 0;

    std::unique_ptr<GClick> findClick(GPoint clickp, uint32_t fastkeys) {
        if (fShader && (fastkeys & GFastKeys::opt_key) && this->hitTest(clickp)) {
            return std::make_unique<GClick>(clickp, [this](const GClick* c) {
                fShader->offset(c->curr() - c->prev());
            });
        }

        if (auto c = this->onFindClick(clickp)) {
            return c;
        }
        if (fShader) {
            if (auto c = fShader->findClick(clickp)) {
                return c;
            }
        }
        return nullptr;
    }
    virtual std::unique_ptr<GClick> onFindClick(GPoint)  = 0;

    virtual bool handleSym(unsigned);

    GColor color() const { return fPaint.color(); }
    void setColor(const GColor& c) {
        fPaint.setColor(c);
        if (fShader) {
            fShader->setColor(c);
        }
    }

    bool hasShader() const {
        return fShader.get() != nullptr;
    }
    void clearShader() {
        fShader = nullptr;
    }

    virtual void setBitmap(const GBitmap& bm) {
        fShader = Shader::Bitmap(bm, this->bounds());
    }

    virtual void setGradient(const GColor colors[], size_t n) {
        auto r = this->bounds().inset(10, 10);
        fShader = Shader::LinearGradient(r.TL(), r.BR(), colors, n);
    }

    void installShader() {
        fPaint.setShader(nullptr);
        if (fShader) {
            fShader->install(&fPaint);
        }
    }
};

#endif
