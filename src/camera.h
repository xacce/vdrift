/************************************************************************/
/*                                                                      */
/* This file is part of VDrift.                                         */
/*                                                                      */
/* VDrift is free software: you can redistribute it and/or modify       */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* VDrift is distributed in the hope that it will be useful,            */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with VDrift.  If not, see <http://www.gnu.org/licenses/>.      */
/*                                                                      */
/************************************************************************/

#ifndef _CAMERA_H
#define _CAMERA_H

#include "mathvector.h"
#include "quaternion.h"
#include "coordinatesystem.h"
#include <string>

///base class for a camera
class CAMERA
{
public:
	CAMERA(const std::string & camera_name) : name(camera_name), fov(0) {}

	virtual ~CAMERA() {}

	const std::string & GetName() const { return name; }

	void SetFOV(float value) { fov = std::max(0.0f, std::min(120.0f, value)); }

	float GetFOV() const { return fov; }

	virtual const MATHVECTOR <float, 3> & GetPosition() const { return position; }

	virtual const QUATERNION <float> & GetOrientation() const { return rotation; }

	// reset position, orientation
	virtual void Reset(const MATHVECTOR <float, 3> & newpos, const QUATERNION <float> & newquat) {};

	// update position, orientation
	virtual void Update(const MATHVECTOR <float, 3> & newpos, const QUATERNION <float> & newquat, float dt) {};

	// move relative to current position, orientation
	virtual void Move(float dx, float dy, float dz) {};

	// rotate relative to current position, orientation
	virtual void Rotate(float up, float left) {};

protected:
	const std::string name;
	MATHVECTOR <float, 3> position;
	QUATERNION <float> rotation;
	float fov;
};

inline float AngleBetween(MATHVECTOR <float, 3> vec1, MATHVECTOR <float, 3> vec2)
{
	float dotprod = vec1.Normalize().dot(vec2.Normalize());
	float angle = acos(dotprod);
	float epsilon = 1e-6;
	if (fabs(dotprod) <= epsilon) angle = M_PI * 0.5;
	if (dotprod >= 1.0-epsilon) angle = 0.0;
	if (dotprod <= -1.0+epsilon) angle = M_PI;
	return angle;
}

inline QUATERNION <float> LookAt(
	MATHVECTOR <float, 3> eye,
	MATHVECTOR <float, 3> center,
	MATHVECTOR <float, 3> up)
{
	QUATERNION <float> rotation;

	MATHVECTOR <float, 3> forward(center - eye);
	forward = forward.Normalize();
	MATHVECTOR <float, 3> side = (forward.cross(up)).Normalize();
	MATHVECTOR <float, 3> realup = side.cross(forward);

	//rotate so the camera is pointing along the forward line
	float theta = AngleBetween(forward, direction::Forward);
	assert(theta == theta);
	if (fabs(theta) > 0.001)
	{
		MATHVECTOR <float, 3> axis = forward.cross(direction::Forward).Normalize();
		rotation.Rotate(-theta, axis[0], axis[1], axis[2]);
	}

	//now rotate the camera so it's pointing up
	MATHVECTOR <float, 3> curup = direction::Up;
	rotation.RotateVector(curup);

	float rollangle = AngleBetween(realup, curup);
	if (curup.dot(side) > 0.0)
	{
		rollangle = 2 * M_PI - rollangle;
	}
	assert(rollangle == rollangle);

	MATHVECTOR <float, 3> axis = forward;
	rotation.Rotate(rollangle, axis[0], axis[1], axis[2]);

	return rotation;
}

#endif // _CAMERA_H
