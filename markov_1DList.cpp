/*
 *  markov_1DList.cpp
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/13/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

#include "ext.h"

#include "markov_defines.h"
#include "markov_classes.h"
#include "markov_enums.h"
#include "markov_1DNode.h"
#include "markov_1DList.h"

//	Constructor - TODO: Why is this required?
c1DList :: c1DList()
{
	this->FirstNode = NULL;
	this->LastNode = NULL;
	this->Length = 0;
}

//	Constructor: TODO: Reqd? Some issue found in cFeatures::WhereInBar2MeterCount(...) with value or ref argument of c1DList - solved with ptr argument - but not with the below constructor; what?? why?? how???
c1DList :: c1DList(const c1DList &OtherList)
{
	this->FirstNode = OtherList.FirstNode;
	this->LastNode = OtherList.LastNode;
	this->Length = OtherList.Length;
}

//	Constructor
c1DList :: c1DList(t1DNode *FirstNode = NULL, t1DNode *LastNode = NULL, unsigned int Length = 0)
{
	this->FirstNode = FirstNode;
	this->LastNode = LastNode;
	this->Length = Length;
}

//	Destructor
c1DList :: ~c1DList()
{
	FirstNode = NULL;
	LastNode = NULL;
	Length = 0;
}

//	GetValue - return the value at given index
float c1DList :: GetValue(unsigned int Index)
{
	t1DNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Index && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
		return TempNode->Value;
	else
		return 0; // TODO: Better way of handling invalid index
		//throw("Invalid index");
}

//	SetValue - set the value at given index
int c1DList :: SetValue(unsigned int Index, float Value)
{
	t1DNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Index && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
	{
		TempNode->Value = Value;
		return eRETVAL_SUCCESS;
	}
	else
		return eRETVAL_FAILURE; // TODO: Better way of handling invalid index
		//throw("Invalid index");
}

//	GetLength - return the length of the stream
unsigned int c1DList :: GetLength(void)
{
	return Length;
}

//	Append - append a node at the end of the stream
int	c1DList :: Append(float Value)
{
	t1DNode	*NewNode = new t1DNode(Value, LastNode, NULL);
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
int c1DList :: Delete(unsigned int Index)
{
	t1DNode *TempNode = FirstNode;
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
		return eRETVAL_FAILURE; // TODO: Better way of handling invalid index
		//throw("Invalid index");
}

//	DeleteAll - delete all nodes
int c1DList :: DeleteAll(void)
{
	int retval = eRETVAL_UNDEFINED;
	if(FirstNode != NULL)
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
int	c1DList :: AppendAll(c1DList *OtherList)
{
	int retval = eRETVAL_UNDEFINED;
	if(OtherList==NULL)
		retval = eRETVAL_NULL_INPUT;
	else
	{
		t1DNode *TempNode = OtherList->FirstNode;
		for(int i=OtherList->Length; i>0; i--)
		{
			Append(TempNode->Value);
			TempNode = TempNode->Next;
		}
		retval = eRETVAL_SUCCESS;
	}
	return retval;
}

//	Assign - copy/replace list from another c1DList
int	c1DList :: Assign(c1DList *OtherList)
{
	int retval = DeleteAll();
	retval |= AppendAll(OtherList);
	return retval;
}

//	Equals - check equality with other list
bool c1DList :: Equals(c1DList *OtherList)
{
	if (OtherList != NULL)
	{
		if(Length != OtherList->Length)
			return FALSE;
		else
		{
			t1DNode *TempNode1 = FirstNode;
			t1DNode *TempNode2 = OtherList->FirstNode;
			for(int i=0; (unsigned int)i<Length; i++)
			{
				if(TempNode1->Value != TempNode2->Value)
					return FALSE;
				TempNode1 = TempNode1->Next;
				TempNode2 = TempNode2->Next;
			}
			return TRUE;
		}
	}
	else
	{
		return FALSE;
	}
}

//	IsLessThan - check inequality with other list
//	Checks nodewise - earlier nodes take prominence
bool c1DList :: IsLessThan(c1DList *OtherList)
{
	if(Length != OtherList->Length)
		return FALSE; // Actually, undefined
	else
	{
		t1DNode *TempNode1 = FirstNode;
		t1DNode *TempNode2 = OtherList->FirstNode;
		for(int i=0; (unsigned int)i<Length; i++)
		{
			if(TempNode1->Value < TempNode2->Value)
				return TRUE;
			else if(TempNode1->Value > TempNode2->Value)
				return FALSE;
			// Else, if they're equal, check the next node
			TempNode1 = TempNode1->Next;
			TempNode2 = TempNode2->Next;
		}
		return FALSE; // If they're equal
	}
}

//	IsGreaterThan - check inequality with other list
//	Checks nodewise - earlier nodes take prominence
bool c1DList :: IsGreaterThan(c1DList *OtherList)
{
	if(Length != OtherList->Length)
		return FALSE; // Actually, undefined
	else
	{
		t1DNode *TempNode1 = FirstNode;
		t1DNode *TempNode2 = OtherList->FirstNode;
		for(int i=0; (unsigned int)i<Length; i++)
		{
			if(TempNode1->Value > TempNode2->Value)
				return TRUE;
			else if(TempNode1->Value < TempNode2->Value)
				return FALSE;
			// Else, if they're equal, check the next node
			TempNode1 = TempNode1->Next;
			TempNode2 = TempNode2->Next;
		}
		return FALSE; // If they're equal
	}
}

//	Overloaded '=' for another object pointer
c1DList *c1DList :: operator = (c1DList *OtherList)
{
	Assign(OtherList);
	return this;
}

//	Overloaded '=' for another object
c1DList *c1DList :: operator = (c1DList OtherList)
{
	Assign(&OtherList);
	return this;
}

//	Overloaded '==' for another object pointer
bool c1DList :: operator == (c1DList *OtherList)
{
	return Equals(OtherList);
}

//	Overloaded '==' for another object
bool c1DList :: operator == (c1DList OtherList)
{
	return Equals(&OtherList);
}

//	Overloaded '<' for another object pointer
bool c1DList :: operator < (c1DList *OtherList)
{
	return IsLessThan(OtherList);
}

//	Overloaded '<' for another object
bool c1DList :: operator < (c1DList OtherList)
{
	return IsLessThan(&OtherList);
}

//	Overloaded '>' for another object pointer
bool c1DList :: operator > (c1DList *OtherList)
{
	return IsGreaterThan(OtherList);
}

//	Overloaded '>' for another object
bool c1DList :: operator > (c1DList OtherList)
{
	return IsGreaterThan(&OtherList);
}

//	Overloaded '<=' for another object pointer
bool c1DList :: operator <= (c1DList *OtherList)
{
	return !IsGreaterThan(OtherList);
}

//	Overloaded '<=' for another object
bool c1DList :: operator <= (c1DList OtherList)
{
	return !IsGreaterThan(&OtherList);
}

//	Overloaded '>=' for another object pointer
bool c1DList :: operator >= (c1DList *OtherList)
{
	return !IsLessThan(OtherList);
}

//	Overloaded '>=' for another object
bool c1DList :: operator >= (c1DList OtherList)
{
	return !IsLessThan(&OtherList);
}

//	Overloaded '[]'
float & c1DList :: operator [] (const unsigned int Index)
{
	t1DNode *TempNode = FirstNode;
	for(int i=0; (unsigned int)i<Index && i<(int)Length-1; i++)
	{
		TempNode = TempNode->Next;
	}
	if(TempNode!=NULL)
		return TempNode->Value;
	else
	{
		throw("Invalid index"); // TODO: Proper handling of this; causes crash sometimes
	}
}