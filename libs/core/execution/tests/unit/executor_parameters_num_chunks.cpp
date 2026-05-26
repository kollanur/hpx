//  Copyright (c) 2026 Bharath Kollanur
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/algorithm.hpp>
#include <hpx/execution.hpp>
#include <hpx/init.hpp>
#include <hpx/modules/testing.hpp>
#include <hpx/program_options.hpp>

#include <cstddef>
#include <functional>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

struct execution_parameters
{
    template <typename Executor>
    friend void tag_override_invoke(
        hpx::execution::experimental::collect_execution_parameters_t,
        execution_parameters& self, Executor&&, std::size_t const count,
        std::size_t const, std::size_t const num_chunks,
        std::size_t const chunk_size) noexcept
    {
        self.count_ = count;
        self.num_chunks_ = num_chunks;
        self.chunk_size_ = chunk_size;
    }

    std::size_t count_ = 0;
    std::size_t num_chunks_ = 0;
    std::size_t chunk_size_ = 0;
};

namespace hpx::execution::experimental {
    template <>
    struct is_executor_parameters<execution_parameters> : std::true_type
    {
    };
}    // namespace hpx::execution::experimental

void test_collect_execution_parameters_num_chunks()
{
    std::vector<int> data(313000);
    std::iota(data.begin(), data.end(), std::uint64_t(0));

    execution_parameters params;
    auto policy = hpx::execution::par.with(std::ref(params));

    hpx::for_each(policy, data.begin(), data.end(), [](int&) {});

    HPX_TEST_LT(std::size_t(0), params.count_);
    HPX_TEST_LT(std::size_t(0), params.num_chunks_);
    HPX_TEST_LT(std::size_t(0), params.chunk_size_);

    std::size_t const expected_chunks =
        (params.count_ + params.chunk_size_ - 1) / params.chunk_size_;
    HPX_TEST_EQ(expected_chunks, params.num_chunks_);
}

int hpx_main()
{
    test_collect_execution_parameters_num_chunks();

    return hpx::local::finalize();
}

int main(int argc, char* argv[])
{
    using namespace hpx::program_options;

    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    std::vector<std::string> const cfg = {"hpx.os_threads=all"};

    hpx::local::init_params init_args;
    init_args.desc_cmdline = desc_commandline;
    init_args.cfg = cfg;

    HPX_TEST_EQ_MSG(hpx::local::init(hpx_main, argc, argv, init_args), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
