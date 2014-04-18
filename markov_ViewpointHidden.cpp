/*
 *  markov_ViewpointHidden.cpp
 *  markov
 *
 *  Created by Avinash on 10/1/10.
 *  Copyright 2010 GTCMT. All rights reserved.
 *
 */

#include "markov_Gaussian.h"
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
#include "markov_Bundle2DHidden.h"
#include "markov_2DNodeHidden.h"
#include "markov_ViewpointHidden.h"

cViewPointHidden :: cViewPointHidden()
{
	Stream.SetDim(0);
	UseShortTermModel = TRUE;
	PrevPDF = new tBundle2DHidden();
}

cViewPointHidden :: cViewPointHidden(unsigned int Dim = 0)
{
	Stream.SetDim(Dim);
	UseShortTermModel = TRUE;
}

//	Wrapper for tBundle2D :: GrowAll(...)
int	cViewPointHidden :: GrowAll(tTracker * Tracker, void *x)
{
	return Trie.GrowAll(&Stream, &SymbolStream, Tracker, x, &StrokeMeans[0][0], &StrokeVariance[0][0], &StrokeInverse[0][0], &StrokeDeterminant);		

}

//	Wrapper for tBundle2D :: PredictBest(...)
int	cViewPointHidden :: PredictBest(c1DList *Outcome, void *x)
{
	return Trie.PredictBest(&Stream, Outcome, x);
}

int	cViewPointHidden :: PredictBest(tBundle2DHidden *Outcome, void *x)
{
	return Trie.PredictBest(&SearchStream, Outcome, x);
}

void cViewPointHidden :: SetUseShortTermModel(bool TrueOrFalse)
{
	UseShortTermModel = TrueOrFalse;
}

int cViewPointHidden :: AddToStream(float input1, void *x)
{
	c1DList Vector(NULL,NULL,0);
	Vector.Append(input1);
	
	if (Stream.GetDim() == 0)
	{
		Stream.SetDim(1);
	}
	
	// Check for dimensions of the input stream
	// Add only if it is a single dimension stream
	if (Stream.GetDim() == 1)
	{
		Stream.Append(&Vector);
		
		// If the tree has a maximum order limit, left shift the input stream
		// so that only that part is fed into the tree
		if (Stream.GetLength() > MAX_ORDER)
		{
			Stream.LeftShift();
		}
		
		// Grow the tree
		Trie.GrowAll(&Stream, &SymbolStream, &Tracker, x, &StrokeMeans[(unsigned int)input1][0], &StrokeVariance[0][0], &StrokeInverse[0][0], &StrokeDeterminant);		
		
		Vector.DeleteAll();
		
		return eRETVAL_SUCCESS;
	}
	else
	{
		return eRETVAL_FAILURE;
	}
}

// This function does the following:
// 1. Add the state to the StateStream
// 2. Add the symbol to the SymbolStream
// 3. Add the state to the Trie
// 4. Add the symbol to the current state in the Trie.

// The function is recursive so it should update each level of the
// trie, until the entire input stream of both states and symbols is
// covered.

int cViewPointHidden :: AddToStream(float InputState, float InputSymbol, void *x)
{
	// Add State to the state stream
	
	c1DList Vector(NULL,NULL,0);
	Vector.Append(InputState);
	
	if (Stream.GetDim() == 0)
	{
		Stream.SetDim(1);
	}
	
	// Check for dimensions of the input stream
	// Add only if it is a single dimension stream
	if (Stream.GetDim() == 1)
	{
		Stream.Append(&Vector);
		
		// If the tree has a maximum order limit, left shift the input stream
		// so that only that part is fed into the tree
		if (Stream.GetLength() > MAX_ORDER)
		{
			Stream.LeftShift();
		}
	}
	
	Vector.DeleteAll();
	
	// Add symbol to the symbol stream
	
	Vector.Append(InputSymbol);
	
	if (SymbolStream.GetDim() == 0)
	{
		SymbolStream.SetDim(1);
	}
	
	if (SymbolStream.GetDim() == 1)
	{
		SymbolStream.Append(&Vector);
		
		if (SymbolStream.GetLength() > MAX_ORDER)
		{
			SymbolStream.LeftShift();
		}
	}
	
	// Grow the tree
	Trie.GrowAll(&Stream, &SymbolStream, &Tracker, x, &StrokeMeans[((unsigned int)InputState-1)][0], &StrokeVariance[0][0], &StrokeInverse[0][0], &StrokeDeterminant);		
	
	Vector.DeleteAll();
	
	return eRETVAL_SUCCESS;
}

