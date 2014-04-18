/*
 *  markov_1DVectorList.cpp
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/14/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

#include "ext.h"
#include "ext_obex.h"

#include "markov_defines.h"
#include "markov_classes.h"
#include "markov_enums.h"
#include "markov_1DNode.h"
#include "markov_1DList.h"
#include "markov_1DVectorNode.h"
#include "markov_1DVectorList.h"

//	Constructor
c1DVectorList :: c1DVectorList()
{
	this->Dim = 0;
	this->Length = 0;
	this->FirstNode = NULL;
	this->LastNode = NULL;
}

//	Constructor
c1DVectorList :: c1DVectorList(unsigned int Dim = 0, unsigned int Length = 0, c1DVectorNode *FirstNode = NULL, c1DVectorNode *LastNode = NULL)
{
	this->Dim = Dim;
	this->Length = Length;
	this->FirstNode = FirstNode;
	this->LastNode = LastNode;
}

//	Destructor
c1DVectorList :: ~c1DVectorList()
{
	// DeleteAll(); // Can't call DeleteAll() - as we don't expect all pointed data to be lost when this destructor is called at the end of intermediate operations
	// TODO: Or do we?
	Dim = 0;
	Length = 0;
	FirstNode = NULL;
	LastNode = NULL;
}

//	GetVector - return the list at given index
c1DList * c1DVectorList :: GetVector(unsigned int Index)
{
	c1DVectorNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Index && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
		return &TempNode->Vector;
	else
		return NULL;
}


//	GetValue - return the value at given DimIndex of the vector at the given VectorIndex
float c1DVectorList :: GetValue(unsigned int VectorIndex, unsigned int DimIndex)
{
	c1DVectorNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<VectorIndex && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
		return TempNode->Vector.GetValue(DimIndex);
	else
		return eRETVAL_FAILURE;
}

//	GetLength - return the length of the stream
unsigned int c1DVectorList :: GetLength(void)
{
	return Length;
}

//	SetLength - set the length of the list
void c1DVectorList :: SetLength(unsigned int Length)
{
	this->Length = Length;
}

//	GetDim - return the number of dimensions of the vectors of the list
unsigned int c1DVectorList :: GetDim(void)
{
	return Dim;
}

//	SetDim - set the number of dimensions fo the vectors of the list
void c1DVectorList :: SetDim(unsigned int Dim)
{
	this->Dim = Dim;
}

//	Append - append a node at the end of the stream
int	c1DVectorList :: Append(c1DList *Vector)
{
	c1DVectorNode *NewNode = new c1DVectorNode(Vector, LastNode, NULL);
	if(FirstNode == NULL)
	{
		FirstNode = NewNode;
		LastNode = NewNode;
	}
	else
	{
		LastNode->Next = NewNode;
		LastNode = NewNode;
	}
	Length++;
	return eRETVAL_SUCCESS;
}

//	Delete - delete a node
int c1DVectorList :: Delete(unsigned int Index)
{
	if(Index>=Length)
		return eRETVAL_FAILURE;
	c1DVectorNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Index && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		if(TempNode->Prev!=NULL)
			TempNode->Prev->Next = TempNode->Next;
		else
			FirstNode=TempNode->Next;
		if(TempNode->Next!=NULL)
			TempNode->Next->Prev = TempNode->Prev;
		else
			LastNode=TempNode->Prev;
		delete TempNode;
		Length--;
		return eRETVAL_SUCCESS;
	}
	else
	{
		//throw("Invalid index");
		return eRETVAL_FAILURE;
	}
}

//	DeleteAll - delete all nodes
int c1DVectorList :: DeleteAll(void)
{
	int retval = eRETVAL_UNDEFINED;
	if(FirstNode!=NULL)
	{
		FirstNode = LastNode->Prev;
		while(FirstNode!=NULL)
		{
			delete LastNode;
			Length--;
			FirstNode->Next = NULL;
			LastNode = FirstNode;
			FirstNode = LastNode->Prev;
		}
		delete LastNode;
		Length--;
		LastNode = FirstNode;
	}
	retval = eRETVAL_SUCCESS;
	return retval;
}

//	AppendAll - append all nodes from another c1DList; in effect, combine them
int	c1DVectorList :: AppendAll(c1DVectorList *OtherList = NULL)
{
	int retval = eRETVAL_UNDEFINED;
	if(OtherList==NULL)
		retval = eRETVAL_NULL_INPUT;
	else
	{
		c1DVectorNode *TempNode = OtherList->FirstNode;
		for(int i=OtherList->Length; i>0; i--)
		{
			Append(&TempNode->Vector);
			TempNode = TempNode->Next;
		}
		retval = eRETVAL_SUCCESS;
	}
	return retval;
}

//	Assign - copy/replace list from another c1DList
int	c1DVectorList :: Assign(c1DVectorList *OtherList)
{
	int retval = DeleteAll();
	retval |= AppendAll(OtherList);
	return retval;
}

// LeftShift - shift all nodes to the left so that the first node gets pushed out
int c1DVectorList :: LeftShift(void)
{
	c1DVectorNode* TempNode = NULL;
	
	if (FirstNode == NULL)
	{
		// List has no nodes, so don't do anything
		return eRETVAL_SUCCESS;
	}
	else if (FirstNode == LastNode)
	{
		// If List has only one node, then delete it
		DeleteAll();
		return	eRETVAL_SUCCESS;
	}
	else 
	{
		// If list has multiple nodes, remove the link to the first node
		// and make the second node the new first node
		TempNode = FirstNode->Next;
		TempNode->Prev = NULL;
		delete FirstNode;
		FirstNode = TempNode;
		Length--;
		return eRETVAL_SUCCESS;
	}
}

//	Overloaded '=' for another pointer
c1DVectorList *c1DVectorList :: operator = (c1DVectorList *OtherList)
{
	Assign(OtherList);
	return this;
}

//	Overloaded '=' for another object
c1DVectorList *c1DVectorList :: operator = (c1DVectorList OtherList)
{
	Assign(&OtherList);
	return this;
}

unsigned int c1DVectorList :: AddFeatures(float* FeatureVector, c1DList* Vector)
{
	// Create a new vector node
	c1DVectorNode* NewNode = new c1DVectorNode(Vector, LastNode, NULL);
	
	for(unsigned int i = 0; i < NUM_FEATURES; i++)
	{
		NewNode->FeatureVector[i] = *(FeatureVector+i);
	}
	
	if(FirstNode == NULL)
	{
		FirstNode = NewNode;
		LastNode = NewNode;
	}
	else
	{
		LastNode->Next = NewNode;
		LastNode = NewNode;
	}
	Length++;
	return eRETVAL_SUCCESS;
}

//	Output the list - one vector at a time
//	For debugging only; may impair performance
void c1DVectorList :: Post(void *pOutlet)
{
	c1DVectorNode *TempVectorNode = FirstNode;
	for(int i=0; (unsigned int)i<Length; i++)
	{
		t_atom	*av = new t_atom[Dim];
		t1DNode *TempNode = TempVectorNode->Vector.FirstNode;
		for(int j=0; (unsigned int)j<Dim; j++)
		{
			atom_setfloat(av+j, TempNode->Value);
			TempNode = TempNode->Next;
		}
		outlet_list(pOutlet, 0L, Dim, av);
		delete av;
		TempVectorNode = TempVectorNode->Next;
	}
}