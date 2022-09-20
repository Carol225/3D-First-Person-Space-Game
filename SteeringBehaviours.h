//
//  SteeringBehaviours.h
//
//  A module to handle steering behaviours.
//

#pragma once

#include "ObjLibrary/Vector3.h"

class Entity;
class Spaceship;



//
//  SteeringBehaviours
//
//  A namespace to handle steering behaviours for Spaceships.
//
namespace SteeringBehaviours
{
	double getMinArrivalTime (double max_acceleration,
	                          double max_speed_at_impact,
	                          double distance);
	double getOptimalSpeedAtDistance (double max_acceleration,
	                                  double max_speed_at_impact,
	                                  double distance);

	ObjLibrary::Vector3 arrival0 (
	               const ObjLibrary::Vector3& relative_position,
	               double max_speed_at_impact,
	               double max_acceleration);

	ObjLibrary::Vector3 arrival (
	                 const ObjLibrary::Vector3& agent_position,
	                 const ObjLibrary::Vector3& target_position,
	                 const ObjLibrary::Vector3& agent_velocity,
	                 const ObjLibrary::Vector3& target_velocity,
	                 double max_speed_at_impact,
	                 double max_acceleration);

//
//  pursue
//
//  Purpose: To steer the specified Spaceship to pursue the
//           specified Entity.  It will aim at to hit the future
//           position of the target while moving at the
//           specified relative speed.
//  Parameter(s):
//    <1> agent: The spaceship to control
//    <2> target: The target to pursue
//    <3> max_speed_at_impact: The relative speed for spaceship
//                             when it intercepts the target
//    <4> max_acceleration: The maximum spaceship accleration
//  Preconditions:
//    <1> max_speed_at_impact >= 0.0
//    <2> max_acceleration    >  0.0
//  Returns: The desired velocity for agent.
//  Side Effect: N/A
//
	ObjLibrary::Vector3 pursue (const Spaceship& agent,
	                            const Entity& target,
	                            double max_speed_at_impact,
	                            double max_acceleration);

	void drawPursue (const Spaceship& agent,
	                 const Entity& target,
	                 double max_speed_at_impact,
	                 double max_acceleration,
	                 const ObjLibrary::Vector3& colour);

//
//  escort
//
//  Purpose: To steer the specified Spaceship to match velocity
//           with the specified Entity at the specified offset.
//  Parameter(s):
//    <1> agent: The spaceship to control
//    <2> target: The spaceship to escort
//    <3> offset: The offset in target's local coordinate system
//    <4> max_acceleration: The maximum spaceship accleration
//  Preconditions:
//    <1> max_acceleration > 0.0
//  Returns: The desired velocity for agent.
//  Side Effect: N/A
//
	ObjLibrary::Vector3 escort (
	                          const Spaceship& agent,
	                          const Entity& target,
	                          const ObjLibrary::Vector3& offset,
	                          double max_acceleration);

	void drawEscort (const Spaceship& agent,
	                 const Entity& target,
	                 const ObjLibrary::Vector3& offset,
	                 double max_acceleration,
	                 const ObjLibrary::Vector3& colour);

//
//  getAvoidDistance
//
//  Purpose: To determine how far away the specified Spaceship
//           should stay from the center of the specified Entity.
//  Parameter(s):
//    <1> agent: The spaceship to control
//    <2> target: The target to avoid
//    <3> max_acceleration: The maximum spaceship accleration
//  Preconditions:
//    <1> max_acceleration > 0.0
//  Returns: The maximum distance to avoid at.  This is the
//           distance from the center of target's collision
//           sphere, not from the edge.
//  Side Effect: N/A
//
	double getAvoidDistance (const Spaceship& agent,
	                         const Entity& target,
	                         double max_acceleration);

	bool isAvoid (const Spaceship& agent,
	              const Entity& target,
	              double max_acceleration);

//
//  avoid
//
//  Purpose: To steer the specified Spaceship to avoid the
//           specified Entity.
//  Parameter(s):
//    <1> agent: The spaceship to control
//    <2> target: The target to avoid
//    <3> max_acceleration: The maximum spaceship accleration
//    <4> max_delta_speed: The maximum change in velocity
//  Preconditions:
//    <1> max_acceleration > 0.0
//    <2> max_delta_speed  > 0.0
//  Returns: The desired velocity for agent.  If no avoidance is
//           needed, the agents's current velocity is returned.
//  Side Effect: N/A
//
	ObjLibrary::Vector3 avoid (const Spaceship& agent,
	                           const Entity& target,
	                           double max_acceleration,
	                           double max_delta_speed);

	void drawAvoid (const Spaceship& agent,
	                const Entity& target,
	                double max_acceleration,
	                const ObjLibrary::Vector3& colour);

}  // end of namespace SteeringBehaviours
