/*
 *  markov_Bundle2DHidden.cpp
 *  markov
 *
 *  Created by Avinash on 9/30/10.
 *  Copyright 2010 GTCMT. All rights reserved.
 *
 */

#include "ext.h"
#include "ext_obex.h"

#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <fstream>


#include "markov_defines.h"
#include "markov_enums.h"
#include "markov_classes.h"
#include "markov_1DNode.h"
#include "markov_1DList.h"
#include "markov_1DVectorNode.h"
#include "markov_1DVectorList.h"
#include "markov_Bundle2D.h"
#include "markov_2DNode.h"
#include "markov_tracker.h"
#include "markov_qaidaControl.h"
#include "markov_Gaussian.h"
#include "markov_2DNodeHidden.h"
#include "markov_Bundle2DHidden.h"

using namespace std;

//	Constructor
tBundle2DHidden :: tBundle2DHidden()
{
	this->Top = NULL;
	this->FirstNode = NULL;
	this->LastNode = NULL;
	this->Length = 0;
	this->TotalCount = 0;
	this->TotalSize = 0;
};

//	Constructor
tBundle2DHidden :: tBundle2DHidden(t2DNodeHidden * Top = NULL, t2DNodeHidden *FirstNode = NULL, t2DNodeHidden *LastNode = NULL, unsigned short int Length = 0, unsigned long int TotalCount = 0, unsigned long int TotalSize = 0)
{
	this->Top = Top;
	this->FirstNode = FirstNode;
	this->LastNode = LastNode;
	this->Length = Length;
	this->TotalCount = TotalCount;
	this->TotalSize = TotalSize;
};

//	Destructor
tBundle2DHidden :: ~tBundle2DHidden()
{
	Top = NULL;
	while(Length>0)
	{
		FirstNode = LastNode->Left; // FirstNode used as TempNode
		TotalCount -= LastNode->Count;
		if (LastNode->Bottom != NULL)
		{
			TotalSize -= (LastNode->Bottom->TotalSize);
			delete LastNode->Bottom; // TODO: Any pass by value vs. pointer issues here?
			LastNode->Bottom = NULL;
		}
		delete LastNode; // TODO: Any pass by value vs. pointer issues here?
		Length--;
		//TODO: Try removing this check.. it should work without it. 
		if (FirstNode != NULL)
			FirstNode->Right = NULL;
		LastNode = FirstNode;
	}
};

//***************************************
#pragma mark -
#pragma mark Data Structure Functions
//***************************************

//	GetVector - return the list at given index
c1DList * tBundle2DHidden :: GetVector(unsigned int Index)
{
	t2DNodeHidden *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Index && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Right;
	}
	if(TempNode!=NULL)
		return &TempNode->Vector;
	else
		return NULL;
}


//	GetValue - return the value at given DimIndex of the vector at the given VectorIndex
float tBundle2DHidden :: GetValue(unsigned int VectorIndex, unsigned int DimIndex)
{
	t2DNodeHidden *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<VectorIndex && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Right;
	}
	if(TempNode!=NULL)
		return TempNode->Vector.GetValue(DimIndex);
	else
		return eRETVAL_FAILURE;
}

double tBundle2DHidden :: GetProb(unsigned int Index)
{
	t2DNodeHidden *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Index && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Right;
	}
	if(TempNode!=NULL)
		return TempNode->Prob;
	else
		return NULL;
}

// Return the entropy instead of the probability of the node at a given index
double tBundle2DHidden :: GetEntropy(unsigned int Index) 
{	
	t2DNodeHidden *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Index && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Right;
	}
	if(TempNode!=NULL)
	{
		if (TempNode->Prob != 0)
		{
			return -((log(TempNode->Prob))/(log(2)));
		}
		else {
			return -1;
		}
	}
	else
	{
		return NULL;
	}
}

//	GetLength - return the length (size) of the bundle
unsigned int tBundle2DHidden :: GetLength(void)
{
	return Length;
}

//	Append value at the right end
int	tBundle2DHidden :: Append(c1DList *Vector)
{		
	t2DNodeHidden	*NewNode = new t2DNodeHidden(LastNode, NULL, this, NULL, Vector, 0, 0);
	if(FirstNode == NULL)
	{
		FirstNode = NewNode;
		LastNode = NewNode;
	}
	else
	{
		LastNode->Right = NewNode;
		LastNode = NewNode;
	}
	Length++;
	TotalSize++;
	return 0;
}

int tBundle2DHidden :: Append(c1DList *Vector, float Prob, float* MeanVector, float* Variance, float* Inverse, float* Determinant)
{
	t2DNodeHidden	*NewNode = new t2DNodeHidden(LastNode, NULL, this, NULL, Vector, 0, Prob, MeanVector, Variance, Inverse, Determinant);
	if(FirstNode == NULL)
	{
		FirstNode = NewNode;
		LastNode = NewNode;
	}
	else
	{
		LastNode->Right = NewNode;
		LastNode = NewNode;
	}
	Length++;
	TotalSize++;
	return 0;
	
}

//	Insert value at its sorted position if it has not been inserted earlier (no duplicates)
int	tBundle2DHidden :: Insert(c1DList *Vector)
{
	if(FirstNode == NULL)
	{
		t2DNodeHidden	*NewNode = new t2DNodeHidden(NULL, NULL, this, NULL, Vector, 0, 0);
		FirstNode = NewNode;
		LastNode = NewNode;
		Length++;
		TotalSize++;
	}
	else
	{
		unsigned int Found = 0;
		t2DNodeHidden *TempNode1 = NULL;;
		t2DNodeHidden *TempNode2 = FirstNode;
		for(int i=0; i<(int)Length; i++)
		{
			if(TempNode2->Vector < Vector)
			{
				TempNode1 = TempNode2;
				TempNode2 = TempNode2->Right;
				continue;
			}
			else if(TempNode2->Vector == Vector)
			{
				Found = 1;
				break;
			}
			else
			{
				break;
			}
		}
		if(!Found)
		{
			t2DNodeHidden	*NewNode = new t2DNodeHidden(TempNode1, TempNode2, this, NULL, Vector, 0, 0);
			if(TempNode1 != NULL)	TempNode1->Right = NewNode;
			if(TempNode2 != NULL)	TempNode2->Left = NewNode;
			else					LastNode = NewNode;
			Length++;
			TotalSize++;
		}
	}
	return eRETVAL_SUCCESS;
}

//	Find the node containing the value passed in & return a pointer to the node
t2DNodeHidden *tBundle2DHidden :: Find(c1DList *Vector)
{
	t2DNodeHidden * TempNode = NULL;
	
	// If the bundle has nodes to begin with
	if (Length != 0)
	{
		TempNode = FirstNode;
		while(TempNode!=NULL)
		{
			if(TempNode->Vector == Vector)
			{
				break;
			}
			TempNode = TempNode->Right;
		}
	}
	
	return TempNode;
}

void tBundle2DHidden :: DeleteBundle(void)
{
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* TempNode2 = NULL;
	
	// Check if the bundle is empty
	if (FirstNode == NULL)
	{
		// If yes, then you're fine, no need to do anything
	}
	else
	{
		// If no, then delete each t2DNodeHidden - DeleteNode()
		// If the node has a bundle below it then call delete on that bundle
		// The recursion should carry it through
		TempNode = FirstNode;
		
		while(TempNode != NULL)
		{
			TempNode2 = TempNode->Right;
			if (TempNode->Bottom != NULL)
			{
				TotalSize -= TempNode->Bottom->TotalSize;
				delete TempNode->Bottom;
				TempNode->Bottom = NULL;
			}
			TotalCount -= TempNode->Count;
			delete TempNode;
			Length--;
			TotalSize--;
			TempNode = TempNode2;
			FirstNode = TempNode;
		}
		LastNode = NULL;
	}
}

//***************************************
#pragma mark -
#pragma mark Learning functions
//***************************************

// For the hidden markov model, GrowAll() has a slightly more complex function.
// There are two streams now to keep track of for each model - StateStream (called Stream) and the SymbolStream
// Each state is added to the trie in its respective place, and for every state, the emitted symbol also needs
// to be updated. This means that the symbol stream needs to be accessed by the two inner functions, 
// Grow() and Update()

int	tBundle2DHidden :: GrowAll(c1DVectorList * StateStream, c1DVectorList * SymbolStream, tTracker * Tracker, void *x, float* StrokeMeans, float* StrokeVariance, float* StrokeInverse, float* StrokeDeterminant)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	unsigned int depth = 0;
	
	if(StateStream != NULL)
	{
		c1DVectorNode *CurrNode = StateStream->LastNode;
		c1DVectorNode *CurrSymbol = SymbolStream->LastNode;
		
		// Append tracker node - Right now it appends one node at a time.
		// It has to make sure that the tracker is only as long as the max order.
		// so for a model of order 10, the tracker should have 11 nodes.
		if (Tracker->Length <= MAX_ORDER)
		{
			Tracker->Append();
		}
		
		while((CurrNode != NULL) && (CurrSymbol != NULL))
		{
			//Reset Order - Everytime grow() is called from here, it means a new series is starting
			Tracker->ResetOrder();
			
			//Update NumTokens - the variable 'depth' keeps track of what order we're dealing with
			//Since every call to grow() denotes a new series, the NumTokens for that order must be 
			//updated accordingly
			Tracker->UpdateNumTokens(depth);
			// POST((t_object *x), "	GrowAll: Update NumTokens(%d): %d", depth, Tracker->GetNumTokens(depth));
			
			RetVal |= Grow(CurrNode, CurrSymbol, Tracker, x, StrokeMeans, StrokeVariance, StrokeInverse, StrokeDeterminant);
			CurrNode = CurrNode->Prev;
			CurrSymbol = CurrSymbol->Prev;
			
			depth++;
		}
	}
	else
	{
		RetVal = eRETVAL_FAILURE;
	}
	return RetVal;
}

