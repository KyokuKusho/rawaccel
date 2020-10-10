#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "rawaccel-settings.h"
#include "x64-util.hpp"

namespace rawaccel {

    /// <summary> Struct to hold vector rotation details. </summary>
    struct rotator {

        /// <summary> Rotational vector, which points in the direction of the post-rotation positive x axis. </summary>
        vec2d rot_vec = { 1, 0 };

        /// <summary>
        /// Rotates given input vector according to struct's rotational vector.
        /// </summary>
        /// <param name="input">Input vector to be rotated</param>
        /// <returns>2d vector of rotated input.</returns>
        inline vec2d apply(const vec2d& input) const {
            return {
                input.x * rot_vec.x - input.y * rot_vec.y,
                input.x * rot_vec.y + input.y * rot_vec.x
            };
        }

        rotator(double degrees) {
            double rads = degrees * M_PI / 180;
            rot_vec = { cos(rads), sin(rads) };
        }

        rotator() = default;
    };

    /// <summary> Struct to hold clamp (min and max) details for acceleration application </summary>
    struct accel_scale_clamp {
        double lo = 0;
        double hi = 128;

        /// <summary>
        /// Clamps given input to min at lo, max at hi.
        /// </summary>
        /// <param name="scale">Double to be clamped</param>
        /// <returns>Clamped input as double</returns>
        inline double operator()(double scale) const {
            return clampsd(scale, lo, hi);
        }

        accel_scale_clamp(double cap) {
            if (cap <= 0) {
                // use default, effectively uncapped accel
                return;
            }

            if (cap < 1) {
                // assume negative accel
                lo = cap;
                hi = 1;
            }
            else hi = cap;
        }

        accel_scale_clamp() = default;
    };

    struct gain::implementations {
        static inline double tanh(double x) {
            return ::tanh(x);
        }

        static inline double gd(double x) { 
            return M_2_PI * atan(sinh(x * M_PI_2));
        }
    };


    struct gain::function {
        mode tag = {};

        inline double operator()(double x) const {
            using impls = implementations;

            switch (tag) {
            case mode::tanh: return impls::tanh(x);
            case mode::gd:   return impls::gd(x);
            default:         return 0;
            }
        }
    };

    struct accelerator {

        union gain_union_t {
            gain::function fn = {};
            gain::lookup_value_t* lut;
        } gain_u = {};
        
        struct transfer_constants {
            double M = 0;
            double A = 0;
            double C = 0;
            double G = 0;
        } constants;

        accel_scale_clamp clamp;

        accelerator(const accel_args& args) : clamp(args.hard_cap) {
            if (args.gamma == 0 || args.motivity <= 1) {
                constants.M = 1; // edge case of unable to have contrast
                return;
            }

            if (args.synchronous_speed > 0) {
                constants.A = log(args.synchronous_speed);
                constants.C = log(args.motivity);
                constants.G = args.gamma / constants.C;
            }
            else {
                constants.M = args.motivity; // edge case of always at max val
            }
        }

        accelerator(const accel_args& args, gain::mode mode) : accelerator(args) {
            gain_u.fn.tag = mode;
        }

        accelerator(const accel_args& args, gain::lookup_value_t* lut) : accelerator(args) {
            gain_u.lut = lut;
        }

        accelerator() = default;

        template <typename Func>
        inline double accel_impl(double speed, Func fn) const {
            if (constants.M > 0)
                return constants.M;

            auto transferred = constants.G * (log(speed) - constants.A);
            return clamp(exp(fn(transferred) * constants.C));
        }

        inline double apply(double speed) const {
            return accel_impl(speed, [this](double x) {
                return gain_u.fn(x);
            });
        }

        inline double apply_lookup(double speed) const {
            return accel_impl(speed, [this](double x) {
                // TODO
                return 1; 
            });
        }

    };

    /// <summary> Struct to hold variables and methods for modifying mouse input </summary>
    struct mouse_modifier {
        bool apply_rotate = false;
        bool apply_accel = false;
        bool combine_magnitudes = true;
        rotator rotate;
        vec2<accelerator> accels;
        vec2d sensitivity = { 1, 1 };

        mouse_modifier(const settings& args) :
            apply_accel(args.apply_accel), combine_magnitudes(args.combine_mags)
        {
            if (apply_accel) {
                accels.x = accelerator(args.argsv.x, args.modes.x);
                accels.y = accelerator(args.argsv.y, args.modes.y);
            }

            if (args.degrees_rotation != 0) {
                rotate = rotator(args.degrees_rotation);
                apply_rotate = true;
            }
            
            if (args.sens.x != 0) sensitivity.x = args.sens.x;
            if (args.sens.y != 0) sensitivity.y = args.sens.y;
        }

        void modify(vec2d& movement, milliseconds time) {
            apply_rotation(movement);
            apply_acceleration(movement, [=] { return time; });
            apply_sensitivity(movement);
        }

        inline void apply_rotation(vec2d& movement) {
            if (apply_rotate) movement = rotate.apply(movement);
        }

        template <typename TimeSupplier>
        inline void apply_acceleration(vec2d& movement, TimeSupplier time_supp) {
            if (apply_accel) {
                milliseconds time = time_supp();

                if (combine_magnitudes) {
                    double mag = sqrtsd(movement.x * movement.x + movement.y * movement.y);
                    if (mag == 0) return;
                    double speed = mag / time;
                    double scale = accels.x.apply(speed);
                    movement.x *= scale;
                    movement.y *= scale;
                }
                else {
                    if (movement.x != 0) 
                        movement.x *= accels.x.apply(fabs(movement.x) / time);
                    if (movement.y != 0)
                        movement.y *= accels.y.apply(fabs(movement.y) / time);
                }
            }
        }

        inline void apply_sensitivity(vec2d& movement) {
            movement.x *= sensitivity.x;
            movement.y *= sensitivity.y;
        }

        mouse_modifier() = default;
    };

} // rawaccel
