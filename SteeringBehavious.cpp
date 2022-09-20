//
//  SteeringBehaviours.h
//
//  A module to handle steering behaviours.
//

#pragma once

#include "SteeringBehaviours.h"

#include <cassert>
#include <cmath>

#include "GetGlut.h"
#include "ObjLibrary/Vector3.h"

#include "CoordinateSystem.h"
#include "Entity.h"
#include "Spaceship.h"

using namespace ObjLibrary;
namespace
{
	const double AS_FAST_AS_POSSIBLE = 1.0e10;

	const double AHEAD_TIME_FACTOR = 1.0;  // 0.5;

	void drawMarker (const Vector3& position)
	{
		glPushMatrix();
			glTranslated(position.x, position.y, position.z);
			glScaled(3.0, 3.0, 3.0);
			glutWireOctahedron();
		glPopMatrix();
	}
}



double SteeringBehaviours :: getMinArrivalTime (double max_acceleration,
                                                double max_speed_at_impact,
                                                double distance)
{
	assert(max_acceleration    >  0.0);
	assert(max_speed_at_impact >= 0.0);
	assert(distance            >= 0.0);

	//
	//  This is adapted from the quadratic formula
	//
	//          -B  +/-  sqrt(B*B - 4*A*C)
	//  t  =  ------------------------------
	//                     2A
	//
	//  A = 1/2 * acceleration (a)  \
	//  B = max_speed_at_impact (s)  } all always non-negative
	//  C = -distance (d)           /
	//
	//  We require a positive-time answer, so we discard the
	//    negative-square-root solution (always in the past
	//    given that s >= 0).  Simplifying the equation gives:
	//
	//          sqrt(s*s + 2*a*d) - s
	//  t  =  -------------------------
	//                    a
	//

	double under_root = max_speed_at_impact * max_speed_at_impact + 2 * max_acceleration * distance;
	assert(under_root >= 0.0);
	double resolved_root = sqrt(under_root);
	assert(resolved_root >= 0.0);
	assert(resolved_root >= max_speed_at_impact);
	double top = resolved_root - max_speed_at_impact;
	assert(top >= 0.0);
	assert(max_acceleration > 0.0);
	double time = top / max_acceleration;
	assert(time >= 0.0);
	return time;
}

double SteeringBehaviours :: getOptimalSpeedAtDistance (double max_acceleration,
                                                        double max_speed_at_impact,
                                                        double distance)
{
	assert(max_acceleration    >  0.0);
	assert(max_speed_at_impact >= 0.0);
	assert(distance            >= 0.0);

	static const double SAFE_SPEED_FACTOR = 1.0;//  0.5;

	double arrival_time = getMinArrivalTime(max_acceleration, max_speed_at_impact, distance);
	assert(arrival_time >= 0.0);
	double max_speed_here = max_speed_at_impact + max_acceleration * arrival_time;
	double safe_speed_at_distance = max_speed_here * SAFE_SPEED_FACTOR;
	assert(safe_speed_at_distance >= 0.0);
	return safe_speed_at_distance;
}

ObjLibrary::Vector3 SteeringBehaviours :: arrival0 (const ObjLibrary::Vector3& relative_position,
                                                    double max_speed_at_impact,
                                                    double max_acceleration)
{
	assert(max_speed_at_impact >= 0.0);
	assert(max_acceleration    >  0.0);

	double distance = relative_position.getNorm();
	double safe_speed_here = getOptimalSpeedAtDistance(max_acceleration, max_speed_at_impact, distance);
	assert(safe_speed_here >= 0.0);
	return relative_position.getCopyWithNormSafe(safe_speed_here);
}

ObjLibrary::Vector3 SteeringBehaviours :: arrival (const ObjLibrary::Vector3& agent_position,
                                                   const ObjLibrary::Vector3& target_position,
                                                   const ObjLibrary::Vector3& agent_velocity,
                                                   const ObjLibrary::Vector3& target_velocity,
                                                   double max_speed_at_impact,
                                                   double max_acceleration)
{
	assert(max_speed_at_impact >= 0.0);
	assert(max_acceleration    >  0.0);

	Vector3 relative_position = target_position - agent_position;
	Vector3 relative_desired  = arrival0(relative_position, max_speed_at_impact, max_acceleration);
	return relative_desired + target_velocity;  // in pursue and escort in suggested approach
}

ObjLibrary::Vector3 SteeringBehaviours :: pursue (const Spaceship& agent,
                                                  const Entity& target,
                                                  double max_speed_at_impact,
                                                  double max_acceleration)
{
	assert(max_speed_at_impact >= 0.0);
	assert(max_acceleration    >  0.0);

	double current_distance = agent.getPosition().getDistance(target.getPosition());
	double min_time = getMinArrivalTime(max_acceleration, max_speed_at_impact, current_distance);
	double ahead_time = min_time * AHEAD_TIME_FACTOR;

	Vector3 agent_ahead_position  =  agent.getPosition() +  agent.getVelocity() * ahead_time;
	Vector3 escort_ahead_position = target.getPosition() + target.getVelocity() * ahead_time;
	return arrival(agent_ahead_position, escort_ahead_position,
	               agent.getVelocity(),  target.getVelocity(),
	               max_speed_at_impact, max_acceleration);
}

