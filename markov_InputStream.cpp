/*
 *  markov_InputStream.cpp
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/7/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

#include "markov_classes.h"
#include "markov_1DNode.h"
#include "markov_1DList.h"
#include "markov_InputStream.h"
/*
//	Constructor
tInputStream :: tInputStream (t1DNode *FirstNode = NULL, t1DNode *LastNode = NULL, unsigned int Length = 0) : c1DList :: c1DList(t1DNode *FirstNode = NULL, t1DNode *LastNode = NULL, unsigned int Length = 0)
//tInputStream :: tInputStream () : c1DList (),
//FirstNode(NULL), LastNode(NULL), Length(0)
{
	c1DList(FirstNode, LastNode, Length);
	this->FirstNode = FirstNode;
	this->LastNode = LastNode;
	this->Length = Length;
}

//	Destructor
tInputStream :: ~tInputStream()
{
	// ~c1DList();
	while(Length>0)
	{
		FirstNode = LastNode->Prev;
		delete LastNode;
		FirstNode->Next = NULL;
		LastNode = FirstNode;
		Length--;
	}
}
*/