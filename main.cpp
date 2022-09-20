//
//  main.cpp
//

#include <cassert>
#include <cctype>  // for toupper
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>  // for min/max
#include <chrono>

#include "GetGlut.h"
#include "Sleep.h"

#include "ObjLibrary/Vector3.h"
#include "ObjLibrary/ObjModel.h"
#include "ObjLibrary/DisplayList.h"
#include "ObjLibrary/SpriteFont.h"

#include "PerlinNoiseField3.h"
#include "SteeringBehaviours.h"
#include "Game.h"

using namespace std;
using namespace chrono;
using namespace ObjLibrary;

void initDisplay ();
void initTime ();
void printOptimalSpeedData ();

unsigned char fixShift (unsigned char key);
void keyboardDown (unsigned char key, int x, int y);
void keyboardUp (unsigned char key, int x, int y);
void specialDown (int special_key, int x, int y);
void specialUp (int special_key, int x, int y);

void update ();
void handleInput (double delta_time);

void reshape (int w, int h);
void display ();
void drawOverlays ();

namespace
{
	int window_width  = 640;
	int window_height = 480;
	SpriteFont font;

	const unsigned int KEY_PRESSED_COUNT = 0x100 + 5;
	const unsigned int KEY_PRESSED_RIGHT = 0x100 + 0;
	const unsigned int KEY_PRESSED_LEFT  = 0x100 + 1;
	const unsigned int KEY_PRESSED_UP    = 0x100 + 2;
	const unsigned int KEY_PRESSED_DOWN  = 0x100 + 3;
	const unsigned int KEY_PRESSED_END   = 0x100 + 4;
	bool key_pressed[KEY_PRESSED_COUNT];

	const int PHYSICS_PER_SECOND = 60;
	const double SECONDS_PER_PHYSICS = 1.0 / PHYSICS_PER_SECOND;
	const microseconds PHYSICS_MICROSECONDS(1000000 / PHYSICS_PER_SECOND);
	const unsigned int MAXIMUM_UPDATES_PER_FRAME = 10;
	const unsigned int FAST_PHYSICS_FACTOR = 10;
	const double SIMULATE_SLOW_SECONDS = 0.05;

	system_clock::time_point next_update_time;
	const unsigned int SMOOTH_RATE_COUNT = MAXIMUM_UPDATES_PER_FRAME * 2 + 2;
	system_clock::time_point old_frame_times [SMOOTH_RATE_COUNT];
	system_clock::time_point old_update_times[SMOOTH_RATE_COUNT];
	unsigned int next_old_update_index = 0;
	unsigned int next_old_frame_index  = 0;

	bool g_is_paused     = false;
	bool g_is_show_debug = false;

	Game* gp_game = nullptr;

}  // end of anonymous namespace



int main (int argc, char* argv[])
{
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(0, 0);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
	glutCreateWindow("CS 409 Assignment 5 Solution");
	glutKeyboardFunc(keyboardDown);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(specialDown);
	glutSpecialUpFunc(specialUp);
	glutIdleFunc(update);
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);

	//PerlinNoiseField3 pnf;
	//pnf.printPerlin(40, 60, 0.1f);
	//printOptimalSpeedData();

	// change this to an absolute path on Mac computers
	string path = "Models/";
	font.load(path + "Font.bmp");
	Game::loadModels(path);

	initDisplay();
	gp_game = new Game();
	initTime();  // should be last

	glutMainLoop();

	return 1;
}

void initDisplay ()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glColor3f(0.0, 0.0, 0.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);  // additive blending

	glutPostRedisplay();
}

void initTime ()
{
	system_clock::time_point start_time = system_clock::now();
	next_update_time = start_time;

	for(unsigned int i = 1; i < SMOOTH_RATE_COUNT; i++)
	{
		unsigned int steps_back = SMOOTH_RATE_COUNT - i;
		old_update_times[i] = start_time - PHYSICS_MICROSECONDS * steps_back;
		old_frame_times [i] = start_time - PHYSICS_MICROSECONDS * steps_back;
	}
}

void printOptimalSpeedData ()
{
	double a = 25;
	cout << "a\ts0\td\tt\tst" << endl;
	for(unsigned int s0 = 1; s0 < 1000; s0 *= 2)
		for(unsigned int d = 1; d < 1100; d *= 2)
		{
			double t  = SteeringBehaviours::getMinArrivalTime        (a, s0, d);
			double st = SteeringBehaviours::getOptimalSpeedAtDistance(a, s0, d);
			cout << a << "\t"  << s0 << "\t" << d << "\t" << t << "\t" << st << endl;
		}
}


unsigned char fixShift (unsigned char key)
{
	switch(key)
	{
	case '<':  return ',';
	case '>':  return '.';
	case '?':  return '/';
	case ':':  return ';';
	case '"':  return '\'';
	default:
		return tolower(key);
	}
}

