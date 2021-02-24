#pragma once

#include <utility-rawinput.hpp>
#include <rawaccel-io.hpp>
#include <rawaccel-version.h>
#include <rawaccel-validate.hpp>
#include <rawaccel-error.hpp>

#include <algorithm>
#include <cstdio>
#include <memory>
#include <type_traits>

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Runtime::InteropServices;
using namespace System::Reflection;

using namespace Windows::Forms;

using namespace Newtonsoft::Json;

namespace ra = rawaccel;

ra::settings default_settings; // effectively const for the gui

#pragma managed(push, off)

static UINT msg_changed() 
{
    static UINT msg = RegisterWindowMessageW(L"rawaccel-settings-change");
    return msg;
}

#pragma managed(pop)

public ref struct Notifications
{
    static initonly int SettingsChanged = msg_changed();
};

[JsonConverter(Converters::StringEnumConverter::typeid)]
public enum class AccelMode
{
    jump, uncapped, classic, natural, power, motivity, noaccel
};

public enum class TableMode 
{
    off, binlog, linear
};

[StructLayout(LayoutKind::Sequential)]
public value struct TableArgs
{
    [JsonIgnore]
    TableMode mode;

    [MarshalAs(UnmanagedType::U1)]
    bool transfer;

    [MarshalAs(UnmanagedType::U1)]
    unsigned char partitions;

    short numElements;
    double start;
    double stop;
};

generic <typename T>
[StructLayout(LayoutKind::Sequential)]
public value struct Vec2
{
    T x;
    T y;
};

[StructLayout(LayoutKind::Sequential)]
public value struct AccelArgs
{
    AccelMode mode;

    [MarshalAs(UnmanagedType::U1)]
    bool gain;

    [JsonProperty(Required = Required::Default)]
    TableArgs lutArgs;

    double offset;
    double accelUncapped;
    double accelNatural;
    double accelMotivity;
    double motivity;
    double power;
    double scale;
    double exponent;
    double limit;
    double midpoint;
    [JsonProperty("cap / jump")]
    Vec2<double> cap;
};

[StructLayout(LayoutKind::Sequential)]
public value struct DomainArgs
{
    Vec2<double> domainXY;
    double lpNorm;
};

[JsonObject(ItemRequired = Required::Always)]
[StructLayout(LayoutKind::Sequential, CharSet = CharSet::Unicode)]
public ref struct DriverSettings
{
    literal double WriteDelayMs = ra::WRITE_DELAY;
    literal String^ DefaultPath = "settings.json";

    [JsonProperty("Degrees of rotation")]
    double rotation;

    [JsonProperty("Degrees of angle snapping", Required = Required::Default)]
    double snap;

    [JsonProperty("Use x as whole/combined accel")]
    [MarshalAs(UnmanagedType::U1)]
    bool combineMagnitudes;

    double dpi;

    double speedCap;

    [JsonProperty("Accel parameters")]
    Vec2<AccelArgs> args;

    [JsonProperty("Sensitivity multipliers")]
    Vec2<double> sensitivity;

    [JsonProperty("Negative directional multipliers")]
    Vec2<double> directionalMultipliers;

    [JsonProperty("Stretches domain for horizontal vs vertical inputs")]
    DomainArgs domainArgs;

    [JsonProperty("Stretches accel range for horizontal vs vertical inputs")]
    Vec2<double> rangeXY;

    [JsonProperty(Required = Required::Default)]
    double minimumTime;
    [JsonProperty(Required = Required::Default)]
    double maximumTime;

    [JsonProperty("Device ID", Required = Required::Default)]
    [MarshalAs(UnmanagedType::ByValTStr, SizeConst = ra::MAX_DEV_ID_LEN)]
    String^ deviceID = "";

    bool ShouldSerializeminimumTime() 
    { 
        return minimumTime != ra::DEFAULT_TIME_MIN;
    }

    bool ShouldSerializemaximumTime()
    {
        return maximumTime != ra::DEFAULT_TIME_MAX;
    }

    DriverSettings() 
    {
        Marshal::PtrToStructure(IntPtr(&default_settings), this);
    }

    static property DriverSettings^ Default
    {
        DriverSettings^ get()
        {
            return gcnew DriverSettings();
        }    
    }

    bool IsDefaultEquivalent()
    {
        return String::IsNullOrEmpty(deviceID) &&
            sensitivity.x == 1 &&
            sensitivity.y == 1 &&
            directionalMultipliers.x <= 0 &&
            directionalMultipliers.y <= 0 &&
            rotation == 0 &&
            snap == 0 &&
            args.x.lutArgs.mode == TableMode::off &&
            args.y.lutArgs.mode == TableMode::off &&
            args.x.mode == AccelMode::noaccel &&
            (combineMagnitudes || args.y.mode == AccelMode::noaccel);
    }

    void ToFile(String^ path)
    {
        using namespace Newtonsoft::Json::Linq;

        JObject^ thisJO = JObject::FromObject(this);
        String^ modes = String::Join(" | ", Enum::GetNames(AccelMode::typeid));
        thisJO->AddFirst(gcnew JProperty("### Mode Types ###", modes));
        File::WriteAllText(path, thisJO->ToString(Formatting::Indented));
    }

    static DriverSettings^ FromFile(String^ path)
    {
        if (!File::Exists(path))
        {
            throw gcnew FileNotFoundException(
                String::Format("Settings file not found at {0}", path));
        }

        auto settings = JsonConvert::DeserializeObject<DriverSettings^>(
            File::ReadAllText(path));

        if (settings == nullptr) {
             throw gcnew JsonException(String::Format("{0} contains invalid JSON", path));
        }

        return settings;
    }

};

