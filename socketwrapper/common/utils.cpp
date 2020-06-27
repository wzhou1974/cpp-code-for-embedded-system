#include "common/inc/utils.h"
#include "hash-library/hashlib-md5.h"
#include <array>
#include <assert.h>
#include <memory>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdlib.h>		// for BUFSIZ
#include <string.h>		// for memset
#include "boost/asio.hpp"

namespace socketwrapper {

std::string md5sum(const std::string& path)
{
	std::array<char, 1024> buffer;
	MD5 md5;
	std::unique_ptr<FILE, decltype(&std::fclose)> file_ptr(fopen(path.c_str(), "r+"), &fclose);

	while (feof(file_ptr.get()) == 0) {
		size_t len = fread(buffer.data(), 1, buffer.size(), file_ptr.get());
		md5.add(buffer.data(), len);
	}

	return 	md5.getHash();
}

std::string md5sum(const char * const data, size_t len)
{
	MD5 md5;

	md5.add(data, len);
	return	md5.getHash();
}

std::string GetCurrentTime()
{
	using namespace std::chrono;

	auto tm = system_clock::to_time_t(system_clock::now());
	
	// The format spcifier : https://zh.cppreference.com/w/cpp/io/manip/put_time
	std::ostringstream ss;
	ss << std::put_time(std::localtime(&tm), "%Y%m%d%H%M%S");

	return ss.str();
}

void SyncLock::Pend()
{
	std::unique_lock<std::mutex> lock(mutex_);

	running_ = false;

    while (!running_) {
        condition_.wait(lock, [this] {return running_ != false;});
    }

    assert(running_ == true);
}

void SyncLock::Resume()
{
    std::unique_lock<std::mutex> lock(mutex_);

    running_ = true;
    condition_.notify_one();
}

bool IsPortInUse(unsigned short port)
{
    using namespace boost::asio;
    using ip::tcp;

    io_service svc;
    tcp::acceptor acept(svc);

    boost::system::error_code ec;
    acept.open(tcp::v4(), ec);
    acept.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    acept.bind({ tcp::v4(), port }, ec);

    return ec == error::address_in_use;
}

}   // namespace socketwrapper
