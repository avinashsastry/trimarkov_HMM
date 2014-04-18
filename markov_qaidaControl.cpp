/*
 *  markov_qaidaControl.cpp
 *  markov
 *
 *  Created by Avinash Sastry on 4/23/10.
 *  Copyright 2010 Georgia Institute of Technology. All rights reserved.
 *
 */
#include "ext.h"
#include "ext_obex.h"

#include "markov_defines.h"
#include "markov_enums.h"
#include "markov_classes.h"
#include "markov_1DList.h"
#include "markov_Bundle2D.h"
#include "markov_2DNode.h"
#include "markov_qaidaControl.h"

// Contructor
cQaidaControl :: cQaidaControl() {
	this->ThemeLength = 0;
	this->PhraseLength = 0;
	this->CycleControl = 0;
	this->Strictness = 3;
	this->Deviation = 1;
	FillStrokeMatrix();
	FillOpenClosed();
}

float cQaidaControl :: GetThemeLength() {
	return ThemeLength;
}

float cQaidaControl :: GetPhraseLength() {
	return PhraseLength;
}

t2DNode* cQaidaControl :: GetPhraseFirstNode() {
	if (Phrase.FirstNode != NULL)
	{
		return Phrase.FirstNode;
	}
	else
	{
		return NULL;
	}
}

// Add stroke - add a new stroke to the list of usable strokes
unsigned int cQaidaControl :: AddStroke(float InputStroke)
{
	t2DNode* TempNode = NULL;
	c1DList Vector(NULL, NULL, 0);
	Vector.Append(InputStroke);
	
	// If the node exists, don't add
	TempNode = ThemeStrokes.Find(&Vector);
	
	if (TempNode != NULL)
	{
		// Do nothing
	}
	else
	{
		// Add the stroke
		ThemeStrokes.Append(&Vector);
	}
	
	return eRETVAL_SUCCESS;
}

// Add duration - add a new duration to the list of usable durations
unsigned int cQaidaControl :: AddDuration(float InputDuration)
{
	t2DNode* TempNode = NULL;
	c1DList Vector(NULL, NULL, 0);
	Vector.Append(InputDuration);
	
	// If the node exists, don't add
	TempNode = ThemeDurations.Find(&Vector);
	
	if (TempNode != NULL)
	{
		// Do nothing
	}
	else
	{
		// Add the duration
		ThemeDurations.Append(&Vector);
	}
	return eRETVAL_SUCCESS;
}

// RecordTheme - Know when to start recording the theme and record till it gets the stop command
unsigned int cQaidaControl :: RecordTheme(float InputStroke, float InputDuration)
{
	// When the theme starts, I run this function.
	// All it does is add the notes and durations
	// to the Open and Closed versions of the theme.
	c1DList Vector;
	float ClosedStroke = 0;
	
	if (InputDuration != 0)
	{
		// AddStroke to ThemeStrokes
		AddStroke(InputStroke);
		
		// AddDuration to ThemeDurations
		AddDuration(InputDuration);
		
		// Calculate ThemeLength based on duration
		ThemeLength += 4.0/(InputDuration);
		
		// Add Stroke and Duration to the Theme
		Vector.Append(InputDuration);
		Vector.Append(InputStroke);
		Theme.Append(&Vector);
		Vector.DeleteAll();
		
		// Add ClosedStroke and Duration to Closed Theme
		ClosedStroke = GetClosed(InputStroke);
		Vector.Append(InputDuration);
		Vector.Append(ClosedStroke);
		ClosedTheme.Append(&Vector);
		Vector.DeleteAll();
		
		return eRETVAL_SUCCESS;
	}
	else {
		return eRETVAL_FAILURE;
	}
}

// Reset Theme
unsigned int cQaidaControl :: ResetTheme()
{
	ThemeStrokes.DeleteBundle();
	ThemeDurations.DeleteBundle();
	ThemeLength = 0;
	
	Theme.DeleteBundle();
	ClosedTheme.DeleteBundle();
	return eRETVAL_SUCCESS;
}

// Reset Phrase
unsigned int cQaidaControl :: ResetPhrase()
{
	Phrase.DeleteBundle();
	PhraseLength = 0;
	ClosedPhrase.DeleteBundle();
	return eRETVAL_SUCCESS;
}

