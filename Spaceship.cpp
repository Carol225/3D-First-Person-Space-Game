//
//  Spaceship.h
//
//  A module to represent a spaceship.
//

#include "Spaceship.h"

#include <cassert>
#include <climits>
#include <cmath>

#include "GetGlut.h"
#include "ObjLibrary/Vector3.h"
#include "ObjLibrary/DisplayList.h"

#include "CoordinateSystem.h"
#include "SteeringBehaviours.h"
#include "Game.h"
#include "Entity.h"

using namespace ObjLibrary;
namespace
{
	const double FAR_AWAY = 1.0e6;

	const double INTERCEPT_SPEED = 100.0;
}


const unsigned int Spaceship :: NO_CRYSTAL;  // value is set in header file



Spaceship :: Spaceship ()
		: Entity()
		, m_is_alive(false)
		, m_acceleration_main(1.0)
		, m_acceleration_manoeuver(1.0)
		, m_rotation_rate_radians(1.0)
		, m_escort_offset(Vector3(1.0, 0.0, 0.0))
		, m_crystal_chased_index(NO_CRYSTAL)
{
	assert(!isInitialized());
	assert(invariant());
}

Spaceship :: Spaceship (const ObjLibrary::Vector3& position,
                        const ObjLibrary::Vector3& velocity,
                        double mass,
                        double radius,
                        double acceleration_main,
                        double acceleration_manoeuver,
                        double rotation_rate_radians,
                        const ObjLibrary::DisplayList& display_list,
                        const ObjLibrary::Vector3& escort_offset)
		: Entity(position, velocity, mass, radius, display_list, radius)
		, m_is_alive(true)
		, m_acceleration_main(acceleration_main)
		, m_acceleration_manoeuver(acceleration_manoeuver)
		, m_rotation_rate_radians(rotation_rate_radians)
		, m_escort_offset(escort_offset)
		, m_crystal_chased_index(NO_CRYSTAL)
{
	assert(mass                   >  0.0);
	assert(radius                 >= 0.0);
	assert(acceleration_main      >  0.0);
	assert(acceleration_manoeuver >  0.0);
	assert(rotation_rate_radians  >  0.0);
	assert(display_list.isReady());

	assert(isInitialized());
	assert(invariant());
}



Vector3 Spaceship :: getFollowCameraPosition (double back_distance,
                                              double up_distance) const
{
	assert(isInitialized());

	CoordinateSystem camera = m_coords;
	camera.addPosition(camera.getForward() * -back_distance);
	camera.addPosition(camera.getUp()      *  up_distance);
	return camera.getPosition();
}

void Spaceship :: setupFollowCamera (double back_distance,
                                     double up_distance) const
{
	assert(isInitialized());

	CoordinateSystem camera = m_coords;
	camera.addPosition(camera.getForward() * -back_distance);
	camera.addPosition(camera.getUp() * up_distance);
	camera.setupCamera();
}

void Spaceship :: drawPath (const Entity& black_hole,
                            unsigned int point_count,
                            const ObjLibrary::Vector3& colour) const
{
	assert(isInitialized());

	Spaceship future = *this;

	glBegin(GL_LINE_STRIP);
		glColor3d(colour.x, colour.y, colour.z);
		glVertex3d(future.getPosition().x, future.getPosition().y, future.getPosition().z);

		for(unsigned int i = 1; i < point_count; i++)
		{
			double distance   = black_hole.getPosition().getDistance(future.getPosition());
			double delta_time = sqrt(distance) / 25.0;

			future.updatePhysics(delta_time, black_hole);

			double fraction = sqrt(1.0 - (double)(i) / point_count);
			glColor3d(colour.x * fraction, colour.y * fraction, colour.z * fraction);
			glVertex3d(future.getPosition().x, future.getPosition().y, future.getPosition().z);
		}
	glEnd();
}

void Spaceship :: drawAI (const Game& game,
                          const ObjLibrary::Vector3& colour) const
{
	// draw avoiding asteroid (if applicable)
	for(unsigned int a = 0; a < game.getAsteroidCount(); a++)
		if(SteeringBehaviours::isAvoid(*this, game.getAsteroid(a), m_acceleration_manoeuver))
		{
			SteeringBehaviours::drawAvoid(*this, game.getAsteroid(a), m_acceleration_manoeuver, colour);
			return;  // don't draw anything else
		}

	if(m_crystal_chased_index < game.getTotalCrystalCount())
	{
		const Crystal& crystal = game.getCrystal(m_crystal_chased_index);
		SteeringBehaviours::drawPursue(*this, crystal, INTERCEPT_SPEED, m_acceleration_manoeuver, colour);
	}
	else
		SteeringBehaviours::drawEscort(*this, game.getPlayer(), m_escort_offset, m_acceleration_manoeuver, colour);
}



void Spaceship :: markDead ()
{
	m_is_alive = false;

	assert(invariant());
}

void Spaceship :: thrustMainEngine (double delta_time)
{
	assert(isInitialized());
	assert(delta_time >= 0.0);

	assert(m_coords.getForward().isUnit());
	m_velocity += m_coords.getForward() * m_acceleration_main * delta_time;

	assert(invariant());
}

void Spaceship :: thrustManoeuver (double delta_time,
                                   const Vector3& direction_world,
                                   double strength_fraction)
{
	assert(isInitialized());
	assert(delta_time >= 0.0);
	assert(direction_world.isUnit());
	assert(strength_fraction >= 0.0);
	assert(strength_fraction <= 1.0);

	double delta_velocity = m_acceleration_manoeuver * delta_time * strength_fraction;
	m_velocity += direction_world * delta_velocity;

	assert(invariant());
}

