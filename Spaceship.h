//
//  Spaceship.h
//
//  A module to represent a spaceship.
//

#pragma once

#include <cassert>
#include <climits>

#include "ObjLibrary/Vector3.h"
#include "ObjLibrary/DisplayList.h"

#include "CoordinateSystem.h"
#include "Entity.h"

class Game;



//
//  Spaceship
//
//  A class to represent a spaceship.  This can be the player
//    ship or a drone.
//
//  Class Invariant:
//    <1> m_acceleration_main      > 0.0
//    <2> m_acceleration_manoeuver > 0.0
//    <3> m_rotation_rate_radians  > 0.0
//
class Spaceship : public Entity
{
public:
//
//  NO_CRYSTAL
//
//  A special constant indicating that this Spaceship is not a
//    drone chasing a crystal.
//
	static const unsigned int NO_CRYSTAL = UINT_MAX;

public:
//
//  Constructor
//
//  Purpose: To create an Spaceship without initializing it.
//  Parameter(s): N/A
//  Preconditions: N/A
//  Returns: N/A
//  Side Effect: A new Spaceship is created.  It is not
//               initialized.
//
	Spaceship ();

//
//  Constructor
//
//  Purpose: To create an Spaceship with the specified position,
//           velocity, mass, and radius.  It will be displayed
//           with the specified DisplayList.
//  Parameter(s):
//    <1> position: The starting position
//    <2> velocity: The starting velocity
//    <3> mass: The mass when the fuel tanks are empty
//    <4> radius: The outer collision radius
//    <5> acceleration_main: The acceleration provided by the
//                           main engine
//    <6> acceleration_manoeuver: The acceleration provided by
//                                the manoeuvering engine
//    <7> rotation_rate_radians: The rotation rate in radians
//                               per second
//    <8> display_list: The DisplayList for this spaceship
//    <9> escort_offset: The offset for this drone to escort the
//                       player spaceship at in the player's
//                       local coordinate system
//  Preconditions:
//    <1> mass                   >  0.0
//    <2> radius                 >= 0.0
//    <3> acceleration_main      >  0.0
//    <4> acceleration_manoeuver >  0.0
//    <5> rotation_rate_radians  >  0.0
//    <6> display_list.isReady()
//  Returns: N/A
//  Side Effect: A new Spaceship is created at position position
//               with velocity velocity.  It has a mass of mass
//               and a radius of radius.
//
	Spaceship (const ObjLibrary::Vector3& position,
	           const ObjLibrary::Vector3& velocity,
	           double mass,
	           double radius,
	           double acceleration_main,
	           double acceleration_manoeuver,
	           double rotation_rate_radians,
	           const ObjLibrary::DisplayList& display_list,
	           const ObjLibrary::Vector3& escort_offset);