// Generate the next note
float cQaidaControl :: GenerateNextStroke(tBundle2D* PDFStroke)
{
	// This function is used to screen the predictions
	// of the markov model. For eg:
	// If on a Roll(), I get a stroke that is not part
	// of the theme, or I get a duration that does not 
	// match the meter, then I should filter it out
	// to make sure that I stay within the constraints
	// for the qaida
	
	// Initialize variables
	unsigned int retval1 = eRETVAL_UNDEFINED;
	c1DList outcome1;
	t2DNode* TempNode = NULL;
	
	int counter = 0;
	double Distance = 0;
	
	while ((retval1 != eRETVAL_SUCCESS) && (counter < Strictness))
	{
		// Update the counter
		counter++;
		
		// Roll for stroke
		retval1 = PDFStroke->Roll(&outcome1, NULL);
		
		// If a stroke was predicted, screen the stroke
		if (retval1 & eRETVAL_PREDICTION_MADE)
		{
			TempNode = ThemeStrokes.Find(&outcome1);
			
			if (TempNode != NULL)
			{
				// If the stroke was in the theme,
				// Use the entropy distance to see whether
				// it is a good choice or not
				Distance = PDFStroke->DistanceToMostLikelyNode(&outcome1);
					
					if (Distance >= 0)
					{
						// Set a limit here to control entropy distance
						if (Distance < Deviation) {
							retval1 = eRETVAL_SUCCESS;
						}
						else {
							retval1 = eRETVAL_FAILURE;
						}
					}
					else {
						retval1 = eRETVAL_FAILURE;
					}
			}
			else
			{
				// If there are some valid substitutions
				// check here and pass it if it is one of them
				retval1 = eRETVAL_FAILURE;
			}
		}
		else
		{
			retval1 = eRETVAL_FAILURE;
		}
	}
	return outcome1.GetValue(0);
}

// Generate the next duration
float cQaidaControl :: GenerateNextDuration(tBundle2D* PDFDuration)
{
	unsigned int retval2 = eRETVAL_UNDEFINED;
	c1DList outcome2;
	t2DNode* TempNode = NULL;
	int counter = 0;
	
	while ((retval2 != eRETVAL_SUCCESS) && (counter < 3))
	{
		//Update counter
		counter++;
		
		// Roll for duration
		retval2 = PDFDuration->Roll(&outcome2, NULL);
		
		// If a duration was predicted, screen the duration
		if (retval2 & eRETVAL_PREDICTION_MADE)
		{
			TempNode = ThemeDurations.Find(&outcome2);
			
			if (TempNode != NULL)
			{
				retval2 = eRETVAL_SUCCESS;
			}
			else
			{
				// Check for half the duration
				outcome2.SetValue(0, outcome2.GetValue(0)*2);
				TempNode = NULL;
				TempNode = ThemeDurations.Find(&outcome2);
				
				if (TempNode != NULL)
				{
					retval2 = eRETVAL_SUCCESS;
				}
				else
				{
					// Check for twice the duration
					outcome2.SetValue(0, outcome2.GetValue(0)/2);
					TempNode = NULL;
					TempNode = ThemeDurations.Find(&outcome2);
					if (TempNode != NULL)
					{
						retval2 = eRETVAL_SUCCESS;
					}
					else
					{
						retval2 = eRETVAL_FAILURE;
					}
				}
			}
		}
		else
		{
			retval2 = eRETVAL_FAILURE;
		}
	}
	
	return outcome2.GetValue(0);
}

