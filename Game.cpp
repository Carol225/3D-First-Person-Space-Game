//
//  Game.cpp
//

#include "Game.h"

#include <cassert>
#include <climits>
#include <vector>
#include <algorithm>  // for min/max

#include "GetGlut.h"
#include "ObjLibrary/Vector3.h"
#include "ObjLibrary/ObjModel.h"
#include "ObjLibrary/DisplayList.h"

#include "Gravity.h"
#include "CoordinateSystem.h"
#include "Entity.h"
#include "BlackHole.h"
#include "Asteroid.h"
#include "Crystal.h"
#include "Spaceship.h"
#include "Collisions.h"

using namespace std;
using namespace ObjLibrary;

namespace
{
	const double TWO_PI  = 6.283185307179586476925286766559;

	const unsigned int ASTEROID_COUNT = 100;
	const unsigned int DRONE_COUNT    = 5;

	const double BLACK_HOLE_RADIUS  =    50.0;
	const double DISK_RADIUS        = 10000.0;
	const double PLAYER_RADIUS      =     4.0;
	const double DRONE_RADIUS       =     2.0;
	const double DEBUG_MAX_DISTANCE =  2000.0;

	static const double BLACK_HOLE_MASS = 5.0e16;  // kg
	static const double PLAYER_MASS     = 1000.0;  // kg
	static const double DRONE_MASS      =  100.0;  // kg

	DisplayList g_skybox_display_list;
	DisplayList g_disk_display_list;
	DisplayList g_crystal_display_list;
	DisplayList g_player_display_list;
	DisplayList ga_drone_display_lists[DRONE_COUNT];

	static const unsigned int ASTEROID_MODEL_COUNT = 25;
	ObjModel ga_asteroid_models[ASTEROID_MODEL_COUNT];

	const double CRYSTAL_KNOCK_OFF_RANGE = 200.0;
	const unsigned int CRYSTAL_KNOCK_OFF_COUNT = 10;
	const double CRYSTAL_KNOCK_OFF_SPEED = 10.0;

	const double  CAMERA_BACK_DISTANCE  =   20.0;
	const double  CAMERA_UP_DISTANCE    =    5.0;
	const double  PLAYER_START_DISTANCE = 1000.0;
	const Vector3 PLAYER_START_FORWARD(1.0, 0.0, 0.0);



	double random01 ()
	{
		return rand() / (RAND_MAX + 1.0);
	}

	double random2 (double min_value, double max_value)
	{
		assert(min_value <= max_value);

		return min_value + random01() * (max_value - min_value);
	}

}  // end of anonymous namespace



bool Game :: isModelsLoaded ()
{
	return g_skybox_display_list.isReady();
}

void Game :: loadModels (const std::string& path)
{
	assert(!isModelsLoaded());

	assert(DRONE_COUNT == 5);
	static const string DRONE_MATERIAL[DRONE_COUNT] =
	{
		"grapple_body_red",
		"grapple_body_orange",
		"grapple_body_yellow",
		"grapple_body_green",
		"grapple_body_cyan",
	};

	g_skybox_display_list  = ObjModel(path + "Skybox.obj")     .getDisplayList();
	g_disk_display_list    = ObjModel(path + "Disk.obj")       .getDisplayList();
	g_crystal_display_list = ObjModel(path + "Crystal.obj")    .getDisplayList();
	g_player_display_list  = ObjModel(path + "Sagittarius.obj").getDisplayList();

	assert(ASTEROID_MODEL_COUNT <= 26);  // only 26 letters to use
	for(unsigned m = 0; m < ASTEROID_MODEL_COUNT; m++)
	{
		string filename = "AsteroidA.obj";
		assert(filename[8] == 'A');
		filename[8] = 'A' + m;
		ga_asteroid_models[m].load(path + filename);
	}

	ObjModel drone_model = ObjModel(path + "Grapple.obj");
	for(unsigned d = 0; d < DRONE_COUNT; d++)
		ga_drone_display_lists[d] = drone_model.getDisplayListMaterial(DRONE_MATERIAL[d]);

	assert(isModelsLoaded());
}




