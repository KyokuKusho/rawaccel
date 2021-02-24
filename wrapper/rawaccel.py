import ctypes as ct
from enum import IntEnum

lib = ct.WinDLL('./wrapper.dll')
proto = ct.WINFUNCTYPE(ct.c_double, ct.c_double)

class mode(IntEnum):
    binlog = 1
    linear = 2
    
def make_safe(fn):
    def wrapped(x):
        try:
            return fn(x)
        except Exception as e:
            raise SystemExit(e)
    return wrapped

def fill(func, mode, start, stop, num, transfer, by_component):
    """fill the lookup table by applying a callable over a range
    
    func - callable with the signature: float -> float
    invoked sequentially, starting from the value represented 
    by start, to stop (inclusive)
    
    mode - represents the space in which func is applied
    
    start, stop, num - mode dependent arguments
    
    transfer - when True, values returned by func are interpreted as
    output magnitudes, as opposed to being values of magnitude ratio, 
    or sensitivity
    
    by_component - sets the flag to do separate lookups for x/y input
    
    notes:
    
        - the number of elements in the range represented by mode and 
        related arguments should be between 2 and 1025
    
        - regarding the output of the internal lookup function:
    
            - output is clamped for input that falls below the range
            (this works like an offset)
            
            - for input that is inside or past the range, 
            linear interp/extrapolation is performed
    """
    lib.fill(proto(make_safe(func)), 
            ct.c_int(mode), 
            ct.c_double(start), 
            ct.c_double(stop), 
            ct.c_int(num), 
            ct.c_bool(transfer), 
            ct.c_bool(by_component))

def lin_fill(func, start = 0.5, stop = 200, num = 100, transfer = False, by_component = False):
    """fill by applying func to linearly spaced numbers in [start, stop]
    
    start - float, starting value
    
    stop - float, last value
    
    num - int, number of elements in the range, and the size of the lookup table
    """
    fill(func, mode.linear, start, stop, num, transfer, by_component)

def log_fill(func, start = -1, stop = 8, num = 10, transfer = False, by_component = False):
    """fill by applying func over the range [2**start, 2**stop]

    start - int, binary logarithm of the starting value
    
    stop - int, binary logarithm of the last value
    
    num - int, number of elements linearly spaced between 
    each exponential step, including the lower bound
    
    lookup table size = (stop - start) * num + 1
    """
    fill(func, mode.binlog, start, stop, num, transfer, by_component)
    
def fill_xy(xfunc, xmode, xstart, xstop, xnum, xtransfer, 
            yfunc, ymode, ystart, ystop, ynum, ytransfer):
    """
    fill separate tables for x and y components
    """
    lib.fill_xy(proto(make_safe(xfunc)), 
                ct.c_int(xmode), 
                ct.c_double(xstart), 
                ct.c_double(xstop), 
                ct.c_int(xnum), 
                ct.c_bool(xtransfer), 
                proto(make_safe(yfunc)), 
                ct.c_int(ymode), 
                ct.c_double(ystart), 
                ct.c_double(ystop), 
                ct.c_int(ynum), 
                ct.c_bool(ytransfer))

'''
use these setters to override the default values for other arguments
sent to the driver when calling fill
'''

def set_rotation(degrees):
    lib.set_rotation(ct.c_double(degrees))

def set_snap(degrees):
    lib.set_snap(ct.c_double(degrees))

def set_dpi(dpi):
    lib.set_dpi(ct.c_double(dpi))
    
def set_sens(x, y):
    lib.set_sens(ct.c_double(x), ct.c_double(y))

def set_domain_args(x=1, y=1, lp_norm=2):
    lib.set_domain_args(ct.c_double(x), ct.c_double(y), ct.c_double(lp_norm))
    
def set_range_weights(x=1, y=1):
    lib.set_range_args(ct.c_double(x), ct.c_double(y))

def set_time_ms_clamp(lo, hi):
    lib.set_time_clamp(ct.c_double(lo), ct.c_double(hi))
    
def set_directional_mults(dm_x=1, dm_y=1):
    lib.set_dir_mults(ct.c_double(dm_x), ct.c_double(dm_y))
    
def set_speed_cap(x):
    lib.set_speed_cap(ct.c_double(x))

def set_device_id(dev_id):
    lib.set_device_id(ct.c_wchar_p(dev_id))
    