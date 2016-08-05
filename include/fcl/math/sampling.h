/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013-2014, Willow Garage, Inc.
 *  Copyright (c) 2014-2016, Open Source Robotics Foundation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Open Source Robotics Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Jia Pan */

#ifndef FCL_MATH_SAMPLING_H
#define FCL_MATH_SAMPLING_H

#include <random>
#include <cassert>
#include "fcl/math/constants.h"

namespace fcl
{

/// @brief Random number generation. An instance of this class
/// cannot be used by multiple threads at once (member functions
/// are not const). However, the constructor is thread safe and
/// different instances can be used safely in any number of
/// threads. It is also guaranteed that all created instances will
/// have a different random seed.

class RNG
{
public:
	
  /// @brief Constructor. Always sets a different random seed
  RNG();
		
  /// @brief Generate a random real between 0 and 1
  double uniform01()
  {
    return uniDist_(generator_);
  }
		
  /// @brief Generate a random real within given bounds: [\e lower_bound, \e upper_bound)
  double uniformReal(double lower_bound, double upper_bound)
  {
    assert(lower_bound <= upper_bound);
    return (upper_bound - lower_bound) * uniDist_(generator_) + lower_bound;
  }
		
  /// @brief Generate a random integer within given bounds: [\e lower_bound, \e upper_bound]
  int uniformInt(int lower_bound, int upper_bound)
  {
    int r = (int)floor(uniformReal((double)lower_bound, (double)(upper_bound) + 1.0));
    return (r > upper_bound) ? upper_bound : r;
  }
		
  /// @brief Generate a random boolean
  bool uniformBool()
  {
    return uniDist_(generator_) <= 0.5;
  }
		
  /// @brief Generate a random real using a normal distribution with mean 0 and variance 1
  double gaussian01()
  {
    return normalDist_(generator_);
  }
		
  /// @brief Generate a random real using a normal distribution with given mean and variance
  double gaussian(double mean, double stddev)
  {
    return normalDist_(generator_) * stddev + mean;
  }
		
  /// @brief Generate a random real using a half-normal distribution. The value is within specified bounds [\e r_min, \e r_max], but with a bias towards \e r_max. The function is implemended using a Gaussian distribution with mean at \e r_max - \e r_min. The distribution is 'folded' around \e r_max axis towards \e r_min. The variance of the distribution is (\e r_max - \e r_min) / \e focus. The higher the focus, the more probable it is that generated numbers are close to \e r_max.
  double halfNormalReal(double r_min, double r_max, double focus = 3.0);
		
  /// @brief Generate a random integer using a half-normal distribution. The value is within specified bounds ([\e r_min, \e r_max]), but with a bias towards \e r_max. The function is implemented on top of halfNormalReal()
  int halfNormalInt(int r_min, int r_max, double focus = 3.0);
		
  /// @brief Uniform random unit quaternion sampling. The computed value has the order (x,y,z,w) 
  void quaternion(double value[4]);
		
  /// @brief Uniform random sampling of Euler roll-pitch-yaw angles, each in the range [-pi, pi). The computed value has the order (roll, pitch, yaw) */
  void   eulerRPY(double value[3]);
		
  /// @brief Uniform random sample on a disk with radius from r_min to r_max
  void disk(double r_min, double r_max, double& x, double& y);
		
  /// @brief Uniform random sample in a ball with radius from r_min to r_max
  void ball(double r_min, double r_max, double& x, double& y, double& z);
		
  /// @brief Set the seed for random number generation. Use this function to ensure the same sequence of random numbers is generated.
  static void setSeed(std::uint_fast32_t seed);
		
  /// @brief Get the seed used for random number generation. Passing the returned value to setSeed() at a subsequent execution of the code will ensure deterministic (repeatable) behaviour. Useful for debugging.
  static std::uint_fast32_t getSeed();
		
private:
	
  std::mt19937                     generator_;
  std::uniform_real_distribution<> uniDist_;
  std::normal_distribution<>       normalDist_;
		
};

class SamplerBase
{
public:
  mutable RNG rng;
};

template<std::size_t N>
class SamplerR : public SamplerBase
{
public:
  SamplerR() {}