Game :: Game ()
		: m_black_hole(Vector3::ZERO, BLACK_HOLE_MASS,
		               BLACK_HOLE_RADIUS, DISK_RADIUS, g_disk_display_list)
		, mv_asteroids()  // initialized below
		, mv_crystals()   // starts empty
		, m_player()      // initialized below
		, mv_drones()     // initialized below
		, m_crystals_collected(0)
{
	assert(isModelsLoaded());

	initAsteroids();
	initSpaceships();
}



const Asteroid& Game :: getAsteroid (unsigned int index) const
{
	assert(index < getAsteroidCount());

	return mv_asteroids[index];
}

unsigned int Game :: getNonGoneCrystalCount () const
{
	unsigned int count = 0;
	for(unsigned c = 0; c < mv_crystals.size(); c++)
		if(!mv_crystals[c].isGone())
			count++;
	return count;
}

const Crystal& Game :: getCrystal (unsigned int index) const
{
	assert(index < getTotalCrystalCount());

	return mv_crystals[index];
}

Crystal& Game :: getCrystal (unsigned int index)
{
	assert(index < getTotalCrystalCount());

	return mv_crystals[index];
}

bool Game :: isCrystalChased (unsigned int index)
{
	assert(index < getTotalCrystalCount());

	for(unsigned int d = 0; d < mv_drones.size(); d++)
	{
		const Spaceship& drone = mv_drones[d];
		if(drone.isAlive())
			if(drone.getCrystalChased() == index)
				return true;
	}
	return false;
}

unsigned int Game :: getLivingDroneCount () const
{
	unsigned int count = 0;
	for(unsigned int d = 0; d < mv_drones.size(); d++)
		if(mv_drones[d].isAlive())
			count++;
	return count;
}

ObjLibrary::Vector3 Game :: getFollowCameraPosition () const
{
	return m_player.getFollowCameraPosition(CAMERA_BACK_DISTANCE, CAMERA_UP_DISTANCE);
}

void Game :: setupFollowCamera () const
{
	m_player.setupFollowCamera(CAMERA_BACK_DISTANCE, CAMERA_UP_DISTANCE);
}

void Game :: draw (bool is_show_debug) const
{
	static const Vector3 PLAYER_COLOUR(0.0, 0.0, 1.0);

	assert(DRONE_COUNT == 5);
	static const Vector3 DRONE_AI_COLOUR[DRONE_COUNT] =
	{
		Vector3(1.0, 0.0, 0.0),
		Vector3(1.0, 0.5, 0.0),
		Vector3(1.0, 1.0, 0.0),
		Vector3(0.0, 1.0, 0.0),
		Vector3(0.0, 1.0, 1.0),
	};

	setupFollowCamera();
	drawSkybox();  // has to be first

	const Vector3& player_position = m_player.getPosition();
	for(unsigned a = 0; a < mv_asteroids.size(); a++)
	{
		const Asteroid& asteroid = mv_asteroids[a];
		asteroid.draw();

		if(is_show_debug)
		{
			asteroid.drawAxes(asteroid.getRadius() + 50.0);
			if(asteroid.getPosition().isDistanceLessThan(player_position, DEBUG_MAX_DISTANCE))
				asteroid.drawSurfaceEquators();
		}
	}

	for(unsigned c = 0; c < mv_crystals.size(); c++)
	{
		const Crystal& crystal = mv_crystals[c];
		if(!crystal.isGone())
			crystal.draw();
	}

	if(m_player.isAlive())
	{
		m_player.draw();
		m_player.drawPath(m_black_hole, 1000, PLAYER_COLOUR);
	}

	for(unsigned int d = 0; d < mv_drones.size(); d++)
	{
		assert(d < DRONE_COUNT);

		const Spaceship& drone = mv_drones[d];
		if(drone.isAlive())
		{
			drone.draw();
			drone.drawPath(m_black_hole, 100, DRONE_AI_COLOUR[d]);
			if(is_show_debug)
				drone.drawAI(*this, DRONE_AI_COLOUR[d]);
		}
	}

	m_black_hole.draw();  // must be last
}



