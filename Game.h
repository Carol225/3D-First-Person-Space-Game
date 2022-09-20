//
//  Game.h
//
//  A module to store the current state of the game.
//

#pragma once

#include <string>
#include <vector>

#include "ObjLibrary/Vector3.h"

#include "CoordinateSystem.h"
#include "Entity.h"
#include "BlackHole.h"
#include "Asteroid.h"
#include "Crystal.h"
#include "Spaceship.h"



//
//  Game
//
//  A class to store the current state of the game.
//
class Game
{
public:
	static bool isModelsLoaded ();
	static void loadModels (const std::string& path);

public:
	Game ();

	Game (const Game& game) = default;
	~Game () = default;
	Game& operator= (const Game& game) = default;

	bool isOver () const
	{  return !m_player.isAlive();  }

	unsigned int getAsteroidCount () const
	{  return mv_asteroids.size();  }
	const Asteroid& getAsteroid (unsigned int index) const;

	unsigned int getTotalCrystalCount () const
	{  return mv_crystals.size();  }
	unsigned int getNonGoneCrystalCount () const;
	const Crystal& getCrystal (unsigned int index) const;
	Crystal& getCrystal (unsigned int index);
	bool isCrystalChased (unsigned int index);

	const Spaceship& getPlayer () const
	{  return m_player;  }
	unsigned int getLivingDroneCount () const;

	unsigned int getCrystalsCollected () const
	{  return m_crystals_collected;  }

	ObjLibrary::Vector3 getFollowCameraPosition () const;
	void setupFollowCamera () const;
	void draw (bool is_show_debug) const;

	void update (double delta_time);
	void knockOffCrystals ();

	void playerMainEngine (double delta_time)
	{  m_player.thrustMainEngine(delta_time);  }

	void playerManoeuverForward (double delta_time)
	{  m_player.thrustManoeuver(delta_time,  m_player.getForward(), 1.0);  }
	void playerManoeuverBackward (double delta_time)
	{  m_player.thrustManoeuver(delta_time, -m_player.getForward(), 1.0);  }
	void playerManoeuverUp (double delta_time)
	{  m_player.thrustManoeuver(delta_time,  m_player.getUp(), 1.0);  }
	void playerManoeuverDown (double delta_time)
	{  m_player.thrustManoeuver(delta_time, -m_player.getUp(), 1.0);  }
	void playerManoeuverRight (double delta_time)
	{  m_player.thrustManoeuver(delta_time,  m_player.getRight(), 1.0);  }
	void playerManoeuverLeft (double delta_time)
	{  m_player.thrustManoeuver(delta_time, -m_player.getRight(), 1.0);  }

	void playerRotateClockwise (double delta_time)
	{  m_player.rotateAroundForward(delta_time, false);  }
	void playerRotateCounterClockwise (double delta_time)
	{  m_player.rotateAroundForward(delta_time, true);  }
	void playerRotateUp (double delta_time)
	{  m_player.rotateAroundRight(delta_time, false);  }
	void playerRotateDown (double delta_time)
	{  m_player.rotateAroundRight(delta_time, true);  }
	void playerRotateRight (double delta_time)
	{  m_player.rotateAroundUp(delta_time, true);  }
	void playerRotateLeft (double delta_time)
	{  m_player.rotateAroundUp(delta_time, false);  }

private:
	void initAsteroids ();
	void initSpaceships ();
	double getCircularOrbitSpeed (double distance);

	void drawSkybox () const;

	void updateAI (double delta_time);
	void updatePhysics (double delta_time);
	void handleCollisions ();

	void addCrystal (const ObjLibrary::Vector3& position,
	                 const ObjLibrary::Vector3& asteroid_velocity);

private:
	BlackHole m_black_hole;
	std::vector<Asteroid> mv_asteroids;
	std::vector<Crystal> mv_crystals;
	Spaceship m_player;
	std::vector<Spaceship> mv_drones;
	unsigned int m_crystals_collected;
};
