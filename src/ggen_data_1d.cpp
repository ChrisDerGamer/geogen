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

//#include <iostream>

#include "sqplus.h"


#include "ggen_support.h"
#include "ggen_amplitudes.h"
#include "ggen_data_1d.h"
#include "ggen.h"

extern GGen* ggen_current_object;

/** 
 * Creates a 1D data array and fills it with a constant value
 * @param length of the array
 * @param value to be filled with
 */
GGen_Data_1D::GGen_Data_1D(GGen_Coord length, GGen_Height value)
{
	
	GGen_Script_Assert(length > 1);

	this->length = length;

	/* Allocate the array */
	this->data = new GGen_Height[this->length];

	GGen_Script_Assert(this->data != NULL);

	this->Fill(value);
}

GGen_Data_1D::~GGen_Data_1D()
{
	delete [] this->data;
}

GGen_Data_1D* GGen_Data_1D::Clone()
{
	GGen_Data_1D* victim = new GGen_Data_1D(this->length, 0);
	
	/* Copy the data */
	memcpy(victim->data, this->data, sizeof GGen_Height * victim->length);
	
	return victim;
}

/** 
 * Reads and returns one value from the array
 * @param x coordinate of the value
 */
 

GGen_Size GGen_Data_1D::GetLength()
{
	return this->length;
}
 
GGen_Height GGen_Data_1D::GetValue(GGen_Coord x)
{
	GGen_Script_Assert(x < this->length);
	
	return this->data[x];
}

/** 
 * Reads and returns one value from the array as if its size was scale_to_x
 * @param x coordinate of the value
 * @param target length of the array
 */
GGen_Height GGen_Data_1D::GetValueInterpolated(GGen_Coord x, GGen_Size scale_to_x)
{
	GGen_Script_Assert(x < scale_to_x);

	/* No interpolation needed if the sizes are equal */
	if (scale_to_x == this->length) {
		return this->data[x];
	} else if (scale_to_x > this->length) {

		/* The target scale is larger, interpolation is necessary */
		double ratio = (double) (scale_to_x - 1) / (double) (this->length - 1);

		/* How much does the source tile overlap over the smaller grid? */
		double remainder = (x / ratio) - floor(x / ratio);

		/* Interpolate the value from two sorrounding values */
		return (GGen_Height) ((double) data[(GGen_Coord) floor(x / ratio)] * (1 - remainder) + (double) data[(GGen_Coord) floor(x / ratio) + 1] * (remainder));
	}

	double ratio = (double) (scale_to_x - 1) / (double) (this->length - 1);

	/* The target is smaller, pick the closest value */
	return (GGen_Height) data[(GGen_Coord) floor((double)x / ratio + 0.5)];
}

/** 
 * Sets value on coordinate
 * @param coordinate to modify
 * @param value to use
 */
void GGen_Data_1D::SetValue(GGen_Coord x, GGen_Height value)
{
	GGen_Script_Assert(x < this->length);

	this->data[x] = value;
}

void GGen_Data_1D::SetValueInRange(GGen_Coord from, GGen_Coord to, GGen_Height value)
{
	GGen_Script_Assert(from < this->length || to < this->length || from < to);

	for (GGen_Coord i = from; i <= to; i++) {
		this->data[i] = value;
	}
}

/** 
 * Fills the array with value
 * @param value to be used
 */
void GGen_Data_1D::Fill(GGen_Height value)
{
	for (GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] = value;
	}
}

/** 
 * Adds a flat value to each value in the array
 * @param value to be used
 */
void GGen_Data_1D::Add(GGen_Height value)
{
	for (GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] += value;
	}
}

/** 
 * Combines the array with second array by just adding them together
 * @param addend to be combined with
 */
void GGen_Data_1D::AddArray(GGen_Data_1D* addend)
{
	/* Scale the addend as necessary */
	for (GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] += addend->GetValueInterpolated(i, this->length);
	}
}

/*
 * Adds values from (unscaled) addend to the array
 * @param offset of the addend coords
 * @param addend - the second array
 */
void GGen_Data_1D::AddTo(GGen_Data_1D* addend, GGen_CoordOffset offset)
{
	/* Walk through the items where the array and the addend with ofset intersect */
	for (GGen_Coord i = MAX(0, offset); i < MIN(this->length, offset + addend->length); i++) {
		this->data[i] += addend->data[i - offset];
	}
}

