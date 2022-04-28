#pragma once
#include <cassert>
#include <string>

#define RED 0
#define GREEN 1
#define BLUE 2
#define ALPHA 3

enum class PixelOrder : unsigned {
    XYC, XCY, YXC, YCX, CXY, CYX
};

const std::string _PixelOrder_getRepr(const PixelOrder& i);

typedef unsigned char default_pixel_unit;

#define __I3D__obj_assert(f, s, t) assert(f < _dfw); assert(s < _dsh); assert(t < _dtc);
#define __I3D__obj_calc(f, s, t) __I3D__obj_assert(f, s, t); if constexpr (memblock) { return buff[f*_dsh*_dtc + s*_dtc + t]; } else { return buff[f][s][t]; }

template<PixelOrder order, bool memblock>
struct Image3D {
    typedef default_pixel_unit pixel_unit;
   protected:
    template<typename R, typename T>
    static constexpr inline R at_order(T t, uint first, uint second, uint third) { 
        switch (order) {
            default:
            case PixelOrder::XYC: return t->_obj(first, second, third);
            case PixelOrder::XCY: return t->_obj(first, third, second);
            case PixelOrder::YXC: return t->_obj(second, first, third);
            case PixelOrder::YCX: return t->_obj(second, third, first);
            case PixelOrder::CXY: return t->_obj(third, first, second);
            case PixelOrder::CYX: return t->_obj(third, second, first);
        }
    }

    uint width, height, channels;
    uint _dfw, _dsh, _dtc; // TODO: Test _dsh_X__dtc (i.e., the precomputed product of _dsh and _dtc);

    template <bool b>
    using BufferType = typename std::conditional<b, pixel_unit*, pixel_unit***>::type;
    BufferType<memblock> buff;

    constexpr inline const pixel_unit& _obj(uint f, uint s, uint t) const { __I3D__obj_calc(f, s, t) }
    constexpr inline       pixel_unit& _obj(uint f, uint s, uint t)       { __I3D__obj_calc(f, s, t) }
    
    void init(uint w, uint h, uint c);

   public:
    /**
     * Cria um buffer de imagem não inicializado.
     */
    Image3D(uint width, uint height, uint channels = 3);

    Image3D() : Image3D(0, 0, 0) {}

    /**
     * Cria um buffer de uma imagem existente.
     * forceDefaultChannels: Força o uso da quantidade padrão de channels.
     */
    Image3D(const char *file, bool forceDefaultChannels);

    /**
     * Cria um buffer de uma imagem existente.
     */
    Image3D(const char *const file) : Image3D(file, false) { }

    ~Image3D();


    constexpr inline         pixel_unit& at(uint x, uint y, uint c)       { return at_order<      pixel_unit&,       Image3D*>(this, x, y, c); }
    constexpr inline const   pixel_unit& at(uint x, uint y, uint c) const { return at_order<const pixel_unit&, const Image3D*>(this, x, y, c); }
    constexpr inline       auto& operator()(uint x, uint y, uint c)       { return at(x, y, c); }
    constexpr inline const auto& operator()(uint x, uint y, uint c) const { return at(x, y, c); }


    uint getWidth() const { return width; }
    uint getHeight() const { return height; }
    uint getChannels() const { return channels; }

    void print() const;

    void save(const char *const file) const;

    static const std::string __implementation_type() {
        return (memblock ? "MemBlock@" : "Pointers@") + _PixelOrder_getRepr(order);
    }
};
