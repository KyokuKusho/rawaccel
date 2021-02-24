#pragma once

#include "rawaccel-io-def.h"
#include "rawaccel-version.h"
#include "rawaccel-error.hpp"
#include "rawaccel.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma warning(push)
#pragma warning(disable:4245) // int -> DWORD conversion while passing CTL_CODE

namespace rawaccel {

	inline void io_control(DWORD code, void* in, DWORD in_size, void* out, DWORD out_size) {
		HANDLE ra_handle = INVALID_HANDLE_VALUE;

		ra_handle = CreateFileW(L"\\\\.\\rawaccel", 0, 0, 0, OPEN_EXISTING, 0, 0);

		if (ra_handle == INVALID_HANDLE_VALUE) {
			throw install_error();
		}

		DWORD dummy;

		BOOL success = DeviceIoControl(
			ra_handle,
			code,
			in,
			in_size,
			out,
			out_size,
			&dummy,  // bytes returned
			NULL     // overlapped structure
		);

		CloseHandle(ra_handle);

		if (!success) {
			throw sys_error("DeviceIoControl failed");
		}
	}

	inline void read(io_t& args) {
		io_control(RA_READ, NULL, 0, &args, sizeof(io_t));
	}

	inline void write(const io_t& args) {
		io_control(RA_WRITE, const_cast<io_t*>(&args), sizeof(io_t), NULL, 0);
	}

	inline version_t check_version() {
		version_t ver;

		try {
			io_control(RA_GET_VERSION, NULL, 0, &ver, sizeof(version_t));
		}
		catch (const sys_error&) {
			throw error("driver reinstallation required");
		}

		if (ver < min_driver_version) {
			throw error("driver reinstallation required");
		}

		if (build_version < ver) {
			throw error("newer driver version is installed");
		}

		return ver;
	}

}

#pragma warning(pop)
