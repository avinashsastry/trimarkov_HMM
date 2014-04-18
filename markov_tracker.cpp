/*
 *  markov_tracker.cpp
 *  markov
 *
 *  Created by Rm202 on 2/8/10.
 *  Copyright 2010 Georgia Institute of Technology. All rights reserved.
 *
 */
//#include "ext.h"

//#include <cstdlib>
//#include <algorithm>

#include "markov_defines.h"
#include "markov_enums.h"
//#include "markov_classes.h"
#include "markov_tracker.h"

//	Constructor
tTrackerNode :: tTrackerNode(unsigned int NumTokens = 0, unsigned int NumTypes = 0, double EscapeProb = 0, tTrackerNode *Prev = NULL, tTrackerNode *Next = NULL)
{
	this->NumTokens = NumTokens;
	this->NumTypes = NumTypes;
	this->NumTypes2 = 0;
	this->Weight = 0;
	this->DiscountRatio = 0;
	this->EscapeProb = EscapeProb;
	this->Prev = Prev;
	this->Next = Next;
}
	

//	Destructor
tTrackerNode :: ~tTrackerNode()
{
	this->NumTokens = 0;
	this->NumTypes = 0;
	this->EscapeProb = 0;
	this->Prev = NULL;
	this->Next = NULL;
}



//	Constructor
tTracker :: tTracker(tTrackerNode * FirstNode = NULL, tTrackerNode * LastNode = NULL, unsigned int Length = 0)
{
	this->FirstNode = FirstNode;
	this->LastNode = LastNode;
	this->Length = Length;
	this->Order = 0;
	this->Multiplier = 1;
	this->Offset = 0.1;
	this->Exponent = 0.5;
}

tTracker :: tTracker()
{
	this->FirstNode = NULL;
	this->LastNode = NULL;
	this->Length = 0;
	this->Order = 0;
	this->Multiplier = 1;
	this->Offset = 0.1;
	this->Exponent = 0.5;
}

//	Destructor
tTracker :: ~tTracker()
{
	while(Length>0)
	{
		FirstNode = LastNode->Prev;
		delete LastNode;
		FirstNode->Next = NULL;
		LastNode = FirstNode;
		Length--;
	}
}

//	GetNumTokens - return the number of tokens at a given order
unsigned int tTracker :: GetNumTokens(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
		return TempNode->NumTokens;
	else
		return eRETVAL_FAILURE;
}	

//	GetNumTypes1 - return the number of types at a given order that have occurred exactly once
unsigned int tTracker :: GetNumTypes(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
		return TempNode->NumTypes;
	else
		return eRETVAL_FAILURE;
}	

// GetNumTypes2 - return the number of types that have occurred exactly twice
unsigned int tTracker :: GetNumTypes2(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
		return TempNode->NumTypes2;
	else
		return eRETVAL_FAILURE;
}

//	GetEscapeProb - return the EscapeProb at a given order
double tTracker :: GetEscapeProb(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
		return TempNode->EscapeProb;
	else
		return NULL;
}	

//	GetLength - return the length of the stream
unsigned int tTracker :: GetLength(void)
{
	return Length;
}

//	Append - append a node at the end of the stream
int	tTracker :: Append(void)
{
	tTrackerNode *NewNode = new tTrackerNode();
	if(FirstNode == NULL)
	{
		FirstNode = NewNode;
		LastNode = NewNode;
	}
	else
	{
		LastNode->Next = NewNode;
		NewNode->Prev = LastNode;
		LastNode = NewNode;
	}
	Length++;
	return 0;
}

unsigned int tTracker :: SetParameters(float a, float c, float x)
{
	Multiplier = a;
	Offset = c;
	Exponent = x;
	return eRETVAL_SUCCESS;
}

unsigned int tTracker :: SetParameters(float a, float c)
{
	Mu = a;
	Sigma = c;
	return eRETVAL_SUCCESS;
}

// GetOrder - return the order of the current node that it is tracking
unsigned int tTracker :: GetOrder(void)
{
	return Order;
}

unsigned int tTracker :: UpdateNumTokens(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		TempNode->NumTokens++;
		return eRETVAL_SUCCESS;
	}
	else
		return eRETVAL_FAILURE;
}

unsigned int tTracker :: UpdateNumTypes(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		TempNode->NumTypes++;
		return eRETVAL_SUCCESS;
	}
	else
		return eRETVAL_FAILURE;
}

unsigned int tTracker :: UpdateNumTypes2(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		TempNode->NumTypes2++;
		return eRETVAL_SUCCESS;
	}
	else
		return eRETVAL_FAILURE;
}

unsigned int tTracker :: UpdateEscapeProb(unsigned int Order, void* x)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		// POST((t_object* x), "NumTokens = %d; NumTypes = %d", TempNode->NumTokens, TempNode->NumTypes); 
		if (TempNode->NumTypes == 0)
		{
			TempNode->EscapeProb = (double)1/(double)(TempNode->NumTokens + 1);
		}
		else if (TempNode->NumTypes == TempNode->NumTokens)
		{
			TempNode->EscapeProb = (double)1/(double)(TempNode->NumTokens + 1);
		}
		else
		{
			TempNode->EscapeProb = (double)(TempNode->NumTypes)/(double)(TempNode->NumTokens);
		}
		// POST((t_object* x), "Escape: %f", TempNode->EscapeProb); 
		return eRETVAL_SUCCESS;
	}
	else
		return eRETVAL_FAILURE;
}


unsigned int tTracker :: SubtractNumTypes(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		if (TempNode->NumTypes > 0)
		{
			TempNode->NumTypes--;
		}
		return eRETVAL_SUCCESS;
	}
	else
		return eRETVAL_FAILURE;
}

