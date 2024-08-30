#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "metadata.hpp"


void help() {
	std::cout << NAME_STR << " v" << VERSION << " by " << AUTHOR << " using C++" << std::to_string(__cplusplus).substr(2, 2) << std::endl << std::endl;
	std::cout << "Usage: " << std::endl;
	std::cout << "    " << USAGE << std::endl << std::endl;
}

int exec_system(const std::string &command) {
	return system((command + " > "
		#ifdef  _WIN32
			"nul"
		#else
			"dev/null"
		#endif
	" 2>&1").c_str());
}

void get_input(std::string &buff) {
	std::getline(std::cin, buff);
	buff.erase(std::remove(buff.begin(), buff.end(), '\r'), buff.end());
}

void store_data(const std::string &build_cmd, const std::string &run_cmd) {
	std::ofstream config_file(".os-build-utility.conf");
	config_file << build_cmd << std::endl << run_cmd << std::endl;
}

bool is_valid_docker_tag(const std::string &tag) {
	if (tag.empty() || tag.length() > 128) {
		return false;
	}

	if (tag[0] == '.' || tag[0] == '-') {
		return false;
	}

	if (std::all_of(tag.begin(), tag.end(), [](const char c){ return!std::isalnum(c) && c != '-' && c != '_'; })) {
		return false;
	}

	return true;
}

void build() {
	std::string build_cmd;

	std::ifstream config_file(".os-build-utility.conf");

	if (!config_file) {
		std::cerr << "No configuration file found. Try running " NAME_STR " --configure before building." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::getline(config_file, build_cmd);

	system(build_cmd.c_str());
}

void run() {
	std::string build_cmd;
	std::string run_cmd;

	std::ifstream config_file(".os-build-utility.conf");

	if (!config_file) {
		std::cerr << "No configuration file found. Try running " NAME_STR " --configure before building." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::getline(config_file, build_cmd); // Trash the first line
	std::getline(config_file, run_cmd);

	system(run_cmd.c_str());
}

void configure_docker_dockerfile() {
	std::cout << "What's your OS name (only ASCII letters, numbers, '_', '-' and '.')?" << std::endl;
	std::string os_name;
	get_input(os_name);

	const std::string docker_tag = os_name + "-buildenv";

	if (!is_valid_docker_tag(docker_tag)) {
		std::cerr << "Invalid OS name for Docker tag." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "What's the command to execute in the Docker container to build your OS?" << std::endl;
	std::string docker_build_cmd;
	get_input(docker_build_cmd);

	const std::string build_cmd = "docker run --rm -it -v \"" + std::filesystem::current_path().string() + R"(":"/root/end" )" + docker_tag + " " + docker_build_cmd;

	std::cout << "What's the command to run your OS?" << std::endl;
	std::string run_cmd;
	get_input(run_cmd);

	store_data(build_cmd, run_cmd);

	std::ofstream dockerfile("buildenv/Dockerfile");

	dockerfile << "FROM randomdude/gcc-cross-x86_64-elf" << std::endl << std::endl;
	dockerfile << "RUN apt-get update" << std::endl;
	dockerfile << "RUN apt-get upgrade -y" << std::endl;
	dockerfile << "RUN apt-get install -y nasm" << std::endl;
	dockerfile << "RUN apt-get install -y xorriso" << std::endl;
	dockerfile << "RUN apt-get install -y grub-pc-bin" << std::endl;
	dockerfile << "RUN apt-get install -y grub-common" << std::endl << std::endl;
	dockerfile << "VOLUME /root/env" << std::endl;
	dockerfile << "WORKDIR /root/env" << std::endl;

	system(("docker build buildenv -t " + docker_tag).c_str());
};

void configure_docker() {
	if (!std::filesystem::exists("buildenv/Dockerfile")) {
		std::cout << "Do you want " NAME_STR " to create a Dockerfile for you? [y/N]" << std::endl;
		std::string user_input;
		get_input(user_input);
		std::transform(user_input.begin(), user_input.end(), user_input.begin(), ::tolower);

		if (user_input != "y") {
			configure_docker_dockerfile();
			return;
		}
	}

	std::cout << "What's the command to execute in the Docker container to build your OS?" << std::endl;
	std::string docker_build_cmd;
	get_input(docker_build_cmd);

	std::cout << "What's the tag of the Docker container?" << std::endl;
	std::string docker_tag;
	get_input(docker_tag);

	if (!is_valid_docker_tag(docker_tag)) {
		std::cerr << "Invalid Docker tag." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "What's the path to the environment in the Docker container?" << std::endl;
	std::string docker_env_path;
	get_input(docker_env_path);

	const std::string build_cmd = "docker run --rm -it -v \"" + std::filesystem::current_path().string() + "\":\"" + docker_env_path + "\" " + docker_tag + " " + docker_build_cmd;

	std::cout << "What's the command to run your OS?" << std::endl;
	std::string run_cmd;
	get_input(run_cmd);

	store_data(build_cmd, run_cmd);
}

void configure() {
	if (exec_system("docker --version") == 0) {
		std::cout << "Do you want to use Docker for the build environment? [Y/n]" << std::endl;
		std::string user_input;
		get_input(user_input);

		if (user_input != "n") {
			configure_docker();
			return;
		}
	}

	std::cout << "What's the command to build your OS?" << std::endl;
	std::string build_cmd;
	get_input(build_cmd);

	std::cout << "What's the command to run your OS?" << std::endl;
	std::string run_cmd;
	get_input(run_cmd);

	store_data(build_cmd, run_cmd);
}

int main(const int argc, char **argv){
	const std::vector<std::string> args(argv, argv + argc);

	if (argc == 1 || (argc == 2 && (args[1] == "-h" || args[1] == "--help"))) {
		help();
	}

	if (argc == 2) {
		if (args[1] == "--configure" || args[1] == "-c") {
			configure();
		} else if (args[1] == "--build" || args[1] == "-b") {
			build();
		} else if (args[1] == "--run" || args[1] == "-r") {
			run();
		} else if (args[1] == "-br") {
			build();
			run();
		}
	}

	if (argc == 3) {
		if ((args[1] == "--build" || args[1] == "-b") && (args[2] == "--run" || args[2] == "-r")) {
			build();
			run();
		}
	}
}