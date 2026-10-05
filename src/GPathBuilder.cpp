/*
 *  Copyright 2025 Mike Reed
 */

#include "../include/GPathBuilder.h"
#include "../include/GMatrix.h"

void GPathBuilder::reset() {
    fPts.clear();
    fVbs.clear();
}

void GPathBuilder::transformInPlace(const GMatrix& m) {
    m.mapPoints(fPts.data(), fPts.size());
}

std::shared_ptr<GPath> GPathBuilder::detach() {
    auto path = std::make_shared<GPath>(std::move(fPts), std::move(fVbs));
    this->reset();
    return path;
}
