/*
 *  Copyright 2016 Mike Reed
 */

#ifndef GPaint_DEFINED
#define GPaint_DEFINED

#include "GColor.h"
#include "GBlendMode.h"

class GShader;

class GPaint {
public:
    GPaint() {}
    explicit GPaint(const GColor& c) : fColor(c) {}
    explicit GPaint(std::shared_ptr<GShader> s) : fShader(std::move(s)) {}

    GColor color() const { return fColor; }
    void setColor(GColor c) { fColor = c; }

    float alpha() const { return fColor.a; }
    void setAlpha(float alpha) { fColor.a = alpha; }

    GBlendMode blendMode() const { return fMode; }
    void setBlendMode(GBlendMode m) { fMode = m; }

    // Only used for drawLine()
    // If width < 0 draw a hairline, otherwise use width to construct the stroked line.
    float lineWidth() const { return fLineWidth; }
    bool isHairline() const { return fLineWidth < 0; }
    void setLineWidth(float w) { fLineWidth = w; }
    void setHairline() { this->setLineWidth(-1); }

    GShader* peekShader() const { return fShader.get(); }
    std::shared_ptr<GShader> shareShader() const { return fShader; }
    void  setShader(std::shared_ptr<GShader> s) { fShader = std::move(s); }

private:
    // note: we store both Color and Shader
    //       but if fShader is present (not null), then we ignore Color
    GColor                      fColor = {0, 0, 0, 1};
    std::shared_ptr<GShader>    fShader;
    GBlendMode                  fMode = GBlendMode::kSrcOver;
    float                       fLineWidth = 1;
};

#endif
