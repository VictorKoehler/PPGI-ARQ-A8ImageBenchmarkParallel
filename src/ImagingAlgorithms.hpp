#pragma once
#include <iostream>
#include <unordered_set>
#include <limits>

#include "Image3D.hpp"
#include "benchmark.hpp"

//
// Aqui apenas criamos um atalho para dois ou três fors aninhados
//

// Esse é um for-aninhado na qual as variáveis x e y são do tipo T.
// Os limites de x é (0, xl) e de y é (0, yl)
#define biforT(x, xl, y, yl, T) \
    for (T x = 0; x < xl; x++)  \
        for (T y = 0; y < yl; y++)

// Esse é um for-aninhado-triplo na qual as variáveis x, y e z são do tipo T.
// Os limites de x é (0, xl), de y é (0, yl) e de z é (0, zl)
#define triforT(x, xl, y, yl, z, zl, T) \
    biforT(x, xl, y, yl, T)             \
        for (T z = 0; z < zl; z++)

// Esse é um for-aninhado de x e y, ambos inteiros, variando de 0 a {x|y}l
#define bifor(x, xl, y, yl) biforT(x, xl, y, yl, int)

// Esse é um for-aninhado-triplo de x, y e z, todos inteiros, variando de 0 a {x|y|z}l
#define trifor(x, xl, y, yl, z, zl) triforT(x, xl, y, yl, z, zl, int)

// Esse é um for-aninhado das coordenadas x e y, ambos inteiros, de uma imagem
#define biforImg(img, x_var, y_var) biforT(y_var, img.getHeight(), x_var, img.getWidth(), uint)

// Esse é um for-aninhado-triplo das coordenadas x, y e c, todos inteiros, de uma imagem
#define triforImg(img, x_var, y_var, c_var) triforT(y_var, img.getHeight(), x_var, img.getWidth(), c_var, i2d.getChannels(), uint)


struct ImagingAlgorithmsBase : public ImagingBenchmark {
    virtual const std::vector<std::string> getAlgorithms() const = 0;
    virtual bool isEnabled(const std::string& a) const = 0;
    virtual void setEnabled(const std::string& a, bool isEnabled) = 0;
};

template<typename ImageType>
struct ImagingAlgorithms : public ImagingAlgorithmsBase {
    /**
     * Método Averaging
     * Método #1 de https://www.tannerhelland.com/3643/grayscale-image-algorithm-vb6/
     */
    static void averaging(ImageType &i2d, ImageType &dst) {

        biforImg(i2d, x, y) {
            unsigned int avg = (i2d(x, y, RED) + i2d(x, y, GREEN) + i2d(x, y, BLUE)) / 3;
            dst(x, y, RED) = dst(x, y, GREEN) = dst(x, y, BLUE) = avg;
        }

    }


    /**
     * Método Luma
     * Método #2 de https://www.tannerhelland.com/3643/grayscale-image-algorithm-vb6/
     */
    static void luma(ImageType &i2d, ImageType &dst) {
        biforImg(i2d, x, y) {
            unsigned int avg = (i2d(x, y, RED)*30 + i2d(x, y, GREEN)*59 + i2d(x, y, BLUE)*11) / 100;
            dst(x, y, RED) = dst(x, y, GREEN) = dst(x, y, BLUE) = avg;
        }

    }

    /**
     * Método de detecção de bordas Sobel
     * Formulação da função G=SQRT(G_x^2 + G_y^2): https://en.wikipedia.org/wiki/Sobel_operator
     */
    static void sobel(ImageType &i2d, ImageType &dst) {
        const auto pvmax = std::numeric_limits<typename ImageType::pixel_unit>::max();
        const auto maxdiv_p0 = 4*pvmax, maxdiv_p1 = 2*pvmax;
        const auto maxdiv = std::sqrt(maxdiv_p0*maxdiv_p0 + maxdiv_p1*maxdiv_p1);
        // This comes from the following matrix: [[255 255 255] [255 0 0] [0 0 0]] which maximizes the Sobel filter

        triforImg(i2d, x, y, c) {
            if (x == 0 || y == 0 || x == i2d.getWidth()-1 || y == i2d.getHeight()-1) continue;
            int hor = int(i2d(x-1, y-1, c) + 2*int(i2d(x, y-1, c)) + i2d(x+1, y-1, c)) - int(i2d(x-1, y+1, c) + 2*int(i2d(x, y+1, c)) + i2d(x+1, y+1, c));
            int ver = int(i2d(x-1, y-1, c) + 2*int(i2d(x-1, y, c)) + i2d(x-1, y+1, c)) - int(i2d(x+1, y-1, c) + 2*int(i2d(x+1, y, c)) + i2d(x+1, y+1, c));
            dst(x, y, c) = pvmax*(std::sqrt(hor*hor + ver*ver)/maxdiv);
        }
    }

