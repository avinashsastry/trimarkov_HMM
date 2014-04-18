/*
 *  markov_qaidaControl.h
 *  markov
 *
 *  Created by Avinash Sastry on 4/23/10.
 *  Copyright 2010 Georgia Institute of Technology. All rights reserved.
 *
 */

class cQaidaControl {
	private:
		// Class components
		tBundle2D ThemeStrokes;
		tBundle2D ThemeDurations;
	
		tBundle2D Theme;	// 2-dimension vector list in the form of <duration stroke>
		tBundle2D ClosedTheme;
		float ThemeLength;
	
		tBundle2D Phrase; // 2-dimension vector list in the form <duration stroke>
		tBundle2D ClosedPhrase;
		float PhraseLength;
		
		unsigned int CycleControl;
		unsigned int StrokeMatrix[MAX_STROKES][4];
		unsigned int OpenClosed[MAX_STROKES][2];
	
		double Deviation;
		double Strictness;
	
		friend class tBundle2D;
	
	public:
		// Class functions
		cQaidaControl();
		~cQaidaControl();
		
		// Get Theme length - returns theme length
		float GetThemeLength();
	
		// Get Phrase length - returns phrase length
		float GetPhraseLength();
		
		// Get first node of the phrase bundle
		t2DNode* GetPhraseFirstNode();
	
		// Add stroke - add a new stroke to the list of usable strokes
		unsigned int AddStroke(float InputStroke);
	
		// Add duration - add a new duration to the list of usable durations
		unsigned int AddDuration(float InputDuration);
	
		// RecordTheme - Know when to start recording the theme and record till it gets the stop command
		unsigned int RecordTheme(float InputStroke, float InputDuration);
		
		// Reset Theme
		unsigned int ResetTheme(void);
		
		// Reset Phrase
		unsigned int ResetPhrase(void);
	
		// Generate the next note - returns the generated value as a float
		float GenerateNextStroke(tBundle2D* PDFStroke);
	
		// Generate the next duration - returns the generated value as a float
		float GenerateNextDuration(tBundle2D* PDFDuration);
	
		// Build phrase using the note that was generated
		unsigned int BuildPhrase(tBundle2D* PDFStroke, tBundle2D* PDFDuration);
		
		// Plays the theme and Phrases alternately to keep the Qaida pattern
		// NOTE: Play phrase advances CycleControl so there is no need to do it externally.
		unsigned int PlayQaida(c1DList* DurationVector, c1DList* StrokeVector);
	
		unsigned int PlayPhrase(c1DList* DurationVector, c1DList* StrokeVector);
	
		// Make phrase
		unsigned int MakePhrase(tBundle2D* PDFStroke, tBundle2D* PDFDuration, double* PositionInSong);
	
		// Function to load the stroke mapping into the class
		void FillStrokeMatrix(void);
	
		// Function to load the stroke mapping for Open vs. Closed strokes
		void FillOpenClosed(void);
	
		// Function to get the reduced type
		unsigned int GetReduced(unsigned int Stroke, unsigned int ReductionSet);
		
		//Function to get the closed version of a stroke
		unsigned int GetClosed(unsigned int TablaStroke);
	
		// Advance CycleControl by 1 step
		void AdvanceCycleControl();
	
		unsigned int GetCycleControl();
		
		//Function to set the deviation value - the entropy distance limit
		// Greater deviation adds more randomness
		void SetDeviation(double InputDeviation);
		
		// Strictness controls how often the model will violate the Deviation limit
		// Less strictness means the model can violate the entropy bound more often.
		void SetStrictness(unsigned int InputStrictness);
};