  SamplerR(const VectorNd<N>& lower_bound_,
           const VectorNd<N>& upper_bound_) : lower_bound(lower_bound_),
                                           upper_bound(upper_bound_)
  {
  }

  void setBound(const VectorNd<N>& lower_bound_,
                const VectorNd<N>& upper_bound_)
  {
    lower_bound = lower_bound_;
    upper_bound = upper_bound_;
  }

  void getBound(VectorNd<N>& lower_bound_,
                VectorNd<N>& upper_bound_) const
  {
    lower_bound_ = lower_bound;
    upper_bound_ = upper_bound;
  }

  VectorNd<N> sample() const
  {
    VectorNd<N> q;

    for(std::size_t i = 0; i < N; ++i)
    {
      q[i] = rng.uniformReal(lower_bound[i], upper_bound[i]);
    }

    return q;
  }

private:
  VectorNd<N> lower_bound;
  VectorNd<N> upper_bound;

};


class SamplerSE2 : public SamplerBase
{
public:
  SamplerSE2() {}

  SamplerSE2(const VectorNd<2>& lower_bound_,
             const VectorNd<2>& upper_bound_) : lower_bound(lower_bound_),
                                             upper_bound(upper_bound_)
  {}

  SamplerSE2(FCL_REAL x_min, FCL_REAL x_max,
             FCL_REAL y_min, FCL_REAL y_max) : lower_bound(VectorNd<2>(x_min, y_min)),
                                               upper_bound(VectorNd<2>(x_max, y_max))
                                               
  {}


  void setBound(const VectorNd<2>& lower_bound_,
                const VectorNd<2>& upper_bound_)
  {
    lower_bound = lower_bound_;
    upper_bound = upper_bound_;
  }

  void getBound(VectorNd<2>& lower_bound_,
                VectorNd<2>& upper_bound_) const
  {
    lower_bound_ = lower_bound;
    upper_bound_ = upper_bound;
  }


  VectorNd<3> sample() const
  {
    VectorNd<3> q;
    q[0] = rng.uniformReal(lower_bound[0], lower_bound[1]);
    q[1] = rng.uniformReal(lower_bound[1], lower_bound[2]);
    q[2] = rng.uniformReal(-constants::pi, constants::pi);

    return q;
  }

protected:
  VectorNd<2> lower_bound;
  VectorNd<2> upper_bound;
};


class SamplerSE2_disk : public SamplerBase
{
public:
  SamplerSE2_disk() {}

  SamplerSE2_disk(FCL_REAL cx, FCL_REAL cy,
                  FCL_REAL r1, FCL_REAL r2,
                  FCL_REAL crefx, FCL_REAL crefy)
  {
    setBound(cx, cy, r1, r2, crefx, crefy);
  }

  void setBound(FCL_REAL cx, FCL_REAL cy,
                FCL_REAL r1, FCL_REAL r2,
                FCL_REAL crefx, FCL_REAL crefy)
  {
    c[0] = cx; c[1] = cy;
    cref[0] = crefx; cref[1] = crefy;
    r_min = r1;
    r_max = r2;
  }

  VectorNd<3> sample() const
  {
    VectorNd<3> q;
    FCL_REAL x, y;
    rng.disk(r_min, r_max, x, y);
    q[0] = x + c[0] - cref[0];
    q[1] = y + c[1] - cref[1];
    q[2] = rng.uniformReal(-constants::pi, constants::pi);

    return q;
  }

protected:
  FCL_REAL c[2];
  FCL_REAL cref[2];
  FCL_REAL r_min, r_max;
};

class SamplerSE3Euler : public SamplerBase
{
public:
  SamplerSE3Euler() {}

  SamplerSE3Euler(const VectorNd<3>& lower_bound_,
                  const VectorNd<3>& upper_bound_) : lower_bound(lower_bound_),
                                                  upper_bound(upper_bound_)
  {}

  void setBound(const VectorNd<3>& lower_bound_,
                const VectorNd<3>& upper_bound_)

  {
    lower_bound = lower_bound_;
    upper_bound = upper_bound_;
  }

