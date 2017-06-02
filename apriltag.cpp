//
// Created by huangqinjin on 5/31/17.
//

#include "apriltag.hpp"
#include "apriltag.h"
#include "tag16h5.h"
#include "tag25h7.h"
#include "tag25h9.h"
#include "tag36h10.h"
#include "tag36h11.h"
#include "tag36artoolkit.h"

#include <cstring>

namespace apriltag
{
#define CREATE_FAMILY(name) \
    family name() { return family(name ## _create()); }

    CREATE_FAMILY(tag16h5)
    CREATE_FAMILY(tag25h7)
    CREATE_FAMILY(tag25h9)
    CREATE_FAMILY(tag36h10)
    CREATE_FAMILY(tag36h11)
    CREATE_FAMILY(tag36artoolkit)
}

using namespace apriltag;

family family::get(const char* name)
{
#define TRY_CREATE(name, tagname) \
    if (std::strcmp(name, #tagname) == 0) return tagname ## _create()

    return family([name] {
        TRY_CREATE(name, tag36h11);
        TRY_CREATE(name, tag36h10);
        TRY_CREATE(name, tag25h9);
        TRY_CREATE(name, tag25h7);
        TRY_CREATE(name, tag16h5);

        apriltag_family* tf = nullptr;
        if(std::strcmp(name, "artoolkit") == 0)
            tf = tag36artoolkit_create();
        return tf;
    }());

}

family::family(apriltag_family* tf) : impl(tf) {}

family::family(family&& other) : impl(other.impl)
{
    other.impl = nullptr;
}

family::~family()
{
#define TRY_DESTROY(tf, tagname) \
    if (std::strcmp(tf->name, #tagname) == 0) return tagname ## _destroy(tf)

    if(impl) [this] {
        TRY_DESTROY(impl, tag36h11);
        TRY_DESTROY(impl, tag36h10);
        TRY_DESTROY(impl, tag25h9);
        TRY_DESTROY(impl, tag25h7);
        TRY_DESTROY(impl, tag16h5);

        if(std::strcmp(impl->name, "artoolkit") == 0)
            tag36artoolkit_destroy(impl);
        else
            assert(false);
    }();
}

const char* family::name() const
{
    return impl->name;
}

int detection::tag::id() const
{
    return reinterpret_cast<const apriltag_detection*>(this)->id;
}

identity<const double(&)[2]> detection::tag::center() const
{
    return reinterpret_cast<const apriltag_detection*>(this)->c;
}

identity<const double(&)[2]> detection::tag::corner(int i) const
{
    return reinterpret_cast<const apriltag_detection*>(this)->p[i];
}

identity<const double(&)[4][2]> detection::tag::corners() const
{
    return reinterpret_cast<const apriltag_detection*>(this)->p;
}

detection::tag::operator identity<const double(&)[4][2]>() const
{
    return corners();
}

detection::detection(zarray* d) : impl(d) {}

detection::detection(detection&& other) : impl(other.impl)
{
    other.impl = nullptr;
}

detection::~detection()
{
    if(impl) apriltag_detections_destroy(impl);
}

bool detection::empty() const
{
    return size() == 0;
}

int detection::size() const
{
    return impl ? impl->size : 0;
}

detection::const_iterator detection::begin() const
{
    apriltag_detection** p = nullptr;
    if(impl) p = reinterpret_cast<apriltag_detection**>(impl->data);
    return const_iterator{ p };
}

detection::const_iterator detection::end() const
{
    apriltag_detection** p = nullptr;
    if(impl) p = reinterpret_cast<apriltag_detection**>(impl->data) + impl->size;
    return const_iterator{ p };
}

const detection::tag& detection::operator[](int i) const
{
    return *reinterpret_cast<const tag**>(impl->data)[i];
}

detector::detector(apriltag_detector* impl) : impl(impl) {}

detector::detector(family tf) : tf(static_cast<family&&>(tf))
{
    impl = apriltag_detector_create();
    apriltag_detector_add_family(impl, this->tf.impl);
}

detector::detector(detector&& other) : tf(static_cast<family&&>(other.tf))
{
    impl = other.impl;
    other.impl = nullptr;
}

detector::~detector()
{
    if(impl) apriltag_detector_destroy(impl);
}

int detector::threads() const
{
    return impl->nthreads;
}

void detector::threads(int n)
{
    impl->nthreads = n;
}

int detector::level() const
{
    return (impl->refine_edges != 0) * 1 +
           (impl->refine_pose != 0) * 2 +
           (impl->refine_decode != 0) * 4;
}

void detector::level(int n)
{
    impl->refine_edges = n & 0x01;
    impl->refine_pose = n & 0x02;
    impl->refine_decode = n & 0x04;
}

detection detector::detect(image_u8* img)
{
    return detection(apriltag_detector_detect(impl, img));
}