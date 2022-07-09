#ifndef JUMPORAM_UTILS_H_
#define JUMPORAM_UTILS_H_

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <string>
#include <vector>

class Log {
public:
	Log(std::string name);

	void start();
	unsigned long int end();
	void write_to_file();

private:
	std::string log_name;
	std::chrono::time_point<std::chrono::high_resolution_clock> start_point;
	std::vector<unsigned long int> exp_logs;
};

#endif // JUMPORAM_UTILS_H_
