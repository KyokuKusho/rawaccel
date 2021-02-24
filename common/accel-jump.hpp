#pragma once

#include "rawaccel-base.hpp"

namespace rawaccel {

	struct accel_jump : additive {
		double val;

		double operator()(double) const
		{
			return val;
		}

		accel_jump(const accel_args& args) :
			additive(args.cap.x),
			val(args.cap.y - 1) {}
	};

	struct accel_jump_gain : accel_jump {

		static constexpr bool is_transfer_type = 1;

		double operator()(double x) const
		{
			return val * x;
		}

		using accel_jump::accel_jump;
	};

}
