#pragma once

namespace rawaccel {

    template <typename T> struct vec2 { T x, y; };
    template <typename T> vec2(T, T)->vec2<T>;

    using vec2d = vec2<double>;

    using milliseconds = double;

    inline constexpr int POLL_RATE_MIN = 125;
    inline constexpr int POLL_RATE_MAX = 8000;

    inline constexpr milliseconds DEFAULT_TIME_MIN = 1000.0 / POLL_RATE_MAX / 2;
    inline constexpr milliseconds DEFAULT_TIME_MAX = 1000.0 / POLL_RATE_MIN * 2;

    inline constexpr milliseconds WRITE_DELAY = 1000;

    inline constexpr size_t MAX_DEV_ID_LEN = 200;

    inline constexpr size_t LUT_CAPACITY = 1025;

    enum class accel_mode {
        jump,
        uncapped,
        classic,
        natural,
        power,
        motivity,
        noaccel
    };

    enum class table_mode {
        off,
        binlog,
        linear
    };

    struct table_args {
        table_mode mode = table_mode::off;
        bool transfer = true;
        unsigned char partitions = 2;
        short num_elements = 8;
        double start = 0;
        double stop = 8;
    };

    struct accel_args {
        accel_mode mode = accel_mode::noaccel;
        bool gain_mode = true;
        table_args lut_args = {};

        double offset = 0;
        double accel_uncapped = 0.005;
        double accel_natural = 0.1;
        double accel_motivity = 0.75;
        double motivity = 1.25;
        double power = 2;
        double scale = 1;
        double exponent = 0.05;
        double limit = 1.5;
        double midpoint = 8;
        vec2d cap = { 20, 1.5 };
    };

    struct domain_args {
        static constexpr double max_norm = 32;

        vec2d domain_weights = { 1, 1 };
        double lp_norm = 2;
    };

    struct settings {
        double degrees_rotation = 0;
        double degrees_snap = 0;
        bool combine_mags = true;
        double dpi = 1600;
        double input_speed_cap = 0;
        
        vec2<accel_args> argsv;
        vec2d sens = { 1, 1 };
        vec2d dir_multipliers = {};
        domain_args domain_args = {};
        vec2d range_weights = { 1, 1 };
        milliseconds time_min = DEFAULT_TIME_MIN;
        milliseconds time_max = DEFAULT_TIME_MAX;
        wchar_t device_id[MAX_DEV_ID_LEN] = { 0 };
    };

    struct accel_base {
        static constexpr bool is_additive = false;
        static constexpr bool is_negatable = false;
        static constexpr bool is_transfer_type = false;
    };

    struct additive : accel_base {
        double offset;

        static constexpr bool is_additive = true;

        additive(double distance_offset) :
            offset(distance_offset) {}
    };

    struct negatable : additive {
        double sign;

        static constexpr bool is_negatable = true;

        negatable(double offset, double max_val) :
            additive(offset),
            sign((max_val < 1) ? -1 : 1) {}
    };

    template <typename AccelFunc>
    inline double apply_accel(const AccelFunc& accel, double x)
    {
        double intercept = 0;
        double distance = x;

        if constexpr (AccelFunc::is_additive) {
            intercept = 1;
            distance -= accel.offset;
        }

        if (distance <= 0) return intercept;

        double res = accel(distance);

        if constexpr (AccelFunc::is_negatable) {
            res *= accel.sign;
        }

        if constexpr (AccelFunc::is_transfer_type) {
            res /= x;
        }

        return res + intercept;
    }

}
