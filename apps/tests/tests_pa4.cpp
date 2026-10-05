/**
 *  Copyright 2018 Mike Reed
 */

#include "tests.h"
#include "../../include/GCanvas.h"
#include "../../include/GData.h"
#include "../../include/GPathBuilder.h"

static void test_path(GTestStats* stats) {
    GPathBuilder bu;
    auto path = bu.detach();

    EXPECT_EQ(stats, path->bounds(), GRect::WH(0,0));
    EXPECT_FALSE(stats, GPath::Iter(*path).next().has_value());

    bu.moveTo(10, 20);
    bu.lineTo(30, 0);
    path = bu.detach();
    EXPECT_EQ(stats, path->bounds(), GRect::LTRB(10, 0, 30, 20));

    GPath::Iter iter(*path);
    auto seg = iter.next();
    EXPECT_EQ(stats, seg->verb, GPathVerb::kMove);
    EXPECT_EQ(stats, seg->pts[0], (GPoint{10, 20}));
    seg = iter.next();
    EXPECT_EQ(stats, seg->verb, GPathVerb::kLine);
    EXPECT_EQ(stats, seg->pts[0], (GPoint{10, 20}));
    EXPECT_EQ(stats, seg->pts[1], (GPoint{30, 0}));
    EXPECT_FALSE(stats, iter.next().has_value());

    auto p2 = path->transform(GMatrix::Scale(0.5, 2));
    EXPECT_EQ(stats, p2->bounds(), GRect::LTRB(5, 0, 15, 40));

    const GPoint p[] = { {10, 10}, {20, 20}, {30, 30} };
    int N = 0;
    bu.addPolygon(p, 3);  N++;
    bu.addPolygon(p, 3);  N++;
    bu.addPolygon(p, 3);  N++;
    path = bu.detach();

    GPath::Edger edger(*path);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 3; ++j) {
            auto seg = edger.next();
            EXPECT_TRUE(stats, seg.has_value() &&
                               seg->verb == GPathVerb::kLine &&
                               seg->pts[0] == p[j] &&
                               seg->pts[1] == p[(j+1)%3]);
        }
    }
    EXPECT_FALSE(stats, edger.next().has_value());
}

static bool expect_iter(const GPath& path, const GPoint expected_pts[], int count) {
    GPath::Iter iter(path);
    auto seg = iter.next();
    if (count > 0) {
        if (!seg.has_value() || seg->verb != GPathVerb::kMove || seg->pts[0] != expected_pts[0]) {
            return false;
        }
        for (int i = 1; i < count; ++i) {
            seg = iter.next();
            if (!seg.has_value() || seg->verb != GPathVerb::kLine || seg->pts[1] != expected_pts[i]) {
                return false;
            }
        }
        seg = iter.next();
    }
    return !seg.has_value();
}

static void test_path_rect(GTestStats* stats) {
    auto expected_for_rect = [&](const GPath& path, const GPoint corners[]) {
        // The rect 'points' could have started at any corner, so our test has to account for that.
        std::vector<GPoint> pts;
        std::vector<GPathVerb> vbs;
        // accumulate the iterators data into arrays
        auto iter = GPath::Iter(path);
        while (auto seg = iter.next()) {
            vbs.push_back(seg->verb);
            if (seg->verb == GPathVerb::kMove) {
                pts.push_back(seg->pts[0]);
            } else {
                pts.push_back(seg->pts[1]);
            }
        }

        const auto n = vbs.size();  // expect Move + 3 or 4 lines
        if (n < 4 || n > 5) {
            return false;
        }

        // these corners are in CW order
        size_t index = 9999;
        for (size_t i = 0; i < 4; ++i) {
            if (pts[0] == corners[i]) {
                index = i;
                break;
            }
        }
        if (index >= n) {
            return false;   // first point not found in our list
        }
        for (size_t i = 1; i < n; ++i) {
            if (pts[i] != corners[(index + i) % 4]) {
                return false;
            }
        }
        return true;
    };

    GRect r = GRect::LTRB(10, 20, 30, 40);
    const GPoint corners_cw[]  = {{10, 20}, {30, 20}, {30, 40}, {10, 40}};
    const GPoint corners_ccw[] = {{10, 20}, {10, 40}, {30, 40}, {30, 20}};

    GPathBuilder bu;
    bu.addRect(r, GPathDirection::kCW);
    EXPECT_TRUE(stats, expected_for_rect(*bu.detach(), corners_cw));

    bu.addRect(r, GPathDirection::kCCW);
    EXPECT_TRUE(stats, expected_for_rect(*bu.detach(), corners_ccw));
}