// Build a phrase using the note that was generated and then build its "closed" version as well
unsigned int cQaidaControl :: BuildPhrase(tBundle2D* PDFStroke, tBundle2D* PDFDuration)
{
	c1DList Vector(NULL, NULL, 0);
	float duration = 0;
	float stroke = 0;
	float strokeClosed = 0;
	
	// Check if the last phrase is complete. If yes, then reset the Phrase.
	if (PhraseLength >= ThemeLength)
	{
		// Reset phrase
		ResetPhrase();
	}
	
	duration = GenerateNextDuration(PDFDuration);
	stroke = GenerateNextStroke(PDFStroke);
	strokeClosed = GetClosed((unsigned int)stroke);
	
	PhraseLength += 4.0/duration;
	
	if (PhraseLength <= ThemeLength)
	{
		// Don't do anything.. just continue.
	}
	else
	{
		// The Phrase is longer than the duration
		// Find out by how much and force fit that duration in

		duration = ThemeLength - (PhraseLength - 4.0/duration);
		
		// Set PhraseLength equal to ThemeLength
		PhraseLength = ThemeLength;
	}
	
	Vector.Append(duration);
	Vector.Append(stroke);
	Phrase.Append(&Vector);
	Vector.DeleteAll();
	
	Vector.Append(duration);
	Vector.Append(strokeClosed);
	ClosedPhrase.Append(&Vector);
	Vector.DeleteAll();
	
	return eRETVAL_SUCCESS;
}

//Load the current phrase in to the Duration and Strokes vectors so that it can be played.
unsigned int cQaidaControl :: PlayPhrase(c1DList* DurationVector, c1DList* StrokeVector)
{
	t2DNode* TempNode = NULL;
	unsigned int RetVal = eRETVAL_UNDEFINED;
	
	if(Phrase.FirstNode != NULL)
	{
		for (TempNode = Phrase.FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			DurationVector->Append(TempNode->Vector.GetValue(0));
			StrokeVector->Append(TempNode->Vector.GetValue(1));
		}
		RetVal = eRETVAL_SUCCESS;
	}
	else
	{
		RetVal = eRETVAL_FAILURE;
	}
	
	return RetVal;
}

// Plays the theme and Phrases alternately to keep the Qaida pattern
// NOTE: Play phrase does not advance CycleControl so there you have to do it externally
unsigned int cQaidaControl :: PlayQaida(c1DList* DurationVector, c1DList* StrokeVector)
{
	t2DNode* TempNode = NULL;
	unsigned int RetVal = eRETVAL_UNDEFINED;
	
	switch (CycleControl) {
		case 0:		// Play Variation
			
			if(Phrase.FirstNode != NULL)
			{
				for (TempNode = Phrase.FirstNode; TempNode != NULL; TempNode = TempNode->Right)
				{
					DurationVector->Append(TempNode->Vector.GetValue(0));
					StrokeVector->Append(TempNode->Vector.GetValue(1));
				}
				RetVal = eRETVAL_SUCCESS;
			}
			else
			{
				RetVal = eRETVAL_FAILURE;
			}
			
			break;
			
		case 1:		// Play Theme
			
			if(Theme.FirstNode != NULL)
			{
				for (TempNode = Theme.FirstNode; TempNode != NULL; TempNode = TempNode->Right)
				{
					DurationVector->Append(TempNode->Vector.GetValue(0));
					StrokeVector->Append(TempNode->Vector.GetValue(1));
				}
				RetVal = eRETVAL_SUCCESS;
			}
			else
			{
				RetVal = eRETVAL_FAILURE;
			}
			
			break;
			
		case 2:		// Play ClosedVariation
			
			if(ClosedPhrase.FirstNode != NULL)
			{
				for (TempNode = ClosedPhrase.FirstNode; TempNode != NULL; TempNode = TempNode->Right)
				{
					DurationVector->Append(TempNode->Vector.GetValue(0));
					StrokeVector->Append(TempNode->Vector.GetValue(1));
				}
				RetVal = eRETVAL_SUCCESS;
			}
			else
			{
				RetVal = eRETVAL_FAILURE;
			}
			
			break;
			
		case 3:		// Play Theme
			
			if(Theme.FirstNode != NULL)
			{
				for (TempNode = Theme.FirstNode; TempNode != NULL; TempNode = TempNode->Right)
				{
					DurationVector->Append(TempNode->Vector.GetValue(0));
					StrokeVector->Append(TempNode->Vector.GetValue(1));
				}
				RetVal = eRETVAL_SUCCESS;
			}
			else
			{
				RetVal = eRETVAL_FAILURE;
			}
			
			break;
			
		default:
			break;
	}
	
	return RetVal;
	
}