int cViewPointHidden :: AddToSearchStream(float input1, void *x)
{
	c1DList Vector(NULL, NULL, 0);
	Vector.Append(input1);
	
	// If SearchStream has not been initialized then set dimension
	if(SearchStream.GetDim() == 0)
	{
		SearchStream.SetDim(1);
	}
	
	if (SearchStream.GetDim() == 1)
	{
		// Add the input value to the Search Stream
		SearchStream.Append(&Vector);
		
		// Maintain the order
		if (SearchStream.GetLength() > MAX_ORDER)
		{
			SearchStream.LeftShift();
		}
	}
	
	// If the short term model is active then
	// grow the short term trie on the search stream
	if (IS_SHORT_TERM_MODEL)
	{
		// TrieShortTerm.GrowAll(&SearchStream, &TrackerShortTerm, x);
	}
	//CrossValidate priors
	EntropyPrior = Trie.CrossValidate(&Vector, &Tracker, x);
	
	//Crossvalidate long term
	if (PrevPDFLongTerm != NULL)
	{
		EntropyLongTerm = PrevPDFLongTerm->CrossValidate(&Vector, &Tracker, x);
	}
	else
	{
		EntropyLongTerm = -log10(Tracker.GetEscapeProb(0))/log10(2);
	}
	
	//Crossvalidate the short term
	if (PrevPDFShortTerm != NULL)
	{
		EntropyShortTerm = PrevPDFShortTerm->CrossValidate(&Vector, &TrackerShortTerm, x);
	}
	else
	{
		EntropyShortTerm = -log10(TrackerShortTerm.GetEscapeProb(0))/log10(2);
	}
	
	//Crossvalidate the merged model
	if (PrevPDF != NULL)
	{
		EntropyCombined = PrevPDF->CrossValidate(&Vector, &Tracker, x);
	}
	else
	{
		EntropyCombined = -log10(Tracker.GetEscapeProb(0))/log10(2);
	}
	
	Vector.DeleteAll();
	return eRETVAL_SUCCESS;
}

int cViewPointHidden :: AddToSearchStream(float input1, float input2, void* x)
{
	c1DList Vector(NULL, NULL, 0);
	Vector.Append(input1);
	Vector.Append(input2);
	
	// If SearchStream has not been initialized then set dimension
	if(SearchStream.GetDim() == 0)
	{
		SearchStream.SetDim(2);
	}
	
	// Add the input value to the Search Stream
	if (SearchStream.GetDim() == 2)
	{
		SearchStream.Append(&Vector);
		
		// Maintain the order
		if (SearchStream.GetLength() > MAX_ORDER)
		{
			SearchStream.LeftShift();
		}
	}
	
	// If the short term model is active then
	// grow the short term trie on the search stream
	if (IS_SHORT_TERM_MODEL)
	{
		// TrieShortTerm.GrowAll(&SearchStream, &TrackerShortTerm, x);
	}
	
	//CrossValidate priors
	EntropyPrior = Trie.CrossValidate(&Vector, &Tracker, x);
	
	//Crossvalidate long term
	if (PrevPDFLongTerm != NULL)
	{
		EntropyLongTerm = PrevPDFLongTerm->CrossValidate(&Vector, &Tracker, x);
	}
	else
	{
		EntropyLongTerm = -log10(Tracker.GetEscapeProb(0))/log10(2);
	}
	
	//Crossvalidate the short term
	if (PrevPDFShortTerm != NULL)
	{
		EntropyShortTerm = PrevPDFShortTerm->CrossValidate(&Vector, &TrackerShortTerm, x);
	}
	else
	{
		EntropyShortTerm = -log10(TrackerShortTerm.GetEscapeProb(0))/log10(2);
	}
	
	//Crossvalidate the merged model
	if (PrevPDF != NULL)
	{
		EntropyCombined = PrevPDF->CrossValidate(&Vector, &Tracker, x);
	}
	else
	{
		EntropyCombined = -log10(Tracker.GetEscapeProb(0))/log10(2);
	}
	
	Vector.DeleteAll();
	return eRETVAL_SUCCESS;
}