void SteeringBehaviours :: drawPursue (const Spaceship& agent,
                                       const Entity& target,
                                       double max_speed_at_impact,
                                       double max_acceleration,
                                       const ObjLibrary::Vector3& colour)
{
	assert(max_acceleration > 0.0);

	// calculate future target position
	double current_distance = agent.getPosition().getDistance(target.getPosition());
	double min_time = getMinArrivalTime(max_acceleration, max_speed_at_impact, current_distance);
	double ahead_time = min_time * AHEAD_TIME_FACTOR;
	Vector3 target_ahead_position = target.getPosition() + target.getVelocity() * ahead_time;

	// draw positions
	glColor3d(colour.x, colour.y, colour.z);
	drawMarker(target.getPosition());
	drawMarker(target_ahead_position);
}

ObjLibrary::Vector3 SteeringBehaviours :: escort (const Spaceship& agent,
                                                  const Entity& target,
                                                  const ObjLibrary::Vector3& offset,
                                                  double max_acceleration)
{
	assert(max_acceleration > 0.0);

	Vector3 offset_in_world_coords   = target.getCoordinateSystem().localToWorld(offset);
	Vector3 escort_position          = target.getPosition() + offset_in_world_coords;

	double current_distance = agent.getPosition().getDistance(escort_position);
	double min_time = getMinArrivalTime(max_acceleration, 0.0, current_distance);
	double ahead_time = min_time * AHEAD_TIME_FACTOR;

	Vector3 agent_ahead_position  = agent.getPosition() +  agent.getVelocity() * ahead_time;
	Vector3 escort_ahead_position = escort_position     + target.getVelocity() * ahead_time;
	return arrival(agent_ahead_position, escort_ahead_position,
	               agent.getVelocity(),  target.getVelocity(),
	               0.0, max_acceleration);
}

void SteeringBehaviours :: drawEscort (const Spaceship& agent,
                                       const Entity& target,
                                       const ObjLibrary::Vector3& offset,
                                       double max_acceleration,
                                       const ObjLibrary::Vector3& colour)
{
	assert(max_acceleration > 0.0);

	// calculate current escort position
	Vector3 offset_in_world_coords   = target.getCoordinateSystem().localToWorld(offset);
	Vector3 escort_position          = target.getPosition() + offset_in_world_coords;

	// calculate future escort position
	double current_distance = agent.getPosition().getDistance(escort_position);
	double min_time = getMinArrivalTime(max_acceleration, 0.0, current_distance);
	double ahead_time = min_time * AHEAD_TIME_FACTOR;
	Vector3 escort_ahead_position = escort_position + target.getVelocity() * ahead_time;

	// draw positions
	glColor3d(colour.x, colour.y, colour.z);
	drawMarker(escort_position);
	drawMarker(escort_ahead_position);
}



double SteeringBehaviours :: getAvoidDistance (const Spaceship& agent,
                                               const Entity& target,
                                               double max_acceleration)
{
	assert(max_acceleration > 0.0);

	Vector3 relative_velocity = target.getVelocity() - agent.getVelocity();
	return target.getRadius() + (relative_velocity.getNorm() / max_acceleration) * 10.0;
}

bool SteeringBehaviours :: isAvoid (const Spaceship& agent,
                                    const Entity& target,
                                    double max_acceleration)
{
	assert(max_acceleration > 0.0);

	double distance_to_center = agent.getPosition().getDistance(target.getPosition());
	double avoid_distance = getAvoidDistance(agent, target, max_acceleration);

	if(distance_to_center < avoid_distance)
		return true;
	else
		return false;
}

ObjLibrary::Vector3 SteeringBehaviours :: avoid (const Spaceship& agent,
                                                 const Entity& target,
                                                 double max_acceleration,
                                                 double max_delta_speed)
{
	assert(max_acceleration > 0.0);
	assert(max_delta_speed > 0.0);
	assert(isAvoid(agent, target, max_acceleration));

	double distance_to_center = agent.getPosition().getDistance(target.getPosition());
	double avoid_distance = getAvoidDistance(agent, target, max_acceleration);
	assert(distance_to_center < avoid_distance);

	Vector3 away_from_target = agent.getPosition() - target.getPosition();
	return agent.getVelocity() + away_from_target.getCopyWithNormSafe(max_delta_speed);
}

void SteeringBehaviours :: drawAvoid (const Spaceship& agent,
                                      const Entity& target,
                                      double max_acceleration,
                                      const ObjLibrary::Vector3& colour)
{
	assert(max_acceleration > 0.0);
	assert(isAvoid(agent, target, max_acceleration));

	double distance_to_center = agent.getPosition().getDistance(target.getPosition());
	double avoid_distance     = SteeringBehaviours::getAvoidDistance(agent, target, max_acceleration);
	assert(distance_to_center < avoid_distance);

	glPushMatrix();
		glTranslated(target.getPosition().x, target.getPosition().y, target.getPosition().z);
		glColor3d(colour.x, colour.y, colour.z);
		glutWireSphere(avoid_distance, 20, 15);
	glPopMatrix();
}