//	Grow the tree folowing the values in the stream, pointed to by StreamHead;
//	Recursively finds the branches corresponding to the successive values in the stream,
//	& inserts a new leaf at the last node, or updates the bundle at the leaf if it is already present.
//	Calls Update() routine
//	Recursively calls itself (Grow())
int	tBundle2DHidden :: Grow(c1DVectorNode * StreamHead, c1DVectorNode * CurrSymbol, tTracker * Tracker, void *x, float* StrokeMeans, float* StrokeVariance, float* StrokeInverse, float* StrokeDeterminant)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	t2DNodeHidden * TempNode;
	
	//	If NULL pointer passed in, return failure
	if(StreamHead == NULL || CurrSymbol == NULL)
	{
		RetVal = eRETVAL_FAILURE;
	}
	else
	{
		//	If last node in the stream, update & return
		if(StreamHead->Next == NULL)
		{
			RetVal = Update(&StreamHead->Vector, &CurrSymbol->Vector, Tracker, x, StrokeMeans, StrokeVariance, StrokeInverse, StrokeDeterminant);
		}
		//	Else, find the corresponding node & grow a branch below it
		else
		{
			TempNode = Find(&StreamHead->Vector);
			//	If no node has the value, by chance, introduce the value
			//	This case doesn't happen usually, as the node with the value should ideally have been created beforehand
			if(TempNode == NULL)
			{
				Update(&StreamHead->Vector, &CurrSymbol->Vector, Tracker, x, StrokeMeans, StrokeVariance, StrokeInverse, StrokeDeterminant);
				TempNode = Find(&StreamHead->Vector);
			}
			// This deals with cases like A-A-B or longer
			// Existing nodes like the second '1' must also be counted
			// TODO: Is this required? ANS: NO. Not req. Keep this commented. Or delete it.
			else 
			{
				// Update(&StreamHead->Vector, Tracker, x);
			}
			
			//	If the node doesn't have a branch below it, create the branch
			if(TempNode->Bottom == NULL)
			{
				tBundle2DHidden * NewBundle = new tBundle2DHidden(TempNode, NULL, NULL, 0, 0, 0);
				TempNode->Bottom = NewBundle;
				RetVal = eRETVAL_BRANCH_CREATED;
			}
			else
			{
				RetVal = eRETVAL_BRANCH_UPDATED;
			}
			//	Continue growing down till the end of stream is reached
			
			// Keep track of what order n-gram the next node will be part of so that
			// NumTypes and EscapeProb for that order can be updated
			Tracker->UpdateOrder();
			// POST((t_object *) x, "Order of grow updated: %d", Tracker->GetOrder());
			
			TotalSize -= TempNode->Bottom->TotalSize;
			TempNode->Bottom->Grow(StreamHead->Next, CurrSymbol->Next, Tracker, x, StrokeMeans, StrokeVariance, StrokeInverse, StrokeDeterminant);
			TotalSize += TempNode->Bottom->TotalSize;
		}
	}			
	return	RetVal;
}

//	Update the count & probability of the input value after inserting/finding it at its sorted position
//	Also update the probability of the symbol emitted by that node.
//	Called by Grow() routine
int	tBundle2DHidden :: Update(c1DList *Vector, c1DList * SymbolVector, tTracker * Tracker, void *x, float* StrokeMeans, float* StrokeVariance, float* StrokeInverse, float* StrokeDeterminant)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	unsigned int Order = 0;
	
	if (Tracker != NULL)
		Order = Tracker->GetOrder();
	
	if(FirstNode == NULL)
	{
		//	Create the new (first) node
		// The first node will have a probability of 0.5 allowing for an escape of 0.5
		t2DNodeHidden	*NewNode = new t2DNodeHidden(NULL, NULL, this, NULL, Vector, 1, 0.5, StrokeMeans, StrokeVariance, StrokeInverse, StrokeDeterminant);	
		FirstNode = NewNode;
		LastNode = NewNode;
		Length++;
		TotalCount++;
		TotalSize++;
		
		//POST((t_object* x), (char *)"Created First Node %f", NewNode->Vector.GetValue(0));
		
		// Update the corresponding symbol for this state
		NewNode->UpdateSymbol(SymbolVector, x);
		
		// When a node is created, update the NumTypes for that order
		Tracker->UpdateNumTypes(Order);
		// POST((t_object *x), "Created node: NumTypes(%d): %d", Order, Tracker->GetNumTypes(Order));
		
		// Update escape probability
		Tracker->UpdateEscapeProb(Order, x);
		// Update discount ratio
		Tracker->SetDiscountRatio(Order, x);
		//POST((t_object* x), "Discount Ratio(%d) : %f", Order, Tracker->GetDiscountRatio(Order));
		RetVal = eRETVAL_2DNODE_CREATED;
	}
	else
	{
		unsigned int Found = 0;
		t2DNodeHidden *TempNode1 = NULL;
		t2DNodeHidden *TempNode2 = FirstNode;
		//	Look for correct position in the sorted list of values
		for(int i=0; i<(int)Length; i++)
		{
			if(TempNode2->Vector < Vector)
			{
				TempNode1 = TempNode2;
				TempNode2 = TempNode2->Right;
				continue;
			}
			else if(TempNode2->Vector == Vector)
			{
				Found = 1;
				
				if (TempNode2->Count == 1)
				{
					// When a node is counted more than once, it must be removed from the list
					// of nodes that occur exactly once and added to the NumTypes2
					Tracker->SubtractNumTypes(Order);
					Tracker->UpdateNumTypes2(Order);
					//POST((t_object *x), "2nd hit for node: NumTypes(%d): %d NumTypes2(%d): %d", Order, Tracker->GetNumTypes(Order), Order, Tracker->GetNumTypes2(Order));
				}
				
				if (TempNode2->Count == 2)
				{
					Tracker->SubtractNumTypes2(Order);
					//POST((t_object *x), "3rd hit for node: NumTypes(%d): %d NumTypes2(%d): %d", Order, Tracker->GetNumTypes(Order), Order, Tracker->GetNumTypes2(Order));
				}
				
				TempNode2->Count++;
				TotalCount++;
				
				// Update the corresponding symbol for this state
				//POST((t_object* x), (char *)"Updated Node %f", TempNode2->Vector.GetValue(0));
				TempNode2->UpdateSymbol(SymbolVector, x);
				
				RetVal = eRETVAL_2DNODE_UPDATED;
				break;
			}
			else
			{
				break;
			}
		}
		if(!Found)
		{
			//	Create new node at the position
			t2DNodeHidden	*NewNode = new t2DNodeHidden(TempNode1, TempNode2, this, NULL, Vector, 1, 0, StrokeMeans, StrokeVariance, StrokeInverse, StrokeDeterminant); // TODO: ...
			if(TempNode1 != NULL)	TempNode1->Right = NewNode;
			else					FirstNode = NewNode;
			if(TempNode2 != NULL)	TempNode2->Left = NewNode;
			else					LastNode = NewNode;
			Length++;
			TotalCount++;
			TotalSize++;
			
			// Update the corresponding symbol for this state
			//POST((t_object* x), (char *)"Created Node %f", NewNode->Vector.GetValue(0));
			NewNode->UpdateSymbol(SymbolVector, x);
			
			// When a node is created, update the NumTypes for that order
			Tracker->UpdateNumTypes(Order);
			// POST((t_object *x), "Created node: NumTypes(%d): %d", Order, Tracker->GetNumTypes(Order));
			
			RetVal = eRETVAL_2DNODE_CREATED;
		}
		// Update the probability values (0~1) for all the values in the list
		TempNode2 = FirstNode;
		
		// Update EscapeProb
		Tracker->UpdateEscapeProb(Order, x);
		Tracker->SetDiscountRatio(Order, x);
		//POST((t_object* x), "DiscountRatio(%d) : %f", Order, Tracker->GetDiscountRatio(Order));
		
		for(int i=0; i<(int)Length; i++)
		{
			TempNode2->Prob = (double)(1-Tracker->GetEscapeProb(Order))*(double)(TempNode2->Count)/(double)TotalCount;
			TempNode2 = TempNode2->Right;
		}
	}
	return RetVal;
}

//***************************************
#pragma mark -
#pragma mark Basic Prediction functions
//***************************************

//	Make the best prediction from the model considering all the nodes in the stream starting from the input StreamHead & going right
//	For example, if stream is ABCDE, try to predict based on ABCDE; if unsuccessful, try BCDE, CDE, DE & E successively.
//	Calls Predict()
//	Equivalent to GrowAll()
int	tBundle2DHidden :: PredictBest(c1DVectorList * InputStream, c1DList *Outcome, void *x = NULL)
{
	int RetVal = eRETVAL_UNDEFINED;
	int depth = InputStream->GetLength();
	
	if(InputStream != NULL)
	{
		c1DVectorNode *CurrNode = InputStream->FirstNode;
		RetVal = eRETVAL_FAILURE; // Helps if FirstNode is NULL; TODO: Better way
		while(CurrNode != NULL)
		{
			// POST((t_object *)x, "Bang! PredictBest: %d",CurrNode->Value);
			RetVal = Predict(CurrNode, Outcome, x);
			if(RetVal & eRETVAL_PREDICTION_MADE)
			{
				break;
			}
			CurrNode = CurrNode->Next;
			depth--;
		}
	}
	else
	{
		RetVal = eRETVAL_FAILURE;
		POST((t_object *)x, (char *)"Bang! Failure 3");
	}
	// If prediction not made either due to NULL InputStream, empty (but non-NULL) InputStream or due to a new symbol
	// try to predict using prior probabilities
	if(RetVal & eRETVAL_FAILURE)
	{
		POST((t_object *)x, (char *)"Bang! Failure 11");
		RetVal = Roll(Outcome, x);
		if(RetVal & eRETVAL_PREDICTION_MADE)
		{
			RetVal |= eRETVAL_PRIOR_ALONE_USED;
			POST((t_object *)x, (char *)"Bang! Prior alone used");
		}
		else
		{
			POST((t_object *)x, (char *)"Bang! Failure 12");
		}
	}
	return RetVal;
}

int	tBundle2DHidden :: PredictBest(c1DVectorList *SearchStream, tBundle2DHidden *Outcome, void *x = NULL)
{
	int RetVal = eRETVAL_UNDEFINED;
	int depth = SearchStream->GetLength();
	
	if(SearchStream != NULL)
	{
		c1DVectorNode *CurrNode = SearchStream->FirstNode;
		while(CurrNode != NULL)
		{
			POST((t_object *)x, (char *)"Bang! PredictBest: %f",CurrNode->Vector.GetValue(0));
			RetVal = Predict(CurrNode, Outcome, x);
			
			if(RetVal == eRETVAL_SUCCESS)
			{
				break;
			}
			
			CurrNode = CurrNode->Next;
			depth--;
		}
		
		if (CurrNode == NULL)
		{
			// If everything fails, get priors
			RetVal = ReturnPDF(Outcome, x);
		}
	}
	else
	{
		RetVal = eRETVAL_FAILURE;
		POST((t_object *)x, (char *)"NULL SearchStream");
	}
	return RetVal;
}

