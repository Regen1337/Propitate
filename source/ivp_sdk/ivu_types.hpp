#pragma once
#include <signal.h>
#include <cstddef>

#ifndef _IVP_U_TYPES_INCLUDED_
#define _IVP_U_TYPES_INCLUDED_

#ifndef __PTRDIFF_TYPE__
  #define __PTRDIFF_TYPE__ long int
#endif

//#define IVP_NO_DOUBLE // only use if processor does not support double precision, or if we should only use float precision for some reason.
#define IVP_VECTOR_UNIT_FLOAT // If extra data should be inserted to support vector units
#define IVP_VECTOR_UNIT_DOUBLE // If extra data should be inserted to support double precision vector units

#define IVP_USE(a) a=a
#define IVP_IF(flag) if (flag)

typedef __PTRDIFF_TYPE__ ptrdiff_t;

using IVP_FLOAT = float;
using IVP_INT32 = int;
using IVP_UINT32 = unsigned int;
using IVP_INTP = ptrdiff_t;
using IVP_UINTP = size_t;
using IVP_TIME_CODE = IVP_INT32;
using IVP_U_BOOL = IVP_INT32;
using IVP_HTIME = IVP_FLOAT;
using IVP_ERROR_STRING = const char*;

constexpr inline std::nullptr_t IVP_NO_ERROR{nullptr};
constexpr inline IVP_FLOAT IVP_FLOAT_EPS{1e-10f};
constexpr inline IVP_FLOAT IVP_FLOAT_RES{1e-6f};
constexpr inline IVP_FLOAT IVP_FLOAT_MAX{1e16f};

constexpr inline IVP_FLOAT IVP_MAX_OBJECT_SIZE{1000.0f};
constexpr inline IVP_FLOAT IVP_MIN_EDGE_LEN{0.01f};

enum IVP_BOOL {
  IVP_FALSE = 0,
  IVP_TRUE = 1
};

enum IVP_RETURN_TYPE {
  IVP_FAULT = 0,
  IVP_OK = 1
};


#ifndef IVP_NO_DOUBLE
  using IVP_DOUBLE = double;

  #define IVP_PI 3.14159265358979323846f
  #define IVP_PI_2 1.57079632679489661923f
  #define IVP_DOUBLE_MAX IVP_FLOAT_MAX
  #define IVP_DOUBLE_RES IVP_FLOAT_RES
  #define IVP_DOUBLE_EPS IVP_FLOAT_EPS
  #define IVP_3D_SOLVER_NULLSTELLE_EPS 3e-3f
  #define IVP_MAX_WORLD_DOUBLE 3000.0f

  class IVP_Time {
    private:
      IVP_DOUBLE seconds;
    
    public:
      void operator+=(const IVP_DOUBLE& d) { seconds += d; }
      IVP_DOUBLE getSeconds() const { return seconds; }
      IVP_DOUBLE getTime() const { return seconds; }
      IVP_DOUBLE operator-(const IVP_Time& t) const { return seconds - t.seconds; } // apparently originally casted to a float? but why would u do that?
      void operator-=(const IVP_Time& t) { seconds -= t.seconds; }
      IVP_Time operator+(const IVP_DOUBLE& d) const { return IVP_Time(seconds + d); }

      IVP_Time() = default;
      IVP_Time(IVP_DOUBLE d) : seconds(d) {}
  };
#else
  using IVP_DOUBLE = float;

  constexpr inline double IVP_PI{3.14159265358979323846}
  constexpr inline double IVP_PI_2{1.57079632679489661923}
  constexpr inline double IVP_DOUBLE_MAX{1e20}
  constexpr inline double IVP_DOUBLE_RES{1e-12}
  constexpr inline double IVP_DOUBLE_EPS{1e-10}
  constexpr inline double IVP_3D_SOLVER_NULLSTELLE_EPS{1e-8}
  constexpr inline double IVP_MAX_WORLD_DOUBLE{10000}

  class IVP_Time {
    private:
      IVP_FLOAT seconds;
      IVP_FLOAT subSeconds;
  
    public:
      void operator+=(IVP_Float val) { this->sub_seconds += val; while (this->sub_seconds > 1.0f) { this->seconds++; this->sub_seconds -= 1.0f; } }
      IVP_FLOAT getSeconds() const { return seconds; }
      IVP_FLOAT getTime() const { return seconds + subSeconds; }
      IVP_FLOAT operator-(const IVP_Time& t) const { return (seconds - t.seconds) + (subSeconds - t.subSeconds); }
      void operator-=(const IVP_Time& t) { this->seconds -= t.seconds; this->subSeconds -= t.subSeconds; while (this->subSeconds > 1.0f) { this->seconds++; this->subSeconds -= 1.0f; } while (this->subSeconds < 0.0f) { this->seconds--; this->subSeconds += 1.0f; } }
      IVP_Time operator+(IVP_Float val) const { IVP_Time result; result.seconds = this->seconds; result.sub_seconds = this->sub_seconds + val; while (result.sub_seconds > 1.0f) { result.seconds++; result.sub_seconds -= 1.0f; } return result; }

      IVP_Time() = default;
      IVP_Time(IVP_FLOAT d) : seconds((IVP_FLOAT)(int)d), subSeconds(d - (IVP_FLOAT)(int)d) {}
  };
#endif

constexpr inline float IVP_RES_EPS{IVP_MAX_WORLD_DOUBLE * IVP_DOUBLE_RES};

void IVP_SRAND(IVP_UINT32 seed);
IVP_FLOAT IVP_RAND(); // [0 .. 1]
#endif