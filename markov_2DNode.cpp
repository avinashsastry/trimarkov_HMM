/*
 *  markov_2DNode.cpp
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/7/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

#include "markov_classes.h"
#include "markov_1DNode.h"
#include "markov_1DList.h"
#include "markov_Bundle2D.h"
#include "markov_2DNode.h"

//	Constructor
t2DNode::t2DNode(t2DNode *Left = NULL, t2DNode *Right = NULL, tBundle2D *Bundle = NULL, tBundle2D *Bottom = NULL, c1DList *Vector = NULL, unsigned long int Count = 0, double Prob = 0)
{
	this->Left		=	Left;
	this->Right		=	Right;
	this->Bundle	=	Bundle;
	this->Bottom	=	Bottom;
	this->Vector	=	Vector; // Overloaded '='
	this->Count		=	Count;
	this->Prob		=	Prob;
	//	this->Bottom	=	new tBundle2D;
	/*	if(Bottom!=NULL)
	 {
	 this->Bottom->FirstNode	= Bottom->FirstNode;
	 this->Bottom->LastNode	= Bottom->LastNode;
	 this->Bottom->Length	= Bottom->Length;
	 }*/
};

//	Destructor
t2DNode::~t2DNode()
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