//	Predict folowing the values in the stream, pointed to by StreamHead;
//	Recursively finds the branches corresponding to the successive values in the stream,
//	& rolls the dice at the last node.
//	Calls Roll() routine
//	Recursively calls itself (Predict())
//	Equivalent to Grow()
int	tBundle2DHidden :: Predict(c1DVectorNode *StreamHead, c1DList *Outcome, void *x = NULL)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	t2DNodeHidden * TempNode;
	
	//	If NULL pointer passed in, return failure
	if(StreamHead == NULL)
	{
		RetVal = eRETVAL_FAILURE;
		POST((t_object *)x, (char *)"Bang! Failure 4");
	}
	else
	{
		//	Find the corresponding node & follow the branch below it
		TempNode = Find(&StreamHead->Vector);
		//	TODO: If no node has the value, use escape
		if(TempNode == NULL)
		{
			RetVal = eRETVAL_FAILURE;
			// POST((t_object *)x, "Bang! Failure 5");
		}
		else
		{
			// POST((t_object *)x, "Bang! Predict: %d",TempNode->Value);
			//	TODO: If the node doesn't have a branch below it, ...
			if(TempNode->Bottom == NULL)
			{
				RetVal = eRETVAL_FAILURE;
				// POST((t_object *)x, "Bang! Failure 8");
			}
			else
			{
				//	If last node in the stream, roll the dice & return
				if(StreamHead->Next == NULL)
				{
					RetVal = TempNode->Bottom->Roll(Outcome, x);
				}
				else
				{
					//	Continue following values down till the end of stream is reached
					RetVal = TempNode->Bottom->Predict(StreamHead->Next,Outcome);
				}
			}
		}
	}			
	return	RetVal;
}

// Predict() - uses the Viterbi decoding algorithm to match the 
// symbol sequence of the search stream with a state sequence in the 
// tree. When it reaches the last node, it predicts the next state
// and emits a symbol from that state
int	tBundle2DHidden :: Predict(c1DVectorNode *StreamHead, tBundle2DHidden *PDF, void *x = NULL)
{
	unsigned int RetVal = eRETVAL_FAILURE;
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* MaxNode = NULL;
	double MaxAlpha = 0;	// Initialize the minimum prob to 1, the max value.
	double a = 0;
	double b = 0;
	double Alpha = 0;
	
	if ((StreamHead != NULL) && (FirstNode != NULL))
	{
		for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			// For each node in the bundle,
			// check probability that the node occurred
			// times the probability that the symbol occurred
			
			a = TempNode->Prob;
			b = TempNode->EmissionProbability(&StreamHead->Vector);
			
			Alpha = a*b;
			
			if (Alpha > MaxAlpha)
			{
				MaxAlpha = Alpha;
				MaxNode = TempNode;
			}
		}
		
		// Call Predict() on the bundle below this node if it exists
		if (MaxNode->Bottom != NULL)
		{
			if (StreamHead->Next != NULL)
			{
				RetVal = MaxNode->Bottom->Predict(StreamHead->Next, PDF, x);
			}
			else 
			{
				// If the next symbol is NULL,
				// then we have reached the end of the stream.
				// This is where we predict the next state.
				
				// ReturnPDF for next state		
				MaxNode->Bottom->ReturnPDF(PDF, x);
				RetVal = eRETVAL_SUCCESS;
			}
		}
		else 
		{
			// If the node does not have a bundle below it,
			// then we cannot predict this entire stream.
			// Return with Failure and we shall re-predict on a smaller stream
			
			// TODO: RETURN ESCAPE??
			RetVal = eRETVAL_FAILURE;
		}
	}
	else 
	{
		// If streamhead is NULL or the Bundle has no nodes
		// Say FAIL
		RetVal = eRETVAL_FAILURE;
	}
	
	return RetVal;	
}

double tBundle2DHidden :: EvaluateSearchStream(c1DVectorList *SearchStream, void *x)
{
	if (SearchStream != NULL)
	{
		c1DVectorNode* StreamHead = SearchStream->FirstNode;
		return EvaluateStream(StreamHead, x);
	}
	else {
		return 0;
	}
}

// Function to "evaluate" a given sequence - this means 
// it will return the probability that the sequence could occur
// based on the model

// For a given search stream:
// 1. Start from the first node in the top level bundle
// 2. Find the probability of the first symbol in the stream
// 3. Alpha = 1; Alpha *= a_ij*b_jk
// 4. Proceed to next node and repeat

double tBundle2DHidden :: EvaluateStream(c1DVectorNode *StreamHead, void *x)
{
	t2DNodeHidden* TempNode = NULL;
	double a = 0; // Probability of transition to current state from previous state
	double b = 0; // Probability of emission of current symbol (StreamHead) by the node
    double Alpha = 0;
	double Sum = 0;
	
	// If the node exists
	if ((StreamHead != NULL) && (FirstNode != NULL))
	{
		//POST((t_object*)x, (char *)"Symbol: %f", StreamHead->Vector.GetValue(0));
		
		for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			a = TempNode->Prob;
			b = TempNode->EmissionProbability(&StreamHead->Vector);
			
			if (TempNode->Bottom != NULL)
			{
				Alpha = a*b*(TempNode->Bottom->EvaluateStream(StreamHead->Next, x));
				//POST((t_object*)x, (char *)"Alpha = %f", Alpha);
			}
			else 
			{
				// If there is no bundle below
				// return the current node's Alpha
				Alpha = a*b;
			}
			
			Sum += Alpha;
			//POST((t_object*)x, (char *)"Sum = %f", Sum);
		}
	}
	else {
		// When a StreamHead is NULL, it should return 1
		// so that the Alpha for the node above remains unchanged
		//POST((t_object)*x, (char *)"return 1: NULL symbol");
		Sum = 1;
	}
	return Sum;
}

// Wrapper for EvaluateStreamContinuous() so that it can be called from viewpoint class
double tBundle2DHidden :: EvaluateSearchStreamContinuous(c1DVectorList *SearchStream, c1DList* Vector, tTracker* Tracker, void *x)
{
	double FwdProb = 0;
	unsigned int Order = 0;
	
	if (SearchStream != NULL)
	{
		c1DVectorNode* StreamHead = SearchStream->FirstNode;
		Order = SearchStream->GetLength();
		
		// initialize the weights for adding
		Tracker->SetAllWeights(SearchStream->GetLength());
		
		//Tracker->ResetOrder();
		//FwdProb += Tracker->GetWeight(Order)*EvaluateStreamContinuous(StreamHead, Vector, x, Tracker);
		
		for (StreamHead = SearchStream->FirstNode; StreamHead != NULL; StreamHead = StreamHead->Next)
		{
			Tracker->ResetOrder();
			FwdProb += Tracker->GetWeight(Order)*EvaluateStreamContinuous(StreamHead, Vector, x, Tracker);
			Order--;
		}
		 
	}
	else 
	{
		// Don't do anything
	}
	
	return FwdProb;
}


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

double tBundle2DHidden :: EvaluateStreamContinuous(c1DVectorNode *StreamHead, c1DList * Vector, void *x, tTracker* Tracker)
{
	t2DNodeHidden* TempNode = NULL;
	double Alpha = 0;
	double Sum = 0;
	unsigned int Order = Tracker->GetOrder();
	
	if (StreamHead == NULL)
	{
		// If a NULL search node is sent, then return 0;
		//Sum = 0;
		Sum = Tracker->GetEscapeProb(Order);
	}
	else 
	{
		// If the bundle has nodes
		if ((FirstNode != NULL))
		{
			// This means If trie and streamhead exist
			
			if (StreamHead->Next == NULL)
			{
				// This means there is only one node in the stream.
				// Get its probability w.r.t its vector
				Sum = GetSimpleProbability(Vector, Tracker);
				
				//Sum = GetProbability(StreamHead->FeatureVector, &StreamHead->Vector, Tracker);
			}
			else 
			{
				// This means there are atleast two nodes
				// Alpha is the sum of the probability that this symbol occurred in this level
				
				for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
				{
					// Probability that this node produced the feature vector
					// multiplied by prob that the next node produced the next vector
					Alpha = GetProbability(StreamHead->FeatureVector, &TempNode->Vector, Tracker);
					
					if (TempNode->Bottom != NULL)
					{
						Tracker->UpdateOrder();
						// If bottom bundle exists
						Alpha *= TempNode->Bottom->EvaluateStreamContinuous(StreamHead->Next, Vector, x, Tracker);
						
						Tracker->SubtractOrder();
					}
					else 
					{
						// If no bottom bundle exists, then this means the sequence cannot
						// be found. Leave this alone for now.
					}
					Sum += Alpha;
				}
			}

		}
		else 
		{
			// A trie should exist. In that case this is a bundle without
			// any nodes, and therefore should return 1 so it doesn't mess
			// up the alpha
			Sum = 1;
		}
	}
	
	//POST((t_object*)x, (char *)"Sum = %f", Sum);

	return Sum;
}

//  A duplicate of the EvaluateSearchStreamContinuous() function
//  Returns the FULL forward probability of the sequence
double tBundle2DHidden :: GetForwardProbabilityForStream(c1DVectorList *SearchStream, c1DList* Vector, tTracker* Tracker, void *x)
{
	unsigned int Order = 0;
	double FwdProb = Tracker->GetEscapeProb(Order);
	
	if (SearchStream != NULL)
	{
		c1DVectorNode* StreamHead = SearchStream->FirstNode;
		Order = SearchStream->GetLength();
		
		// initialize the weights for adding
		//Tracker->SetAllWeights(SearchStream->GetLength());
		//Tracker->ResetOrder();
		
		for (StreamHead = SearchStream->FirstNode; StreamHead != NULL; StreamHead = StreamHead->Next)
		{
			FwdProb += Tracker->GetWeight(Order)*GetForwardProbability(StreamHead, x, Tracker);
			Order--;
		}
	}
	else 
	{
		// Don't do anything
	}
	
	return FwdProb;
}

//  This is practically a duplicate of the EvaluateStreamContinuous() function
//  except that this function doesn't just return the "simple probability"
//  or the prior prob for the last element in the sequence.
//  It returns the FULL probability - a_ij*b_jk. 
//  This is for use in the Forward-backward algorithm