void GGen_Data_1D::AddArrayMasked(GGen_Data_1D* addend, GGen_Data_1D* mask, bool relative)
{
	GGen_ExtHeight max = 255;

	if(relative){
		max = mask->Max();
	}

	for(GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] += (GGen_ExtHeight) addend->GetValueInterpolated(i, this->length) * (GGen_ExtHeight) mask->GetValueInterpolated(i, this->length) / max;
	}
}

void GGen_Data_1D::AddMasked(GGen_Height value, GGen_Data_1D* mask, bool relative)
{
	GGen_Height max = 255;
 
	if(relative){
		max = mask->Max();
	}

	for(GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] += (GGen_ExtHeight) value * (GGen_ExtHeight) mask->GetValueInterpolated(i, this->length) / max;
	}
}

/** 
 * Scales each value in the array by a number. 100% = 1, 0.5 = 50%, 2.0 = 200%
 * @param value to be used
 */
void GGen_Data_1D::Multiply(double value)
{
	for (GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] = (GGen_Height) ((double) this->data[i] * value);
	}
}

/** 
 * Scales each value in the array by a value from the second array. 100% = 1, 0.5 = 50%, 2.0 = 200%
 * @param value to be used
 */
void GGen_Data_1D::MultiplyArray(GGen_Data_1D* factor)
{
	for (GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] = (GGen_Height) ((double) this->data[i] * factor->GetValueInterpolated(i, this->length));
	}
}

/** 
 * Inverts signs of all values in the array
 */
void GGen_Data_1D::Invert()
{
	for (GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] = -this->data[i];
	}
}

/**
 * Change size of the array 
 * @param length of the new array
 * @param scale_values - should the values be scaled too?
 */
void GGen_Data_1D::ScaleTo(GGen_Size new_length, bool scale_values)
{
	GGen_Script_Assert(new_length > 1);

	double ratio = new_length / this->length;

	/* Allocate the new array */
	GGen_Height* new_data = new GGen_Height[new_length];

	GGen_Script_Assert(new_data != NULL);

	/* Fill the new array */
	for (GGen_Coord i = 0; i < new_length; i++) {
		new_data[i] = scale_values ? (GGen_Height) ((double) this->GetValueInterpolated(i, new_length) * ratio) : this->GetValueInterpolated(i, new_length);
	}

	/* Relink and delete the original array data */
	delete [] this->data;
	this->data = new_data;
	this->length = new_length;
}

/**
 * Scales the values so they all fit to the passed range
 * @param new minimum value
 * @param new maximum value
 */
void GGen_Data_1D::ScaleValuesTo(GGen_Height new_min, GGen_Height new_max)
{
	GGen_Script_Assert(new_max > new_min);

	GGen_Height min = this->Min();
	GGen_Height max = this->Max() - min;

	if (max == 0) {
		this->Fill(min);
		return;
	}

	new_max -= new_min;

	for (GGen_Coord i = 0; i < length; i++) {
		this->data[i] = new_min + (this->data[i] - min) * new_max / max;
	}
}

/**
 * Change size of the array 
 * @param ratio in scale 100% = 1, 0.5 = 50%, 2.0 = 200%
 * @param scale_values - should the values be scaled too?
 */
void GGen_Data_1D::Scale(double ratio, bool scale_values)
{
	this->ScaleTo((GGen_Coord) ((double) this->length * ratio), scale_values);
}

/*
 * Crops all indices outside given interval from the array. The border points will
 * remain in the area.
 * @param minimum index of the range
 * @param maximum index of the range
 */
void GGen_Data_1D::ResizeCanvas(GGen_Size new_length, GGen_CoordOffset new_zero)
{
	/* Allocate the new array */
	GGen_Height* new_data = new GGen_Height[new_length];

	GGen_Script_Assert(new_data != NULL);

	for (GGen_Coord i = 0; i < new_length; i++) {
		if (i + new_zero >= 0 && i + new_zero < this->length){
			new_data[i] = this->data[i + new_zero];
		} else {
			new_data[i] = 0;
		}
	}

	/* Relink and delete the original array data */
	delete [] this->data;
	this->data = new_data;
	this->length = new_length;
}

/*
 * Clamps all values in the array to certain range
 * @param minimum value of the range
 * @param maximum value of the range
 */