unsigned int cQaidaControl :: MakePhrase(tBundle2D* PDFStroke, tBundle2D* PDFDuration, double* PositionInSong)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	c1DList Vector(NULL, NULL, 0);
	float duration = 0;
	float stroke = 0;
	float strokeClosed = 0;
	
	// Only if Cycle Control is in the Phrase generation stage,
	// make the phrase otherwise just continue without doing anything.
	if (CycleControl == 0)
	{
		// Has the theme been read in? 
		if (ThemeStrokes.FirstNode == NULL || ThemeLength == 0)
		{
			RetVal = eRETVAL_FAILURE;
			return RetVal;
		}
		
		// Now that we're ready to start:
		// Generate stroke
		stroke = GenerateNextStroke(PDFStroke);
		// Generate duration
		duration = GenerateNextDuration(PDFDuration);
		// Get the Closed alternative
		strokeClosed = GetClosed((unsigned int)stroke);
		
		if ((PhraseLength + 4.0/duration) <= ThemeLength)
		{
			Vector.Append(duration);
			Vector.Append(stroke);
			Phrase.Append(&Vector);
			Vector.DeleteAll();
			
			Vector.Append(duration);
			Vector.Append(strokeClosed);
			ClosedPhrase.Append(&Vector);
			Vector.DeleteAll();
				
			// Advance the song position by the duration of the stroke
			*PositionInSong += 4.0/duration;
			PhraseLength += 4.0/duration;
		}
		else 
		{
			// Add the stroke, but set the duration to the difference
			duration = ThemeLength - PhraseLength;
			
			if (duration > 0.25)
			{
				// Add the stroke
				Vector.Append(duration);
				Vector.Append(stroke);
				Phrase.Append(&Vector);
				Vector.DeleteAll();
				
				Vector.Append(duration);
				Vector.Append(strokeClosed);
				ClosedPhrase.Append(&Vector);
				Vector.DeleteAll();
			}
			
			// Advance the song position by the duration of the stroke
			if (duration != 0)
			{
				// Since duration in this case is already converted
				// to the right form.
				*PositionInSong += duration;
				PhraseLength = ThemeLength;
			}
			else
			{
				RetVal = eRETVAL_FAILURE;
				return RetVal;
			}
		}
	}
	
	RetVal = eRETVAL_SUCCESS;
	return RetVal;
}