public ref struct InteropException : public Exception {
public:
    InteropException(String^ what) :
        Exception(what) {}
    InteropException(const char* what) :
        Exception(gcnew String(what)) {}
    InteropException(const std::exception& e) :
        InteropException(e.what()) {}
};

public ref class SettingsErrors
{
public:
    List<String^>^ list;
    int lastX;
    int lastY;

    delegate void MsgHandler(const char*);

    void Add(const char* msg)
    {
        list->Add(msclr::interop::marshal_as<String^>(msg));
    }

    SettingsErrors(DriverSettings^ settings) 
    {
        MsgHandler^ del = gcnew MsgHandler(this, &SettingsErrors::Add);
        GCHandle gch = GCHandle::Alloc(del);
        IntPtr ip = Marshal::GetFunctionPointerForDelegate(del);
        auto fp = static_cast<void (*)(const char*)>(ip.ToPointer());

        ra::settings args;
        Marshal::StructureToPtr(settings, (IntPtr)&args, false);

        list = gcnew List<String^>();
        auto [lx, ly, _] = ra::valid(args, fp);
        lastX = lx;
        lastY = ly;

        gch.Free();
    }

    bool Empty()
    {
        return list->Count == 0;
    }

    virtual String^ ToString() override 
    {
        Text::StringBuilder^ sb = gcnew Text::StringBuilder();

        for each (auto s in list->GetRange(0, lastX))
        {
            sb->AppendFormat("x: {0}\n", s);
        }
        for each (auto s in list->GetRange(lastX, lastY - lastX))
        {
            sb->AppendFormat("y: {0}\n", s);
        }
        for each (auto s in list->GetRange(lastY, list->Count - lastY)) 
        {
            sb->AppendLine(s);
        }

        return sb->ToString();
    }
};

struct device_info {
    std::wstring name;
    std::wstring id;
};

std::vector<device_info> get_unique_device_info() {
    std::vector<device_info> info;

    rawinput_foreach_with_interface([&](const auto& dev, const WCHAR* name) {
        info.push_back({
            L"", // get_property_wstr(name, &DEVPKEY_Device_FriendlyName), /* doesn't work */
            dev_id_from_interface(name)
        });
    });

    std::sort(info.begin(), info.end(),
        [](auto&& l, auto&& r) { return l.id < r.id; });
    auto last = std::unique(info.begin(), info.end(),
        [](auto&& l, auto&& r) { return l.id == r.id; });
    info.erase(last, info.end());

    return info;
}

