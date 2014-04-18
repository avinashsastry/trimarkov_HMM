/*
 *  markov_ViewPoint.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/24/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

//	A view-point with a stream of correspnding symbols and the corresponding trie
class cViewPoint
	{
		
	public:
		
		/*
			A viewpoint consists of the following:
		 
			1. Tries
				a. Long Term Trie
				b. Short Term Trie
		 
			2. Streams for the tries
				a. Input stream for the LT
				b. Search Stream for the ST
			
			3. Tracker for each trie
				a. Tracker Long Term
				b. Tracker Short Term
			
			4. PDFs to store the last predictions
				a. PrevPDF Combined
				b. PrevPDF ST
		 
			5. Entropies for the prev predictions
				a. EntropyPrior
				b. EntropyLongTerm
				c. EntropyShortTerm
				d. EntropyCombined
		*/
		
		// Tries
		tBundle2D			Trie;
		tBundle2D			TrieShortTerm;

		// Streams
		c1DVectorList		Stream;
		c1DVectorList		SearchStream;
		
		// Trackers
		tTracker			Tracker;
		tTracker			TrackerShortTerm;
		
		// PDFs
		tBundle2D*			PrevPDF;
		tBundle2D*			PrevPDFShortTerm;
		tBundle2D*			PrevPDFLongTerm;
		
		// Control Variables 
		bool		UseShortTermModel;
		
		// Entropies
		double EntropyPrior;
		double EntropyLongTerm;
		double EntropyShortTerm;
		double EntropyCombined;
		
		cViewPoint();
		
		cViewPoint(unsigned int Dim);
		
		// Function to control whether the short Term model will be used for prediction or not
		void SetUseShortTermModel(bool TrueOrFalse);
		
		//	Wrapper for tBundle2D :: GrowAll(...)
		int	GrowAll(tTracker * Tracker, void *x);
		
		//	Wrapper for tBundle2D :: PredictBest(...)
		int	PredictBest(c1DList *Outcome, void *x);
		int	PredictBest(tBundle2D *Outcome, void *x);
		
		//	AddToStream(input1);
		//  Adds to the input stream of dimension 1
		//  Calls GrowAll() on the Trie
		int AddToStream(float input1, void *x);
		
		//	AddToStream(input1, input2);
		//  Adds to the input stream of dimension 2
		//  Calls GrowAll() on the Trie
		int AddToStream(float input1, float input2, void *x);
		
		// AddToSearchStream(input1)
		// 1. Adds input value(s) to search stream
		// 2. Calls GrowAll() on the short term trie
		// 3. Crossvalidates the current input with the previous prediction
		int AddToSearchStream(float input1, void *x);
		
		// AddToSearchStream(input1, input2)
		// 1. Adds input value(s) to search stream
		// 2. Calls GrowAll() on the short term trie
		// 3. Crossvalidates the current input with the previous prediction
		int AddToSearchStream(float input1, float input2, void *x);
		
		//	ResetViewpoint();
		int ResetViewpoint(int ResetType);
		
		// Predict Outcome based on the state of its search stream
		int PredictNext(void *x);
		
		int PredictNextKN(void *x);
		
		/**********************************************************************
		 
		 Special Functions - Below are functions that may be specific to some 
		 viewpoints. These functions must be used in the right context so use
		 carefully.
		 
		**********************************************************************/
		
		// PredictStrokePIS()
		// Predicts stroke values given the position of the next event in the song
		// Works only with StrokePIS viewpoint
		int PredictStrokePIS(float PositionInSong, void* x);
	};