double tBundle2DHidden :: GetForwardProbability(c1DVectorNode *StreamHead, void *x, tTracker* Tracker)
{
	t2DNodeHidden* TempNode = NULL;
	double Alpha = 0;
	double Sum = 0;
	unsigned int Order = Tracker->GetOrder();
	
	if (StreamHead == NULL)
	{
		// If a NULL search node is sent, then return 0;
		//Sum = 0;
		//Sum = Tracker->GetEscapeProb(Order);
		Sum = 1;
	}
	else 
	{
		// If the bundle has nodes
		if ((FirstNode != NULL))
		{
			// This means If trie and streamhead exist
			
			if (StreamHead->Next == NULL)
			{
				// This means there is only one node in the stream.
				// Get its probability w.r.t its vector
				Sum = GetProbability(StreamHead->FeatureVector, &StreamHead->Vector, Tracker);
			}
			else 
			{
				// This means there are atleast two nodes
				// Alpha is the sum of the probability that this symbol occurred in this level
				
				for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
				{
					// Probability that this node produced the feature vector
					// multiplied by prob that the next node produced the next vector
					Alpha = GetProbability(StreamHead->FeatureVector, &TempNode->Vector, Tracker);
					
					if (TempNode->Bottom != NULL)
					{
						Tracker->UpdateOrder();
						
						// If bottom bundle exists
						Alpha *= TempNode->Bottom->GetForwardProbability(StreamHead->Next, x, Tracker);
						
						Tracker->SubtractOrder();
					}
					else 
					{
						// If no bottom bundle exists, then this means the sequence cannot
						// be found. Leave this alone for now.
					}
					Sum += Alpha;
				}
			}
		}
		else 
		{
			// A trie should exist. In that case this is a bundle without
			// any nodes, and therefore should return 1 so it doesn't mess
			// up the alpha
			Sum = 1;
		}
	}
	
	//POST((t_object*)x, (char *)"Sum = %f", Sum);
	
	return Sum;	
}

//  Function to find the backward probability of a stream
double tBundle2DHidden :: GetBackwardProbabilityForNode(c1DVectorList* SearchStream, c1DList* NodeVector,  tTracker* Tracker, void* x)
{
	c1DVectorNode* StreamHead = NULL;
	t2DNodeHidden* TempNode = NULL;
	unsigned int Order = Tracker->GetOrder();
	double BackwardProb = Tracker->GetEscapeProb(Order);
	
	// If stream and bundle exist
	if ((SearchStream != NULL) && (FirstNode != NULL)) {
		
		StreamHead = SearchStream->FirstNode;
		
		// Find the node whose Beta you want to find
		TempNode = Find(NodeVector);
		
		if (TempNode != NULL)
		{
			// If there's a bundle below that node
			if (TempNode->Bottom != NULL) {
				// Run GetBackwardProbability on that bundle
				BackwardProb = TempNode->Bottom->GetBackwardProbability(StreamHead, Tracker, x);
			}
			else {
				// Bundle does not exist!
				// Send in either 0 or maybe it's better to send in an escape
				BackwardProb = Tracker->GetEscapeProb(Order);
			}
		}
		else {
				BackwardProb = Tracker->GetEscapeProb(Order);
		}
	}
	
	return BackwardProb;
}


// Function to calculate the backward probability of a particular sequence
// 1. Finds its way down the tree to the last level
// 2. Calculates the backward probability up the tree till it reaches the top
// 3. Uses Duda-Hart's definitions of Alpha and Beta calculations

double tBundle2DHidden :: GetBackwardProbability(c1DVectorNode* StreamHead, tTracker* Tracker,void *x)
{
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* TempNode2 = NULL;
	unsigned int Order = Tracker->GetOrder();
	double Beta = Tracker->GetEscapeProb(Order);
	double Prob = 0;
	
	// If this node exists
	if (StreamHead != NULL && FirstNode != NULL) {
		// If StreamHead is the final node
		if (StreamHead->Next == NULL) {
			// Calculate beta for the streamhead->vector node in this bundle			
			Beta = GetProbability(StreamHead->FeatureVector, &StreamHead->Vector, Tracker);
		}
		else {
			// Call this function for the bundle below each node
			for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
			{
				if (TempNode->Bottom != NULL)
				{
					Prob = GetProbability(StreamHead->FeatureVector, &TempNode->Vector, Tracker);
					//POST((t_object*)x, (char *)"Prob = %f", Prob);
					
					Tracker->UpdateOrder();
					Beta += TempNode->Bottom->GetBackwardProbability(StreamHead->Next, Tracker, x)*Prob;
					Tracker->SubtractOrder();
				}
			}
		}
	}
	else 
	{
		Beta = Tracker->GetEscapeProb(Order);
	}
	
	//POST((t_object*)x, (char *)"Beta = %f", Beta);

	return Beta;
}

unsigned int tBundle2DHidden :: ForwardBackwardAlgorithm(tBundle2DHidden* RootBundle, c1DVectorList* SearchStream, c1DVectorList* FullSearchStream, c1DVectorNode* StreamHead, tTracker* Tracker, void* x)
{
	unsigned int RetVal = 0;
	t2DNodeHidden* TempNode = NULL;
	t2DNode* TempNode2 = NULL;
	tBundle2D* TempBundle = new tBundle2D(NULL, NULL, NULL, 0, 0, 0);
	double Gamma = 0;
	double GammaSum = 0;
	double GammaMax = 0;
	unsigned int Order = Tracker->GetOrder();
	double LearningCriterion = 0;
	
	// If the bundle has nodes
	if (FirstNode != NULL)
	{
		//  For each node in the bundle
		for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			Tracker->ResetOrder();
			double P1 = RootBundle->GetForwardProbability(SearchStream->FirstNode, x, Tracker);
			
			Tracker->ResetOrder();
			double P4 = RootBundle->GetForwardProbability(FullSearchStream->FirstNode, x, Tracker);
			
			Tracker->SetOrder(Order);
			double P2 = TempNode->Prob;
			//double P2 = GetProbability(StreamHead->FeatureVector, &TempNode->Vector, Tracker);
			
			double P3 = Tracker->GetEscapeProb(Order);
			if (TempNode->Bottom != NULL)
			{
				P3 = TempNode->Bottom->GetBackwardProbability(StreamHead->Next, Tracker, x);
			}
			
			POST((t_object*) x, "P1 = %f; P2 = %f; P3 = %f; P4 = %f", P1, P2, P3, P4);
			
			if (P3 == 0)
			{
				POST((t_object* x), "GAMMA ZERO");
				P3 = GetBackwardProbability(StreamHead, Tracker, x);
			}
			
			//  1. Calculate Gamma for it
			Gamma = P1 * P2 * P3 / P4;
			GammaSum += Gamma;
			
			//  2. Store it
			TempBundle->Append(&TempNode->Vector);
			TempBundle->LastNode->Prob = Gamma;
		}
		
		if(isnan(Gamma))
		{
			POST((t_object* x), "GAMMA NAN");
		}
		
		//  Check whether the max change in probability is greater than THETA
		TempNode2 = TempBundle->FirstNode;
		for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			LearningCriterion = (TempNode->Prob) - (TempNode2->Prob/GammaSum);
			
			// Convert this difference to a positive value
			if (LearningCriterion < 0)
			{
				LearningCriterion = -LearningCriterion;
			}
			
			// Find the max change
			if (LearningCriterion > GammaMax) {
				GammaMax = LearningCriterion;
			}
			
			TempNode2 = TempNode2->Right;
		}
	
		//  2. If GammaMax > THETA
		//  Normalize each gamma by the sum of the all the Gammas
		//  For each node in the bundle
		if (GammaMax > THETA)
		{
			TempNode2 = TempBundle->FirstNode;
			
			for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
			{
				TempNode->Prob = TempNode2->Prob/GammaSum;
				TempNode2 = TempNode2->Right;
			}
		}
	}
	else 
	{
		RetVal = eRETVAL_FAILURE;
	}

	return RetVal;
}

//  Propagation function for the ForwardBackwardAlgorithm()
//  Calls it recursively on a bundle, and then on every bundle below it

unsigned int tBundle2DHidden :: AdjustProbabilities(tBundle2DHidden* RootBundle, c1DVectorList* SearchStream, unsigned int Order, tTracker* Tracker, void* x)
{
	unsigned int RetVal = 0;
	unsigned int i = 0;
	t2DNodeHidden* TempNode = NULL;
	c1DVectorNode* VectorNode = NULL;
	c1DVectorList* PartialSearchStream = new c1DVectorList(SearchStream->GetDim(), 0, NULL, NULL);
	
	// If bundle and stream both exist
	if ((FirstNode != NULL) && (SearchStream != NULL))
	{
		// if stream has nodes
		if (SearchStream->FirstNode != NULL)
		{
			// Create a new partial search stream based on the order 
			// that terminates just before this bundle
			
			// If you are running this on level N of the tree
			// Send in order N-1, eg. 0 for top level, 1 for second level, etc.
			
			VectorNode = SearchStream->FirstNode;
			
			while (i < Order) {
				PartialSearchStream->AddFeatures(VectorNode->FeatureVector, &VectorNode->Vector);
				VectorNode = VectorNode->Next;
				i++;
			}
			
			// Call function on this bundle
			ForwardBackwardAlgorithm(RootBundle, PartialSearchStream, SearchStream, VectorNode, Tracker, x);
			
			// If the sequence extends further
			if (VectorNode->Next != NULL)
			{
				// Call for every bundle below this one
				for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
				{
					if (TempNode->Bottom != NULL)
					{
						Tracker->UpdateOrder();
						TempNode->Bottom->AdjustProbabilities(RootBundle, SearchStream, (Order+1), Tracker, x);
						Tracker->SubtractOrder();
					}
				}
			}
		}
	}
	else 
	{
		return eRETVAL_FAILURE;
	}
	
	return RetVal;
}

// Wrapper for the viewpoint class for DecodeStream()
unsigned int tBundle2DHidden :: DecodeSearchStream(c1DVectorList* SearchStream, c1DVectorList* ResultStream, void* x)
{
	if (SearchStream != NULL)
	{
		c1DVectorNode* StreamHead = SearchStream->FirstNode;
		return DecodeStream(StreamHead, ResultStream, x);
	}
	else 
	{
		return eRETVAL_FAILURE;
	}
}

//  Viterbi Decoding - Function to "decode" a stream
//  Given a search stream, the algorithm traverses/finds the most likely
//  sequence of states that led to that search stream

//  Viterbi decoding will be used by the predict function to predict the next
//  set of symbols

