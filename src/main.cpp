#include <iostream>
#include <cinttypes>
#include "ImagingAlgorithms.hpp"

void dummy_warm_hardware_benchmark() {
    BenchClock clock;
    std::cout << "Initializing dummy benchmark...\n";
    ImagingAlgorithms<Image3D<PixelOrder::XYC, false>> d;
    Image3D<PixelOrder::XYC, false> di(5000, 5000, 3);
    d.channel_close_algorithms(di);
    std::cout << "Dummy benchmark took " << clock.getElapsed() << " " STRINGIFY(CLOCK_PRECISION) "\n";
}

/**
 * Main do Projeto
 * Recebe o caminho relativo de imagens como argumentos da linha de comando.
 * Invoca grayscale_algorithms() para cada imagem.
 * A princípio não salva a imagem, a não ser que seja explicitamente solicitado (TODO);
 */
int main(int argc, const char* argv[]) {
    BenchClock clock;

    int argc_decr = 1;
    std::set<std::string> filter;
    if (argc >= 3 && argv[1][0] == '-' && argv[1][1] == 'f') {
        std::stringstream ss(argv[2]);
        std::string item;
        while (std::getline(ss, item, ',')) {
            filter.insert(item);
        }
        argc_decr += 2;
    }
    
    // Setup das variáveis
    int filescount = argc-argc_decr;
    const char** files = argv+argc_decr;

    dummy_warm_hardware_benchmark();

    // Setup das diferentes implementações de benchmarking
    ImagingBenchmark* benchType[] = {
        new ImagingAlgorithms<Image3D<PixelOrder::XYC, false>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::XYC, true>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::XCY, false>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::XCY, true>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::YXC, false>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::YXC, true>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::YCX, false>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::YCX, true>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::CXY, false>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::CXY, true>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::CYX, false>>(),
        new ImagingAlgorithms<Image3D<PixelOrder::CYX, true>>()
    };

    // Carregar as imagens na memória é um processo custoso, geralmente penalizado em imagens grandes compactadas.
    // Para evitar resultados imprecisos, cronometraremos apenas o tempo de execução dos algoritmos de conversão
    // de escala de cinza; desconsiderando, portanto, o tempo gasto e a eficiência da biblioteca de carregamento
    // de imagens (e da gambiarra de copiar para uma estrutura própria do autor deste trabalho).
    std::cout << "Started\n";
    int64_t global_total = 0;

    // Após o cronometro começar, percorremos todas as imagens aplicando-as os algoritmos.
    for (const auto& bench : benchType) {
        if (!filter.empty() && filter.find(bench->getDesc()) == filter.end()) continue;
        int64_t total = 0;
        std::cout << "Evaluating " << bench->getDesc() << "\n";
        for (int i = 0; i < filescount; i++) {
            auto t = bench->benchmark(files[i]);
            total += t;
            std::cout << files[i] << ": " << t << " " STRINGIFY(CLOCK_PRECISION) "\n";
        }
        global_total += total;
        std::cout << "Evaluation of " << bench->getDesc() << " finished with a total of " << total << " " STRINGIFY(CLOCK_PRECISION) "\n";
    }

    auto overhead = clock.getElapsed();
	std::cout << "All benchmarks done, with a total sum of " << global_total << " " STRINGIFY(CLOCK_PRECISION) ",\n";
	std::cout << "or " << overhead << " " STRINGIFY(CLOCK_PRECISION) " counting with the overhead." << std::endl;

    return 0;
}

int _main_test_address() {
    Image3D<PixelOrder::XYC, true> teste(100, 100, 3);
    // printf("%p %p %p\n", (void *)&teste.buff[0][0][0],  (void *)&teste.buff[0][0][1],   (void *)&teste.buff[0][0][2]);
    printf("%lu %lu %lu\n", (uintptr_t)&teste(0, 0, 0),       (uintptr_t)&teste(0, 0, 1),        (uintptr_t)&teste(0, 0, 2));
    printf("%lu %lu %lu\n", (uintptr_t)&teste(0, 1, 0),       (uintptr_t)&teste(0, 2, 0),        (uintptr_t)&teste(0, 99, 0));
    printf("%lu %lu %lu\n", (uintptr_t)&teste(1, 0, 0),       (uintptr_t)&teste(2, 0, 1),        (uintptr_t)&teste(99, 99, 2));
    return 0;
}
