#pragma once

#include <stdexcept>
#include <system_error>
#include <string>

namespace rawaccel {

	namespace detail {
		auto sys_error_str = [](DWORD code, auto msg) {
			auto err = ::std::system_error(code, std::system_category(), msg);
			::std::string what = err.what();
			what += " (";
			what += std::to_string(code);
			what += ')';
			return what;
		};
	}

	class error : public std::runtime_error { 
		using std::runtime_error::runtime_error; 
	};

	class io_error : public error {
		using error::error;
	};

	class install_error : public io_error {
	public:
		install_error() : 
			io_error("Raw Accel driver is not installed, run installer.exe") {}
	};

	class sys_error : public io_error {
	public:
		sys_error(const char* msg, DWORD code = GetLastError()) :
			io_error(detail::sys_error_str(code, msg)) {}
	};
}
