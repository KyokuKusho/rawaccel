#pragma once

#include "accel-union.hpp"

#define _USE_MATH_DEFINES
#include <math.h>

namespace rawaccel {

    inline double radians(double degrees)
    {
        return degrees * M_PI / 180;
    }

    /// <summary> Struct to hold vector rotation details. </summary>
    struct rotator {

        /// <summary> Rotational vector, which points in the direction of the post-rotation positive x axis. </summary>
        vec2d rot_vec = { 1, 0 };

        /// <summary>
        /// Rotates given input vector according to struct's rotational vector.
        /// </summary>
        /// <param name="input">Input vector to be rotated</param>
        /// <returns>2d vector of rotated input.</returns>
        vec2d operator()(const vec2d& input) const {
            return {
                input.x * rot_vec.x - input.y * rot_vec.y,
                input.x * rot_vec.y + input.y * rot_vec.x
            };
        }

        rotator(double degrees)
        {
            if (degrees != 0) {
                rot_vec = {
                    cos(radians(-degrees)),
                    sin(radians(-degrees))
                };
            }
        }

        rotator() = default;
    };

    inline bool should_apply(const rotator& r)
    {
        return r.rot_vec.x != 1;
    }

    struct snapper {
        double threshold = 0;

        vec2d operator()(const vec2d& input, double& ref_angle, double mag) const
        {
            if (ref_angle > M_PI_2 - threshold) {
                ref_angle = M_PI_2;
                return { 0, _copysign(mag, input.y) };
            }
            if (ref_angle < threshold) {
                ref_angle = 0;
                return { _copysign(mag, input.x), 0 };
            }
            return input;
        }

        snapper(double degrees) :
            threshold(radians(minsd(fabs(degrees), 45))) {}

        snapper() = default;
    };

    inline bool should_apply(const snapper& snap)
    {
        return snap.threshold != 0;
    }
    
    inline double clamp_mag(vec2d& v, double v_mag, double max_mag)
    {
        double ratio = minsd(v_mag, max_mag) / v_mag;
        v.x *= ratio;
        v.y *= ratio;
        return ratio;
    }

    struct weighted_distance {
        static constexpr double max_norm_sentinel = 0;

        vec2d sigma = { 1.0, 1.0 };
        double p = 2.0;
       
        double operator()(const vec2d& in) const
        {
            vec2d abs_in = { fabs(in.x), fabs(in.y) };

            if (p == max_norm_sentinel) {
                return maxsd(abs_in.x, abs_in.y);
            }

            double x_p = pow(abs_in.x * sigma.x, p);
            double y_p = pow(abs_in.y * sigma.y, p);
            return pow(x_p + y_p, 1 / p);
        }

        weighted_distance(const domain_args& args) :
            sigma(args.domain_weights)
        {
            if (p <= 0 || p >= domain_args::max_norm) {
                p = max_norm_sentinel;
            }
            else {
                p = args.lp_norm;
            }
        }

        weighted_distance() = default;
    };

    inline bool should_apply(const weighted_distance& dist)
    {
        return dist.p != 2 || dist.sigma.x != 1 || dist.sigma.y != 1;
    }

    struct direction_weight {
        double diff = 0.0;
        double start = 1.0;

        double operator()(double scale, double reference_angle) const
        {
            double correction = M_2_PI * reference_angle * diff + start;
            return (scale - 1) * correction + 1;
        }

        direction_weight(const vec2d& thetas) :
            diff(thetas.y - thetas.x),
            start(thetas.x) {}

        direction_weight() = default;
    };

    inline bool should_apply(const direction_weight& dw)
    {
        return dw.diff != 0;
    }

    /// <summary> Struct to hold variables and methods for modifying mouse input </summary>
    struct mouse_modifier {

        struct {
            bool compute_mag =       1;
            bool rotate =            0;
            bool compute_ref_angle = 0;
            bool snap =              0;
            bool cap_speed =         0;
            bool by_component =      0;
            bool transform_domain =  0;
            bool transform_range =   0;
        } flags;

        rotator rotate;
        snapper snap;
        double dpi_factor = 1;
        double speed_cap = 0;
        weighted_distance distance;
        direction_weight directional;
        vec2d directional_multipliers = {};
        vec2d sensitivity = { 1, 1 }; 
        vec2<accel_union> accel;

#ifdef _KERNEL_MODE
        __forceinline
#endif
        void modify(vec2d& in, const vec2<accel_invoker>& invokers, double time = 1) const
        {
            double magnitude = 0;
            double reference_angle = 0;
            double norm = dpi_factor / time;

            if (flags.compute_mag) {
                magnitude = sqrt(in.x * in.x + in.y * in.y);
            }

            if (flags.rotate) {
                in = rotate(in);
            }

            if (flags.compute_ref_angle) {
                reference_angle = atan2(fabs(in.y), fabs(in.x));
            }

            if (flags.snap) {
                in = snap(in, reference_angle, magnitude);
            }

            if (flags.cap_speed) {
                magnitude *= clamp_mag(in, magnitude * norm, speed_cap);
            }

            if (flags.by_component) {
                in.x *= invokers.x.invoke(accel.x, fabs(in.x) * norm);
                in.y *= invokers.y.invoke(accel.y, fabs(in.y) * norm);
            }
            else {
                if (flags.transform_domain) {
                    magnitude = distance(in);
                }

                double scale = invokers.x.invoke(accel.x, magnitude * norm);

                if (flags.transform_range) {
                    scale = directional(scale, reference_angle);
                }

                in.x *= scale;
                in.y *= scale;
            }

            if (directional_multipliers.x > 0 && in.x < 0) {
                in.x *= directional_multipliers.x;
            }

            if (directional_multipliers.y > 0 && in.y < 0) {
                in.y *= directional_multipliers.y;
            }

            in.x *= sensitivity.x;
            in.y *= sensitivity.y;
        }

        mouse_modifier(const settings& args) :
            rotate(args.degrees_rotation),
            snap(args.degrees_snap),
            dpi_factor(1600 / args.dpi),
            speed_cap(args.input_speed_cap),
            distance(args.domain_args),
            directional(args.range_weights),
            directional_multipliers(args.dir_multipliers),
            sensitivity(args.sens),
            accel({ { args.argsv.x }, { args.argsv.y }})
        {
            flags.by_component = !args.combine_mags;
            flags.cap_speed = speed_cap > 0;
            flags.rotate = should_apply(rotate);
            flags.snap = should_apply(snap);
            flags.transform_domain = should_apply(distance);
            flags.transform_range = should_apply(directional);
            flags.compute_ref_angle = flags.snap || flags.transform_range;
            flags.compute_mag = flags.snap || flags.cap_speed ||
                !flags.transform_domain;
        }

        mouse_modifier() = default;
    };

    struct io_t {
        settings args;
        mouse_modifier mod;
    };

} // rawaccel
