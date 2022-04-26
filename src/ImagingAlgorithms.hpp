#pragma once
#include <iostream>

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

template<typename ImageType>
struct ImagingAlgorithms : public ImagingBenchmark {

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
    static void channel_close_algorithms(ImageType &i2d, ImageType &dst) {
        averaging(i2d, dst);
        luma(i2d, dst);
        blur(i2d, dst);
        desaturation(i2d, dst);
        de_composition_max(i2d, dst);
        de_composition_min(i2d, dst);
    }

    static ImageType channel_close_algorithms(ImageType &i2d) {
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
        return elapsed;
    }

    virtual const std::string getDesc() const override {
        return ImageType::__implementation_type();
    }
};