unsigned int cViewPointHidden :: AddFeaturesToSearchStream(float* FeatureVector, c1DList* DataToken)
{
	SearchStream.AddFeatures(FeatureVector, DataToken);
	
	if (SearchStream.GetLength() > MAX_ORDER)
	{
		SearchStream.LeftShift();
	}
	
	return eRETVAL_SUCCESS;
}

int cViewPointHidden :: PredictNext(void *x)
{
	tBundle2DHidden* OutcomeLT = new tBundle2DHidden();
	tBundle2DHidden* OutcomeST = new tBundle2DHidden();
	tBundle2DHidden* OutcomeCT = new tBundle2DHidden();
	double TempEntropy1 = 0;
	double TempEntropy2 = 0;
	unsigned int retval = eRETVAL_UNDEFINED;
	
	// Call the prediction function. This will give out a probability distribution
	retval = Trie.PredictSmoothBest(&SearchStream, OutcomeLT, &Tracker, x);
	
	// If the prediction fails, assign NULL to the outcome
	if(retval & eRETVAL_FAILURE)
	{
		PrevPDFLongTerm = NULL;
	}
	else
	{
		// Store this probability distribution
		OutcomeLT->Normalize();
		PrevPDFLongTerm = OutcomeLT;
	}
	
	// Call predict on the short term model
	retval = TrieShortTerm.PredictSmoothBest(&SearchStream, OutcomeST, &TrackerShortTerm, x);
	
	if(retval & eRETVAL_FAILURE)
	{
		PrevPDFShortTerm = NULL;
	}
	else
	{
		// Store this probability distribution
		OutcomeST->Normalize();
		PrevPDFShortTerm = OutcomeST;
	}
	
	// If the short term model gave prediction, then merge
	if (!(retval & eRETVAL_FAILURE))
	{
		// Combine the two predictions
		// Call predict again so that the Combined model is the same as the
		// Long term model, then the merge function can be used to combine with
		// the short term model
		retval = Trie.PredictSmoothBest(&SearchStream, OutcomeCT, &Tracker, x);
		
		if (retval & eRETVAL_FAILURE)
		{
			PrevPDF = NULL;
		}
		else
		{
			// Normalize the model and assign the pointer
			OutcomeCT->Normalize();
			PrevPDF = OutcomeCT;
			
			//Get the long term entropy
			TempEntropy1 = PrevPDFLongTerm->TotalEntropy(&Tracker);
			//Get the shortTerm entropy
			TempEntropy2 = PrevPDFShortTerm->TotalEntropy(&TrackerShortTerm);
			
			//Merge the PDFs
			if (UseShortTermModel)
			{
				PrevPDF->MergePDF(PrevPDFShortTerm, TempEntropy2, TempEntropy1);
			}
		}
	}
	else
	{
		PrevPDF = NULL;
	}
	
	return retval;
}