static void test_path_poly(GTestStats* stats) {
    const GPoint pts[] = {{-10, -10}, {10, 0}, {0, 10}};
    GPathBuilder bu;
    bu.addPolygon(pts, std::size(pts));
    auto p = bu.detach();
    EXPECT_TRUE(stats, GRect::LTRB(-10, -10, 10, 10) == p->bounds());
    EXPECT_TRUE(stats, expect_iter(*p, pts, std::size(pts)));

    // degenerate polygon sizes: 0, 1, and 2 points
    bu.addPolygon(pts, 0);
    p = bu.detach();
    EXPECT_TRUE(stats, GRect::WH(0, 0) == p->bounds());
    EXPECT_FALSE(stats, GPath::Iter(*p).next().has_value());

    bu.addPolygon(pts, 1);
    p = bu.detach();
    EXPECT_TRUE(stats, GRect::LTRB(-10, -10, -10, -10) == p->bounds());
    EXPECT_TRUE(stats, expect_iter(*p, pts, 1));

    bu.addPolygon(pts, 2);
    p = bu.detach();
    EXPECT_TRUE(stats, GRect::LTRB(-10, -10, 10, 0) == p->bounds());
    EXPECT_TRUE(stats, expect_iter(*p, pts, 2));
}

static void test_path_transform(GTestStats* stats) {
    GPathBuilder bu;
    const GPoint pts[] = {{10, 20}, {30, 40}, {50, 60}, {70, 80}};
    GRect r = GRect::LTRB(10, 20, 70, 80);

    bu.addPolygon(pts, std::size(pts));
    auto p = bu.detach();
    EXPECT_TRUE(stats, r == p->bounds());

    auto p2 = p->transform(GMatrix::Translate(-30, 20));
    assert(p.get() != p2.get());

    r = r.offset(-30, 20);
    EXPECT_TRUE(stats, r == p2->bounds());

    auto p3 = p->transform(GMatrix());
    assert(p.get() == p3.get());
}

static bool all_zeros(const void* data, size_t size) {
    const char* ptr = (const char*)data;
    for (size_t i = 0; i < size; ++i) {
        if (ptr[i] != 0) {
            return false;
        }
    }
    return true;
}

static void test_path_nodraw(GTestStats* stats) {
    const int w = 4, h = 4;
    const int rowbytes = w*4;
    auto data = GData::Uninitialized(h*rowbytes);
    GBitmap bm({w, h}, rowbytes, data);
    auto canvas = GCreateCanvas(bm);

    const std::shared_ptr<GPath> nodrawpaths[] = {
        GPathBuilder().detach(),
        [](){ GPathBuilder p; p.moveTo(1,1); return p.detach(); }(),
        [](){ GPathBuilder p; p.moveTo(1,1); p.lineTo(2,2); return p.detach(); }(),
        [](){ GPathBuilder p; p.moveTo(1,1); p.lineTo(2,2); p.lineTo(1,1); return p.detach(); }(),
    };
    const GPaint paint;

    for (const auto& p : nodrawpaths) {
        memset(data->data(), 0, data->size());
        canvas->drawPath(*p, paint);
        EXPECT_TRUE(stats, all_zeros(data->data(), data->size()));
    }
}

REGISTER_GTEST(test_path,           "path",           4);
REGISTER_GTEST(test_path_rect,      "path_rect",      4);
REGISTER_GTEST(test_path_poly,      "test_path_poly", 4);
REGISTER_GTEST(test_path_transform, "path_transform", 4);
REGISTER_GTEST(test_path_nodraw,    "path_nodraw",    4);
