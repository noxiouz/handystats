// Copyright (c) 2014 Yandex LLC. All rights reserved.

#ifndef HANDYSTATS_CONFIG_IMPL_HPP_
#define HANDYSTATS_CONFIG_IMPL_HPP_

#include <memory>
#include <vector>

#include <handystats/config/statistics.hpp>
#include <handystats/config/metrics/gauge.hpp>
#include <handystats/config/metrics/counter.hpp>
#include <handystats/config/metrics/timer.hpp>

#include "config/metrics_dump_impl.hpp"
#include "config/core_impl.hpp"

namespace handystats { namespace config {

extern statistics statistics_opts;

namespace metrics {
	extern gauge gauge_opts;
	extern counter counter_opts;
	extern timer timer_opts;
}

extern metrics_dump metrics_dump_opts;
extern core core_opts;

extern
std::vector<
	std::pair<
		std::vector<std::string>,
		rapidjson::Value*
	>
>
pattern_opts;

extern
std::shared_ptr<rapidjson::Document> source;

rapidjson::Value* select_pattern(const std::string&);

void initialize();
void finalize();

}} // namespace handystats::config

#endif // HANDYSTATS_CONFIG_IMPL_HPP_
