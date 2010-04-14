/*

    This file is part of GeoGen.

    GeoGen is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    GeoGen is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GeoGen.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include "ggen_support.h"
#include "ggen_data_1d.h"
#include "ggen_data_2d.h"
#include "ggen_scriptarg.h"

class GGen;

class GGEN_EXPORT GGen{
protected: 
	static GGen* instance;

	GGen_Status status;
public:
	void (*message_callback) (const GGen_String& message, GGen_Message_Level, int line, int column);
	void (*return_callback) (const GGen_String& name, const int16* map, int width, int height);
	void (*progress_callback) (int current_progress, int max_progress);

	vector<GGen_ScriptArg> args;

	uint16 output_width, output_height;

	uint32 max_progress, current_progress;

	GGen_Size max_width, max_height;
	uint16 max_map_count;

	GGen();
	~GGen();

	static GGen* GetInstance();

	GGen_Status GetStatus();

	void ThrowMessage(const GGen_String& message, GGen_Message_Level level, int line = -1, int column = -1);
	//void ThrowMessage(const wchar_t* message, GGen_Message_Level level, int line = -1, int column = -1);

	void SetMessageCallback( void (*message_callback) (const GGen_String& message, GGen_Message_Level, int line, int column));
	void SetReturnCallback( void (*return_callback) (const GGen_String& name, const int16* map, int width, int height) );
	void SetProgressCallback( void (*return_callback) (int current_progress, int max_progress));
	
	virtual bool SetScript(const GGen_String& script) = 0;
	virtual GGen_String GetInfo(const GGen_String& label) = 0;
	virtual int GetInfoInt(const GGen_String& label) = 0;
	virtual vector<GGen_ScriptArg>* LoadArgs();
	virtual int16* Generate() = 0;
	
	void SetMaxWidth(GGen_Size width);
	void SetMaxHeight(GGen_Size height);
	void SetMaxMapCount(uint16 count);

	/* Constraint getters and progress methods must be static to be exported as globals to Squirrel */
	static GGen_Size GetMaxWidth();
	static GGen_Size GetMaxHeight();
	static uint16 GetMaxMapCount();

	static void InitProgress(uint32 max_progress);
	static void SetProgress(uint32 current_progress);
	static void IncreaseProgress();
	
	virtual void RegisterPreset(GGen_Data_1D* preset, const GGen_String& label) = 0;
	virtual void RegisterPreset(GGen_Data_2D* preset, const GGen_String& label) = 0;
	virtual void RegisterPreset(GGen_Amplitudes* preset, const GGen_String& label) = 0;

	void SetSeed(unsigned seed);
};