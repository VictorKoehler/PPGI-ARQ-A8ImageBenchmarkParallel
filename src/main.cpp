#include <sstream>
#include <cinttypes>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_set>
#include "ImagingAlgorithms.hpp"
#include "include/tclap/CmdLine.h"

#ifndef GITFLAG
char const *const GIT_COMMIT = "******";
#endif

void dummy_warm_hardware_benchmark() {
    BenchClock clock;
    std::cout << "# Initializing dummy benchmark...\n";
    ImagingAlgorithms<Image3D<PixelOrder::XYC, false>> d;
    Image3D<PixelOrder::XYC, false> di(5000, 5000, 3);
    d.channel_close_algorithms(di);
    std::cout << "# Dummy benchmark took " << clock.getElapsed() << " " STRINGIFY(CLOCK_PRECISION) "\n";
}

template<typename T>
void auto_shuffle_(T& t) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(t.begin(), t.end(), g);
}

/**
 * Main do Projeto
 * Recebe o caminho relativo de imagens como argumentos da linha de comando.
 * Invoca grayscale_algorithms() para cada imagem.
 * A princípio não salva a imagem, a não ser que seja explicitamente solicitado (TODO);
 */
int main(int argc, const char* argv[]) {
    BenchClock clock;

    // Setup das diferentes implementações de benchmarking
    std::vector<ImagingBenchmark*> benchType = {
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

    // Interpreta a linha de comando
    std::vector<std::string> allowed;
    for (const auto& i : benchType) {
        allowed.push_back(i->getDesc());
    }
    TCLAP::ValuesConstraint<std::string> allowedVals(allowed);

    TCLAP::CmdLine parser("Image benchmark");
    TCLAP::SwitchArg arg_dummy("d", "dummy", "Disables dummy warm benchmark on startup", parser);
    TCLAP::SwitchArg arg_pfilter("", "print-filter-choices", "Print implementations available and exit", parser);
    TCLAP::MultiArg<std::string> arg_filter("f", "filter", "Filter what implementations/algorithms will be used",
        false, &allowedVals, parser);
    TCLAP::UnlabeledMultiArg<std::string> files("files", "Input images", true, "image-path", parser);
    parser.parse(argc, argv);

    if (arg_pfilter.isSet()) {
        auto_shuffle_(allowed);
        for (const auto& i : allowed) std::cout << i << "\n";
        exit(0);
    }

    // (Tenta) diminuir a influência dos boosts curtos e iniciais de processadores modernos
    if (!arg_dummy.isSet()) dummy_warm_hardware_benchmark();

    // Contém os algoritmos filtrados pelo usuário (ou nenhum)
    std::unordered_set<std::string> filter(arg_filter.getValue().begin(), arg_filter.getValue().end());

    // Embaralha a ordem de invocação das implementações para garantir que não haja viés de ordem de execução, cache e boost
    auto_shuffle_(benchType);

    // Carregar as imagens na memória é um processo custoso, geralmente penalizado em imagens grandes compactadas.
    // Para evitar resultados imprecisos, cronometraremos apenas o tempo de execução dos algoritmos de conversão
    // de escala de cinza; desconsiderando, portanto, o tempo gasto e a eficiência da biblioteca de carregamento
    // de imagens (e da gambiarra de copiar para uma estrutura própria do autor deste trabalho).
    std::cout << "# Started Simple Image Benchmark (" << GIT_COMMIT << ")\n";
    int64_t global_total = 0;

    // Após o cronometro começar, percorremos todas as imagens aplicando-as os algoritmos.
    for (const auto& bench : benchType) {
        const auto bname = bench->getDesc();
        if (!filter.empty() && filter.find(bname) == filter.end()) continue;

        int64_t total = 0;
        std::cout << "# Evaluating " << bname << "\n";
        for (const auto& file : files.getValue()) {
            total += bench->benchmark(file.c_str(), true);
        }
        global_total += total;
        std::cout << "# Evaluation of " << bname << " finished with a total of " << total << " " STRINGIFY(CLOCK_PRECISION) "\n";
    }

    auto overhead = clock.getElapsed();
	std::cout << "# All benchmarks done, with a total sum of " << global_total << " " STRINGIFY(CLOCK_PRECISION) ",\n";
	std::cout << "# or " << overhead << " " STRINGIFY(CLOCK_PRECISION) " counting with the overhead." << std::endl;

    for (auto& bench : benchType) {
        delete bench;
    }

    return 0; // TODO: FSANITIZE
}

int _main_test_address() {
    Image3D<PixelOrder::XYC, true> teste(100, 100, 3);
    // printf("%p %p %p\n", (void *)&teste.buff[0][0][0],  (void *)&teste.buff[0][0][1],   (void *)&teste.buff[0][0][2]);
    printf("%lu %lu %lu\n", (uintptr_t)&teste(0, 0, 0),       (uintptr_t)&teste(0, 0, 1),        (uintptr_t)&teste(0, 0, 2));
    printf("%lu %lu %lu\n", (uintptr_t)&teste(0, 1, 0),       (uintptr_t)&teste(0, 2, 0),        (uintptr_t)&teste(0, 99, 0));
    printf("%lu %lu %lu\n", (uintptr_t)&teste(1, 0, 0),       (uintptr_t)&teste(2, 0, 1),        (uintptr_t)&teste(99, 99, 2));
    return 0;
}
