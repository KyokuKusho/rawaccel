#pragma once

#include "rawaccel-base.hpp"

#include <math.h>

namespace rawaccel {
		
	struct accel_natural : negatable {
		double limit;
		double accel;

		double operator()(double x) const
		{
			return limit * (1 - exp(accel * x));
		}

		accel_natural(const accel_args& args) :
			negatable(args.offset, args.limit),
			limit(fabs(args.limit - 1)),
			accel(-args.accel_natural / limit) {}
	};


	struct accel_natural_gain : accel_natural {
		double constant;
		
		static constexpr bool is_transfer_type = 1;

		double operator()(double x) const
		{
			return limit * (x - exp(accel * x) / accel) + constant;
		}

		accel_natural_gain(const accel_args& args) :
			accel_natural(args),
			constant(limit / accel) {}
	};

}
