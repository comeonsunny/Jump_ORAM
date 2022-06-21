#include "Log.hpp"
#include <fstream>

Log::Log(std::string name) : log_name(name)
{
}
Log::~Log() {}

void Log::start()
{
	start_point = std::chrono::high_resolution_clock::now();
}

unsigned long int Log::end()
{
	auto end_point = std::chrono::high_resolution_clock::now();
	unsigned long int time = std::chrono::duration_cast<std::chrono::nanoseconds>(
			end_point - start_point).count();
	exp_logs.push_back(time);
	return exp_logs.back();
}

void Log::write_to_file()
{
    std::ofstream log_file(log_name, std::ios::app);
	for (auto i : exp_logs) {
		log_file << i << " ";
	}
    log_file << std::endl;
    log_file.close();
}