void Game :: update (double delta_time)
{
	updateAI(delta_time);
	updatePhysics(delta_time);
	handleCollisions();
}

void Game :: knockOffCrystals ()
{
	const Vector3& player_position = m_player.getPosition();

	for(unsigned a = 0; a < mv_asteroids.size(); a++)
	{
		Asteroid& asteroid = mv_asteroids[a];
		if(asteroid.isCrystals())
		{
			Vector3 asteroid_position  = asteroid.getPosition();
			Vector3 asteroid_to_player = player_position - asteroid_position;
			double asteroid_radius = asteroid.getRadiusForDirection(asteroid_to_player.getNormalized());
			double maximum_distance = asteroid_radius + CRYSTAL_KNOCK_OFF_RANGE;

			if(asteroid_to_player.isNormLessThan(maximum_distance))
			{
				Vector3 knock_off_position = asteroid_position + asteroid_to_player.getCopyWithNorm(asteroid_radius);
				for(unsigned c = 0; c < CRYSTAL_KNOCK_OFF_COUNT; c++)
					addCrystal(knock_off_position, asteroid.getVelocity());
				asteroid.removeCrystals();
			}
		}
	}
}



void Game :: initAsteroids ()
{
	static const double DISTANCE_MIN = DISK_RADIUS * 0.2;
	static const double DISTANCE_MAX = DISK_RADIUS * 0.8;

	static const double SPEED_FACTOR_MIN = 0.5;
	static const double SPEED_FACTOR_MAX = 1.5;

	static const double OUTER_RADIUS_MIN =  50.0;
	static const double OUTER_RADIUS_MAX = 400.0;
	static const double INNER_FRACTION_MIN = 0.1;
	static const double INNER_FRACTION_MAX = 0.5;

	static const double  COLLISION_AHEAD_DISTANCE  = 1500.0;
	static const double  COLLISION_HALF_SEPERATION =  500.0;
	static const Vector3 COLLISION_POSITION_1(COLLISION_AHEAD_DISTANCE, PLAYER_START_DISTANCE,  COLLISION_HALF_SEPERATION);
	static const Vector3 COLLISION_POSITION_2(COLLISION_AHEAD_DISTANCE, PLAYER_START_DISTANCE, -COLLISION_HALF_SEPERATION);

	// create 2 asteroids to collide in front of player
	double collider_speed1 = getCircularOrbitSpeed(COLLISION_POSITION_1.getNorm()) * 0.9;
	double collider_speed2 = getCircularOrbitSpeed(COLLISION_POSITION_2.getNorm()) * 1.1;
	Vector3 collider_velocity1 = Vector3(0.0, 0.0, -collider_speed1);
	Vector3 collider_velocity2 = Vector3(0.0, 0.0,  collider_speed2);
	double collider_inner_radius1 = OUTER_RADIUS_MAX * INNER_FRACTION_MIN;
	double collider_inner_radius2 = OUTER_RADIUS_MIN * INNER_FRACTION_MAX;

	assert(1 < ASTEROID_MODEL_COUNT);
	assert(!ga_asteroid_models[0].isEmpty());
	assert(!ga_asteroid_models[1].isEmpty());
	mv_asteroids.push_back(Asteroid(COLLISION_POSITION_1, collider_velocity1,
	                                collider_inner_radius1, OUTER_RADIUS_MAX,
	                                ga_asteroid_models[0]));
	mv_asteroids.push_back(Asteroid(COLLISION_POSITION_2, collider_velocity2,
	                                collider_inner_radius2, OUTER_RADIUS_MIN,
	                                ga_asteroid_models[1]));

	// create remaining asteroids
	for(unsigned a = 2; a < ASTEROID_COUNT; a++)
	{
		// choose a random position in a thick shell around the black hole
		double distance = random2(DISTANCE_MIN, DISTANCE_MAX);
		Vector3 position = Vector3::getRandomUnitVector() * distance;

		// choose starting velocity
		double speed_circle = getCircularOrbitSpeed(distance);
		double speed_factor = random2(SPEED_FACTOR_MIN, SPEED_FACTOR_MAX);
		double speed = speed_circle * speed_factor;
		Vector3 velocity = Vector3::getRandomUnitVector().getRejection(position);  // tangent to gravity
		assert(!velocity.isZero());
		velocity.setNorm(speed);

		// mostly smaller asteroids
		double outer_radius = min(random2(OUTER_RADIUS_MIN, OUTER_RADIUS_MAX),
		                          random2(OUTER_RADIUS_MIN, OUTER_RADIUS_MAX));

		double inner_fraction = random2(INNER_FRACTION_MIN, INNER_FRACTION_MAX);
		double inner_radius   = outer_radius * inner_fraction;

		unsigned int model_index = a % ASTEROID_MODEL_COUNT;
		assert(model_index < ASTEROID_MODEL_COUNT);
		assert(!ga_asteroid_models[model_index].isEmpty());

		mv_asteroids.push_back(Asteroid(position, velocity,
		                                inner_radius, outer_radius,
		                                ga_asteroid_models[model_index]));
	}
	assert(mv_asteroids.size() == ASTEROID_COUNT);
}

