/*
 *  markov_Bundle2DHidden.h
 *  markov
 *
 *  Created by Avinash on 9/30/10.
 *  Copyright 2010 GTCMT. All rights reserved.
 *
 */

class tBundle2DHidden
{
	
private:
	
	t2DNodeHidden				* Top;			// Link to the parent bundle (of 2D nodes)
	t2DNodeHidden				* FirstNode;	// Link to the first 2D node
	t2DNodeHidden				* LastNode;		// Link to the last 2D node
	unsigned short int			Length;			// Number of 2D nodes (horizontal) in the bundle
	unsigned long int			TotalCount;		// Sum of 'Count' of all (horizontal) 2D nodes in the bundle - used to update probabilities
	unsigned long int			TotalSize;		// Total number of 2D nodes in the branch (including sub-branches) headed by the bundle - used to keep a rough track of memory usage
	
	friend class t2DNode;
	friend class t2DNodeHidden;
	friend class cViewPoint;
	friend class cViewPointHidden;
	friend class cQaidaControl;
	friend class _markov;
	
public:
	
	//	Constructor
	tBundle2DHidden();
	
	//	Constructor
	tBundle2DHidden(t2DNodeHidden * Top, t2DNodeHidden *FirstNode, t2DNodeHidden *LastNode, unsigned short int Length, unsigned long int TotalCount, unsigned long int TotalSize);
	
	//	Destructor
	~tBundle2DHidden();
	
	//	GetVector - return the list at given index
	c1DList *GetVector(unsigned int Index);
	
	//	GetValue - return the value at given DimIndex of the vector at the given VectorIndex
	float GetValue(unsigned int VectorIndex, unsigned int DimIndex);
	
	// GetProb - return the probability of a node at a given Index
	double GetProb(unsigned int Index);
	
	// GetEntropy - return the negative log of the probability of a node at a given Index
	double GetEntropy(unsigned int Index);
	
	//	GetLength - return the length (size) of the bundle
	unsigned int GetLength(void);
	
	//	Append value at the right end
	int	Append(c1DList *Vector);
	
	// Modified Append - Used in CreateDummyTree() in markov.cpp
	int Append(c1DList *Vector, float Prob, float* MeanVector, float* Variance, float* Inverse, float* Determinant);
	
	//	Insert value at its sorted position if it has not been inserted earlier (no duplicates)
	int	Insert(c1DList *Vector);
	
	//	Find the node containing the value passed in & return a pointer to the node
	t2DNodeHidden *Find(c1DList *Vector);
	
	// Deletes the bundle and the nodes below it.
	void DeleteBundle(void);

	
	/***************************************
	 Learning functions
	 ***************************************/
	
	//	Iteratively grow the tree (bundle) for all the nodes in the stream starting from the input StreamHead
	//	Calls Grow()
	int	GrowAll(c1DVectorList * StateStream, c1DVectorList * SymbolStream, tTracker * Tracker, void *x, float* StrokeMeans, float* StrokeVariance, float* StrokeInverse, float* StrokeDeterminant);
	
	//	Grow the tree folowing the values in the stream, pointed to by StreamHead;
	//	Recursively finds the branches corresponding to the successive values in the stream,
	//	& inserts a new leaf at the last node, or updates the bundle at the leaf if it is already present.
	//	Calls Update() routine
	//	Recursively calls itself (Grow())
	int	Grow(c1DVectorNode * StreamHead, c1DVectorNode * CurrSymbol, tTracker * Tracker, void *x, float* StrokeMeans, float* StrokeVariance, float* StrokeInverse, float* StrokeDeterminant);
	
	//	Update the count & probability of the input value after inserting/finding it at its sorted position
	//	Called by Grow() routine
	int	Update(c1DList *Vector, c1DList * SymbolVector, tTracker * Tracker, void *x, float* StrokeMeans, float* StrokeVariance, float* StrokeInverse, float* StrokeDeterminant);
	
	/***************************************
	 Prediction functions
	 ***************************************/
	
	//	Make the best prediction from the model considering all the nodes in the stream starting from the input StreamHead & going right
	//	For example, if stream is ABCDE, try to predict based on ABCDE; if unsuccessful, try BCDE, CDE, DE & E successively.
	//	Calls Predict()
	int	PredictBest(c1DVectorList *InputStream, c1DList *Outcome, void *x);
	int	PredictBest(c1DVectorList *InputStream, tBundle2DHidden *PDF, void *x);
	
