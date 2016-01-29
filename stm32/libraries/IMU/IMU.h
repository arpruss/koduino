/**
 * @authors Avik De <avikde@gmail.com>

  This file is part of koduino <https://github.com/avikde/koduino>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef IMU_h
#define IMU_h

#include <stdint.h>
#include <math.h>

/** @addtogroup IMU IMU drivers and attitude estimation
 *  @{
 */

/**
 * @brief Base IMU class that any IMU hardware should derive from
 */
class IMU {
public:
  /**
   * @brief Read IMU hardware and set acc, gyr in units of m/s^2 and rad/s
   * @details This function must be implemented by derived classes
   */
  virtual void readSensors() = 0;

  float getZ() {
    return acc[2];
  }

  float acc[3];
  float gyr[3];
protected:
  // OrientationFilter *filt;
};

/** @} */ // end of addtogroup

#endif
