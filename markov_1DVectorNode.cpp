/*
 *  markov_1DVectorNode.cpp
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/14/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

#include "markov_defines.h"
#include "markov_classes.h"
#include "markov_1DNode.h"
#include "markov_1DList.h"
#include "markov_1DVectorNode.h"

//	Constructor
c1DVectorNode :: c1DVectorNode(c1DList *Vector = NULL, c1DVectorNode *Prev = NULL, c1DVectorNode *Next = NULL)
{
	this->Vector = Vector; // overloaded to Assign()
	this->Prev = Prev;
	this->Next = Next;
};

//	Destructor
c1DVectorNode :: ~c1DVectorNode()
{
	Vector.DeleteAll(); // TODO: Any pass by value vs. pointer issues here?
	Vector = NULL;
	Prev = NULL;
	Next = NULL;
};