	//	Predict folowing the values in the stream, pointed to by StreamHead;
	//	Recursively finds the branches corresponding to the successive values in the stream,
	//	& rolls the dice at the last node.
	//	Calls Roll() routine
	//	Recursively calls itself (Predict())
	int	Predict(c1DVectorNode * StreamHead, c1DList *Outcome, void *x);
	int	Predict(c1DVectorNode *StreamHead, tBundle2DHidden *PDF, void *x);
	
	//this is for smoothing - equivalent to GrowAll() or PredictBest()
	int PredictSmoothBest( c1DVectorList* SearchStream, tBundle2DHidden *Outcome, tTracker *Tracker, void*x); 
	
	// This is the inner function that recursively calls itself - equivalent to Grow()/Predict()
	int PredictSmooth( c1DVectorNode * StreamHead, tBundle2DHidden *Outcome, void *x);
	
	// Wrapper to allow Evaluate Stream to be called from cViewPointHidden class
	double EvaluateSearchStream(c1DVectorList *SearchStream, void *x);
	
	// Function to "evaluate" a given sequence - this means 
	// it will return the probability that the sequence could occur
	// based on the model
	// For a given search stream:
	// 1. Start from the first node in the top level bundle
	// 2. Find the probability of the first symbol in the stream
	// 3. Alpha = 1; Alpha *= a_ij*b_jk
	// 4. Proceed to next node and repeat	
	double EvaluateStream(c1DVectorNode *StreamHead, void *x);
	
	// Wrapper for EvaluateStreamContinuous()
	double EvaluateSearchStreamContinuous(c1DVectorList *SearchStream, c1DList* Vector, tTracker* Tracker, void *x);
	
	// Function to "evaluate" a given sequence of features for a 
	// continuous HMM system - this means 
	// it will return the probability that the sequence could occur
	// based on the model
	
	// NOTE: This includes Cross-validation
	
	// For a given search stream:
	// 1. Start from the first node in the top level bundle
	// 2. Find the probability of the first symbol in the stream
	// 3. Alpha = 1; Alpha *= a_ij*b_jk
	// 4. Proceed to next node and repeat
	double EvaluateStreamContinuous(c1DVectorNode *StreamHead, c1DList * Vector, void *x, tTracker* Tracker);
	
	// Wrapper for the viewpoint class for DecodeStream()
	unsigned int DecodeSearchStream(c1DVectorList* SearchStream, c1DVectorList* ResultStream, void* x);
	
	//  Viterbi Decoding - Function to "decode" a stream
	//  Given a search stream, the algorithm traverses/finds the most likely
	//  sequence of states that led to that search stream	
	//  Viterbi decoding will be used by the predict function to predict the next
	//  set of symbols
	unsigned int DecodeStream(c1DVectorNode* StreamHead, c1DVectorList* ResultStream,void* x);
	
	//  A duplicate of the EvaluateSearchStreamContinuous() function
	//  Returns the FULL forward probability of the sequence and 
	//  not the next symbol distribution
	double GetForwardProbabilityForStream(c1DVectorList *SearchStream, c1DList* Vector, tTracker* Tracker, void *x);
	
	//  This is practically a duplicate of the EvaluateStreamContinuous() function
	//  except that this function doesn't just return the "simple probability"
	//  or the prior prob for the last element in the sequence.
	//  It returns the FULL probability - a_ij*b_jk. 
	//  This is for use in the Forward-backward algorithm
	double GetForwardProbability(c1DVectorNode *StreamHead, void *x, tTracker* Tracker);
	
	//  A wrapper function to allow GetbackwardProbability()
	//  to be called in a viewpoint class
	double GetBackwardProbabilityForNode(c1DVectorList* SearchStream, c1DList* NodeVector,  tTracker* Tracker, void* x);
	
	//  GetBackwardProbability()
	//	Function to trace through the trie, and calculate backward
	//	probabilities
	double GetBackwardProbability(c1DVectorNode* StreamHead, tTracker* Tracker,void *x);
	
	//  ForwardBackwardAlgorithm()
	//  Function to calculate the forward and backward probabilities of the search stream
	//  at a point in time t, and adjust weights for that bundle
	//  Implements the Baum-Welsch Algorithm as given in Duda-Hart.
	unsigned int ForwardBackwardAlgorithm(tBundle2DHidden* RootBundle, c1DVectorList* SearchStream, c1DVectorList* FullSearchStream, c1DVectorNode* StreamHead, tTracker* Tracker, void* x);
	
