#pragma once

#include "rawaccel-base.hpp"

#include "accel-jump.hpp"
#include "accel-polynomial.hpp"
#include "accel-natural.hpp"
#include "accel-motivity.hpp"
#include "accel-noaccel.hpp"

namespace rawaccel {

    enum class mode_internal {
        jump_r,
        jump_g,

        uncapped_r,
        uncapped_g,

        classic_r,
        classic_g,

        natural_r,
        natural_g,

        power_r,
        power_g,

        motivity_r,
        motivity_g,

        lut_log,
        lut_lin,

        noaccel
    };

    constexpr mode_internal make_mode(accel_mode mode, bool gain, table_mode lut_mode)
    {
        if (lut_mode != table_mode::off) {
            switch (lut_mode) {
            case table_mode::binlog: return mode_internal::lut_log;
            case table_mode::linear: return mode_internal::lut_lin;
            default: return mode_internal::noaccel;
            }
        }
        else if (mode < accel_mode{} || mode >= accel_mode::noaccel) {
            return mode_internal::noaccel;
        }
        else {
            int m = static_cast<int>(mode) * 2 + (gain ? 1 : 0);
            return static_cast<mode_internal>(m);
        }
    }

    constexpr mode_internal make_mode(const accel_args& args)
    {
        return make_mode(args.mode, args.gain_mode, args.lut_args.mode);
    }

    template <typename Visitor, typename AccelUnion>
    constexpr auto visit_accel(Visitor vis, mode_internal mode, AccelUnion&& u)
    {
        switch (mode) {
        case mode_internal::jump_r:     return vis(u.jump);
        case mode_internal::jump_g:     return vis(u.jumpgain);
        case mode_internal::uncapped_r: return vis(u.uncapped);
        case mode_internal::uncapped_g: return vis(u.uncappedgain);
        case mode_internal::classic_r:  return vis(u.classic);
        case mode_internal::classic_g:  return vis(u.classicgain);
        case mode_internal::natural_r:  return vis(u.natural);
        case mode_internal::natural_g:  return vis(u.naturalgain);
        case mode_internal::power_r:    return vis(u.power);
        case mode_internal::power_g:    return vis(u.powergain);
        case mode_internal::motivity_r: return vis(u.sigmoid);
        case mode_internal::motivity_g: return vis(u.motivity);
        case mode_internal::lut_log:    return vis(u.binlog_lut);
        case mode_internal::lut_lin:    return vis(u.linear_lut);
        default:                        return vis(u.noaccel);
        }
    }

    union accel_union {
        accel_jump jump;
        accel_jump_gain jumpgain;
        accel_uncapped uncapped;
        accel_uncapped_gain uncappedgain;
        accel_classic classic;
        accel_classic_gain classicgain;
        accel_natural natural;
        accel_natural_gain naturalgain;
        accel_power power;
        accel_power_gain powergain;
        accel_sigmoid sigmoid;
        accel_motivity motivity;
        accel_binlog_lut binlog_lut;
        accel_linear_lut linear_lut;
        accel_noaccel noaccel = {};

        accel_union(const accel_args& args, mode_internal mode)
        {
            visit_accel([&](auto& impl) {
                impl = { args };
            }, mode, *this);
        }

        accel_union(const accel_args& args) :
            accel_union(args, make_mode(args)) {}

        accel_union() = default;
    };

    class accel_invoker {
    private:
        double (*accel_callback)(const accel_union&, double) = invoke_impl<accel_noaccel>;

        template <typename T>
        static double invoke_impl(const accel_union& u, double x)
        {
            return apply_accel(reinterpret_cast<const T&>(u), x);
        }

    public:
        accel_invoker(mode_internal mode)
        {
            accel_callback = visit_accel([=](auto&& arg) {
                using T = ttraits::remove_ref_t<decltype(arg)>;

                if constexpr (ttraits::is_same_v<T, accel_motivity>) {
                    static_assert(sizeof(accel_motivity) == sizeof(accel_binlog_lut));
                    return invoke_impl<accel_binlog_lut>;
                }
                else {
                    return invoke_impl<T>;
                }
            }, mode, accel_union{});
        }

        accel_invoker(const accel_args& args) :
            accel_invoker(make_mode(args)) {}

        accel_invoker() = default;

        double invoke(const accel_union& u, double x) const
        {
            return (*accel_callback)(u, x);
        }
    };

    inline vec2<accel_invoker> invokers(const settings& args)
    {
        return { accel_invoker(args.argsv.x), accel_invoker(args.argsv.y) };
    }

}
