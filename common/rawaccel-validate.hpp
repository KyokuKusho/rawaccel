#pragma once

#include "rawaccel-base.hpp"

namespace rawaccel {

	struct valid_ret_t {
		int last_x = 0;
		int last_y = 0;
		int last = 0;

		explicit operator bool() {
			return last == 0;
		}
	};

	auto nop = [](auto&&) { return; };

	template <typename MsgHandler = decltype(nop)>
	valid_ret_t valid(const settings& args, MsgHandler fn = {})
	{
		int count = 0;

		auto error = [&](auto msg) {
			++count;
			fn(msg);
		};

		auto validate_accel = [&error](const accel_args& args) {
			static_assert(LUT_CAPACITY == 1025);

			const auto& lut_args = args.lut_args;

			if (lut_args.partitions <= 0) {
				error("lut partitions"" must be positive");
			}

			if (lut_args.mode == table_mode::linear) {
				if (lut_args.start <= 0) {
					error("lut start"" must be positive");
				}

				if (lut_args.stop <= lut_args.start) {
					error("lut stop must be greater than start");
				}

				if (lut_args.num_elements < 2) {
					error("lut should have at least 2 elements");
				}
				else if (lut_args.num_elements > 1025) {
					error("max num elements is 1025");
				}
			}
			else if (lut_args.mode == table_mode::binlog) {
				int istart = static_cast<int>(lut_args.start);
				int istop = static_cast<int>(lut_args.stop);

				if (lut_args.start < -99) {
					error("lut start should be greater than -99");
				}
				else if (lut_args.stop >= 99) {
					error("lut stop should be less than 99");
				}
				else if (istart != lut_args.start || istop != lut_args.stop) {
					error("lut arguments must be integers");
				}
				else if (istop <= istart) {
					error("lut stop must be greater than start");
				}
				else if (lut_args.num_elements <= 0) {
					error("lut num elements"" must be positive");
				}
				else if (((lut_args.stop - lut_args.start) * lut_args.num_elements) >= 1025) {
					error("binlog mode requires (num * (stop - start)) < 1025");
				}
			}

			if (args.offset < 0) {
				error("offset can not be negative");
			}

			if (args.accel_motivity <= 0 ||
				args.accel_natural <= 0 ||
				args.accel_uncapped <= 0) {
				error("acceleration"" must be positive");
			}

			if (args.motivity <= 1) {
				error("motivity must be greater than 1");
			}

			if (args.power <= 1) {
				error("power must be greater than 1");
			}

			if (args.scale <= 0) {
				error("scale"" must be positive");
			}

			if (args.exponent <= 0) {
				error("exponent"" must be positive");
			}

			if (args.limit <= 0) {
				error("limit"" must be positive");
			}

			if (args.midpoint <= 0) {
				error("midpoint"" must be positive");
			}

			if (args.cap.x <= 0 || args.cap.y <= 0) {
				error("cap"" must be positive");
			}
		};

		valid_ret_t ret;

		validate_accel(args.argsv.x);
		
		if (!args.combine_mags) {
			ret.last_x = count;
			validate_accel(args.argsv.y);
			ret.last_y = count;
		}

		if (args.dpi <= 0) {
			error("dpi"" must be positive");
		}

		if (args.sens.x == 0 || args.sens.y == 0) {
			error("sens multiplier is 0");
		}

		if (args.domain_args.domain_weights.x <= 0 ||
			args.domain_args.domain_weights.y <= 0) {
			error("domain weights"" must be positive");
		}

		if (args.domain_args.lp_norm <= 0) {
			error("Lp norm"" must be positive");
		}

		if (args.dir_multipliers.x < 0 || args.dir_multipliers.y < 0) {
			error("directional multipliers can not be negative");
		}

		if (args.range_weights.x <= 0 || args.range_weights.y <= 0) {
			error("range weights"" must be positive");
		}

		if (args.time_min <= 0) {
			error("minimum time"" must be positive");
		}

		if (args.time_max < args.time_min) {
			error("max time is less than min time");
		}

		ret.last = count;
		return ret;
	}

}