	//  AdjustProbabilities()
	//  Propagation function for the ForwardBackwardAlgorithm()
	//  Calls it recursively on a bundle, and then on every bundle below it
	unsigned int AdjustProbabilities(tBundle2DHidden* RootBundle, c1DVectorList* SearchStream, unsigned int Order, tTracker* Tracker, void* x);
	
	
	//  Function to read in a feature vector and classify it into stroke
	//  The classified vector is written to c1dList* Outcome
	unsigned int ClassifyFeatures(float* FeatureVector, c1DList* Outcome);
	
	// Function to calculate the probability that this feature set belongs to the given category.
	// Should not be confused with Likelihood - likelihood is just an unscaled value.
	// GetProbability() uses Bayes theorem to calculate the probability.
	// Must return a value between 0 and 1;	
	double GetProbability(float* FeatureVector, c1DList* DataToken, tTracker* Tracker);
	
	//  Function to get the prior probability of a symbol from a bundle
	double GetSimpleProbability(c1DList* DataToken, tTracker* Tracker);
	
	//  Function to Roll() for a random state in a bundle.
	//  Used on the resulting PDf of a viewpoint after Predict()
	//  has been called.
	//  1. If the bundle is not NULL
	//  2. Compile a list of cumulative probabilties (0 to 1)
	//  3. Generate a random number between 0 and 1
	//  4. Place it in the corresponding bin in the list
	//  5. Return the state corresponding to the bin
	t2DNodeHidden* RollState(void *x);
	
	//	Determine the prediction outcome based on the probability distribution under the input value after finding it at its sorted position
	//	'Roll the dice'
	//	Called by Predict() routine
	int	Roll(c1DList *Outcome, void *x);
	
	// Returns the Probability distribution at a given level in the tree
	// similar to Roll()
	int	ReturnPDF(tBundle2DHidden *Outcome, void *x);
	
	// PredictKneserNey() - Uses KneserNey Smoothing
	unsigned int PredictKneserNey(c1DVectorList* SearchStream, tBundle2DHidden *Accumulator, tTracker* Tracker, void*x);
	
	double KneserNey(t2DNodeHidden* PredictedSymbol, c1DVectorNode* StreamHead, tTracker* Tracker, void* x);
	
	// This cross-validates with the previous result
	double CrossValidate(c1DList* Vector, tTracker* Tracker, void* x);
	
	int Normalize(void);
	
	t2DNodeHidden* GetMaxProb(void);
	
	double GetSumProb(void);
	
	void MergePDF(tBundle2DHidden* SecondBundle, double ShortTermWeight, double LongTermWeight);
	
	double TotalEntropy(tTracker* Tracker);
	
	unsigned int SplitPDF(tBundle2DHidden* PDFVectorOne, tBundle2DHidden* PDFVectorTwo);
	
	double RelativeEntropy(tTracker* Tracker);
	
	unsigned int ArithmeticCombination(tBundle2DHidden* SecondBundle, tTracker* TrackerOne, tTracker* TrackerTwo, bool IsContinued);
	
	unsigned int GeometricCombination(tBundle2DHidden* SecondBundle, tTracker* TrackerOne, tTracker* TrackerTwo, bool IsContinued);
	
	double EntropyDistance(double Probability1, double Probability2);
	
	double DistanceToMostLikelyNode(c1DList* PredictedNode);
	
	/***************************************
	 Posting routines for debugging
	 ***************************************/
	
	//	Output the specified bundle as a list
	//	For debugging only; may impair performance
	void PostBundle(void *pCountOutlet, void *pProbOutlet);
	
	// Output the vectors and probabilities in the bundle as prints in MAX window
	// That's all it does.
	void PrintBundle(void * x);
	
	//	Output the whole trie as a sequence of lists corresponding to bundles
	//	For debugging only; may impair performance
	//	Recursively calls itself
	void PostTrie(void *pCountOutlet, void *pProbOutlet);
	
	//	Output the whole trie as a sequence of XML nodes so that
	//	we can use the Treeviz program to draw out the tree
	//	Recursively calls itself
	void PostTrieXML(ofstream &OutputFile, int InitialOrder);
};
