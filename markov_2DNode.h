/*
 *  markov_2DNode.h
 *  markov
 *
 *  Created by Trishul Mallikarjuna on 2/7/10.
 *  Copyright 2010 Georgia Tech Center for Music Technology. All rights reserved.
 *
 */

//	Node of a 2D linked list / Tree
class t2DNode
	{
		
	private:
		
		t2DNode				*Left; // Link to the 2D node to the left
		t2DNode				*Right; // Link to the 2D node to the right
		tBundle2D			*Bundle; // Link to the bundle (of 2D nodes) that contains this 2D node
		tBundle2D			*Bottom; // Link to the child bundle
		c1DList				Vector; // Vector of symbol values
		unsigned long int	Count; // Number of times the node has been observed in the input
		double				Prob; // Corresponding probability of occurrence - this is the a_ij(t)
		
		friend class t2DNodeHidden;
		friend class tBundle2D;
		friend class tBundle2DHidden;
		friend class cViewPoint;
		friend class cViewPointHidden;
		friend class cQaidaControl;
		friend class _markov;
		
	public:
		
		//	Constructor
		t2DNode(t2DNode *Left, t2DNode *Right, tBundle2D *Bundle, tBundle2D *Bottom, c1DList *Vector, unsigned long int Count, double Prob);
		
		//	Destructor
		~t2DNode();
		
	};
