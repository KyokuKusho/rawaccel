#pragma once

#include "rawaccel-base.hpp"
#include "accel-lookup.hpp"

#include <math.h>

namespace rawaccel {

	struct accel_sigmoid : accel_base {
		double accel;
		double motivity;
		double midpoint;

		accel_sigmoid(const accel_args& args) :
			accel(-exp(args.accel_motivity)),
			motivity(2 * log(args.motivity)),
			midpoint(log(args.midpoint)) {}

		double operator()(double x) const
		{
			double denom = exp(accel * (log(x) - midpoint)) + 1;
			return exp(motivity / denom - motivity / 2);
		}
	};

	struct accel_motivity : accel_binlog_lut {

		using accel_binlog_lut::operator();

		accel_motivity(const accel_args& args) :
			accel_binlog_lut(args)
		{
			double sum = 0;
			double a = 0;
			auto sigmoid_sum = [&, sig = accel_sigmoid(args)] (double b) mutable {
				double interval = (b - a) / args.lut_args.partitions;
				for (int i = 1; i <= args.lut_args.partitions; i++) {
					sum += sig(a + i * interval) * interval;
				}
				a = b;
				return sum;
			};

			fill([&](double x) {
				double y = sigmoid_sum(x);
				if (!this->transfer) y /= x;
				return y;
			});
		}
	};

}