unsigned int tBundle2DHidden :: DecodeStream(c1DVectorNode* StreamHead, c1DVectorList* ResultStream,void* x)
{
	unsigned int RetVal = eRETVAL_FAILURE;
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* MaxNode = NULL;
	double MaxAlpha = 0;	// Initialize the minimum prob to 1, the max value.
	double a = 0;
	double b = 0;
	double Alpha = 0;
	
	if ((StreamHead != NULL) && (FirstNode != NULL))
	{
		for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			// For each node in the bundle,
			// check probability that the node occurred
			// times the probability that the symbol occurred
			
			a = TempNode->Prob;
			b = TempNode->EmissionProbability(&StreamHead->Vector);
			
			Alpha = a*b;
			
			if (Alpha > MaxAlpha)
			{
				MaxAlpha = Alpha;
				MaxNode = TempNode;
			}
		}
		
		// Append this node to the result stream
		ResultStream->Append(&MaxNode->Vector);
		POST((t_object*)x, (char *)"MaxNode: %f", MaxNode->Vector.GetValue(0));
		
		// Call Decode stream on the bundle below this node if it exists
		if (MaxNode->Bottom != NULL)
		{
			RetVal = MaxNode->Bottom->DecodeStream(StreamHead->Next, ResultStream, x);
		}
		else 
		{
			// There's no need to descend any further.
			RetVal = eRETVAL_SUCCESS;
		}
	}
	else 
	{
		// If streamhead is NULL or the Bundle has no nodes
		// Say FAIL
		RetVal = eRETVAL_FAILURE;
	}
	
	return RetVal;
}

// Function to evaluate the feature vector fed into the bundle and classify it
// into one of its nodes. The outcome is written to *Outcome.
unsigned int tBundle2DHidden :: ClassifyFeatures(float* FeatureVector, c1DList* Outcome)
{
	unsigned int RetVal = eRETVAL_FAILURE;
	double Prob = 0;
	double MaxProb = 0;
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* MaxNode = NULL;
	
	// If bundle has nodes AND feature vector is not null
	if ((FirstNode != NULL) && (FeatureVector != NULL))
	{
		for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			// Prob = Prior*Likelihood
			Prob = (TempNode->Prob)*TempNode->GaussianLikelihood(FeatureVector);
			
			if (Prob != 0)
			{
				if (Prob > MaxProb)
				{
					MaxProb = Prob;
					MaxNode = TempNode;
					
					*Outcome = MaxNode->Vector;
					RetVal = eRETVAL_SUCCESS;
				}
			}
			else 
			{
				// If there is something wrong and 0 prob is returned,
				// return failure
				RetVal = eRETVAL_FAILURE;
			}
		}
	}
	return RetVal;
}

// Function to calculate the probability that this feature set belongs to the given category.
// Should not be confused with Likelihood - likelihood is just an unscaled value.
// GetProbability() uses Bayes theorem to calculate the probability.
// Must return a value between 0 and 1;
double tBundle2DHidden :: GetProbability(float* FeatureVector, c1DList* DataToken, tTracker* Tracker)
{
	double Posterior = 0;
	double Likelihood = 0;
	double Denominator = 0;
	double Prior = 0;
	unsigned int Order = Tracker->GetOrder();
	double Probability = Tracker->GetEscapeProb(Order);
	t2DNodeHidden* TempNode = NULL;
	
	if ((FirstNode != NULL) && (FeatureVector != NULL) && (DataToken != NULL))
	{
		// Check if the node is present
		TempNode = Find(DataToken);
		
		if (TempNode != NULL)
		{
			// If the node exists calculate the probability
			// Using Bayes theorem:
			// Probability = (Likelihood * Prior) / (ScalingFactor)
			
			Likelihood = TempNode->GaussianLikelihood(FeatureVector);
			Prior = TempNode->Prob;
			
			Posterior = Likelihood*Prior;
			
			// Find the denominator
			// denominator = sum(P(i)*prior(i))
			
			for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
			{
				Denominator += TempNode->GaussianLikelihood(FeatureVector)*TempNode->Prob;
			}
			
			Probability = Posterior/Denominator;
		}
		else 
		{
			// If you cannot find the node, 
			// return the escape probability for that level
			Probability = Tracker->GetEscapeProb(Order);
			//Probability = 0;
		}
	}
	else 
	{
		Tracker->GetEscapeProb(Order);
	}
	
	return Probability;
}

//  Function to get the prior probability of a symbol from a bundle
double tBundle2DHidden :: GetSimpleProbability(c1DList* DataToken, tTracker* Tracker)
{
	t2DNodeHidden* TempNode = NULL;
	unsigned int Order = Tracker->GetOrder();
	double Probability = Tracker->GetEscapeProb(Order);
	
	if ((FirstNode != NULL) && (DataToken != NULL))
	{
		// Check if the node is present
		TempNode = Find(DataToken);
		
		if (TempNode != NULL)
		{
			Probability = TempNode->Prob;
		}
		else 
		{
			Probability = Tracker->GetEscapeProb(Order);
		}
	}
	else 
	{
		Probability = Tracker->GetEscapeProb(Order);
	}
	
	return Probability;
}

//  Function to Roll() for a random state in a bundle.
//  Used on the resulting PDf of a viewpoint after Predict()
//  has been called.
//  1. If the bundle is not NULL
//  2. Compile a list of cumulative probabilties (0 to 1)
//  3. Generate a random number between 0 and 1
//  4. Place it in the corresponding bin in the list
//  5. Return the state corresponding to the bin

t2DNodeHidden* tBundle2DHidden :: RollState(void *x = NULL)
{
	double Probabilities[Length];
	double CumulativeProbs[Length];
	t2DNodeHidden* TempNode;
	int i = 1;
	
	double RandVal = rand()/(double)RAND_MAX; // Generate a random value between 0 and 1
	Probabilities[0] = 0;
	CumulativeProbs[0] = 0;
	
	if (FirstNode != NULL)
	{
		for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			
			Probabilities[i] = TempNode->Prob;
			CumulativeProbs[i] = CumulativeProbs[i-1] + TempNode->Prob;
			
			if (RandVal < CumulativeProbs[i])
			{
				break;
			}
			else 
			{
				i++;
			}
		}
		
		// This means RandVal is greater than the last element of CumulativeProbs
		// So return the last node in the bundle
		if (TempNode == NULL)
		{
			TempNode = LastNode;
		}
		
		return TempNode;
	}
	else 
	{
		return NULL;
	}

}

//	Determine the prediction outcome based on the probability distribution under the input value after finding it at its sorted position
//	'Roll the dice'
//	Called by Predict() routine
//	Equivalent to Update()
int	tBundle2DHidden :: Roll(c1DList *Outcome, void *x = NULL)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	
	if(FirstNode == NULL)
	{
		RetVal = eRETVAL_FAILURE;
		//POST((t_object *)x, (char *)"Bang! Failure 6");
	}
	else
	{
		// Local arrays
		c1DList **Vectors = new c1DList *[Length];
		double *Probabilities = new double [Length];
		double *CumulProbs = new double [Length]; // Cumulative probabilities
		// unsigned int *Locations = new unsigned int [Length];
		
		// Fill the local arrays with entries from the linked list
		t2DNodeHidden *TempNode = FirstNode;
		for(int i=0; TempNode!=NULL; i++, TempNode=TempNode->Right)
		{
			Vectors[i] = &TempNode->Vector;
			Probabilities[i] = TempNode->Prob;
		}
		
		// Determine cumulative probabilities
		CumulProbs[0] = Probabilities[0];
		for(int i=1; i<Length; i++)
		{
			CumulProbs[i] = CumulProbs[i-1]+Probabilities[i];
		}
		
		double RandVal = rand()/(double)RAND_MAX;
		
		// Find out where RandVal fits in the cumulative probability distribution
		// A way of making random selection according to the probability distribution
		int i;
		for(i=0; i<Length; i++)
		{
			if(CumulProbs[i]>RandVal)
			{
				break;
			}
		}
		// To account for RandVal=1, if 'i' has exceeded the limit, adjust it
		if(i==Length)
		{
			i=Length-1;
		}
		// 'i' is now the index of the predicted value/symbol
		*Outcome = Vectors[i];
		
		delete Vectors;
		delete Probabilities;
		delete CumulProbs;
		// delete Locations;
		
		RetVal = eRETVAL_PREDICTION_MADE;
		
		// POST((t_object *)x, "Bang! RandVal = %f Ret = %x Outcome = %d",RandVal,RetVal,*Outcome);
	}
	return RetVal;
}

//***************************************
#pragma mark -
#pragma mark Advanced Prediction Functions
//***************************************

//this is for smoothing - equivalent to GrowAll() or PredictBest()
int tBundle2DHidden :: PredictSmoothBest( c1DVectorList* SearchStream, tBundle2DHidden *Accumulator, tTracker* Tracker, void*x) 
{	
	int RetVal = eRETVAL_UNDEFINED;
	int depth = 0;
	
	tBundle2DHidden *Outcome = new tBundle2DHidden(NULL, NULL, NULL, 0, 0, 0);
	t2DNodeHidden * TempNode = NULL;
	t2DNodeHidden * TempNode2 = NULL;
	
	if(SearchStream != NULL)
	{
		//Tracker->SetAllWeights(SearchStream->Length);
		
		Tracker->SetAllWeightsFunction(SearchStream->Length);
		
		RetVal = ReturnPDF(Accumulator, x);
		
		// After we get the priors we need to weight it by Tracker->GetWeight(0)
		TempNode2 = Accumulator->FirstNode;
		for(TempNode2 = Accumulator->FirstNode; TempNode2!=NULL; TempNode2=TempNode2->Right)
		{
			TempNode2->Prob *= Tracker->GetWeight(0);
		}
		
		if (RetVal == eRETVAL_FAILURE)
		{
			// Tree does not have any nodes
			POST((t_object *x), (char *)"Tree does not have any nodes");
		}
		else
		{
			c1DVectorNode *CurrNode = SearchStream->LastNode;
			while(CurrNode != NULL)
			{
				// Reset the local outcome values otherwise you may double count the probs
				Outcome = new tBundle2DHidden(NULL, NULL, NULL, 0, 0, 0);
				
				// POST((t_object *)x, "Bang! PredictBest: %d",CurrNode->Value);
				RetVal = PredictSmooth(CurrNode, Outcome, x);
				
				// Add outcome to accumulator
				if (RetVal == eRETVAL_SUCCESS)
				{
					TempNode = Outcome->FirstNode;
					TempNode2 = Accumulator->FirstNode;
					
					for(TempNode2 = Accumulator->FirstNode; TempNode!=NULL; TempNode=TempNode->Right)
					{
						TempNode2 = Accumulator->Find(&TempNode->Vector);
						if( TempNode2 == NULL)
						{
							// Add the node to Accumulator since it doesn't exist
							Accumulator->Append(&TempNode->Vector);
							TempNode2 = Accumulator->LastNode;
							
							// Assign the probability of the node weighted by the level
							TempNode2->Prob = (TempNode->Prob)*(Tracker->GetWeight(depth+1));
						}
						else
						{
							// since we found the node, we multiply the weight for the depth
							// and then add it to the accumulator
							
							TempNode2->Prob += (TempNode->Prob)*(Tracker->GetWeight(depth+1));
						}
					}
				}
				else if (RetVal == eRETVAL_ESCAPE_USED)
				{
					// In this case, the outcome will contain a single value that is the escape 
					// probability for that level
					// All we have to do is multiply our existing PDF with this value
					
					TempNode = Outcome->FirstNode;
					TempNode2 = Accumulator->FirstNode;
					
					for(int i=0; TempNode!=NULL; i++, TempNode=TempNode->Right)
					{
						if( TempNode2 == NULL)
						{
							// If we come here, it means the accumulator hasn't been created yet.
							// Return Failure
							RetVal = eRETVAL_FAILURE;
							
						}
						else
						{
							// since we found the node, we multiply the weight for the depth
							// and then add it to the accumulator
							TempNode2->Prob *= (Tracker->GetEscapeProb(depth))*(Tracker->GetWeight(depth));
							
						}
					}
				}
				else
				{
					// It shouldn't come here - since it already checks for input stream and tree null pointers
					//POST((t_object *)x, (char *)"Bang! It shouldn't come here - since it already checks for input stream and tree null pointers");
					RetVal = eRETVAL_FAILURE;
				}
				
				CurrNode = CurrNode->Prev;
				depth++;
			}
			
			// We need to normalize the PDF in the Accumulator before sending it out.
			Accumulator->Normalize();
			
			
			RetVal = eRETVAL_SUCCESS;
		}
	}
	else
	{
		RetVal = eRETVAL_FAILURE;
		POST((t_object *)x, (char *)"Bang! Failure 3");
	}
	return RetVal;
}