void cQaidaControl :: FillStrokeMatrix(void)
{
	// Fill in the normal strokes
	for (int i = 0; i < MAX_STROKES; i++)
	{
		StrokeMatrix[i][0] = i+1;
	}
	
	// Fill in the reduced set 1
	StrokeMatrix[0][1] = 1;
	StrokeMatrix[1][1] = 2;
	StrokeMatrix[2][1] = 3;
	StrokeMatrix[3][1] = 4;
	StrokeMatrix[4][1] = 5;
	StrokeMatrix[5][1] = 6;
	StrokeMatrix[6][1] = 7;
	StrokeMatrix[7][1] = 8;
	StrokeMatrix[8][1] = 20;
	StrokeMatrix[9][1] = 20;
	StrokeMatrix[10][1] = 11;
	StrokeMatrix[11][1] = 20;
	StrokeMatrix[12][1] = 13;
	StrokeMatrix[13][1] = 14;
	StrokeMatrix[14][1] = 15;
	StrokeMatrix[15][1] = 16;
	StrokeMatrix[16][1] = 17;
	StrokeMatrix[17][1] = 18;
	StrokeMatrix[18][1] = 20;
	StrokeMatrix[19][1] = 20;
	StrokeMatrix[20][1] = 22;
	StrokeMatrix[21][1] = 22;
	StrokeMatrix[22][1] = 22;
	StrokeMatrix[23][1] = 24;
	StrokeMatrix[24][1] = 25;
	StrokeMatrix[25][1] = 26;
	StrokeMatrix[26][1] = 27;
	StrokeMatrix[27][1] = 28;
	StrokeMatrix[28][1] = 29;
	StrokeMatrix[29][1] = 30;
	StrokeMatrix[30][1] = 31;
	StrokeMatrix[31][1] = 32;
	StrokeMatrix[32][1] = 33;
	StrokeMatrix[33][1] = 17;
	StrokeMatrix[34][1] = 35;
	StrokeMatrix[35][1] = 17;
	StrokeMatrix[36][1] = 31;
	StrokeMatrix[37][1] = 38;
	StrokeMatrix[38][1] = 39;
	StrokeMatrix[39][1] = 40;
	StrokeMatrix[40][1] = 41;
	StrokeMatrix[41][1] = 42;
	
	// Fill in reduced set 2
	StrokeMatrix[0][2] = 31;
	StrokeMatrix[1][2] = 2;
	StrokeMatrix[2][2] = 8;
	StrokeMatrix[3][2] = 8;
	StrokeMatrix[4][2] = 5;
	StrokeMatrix[5][2] = 2;
	StrokeMatrix[6][2] = 7;
	StrokeMatrix[7][2] = 8;
	StrokeMatrix[8][2] = 20;
	StrokeMatrix[9][2] = 20;
	StrokeMatrix[10][2] = 22;
	StrokeMatrix[11][2] = 20;
	StrokeMatrix[12][2] = 25;
	StrokeMatrix[13][2] = 31;
	StrokeMatrix[14][2] = 25;
	StrokeMatrix[15][2] = 16;
	StrokeMatrix[16][2] = 17;
	StrokeMatrix[17][2] = 7;
	StrokeMatrix[18][2] = 20;
	StrokeMatrix[19][2] = 20;
	StrokeMatrix[20][2] = 22;
	StrokeMatrix[21][2] = 22;
	StrokeMatrix[22][2] = 22;
	StrokeMatrix[23][2] = 22;
	StrokeMatrix[24][2] = 25;
	StrokeMatrix[25][2] = 31;
	StrokeMatrix[26][2] = 31;
	StrokeMatrix[27][2] = 31;
	StrokeMatrix[28][2] = 31;
	StrokeMatrix[29][2] = 31;
	StrokeMatrix[30][2] = 31;
	StrokeMatrix[31][2] = 31;
	StrokeMatrix[32][2] = 31;
	StrokeMatrix[33][2] = 17;
	StrokeMatrix[34][2] = 5;
	StrokeMatrix[35][2] = 17;
	StrokeMatrix[36][2] = 31;
	StrokeMatrix[37][2] = 25;
	StrokeMatrix[38][2] = 2;
	StrokeMatrix[39][2] = 2;
	StrokeMatrix[40][2] = 31;
	StrokeMatrix[41][2] = 42;
	
	// Fill in reduced set 3
	StrokeMatrix[0][3] = 1;
	StrokeMatrix[1][3] = 0;
	StrokeMatrix[2][3] = 0;
	StrokeMatrix[3][3] = 0;
	StrokeMatrix[4][3] = 1;
	StrokeMatrix[5][3] = 0;
	StrokeMatrix[6][3] = 0;
	StrokeMatrix[7][3] = 0;
	StrokeMatrix[8][3] = 0;
	StrokeMatrix[9][3] = 0;
	StrokeMatrix[10][3] = 1;
	StrokeMatrix[11][3] = 0;
	StrokeMatrix[12][3] = 0;
	StrokeMatrix[13][3] = 1;
	StrokeMatrix[14][3] = 0;
	StrokeMatrix[15][3] = 0;
	StrokeMatrix[16][3] = 0;
	StrokeMatrix[17][3] = 0;
	StrokeMatrix[18][3] = 0;
	StrokeMatrix[19][3] = 0;
	StrokeMatrix[20][3] = 1;
	StrokeMatrix[21][3] = 1;
	StrokeMatrix[22][3] = 1;
	StrokeMatrix[23][3] = 1;
	StrokeMatrix[24][3] = 0;
	StrokeMatrix[25][3] = 1;
	StrokeMatrix[26][3] = 1;
	StrokeMatrix[27][3] = 1;
	StrokeMatrix[28][3] = 1;
	StrokeMatrix[29][3] = 1;
	StrokeMatrix[30][3] = 1;
	StrokeMatrix[31][3] = 1;
	StrokeMatrix[32][3] = 1;
	StrokeMatrix[33][3] = 0;
	StrokeMatrix[34][3] = 1;
	StrokeMatrix[35][3] = 0;
	StrokeMatrix[36][3] = 1;
	StrokeMatrix[37][3] = 0;
	StrokeMatrix[38][3] = 0;
	StrokeMatrix[39][3] = 0;
	StrokeMatrix[40][3] = 1;
	StrokeMatrix[41][3] = 42;
}

