/*
 *  markov_2DNodeHidden.cpp
 *  markov
 *
 *  Created by Avinash on 9/30/10.
 *  Copyright 2010 GTCMT. All rights reserved.
 *
 */
#undef check

#include "ext.h"
#include "ext_obex.h"

#include "armadillo"

#include "markov_defines.h"
#include "markov_enums.h"
#include "markov_classes.h"
#include "markov_1DNode.h"
#include "markov_1DList.h"
#include "markov_Bundle2D.h"
#include "markov_2DNode.h"
#include "markov_Gaussian.h"
#include "markov_2DNodeHidden.h"
#include "markov_Bundle2DHidden.h"

//	Constructor
t2DNodeHidden :: t2DNodeHidden(t2DNodeHidden *Left = NULL, t2DNodeHidden*Right = NULL, tBundle2DHidden *Bundle = NULL, tBundle2DHidden *Bottom = NULL, c1DList *Vector = NULL, unsigned long int Count = 0, double Prob = 0)
{
	this->Left		=	Left;
	this->Right		=	Right;
	this->Bundle	=	Bundle;
	this->Bottom	=	Bottom;
	this->Vector	=	Vector; // Overloaded '='
	this->Count		=	Count;
	this->Prob		=	Prob;
	this->Emission	=	tBundle2D(NULL, NULL, NULL, 0, 0, 0);
};

t2DNodeHidden :: 	t2DNodeHidden(t2DNodeHidden *Left, t2DNodeHidden *Right, tBundle2DHidden *Bundle, tBundle2DHidden *Bottom, c1DList *Vector, unsigned long int Count, double Prob, float* MeanVector, float* Variance, float* Inverse, float* Determinant)
{
	this->Left		=	Left;
	this->Right		=	Right;
	this->Bundle	=	Bundle;
	this->Bottom	=	Bottom;
	this->Vector	=	Vector; // Overloaded '='
	this->Count		=	Count;
	this->Prob		=	Prob;
	this->Emission	=	tBundle2D(NULL, NULL, NULL, 0, 0, 0);
	
	// initialize the gaussian
	this->EmissionDistribution =  tGaussian(MeanVector, Variance, Inverse, Determinant);
};

//	Destructor
t2DNodeHidden :: ~t2DNodeHidden()
{
	Vector.DeleteAll(); // TODO: Any pass by value vs. pointer issues here?
	Vector	=	NULL;
	Left	=	NULL;
	Right	=	NULL;
	Bundle	=	NULL;
	if(Bottom!=NULL) // TODO: Reqd?
		delete Bottom;
	Bottom	=	NULL;
};

// Emit a random symbol based on the probabilities of the symbols
// Used along with the Predict functions
unsigned int t2DNodeHidden :: Emit(c1DList *outcome, void *x)
{
	unsigned int RetVal = eRETVAL_FAILURE;
	
	if (Emission.FirstNode != NULL)
	{
		RetVal = Emission.Roll(outcome, x);
	}
	return RetVal;
};

// Function to Update the symbols probabilities in a node (state) in the
// HMM model.
// If the symbol is not NULL,
// Scan through the bundle and try to find it
// If it exists, add one to its count.
// If it doesn't, then create a new node.
// Now update all the probabilities in the bundle.

unsigned int t2DNodeHidden :: UpdateSymbol(c1DList *SymbolVector, void *x)
{
	unsigned int RetVal = eRETVAL_FAILURE;
	t2DNode *TempNode = NULL;
	
	if (SymbolVector != NULL)
	{
		// Find the SymbolVector in the Emission Bundle
		TempNode = Emission.Find(SymbolVector);
		
		if (TempNode == NULL)
		{
			Emission.Append(SymbolVector);
		}

		// This I just discovered: Append does NOT add one to the count!
		// Very surprising. So I am updating the count here.
		
		TempNode = Emission.Find(SymbolVector);
		
		// This time the node should exist for sure
		TempNode->Count++;
		Emission.TotalCount++;
		
		// Update probabilities of all nodes in Emission.
		TempNode = Emission.FirstNode;
		
		for (TempNode = Emission.FirstNode; TempNode != NULL; TempNode = TempNode->Right) 
		{
			TempNode->Prob = ((double)TempNode->Count)/((double)Emission.TotalCount);
			//POST((t_object* x), (char *)"Updated Symbol %f: %f", TempNode->Vector.GetValue(0), TempNode->Prob);
		}
		
		RetVal = eRETVAL_SUCCESS;
	}
	
	return RetVal;
}

// Function to return the emission probability of a given Symbol (or vector of symbols)
double t2DNodeHidden :: EmissionProbability(c1DList *Vector)
{
	double Prob = 0;
	t2DNode *TempNode = NULL;
	
	// If the symbol is not NULL
	if (Vector != NULL)
	{
		TempNode = Emission.Find(Vector);
		
		if (TempNode != NULL)
		{
			// If the symbol was found
			// return the prob
			
			Prob = TempNode->Prob;
		}
		else
		{
			// If not found, return 0.
			Prob = 0;
		}
	}
	else 
	{
		// If the symbol is NULL return 0
		Prob = 0;
	}
	
	return Prob;
}

double t2DNodeHidden :: GaussianLikelihood(float* Features)
{
	return EmissionDistribution.GetLikelihood(Features, NUM_FEATURES);
}