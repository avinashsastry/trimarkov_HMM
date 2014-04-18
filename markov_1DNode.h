/*
 *  markov_1DNode.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/7/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

//	Node of a 1D linked list / Note
class t1DNode
	{
		
	private:
		
		float Value; // Node symbol value
		t1DNode *Prev; // Link to previous node
		t1DNode *Next; // Link to next node
		
		friend class c1DList;
		friend class c1DVectorList;
		friend class tBundle2D;
		friend class _markov;
		
	public:
		
		//	Constructor
		t1DNode(float Value, t1DNode *Prev, t1DNode *Next);
		
		//	Destructor
		~t1DNode();
		
	};