// This is the inner function that recursively calls itself - equivalent to Grow()/Predict()
int tBundle2DHidden :: PredictSmooth( c1DVectorNode * StreamHead, tBundle2DHidden *Outcome, void *x) {
	
	unsigned int RetVal = eRETVAL_UNDEFINED;
	t2DNodeHidden * TempNode;
	
	//	If NULL pointer passed in, return failure
	if(StreamHead == NULL)
	{
		RetVal = eRETVAL_FAILURE;
		POST((t_object *)x, (char *)"Bang! Failure 4 - StreamHead = NULL");
	}
	else
	{
		//	Find the corresponding node & follow the branch below it
		//POST((t_object *)x, "Searching for: %f-%f:", StreamHead->Vector.GetValue(0), StreamHead->Vector.GetValue(1));  
		TempNode = Find(&StreamHead->Vector);
		//	TODO: If no node has the value, by chance, ...
		if(TempNode == NULL)
		{
			//POST((t_object *)x, "This means it didn't find the symbol in the tree (use partial match)");
			RetVal = eRETVAL_ESCAPE_USED;
		}
		else
		{
			// POST((t_object *)x, "Bang! Predict: %d",TempNode->Value);
			//	TODO: If the node doesn't have a branch below it, ...
			if(TempNode->Bottom == NULL)
			{
				RetVal = eRETVAL_FAILURE;
				
				//POST((t_object *)x, (char *)"Bang! Failure 8..it doesn't have any children");
			}
			else
			{
				//	If last node in the stream, roll the dice & return
				if(StreamHead->Next == NULL)
				{
					RetVal = TempNode->Bottom->ReturnPDF(Outcome, x);
				}
				else
				{
					//	Continue following values down till the end of stream is reached
					RetVal = TempNode->Bottom->PredictSmooth(StreamHead->Next, Outcome, x);
				}
			}
		}
	}			
	return	RetVal;
}

//	needed to get the pdf values ...similar to what roll() does, but no decision made yet.  
//	Called by PredictSmooth() routine
//	Equivalent to ...
int	tBundle2DHidden :: ReturnPDF(tBundle2DHidden *Outcome, void *x = NULL)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	t2DNodeHidden *TempNode = NULL;
	t2DNodeHidden *TempNode2 = NULL;
	
	if(FirstNode == NULL)
	{
		RetVal = eRETVAL_FAILURE;
		POST((t_object *)x, (char *)"Bundle Empty!");
	}
	else
	{
		// Fill the local arrays with entries from the linked list
		TempNode = FirstNode;
		
		if (Outcome->FirstNode != NULL)
		{
			TempNode2 = Outcome->FirstNode;
		}
		else
		{
			TempNode2 = NULL;
		}
		
		for(TempNode = FirstNode; TempNode!=NULL; TempNode=TempNode->Right)
		{
			// Find the node in Outcome
			TempNode2 = Outcome->Find(&TempNode->Vector);
			
			if (TempNode2 == NULL)
			{
				Outcome->Append(&TempNode->Vector);
				POST((t_object*)x, (char *)"Node: %f", TempNode->Vector.GetValue(0));
				//since you just added it, you know it's the last node
				TempNode2 = Outcome->LastNode;
				TempNode2->Prob	= TempNode->Prob;
				POST((t_object*)x, (char *)"Prob: %f", TempNode->Prob);
				
				// Copy the emission matrix
				TempNode2->Emission = TempNode->Emission;
			}
			else
			{
				TempNode2->Prob	= TempNode->Prob;
			}
		}
		
		RetVal = eRETVAL_SUCCESS;
	}
	
	return RetVal;
}

// PredictKneserNey() - Uses KneserNey Smoothing
unsigned int tBundle2DHidden :: PredictKneserNey(c1DVectorList* SearchStream, tBundle2DHidden *Accumulator, tTracker* Tracker, void*x)
{
	int RetVal = eRETVAL_UNDEFINED;
	double Prob = 0;
	
	tBundle2DHidden *Priors = new tBundle2DHidden(NULL, NULL, NULL, 0, 0, 0);
	t2DNodeHidden * TempNode = NULL;
	t2DNodeHidden * TempNode2 = NULL;
	
	if(SearchStream != NULL)
	{
		// If search stream exists then first get the prior probs/find out the number of nodes
		RetVal = ReturnPDF(Priors, x);
		
		// Once you get the priors, call KneserNey on each node of the prior
		for (TempNode = Priors->FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			Prob = KneserNey(TempNode, SearchStream->FirstNode, Tracker, x);
			
			// After we've got the probability value
			// Search for this node/symbol in the accumulator
			TempNode2 = Accumulator->Find(&TempNode->Vector);
			if (TempNode2 == NULL)
			{
				// If if doesn't exist and it shouldn't, then add the node and prob to accumulator
				Accumulator->Append(&TempNode->Vector);
				TempNode2 = Accumulator->LastNode;
				
				// Assign the probability of the node weighted by the level
				// If the kneser nay returned a NULL probability, that means 
				// there was no search stream. Assign the prior probability in this case
				TempNode2->Prob = Prob;
			}
			else
			{
				// And it shouldn't come here
				// In case it does, just assign the probability without creating a node
				POST((t_object* )x, (char *)"Accumulator not reset - it shouldn't come here");
				TempNode2->Prob = Prob;
			}
		}
		// Normalize the accumulator
		Accumulator->Normalize();
		return eRETVAL_SUCCESS;
	}
	else
	{
		return eRETVAL_FAILURE;
	}
}

double tBundle2DHidden :: KneserNey(t2DNodeHidden* PredictedSymbol, c1DVectorNode* StreamHead, tTracker* Tracker, void* x)
{
	double Prob = 0;
	double FirstTerm = 0;
	double SecondTerm = 0;
	unsigned int Order = 0;
	unsigned int NodeCount = 0;
	unsigned int ChildrenCount = 0;
	unsigned int SumCount = 0;
	double Discount = 0;
	t2DNodeHidden* TempNode = NULL;
	tBundle2DHidden* TempBundle = NULL;
	c1DVectorNode* CurrNode = StreamHead;
	
	if (CurrNode != NULL)
	{
		// Look for the symbol in the bundle below the search stream
		TempNode = Find(&CurrNode->Vector);
		if (TempNode != NULL)
			TempBundle = TempNode->Bottom;
		else
			TempBundle = NULL;
		
		while(CurrNode->Next != NULL)
		{
			CurrNode = CurrNode->Next;
			Order++;
			
			if (CurrNode != NULL)
			{
				if (TempNode!= NULL)
				{
					TempBundle = TempNode->Bottom;
					if (TempBundle == NULL)
					{
						break;
					}
					else
					{
						TempNode = TempBundle->Find(&CurrNode->Vector);
					}
				}
				else
				{
					TempBundle = NULL;
				}
			}
			else
			{
				break;
			}
		}
		
		// When the child or the bundle below the last node in the tree do not exist
		if ((TempNode == NULL) || (TempBundle == NULL))
		{
			FirstTerm = 0;
			SecondTerm = Tracker->GetEscapeProb(Order);
		}
		else
		{
			// TempNode now points to the node in the tree corresponding to the last node
			// in the search stream. Set TempBundle to TempNode->Bottom;
			TempBundle = TempNode->Bottom;
			
			if (TempBundle != NULL)
			{
				if (TempBundle->FirstNode == NULL)
				{
					// When the bundle exists but no children in it
					FirstTerm = 0;
					SecondTerm = Tracker->GetEscapeProb(Order);
				}
				else
				{
					// Get the count of the node
					TempNode = TempBundle->Find(&PredictedSymbol->Vector);
					Order++;
					
					if (TempNode == NULL)
					{
						// If the bundle exists and has children but the predicted child does not exist
						NodeCount = 0;
					}
					else
					{
						NodeCount = TempNode->Count;
					}
					
					// Set the discount ratio
					if (NodeCount > 1)
					{
						Discount = 0;
					}
					else
					{
						Discount = 1;
					}
					
					// Old statement
					//Discount = Tracker->GetDiscountRatio(Order);
					
					
					// Get Sum Count
					ChildrenCount = TempBundle->Length;
					SumCount = TempBundle->TotalCount;
					
					// KneserNeyTermOne
					
					FirstTerm = (double)NodeCount*(1-Discount)/(double)SumCount;
					
					// KneserNey Term Two
					SecondTerm = ((double)Discount*(double)ChildrenCount/(double)SumCount);
				}
			}
			else
			{
				FirstTerm = 0;
				SecondTerm = Tracker->GetEscapeProb(Order);
			}
		}
		Prob = FirstTerm + SecondTerm*KneserNey(PredictedSymbol, StreamHead->Next, Tracker, x);
	}
	else 
	{
		TempNode = Find(&PredictedSymbol->Vector);
		if (TempNode != NULL)
		{
			Prob = TempNode->Prob;
		}
		else
		{
			//Order should be 0 if it comes here, so it's getting the escape at the first level
			Prob = Tracker->GetEscapeProb(Order);
		}
	}
	return Prob;
}

