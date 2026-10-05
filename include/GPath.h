/*
 *  Copyright 2018 Mike Reed
 */

#ifndef GPath_DEFINED
#define GPath_DEFINED

#include "GMatrix.h"
#include "GPoint.h"
#include "GRect.h"

enum class GPathVerb {
    kMove,  // returns pts[0] from Iter
    kLine,  // returns pts[0]..pts[1] from Iter and Edger
};

enum class GPathDirection {
    kCW,  // clockwise
    kCCW, // counter-clockwise
};

class GPath : public std::enable_shared_from_this<GPath> {
public:
    /**
     *  Return the bounds of all of the control-points in the path.
     *
     *  If there are no points, returns an empty rect (all zeros)
     */
    GRect bounds() const;

    size_t countPoints() const { return fPts.size(); }

    /**
     *  Create a new path by transforming the points in this path.
     */
    [[nodiscard]] std::shared_ptr<GPath> transform(const GMatrix&) const;

    [[nodiscard]] std::shared_ptr<GPath> offset(float dx, float dy) const {
        return this->transform(GMatrix::Translate(dx, dy));
    }

    /*
     * kMove : pts[0]    ... the start of a new contour
     * kLine : pts[0..1] ... the start and end of that line segment
     */
    struct Segment {
        GPathVerb   verb;
        GPoint      pts[2];
    };

    /**
     *  Return's each Verb in the path, and its associated point(s).
     *  Unlike Edge::next(), this will return a 'segment' for the start of
     *  each contour.
     *  e.g.
     *      path.moveTo(A).lineTo(B).lineTo(C).moveTo(D).lineTo(E)
     *  will return
     *      kMove   A
     *      kLine   A..B
     *      kLine   B..C
     *      kMove   D
     *      kLine   D..E
     */
    class Iter {
    public:
        Iter(const GPath&);
        std::optional<Segment> next();

    private:
        const GPoint*    fCurrPt;
        const GPathVerb* fCurrVb;
        const GPathVerb* fStopVb;
    };

    /**
     *  Walks the path, returning "edges" only. Thus it does not return kMove,
     *  but will return a cons'd-up final closing "edge" for each contour.
     *  e.g.
     *      path.moveTo(A).lineTo(B).lineTo(C).moveTo(D).lineTo(E)
     *  will return
     *      kLine   A..B
     *      kLine   B..C
     *      kLine   C..A    // cons'd up to close the contour
     *      kLine   D..E
     *      kLine   E..D    // cons'd up to close the contour
     */
    class Edger {
    public:
        Edger(const GPath&);
        std::optional<Segment> next();

    private:
        const GPoint*    fPrevMove;
        const GPoint*    fCurrPt;
        const GPathVerb* fCurrVb;
        const GPathVerb* fStopVb;
        GPathVerb        fPrevVerb;
    };

    GPath(std::vector<GPoint> pts, std::vector<GPathVerb> vbs)
        : fPts(std::move(pts))
        , fVbs(std::move(vbs))
    {}

private:
    GPath(const GPath&) = delete;
    GPath& operator=(const GPath&) = delete;

    friend class GPathBuilder;

    const std::vector<GPoint>    fPts;
    const std::vector<GPathVerb> fVbs;
};

#endif