void keyboardDown (unsigned char key, int x, int y)
{
	key = fixShift(key);

	// mark key as pressed
	key_pressed[key] = true;

	switch (key)
	{
	case 27: // on [ESC]
		exit(0); // normal exit
		break;
	}
}

void keyboardUp (unsigned char key, int x, int y)
{
	key = fixShift(key);
	key_pressed[key] = false;
}

void specialDown (int special_key, int x, int y)
{
	switch(special_key)
	{
	case GLUT_KEY_RIGHT:
		key_pressed[KEY_PRESSED_RIGHT] = true;
		break;
	case GLUT_KEY_LEFT:
		key_pressed[KEY_PRESSED_LEFT] = true;
		break;
	case GLUT_KEY_UP:
		key_pressed[KEY_PRESSED_UP] = true;
		break;
	case GLUT_KEY_DOWN:
		key_pressed[KEY_PRESSED_DOWN] = true;
		break;
	case GLUT_KEY_END:
		key_pressed[KEY_PRESSED_END] = true;
		break;
	}
}

void specialUp (int special_key, int x, int y)
{
	switch(special_key)
	{
	case GLUT_KEY_RIGHT:
		key_pressed[KEY_PRESSED_RIGHT] = false;
		break;
	case GLUT_KEY_LEFT:
		key_pressed[KEY_PRESSED_LEFT] = false;
		break;
	case GLUT_KEY_UP:
		key_pressed[KEY_PRESSED_UP] = false;
		break;
	case GLUT_KEY_DOWN:
		key_pressed[KEY_PRESSED_DOWN] = false;
		break;
	case GLUT_KEY_END:
		key_pressed[KEY_PRESSED_END] = false;
		break;
	}
}



void update ()
{
	system_clock::time_point current_time = system_clock::now();
	for(unsigned int i = 0; i < MAXIMUM_UPDATES_PER_FRAME &&
	                        next_update_time < current_time; i++)
	{
		double delta_time = SECONDS_PER_PHYSICS;
		if(g_is_paused)
			delta_time = 0.0;
		else if(key_pressed['g'])
			delta_time *= FAST_PHYSICS_FACTOR;

		handleInput(delta_time);
		if(delta_time > 0.0)
		{
			assert(gp_game != nullptr);
			gp_game->update(delta_time);

			old_update_times[next_old_update_index % SMOOTH_RATE_COUNT] = current_time;
			next_old_update_index++;

			if(key_pressed['u'])
				sleep(SIMULATE_SLOW_SECONDS);
		}

		next_update_time += PHYSICS_MICROSECONDS;
		current_time = system_clock::now();
	}

	if(current_time < next_update_time)
	{
		system_clock::duration sleep_time = next_update_time - current_time;
		sleep(duration<double>(sleep_time).count());
	}

	glutPostRedisplay();
}

void handleInput (double delta_time)
{
	assert(gp_game != nullptr);

	//
	//  Accelerate player - depends on physics rate
	//

	if(key_pressed[' '])
		gp_game->playerMainEngine(delta_time);
	if(key_pressed[';'] || key_pressed['\''])  // either key
		gp_game->playerManoeuverForward(delta_time);
	if(key_pressed['/'])
		gp_game->playerManoeuverBackward(delta_time);
	if(key_pressed['w'] || key_pressed['e'])  // either key
		gp_game->playerManoeuverUp(delta_time);
	if(key_pressed['s'])
		gp_game->playerManoeuverDown(delta_time);
	if(key_pressed['d'])
		gp_game->playerManoeuverRight(delta_time);
	if(key_pressed['a'])
		gp_game->playerManoeuverLeft(delta_time);

	//
	//  Rotate player - independant of physics rate
	//

	if(key_pressed['.'])
		gp_game->playerRotateCounterClockwise(SECONDS_PER_PHYSICS);
	if(key_pressed[','])
		gp_game->playerRotateClockwise(SECONDS_PER_PHYSICS);
	if(key_pressed[KEY_PRESSED_UP])
		gp_game->playerRotateUp(SECONDS_PER_PHYSICS);
	if(key_pressed[KEY_PRESSED_DOWN])
		gp_game->playerRotateDown(SECONDS_PER_PHYSICS);
	if(key_pressed[KEY_PRESSED_LEFT])
		gp_game->playerRotateLeft(SECONDS_PER_PHYSICS);
	if(key_pressed[KEY_PRESSED_RIGHT])
		gp_game->playerRotateRight(SECONDS_PER_PHYSICS);

	//
	//  Other
	//

	// 'g' is handled in update
	if(key_pressed['k'])
	{
		gp_game->knockOffCrystals();
		key_pressed['k'] = false;  // only once per keypress
	}
	if(key_pressed['p'])
	{
		g_is_paused = !g_is_paused;
		key_pressed['p'] = false;  // only once per keypress
	}
	if(key_pressed['t'])
	{
		g_is_show_debug = !g_is_show_debug;
		key_pressed['t'] = false;  // only once per keypress
	}
	// 'u' is handled in update
	// 'y' is handled in draw
	if(key_pressed[KEY_PRESSED_END])
	{
		delete gp_game;
		gp_game = new Game();
		key_pressed[KEY_PRESSED_END] = false;  // only once per keypress
	}
}