int cViewPointHidden :: PredictNextKN(void *x)
{
	tBundle2DHidden* OutcomeLT = new tBundle2DHidden();
	tBundle2DHidden* OutcomeST = new tBundle2DHidden();
	tBundle2DHidden* OutcomeCT = new tBundle2DHidden();
	double TempEntropy1 = 0;
	double TempEntropy2 = 0;
	unsigned int retval = eRETVAL_UNDEFINED;
	
	// Call the prediction function. This will give out a probability distribution
	retval = Trie.PredictKneserNey(&SearchStream, OutcomeLT, &Tracker, x);
	
	// If the prediction fails, assign NULL to the outcome
	if(retval & eRETVAL_FAILURE)
	{
		PrevPDFLongTerm = NULL;
	}
	else
	{
		// Store this probability distribution
		OutcomeLT->Normalize();
		PrevPDFLongTerm = OutcomeLT;
	}
	
	// Call predict on the short term model
	retval = TrieShortTerm.PredictKneserNey(&SearchStream, OutcomeST, &TrackerShortTerm, x);
	
	if(retval & eRETVAL_FAILURE)
	{
		PrevPDFShortTerm = NULL;
	}
	else
	{
		// Store this probability distribution
		OutcomeST->Normalize();
		PrevPDFShortTerm = OutcomeST;
	}
	
	// If the short term model gave prediction, then merge
	if (!(retval & eRETVAL_FAILURE))
	{
		// Combine the two predictions
		// Call predict again so that the Combined model is the same as the
		// Long term model, then the merge function can be used to combine with
		// the short term model
		retval = Trie.PredictKneserNey(&SearchStream, OutcomeCT, &Tracker, x);
		
		if (retval & eRETVAL_FAILURE)
		{
			PrevPDF = NULL;
		}
		else
		{
			// Normalize the model and assign the pointer
			OutcomeCT->Normalize();
			PrevPDF = OutcomeCT;
			
			//Get the long term entropy
			TempEntropy1 = PrevPDFLongTerm->TotalEntropy(&Tracker);
			//Get the shortTerm entropy
			TempEntropy2 = PrevPDFShortTerm->TotalEntropy(&TrackerShortTerm);
			
			//Merge the PDFs
			PrevPDF->MergePDF(PrevPDFShortTerm, TempEntropy2, TempEntropy1);
		}
	}
	else
	{
		PrevPDF = NULL;
	}
	
	return retval;
}

double cViewPointHidden :: EvaluateStream(void *x)
{
	return Trie.EvaluateSearchStream(&SearchStream, x);
}

//  This function finds the forward probability of 
//  every possible stream given the set of features
//  and calculates a probability distribution over
//  the different possible symbols
double cViewPointHidden :: EvaluateStreamContinuous(void *x)
{
	double ForwardProb = 0;
	t2DNodeHidden* TempNode = NULL;
	
	// Create a NULL distribution with the initialized
	// gaussians and all prior probs set to NULL
	UpdateNodesInPDF(0);
	
	for (TempNode = PrevPDF->FirstNode; TempNode != NULL; TempNode = TempNode->Right)
	{
		ForwardProb = Trie.EvaluateSearchStreamContinuous(&SearchStream, &TempNode->Vector, &Tracker, x);
		//  Set the forward prob as the prior for the PDF
		TempNode->Prob = ForwardProb;	
	}
	
	return 0;
}

//  Calculates the backward probability for the search sequence,
//  given that it ended on a particular end-state 
double cViewPointHidden :: GetBackwardProbability(c1DList* NodeVector, void * x)
{
	// Call GetBackwardProbability() on the top level bundle
	// Atleast for now, till I figure out exactly what I need from 
	// this function

	return Trie.GetBackwardProbabilityForNode(&SearchStream, NodeVector, &Tracker, x); 
}

//  ForwardBackwardAlgorithm()
//  Top level function to run the Fwd-Back Algo on
//  the entire tree at every time step.
double cViewPointHidden :: ForwardBackwardAlgorithm(void* x)
{
	Tracker.SetAllWeights(SearchStream.GetLength());
	Tracker.ResetOrder();
	return Trie.AdjustProbabilities(&Trie, &SearchStream, 0, &Tracker, x);
}

unsigned int cViewPointHidden :: DecodeStream(c1DVectorList *ResultStream ,void *x)
{
	return Trie.DecodeSearchStream(&SearchStream, ResultStream, x);
}

