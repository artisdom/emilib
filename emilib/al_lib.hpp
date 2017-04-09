// By Emil Ernerfeldt 2012-2016
// LICENSE:
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
// Wrapper around OpenAL, a library for playing sounds.

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "al_lib_fwd.hpp"

using ALCdevice = struct ALCdevice_struct;
using ALCcontext = struct ALCcontext_struct;

namespace al {

// ----------------------------------------------------------------------------

struct Vec3f { float data[3]; };

void check_for_al_error();

// ----------------------------------------------------------------------------

/// A loaded sound. Can be played via Source. Many Source:s can play the same Sound at the same.
class Sound
{
public:
	static Sound load_wav(const char* path);

	Sound(Sound&& o) noexcept : _debug_name(o._debug_name), _buffer_id(o._buffer_id), _size_bytes(o._size_bytes) { o._buffer_id = 0; o._size_bytes = 0; }
	~Sound();

	/// Memory usage.
	unsigned size_bytes() const { return _size_bytes; }

private:
	friend class Source;
	Sound(const Sound&) = delete;
	Sound& operator=(const Sound&) = delete;

	Sound(const char* debug_name, unsigned buffer_id, unsigned size_bytes);

	const char* _debug_name;
	unsigned    _buffer_id;
	unsigned    _size_bytes;
};

// ----------------------------------------------------------------------------

/// A sound source. Has position, and a sound to play.
class Source
{
public:
	enum State
	{
		INITIAL,
		PLAYING,
		PAUSED,
		STOPPED,
	};

	/// Returns the maximum number of sources possible to have instantiated at the same time
	static int max_sources();

	Source();
	~Source();

	void set_state(State arg);
	State state() const;

	void play();
	void pause();
	void stop();
	void rewind();

	void set_sound(Sound_SP sound);
	const Sound_SP& sound() const;

	/// Volume, [0,1]. >1 MAY work.
	void set_gain(float gain);
	float gain() const;

	/// sets pitch (clamped to [0,2]), does affect speed
	void set_pitch(float pitch);
	/// get current pitch
	float pitch() const;

	void set_pos(Vec3f);
	Vec3f pos() const;

	void set_vel(Vec3f);
	Vec3f vel() const;

	void set_direction(Vec3f);
	Vec3f direction() const;

	/**
	 * Indicate distance above which sources are not
	 * attenuated using the inverse clamped distance model.
	 * Default: +inf
	 */
	void set_max_distance(float arg);
	float max_distance() const;

	/// Controls how fast the sound falls off with distance
	void set_rolloff_factor(float arg);
	float rolloff_factor() const;

	/**
	 * source specific reference distance
	 * At 0.0, no distance attenuation occurs.
	 * Default is 1.0.
	 */
	void set_reference_distance(float arg);
	float reference_distance() const;

	void set_min_gain(float arg);
	float min_gain() const;

	void set_max_gain(float arg);
	float max_gain() const;

	void set_cone_outer_gain(float arg);
	float cone_outer_gain() const;

	void set_cone_inner_angle(float arg);
	float cone_inner_angle() const;

	void set_cone_outer_angle(float arg);
	float cone_outer_angle() const;

	/// is the position relative to the listener? false by default
	void set_relative_to_listener(bool arg);
	bool relative_to_listener() const;

	void set_looping(bool);
	bool looping() const;

private:
	Source(const Source&) = delete;
	Source& operator=(const Source&) = delete;

	unsigned _source;
	Sound_SP _sound;
	float    _gain = 1;
};

// ----------------------------------------------------------------------------

/// All Listeners are really the same.
/// TODO: static interface.
class Listener
{
public:
	void set_pos(Vec3f);
	Vec3f pos() const;

	void set_vel(Vec3f);
	Vec3f vel() const;

	void set_orientation(const Vec3f& forward, const Vec3f& up);

	Vec3f direction() const;
	Vec3f up() const;

	void set_gain(float);
	float gain() const;
};

// ----------------------------------------------------------------------------

/// You should have only one of these.
class SoundMngr
{
public:
	/// Look for sounds relative to sfx_dir
	explicit SoundMngr(const std::string& sfx_dir);
	~SoundMngr();

	//------------------------------------------------------------------------------

	/// sound_name == "subdir/foo.wav"
	void prefetch(const std::string& sound_name);

	/// Recursively prefetch all textures in sfx_dir/sub_folder
	void prefetch_all(const std::string& sub_folder = "");

	/// Fire and forget - or keep the returned source and modify it.
	/// Returns nullptr on fail
	Source_SP play(const std::string& sound_name);

	//------------------------------------------------------------------------------
	// Global settings

	bool is_working() const;

	Listener* listener() { return &_listener; }

	enum DistanceModel
	{
		NONE,
		INVERSE_DISTANCE,
		INVERSE_DISTANCE_CLAMPED,
	};

	/// set speed of sound. 344 by default (speed of sound in air in meters/second)
	void set_doppler_vel(float vel);
	/// get speed of sound. 344 by default (speed of sound in air in meters/second)
	float doppler_vel();

	/// default is 1, used to (de)exaggerate the effect of the Doppler effect
	void set_doppler_factor(float factor);
	/// default is 1, used to (de)exaggerate the effect of the Doppler effect
	float doppler_factor();

	/// default is INVERSE_DISTANCE
	void set_distance_model(DistanceModel model);
	/// default is INVERSE_DISTANCE
	DistanceModel distance_model();

	const char* vendor();
	const char* version();
	const char* renderer();
	const char* extensions();

	//------------------------------------------------------------------------------

	void print_memory_usage() const;

private:
	Sound_SP load_sound(const std::string& sound_name, bool is_hot);
	Source_SP get_source();

	using SoundMap   = std::unordered_map<std::string, Sound_SP>;
	using SourceList = std::vector<Source_SP>;

	std::string _sfx_dir;
	ALCdevice*  _device  = nullptr;
	ALCcontext* _context = nullptr;
	Listener    _listener;

	SoundMap    _map;
	SourceList  _sources;
};

} // namespace al
