/*
 *  markov_ViewpointHidden.h
 *  markov
 *
 *  Created by Avinash on 10/1/10.
 *  Copyright 2010 GTCMT. All rights reserved.
 *
 */

//	A view-point with a stream of correspnding symbols and the corresponding trie
class cViewPointHidden
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
		
	// Streams
	c1DVectorList			Stream;
	c1DVectorList			SymbolStream;
	c1DVectorList			SearchStream;
	
	// Trackers
	tTracker				Tracker;
	tTracker				TrackerShortTerm;
	
	// PDFs
	tBundle2DHidden*		PrevPDF;
	tBundle2DHidden*		PrevPDFShortTerm;
	tBundle2DHidden*		PrevPDFLongTerm;
	
	// Tries
	tBundle2DHidden			Trie;
	tBundle2DHidden			TrieShortTerm;
	
	
	// Control Variables 
	bool					UseShortTermModel;
	
	// Entropies
	double EntropyPrior;
	double EntropyLongTerm;
	double EntropyShortTerm;
	double EntropyCombined;
	
	// Data for Strokes Viewpoint
	float StrokePriors[NUM_STROKES];
	float StrokeMeans[NUM_STROKES][NUM_FEATURES];
	float StrokeVariance[NUM_FEATURES][NUM_FEATURES];
	float StrokeInverse[NUM_FEATURES][NUM_FEATURES];
	float StrokeDeterminant;
	
	cViewPointHidden();
	
	cViewPointHidden(unsigned int Dim);
	
	// Function to control whether the short Term model will be used for prediction or not
	void SetUseShortTermModel(bool TrueOrFalse);
	
	//	Wrapper for tBundle2D :: GrowAll(...)
	int	GrowAll(tTracker * Tracker, void *x);
	
	//	Wrapper for tBundle2D :: PredictBest(...)
	int	PredictBest(c1DList *Outcome, void *x);
	int	PredictBest(tBundle2DHidden *Outcome, void *x);
	
	//  Calls evaluateStream() on the search stream
	//  Returns the probability that the stream was
	//  produced by the model - for a discrete HMM model
	double EvaluateStream(void* x);
	
	//  Calls evaluateStream() on the search stream
	//  Returns the probability that the stream was
	//  produced by the model - for a continuous HMM model
	double EvaluateStreamContinuous(void *x);
	
	//  Function to evaluate the backward probability for a given 
	//  search stream with a given end state - somehow this doesn't make sense.
	//  Will fix.
	double GetBackwardProbability(c1DList* NodeVector, void * x);
	
	//  ForwardBackwardAlgorithm()
	//  Top level function to run the Fwd-Back Algo on
	//  the entire tree at every time step.
	double ForwardBackwardAlgorithm(void* x);
	
	// Calls DecodeStream() on the search stream
	// Returns the sequence of nodes that give the 
	// highest probability of the observed sequence.
	unsigned int DecodeStream(c1DVectorList *ResultStream ,void *x);
	
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
	
	// UpdatePriorsInPDF()
	// 1. Reads the top level of the Trie
	// 2. Creates a duplicate bundle and stores it in PrevPDF
	// 3. Assigns the prior probs and initializes the gaussians
	unsigned int UpdateNodesInPDF(double ForwardProb);
	
	//	ResetViewpoint();
	int ResetViewpoint(int ResetType);
	
	// Predict Outcome based on the state of its search stream
	int PredictNext(void *x);
	
	int PredictNextKN(void *x);
	
	//  AddFeaturesToSearchStream(float* FeatureVector, c1DList* DataToken)
	//  1. Creates a new vector node in the Search Stream.
	//  2. Adds the features to it.
	unsigned int AddFeaturesToSearchStream(float* FeatureVector, c1DList* DataToken);
	
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
