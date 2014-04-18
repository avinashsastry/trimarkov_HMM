/*
 *  markov_1DList.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/13/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

//	Linked list of input note stream
class c1DList
	{
		
	private:
		
		t1DNode			* FirstNode; // Link to first node
		t1DNode			* LastNode; // Link to last node
		unsigned int	Length; // Number of nodes in the stream
		
		friend class c1DVectorNode;
		friend class c1DVectorList;
		friend class t2DNode;
		friend class tBundle2D;
		friend class cQaidaControl;
		friend class _markov;
	
	public:
		
		c1DList();
		
		c1DList(const c1DList &OtherList);
		
		//	Constructor
		c1DList(t1DNode *FirstNode, t1DNode *LastNode, unsigned int Length);
		
		//	Destructor
		~c1DList();
		
		//	GetValue - return the value at given index
		float GetValue(unsigned int Index);
		
		//	SetValue - set the value at given index
		int SetValue(unsigned int Index, float Value);
		
		//	GetLength - return the length of the list
		unsigned int GetLength(void);
		
		//	Append - append a node at the end of the list
		int	Append(float Value);
		
		//	Delete - delete a node
		int Delete(unsigned int Index);
		
		//	DeleteAll - delete all nodes
		int DeleteAll(void);
		
		//	AppendAll - append all nodes from another c1DList; in effect, combine them
		int	AppendAll(c1DList *OtherList);
		
		//	Assign - copy list from another c1DList
		int	Assign(c1DList *OtherList);
		
		//	Equals - check equality with other list
		bool Equals(c1DList *OtherList);
		
		//	IsLessThan - check inequality with other list
		//	Checks nodewise - earlier nodes take prominence
		bool IsLessThan(c1DList *OtherList);
		
		//	IsGreaterThan - check inequality with other list
		//	Checks nodewise - earlier nodes take prominence
		bool IsGreaterThan(c1DList *OtherList);
		
		//	Overloaded '=' for another pointer
		c1DList *operator = (c1DList *OtherList);
		
		//	Overloaded '=' for another object
		c1DList *operator = (c1DList OtherList);

		//	Overloaded '==' for another object pointer
		bool operator == (c1DList *OtherList);
		
		//	Overloaded '==' for another object
		bool operator == (c1DList OtherList);
		
		//	Overloaded '<' for another object pointer
		bool operator < (c1DList *OtherList);
		
		//	Overloaded '<' for another object
		bool operator < (c1DList OtherList);
		
		//	Overloaded '>' for another object pointer
		bool operator > (c1DList *OtherList);
		
		//	Overloaded '>' for another object
		bool operator > (c1DList OtherList);
		
		//	Overloaded '<=' for another object pointer
		bool operator <= (c1DList *OtherList);
		
		//	Overloaded '<=' for another object
		bool operator <= (c1DList OtherList);
		
		//	Overloaded '>=' for another object pointer
		bool operator >= (c1DList *OtherList);
		
		//	Overloaded '>=' for another object
		bool operator >= (c1DList OtherList);
		
		//	Overloaded '[]'
		float &operator [] (const unsigned int Index);
		
	};