void GGen_Data_1D::Clamp(GGen_Height min, GGen_Height max)
{
	GGen_Script_Assert(max > min);

	for (GGen_Coord i = 0; i < this->length; i++) {
		if(this->data[i] > max) this->data[i] = max;
		else if(this->data[i] < min) this->data[i] = min;
	}
}

/*
 * Inverts order of items in the array
 */
void GGen_Data_1D::Flip()
{
	GGen_Height temp;
	
	/* Go through the first half of the array and flip the value with its counterpart indexed from end */
	for (GGen_Coord i = 0; i < this->length / 2; i++) {
		temp = this->data[i];
		this->data[i] = this->data[this->length - 1 - i];
		this->data[length - 1 - i] = temp;
	}
}

/*
 * Returns the lowest value in the array
 */
GGen_Height GGen_Data_1D::Min()
{
	GGen_Height temp = GGEN_MAX_HEIGHT;

	for (GGen_Coord i = 0; i < this->length; i++) {
		temp = temp > this->data[i] ? this->data[i] : temp;
	}

	return temp;
}

/*
 * Returns the highest value in the array
 */
GGen_Height GGen_Data_1D::Max()
{
	GGen_Height temp = GGEN_MIN_HEIGHT;

	for (GGen_Coord i = 0; i < this->length; i++){
		temp = temp < this->data[i] ? this->data[i] : temp;
	}

	return temp;
}

/*
 * Shifts all indices in the array left (if the distance is < 0) or right (otherwise)
 * @param difference of original and shifted indices
 * @param shift mode
 */
void GGen_Data_1D::Shift(GGen_CoordOffset distance, GGen_Overflow_Mode mode)
{
	GGen_Script_Assert(distance < this->length && distance != 0 && distance > -this->length);
	
	/* Cycle mode */
	if (mode == GGEN_CYCLE) {
		/* Allocate the new array */
		GGen_Height* new_data = new GGen_Height[length];

		GGen_Script_Assert(new_data != NULL);

		/* Fill the new array with shifted data */
		for (GGen_Coord i = 0; i < this->length; i++) {
			/* Some values can be just plainly shifted */
			if ((distance > 0 && i < this->length - distance) || (distance < 0 && (signed) i >= -distance)) {
				new_data[i + distance] = this->data[i];
			}
			/* Some must go through the right "border" */
			else if (distance > 0){
				new_data[i - this->length + distance] = this->data[i];
			}
			/* And some must go through the left "border" */
			else{
				new_data[i + this->length + distance] = this->data[i];
			}
		}

		/* Relink and delete the original data array */
		delete [] this->data;
		this->data = new_data;
	} else {
		/* Discard or Discard&fill mode */
		GGen_Height temp;
		
		/* Positive distance -> shift right */
		if (distance > 0) {
			for (GGen_CoordOffset i = this->length - 1; i >= 0; i--) {
				/* Some values can be just plainly shifted... */
				if(i > distance - 1){
					this->data[i] = this->data[i - distance];
					
					if (mode == GGEN_DISCARD_AND_FILL) {
						temp = this->data[i];
					}
				} else {
					/* And some must be filled with zeros / closest value */
					if (mode == GGEN_DISCARD_AND_FILL) {
						this->data[i] = temp;
					} else {
						this->data[i] = 0;
					}
				}
			}	
		}
		/* Negative distance -> shift left */
		else{
			for (GGen_CoordOffset i = 0; i < length; i++) {
				/* Some values can be just plainly shifted... */
				if (i < this->length + distance) {
					this->data[i] = this->data[i - distance];
					
					if (mode == GGEN_DISCARD_AND_FILL) {
						temp = this->data[i];
					}
				} else {
					/* And some must be filled with zeros / closest value */
					if (mode == GGEN_DISCARD_AND_FILL){
						this->data[i] = temp;
					} else {
						this->data[i] = 0;
					}
				}
			}				
		}
	}
}

/*
 * Unifies the data with data from the other array. The other array is scaled to fit the object.
 * Negative data are treated the same as positive - the higher value remains.
 * @param the victim
 */
void GGen_Data_1D::Union(GGen_Data_1D* victim)
{
	for (GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] = MIN(this->data[i], victim->GetValueInterpolated(i, this->length));
	}
}

/*
 * Unifies the data with data from the other array. The other array is scaled to fit the object.
 * Negative data are treated the same as positive - the higher value remains.
 * @param the victim
 */
void GGen_Data_1D::Intersection(GGen_Data_1D* victim)
{
	for (GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] = MAX(this->data[i], victim->GetValueInterpolated(i, this->length));
	}
}