unsigned int tTracker :: SubtractNumTypes2(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		if (TempNode->NumTypes2 > 0)
		{
			TempNode->NumTypes2--;
		}
		return eRETVAL_SUCCESS;
	}
	else
		return eRETVAL_FAILURE;
}

unsigned int tTracker :: UpdateOrder(void)
{
	Order++;
	return eRETVAL_SUCCESS;
}

unsigned int tTracker :: SubtractOrder(void)
{
	if (Order > 0) {
		Order--;
	}
	return eRETVAL_SUCCESS;
}

unsigned int tTracker :: ResetOrder(void)
{
	Order = 0;
	return eRETVAL_SUCCESS;
}

unsigned int tTracker :: SetOrder(unsigned int order)
{
	Order = order;
	return eRETVAL_SUCCESS;
}

void tTracker:: SetWeight(unsigned int Order, double val)
{

	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		TempNode->Weight=val;
	}

}

double tTracker:: GetWeight(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		return TempNode->Weight;
	}
	else 
	{
		return eRETVAL_FAILURE;
	}
}

unsigned int tTracker :: SetAllWeights(unsigned int length)
{
	tTrackerNode *TempNode = FirstNode;
	unsigned int i = 0;
	
	for (TempNode=FirstNode; TempNode!=NULL && i<length+1; TempNode = TempNode->Next)
	{
		//TempNode->Weight = pow(0.5, (length-i)+1);
		TempNode->Weight = (1.0)/((length-i)+1.0);
		i++;
	}
	return eRETVAL_SUCCESS;
}

// Function to set the weights for the orders using certain parameters.
// The weights for order n are given by:
//
// a*( (1-c/a) * pow((n/MAX_ORDER), x))+c
//
// a = multiplier
// c = offset (c > 0 for priors to be considered)
// x = exponent (0 < x < infinity)
//
// For example: These values are safe
// a = 1;
// c = 0.1;
// x = 0.5

unsigned int tTracker :: SetAllWeightsFunction(unsigned int length)
{
	tTrackerNode *TempNode = FirstNode;
	unsigned int i = 0;
	double MaxOrder = MAX_ORDER;
	
// If all the variables are un-initialized then set them to defaults.
	if ((Multiplier == 0) && (Exponent == 0) && (Offset == 0))
	{
		Multiplier = 1;
		Exponent = 0.5;
		Offset = 0.1;
	}
	
	for (TempNode=FirstNode; TempNode!=NULL && i<length+1; TempNode = TempNode->Next)
	{
		if ((i == 0) && (Exponent < 0))
		{
			// In this case the result will be infinity. I'll use a 0.5 instead otherwise will get too much weight.
			TempNode->Weight = Multiplier*((1-(Offset/Multiplier))*pow((0.5/MaxOrder),Exponent)) + Offset;	
		}
		else
		{
			TempNode->Weight = Multiplier*((1-(Offset/Multiplier))*pow((i/MaxOrder),Exponent)) + Offset;
		}
		i++;
	}
	return eRETVAL_SUCCESS;
}


unsigned int tTracker :: SetAllWeightsGaussian(unsigned int length)
{
	tTrackerNode *TempNode = FirstNode;
	unsigned int i = 0;
	double MaxOrder = MAX_ORDER;
	
	// If all the variables are un-initialized then set them to defaults.
	if ((Mu == 0) && (Sigma == 0))
	{
		Mu=MaxOrder;
		Sigma = 2;

	}
	
	for (TempNode=FirstNode; TempNode!=NULL && i<length+1; TempNode = TempNode->Next)
	{
		if(Sigma==0)
		{
			TempNode->Weight = 1;

		}
		else
		{
		
			TempNode->Weight = exp(-pow((i-Mu),2)/(2*pow(Sigma,2)));
		}
		
		/*if ((i == 0) && (Exponent < 0))
		{
			// In this case the result will be infinity. I'll use a 0.5 instead otherwise will get too much weight.
			TempNode->Weight = Multiplier*((1-(Offset/Multiplier))*pow((0.5/MaxOrder),Exponent)) + Offset;	
		}
		else
		{
			TempNode->Weight = Multiplier*((1-(Offset/Multiplier))*pow((i/MaxOrder),Exponent)) + Offset;
		}*/
		i++;
	}
	return eRETVAL_SUCCESS;
}







void tTracker :: DeleteAll(void)
{
	while(Length>0)
	{
		FirstNode = LastNode->Prev;
		delete LastNode;
		if (FirstNode != NULL)
		{
			FirstNode->Next = NULL;
		}
		LastNode = FirstNode;
		Length--;
	}
}

double tTracker :: GetDiscountRatio(unsigned int Order)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
		return TempNode->DiscountRatio;
	else
		return NULL;
}

unsigned int tTracker :: SetDiscountRatio(unsigned int Order, void * x)
{
	tTrackerNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Order && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{ 
		if ((TempNode->NumTypes == 0) && (TempNode->NumTypes2 == 0))
		{
			TempNode->DiscountRatio = 1;
			//POST((t_object* x), "Discount ratio set to 1 since N1 and N2 are 0"); 
		}
		else 
		{
			TempNode->DiscountRatio = (double)(TempNode->NumTypes)/(double)(TempNode->NumTypes + 2*TempNode->NumTypes2);
		}

		//POST((t_object* x), "N1 = %d; N2 = %d; D=%f", TempNode->NumTypes, TempNode->NumTypes2, TempNode->DiscountRatio);
		return eRETVAL_SUCCESS;
	}
	else
		return eRETVAL_FAILURE;
}