void cQaidaControl :: FillOpenClosed()
{
	// Fill in the normal strokes
	for (int i = 0; i < MAX_STROKES; i++)
	{
		OpenClosed[i][0] = i+1;
	}
	
	// Fill in the Closed Equivalents
	OpenClosed[0][1] = 1;
	OpenClosed[1][1] = 16;
	OpenClosed[2][1] = 3;
	OpenClosed[3][1] = 4;
	OpenClosed[4][1] = 1;
	OpenClosed[5][1] = 6;
	OpenClosed[6][1] = 17;
	OpenClosed[7][1] = 25;
	OpenClosed[8][1] = 9;
	OpenClosed[9][1] = 10;
	OpenClosed[10][1] = 11;
	OpenClosed[11][1] = 12;
	OpenClosed[12][1] = 13;
	OpenClosed[13][1] = 14;
	OpenClosed[14][1] = 15;
	OpenClosed[15][1] = 16;
	OpenClosed[16][1] = 17;
	OpenClosed[17][1] = 18;
	OpenClosed[18][1] = 19;
	OpenClosed[19][1] = 22;
	OpenClosed[20][1] = 21;
	OpenClosed[21][1] = 22;
	OpenClosed[22][1] = 23;
	OpenClosed[23][1] = 24;
	OpenClosed[24][1] = 25;
	OpenClosed[25][1] = 26;
	OpenClosed[26][1] = 27;
	OpenClosed[27][1] = 28;
	OpenClosed[28][1] = 29;
	OpenClosed[29][1] = 30;
	OpenClosed[30][1] = 31;
	OpenClosed[31][1] = 32;
	OpenClosed[32][1] = 33;
	OpenClosed[33][1] = 34;
	OpenClosed[34][1] = 35;
	OpenClosed[35][1] = 36;
	OpenClosed[36][1] = 37;
	OpenClosed[37][1] = 38;
	OpenClosed[38][1] = 39;
	OpenClosed[39][1] = 40;
	OpenClosed[40][1] = 41;
	OpenClosed[41][1] = 42; 
}

unsigned int cQaidaControl :: GetReduced(unsigned int TablaStroke, unsigned int ReductionSet)
{
	unsigned int ReducedStroke = 0;
	
	for(int i = 0; i < MAX_STROKES; i++)
	{
		if (TablaStroke == (unsigned int)i+1)
		{
			ReducedStroke =  StrokeMatrix[i][ReductionSet];
		}
	}
	
	return ReducedStroke;
}

unsigned int cQaidaControl :: GetClosed(unsigned int TablaStroke)
{
	unsigned int ClosedStroke = 0;
	if (TablaStroke < MAX_STROKES)
	{
		ClosedStroke = OpenClosed[TablaStroke-1][1];
		return ClosedStroke;
	}
	else {
		return 0;
	}
}

void cQaidaControl :: AdvanceCycleControl()
{
	if ((CycleControl >= 0) && (CycleControl < 3))
	{
		CycleControl++;
	}
	else
	{
		CycleControl = 0;
	}
}

unsigned int cQaidaControl :: GetCycleControl()
{
	return CycleControl;
}

//Function to set the deviation value - the entropy distance limit
// Greater deviation adds more randomness
void cQaidaControl :: SetDeviation(double InputDeviation)
{
	if (InputDeviation > 0)
	{
		Deviation = InputDeviation;
	}
	else {
		// Do nothing
	}

}

// Strictness controls how often the model will violate the Deviation limit
// Less strictness means the model can violate the entropy bound more often.
void cQaidaControl :: SetStrictness(unsigned int InputStrictness)
{
	if (InputStrictness >= 0)
	{
		Strictness = InputStrictness;
	}
	else {
		// Do nothing
	}
}
	