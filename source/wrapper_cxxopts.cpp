#include "wrapper_cxxopts.h"
#include "predefined.h"
#include <cxxopts.hpp>
#include <cstring>
#include <string>
#include <iostream>

extern int8_t parse_arguments_hls(main_arguments *args, int32_t argc, char *argv[]) {
	try {
		cxxopts::Options options("hls", "hls stream generator");
		options.add_options()(
		    "s,segments",
		    "segments path",
		    cxxopts::value<std::string>()->default_value("/var/www/hls"))(
		    "m,manager",
		    "manager ip",
		    cxxopts::value<std::string>()->default_value(""));

		auto result = options.parse(argc, argv);
		const std::string &option_segment = result["segments"].as<std::string>();
		const std::string &option_manager = result["manager"].as<std::string>();
		std::strncpy(args->path_segments, option_segment.c_str(), sizeof(args->path_segments));
		if (!option_manager.empty()) {
			std::snprintf(args->path_zookeeper, sizeof(args->path_zookeeper), "%s:%d", option_manager.c_str(), NET_ZOOKEEPER_PORT);
			std::snprintf(args->path_kafka, sizeof(args->path_kafka), "%s:%d", option_manager.c_str(), NET_KAFKA_PORT);
		} else {
			std::strncpy(args->path_zookeeper, "", sizeof(args->path_zookeeper));
			std::strncpy(args->path_kafka, "", sizeof(args->path_kafka));
		}

		return 0;
	} catch (const std::exception &e) {
		return -1;
	}
}

extern int8_t parse_arguments_dash(main_arguments *args, int32_t argc, char *argv[]) {
	try {
		cxxopts::Options options("dash", "dash stream generator");
		options.add_options()(
		    "s,segments",
		    "segments path",
		    cxxopts::value<std::string>()->default_value("/var/www/dash"))(
		    "m,manager",
		    "manager ip",
		    cxxopts::value<std::string>()->default_value(""));

		auto result = options.parse(argc, argv);
		const std::string &option_segment = result["segments"].as<std::string>();
		const std::string &option_manager = result["manager"].as<std::string>();
		std::strncpy(args->path_segments, option_segment.c_str(), sizeof(args->path_segments));
		if (!option_manager.empty()) {
			std::snprintf(args->path_zookeeper, sizeof(args->path_zookeeper), "%s:%d", option_manager.c_str(), NET_ZOOKEEPER_PORT);
			std::snprintf(args->path_kafka, sizeof(args->path_kafka), "%s:%d", option_manager.c_str(), NET_KAFKA_PORT);
		} else {
			std::strncpy(args->path_zookeeper, "", sizeof(args->path_zookeeper));
			std::strncpy(args->path_kafka, "", sizeof(args->path_kafka));
		}

		return 0;
	} catch (const std::exception &e) {
		return -1;
	}
}