void GGen_Data_1D::Abs()
{
	for (GGen_Index i = 0; i < this->length; i++) {
		if (this->data[i] < 0) {
			this->data[i] = -this->data[i];
		}
	}
}

/*
 * Reduces all data in the array to only two values. Values above treshold will be ones, values below
 * (treshold included) will be zeros.
 * @param treshold - maximum zero value
 */
void GGen_Data_1D::Monochrome(GGen_Height treshold)
{
	for (GGen_Coord i = 0; i < this->length; i++) {
		this->data[i] = this->data[i] > treshold ? 1 : 0;
	}	
}

/*
 * Replaces the array  data with information about steepness of slopes at every point but borders
 */
void GGen_Data_1D::SlopeMap()
{
	/* Allocate the new array */
	GGen_Height* new_data = new GGen_Height[this->length];

	GGen_Script_Assert(new_data != NULL);

	/* Calculate the slopes */
	for(GGen_Coord i = 1; i < this->length - 1; i++){
		new_data[i] = abs(this->data[i - 1] - this->data[i + 1]);
	}

	/* Make sure the border items are treated correctly (use the neighbouring values) */
	new_data[0] = new_data[1];
	new_data[this->length-1] = new_data[this->length-2];

	/* Relink and delete the original array data */
	delete [] this->data;
	this->data = new_data;	
}

/*
 * Reduces all slopes in the array to max. 45 degrees (=1:1)
 * @param switch if the substract from or add values to too steep slopes
 */
void GGen_Data_1D::Normalize(GGen_Normalization_Mode mode)
{
	/* Additive mode */
	if (mode == GGEN_ADDITIVE) {
		/* Fix left-to-right "downhills" */
		for (GGen_Coord i = 1; i < this->length; i++) {
			if (this->data[i] < this->data[i - 1] - 1) {
				this->data[i] = this->data[i - 1] - 1;
			}
		}

		/* Fix right-to-left "downhills" */
		for (GGen_Coord i = this->length - 2; i > 0; i--) {
			if (this->data[i] < this->data[i + 1] - 1) {
				this->data[i] = this->data[i + 1] - 1;
			}
		}
	} else {
		/* Substractive mode */
		/* Fix left-to-right "uphills" */
		for (GGen_Coord i = 1; i < this->length; i++) {
			if (this->data[i] > this->data[i - 1] + 1) {
				this->data[i] = this->data[i - 1] + 1;
			}
		}

		/* Fix right-to-left "uphills" */
		for (GGen_Coord i = this->length - 2; i > 0; i--) {
			if (this->data[i] > this->data[i + 1] + 1) {
				this->data[i] = this->data[i + 1] + 1;
			}
		}
	}

	/* Make sure the border items are treated correctly (use the neighbouring values) */
	this->data[0] = this->data[1];
	this->data[this->length-1] = this->data[this->length-2];
}

/**
 * Creates a smooth gradient of values
 * @param from - where should the gradient start
 * @param to - where should it end
 * @param from which value will the gradient start
 * @param to - maximum value
 * @param fill_flat - should be the areas outside the gradient area be filled with min/max values?
 */
void GGen_Data_1D::Gradient(GGen_Coord from, GGen_Coord to, GGen_Height from_value, GGen_Height to_value, bool fill_flat)
{
	GGen_Script_Assert(from < this->length || to < this->length);
	
	/* Swap values if necessary */
	if (from > to) {
		GGen_Coord temp = to;
		to = from;
		from = temp;
		
		GGen_Height temp2 = to_value;
		to_value = from_value;
		from_value = temp2;
	}

	GGen_Height base = from_value;
	GGen_ExtHeight offset = (GGen_ExtHeight) (to_value - base);
	GGen_Distance max_distance = to - from;
	
	for (GGen_Coord i = 0; i < this->length; i++) {
		/* Calculate current distance from "from" and "to" */
		GGen_Distance distance_from = (GGen_Distance) abs(i - from);
		GGen_Distance distance_to = (GGen_Distance) abs(i - to);

		if (distance_from > max_distance) {
			this->data[i] = fill_flat ? to_value: this->data[i];
		} else if (distance_to > max_distance) {
			this->data[i] = fill_flat ? from_value : this->data[i];
		} else {
			this->data[i] = base + (GGen_Height) (offset * (signed) distance_to / (signed) max_distance);
		}
	}
}