  void getBound(VectorNd<3>& lower_bound_,
                VectorNd<3>& upper_bound_) const
  {
    lower_bound_ = lower_bound;
    upper_bound_ = upper_bound;
  }

  VectorNd<6> sample() const
  {
    VectorNd<6> q;
    q[0] = rng.uniformReal(lower_bound[0], upper_bound[0]);
    q[1] = rng.uniformReal(lower_bound[1], upper_bound[1]);
    q[2] = rng.uniformReal(lower_bound[2], upper_bound[2]);

    FCL_REAL s[4];
    rng.quaternion(s);

    Quaternion3d quat(s[0], s[1], s[2], s[3]);
    Vector3d angles = quat.toRotationMatrix().eulerAngles(0, 1, 2);

    q[3] = angles[0];
    q[4] = angles[1];
    q[5] = angles[2];

    return q;
  }

protected:
  VectorNd<3> lower_bound;
  VectorNd<3> upper_bound;
  
};

class SamplerSE3Quat : public SamplerBase
{
public:
  SamplerSE3Quat() {}

  SamplerSE3Quat(const VectorNd<3>& lower_bound_,
                 const VectorNd<3>& upper_bound_) : lower_bound(lower_bound_),
                                                 upper_bound(upper_bound_)
  {}

  void setBound(const VectorNd<3>& lower_bound_,
                const VectorNd<3>& upper_bound_)

  {
    lower_bound = lower_bound_;
    upper_bound = upper_bound_;
  }

  void getBound(VectorNd<3>& lower_bound_,
                VectorNd<3>& upper_bound_) const
  {
    lower_bound_ = lower_bound;
    upper_bound_ = upper_bound;
  }

  VectorNd<7> sample() const
  {
    VectorNd<7> q;
    q[0] = rng.uniformReal(lower_bound[0], upper_bound[0]);
    q[1] = rng.uniformReal(lower_bound[1], upper_bound[1]);
    q[2] = rng.uniformReal(lower_bound[2], upper_bound[2]);

    FCL_REAL s[4];
    rng.quaternion(s);

    q[3] = s[0];
    q[4] = s[1];
    q[5] = s[2];
    q[6] = s[3];
    return q;
  }

protected:
  VectorNd<3> lower_bound;
  VectorNd<3> upper_bound;
};

class SamplerSE3Euler_ball : public SamplerBase
{
public:
  SamplerSE3Euler_ball() {}

  SamplerSE3Euler_ball(FCL_REAL r_) : r(r_)
  {
  }

  void setBound(const FCL_REAL& r_)
  {
    r = r_;
  }
  
  void getBound(FCL_REAL& r_) const
  {
    r_ = r;
  }

  VectorNd<6> sample() const
  {
    VectorNd<6> q;
    FCL_REAL x, y, z;
    rng.ball(0, r, x, y, z);
    q[0] = x;
    q[1] = y;
    q[2] = z;

    FCL_REAL s[4];
    rng.quaternion(s);

    Quaternion3d quat(s[0], s[1], s[2], s[3]);
    Vector3d angles = quat.toRotationMatrix().eulerAngles(0, 1, 2);
    q[3] = angles[0];
    q[4] = angles[1];
    q[5] = angles[2];
    
    return q;
  }

protected:
  FCL_REAL r;

};


class SamplerSE3Quat_ball : public SamplerBase
{
public:
  SamplerSE3Quat_ball() {}

  SamplerSE3Quat_ball(FCL_REAL r_) : r(r_)
  {}

  void setBound(const FCL_REAL& r_)
  {
    r = r_;
  }

  void getBound(FCL_REAL& r_) const
  {
    r_ = r;
  }

  VectorNd<7> sample() const
  {
    VectorNd<7> q;
    FCL_REAL x, y, z;
    rng.ball(0, r, x, y, z);
    q[0] = x;
    q[1] = y;
    q[2] = z;

    FCL_REAL s[4];
    rng.quaternion(s);

    q[3] = s[0];
    q[4] = s[1];
    q[5] = s[2];
    q[6] = s[3];
    return q;
  }

protected:
  FCL_REAL r;
};



}

#endif
