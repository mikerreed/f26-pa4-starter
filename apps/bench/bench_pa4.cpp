/**
 *  Copyright 2019 Mike Reed
 */

#include "bench.h"
#include "../../include/GPathBuilder.h"

class GradientBench : public ShaderBench {
public:
    GradientBench(const GColor colors[], int count, const char* name) : ShaderBench(name, 20) {
        fShader = GShader::LinearGradient({0, 0}, GPoint{W, H}, colors, count);
    }
};

class PathBench : public GBenchmark {
    const char* fName;
    std::shared_ptr<GPath> fPath;

public:
    enum { W = 100, H = 100 };

    PathBench(const char name[], float scale, bool clip) : fName(name) {
        GRandom rand;

        auto rp = [&]() {
            float x = rand.nextF() * W * scale;
            float y = rand.nextF() * H * scale;
            return GPoint{x, y};
        };

        constexpr int CTRS = 10;
        constexpr int PTS  = 10;
        GPathBuilder bu;
        for (int c = 0; c < CTRS; ++c) {
            bu.moveTo(rp());
            for (int p = 0; p < PTS; ++p) {
                bu.lineTo(rp());
            }
        }
        fPath = bu.detach();

        if (clip) {
            const auto r = fPath->bounds();
            fPath = fPath->offset((r.left + r.right) * 0.5f,
                                  (r.top + r.bottom) * 0.5f);
        }
    }

    const char* name() const override { return fName; }
    GISize size() const override { return { W, H }; }
    void draw(GCanvas* canvas) override {
        for (int loops = 0; loops < 100; ++loops) {
            canvas->drawPath(*fPath, GPaint());
        }
    }
};

static const GColor gGrad1Colors[] = {{ 1, 0, 1, 1 }};
static const GColor gGrad2Colors[] = {{ 1, 0, 0, 1 }, { 0, 1, 1, 1 }};
static const GColor gGrad3Colors[] = {{ 1, 0, 0, 1 }, { 0, 1, 1, 1 }, {0, 1, 0, 0}};

REGISTER_GBENCH(4, return new GradientBench(gGrad1Colors, 1, "gradient_1"); );
REGISTER_GBENCH(4, return new GradientBench(gGrad2Colors, 2, "gradient_2"); );
REGISTER_GBENCH(4, return new GradientBench(gGrad3Colors, 3, "gradient_3"); );
REGISTER_GBENCH(4, return new PathBench("path_small", 0.1f, false); );
REGISTER_GBENCH(4, return new PathBench("path_big",   1.0f, false); );
REGISTER_GBENCH(4, return new PathBench("path_bigc",  1.0f,  true); );