void Game :: initSpaceships ()
{
	const double PLAYER_FORWARD_POWER  = 500.0;  // m/s^2
	const double PLAYER_MANEUVER_POWER =  50.0;  // m/s^2
	const double PLAYER_ROTATION_RATE  =   3.0;  // radians / second

	const double DRONE_FORWARD_POWER  = 250.0;  // m/s^2
	const double DRONE_MANEUVER_POWER =  25.0;  // m/s^2
	const double DRONE_ROTATION_RATE  =   1.0;  // radians / second
	static const Vector3 DRONE_OFFSET_BASE(0.0, 10.0, 0.0);

	double  player_speed    = getCircularOrbitSpeed(PLAYER_START_DISTANCE);
	Vector3 player_position(0.0, PLAYER_START_DISTANCE, 0.0);
	Vector3 player_velocity = PLAYER_START_FORWARD * player_speed;

	assert(g_player_display_list.isReady());
	m_player = Spaceship(player_position, player_velocity,
	                     PLAYER_MASS, PLAYER_RADIUS,
	                     PLAYER_FORWARD_POWER, PLAYER_MANEUVER_POWER, PLAYER_ROTATION_RATE,
	                     g_player_display_list, Vector3::ZERO);

	for(unsigned int d = 0; d < DRONE_COUNT; d++)
	{
		double radians = d * TWO_PI / DRONE_COUNT;
		Vector3 drone_offset = DRONE_OFFSET_BASE.getRotatedX(radians);
		Vector3 drone_position = player_position + drone_offset;

		assert(ga_drone_display_lists[d].isReady());
		mv_drones.push_back(Spaceship(drone_position, player_velocity,
		                              DRONE_MASS, DRONE_RADIUS,
		                              DRONE_FORWARD_POWER, DRONE_MANEUVER_POWER, DRONE_ROTATION_RATE,
		                              ga_drone_display_lists[d], drone_offset));
	}
}

double Game :: getCircularOrbitSpeed (double distance)
{
	assert(distance > 0.0);

	return sqrt(GRAVITY * m_black_hole.getMass() / distance);
}



void Game :: drawSkybox () const
{
	glPushMatrix();
		Vector3 camera = getFollowCameraPosition();
		glTranslated(camera.x, camera.y, camera.z);
		glRotated(90.0, 0.0, 0.0, 1.0);  // line band of clouds on skybox up with accretion disk
		glScaled(5000.0, 5000.0, 5000.0);

		glDepthMask(GL_FALSE);
		g_skybox_display_list.draw();
		glDepthMask(GL_TRUE);
	glPopMatrix();
}



void Game :: updateAI (double delta_time)
{
	for(unsigned int d = 0; d < mv_drones.size(); d++)
	{
		Spaceship& drone = mv_drones[d];
		if(drone.isAlive())
			drone.updateAI(delta_time, *this);
	}
}

