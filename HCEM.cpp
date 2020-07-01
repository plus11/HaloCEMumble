// Copyright 2019 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.
#include "../mumble_plugin_main.h" // Include standard plugin header.
static int fetch(float *avatar_pos, float *avatar_front, float *avatar_top, float *camera_pos, float *camera_front, float *camera_top, std::string &context, std::wstring &identity) {
	for (int i=0;i<3;i++) {
		avatar_pos[i] = avatar_front[i] = avatar_top[i] = camera_pos[i] = camera_front[i] = camera_top[i] = 0.0f;
	}
	
	// Boolean values to check if game addresses retrieval is successful and if the player is in-game
	bool ok, state;
	// Create containers to stuff our raw data into, so we can convert it to Mumble's coordinate system
	float avatar_pos_corrector[3], avatar_front_corrector[3], avatar_top_corrector[3];
	// Peekproc and assign game addresses to our containers, so we can retrieve positional data
	ok = peekProc(pModule + 0x<offset>, &state, 1) && // Magical state value: 1 when in-game and 0 when in main menu.
		peekProc(pModule + 0x<offset>, avatar_pos_corrector, 12) && // Avatar Position values (X, Z and Y, respectively).
		peekProc(pModule + 0x<offset>, avatar_front_corrector, 12) && // Avatar Front Vector values (X, Z and Y, respectively).
		peekProc(pModule + 0x<offset>, avatar_top_corrector, 12); // Avatar Top Vector values (X, Z and Y, respectively).
	// This prevents the plugin from linking to the game in case something goes wrong during values retrieval from memory addresses.
	if (! ok) {
		return false;
	}
	if (! state) { // If not in-game
		context.clear(); // Clear context
		identity.clear(); // Clear identity
		// Set vectors values to 0.
		for (int i=0;i<3;i++)
			avatar_pos[i] = avatar_front[i] = avatar_top[i] = camera_pos[i] =  camera_front[i] = camera_top[i] = 0.0f;
		
		return true; // This tells Mumble to ignore all vectors.
	}
	
        /*
	Mumble | Game
	X      | X
	Y      | Z
	Z      | Y
	*/
	avatar_pos[0] = avatar_pos_corrector[0];
	avatar_pos[1] = avatar_pos_corrector[2];
	avatar_pos[2] = avatar_pos_corrector[1];
	avatar_front[0] = avatar_front_corrector[0];
	avatar_front[1] = avatar_front_corrector[2];
	avatar_front[2] = avatar_front_corrector[1];
	avatar_top[0] = avatar_top_corrector[0];
	avatar_top[1] = avatar_top_corrector[2];
	avatar_top[2] = avatar_top_corrector[1];
	
	// Sync camera with avatar
	for (int i=0;i<3;i++) {
		avatar_pos[i]/=32.0f; // Scale to meters
		camera_pos[i] = avatar_pos[i];
		camera_front[i] = avatar_front[i];
		camera_top[i] = avatar_top[i];
	}
	return true;
}
static int trylock(const std::multimap<std::wstring, unsigned long long int> &pids) {
	if (! initialize(pids, L"<executable name>.exe")) { // Retrieve game executable's memory address
		return false;
	}
	// Check if we can get meaningful data from it
	float apos[3], afront[3], atop[3], cpos[3], cfront[3], ctop[3];
	std::wstring sidentity;
	std::string scontext;
	if (fetch(apos, afront, atop, cpos, cfront, ctop, scontext, sidentity)) {
		return true;
	} else {
		generic_unlock();
		return false;
	}
}
static const std::wstring longdesc() {
	return std::wstring(L"Supports <game name> version <game version> without context or identity support."); // Plugin long description
}
static std::wstring description(L"<game name> (v<game version>)"); // Plugin short description
static std::wstring shortname(L"<game name>"); // Plugin short name
static int trylock1() {
	return trylock(std::multimap<std::wstring, unsigned long long int>());
}
static MumblePlugin <game acronym, lowercase>plug = {
	MUMBLE_PLUGIN_MAGIC,
	description,
	shortname,
	NULL,
	NULL,
	trylock1,
	generic_unlock,
	longdesc,
	fetch
};
static MumblePlugin2 <game acronym, lowercase>plug2 = {
	MUMBLE_PLUGIN_MAGIC_2,
	MUMBLE_PLUGIN_VERSION,
	trylock
};
extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin *getMumblePlugin() {
	return &<game acronym, lowercase>plug;
}
extern "C" MUMBLE_PLUGIN_EXPORT MumblePlugin2 *getMumblePlugin2() {
	return &<game acronym, lowercase>plug2;
}