void Spaceship :: rotateAroundForward (double delta_time,
                                       bool is_backwards)
{
	assert(delta_time >= 0.0);

	double max_radians = m_rotation_rate_radians * delta_time;
	if(is_backwards)
		m_coords.rotateAroundForward(-max_radians);
	else
		m_coords.rotateAroundForward(max_radians);

	assert(invariant());
}

void Spaceship :: rotateAroundUp (double delta_time,
                                  bool is_backwards)
{
	assert(delta_time >= 0.0);

	double max_radians = m_rotation_rate_radians * delta_time;
	if(is_backwards)
		m_coords.rotateAroundUp(-max_radians);
	else
		m_coords.rotateAroundUp(max_radians);

	assert(invariant());
}

void Spaceship :: rotateAroundRight (double delta_time,
                                     bool is_backwards)
{
	assert(delta_time >= 0.0);

	double max_radians = m_rotation_rate_radians * delta_time;
	if(is_backwards)
		m_coords.rotateAroundRight(-max_radians);
	else
		m_coords.rotateAroundRight(max_radians);

	assert(invariant());
}

void Spaceship :: updateAI (double delta_time,
                            Game& game)
{
	assert(delta_time > 0.0);

	updateCrystalChased(game);
	Vector3 desired = calculateDesiredVelocity(delta_time, game);
	flyToMatchVelocity(delta_time, desired);

	assert(invariant());
}



void Spaceship :: updateCrystalChased (Game& game)
{
	static const double DETECT_CRYSTALS_DISTANCE = 1000.0;

	// abandon current a crystal if it is gone
	if(m_crystal_chased_index < game.getTotalCrystalCount())
	{
		if(game.getCrystal(m_crystal_chased_index).isGone())
			m_crystal_chased_index = NO_CRYSTAL;
	}
	else
		m_crystal_chased_index = NO_CRYSTAL;

	// choose a crystal if possible
	if(m_crystal_chased_index == NO_CRYSTAL)
	{
		unsigned int closest_index    = NO_CRYSTAL;
		double       closest_distance = FAR_AWAY;

		for(unsigned int c = 0; c < game.getTotalCrystalCount(); c++)
		{
			const Crystal& crystal = game.getCrystal(c);
			if(!crystal.isGone())
				if(!game.isCrystalChased(c))
				{
					double distance = crystal.getPosition().getDistance(getPosition());
					if(distance < DETECT_CRYSTALS_DISTANCE &&
					   distance < closest_distance)
					{
						closest_index    = c;
						closest_distance = distance;
					}
				}
		}

		if(closest_index != NO_CRYSTAL)
			m_crystal_chased_index = closest_index;
	}
}

ObjLibrary::Vector3 Spaceship :: calculateDesiredVelocity (double delta_time,
                                                           Game& game)
{
	double avoid_delta_speed = m_acceleration_manoeuver * delta_time;
	for(unsigned int a = 0; a < game.getAsteroidCount(); a++)
		if(SteeringBehaviours::isAvoid(*this, game.getAsteroid(a), m_acceleration_manoeuver))
			return SteeringBehaviours::avoid(*this, game.getAsteroid(a), m_acceleration_manoeuver, avoid_delta_speed);

	// not avoiding anything
	if(m_crystal_chased_index < game.getTotalCrystalCount())
	{
		const Crystal& crystal = game.getCrystal(m_crystal_chased_index);
		assert(!crystal.isGone());
		return SteeringBehaviours::pursue(*this, crystal, INTERCEPT_SPEED, m_acceleration_manoeuver);
	}
	else
		return SteeringBehaviours::escort(*this, game.getPlayer(), m_escort_offset, m_acceleration_manoeuver);
}

void Spaceship :: flyToMatchVelocity (double delta_time,
                                      const ObjLibrary::Vector3& desired)
{
	assert(delta_time > 0.0);

	static const double MAIN_ENGINE_MIN_CHANGE  = 50.0;  // m/s
	static const double MAIN_ENGINE_MIN_RADIANS =  0.1;

	const Vector3& current = getVelocity();
	Vector3 relative = desired - current;

	if(relative.isZero())
	{
		// do nothing, course is already perfect
	}
	else if(relative.isNormLessThan(MAIN_ENGINE_MIN_CHANGE))
	{
		// fire maneuvering engines
		double max_delta_velocity = m_acceleration_manoeuver * delta_time;
		double max_change = relative.getNorm() * 0.2;
		double fraction = max_change / max_delta_velocity;
		if(fraction > 1.0)
			fraction = 1.0;

		thrustManoeuver(delta_time, relative.getNormalized(), fraction);
	}
	else if(relative.getAngleSafe(m_coords.getForward()) > MAIN_ENGINE_MIN_RADIANS)
	{
		// rotate to line up main engines
		double max_radians = m_rotation_rate_radians * delta_time;
		m_coords.rotateToVector(relative, max_radians);
	}
	else
	{
		// fire main engines
		thrustMainEngine(delta_time);
	}

	assert(invariant());
}

bool Spaceship :: invariant () const
{
	if(m_acceleration_main      <= 0.0) return false;
	if(m_acceleration_manoeuver <= 0.0) return false;
	if(m_rotation_rate_radians  <= 0.0) return false;
	return true;
}