void Game :: updatePhysics (double delta_time)
{
	for(unsigned a = 0; a < mv_asteroids.size(); a++)
		mv_asteroids[a].updatePhysics(delta_time, m_black_hole);

	for(unsigned c = 0; c < mv_crystals.size(); c++)
		if(!mv_crystals[c].isGone())
			mv_crystals[c].updatePhysics(delta_time, m_black_hole);

	if(m_player.isAlive())
		m_player.updatePhysics(delta_time, m_black_hole);

	for(unsigned int d = 0; d < mv_drones.size(); d++)
	{
		Spaceship& drone = mv_drones[d];
		if(drone.isAlive())
			drone.updatePhysics(delta_time, m_black_hole);
	}
}

void Game :: handleCollisions ()
{
/*
	if(Collisions::isCollision(m_player, m_black_hole))
		m_player.markDead();
	for(unsigned int d = 0; d < mv_drones.size(); d++)
	{
		Spaceship& drone = mv_drones[d];
		if(Collisions::isCollision(drone, m_black_hole))
			drone.markDead();
	}
*/
	for(unsigned c = 0; c < mv_crystals.size(); c++)
	{
		Crystal& crystal = mv_crystals[c];
		if(!crystal.isGone())
		{
			//if(Collisions::isCollision(m_black_hole, crystal))
			//	crystal.markGone();
			//else
			if(Collisions::isCollision(m_player, crystal))
			{
				assert(!crystal.isGone());
				crystal.markGone();
				m_crystals_collected++;
			}
			else
			{
				for(unsigned int d = 0; d < mv_drones.size(); d++)
					if(Collisions::isCollision(mv_drones[d], crystal))
					{
						assert(!crystal.isGone());
						crystal.markGone();
						m_crystals_collected++;
						break;  // don't check any more drones
					}
			}
		}
	}

	for(unsigned a = 0; a < mv_asteroids.size(); a++)
	{
		Asteroid& asteroid = mv_asteroids[a];

		for(unsigned a2 = a + 1; a2 < mv_asteroids.size(); a2++)
		{
			Asteroid& asteroid2 = mv_asteroids[a2];
			if(Collisions::isCollision(asteroid, asteroid2))
				Collisions::elastic(asteroid, asteroid2);
		}

		for(unsigned c = 0; c < mv_crystals.size(); c++)
		{
			Crystal& crystal = mv_crystals[c];
			if(!crystal.isGone())
				if(Collisions::isCollision(crystal, asteroid))
				{
					Collisions::elastic(crystal, asteroid);
					//Collisions::bounceOff(crystal, asteroid);  // does about the same thing
				}
		}

		if(Collisions::isCollision(m_player, asteroid))
			m_player.markDead();
		for(unsigned int d = 0; d < mv_drones.size(); d++)
		{
			Spaceship& drone = mv_drones[d];
			if(Collisions::isCollision(drone, asteroid))
				drone.markDead();
		}
	}
}

void Game :: addCrystal (const ObjLibrary::Vector3& position,
                         const ObjLibrary::Vector3& asteroid_velocity)
{
	static const unsigned int EXPAND_VECTOR = UINT_MAX;

	Vector3 crystal_velocity = asteroid_velocity + Vector3::getRandomUnitVector() * CRYSTAL_KNOCK_OFF_SPEED;

	// look for an existing already-gone crystal to replace
	//  -> this will speed up searching the list elsewhere
	unsigned int index = EXPAND_VECTOR;
	for(unsigned c = 0; c < mv_crystals.size(); c++)
		if(mv_crystals[c].isGone())
		{
			index = c;
			break;
		}

	assert(g_crystal_display_list.isReady());
	if(index == EXPAND_VECTOR)
		mv_crystals.push_back(Crystal(position, crystal_velocity, g_crystal_display_list));
	else
		mv_crystals[index] =  Crystal(position, crystal_velocity, g_crystal_display_list);
}