void GGen_Data_1D::Noise(GGen_Size min_feature_size, GGen_Size max_feature_size, GGen_Amplitudes* amplitudes)
{
	GGen_Script_Assert(amplitudes != NULL);

	uint16 frequency = GGen_log2(max_feature_size);
	GGen_Height amplitude = amplitudes->data[frequency];

	GGen_Height* new_data = new GGen_Height[this->length];

	GGen_Script_Assert(new_data != NULL);

	this->Fill(0);

	for (GGen_Size wave_length = max_feature_size; wave_length >= 1; wave_length /= 2) {
		frequency = GGen_log2(wave_length);
		amplitude = amplitudes->data[frequency];

		if (wave_length < min_feature_size) break;

		for (GGen_Coord i = 0; i < this->length; i += wave_length) {
			new_data[i] = GGen_Random<GGen_Height>(-amplitude, amplitude);
		}

		if (wave_length > 1) {
			for (GGen_Coord i = 0; i < this->length; i++) {
				if (i % wave_length == 0) continue;

				GGen_Coord nearest = i - i % wave_length;

				GGen_Coord next = i - i % wave_length + wave_length;

				next = next >= this->length ? 0 : next;

				double vertical = (1 - cos( (i % wave_length) * 3.1415927 / wave_length)) * .5;

				this->data[i] +=  (GGen_Height) ( (double) new_data[nearest] * (1 - vertical) + (double) new_data[next] * vertical);
			} 
		}

		for (GGen_Coord i = 0; i < this->length; i += wave_length) {
			this->data[i] += new_data[i];
		}

	}

	delete [] new_data;
}

/*
 * Smooths the values using the linear blur algorithm.
 * @param radius from which are the values caulculated into the mean
 * @power weight of individual values. The current cell has always weight 1024
 */
void GGen_Data_1D::Smooth(GGen_Distance radius)
{
	GGen_Script_Assert(radius > 0 && radius < this->length);
	
	/* Allocate the new array */
	GGen_Height* new_data = new GGen_Height[this->length];

	GGen_Script_Assert(new_data != NULL);

	/* Calculate size of the filter window */
	GGen_Size window_size = radius * 2 + 1;

	/* Prefill the window with value of the left edge + n leftmost values (where n is radius) */
	GGen_ExtHeight window_value = this->data[0] * radius;

	for (GGen_Distance i = 0; i < radius; i++) {
		window_value += this->data[i];
	}

	/* In every step shift the window one tile to the right  (= substract its leftmost cell and add
	value of rightmost + 1). i represents position of the central cell of the window. */
	for(GGen_Coord i = 0; i < this->length; i++){
		/* If the window is approaching right border, use the rightmost value as fill. */
		if(i < radius){
			window_value += this->data[i + radius] - this->data[0];
		}
		else if(i + radius < this->length){
			window_value += this->data[i + radius] - this->data[i - radius];
		}
		else{
			window_value += this->data[this->length - 1] - this->data[i - radius];
		}

		/* Set the value of current tile to arithmetic average of window tiles. */
		new_data[i] = window_value / window_size;
	}

	/* Relink and delete the original array data */
	delete [] this->data;
	this->data = new_data;
}

/*
 * Shifts the array values so given percentage of it is under zero (zero represents the water level).
 * @param percentage of the map to be flooded
 */
void GGen_Data_1D::Flood(double water_amount)
{
	GGen_Script_Assert(water_amount < 1 && water_amount > 0);

	/* Calculate how many tiles should be flooded in total */
	GGen_Index target = (GGen_Index) (water_amount * (double) this->length);
	
	GGen_Index last_amount = 0;

	GGen_Height level = this->Min();
	GGen_Height max = this->Max();

	/* Go through the array values from bottom up and try to find the best fit to target water amount */
	while (level < max) {
		/* Calculate the amount of waters above current level */
		GGen_Index amount = 0;
		for (GGen_Coord i = 0; i < this->length; i++) {
			if (this->data[i] >= level) {
				amount++;
			}
		}

		/* Is current level higher than the target? */
		if (amount <= target) {
			/* Find if this level fits better than the previous (the closest fit applies) */
			if (amount - target < target - last_amount) {
				break;
			} else {
				level--;
				break;
			}
		}

		last_amount = amount;

		level++;
	}

	/* Shift the heights so given portion of the array is under zero */
	this->Add(-level);
}