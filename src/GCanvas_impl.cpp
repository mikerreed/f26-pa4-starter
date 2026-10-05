/*
 *  Copyright 2018 Mike Reed
 */

#include "../include/GCanvas.h"
#include "../include/GPathBuilder.h"

void GCanvas::drawRect(const GRect& r, const GPaint& paint) {
    GPathBuilder bu;
    bu.addRect(r);
    this->drawPath(*bu.detach(), paint);
}

void GCanvas::drawConvexPolygon(const GPoint pts[], int count, const GPaint& paint) {
    GPathBuilder bu;
    bu.addPolygon(pts, count);
    this->drawPath(*bu.detach(), paint);
}

