#include <algorithm>
#include <cstdlib>
#include <cctype>
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

void get_input_lowercase(std::string &buff) {
	get_input(buff);
	std::transform(buff.begin(), buff.end(), buff.begin(), ::tolower);
}

void str_replace(std::string &from, const std::string &to, const std::string &str) {
	size_t pos = from.find(str);

	while (pos != std::string::npos) {
		from.replace(pos, str.length(), to);
		pos = from.find(str, pos + to.length());
	}
}

void store_data(const std::string &build_cmd, const std::string &run_cmd) {
	std::ofstream config_file(".os-build-utility.conf");
	config_file << build_cmd << std::endl << run_cmd << std::endl;
	config_file.close();
}

std::string trim(const std::string &string) {
	std::string str = string;

	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](const unsigned char ch) {
		return !std::isspace(ch);
	}));

	str.erase(std::find_if(str.rbegin(), str.rend(), [](const unsigned char ch) {
		return !std::isspace(ch);
	}).base(), str.end());

	return str;
}

void get_data(std::string &build_cmd, std::string &run_cmd) {
	std::ifstream config_file(".os-build-utility.conf");

	if (!config_file) {
		std::cerr << "No configuration file found. Try running '" NAME_STR " --configure' before running." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::getline(config_file, build_cmd);

	while (trim(build_cmd).empty() || trim(build_cmd).at(0) == '#') {
		std::getline(config_file, build_cmd);
	}

	std::getline(config_file, run_cmd);

	while (trim(run_cmd).empty() || trim(run_cmd).at(0) == '#') {
		std::getline(config_file, run_cmd);
	}

	str_replace(build_cmd, "%cd%", std::filesystem::current_path().string());
	str_replace(run_cmd, "%cd%", std::filesystem::current_path().string());

	config_file.close();
}

bool is_valid_docker_tag(const std::string &tag) {
	if (tag.empty() || tag.length() > 128) {
		return false;
	}

	if (tag[0] == '.' || tag[0] == '-') {
		return false;
	}

	if (std::all_of(tag.begin(), tag.end(), [](const char c) {
		return !std::isalnum(c) && !std::islower(c) && c != '.' && c != '-' && c != '_';
	})) {
		return false;
	}

	return true;
}

void build() {
	std::string build_cmd, run_cmd;

	get_data(build_cmd, run_cmd);

	std::cout << NAME_STR " - Building: \"" << build_cmd << "\"" << std::endl;

	if (const int err_code = system(build_cmd.c_str()); err_code != 0) {
		std::cerr << "Build failed with exit code " << err_code << "." << std::endl;
		exit(err_code);
	}
}

void run() {
    std::string build_cmd, run_cmd;

	get_data(build_cmd, run_cmd);

    std::cout << NAME_STR " - Running: \"" << run_cmd << "\"" << std::endl;

    if (const int err_code = system(run_cmd.c_str()); err_code != 0) {
		std::cerr << "Run failed with exit code " << err_code << "." << std::endl;
		exit(err_code);
	}
}

void configure_docker_dockerfile() {
	std::cout << "What's your OS name to make the Docker container's tag?" << std::endl;
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

	const std::string build_cmd = R"(docker run --rm -it -v "%cd%":"/root/end" )" + docker_tag + " " + docker_build_cmd;

	std::cout << "What's the command to run your OS?" << std::endl;
	std::string run_cmd;
	get_input(run_cmd);

	store_data(build_cmd, run_cmd);

	std::filesystem::create_directories(std::filesystem::current_path().string() + "/buildenv");
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

	dockerfile.close();

	system(("docker build buildenv -t " + docker_tag).c_str());
};

void configure_docker() {
	if (!std::filesystem::exists("buildenv/Dockerfile")) {
		std::cout << "Do you want " NAME_STR " to create a Dockerfile for you? [y/N]" << std::endl;
		std::string user_input;
		get_input_lowercase(user_input);

		if (user_input == "y") {
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

	const std::string build_cmd = R"(docker run --rm -it -v "%cd%":")" + docker_env_path + "\" " + docker_tag + " " + docker_build_cmd;

	std::cout << "What's the command to run your OS?" << std::endl;
	std::string run_cmd;
	get_input(run_cmd);

	store_data(build_cmd, run_cmd);
}

void configure() {
	if (exec_system("docker --version") == 0) {
		std::cout << "Do you want to use Docker for the build environment? [Y/n]" << std::endl;
		std::string user_input;
		get_input_lowercase(user_input);

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

void edit_config() {
	std::cout << "Do you want to edit the current configuration or create a new one? [E/n]" << std::endl;
	std::string user_input;
	get_input_lowercase(user_input);

	if (user_input == "n") {
		configure();
	} else {
		std::string config_build_cmd;
		std::string config_run_cmd;

		get_data(config_build_cmd, config_run_cmd);

		std::cout << "What's the command to build your OS?" << std::endl << "Current one (leave blank to reuse): \"" << config_build_cmd << "\"" << std::endl;
		std::string build_cmd;
		get_input(build_cmd);

		std::cout << "What's the command to run your OS?" << std::endl << "Current one (leave blank to reuse): \"" << config_run_cmd << "\"" << std::endl;
		std::string run_cmd;
		get_input(run_cmd);

		if (build_cmd.empty()) {
			build_cmd = config_build_cmd;
		}

		if (run_cmd.empty()) {
			run_cmd = config_run_cmd;
		}

		store_data(build_cmd, run_cmd);
	}
}

int main(const int argc, char **argv){
	const std::vector<std::string> args(argv, argv + argc);

	if (argc == 1 || (argc == 2 && (args[1] == "-h" || args[1] == "--help"))) {
		help();
	}

	if (argc == 2) {
		if (args[1] == "--configure" || args[1] == "-c") {
			if (std::filesystem::exists(".os-build-utility.conf")) {
				edit_config();
			} else {
				configure();
			}

			return EXIT_SUCCESS;
		}

		if (args[1] == "--build" || args[1] == "-b") {
			build();
			return EXIT_SUCCESS;
		}

		if (args[1] == "--run" || args[1] == "-r") {
			run();
			return EXIT_SUCCESS;
		}

		if (args[1] == "-br" || args[1] == "-rb") {
			build();
			run();
			return EXIT_SUCCESS;
		}
	}

	if (argc == 3) {
		if ((args[1] == "--build" || args[1] == "-b") && (args[2] == "--run" || args[2] == "-r") &&
		    (args[1] == "--run" || args[1] == "-r") && (args[2] == "--build" || args[2] == "-b")) {
			build();
			run();
			return EXIT_SUCCESS;
		}
	}

	std::cerr << "Invalid arguments." << std::endl;
	return EXIT_FAILURE;
}