unsigned int cViewPointHidden :: UpdateNodesInPDF(double ForwardProb)
{
	t2DNodeHidden* TempNode = NULL;
	t2DNodeHidden* TempNode2 = NULL;
	float InputStroke = 0;
	
	//  Check if PrevPDF exists and create it if it doesn't
	if (PrevPDF == NULL)
	{
		PrevPDF = new tBundle2DHidden();
	}
	
	if (Trie.FirstNode != NULL)
	{
		// For each node in the Trie
		for (TempNode = Trie.FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			// Find the node in the trie in PrevPDF
			TempNode2 = PrevPDF->Find(&TempNode->Vector);
			
			// If you find it
			if (TempNode2 != NULL)
			{
				// Set its prob
				TempNode2->Prob = ForwardProb;
			}
			else 
			{
				// If you don't find it, create the node
				// and set its probabilty to the forward prob.
				
				// Get the stroke in the vector
				InputStroke = TempNode->Vector.GetValue(0);
				
				// Append the node and then update the prob
				PrevPDF->Append(&TempNode->Vector, ForwardProb, &StrokeMeans[((unsigned int)InputStroke-1)][0], &StrokeVariance[0][0], &StrokeInverse[0][0], &StrokeDeterminant);	
			}
		}
	}
	else 
	{
		// Trie does not exist. Do nothing.
	}
	
	return eRETVAL_SUCCESS;
}

int cViewPointHidden :: ResetViewpoint(int ResetType)
{ 
	switch(ResetType)
	{
			// reset input stream
		case 1: 
			Stream.DeleteAll();
			break;
			// reset tree
		case 2:
			// Reset both tries
			Trie.DeleteBundle();
			
			// Reset tracker
			Tracker.DeleteAll();
			
			// Erase the Streams
			Stream.DeleteAll();
			SearchStream.DeleteAll();
			SymbolStream.DeleteAll();
			break;
			//reset short term tries
		case 3:
			SearchStream.DeleteAll();
			TrieShortTerm.DeleteBundle();
			TrackerShortTerm.DeleteAll();
			// Set all PDFs to NULL
			PrevPDF = NULL;
			PrevPDFShortTerm = NULL;
			PrevPDFLongTerm = NULL;
			break;
		case 4:
			// reset search streams
			SearchStream.DeleteAll();
			PrevPDF = NULL;
			PrevPDFLongTerm = NULL;
			PrevPDFShortTerm = NULL;
			break;
		default:
			break;
	}
	// Complete reset of the viewpoint
	return eRETVAL_SUCCESS;
}

/**********************************************************************
 
 Special Functions - Below are functions that may be specific to some 
 viewpoints. These functions must be used in the right context so use
 carefully.
 
 **********************************************************************/

// PredictStrokePIS()
// Predicts stroke values given the position of the next event in the song
// Works only with StrokePIS viewpoint
// This should use only the ST model otherwise it may end up
// picking up junk that is not consistent with the song

int cViewPointHidden :: PredictStrokePIS(float PositionInSong, void* x)
{
	tBundle2DHidden* OutcomeST = new tBundle2DHidden();
	t2DNodeHidden* TempNode = NULL;
	c1DList Vector(NULL, NULL, 0);
	int retval = eRETVAL_UNDEFINED;
	
	Vector.Append(PositionInSong);
	
	// Call predict on the short term model
	retval = TrieShortTerm.PredictSmoothBest(&SearchStream, OutcomeST, &TrackerShortTerm, x);
	
	if(retval & eRETVAL_FAILURE)
	{
		PrevPDFShortTerm = NULL;
	}
	else
	{
		// If we made a prediction on the ST,
		// then we need to filter the strokes 
		// based on the position given.
		
		// First reset PrevPDFShortTerm
		PrevPDFShortTerm = new tBundle2DHidden();
		
		for (TempNode = OutcomeST->FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			// if the PIS matches, then 
			if (TempNode->Vector.GetValue(1) == Vector.GetValue(0))
			{
				// Append the node to PrevPDFShortTerm
				PrevPDFShortTerm->Append(&TempNode->Vector);
				// This node will now be the last node
				// So set its probability
				PrevPDFShortTerm->LastNode->Prob = TempNode->Prob;
			}
			else
			{
				// Do absolutely nothing
			}
		}
		
		if (PrevPDFShortTerm->FirstNode != NULL)
		{
			// Store this probability distribution
			PrevPDFShortTerm->Normalize();
		}
		else
		{
			PrevPDFShortTerm = OutcomeST;
			PrevPDFShortTerm->Normalize();
		}
	}
	return eRETVAL_SUCCESS;
}