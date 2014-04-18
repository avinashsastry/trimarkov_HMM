/*
 *  markov_1DNode.cpp
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/7/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

#include "markov_1DNode.h"

//	Constructor
t1DNode :: t1DNode(float Value = 0, t1DNode *Prev = NULL, t1DNode *Next = NULL)
{
	this->Value = Value;
	this->Prev = Prev;
	this->Next = Next;
};

//	Destructor
t1DNode :: ~t1DNode()
{
	Value = 0;
	Prev = NULL;
	Next = NULL;
};