    /**
     * Método Blur colorido
     */
    static void blur(ImageType &i2d, ImageType &dst) {
        biforImg(i2d, x, y) {
            uint pix[] = {0, 0, 0}, cc = 0;
            for (int dy = -2; dy <= 2; dy++) {
                if (y + dy < 0 || y + dy >= i2d.getHeight()) continue;
                for (int dx = -2; dx <= 2; dx++) {
                    if (x + dx < 0 || x + dx >= i2d.getWidth()) continue;
                    for (int c = 0; c < 3; c++) {
                        pix[c] += i2d(x + dx, y + dy, c);
                    }
                    cc++;
                }
            }
            for (int c = 0; c < 3; c++) {
                dst(x, y, c) = (unsigned char)(pix[c]/cc);
            }
        }
    }
    /**
     * Método Desaturação
     */
    static void desaturation(ImageType &i2d, ImageType &dst) {
        biforImg(i2d, x, y) {
            unsigned int gray =(std::max(std::max(i2d(x, y, RED), i2d(x, y, GREEN)), i2d(x, y, BLUE))+
                                std::min(std::min(i2d(x, y, RED), i2d(x, y, GREEN)), i2d(x, y, BLUE)))/2;
            dst(x, y, RED) = dst(x, y, GREEN) = dst(x, y, BLUE) = gray;
        }
    }
    /**
     * Método Decomposição de max
     */
    static void de_composition_max(ImageType &i2d, ImageType &dst) {
        biforImg(i2d, x, y) {
            unsigned int gray = std::max(std::max(i2d(x, y, RED), i2d(x, y, GREEN)), i2d(x, y, BLUE));
            dst(x, y, RED) = dst(x, y, GREEN) = dst(x, y, BLUE) = gray;
        }
    }
    /**
     * Decomposição de min
     */
    static void de_composition_min(ImageType &i2d, ImageType &dst) {
        biforImg(i2d, x, y) {
            unsigned int gray = std::min(std::min(i2d(x, y, RED), i2d(x, y, GREEN)), i2d(x, y, BLUE));
            dst(x, y, RED) = dst(x, y, GREEN) = dst(x, y, BLUE) = gray;
        }
    }

    /**
     * Pequeno helper para as chamadas dos algoritmos.
     * i2d: Ponteiro para a imagem original.
     * dst: Destino/Buffer temporário para a escrita da saída.
     */
    void channel_close_algorithms(ImageType &i2d, ImageType &dst) const {
        if (isEnabled("averaging")) averaging(i2d, dst);
        if (isEnabled("luma")) luma(i2d, dst);
        if (isEnabled("sobel")) sobel(i2d, dst);
        if (isEnabled("blur")) blur(i2d, dst);
        if (isEnabled("desaturation")) desaturation(i2d, dst);
        if (isEnabled("de_composition_max")) de_composition_max(i2d, dst);
        if (isEnabled("de_composition_min")) de_composition_min(i2d, dst);
    }

    ImageType channel_close_algorithms(ImageType &i2d) const {
        ImageType dst(i2d.getWidth(), i2d.getHeight(), i2d.getChannels());
        channel_close_algorithms(i2d, dst);
        return dst;
    }

    virtual int64_t benchmark(const char *file, bool verbose) const override {
        ImageType i2d(file); // Carrega a imagem e o buffer auxiliar na memória.
        ImageType dst(i2d.getWidth(), i2d.getHeight(), i2d.getChannels());

        BenchClock clock;
        channel_close_algorithms(i2d, dst); // Cronometra a execução desta função
        auto elapsed = clock.getElapsed();
        if (verbose) std::cout << getDesc() << ", " << file << ", " << i2d.getWidth() << ", " << i2d.getHeight() << ", "
                               << i2d.getChannels() << ", " << elapsed << " " STRINGIFY(CLOCK_PRECISION) "\n";
        //dst.save(("output/"+std::string(file)).c_str());
        return elapsed;
    }

    virtual const std::string getDesc() const override {
        return ImageType::__implementation_type();
    }

    bool isEnabled(const std::string& a) const override {
        return enabled.empty() || enabled.find(a) != enabled.end();
    }

    void setEnabled(const std::string& a, bool isEnabled_) override {
        if (isEnabled_) enabled.insert(a);
        else enabled.erase(a);
    }

    const std::vector<std::string> getAlgorithms() const override {
        return {"averaging", "luma", "sobel", "blur", "desaturation", "de_composition_max", "de_composition_min"};
    }

   protected:
    std::unordered_set<std::string> enabled;
};