double tBundle2DHidden :: CrossValidate(c1DList* Vector, tTracker* Tracker, void* x)
{
	t2DNodeHidden* TempNode = NULL;
	double Entropy = 0;
	double Prob = 0;
	TempNode = Find(Vector);
	
	if (TempNode != NULL)
	{
		//POST((t_object *)x, "CrossValidation: %f-%f: %f", TempNode->Vector.GetValue(0), TempNode->Vector.GetValue(1), -((log10(TempNode->Prob))/log10(2)));
		if (TempNode->Prob == 0)
		{
			Prob = Tracker->GetEscapeProb(0);
		}
		else
		{
			Prob = TempNode->Prob;
		}
	}
	else 
	{
		Prob = Tracker->GetEscapeProb(0);
	}
	
	Entropy = -((log10(Prob))/log10(2));
	return Entropy;
}

int tBundle2DHidden :: Normalize(void)
{
	double SumProb = NULL;
	t2DNodeHidden* TempNode2 = FirstNode;
	
	// Check if the bundle is null - return failure if it is
	if (FirstNode == NULL)
	{
		return eRETVAL_FAILURE;
	}
	else
	{
		// If the bundle is not null, then search for the node with the max probability.
		SumProb = GetSumProb();
		
		if (SumProb != 0)
		{
			// Once we find this node, go through each node of the bundle
			// and divide its probability by the max probability
			for (TempNode2 = FirstNode; TempNode2 != NULL; TempNode2 = TempNode2->Right)
			{
				TempNode2->Prob = (TempNode2->Prob)/(SumProb);
			}
			return eRETVAL_SUCCESS;
		}
		else
		{
			return eRETVAL_FAILURE;
		}
		
	}
}

t2DNodeHidden* tBundle2DHidden :: GetMaxProb(void)
{
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* MaxNode = NULL;
	double MaxProb = 0;
	
	if (FirstNode == NULL)
	{
		return NULL;
	}
	else
	{
		for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			if(TempNode->Prob > MaxProb)
			{
				MaxProb = TempNode->Prob;
				MaxNode = TempNode;
			}
		}
		return MaxNode;
	}		
}

double tBundle2DHidden :: GetSumProb(void)
{
	t2DNodeHidden* TempNode = NULL;
	double SumProb = 0;
	
	if (FirstNode == NULL)
	{
		return NULL;
	}
	else
	{
		for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			SumProb += TempNode->Prob;
		}
		return SumProb;
	}		
}

void tBundle2DHidden :: MergePDF(tBundle2DHidden* SecondBundle, double ShortTermWeight, double LongTermWeight) 
{
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* TempNode2 = NULL;
	
	// If short term model doesn't exist, or if the entropy is infinity, return
	if (SecondBundle == NULL || ShortTermWeight == INFINITY || 	isnan(ShortTermWeight))
	{
		return;
	}
	
	TempNode = SecondBundle->FirstNode;
	TempNode2 = FirstNode;
	
	// Weight the long term model by short term weight
	for(TempNode2 = FirstNode; TempNode2 != NULL; TempNode2 = TempNode2->Right)
	{
		TempNode2->Prob = (ShortTermWeight)*(TempNode2->Prob);
	}
	
	// Weight the short term model
	for(TempNode = SecondBundle->FirstNode; TempNode!=NULL; TempNode=TempNode->Right)
	{
		TempNode2 = Find(&TempNode->Vector);
		if( TempNode2 == NULL)
		{
			// Add the node to this bundle since it doesn't exist
			Append(&TempNode->Vector);
			TempNode2 = LastNode;
			
			// Assign the probability of the node weighted by the long term weight
			TempNode2->Prob = (TempNode->Prob)*LongTermWeight;
		}
		else
		{
			// since we found the node, we multiply the weight for the depth
			// and then add it to the accumulator
			
			TempNode2->Prob = (TempNode2->Prob) + (TempNode->Prob)*(LongTermWeight);
		}
	}
	
	Normalize();
}

// Implements H = sum(Ei*Pi)
double tBundle2DHidden :: TotalEntropy(tTracker* Tracker)
{
	double SumEntropy = 0;
	t2DNodeHidden* TempNode = NULL;
	
	if (FirstNode != NULL)
	{
		for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			if (TempNode->Prob <= 0)
			{
				SumEntropy += (Tracker->GetEscapeProb(0))*(-log10(Tracker->GetEscapeProb(0))/log10(2));
			}
			else
			{
				SumEntropy += (TempNode->Prob)*(-log10(TempNode->Prob)/log10(2));
			}
		}
		
		return SumEntropy;
	}
	else
	{
		return (Tracker->GetEscapeProb(0))*(-log10(Tracker->GetEscapeProb(0))/log10(2));
	}
}

// Split a given PDF into different PDFs, one corresponding to the number of
// different components that make up the vector. The probabilities are the same
// but the vectors are split up so that it can be merged with the 
// basic types
unsigned int tBundle2DHidden :: SplitPDF(tBundle2DHidden* PDFVectorOne, tBundle2DHidden* PDFVectorTwo)
{
	// For starters I'm just splitting the vectors and copying the PDFs so it's nothing
	// special. But I may want to consider adding all the duration probabilities and 
	// normalizing. Strokes can probably stay the same
	
	unsigned int VectorLength = 0;
	float VectorOneValue = 0;
	float VectorTwoValue = 0;
	c1DList VectorOne(NULL,NULL,0);
	c1DList VectorTwo(NULL,NULL,0);
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* TempNode2 = NULL;
	
	// Return if the bundle doesn't have any nodes
	if (FirstNode == NULL)
	{
		return eRETVAL_FAILURE;
	}
	
	// Make sure the PDF nodes to be returned have been initialized
	if (PDFVectorOne != NULL && PDFVectorTwo != NULL)
	{
		// Count the number of vectors in the nodes of the original PDf
		VectorLength = FirstNode->Vector.GetLength();
		
		// Split the vector into its components - we're dealing with only two right now
		if (VectorLength == 2)
		{
			// Populate the new PDFs with these vector components and their corresponding probabilities
			for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
			{
				// Split the vector values
				VectorOneValue = TempNode->Vector.GetValue(0);
				VectorTwoValue = TempNode->Vector.GetValue(1);
				
				// Create new Vectors with these values
				VectorOne.Append(VectorOneValue);
				VectorTwo.Append(VectorTwoValue);
				
				// Search to see if this vector exists in PDF1
				TempNode2 = PDFVectorOne->Find(&VectorOne);
				
				// If it doesn't exist, create the node
				if (TempNode2 == NULL)
				{
					// Create the node for PDF1
					PDFVectorOne->Append(&VectorOne);
					TempNode2 = PDFVectorOne->LastNode;
					TempNode2->Prob = TempNode->Prob;
				}
				else
				{
					// No need to create the node, since it exists
					// but add the probability
					TempNode2->Prob += TempNode->Prob;
				}
				
				// Reset Tempnode2
				TempNode2 = NULL;
				
				// Search to see if this vector exists in PDF2
				TempNode2 = PDFVectorTwo->Find(&VectorTwo);
				
				// If it doesn't exist, create the node
				if (TempNode2 == NULL)
				{
					// Create the node for PDF2
					PDFVectorTwo->Append(&VectorTwo);
					TempNode2 = PDFVectorTwo->LastNode;
					TempNode2->Prob = TempNode->Prob;
				}
				else
				{
					// No need to create the node, since it exists
					// but add the probability
					TempNode2->Prob += TempNode->Prob;
				}
				
				// Vectors One and Two can now be reset since they have been used.
				VectorOne.DeleteAll();
				VectorTwo.DeleteAll();
			}
			
			// After splitting the distribution normalize each of the components
			PDFVectorOne->Normalize();
			PDFVectorTwo->Normalize();
			
			return eRETVAL_SUCCESS;
		}
		else
		{
			return eRETVAL_FAILURE;
		}
	}
	else
	{
		return eRETVAL_FAILURE;
	}
	return eRETVAL_FAILURE; // TODO: Required?
}

// Used to calculate the weight that is to be applied to a model
// while merging
double tBundle2DHidden :: RelativeEntropy(tTracker* Tracker)
{
	double Entropy = 0;
	double MinProb = 0;
	double MaxEntropy = 0;
	double RelativeEntropy = 0;
	t2DNodeHidden* TempNode = FirstNode;
	
	if (FirstNode == NULL)
	{
		return eRETVAL_FAILURE;
	}
	else
	{
		Entropy = TotalEntropy(Tracker);
		
		MinProb = FirstNode->Prob;
		
		for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			if (TempNode->Prob < MinProb)
			{
				MinProb = TempNode->Prob;
			}
		}
		
		if (MinProb == 0)
		{
			MinProb = Tracker->GetEscapeProb(0);
		}
		
		MaxEntropy = (-log10(MinProb)/log10(2));
		
		// In a case where, only one node is present,
		// that node gets a normzalized probability
		// of 1. Which means the entropy of that distribution
		// becomes 0. 
		
		if (MaxEntropy != 0)
		{
			RelativeEntropy = Entropy/MaxEntropy;
		}
		else
		{
			// Relative Entropy tends to INFINITY in this case.
			RelativeEntropy = 1000;
		}
		
		return RelativeEntropy;
	}
}

// Used to calculate the arithmetic combination of a set of viewpoints
// This function DOES NOT normalize the PDF after adding. This is so that 
// it can be applied for multiple viewpoints. After merging all the viewpoints
// the resulting PDF must be normalized by YOU.
unsigned int tBundle2DHidden :: ArithmeticCombination(tBundle2DHidden* SecondBundle, tTracker* TrackerOne, tTracker* TrackerTwo, bool IsContinued)
{
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* TempNode2 = NULL;
	
	double WeightOne = 0;
	double WeightTwo = 0;
	
	double Exponent = -1;
	
	// If short term model doesn't exist, or if the entropy is infinity, return
	if (SecondBundle == NULL)
	{
		return eRETVAL_FAILURE;
	}
	
	// Get the weights
	WeightOne = pow(RelativeEntropy(TrackerOne), Exponent);
	WeightTwo = pow(SecondBundle->RelativeEntropy(TrackerTwo), Exponent);
	
	// This is a temporary fix to make it work cos otherwise I'll have to write a whole new function
	// This wil occur if the first PDF is not normalized and will return entropies of less than zero.
	// Assuming that I just want to add the pdfs weigthed by entropies and normalize later, this will work.
	if (WeightOne < 0 || IsContinued)
	{
		WeightOne = 1;
	}
	
	TempNode = SecondBundle->FirstNode;
	TempNode2 = FirstNode;
	
	// Weight the long term model by short term weight
	for(TempNode2 = FirstNode; TempNode2 != NULL; TempNode2 = TempNode2->Right)
	{
		TempNode2->Prob = (WeightOne)*(TempNode2->Prob);
	}
	
	// Weight the short term model
	for(TempNode = SecondBundle->FirstNode; TempNode!=NULL; TempNode=TempNode->Right)
	{
		TempNode2 = Find(&TempNode->Vector);
		if( TempNode2 == NULL)
		{
			// Add the node to this bundle since it doesn't exist
			Append(&TempNode->Vector);
			TempNode2 = LastNode;
			
			// Assign the probability of the node weighted by the long term weight
			TempNode2->Prob = (TempNode->Prob)*WeightTwo;
		}
		else
		{
			// since we found the node, we multiply the weight for the depth
			// and then add it to the accumulator
			
			TempNode2->Prob = (TempNode2->Prob) + (TempNode->Prob)*(WeightTwo);
		}
	}
	
	return eRETVAL_SUCCESS;
}