void reshape (int w, int h)
{
	glViewport (0, 0, w, h);

	window_width  = w;
	window_height = h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLdouble)w / (GLdouble)h, 1.0, 100000.0);
	glMatrixMode(GL_MODELVIEW);

	glutPostRedisplay();
}

void display ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// clear the screen - any drawing before here will not display

	glLoadIdentity();
	assert(gp_game != nullptr);
	gp_game->draw(g_is_show_debug);
	drawOverlays();

	if(key_pressed['y'])
		sleep(SIMULATE_SLOW_SECONDS);  // simulate slow drawing

	// send the current image to the screen - any drawing after here will not display
	glutSwapBuffers();
}

void drawOverlays ()
{
	SpriteFont::setUp2dView(window_width, window_height);

	system_clock::time_point current_time = system_clock::now();

	// display frame rate

	unsigned int oldest_frame_index = (next_old_frame_index + 1) % SMOOTH_RATE_COUNT;
	duration<float> total_frame_duration = current_time - old_frame_times[oldest_frame_index];
	float average_frame_duration = total_frame_duration.count() / (SMOOTH_RATE_COUNT - 1);
	float average_frame_rate = 1.0f / average_frame_duration;

	stringstream smoothed_frame_rate_ss;
	smoothed_frame_rate_ss << "Frame rate:\t" << setprecision(3) << average_frame_rate;
	font.draw(smoothed_frame_rate_ss.str(), 16, 16);

	// update frame rate values

	old_frame_times[next_old_frame_index % SMOOTH_RATE_COUNT] = current_time;
	next_old_frame_index++;

	// display physics rate

	unsigned int oldest_update_index = (next_old_update_index + 1) % SMOOTH_RATE_COUNT;
	duration<float> total_update_duration = current_time - old_update_times[oldest_update_index];
	float average_update_duration = total_update_duration.count() / (SMOOTH_RATE_COUNT - 1);
	float average_update_rate = 1.0f / average_update_duration;

	stringstream smoothed_update_rate_ss;
	smoothed_update_rate_ss << "Update rate:\t" << setprecision(3) << average_update_rate;
	font.draw(smoothed_update_rate_ss.str(), 16, 40);

	// display crystal information

	assert(gp_game != nullptr);
	stringstream crystals_ss;
	crystals_ss << "Drifting crystals:\t" << gp_game->getNonGoneCrystalCount();
	font.draw(crystals_ss.str(), 16, 64);

	stringstream collected_ss;
	collected_ss << "Collected crystals:\t" << gp_game->getCrystalsCollected();
	font.draw(collected_ss.str(), 16, 88);

	// display drone information

	assert(gp_game != nullptr);
	stringstream drones_ss;
	drones_ss << "Living Drones: " << gp_game->getLivingDroneCount();
	font.draw(drones_ss.str(), 16, 112);
/*
	// display player information

	assert(gp_game != nullptr);
	stringstream player_ss;
	player_ss << "Player Speed: " << gp_game->getPlayer().getVelocity().getNorm();
	font.draw(player_ss.str(), 16, 136);
*/
	// display control keys

	unsigned char byte_g = key_pressed['g'] ? 0x00 : 0xFF;
	unsigned char byte_t = g_is_show_debug  ? 0x00 : 0xFF;
	unsigned char byte_y = key_pressed['y'] ? 0x00 : 0xFF;
	unsigned char byte_u = key_pressed['u'] ? 0x00 : 0xFF;

	font.draw("[G]:\tAccelerate time",  window_width - 256,  16, byte_g, 0xFF, byte_g);
	font.draw("[T]:\tToggle debugging", window_width - 256,  48, byte_t, 0xFF, byte_t);
	font.draw("[Y]:\tSlow display",     window_width - 256,  80, byte_y, 0xFF, byte_y);
	font.draw("[U]:\tSlow physics",     window_width - 256, 112, byte_u, 0xFF, byte_u);

	// display "GAME OVER" if appropriate

	if(gp_game->isOver())
		font.draw("GAME OVER", window_width / 2, window_height / 2);

	SpriteFont::unsetUp2dView();
}