	Spaceship (const Spaceship& to_copy) = default;
	~Spaceship () = default;
	Spaceship& operator= (const Spaceship& to_copy) = default;

//
//  isAlive
//
//  Purpose: To determine whether this Spaceship is still alive.
//  Parameter(s): N/A
//  Preconditions:
//    <1> isInitialized()
//  Returns: Whether this Spaceship is alive.
//  Side Effect: N/A
//
	bool isAlive () const
	{
		assert(isInitialized());

		return m_is_alive;
	}

//
//  getCrystalChased
//
//  Purpose: To determine which crystal this Spaceship is
//           chasing.
//  Parameter(s): N/A
//  Preconditions: N/A
//  Returns: What index of the crystal that this drone is
//           chasing.  If this drone is not chasing a crystal,
//           NO_CRYSTAL is returned.
//  Side Effect: N/A
//
	unsigned int getCrystalChased () const
	{
		return m_crystal_chased_index;
	}

//
//  getFollowCameraPosition
//
//  Purpose: To determine the camera position to look past this
//           Spaceship from the specified relative offset.
//  Parameter(s):
//    <1> back_distance: How far behind the spaceship the camera
//                       should be
//    <2> up_distance: How far above the spaceship the camera
//                     should be
//  Preconditions:
//    <1> isInitialized()
//  Returns: The camera position.  The camera is assumed to have
//           the same orientation as this Spaceship.
//  Side Effect: N/A
//
	ObjLibrary::Vector3 getFollowCameraPosition (
	                                  double back_distance,
	                                  double up_distance) const;

//
//  setupFollowCamera
//
//  Purpose: To set up the camera position to look past this
//           Spaceship from the specified relative offset.
//  Parameter(s):
//    <1> back_distance: How far behind the spaceship the camera
//                       should be
//    <2> up_distance: How far above the spaceship the camera
//                     should be
//  Preconditions:
//    <1> isInitialized()
//  Returns: N/A
//  Side Effect: The camera is set up to look in the forward
//               direction of this spaceship from behind and
//               above it.
//
	void setupFollowCamera (double back_distance,
	                        double up_distance) const;

//
//  drawPath
//
//  Purpose: To display the path that this Spaceship will follow
//           if it is only affected by gravity from the
//           specified black hole.
//  Parameter(s):
//    <1> black_hole: The black hole
//    <2> point_count: How many points ahead to display
//    <3> colour: How colour of the path
//  Preconditions:
//    <1> isInitialized()
//  Returns: N/A
//  Side Effect: A path of point_count vertexes is displayed for
//               this Spaceship is displayed.  It will start
//               with a colour of colour and then fade to black
//               at the end.
//
	void drawPath (const Entity& black_hole,
	               unsigned int point_count,
	               const ObjLibrary::Vector3& colour) const;

//
//  drawAI
//
//  Purpose: To display the current AI state for this spaceship.
//  Parameter(s):
//    <1> game: The state of the game
//    <2> colour: The colour for the AI information
//  Preconditions:
//    <1> isInitialized()
//  Returns: N/A
//  Side Effect: If this Spaceship is chasing a crystal, that
//               crystal is marked in colour colour.  If it is
//               escorting the player, it's escort position is
//               marked in colour colour.
//
	void drawAI (const Game& game,
	             const ObjLibrary::Vector3& colour) const;

//
//  markDead
//
//  Purpose: To destroy this Spaceship.  This likely happens as
//           the result of a collision.
//  Parameter(s): N/A
//  Preconditions:
//    <1> isInitialized()
//  Returns: N/A
//  Side Effect: This Spaceship is marked as dead.
//
	void markDead ();

//
//  thrustMainEngine
//
//  Purpose: To fire the main engines of this spaceship, causing
//           it to accelerate quickly forward.
//  Parameter(s):
//    <1> delta_time: The length of the time step in seconds
//  Preconditions:
//    <1> isInitialized()
//    <2> delta_time >= 0.0
//  Returns: N/A
//  Side Effect: This spaceship accelerates forward.
//
	void thrustMainEngine (double delta_time);

//
//  thrustManoeuver
//
//  Purpose: To fire the main engines of this spaceship, causing
//           it to accelerate slowly in the specified direction.
//  Parameter(s):
//    <1> delta_time: The length of the time step in seconds
//    <2> direction_world: The direction to accelerate in world
//                         coordinates
//    <3> strength_fraction: What fraction of maximum power to
//                           accelerate at
//  Preconditions:
//    <1> isInitialized()
//    <2> delta_time >= 0.0
//    <3> direction_world.isUnit()
//    <4> strength_fraction >= 0.0
//    <5> strength_fraction <= 1.0
//  Returns: N/A
//  Side Effect: This spaceship accelerates in direction
//               direction_world at strength_fraction of maximum
//               maneouvering power.
//
	void thrustManoeuver (
	                 double delta_time,
	                 const ObjLibrary::Vector3& direction_world,
	                 double strength_fraction);

//
//  rotateAroundForward
//  rotateAroundUp
//  rotateAroundRight
//
//  Purpose: To rotate this spaceship around the indicated axis.
//  Parameter(s):
//    <1> delta_time: The length of the time step in seconds
//    <2> is_backwards: Whether to rotate backwards
//  Preconditions:
//    <1> isInitialized()
//    <2> delta_time >= 0.0
//  Returns: N/A
//  Side Effect: This spaceship is rotated.
//
	void rotateAroundForward (double delta_time,
	                          bool is_backwards);
	void rotateAroundUp (double delta_time,
	                     bool is_backwards);
	void rotateAroundRight (double delta_time,
	                        bool is_backwards);

//
//  updateAI
//
//  Purpose: To perform the AI updates for this Entity for one
//           time step.
//  Parameter(s):
//    <1> delta_time: The length of the time step in seconds
//    <2> game: The state of the game
//  Preconditions:
//    <1> delta_time > 0.0
//  Returns: N/A
//  Side Effect: The AI for this Spaceship runs for one time
//               step.
//
	void updateAI (double delta_time,
	               Game& game);

private:
//
//  updateCrystalChased
//
//  Purpose: To determine which crystal should be chased.
//  Parameter(s):
//    <1> game: The state of the game
//  Preconditions: N/A
//  Returns: N/A
//  Side Effect: The crystal chased is updated.  If this drone
//               should not chase a crystal, the value is set to
//               NO_CRYSTAL.
//
	void updateCrystalChased (Game& game);

//
//  calculateDesiredVelocity
//
//  Purpose: To determine the desired velocity for this drone.
//  Parameter(s):
//    <1> delta_time: The length of the time step in seconds
//    <2> game: The state of the game
//  Preconditions:
//    <1> delta_time > 0.0
//  Returns: The desired velocity.  If this drone is near an
//           asteroid, this will be away form the asteroid.
//           Otherwise, it will be to intercept the crystal
//           being chased.  If there is no such crystal, it
//           will be to escort the player.
//  Side Effect: N/A
//
	ObjLibrary::Vector3 calculateDesiredVelocity (
	                                          double delta_time,
	                                          Game& game);

//
//  flyToMatchVelocity
//
//  Purpose: To fire the engines of this Spaceship, accelerating
//           it towards the specified velocity.
//  Parameter(s):
//    <1> delta_time: The length of the time step in seconds
//    <2> desired: The velocity to accelerate towards
//  Preconditions:
//    <1> isInitialized()
//    <2> delta_time > 0.0
//  Returns: N/A
//  Side Effect: This Spaceship is accelerated towards velocity
//               desired.  If there is a large difference from
//               the current velocity, this Spaceship is rotated
//               and the main engines are used.  Otherwise, the
//               maneuvering engines are used.
//
	void flyToMatchVelocity (
	                        double delta_time,
	                        const ObjLibrary::Vector3& desired);

//
//  invariant
//
//  Purpose: To determine whether the class invariant is true.
//  Parameter(s): N/A
//  Preconditions: N/A
//  Returns: Whether the class invariant is true.
//  Side Effect: N/A
//
	bool invariant () const;

private:
	bool m_is_alive;
	double m_acceleration_main;
	double m_acceleration_manoeuver;
	double m_rotation_rate_radians;

	// for AI
	ObjLibrary::Vector3 m_escort_offset;
	unsigned int m_crystal_chased_index;
};