public ref struct RawInputInterop
{
    static void AddHandlesFromID(String^ deviceID, List<IntPtr>^ rawInputHandles)
    {
        try
        {
            std::vector<HANDLE> nativeHandles = rawinput_handles_from_dev_id(
                msclr::interop::marshal_as<std::wstring>(deviceID));

            for (auto nh : nativeHandles) rawInputHandles->Add(IntPtr(nh));
        }
        catch (const std::exception& e)
        {
            throw gcnew InteropException(e);
        }
    }

    static List<ValueTuple<String^, String^>>^ GetDeviceIDs()
    {
        try
        {
            auto managed = gcnew List<ValueTuple<String^, String^>>();

            for (auto&& [name, id] : get_unique_device_info())
            {
                managed->Add(
                    ValueTuple<String^, String^>(
                        msclr::interop::marshal_as<String^>(name),
                        msclr::interop::marshal_as<String^>(id)));
            }

            return managed;
        }
        catch (const std::exception& e)
        {
            throw gcnew InteropException(e);
        }
    }

};

public ref class ManagedAccel
{
    ra::io_t* const instance = new ra::io_t();

    ra::vec2<ra::accel_invoker>* const invokers = 
        new ra::vec2<ra::accel_invoker>();

public:

    virtual ~ManagedAccel()
    {
        delete instance;
        delete invokers;
    }

    !ManagedAccel()
    {
        delete instance;
        delete invokers;
    }

    Tuple<double, double>^ Accelerate(int x, int y, double time)
    {
        ra::vec2d in_out_vec = {
            (double)x,
            (double)y
        };

        instance->mod.modify(in_out_vec, *invokers, time);

        return gcnew Tuple<double, double>(in_out_vec.x, in_out_vec.y);
    }

    void SetInvokers()
    {
        *invokers = ra::invokers(instance->args);
    }

    void Activate()
    {
        try {
            ra::write(*instance);
        }
        catch (const ra::error& e) {
            throw gcnew InteropException(e);
        }
    }

    property DriverSettings^ Settings
    {
        DriverSettings^ get()
        {
            DriverSettings^ settings = gcnew DriverSettings();
            Marshal::PtrToStructure(IntPtr(&instance->args), settings);
            return settings;
        }

        void set(DriverSettings^ val)
        {
            Marshal::StructureToPtr(val, IntPtr(&instance->args), false);
            instance->mod = { instance->args };
            SetInvokers();
        }

    }

    static ManagedAccel^ GetActive()
    {
        auto active = gcnew ManagedAccel();
        try {
            ra::read(*active->instance);
            active->SetInvokers();
        }
        catch (const ra::error& e) {
            throw gcnew InteropException(e);
        }

        return active;
    }

    static void Send(DriverSettings^ settings) 
    {
        ManagedAccel^ accel = gcnew ManagedAccel();
        Marshal::StructureToPtr(settings, IntPtr(&accel->instance->args), false);
        accel->instance->mod = { accel->instance->args };
        accel->Activate();
    }
};

public ref struct VersionHelper
{
    literal String^ VersionString = RA_VER_STRING;

    static Version^ WrapperVersion()
    {
        return VersionHelper::typeid->Assembly->GetName()->Version;
    }

    static Version^ CheckDriverVersion()
    {
        ra::version_t v;

        try {
            v = ra::check_version();
        }
        catch (const ra::error& e) {
            throw gcnew InteropException(e);
        }

        return gcnew Version(v.major, v.minor, v.patch, 0);
    }
};

#pragma managed(push, off)

using fill_callback_t = double (*)(double);

struct fill_args_t {
    fill_callback_t cb = nullptr;
    ra::table_args args = {};
};

