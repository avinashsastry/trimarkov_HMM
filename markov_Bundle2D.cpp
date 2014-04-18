/*
 *  markov_Bundle2D.cpp
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/7/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
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

using namespace std;

//	Constructor
tBundle2D :: tBundle2D()
{
	this->Top = NULL;
	this->FirstNode = NULL;
	this->LastNode = NULL;
	this->Length = 0;
	this->TotalCount = 0;
	this->TotalSize = 0;
};

//	Constructor
tBundle2D :: tBundle2D(t2DNode * Top = NULL, t2DNode *FirstNode = NULL, t2DNode *LastNode = NULL, unsigned short int Length = 0, unsigned long int TotalCount = 0, unsigned long int TotalSize = 0)
{
	this->Top = Top;
	this->FirstNode = FirstNode;
	this->LastNode = LastNode;
	this->Length = Length;
	this->TotalCount = TotalCount;
	this->TotalSize = TotalSize;
};

//	Destructor
tBundle2D :: ~tBundle2D()
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
c1DList * tBundle2D :: GetVector(unsigned int Index)
{
	t2DNode *TempNode = FirstNode;
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
float tBundle2D :: GetValue(unsigned int VectorIndex, unsigned int DimIndex)
{
	t2DNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<VectorIndex && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Right;
	}
	if(TempNode!=NULL)
		return TempNode->Vector.GetValue(DimIndex);
	else
		return eRETVAL_FAILURE;
}

float tBundle2D :: GetProb(unsigned int Index)
{
	t2DNode *TempNode = FirstNode;
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
float tBundle2D :: GetEntropy(unsigned int Index) 
{	
	t2DNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Index && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Right;
	}
	if(TempNode!=NULL)
		return -((log(TempNode->Prob))/(log(2)));
	else
		return NULL;
}

//	GetLength - return the length (size) of the bundle
unsigned int tBundle2D :: GetLength(void)
{
	return Length;
}

//	Append value at the right end
int	tBundle2D :: Append(c1DList *Vector)
{
	t2DNode	*NewNode = new t2DNode(LastNode, NULL, this, NULL, Vector, 0, 0);
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
int	tBundle2D :: Insert(c1DList *Vector)
{
	if(FirstNode == NULL)
	{
		t2DNode	*NewNode = new t2DNode(NULL, NULL, this, NULL, Vector, 0, 0);
		FirstNode = NewNode;
		LastNode = NewNode;
		Length++;
		TotalSize++;
	}
	else
	{
		unsigned int Found = 0;
		t2DNode *TempNode1 = NULL;;
		t2DNode *TempNode2 = FirstNode;
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
			t2DNode	*NewNode = new t2DNode(TempNode1, TempNode2, this, NULL, Vector, 0, 0);
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
t2DNode *tBundle2D :: Find(c1DList *Vector)
{
	t2DNode * TempNode = FirstNode;
	while(TempNode!=NULL)
	{
		if(TempNode->Vector == Vector)
		{
			break;
		}
		TempNode = TempNode->Right;
	}
	return TempNode;
}

void tBundle2D :: DeleteBundle(void)
{
	t2DNode* TempNode = NULL;
	t2DNode* TempNode2 = NULL;
	
	// Check if the bundle is empty
	if (FirstNode == NULL)
	{
		// If yes, then you're fine, no need to do anything
	}
	else
	{
		// If no, then delete each t2dnode - DeleteNode()
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

unsigned int tBundle2D :: MergeOpenClosed(tBundle2D* SplitPDFReducedStrokePIS1, cQaidaControl* QaidaController)
{
	float Stroke = 0;
	float OpenMultiplier = 0;
	float ClosedMultiplier = 0;
	float RestMultiplier = 0;
	c1DList Vector;
	t2DNode* TempNode = NULL;
	t2DNode* TempNode2 = NULL;
	
	if (SplitPDFReducedStrokePIS1 != NULL)
	{
		// Get the values for each of the multipliers
		
		//First for open
		Vector.Append(0);
		TempNode2 = SplitPDFReducedStrokePIS1->Find(&Vector);
		if (TempNode2 != NULL)
		{
			OpenMultiplier = TempNode2->Prob;
		}
		Vector.DeleteAll();
		
		// Closed
		Vector.Append(1);
		TempNode2 = SplitPDFReducedStrokePIS1->Find(&Vector);
		if (TempNode2 != NULL)
		{
			ClosedMultiplier = TempNode2->Prob;
		}
		Vector.DeleteAll();
		
		// Rest
		Vector.Append(42);
		TempNode2 = SplitPDFReducedStrokePIS1->Find(&Vector);
		if (TempNode2 != NULL)
		{
			RestMultiplier = TempNode2->Prob;
		}
		Vector.DeleteAll();
						 
		if ((FirstNode != NULL) && (SplitPDFReducedStrokePIS1->FirstNode != NULL))
		{
			for(TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
			{
				// Check if the stroke in the node is open or closed
				Stroke = TempNode->Vector.GetValue(0);
				if (QaidaController->StrokeMatrix[(unsigned int)Stroke-1][3] == 0)
				{
					// Multiply the probabilities
					TempNode->Prob = TempNode->Prob*OpenMultiplier;
				}
				else if (QaidaController->StrokeMatrix[(unsigned int)Stroke-1][3] == 1)
				{
					TempNode->Prob = TempNode->Prob*ClosedMultiplier;
				}
				else if (QaidaController->StrokeMatrix[(unsigned int)Stroke-1][3] == 42)
				{
					TempNode->Prob = TempNode->Prob*RestMultiplier;
				}
			}
		}
		return eRETVAL_SUCCESS;
	}
	else
	{
		return eRETVAL_FAILURE;
	}
}

//***************************************
#pragma mark -
#pragma mark Learning functions
//***************************************

//	Iteratively grow the tree (bundle) for all the nodes in the stream starting from the input StreamHead
//	Calls Grow()
int	tBundle2D :: GrowAll(c1DVectorList * InputStream, tTracker * Tracker, void *x)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	unsigned int depth = 0;
	
	if(InputStream != NULL)
	{
		c1DVectorNode *CurrNode = InputStream->LastNode;
		
		// Append tracker node - Right now it appends one node at a time.
		// It has to make sure that the tracker is only as long as the max order + 1.
		// so for a model of order 10, the tracker should have 11 nodes.
		if (Tracker->Length <= MAX_ORDER)
		{
			Tracker->Append();
		}
		
		while(CurrNode != NULL)
		{
			//Reset Order - Everytime grow() is called from here, it means a new series is starting
			Tracker->ResetOrder();
			
			//Update NumTokens - the variable 'depth' keeps track of what order we're dealing with
			//Since every call to grow() denotes a new series, the NumTokens for that order must be 
			//updated accordingly
			Tracker->UpdateNumTokens(depth);
			// POST((t_object *x), "	GrowAll: Update NumTokens(%d): %d", depth, Tracker->GetNumTokens(depth));
			
			RetVal |= Grow(CurrNode, Tracker, x);
			CurrNode = CurrNode->Prev;
			
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
int	tBundle2D :: Grow(c1DVectorNode * StreamHead, tTracker * Tracker, void *x)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	t2DNode * TempNode;
	
	//	If NULL pointer passed in, return failure
	if(StreamHead == NULL)
	{
		RetVal = eRETVAL_FAILURE;
	}
	else
	{
		//	If last node in the stream, update & return
		if(StreamHead->Next == NULL)
		{
			RetVal = Update(&StreamHead->Vector, Tracker, x);
		}
		//	Else, find the corresponding node & grow a branch below it
		else
		{
			TempNode = Find(&StreamHead->Vector);
			//	If no node has the value, by chance, introduce the value
			//	This case doesn't happen usually, as the node with the value should ideally have been created beforehand
			if(TempNode == NULL)
			{
				Update(&StreamHead->Vector, Tracker, x);
				TempNode = Find(&StreamHead->Vector);
			}
			// This deals with cases like 1-1-2 or longer
			// Existing nodes like the second '1' must also be counted
			// TODO: Is this required?
			else 
			{
				// Update(&StreamHead->Vector, Tracker, x);
			}
			
			//	If the node doesn't have a branch below it, create the branch
			if(TempNode->Bottom == NULL)
			{
				tBundle2D * NewBundle = new tBundle2D(TempNode, NULL, NULL, 0, 0, 0);
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
			TempNode->Bottom->Grow(StreamHead->Next, Tracker, x);
			TotalSize += TempNode->Bottom->TotalSize;
		}
	}			
	return	RetVal;
}

//	Update the count & probability of the input value after inserting/finding it at its sorted position
//	Called by Grow() routine
int	tBundle2D :: Update(c1DList *Vector, tTracker * Tracker, void *x)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	unsigned int Order = 0;
	
	if (Tracker != NULL)
		Order = Tracker->GetOrder();
	
	if(FirstNode == NULL)
	{
		//	Create the new (first) node
		// The first node will have a probability of 0.5 allowing for an escape of 0.5
		t2DNode	*NewNode = new t2DNode(NULL, NULL, this, NULL, Vector, 1, 0.5);	//	TODO: ...
		FirstNode = NewNode;
		LastNode = NewNode;
		Length++;
		TotalCount++;
		TotalSize++;
		
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
		t2DNode *TempNode1 = NULL;;
		t2DNode *TempNode2 = FirstNode;
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
			t2DNode	*NewNode = new t2DNode(TempNode1, TempNode2, this, NULL, Vector, 1, 0); // TODO: ...
			if(TempNode1 != NULL)	TempNode1->Right = NewNode;
			else					FirstNode = NewNode;
			if(TempNode2 != NULL)	TempNode2->Left = NewNode;
			else					LastNode = NewNode;
			Length++;
			TotalCount++;
			TotalSize++;
			
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
int	tBundle2D :: PredictBest(c1DVectorList * InputStream, c1DList *Outcome, void *x = NULL)
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

int	tBundle2D :: PredictBest(c1DVectorList *InputStream, tBundle2D *Outcome, void *x = NULL)
{
	int RetVal = eRETVAL_UNDEFINED;
	int depth = InputStream->GetLength();
	
	if(InputStream != NULL)
	{
		c1DVectorNode *CurrNode = InputStream->FirstNode;
		while(CurrNode != NULL)
		{
			// POST((t_object *)x, "Bang! PredictBest: %d",CurrNode->Value);
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
		POST((t_object *)x, (char *)"Bang! Failure 3");
	}
	return RetVal;
}

//	Predict folowing the values in the stream, pointed to by StreamHead;
//	Recursively finds the branches corresponding to the successive values in the stream,
//	& rolls the dice at the last node.
//	Calls Roll() routine
//	Recursively calls itself (Predict())
//	Equivalent to Grow()
int	tBundle2D :: Predict(c1DVectorNode *StreamHead, c1DList *Outcome, void *x = NULL)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	t2DNode * TempNode;
	
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

int	tBundle2D :: Predict(c1DVectorNode *StreamHead, tBundle2D *Outcome, void *x = NULL)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	t2DNode * TempNode;
	
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
		//	TODO: If no node has the value, by chance, ...
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
				//  OR
				//  Return the PDF at that bundle!
				if(StreamHead->Next == NULL)
				{
					RetVal = TempNode->Bottom->ReturnPDF(Outcome, x);
					//RetVal = TempNode->Bottom->Roll(Outcome, x);
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

//	Determine the prediction outcome based on the probability distribution under the input value after finding it at its sorted position
//	'Roll the dice'
//	Called by Predict() routine
//	Equivalent to Update()
int	tBundle2D :: Roll(c1DList *Outcome, void *x = NULL)
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
		t2DNode *TempNode = FirstNode;
		for(int i=0; TempNode!=NULL; i++, TempNode=TempNode->Right)
		{
			Vectors[i] = &TempNode->Vector;
			Probabilities[i] = TempNode->Prob;
		}
	
		/*// Sort the Probabilities array in ascending order, & tab the Locations
		 for(int i=0; i<Length-1; i++)
		 {
		 double minProb = Probabilities[i];
		 unsigned int minLoc = i;
		 for(int j=i+1; j<Length; j++)
		 {
		 if(Probabilities[j]<Probabilities[minLoc])
		 {
		 minLoc = j;
		 minProb = Probabilities[j];
		 }
		 }
		 Probabilities[minLoc] = Probabilities[i];
		 Probabilities[i] = minProb;
		 Locations[i] = minLoc;
		 }*/
					
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
int tBundle2D :: PredictSmoothBest( c1DVectorList* SearchStream, tBundle2D *Accumulator, tTracker* Tracker, void*x) 
{	
	int RetVal = eRETVAL_UNDEFINED;
	int depth = 0;
	
	tBundle2D *Outcome = new tBundle2D(NULL, NULL, NULL, 0, 0, 0);
	t2DNode * TempNode = NULL;
	t2DNode * TempNode2 = NULL;
	
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
				Outcome = new tBundle2D(NULL, NULL, NULL, 0, 0, 0);
				
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
int tBundle2D :: PredictSmooth( c1DVectorNode * StreamHead, tBundle2D *Outcome, void *x) {
	
	unsigned int RetVal = eRETVAL_UNDEFINED;
	t2DNode * TempNode;
	
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

//	needed to get the pdf values ...similar to what rolll does, but no decision made yet.  
//	Called by PredictSmooth() routine
//	Equivalent to ...
int	tBundle2D :: ReturnPDF(tBundle2D *Outcome, void *x = NULL)
{
	unsigned int RetVal = eRETVAL_UNDEFINED;
	t2DNode *TempNode = NULL;
	t2DNode *TempNode2 = NULL;
	
	if(FirstNode == NULL)
	{
		RetVal = eRETVAL_FAILURE;
		POST((t_object *)x, (char *)"Bang! Failure 6");
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
				//since you just added it, you know it's the last node
				TempNode2 = Outcome->LastNode;
				TempNode2->Prob	= TempNode->Prob;
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
unsigned int tBundle2D :: PredictKneserNey(c1DVectorList* SearchStream, tBundle2D *Accumulator, tTracker* Tracker, void*x)
{
	int RetVal = eRETVAL_UNDEFINED;
	double Prob = 0;
	
	tBundle2D *Priors = new tBundle2D(NULL, NULL, NULL, 0, 0, 0);
	t2DNode * TempNode = NULL;
	t2DNode * TempNode2 = NULL;
	
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

double tBundle2D :: KneserNey(t2DNode* PredictedSymbol, c1DVectorNode* StreamHead, tTracker* Tracker, void* x)
{
	double Prob = 0;
	double FirstTerm = 0;
	double SecondTerm = 0;
	unsigned int Order = 0;
	unsigned int NodeCount = 0;
	unsigned int ChildrenCount = 0;
	unsigned int SumCount = 0;
	double Discount = 0;
	t2DNode* TempNode = NULL;
	tBundle2D* TempBundle = NULL;
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
				
				/*
				if (NodeCount - Discount > 0)
				{
					FirstTerm = (double)(NodeCount-Discount)/(double)SumCount;
				}
				else if (Discount == 0)
				{
					// This means nodecount is 0 and discount is 0
					FirstTerm = Tracker->GetEscapeProb(Order);
				}
				else
				{
					FirstTerm = 0;
				}
				 */
				
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

double tBundle2D :: CrossValidate(c1DList* Vector, tTracker* Tracker, void* x)
{
	t2DNode* TempNode = NULL;
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

int tBundle2D :: Normalize(void)
{
	double SumProb = NULL;
	t2DNode* TempNode2 = FirstNode;
	
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

t2DNode* tBundle2D :: GetMaxProb(void)
{
	t2DNode* TempNode = NULL;
	t2DNode* MaxNode = NULL;
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

double tBundle2D :: GetSumProb(void)
{
	t2DNode* TempNode = NULL;
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

void tBundle2D :: MergePDF(tBundle2D* SecondBundle, double ShortTermWeight, double LongTermWeight) 
{
	t2DNode* TempNode = NULL;
	t2DNode* TempNode2 = NULL;
	
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
double tBundle2D :: TotalEntropy(tTracker* Tracker)
{
	double SumEntropy = 0;
	t2DNode* TempNode = NULL;
	
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
unsigned int tBundle2D :: SplitPDF(tBundle2D* PDFVectorOne, tBundle2D* PDFVectorTwo)
{
	// For starters I'm just splitting the vectors and copying the PDFs so it's nothing
	// special. But I may want to consider adding all the duration probabilities and 
	// normalizing. Strokes can probably stay the same
	
	unsigned int VectorLength = 0;
	float VectorOneValue = 0;
	float VectorTwoValue = 0;
	c1DList VectorOne(NULL,NULL,0);
	c1DList VectorTwo(NULL,NULL,0);
	t2DNode* TempNode = NULL;
	t2DNode* TempNode2 = NULL;
	
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
double tBundle2D :: RelativeEntropy(tTracker* Tracker)
{
	double Entropy = 0;
	double MinProb = 0;
	double MaxEntropy = 0;
	double RelativeEntropy = 0;
	t2DNode* TempNode = FirstNode;
	
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
unsigned int tBundle2D :: ArithmeticCombination(tBundle2D* SecondBundle, tTracker* TrackerOne, tTracker* TrackerTwo, bool IsContinued)
{
	t2DNode* TempNode = NULL;
	t2DNode* TempNode2 = NULL;
	
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
unsigned int tBundle2D :: GeometricCombination(tBundle2D* SecondBundle, tTracker* TrackerOne, tTracker* TrackerTwo, bool IsContinued)
{
	t2DNode* TempNode = NULL;
	t2DNode* TempNode2 = NULL;
	
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

void tBundle2D :: MapToNotes(unsigned int StrokeMatrix[42][4], unsigned int ReductionSet)
{
	t2DNode* TempNode = NULL;
	t2DNode* TempNode2 = NULL;
	c1DList Vector(NULL,NULL,0);
	float Stroke = 0;
	float RealStroke = 0;
	
	//Check if first node == NULL
	if (FirstNode != NULL)
	{
		// If not then go through the entire PDF
		for (TempNode = FirstNode; TempNode != NULL; TempNode = TempNode->Right)
		{
			//For each node get the value of the vector
			Stroke = TempNode->Vector.GetValue(0);
			
			// Search for this value in StrokeMatrix[i][1]
			for (int i = 0; i < MAX_STROKES; i++)
			{
				// If it finds the value, then get the normal stroke name
				if (StrokeMatrix[i][ReductionSet] == Stroke)
				{
					RealStroke = StrokeMatrix[i][0];
					
					// Create a vector out of this stroke matrix
					Vector.Append(RealStroke);
					
					// Search for the vector in the existing PDF
					TempNode2 = Find(&Vector);
					
					// If you find the node, don't do anything
					// else create a new node and assign this probability
					
					if (TempNode2 == NULL)
					{
						Append(&Vector);
						TempNode2 = LastNode;
						TempNode2->Prob = TempNode->Prob;
					}
					else
					{
						// This node exists so I'm not doing to touch it
					}
					Vector.DeleteAll();
				}
			}
		}
		//Normalize();
		return;
	}
	else
	{
		return;
	}
}

// Function to search the bundle for a node and compare it with the most likely node
// Made for cQaidaController so that it can get the distance
double tBundle2D :: DistanceToMostLikelyNode(c1DList* PredictedNode)
{
	t2DNode* MostLikelyNode = NULL;
	t2DNode* CurrentPrediction = NULL;
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
double tBundle2D :: EntropyDistance(double Probability1, double Probability2)
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
void tBundle2D :: PostBundle(void *pCountOutlet, void *pProbOutlet)
{
	t2DNode *TempNode = FirstNode;
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

//	Output the whole trie as a sequence of lists corresponding to bundles
//	For debugging only; may impair performance
//	Recursively calls itself
void tBundle2D :: PostTrie(void *pCountOutlet, void *pProbOutlet)
{
	t2DNode *TempNode = FirstNode;
	
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
void tBundle2D :: PostTrieXML(ofstream &OutputFile, int InitialOrder)
{
	// Declarations
	//int Name = 0;
	int Count = 0;
	char Year[4];
	char Bol[3];
	char CountChar[6];
	t2DNode *TempNode = FirstNode;
	
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