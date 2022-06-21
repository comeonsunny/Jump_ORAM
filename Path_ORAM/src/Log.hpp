#ifndef LOG_HPP
#define LOG_HPP
#include <string>
#include <chrono>
#include <vector>
class Log {
public:
	Log(std::string name);
	~Log();
	void start();
	unsigned long int end();
	void write_to_file();

private:
	std::string log_name;
	std::chrono::time_point<std::chrono::high_resolution_clock> start_point;
	std::vector<unsigned long int> exp_logs;
};
#endif