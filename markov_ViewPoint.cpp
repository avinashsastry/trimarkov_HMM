/*
 *  markov_ViewPoint.cpp
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/24/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

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
#include "markov_ViewPoint.h"

cViewPoint :: cViewPoint()
{
	Stream.SetDim(0);
	UseShortTermModel = TRUE;
}

cViewPoint :: cViewPoint(unsigned int Dim = 0)
{
	Stream.SetDim(Dim);
	UseShortTermModel = TRUE;
}

//	Wrapper for tBundle2D :: GrowAll(...)
int	cViewPoint :: GrowAll(tTracker * Tracker, void *x)
{
	return Trie.GrowAll(&Stream, Tracker, x);
}

//	Wrapper for tBundle2D :: PredictBest(...)
int	cViewPoint :: PredictBest(c1DList *Outcome, void *x)
{
	return Trie.PredictBest(&Stream, Outcome, x);
}

int	cViewPoint :: PredictBest(tBundle2D *Outcome, void *x)
{
	return Trie.PredictBest(&Stream, Outcome, x);
}

void cViewPoint :: SetUseShortTermModel(bool TrueOrFalse)
{
	UseShortTermModel = TrueOrFalse;
}

int cViewPoint :: AddToStream(float input1, void *x)
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
		Trie.GrowAll(&Stream, &Tracker, x);
		
		Vector.DeleteAll();
		
		return eRETVAL_SUCCESS;
	}
	else
	{
		return eRETVAL_FAILURE;
	}
}

int cViewPoint :: AddToStream(float input1, float input2, void *x)
{
	c1DList Vector(NULL,NULL,0);
	Vector.Append(input1);
	Vector.Append(input2);
	
	if (Stream.GetDim() == 0)
	{
		Stream.SetDim(2);
	}
	
	// Check for dimensions of the input stream
	// Add only if it is a single dimension stream
	if (Stream.GetDim() == 2)
	{
		Stream.Append(&Vector);
		
		// If the tree has a maximum order limit, left shift the input stream
		// so that only that part is fed into the tree
		if (Stream.GetLength() > MAX_ORDER)
		{
			Stream.LeftShift();
		}
		
		// Grow the tree
		Trie.GrowAll(&Stream, &Tracker, x);		
		
		Vector.DeleteAll();
		
		return eRETVAL_SUCCESS;
	}
	else
	{
		return eRETVAL_FAILURE;
	}
}

int cViewPoint :: AddToSearchStream(float input1, void *x)
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
		TrieShortTerm.GrowAll(&SearchStream, &TrackerShortTerm, x);
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

int cViewPoint :: AddToSearchStream(float input1, float input2, void* x)
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
		TrieShortTerm.GrowAll(&SearchStream, &TrackerShortTerm, x);
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

int cViewPoint :: PredictNext(void *x)
{
	tBundle2D* OutcomeLT = new tBundle2D();
	tBundle2D* OutcomeST = new tBundle2D();
	tBundle2D* OutcomeCT = new tBundle2D();
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

int cViewPoint :: PredictNextKN(void *x)
{
	tBundle2D* OutcomeLT = new tBundle2D();
	tBundle2D* OutcomeST = new tBundle2D();
	tBundle2D* OutcomeCT = new tBundle2D();
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

int cViewPoint :: ResetViewpoint(int ResetType)
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
			TrieShortTerm.DeleteBundle();
			
			// Reset both Trackers
			Tracker.DeleteAll();
			TrackerShortTerm.DeleteAll();
			
			// Set all PDFs to NULL
			PrevPDF = NULL;
			PrevPDFShortTerm = NULL;
			PrevPDFLongTerm = NULL;
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

int cViewPoint :: PredictStrokePIS(float PositionInSong, void* x)
{
	tBundle2D* OutcomeST = new tBundle2D();
	t2DNode* TempNode = NULL;
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
		PrevPDFShortTerm = new tBundle2D();
		
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