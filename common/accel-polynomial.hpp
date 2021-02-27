#pragma once

#include "rawaccel-base.hpp"

#include <math.h>

namespace rawaccel {

	struct power_traits {

		static double def(double x, double scale, double degree)
		{
			if (degree == 1) return scale * x;
			return pow(scale * x, degree);
		}

		static double inv(double y, double scale, double degree)
		{
			if (degree == 1) return y / scale;
			return pow(y, 1 / degree) / scale;
		}

		static double scale(double x, double y, double degree)
		{
			return inv(y, x, degree);
		}

		static double anti_deriv(double x, double scale, double degree)
		{
			return x * def(x, scale, degree) / (degree + 1);
		}

	};

	struct accel_power : accel_base {
		double scale;
		double exponent;

		using traits = power_traits;

		double operator()(double x) const
		{
			return traits::def(x, scale, exponent);
		}

		accel_power(const accel_args& args) :
			scale(args.scale),
			exponent(args.exponent) {}
	};

	using accel_power_gain = accel_power;

	struct accel_uncapped : additive {
		double power;
		double accel;

		using traits = power_traits;

		double operator()(double x) const
		{
			return traits::def(x, accel, power);
		}

		accel_uncapped(const accel_args& args) :
			additive(args.offset),
			power(args.power - 1),
			accel(args.accel_uncapped) {}
	};

	struct accel_uncapped_gain : accel_uncapped {

		static constexpr bool is_transfer_type = 1;

		double operator()(double x) const
		{
			return traits::anti_deriv(x, accel, power);
		}

		using accel_uncapped::accel_uncapped;
	};

	struct accel_classic : negatable {
		vec2d cap;
		double power;
		double accel;

		using traits = power_traits;

		double operator()(double x) const
		{
			if (x >= cap.x) return cap.y;

			return traits::def(x, accel, power);
		}

		accel_classic(const accel_args& args) :
			negatable(args.offset, args.cap.y),
			cap({ args.cap.x - args.offset, fabs(args.cap.y - 1) }),
			power(args.power - 1),
			accel(traits::scale(cap.x, cap.y, power)) {}
	};

	struct accel_classic_gain : accel_classic {
		double constant;

		static constexpr bool is_transfer_type = 1;

		double operator()(double x) const
		{
			if (x >= cap.x) return cap.y * x + constant;

			return traits::anti_deriv(x, accel, power);
		}

		accel_classic_gain(const accel_args& args) :
			accel_classic(args),
			constant(traits::anti_deriv(cap.x, accel, power) - cap.y * cap.x) {}
	};

}
