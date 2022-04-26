#define cimg_display 0
#include "include/CImg.h"
#include "Image3D.hpp"

const std::string _PixelOrder_getRepr(const PixelOrder& i) {
    std::string arr[] = {"XYC", "XCY", "YXC", "YCX", "CXY", "CYX"};
    return arr[int(i)];
}

struct OrderStorage {
    uint first, second, third;
    OrderStorage _obj(uint f, uint s, uint t) { return {f, s, t}; }
};

template<PixelOrder order, bool memblock>
void Image3D<order, memblock>::init(uint w, uint a, uint c) {
    width = w;
    height = a;
    channels = c;
    OrderStorage o;
    auto _soorder = at_order<OrderStorage, OrderStorage*>(&o, width, height, channels);
    _dfw = _soorder.first;
    _dsh = _soorder.second;
    _dtc = _soorder.third;
    // _dsh_X__dtc = _dsh*_dtc;
    if (_dfw*_dsh*_dtc == 0) return;

    if constexpr (memblock) {
        buff = new pixel_unit[_dfw*_dsh*_dtc];
    } else {
        buff = new pixel_unit **[_dfw];
        for (uint c = 0; c < _dfw; c++) {
            buff[c] = new pixel_unit *[_dsh];
            for (uint i = 0; i < _dsh; i++)
                buff[c][i] = new pixel_unit[_dtc];
        }
    }
}

template<PixelOrder order, bool memblock>
Image3D<order, memblock>::Image3D(uint width, uint height, uint channels) {
    init(width, height, channels);
}

template<PixelOrder order, bool memblock>
Image3D<order, memblock>::Image3D(const char *file, bool forceDefaultChannels) {
    cimg_library::CImg<pixel_unit> image(file);
    init(image.width(), image.height(), forceDefaultChannels ? 3 : image.spectrum());

    cimg_forXYC(image,x,y,c) {
        if (c < int(channels)) {
            at(x, y, c) = image(x,y,c);
        }
    }
}

template<PixelOrder order, bool memblock>
Image3D<order, memblock>::~Image3D() {
    if (_dfw*_dsh*_dtc == 0) return;

    if constexpr (!memblock) {
        for (uint c = 0; c < _dfw; c++) {
            for (uint i = 0; i < _dsh; i++)
                delete[] (buff[c][i]);
            delete[] buff[c];
        }
    }
    delete[] buff;
}


template<PixelOrder order, bool memblock>
void Image3D<order, memblock>::save(const char *const file) const {
    cimg_library::CImg<pixel_unit> image(width, height, 1, channels);
    cimg_forXYC(image,x,y,c) {
        image(x,y,c) = at(x, y, c);
    }
    image.save(file);
}

template<PixelOrder order, bool memblock>
void Image3D<order, memblock>::print() const {
    //printf("lxa: %ux%u\n", i2d->width, i2d->height);
    for (uint x = 0; x < width; x++) {
        for (uint y = 0; y < height; y++) {
            printf("(");
            for (uint c = 0; c < channels; c++) {
                printf(c == 0 ? "%02hhX" : ",%02hhX", at(x, y, c));
            }
            printf(") ");
        }
        printf("\n");
    }
    printf("\n");
}

template class Image3D<PixelOrder::XYC, false>;
template class Image3D<PixelOrder::XYC, true>;
template class Image3D<PixelOrder::XCY, false>;
template class Image3D<PixelOrder::XCY, true>;
template class Image3D<PixelOrder::YXC, false>;
template class Image3D<PixelOrder::YXC, true>;
template class Image3D<PixelOrder::YCX, false>;
template class Image3D<PixelOrder::YCX, true>;
template class Image3D<PixelOrder::CXY, false>;
template class Image3D<PixelOrder::CXY, true>;
template class Image3D<PixelOrder::CYX, false>;
template class Image3D<PixelOrder::CYX, true>;