static int fill_luts(const fill_args_t* fx, 
                     const fill_args_t* fy = nullptr,
                     bool by_component = false)
{
    ra::check_version(); 

    auto io_ptr = std::make_unique<ra::io_t>();
    auto& settings = io_ptr->args;
    settings = default_settings;

    settings.argsv.x.lut_args = fx->args;

    settings.combine_mags = fy == nullptr;

    if (fy) {
        settings.argsv.y.lut_args = fy->args;
    }
    else if (by_component) {
        settings.domain_args.lp_norm = ra::domain_args::max_norm;
    }

    auto valid = ra::valid(io_ptr->args, [](auto msg) {
        printf("%s\n", msg);
    });

    if (!valid) return 1;

    io_ptr->mod = { settings };

    auto fill = [](auto* fill_args, ra::accel_union& u) {
        switch (fill_args->args.mode) {
        case ra::table_mode::binlog:
            u.binlog_lut.fill(fill_args->cb);
            break;
        case ra::table_mode::linear:
            u.linear_lut.fill(fill_args->cb);
            break;
        default:
            throw ra::error("unsupported mode");
        }
    };

    fill(fx, io_ptr->mod.accel.x);
    if (fy) fill(fy, io_ptr->mod.accel.y);
    ra::write(*io_ptr);
    SendNotifyMessageW(HWND_BROADCAST, msg_changed(), 0, 0);
    return 0;
}

#define API __declspec(dllexport)

extern "C" {

API
int fill(fill_callback_t cb,
         ra::table_mode mode,
         double start,
         double stop,
         int num,
         bool transfer,
         bool by_component)
{
    if (num > ra::LUT_CAPACITY) {
        puts("num is too large\n");
        return 1;
    }
    else if (num <= 0) {
        puts("num must be positive");
        return 1;
    }

    fill_args_t fill_args;

    fill_args.cb = cb;
    fill_args.args = {
        mode, transfer, 1, static_cast<short>(num), start, stop 
    };
    
    try {
        return fill_luts(&fill_args, nullptr, by_component);
    }
    catch (const std::exception& e) {
        printf("%s\n", e.what());
        return 1;
    }
}

API
int fill_xy(fill_callback_t cb_x, 
            ra::table_mode mode_x, 
            double start_x, 
            double stop_x, 
            int num_x, 
            bool transfer_x,

            fill_callback_t cb_y,
            ra::table_mode mode_y,
            double start_y,
            double stop_y,
            int num_y,
            bool transfer_y)
{
    if (num_x > ra::LUT_CAPACITY || num_y > ra::LUT_CAPACITY) {
        puts("num is too large\n");
        return 1;
    }
    else if (num_x <= 0 || num_y <= 0) {
        puts("num must be positive");
        return 1;
    }

    ra::vec2<fill_args_t> fill_args;

    fill_args.x.cb = cb_x;
    fill_args.x.args = {
        mode_x, transfer_x, 1, static_cast<short>(num_x), start_x, stop_x
    };

    fill_args.y.cb = cb_y;
    fill_args.y.args = {
        mode_y, transfer_y, 1, static_cast<short>(num_y), start_y, stop_y
    };

    try {
        return fill_luts(&fill_args.x, &fill_args.y);
    }
    catch (const std::exception& e) {
        printf("%s\n", e.what());
        return 1;
    }
}

API
int set_rotation(double deg) {
    default_settings.degrees_rotation = deg;
    return 0;
}

API
int set_snap(double deg) {
    default_settings.degrees_snap = deg;
    return 0;
}

API
int set_dpi(double dpi) {
    default_settings.dpi = dpi;
    return 0;
}

API
int set_sens(double x, double y) {
    default_settings.sens = { x, y };
    return 0;
}

API
int set_domain_args(double x, double y, double lp_norm) {
    default_settings.domain_args = { { x, y }, lp_norm };
    return 0;
}

API
int set_range_args(double x, double y) {
    default_settings.range_weights = { x, y };
    return 0;
}

API
int set_time_clamp(double lo, double hi) {
    default_settings.time_min = lo;
    default_settings.time_max = hi;
    return 0;
}

API
int set_dir_mults(double x, double y) {
    default_settings.dir_multipliers = { x, y };
    return 0;
}

API
int set_speed_cap(double x) {
    default_settings.input_speed_cap = x;
    return 0;
}

API
int set_device_id(const wchar_t* id) {
    return wcsncpy_s(default_settings.device_id, id, ra::MAX_DEV_ID_LEN);
}

} // extern C

#pragma managed(pop)
