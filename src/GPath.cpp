/*
 *  Copyright 2018 Mike Reed
 */

#include "../include/GPath.h"
#include "../include/GMatrix.h"

/////////////////////////////////////////////////////////////////

static bool is_identity(const GMatrix& m) {
    return m[0] == 1 && m[3] == 1 &&
           m[1] == 0 && m[2] == 0 && m[4] == 0 && m[5] == 0;
}

std::shared_ptr<GPath> GPath::transform(const GMatrix& m) const {
    if (fPts.empty() || is_identity(m)) {
        return const_cast<GPath*>(this)->shared_from_this();
    }
    std::vector<GPoint> dst(fPts.size());
    m.mapPoints(dst.data(), fPts.data(), fPts.size());
    return std::make_shared<GPath>(std::move(dst), fVbs);
}

GPath::Iter::Iter(const GPath& path) {
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
}

std::optional<GPath::Segment> GPath::Iter::next() {
    assert(fCurrVb <= fStopVb);
    if (fCurrVb == fStopVb) {
        return {};
    }
    Segment seg;
    switch (seg.verb = *fCurrVb++) {
        case GPathVerb::kMove:
            seg.pts[0] = *fCurrPt++;
            break;
        case GPathVerb::kLine:
            seg.pts[0] = fCurrPt[-1];
            seg.pts[1] = *fCurrPt++;
            break;
    }
    return seg;
}

GPath::Edger::Edger(const GPath& path) {
    fPrevMove = nullptr;
    fCurrPt = path.fPts.data();
    fCurrVb = path.fVbs.data();
    fStopVb = fCurrVb + path.fVbs.size();
    fPrevVerb = GPathVerb::kMove; // just don't trigger an auto-close
}

std::optional<GPath::Segment> GPath::Edger::next() {
    assert(fCurrVb <= fStopVb);
    bool do_return = false;
    Segment seg;
    while (fCurrVb < fStopVb) {
        switch (*fCurrVb++) {
            case GPathVerb::kMove:
                if (fPrevVerb == GPathVerb::kLine) {
                    seg.pts[0] = fCurrPt[-1];
                    seg.pts[1] = *fPrevMove;
                    do_return = true;
                }
                fPrevMove = fCurrPt++;
                fPrevVerb = GPathVerb::kMove;
                break;
            case GPathVerb::kLine:
                seg.pts[0] = fCurrPt[-1];
                seg.pts[1] = *fCurrPt++;
                do_return = true;
                fPrevVerb = GPathVerb::kLine;
                break;
        }
        if (do_return) {
            seg.verb = GPathVerb::kLine;
            return seg;
        }
    }
    // Create the synthetic last line?
    if (fPrevVerb == GPathVerb::kLine) {
        seg.pts[0] = fCurrPt[-1];
        seg.pts[1] = *fPrevMove;
        fPrevVerb = GPathVerb::kMove;
        seg.verb = GPathVerb::kLine;
        return seg;
    } else {
        return {};
    }
}
