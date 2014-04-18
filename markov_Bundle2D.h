/*
 *  markov_Bundle2D.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/7/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */
#include "markov_classes.h"
#include <fstream>
using namespace std;

//	Bundle of 2D Nodes - collection of multiple branches sprouting out of a single one
class tBundle2D
	{
		
	private:
		
		t2DNode				* Top;			// Link to the parent bundle (of 2D nodes)
		t2DNode				* FirstNode;	// Link to the first 2D node
		t2DNode				* LastNode;		// Link to the last 2D node
		unsigned short int	Length;			// Number of 2D nodes (horizontal) in the bundle
		unsigned long int	TotalCount;		// Sum of 'Count' of all (horizontal) 2D nodes in the bundle - used to update probabilities
		unsigned long int	TotalSize;		// Total number of 2D nodes in the branch (including sub-branches) headed by the bundle - used to keep a rough track of memory usage
		
		friend class t2DNode;
		friend class t2DNodeHidden;
		friend class tBundle2DHidden;
		friend class cViewPoint;
		friend class cViewPointHidden;
		friend class cQaidaControl;
		friend class _markov;
		
	public:
		
		//	Constructor
		tBundle2D();
		
		//	Constructor
		tBundle2D(t2DNode * Top, t2DNode *FirstNode, t2DNode *LastNode, unsigned short int Length, unsigned long int TotalCount, unsigned long int TotalSize);
		
		//	Destructor
		~tBundle2D();
		
		//	GetVector - return the list at given index
		c1DList *GetVector(unsigned int Index);
		
		//	GetValue - return the value at given DimIndex of the vector at the given VectorIndex
		float GetValue(unsigned int VectorIndex, unsigned int DimIndex);
				
		// GetProb - return the probability of a node at a given Index
		float GetProb(unsigned int Index);
		
		// GetEntropy - return the negative log of the probability of a node at a given Index
		float GetEntropy(unsigned int Index);
				
		//	GetLength - return the length (size) of the bundle
		unsigned int GetLength(void);
		
		//	Append value at the right end
		int	Append(c1DList *Vector);
		
		//	Insert value at its sorted position if it has not been inserted earlier (no duplicates)
		int	Insert(c1DList *Vector);
		
		//	Find the node containing the value passed in & return a pointer to the node
		t2DNode *Find(c1DList *Vector);
		
		// Deletes the bundle and the nodes below it.
		void DeleteBundle(void);
		
		unsigned int MergeOpenClosed(tBundle2D* SplitPDFReducedStrokePIS1, cQaidaControl* QaidaController);
		
		/***************************************
		 Learning functions
		 ***************************************/
		
		//	Iteratively grow the tree (bundle) for all the nodes in the stream starting from the input StreamHead
		//	Calls Grow()
		int	GrowAll(c1DVectorList * InputStream, tTracker * Tracker, void *x = NULL);
		
		//	Grow the tree folowing the values in the stream, pointed to by StreamHead;
		//	Recursively finds the branches corresponding to the successive values in the stream,
		//	& inserts a new leaf at the last node, or updates the bundle at the leaf if it is already present.
		//	Calls Update() routine
		//	Recursively calls itself (Grow())
		int	Grow(c1DVectorNode *StreamHead, tTracker * Tracker, void *x = NULL);
		
		//	Update the count & probability of the input value after inserting/finding it at its sorted position
		//	Called by Grow() routine
		int	Update(c1DList *Vector, tTracker * Tracker, void *x = NULL);
		
		/***************************************
		 Prediction functions
		 ***************************************/
		
		//	Make the best prediction from the model considering all the nodes in the stream starting from the input StreamHead & going right
		//	For example, if stream is ABCDE, try to predict based on ABCDE; if unsuccessful, try BCDE, CDE, DE & E successively.
		//	Calls Predict()
		int	PredictBest(c1DVectorList *InputStream, c1DList *Outcome, void *x);
		int	PredictBest(c1DVectorList *InputStream, tBundle2D *Outcome, void *x);

		//	Predict folowing the values in the stream, pointed to by StreamHead;
		//	Recursively finds the branches corresponding to the successive values in the stream,
		//	& rolls the dice at the last node.
		//	Calls Roll() routine
		//	Recursively calls itself (Predict())
		int	Predict(c1DVectorNode * StreamHead, c1DList *Outcome, void *x);
		int	Predict(c1DVectorNode * StreamHead, tBundle2D *Outcome, void *x);
		
		//this is for smoothing - equivalent to GrowAll() or PredictBest()
		int PredictSmoothBest( c1DVectorList* SearchStream, tBundle2D *Outcome, tTracker *Tracker, void*x); 
		
		// This is the inner function that recursively calls itself - equivalent to Grow()/Predict()
		int PredictSmooth( c1DVectorNode * StreamHead, tBundle2D *Outcome, void *x);
		
		//	Determine the prediction outcome based on the probability distribution under the input value after finding it at its sorted position
		//	'Roll the dice'
		//	Called by Predict() routine
		int	Roll(c1DList *Outcome, void *x);
		
		// Returns the Probability distribution at a given level in the tree
		// similar to Roll()
		int	ReturnPDF(tBundle2D *Outcome, void *x);
		
		// PredictKneserNey() - Uses KneserNey Smoothing
		unsigned int PredictKneserNey(c1DVectorList* SearchStream, tBundle2D *Accumulator, tTracker* Tracker, void*x);
		
		double KneserNey(t2DNode* PredictedSymbol, c1DVectorNode* StreamHead, tTracker* Tracker, void* x);
		
		// This cross-validates with the previous result
		double CrossValidate(c1DList* Vector, tTracker* Tracker, void* x);
		
		int Normalize(void);
		
		t2DNode* GetMaxProb(void);
		
		double GetSumProb(void);
		
		void MergePDF(tBundle2D* SecondBundle, double ShortTermWeight, double LongTermWeight);
		
		double TotalEntropy(tTracker* Tracker);
		
		unsigned int SplitPDF(tBundle2D* PDFVectorOne, tBundle2D* PDFVectorTwo);
		
		double RelativeEntropy(tTracker* Tracker);
		
		unsigned int ArithmeticCombination(tBundle2D* SecondBundle, tTracker* TrackerOne, tTracker* TrackerTwo, bool IsContinued);
		
		unsigned int GeometricCombination(tBundle2D* SecondBundle, tTracker* TrackerOne, tTracker* TrackerTwo, bool IsContinued);
		
		void MapToNotes(unsigned int StrokeMatrix[42][4], unsigned int ReductionSet);
		
		double EntropyDistance(double Probability1, double Probability2);
		
		double DistanceToMostLikelyNode(c1DList* PredictedNode);
 		
		/***************************************
		 Posting routines for debugging
		 ***************************************/
		
		//	Output the specified bundle as a list
		//	For debugging only; may impair performance
		void PostBundle(void *pCountOutlet, void *pProbOutlet);
		
		//	Output the whole trie as a sequence of lists corresponding to bundles
		//	For debugging only; may impair performance
		//	Recursively calls itself
		void PostTrie(void *pCountOutlet, void *pProbOutlet);
		
		//	Output the whole trie as a sequence of XML nodes so that
		//	we can use the Treeviz program to draw out the tree
		//	Recursively calls itself
		void PostTrieXML(ofstream &OutputFile, int InitialOrder);
	};