// Used to calculate the geometric combination of a set of viewpoints
// This function DOES NOT normalize the PDF after adding. This is so that 
// it can be applied for multiple viewpoints. After merging all the viewpoints
// the resulting PDF must be normalized by YOU.
unsigned int tBundle2DHidden :: GeometricCombination(tBundle2DHidden* SecondBundle, tTracker* TrackerOne, tTracker* TrackerTwo, bool IsContinued)
{
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* TempNode2 = NULL;
	
	double WeightOne = 0;
	double WeightTwo = 0;
	
	double Exponent = 0;
	
	// If short term model doesn't exist, or if the entropy is infinity, return
	if (SecondBundle == NULL)
	{
		return eRETVAL_FAILURE;
	}
	
	// Get the weights
	WeightOne = pow(RelativeEntropy(TrackerOne), Exponent);
	WeightTwo = pow(SecondBundle->RelativeEntropy(TrackerTwo), Exponent);
	
	// This is a temporary fix to make it work cos otherwise I'll have to write a whole new function
	// This wil occur if the first PDF is not normalized and will return entropies of less than zero.
	// Assuming that I just want to add the pdfs weigthed by entropies and normalize later, this will work.
	if (IsContinued)
	{
		WeightOne = 1;
	}
	
	TempNode = SecondBundle->FirstNode;
	TempNode2 = FirstNode;
	
	// Weight the long term model by short term weight
	for(TempNode2 = FirstNode; TempNode2 != NULL; TempNode2 = TempNode2->Right)
	{
		TempNode2->Prob = pow(TempNode2->Prob, WeightOne);
	}
	
	// Weight the short term model
	for(TempNode = SecondBundle->FirstNode; TempNode!=NULL; TempNode=TempNode->Right)
	{
		TempNode2 = Find(&TempNode->Vector);
		if( TempNode2 == NULL)
		{
			//TEMPORARY HACK: Don't do anything if you don't find the node
			
			// Add the node to this bundle since it doesn't exist
			//Append(&TempNode->Vector);
			//TempNode2 = LastNode;
			
			// Assign the probability of the node weighted by the long term weight
			//TempNode2->Prob = pow(TempNode->Prob, WeightTwo);
		}
		else
		{
			// since we found the node, we multiply the weight for the depth
			// and then add it to the accumulator
			
			TempNode2->Prob = (TempNode2->Prob) * pow(TempNode->Prob, WeightTwo);
		}
	}
	
	return eRETVAL_SUCCESS;
}

// Function to search the bundle for a node and compare it with the most likely node
// Made for cQaidaController so that it can get the distance
double tBundle2DHidden :: DistanceToMostLikelyNode(c1DList* PredictedNode)
{
	t2DNodeHidden* MostLikelyNode = NULL;
	t2DNodeHidden* CurrentPrediction = NULL;
	double Distance = 0;
	double Prob1 = 0;
	double Prob2 = 0;
	
	MostLikelyNode = GetMaxProb();
	CurrentPrediction = Find(PredictedNode);
	
	if ((MostLikelyNode != NULL) && (CurrentPrediction != NULL))
	{
		Prob1 = MostLikelyNode->Prob;
		Prob2 = CurrentPrediction->Prob;
		Distance = EntropyDistance(Prob1, Prob2);
		
		return Distance;
	}
	else {
		return -1;
	}
}


// Entropy distance is a metric that quantifies how likely it would be 
// to predict one symbol, knowing that another symbol was the ideal choice.
// It is simply the distance (or difference) between the entropies of the two symbols.
double tBundle2DHidden :: EntropyDistance(double Probability1, double Probability2)
{
	double Entropy1 = 0;
	double Entropy2 = 0;
	double Distance = -1;
	
	// If probabilities are between 0 and 1
	// then take negative logs and subtract
	if ((Probability1 > 0) && (Probability1 <= 1))
	{
		if ((Probability2 > 0) && (Probability2 <= 1))
		{
			Entropy1 = -log(Probability1)/log(2);
			Entropy2 = -log(Probability2)/log(2);
			
			// Absolute Value of difference between the probs
			Distance = (Entropy1 - Entropy2);
			
			if (Distance >= 0)
			{
				Distance = Distance;
			}
			else {
				Distance = -Distance;
			}
			
			// NOTE: It gave an error when I tried to use the 
			// abs() function, so I just went with a simple 
			// implementation instead.
			
			return Distance;
			
		}
		else {
			// Return -1, this is means error
			return Distance;
		}
		
	}
	else {
		// Return -1, this is means error
		return Distance;
	}
}

//***************************************
#pragma mark -
#pragma mark Posting routines for debugging
//***************************************

//	Output the specified bundle as a list
//	For debugging only; may impair performance
void tBundle2DHidden :: PostBundle(void *pCountOutlet, void *pProbOutlet)
{
	t2DNodeHidden *TempNode = FirstNode;
	double	*TempArray = new double[Length];
	t_atom	*av_count = new t_atom[Length];
	t_atom	*av_prob = new t_atom[Length];
	for(int i=0; (unsigned int)i<Length; i++)
	{
		atom_setlong(av_count+i, TempNode->Vector.GetValue(0));
		TempArray[i] = TempNode->Prob;
		TempNode = TempNode->Right;
	}
	atom_setdouble_array(Length, av_prob, Length, TempArray);
	outlet_list(pProbOutlet, 0L, Length, av_prob);
	outlet_list(pCountOutlet, 0L, Length, av_count);
	delete	TempArray;
	delete	av_count;
	delete	av_prob;
}

// Output the probabilities in the bundle as prints in MAX window
// That's all it does.
void tBundle2DHidden :: PrintBundle(void * x)
{
	if (FirstNode != NULL)
	{
		t2DNodeHidden* TempNode = FirstNode;
		
		POST((t_object *x), (char*)"---- Bundle Start ----");
		
		// Print bundle
		for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			POST((t_object *x), (char*)"Vector: %f, Prob: %f", TempNode->Vector.GetValue(0), TempNode->Prob);
		}
	}
	else {
		POST((t_object *x), (char*)"Empty Bundle!");
	}
}

//	Output the whole trie as a sequence of lists corresponding to bundles
//	For debugging only; may impair performance
//	Recursively calls itself
void tBundle2DHidden :: PostTrie(void *pCountOutlet, void *pProbOutlet)
{
	t2DNodeHidden *TempNode = FirstNode;
	
	//	First put out the values in the bundle
	unsigned short int Length = this->Length;
	double	*TempArray = new double[Length];
	t_atom	*av_vector = new t_atom[Length];
	t_atom	*av_count = new t_atom[Length];
	for(int i=0; (unsigned int)i<Length; i++)
	{
		atom_setlong(av_vector+i, TempNode->Vector.GetValue(0));
		TempArray[i] = TempNode->Count;
		TempNode = TempNode->Right;
	}
	atom_setdouble_array(Length, av_count, Length, TempArray);
	outlet_list(pProbOutlet, 0L, Length, av_vector);
	outlet_list(pCountOutlet, 0L, Length, av_count);
	delete	TempArray;
	delete	av_count;
	delete	av_vector;
	
	//	Then put out the values in the bundles, if any, below each node of the current bundle
	TempNode = FirstNode;
	while(TempNode != NULL)
	{
		// POST((t_object *)this, "    -");
		if(TempNode->Bottom != NULL)
		{
			TempNode->Bottom->PostTrie(pCountOutlet, pProbOutlet);
			// POST((t_object *)this, "        o");
		}
		TempNode = TempNode->Right;
	}
}

//	Output the whole trie as a sequence of XML nodes so that
//	we can use the Treeviz program to draw out the tree
//	Recursively calls itself
void tBundle2DHidden :: PostTrieXML(ofstream &OutputFile, int InitialOrder)
{
	// Declarations
	//int Name = 0;
	int Count = 0;
	char Year[4];
	char Bol[3];
	char CountChar[6];
	t2DNodeHidden *TempNode = FirstNode;
	
	// Check that the file is not null
	if (OutputFile != NULL)
	{
		// Navigate into the first node of the top bundle
		
		TempNode = FirstNode;
		while(TempNode != NULL)
		{
			InitialOrder++;
			
			// Get the name of the symbol
			int Name = TempNode->Vector.GetValue(0);
			
			// Get the count
			Count = TempNode->Count;
			
			// Get the name string
			sprintf(Bol, "%d", Name);
			
			// Get the count string
			sprintf(CountChar, "%d", Count);
			
			// Get the year string
			sprintf(Year, "%04d", Count);
			
			// For each node of the tree print:
			// <Folder name="Symbol 1" created="0000-00-00 00:00:00">
			//		<File name="Count" created="0000-00-00 00:00:00" size="4" />
			for (int i = 0; i < InitialOrder; i++)
			{
				OutputFile << "\t";
			}
			
			OutputFile << "<Folder name=\"Symbol " << Bol << "\" created=\"" << Year << "-00-00 00:00:00\">\n";
			
			for (int i = 0; i < InitialOrder; i++)
			{
				OutputFile << "\t";
			}
			
			OutputFile << "\t<File name=\"Count\" created=\"" << Year << "-00-00 00:00:00\" size=\"" << CountChar << "\" />\n";
			
			
			if(TempNode->Bottom != NULL)
			{
				TempNode->Bottom->PostTrieXML(OutputFile, InitialOrder);
			}
			
			for (int i = 0; i < InitialOrder; i++)
			{
				OutputFile << "\t";
			}
			
			// Close the folder tag
			// </Folder>
			OutputFile << "</Folder>\n";
			
			// Move to the next node
			TempNode = TempNode->Right;
		}
	}
	else
	{
		// Nothing
	}
}