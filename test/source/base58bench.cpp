#include "reference.h"
#include <base58/base58.h>

#include <doctest/doctest.h>
#include <fmt/ostream.h>
#include <nanobench.h>

#include <string>

TEST_CASE("base58_bench") {
    // create some random data
    auto input = std::string();
    auto rng = ankerl::nanobench::Rng();
    for (size_t i = 0; i < 32; ++i) {
        input.push_back(static_cast<char>(rng()));
    }

    auto referenceOutput = std::string();
    auto bench = ankerl::nanobench::Bench();
    bench.relative(true);
    bench.run("encodeReference", [&] {
        referenceOutput = encodeReference(input.data(), input.size());
        ankerl::nanobench::doNotOptimizeAway(referenceOutput);
    });

    auto ankerlOutput = std::string();
    bench.run("base58::encode", [&] {
        ankerlOutput.clear();
        ankerl::base58::encode(input.data(), input.size(), ankerlOutput);
        ankerl::nanobench::doNotOptimizeAway(ankerlOutput);
    });

    REQUIRE(referenceOutput == ankerlOutput);
}

// see https://nanobench.ankerl.com/tutorial.html#asymptotic-complexity
//
// The algorithm very clearly has O(n^2) complexity:
//
//   |   coefficient |   err% | complexity
//   |--------------:|-------:|------------
//   | 2.1596209e-10 |   0.5% | O(n^2)
//   | 2.1929290e-14 |  31.5% | O(n^3)
//   | 1.4632549e-07 |  55.6% | O(n log n)
//   | 1.8807427e-06 |  66.7% | O(n)
//   | 4.6264603e-04 | 202.7% | O(log n)
//   | 2.8209592e-03 | 228.8% | O(1)
TEST_CASE("base58_complexity") {
    auto bench = ankerl::nanobench::Bench();
    auto rng = ankerl::nanobench::Rng();

    auto input = std::vector<uint8_t>();

    // Running the benchmark multiple times, with different number of elements
    for (auto inputSize : {10U, 20U, 50U, 100U, 200U, 500U, 1000U, 2000U, 5000U, 10000U}) {
        while (input.size() < inputSize) {
            input.push_back(rng());
        }
        auto output = std::string();
        bench.complexityN(input.size()).run("base58::encode", [&] {
            output.clear();
            ankerl::base58::encode(input.data(), input.size(), output);
            ankerl::nanobench::doNotOptimizeAway(output);
        });
    }

    // calculate BigO complexy best fit and print the results
    fmt::print("{}\n", bench.complexityBigO());
}
